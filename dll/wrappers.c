
/***********************************************************************

  $Id$

  Wrappers with error checking

  Copyright (c) 2006, 2015 Steven H.Levine

  22 Jul 06 SHL Baseline
  29 Jul 06 SHL Add xgets_stripped
  18 Aug 06 SHL Correct Runtime_Error line number report
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  01 Sep 07 GKY Add xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundry
  06 Oct 07 SHL Add xDos...() wrappers to support systems wo/large file support (Gregg, Steven)
  05 May 08 SHL Add FORTIFY support
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  17 Jun 09 SHL Correct missing rc set
  12 Jul 09 GKY Add xDosQueryAppType and xDosAlloc... to allow FM/2 to load in high memory
  15 Nov 09 GKY Rework xDosQueryAppType to remove HIMEM ifdefs
  26 Aug 11 GKY Add a low mem version of xDosAlloc* wrappers; move error checking into all the
		xDosAlloc* wrappers.
  09 Oct 11 GKY Modify xfsopen so it doesn't fail when called with r+ because the file doesn't exist.
		We should be creating the file unless it is set to fail silently.
  12 Aug 15 JBS Ticket #522: Ensure no "highmem-unsafe" functions are called directly
		1) New functions have been added
		2) Code for unsafe-but-not-yet-used-by-FM/2 functions have been added in an
		   "#if 0" block for quick implementation should FM/2 start to use them.
		   Among these. xDosOpenL and xWinUpper still need work. The rest are ready for use.
  20 Aug 15 SHL Add xDos...MutexSem and xDos..EventSem wrappers

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_LONGLONG
#include <os2.h>

#include "fm3dll.h"
#include "init.h"			// Data declaration(s)
#include "wrappers.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "command.h"
#include "tools.h"
#include "avl.h"
#include "strips.h"			// bstrip

#include "fortify.h"			// GetPString
#include "info.h"                       // driveflags
#include "notebook.h"                   // fVerify
#include "pathutil.h"                   // MaxComLineStrg

// Data definitions
static PSZ pszSrcFile = __FILE__;

#pragma data_seg(GLOBAL1)
BOOL fNoLargeFileSupport;

APIRET xDosDupHandle(HFILE hFile,
		     PHFILE phFile)
{
  APIRET	rc;
  HFILE	hFileLow = *phFile;

  rc = DosDupHandle(hFile, &hFileLow);
  *phFile = hFileLow;
  return rc;
}

APIRET xDosForceDelete(PSZ pszFileName)
{
  APIRET	rc;
  CHAR szFileNameLow[CCHMAXPATH];

  strcpy(szFileNameLow, pszFileName);
  rc = DosForceDelete(szFileNameLow);
  return rc;
}

APIRET xDosQueryAppType(PCSZ pszName,
			PULONG pFlags)
{
  APIRET rc;
  ULONG ulFlagsLow;
  CHAR szPgm[CCHMAXPATH];

  strcpy(szPgm, pszName);
  rc = DosQueryAppType(szPgm, &ulFlagsLow);
  *pFlags = ulFlagsLow;
  return rc;
}

APIRET xDosQueryHType(HFILE hFile,
		      PULONG pulType,
		      PULONG pulAttr)
{
  APIRET	rc;
  ULONG		ulTypeLow, ulAttrLow;

  rc = DosQueryHType(hFile, &ulTypeLow, &ulAttrLow);
  *pulType = ulTypeLow;
  *pulAttr = ulAttrLow;
  return rc;
}

APIRET xDosStartSession(PSTARTDATA psd,
			PULONG pulSessionID,
			PPID ppid)
{
  APIRET	rc;
  ULONG		ulSessionIDLow;
  PID		pidLow;
  PSTARTDATA	psdLow;
  CHAR		*pbSafe;
  size_t	cbSafe = sizeof(STARTDATA);
  UINT		ul0, ul1, ul2, ul3, ul4, ul5;

  if (HIGH_MEMORY_ADDRESS(psd->PgmTitle))
  {
    ul0 = strlen((const char *)psd->PgmTitle) + 1;
    cbSafe += ul0;
  }
  else
      ul0 = 0;
  if (HIGH_MEMORY_ADDRESS(psd->PgmName))
  {
    ul1 = strlen((const char *)psd->PgmName) + 1;
    cbSafe += ul1;
  }
  else
      ul1 = 0;
  if (HIGH_MEMORY_ADDRESS(psd->TermQ))
  {
    ul2 = strlen((const char *)psd->TermQ) + 1;
    cbSafe += ul2;
  }
  else
    ul2 = 0;
  if (HIGH_MEMORY_ADDRESS(psd->PgmInputs))
  {
    ul3 = strlen((const char *)psd->PgmInputs) + 1;
    cbSafe += ul3;
  }
  else
    ul3= 0;
  if (HIGH_MEMORY_ADDRESS(psd->Environment))
  {
    const char *psz = (const char *)psd->Environment;
    while (*psz)
	psz = strchr(psz, '\0') + 1;
    ul4 = (unsigned int *)psz - (unsigned int *)psd->Environment + 1;
    cbSafe += ul4;
  }
  else
    ul4 = 0;
  if (HIGH_MEMORY_ADDRESS(psd->IconFile))
  {
    ul5 = strlen((const char *)psd->IconFile) + 1;
    cbSafe += ul5;
  }
  else
    ul5 = 0;
  if (HIGH_MEMORY_ADDRESS(psd->ObjectBuffer) && psd->ObjectBuffLen)
    cbSafe += psd->ObjectBuffLen;

  rc = xDosAllocMemLow((void *)&psdLow,
		       cbSafe,
		       pszSrcFile,
		       __LINE__);
  if (rc)
    return ERROR_NOT_ENOUGH_MEMORY;

  memcpy((PVOID)psdLow, (PVOID)psd, sizeof(STARTDATA));
  pbSafe = (CHAR *)psdLow + sizeof(STARTDATA);
  if (ul0)
  {
    memcpy(pbSafe, psd->PgmTitle, ul0);
    psdLow->PgmTitle = pbSafe;
    pbSafe += ul0;
  }
  if (ul1)
  {
    memcpy(pbSafe, psd->PgmName, ul1);
    psdLow->PgmName = pbSafe;
    pbSafe += ul1;
  }
  if (ul2)
  {
    memcpy(pbSafe, psd->TermQ, ul2);
    psdLow->TermQ = (UCHAR *)pbSafe;
    pbSafe += ul2;
  }
  if (ul3)
  {
    memcpy(pbSafe, psd->PgmInputs, ul3);
    psdLow->PgmInputs = (UCHAR *)pbSafe;
    pbSafe += ul3;
  }
  if (ul4)
  {
    memcpy(pbSafe, psd->Environment, ul4);
    psdLow->Environment = (UCHAR *)pbSafe;
    pbSafe += ul4;
  }
  if (ul5)
  {
    memcpy(pbSafe, psd->IconFile, ul5);
    psdLow->IconFile = pbSafe;
    pbSafe += ul5;
  }
  if (HIGH_MEMORY_ADDRESS(psd->ObjectBuffer) && psd->ObjectBuffLen)
    psdLow->ObjectBuffer = pbSafe;

  rc = DosStartSession(psdLow,
		       pulSessionID ? &ulSessionIDLow : NULL,
		       ppid ? &pidLow : NULL);

  /* Set return values */
  if (HIGH_MEMORY_ADDRESS(psd->ObjectBuffer) && psd->ObjectBuffLen)
    memcpy(psd->ObjectBuffer, psdLow->ObjectBuffer, psd->ObjectBuffLen);
  if (pulSessionID)
    *pulSessionID = ulSessionIDLow;
  if (ppid)
    *ppid = pidLow;

  /* cleanup and return. */
  xfree(psdLow, pszSrcFile, __LINE__);
  return rc;
}


//
//	"Other" wrapper functions
//
/**
 * xDosAllocSharedMem uses OBJ_ANY on systems that support high memory use
 * and falls back to low memory allocation where it is not supported.
 * Flags are hard coded PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE
 * The wrapper provides error checking.
 */

APIRET xDosAllocSharedMem(PPVOID ppb,
			  PSZ pszName,
			  ULONG cb,
			  PCSZ pszSrcFile,
			  UINT uiLineNumber)
{
  APIRET rc;

  rc = DosAllocSharedMem(ppb, pszName, cb,
			 PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE | OBJ_ANY);
  //DbgMsg(pszSrcFile, __LINE__, "ppb %p", *ppb);
  if (rc)
    rc = DosAllocSharedMem(ppb, pszName, cb, PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE);
  if (rc)
    Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));
  return rc;
}

/**
 * xDosAllocMem uses OBJ_ANY on systems that support high memory use
 * and falls back to low memory allocation where it is not supported.
 * Flags are hard coded PAG_COMMIT | PAG_READ | PAG_WRITE.
 * The wrapper provides error checking.
 */

APIRET xDosAllocMem(PPVOID ppb,
		    ULONG cb,
		    PCSZ pszSrcFile,
		    UINT uiLineNumber)
{
  APIRET rc;

  rc = DosAllocMem(ppb, cb, PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_ANY);
  //DbgMsg(pszSrcFile, uiLineNumber, "ppb %p %x", *ppb, rc);
  if (rc)
    rc = DosAllocMem(ppb, cb, PAG_COMMIT | PAG_READ | PAG_WRITE);
  if (rc)
    Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));
  //DbgMsg(pszSrcFile, uiLineNumber, "ppb %p", *ppb);
  return rc;
}

/**
 * xDosAllocMemLow doesn't use OBJ_ANY. It should be used when the buffer
 * is going to be used by 16 functions that fail to thunk high memory addresses properly
 * such as DosQueryAppType, DosOpenL, DosGetMessage  and DosReadQueue (probably others)
 * Flags are hard coded PAG_COMMIT | PAG_READ | PAG_WRITE.
 * The wrapper provides error checking.
 */

APIRET xDosAllocMemLow(PPVOID ppb,
		       ULONG cb,
		       PCSZ pszSrcFile,
		       UINT uiLineNumber)
{
  APIRET rc;

  rc = DosAllocMem(ppb, cb, PAG_COMMIT | PAG_READ | PAG_WRITE);
  if (rc)
    Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));
  //DbgMsg(pszSrcFile, uiLineNumber, "ppb %p", *ppb);
  return rc;
}

APIRET xDosGetInfoBlocks(PTIB *pptib,
			 PPIB *pppib)
{
  APIRET apiret = DosGetInfoBlocks(pptib, pppib);

  if (apiret) {
    Dos_Error(MB_CANCEL, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSGETINFOBLOCKS);
    *pppib = 0;
    *pptib = 0;
  }
  return apiret;
}

/**
 * DosRequestMutexSem wrapper
 */

APIRET xDosRequestMutexSem(HMTX hmtx, ULONG ulTimeout)
{
  APIRET apiret = DosRequestMutexSem(hmtx, ulTimeout);

  if (apiret && (ulTimeout == SEM_INDEFINITE_WAIT || apiret != ERROR_TIMEOUT)) {
    Dos_Error(MB_CANCEL, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSREQUESTMUTEXSEM);
  }
  return apiret;
}

/**
 * DosReleaseMutexSem wrapper
 */

APIRET xDosReleaseMutexSem(HMTX hmtx)
{
  APIRET apiret = DosReleaseMutexSem(hmtx);

  if (apiret) {
    Dos_Error(MB_CANCEL, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSRELEASEMUTEXSEM);
  }
  return apiret;
}

/**
 * DosCreateEventSem wrapper
 */

APIRET xDosCreateEventSem (PSZ pszName,PHEV phev, ULONG flAttr, BOOL32 fState)
{
  APIRET apiret = DosCreateEventSem (pszName,phev, flAttr, fState);
  if (apiret) {
    Dos_Error(MB_CANCEL, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSCREATEEVENTSEM);
  }
  return apiret;
}

/**
 * DosWaitEventSem wrapper
 */

APIRET xDosWaitEventSem(HEV hev, ULONG ulTimeout)
{
  APIRET apiret = DosWaitEventSem(hev, ulTimeout);

  if (apiret && (ulTimeout == SEM_INDEFINITE_WAIT || apiret != ERROR_TIMEOUT)) {
    Dos_Error(MB_CANCEL, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSWAITEVENTSEM);
  }
  return apiret;
}

/**
 * DosPostEventSem wrapper
 */

APIRET xDosPostEventSem(HEV hev)
{
  APIRET apiret = DosPostEventSem(hev);

  if (apiret && apiret != ERROR_ALREADY_POSTED)	{
    Dos_Error(MB_CANCEL, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSPOSTEVENTSEM);
  }
  return apiret;
}

/**
 * DosResetEventSem wrapper
 */

APIRET xDosResetEventSem(HEV hev, PULONG pulPostCt)
{
  APIRET apiret = DosResetEventSem(hev, pulPostCt);

  if (apiret && apiret != ERROR_ALREADY_RESET) {
    Dos_Error(MB_CANCEL, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSRESETEVENTSEM);
  }
  return apiret;
}

/**
 * DosFindFirst wrapper
 */

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
    rc = DosQueryPathInfo (pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf);

  return rc;
}

/**
 * Wrap DosSetPathInfo to avoid spurious ERROR_INVALID_NAME returns and
 * support systems without large file support
 *
 * Some kernels do not correctly handle FILESTATUS3 and PEAOP2 buffers
 * that cross a 64K boundary.
 * When this occurs, they return ERROR_INVALID_NAME.
 *
 * This code works around the problem because if the passed buffer crosses
 * the boundary the alternate buffer will not because both are on the stack
 * and we don't put enough additional data on the stack for this to occur.
 *
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
  BOOL crosses = ((ULONG)pInfoBuf ^
		  ((ULONG)pInfoBuf + cbInfoBuf - 1)) & ~0xffff;
  BOOL fResetVerify = FALSE;

  if (fVerify && driveflags[toupper(*pszPathName) - 'A'] & DRIVE_WRITEVERIFYOFF) {
    DosSetVerify(FALSE);
    fResetVerify = TRUE;
  }
  switch (ulInfoLevel) {
    case FIL_STANDARD:
      if (crosses) {
	fs3 = *(PFILESTATUS3)pInfoBuf;	// Copy to buffer that does not cross 64K boundary
	rc = DosSetPathInfo(pszPathName, ulInfoLevel, &fs3, cbInfoBuf, flOptions);
      }
      else
	     rc = DosSetPathInfo(pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf, flOptions);
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
  if (fResetVerify) {
    DosSetVerify(fVerify);
    fResetVerify = FALSE;
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

/**
 * Wrapper for fopen it works around DosOpenL's failure to
 * thunk properly so that fm2 can be loaded in high memory
 * It also gives the option of reporting file open errors
 * If fSilent is TRUE it fails silently; if FALSE it produces a
 * runtime error dialog. Note pszMode must be passed on the stack
 * to xfopen to avoid the thunking problem.
 */

FILE *xfopen(PCSZ pszFileName, PCSZ pszMode, PCSZ pszSrcFile,
	     UINT uiLineNumber, BOOL fSilent)
{
  CHAR FileName[CCHMAXPATH];
  FILE *fp;

  strcpy(FileName, pszFileName);
  fp = fopen(FileName, pszMode);

  if (!fp && !fSilent)
    Runtime_Error(pszSrcFile, uiLineNumber, "fopen");
  return fp;
}

/**
 * Wrapper for _fsopen it works around DosOpenL's failure to
 * thunk properly so that fm2 can be loaded in high memory
 * It also gives the option of reporting file open errors
 * If fSilent is TRUE it fails silently; if FALSE it produces a
 * runtime error dialog. Note pszMode must be passed on the stack
 * to xfopen to avoid the thunking problem
 */

FILE *xfsopen(PCSZ pszFileName, PCSZ pszMode, INT fSharemode, PCSZ pszSrcFile,
	      UINT uiLineNumber, BOOL fSilent)
{
  CHAR FileName[CCHMAXPATH];
  FILE *fp;

  strcpy(FileName, pszFileName);
  fp = _fsopen(FileName, pszMode, fSharemode);
  if (!fp && !strcmp(pszMode,  "r+") && !fSilent)
    fp = _fsopen(FileName, "w+", fSharemode);

  if (!fp && !fSilent)
    Runtime_Error(pszSrcFile, uiLineNumber, "_fsopen");
  return fp;
}

//== xfree - safe free ==

VOID xfree(PVOID pv, PCSZ pszSrcFile, UINT uiLineNumber)
{
  if (pv && pv != NullStr) {
#   ifdef FORTIFY
    Fortify_free(pv, pszSrcFile, uiLineNumber);
#   else
    free(pv);
#    endif

  }
}

//== xmalloc() malloc with error checking ==

PVOID xmalloc(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber)
{
# ifdef FORTIFY
  PVOID pv = Fortify_malloc(cBytes, pszSrcFile, uiLineNumber);
# else
  PVOID pv = malloc(cBytes);
#  endif

  if (!pv)
    Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));

  return pv;
}

//== xmallocz() malloc and zero with error checking ==

PVOID xmallocz(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber)
{
  PVOID pv = xmalloc(cBytes, pszSrcFile, uiLineNumber);

  if (pv)
    memset(pv, 0, cBytes);

  return pv;
}

//== xrealloc() realloc with error checking ==

PVOID xrealloc(PVOID pvIn, size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber)
{
  if (pvIn != NullStr) {
#   ifdef FORTIFY
    PVOID pv = Fortify_realloc(pvIn, cBytes, pszSrcFile, uiLineNumber);
#   else
    PVOID pv = realloc(pvIn, cBytes);
#    endif

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
# ifdef FORTIFY
  PSZ psz = Fortify_strdup(pszIn, pszSrcFile, uiLineNumber);
# else
  PSZ psz = strdup(pszIn);
#  endif

  if (!psz)
    Runtime_Error(pszSrcFile, uiLineNumber, GetPString(IDS_OUTOFMEMORY));

  return psz;
}

#if 0
/*
 * JBS: Wrappers for functions which...
 *	- are identified by klibc as "highmem-unsafe"
 *	- not yet used by FM/2
 */

// .H code for these functions
// The following are ready to go.
APIRET xDosCreatePipe(PHFILE phfReadHandle,
		      PHFILE phfWriteHandle,
		      ULONG ulPipeSize);

APIRET xDosQueryFHState(HFILE hFile,
			PULONG pulMode);

#ifdef INCL_DOSNMPIPES
APIRET xDosQueryNPHState(HPIPE hpipe,
			 PULONG pulState);
#endif

APIRET xDosSetDateTime(DATETIME *pdt);

APIRET xDosSetFilePtr(HFILE hFile,
		      LONG lOffset,
		      ULONG ulOrigin,
		      PULONG pulPos);

APIRET xDosSetFilePtrL(HFILE hFile,
		       LONGLONG llOffset,
		       ULONG ulOrigin,
		       PLONGLONG pllPos);

#ifdef INCL_DOSPROCESS
APIRET xDosWaitChild(ULONG ulAction,
		     ULONG ulWait,
		     PRESULTCODES pReturnCodes,
		     PPID ppidOut,
		     PID pidIn);
#endif

APIRET xDosWaitNPipe(PCSZ pszName,
		     ULONG ulTimeout);

// The following functions still need work
ULONG xWinUpper(HAB hab,
		ULONG idcp,
		ULONG idcc,
		PSZ psz);

APIRET xDosOpenL(PCSZ pszFileName,
		 PHFILE phFile,
		 PULONG pulAction,
		 LONGLONG llFileSize,
		 ULONG ulAttribute,
		 ULONG ulOpenFlags,
		 ULONG ulOpenMode,
		 PEAOP2 pEABuf);



// .C code for the functions above
APIRET xDosCreatePipe(PHFILE phfReadHandle,
		      PHFILE phfWriteHandle,
		      ULONG ulPipeSize)
{
  APIRET   rc;
  HFILE   h1, h2;
  PHFILE  ph1 = NULL;
  PHFILE  ph2 = NULL;

  if (phfReadHandle)
  {
      h1 = *phfReadHandle;
      ph1 = &h1;
  }
  if (phfWriteHandle)
  {
      h2 = *phfWriteHandle;
      ph2 = &h2;
  }

  rc = DosCreatePipe(ph1, ph2, ulPipeSize);

  if (phfReadHandle)
      *phfReadHandle = h1;
  if (phfWriteHandle)
      *phfWriteHandle = h2;
  return rc;
}

// Code pEABuf
APIRET xDosOpenL(PCSZ pszFileName,
		 PHFILE phFile,
		 PULONG pulAction,
		 LONGLONG llFileSize,
		 ULONG ulAttribute,
		 ULONG ulOpenFlags,
		 ULONG ulOpenMode,
		 PEAOP2 pEABuf)
{
  APIRET  rc;
  ULONG   ul1;
  PULONG  pul1 = NULL;
  HFILE   hf1;
  PHFILE  phf1 = NULL;
  char    szFileNameLow[CCHMAXPATH];

  strcpy(szFileNameLow, pszFileName);

  if (phFile)
  {
      hf1 = *phFile;
      phf1 = &hf1;
  }
  if (pulAction)
  {
      ul1 = *pulAction;
      pul1 = &ul1;
  }

/** @todo pEABuf */

  rc = DosOpenL(szFileNameLow, phf1, pul1, llFileSize, ulAttribute,
		  ulOpenFlags, ulOpenMode, pEABuf);

  if (phFile)
      *phFile = hf1;
  if (pulAction)
      *pulAction = ul1;

  return rc;
}

APIRET xDosQueryNPHState(HPIPE hpipe,
			 PULONG pulState)
{
  APIRET  rc;
  ULONG   ul1;
  PULONG  pul1 = NULL;

  if (pulState)
  {
      ul1 = *pulState;
      pul1 = &ul1;
  }

  rc = DosQueryNPHState(hpipe, pul1);

  if (pulState)
      *pulState = ul1;
  return rc;
}

APIRET xDosQueryFHState(HFILE hFile,
			PULONG pulMode)
{
  APIRET  rc;
  ULONG   ul1;
  PULONG  pul1 = NULL;

  if (pulMode)
  {
      ul1 = *pulMode;
      pul1 = &ul1;
  }

  rc = DosQueryFHState(hFile, pul1);

  if (pulMode)
      *pulMode = ul1;

  return rc;
}

APIRET xDosSetDateTime(DATETIME *pdt)
{
  APIRET      rc;
  DATETIME    dt1;
  PDATETIME   pdt1 = NULL;

  if (pdt)
  {
      dt1 = *pdt;
      pdt1 = &dt1;
  }

  rc = DosSetDateTime(pdt1);

  return rc;
}

APIRET xDosSetFilePtr(HFILE hFile,
		      LONG lOffset,
		      ULONG ulOrigin,
		      PULONG pulPos)
{
  APIRET rc;
  ULONG  ul1;
  PULONG pul1 = NULL;

  if (pulPos)
  {
      ul1 = *pulPos;
      pul1 = &ul1;
  }

  rc = DosSetFilePtr(hFile, lOffset, ulOrigin, pul1);

  if (pulPos)
      *pulPos = ul1;

  return rc;
}

APIRET xDosSetFilePtrL(HFILE hFile,
		       LONGLONG llOffset,
		       ULONG ulOrigin,
		       PLONGLONG pllPos)
{
  APIRET rc;
  LONGLONG ll1;
  PLONGLONG pll1 = NULL;

  if (pllPos)
  {
      ll1 = *pllPos;
      pll1 = &ll1;
  }

  rc = DosSetFilePtrL(hFile, llOffset, ulOrigin, pll1);

  if (pllPos)
      *pllPos = ll1;

  return rc;
}

APIRET xDosWaitChild(ULONG ulAction,
		     ULONG ulWait,
		     PRESULTCODES pReturnCodes,
		     PPID ppidOut,
		     PID pidIn)
{
  APIRET          rc;
  RESULTCODES     res;
  PRESULTCODES    pres = NULL;
  PID             pid;
  PPID            ppid = NULL;

  if (pReturnCodes)
  {
      res = *pReturnCodes;
      pres = &res;
  }
  if (ppidOut)
  {
      pid = *ppidOut;
      ppid = &pid;
  }

  rc = DosWaitChild(ulAction, ulWait, pres, ppid, pidIn);

  if (pReturnCodes)
      *pReturnCodes = res;
  if (ppidOut)
      *ppidOut = pid;

  return rc;
}

APIRET xDosWaitNPipe(PCSZ pszName,
		     ULONG ulTimeout)
{
  APIRET rc;
  char szNameLow[CCHMAXPATH];

  strcpy(szNameLow, pszName);
  rc = DosWaitNPipe(szNameLow, ulTimeout);
  return rc;
}

ULONG xWinUpper(HAB hab,
		ULONG idcp,
		ULONG idcc,
		PSZ psz)
{
  ULONG rc;

  if (!HIGH_MEMORY_ADDRESS(psz))
    rc = WinUpper(hab, idcp, idcc, psz);
  else {
    size_t cch = strlen(psz);
    char *pszTmp = xmalloc(cch + 3, pszSrcFile, __LINE__);
    if (pszTmp)
      {
	  memcpy(pszTmp, psz, cch + 1);
	  pszTmp[cch + 1] = '\0';
	  pszTmp[cch + 2] = '\0';
	  rc = WinUpper(hab, idcp, idcc, pszTmp);
	  if (rc > 0)
	    memcpy(psz, pszTmp, rc <= cch ? rc + 1 : rc);
	  xfree(pszTmp, pszSrcFile, __LINE__);
      }
      else
      {
	  PSZ pszStart = psz;
	  PSZ pszNext;
	  while (*psz)
	  {
	      pszNext = (PSZ)WinNextChar(hab, idcp, idcc, psz);
	      if (pszNext - psz == 1)
		  *psz = WinUpperChar(hab, idcp, idcc, *psz);
	      else if (pszNext - psz == 2)
		  *(PUSHORT)psz = WinUpperChar(hab, idcp, idcc, *(PUSHORT)psz); /* a wild guess. */
	      else
		  break;
	      psz = (char *)pszNext;
	  }
	  rc = psz - pszStart;
      }
  }
  return rc;
}

#endif


#pragma alloc_text(WRAPPERS1,xfree,xfopen,xfsopen,xmalloc,xrealloc,xstrdup)
#pragma alloc_text(WRAPPERS2,xDosSetPathInfo,xDosFindFirst,xDosFindNext)
