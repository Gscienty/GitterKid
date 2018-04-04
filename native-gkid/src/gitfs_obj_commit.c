#include "gitfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

struct gitperson *__transfer_person_log (const char *ch) {
        char *fields[4];
        register int i;
        for (i = 0; ch && i < 4; i++) {
            fields[i] = (char *) ch;
            char *space_ptr = strchr (ch, ' ');
            if (space_ptr != NULL) {
                *space_ptr = 0;
                ch = space_ptr + 1;
            }
            else {
                ch = NULL;
            }
        }
        if (i != 4) {
            //author format error
            DBG_LOG (DBG_ERROR, "author format error");
            return NULL;
        }

        struct gitperson *ret = (struct gitperson *) malloc (sizeof (*ret));
        ret->name = fields[0];
        ret->mail = fields[1];
        char *ep;
        if ((ret->timestamp = strtol (fields[2], &ep, 10)) == 0 && *ep) {
            // timestamp error;
            DBG_LOG(DBG_ERROR, "timestamp format error");
            free (ret);
            return NULL;
        }

        ret->timezone = fields[3];

        return ret;
}

struct gitobj_commit *__transfer_commit (struct __bytes bytes) {
    struct gitobj_commit *ret = (struct gitobj_commit *) malloc (sizeof (*ret));
    if (ret == NULL) {
        // have not enough free memory
        DBG_LOG (DBG_ERROR, "get_gitobj_commit: have not enough free memory");
        return NULL;
    }
    ret->parent_head = ret->parent_tail = ret->parent_cursor = NULL;
    ret->tree_sign = ret->message = NULL;
    ret->author = ret->committer = NULL;

    register unsigned char *ch;
    const unsigned char *top = bytes.buf + bytes.len;
    for (ch = bytes.buf; *ch && ch < top;) {
        unsigned char *end_ptr = strchr (ch, '\n');
        if (end_ptr == NULL) {
            break;
        }
        *end_ptr = 0;
        if (end_ptr == ch) {
            ret->message = ch + 1;
            break;
        }
        char *space_ptr = strchr (ch, ' ');
        *space_ptr = 0;
        if (strcmp (ch, "tree") == 0) {
            ch = space_ptr + 1;
            ret->tree_sign = ch;
        }
        else if (strcmp (ch, "author") == 0) {
            struct gitperson *author = __transfer_person_log (space_ptr + 1);
            if (author == NULL) {
                // cannnot transfer author
                DBG_LOG (DBG_ERROR, "get_gitobj_commit: author format error");
                __gitobj_commit_dtor (ret);
                return NULL;
            }
            ret->author = author;
        }
        else if (strcmp (ch, "committer") == 0) {
            struct gitperson *committer = __transfer_person_log (space_ptr + 1);
            if (committer == NULL) {
                // cannnot transfer committer
                DBG_LOG (DBG_ERROR, "get_gitobj_commit: committer format error");
                __gitobj_commit_dtor (ret);
                return NULL;
            }
            ret->committer = committer;
        }
        else if (strcmp (ch, "parent") == 0) {
            ch = space_ptr + 1;
            struct gitobj_commitpatent *parent = (struct gitobj_commitpatent *) malloc (sizeof (*parent));
            if (parent == NULL) {
                // not enough free memory
                DBG_LOG (DBG_ERROR, "get_gitobj_commit: not enough free memory");
                __gitobj_commit_dtor (ret);
                return NULL;
            }
            parent->sign = ch;
            parent->prev = ret->parent_tail;
            parent->next = NULL;
            if (ret->parent_head == NULL) {
                ret->parent_head = parent;
            }
            if (ret->parent_tail != NULL) {
                ret->parent_tail->next = parent;
            }
            ret->parent_tail = parent;
        }

        ch = end_ptr + 1;
    }
    return ret;
}

struct gitobj *__packitem_transfer_commit (struct __gitpack_item item) {
    struct gitobj *ret = (struct gitobj *) malloc (sizeof (*ret));
    if (ret == NULL) {
        DBG_LOG (DBG_ERROR, "__packitem_transfer_commit: have not enough free memory");
        return NULL;
    }

    ret->buf = item.bytes.buf;
    ret->path = NULL;
    ret->sign = NULL;
    ret->type = GIT_OBJ_TYPE_COMMIT;
    ret->size = item.bytes.len;
    ret->body = item.bytes.buf;
    ret->ptr = __transfer_commit (item.bytes);

    return ret;
}

void __gitobj_commit_dtor (struct gitobj_commit *obj) {
    if (obj == NULL) {
        return ;
    }

    if (obj->author != NULL) {
        free (obj->author);
    }
    if (obj->committer != NULL) {
        free (obj->committer);
    }
    if (obj->parent_head != NULL) {
        while (obj->parent_head) {
            struct gitobj_commitpatent *next = obj->parent_head->next;
            free (obj->parent_head);
            obj->parent_head = next;
        }
    }

    free (obj);
}

struct gitobj_commit *gitobj_get_commit (struct gitobj *obj) {
    if (obj == NULL) {
        DBG_LOG (DBG_ERROR, "gitobj_get_commit: object is null");
        return NULL;
    }

    if (obj->type == GIT_OBJ_TYPE_COMMIT) {
        return (struct gitobj_commit *) obj->ptr;
    }
    else {
        DBG_LOG (DBG_ERROR, "gitobj_get_commit: object's type error");
        return NULL;
    }
}

char *gitobj_commit_get_treesign (struct gitobj_commit *commit_obj) {
    if (commit_obj == NULL) {
        DBG_LOG (DBG_ERROR, "gitobj_commit_get_treesign: commit object is null");
        return NULL;
    }
    return commit_obj->tree_sign;
}

struct gitperson *gitobj_commit_get_author (struct gitobj_commit *commit_obj) {
    if (commit_obj == NULL) {
        DBG_LOG (DBG_ERROR, "gitobj_commit_get_author: commit object is null");
        return NULL;
    }
    return commit_obj->author;
}

struct gitperson *gitobj_commit_get_committer (struct gitobj_commit *commit_obj) {
    if (commit_obj == NULL) {
        DBG_LOG (DBG_ERROR, "gitobj_commit_get_committer: commit object is null");
        return NULL;
    }
    return commit_obj->committer;
}

char *gitobj_commit_get_message (struct gitobj_commit *commit_obj) {
    if (commit_obj == NULL) {
        DBG_LOG (DBG_ERROR, "gitobj_commit_get_message: commit object is null");
        return NULL;
    }
    return commit_obj->message;
}

void gitobj_commitparents_reset (struct gitobj_commit *commit_obj) {
    if (commit_obj == NULL) {
        DBG_LOG (DBG_ERROR, "gitobj_commitparents_reset: commit object is null");
        return;
    }
    commit_obj->parent_cursor = NULL;
}

int gitobj_commitparents_hasnext (struct gitobj_commit *commit_obj) {
    if (commit_obj == NULL) {
        DBG_LOG (DBG_ERROR, "gitobj_commitparents_hasnext: commit object is null");
        return 0;
    }
    
    return (commit_obj->parent_cursor == NULL ? commit_obj->parent_head : commit_obj->parent_cursor->next) != NULL;
}

struct gitobj_commitpatent *gitobj_commitparents_next (struct gitobj_commit *commit_obj) {
    if (commit_obj == NULL) {
        DBG_LOG (DBG_ERROR, "gitobj_commitparents_next: commit object is null");
        return NULL;
    }

    return (commit_obj->parent_cursor = (commit_obj->parent_cursor == NULL ? commit_obj->parent_head : commit_obj->parent_cursor->next));
}

char *gitobj_commitpatent_get_sign (struct gitobj_commitpatent *commit_parent_obj) {
    if (commit_parent_obj == NULL) {
        DBG_LOG (DBG_ERROR, "gitobj_commitpatent_get_sign: commit's parent object is null");
        return NULL;
    }
    return commit_parent_obj->sign;
}

char *gitperson_get_name (struct gitperson *person_log) {
    if (person_log == NULL) {
        DBG_LOG (DBG_ERROR, "gitperson_get_name: person log object is null");
        return NULL;
    }
    return person_log->name;
}

char *gitperson_get_mail (struct gitperson *person_log) {
    if (person_log == NULL) {
        DBG_LOG (DBG_ERROR, "gitperson_get_mail: person log object is null");
        return NULL;
    }
    return person_log->mail;
}

unsigned long gitperson_get_timestamp (struct gitperson *person_log) {
    if (person_log == NULL) {
        DBG_LOG (DBG_ERROR, "gitperson_timestamp: person log object is null");
        return 0;
    }
    return person_log->timestamp;
}

char *gitperson_get_timezone (struct gitperson *person_log) {
    if (person_log == NULL) {
        DBG_LOG (DBG_ERROR, "git_person_timezone: person log object is null");
        return NULL;
    }
    return person_log->timezone;
}