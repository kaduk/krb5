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

static krb5_boolean fast_auth(krb5_context context, krb5_principal client,
                              krb5_principal server, char *target_user,
                              krb5_ccache cc);
static krb5_error_code get_best_principal(krb5_context context, char **plist,
                                          krb5_principal *client);

krb5_boolean
krb5_auth_check(krb5_context context, krb5_principal client_pname,
                char *hostname, opt_info *options, char *target_user,
                krb5_ccache cc, int *path_passwd, uid_t target_uid)
{
    krb5_principal client, server;
    krb5_verify_init_creds_opt vfy_opts;
    krb5_creds tgt, tgtq, in_creds, *out_creds;
    krb5_error_code retval = 0;
    int got_it = 0;
    krb5_boolean zero_password;

    *path_passwd = 0;
    memset(&tgtq, 0, sizeof(tgtq));
    memset(&tgt, 0, sizeof(tgt));
    memset(&in_creds, 0, sizeof(krb5_creds));


    retval = krb5_copy_principal(context,  client_pname, &client);
    if (retval) {
        com_err(prog_name, retval, _("while copying client principal"));
        return FALSE;
    }

    if (auth_debug)
        dump_principal(context, "krb5_auth_check: Client principal name",
                       client);

    retval = krb5_sname_to_principal(context, hostname, NULL, KRB5_NT_SRV_HST,
                                     &server);
    if (retval) {
        com_err(prog_name, retval,
                _("while creating server %s principal name"), hostname);
        krb5_free_principal(context, client);
        return FALSE;
    }

    if (auth_debug)
        dump_principal(context, "krb5_auth_check: Server principal name",
                       server);

    /* Check if ticket is already in the cache; if it is, then use it. */
    if (fast_auth(context, client, server, target_user, cc) == TRUE) {
        if (auth_debug)
            fprintf (stderr,"Authenticated via fast_auth \n");
        return TRUE;
    }

    /* Check to see if the local tgt is in the cache */
    retval = krb5_copy_principal(context, client, &tgtq.client);
    if (retval) {
        com_err(prog_name, retval, _("while copying client principal"));
        return FALSE;
    }

    retval = ksu_tgtname(context, krb5_princ_realm(context, client),
                         krb5_princ_realm(context, client), &tgtq.server);
    if (retval) {
        com_err(prog_name, retval, _("while creating tgt for local realm"));
        krb5_free_principal(context, client);
        krb5_free_principal(context, server);
        return FALSE;
    }

    if (auth_debug)
        dump_principal(context, "local tgt principal name", tgtq.server );
    retval = krb5_cc_retrieve_cred(context, cc,
                                   KRB5_TC_MATCH_SRV_NAMEONLY | KRB5_TC_SUPPORTED_KTYPES,
                                   &tgtq, &tgt);

    if (retval == 0)
        retval = krb5_check_exp(context, tgt.times);

    if (retval) {
        if ((retval != KRB5_CC_NOTFOUND) &&
            (retval != KRB5KRB_AP_ERR_TKT_EXPIRED)) {
            com_err(prog_name, retval, _("while retrieving creds from cache"));
            return FALSE;
        }
    } else {
        got_it = 1;
    }

    if (got_it == 0) {
#ifdef GET_TGT_VIA_PASSWD
        if (krb5_seteuid(0) || krb5_seteuid(target_uid)) {
            com_err("ksu", errno, _("while switching to target uid"));
            return FALSE;
        }

        fprintf(stderr, _("WARNING: Your password may be exposed if you enter "
                          "it here and are logged \n"));
        fprintf(stderr, _("         in remotely using an unsecure "
                          "(non-encrypted) channel. \n"));

        /* Get the ticket granting ticket, via passwd (prompt for passwd). */
        if (krb5_get_tkt_via_passwd(context, &cc, client, tgtq.server,
                                    options, & zero_password) == FALSE) {
            krb5_seteuid(0);
            return FALSE;
        }
        *path_passwd = 1;
        if (krb5_seteuid(0)) {
            com_err("ksu", errno, _("while reclaiming root uid"));
            return FALSE;
        }
#else
        plain_dump_principal(context, client);
        fprintf(stderr,
                _("does not have any appropriate tickets in the cache.\n"));
        return FALSE;
#endif /* GET_TGT_VIA_PASSWD */
    }

    retval = krb5_copy_principal(context, client, &in_creds.client);
    if (retval) {
        com_err(prog_name, retval, _("while copying client principal"));
        return (FALSE) ;
    }

    retval = krb5_copy_principal(context, server, &in_creds.server);
    if (retval) {
        com_err(prog_name, retval, _("while copying client principal"));
        return FALSE;
    }

    retval = krb5_get_credentials(context, 0, cc, &in_creds, &out_creds);
    if (retval) {
        com_err(prog_name, retval, _("while getting credentials from kdc"));
        return (FALSE);
    }

    if (auth_debug) {
        fprintf(stderr,"krb5_auth_check: got ticket for end server \n");
        dump_principal(context, "out_creds->server", out_creds->server );
    }

    krb5_verify_init_creds_opt_init(&vfy_opts);
    krb5_verify_init_creds_opt_set_ap_req_nofail(&vfy_opts, 1);
    retval = krb5_verify_init_creds(context, out_creds, server, NULL /*keytab*/,
                                    NULL /*output ccache*/,
                                    &vfy_opts);
    if (retval) {
        com_err(prog_name, retval, _("while verifying ticket for server"));
        return FALSE;
    }
    return TRUE;
}

/* fast_auth checks if a ticket for the end server is already in
   the cache, if it is, we don't need a tgt. */
static krb5_boolean
fast_auth(krb5_context context, krb5_principal client, krb5_principal server,
          char *target_user, krb5_ccache cc)
{
    krb5_creds tgt, tgtq;
    krb5_verify_init_creds_opt vfy_opts;
    krb5_error_code retval;

    memset(&tgtq, 0, sizeof(tgtq));
    memset(&tgt, 0, sizeof(tgt));

    retval = krb5_copy_principal(context, client, &tgtq.client);
    if (retval) {
        com_err(prog_name, retval, _("while copying client principal"));
        return (FALSE) ;
    }

    retval = krb5_copy_principal(context, server, &tgtq.server);
    if (retval) {
        com_err(prog_name, retval, _("while copying client principal"));
        return (FALSE) ;
    }

    retval = krb5_cc_retrieve_cred(context, cc, KRB5_TC_MATCH_SRV_NAMEONLY | KRB5_TC_SUPPORTED_KTYPES,
                                   &tgtq, &tgt);
    if (retval) {
        if (auth_debug)
            com_err(prog_name, retval, _("while Retrieving credentials"));
        return FALSE;
    }
    krb5_verify_init_creds_opt_init(&vfy_opts);
    krb5_verify_init_creds_opt_set_ap_req_nofail(&vfy_opts, 1);
    retval = krb5_verify_init_creds(context, &tgt, server, NULL /*keytab*/,
                                    NULL /*output ccache*/, &vfy_opts);
    if (retval) {
        com_err(prog_name, retval, _("while verifying ticket for server"));
        return FALSE;
    }
    return TRUE;
}

krb5_boolean
krb5_get_tkt_via_passwd(krb5_context context, krb5_ccache *ccache,
                        krb5_principal client, krb5_principal server,
                        opt_info *options, krb5_boolean *zero_password)
{
    krb5_error_code code;
    krb5_creds my_creds;
    krb5_timestamp now;
    unsigned int pwsize;
    char password[255], *client_name, prompt[255];
    int result;

    *zero_password = FALSE;

    code = krb5_unparse_name(context, client, &client_name);
    if (code) {
        com_err (prog_name, code, _("when unparsing name"));
        return (FALSE);
    }

    memset(&my_creds, 0, sizeof(my_creds));

    code = krb5_copy_principal(context, client, &my_creds.client);
    if (code) {
        com_err(prog_name, code, _("while copying principal"));
        return FALSE;
    }

    code = krb5_copy_principal(context, server, &my_creds.server);
    if (code) {
        com_err(prog_name, code, _("while copying principal"));
        return FALSE;
    }

    code = krb5_timeofday(context, &now);
    if (code) {
        com_err(prog_name, code, _("while getting time of day"));
        return FALSE;
    }

    /* Start timer when request gets to KDC. */
    my_creds.times.starttime = 0;

    my_creds.times.endtime = now + options->lifetime;
    if (options->opt & KDC_OPT_RENEWABLE)
        my_creds.times.renew_till = now + options->rlife;
    else
        my_creds.times.renew_till = 0;

    result = snprintf(prompt, sizeof(prompt), _("Kerberos password for %s: "),
                      client_name);
    if (SNPRINTF_OVERFLOW(result, sizeof(prompt))) {
        fprintf(stderr,
                _("principal name %s too long for internal buffer space\n"),
                client_name);
        return FALSE;
    }

    pwsize = sizeof(password);

    code = krb5_read_password(context, prompt, 0, password, &pwsize);
    if (code) {
        com_err(prog_name, code, _("while reading password for '%s'\n"),
                client_name);
        memset(password, 0, sizeof(password));
        return FALSE;
    }

    if (pwsize == 0) {
        fprintf(stderr, _("No password given\n"));
        *zero_password = TRUE;
        memset(password, 0, sizeof(password));
        return (FALSE);
    }

    code = krb5_get_in_tkt_with_password(context, options->opt,
                                         0, NULL, NULL,
                                         password, *ccache, &my_creds, 0);
    memset(password, 0, sizeof(password));

    if (code) {
        if (code == KRB5KRB_AP_ERR_BAD_INTEGRITY)
            fprintf(stderr, _("%s: Password incorrect\n"), prog_name);
        else
            com_err(prog_name, code, _("while getting initial credentials"));
        return FALSE;
    }
    return TRUE;
}

void
dump_principal(krb5_context context, char *str, krb5_principal p)
{
    char *stname;

    if (krb5_unparse_name(context, p, &stname) != 0) {
        fprintf(stderr, _(" %s while unparsing name\n"),
                error_message(retval));
    }
    fprintf(stderr, " %s: %s\n", str, stname);
}

void
plain_dump_principal(krb5_context context, krb5_principal p)
{
    char *stname;
    krb5_error_code retval;

    if (krb5_unparse_name(context, p, &stname) != 0) {
        fprintf(stderr, _(" %s while unparsing name\n"),
                error_message(retval));
    }
    fprintf(stderr, "%s ", stname);
}

/*
 * Returns the principal that is closest to client. plist contains
 * a principal list obtained from .k5login and perhaps .k5users file.
 * This routine gets called before getting the password for a tgt.
 * A principal is picked that has the best chance of getting in.
 */
static krb5_error_code
get_best_principal(krb5_context context, char **plist, krb5_principal *client)
{
    krb5_error_code retval =0;
    krb5_principal temp_client, best_client = NULL;
    int i = 0, nelem;

    if (plist == NULL)
        return 0;

    nelem = krb5_princ_size(context, *client);

    while (plist[i] != NULL) {
        retval = krb5_parse_name(context, plist[i], &temp_client);
        if (retval)
            return retval;

        if (data_eq(*krb5_princ_realm(context, *client),
                    *krb5_princ_realm(context, temp_client))) {
            if (nelem > 0 &&
                krb5_princ_size(context, *client) > 0 &&
                krb5_princ_size(context, temp_client) > 0) {
                krb5_data *p1 =
                    krb5_princ_component(context, *client, 0);
                krb5_data *p2 =
                    krb5_princ_component(context, temp_client, 0);

                if (data_eq(*p1, *p2)) {
                    if (auth_debug) {
                        fprintf(stderr,
                                "get_best_principal: compare with %s\n",
                                plist[i]);
                    }
                    if (best_client != NULL) {
                        if (krb5_princ_size(context, best_client) >
                            krb5_princ_size(context, temp_client)) {
                            best_client = temp_client;
                        }
                    } else {
                        best_client = temp_client;
                    }
                }
            }

        }
        i++;
    }
    if (best_client != NULL) *client = best_client;
    return 0;
}
