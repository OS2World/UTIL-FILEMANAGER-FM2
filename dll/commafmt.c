
/***********************************************************************

  $Id$

  Command formatting tools

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  Revisions	06 Jan 04 SHL - Drop hundfmt, clean commafmt

***********************************************************************/

/*
**  COMMAFMT.C
**  Public domain by Bob Stout
**
**  Notes:  1. Use static buffer to eliminate error checks on buffer overflow
**             and reduce code size.
**          2. By making the numeric argument a long and prototyping it before
**             use, passed numeric arguments will be implicitly cast to longs
**             thereby avoiding int overflow.
**          3. Use the thousands grouping and thousands separator from the
**             ANSI locale to make this more robust.
*/

#pragma alloc_text(MISC8,commafmt,hundfmt)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

size_t commafmt(char   *buf,            /* Buffer for formatted string  */
		int     bufsize,        /* Size of buffer               */
		long    N)              /* Number to convert            */
{
	int len = 1;
	int posn = 1;
	int sign = 1;

	char *ptr = buf + bufsize - 1;

	if (bufsize < 2)
	{
ABORT:          *buf = 0;
		return 0;
	}

	*ptr-- = 0;
	--bufsize;
	if (N < 0)
	{
		sign = -1;
		N = -N;
	}

	for ( ; len <= bufsize; ++len, ++posn)
	{
		*ptr-- = (char)((N % 10L) + '0');
		N /= 10;
		if (!N)
			break;
		if (posn % 3 == 0)
		{
			*ptr-- = ',';
			++len;
		}
		if (len >= bufsize)
			goto ABORT;
	}

	if (sign < 0)
	{
		if (bufsize == 0)
			goto ABORT;
		*ptr-- = '-';
		++len;
	}

	strcpy(buf, ++ptr);		// Left align
	return len;
}


#if 0 // fixme

size_t hundfmt (char *buf,int bufsize,unsigned long N) {

  char           tbuf[34];
  register char *pt,*p;
  register int   len;

  sprintf(tbuf,"%02lu",N);
  len = strlen(tbuf);
  pt = tbuf;
  p = buf;
  bufsize--;
  while(*pt && (p - buf) < bufsize) {
    if(len == 2) {
      *p = '.';
      p++;
    }
    *p = *pt;
    p++;
    pt++;
    len--;
  }
  *p = 0;
  return p - buf;
}

#endif 0 // fixme
