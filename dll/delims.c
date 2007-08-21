
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2007 Steven H.Levine

  Delimit chars

  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#include <os2.h>
#include <stdlib.h>
#include <string.h>

char *skip_delim(char *a, register char *delim)
{

  register char *p = a;

  if (p && delim) {
    while (*p) {
      if (strchr(delim, *p))
	p++;
      else
	break;
    }
  }
  return p;
}

char *to_delim(char *a, register char *delim)
{

  register char *p = a;

  if (p && delim) {
    while (*p) {
      if (strchr(delim, *p))
	break;
      p++;
    }
  }
  return p;
}

#pragma alloc_text(MISC8,skip_delim,to_delim)
