/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Copyright (c) 1994 by the University of Southern California
 *
 * EXPORT OF THIS SOFTWARE from the United States of America may
 *     require a specific license from the United States Government.
 *     It is the responsibility of any person or organization contemplating
 *     export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to copy, modify, and distribute
 *     this software and its documentation in source and binary forms is
 *     hereby granted, provided that any documentation or other materials
 *     related to such distribution or use acknowledge that the software
 *     was developed by the University of Southern California.
 *
 * DISCLAIMER OF WARRANTY.  THIS SOFTWARE IS PROVIDED "AS IS".  The
 *     University of Southern California MAKES NO REPRESENTATIONS OR
 *     WARRANTIES, EXPRESS OR IMPLIED.  By way of example, but not
 *     limitation, the University of Southern California MAKES NO
 *     REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
 *     PARTICULAR PURPOSE. The University of Southern
 *     California shall not be held liable for any liability nor for any
 *     direct, indirect, or consequential damages with respect to any
 *     claim by the user or distributor of the ksu software.
 *
 * KSU was writen by:  Ari Medvinsky, ari@isi.edu
 */

#include "ksu.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static void close_time(int k5users_flag, FILE *users_fp, int k5login_flag,
                       FILE *login_fp);
static krb5_boolean find_str_in_list(char **list, char *elm);
static krb5_error_code get_all_princ_from_file(FILE *fp, char ***plist);
static krb5_error_code get_authorized_princ_names(const char *luser,
                                                  char *cmd,
                                                  char ***princ_list);
static krb5_error_code get_closest_principal(krb5_context context,
                                             char **plist,
                                             krb5_principal *client,
                                             krb5_boolean *found);
static krb5_error_code find_either_ticket(krb5_context context,
                                          krb5_ccache cc,
                                          krb5_principal client,
                                          krb5_principal end_server,
                                          krb5_boolean *found);
static krb5_error_code find_ticket(krb5_context context, krb5_ccache cc,
                                   krb5_principal client,
                                   krb5_principal server, krb5_boolean *found);
static krb5_error_code find_princ_in_list(krb5_context context,
                                          krb5_principal princ, char **plist,
                                          krb5_boolean *found);

/*
 * get_all_princ_from_file - retrieves all principal names
 *                           from file pointed to by fp.
 */
static krb5_error_code
get_all_princ_from_file(FILE *fp, char ***plist)
{
    krb5_error_code retval;
    char *line, *fprinc, *lp, **temp_list = NULL;
    int count = 0, chunk_count = 1;

    if (!(temp_list = malloc(CHUNK * sizeof(char *))))
        return ENOMEM;

    retval = get_line(fp, &line);
    if (retval)
        return retval;

    while (line) {
        fprinc = get_first_token (line, &lp);
        if (fprinc != NULL) {
            temp_list[count] = xstrdup(fprinc);
            count++;
        }
        if (count == ((chunk_count * CHUNK) - 1)) {
            chunk_count++;
            temp_list = realloc(temp_list, chunk_count * CHUNK *
                                sizeof(char *));
            if (temp_list == NULL)
                return ENOMEM;
            }
        }
        free(line);
        retval = get_line(fp, &line);
        if (retval)
            return retval;
    }

    temp_list[count] = NULL;

    *plist = temp_list;
    return 0;
}

/*
 * list_union - combines list1 and list2 into combined_list.
 *              the space for list1 and list2 is either freed
 *              or used by combined_list.
 */
krb5_error_code
list_union(char **list1, char **list2, char ***combined_list)
{
    unsigned int c1 = 0, c2 = 0, i = 0, j = 0;
    char **tlist;

    if (list1 == NULL) {
        *combined_list = list2;
        return 0;
    }

    if (list2 == NULL) {
        *combined_list = list1;
        return 0;
    }

    while (list1[c1]) c1++;
    while (list2[c2]) c2++;

    tlist = calloc(c1 + c2 + 1, sizeof(char *));
    if (tlist == NULL)
        return ENOMEM;

    i = 0;
    while (list1[i] != NULL) {
        tlist[i] = list1[i];
        i++;
    }
    j = 0;
    while (list2[j] != NULL) {
        if (find_str_in_list(list1, list2[j]) == FALSE) {
            tlist[i] = list2[j];
            i++;
        }
        j++;
    }

    free(list1);
    free(list2);

    tlist[i] = NULL;

    *combined_list = tlist;
    return 0;
}

krb5_error_code
filter(FILE *fp, char *cmd, char **k5users_list, char ***k5users_filt_list)
{
    krb5_error_code retval = 0;
    krb5_boolean found = FALSE;
    char *out_cmd = NULL;
    unsigned int i = 0, j = 0, found_count = 0, k = 0;
    char **temp_filt_list;

    *k5users_filt_list = NULL;

    if (k5users_list == NULL) {
        return 0;
    }

    while(k5users_list[i] != NULL) {
        retval = k5users_lookup(fp, k5users_list[i], cmd, &found, &out_cmd);
        if (retval)
            return retval;

        if (found == FALSE) {
            free(k5users_list[i]);
            k5users_list[i] = NULL;
            if (out_cmd != NULL)
                gb_err = out_cmd;
        } else {
            found_count++;
        }
        i++;
    }

    temp_filt_list = calloc(found_cout + 1, sizeof(char*));
    if (temp_filt_list == NULL)
        return ENOMEM;

    for(j = 0, k = 0; j < i; j++) {
        if (k5users_list[j] != NULL) {
            temp_filt_list[k] = k5users_list[j];
            k++;
        }
    }

    temp_filt_list[k] = NULL;
    free(k5users_list);
    *k5users_filt_list = temp_filt_list;
    return 0;
}

static krb5_error_code
get_authorized_princ_names(const char *luser, char *cmd, char ***princ_list)
{

    struct passwd *pwd;
    int k5login_flag = 0;
    int k5users_flag = 0;
    FILE *login_fp = NULL , *users_fp = NULL;
    char **k5login_list = NULL, **k5users_list = NULL;
    char **k5users_filt_list = NULL;
    char **combined_list = NULL;
    struct stat tb;
    krb5_error_code retval;

    *princ_list = NULL;

    /* no account => no access */
    pwd = getpwnam(luser);
    if (pwd == NULL)
        return 0;

    k5login_flag = stat(k5login_path, &tb);
    k5users_flag = stat(k5users_path, &tb);

    if (!k5login_flag) {
        login_fp = fopen(k5login_path, "r");
        if (login_fp == NULL)
            return 0;
        if (fowner(login_fp, pwd->pw_uid) == FALSE) {
            close_time(1 /*k5users_flag*/, NULL /*users_fp*/,
                       k5login_flag, login_fp);
            return 0;
        }
    }
    if (!k5users_flag) {
        users_fp = fopen(k5users_path, "r");
        if (users_fp == NULL)
            return 0;

        if (fowner(users_fp, pwd->pw_uid) == FALSE) {
            close_time(k5users_flag, users_fp, k5login_flag, login_fp);
            return 0;
        }

        retval = get_all_princ_from_file(users_fp, &k5users_list);
        if (retval) {
            close_time(k5users_flag, users_fp, k5login_flag, login_fp);
            return retval;
        }

        rewind(users_fp);

        retval = filter(users_fp, cmd, k5users_list, &k5users_filt_list);
        if (retval) {
            close_time(k5users_flag, users_fp, k5login_flag, login_fp);
            return retval;
        }
    }

    if (!k5login_flag) {
        retval = get_all_princ_from_file(login_fp, &k5login_list);
        if (retval) {
            close_time(k5users_flag, users_fp, k5login_flag, login_fp);
            return retval;
        }
    }

    close_time(k5users_flag, users_fp, k5login_flag, login_fp);

    if (cmd != NULL) {
        retval = list_union(k5login_list, k5users_filt_list, &combined_list);
        if (retval) {
            close_time(k5users_flag, users_fp, k5login_flag, login_fp);
            return retval;
        }
        *princ_list = combined_list;
        return 0;
    } else {
        if (k5users_filt_list != NULL)
            free(k5users_filt_list);
        *princ_list = k5login_list;
        return 0;
    }
}

static void
close_time(int k5users_flag, FILE *users_fp, int k5login_flag, FILE *login_fp)
{
    if (!k5users_flag)
        fclose(users_fp);
    if (!k5login_flag)
        fclose(login_fp);
}

static krb5_boolean
find_str_in_list(char **list, char *elm)
{
    int i = 0;
    krb5_boolean found = FALSE;

    if (list == NULL)
        return found;

    while (list[i]) {
        if (strcmp(list[i], elm) == 0) {
            found = TRUE;
            break;
        }
        i++;
    }

    return found;
}

/*
 * Returns the principal that is closes to client (can be the the client
 * himself). plist contains a principal list obtained from .k5login
 * and .k5users file.
 * A principal is picked that has the best chance of getting in.
 */
static krb5_error_code
get_closest_principal(krb5_context context, char **plist,
                      krb5_principal *client, krb5_boolean *found)
{
    krb5_error_code retval = 0;
    krb5_principal temp_client = NULL, best_client = NULL;
    int i = 0, j = 0, cnelem, pnelem;
    krb5_boolean got_one;

    *found = FALSE;

    if (plist == NULL)
        return 0;

    cnelem = krb5_princ_size(context, *client);

    while (plist[i] != NULL) {
        pnelem = krb5_princ_size(context, temp_client);

        if (cnelem > pnelem) {
            i++;
            continue;
        }

        if (data_eq(*krb5_princ_realm(context, *client),
                    *krb5_princ_realm(context, temp_client))) {
            got_one = TRUE;
            for (j = 0; j < cnelem; j++) {
                krb5_data *p1 = krb5_princ_component(context, *client, j);
                krb5_data *p2 = krb5_princ_component(context, temp_client, j);

                if ((p1 == NULL) || (p2 == NULL) || !data_eq(*p1, *p2)) {
                    got_one = FALSE;
                    break;
                }
            }
            if (got_one == TRUE) {
                if (best_client) {
                    if (krb5_princ_size(context, best_client) >
                        krb5_princ_size(context, temp_client)) {
                        best_client = temp_client;
                    }
                } else
                    best_client = temp_client;
            }
        }
        i++;
    }

    if (best_client) {
        *found = TRUE;
        *client = best_client;
    }

    return 0;
}

/*
 * find_either_ticket checks to see whether there is a ticket for the
 * end server or a tgt, if neither is there the return FALSE.
 */
static krb5_error_code
find_either_ticket(krb5_context context, krb5_ccache cc, krb5_principal client,
                   krb5_principal end_server, krb5_boolean *found)
{
    krb5_principal kdc_server;
    krb5_error_code retval;
    krb5_boolean temp_found = FALSE;
    const char *cc_source_name;
    struct stat st_temp;

    cc_source_name = krb5_cc_get_name(context, cc);

    if (stat(cc_source_name, &st_temp) == 0) {
        retval = find_ticket(context, cc, client, end_server, &temp_found);
        if (retval)
            return retval;

        if (temp_found == FALSE) {
            retval = ksu_tgtname(context,
                                 krb5_princ_realm(context, client),
                                 krb5_princ_realm(context, client),
                                 &kdc_server);
            if (retval)
                return retval;

            retval = find_ticket(context, cc,client, kdc_server, &temp_found);
            if(retval)
                return retval;
        } else if (auth_debug) {
            printf("find_either_ticket: found end server ticket\n");
        }
    }

    *found = temp_found;
    return 0;
}

static krb5_error_code
find_ticket(krb5_context context, krb5_ccache cc, krb5_principal client,
            krb5_principal server, krb5_boolean *found)
{
    krb5_creds tgt, tgtq;
    krb5_error_code retval;

    *found = FALSE;

    memset(&tgtq, 0, sizeof(tgtq));
    memset(&tgt, 0, sizeof(tgt));

    retval= krb5_copy_principal(context,  client, &tgtq.client);
    if (retval)
        return retval;

    retval= krb5_copy_principal(context,  server, &tgtq.server);
    if (retval)
        return retval;

    retval = krb5_cc_retrieve_cred(context, cc, KRB5_TC_MATCH_SRV_NAMEONLY |
                                   KRB5_TC_SUPPORTED_KTYPES, &tgtq, &tgt);

    if (retval == 0)
        retval = krb5_check_exp(context, tgt.times);

    if (retval) {
        if ((retval != KRB5_CC_NOTFOUND) &&
            (retval != KRB5KRB_AP_ERR_TKT_EXPIRED)) {
            return retval;
        }
    } else {
        *found = TRUE;
        return 0;
    }

    free(tgtq.server);
    free(tgtq.client);
    return 0;
}

static krb5_error_code
find_princ_in_list(krb5_context context, krb5_principal princ, char **plist,
                   krb5_boolean *found)
{
    int i = 0;
    char *princname;
    krb5_error_code retval;

    *found = FALSE;

    if (plist == NULL)
        return 0;

    retval = krb5_unparse_name(context, princ, &princname);
    if (retval)
        return retval;

    while (plist[i] != NULL) {
        if (strcmp(plist[i], princname) == 0) {
            *found = TRUE;
            break;
        }
        i++;
    }

    return 0;
}

typedef struct princ_info {
    krb5_principal p;
    krb5_boolean found;
} princ_info;

/*
 * get_best_princ_for_target -
 * sets the client name, path_out gets set, if authorization is not possible
 * path_out gets set to ...
 */
krb5_error_code
get_best_princ_for_target(krb5_context context, uid_t source_uid,
                          uid_t target_uid, char *source_user,
                          char *target_user, krb5_ccache cc_source,
                          opt_info *options, char *cmd, char *hostname,
                          krb5_principal *client, int *path_out)
{
    princ_info princ_trials[10];
    const char *cc_source_name;
    krb5_principal cc_def_princ = NULL;
    krb5_principal temp_client;
    krb5_principal target_client;
    krb5_principal source_client;
    krb5_principal end_server;
    krb5_error_code retval;
    char **aplist = NULL;
    krb5_boolean found = FALSE;
    struct stat tb;
    int count = 0;
    int i;
    struct stat st_temp;

    *path_out = 0;

    /* -n option was specified client is set we are done */
    if (options->princ)
        return 0;

    cc_source_name = krb5_cc_get_name(context, cc_source);

    if (stat(cc_source_name, &st_temp) == 0) {
        retval = krb5_cc_get_principal(context, cc_source, &cc_def_princ);
        if (retval)
            return retval;
    }

    retval = krb5_parse_name(context, target_user, &target_client);
    if (retval)
        return retval;

    retval = krb5_parse_name(context, source_user, &source_client);
    if (retval)
        return retval;

    if (source_uid == 0) {
        if (target_uid != 0) {
            *client = target_client; /* this will be used to restrict
                                        the cache copty */
        } else {
            if (cc_def_princ)
                *client = cc_def_princ;
            else
                *client = target_client;
        }

        if (auth_debug)
            printf(" GET_best_princ_for_target: via source_uid == 0\n");

        return 0;
    }

    /* from here on, the code is for source_uid != 0 */

    if (source_uid && (source_uid == target_uid)) {
        if (cc_def_princ)
            *client = cc_def_princ;
        else
            *client = target_client;
        if (auth_debug)
            printf("GET_best_princ_for_target: via source_uid == target_uid\n");
        return 0;
    }

    /* Become root, then target for looking at .k5login.*/
    if (krb5_seteuid(0) || krb5_seteuid(target_uid)) {
        return errno;
    }

    /* if .k5users and .k5login do not exist */
    if ((stat(k5login_path, &tb) != 0) && (stat(k5users_path, &tb) != 0)) {
        *client = target_client;

        if (cmd != NULL)
            *path_out = NOT_AUTHORIZED;

        if (auth_debug)
            printf(" GET_best_princ_for_target: via no auth files path\n");

        return 0;
    } else {
        retval = get_authorized_princ_names(target_user, cmd, &aplist);
        if (retval)
            return retval;

        /* .k5users or .k5login exist, but no authorization */
        if ((aplist == NULL) || (aplist[0] == NULL)) {
            *path_out = NOT_AUTHORIZED;
            if (auth_debug)
                printf("GET_best_princ_for_target: via empty auth files path\n");
            return 0;
        }
    }

    retval = krb5_sname_to_principal(context, hostname, NULL,
                                     KRB5_NT_SRV_HST, &end_server);
    if (retval)
        return retval;


    /*
     * first see if default principal of the source cache
     * can get us in, then the target_user@realm, then the
     * source_user@realm. If all of them fail, try any
     * other ticket in the cache.
     */
    if (cc_def_princ != NULL)
        princ_trials[count++].p = cc_def_princ;
    else
        princ_trials[count++].p = NULL;

    princ_trials[count++].p = target_client;
    princ_trials[count++].p = source_client;

    for (i = 0; i < count; i++)
        princ_trials[i].found = FALSE;

    for (i = 0; i < count; i++) {
        if (princ_trials[i].p != NULL) {
            retval = find_princ_in_list(context, princ_trials[i].p, aplist,
                                        &found);
            if (retval)
                return retval;

            if (found == TRUE) {
                princ_trials[i].found = TRUE;
                retval = find_either_ticket(context, cc_source,
                                            princ_trials[i].p,
                                            end_server, &found);
                if (retval)
                    return retval;
                if (found == TRUE) {
                    *client = princ_trials[i].p;
                    if (auth_debug)
                        printf("GET_best_princ_for_target: via ticket file, choice #%d\n", i);
                    return 0;
                }
            }
        }
    }

    /* Out of preferred principals, see if there is any ticket that will
       get us in */
    i = 0;
    while (aplist[i] != NULL) {
        retval = krb5_parse_name(context, aplist[i], &temp_client);
        if (retval)
            return retval;

        retval = find_either_ticket(context, cc_source, temp_client,
                                    end_server, &found);
        if (retval)
            return retval;

        if (found == TRUE) {
            if (auth_debug)
                printf("GET_best_princ_for_target: via ticket file, choice: any ok ticket \n" );
            *client = temp_client;
            return 0;
        }

        krb5_free_principal(context, temp_client);
        i++;
    }

    /* No tickets qualified, select a principal that may be used
       for password promting */
    for(i = 0; i < count; i ++) {
        if (princ_trials[i].found == TRUE) {
            *client = princ_trials[i].p;

            if (auth_debug)
                printf("GET_best_princ_for_target: via prompt passwd list choice #%d \n", i);
            return  0;
        }
    }

#ifdef PRINC_LOOK_AHEAD
    for (i = 0; i < count; i ++) {
        if (princ_trials[i].p != NULL) {
            retval = krb5_copy_principal(context, princ_trials[i].p,
                                         &temp_client);
            if (retval)
                return retval;

            /* Get the client name that is the closest
               to the three princ in trials */
            retval = get_closest_principal(context, aplist, &temp_client,
                                           &found);
            if (retval)
                return retval;

            if (found == TRUE) {
                *client = temp_client;
                if (auth_debug)
                    printf("GET_best_princ_for_target: via prompt passwd list choice: approximation of princ in trials # %d \n", i);
                return 0;
            }
            krb5_free_principal(context, temp_client);
        }
    }
#endif /* PRINC_LOOK_AHEAD */

    if(auth_debug)
        printf( "GET_best_princ_for_target: out of luck, can't get appropriate default principal\n");

    *path_out = NOT_AUTHORIZED;
    return 0;
}
