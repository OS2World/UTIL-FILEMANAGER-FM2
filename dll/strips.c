
/***********************************************************************

  $Id$

  String strippers

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  26 Jul 06 SHL Add chop_at_crnl
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Dec 07 GKY Add remove_first_occurence_of_character

***********************************************************************/

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

VOID chop_at_crnl(PSZ pszSrc)
{
  // Chop line at CR or NL
  PSZ psz = strchr(pszSrc, '\r');

  if (psz)
    *psz = 0;
  psz = strchr(pszSrc, '\n');
  if (psz)
    *psz = 0;
}

PSZ convert_nl_to_nul(PSZ pszSrc)
{
  // Convert newline to nul, return pointer to next or NULL
  PSZ psz = strchr(pszSrc, '\n');

  if (psz) {
    *psz = 0;
    psz++;
  }
  return psz;
}

void strip_trail_char(char *pszStripChars, char *pszSrc)
{
  char *psz;

  if (pszSrc && *pszSrc && pszStripChars && *pszStripChars) {
    psz = pszSrc + strlen(pszSrc) - 1;
    // while not empty and tail char in strip list
    while (*pszSrc && strchr(pszStripChars, *psz) != NULL) {
      *psz = 0;
      psz--;
    }
  }
}

void strip_lead_char(char *pszStripChars, char *pszSrc)
{
  char *psz = pszSrc;

  if (pszSrc && *pszSrc && pszStripChars && *pszStripChars) {
    // while lead char in strip list
    while (*psz && strchr(pszStripChars, *psz) != NULL)
      psz++;
    if (psz != pszSrc)
      memmove(pszSrc, psz, strlen(psz) + 1);
  }
}

VOID remove_first_occurence_of_character(char *pszRemoveChar, char *pszSrc)
{
  PSZ pszStrLocation;

  pszStrLocation = strchr(pszSrc, *pszRemoveChar);
  if (pszStrLocation)
    memmove(pszStrLocation, pszStrLocation + 1, strlen(pszStrLocation) + 1);
}

VOID remove_last_occurence_of_character(char *pszRemoveChar, char *pszSrc)
{
  PSZ pszStrLocation;

  pszStrLocation = strrchr(pszSrc, *pszRemoveChar);
  if (pszStrLocation)
    memmove(pszStrLocation, pszStrLocation + 1, strlen(pszStrLocation) + 1);
}

#pragma alloc_text(MISC8,chop_at_crnl,convert_nl_to_nul,strip_trail_char,strip_lead_char)
