
/***********************************************************************

  $Id$

  Wrappers with error checking

  Copyright (c) 2006 Steven H.Levine

  22 Jul 06 SHL Baseline

***********************************************************************/

#define INCL_WIN
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fm3dll.h"
#include "fm3str.h"

#pragma alloc_text(WRAPPERS1,xfree,xfopen,xfsopen,xmalloc,xrealloc)

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

