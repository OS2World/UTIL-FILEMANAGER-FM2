
/***********************************************************************

  $Id$

  Wrappers with error checking

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2015 Steven H. Levine

  08 Dec 08 SHL Add missing OS2_INCLUDED check
  12 Jul 09 GKY Add xDosQueryAppType and xDoxAlloc... to allow FM/2 to load in high memory
  26 Aug 11 GKY Add a low mem version of xDosAlloc* wrappers; move error checking into all the
		xDosAlloc* wrappers.
  09 Aug 15 SHL Add xDosGetInfoBlocks

***********************************************************************/

#if !defined(WRAPPERS_H)
#define WRAPPERS_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

APIRET xDosQueryAppType(PCSZ pszName, PULONG pFlags);

APIRET xDosAllocSharedMem(PPVOID ppb,
			  PSZ pszName,
			  ULONG cb,
			  PCSZ pszSrcFile,
			  UINT uiLineNumber);

APIRET xDosAllocMem(PPVOID ppb,
		    ULONG cb,
		    PCSZ pszSrcFile,
		    UINT uiLineNumber);

APIRET xDosAllocMemLow(PPVOID ppb,
		       ULONG cb,
		       PCSZ pszSrcFile,
		       UINT uiLineNumber);

APIRET xDosFindFirst(PSZ pszFileSpec,
		     PHDIR phdir,
		     ULONG flAttribute,
		     PVOID pfindbuf,
		     ULONG cbBuf,
		     PULONG pcFileNames,
		     ULONG ulInfoLevel);
APIRET xDosFindNext(HDIR hDir,
		    PVOID pfindbuf,
		    ULONG cbfindbuf,
		    PULONG pcFilenames,
		    ULONG ulInfoLevel);	// 06 Oct 07 SHL Added

#ifdef INCL_DOSPROCESS	// // PPIB PTIB
APIRET xDosGetInfoBlocks(PTIB *pptib,
			 PPIB *pppib);	// 2015-08-09 SHL added
#endif

APIRET xDosSetPathInfo(PSZ pszPathName,
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

// Data declarations
extern BOOL fNoLargeFileSupport;

#endif // WRAPPERS_H
