
/***********************************************************************

  $Id$

  Wrappers with error checking

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  08 Dec 08 SHL Add missing OS2_INCLUDED check
  12 Jul 09 GKY Add xDosQueryAppType and xDoxAlloc... to allow FM/2 to load in high memory
  26 Aug 11 GKY Add a low mem version of xDosAlloc* wrappers; move error checking into all the
		xDosAlloc* wrappers.
  12 Aug 15 JBS Ticket #522: Ensure no "highmem-unsafe" functions are called directly
		1) New functions have been added
		2) Code for unsafe-but-not-yet-used-by-FM/2 functinos have been added in an
		   "#if 0" block in wrappers.c for quick implementation should FM/2 start to use them.
		   Among these. xDosOpenL and xWinUpper still need work. The rest are ready for use.

***********************************************************************/

#if !defined(WRAPPERS_H)
#define WRAPPERS_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

// Functions wrapped because they are not safe with high memory addresses

APIRET xDosDupHandle(HFILE hFile,
		     PHFILE phFile);

APIRET xDosForceDelete(PSZ pszFileName);

APIRET xDosQueryHType(HFILE hFile,
		      PULONG pulType,
		      PULONG pulAttr);

APIRET xDosQueryAppType(PCSZ pszName,
			PULONG pFlags);

#ifdef INCL_DOSSESMGR
APIRET xDosStartSession(PSTARTDATA psd,
			PULONG pulSessionID,
			PPID ppid);
#endif

// Functions wrapped for other reasons

APIRET xDosAllocMem(PPVOID ppb,
		    ULONG cb,
		    PCSZ pszSrcFile,
		    UINT uiLineNumber);

APIRET xDosAllocMemLow(PPVOID ppb,
		       ULONG cb,
		       PCSZ pszSrcFile,
		       UINT uiLineNumber);

APIRET xDosAllocSharedMem(PPVOID ppb,
			  PSZ pszName,
			  ULONG cb,
			  PCSZ pszSrcFile,
			  UINT uiLineNumber);

APIRET xDosFindFirst(PSZ pszFileSpec,
		     PHDIR phdir,
		     ULONG  flAttribute,
		     PVOID  pfindbuf,
		     ULONG  cbBuf,
		     PULONG pcFileNames,
		     ULONG  ulInfoLevel);

APIRET xDosFindNext(HDIR   hDir,
		    PVOID  pfindbuf,
		    ULONG  cbfindbuf,
		    PULONG pcFilenames,
		    ULONG  ulInfoLevel);	// 06 Oct 07 SHL Added

#ifdef INCL_DOSPROCESS  // // PPIB PTIB
APIRET xDosGetInfoBlocks(PTIB *pptib,
			 PPIB *pppib);  // 2015-08-09 SHL added
#endif

APIRET xDosSetPathInfo(PSZ   pszPathName,
		       ULONG ulInfoLevel,
		       PVOID pInfoBuf,
		       ULONG cbInfoBuf,
		       ULONG flOptions);

PSZ xfgets(PSZ pszBuf, size_t cMaxBytes, FILE * fp, PCSZ pszSrcFile,
	   UINT uiLineNumber);
PSZ xfgets_bstripcr(PSZ pszBuf, size_t cMaxBytes, FILE * fp, PCSZ pszSrcFile,
		    UINT uiLineNumber);
FILE *xfopen(PCSZ pszFileName, PCSZ pszMode, PCSZ pszSrcFile,
	     UINT uiLineNumber, BOOL fSilent);
VOID xfree(PVOID pv, PCSZ pszSrcFile, UINT uiLineNumber);
FILE *xfsopen(PCSZ pszFileName, PCSZ pszMode, INT fSharemode, PCSZ pszSrcFile,
	      UINT uiLineNumber, BOOL fSilent);
PVOID xmalloc(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber);
PVOID xmallocz(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber);
PVOID xrealloc(PVOID pvIn, size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber);
PVOID xstrdup(PCSZ pszIn, PCSZ pszSrcFile, UINT uiLineNumber);

#define HIGH_MEMORY_ADDRESS(p) (((ULONG)p) >= (512*1024*1024))

// Data declarations
extern BOOL fNoLargeFileSupport;


#endif // WRAPPERS_H
