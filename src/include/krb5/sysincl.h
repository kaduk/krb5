/*
 * $Source$
 * $Author$
 * $Id$
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * For copying and distribution information, please see the file
 * <krb5/copyright.h>.
 *
 * System include files, for various things.
 */


#ifndef KRB5_SYSINCL__
#define KRB5_SYSINCL__

#ifndef KRB5_SYSTYPES__
#define KRB5_SYSTYPES__
#include <sys/types.h>			/* needed for much of the reset */
#endif /* KRB5_SYSTYPES__ */

#include <krb5/osconf.h>		/* USE*TIME_H macros */
#ifdef USE_TIME_H
#include <time.h>
#endif
#ifdef USE_SYS_TIME_H
#include <sys/time.h>			/* struct timeval, utimes() */
#endif
#include <sys/stat.h>			/* struct stat, stat() */
#include <sys/param.h>			/* MAXPATHLEN */
#if defined(unix) || defined(__unix__)
#include <sys/file.h>			/* prototypes for file-related
					   syscalls; flags for open &
					   friends */
#ifdef  SYSV
#include <sys/fcntl.h>
#endif

#ifndef L_SET
#define L_SET           0       /* absolute offset */
#define L_INCR          1       /* relative to current offset */
#define L_XTND          2       /* relative to end of file */
#endif /* L_SET */

#endif
#endif /* KRB5_SYSINCL__ */
