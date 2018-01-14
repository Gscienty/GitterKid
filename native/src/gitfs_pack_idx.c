#include "gitfs.h"
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

DIR *__gitpack_packdir_get (const char *path, size_t path_len) {
    char *pack_path = (char *) malloc (sizeof (char) * (path_len + 14));
    if (pack_path == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_packdir_get: have not enough free memory");
        return NULL;
    }

    strcpy (pack_path, path);
    strcpy (pack_path + path_len, "objects/pack/");
    
    DIR *ret = opendir (pack_path);

    free (pack_path);

    return ret;
}

struct __gitpack *__gitpack_get (const char *path, size_t path_len, const char *idx_name) {
    struct __gitpack *ret = (struct __gitpack *) malloc (sizeof (*ret));
    if (ret == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_get: have not enough free memory");
        return NULL;
    }

    ret->sign = (char *) malloc (sizeof (char) * 41);
    if (ret->sign == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_get: have not enough free memory");
        free (ret);
        return NULL;
    }
    strncpy (ret->sign, idx_name + 5, 40);
    
    ret->idx_path = (char *) malloc (sizeof (char) * (path_len + 63));
    if (ret->idx_path == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_get: have not enough free memory");
        free (ret->sign);
        free (ret);
        return NULL;
    }
    strcpy (ret->idx_path, path);
    strcpy (ret->idx_path + path_len, "objects/pack/pack-");
    strcpy (ret->idx_path + path_len + 18, ret->sign);
    strcpy (ret->idx_path + path_len + 58, ".idx");

    ret->pack_path = (char *) malloc (path_len + 64);
    if (path == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_packfile_path_get: have not enough free memory");
        free (ret->sign);
        free (ret->idx_path);
        free (ret);
        return NULL;
    }
    strcpy (ret->pack_path, path);
    strcpy (ret->pack_path + path_len, "objects/pack/pack-");
    strncpy (ret->pack_path + path_len + 18, ret->sign, 40);
    strcpy (ret->pack_path + path_len + 58, ".pack");

    ret->count = 0;
    ret->rd_tree = NULL;
    ret->prev = ret->next = NULL;
    return ret;
}

void __gitpack_collection_dispose (struct __gitpack_collection *obj) {
    if (obj == NULL) {
        return ;
    }

    // TODO: dispose
}

struct __opend_mmap_file {
    int fd;
    unsigned char *val;
    size_t len;
};

void __gitpack_idxfile_close (struct __opend_mmap_file *mmaped) {
    if (mmaped == NULL) return;
    munmap (mmaped->val, mmaped->len);
    close (mmaped->fd);
}

struct __opend_mmap_file *__gitpack_idxfile_open (struct __gitpack *pack) {
    if (pack == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_idxfile_open: pack is null");
        return NULL;
    }
    struct __opend_mmap_file *ret = (struct __opend_mmap_file *) malloc (sizeof (ret));
    if (ret == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_idxfile_open: have not enough free memory");
        return NULL;
    }

    ret->fd = open (pack->idx_path, O_RDONLY);
    if (ret->fd == -1) {
        DBG_LOG (DBG_ERROR, "__gitpack_idxfile_open: idx file cannot opend");
        free (ret);
        return NULL;
    }
    struct stat idx_st;
    if (fstat (ret->fd, &idx_st) != 0) {
        DBG_LOG (DBG_ERROR, "__gitpack_idxfile_open: idx file cannot get st");
        close (ret->fd);
        free (ret);
        return NULL;
    }
    ret->len = idx_st.st_size;
    ret->val = mmap (NULL, idx_st.st_size, PROT_READ, MAP_SHARED, ret->fd, 0);
    if ((void *) ret->val == (void *) -1) {
        DBG_LOG (DBG_ERROR, "__gitpack_idxfile_open: idx file cannot mmap");
        close (ret->fd);
        free (ret);
        return NULL;
    }

    return ret;
}

int __gitpack_item_count_get (struct __opend_mmap_file *mmaped) {
    if (mmaped == NULL) return 0;
    unsigned int *p = (unsigned int *) (mmaped->val + 8);
    int count = 0;
    int i;
    for (i = 0; i < 256; i++) {
        int n = ntohl (p[i]);
        if (n > count) count = n;
    }

    return count;
}

#define __GITPACK_NTH_SIGN(val, n) ((val) + 8 + 1024 + 20 * (n))
#define __GITPACK_NTH_OFF(val, nr, n) ntohl (*((int *) ((val) + 8 + 1024 + 24 * (nr) + 4 * (n))))

void __gitpack_quicksort_indexes (unsigned char *val, struct __gitpack *pack, int *indexes, int start, int end) {
    if (start >= end) return;

    int i = start;
    int j = end;
    int k = indexes[start];

    while (i < j) {
        while (i < j && __GITPACK_NTH_OFF (val, pack->count, k) < __GITPACK_NTH_OFF (val, pack->count, indexes[j])) j--;
        indexes[i] = indexes[j];
        while (i < j && __GITPACK_NTH_OFF (val, pack->count, indexes[i]) < __GITPACK_NTH_OFF (val, pack->count, k)) i++;
        indexes[j] = indexes[i];
    }
    indexes[i] = k;

    __gitpack_quicksort_indexes (val, pack, indexes, start, i - 1);
    __gitpack_quicksort_indexes (val, pack, indexes, i + 1, end);
}

int *__gitpack_sortedindexes_get (unsigned char *val, struct __gitpack *pack) {
    int *ret = (int *) malloc (sizeof (*ret) * pack->count);
    int i = 0;
    for (i = 0; i < pack->count; i++) ret[i] = i;

    __gitpack_quicksort_indexes (val, pack, ret, 0, pack->count - 1);

    return ret;
}

void * __sign_dup (void *off) {
    void *ret = (void *) malloc (20);
    if (ret == NULL) {
        DBG_LOG (DBG_ERROR, "__sign_dup: have not enough free memory");
        return NULL;
    }
    
    memcpy (ret, off, 20);

    return ret;
}

struct rdt *__gitpack_rdt_build (struct __opend_mmap_file *mmaped, struct __gitpack *pack, size_t packsize) {
    if (pack == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_rdt_build: pack is null");
        return NULL;
    }
    if (mmaped == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_rdt_build: mmaped is null");
        return NULL;
    }
    struct rdt *ret = rdt_build ();

    int *indexes = __gitpack_sortedindexes_get (mmaped->val, pack);
    int i = 0;
    for (i = 0; i < pack->count; i++) {
        rdt_insert (
            ret,
            __sign_dup (__GITPACK_NTH_SIGN (mmaped->val, indexes[i])),
            __GITPACK_NTH_OFF (mmaped->val, pack->count, indexes[i]),
            i + 1 == pack->count
                ? packsize - 20 - __GITPACK_NTH_OFF (mmaped->val, pack->count, indexes[i])
                : __GITPACK_NTH_OFF (mmaped->val, pack->count, indexes[i + 1]) - __GITPACK_NTH_OFF (mmaped->val, pack->count, indexes[i])
        );
    }
    free (indexes);

    return ret;
}

size_t __gitpack_size_get (struct __gitpack *pack) {
    struct stat st;
    if (stat (pack->pack_path, &st) != 0) {
        return 0;
    }
    return st.st_size;
}

struct __gitpack_collection *__gitpack_collection_get (struct git_repo *repo) {
    if (repo == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_collection_get: repo is null");
        return NULL;
    }
    struct __gitpack_collection *ret = (struct __gitpack_collection *) malloc (sizeof (*ret));
    if (ret == NULL) {
        DBG_LOG (DBG_ERROR, "__gitpack_collection_get: have not enough free memory");
        return NULL;
    }
    ret->head = ret->tail = NULL;

    size_t path_len = strlen (repo->path);

    DIR *dir = __gitpack_packdir_get (repo->path, path_len);
    if (dir == NULL) {
        free (ret);
        return NULL;
    }

    struct dirent *ent;
    while ((ent = readdir (dir))) {
        if (ent->d_type == DT_REG) {
            if (strcmp (ent->d_name + 45, ".idx") != 0) continue;

            struct __gitpack *pack = __gitpack_get (repo->path, path_len, ent->d_name);
            if (pack == NULL) {
                __gitpack_collection_dispose (ret);
                closedir (dir);
                return NULL;
            }
            

            struct __opend_mmap_file *idx_mmaped = __gitpack_idxfile_open (pack);
            if (idx_mmaped == NULL) {
                __gitpack_collection_dispose (ret);
                closedir (dir);
                return NULL;
            }

            // get count
            pack->count = __gitpack_item_count_get (idx_mmaped);
            // build red black tree
            pack->rd_tree = __gitpack_rdt_build (idx_mmaped, pack, __gitpack_size_get (pack));

            __gitpack_idxfile_close (idx_mmaped);

            if (ret->head == NULL) ret->head = pack;
            if (ret->tail != NULL) ret->tail->next = pack;
            ret->tail = pack;
        }
    }
    closedir (dir);

    return ret;
}