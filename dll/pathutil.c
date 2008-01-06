
/***********************************************************************

  $Id: $

  Path handling utility functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Move from arccnrs.c and comp.c to here

***********************************************************************/

#include <string.h>

#define INCL_WIN
#define INCL_LONGLONG

#include "pathutil.h"
#include "fm3dll.h"			// needs_quoting

// #pragma data_seg(DATA1)

/**
 * Build full path name in callers buffer given directory
 * name and filename
 * @param pszPathName points to drive/directory if not NULL
 * @returns pointer to full path name in caller's buffer
 * @note OK for pszFullPathName and pszPathName to point to same buffer
 *
 */

PSZ BldFullPathName(PSZ pszFullPathName, PSZ pszPathName, PSZ pszFileName)
{
  UINT c = pszPathName ? strlen(pszPathName) : 0;
  if (c > 0) {
    memcpy(pszFullPathName, pszPathName, c);
    if (pszFullPathName[c - 1] != '\\')
      pszFullPathName[c++] = '\\';
  }
  strcpy(pszFullPathName + c, pszFileName);
  return pszFullPathName;
}

/**
 * Build quoted full path name in callers buffer given
 * directory name and filename
 * @param pszPathName points to drive/directory if not NULL
 * @returns pointer to quoted path name in caller's buffer
 */

PSZ BldQuotedFullPathName(PSZ pszFullPathName, PSZ pszPathName, PSZ pszFileName)
{
  UINT c = pszPathName ? strlen(pszPathName) : 0;
  BOOL q = needs_quoting(pszPathName) ||
	   needs_quoting(pszFileName);
  PSZ psz = pszFullPathName;

  if (q)
    *psz++ = '"';
  if (c > 0) {
    memcpy(psz, pszPathName, c);
    psz += c;
    if (*(psz - 1) != '\\')
      *psz++ = '\\';
  }
  strcpy(psz, pszFileName);
  if (q) {
    psz += strlen(psz);
    *psz++ = '"';
    *psz = 0;
  }
  return pszFullPathName;
}

/**
 * Build quoted full path name in callers buffer given a filename
 * @returns pointer to quoted file name in caller's buffer
 */

PSZ BldQuotedFileName(PSZ pszQuotedFileName, PSZ pszFileName)
{
  BOOL q = needs_quoting(pszFileName);
  PSZ psz = pszQuotedFileName;

  if (q)
    *psz++ = '"';
  strcpy(psz, pszFileName);
  if (q) {
    psz += strlen(psz);
    *psz++ = '"';
    *psz = 0;
  }
  return pszQuotedFileName;
}

#pragma alloc_text(PATHUTIL,BldFullPathName)
#pragma alloc_text(PATHUTIL,BldQuotedFileName)
#pragma alloc_text(PATHUTIL,BldQuotedFullPathName)
