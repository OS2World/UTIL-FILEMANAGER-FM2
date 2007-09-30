
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
#define INCL_LONGLONG
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fm3dll.h"
#include "fm3str.h"

static PSZ pszSrcFile = __FILE__;

APIRET xDosFindFirst(PSZ    pszFileSpec,
                     PHDIR  phdir,
                     ULONG  flAttribute,
                     PVOID  pfindbuf,
                     ULONG  cbBuf,
                     PULONG pcFileNames,
                     ULONG  ulInfoLevel)
{
    APIRET rc;
    if (fNoLargeFileSupport) {
        switch (ulInfoLevel) {
    case FIL_STANDARDL: {
      FILEFINDBUF3 ff3;
      ff3 = *(PFILEFINDBUF3)pfindbuf;
      ulInfoLevel = FIL_STANDARD;
      rc = DosFindFirst(pszFileSpec, phdir, flAttribute, &ff3, cbBuf,
                        pcFileNames, ulInfoLevel);
      *(PFILEFINDBUF3)pfindbuf = ff3;   // Copy aligned data
      ((PFILEFINDBUF3L)pfindbuf) -> cbFile = ff3.cbFile;  // Copy unaligned data
      ((PFILEFINDBUF3L)pfindbuf) -> cbFileAlloc = ff3.cbFileAlloc;
      ((PFILEFINDBUF3L)pfindbuf) -> attrFile = ff3.attrFile;
      ((PFILEFINDBUF3L)pfindbuf) -> cchName = ff3.cchName;
      ((PFILEFINDBUF3L)pfindbuf) -> achName[CCHMAXPATHCOMP] = *ff3.achName;
      break;
    }
    case  FIL_QUERYEASIZEL:{
      FILEFINDBUF4 ff4;
      ff4 = *(PFILEFINDBUF4)pfindbuf;
      ulInfoLevel = FIL_QUERYEASIZE;
      rc = DosFindFirst(pszFileSpec, phdir, flAttribute, &ff4, cbBuf,
                        pcFileNames, ulInfoLevel);
      //(PFILEFINDBUF4L)pfindbuf = (PFILEFINDBUF4L)pff4;   // Copy aligned data
      ((PFILEFINDBUF4L)pfindbuf) -> oNextEntryOffset = ff4.oNextEntryOffset;
      ((PFILEFINDBUF4L)pfindbuf) -> fdateCreation = ff4.fdateCreation;
      ((PFILEFINDBUF4L)pfindbuf) -> ftimeCreation = ff4.ftimeCreation;
      ((PFILEFINDBUF4L)pfindbuf) -> fdateLastAccess = ff4.fdateLastAccess;
      ((PFILEFINDBUF4L)pfindbuf) -> ftimeLastAccess = ff4.ftimeLastAccess;
      ((PFILEFINDBUF4L)pfindbuf) -> fdateLastWrite = ff4.fdateLastWrite;
      ((PFILEFINDBUF4L)pfindbuf) -> ftimeLastWrite = ff4.ftimeLastWrite;
      ((PFILEFINDBUF4L)pfindbuf) -> cbFile = ff4.cbFile;  // Copy unaligned data
      ((PFILEFINDBUF4L)pfindbuf) -> cbFileAlloc = ff4.cbFileAlloc;
      ((PFILEFINDBUF4L)pfindbuf) -> attrFile = ff4.attrFile;
      ((PFILEFINDBUF4L)pfindbuf) -> cbList = ff4.cbList;
      ((PFILEFINDBUF4L)pfindbuf) -> cchName = ff4.cchName;
      ((PFILEFINDBUF4L)pfindbuf) -> achName[CCHMAXPATHCOMP] = *ff4.achName;
      break;
    }
    default:
	Runtime_Error(pszSrcFile, __LINE__, "ulInfoLevel %u unexpected", ulInfoLevel);
        rc = ERROR_INVALID_PARAMETER;
        }
    }
    else
    rc = DosFindFirst(pszFileSpec, phdir, flAttribute, pfindbuf, cbBuf,
                      pcFileNames, ulInfoLevel);
    return rc;
}

APIRET xDosFindNext(HDIR   hDir,
                    PVOID  pfindbuf,
                    ULONG  cbfindbuf,
                    PULONG pcFilenames)
{
  APIRET rc;
  if (fNoLargeFileSupport) {
    if (((PFILEFINDBUF4L)pfindbuf)->cbList) {
      FILEFINDBUF4 ff4;
      ff4 = *(PFILEFINDBUF4)pfindbuf;
      rc = DosFindNext(hDir, &ff4, cbfindbuf, pcFilenames);
      *(PFILEFINDBUF4)pfindbuf = ff4;   // Copy aligned data
      ((PFILEFINDBUF4L)pfindbuf) -> cbFile = ff4.cbFile;  // Copy unaligned data
      ((PFILEFINDBUF4L)pfindbuf) -> cbFileAlloc = ff4.cbFileAlloc;
      ((PFILEFINDBUF4L)pfindbuf) -> attrFile = ff4.attrFile;
      ((PFILEFINDBUF4L)pfindbuf) -> cbList = ff4.cbList;
      ((PFILEFINDBUF4L)pfindbuf) -> cchName = ff4.cchName;
      ((PFILEFINDBUF4L)pfindbuf) -> achName[CCHMAXPATHCOMP] = *ff4.achName;
    }
    else {
      FILEFINDBUF3 ff3;
      ff3 = *(PFILEFINDBUF3)pfindbuf;
      rc = DosFindNext(hDir, &ff3, cbfindbuf, pcFilenames);
      *(PFILEFINDBUF3)pfindbuf = ff3;   // Copy aligned data
      ((PFILEFINDBUF3L)pfindbuf) -> cbFile = ff3.cbFile;  // Copy unaligned data
      ((PFILEFINDBUF3L)pfindbuf) -> cbFileAlloc = ff3.cbFileAlloc;
      ((PFILEFINDBUF3L)pfindbuf) -> attrFile = ff3.attrFile;
      ((PFILEFINDBUF3L)pfindbuf) -> cchName = ff3.cchName;
      ((PFILEFINDBUF3L)pfindbuf) -> achName[CCHMAXPATHCOMP] = *ff3.achName;
    }
  }
  else
    rc = DosFindNext(hDir, pfindbuf, cbfindbuf, pcFilenames);

  return rc;
}

/**
 * Wrap DosSetPathInfo to avoid spurious ERROR_INVALID_NAME returns
 * Some kernels to do not correctly handle FILESTATUS3 and PEAOP2 buffers
 * that cross a 64K boundary.
 * When this occurs, they return ERROR_INVALID_NAME.
 * This code works around the problem because if the passed buffer crosses
 * the boundary the alternate buffer will not because both are on the stack
 * and we don't put enough additional data on the stack for this to occur.
 * It is caller's responsitibility to report errors
 * @param pInfoBuf pointer to FILESTATUS3(L) or EAOP2 buffer
 * @param ulInfoLevel FIL_STANDARD(L) or FIL_QUERYEASIZE
 * @returns Same as DosSetPathInfo
 */

APIRET xDosSetPathInfo(PSZ pszPathName,
        	       ULONG ulInfoLevel,
		       PVOID pInfoBuf,
		       ULONG cbInfoBuf,
                       ULONG flOptions)
{
    FILESTATUS3 alt_fs3;
    FILESTATUS3L alt_fs3L;
    EAOP2 alt_eaop2;
    BOOL large = FALSE;
    APIRET rc;
    if (fNoLargeFileSupport && ulInfoLevel == FIL_STANDARDL){
      large = TRUE;
      ulInfoLevel = FIL_STANDARD;
    }
    rc = DosSetPathInfo(pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf, flOptions);
    if (rc == ERROR_INVALID_NAME) {
      switch (ulInfoLevel) {
        case FIL_STANDARD:
	  alt_fs3 = *(PFILESTATUS3)pInfoBuf;	// Copy
          rc = DosSetPathInfo(pszPathName, ulInfoLevel, &alt_fs3, cbInfoBuf, flOptions);
          if (fNoLargeFileSupport && large) {
            *(PFILESTATUS3)pInfoBuf = alt_fs3;   // Copy aligned data
            ((PFILESTATUS3L)pInfoBuf) -> cbFile = alt_fs3.cbFile;  // Copy unaligned data
            ((PFILESTATUS3L)pInfoBuf) -> cbFileAlloc = alt_fs3.cbFileAlloc;
            ((PFILESTATUS3L)pInfoBuf) -> attrFile = alt_fs3.attrFile;
          }
          break;
        case FIL_STANDARDL:
          alt_fs3L = *(PFILESTATUS3L)pInfoBuf;	// Copy
          rc = DosSetPathInfo(pszPathName, ulInfoLevel, &alt_fs3L, cbInfoBuf, flOptions);
          break;
        case FIL_QUERYEASIZE:
          alt_eaop2 = *(PEAOP2)pInfoBuf;	// Copy
          rc = DosSetPathInfo(pszPathName, ulInfoLevel, &alt_eaop2, cbInfoBuf, flOptions);
          break;
      default:
	Runtime_Error(pszSrcFile, __LINE__, "ulInfoLevel %u unexpected", ulInfoLevel);
	rc = ERROR_INVALID_PARAMETER;
      } // switch
    }
    return rc;
}

APIRET xDosQueryPathInfo (PSZ pszPathName, ULONG ulInfoLevel, PVOID pInfoBuf, ULONG cbInfoBuf)
{
  FILESTATUS3 alt_fs3;
  FILESTATUS4 alt_fs4;
  APIRET rc;
  if (fNoLargeFileSupport){
  switch (ulInfoLevel) {
    case FIL_STANDARDL:
	  alt_fs3 = *(PFILESTATUS3)pInfoBuf;	// Copy
          rc = DosQueryPathInfo(pszPathName, ulInfoLevel, &alt_fs3, cbInfoBuf);
          *(PFILESTATUS3)pInfoBuf = alt_fs3;   // Copy aligned data
          ((PFILESTATUS3L)pInfoBuf) -> cbFile = alt_fs3.cbFile;  // Copy unaligned data
          ((PFILESTATUS3L)pInfoBuf) -> cbFileAlloc = alt_fs3.cbFileAlloc;
          ((PFILESTATUS3L)pInfoBuf) -> attrFile = alt_fs3.attrFile;
          break;
    case FIL_QUERYEASIZEL:
          alt_fs4 = *(PFILESTATUS4)pInfoBuf;	// Copy
          rc = DosQueryPathInfo(pszPathName, ulInfoLevel, &alt_fs4, cbInfoBuf);
          *(PFILESTATUS4)pInfoBuf = alt_fs4;   // Copy aligned data
          ((PFILESTATUS4L)pInfoBuf) -> cbFile = alt_fs4.cbFile;  // Copy unaligned data
          ((PFILESTATUS4L)pInfoBuf) -> cbFileAlloc = alt_fs4.cbFileAlloc;
          ((PFILESTATUS4L)pInfoBuf) -> attrFile = alt_fs4.attrFile;
          ((PFILESTATUS4L)pInfoBuf) -> cbList = alt_fs4.cbList;
          break;
      default:
	Runtime_Error(pszSrcFile, __LINE__, "ulInfoLevel %u unexpected", ulInfoLevel);
	rc = ERROR_INVALID_PARAMETER;
      } // switch
  }
  else
    DosQueryPathInfo (pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf);
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
#pragma alloc_text(WRAPPERS2,xDosSetPathInfo,xDosFindFirst,xDosFindNext)
