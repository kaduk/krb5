/* std2ps.c - stdio-backed abstraction for PStreams */

/* 
 * isode/psap/std2ps.c
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include <stdio.h>
#include "psap.h"

/*  */

/* ARGSUSED */

static int  std_read (ps, data, n, in_line)
PS	ps;
PElementData data;
PElementLen n;
int	in_line;
{
    int	    i;

    if ((i = fread ((char *) data, sizeof *data, (int) n,
		    (FILE *) ps -> ps_addr)) == NOTOK)
	ps -> ps_errno = PS_ERR_IO;

    return i;
}


/* ARGSUSED */

static int  std_write (ps, data, n, in_line)
PS	ps;
PElementData data;
PElementLen n;
int	in_line;
{
    int	    i;


    if ((i = fwrite ((char *) data, sizeof *data, (int) n,
		     (FILE *) ps -> ps_addr)) == NOTOK)
	ps -> ps_errno = PS_ERR_IO;

    return i;
}


int  std_flush (ps)
PS	ps;
{
    if (fflush ((FILE *) ps -> ps_addr) != EOF)
	return OK;

    return ps_seterr (ps, PS_ERR_IO, NOTOK);
}

/*  */

int	std_open (ps)
register PS	ps;
{
    ps -> ps_readP = std_read;
    ps -> ps_writeP = std_write;
    ps -> ps_flushP = std_flush;

    return OK;
}
