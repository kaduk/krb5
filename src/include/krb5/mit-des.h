/*
 * include/krb5/mit-des.h
 *
 * Copyright 1989,1990 by the Massachusetts Institute of Technology.
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
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * MIT Data Encryption Standard software implementation declarations.
 */


#ifndef KRB5_MIT_DES__
#define KRB5_MIT_DES__

typedef krb5_octet mit_des_cblock[8];	/* crypto-block size */

/* Key schedule--used internally by DES routines to gain some speed */
typedef struct mit_des_ks_struct {
    mit_des_cblock _;
} mit_des_key_schedule[16];

#define MIT_DES_ENCRYPT	1
#define MIT_DES_DECRYPT	0

typedef struct mit_des_ran_key_seed {
    krb5_octet sequence_number[8];
    mit_des_key_schedule random_sequence_key;
} mit_des_random_key_seed;

/* the first byte of the key is already in the keyblock */

#define MIT_DES_BLOCK_LENGTH 		(8*sizeof(krb5_octet))
#define	MIT_DES_CBC_CRC_PAD_MINIMUM	CRC32_CKSUM_LENGTH
#define MIT_DES_KEYSIZE		 	(8*sizeof(krb5_octet))

#define MIT_DES_CBC_CKSUM_LENGTH	(4*sizeof(krb5_octet))

/* cryptosystem entry descriptor for MIT's DES encryption library */
extern krb5_cs_table_entry krb5_raw_des_cst_entry;
extern krb5_cs_table_entry krb5_des_crc_cst_entry;
extern krb5_cs_table_entry krb5_des_md5_cst_entry;
extern krb5_checksum_entry	krb5_des_cbc_cksumtable_entry;

#endif /* KRB5_MIT_DES__ */
