
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2008 Steven H.Levine

  Case insensitive strings

  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#include <string.h>
#include <ctype.h>

#include <os2.h>

#include "stristr.h"

CHAR *stristr(const register CHAR * t, const CHAR * s)
{
  // case-insensitive strstr()

  const register CHAR *t1, *s1;

  while (*t) {
    t1 = t;
    s1 = s;
    while (*s1) {
      if (toupper(*s1) != toupper(*t))
	break;
      else {
	s1++;
	t++;
      }
    }
    if (!*s1)
      return (CHAR *)t1;
    t = t1 + 1;
  }
  return NULL;
}

#if 0	// JBS	11 Sep 08
CHAR *strnistr(register CHAR * t, CHAR * s, LONG len)
{
  // case-insensitive strnstr()

  register CHAR *s1;
  register LONG len2;

  len2 = 0;
  while (len > len2) {
    s1 = s;
    while (len2 < len) {
      if (toupper(*s1) != toupper(t[len2]))
	break;
      else {
	len2++;
	s1++;
      }
    }
    if (!*s1)
      return t + (len2 - strlen(s));
    len2++;
  }
  return NULL;
}
#endif

CHAR *strnstr(register CHAR * t, CHAR * s, LONG len)
{
  register CHAR *s1;
  register LONG len2;

  len2 = 0;
  while (len > len2) {
    s1 = s;
    while (len2 < len) {
      if (*s1 != t[len2])
	break;
      else {
	len2++;
	s1++;
      }
    }
    if (!*s1)
      return t + (len2 - strlen(s));
    len2++;
  }
  return NULL;
}

CHAR *findstring(CHAR * findthis, ULONG lenthis, CHAR * findin,
		 ULONG lenin, BOOL sensitive)
{
  register CHAR *this, *in;
  register ULONG lenthis2, lenin2;

  if (!findthis || !findin || !lenthis || !lenin)
    return NULL;
  do {
    this = findthis;
    lenthis2 = lenthis;
    in = findin;
    lenin2 = lenin;
    while (lenthis2 && lenin2) {
      if (!sensitive) {
	if (toupper(*this) != toupper(*in))
	  break;
	else {
	  this++;
	  in++;
	  lenthis2--;
	  lenin2--;
	}
      }
      else {
	if (*this != *in)
	  break;
	else {
	  this++;
	  in++;
	  lenthis2--;
	  lenin2--;
	}
      }
    }
    if (lenthis2) {
      lenin--;
      findin++;
    }
  } while (lenin && lenthis2);
  return (lenthis2) ? NULL : in - lenthis;
}

#pragma alloc_text(MISC8,stristr,strnstr,strnistr,findstring)
