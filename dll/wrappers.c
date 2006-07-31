
/***********************************************************************

  $Id$

  Wrappers with error checking

  Copyright (c) 2006 Steven H.Levine

  22 Jul 06 SHL Baseline
  29 Jul 06 SHL Add xgets_stripped

***********************************************************************/

#define INCL_WIN
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fm3dll.h"
#include "fm3str.h"

#pragma alloc_text(WRAPPERS1,xfree,xfopen,xfsopen,xmalloc,xrealloc)

PSZ xfgets(PSZ pszBuf, size_t cMaxBytes, FILE *fp, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PSZ psz = fgets(pszBuf,cMaxBytes,fp);
  if (!psz) {
    if (ferror(fp))
      Runtime_Error(pszSrcFile, uiLineNumber, "fgets");
  }
  else {
    size_t c = strlen(psz);
    if (c + 1 > cMaxBytes)
      Runtime_Error(pszSrcFile, uiLineNumber, "buffer overflow");
    else if (!c || (psz[c-1] != '\n' && psz[c-1] != '\r'))
      Runtime_Error(pszSrcFile, uiLineNumber, "missing EOL");
  }
  return psz;
}

PSZ xfgets_bstripcr(PSZ pszBuf, size_t cMaxBytes, FILE *fp, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PSZ psz = xfgets(pszBuf,cMaxBytes,fp,pszSrcFile,__LINE__);
  if (psz)
    bstripcr(psz);
  return psz;
}

FILE *xfopen(PCSZ pszFileName, PCSZ pszMode, PCSZ pszSrcFile, UINT uiLineNumber)
{
  FILE* fp = fopen(pszFileName, pszMode);
  if (!fp)
    Runtime_Error(pszSrcFile, uiLineNumber, "fopen");
  return fp;
}

FILE *xfsopen(PCSZ pszFileName,PCSZ pszMode,INT fSharemode, PCSZ pszSrcFile, UINT uiLineNumber)
{
  FILE* fp = _fsopen((PSZ)pszFileName, (PSZ)pszMode, fSharemode);
  if (!fp)
    Runtime_Error(pszSrcFile, uiLineNumber, "_fsopen");
  return fp;
}


//== xfree - safe free ==

VOID xfree (PVOID pv)
{
  if (pv)
    free(pv);
}

//== xmalloc() malloc with error checking ==

PVOID xmalloc(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PVOID pv = malloc(cBytes);

  if (!pv)
    Runtime_Error(pszSrcFile,uiLineNumber,GetPString(IDS_OUTOFMEMORY));

  return pv;
}

//== xmallocz() malloc and zero with error checking ==

PVOID xmallocz(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PVOID pv = malloc(cBytes);

  if (!pv)
    Runtime_Error(pszSrcFile,uiLineNumber,GetPString(IDS_OUTOFMEMORY));
  else
    memset(pv, 0, cBytes);

  return pv;
}

//== xrealloc() realloc with error checking ==

PVOID xrealloc(PVOID pvIn, size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PVOID pv = realloc(pvIn, cBytes);

  if (!pv && cBytes)
    Runtime_Error(pszSrcFile,uiLineNumber,GetPString(IDS_OUTOFMEMORY));

  return pv;

}

//== xstrdup() strdup with error checking ==

PVOID xstrdup(PCSZ pszIn, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PSZ psz = strdup(pszIn);

  if (!psz)
    Runtime_Error(pszSrcFile, uiLineNumber,GetPString(IDS_OUTOFMEMORY));

  return psz;
}
