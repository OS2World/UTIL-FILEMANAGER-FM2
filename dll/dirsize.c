
/***********************************************************************

  $Id$

  Directory sizes

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2007 Steven H. Levine

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

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <process.h>			// _beginthread

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG

#include "fm3dlg.h"
#include "fm3str.h"
#include "dircnrs.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "fm3dll.h"

#include "fortify.h"

typedef struct
{
  CHAR *pszFileName;
  HWND hwndCnr;
  CHAR *pchStopFlag;
  DIRCNRDATA *pDCD;
} DIRSIZE;

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
  /*if (!stricmp(FileSystem, NTFS)) {
    saymsg(MB_OK,
           HWND_DESKTOP,
           NullStr,
           GetPString(IDS_NTFSDRIVERFAILSTEXT));
    return FALSE;
  } */
  ulBufBytes = sizeof(FILEFINDBUF4L) * FilesToGet;
  pffbArray = xmalloc(ulBufBytes, pszSrcFile, __LINE__);
  if (!pffbArray)
    return FALSE;			// Error already reported

  strcpy(maskstr, pszFileName);
  if (maskstr[strlen(maskstr) - 1] != '\\')
    strcat(maskstr, "\\");
  pEndMask = &maskstr[strlen(maskstr)];	// Point after last backslash
  strcat(maskstr, "*");

  hdir = HDIR_CREATE;
  ulFindCnt = 1;
  // memset(pffbArray, 0, sizeof(FILEFINDBUF4L));	// 11 Aug 07 SHL bypass memset
  DosError(FERR_DISABLEHARDERR);
  // Check directory exists
  rc = xDosFindFirst(pszFileName, &hdir,
		     FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
		     FILE_SYSTEM | FILE_HIDDEN | MUST_HAVE_DIRECTORY,
		     pffbArray, ulBufBytes, &ulFindCnt, FIL_QUERYEASIZEL);

  if (!rc)
    DosFindClose(hdir);

  /*
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
      Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__, "CM_ALLOCRECORD");
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
    pci->pszDispAttr = NullStr;
    pci->pszSubject = NullStr;
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
  // fixme to understand this - appears to be indirectly saving length, but why?
  pci->pszDisplayName = pci->pszFileName + strlen(pci->pszFileName);
  pci->pszLongName = pci->pszFileName;	// fixme to be sure?
  pci->rc.pszIcon = pci->pszFileName;
  pci->rc.flRecordAttr |= CRA_RECORDREADONLY;
  if (fForceUpper)
    strupr(pci->pszFileName);
  else if (fForceLower)
    strlwr(pci->pszFileName);
  pci->pszDisplayName = pci->pszFileName + strlen(pci->pszFileName);
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

    float fltPct = 0.0;
    USHORT c;
    CHAR szCurDir[80];
    CHAR szSubDir[80];
    CHAR szAllDir[80];
    CHAR szBar[80];

    // cbFile = currect directory usage in bytes
    // easize = subdirectory usage in bytes
    CommaFmtULL(szCurDir, sizeof(szCurDir), pci->cbFile, 'K');
    *szBar = 0;

    if (ullTotalBytes) {
      register UINT cBar;

      if (isroot) {
	FSALLOCATE fsa;
	APIRET rc;

	memset(&fsa, 0, sizeof(fsa));
	rc = DosQueryFSInfo(toupper(*pci->pszFileName) - '@', FSIL_ALLOC, &fsa,
			    sizeof(FSALLOCATE));
	if (!rc) {
	  fltPct = (ullTotalBytes * 100.0) /
	    ((float)fsa.cUnit * (fsa.cSectorUnit * fsa.cbSector));
	}
	// Need unique buffer 23 Jul 07 SHL
        pci->pszLongName = xmalloc(3, pszSrcFile, __LINE__);
        if (pci->pszLongName) {
          pci->pszLongName[0] = 0;	// Make null string
          pci->pszLongName[1] = 1;	// Flag root - hack cough
          pci->pszLongName[2] = 0;        // terminate anyway
        }
      }
      else
	fltPct = (((float)pci->cbFile + pci->easize) * 100.0) / ullTotalBytes;

      cBar = (UINT) fltPct / 2;
      if (cBar)
	memset(szBar, '#', cBar);
      if (cBar * 2 != (UINT) fltPct) {
	szBar[cBar] = '=';
	cBar++;
      }
      if (cBar < 50)
	memset(szBar + cBar, ' ', 50 - cBar);
      szBar[50] = 0;
    }

    pci->flags = (ULONG) fltPct;
    CommaFmtULL(szSubDir, sizeof(szSubDir), pci->easize, 'K');
    CommaFmtULL(szAllDir, sizeof(szAllDir), pci->cbFile + pci->easize, 'K');
    c = pci->pszDisplayName - pci->pszFileName;
    pci->pszFileName = xrealloc(pci->pszFileName,
				CCHMAXPATH,
				pszSrcFile,
				__LINE__);	// 23 Jul 07 SHL
    sprintf(pci->pszFileName + c,
	    "  %s + %s = %s (%.02lf%%%s)\r%s",
	    szCurDir,
	    szSubDir,
	    szAllDir,
	    fltPct,
	    isroot ? GetPString(IDS_OFDRIVETEXT) : NullStr,
	    szBar);
    pci->pszFileName = xrealloc(pci->pszFileName,
				strlen(pci->pszFileName) + 1,
				pszSrcFile,
				__LINE__);	// 23 Jul 07 SHL
    pci->pszDisplayName = pci->pszFileName + c;
    WinSendMsg(hwndCnr,
	       CM_INVALIDATERECORD, MPFROMP(&pci), MPFROM2SHORT(1, 0));
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
    p = strchr(pciParent->pszFileName, '\r');
    if (p)
      *p = 0;
    fprintf(fp, "%*.*s%s %lu %s%s\n",
	    indent * 2, indent * 2, " ",
	    pciParent->pszFileName,
	    pciParent->attrFile,
	    GetPString(IDS_FILETEXT), &"s"[pciParent->attrFile == 1]);
    if (p)
      *p = '\r';
    if (pciParent->rc.flRecordAttr & CRA_EXPANDED) {
      pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pciParent),
				  MPFROM2SHORT(CMA_FIRSTCHILD,
					       CMA_ITEMORDER));
      while (pci && (INT) pci != -1) {
	DosSleep(0); //26 Aug 07 GKY 1
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
    Runtime_Error(pszSrcFile, __LINE__, "no data");
    return;
  }
# ifdef FORTIFY
  Fortify_EnterScope();
# endif

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
# endif
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
# ifdef FORTIFY
  Fortify_EnterScope();
# endif
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
      RestorePresParams(hwnd, "DirSizes");
      sprintf(s, GetPString(IDS_DIRSIZETITLETEXT), pState->szDirName);
      WinSetWindowText(hwnd, s);
    }
    {
      SWP swp;
      ULONG size = sizeof(SWP);

      PrfQueryProfileData(fmprof, FM3Str, "DirSizes.Position", (PVOID) &swp, &size);
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
      if (_beginthread(FillCnrThread, NULL, 122880L * 5, (PVOID)dirsize) ==
	  -1) {
	Runtime_Error(pszSrcFile, __LINE__,
		      GetPString(IDS_COULDNTSTARTTHREADTEXT));
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
	  WinSetDlgItemText(hwnd,
			    DSZ_FREESPACE, GetPString(IDS_FREESPACEUTEXT));
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
    PresParamChanged(hwnd, "DirSizes", mp1, mp2);
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
	    p = strchr(pci->pszFileName, '\r');
	    if (p) {
              // draw text
              if (*(pci->pszLongName + 1) == 1)  // is root record
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
	      GpiQueryTextBox(oi->hps, strlen(pci->pszFileName),
			      pci->pszFileName, TXTBOX_COUNT, aptl);
	      boxHeight = aptl[TXTBOX_TOPRIGHT].y - aptl[TXTBOX_BOTTOMRIGHT].y;
	      boxHeight -= 4;

	      // Calculate nominal baseline of graph box
	      // rclItem.yBottom is at center of icon because it is
	      yBottom = oi->rclItem.yBottom - boxHeight + 3;

	      // Place text above graph box with a bit of whitespace between
	      ptl.x = oi->rclItem.xLeft;
	      ptl.y = yBottom + boxHeight + 6;	// 03 Aug 07 SHL
	      // GpiMove(oi->hps, &ptl);
	      GpiCharStringAt(oi->hps, &ptl, strlen(pci->pszFileName),
			      pci->pszFileName);

	      *p = '\r';		// Restore

	      // draw the graph box
	      // GpiQueryTextBox(oi->hps, 1, "#", TXTBOX_COUNT, aptl);	// 03 Aug 07 SHL
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
	      ptl.x = oi->rclItem.xLeft + 203;
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
		if (*(pci->pszLongName + 1) == 1)	// is root record
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
		if (*(pci->pszLongName + 1) == 1)
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
		if (*(pci->pszLongName + 1) != 1) {
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
	    strncpy(szTemp, pci->pszFileName,
		    pci->pszDisplayName - pci->pszFileName);
	    strrev(szTemp);
	    if (*szFileName && *szTemp != '\\')
	      strcat(szFileName, "\\");
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

	    strcpy(s, "ICON");
	    PrfQueryProfileData(fmprof, appname, "DirflWindowAttr",
				(PVOID) & flWindowAttr, &size);
	    if (flWindowAttr & CV_DETAIL) {
	      if (IsRoot(szFileName))
		strcpy(s, "TREE");
	      else
		strcpy(s, "DETAILS");
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
	Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      else {

	CHAR szFileName[CCHMAXPATH];
	FILE *fp;

	save_dir2(szFileName);
	sprintf(&szFileName[strlen(szFileName)], "\\%csizes.Rpt",
		(pState) ? toupper(*pState->szDirName) : '+');
	if (export_filename(hwnd, szFileName, FALSE) && *szFileName) {
	  if (stricmp(szFileName, "PRN") &&
	      strnicmp(szFileName, "\\DEV\\LPT", 8) &&
	      !strchr(szFileName, '.'))
	    strcat(szFileName, ".RPT");
	  fp = fopen(szFileName, "a+");
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
	  // fixme to use thread - too slow on large trees
	  ExpandAll(WinWindowFromID(hwnd, DSZ_CNR),
		    (SHORT1FROMMP(mp1) == DSZ_EXPAND), pci);
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
        PrfWriteProfileData(fmprof, FM3Str, "DirSizes.Position", (PVOID) &swp,
                            size);
      }
      pState = INSTDATA(hwnd);
      if (!pState)
	Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      else {
	if (pState->working) {
	  pState->dying = TRUE;
	  pState->chStopFlag = (BYTE)0xff;
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
      pState->chStopFlag = (BYTE)0xff;
    DosSleep(1);
    break;

  case WM_DESTROY:
    pState = INSTDATA(hwnd);
    if (pState) {
      pState->chStopFlag = (BYTE)0xff;
      if (pState->hptr)
	WinDestroyPointer(pState->hptr);
      DosSleep(16); //05 Aug 07 GKY 33
      xfree(pState, pszSrcFile, __LINE__); // Let's hope no one is still looking
# ifdef FORTIFY
  Fortify_LeaveScope();
# endif
    }
    DosPostEventSem(CompactSem);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(DIRSIZE,ProcessDir,FillCnrThread,DirSizeProc)
#pragma alloc_text(DIRSIZE2,PrintToFile,FillInRecSizes,SortSizeCnr)

