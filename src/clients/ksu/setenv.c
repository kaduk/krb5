/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* based on @(#)setenv.c        5.2 (Berkeley) 6/27/88 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "autoconf.h"

static char *_findenv(char *name, int *offset);

#ifndef HAVE_SETENV
extern int setenv(char *name, char *value, int rewrite);
#endif
#ifndef HAVE_UNSETENV
extern void unsetenv(char *name);
#endif

/*
 * setenv --
 *      Set the value of the environmental variable "name" to be
 *      "value".  If rewrite is set, replace any current value.
 */
#ifndef HAVE_SETENV
int
setenv(char *name, char *value, int rewrite)
{
    extern char **environ;
    /* if we've allocated space before */
    static int alloced;
    register char *C;
    int l_value, offset;

    /* Clean up from callers that left `=' in value. */
    if (*value == '=')
        ++value;
    l_value = strlen(value);
    /* find it if it already exists */
    C = _findenv(name, &offset);
    if (C != NULL) {
        if (!rewrite)
            return 0;
        if (strlen(C) >= l_value) {
            /* The old one is larger; just copy -- keep the trailing NUL. */
            strncpy(C, value, l_value + 1);
            return(0);
        }
    } else {
        /* Create a new slot. */
        register int cnt;
        register char **P;

        for (P = environ, cnt = 0; *P != NULL; ++P, ++cnt);
        if (alloced) {
            /* Just increase the size. */
            environ = realloc(environ, sizeof(char *) * (cnt + 2));
            if (environ == NULL)
                return -1;
        } else {
            /* Get new space and copy old entries into it. */
            P = malloc((sizeof(char *) * (cnt + 2)));
            if (P == NULL)
                return -1;
            memcpy(P, environ, cnt * sizeof(char *));
            environ = P;
        }
        environ[cnt + 1] = NULL;
        offset = cnt;
    }
    /* Trim any `=' from name. */
    for (C = name; *C != '\0' && *C != '='; ++C);
    /* name + '=' + value + '\0' */
    environ[offset] = malloc((size_t)(C - name) + l_value + 2);
    if (environ == NULL)
        return -1;
    for (C = environ[offset]; *name != '\0' && *name != '='; ++C, ++name)
        *C = *name;
    for (*C++ = '='; (*C++ = *value++) != NULL;);
    *C = '=';
    ++C;
    /* Need the NUL, too. */
    strncpy(C, value, l_value + 1);
    return 0;
}
#endif

/*
 * unsetenv(name) --
 *      Delete environmental variable "name".
 */
#ifndef HAVE_UNSETENV
void
unsetenv(char *name)
{
    extern char **environ;
    register char **P;
    int offset;

    /* Loop since it may be set multiple times. */
    while (_findenv(name, &offset) != NULL) {
        for (P = &environ[offset];; ++P) {
            *P = *(P + 1);
            if (*P == NULL)
                break;
        }
    }
}
#endif

/* based on @(#)getenv.c        5.5 (Berkeley) 6/27/88 */

/*
 * getenv --
 *      Returns ptr to value associated with name, if any, else NULL.
 */
#ifndef HAVE_GETENV
char *
getenv(char *name)
{
    int offset;

    return _findenv(name, &offset);
}
#endif

/*
 * _findenv --
 *      Returns pointer to value associated with name, if any, else NULL.
 *      Sets offset to be the offset of the name/value combination in the
 *      environmental array, for use by setenv(3) and unsetenv(3).
 *      Explicitly removes '=' in argument name.
 *
 */
static char *
_findenv(char *name, int *offset)
{
    extern char **environ;
    register int len;
    register char **P, *C;

    /* Get the length of the key we're looking up. */
    for (C = name, len = 0; *C != '\0' && *C != '='; ++C, ++len);
    /* Look for the name in the environ. */
    for (P = environ; *P != NULL; ++P) {
        if (!strncmp(*P, name, len)) {
            if (*(C = *P + len) == '=') {
                *offset = P - environ;
                return(++C);
            }
        }
    }
    return(NULL);
}
