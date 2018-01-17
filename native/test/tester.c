#include "user.h"
#include "gitfs.h"
#include <stdio.h>
#include <string.h>

int test_enum_passwd () {
    struct db *db = build_passwd_handle ();
    open_passwd (db);

    reset_passwd_cursor (db);

    do {
        struct passwd *pw = get_current_passwd (db);
        printf ("%d:%s:%d:%s:%s:%s\n",
            get_passwd_uid (pw),
            get_passwd_username (pw),
            get_passwd_gid (pw),
            get_passwd_gecos (pw),
            get_passwd_dir (pw),
            get_passwd_shell (pw)
        );
    } while (move_passwd_cursor_next (db) == 0);

    dtor_passwd (db);

    return 0;
}

int test_enum_group () {
    struct db *db = build_group_handle ();
    open_group (db);

    reset_group_cursor (db);
    do {
        struct group *grp = get_current_group (db);
        printf ("%s:%s:%d\n",
            get_group_name (grp),
            get_group_passwd (grp),
            get_group_gid (grp)
        );
        
        struct group_member *mem = get_group_member_cursor (grp);
        reset_group_member_cursor (mem);
        do {
            printf ("\t\t%s\t%d\n", get_current_group_member_name (mem), get_group_member_count (mem));
        } while (group_member_movenext (mem) == 0);

        dtor_group_member (mem);

    } while (move_group_cursor_next (db) == 0);

    dtor_group (db);

    return 0;
}

int test_market_init () {
    struct gitmarket *market = get_gitmarket("/home/ant");

    gitmarket_reset (market);

    while (gitmarket_hasnext (market)) {
        struct gitrepo *repo = gitmarket_next (market);
        
        if (strcmp (gitrepo_get_name (repo), "repo") == 0) {
            struct gitobj *obj = gitrepo_get_gitobj (repo, "ce013625030ba8dba906f756967f9e9ca394464a");
        }
    }
    return 0;
}

int test_create_account () {
    struct db *db = build_passwd_handle ();
    printf ("open_passwd: %d\n", open_passwd (db));
    printf ("%d\n", create_account (db, "test", "/home/test", "/bin/sh", 1000));
    return 0;
}


int test_branches () {
    struct gitmarket *market = get_gitmarket ("/home/ant");

    gitmarket_reset (market);

    while (gitmarket_hasnext (market)) {
        struct gitrepo *repo = gitmarket_next (market);
        
        if (strcmp (gitrepo_get_name (repo), "gitterRepo") == 0) {
            struct gitbranches *branches = gitrepo_get_branches (repo);
            gitbranches_reset (branches);

            while (gitbranches_hasnext (branches)) {
                struct gitbranch *branch = gitbranches_next (branches);
                printf ("%s\t%s\n", 
                    gitbranch_get_name (branch),
                    gitbranch_get_lastcommit_sign (branch)
                );
            }
            break;
        }
    }
    return 0;
}

int test_commits_parent () {
    //659cba9dc3f15adc2ba51f0fb66b92ef7127186d
    struct gitmarket *market = get_gitmarket ("/home/ant");

    gitmarket_reset (market);

    while (gitmarket_hasnext (market)) {
        struct gitrepo *repo = gitmarket_next (market);
        
        if (strcmp (gitrepo_get_name (repo), "gitterRepo") == 0) {
            struct gitobj *obj = gitrepo_get_gitobj (repo, "54415a12532e7e77dfc4037962e870abe72e0300");
            struct gitobj_commit *commit = gitobj_get_commit (obj);

            gitobj_commit_patent_reset (commit);
            do {
                struct gitobj_commit_patent *parent = gitobj_commit_patent_current (commit);
                printf ("%s\n", gitobj_commit_patent_sign (parent));
            } while (gitobj_commit_patent_movenext (commit) == 0);

            break;
        }
    }
    return 0;
}

int test_pack () {
    struct gitmarket *market = get_gitmarket ("/home/ant");
    gitmarket_reset (market);

    while (gitmarket_hasnext (market)) {
        struct gitrepo *repo = gitmarket_next (market);
        if (strcmp (gitrepo_get_name (repo), "gitterRepo") == 0) {
            // printf ("TYPE: %d\n", get_gitobj (repo, "6074c189b9f709892de2f413f08b1be6b9eab0af")->type);
            // printf ("TYPE: %d\n", get_gitobj (repo, "1aea93543e697af2f00499c53116bbb3cc8ffcbf")->type);
            // printf ("TYPE: %d\n", get_gitobj (repo, "ec4735f69b5d60b91d95634f511200ad37486ba4")->type);
            // printf ("TYPE: %d\n", get_gitobj (repo, "e87a0306f7cf5ef89dec674bc2ecaba08ef80eb6")->type);
            struct gitobj *ret = gitrepo_get_gitobj (repo, "997cc5e85aff5cc35e317cd993f9979c858b17fa");
            // int i;
            // for (i = 0; i < ret->size; i++) putchar (ret->body[i]);
            //struct __gitpack_collection *pack_coll = __gitpack_collection_get (repo);

            //printf ("%d\n", __gitpack_getobj__charstring (pack_coll, "3b846780303b8994088f884c3dfed48c04e6a2b0"));

            //     __git_pack_get_obj (git_packes_get_current (packes), "0f45d6566321ef72e498d1e289398fab1fd5026f");
            //     __git_pack_get_obj (git_packes_get_current (packes), "ca1c10bf904926224e1358910d637320e91945d3");

            //     // __git_pack_get_obj (git_packes_get_current (packes), "e53ee7b4ea4576d131fdce313a4c5204bd8d0eec");
            //     // __git_pack_get_obj (git_packes_get_current (packes), "c1a37f7eacfbd0c1d6d3250b92e81999e99fb1c7");

            //     // __git_pack_get_obj (git_packes_get_current (packes), "fbfba1e8bf83a2b241e29b93069eb8492e794f50");
            //     // __git_pack_get_obj (git_packes_get_current (packes), "d139fedcdb55b4f266db2abecac304cafadc2e29");

        }
    }
    return 0;
}

int main() {
    // test_enum_passwd ();
    // test_enum_group ();
    // test_user_create_account ();
    // test_market_init ();
    // test_create_account ();
    // test_branches ();
    // test_commits_parent ();
    test_pack ();
    return 0;
}