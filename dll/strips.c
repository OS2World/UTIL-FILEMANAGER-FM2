
/***********************************************************************

  $Id$

  String strippers

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  Revisions	01 Aug 04 SHL - Rework lstrip/rstrip usage

***********************************************************************/

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#pragma alloc_text(MISC8,strip_trail_char,strip_lead_char)

void strip_trail_char (char *pszStripChars,char *pszSrc) {

  char *psz;

  if(pszSrc && *pszSrc && pszStripChars && *pszStripChars) {
    psz = pszSrc + strlen(pszSrc) - 1;
    // while not empty and tail char in strip list
    while (*pszSrc && strchr(pszStripChars,*psz) != NULL) {
      *psz = 0;
      psz--;
    }
  }
}

void strip_lead_char (char *pszStripChars,char *pszSrc) {

  char *psz = pszSrc;

  if(pszSrc && *pszSrc && pszStripChars && *pszStripChars) {
    // while lead char in strip list
    while(*psz && strchr(pszStripChars,*psz) != NULL)
      psz++;
    if(psz != pszSrc)
      memmove(pszSrc,psz,strlen(psz) + 1);
  }
}

