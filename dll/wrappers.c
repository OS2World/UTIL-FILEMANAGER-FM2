
/***********************************************************************

  $Id$

  Wrappers with error checking

  Copyright (c) 2006 Steven H.Levine

  22 Jul 06 SHL Baseline
  29 Jul 06 SHL Add xgets_stripped
  18 Aug 06 SHL Correct Runtime_Error line number report
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  01 Sep 07 GKY Add xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundry

***********************************************************************/

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fm3dll.h"
#include "fm3str.h"

APIRET APIENTRY  xDosSetPathInfo(PSZ   pszPathName,
                                 ULONG ulInfoLevel,
                                 PVOID pInfoBuf,
                                 ULONG cbInfoBuf,
                                 ULONG flOptions)
{
    APIRET rc;

    rc = DosSetPathInfo(pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf, flOptions);
    if (rc)
      /* Some kernels to do not handle fs3 buffers that cross 64K boundaries
        and return ERROR_INVALID_NAME
        If code works around the problem because if fs3 crosses the boundary
        fsa2 will not because we don't have enough data on the stack for this
        to occur 25 Aug 07 SHL
      */
        if (rc == ERROR_INVALID_NAME){
          if (ulInfoLevel == FIL_STANDARD){
            FILESTATUS3 fs3x;
            fs3x = *(PFILESTATUS3) pInfoBuf;
            rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3x, sizeof(fs3x), flOptions);
          }
          else {
            EAOP2 eaop2x;
            eaop2x = *(PEAOP2) pInfoBuf;
            rc = DosSetPathInfo(pszPathName, ulInfoLevel, &eaop2x, sizeof(eaop2x), flOptions);
          }
        }
    return rc;
}

PSZ xfgets(PSZ pszBuf, size_t cMaxBytes, FILE * fp, PCSZ pszSrcFile,
	   UINT uiLineNumber)
{
  PSZ psz = fgets(pszBuf, cMaxBytes, fp);

  if (!psz) {
    if (ferror(fp))
      Runtime_Error(pszSrcFile, uiLineNumber, "fgets");
  }
  else {
    size_t c = strlen(psz);

    if (c + 1 > cMaxBytes)
      Runtime_Error(pszSrcFile, uiLineNumber, "buffer overflow");
    else if (!c || (psz[c - 1] != '\n' && psz[c - 1] != '\r'))
      Runtime_Error(pszSrcFile, uiLineNumber, "missing EOL");
  }
  return psz;
}

PSZ xfgets_bstripcr(PSZ pszBuf, size_t cMaxBytes, FILE * fp, PCSZ pszSrcFile,
		    UINT uiLineNumber)
{
  PSZ psz = xfgets(pszBuf, cMaxBytes, fp, pszSrcFile, uiLineNumber);

  if (psz)
    bstripcr(psz);
  return psz;
}

FILE *xfopen(PCSZ pszFileName, PCSZ pszMode, PCSZ pszSrcFile,
	     UINT uiLineNumber)
{
  FILE *fp = fopen(pszFileName, pszMode);

  if (!fp)
    Runtime_Error(pszSrcFile, uiLineNumber, "fopen");
  return fp;
}

FILE *xfsopen(PCSZ pszFileName, PCSZ pszMode, INT fSharemode, PCSZ pszSrcFile,
	      UINT uiLineNumber)
{
  FILE *fp = _fsopen((PSZ) pszFileName, (PSZ) pszMode, fSharemode);

  if (!fp)
    Runtime_Error(pszSrcFile, uiLineNumber, "_fsopen");
  return fp;
}

//== xfree - safe free ==

VOID xfree(PVOID pv)
{
  if (pv)
    free(pv);
}

//== xmalloc() malloc with error checking ==

PVOID xmalloc(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PVOID pv = malloc(cBytes);

  if (!pv)
    Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));

  return pv;
}

//== xmallocz() malloc and zero with error checking ==

PVOID xmallocz(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PVOID pv = malloc(cBytes);

  if (!pv)
    Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));
  else
    memset(pv, 0, cBytes);

  return pv;
}

//== xrealloc() realloc with error checking ==

PVOID xrealloc(PVOID pvIn, size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PVOID pv = realloc(pvIn, cBytes);

  if (!pv && cBytes)
    Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));

  return pv;

}

//== xstrdup() strdup with error checking ==

PVOID xstrdup(PCSZ pszIn, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PSZ psz = strdup(pszIn);

  if (!psz)
    Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));

  return psz;
}

#pragma alloc_text(WRAPPERS1,xfree,xfopen,xfsopen,xmalloc,xrealloc, xstrdup)

