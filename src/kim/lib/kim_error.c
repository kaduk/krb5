/*
 * $Header$
 *
 * Copyright 2006 Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 * require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
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
 */

#include "kim_private.h"
#include <com_err.h>
#include <CredentialsCache.h>

struct kim_error_opaque {
    kim_error_code code;
    kim_string message;
};

struct kim_error_opaque kim_no_memory_error_struct = { KIM_OUT_OF_MEMORY_ECODE, NULL, };
kim_error kim_no_memory_error = &kim_no_memory_error_struct;

struct kim_error_opaque kim_error_initializer = { KIM_NO_ERROR_ECODE, NULL };

/* ------------------------------------------------------------------------ */

static inline kim_error kim_error_allocate (kim_error      *out_error,
                                            kim_error_code  in_code)
{
    kim_error err = KIM_NO_ERROR;
    kim_error error = NULL;
    
    /* short circuit out of memory errors so we don't loop */
    switch (in_code) {
        case 0:
            return KIM_NO_ERROR;
            
        case KIM_OUT_OF_MEMORY_ECODE:
        case ENOMEM:
        case KRB5_CC_NOMEM:
        case ccErrNoMem:
            //        case klMemFullErr:
            //        case memFullErr:
            return kim_no_memory_error;
    }
    
    if (!err) {
        error = malloc (sizeof (*error));
        if (!error) {
            err = kim_no_memory_error;
        } else {
            *error = kim_error_initializer;
        }
    }
    
    if (!err) {
        error->code = in_code;
        
        *out_error = error;
        error = NULL;
    }
    
    kim_error_free (&error);
    
    return check_error (err);    
}

/* ------------------------------------------------------------------------ */

static kim_boolean kim_error_is_builtin (kim_error in_error)
{
    return (in_error == KIM_NO_ERROR ||
            in_error == kim_no_memory_error);
}

#pragma mark -- Helper Functions --

/* These helper functions exist so we get type checking on common errors */

/* ------------------------------------------------------------------------ */

kim_error _kim_error_create_for_param (kim_string in_function, 
                                       unsigned int in_argument_position,
                                       kim_string in_argument_name,
                                       kim_string in_invalid_value)
{
    return kim_error_create_from_code (KIM_PARAMETER_ECODE, 
                                       in_function, 
                                       in_argument_position, 
                                       in_argument_name,
                                       in_invalid_value);
}

#pragma mark -- Generic Functions --

/* ------------------------------------------------------------------------ */

kim_error kim_error_create_from_code (kim_error_code in_code, 
                                      ...)
{
    kim_error err = KIM_NO_ERROR;
    va_list args;
    
    va_start (args, in_code);
    err = kim_error_create_from_code_va (in_code, args);
    va_end (args);
    
    return check_error (err);    
}

/* ------------------------------------------------------------------------ */

kim_error kim_error_create_from_code_va (kim_error_code in_code, 
                                         va_list        in_args)
{
    kim_error err = KIM_NO_ERROR;
    kim_error error = KIM_NO_ERROR;
    
    if (!err) {
        err = kim_error_allocate (&error, in_code);
    }
    
    if (!err && !kim_error_is_builtin (error)) {
        err = kim_string_create_from_format_va_retcode (&error->message, 
                                                        error_message (in_code), 
                                                        in_args);
    }
    
    if (!err) {
        err = error;
        error = NULL;  /* take ownership */
    }
    
    kim_error_free (&error);
    
    return err;
}


/* ------------------------------------------------------------------------ */

kim_error kim_error_create_from_krb5_error (krb5_context    in_context, 
                                            krb5_error_code in_code)
{
    kim_error err = KIM_NO_ERROR;
    kim_error error = KIM_NO_ERROR;
    
    if (!err) {
        err = kim_error_allocate (&error, in_code);
    }
    
    if (!err && !kim_error_is_builtin (error)) {
        const char *message = krb5_get_error_message (in_context, in_code);
        
        err = kim_string_copy (&error->message, message);
        
        if (message) { krb5_free_error_message (in_context, message); }
    }
    
    if (!err) {
        err = error;
        error = NULL;  /* take ownership */
    }
    
    kim_error_free (&error);
    
    return err;
}
        
/* ------------------------------------------------------------------------ */

kim_error kim_error_copy (kim_error *out_error,
                          kim_error  in_error)
{
    kim_error err = KIM_NO_ERROR;
    kim_error error = KIM_NO_ERROR;
    
    if (!err && !out_error) { err = param_error (1, "out_error", "NULL"); }
    if (!err && !in_error ) { err = param_error (2, "in_error", "NULL"); }
    
    if (!err) { 
        if (kim_error_is_builtin (in_error)) {
            error = in_error;
            
        } else {
            err = kim_error_allocate (&error, in_error->code);
            
            if (!err && in_error->message) {
                err = kim_string_copy (&error->message, in_error->message);
            }
        }
    }
    
    if (!err) {
        *out_error = error;
        error = KIM_NO_ERROR;
    }
    
    kim_error_free (&error);
    
    return check_error (err);
}

/* ------------------------------------------------------------------------ */

kim_error_code kim_error_get_code (kim_error in_error)
{
    if (!in_error) {  /* KIM_NO_ERROR */
        return KIM_NO_ERROR_ECODE;
    } else {
        return in_error->code;
    }
}

/* ------------------------------------------------------------------------ */

kim_string kim_error_get_display_string (kim_error in_error)
{
    if (!in_error) {  /* KIM_NO_ERROR */
        return error_message (KIM_NO_ERROR_ECODE);
        
    } else if (in_error->message) {
        return in_error->message;
        
    } else {
        /* Note: kim_no_memory_error will get here because its string is NULL */
        return error_message (in_error->code);
    }
}

/* ------------------------------------------------------------------------ */

kim_error_code kim_error_free (kim_error *io_error)
{
    kim_error_code code = 0;
    
    if (io_error && *io_error) {
        code = (*io_error)->code;
        
        if (kim_error_is_builtin (*io_error)) {
            kim_string_free (&(*io_error)->message);
            free (*io_error);
            *io_error = KIM_NO_ERROR;
        }
    }
    
    return code;
}

#pragma mark -- Debugging Functions --

/* ------------------------------------------------------------------------ */

kim_error _check_error (kim_error  in_err, 
                        kim_string in_function, 
                        kim_string in_file, 
                        int        in_line)
{
    if (in_err) {
        kim_error_code code = kim_error_get_code (in_err);
        kim_string message = kim_error_get_display_string (in_err);
        
        kim_debug_printf ("%s(): got %d ('%s') at %s: %d", 
                          in_function, code, message, in_file, in_line);
    }
    
    return in_err;
}
