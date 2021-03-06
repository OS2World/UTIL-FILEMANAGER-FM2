
/***********************************************************************

  $Id$

  Directory sizes

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2015 Steven H. Levine

  16 Oct 02 SHL Handle large partitions
  12 Feb 03 SHL Use CBLIST_TO_EASIZE
  21 Nov 03 SHL Avoid VAC \ after // bug (wierd)
  21 Nov 03 SHL Correct minor typos
  21 Nov 03 SHL Total drives >4GB better
  24 May 05 SHL Rework for CNRITEM.szSubject
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  26 May 05 SHL More large file formatting updates
  06 Jun 05 SHL Drop obsoletes
  19 Jun 05 SHL More 64-bit math fixes
  08 Aug 05 SHL Avoid Expand/Collapse hangs while working
  17 Jul 06 SHL Use Runtime_Error
  19 Oct 06 SHL Correct . and .. detect
  18 Feb 07 GKY Add new drive type icons
  22 Mar 07 GKY Use QWL_USER
  23 Jul 07 SHL Sync with naming standards
  03 Aug 07 GKY Enlarged and made setable everywhere Findbuf (speed file loading)
  03 Aug 07 SHL DirSizeProc; correct sizing and positioning to be deterministic
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  13 Aug 07 SHL ProcessDir: remove unneeded reallocs.  Sanitize code
  13 Aug 07 SHL Move #pragma alloc_text to end for OpenWatcom compat
  26 Aug 07 GKY DosSleep(1) in loops changed to (0)
  29 Feb 08 GKY Use xfree where appropriate
  29 Feb 08 GKY Add presparams & update appearence of "Sizes" dialog
  07 Jul 08 GKY Fixed trap in PMCTLS (strlen) inadequate memory allocation
  07 Jul 08 GKY Fixed trap by no longer allocating pci->pszLongName as flag but pointing isroot
		version to NullStr and all others to NULL.
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory; use pTmpDir for temp files
  03 Aug 08 GKY Reworked FillInRecSizes to use pci->pszDisplayName for display names and
		created a more consitent string for passing to DRAWITEM. Finally (I hope) fixed
		the strlen trap.
  23 Aug 08 GKY Fix memory leak (failure to free cnritems)
  10 Dec 08 SHL Integrate exception handler support
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.
  13 Dec 09 GKY Fixed separate paramenters. Please note that appname should be used in
                profile calls for user settings that work and are setable in more than one
                miniapp; FM3Str should be used for setting only relavent to FM/2 or that
                aren't user settable; realappname should be used for setting applicable to
                one or more miniapp but not to FM/2
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10).
                Mostly cast CHAR CONSTANT * as CHAR *.
  20 Nov 10 GKY Check that pTmpDir IsValid and recreate if not found; Fixes hangs caused
                by temp file creation failures.
  12 Jun 11 GKY Added SleepIfNeeded in the container fill loop
  20 Sep 15 GKY Move directory expansion to a thread. Split ExpandAll into ExpandAll and CollapseAll
                to try to speed proccessing else where.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// #include <process.h>			// _beginthread

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "draglist.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "newview.h"			// Data declarations
#include "fm3dlg.h"
#include "fm3str.h"
#include "dircnrs.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "filldir.h"			// EmptyCnr...
#include "dirsize.h"
#include "select.h"			// ExpandAll
#include "valid.h"			// CheckDrive
#include "common.h"			// OpenDirCnr
#include "shadow.h"			// OpenObject
#include "presparm.h"			// PresParamChanged
#include "i18nutil.h"			// commafmt
#include "getnames.h"			// export_filename
#include "wrappers.h"			// xDosFindNext
#include "dirs.h"			// save_dir2
#include "misc.h"			// PostMsg
#include "fortify.h"
#include "excputil.h"			// xbeginthread
#include "pathutil.h"                   // AddBackslashToPath
#include "tmrsvcs.h"

typedef struct
{
  CHAR *pszFileName;
  HWND hwndCnr;
  CHAR *pchStopFlag;
  DIRCNRDATA *pDCD;
} DIRSIZE;

typedef struct
{
  HWND hwndCnr;
  PCNRITEM pci;
} EXPANDSIZE;

typedef struct
{
  CHAR szDirName[CCHMAXPATH];
  CHAR chStopFlag;
  BOOL dying;
  BOOL working;
  HPOINTER hptr;
} tState;

static PSZ pszSrcFile = __FILE__;

static SHORT APIENTRY SortSizeCnr(PMINIRECORDCORE p1,
				  PMINIRECORDCORE p2,
				  PVOID SortFlags)
{
  ULONGLONG size1;
  ULONGLONG size2;

  size1 = ((PCNRITEM) p1)->cbFile + ((PCNRITEM) p1)->easize;
  size2 = ((PCNRITEM) p2)->cbFile + ((PCNRITEM) p2)->easize;
  return (size1 < size2) ? 1 : (size1 == size2) ? 0 : -1;
}

static BOOL ProcessDir(HWND hwndCnr,
		       CHAR *pszFileName,
		       PCNRITEM pciParent,
		       CHAR *pchStopFlag,
		       BOOL top,
		       PULONGLONG pullTotalBytes)
{
  CHAR maskstr[CCHMAXPATH];
  CHAR szBuf[CCHMAXPATH];
  CHAR FileSystem[CCHMAXPATH];
  CHAR *pEndMask;
  register char *p;
  register char *sp;
  register char *pp;
  ULONG ulFindCnt;
  ULONGLONG ullCurDirBytes = 0;
  ULONGLONG ullSubDirBytes = 0;
  ULONGLONG ull;
  HDIR hdir;
  PFILEFINDBUF4L pffbArray;
  APIRET rc;
  RECORDINSERT ri;
  PCNRITEM pci;
  ULONG ulBufBytes;

  *pullTotalBytes = 0;			// In case we fail

  CheckDrive(toupper(*pszFileName), FileSystem, NULL);
  ulBufBytes = sizeof(FILEFINDBUF4L) * FilesToGet;
  pffbArray = xmalloc(ulBufBytes, pszSrcFile, __LINE__);
  if (!pffbArray)
    return FALSE;			// Error already reported

  strcpy(maskstr, pszFileName);
  AddBackslashToPath(maskstr);
  pEndMask = &maskstr[strlen(maskstr)];	// Point after last backslash
  strcat(maskstr, "*");

  hdir = HDIR_CREATE;
  ulFindCnt = 1;
  DosError(FERR_DISABLEHARDERR);
  // Check directory exists
  rc = xDosFindFirst(pszFileName, &hdir,
		     FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
		     FILE_SYSTEM | FILE_HIDDEN | MUST_HAVE_DIRECTORY,
		     pffbArray, ulBufBytes, &ulFindCnt, FIL_QUERYEASIZEL);

  if (!rc)
    DosFindClose(hdir);

  /**
   * the "|| strlen(pszFileName) < 4 below works around an OS/2 bug
   * that prevents FAT root directories from being found when
   * requesting EASIZE.  sheesh.
   */
  if (((!rc || rc == ERROR_NO_MORE_FILES) && (pffbArray->attrFile & FILE_DIRECTORY)) ||
      strlen(pszFileName) < 4) {
    if (*pchStopFlag) {
      free(pffbArray);
      return FALSE;
    }
    pci = WinSendMsg(hwndCnr, CM_ALLOCRECORD, MPFROMLONG(EXTRA_RECORD_BYTES),
		     MPFROMLONG(1));
    if (!pci) {
      Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__, PCSZ_CM_ALLOCRECORD);
      xfree(pffbArray, pszSrcFile, __LINE__);
      return FALSE;
    }
    if (!rc) {
      ullCurDirBytes = pffbArray->cbFile;
      ullCurDirBytes += CBLIST_TO_EASIZE(pffbArray->cbList);
    }
    else
      DosError(FERR_DISABLEHARDERR);
    pci->rc.hptrIcon = hptrDir;
    pci->attrFile = 0;
    pci->pszDispAttr = NULL;
    pci->pszSubject = NULL;
  } // if got something
  else {
    // No match
    xfree(pffbArray, pszSrcFile, __LINE__);
    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      GetPString(IDS_CANTFINDDIRTEXT), pszFileName);
    return FALSE;
  }

  if (strlen(pszFileName) < 4 || top)
    pci->pszFileName = xstrdup(pszFileName, pszSrcFile, __LINE__);
  else {
    p = strrchr(pszFileName, '\\');
    if (!p)
      p = pszFileName;
    else
      p++;				// After last backslash
    // Handle quoted names
    // fixme to understand this - why lose path prefix?
    sp = strchr(pszFileName, ' ') != NULL ? "\"" : NullStr;
    pp = szBuf;
    if (*sp)
      *pp++ = *sp;			// Need quotes
    strcpy(pp, p);
    if (*sp)
      strcat(pp, sp);
    pci->pszFileName = xstrdup(szBuf, pszSrcFile, __LINE__);
  }
  // Use pszDisplayname for display so no need to save length of pszFileName 03 Aug 08 GKY
  pci->pszDisplayName = pci->pszFileName;
  pci->rc.pszIcon = pci->pszFileName;
  pci->rc.flRecordAttr |= CRA_RECORDREADONLY;
  if (fForceUpper)
    strupr(pci->pszFileName);
  else if (fForceLower)
    strlwr(pci->pszFileName);
  memset(&ri, 0, sizeof(RECORDINSERT));
  ri.cb = sizeof(RECORDINSERT);
  ri.pRecordOrder = (PRECORDCORE) CMA_END;
  ri.pRecordParent = (PRECORDCORE) pciParent;
  ri.zOrder = (USHORT) CMA_TOP;
  ri.cRecordsInsert = 1;
  ri.fInvalidateRecord = TRUE;
  if (!WinSendMsg(hwndCnr, CM_INSERTRECORD, MPFROMP(pci), MPFROMP(&ri))) {
    xfree(pffbArray, pszSrcFile, __LINE__);
    return FALSE;
  }

  // Find files and directories in this directory
  hdir = HDIR_CREATE;
  // 13 Aug 07 SHL fixme to know if need to support fRemoteBug here like objcnr.c?
  ulFindCnt = FilesToGet;
  rc = xDosFindFirst(maskstr, &hdir,
		     FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
		     FILE_SYSTEM | FILE_HIDDEN | FILE_DIRECTORY,
		     pffbArray, ulBufBytes, &ulFindCnt, FIL_QUERYEASIZEL);
  if (!rc) {
    PFILEFINDBUF4L pffbFile;
    ULONG x;
    ITIMER_DESC itdSleep = { 0 };		// 30 May 11 GKY

    InitITimer(&itdSleep, 500);
    while (!rc) {

      priority_normal();
      pffbFile = pffbArray;
      for (x = 0; x < ulFindCnt; x++) {
	// Total size skipping . and ..
	if ((~pffbFile->attrFile & FILE_DIRECTORY) ||
	    (pffbFile->achName[0] != '.' ||
	     (pffbFile->achName[1] &&
	      (pffbFile->achName[1] != '.' || pffbFile->achName[2])))) {
	  ullCurDirBytes += pffbFile->cbFile;
	  ullCurDirBytes += CBLIST_TO_EASIZE(pffbFile->cbList) & 0x3ff;

	  if (*pchStopFlag)
	    break;
	  if (~pffbFile->attrFile & FILE_DIRECTORY)
	    pci->attrFile++;		// Bump file count
	  else {
	    // Recurse into subdir
	    strcpy(pEndMask, pffbFile->achName);	// Append dirname to base dirname
	    ProcessDir(hwndCnr, maskstr, pci, pchStopFlag, FALSE, &ull);
	    ullSubDirBytes += ull;
	  }
	}
	if (!pffbFile->oNextEntryOffset)
	  break;
	pffbFile = (PFILEFINDBUF4L)((PBYTE)pffbFile + pffbFile->oNextEntryOffset);

      }	// for matches
      if (*pchStopFlag)
	break;
      DosSleep(0);
      ulFindCnt = FilesToGet;
      DosError(FERR_DISABLEHARDERR);
      rc = xDosFindNext(hdir, pffbArray, ulBufBytes, &ulFindCnt, FIL_QUERYEASIZEL);
      SleepIfNeeded(&itdSleep, 1);
    } // while more found

    DosFindClose(hdir);
    priority_normal();
  } // if got files or directories

  if (rc && rc != ERROR_NO_MORE_FILES) {
    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      GetPString(IDS_CANTFINDDIRTEXT), pszFileName);
  }

  xfree(pffbArray, pszSrcFile, __LINE__);

  pci->cbFile = ullCurDirBytes;
  pci->easize = ullSubDirBytes;		// hack cough
  WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPFROMP(&pci),
	     MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));

  *pullTotalBytes = ullCurDirBytes + ullSubDirBytes;
  return TRUE;
}

static VOID FillInRecSizes(HWND hwndCnr, PCNRITEM pciParent,
			   ULONGLONG ullTotalBytes, CHAR * pchStopFlag,
			   BOOL isroot)
{
  PCNRITEM pci = pciParent;
  SHORT attrib = CMA_FIRSTCHILD;

  if (pci) {

    float fltPct = (float) 0.0;
    CHAR szCurDir[80];
    CHAR szSubDir[80];
    CHAR szAllDir[80];
    CHAR szBar[101];
    CHAR szBuf[CCHMAXPATH + 341];

    // cbFile = currect directory usage in bytes
    // easize = subdirectory usage in bytes
    CommaFmtULL(szCurDir, sizeof(szCurDir), pci->cbFile, 'K');
    *szBar = 0;
    pci->pszLongName = NULL;
    memset(szBuf, 0, sizeof(szBuf));
    if (ullTotalBytes) {
      UINT cBar;

      if (isroot) {
	FSALLOCATE fsa;
	APIRET rc;


	memset(&fsa, 0, sizeof(fsa));
	rc = DosQueryFSInfo(toupper(*pci->pszFileName) - '@', FSIL_ALLOC, &fsa,
			    sizeof(FSALLOCATE));
	if (!rc) {
	  fltPct = (float) (ullTotalBytes * 100.0) /
	    ((float)fsa.cUnit *  (fsa.cSectorUnit * fsa.cbSector));
	}
	// Need unique buffer 23 Jul 07 SHL
	pci->pszLongName = NullStr;
      }
      else
	fltPct = (float) (((float)pci->cbFile + pci->easize) * 100.0) / ullTotalBytes;

      //Second line for graph reworked 03 AUG 08 GKY
      memset(szBar, ' ', sizeof(szBar));
      cBar = (UINT) fltPct / 2;
      if (cBar && cBar * 2 != (UINT) fltPct)
	szBar[cBar] = '=';
      szBar[100] = 0;
    }

    pci->flags = (ULONG) fltPct;
    CommaFmtULL(szSubDir, sizeof(szSubDir), pci->easize, 'K');
    CommaFmtULL(szAllDir, sizeof(szAllDir), pci->cbFile + pci->easize, 'K');
    sprintf(szBuf,
	    "%s  %s + %s = %s (%.02lf%%%s)\r%s",
	    pci->pszFileName,
	    szCurDir,
	    szSubDir,
	    szAllDir,
	    fltPct,
	    isroot ? GetPString(IDS_OFDRIVETEXT) : NullStr,
	    szBar);
    pci->pszDisplayName = xstrdup(szBuf, pszSrcFile, __LINE__);
    // use DisplayName for display hopefully fixes "strlen" trap 02 AUG 08 GKY
    if (pci->pszDisplayName) {
#     ifdef FORTIFY
      Fortify_ChangeScope(pci->pszDisplayName, -1);
      Fortify_ChangeScope(pci->pszFileName, -1);
#     endif
      WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPFROMP(&pci), MPFROM2SHORT(1, 0));
    }
    isroot = FALSE;
  }
  else
    attrib = CMA_FIRST;
  pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pci),
			      MPFROM2SHORT(attrib, CMA_ITEMORDER));
  while (pci && (INT) pci != -1) {
    if (*pchStopFlag)
      break;
    FillInRecSizes(hwndCnr, pci, ullTotalBytes, pchStopFlag, isroot);
    isroot = FALSE;
    pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pci),
				MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
  }
}

static VOID PrintToFile(HWND hwndCnr, ULONG indent, PCNRITEM pciParent,
			FILE * fp)
{
  PCNRITEM pci;
  CHAR *p;

  if (!pciParent) {
    pciParent = WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(NULL),
			   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
    indent = 0;
  }
  if (pciParent) {
    p = strchr(pciParent->pszDisplayName, '\r'); // GKY use display name for display
    if (p)
      *p = 0;
    fprintf(fp, "%*.*s%s %lu %s%s\n",
	    indent * 2, indent * 2, " ",
	    pciParent->pszDisplayName,
	    pciParent->attrFile,
	    GetPString(IDS_FILETEXT), &"s"[pciParent->attrFile == 1]);
    if (p)
      *p = '\r';
    if (pciParent->rc.flRecordAttr & CRA_EXPANDED) {
      pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pciParent),
				  MPFROM2SHORT(CMA_FIRSTCHILD,
					       CMA_ITEMORDER));
      while (pci && (INT) pci != -1) {
	DosSleep(0); 
	PrintToFile(hwndCnr, indent + 1, pci, fp);
	pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pci),
				    MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
      } //while
    }
  }
}

static VOID FillCnrThread(VOID *args)
{
  HAB hab;
  HMQ hmq;
  DIRSIZE *dirsize = (DIRSIZE *)args;
  HWND hwndCnr;
  ULONGLONG ull;

  if (!dirsize) {
    Runtime_Error(pszSrcFile, __LINE__, NULL);
    return;
  }
# ifdef FORTIFY
  Fortify_EnterScope();
  Fortify_BecomeOwner(dirsize);		// We free dirsize
#  endif

  hwndCnr = dirsize->hwndCnr;

  DosError(FERR_DISABLEHARDERR);

  // priority_normal();
  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 0);
    if (hmq) {
      WinCancelShutdown(hmq, TRUE);
      ProcessDir(hwndCnr, dirsize->pszFileName,
		 (PCNRITEM) NULL, dirsize->pchStopFlag, TRUE, &ull);
      DosPostEventSem(CompactSem);
      WinEnableWindowUpdate(hwndCnr, FALSE);
      FillInRecSizes(hwndCnr, NULL, ull, dirsize->pchStopFlag, TRUE);
      WinEnableWindowUpdate(hwndCnr, TRUE);
      WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPVOID,
		 MPFROM2SHORT(0, CMA_ERASE | CMA_TEXTCHANGED));
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }

  xfree(dirsize, pszSrcFile, __LINE__);
  PostMsg(WinQueryWindow(hwndCnr, QW_PARENT),
	  UM_CONTAINER_FILLED, MPVOID, MPVOID);
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

static VOID ExpandCnrThread(VOID *args)
{
  HAB hab;
  HMQ hmq;
  INT x = 0;
  EXPANDSIZE *expandsize = (EXPANDSIZE *)args;

  if (!expandsize) {
    Runtime_Error(pszSrcFile, __LINE__, NULL);
    return;
  }
# ifdef FORTIFY
  Fortify_EnterScope();
  Fortify_BecomeOwner(expandsize);		// We free dirsize
#  endif

  DosError(FERR_DISABLEHARDERR);

  priority_idle();
  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 0);
    if (hmq) {
      WinCancelShutdown(hmq, TRUE);
      ExpandAll(expandsize->hwndCnr, x, expandsize->pci);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }

  xfree(expandsize, pszSrcFile, __LINE__);
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

MRESULT EXPENTRY DirSizeProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  tState *pState;
  PCNRITEM pci;
  CHAR szBytes[44];
  CHAR sz[66];

  switch (msg) {
  case WM_INITDLG:
    if (!mp2) {
      WinDismissDlg(hwnd, 0);
      break;
    }
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
    pState = xmallocz(sizeof(tState), pszSrcFile, __LINE__);
    if (!pState) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    strcpy(pState->szDirName, (CHAR *)mp2);
    WinSetWindowPtr(hwnd, QWL_USER, (PVOID) pState);
    pState->hptr = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, DIRSIZE_ICON);
    WinDefDlgProc(hwnd, WM_SETICON, MPFROMLONG(pState->hptr), MPVOID);
    {
      CHAR s[CCHMAXPATH + 81];
      RestorePresParams(hwnd, PCSZ_DIRSIZES);
      sprintf(s, GetPString(IDS_DIRSIZETITLETEXT), pState->szDirName);
      WinSetWindowText(hwnd, s);
    }
    {
      SWP swp;
      ULONG size = sizeof(SWP);

      PrfQueryProfileData(fmprof, FM3Str, "DirSizes.Position", (PVOID) &swp, &size);
      swp.fl &= ~SWP_SIZE;		// 04 Feb 09 SHL ignore saved size
      WinSetWindowPos(hwnd,
		      HWND_TOP,
		      swp.x,
		      swp.y,
		      swp.cx,
		      swp.cy,
		      swp.fl);
    }
    {
      DIRSIZE *dirsize;

      dirsize = xmalloc(sizeof(DIRSIZE), pszSrcFile, __LINE__);
      if (!dirsize) {
	WinDismissDlg(hwnd, 0);
	break;
      }
      dirsize->pchStopFlag = (CHAR *)&pState->chStopFlag;
      dirsize->pszFileName = pState->szDirName;
      dirsize->hwndCnr = WinWindowFromID(hwnd, DSZ_CNR);
      if (xbeginthread(FillCnrThread,
		       122880 * 5,
		       dirsize,
		       pszSrcFile,
		       __LINE__) == -1)
      {
	xfree(dirsize, pszSrcFile, __LINE__);
	WinDismissDlg(hwnd, 0);
	break;
      }
      pState->working = TRUE;
      WinEnableWindow(WinWindowFromID(hwnd, DSZ_COLLAPSE), FALSE);
      WinEnableWindow(WinWindowFromID(hwnd, DSZ_EXPAND), FALSE);
      WinEnableWindow(WinWindowFromID(hwnd, DSZ_PRINT), FALSE);
    }
    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    break;

  case UM_SETUP:
    {
      CNRINFO cnri;
      FSALLOCATE fsa;
      APIRET rc;

      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendDlgItemMsg(hwnd, DSZ_CNR, CM_QUERYCNRINFO,
			MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
      cnri.cyLineSpacing = 0;
      cnri.cxTreeIndent = 12;
      cnri.flWindowAttr = CV_TREE | CV_FLOW | CA_TREELINE | CA_OWNERDRAW;
      WinSendDlgItemMsg(hwnd, DSZ_CNR, CM_SETCNRINFO, MPFROMP(&cnri),
			MPFROMLONG(CMA_FLWINDOWATTR | CMA_TREEICON |
				   CMA_LINESPACING | CMA_CXTREEINDENT));
      pState = INSTDATA(hwnd);
      if (pState && isalpha(*pState->szDirName)) {
	memset(&fsa, 0, sizeof(fsa));
	rc =
	  DosQueryFSInfo(toupper(*pState->szDirName) - '@', FSIL_ALLOC, &fsa,
			 sizeof(FSALLOCATE));
	if (!rc) {

	  CHAR s[132], tf[80], tb[80], tu[80];

	  CommaFmtULL(tf, sizeof(tf),
		      (ULONGLONG) fsa.cUnitAvail *
		      (fsa.cSectorUnit * fsa.cbSector), 'M');
	  CommaFmtULL(tb, sizeof(tb),
		      (ULONGLONG) fsa.cUnit *
		      (fsa.cSectorUnit * fsa.cbSector), 'M');
	  CommaFmtULL(tu, sizeof(tu),
		      (ULONGLONG) (fsa.cUnit - fsa.cUnitAvail) *
		      (fsa.cSectorUnit * fsa.cbSector), 'M');
	  sprintf(s, GetPString(IDS_FREESPACETEXT), tf, tb, tu);
	  WinSetDlgItemText(hwnd, DSZ_FREESPACE, s);
	}
	else
	  WinSetDlgItemText(hwnd, DSZ_FREESPACE, (CHAR *) GetPString(IDS_FREESPACEUTEXT));
      }
    }
    return 0;

  case UM_CONTAINER_FILLED:
    pState = INSTDATA(hwnd);
    if (!pState || pState->dying) {
      if (pState)
	pState->working = FALSE;
      WinDismissDlg(hwnd, 0);
      return 0;
    }
    pState->working = FALSE;
    WinEnableWindow(WinWindowFromID(hwnd, DSZ_COLLAPSE), TRUE);
    WinEnableWindow(WinWindowFromID(hwnd, DSZ_EXPAND), TRUE);
    WinEnableWindow(WinWindowFromID(hwnd, DSZ_PRINT), TRUE);

    pci = WinSendDlgItemMsg(hwnd, DSZ_CNR, CM_QUERYRECORD, MPVOID,
			    MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
    if (pci && (INT) pci != -1)
      WinSendDlgItemMsg(hwnd, DSZ_CNR, CM_EXPANDTREE, MPFROMP(pci), MPVOID);
    *sz = 0;
    pci = WinSendDlgItemMsg(hwnd, DSZ_CNR, CM_QUERYRECORDEMPHASIS,
			    MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
    if (pci && (INT) pci != -1) {
      commafmt(szBytes, sizeof(szBytes), pci->attrFile);
      sprintf(sz,
	      "%s %s%s",
	      szBytes, GetPString(IDS_FILETEXT), &"s"[pci->attrFile == 1]);
    }
    WinSetDlgItemText(hwnd, DSZ_NUMFILES, sz);

    WinSendDlgItemMsg(hwnd, DSZ_CNR, CM_SORTRECORD, MPFROMP(SortSizeCnr),
		      MPVOID);
    if (!fAlertBeepOff)
      DosBeep(500, 25);			// Wake up user
    return 0;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
    break;

  case UM_STRETCH:
    {
      SWP swpC, swp;

      WinQueryWindowPos(hwnd, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
	WinQueryWindowPos(WinWindowFromID(hwnd, DSZ_CNR), &swpC);
	WinSetWindowPos(WinWindowFromID(hwnd, DSZ_CNR), HWND_TOP,
			SysVal(SV_CXSIZEBORDER),
			swpC.y,
			swp.cx - (SysVal(SV_CXSIZEBORDER) * 2),
			(swp.cy - swpC.y) - (SysVal(SV_CYTITLEBAR) +
					     SysVal(SV_CYSIZEBORDER)),
			SWP_MOVE | SWP_SIZE);
      }
    }
    return 0;

  case WM_PRESPARAMCHANGED:
    PresParamChanged(hwnd, PCSZ_DIRSIZES, mp1, mp2);
    break;

  case WM_DRAWITEM:
    if (mp2) {

      OWNERITEM *oi = mp2;
      CNRDRAWITEMINFO *cnd;
      PCNRITEM pci;

      if (oi->idItem == CMA_TEXT) {

	cnd = (CNRDRAWITEMINFO *)oi->hItem;

	if (cnd) {
	  pci = (PCNRITEM)cnd->pRecord;

	  if (pci) {
	    POINTL aptl[TXTBOX_COUNT];
	    POINTL ptl;
	    PSZ p;
	    LONG clr;
	    LONG x;
	    LONG yBottom;
	    INT boxHeight;
	    p = strchr(pci->pszDisplayName, '\r');
	    if (p) {
	      // draw text
	      if (pci->pszLongName == NullStr)  // is root record
		GpiSetColor(oi->hps, CLR_DARKRED);
	      else if (!pci->cbFile)		// no size
		GpiSetColor(oi->hps, CLR_DARKGRAY);
	      else if (!pci->easize)	// no size below
		GpiSetColor(oi->hps, CLR_DARKBLUE);
	      else
		GpiSetColor(oi->hps, CLR_BLACK);
	      GpiSetBackMix(oi->hps, BM_LEAVEALONE);
	      GpiSetMix(oi->hps, FM_OVERPAINT);

	      *p = 0;			// Make 1 line high

	      // Calculate nominal graph box height based on font size
	      GpiQueryTextBox(oi->hps, p - pci->pszDisplayName,
			      pci->pszDisplayName, TXTBOX_COUNT, aptl);
	      boxHeight = aptl[TXTBOX_TOPRIGHT].y - aptl[TXTBOX_BOTTOMRIGHT].y;
	      boxHeight -= 4;

	      // Calculate nominal baseline of graph box
	      // rclItem.yBottom is at center of icon because it is
	      yBottom = oi->rclItem.yBottom - boxHeight + 3;

	      // Place text above graph box with a bit of whitespace between
	      ptl.x = oi->rclItem.xLeft;
	      ptl.y = yBottom + boxHeight + 6;
	      GpiCharStringAt(oi->hps, &ptl, p - pci->pszDisplayName,
			      pci->pszDisplayName);

	      *p = '\r';		// Restore

	      // draw the graph box
	      // draw black outline
	      GpiSetColor(oi->hps, CLR_BLACK);
	      ptl.x = oi->rclItem.xLeft;
	      ptl.y = yBottom + 2;
	      GpiMove(oi->hps, &ptl);
	      ptl.x = oi->rclItem.xLeft + 201;
	      ptl.y = yBottom + boxHeight;
	      GpiBox(oi->hps, DRO_OUTLINE, &ptl, 0, 0);
	      // fill with gray
	      GpiSetColor(oi->hps, CLR_PALEGRAY);
	      ptl.x = oi->rclItem.xLeft + 1;
	      ptl.y = yBottom + 3;
	      GpiMove(oi->hps, &ptl);
	      ptl.x = oi->rclItem.xLeft + 200;
	      ptl.y = yBottom + boxHeight - 1;
	      GpiBox(oi->hps, DRO_OUTLINEFILL, &ptl, 0, 0);

	      // draw shadow at bottom & right sides
	      GpiSetColor(oi->hps, CLR_DARKGRAY);
	      ptl.x = oi->rclItem.xLeft + 1;
	      ptl.y = yBottom + 3;
	      GpiMove(oi->hps, &ptl);
	      ptl.x = oi->rclItem.xLeft + 200;
	      GpiLine(oi->hps, &ptl);
	      ptl.y = yBottom + boxHeight - 1;
	      GpiLine(oi->hps, &ptl);

	      // draw highlight at top and left sides
	      GpiSetColor(oi->hps, CLR_WHITE);
	      ptl.x = oi->rclItem.xLeft + 1;
	      GpiLine(oi->hps, &ptl);
	      ptl.y = yBottom + 3;
	      GpiLine(oi->hps, &ptl);

	      // draw shadow of box
	      GpiSetColor(oi->hps, CLR_DARKGRAY);
	      ptl.x = oi->rclItem.xLeft + 2;
	      ptl.y = yBottom + boxHeight;
	      GpiMove(oi->hps, &ptl);
	      ptl.x = oi->rclItem.xLeft + 201;
	      GpiLine(oi->hps, &ptl);
	      ptl.y = yBottom + boxHeight - 2;
	      GpiLine(oi->hps, &ptl);
	      ptl.x--;
	      GpiMove(oi->hps, &ptl);
	      ptl.y = yBottom + 1;
	      GpiLine(oi->hps, &ptl);
	      ptl.x = oi->rclItem.xLeft + 2;
	      GpiLine(oi->hps, &ptl);

	      // fill box with graph bar, flags is integer %
	      if (pci->flags) {
		if (pci->pszLongName == NullStr)	// is root record
		  GpiSetColor(oi->hps, CLR_DARKGREEN);
		else
		  GpiSetColor(oi->hps, CLR_RED);
		ptl.x = oi->rclItem.xLeft + 2;
		ptl.y = yBottom + 3;
		GpiMove(oi->hps, &ptl);
		ptl.x = oi->rclItem.xLeft + pci->flags * 2;
		ptl.y = yBottom + boxHeight - 1;
		GpiBox(oi->hps, DRO_OUTLINEFILL, &ptl, 0, 0);

		// draw highlights and shadows on graph
		if (pci->pszLongName == NullStr)
		  GpiSetColor(oi->hps, CLR_GREEN);
		else
		  GpiSetColor(oi->hps, CLR_PALEGRAY);
		if (pci->flags > 5) {
		  ptl.x = oi->rclItem.xLeft + 1;
		  ptl.y = yBottom + 3;
		  GpiMove(oi->hps, &ptl);
		  ptl.y = yBottom + boxHeight - 1;
		  GpiLine(oi->hps, &ptl);
		}
		else {
		  ptl.y = yBottom + boxHeight - 1;
		  GpiMove(oi->hps, &ptl);
		}
		ptl.x = oi->rclItem.xLeft + pci->flags * 2;
		GpiLine(oi->hps, &ptl);
		if (pci->pszLongName == NULL) {
		  GpiSetColor(oi->hps, CLR_DARKRED);
		  ptl.x = oi->rclItem.xLeft + 2;
		  ptl.y = yBottom + 3;
		  GpiMove(oi->hps, &ptl);
		  ptl.x = oi->rclItem.xLeft + pci->flags * 2;
		  GpiLine(oi->hps, &ptl);
		}
	      }

	      // draw hash marks in box
	      GpiSetColor(oi->hps, CLR_WHITE);
	      clr = CLR_WHITE;
	      for (x = 1; x < 20; x++) {
		if (clr == CLR_WHITE && x * 10 > pci->flags * 2) {
		  clr = CLR_BLACK;
		  GpiSetColor(oi->hps, CLR_BLACK);
		}
		ptl.x = oi->rclItem.xLeft + 1 + x * 10;
		ptl.y = yBottom + boxHeight - 1;
		GpiMove(oi->hps, &ptl);
		switch (x) {
		case 1:
		case 3:
		case 5:
		case 7:
		case 9:
		case 11:
		case 13:
		case 15:
		case 17:
		case 19:
		  ptl.y -= 1;
		  break;
		case 10:
		  ptl.y -= 4;
		  break;
		case 2:
		case 4:
		case 6:
		case 8:
		case 12:
		case 14:
		case 16:
		case 18:
		  ptl.y -= 2;
		  break;
		}
		GpiLine(oi->hps, &ptl);
	      } // for x
	      return MRFROMLONG(TRUE);
	    }
	  }
	}
      }
    }
    return FALSE;			// Let PM draw

  case WM_CONTROL:
    switch (SHORT2FROMMP(mp1)) {
    case CN_ENTER:
      if (mp2) {
	PCNRITEM pci = (PCNRITEM)((PNOTIFYRECORDENTER)mp2)->pRecord;
	CHAR szFileName[CCHMAXPATH];	// 23 Jul 07 SHL
	CHAR szTemp[CCHMAXPATH];

	if (pci) {
	  *szFileName = 0;
	  while (pci && (INT) pci != -1) {
	    memset(szTemp, 0, sizeof(szTemp));
	    strcpy(szTemp, pci->pszFileName);
            strrev(szTemp);
            AddBackslashToPath(szFileName);
	    strcat(szFileName, szTemp);
	    pci = WinSendDlgItemMsg(hwnd, DSZ_CNR, CM_QUERYRECORD,
				    MPFROMP(pci),
				    MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER));
	  }
	  strrev(szFileName);
	  if (!fVTreeOpensWPS)
	    OpenDirCnr((HWND)0,
		       hwndMain ? hwndMain : HWND_DESKTOP,
		       hwnd,
		       FALSE,
		       szFileName);
	  else {

	    ULONG size = sizeof(ULONG);
	    ULONG flWindowAttr = CV_ICON;
	    CHAR s[33];

	    strcpy(s, PCSZ_ICON);
	    PrfQueryProfileData(fmprof, appname, "DirflWindowAttr",
				(PVOID) &flWindowAttr, &size);
	    if (flWindowAttr & CV_DETAIL) {
	      if (IsRoot(szFileName))
		strcpy(s, PCSZ_TREE);
	      else
		strcpy(s, Details);
	    }
	    OpenObject(szFileName, s, hwnd);
	  }
	}
      }
      break;
    case CN_EMPHASIS:
      pState = INSTDATA(hwnd);
      if (pState && !pState->working && mp2) {

	PNOTIFYRECORDEMPHASIS pre = mp2;

	pci = (PCNRITEM) ((pre) ? pre->pRecord : NULL);
	if (pci && (pre->fEmphasisMask & CRA_SELECTED) &&
	    (pci->rc.flRecordAttr & CRA_SELECTED)) {
	  commafmt(szBytes, sizeof(szBytes), pci->attrFile);
	  sprintf(sz,
		  "%s %s%s",
		  szBytes,
		  GetPString(IDS_FILETEXT), &"s"[pci->attrFile == 1]);
	  WinSetDlgItemText(hwnd, DSZ_NUMFILES, sz);
	}
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_DIRSIZE, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DSZ_PRINT:
      // Save button
      pState = INSTDATA(hwnd);
      if (!pState)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {

	CHAR szFileName[CCHMAXPATH];
        FILE *fp;
        CHAR *modea = "a+";

        if (pTmpDir && !IsValidDir(pTmpDir))
          DosCreateDir(pTmpDir, 0);
        //if (pTmpDir)
	//  strcpy(szFileName, pTmpDir);
	else if (!pTmpDir)
	  strcpy(szFileName, pFM2SaveDirectory);
	sprintf(&szFileName[strlen(szFileName)], "%s%csizes.Rpt", PCSZ_BACKSLASH,
		(pState) ? toupper(*pState->szDirName) : '+');
	if (export_filename(hwnd, szFileName, FALSE) && *szFileName) {
	  if (stricmp(szFileName, "PRN") &&
	      strnicmp(szFileName, "\\DEV\\LPT", 8) &&
	      !strchr(szFileName, '.'))
	    strcat(szFileName, ".RPT");
	  fp = xfopen(szFileName, modea, pszSrcFile, __LINE__, TRUE);
	  if (!fp) {
	    saymsg(MB_CANCEL,
		   hwnd,
		   GetPString(IDS_ERRORTEXT),
		   GetPString(IDS_COMPCANTOPENTEXT), szFileName);
	  }
	  else {
	    WinSetPointer(HWND_DESKTOP, hptrBusy);
	    PrintToFile(WinWindowFromID(hwnd, DSZ_CNR), 0, NULL, fp);
	    fclose(fp);
	    WinSetPointer(HWND_DESKTOP, hptrArrow);
	  }
	}
      }
      break;

    case DSZ_EXPAND:
    case DSZ_COLLAPSE:
      pState = INSTDATA(hwnd);
      if (pState) {
	pci = (PCNRITEM) WinSendDlgItemMsg(hwnd, DSZ_CNR,
					   CM_QUERYRECORDEMPHASIS,
					   MPFROMLONG(CMA_FIRST),
					   MPFROMSHORT(CRA_CURSORED));
	if (pci) {
	  WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_HELP), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DSZ_COLLAPSE), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DSZ_EXPAND), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DSZ_PRINT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DID_CANCEL), FALSE);
          if (SHORT1FROMMP(mp1) == DSZ_EXPAND) {
            EXPANDSIZE *expandsize;

            expandsize = xmalloc(sizeof(EXPANDSIZE), pszSrcFile, __LINE__);
            if (expandsize) {

            expandsize->pci = pci;
            expandsize->hwndCnr = WinWindowFromID(hwnd, DSZ_CNR);
            if (xbeginthread(ExpandCnrThread,
                             122880 * 5,
                             expandsize,
                             pszSrcFile,
                             __LINE__) == -1)
              xfree(expandsize, pszSrcFile, __LINE__);
            }
          }
          else
            CollapseAll(WinWindowFromID(hwnd, DSZ_CNR), pci);
	  WinEnableWindow(WinWindowFromID(hwnd, DID_OK), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_HELP), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, DSZ_COLLAPSE), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, DSZ_EXPAND), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, DSZ_PRINT), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, DID_CANCEL), TRUE);
	}
      }
      break;

    case DID_OK:
    case DID_CANCEL:
      {
	SWP swp;
	ULONG size = sizeof(SWP);

	WinQueryWindowPos(hwnd, &swp);
	PrfWriteProfileData(fmprof, FM3Str, "DirSizes.Position", (PVOID) &swp,	size);
      }
      pState = INSTDATA(hwnd);
      if (!pState)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	if (pState->working) {
	  pState->dying = TRUE;
	  pState->chStopFlag = (CHAR)0xff;
          if (!fAlertBeepOff)
	    DosBeep(1000, 100);		// Complain?
	}
	else
	  WinDismissDlg(hwnd, 0);
      }
      break;
    }					// switch mp1
    return 0;

  case WM_CLOSE:
    pState = INSTDATA(hwnd);
    if (pState)
      pState->chStopFlag = (CHAR)0xff;
    DosSleep(1);
    break;

  case WM_DESTROY:
    pState = INSTDATA(hwnd);
    EmptyCnr(hwnd);
    if (pState) {
      pState->chStopFlag = (CHAR)0xff;
      if (pState->hptr)
	WinDestroyPointer(pState->hptr);
      DosSleep(16);
      xfree(pState, pszSrcFile, __LINE__); // Let's hope no one is still looking
#     ifdef FORTIFY
      Fortify_LeaveScope();
#      endif
    }
    DosPostEventSem(CompactSem);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(DIRSIZE,ProcessDir,FillCnrThread,DirSizeProc)
#pragma alloc_text(DIRSIZE2,PrintToFile,FillInRecSizes,SortSizeCnr)

