/*
 * kdc/policy.c
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
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
 * Policy decision routines for KDC.
 */


#include <krb5/copyright.h>


#include <krb5/krb5.h>
#include <krb5/kdb.h>

#include "kdc_util.h"

int
against_local_policy_as(request, client, server, kdc_time, status)
register krb5_kdc_req *request;
krb5_db_entry client;
krb5_db_entry server;
krb5_timestamp kdc_time;
char	**status;
{
#if 0
     /* An AS request must include the addresses field */
    if (request->addresses == 0) {
	*status = "NO ADDRESS";
	return KRB5KDC_ERR_POLICY;
    }
#endif
    
    return 0;			/* not against policy */
}

/*
 * This is where local policy restrictions for the TGS should placed.
 */
krb5_error_code
against_local_policy_tgs(request, server, ticket, status)
register krb5_kdc_req *request;
krb5_db_entry server;
krb5_ticket *ticket;
char **status;
{
#if 0
    /*
     * For example, if your site wants to disallow ticket forwarding,
     * you might do something like this:
     */
    
    if (isflagset(request->kdc_options, KDC_OPT_FORWARDED)) {
	*status = "FORWARD POLICY";
	return KRB5KDC_ERR_POLICY;
    }
#endif
    
    return 0;				/* not against policy */
}







