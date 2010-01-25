/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * include/krb5/kdb.h
 *
 * Copyright 1990,1991 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 *
 * KDC Database interface definitions.
 */

/*
 * Copyright (C) 1998 by the FundsXpress, INC.
 *
 * All rights reserved.
 *
 * Export of this software from the United States of America may require
 * a specific license from the United States Government.  It is the
 * responsibility of any person or organization contemplating export to
 * obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of FundsXpress. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  FundsXpress makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/* This API is not considered as stable as the main krb5 API.
 *
 * - We may make arbitrary incompatible changes between feature
 *   releases (e.g. from 1.7 to 1.8).
 * - We will make some effort to avoid making incompatible changes for
 *   bugfix releases, but will make them if necessary.
 */

#ifndef KRB5_KDB5__
#define KRB5_KDB5__

#include <krb5.h>

/* Salt types */
#define KRB5_KDB_SALTTYPE_NORMAL        0
#define KRB5_KDB_SALTTYPE_V4            1
#define KRB5_KDB_SALTTYPE_NOREALM       2
#define KRB5_KDB_SALTTYPE_ONLYREALM     3
#define KRB5_KDB_SALTTYPE_SPECIAL       4
#define KRB5_KDB_SALTTYPE_AFS3          5
#define KRB5_KDB_SALTTYPE_CERTHASH      6

/* Attributes */
#define KRB5_KDB_DISALLOW_POSTDATED     0x00000001
#define KRB5_KDB_DISALLOW_FORWARDABLE   0x00000002
#define KRB5_KDB_DISALLOW_TGT_BASED     0x00000004
#define KRB5_KDB_DISALLOW_RENEWABLE     0x00000008
#define KRB5_KDB_DISALLOW_PROXIABLE     0x00000010
#define KRB5_KDB_DISALLOW_DUP_SKEY      0x00000020
#define KRB5_KDB_DISALLOW_ALL_TIX       0x00000040
#define KRB5_KDB_REQUIRES_PRE_AUTH      0x00000080
#define KRB5_KDB_REQUIRES_HW_AUTH       0x00000100
#define KRB5_KDB_REQUIRES_PWCHANGE      0x00000200
#define KRB5_KDB_DISALLOW_SVR           0x00001000
#define KRB5_KDB_PWCHANGE_SERVICE       0x00002000
#define KRB5_KDB_SUPPORT_DESMD5         0x00004000
#define KRB5_KDB_NEW_PRINC              0x00008000
#define KRB5_KDB_OK_AS_DELEGATE         0x00100000
#define KRB5_KDB_OK_TO_AUTH_AS_DELEGATE 0x00200000 /* S4U2Self OK */
#define KRB5_KDB_NO_AUTH_DATA_REQUIRED  0x00400000

/* Creation flags */
#define KRB5_KDB_CREATE_BTREE           0x00000001
#define KRB5_KDB_CREATE_HASH            0x00000002

#if !defined(_WIN32)

/*
 * Note --- these structures cannot be modified without changing the
 * database version number in libkdb.a, but should be expandable by
 * adding new tl_data types.
 */
typedef struct _krb5_tl_data {
    struct _krb5_tl_data* tl_data_next;         /* NOT saved */
    krb5_int16            tl_data_type;
    krb5_ui_2             tl_data_length;
    krb5_octet          * tl_data_contents;
} krb5_tl_data;

/*
 * If this ever changes up the version number and make the arrays be as
 * big as necessary.
 *
 * Currently the first type is the enctype and the second is the salt type.
 */
typedef struct _krb5_key_data {
    krb5_int16            key_data_ver;         /* Version */
    krb5_int16            key_data_kvno;        /* Key Version */
    krb5_int16            key_data_type[2];     /* Array of types */
    krb5_ui_2             key_data_length[2];   /* Array of lengths */
    krb5_octet          * key_data_contents[2]; /* Array of pointers */
} krb5_key_data;

#define KRB5_KDB_V1_KEY_DATA_ARRAY      2       /* # of array elements */

typedef struct _krb5_keysalt {
    krb5_int16            type;
    krb5_data             data;                 /* Length, data */
} krb5_keysalt;

typedef struct _krb5_db_entry_new {
    krb5_magic            magic;                /* NOT saved */
    krb5_ui_2             len;
    krb5_ui_4             mask;                 /* members currently changed/set */
    krb5_flags            attributes;
    krb5_deltat           max_life;
    krb5_deltat           max_renewable_life;
    krb5_timestamp        expiration;           /* When the client expires */
    krb5_timestamp        pw_expiration;        /* When its passwd expires */
    krb5_timestamp        last_success;         /* Last successful passwd */
    krb5_timestamp        last_failed;          /* Last failed passwd attempt */
    krb5_kvno             fail_auth_count;      /* # of failed passwd attempt */
    krb5_int16            n_tl_data;
    krb5_int16            n_key_data;
    krb5_ui_2             e_length;             /* Length of extra data */
    krb5_octet          * e_data;               /* Extra data to be saved */

    krb5_principal        princ;                /* Length, data */
    krb5_tl_data        * tl_data;              /* Linked list */
    krb5_key_data       * key_data;             /* Array */
} krb5_db_entry;

typedef struct _osa_policy_ent_t {
    int               version;
    char      *name;
    krb5_ui_4       pw_min_life;
    krb5_ui_4       pw_max_life;
    krb5_ui_4       pw_min_length;
    krb5_ui_4       pw_min_classes;
    krb5_ui_4       pw_history_num;
    krb5_ui_4       policy_refcnt;
    /* Only valid if version > 1 */
    krb5_ui_4       pw_max_fail;                /* pwdMaxFailure */
    krb5_ui_4       pw_failcnt_interval;        /* pwdFailureCountInterval */
    krb5_ui_4       pw_lockout_duration;        /* pwdLockoutDuration */
} osa_policy_ent_rec, *osa_policy_ent_t;

typedef       void    (*osa_adb_iter_policy_func) (void *, osa_policy_ent_t);

typedef struct __krb5_key_salt_tuple {
    krb5_enctype        ks_enctype;
    krb5_int32          ks_salttype;
} krb5_key_salt_tuple;

#define KRB5_KDB_MAGIC_NUMBER           0xdbdbdbdb
#define KRB5_KDB_V1_BASE_LENGTH         38

#define KRB5_TL_LAST_PWD_CHANGE         0x0001
#define KRB5_TL_MOD_PRINC               0x0002
#define KRB5_TL_KADM_DATA               0x0003
#define KRB5_TL_KADM5_E_DATA            0x0004
#define KRB5_TL_RB1_CHALLENGE           0x0005
#ifdef SECURID
#define KRB5_TL_SECURID_STATE           0x0006
#define KRB5_TL_DB_ARGS                 0x7fff
#endif /* SECURID */
#define KRB5_TL_USER_CERTIFICATE        0x0007
#define KRB5_TL_MKVNO                   0x0008
#define KRB5_TL_ACTKVNO                 0x0009
#define KRB5_TL_MKEY_AUX                0x000a

/* version number for KRB5_TL_ACTKVNO data */
#define KRB5_TL_ACTKVNO_VER     1

/* version number for KRB5_TL_MKEY_AUX data */
#define KRB5_TL_MKEY_AUX_VER    1

typedef struct _krb5_actkvno_node {
    struct _krb5_actkvno_node *next;
    krb5_kvno      act_kvno;
    krb5_timestamp act_time;
} krb5_actkvno_node;

typedef struct _krb5_mkey_aux_node {
    struct _krb5_mkey_aux_node *next;
    krb5_kvno        mkey_kvno; /* kvno of mkey protecting the latest_mkey */
    krb5_key_data    latest_mkey; /* most recent mkey */
} krb5_mkey_aux_node;

typedef struct _krb5_keylist_node {
    krb5_keyblock keyblock;
    krb5_kvno     kvno;
    struct _krb5_keylist_node *next;
} krb5_keylist_node;

/*
 * Determines the number of failed KDC requests before DISALLOW_ALL_TIX is set
 * on the principal.
 */
#define KRB5_MAX_FAIL_COUNT             5

/* XXX depends on knowledge of krb5_parse_name() formats */
#define KRB5_KDB_M_NAME         "K/M"   /* Kerberos/Master */

/* prompts used by default when reading the KDC password from the keyboard. */
#define KRB5_KDC_MKEY_1 "Enter KDC database master key"
#define KRB5_KDC_MKEY_2 "Re-enter KDC database master key to verify"


extern char *krb5_mkey_pwd_prompt1;
extern char *krb5_mkey_pwd_prompt2;

/*
 * These macros specify the encoding of data within the database.
 *
 * Data encoding is little-endian.
 */
#ifdef _KRB5_INT_H
#include "k5-platform.h"
#define krb5_kdb_decode_int16(cp, i16)          \
    *((krb5_int16 *) &(i16)) = load_16_le(cp)
#define krb5_kdb_decode_int32(cp, i32)          \
    *((krb5_int32 *) &(i32)) = load_32_le(cp)
#define krb5_kdb_encode_int16(i16, cp)  store_16_le(i16, cp)
#define krb5_kdb_encode_int32(i32, cp)  store_32_le(i32, cp)
#endif /* _KRB5_INT_H */

#define KRB5_KDB_OPEN_RW                0
#define KRB5_KDB_OPEN_RO                1

#ifndef KRB5_KDB_SRV_TYPE_KDC
#define KRB5_KDB_SRV_TYPE_KDC           0x0100
#endif

#ifndef KRB5_KDB_SRV_TYPE_ADMIN
#define KRB5_KDB_SRV_TYPE_ADMIN         0x0200
#endif

#ifndef KRB5_KDB_SRV_TYPE_PASSWD
#define KRB5_KDB_SRV_TYPE_PASSWD        0x0300
#endif

#ifndef KRB5_KDB_SRV_TYPE_OTHER
#define KRB5_KDB_SRV_TYPE_OTHER         0x0400
#endif

#define KRB5_KDB_OPT_SET_DB_NAME        0
#define KRB5_KDB_OPT_SET_LOCK_MODE      1

#define KRB5_DB_LOCKMODE_SHARED       0x0001
#define KRB5_DB_LOCKMODE_EXCLUSIVE    0x0002
#define KRB5_DB_LOCKMODE_DONTBLOCK    0x0004
#define KRB5_DB_LOCKMODE_PERMANENT    0x0008

/* libkdb.spec */
krb5_error_code krb5_db_setup_lib_handle(krb5_context kcontext);
krb5_error_code krb5_db_open( krb5_context kcontext, char **db_args, int mode );
krb5_error_code krb5_db_init  ( krb5_context kcontext );
krb5_error_code krb5_db_create ( krb5_context kcontext, char **db_args );
krb5_error_code krb5_db_inited  ( krb5_context kcontext );
krb5_error_code kdb5_db_create ( krb5_context kcontext, char **db_args );
krb5_error_code krb5_db_fini ( krb5_context kcontext );
const char * krb5_db_errcode2string ( krb5_context kcontext, long err_code );
krb5_error_code krb5_db_destroy ( krb5_context kcontext, char **db_args );
krb5_error_code krb5_db_promote ( krb5_context kcontext, char **db_args );
krb5_error_code krb5_db_get_age ( krb5_context kcontext, char *db_name, time_t *t );
krb5_error_code krb5_db_set_option ( krb5_context kcontext, int option, void *value );
krb5_error_code krb5_db_lock ( krb5_context kcontext, int lock_mode );
krb5_error_code krb5_db_unlock ( krb5_context kcontext );
krb5_error_code krb5_db_get_principal ( krb5_context kcontext,
                                        krb5_const_principal search_for,
                                        krb5_db_entry *entries,
                                        int *nentries,
                                        krb5_boolean *more );
krb5_error_code krb5_db_free_principal ( krb5_context kcontext,
                                         krb5_db_entry *entry,
                                         int count );
krb5_error_code krb5_db_put_principal ( krb5_context kcontext,
                                        krb5_db_entry *entries,
                                        int *nentries);
krb5_error_code krb5_db_delete_principal ( krb5_context kcontext,
                                           krb5_principal search_for,
                                           int *nentries );
krb5_error_code krb5_db_iterate ( krb5_context kcontext,
                                  char *match_entry,
                                  int (*func) (krb5_pointer, krb5_db_entry *),
                                  krb5_pointer func_arg );
krb5_error_code krb5_supported_realms ( krb5_context kcontext,
                                        char **realms );
krb5_error_code krb5_free_supported_realms ( krb5_context kcontext,
                                             char **realms );
krb5_error_code krb5_db_set_master_key_ext ( krb5_context kcontext,
                                             char *pwd,
                                             krb5_keyblock *key );
krb5_error_code krb5_db_set_mkey ( krb5_context context,
                                   krb5_keyblock *key);
krb5_error_code krb5_db_get_mkey ( krb5_context kcontext,
                                   krb5_keyblock **key );

krb5_error_code krb5_db_set_mkey_list( krb5_context context,
                                       krb5_keylist_node * keylist);

krb5_error_code krb5_db_get_mkey_list( krb5_context kcontext,
                                       krb5_keylist_node ** keylist);

krb5_error_code krb5_db_free_master_key ( krb5_context kcontext,
                                          krb5_keyblock *key );
krb5_error_code krb5_db_store_master_key  ( krb5_context kcontext,
                                            char *keyfile,
                                            krb5_principal mname,
                                            krb5_kvno kvno,
                                            krb5_keyblock *key,
                                            char *master_pwd);
krb5_error_code krb5_db_store_master_key_list  ( krb5_context kcontext,
                                                 char *keyfile,
                                                 krb5_principal mname,
                                                 krb5_keylist_node *keylist,
                                                 char *master_pwd);
krb5_error_code krb5_db_fetch_mkey  ( krb5_context   context,
                                      krb5_principal mname,
                                      krb5_enctype   etype,
                                      krb5_boolean   fromkeyboard,
                                      krb5_boolean   twice,
                                      char          *db_args,
                                      krb5_kvno     *kvno,
                                      krb5_data     *salt,
                                      krb5_keyblock *key);
krb5_error_code krb5_db_verify_master_key ( krb5_context   kcontext,
                                            krb5_principal mprinc,
                                            krb5_kvno      kvno,
                                            krb5_keyblock  *mkey );
krb5_error_code
krb5_db_fetch_mkey_list( krb5_context    context,
                         krb5_principal  mname,
                         const krb5_keyblock * mkey,
                         krb5_kvno             mkvno,
                         krb5_keylist_node  **mkeys_list );

krb5_error_code
krb5_db_free_mkey_list( krb5_context         context,
                        krb5_keylist_node  *mkey_list );

krb5_error_code
krb5_dbe_find_enctype( krb5_context     kcontext,
                       krb5_db_entry    *dbentp,
                       krb5_int32               ktype,
                       krb5_int32               stype,
                       krb5_int32               kvno,
                       krb5_key_data    **kdatap);


krb5_error_code krb5_dbe_search_enctype ( krb5_context kcontext,
                                          krb5_db_entry *dbentp,
                                          krb5_int32 *start,
                                          krb5_int32 ktype,
                                          krb5_int32 stype,
                                          krb5_int32 kvno,
                                          krb5_key_data **kdatap);

krb5_error_code
krb5_db_setup_mkey_name ( krb5_context context,
                          const char *keyname,
                          const char *realm,
                          char **fullname,
                          krb5_principal *principal);

krb5_error_code
krb5_dbekd_decrypt_key_data( krb5_context         context,
                             const krb5_keyblock        * mkey,
                             const krb5_key_data        * key_data,
                             krb5_keyblock      * dbkey,
                             krb5_keysalt       * keysalt);

krb5_error_code
krb5_dbekd_encrypt_key_data( krb5_context                 context,
                             const krb5_keyblock        * mkey,
                             const krb5_keyblock        * dbkey,
                             const krb5_keysalt         * keysalt,
                             int                          keyver,
                             krb5_key_data              * key_data);

krb5_error_code
krb5_dbe_fetch_act_key_list(krb5_context          context,
                            krb5_principal       princ,
                            krb5_actkvno_node  **act_key_list);

krb5_error_code
krb5_dbe_find_act_mkey( krb5_context          context,
                        krb5_keylist_node   * mkey_list,
                        krb5_actkvno_node   * act_mkey_list,
                        krb5_kvno           * act_kvno,
                        krb5_keyblock      ** act_mkey);

krb5_error_code
krb5_dbe_find_mkey( krb5_context         context,
                    krb5_keylist_node * mkey_list,
                    krb5_db_entry      * entry,
                    krb5_keyblock      ** mkey);

krb5_error_code
krb5_dbe_lookup_mkvno( krb5_context    context,
                       krb5_db_entry * entry,
                       krb5_kvno     * mkvno);

krb5_error_code
krb5_dbe_lookup_mod_princ_data( krb5_context          context,
                                krb5_db_entry       * entry,
                                krb5_timestamp      * mod_time,
                                krb5_principal      * mod_princ);

krb5_error_code
krb5_dbe_lookup_mkey_aux( krb5_context         context,
                          krb5_db_entry      * entry,
                          krb5_mkey_aux_node ** mkey_aux_data_list);
krb5_error_code
krb5_dbe_update_mkvno( krb5_context    context,
                       krb5_db_entry * entry,
                       krb5_kvno       mkvno);

krb5_error_code
krb5_dbe_lookup_actkvno( krb5_context         context,
                         krb5_db_entry      * entry,
                         krb5_actkvno_node ** actkvno_list);

krb5_error_code
krb5_dbe_update_mkey_aux( krb5_context          context,
                          krb5_db_entry       * entry,
                          krb5_mkey_aux_node  * mkey_aux_data_list);

krb5_error_code
krb5_dbe_update_actkvno(krb5_context    context,
                        krb5_db_entry * entry,
                        const krb5_actkvno_node *actkvno_list);

krb5_error_code
krb5_dbe_update_last_pwd_change( krb5_context     context,
                                 krb5_db_entry  * entry,
                                 krb5_timestamp   stamp);

krb5_error_code
krb5_dbe_lookup_tl_data( krb5_context          context,
                         krb5_db_entry       * entry,
                         krb5_tl_data        * ret_tl_data);

krb5_error_code
krb5_dbe_create_key_data( krb5_context          context,
                          krb5_db_entry       * entry);


krb5_error_code
krb5_dbe_update_mod_princ_data( krb5_context          context,
                                krb5_db_entry       * entry,
                                krb5_timestamp        mod_date,
                                krb5_const_principal  mod_princ);

void *krb5_db_alloc( krb5_context kcontext,
                     void *ptr,
                     size_t size );

void krb5_db_free( krb5_context kcontext,
                   void *ptr);


krb5_error_code
krb5_dbe_lookup_last_pwd_change( krb5_context          context,
                                 krb5_db_entry       * entry,
                                 krb5_timestamp      * stamp);

krb5_error_code
krb5_dbe_delete_tl_data( krb5_context    context,
                         krb5_db_entry * entry,
                         krb5_int16      tl_data_type);

krb5_error_code
krb5_dbe_update_tl_data( krb5_context          context,
                         krb5_db_entry       * entry,
                         krb5_tl_data        * new_tl_data);

krb5_error_code
krb5_dbe_cpw( krb5_context        kcontext,
              krb5_keyblock       * master_key,
              krb5_key_salt_tuple       * ks_tuple,
              int                         ks_tuple_count,
              char              * passwd,
              int                         new_kvno,
              krb5_boolean        keepold,
              krb5_db_entry     * db_entry);


krb5_error_code
krb5_dbe_ark( krb5_context        context,
              krb5_keyblock       * master_key,
              krb5_key_salt_tuple       * ks_tuple,
              int                         ks_tuple_count,
              krb5_db_entry     * db_entry);

krb5_error_code
krb5_dbe_crk( krb5_context        context,
              krb5_keyblock       * master_key,
              krb5_key_salt_tuple       * ks_tuple,
              int                         ks_tuple_count,
              krb5_boolean        keepold,
              krb5_db_entry     * db_entry);

krb5_error_code
krb5_dbe_apw( krb5_context        context,
              krb5_keyblock       * master_key,
              krb5_key_salt_tuple       * ks_tuple,
              int                         ks_tuple_count,
              char              * passwd,
              krb5_db_entry     * db_entry);

int
krb5_db_get_key_data_kvno( krb5_context    context,
                           int             count,
                           krb5_key_data * data);


/* default functions. Should not be directly called */
/*
 *   Default functions prototype
 */

krb5_error_code
krb5_dbe_def_search_enctype( krb5_context kcontext,
                             krb5_db_entry *dbentp,
                             krb5_int32 *start,
                             krb5_int32 ktype,
                             krb5_int32 stype,
                             krb5_int32 kvno,
                             krb5_key_data **kdatap);

krb5_error_code
krb5_def_store_mkey( krb5_context context,
                     char *keyfile,
                     krb5_principal mname,
                     krb5_kvno kvno,
                     krb5_keyblock *key,
                     char *master_pwd);

krb5_error_code
krb5_def_store_mkey_list( krb5_context context,
                          char *keyfile,
                          krb5_principal mname,
                          krb5_keylist_node *keylist,
                          char *master_pwd);

krb5_error_code
krb5_db_def_fetch_mkey( krb5_context   context,
                        krb5_principal mname,
                        krb5_keyblock *key,
                        krb5_kvno     *kvno,
                        char          *db_args);

krb5_error_code
krb5_def_verify_master_key( krb5_context   context,
                            krb5_principal mprinc,
                            krb5_kvno      kvno,
                            krb5_keyblock *mkey);

krb5_error_code
krb5_def_fetch_mkey_list( krb5_context            context,
                          krb5_principal        mprinc,
                          const krb5_keyblock  *mkey,
                          krb5_kvno             mkvno,
                          krb5_keylist_node  **mkeys_list);

krb5_error_code kdb_def_set_mkey ( krb5_context kcontext,
                                   char *pwd,
                                   krb5_keyblock *key );

krb5_error_code kdb_def_set_mkey_list ( krb5_context kcontext,
                                        krb5_keylist_node *keylist );

krb5_error_code kdb_def_get_mkey ( krb5_context kcontext,
                                   krb5_keyblock **key );

krb5_error_code kdb_def_get_mkey_list ( krb5_context kcontext,
                                        krb5_keylist_node **keylist );

krb5_error_code
krb5_dbe_def_cpw( krb5_context    context,
                  krb5_keyblock       * master_key,
                  krb5_key_salt_tuple   * ks_tuple,
                  int                     ks_tuple_count,
                  char          * passwd,
                  int                     new_kvno,
                  krb5_boolean    keepold,
                  krb5_db_entry * db_entry);

krb5_error_code
krb5_def_promote_db(krb5_context, char *, char **);

krb5_error_code
krb5_dbekd_def_decrypt_key_data( krb5_context             context,
                                 const krb5_keyblock    * mkey,
                                 const krb5_key_data    * key_data,
                                 krb5_keyblock          * dbkey,
                                 krb5_keysalt           * keysalt);

krb5_error_code
krb5_dbekd_def_encrypt_key_data( krb5_context             context,
                                 const krb5_keyblock    * mkey,
                                 const krb5_keyblock    * dbkey,
                                 const krb5_keysalt     * keysalt,
                                 int                      keyver,
                                 krb5_key_data          * key_data);

krb5_error_code
krb5_dbekd_def_decrypt_key_data( krb5_context     context,
                                 const krb5_keyblock    * mkey,
                                 const krb5_key_data    * key_data,
                                 krb5_keyblock  * dbkey,
                                 krb5_keysalt   * keysalt);

krb5_error_code
krb5_dbekd_def_encrypt_key_data( krb5_context             context,
                                 const krb5_keyblock    * mkey,
                                 const krb5_keyblock    * dbkey,
                                 const krb5_keysalt     * keysalt,
                                 int                      keyver,
                                 krb5_key_data          * key_data);

krb5_error_code
krb5_db_create_policy( krb5_context kcontext,
                       osa_policy_ent_t policy);

krb5_error_code
krb5_db_get_policy ( krb5_context kcontext,
                     char *name,
                     osa_policy_ent_t *policy,
                     int *nentries);

krb5_error_code
krb5_db_put_policy( krb5_context kcontext,
                    osa_policy_ent_t policy);

krb5_error_code
krb5_db_iter_policy( krb5_context kcontext,
                     char *match_entry,
                     osa_adb_iter_policy_func func,
                     void *data);

krb5_error_code
krb5_db_delete_policy( krb5_context kcontext,
                       char *policy);

void
krb5_db_free_policy( krb5_context kcontext,
                     osa_policy_ent_t policy);


krb5_error_code
krb5_db_set_context(krb5_context, void *db_context);

krb5_error_code
krb5_db_get_context(krb5_context, void **db_context);

void
krb5_dbe_free_key_data_contents(krb5_context, krb5_key_data *);

void
krb5_dbe_free_key_list(krb5_context, krb5_keylist_node *);

void
krb5_dbe_free_actkvno_list(krb5_context, krb5_actkvno_node *);

void
krb5_dbe_free_mkey_aux_list(krb5_context, krb5_mkey_aux_node *);

void
krb5_dbe_free_tl_data(krb5_context, krb5_tl_data *);

#define KRB5_KDB_DEF_FLAGS      0

#define KDB_MAX_DB_NAME                 128
#define KDB_REALM_SECTION               "realms"
#define KDB_MODULE_POINTER              "database_module"
#define KDB_MODULE_DEF_SECTION          "dbdefaults"
#define KDB_MODULE_SECTION              "dbmodules"
#define KDB_LIB_POINTER                 "db_library"
#define KDB_DATABASE_CONF_FILE          DEFAULT_SECURE_PROFILE_PATH
#define KDB_DATABASE_ENV_PROF           KDC_PROFILE_ENV

#define KRB5_KDB_OPEN_RW                0
#define KRB5_KDB_OPEN_RO                1

#define KRB5_KDB_OPT_SET_DB_NAME        0
#define KRB5_KDB_OPT_SET_LOCK_MODE      1

/*
 * A krb5_context can hold one database object.  Modules should use
 * context->dal_handle->db_context to store state associated with the database
 * object.
 *
 * Some module functions are mandatory for KDC operation; others are optional
 * or apply only to administrative operations.  If a function is optional, a
 * module can leave the function pointer as NULL.  Alternatively, modules can
 * return KRB5_KDB_DBTYPE_NOSUP when asked to perform an inapplicable action.
 *
 * Some module functions have default implementations which will call back into
 * the vtable interface.  Leave these functions as NULL to use the default
 * implementations.
 *
 * The documentation in these comments describes the DAL as it is currently
 * implemented and used, not as it should be.  So if anything seems off, that
 * probably means the current state of things is off.
 */

typedef struct _kdb_vftabl {
    short int maj_ver;
    short int min_ver;

    /*
     * Mandatory: Invoked after the module library is loaded, when the first DB
     * using the module is opened, across all contexts.
     */
    krb5_error_code (*init_library)(void);

    /*
     * Mandatory: Invoked before the module library is unloaded, after the last
     * DB using the module is closed, across all contexts.
     */
    krb5_error_code (*fini_library)(void);

    /*
     * Mandatory: Initialize a database object.  Profile settings should be
     * read from conf_section inside KDB_MODULE_SECTION.  db_args communicates
     * command-line arguments for module-specific flags.  mode will be one of
     * KRB5_KDB_OPEN_{RW,RO} or'd with one of
     * KRB5_KDB_SRV_TYPE_{KDC,ADMIN,PASSWD,OTHER}.
     *
     * A db_args value of "temporary" is generated programattically by
     * kdb5_util load.  If this db_args value is present, the module should
     * open a side copy of the database suitable for loading in a propagation
     * from master to slave.  This side copy will later be promoted with
     * promote_db, allowing complete updates of the DB with no loss in read
     * availability.  If the module cannot comply with this architecture, it
     * should return an error.
     */
    krb5_error_code (*init_module)(krb5_context kcontext, char *conf_section,
                                   char **db_args, int mode);

    /*
     * Mandatory: Finalize the database object contained in a context.  Free
     * any state contained in the db_context pointer and null it out.
     */
    krb5_error_code (*fini_module)(krb5_context kcontext);

    /*
     * Optional: Create, but do not open, a database.  conf_section and db_args
     * have the same meaning as in init_module.  This function may return an
     * error if the database already exists.  Used by kdb5_util create.
     */
    krb5_error_code (*db_create)(krb5_context kcontext, char *conf_section,
                                 char **db_args);

    /*
     * Optional: Destroy a database.  conf_section and db_args have the same
     * meaning as in init_module.  Used by kdb5_util destroy.  In current
     * usage, the database is destroyed while open, so the module should handle
     * that.
     */
    krb5_error_code (*db_destroy)(krb5_context kcontext, char *conf_section,
                                  char **db_args);

    /*
     * Optional: Set *age to the last modification time of the database.  Used
     * by the KDC lookaside cache to ensure that lookaside entries are not used
     * if the database has changed since the entry was recorded.
     *
     * If this function is unimplemented, lookaside cache entries will
     * effectively expire immediately.  Another option is to supply the current
     * time, which will cause lookaside cache entries to last for one second.
     */
    krb5_error_code (*db_get_age)(krb5_context kcontext, char *db_name,
                                  time_t *age);

    /*
     * Optional: Set a database option.  This function is not currently used by
     * any callers.  Behavior depends on the value of option:
     *
     * KRB5_KDB_OPT_SET_DB_NAME: Interpret value as a C string.  Set the
     * database name (e.g. a filename for an embedded database).
     *
     * KRB5_KDB_OPT_SET_LOCK_MODE: Interpret value as a pointer to
     * krb5_boolean.  If *value points to TRUE, set the database to
     * non-blocking lock mode, causing operations to return OSA_ADB_CANTLOCK_DB
     * when it would otherwise wait to obtain a lock.  Set *value to the old
     * value of the non-blocking flag.
     */
    krb5_error_code (*db_set_option)(krb5_context kcontext, int option,
                                     void *value);

    /*
     * Optional: Lock the database, with semantics depending on the mode
     * argument:
     *
     * KRB5_DB_LOCKMODE_SHARED: Lock may coexist with other shared locks.
     * KRB5_DB_LOCKMODE_EXCLUSIVE: Lock may not coexist with other locks.
     * KRB5_DB_LOCKMODE_PERMANENT: Exclusive lock surviving process exit.
     * (KRB5_DB_LOCKMODE_DONTBLOCK is unused and unimplemented.)
     *
     * Used by the "kadmin lock" command, incremental propagation, and
     * kdb5_util dump.  Incremental propagation support requires shared locks
     * to operate.  kdb5_util dump will work if the module returns
     * KRB5_PLUGIN_OP_NOTSUPP; note that this is *not* the usual "operation
     * not supported" error code.
     */
    krb5_error_code (*db_lock)(krb5_context kcontext, int mode);

    /* Optional: Release a lock created with db_lock. */
    krb5_error_code (*db_unlock)(krb5_context kcontext);

    /*
     * Mandatory: Fill in *entries with the entry for the principal search_for.
     * The module must allocate each entry field separately, as callers may
     * free individual fields using db_free.  If the principal is not found,
     * set *nentries to 0 and return success.  The meaning of flags are as
     * follows (some of these may be processed by db_invoke methods such as
     * KRB5_KDB_METHOD_SIGN_AUTH_DATA rather than by db_get_principal):
     *
     * KRB5_KDB_FLAG_CANONICALIZE: Indicates that a KDC client requested name
     *     canonicalization.  The module may return an out-of-realm referral by
     *     filling in an out-of-realm principal name in entries->princ and null
     *     values elsewhere.  The module may return an in-realm alias by
     *     filling in an in-realm principal name in entries->princ other than
     *     the one requested.
     *
     * KRB5_KDB_INCLUDE_PAC: Set by the KDC during an AS request when the
     *     client requested PAC information during padata, and during most TGS
     *     requests.  Indicates that the module should include PAC information
     *     when generating authorization data.
     *
     * KRB5_KDB_FLAG_CLIENT_REFERRALS_ONLY: Set by the KDC when looking up the
     *     client entry in an AS request.  Indicates that the module should
     *     return out-of-realm referral information in lieu of cross-realm TGT
     *     information.
     *
     * KRB5_KDB_FLAG_MAP_PRINCIPALS: Set by the KDC when looking up the client
     *     entry during TGS requests, except for S4U TGS requests and requests
     *     where the server entry has the KRB5_KDB_NO_AUTH_DATA_REQUIRED
     *     attribute.  Indicates that the module should map foreign principals
     *     to local principals if it supports doing so.
     *
     * KRB5_KDB_FLAG_PROTOCOL_TRANSITION: Set by the KDC when looking up the
     *     client entry during an S4U2Self TGS request.  This affects the PAC
     *     information which should be included when authorization data is
     *     generated; see the Microsoft S4U specification for details.
     *
     * KRB5_KDB_FLAG_CONSTRAINED_DELEGATION: Set by the KDC when looking up the
     *     client entry during an S4U2Proxy TGS request.  Also affects PAC
     *     generation.
     *
     * KRB5_KDB_FLAG_CROSS_REALM: Set by the KDC when looking up a client entry
     *     during a TGS request, if the client principal is not part of the
     *     realm being served.
     *
     * The nentries and more arguments appear to be intended to account for
     * multiple entries for a principal, but this functionality is neither
     * implemented nor used.  *nentries is set to 1 by all callers and should
     * be set to 0 or 1 on return; the module should always set *more to FALSE.
     * Callers will typically error out if modules behave otherwise.
     */
    krb5_error_code (*db_get_principal)(krb5_context kcontext,
                                        krb5_const_principal search_for,
                                        unsigned int flags,
                                        krb5_db_entry *entries, int *nentries,
                                        krb5_boolean *more);

    /*
     * Mandatory: Free the memory associated with principal entries.  Do not
     * free entry itself.  All callers ignore the return value.  Entries may
     * have been constructed by the caller (using the db_alloc function to
     * allocate associated memory); thus, a plugin must allocate each field
     * of a principal entry separately.
     */
    krb5_error_code (*db_free_principal)(krb5_context kcontext,
                                         krb5_db_entry *entry, int count);

    /*
     * Optional: Create or modify one or more principal entries.  All callers
     * operate on a single entry.  db_args communicates command-line arguments
     * for module-specific flags.
     *
     * The mask field of an entry indicates the changed fields.  Mask values
     * are defined in kadmin's admin.h header.  If KADM5_PRINCIPAL is set in
     * the mask, the entry is new; otherwise it already exists.  All fields of
     * an entry are expected to contain correct values, regardless of whether
     * they are specified in the mask, so it is acceptable for a module to
     * ignore the mask and update the entire entry.
     */
    krb5_error_code (*db_put_principal)(krb5_context kcontext,
                                        krb5_db_entry *entries, int *nentries,
                                        char **db_args);

    /*
     * Optional: Delete the entry for the principal search_for.  If the
     * principal does not exist, set *nentries to 0 and return success; if it
     * did exist, set *nentries to 1.
     */
    krb5_error_code (*db_delete_principal)(krb5_context kcontext,
                                           krb5_const_principal search_for,
                                           int *nentries);

    /*
     * Optional: For each principal entry in the database, invoke func with the
     * argments func_arg and the entry data.  If match_entry is specified, the
     * module may narrow the iteration to principal names matching that regular
     * expression; a module may alternatively ignore match_entry.
     */
    krb5_error_code (*db_iterate)(krb5_context kcontext,
                                  char *match_entry,
                                  int (*func)(krb5_pointer, krb5_db_entry *),
                                  krb5_pointer func_arg);

    /*
     * Optional: Create a password policy entry.  Return an error if the policy
     * already exists.
     */
    krb5_error_code (*db_create_policy)(krb5_context kcontext,
                                        osa_policy_ent_t policy);

    /*
     * Optional: If a password policy entry exists with the name name, allocate
     * a policy entry in *policy, fill it in with the policy information, and
     * set *cnt to 1.  If the entry does not exist, set *cnt to 0 and return
     * success, or return an error (existing module implementations are not
     * consistent).
     */
    krb5_error_code (*db_get_policy)(krb5_context kcontext, char *name,
                                     osa_policy_ent_t *policy, int *cnt);

    /*
     * Optional: Modify an existing password policy entry to match the values
     * in policy.  Return an error if the policy does not already exist.
     */
    krb5_error_code (*db_put_policy)(krb5_context kcontext,
                                     osa_policy_ent_t policy);

    /*
     * Optional: For each password policy entry in the database, invoke func
     * with the argments data and the entry data.  If match_entry is specified,
     * the module may narrow the iteration to policy names matching that
     * regular expression; a module may alternatively ignore match_entry.
     */
    krb5_error_code (*db_iter_policy)(krb5_context kcontext, char *match_entry,
                                      osa_adb_iter_policy_func func,
                                      void *data);

    /*
     * Optional: Delete the password policy entry with the name policy.  Return
     * an error if the entry does not exist.
     */
    krb5_error_code (*db_delete_policy)(krb5_context kcontext, char *policy);

    /* Optional: Free a policy entry returned by db_get_policy. */
    void (*db_free_policy)(krb5_context kcontext, osa_policy_ent_t val);

    /*
     * Optional: Fill in *realms with an array of realm names.  This function
     * is not used or implemented.
     */
    krb5_error_code (*db_supported_realms)(krb5_context kcontext,
                                           char **realms);

    /* Optional: Free a realm list returned by db_supported_realms. */
    krb5_error_code (*db_free_supported_realms)(krb5_context kcontext,
                                                char **realms);

    /*
     * Optional: Convert an error code returned by a module function (casted
     * from krb5_error_code to long) into a string.  If this function is
     * implemented, libkdb5 will invoke it and call krb5_set_error_message with
     * the result.  This function may never return NULL.
     *
     * This function is not productively implemented in current modules, and it
     * is better for a module to simply call krb5_set_error_message inside
     * modules when appropriate.
     */
    const char *(*errcode_2_string)(krb5_context kcontext, long err_code);

    /* Optional: Free an error string returned by errcode_2_string. */
    void (*release_errcode_string)(krb5_context kcontext, const char *msg);

    /*
     * Mandatory: Has the semantics of realloc(ptr, size).  Callers use this to
     * allocate memory for new or changed principal entries, so the module
     * should expect to potentially see this memory in db_free_principal.
     */
    void *(*db_alloc)(krb5_context kcontext, void *ptr, size_t size);

    /*
     * Mandatory: Has the semantics of free(ptr).  Callers use this to free
     * fields from a principal entry (such as key data) before changing it in
     * place, and in some cases to free data they allocated with db_alloc.
     */
    void (*db_free)(krb5_context kcontext, void *ptr);

    /*
     * Optional with default: Inform the module of the master key.  The module
     * may remember an alias to the provided memory.  This function is called
     * at startup by the KDC and kadmind; both supply a NULL pwd argument.  The
     * module should not need to use a remembered master key value, so current
     * modules do nothing with it besides return it from get_master_key, which
     * is never used.  The default implementation does nothing.
     */
    krb5_error_code (*set_master_key)(krb5_context kcontext, char *pwd,
                                      krb5_keyblock *key);

    /*
     * Optional with default: Retrieve an alias to the master keyblock as
     * previously set by set_master_key.  This function is not used.  The
     * default implementation returns success without modifying *key, which
     * would be an invalid implementation if it were ever used.
     */
    krb5_error_code (*get_master_key)(krb5_context kcontext,
                                      krb5_keyblock **key);

    /*
     * Optional with default: Inform the module of the master key.  The module
     * may remember an alias to the provided memory.  This function is called
     * at startup by the KDC and kadmind with the value returned by
     * fetch_master_key_list.  The default implementation does nothing.
     */
    krb5_error_code (*set_master_key_list)(krb5_context kcontext,
                                           krb5_keylist_node *keylist);

    /*
     * Optional with default: Retrieve an alias to the master key list as
     * previously set by set_master_key_list.  This function is used by the KDB
     * keytab implementation in libkdb5, which is used by kadmind.  The default
     * implementation returns success without modifying *keylist, which is an
     * invalid implementation.
     */
    krb5_error_code (*get_master_key_list)(krb5_context kcontext,
                                           krb5_keylist_node **keylist);

    /* This function has no entry point in libkdb5; leave it as NULL. */
    krb5_error_code (*setup_master_key_name)(krb5_context kcontext,
                                             char *keyname, char *realm,
                                             char **fullname,
                                             krb5_principal *principal);

    /*
     * Optional with default: Save a master keyblock into the stash file
     * db_arg.  master_pwd indicates the password used to derive the keyblock,
     * if it is known.  mname is the name of the master principal for the
     * realm.
     *
     * The default implementation ignores master_pwd and saves the master key
     * in a keytab-format file.
     */
    krb5_error_code (*store_master_key)(krb5_context kcontext, char *db_arg,
                                        krb5_principal mname, krb5_kvno kvno,
                                        krb5_keyblock *key, char *master_pwd);

    /*
     * Optional with default: Retrieve a master keyblock from the stash file
     * db_args, filling in *key and *kvno.  mname is the name of the master
     * principal for the realm.
     *
     * The default implementation reads the master keyblock from a keytab or
     * old-format stash file.
     */
    krb5_error_code (*fetch_master_key)(krb5_context kcontext,
                                        krb5_principal mname,
                                        krb5_keyblock *key, krb5_kvno *kvno,
                                        char *db_args);

    /*
     * Optional with default: Verify that the keyblock mkey is a valid master
     * key for the realm.  This function used to be used by the KDC and
     * kadmind, but is now used only by kdb5_util dump -mkey_convert.
     *
     * The default implementation retrieves the master key principal and
     * attempts to decrypt its key with mkey.  This only works for the current
     * master keyblock.
     */
    krb5_error_code (*verify_master_key)(krb5_context kcontext,
                                         krb5_principal mprinc, krb5_kvno kvno,
                                         krb5_keyblock *mkey);

    /*
     * Optional with default: Given a keyblock for some version of the
     * database's master key, fetch the decrypted master key values from the
     * database and store the list into *mkeys_list.  The caller will free
     * *mkeys_list using a libkdb5 function which uses the standard free()
     * function, so the module must not use a custom allocator.
     *
     * The default implementation tries the key against the current master key
     * data and all KRB5_TL_MKEY_AUX values, which contain copies of the master
     * keys encrypted with old master keys.
     */
    krb5_error_code (*fetch_master_key_list)(krb5_context kcontext,
                                             krb5_principal mname,
                                             const krb5_keyblock *key,
                                             krb5_kvno kvno,
                                             krb5_keylist_node **mkeys_list);

    /*
     * Optional with default: Save a list of master keyblocks, obtained from
     * fetch_master_key_list, into the stash file db_arg.  The caller will set
     * master_pwd to NULL, so the module should just ignore it.  mname is the
     * name of the master principal for the realm.
     *
     * The default implementation saves the list of master keys in a
     * keytab-format file.
     */
    krb5_error_code (*store_master_key_list)(krb5_context kcontext,
                                             char *db_arg,
                                             krb5_principal mname,
                                             krb5_keylist_node *keylist,
                                             char *master_pwd);

    /*
     * Optional with default: Starting at position *start, scan the key data of
     * a database entry for a key matching the enctype ktype, the salt type
     * stype, and the version kvno.  Store the resulting key into *kdatap and
     * set *start to the position after the key found.  If ktype is negative,
     * match any enctype.  If stype is negative, match any salt type.  If kvno
     * is zero or negative, find the most recent key version satisfying the
     * other constraints.
     */
    krb5_error_code (*dbe_search_enctype)(krb5_context kcontext,
                                          krb5_db_entry *dbentp,
                                          krb5_int32 *start, krb5_int32 ktype,
                                          krb5_int32 stype, krb5_int32 kvno,
                                          krb5_key_data **kdatap);


    /*
     * Optional with default: Change the key data for db_entry to include keys
     * derived from the password passwd in each of the specified key-salt
     * types, at version new_kvno.  Discard the old key data if keepold is not
     * set.
     *
     * The default implementation uses the keyblock master_key to encrypt each
     * new key, via the function dbekd_encrypt_key_data.
     */
    krb5_error_code (*db_change_pwd)(krb5_context context,
                                     krb5_keyblock *master_key,
                                     krb5_key_salt_tuple *ks_tuple,
                                     int ks_tuple_count, char *passwd,
                                     int new_kvno, krb5_boolean keepold,
                                     krb5_db_entry *db_entry);

    /*
     * Optional with default: Promote a temporary database to be the live one.
     * kdb5_util load opens the database with the "temporary" db_arg and then
     * invokes this function when the load is complete, thus replacing the live
     * database with no loss of read availability.
     *
     * The default implementation returns KRB5_PLUGIN_OP_NOTSUPP (which is
     * *not* the usual "operation not supported" error code); kdb5_util dump
     * recognizes and ignores this error code.
     */
    krb5_error_code (*promote_db)(krb5_context context, char *conf_section,
                                  char **db_args);

    /*
     * Optional with default: Decrypt the key in key_data with master keyblock
     * mkey, placing the result into dbkey.  Copy the salt from key_data, if
     * any, into keysalt.  Either dbkey or keysalt may be left unmodified on
     * successful return if key_data does not contain key or salt information.
     *
     * The default implementation expects the encrypted key (in krb5_c_encrypt
     * format) to be stored in key_data_contents[0], with length given by
     * key_data_length[0].  If key_data_ver is 2, it expects the salt to be
     * stored, unencrypted, in key_data_contents[1], with length given by
     * key_data_length[1].
     */
    krb5_error_code (*dbekd_decrypt_key_data)(krb5_context kcontext,
                                              const krb5_keyblock *mkey,
                                              const krb5_key_data *key_data,
                                              krb5_keyblock *dbkey,
                                              krb5_keysalt *keysalt);

    /*
     * Optional with default: Encrypt dbkey with master keyblock mkey, placing
     * the result into key_data along with keysalt.
     *
     * The default implementation stores the encrypted key (in krb5_c_encrypt
     * format) in key_data_contents[0] and the length in key_data_length[0].
     * If keysalt is specified, it sets key_data_ver to 2, and stores the salt
     * in key_data_contents[1] and its length in key_data_length[1].  If
     * keysalt is not specified, key_data_ver is set to 1.
     */
    krb5_error_code (*dbekd_encrypt_key_data)(krb5_context kcontext,
                                              const krb5_keyblock *mkey,
                                              const krb5_keyblock *dbkey,
                                              const krb5_keysalt *keysalt,
                                              int keyver,
                                              krb5_key_data *key_data);

    /*
     * Optional: Perform an operation on input data req with output stored in
     * rep.  Return KRB5_KDB_DBTYPE_NOSUP if the module does not implement the
     * method.  Defined methods are:
     *
     * KRB5_KDB_METHOD_SIGN_AUTH_DATA: req contains a krb5_sign_auth_data_req
     *     structure.  Generate signed authorization data, such as a Windows
     *     PAC, for the ticket to be returned to the client.  Place the signed
     *     authorization data in rep using a krb5_sign_auth_data_rep structure.
     *     This function will be invoked for an AS request if the client
     *     included padata requesting a PAC.  This function will be invoked for
     *     a TGS request if there is authorization data in the TGT, if the
     *     client is from another realm, or if the TGS request is an S4U2Self
     *     or S4U2Proxy request.
     *
     * KRB5_KDB_METHOD_CHECK_TRANSITED_REALMS: req contains a
     *     kdb_check_transited_realms_req structure.  Perform a policy check on
     *     a cross-realm ticket's transited field and return an error (other
     *     than KRB5_KDB_DBTYPE_NOSUP) if the check fails.  Leave rep alone.
     *
     * KRB5_KDB_METHOD_CHECK_POLICY_AS: req contains a kdb_check_policy_as_req
     *     structure.  Perform a policy check on an AS request, in addition to
     *     the standard policy checks.  Return 0 if the AS request is allowed
     *     or an appropriate error (such as KDC_ERR_POLICY) if it is
     *     disallowed.  Place in rep a kdb_check_policy_as_rep structure
     *     containing a status string and e_data value to return to the client
     *     if the policy check fails.  The status string may be NULL, but must
     *     not contain allocated data as it will not be freed.  The e_data
     *     structure may be empty; if not, it will be freed by the caller using
     *     the standard free function.
     *
     * KRB5_KDB_METHOD_CHECK_POLICY_TGS: Same as above, except the structures
     *     are kdb_check_policy_tgs_req and kdb_check_policy_tgs_rep.
     *
     * KRB5_KDB_METHOD_AUDIT_AS: req contains a kdb_audit_as_req structure.
     *     Informs the module of a successful or unsuccessful AS request.  Do
     *     not place any data in rep.
     *
     * KRB5_KDB_METHOD_AUDIT_TGS: Same as above, except req contains a
     *     kdb_audit_tgs_req structure.
     *
     * KRB5_KDB_METHOD_REFRESH_POLICY: req and rep are NULL.  Informs the
     *     module that the KDC received a request to reload configuration
     *     (that is, a SIGHUP).
     *
     * KRB5_KDB_METHOD_CHECK_ALLOWED_TO_DELEGATE: req contains a
     *     kdb_check_allowed_to_delegate_req structure.  Perform a policy check
     *     on proxy being allowed to act on behalf of client to server.  Return
     *     0 if policy allows it, or an appropriate error (such as
     *     KRB5KDC_ERR_POLICY) if not.  If this method is not implemented, all
     *     S4U2Proxy delegation requests will be rejected.  Do not place any
     *     data in rep.
     */
    krb5_error_code (*db_invoke)(krb5_context context, unsigned int method,
                                 const krb5_data *req, krb5_data *rep);
} kdb_vftabl;

#endif /* !defined(_WIN32) */

#endif /* KRB5_KDB5__ */
