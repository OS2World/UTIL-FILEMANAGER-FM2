
/***********************************************************************

  $Id$

  Wrappers with error checking

  Copyright (c) 2006 Steven H.Levine

  22 Jul 06 SHL Baseline
  29 Jul 06 SHL Add xgets_stripped
  18 Aug 06 SHL Correct Runtime_Error line number report
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  01 Sep 07 GKY Add xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundry
  06 Oct 07 SHL Add xDos...() wrappers to support systems wo/large file support (Gregg, Steven)

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_LONGLONG
#include <os2.h>

#include "fm3dll.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString

static PSZ pszSrcFile = __FILE__;

APIRET xDosFindFirst(PSZ pszFileSpec,
		     PHDIR phdir,
		     ULONG flAttribute,
		     PVOID pfindbuf,
		     ULONG cbBuf,
		     PULONG pcFileNames,
		     ULONG ulInfoLevel)
{
  APIRET rc;
  if (fNoLargeFileSupport) {
    switch (ulInfoLevel) {
    case FIL_STANDARDL:
      {
	FILEFINDBUF3 ffb3;
	ulInfoLevel = FIL_STANDARD;
	*pcFileNames = 1;		// fixme to support larger counts
	rc = DosFindFirst(pszFileSpec, phdir, flAttribute, &ffb3, sizeof(ffb3),
			  pcFileNames, ulInfoLevel);
	if (!rc) {
	  *(PFILEFINDBUF3)pfindbuf = ffb3;	// Copy aligned data
	  ((PFILEFINDBUF3L)pfindbuf)->cbFile = ffb3.cbFile;	// Copy unaligned data
	  ((PFILEFINDBUF3L)pfindbuf)->cbFileAlloc = ffb3.cbFileAlloc;
	  ((PFILEFINDBUF3L)pfindbuf)->attrFile = ffb3.attrFile;
	  ((PFILEFINDBUF3L)pfindbuf)->cchName = ffb3.cchName;
	  memcpy(((PFILEFINDBUF3L)pfindbuf)->achName, ffb3.achName, ffb3.cchName + 1);
	}
      }
      break;
    case FIL_QUERYEASIZEL:
      {
	FILEFINDBUF4 ffb4;
	*pcFileNames = 1;		// fixme to support larger counts
	ulInfoLevel = FIL_QUERYEASIZE;
	rc = DosFindFirst(pszFileSpec, phdir, flAttribute, &ffb4, sizeof(ffb4),
			  pcFileNames, ulInfoLevel);
	if (!rc) {
	  *(PFILEFINDBUF4)pfindbuf = ffb4;	// Copy aligned data
	  ((PFILEFINDBUF4L)pfindbuf)->cbFile = ffb4.cbFile;	// Copy unaligned data
	  ((PFILEFINDBUF4L)pfindbuf)->cbFileAlloc = ffb4.cbFileAlloc;
	  ((PFILEFINDBUF4L)pfindbuf)->attrFile = ffb4.attrFile;
	  ((PFILEFINDBUF4L)pfindbuf)->cbList = ffb4.cbList;
	  ((PFILEFINDBUF4L)pfindbuf)->cchName = ffb4.cchName;
	  memcpy(((PFILEFINDBUF4L)pfindbuf)->achName, ffb4.achName, ffb4.cchName + 1);
	}
      }
      break;
    default:
      Runtime_Error(pszSrcFile, __LINE__, "ulInfoLevel %u unexpected", ulInfoLevel);
      rc = ERROR_INVALID_PARAMETER;
    } // switch
  }
  else
    rc = DosFindFirst(pszFileSpec, phdir, flAttribute, pfindbuf, cbBuf,
		      pcFileNames, ulInfoLevel);
  return rc;
}

APIRET xDosFindNext(HDIR hDir,
		    PVOID pfindbuf,
		    ULONG cbfindbuf,
		    PULONG pcFileNames,
		    ULONG ulInfoLevel)
{
  APIRET rc;
  if (fNoLargeFileSupport) {
    switch (ulInfoLevel) {
    case FIL_STANDARDL:
      {
	FILEFINDBUF3 ffb3;
	*pcFileNames = 1;		// fixme to support larger counts
	rc = DosFindNext(hDir, &ffb3, sizeof(ffb3), pcFileNames);
	if (!rc) {
	  *(PFILEFINDBUF3)pfindbuf = ffb3;	// Copy aligned data
	  ((PFILEFINDBUF3L)pfindbuf)->cbFile = ffb3.cbFile;	// Copy unaligned data
	  ((PFILEFINDBUF3L)pfindbuf)->cbFileAlloc = ffb3.cbFileAlloc;
	  ((PFILEFINDBUF3L)pfindbuf)->attrFile = ffb3.attrFile;
	  ((PFILEFINDBUF3L)pfindbuf)->cchName = ffb3.cchName;
	  memcpy(((PFILEFINDBUF3L)pfindbuf)->achName, ffb3.achName, ffb3.cchName + 1);
	}
      }
      break;
    case FIL_QUERYEASIZEL:
      {
	FILEFINDBUF4 ffb4;
	*pcFileNames = 1;		// fixme to support larger counts
	rc = DosFindNext(hDir, &ffb4, sizeof(ffb4), pcFileNames);
	if (!rc) {
	  *(PFILEFINDBUF4)pfindbuf = ffb4;	// Copy aligned data
	  ((PFILEFINDBUF4L)pfindbuf)->cbFile = ffb4.cbFile;	// Copy unaligned data
	  ((PFILEFINDBUF4L)pfindbuf)->cbFileAlloc = ffb4.cbFileAlloc;
	  ((PFILEFINDBUF4L)pfindbuf)->attrFile = ffb4.attrFile;
	  ((PFILEFINDBUF4L)pfindbuf)->cbList = ffb4.cbList;
	  ((PFILEFINDBUF4L)pfindbuf)->cchName = ffb4.cchName;
	  memcpy(((PFILEFINDBUF4L)pfindbuf)->achName, ffb4.achName, ffb4.cchName + 1);
	}
      }
      break;
    default:
      Runtime_Error(pszSrcFile, __LINE__, "ulInfoLevel %u unexpected", ulInfoLevel);
      rc = ERROR_INVALID_PARAMETER;
    } // switch
  }
  else
    rc = DosFindNext(hDir, pfindbuf, cbfindbuf, pcFileNames);

  return rc;
}

/**
 * DosQueryPathInfo wrapper
 * Translate request for systems without large file support
 */

APIRET xDosQueryPathInfo (PSZ pszPathName, ULONG ulInfoLevel, PVOID pInfoBuf, ULONG cbInfoBuf)
{
  FILESTATUS3 fs3;
  FILESTATUS4 fs4;
  APIRET rc;

  if (fNoLargeFileSupport) {
    switch (ulInfoLevel) {
    case FIL_STANDARDL:
      rc = DosQueryPathInfo(pszPathName, ulInfoLevel, &fs3, sizeof(fs3));
      if (!rc) {
	*(PFILESTATUS3)pInfoBuf = fs3;	// Copy aligned data
	((PFILESTATUS3L)pInfoBuf)->cbFile = fs3.cbFile;	// Copy unaligned data
	((PFILESTATUS3L)pInfoBuf)->cbFileAlloc = fs3.cbFileAlloc;
	((PFILESTATUS3L)pInfoBuf)->attrFile = fs3.attrFile;
      }
      break;
    case FIL_QUERYEASIZEL:
      rc = DosQueryPathInfo(pszPathName, ulInfoLevel, &fs4, sizeof(fs4));
      if (!rc) {
	*(PFILESTATUS4)pInfoBuf = fs4;	// Copy aligned data
	((PFILESTATUS4L)pInfoBuf)->cbFile = fs4.cbFile;	// Copy unaligned data
	((PFILESTATUS4L)pInfoBuf)->cbFileAlloc = fs4.cbFileAlloc;
	((PFILESTATUS4L)pInfoBuf)->attrFile = fs4.attrFile;
	((PFILESTATUS4L)pInfoBuf)->cbList = fs4.cbList;
      }
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

/**
 * Wrap DosSetPathInfo to avoid spurious ERROR_INVALID_NAME returns and
 * support systems without large file support
 *
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
    FILESTATUS3 fs3;
    FILESTATUS3 fs3_a;
    FILESTATUS3L fs3l;
    EAOP2 eaop2;
    APIRET rc;

    switch (ulInfoLevel) {
      case FIL_STANDARD:
	fs3 = *(PFILESTATUS3)pInfoBuf;	// Copy to buffer that does not cross 64K boundary
	rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3, cbInfoBuf, flOptions);
	break;

      case FIL_STANDARDL:
	if (fNoLargeFileSupport) {
	  ulInfoLevel = FIL_STANDARD;
	  fs3 = *(PFILESTATUS3)pInfoBuf;	// Copy aligned data
	  // Check size too big to handle
	  if (((PFILESTATUS3L)pInfoBuf)->cbFile >= 1LL << 32 ||
	      ((PFILESTATUS3L)pInfoBuf)->cbFileAlloc >= 2LL << 32)
	  {
	    rc = ERROR_INVALID_PARAMETER;
	  }
	  else {
	    fs3.cbFile = ((PFILESTATUS3L)pInfoBuf)->cbFile;	// Copy unaligned data
	    fs3.cbFileAlloc = ((PFILESTATUS3L)pInfoBuf)->cbFileAlloc;
	    fs3.attrFile = ((PFILESTATUS3L)pInfoBuf)->attrFile;
	    rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3, sizeof(fs3), flOptions);
	  }
	  if (rc == ERROR_INVALID_NAME) {
	    // fixme to validate counts?
	    fs3_a = fs3;		// Copy to buffer that does not cross
	    rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3_a, sizeof(fs3_a), flOptions);
	  }
	}
	else {
	  rc = DosSetPathInfo(pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf, flOptions);
	  if (rc == ERROR_INVALID_NAME) {
	    fs3l = *(PFILESTATUS3L)pInfoBuf;	// Copy to buffer that does not cross
	    rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3l, sizeof(fs3l), flOptions);
	  }
	}
	break;
      case FIL_QUERYEASIZE:
	rc = DosSetPathInfo(pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf, flOptions);
	if (rc == ERROR_INVALID_NAME) {
	  // fixme to validate counts?
	  eaop2 = *(PEAOP2)pInfoBuf;	// Copy to buffer that does not cross
	  rc = DosSetPathInfo(pszPathName, ulInfoLevel, &eaop2, sizeof(eaop2), flOptions);
        }
        break;
      default:
	Runtime_Error(pszSrcFile, __LINE__, "ulInfoLevel %u unexpected", ulInfoLevel);
	rc = ERROR_INVALID_PARAMETER;
    } // switch

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
  if (pv && pv != NullStr)
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
  if (pvIn != NullStr) {
    PVOID pv = realloc(pvIn, cBytes);

    if (!pv && cBytes)
      Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));

    return pv;
  }
  else
    return xmalloc(cBytes, pszSrcFile, uiLineNumber);
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
