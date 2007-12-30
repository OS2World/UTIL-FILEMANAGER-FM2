
/***********************************************************************

  $Id$

  Compare directories

  Copyright (c) 1993-02 M. Kimes
  Copyright (c) 2003, 2007 Steven H. Levine

  16 Oct 02 MK Baseline
  04 Nov 03 SHL Force window refresh after subdir toggle
  01 Aug 04 SHL Rework lstrip/rstrip usage
  24 May 05 SHL Rework Win_Error usage
  24 May 05 SHL Rework for CNRITEM.szSubject
  25 May 05 SHL Rework with ULONGLONG
  06 Jun 05 SHL Drop unused
  12 Jul 06 SHL Renames and comments
  13 Jul 06 SHL Use Runtime_Error
  26 Jul 06 SHL Drop unreachable CN_... code
  29 Jul 06 SHL Use xfgets_bstripcr
  15 Aug 06 SHL Turn off hide not selected on dir change
  19 Oct 06 SHL Correct . and .. detect
  03 Nov 06 SHL Count thread usage
  22 Mar 07 GKY Use QWL_USER
  29 Jul 07 SHL Use Win_Error to report container errors
  01 Aug 07 SHL Rework to sync with CNRITEM mods
  01 Aug 07 SHL Rework to remove vast amount of duplicate code
  03 Aug 07 GKY Enlarged and made setable everywhere Findbuf (speed file loading)
  06 Aug 07 SHL Move BldFullPathName here to be near primary caller
  07 Aug 07 SHL COMP_COLLECT: Avoid collecting empty entries when nothing selected
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  13 Aug 07 SHL Sync code with other FilesToGet usage
  13 Aug 07 SHL Move #pragma alloc_text to end for OpenWatcom compat
  20 Aug 07 SHL Correct remaining pcil/pcir typos (we hope)
  20 Aug 07 SHL Revert to DosSleep(0)
  20 Aug 07 SHL Use GetMSecTimer for timing
  20 Aug 07 SHL A few more speed up tweaks.  Some experimental timing code
  26 Aug 07 GKY DosSleep(1) in loops changed to (0)
  27 Sep 07 SHL Correct ULONGLONG size formatting
  30 Dec 07 GKY Use TestCDates for compare by file date/time

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSERRORS
#define INCL_GPI
#define INCL_LONGLONG
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <io.h>
#include <process.h>			// _beginthread

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

typedef struct
{
  CHAR filename[CCHMAXPATH];
  CHAR dirname[CCHMAXPATH];
  BOOL recurse;
}
SNAPSTUFF;

static PSZ pszSrcFile = __FILE__;

/**
 * Build full path name in callers buffer given directory
 * name and filename
 * @param pszPathName points to drive/directory if not NULL
 * @returns pointer to full path name in caller's buffer
 * @note OK for pszFullPathName and pszPathName to point to same buffer
 *
 */

PSZ BldFullPathName(PSZ pszFullPathName, PSZ pszPathName, PSZ pszFileName)
{
  UINT c = pszPathName ? strlen(pszPathName) : 0;
  if (c > 0) {
    memcpy(pszFullPathName, pszPathName, c);
    if (pszFullPathName[c - 1] != '\\')
      pszFullPathName[c++] = '\\';
  }
  strcpy(pszFullPathName + c, pszFileName);
  return pszFullPathName;
}

//=== SnapShot() Write directory tree to file and recurse if requested ===

static VOID SnapShot(char *path, FILE *fp, BOOL recurse)
{
  PFILEFINDBUF4L pffb;
  char *mask, *enddir;
  HDIR hdir = HDIR_CREATE;
  ULONG ulFindCnt;

  // 13 Aug 07 SHL fimxe to use FileToGet
  pffb = xmalloc(sizeof(FILEFINDBUF4L), pszSrcFile, __LINE__);
  if (pffb) {
    mask = xmalloc(CCHMAXPATH, pszSrcFile, __LINE__);
    if (mask) {
      BldFullPathName(mask, path, "*");
      // sprintf(mask,
      //	 "%s%s*",
      //	 path, (path[strlen(path) - 1] != '\\') ? "\\" : NullStr);
      enddir = strrchr(mask, '\\');
      enddir++;
      ulFindCnt = 1;
      // 13 Aug 07 SHL fixme to report errors
      if (!xDosFindFirst(mask,
	         	 &hdir,
			 FILE_NORMAL | FILE_DIRECTORY |
			 FILE_ARCHIVED | FILE_READONLY | FILE_HIDDEN |
			 FILE_SYSTEM,
			 pffb, sizeof(FILEFINDBUF4L), &ulFindCnt, FIL_QUERYEASIZEL)) {
	do {
	  strcpy(enddir, pffb->achName);
	  if (!(pffb->attrFile & FILE_DIRECTORY))
	    // 27 Sep 07 SHL fixme to use CommaFmtULL
	    fprintf(fp,
		    "\"%s\",%u,%llu,%04u/%02u/%02u,%02u:%02u:%02u,%lu,%lu,N\n",
		    mask,
		    enddir - mask,
		    pffb->cbFile,
		    (pffb->fdateLastWrite.year + 1980),
		    pffb->fdateLastWrite.month,
		    pffb->fdateLastWrite.day,
		    pffb->ftimeLastWrite.hours,
		    pffb->ftimeLastWrite.minutes,
		    pffb->ftimeLastWrite.twosecs,
		    pffb->attrFile,
		    pffb->cbList > 4 ? pffb->cbList / 2 : 0);
	  // Skip . and ..
	  else if (recurse &&
		   (pffb->achName[0] != '.' ||
		    (pffb->achName[1] &&
		     (pffb->achName[1] != '.' || pffb->achName[2])))) {
	    SnapShot(mask, fp, recurse);
	  }
	  ulFindCnt = 1;
	} while (!xDosFindNext(hdir, pffb, sizeof(FILEFINDBUF4L), &ulFindCnt, FIL_QUERYEASIZEL));
	DosFindClose(hdir);
      }
      free(mask);
    }
    free(pffb);
  }
}

//=== StartSnap() Write directory tree to snapshot file ===

static VOID StartSnap(VOID * dummy)
{
  SNAPSTUFF *sf = (SNAPSTUFF *) dummy;
  FILE *fp;
  CHAR *p;

  if (sf) {
    if (*sf->dirname && *sf->filename) {
      priority_normal();
      p = sf->dirname;
      while (*p) {
	if (*p == '/')
	  *p = '\\';
	p++;
      }
      if (*(p - 1) != '\\') {
	*p = '\\';
	p++;
      }
      fp = xfopen(sf->filename, "w", pszSrcFile, __LINE__);
      if (fp) {
	fprintf(fp, "\"%s\"\n", sf->dirname);
	SnapShot(sf->dirname, fp, sf->recurse);
	fclose(fp);
      }
    }
    free(sf);
  }
}

//=== CompareFilesThread() Compare files and update container select flags ===

static VOID CompareFilesThread(VOID * args)
{
  FCOMPARE fc;
  HAB hab2;
  HMQ hmq2;
  FILE *fp1, *fp2;
  ULONG len1, len2;
  ULONG offset = 0;
  LONG numread1, numread2;
  CHAR s[1024], ss[1024], *p1, *p2;

  if (args) {
    fc = *(FCOMPARE *) args;
    hab2 = WinInitialize(0);
    if (hab2) {
      hmq2 = WinCreateMsgQueue(hab2, 0);
      if (hmq2) {
	WinCancelShutdown(hmq2, TRUE);
	IncrThreadUsage();
	if (!IsFile(fc.file1) || IsRoot(fc.file1)) {
	  p1 = strrchr(fc.file2, '\\');
	  if (p1) {
	    if (fc.file1[strlen(fc.file1) - 1] == '\\')
	      p1++;
	    strcat(fc.file1, p1);
	  }
	}
	else if (!IsFile(fc.file2) || IsRoot(fc.file2)) {
	  p1 = strrchr(fc.file1, '\\');
	  if (p1) {
	    if (fc.file2[strlen(fc.file2) - 1] == '\\')
	      p1++;
	    strcat(fc.file2, p1);
	  }
	}
	sprintf(s, GetPString(IDS_COMPCOMPARETEXT), fc.file1);
	AddToListboxBottom(fc.hwndList, s);
	sprintf(s, GetPString(IDS_COMPTOTEXT), fc.file2);
	AddToListboxBottom(fc.hwndList, s);
	fp1 = _fsopen(fc.file1, "rb", SH_DENYNO);
	if (!fp1) {
	  sprintf(s, GetPString(IDS_COMPCANTOPENTEXT), fc.file1);
	  AddToListboxBottom(fc.hwndList, s);
	  WinSetWindowText(fc.hwndHelp, GetPString(IDS_ERRORTEXT));
	}
	else {
	  fp2 = _fsopen(fc.file2, "rb", SH_DENYNO);
	  if (!fp2) {
	    sprintf(s, GetPString(IDS_COMPCANTOPENTEXT), fc.file2);
	    AddToListboxBottom(fc.hwndList, s);
	    WinSetWindowText(fc.hwndHelp, GetPString(IDS_ERRORTEXT));
	  }
	  else {
	    len1 = filelength(fileno(fp1));
	    len2 = filelength(fileno(fp2));
	    if (len1 != len2) {
	      strcpy(s, GetPString(IDS_COMPDIFSIZESTEXT));
	      AddToListboxBottom(fc.hwndList, s);
	      sprintf(s, GetPString(IDS_COMPVSBYTESTEXT), len1, len2);
	      AddToListboxBottom(fc.hwndList, s);
	      WinSetWindowText(fc.hwndHelp,
			       GetPString(IDS_COMPDONTMATCHTEXT));
	    }
	    else {
	      WinSetWindowText(fc.hwndHelp,
			       GetPString(IDS_COMPCOMPARINGTEXT));
	      while (WinIsWindow(hab2, fc.hwndList)) {
		numread1 = fread(s, 1, 1024, fp1);
		numread2 = fread(ss, 1, 1024, fp2);
		if (numread1 != numread2 || feof(fp1) != feof(fp2)) {
		  sprintf(s, GetPString(IDS_COMPREADERRORTEXT),
			  offset, offset);
		  AddToListboxBottom(fc.hwndList, s);
		  WinSetWindowText(fc.hwndHelp, GetPString(IDS_ERRORTEXT));
		  break;
		}
		else if (!numread1 && feof(fp1) && feof(fp2)) {
		  AddToListboxBottom(fc.hwndList,
				     GetPString(IDS_COMPFILESMATCHTEXT));
		  if (!stricmp(fc.file1, fc.file2))
		    AddToListboxBottom(fc.hwndList,
				       GetPString(IDS_COMPWONDERWHYTEXT));
		  WinSetWindowText(fc.hwndHelp,
				   GetPString(IDS_COMPCOMPLETETEXT));
		  break;
		}
		else if (numread1 <= 0 || numread2 <= 0) {
		  if (offset == len1)
		    break;
		  else {
		    sprintf(s, GetPString(IDS_COMPMATCHREADERRORTEXT),
			    offset, offset);
		    WinSetWindowText(fc.hwndHelp,
				     GetPString(IDS_COMPODDERRORTEXT));
		    AddToListboxBottom(fc.hwndList, s);
		    break;
		  }
		}
		else if (memcmp(s, ss, numread1)) {
		  p1 = s;
		  p2 = ss;
		  while (p1 < s + numread1) {
		    if (*p1 != *p2) {
		      sprintf(s, GetPString(IDS_COMPMISMATCHERRORTEXT),
			      offset + (p1 - s), offset + (p1 - s));
		      AddToListboxBottom(fc.hwndList, s);
		      WinSetWindowText(fc.hwndHelp,
				       GetPString(IDS_COMPDONTMATCHTEXT));
		      break;
		    }
		    p1++;
		    p2++;
		  }
		  break;
		}
		offset += numread1;
	      }
	    }
	    fclose(fp2);
	  }
	  fclose(fp1);
	}
	DecrThreadUsage();
	WinDestroyMsgQueue(hmq2);
      }
      WinTerminate(hab2);
    }
  }
}

//=== CFileDlgProc() Select directories to compare dialog procedure ===

MRESULT EXPENTRY CFileDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  FCOMPARE *fc;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2)
      WinDismissDlg(hwnd, 0);
    else {
      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      fc = (FCOMPARE *) mp2;
      fc->hwndReport = hwnd;
      fc->hwndList = WinWindowFromID(hwnd, FCMP_LISTBOX);
      fc->hwndHelp = WinWindowFromID(hwnd, FCMP_HELP);
      if (!*fc->file1 || !fc->file2) {
	WinDismissDlg(hwnd, 0);
	break;
      }
      MakeFullName(fc->file1);
      MakeFullName(fc->file2);
      if (!stricmp(fc->file1, fc->file2)) {
	saymsg(MB_CANCEL, hwnd,
	       GetPString(IDS_COMPSILLYALERTTEXT),
	       GetPString(IDS_COMPTOITSELFTEXT));
	WinDismissDlg(hwnd, 0);
	break;
      }
      if (_beginthread(CompareFilesThread, NULL, 65536, (PVOID) fc) == -1) {
	Runtime_Error(pszSrcFile, __LINE__,
		      GetPString(IDS_COULDNTSTARTTHREADTEXT));
	WinDismissDlg(hwnd, 0);
      }
    }
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, FCMP_HELP),
			(HPS) 0, FALSE, TRUE);
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      WinDismissDlg(hwnd, 0);
      break;
    case DID_CANCEL:
      WinDismissDlg(hwnd, 1);
      break;
    }
    return 0;

  case WM_DESTROY:
    DosSleep(50);			// 05 Aug 07 GKY 100
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

//=== ActionCnrThread() Do requested action on container contents ===

static VOID ActionCnrThread(VOID *args)
{
  COMPARE *cmp = (COMPARE *)args;
  HAB hab;
  HMQ hmq;
  HWND hwndCnrS, hwndCnrD;
  PCNRITEM pci, pciD, pciNextS, pciNextD;
  CHAR szNewName[CCHMAXPATH], szDirName[CCHMAXPATH], *p;
  APIRET rc;

  if (!cmp) {
    Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
    return;
  }

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 0);
    if (hmq) {
      WinCancelShutdown(hmq, TRUE);
      IncrThreadUsage();
      priority_normal();
      switch (cmp->action) {
      case COMP_DELETELEFT:
	hwndCnrS = WinWindowFromID(cmp->hwnd, COMP_LEFTDIR);
	hwndCnrD = WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR);
	cmp->action = IDM_DELETE;
	break;
      case COMP_DELETERIGHT:
	hwndCnrS = WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR);
	hwndCnrD = WinWindowFromID(cmp->hwnd, COMP_LEFTDIR);
	cmp->action = IDM_DELETE;
	break;
      case COMP_MOVELEFT:
	cmp->action = IDM_MOVE;
	hwndCnrS = WinWindowFromID(cmp->hwnd, COMP_LEFTDIR);
	hwndCnrD = WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR);
	break;
      case COMP_MOVERIGHT:
	cmp->action = IDM_MOVE;
	hwndCnrS = WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR);
	hwndCnrD = WinWindowFromID(cmp->hwnd, COMP_LEFTDIR);
	break;
      case COMP_COPYLEFT:
	cmp->action = IDM_COPY;
	hwndCnrS = WinWindowFromID(cmp->hwnd, COMP_LEFTDIR);
	hwndCnrD = WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR);
	break;
      case COMP_COPYRIGHT:
	cmp->action = IDM_COPY;
	hwndCnrS = WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR);
	hwndCnrD = WinWindowFromID(cmp->hwnd, COMP_LEFTDIR);
	break;
      default:
	Runtime_Error(pszSrcFile, __LINE__, "bad case %u", cmp->action);
	goto Abort;
      }

      pci = WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPVOID,
		       MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
      pciD = WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPVOID,
			MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));

      while (pci && (INT)pci != -1 && pciD && (INT)pciD != -1) {

	pciNextS = WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPFROMP(pci),
			  MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
	pciNextD = WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPFROMP(pciD),
			   MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));

	if (*pci->pszFileName && pci->rc.flRecordAttr & CRA_SELECTED) {

	  // Source name not blank
	  switch (cmp->action) {
	  case IDM_DELETE:
	    if (!unlinkf("%s", pci->pszFileName)) {
	      WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pci),
			 MPFROM2SHORT(FALSE, CRA_SELECTED));

	      if (!*pciD->pszFileName) {
		// Other side is blank - remove from both sides
		RemoveCnrItems(hwndCnrS, pci, 1, CMA_FREE | CMA_INVALIDATE);
		if (pciD->rc.flRecordAttr & CRA_SELECTED)
		  WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciD),
			     MPFROM2SHORT(FALSE, CRA_SELECTED));
		RemoveCnrItems(hwndCnrD, pciD, 1, CMA_FREE | CMA_INVALIDATE);
	      }
	      else {
		// Other side is not blank - update just this side
		FreeCnrItemData(pci);
		pci->pszDisplayName = pci->pszFileName;
		pci->rc.pszIcon = pci->pszFileName;
		pci->flags = 0;
		WinSendMsg(hwndCnrS, CM_INVALIDATERECORD, MPFROMP(&pci),
			   MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
	      }
	      if (hwndCnrS == WinWindowFromID(cmp->hwnd, COMP_LEFTDIR))
		cmp->cmp->totalleft--;
	      else
		cmp->cmp->totalright--;
	      DosSleep(0);		// 8-26-07 GKY 1
	    }
	    break;

	  case IDM_MOVE:
	    if (hwndCnrS == WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR))
	      BldFullPathName(szNewName, cmp->leftdir, pci->pszDisplayName);
	      //sprintf(szNewName, "%s%s%s",
	      //        cmp->leftdir,
	      //        cmp->leftdir[strlen(cmp->leftdir) - 1] == '\\' ?
	      //	NullStr : "\\",
	      //	pci->pszDisplayName);
	    else
	      BldFullPathName(szNewName, cmp->rightdir, pci->pszDisplayName);
	      //sprintf(szNewName, "%s%s%s",
	      //        cmp->rightdir,
	     //         cmp->rightdir[strlen(cmp->rightdir) - 1] == '\\' ?
	     //	NullStr : "\\",
	     //         pci->pszDisplayName);
	    // Make directory if required
	    strcpy(szDirName, szNewName);
	    p = strrchr(szDirName, '\\');
	    if (p) {
	      if (p > szDirName + 2)
		p++;
	      *p = 0;
	      if (IsFile(szDirName) == -1)
		MassMkdir(hwndMain, szDirName);
	    }
	    rc = docopyf(MOVE, pci->pszFileName, "%s", szNewName);
	    if (!rc && stricmp(pci->pszFileName, szNewName)) {
	      WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pci),
			 MPFROM2SHORT(FALSE, CRA_SELECTED));
	      if (pciD->rc.flRecordAttr & CRA_SELECTED)
		WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciD),
			   MPFROM2SHORT(FALSE, CRA_SELECTED));
	      FreeCnrItemData(pciD);
	      pciD->pszFileName = xstrdup(szNewName, pszSrcFile, __LINE__);
	      if (hwndCnrS == WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR)) {
		pciD->pszDisplayName = pciD->pszFileName + strlen(cmp->leftdir);
		if (cmp->leftdir[strlen(cmp->leftdir) - 1] != '\\')
		  pciD->pszDisplayName++;
	      }
	      else {
		pciD->pszDisplayName = pciD->pszFileName + strlen(cmp->rightdir);
		if (cmp->rightdir[strlen(cmp->rightdir) - 1] != '\\')
		  pciD->pszDisplayName++;
	      }
	      // 02 Aug 07 SHL fixme to know if LongName transfer is correct?
	      pciD->pszLongName = pci->pszLongName;
	      if (pciD->pszSubject != NullStr) {
		xfree(pciD->pszSubject);
		pciD->pszSubject = NullStr;
	      }
	      pciD->attrFile = pci->attrFile;
	      pciD->pszDispAttr = pci->pszDispAttr;
	      pciD->flags = 0;		// Just on one side
	      pciD->date = pci->date;
	      pciD->time = pci->time;
	      pciD->ladate = pci->ladate;
	      pciD->latime = pci->latime;
	      pciD->crdate = pci->crdate;
	      pciD->crtime = pci->crtime;
	      pciD->cbFile = pci->cbFile;
	      pciD->easize = pci->easize;

	      if (pci->pszFileName != NullStr) {
		xfree(pci->pszFileName);
		pci->pszFileName = NullStr;
		pci->pszDisplayName = pci->pszFileName;
		pci->rc.pszIcon = pci->pszFileName;
	      }
	      if (pci->pszSubject != NullStr) {
		xfree(pci->pszSubject);
		pci->pszSubject = NullStr;
	      }
	      pci->flags = 0;

	      WinSendMsg(hwndCnrS, CM_INVALIDATERECORD, MPFROMP(&pci),
			 MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
	      WinSendMsg(hwndCnrD, CM_INVALIDATERECORD, MPFROMP(&pciD),
			 MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
	    }
	    else if (rc) {
	      rc = Dos_Error(MB_ENTERCANCEL,
			     rc,
			     HWND_DESKTOP,
			     pszSrcFile,
			     __LINE__,
			     GetPString(IDS_COMPMOVEFAILEDTEXT),
			     pci->pszFileName, szNewName);
	      if (rc == MBID_CANCEL)	// Cause loop to break
		pciNextS = NULL;
	    }
	    break;

	  case IDM_COPY:
	    if (hwndCnrS == WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR))
	      BldFullPathName(szNewName, cmp->leftdir, pci->pszDisplayName);
	      //sprintf(szNewName, "%s%s%s",
	      //        cmp->leftdir,
	      //        cmp->leftdir[strlen(cmp->leftdir) - 1] == '\\' ?
	      //	NullStr : "\\",
	      //         pci->pszDisplayName);
	    else
	      BldFullPathName(szNewName, cmp->rightdir, pci->pszDisplayName);
	      //sprintf(szNewName, "%s%s%s",
	      //        cmp->rightdir,
	      //        cmp->rightdir[strlen(cmp->rightdir) - 1] == '\\' ?
	      //	NullStr : "\\",
	      //        pci->pszDisplayName);
	    // Make directory if required
	    strcpy(szDirName, szNewName);
	    p = strrchr(szDirName, '\\');
	    if (p) {
	      if (p > szDirName + 2)
		p++;
	      *p = 0;
	      if (IsFile(szDirName) == -1)
		MassMkdir(hwndMain, szDirName);
	    }
	    rc = docopyf(COPY, pci->pszFileName, "%s", szNewName);
	    if (rc) {
	      rc = Dos_Error(MB_ENTERCANCEL,
			     rc,
			     HWND_DESKTOP,
			     pszSrcFile,
			     __LINE__,
			     GetPString(IDS_COMPCOPYFAILEDTEXT),
			     pci->pszFileName, szNewName);
	      if (rc == MBID_CANCEL)
		pciNextS = NULL;		// Cause loop to break
	    }
	    else {
	      WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pci),
			 MPFROM2SHORT(FALSE, CRA_SELECTED));
	      if (pciD->rc.flRecordAttr & CRA_SELECTED)
		WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciD),
			   MPFROM2SHORT(FALSE, CRA_SELECTED));
	      FreeCnrItemData(pciD);
	      pciD->pszFileName = xstrdup(szNewName, pszSrcFile, __LINE__);
	      if (hwndCnrS == WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR)) {
		pciD->pszDisplayName = pciD->pszFileName + strlen(cmp->leftdir);
		if (cmp->leftdir[strlen(cmp->leftdir) - 1] != '\\')
		  pciD->pszDisplayName++;
	      }
	      else {
		pciD->pszDisplayName = pciD->pszFileName + strlen(cmp->rightdir);
		if (cmp->rightdir[strlen(cmp->rightdir) - 1] != '\\')
		  pciD->pszDisplayName++;
	      }
	      pciD->attrFile = pci->attrFile;
	      pciD->pszDispAttr = pci->pszDispAttr;
	      pciD->flags = CNRITEM_EXISTS;	// Now on both sides
	      pciD->date = pci->date;
	      pciD->time = pci->time;
	      pciD->ladate = pci->ladate;
	      pciD->latime = pci->latime;
	      pciD->crdate = pci->crdate;
	      pciD->crtime = pci->crtime;
	      pciD->cbFile = pci->cbFile;
	      pciD->easize = pci->easize;

	      // Forget status until we regenerate it
	      if (pci->pszSubject != NullStr) {
		xfree(pci->pszSubject);
		pci->pszSubject = NullStr;
	      }
	      pci->flags = CNRITEM_EXISTS;

	      WinSendMsg(hwndCnrS, CM_INVALIDATERECORD, MPFROMP(&pci),
			 MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
	      WinSendMsg(hwndCnrD, CM_INVALIDATERECORD, MPFROMP(&pciD),
			 MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
	    }
	    break;

	  default:
	    break;
	  } // switch

	} // if have name

	pci = pciNextS;
	pciD = pciNextD;

      }	// while
    Abort:
      WinDestroyMsgQueue(hmq);
    }
    DecrThreadUsage();
    WinTerminate(hab);
  }
  PostMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPFROMLONG(1L), MPVOID);
  PostMsg(cmp->hwnd, WM_COMMAND, MPFROM2SHORT(IDM_DESELECTALL, 0), MPVOID);
  free(cmp);
}

//=== SelectCnrsThread() Update container selection flags thread ===

static VOID SelectCnrsThread(VOID * args)
{
  COMPARE *cmp = (COMPARE *) args;
  HAB hab;
  HMQ hmq;

  if (!cmp) {
    Runtime_Error(pszSrcFile, __LINE__, "no data");
    return;
  }

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 0);
    if (hmq) {
      WinCancelShutdown(hmq, TRUE);
      IncrThreadUsage();
      priority_normal();
      switch (cmp->action) {
      case IDM_INVERT:
	InvertAll(WinWindowFromID(cmp->hwnd, COMP_LEFTDIR));
	InvertAll(WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR));
	break;

      case IDM_DESELECTALL:
	Deselect(WinWindowFromID(cmp->hwnd, COMP_LEFTDIR));
	Deselect(WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR));
	break;

      default:
	SpecialSelect(WinWindowFromID(cmp->hwnd, COMP_LEFTDIR),
		      WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR),
		      cmp->action, cmp->reset);
	break;
      }
      if (!PostMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPFROMLONG(1L), MPVOID))
	WinSendMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPFROMLONG(1L), MPVOID);
      WinDestroyMsgQueue(hmq);
    }
    DecrThreadUsage();
    WinTerminate(hab);
  }
  free(cmp);
}

/**
 * Build FILELIST given pathname
 */

static VOID FillDirList(CHAR *str, INT skiplen, BOOL recurse,
			FILELIST ***list, INT *numfiles, INT *numalloc)
{
  CHAR *enddir;
  ULONG x;
  CHAR *maskstr;
  PFILEFINDBUF4L pffbArray;
  PFILEFINDBUF4L pffbFile;
  HDIR hDir;
  ULONG ulFindCnt;
  ULONG ulBufBytes = sizeof(FILEFINDBUF4L) * FilesToGet;
  APIRET rc;

  if (!str || !*str) {
    Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
    return;
  }

  // DbgMsg(pszSrcFile, __LINE__, "FillDirList start %s", str);

  maskstr = xmalloc(CCHMAXPATH, pszSrcFile, __LINE__);
  if (!maskstr)
    return;
  pffbArray = xmalloc(ulBufBytes, pszSrcFile, __LINE__);
  if (!pffbArray) {
    free(maskstr);
    return;
  }
  x = strlen(str);
  memcpy(maskstr, str, x + 1);
  enddir = maskstr + x;
  if (*(enddir - 1) != '\\') {
    *enddir = '\\';
    enddir++;
    *enddir = 0;
  }
  *enddir = '*';
  *(enddir + 1) = 0;
  hDir = HDIR_CREATE;
  DosError(FERR_DISABLEHARDERR);
  ulFindCnt = FilesToGet;
  rc = xDosFindFirst(maskstr, &hDir,
	 	     FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
		     FILE_SYSTEM | FILE_HIDDEN |
		     (recurse ? FILE_DIRECTORY : 0),
		     pffbArray, ulBufBytes, &ulFindCnt, FIL_QUERYEASIZEL);
  if (!rc) {
    do {
      pffbFile = pffbArray;
      for (x = 0; x < ulFindCnt; x++) {
	if (pffbFile->attrFile & FILE_DIRECTORY) {
	  // Skip . and ..
	  if (recurse &&
	      (pffbFile->achName[0] != '.' ||
	       (pffbFile->achName[1] &&
		(pffbFile->achName[1] != '.' || pffbFile->achName[2])))) {
	    if (fForceUpper)
	      strupr(pffbFile->achName);
	    else if (fForceLower)
	      strlwr(pffbFile->achName);
	    memcpy(enddir, pffbFile->achName, pffbFile->cchName + 1);
	    FillDirList(maskstr, skiplen, recurse, list, numfiles, numalloc);
	  }
	}
	else {
	  if (fForceUpper)
	    strupr(pffbFile->achName);
	  else if (fForceLower)
	    strlwr(pffbFile->achName);
	  memcpy(enddir, pffbFile->achName, pffbFile->cchName + 1);
	  if (AddToFileList(maskstr + skiplen,
			    pffbFile, list, numfiles, numalloc)) {
	    goto Abort;
	  }
	}
	pffbFile = (PFILEFINDBUF4L)((PBYTE)pffbFile + pffbFile->oNextEntryOffset);
      } // for
      DosError(FERR_DISABLEHARDERR);
      ulFindCnt = FilesToGet;
      rc = xDosFindNext(hDir, pffbArray, ulBufBytes, &ulFindCnt, FIL_QUERYEASIZEL);
    } while (!rc);

Abort:

    DosFindClose(hDir);
    DosSleep(0);
  }

  if (rc && rc != ERROR_NO_MORE_FILES) {
    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      GetPString(IDS_CANTFINDDIRTEXT), maskstr);
  }

  free(maskstr);
  free(pffbArray);

  // DbgMsg(pszSrcFile, __LINE__, "FillDirList finish %s", str);
}

//=== CompNames() Compare names for qsort ===

static int CompNames(const void *n1, const void *n2)
{
  FILELIST *fl1 = *(FILELIST **) n1;
  FILELIST *fl2 = *(FILELIST **) n2;

  return stricmp(fl1->fname, fl2->fname);
}

// 20 Aug 07 SHL experimental fixme

typedef struct {
  // Caller must init
  UINT sleepTime;		// How long to sleep
  UINT interval;		// How often to sleep
  // Owned by SleepIfNeeded
  UINT modulo;			// How often to call GetMSecTimer
  UINT cntr;			// Call counter
  ULONG lastMSec;		// Last time DosSleep invoked
} SLEEP_DESC;

VOID SleepIfNeeded(BOOL id, UINT interval, UINT sleepTime)
{
  static ULONG lastMSec[10];
  static UINT cntr;
  static UINT modulo = 32;
  BOOL yes = ++cntr >= modulo;

  if (yes) {
    ULONG newMSec = GetMSecTimer();
    // 1st time will have large difference, but don't care
    ULONG diff = newMSec - lastMSec[id];
    cntr = 0;
    yes = diff >= interval;
    // Try to tune modulo counter to approx 12% error
    if (yes) {
      lastMSec[id] = newMSec;
      if (diff >= interval + (interval / 8) && modulo > 0)
	modulo--;
    }
    else {
      if (diff < interval - (interval / 8))
	modulo++;
    }
    DosSleep(sleepTime);
  }
}

//=== FillCnrsThread() Fill left and right containers ===

static VOID FillCnrsThread(VOID *args)
{
  COMPARE *cmp = (COMPARE *) args;
  HAB hab;
  HMQ hmq;
  BOOL notified = FALSE;

#if 0
  ULONG lastMSec = GetMSecTimer();
  ULONG ul;
#endif

  HWND hwndLeft, hwndRight;
  CHAR szBuf[CCHMAXPATH];
  CNRINFO cnri;

  if (!cmp) {
    Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
    _endthread();
  }

  // DbgMsg(pszSrcFile, __LINE__, "FillCnrsThread enter");

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize(0);
  if (!hab)
    Win_Error(NULLHANDLE, NULLHANDLE, pszSrcFile, __LINE__, "WinInitialize");
  else {
    hmq = WinCreateMsgQueue(hab, 0);
    if (!hmq)
      Win_Error(NULLHANDLE, NULLHANDLE, pszSrcFile, __LINE__,
		"WinCreateMsgQueue");
    else {
      INT x;
      INT l;
      INT r;
      UINT cntr;
      FILELIST **filesl = NULL;
      FILELIST **filesr = NULL;
      INT numfilesl = 0;
      INT numfilesr = 0;
      INT numallocl = 0;
      INT numallocr = 0;
      INT ret = 0;
      UINT lenl;			// Directory prefix length
      UINT lenr;
      UINT recsNeeded;
      PCNRITEM pcilFirst;
      PCNRITEM pcirFirst;
      PCNRITEM pcil;
      PCNRITEM pcir;
      RECORDINSERT ri;
      CHAR *pch;

      WinCancelShutdown(hmq, TRUE);
      IncrThreadUsage();
      hwndLeft = WinWindowFromID(cmp->hwnd, COMP_LEFTDIR);
      hwndRight = WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR);
      lenl = strlen(cmp->leftdir);
      if (cmp->leftdir[strlen(cmp->leftdir) - 1] != '\\')
	lenl++;
      lenr = strlen(cmp->rightdir);
      if (cmp->rightdir[strlen(cmp->rightdir) - 1] != '\\')
	lenr++;
      priority_normal();
      // Clear containers
      RemoveCnrItems(hwndRight, NULL, 0, CMA_FREE | CMA_INVALIDATE);
      RemoveCnrItems(hwndLeft, NULL, 0, CMA_FREE | CMA_INVALIDATE);
      cmp->cmp->totalleft = cmp->cmp->totalright = 0;

      // Build list of all files in left directory
      if (fForceLower)
	strlwr(cmp->leftdir);
      else if (fForceUpper)
	strupr(cmp->leftdir);
      FillDirList(cmp->leftdir, lenl, cmp->includesubdirs,
		  &filesl, &numfilesl, &numallocl);

      if (filesl)
	qsort(filesl, numfilesl, sizeof(CHAR *), CompNames);

      // DbgMsg(pszSrcFile, __LINE__, "FillCnrsThread sorted filesl");

      // Build list of all files in right directory
      if (!*cmp->rightlist) {
	if (fForceLower)
	  strlwr(cmp->rightdir);
	else if (fForceUpper)
	  strupr(cmp->rightdir);
	FillDirList(cmp->rightdir, lenr, cmp->includesubdirs,
		    &filesr, &numfilesr, &numallocr);
      }
      else {
	// Use snapshot file
	FILE *fp;
	FILEFINDBUF4L fb4;
	CHAR str[CCHMAXPATH * 2], *p;

	memset(&fb4, 0, sizeof(fb4));
	fp = fopen(cmp->rightlist, "r");
	if (!fp)
	  Runtime_Error(pszSrcFile, __LINE__, "can not open %s (%d)",
			cmp->rightlist, errno);
	else {
	  while (!feof(fp)) {
	    // First get name of directory
	    if (!xfgets_bstripcr(str, sizeof(str), fp, pszSrcFile, __LINE__))
	      break;			// EOF
	    p = str;
	    if (*p == '\"') {
	      // Quoted
	      p++;
	      if (*p && *p != '\"') {
		p = strchr(p, '\"');
		if (p) {
		  *p = 0;
		  if (*(str + 1)) {
		    strcpy(cmp->rightdir, str + 1);
		    if (fForceUpper)
		      strupr(cmp->rightdir);
		    else if (fForceLower)
		      strlwr(cmp->rightdir);
		    p = cmp->rightdir + (strlen(cmp->rightdir) - 1);
		    if (p - cmp->rightdir > 3 && *p == '\\')
		      *p = 0;		// Chop trailing slash
		    break;
		  }
		}
	      }
	    }
	  } // while !EOF

	  memset(&cnri, 0, sizeof(cnri));
	  cnri.cb = sizeof(cnri);
	  cnri.pszCnrTitle = cmp->rightdir;
	  if (!WinSendMsg(hwndRight, CM_SETCNRINFO,
		     MPFROMP(&cnri), MPFROMLONG(CMA_CNRTITLE))) {
	    Win_Error(hwndRight, cmp->hwnd, pszSrcFile, __LINE__, "CM_SETCNRINFO");
	  }

	  if (*cmp->rightdir) {
	    lenr = strlen(cmp->rightdir);
	    if (cmp->rightdir[strlen(cmp->rightdir) - 1] != '\\')
	      lenr++;
	    while (!feof(fp)) {
	      if (!xfgets_bstripcr
		  (str, sizeof(str), fp, pszSrcFile, __LINE__))
		break;
	      p = str;
	      if (*p == '\"') {
		p++;
		if (*p && *p != '\"') {
		  p = strchr(p, '\"');
		  if (p) {
		    *p = 0;
		    p++;
		    if (*p == ',') {
		      p++;
		      if (!cmp->includesubdirs && atol(p) > lenr)
			continue;
		      p = strchr(p, ',');
		      if (p) {
			p++;
			// 27 Sep 07 SHL fixme to do ULONGLONG conversion
			fb4.cbFile = atol(p);
			p = strchr(p, ',');
			if (p) {
			  p++;
			  fb4.fdateLastWrite.year = atol(p) - 1980;
			  p = strchr(p, '/');
			  if (p) {
			    p++;
			    fb4.fdateLastWrite.month = atol(p);
			    p = strchr(p, '/');
			    if (p) {
			      p++;
			      fb4.fdateLastWrite.day = atol(p);
			      p = strchr(p, ',');
			      if (p) {
				p++;
				fb4.ftimeLastWrite.hours = atol(p);
				p = strchr(p, ':');
				if (p) {
				  p++;
				  fb4.ftimeLastWrite.minutes = atol(p);
				  p = strchr(p, ':');
				  if (p) {
				    p++;
				    fb4.ftimeLastWrite.twosecs = atol(p);
				    p = strchr(p, ',');
				    if (p) {
				      p++;
				      fb4.attrFile = atol(p);
				      p = strchr(p, ',');
				      if (p) {
					p++;
					fb4.cbList = atol(p) * 2;
					if (fForceUpper)
					  strupr(str + 1);
					else if (fForceLower)
					  strlwr(str + 1);
					if (AddToFileList((str + 1) + lenr,
							  &fb4,
							  &filesr,
							  &numfilesr,
							  &numallocr))
					  break;
				      }
				    }
				  }
				}
			      }
			    }
			  }
			}
		      }
		    }
		  }
		}
	      }
	    } // while
	  } // if have rightdir
	  fclose(fp);
	}
      }	// if snapshot file

      if (filesr)
	qsort(filesr, numfilesr, sizeof(CHAR *), CompNames);

      // DbgMsg(pszSrcFile, __LINE__, "FillCnrsThread sorted filesr");

      // We now have two lists of files, both sorted.
      // Count total number of container entries required on each side
      l = r = 0;
      recsNeeded = 0;
      while ((filesl && filesl[l]) || (filesr && filesr[r])) {

	if (filesl && filesl[l]) {
	  if (filesr && filesr[r])
	    x = stricmp(filesl[l]->fname, filesr[r]->fname);
	  else
	    x = -1;			// Left side list longer
	}
	else
	  x = +1;			// Right side list longer

	if (x <= 0)
	  l++;				// On left side
	if (x >= 0)
	  r++;				// On right side

	recsNeeded++;			// Keep count of how many entries req'd

      }	// while

      WinSendMsg(cmp->hwnd, UM_CONTAINERHWND, MPVOID, MPVOID);

      // Now insert records into the containers
      cntr = 0;
      l = r = 0;
      if (recsNeeded) {
	pcilFirst = WinSendMsg(hwndLeft,
			       CM_ALLOCRECORD,
			       MPFROMLONG(EXTRA_RECORD_BYTES),
			       MPFROMLONG(recsNeeded));
	if (!pcilFirst) {
	  Win_Error(hwndLeft, cmp->hwnd, pszSrcFile, __LINE__, "CM_ALLOCRECORD %u failed",
		    recsNeeded);
	  recsNeeded = 0;
	}
      }
      if (recsNeeded) {
	pcirFirst = WinSendMsg(hwndRight, CM_ALLOCRECORD,
			       MPFROMLONG(EXTRA_RECORD_BYTES),
			       MPFROMLONG(recsNeeded));
	if (!pcirFirst) {
	  Win_Error(hwndRight, cmp->hwnd, pszSrcFile, __LINE__, "CM_ALLOCRECORD %u failed",
		    recsNeeded);
	  recsNeeded = 0;
	  FreeCnrItemList(hwndLeft, pcilFirst);
	}
      }

      if (recsNeeded) {

	// DbgMsg(pszSrcFile, __LINE__, "FillCnrsThread filling");

	pcil = pcilFirst;
	pcir = pcirFirst;
	while ((filesl && filesl[l]) || (filesr && filesr[r])) {
	  pcir->hwndCnr = hwndRight;
	  pcir->rc.hptrIcon = (HPOINTER) 0;
	  pcil->hwndCnr = hwndLeft;
	  pcil->rc.hptrIcon = (HPOINTER) 0;

	  if (filesl && filesl[l]) {
	    if (filesr && filesr[r])
	      x = stricmp(filesl[l]->fname, filesr[r]->fname);
	    else
	      x = -1;			// Left side list longer
	  }
	  else
	    x = +1;			// Right side list longer

	  if (x <= 0) {
	    // File appears on left side
	    BldFullPathName(szBuf, cmp->leftdir, filesl[l]->fname);
	    //sprintf(szBuf, "%s%s%s", cmp->leftdir,
	    //        (cmp->leftdir[strlen(cmp->leftdir) - 1] == '\\') ?
	    //        NullStr : "\\", filesl[l]->fname);
	    pcil->pszFileName = xstrdup(szBuf, pszSrcFile, __LINE__);
	    pcil->pszDisplayName = pcil->pszFileName + lenl;
	    pcil->attrFile = filesl[l]->attrFile;
	    pcil->pszDispAttr = FileAttrToString(pcil->attrFile);
	    pcil->cbFile = filesl[l]->cbFile;
	    pcil->easize = filesl[l]->easize;
	    pcil->date.day = filesl[l]->date.day;
	    pcil->date.month = filesl[l]->date.month;
	    pcil->date.year = filesl[l]->date.year + 1980;
	    pcil->time.seconds = filesl[l]->time.twosecs * 2;
	    pcil->time.minutes = filesl[l]->time.minutes;
	    pcil->time.hours = filesl[l]->time.hours;
	    pcil->ladate.day = filesl[l]->ladate.day;
	    pcil->ladate.month = filesl[l]->ladate.month;
	    pcil->ladate.year = filesl[l]->ladate.year + 1980;
	    pcil->latime.seconds = filesl[l]->latime.twosecs * 2;
	    pcil->latime.minutes = filesl[l]->latime.minutes;
	    pcil->latime.hours = filesl[l]->latime.hours;
	    pcil->crdate.day = filesl[l]->crdate.day;
	    pcil->crdate.month = filesl[l]->crdate.month;
	    pcil->crdate.year = filesl[l]->crdate.year + 1980;
	    pcil->crtime.seconds = filesl[l]->crtime.twosecs * 2;
	    pcil->crtime.minutes = filesl[l]->crtime.minutes;
	    pcil->crtime.hours = filesl[l]->crtime.hours;
	    if (*cmp->dcd.mask.szMask) {
	      if (!Filter((PMINIRECORDCORE) pcil, (PVOID)&cmp->dcd.mask)) {
		pcil->rc.flRecordAttr |= CRA_FILTERED;
		pcir->rc.flRecordAttr |= CRA_FILTERED;
	      }
	    }
	  } // if on left

	  if (x >= 0) {
	    // File appears on right side
	    BldFullPathName(szBuf, cmp->rightdir, filesr[r]->fname);
	    //sprintf(szBuf, "%s%s%s", cmp->rightdir,
	    //        (cmp->rightdir[strlen(cmp->rightdir) - 1] == '\\') ?
	    //        NullStr : "\\", filesr[r]->fname);
	    pcir->pszFileName = xstrdup(szBuf, pszSrcFile, __LINE__);	// 31 Jul 07 SHL
	    pcir->pszDisplayName = pcir->pszFileName + lenr;
	    pcir->attrFile = filesr[r]->attrFile;
	    // pcir->rc.hptrIcon = hptrFile;
	    pcir->pszDispAttr = FileAttrToString(pcir->attrFile);
	    pcir->cbFile = filesr[r]->cbFile;
	    pcir->easize = filesr[r]->easize;
	    pcir->date.day = filesr[r]->date.day;
	    pcir->date.month = filesr[r]->date.month;
	    pcir->date.year = filesr[r]->date.year + 1980;
	    pcir->time.seconds = filesr[r]->time.twosecs * 2;
	    pcir->time.minutes = filesr[r]->time.minutes;
	    pcir->time.hours = filesr[r]->time.hours;
	    pcir->ladate.day = filesr[r]->ladate.day;
	    pcir->ladate.month = filesr[r]->ladate.month;
	    pcir->ladate.year = filesr[r]->ladate.year + 1980;
	    pcir->latime.seconds = filesr[r]->latime.twosecs * 2;
	    pcir->latime.minutes = filesr[r]->latime.minutes;
	    pcir->latime.hours = filesr[r]->latime.hours;
	    pcir->crdate.day = filesr[r]->crdate.day;
	    pcir->crdate.month = filesr[r]->crdate.month;
	    pcir->crdate.year = filesr[r]->crdate.year + 1980;
	    pcir->crtime.seconds = filesr[r]->crtime.twosecs * 2;
	    pcir->crtime.minutes = filesr[r]->crtime.minutes;
	    pcir->crtime.hours = filesr[r]->crtime.hours;
	    // Bypass check if already filtered on left side
	    if (~pcir->rc.flRecordAttr & CRA_FILTERED &&
		*cmp->dcd.mask.szMask) {
	      if (!Filter((PMINIRECORDCORE)pcir, (PVOID)&cmp->dcd.mask)) {
		pcil->rc.flRecordAttr |= CRA_FILTERED;
		pcir->rc.flRecordAttr |= CRA_FILTERED;
	      }
	    }
	  } // if on right

	  if (x == 0) {
	    // File appears on both sides
	    pcil->flags |= CNRITEM_EXISTS;
	    pcir->flags |= CNRITEM_EXISTS;
	    pch = szBuf;
	    // Subject field holds status messages
	    *pch = 0;
	    if (pcil->cbFile + pcil->easize > pcir->cbFile + pcir->easize) {
	      pcil->flags |= CNRITEM_LARGER;
	      pcir->flags |= CNRITEM_SMALLER;
	      strcpy(pch, GetPString(IDS_LARGERTEXT));
	      pch += 6;
	    }
	    else if (pcil->cbFile + pcil->easize <
		     pcir->cbFile + pcir->easize) {
	      pcil->flags |= CNRITEM_SMALLER;
	      pcir->flags |= CNRITEM_LARGER;
	      strcpy(pch, GetPString(IDS_SMALLERTEXT));
	      pch += 7;
            }
            ret = TestCDates(&pcir->date, &pcir->time,
                             &pcil->date, &pcil->time);
            if (ret == 1)
              /*((pcil->date.year > pcir->date.year) ? TRUE :
		(pcil->date.year < pcir->date.year) ? FALSE :
		(pcil->date.month > pcir->date.month) ? TRUE :
		(pcil->date.month < pcir->date.month) ? FALSE :
		(pcil->date.day > pcir->date.day) ? TRUE :
		(pcil->date.day < pcir->date.day) ? FALSE :
		(pcil->time.hours > pcir->time.hours) ? TRUE :
		(pcil->time.hours < pcir->time.hours) ? FALSE :
		(pcil->time.minutes > pcir->time.minutes) ? TRUE :
		(pcil->time.minutes < pcir->time.minutes) ? FALSE :
		(pcil->time.seconds > pcir->time.seconds) ? TRUE :
		(pcil->time.seconds < pcir->time.seconds) ? FALSE : FALSE)*/ {
	      pcil->flags |= CNRITEM_NEWER;
	      pcir->flags |= CNRITEM_OLDER;
	      if (pch != szBuf) {
		strcpy(pch, ", ");
		pch += 2;
	      }
	      strcpy(pch, GetPString(IDS_NEWERTEXT));
	      pch += 5;
	    }
            else if (ret == -1)
              /*((pcil->date.year < pcir->date.year) ? TRUE :
		     (pcil->date.year > pcir->date.year) ? FALSE :
		     (pcil->date.month < pcir->date.month) ? TRUE :
		     (pcil->date.month > pcir->date.month) ? FALSE :
		     (pcil->date.day < pcir->date.day) ? TRUE :
		     (pcil->date.day > pcir->date.day) ? FALSE :
		     (pcil->time.hours < pcir->time.hours) ? TRUE :
		     (pcil->time.hours > pcir->time.hours) ? FALSE :
		     (pcil->time.minutes < pcir->time.minutes) ? TRUE :
		     (pcil->time.minutes > pcir->time.minutes) ? FALSE :
		     (pcil->time.seconds < pcir->time.seconds) ? TRUE :
		     (pcil->time.seconds > pcir->time.seconds) ? FALSE :
		     FALSE)*/ {
	      pcil->flags |= CNRITEM_OLDER;
	      pcir->flags |= CNRITEM_NEWER;
	      if (pch != szBuf) {
		strcpy(pch, ", ");
		pch += 2;
	      }
	      strcpy(pch, GetPString(IDS_OLDERTEXT));
	      pch += 5;
	    }
	    pcil->pszSubject = *szBuf ?
				 xstrdup(szBuf, pszSrcFile, __LINE__) :
				 NullStr;

	  } // if on both sides

	  if (x <= 0) {
	    free(filesl[l]);
	    l++;
	  }

	  if (x >= 0) {
	    free(filesr[r]);
	    r++;
	  }

	  // Ensure empty buffers point somewhere
	  if (!pcil->pszFileName) {
	    pcil->pszFileName = NullStr;
	    pcil->pszDisplayName = pcil->pszFileName;
	  }

	  if (!pcir->pszFileName) {
	    pcir->pszFileName = NullStr;
	    pcir->pszDisplayName = pcir->pszFileName;
	  }

	  pcil->rc.pszIcon = pcil->pszDisplayName;
	  pcir->rc.pszIcon = pcir->pszDisplayName;

	  pcil->pszLongName = NullStr;
	  pcir->pszLongName = NullStr;

	  if (!pcil->pszSubject)
	    pcil->pszSubject = NullStr;
	  if (!pcir->pszSubject)
	    pcir->pszSubject = NullStr;

	  if (!pcil->pszDispAttr)
	    pcil->pszDispAttr = NullStr;
	  if (!pcir->pszDispAttr)
	    pcir->pszDispAttr = NullStr;

#if 0					// 20 Aug 07 SHL fixme to be gone
	  if (!(cntr % 500))
	    DosSleep(1);
	  else if (!(cntr % 50))
	    DosSleep(0);
	  cntr++;
#endif
#if 0					// 20 Aug 07 SHL
	  if (cntr++ % 256 == 0) {
	    ul = GetMSecTimer();
	    if (ul - lastMSec >= 200) {
	      lastMSec = ul;
	      DosSleep(1);
	    }
	  }
#endif
#if 1					// 20 Aug 07 SHL
	  SleepIfNeeded(0, 500, 1);
#endif

	  pcil = (PCNRITEM) pcil->rc.preccNextRecord;
	  pcir = (PCNRITEM) pcir->rc.preccNextRecord;

	} // while filling left or right

	if (filesl)
	  free(filesl);			// Free header - have already freed elements
	filesl = NULL;
	if (filesr)
	  free(filesr);
	filesr = NULL;
	// Insert 'em
	WinSendMsg(cmp->hwnd, UM_CONTAINERDIR, MPVOID, MPVOID);

	memset(&ri, 0, sizeof(RECORDINSERT));
	ri.cb = sizeof(RECORDINSERT);
	ri.pRecordOrder = (PRECORDCORE) CMA_END;
	ri.pRecordParent = (PRECORDCORE) NULL;
	ri.zOrder = (ULONG) CMA_TOP;
	ri.cRecordsInsert = recsNeeded;
	ri.fInvalidateRecord = FALSE;
	if (!WinSendMsg(hwndLeft, CM_INSERTRECORD,
			MPFROMP(pcilFirst), MPFROMP(&ri))) {
	  Win_Error(hwndLeft, cmp->hwnd, pszSrcFile, __LINE__, "CM_INSERTRECORD");
	  FreeCnrItemList(hwndLeft, pcilFirst);
	  numfilesl = 0;
	}

	memset(&ri, 0, sizeof(RECORDINSERT));
	ri.cb = sizeof(RECORDINSERT);
	ri.pRecordOrder = (PRECORDCORE) CMA_END;
	ri.pRecordParent = (PRECORDCORE) NULL;
	ri.zOrder = (ULONG) CMA_TOP;
	ri.cRecordsInsert = recsNeeded;
	ri.fInvalidateRecord = FALSE;

	if (!WinSendMsg(hwndRight, CM_INSERTRECORD,
			MPFROMP(pcirFirst), MPFROMP(&ri))) {
	  Win_Error(hwndRight, cmp->hwnd, pszSrcFile, __LINE__, "CM_INSERTRECORD");
	  RemoveCnrItems(hwndLeft, NULL, 0, CMA_FREE | CMA_INVALIDATE);
	  FreeCnrItemList(hwndRight, pcirFirst);
	  numfilesr = 0;
	}

	cmp->cmp->totalleft = numfilesl;
	cmp->cmp->totalright = numfilesr;

	// DbgMsg(pszSrcFile, __LINE__, "FillCnrsThread filled");

      }	// if recsNeeded

      Deselect(hwndLeft);
      Deselect(hwndRight);

      // DbgMsg(pszSrcFile, __LINE__, "FillCnrsThread deselected");

      if (!PostMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID))
	WinSendMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
      notified = TRUE;

      // DbgMsg(pszSrcFile, __LINE__, "FillCnrsThread FILLED posted");

      if (filesl)
	FreeList((CHAR **)filesl);	// Must have failed to create container
      if (filesr)
	FreeList((CHAR **)filesr);

      WinDestroyMsgQueue(hmq);
    }
    DecrThreadUsage();
    WinTerminate(hab);
  }
  if (!notified)
    PostMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
  free(cmp);
  DosPostEventSem(CompactSem);

  // DbgMsg(pszSrcFile, __LINE__, "FillCnrsThread exit");
}

// fixme to be gone - use variable
#define hwndLeft	(WinWindowFromID(hwnd,COMP_LEFTDIR))
#define hwndRight	(WinWindowFromID(hwnd,COMP_RIGHTDIR))

// 20 Aug 07 SHL fixme experimental

BOOL NeedGUIUpdate(BOOL id)
{
  static ULONG lastMSec[10];
  static UINT cntr;
  static UINT modulo = 32;
  BOOL yes = ++cntr >= modulo;

  if (yes) {
    ULONG newMSec = GetMSecTimer();
    // 1st time will have large difference, but don't care
    ULONG diff = newMSec - lastMSec[id];
    cntr = 0;
    yes = diff >= 500;
    // Try to tune modulo counter to 10% error
    if (yes) {
      lastMSec[id] = newMSec;
      if (diff >= 550 && modulo > 0)
	modulo--;
    }
    else {
      if (diff < 450)
	modulo++;
    }
  }
  return yes;
}

//=== CompareDlgProc() Compare directories dialog procedure ===

MRESULT EXPENTRY CompareDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  COMPARE *cmp;
  BOOL temp;

  static HPOINTER hptr;

  switch (msg) {
  case WM_INITDLG:
    cmp = (COMPARE *) mp2;
    if (!cmp) {
      Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      WinDismissDlg(hwnd, 0);
    }
    else {
      if (!hptr)
	hptr = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, COMPARE_ICON);
      WinDefDlgProc(hwnd, WM_SETICON, MPFROMLONG(hptr), MPVOID);
      cmp->hwnd = hwnd;
      WinSetWindowPtr(hwnd, QWL_USER, (PVOID) cmp);
      SetCnrCols(hwndLeft, TRUE);
      SetCnrCols(hwndRight, TRUE);
      WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
      PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
      {
	USHORT ids[] = { COMP_LEFTDIR, COMP_RIGHTDIR, COMP_TOTALLEFT,
			 COMP_TOTALRIGHT, COMP_SELLEFT, COMP_SELRIGHT,
			 0
		       };
	INT x;

	for (x = 0; ids[x]; x++)
	  SetPresParams(WinWindowFromID(hwnd, ids[x]),
			&RGBGREY,
			&RGBBLACK, &RGBBLACK, GetPString(IDS_8HELVTEXT));
      }
    }
    break;

  case UM_STRETCH:
    {
      SWP swp, swpC;
      LONG titl, szbx, szby, sz;
      HWND hwndActive;

      WinQueryWindowPos(hwnd, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
	hwndActive = WinQueryFocus(HWND_DESKTOP);
	szbx = SysVal(SV_CXSIZEBORDER);
	szby = SysVal(SV_CYSIZEBORDER);
	titl = SysVal(SV_CYTITLEBAR);
	titl += 26;
	swp.cx -= (szbx * 2);
	sz = (swp.cx / 8);
	WinQueryWindowPos(WinWindowFromID(hwnd, COMP_LEFTDIR), &swpC);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_LEFTDIR), HWND_TOP,
			szbx + 6,
			swpC.y,
			(swp.cx / 2) - (szbx + 6),
			((swp.cy - swpC.y) - titl) - szby,
			SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_RIGHTDIR), HWND_TOP,
			(swp.cx / 2) + (szbx + 6),
			swpC.y,
			(swp.cx / 2) - (szbx + 6),
			((swp.cy - swpC.y) - titl) - szby,
			SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_TOTALLEFTHDR), HWND_TOP,
			szbx + 6,
			((swp.cy - titl) - szby) + 4,
			sz - (szbx + 6), 20, SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_TOTALLEFT), HWND_TOP,
			sz + (szbx + 6),
			((swp.cy - titl) - szby) + 4,
			sz - (szbx + 6), 20, SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_SELLEFTHDR), HWND_TOP,
			(sz * 2) + (szbx + 6),
			((swp.cy - titl) - szby) + 4,
			sz - (szbx + 6), 20, SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_SELLEFT), HWND_TOP,
			(sz * 3) + (szbx + 6),
			((swp.cy - titl) - szby) + 4,
			sz - (szbx + 6), 20, SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_TOTALRIGHTHDR), HWND_TOP,
			(sz * 4) + (szbx + 6),
			((swp.cy - titl) - szby) + 4,
			sz - (szbx + 6), 20, SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_TOTALRIGHT), HWND_TOP,
			(sz * 5) + (szbx + 6),
			((swp.cy - titl) - szby) + 4,
			sz - (szbx + 6), 20, SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_SELRIGHTHDR), HWND_TOP,
			(sz * 6) + (szbx + 6),
			((swp.cy - titl) - szby) + 4,
			sz - (szbx + 6), 20, SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, COMP_SELRIGHT), HWND_TOP,
			(sz * 7) + (szbx + 6),
			((swp.cy - titl) - szby) + 4,
			sz - (szbx + 6), 20, SWP_MOVE | SWP_SIZE);
	PaintRecessedWindow(WinWindowFromID(hwnd, COMP_TOTALLEFT),
			    (HPS) 0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, COMP_SELLEFT),
			    (HPS) 0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, COMP_TOTALRIGHT),
			    (HPS) 0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, COMP_SELRIGHT),
			    (HPS) 0, FALSE, FALSE);
	PaintRecessedWindow(hwndLeft, (HPS) 0,
			    (hwndActive == hwndLeft), TRUE);
	PaintRecessedWindow(hwndRight, (HPS) 0,
			    (hwndActive == hwndRight), TRUE);
      }
    }
    return 0;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
    break;

  case UM_SETUP:
    {
      CNRINFO cnri;
      BOOL tempsubj;

      cmp = INSTDATA(hwnd);
      if (cmp) {
	cmp->dcd.size = sizeof(DIRCNRDATA);
	cmp->dcd.type = DIR_FRAME;
	cmp->dcd.hwndFrame = hwnd;
	cmp->dcd.hwndClient = hwnd;
	cmp->dcd.mask.attrFile = (FILE_DIRECTORY | FILE_ARCHIVED |
				  FILE_READONLY | FILE_SYSTEM | FILE_HIDDEN);
	LoadDetailsSwitches("DirCmp", &cmp->dcd);
	cmp->dcd.detailslongname = FALSE;
	cmp->dcd.detailsicon = FALSE;	// TRUE;
      }
      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendDlgItemMsg(hwnd, COMP_LEFTDIR, CM_QUERYCNRINFO,
			MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
      cnri.flWindowAttr |= (CA_OWNERDRAW | CV_MINI);
      cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 68;
      WinSendDlgItemMsg(hwnd, COMP_LEFTDIR, CM_SETCNRINFO, MPFROMP(&cnri),
			MPFROMLONG(CMA_FLWINDOWATTR | CMA_XVERTSPLITBAR));
      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendDlgItemMsg(hwnd, COMP_RIGHTDIR, CM_QUERYCNRINFO,
			MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
      cnri.flWindowAttr |= (CA_OWNERDRAW | CV_MINI);
      cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 54;
      WinSendDlgItemMsg(hwnd, COMP_RIGHTDIR, CM_SETCNRINFO, MPFROMP(&cnri),
			MPFROMLONG(CMA_FLWINDOWATTR | CMA_XVERTSPLITBAR));
      AdjustCnrColRO(hwndLeft, GetPString(IDS_FILENAMECOLTEXT), TRUE, FALSE);
      AdjustCnrColRO(hwndLeft, GetPString(IDS_LONGNAMECOLTEXT), TRUE, FALSE);
      AdjustCnrColRO(hwndRight, GetPString(IDS_FILENAMECOLTEXT), TRUE, FALSE);
      AdjustCnrColRO(hwndRight, GetPString(IDS_LONGNAMECOLTEXT), TRUE, FALSE);
      AdjustCnrColsForPref(hwndLeft, cmp->leftdir, &cmp->dcd, TRUE);
      tempsubj = cmp->dcd.detailssubject;
      cmp->dcd.detailssubject = FALSE;
      AdjustCnrColsForPref(hwndRight, cmp->rightdir, &cmp->dcd, TRUE);
      if (*cmp->rightlist) {
	AdjustCnrColVis(hwndRight, GetPString(IDS_LADATECOLTEXT), FALSE,
			FALSE);
	AdjustCnrColVis(hwndRight, GetPString(IDS_LATIMECOLTEXT), FALSE,
			FALSE);
	AdjustCnrColVis(hwndRight, GetPString(IDS_CRDATECOLTEXT), FALSE,
			FALSE);
	AdjustCnrColVis(hwndRight, GetPString(IDS_CRTIMECOLTEXT), FALSE,
			FALSE);
      }
      cmp->dcd.detailssubject = tempsubj;
    }
    return 0;

  case WM_DRAWITEM:
    if (mp2) {
      POWNERITEM pown = (POWNERITEM)mp2;
      PCNRDRAWITEMINFO pcown;
      PCNRITEM pci;

      pcown = (PCNRDRAWITEMINFO)pown->hItem;
      if (pcown) {
	pci = (PCNRITEM) pcown->pRecord;
	// 01 Aug 07 SHL if field null or blank, we draw
	// fixme to know why - probably to optimize and bypass draw?
	if (pci && (INT)pci != -1 && !*pci->pszFileName)
	  return MRFROMLONG(TRUE);
      }
    }
    return 0;

  case UM_CONTAINERHWND:
    WinSetDlgItemText(hwnd, COMP_NOTE, GetPString(IDS_COMPHOLDBLDLISTTEXT));
    return 0;

  case UM_CONTAINERDIR:
    WinSetDlgItemText(hwnd, COMP_NOTE, GetPString(IDS_COMPHOLDFILLCNRTEXT));
    return 0;

  case UM_CONTAINER_FILLED:
    cmp = INSTDATA(hwnd);
    if (!cmp) {
      Runtime_Error(pszSrcFile, __LINE__, "pCompare NULL");
      WinDismissDlg(hwnd, 0);
    }
    else {
      CHAR s[81];

      // DbgMsg(pszSrcFile, __LINE__, "CompareDlgProc UM_CONTAINER_FILLED enter");

      cmp->filling = FALSE;
      WinEnableWindow(hwndLeft, TRUE);
      WinEnableWindow(hwndRight, TRUE);
      WinEnableWindowUpdate(hwndLeft, TRUE);
      WinEnableWindowUpdate(hwndRight, TRUE);
      sprintf(s, " %d", cmp->totalleft);
      WinSetDlgItemText(hwnd, COMP_TOTALLEFT, s);
      sprintf(s, " %d", cmp->totalright);
      WinSetDlgItemText(hwnd, COMP_TOTALRIGHT, s);
      sprintf(s, " %d", cmp->selleft);
      WinSetDlgItemText(hwnd, COMP_SELLEFT, s);
      sprintf(s, " %d", cmp->selright);
      WinSetDlgItemText(hwnd, COMP_SELRIGHT, s);
      WinEnableWindow(WinWindowFromID(hwnd, DID_OK), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, DID_CANCEL), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, COMP_COLLECT), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBOTH), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTONE), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTNEWER), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTOLDER), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBIGGER), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSMALLER), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBOTH), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTONE), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTNEWER), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTOLDER), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBIGGER), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTSMALLER), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTALL), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAMECONTENT), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTIDENTICAL), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAME), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, IDM_INVERT), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, COMP_SETDIRS), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETELEFT), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, COMP_FILTER), TRUE);
      if (!*cmp->rightlist) {
	WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYLEFT), TRUE);
	WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVELEFT), TRUE);
	WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETERIGHT), TRUE);
	WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYRIGHT), TRUE);
	WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVERIGHT), TRUE);
      }
      WinEnableWindow(WinWindowFromID(hwnd, COMP_INCLUDESUBDIRS), TRUE);
      if (*cmp->dcd.mask.szMask)
	WinSetDlgItemText(hwnd, COMP_NOTE,
			  GetPString(IDS_COMPREADYFILTEREDTEXT));
      else
	WinSetDlgItemText(hwnd, COMP_NOTE, GetPString(IDS_COMPREADYTEXT));

      // DbgMsg(pszSrcFile, __LINE__, "CompareDlgProc UM_CONTAINER_FILLED exit");

    }
    break;

  case WM_INITMENU:
    cmp = INSTDATA(hwnd);
    if (cmp) {
      switch (SHORT1FROMMP(mp1)) {
      case IDM_COMMANDSMENU:
	SetupCommandMenu(cmp->dcd.hwndLastMenu, hwnd);
	break;
      }
    }
    break;

  case WM_MENUEND:
    cmp = INSTDATA(hwnd);
    if (cmp) {
      if ((HWND) mp2 == cmp->dcd.hwndLastMenu) {
	MarkAll(hwndLeft, TRUE, FALSE, TRUE);
	MarkAll(hwndRight, TRUE, FALSE, TRUE);
	WinDestroyWindow(cmp->dcd.hwndLastMenu);
	cmp->dcd.hwndLastMenu = (HWND) 0;
      }
    }
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case COMP_INCLUDESUBDIRS:
      switch (SHORT2FROMMP(mp1)) {
      case BN_CLICKED:
	cmp = INSTDATA(hwnd);
	if (cmp)
	  *cmp->rightlist = 0;
	PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	break;
      }
      break;
    case COMP_HIDENOTSELECTED:
      switch (SHORT2FROMMP(mp1)) {
      case BN_CLICKED:
	WinSendMsg(hwnd, UM_HIDENOTSELECTED, MPVOID, MPVOID);
	break;
      }
      break;

    case COMP_LEFTDIR:
    case COMP_RIGHTDIR:
      switch (SHORT2FROMMP(mp1)) {
      case CN_KILLFOCUS:
	PaintRecessedWindow(WinWindowFromID(hwnd, SHORT1FROMMP(mp1)),
			    (HPS) 0, FALSE, TRUE);
	break;

      case CN_SETFOCUS:
	PaintRecessedWindow(WinWindowFromID(hwnd, SHORT1FROMMP(mp1)),
			    (HPS) 0, TRUE, TRUE);
	break;

      case CN_ENTER:
	if (mp2) {

	  PCNRITEM pci = (PCNRITEM) ((PNOTIFYRECORDENTER) mp2)->pRecord;
	  HWND hwndCnr = WinWindowFromID(hwnd, SHORT1FROMMP(mp1));

	  SetShiftState();
	  if (pci) {
	    if (pci->rc.flRecordAttr & CRA_INUSE || !pci || !*pci->pszFileName)
	      break;
	    WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		       MPFROM2SHORT(TRUE, CRA_INUSE));
	    if (pci->attrFile & FILE_DIRECTORY) {
	      if ((shiftstate & (KC_CTRL | KC_SHIFT)) == (KC_CTRL | KC_SHIFT))
		OpenObject(pci->pszFileName, Settings, hwnd);
	      else
		OpenObject(pci->pszFileName, Default, hwnd);
	    }
	    else
	      DefaultViewKeys(hwnd, hwnd, HWND_DESKTOP, NULL,
			      pci->pszFileName);
	    WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS,
		       MPFROMP(pci),
		       MPFROM2SHORT(FALSE, CRA_INUSE |
				    ((fUnHilite) ? CRA_SELECTED : 0)));
	  }
	}
	break;

      case CN_CONTEXTMENU:
	cmp = INSTDATA(hwnd);
	if (cmp) {

	  PCNRITEM pci = (PCNRITEM) mp2;
	  USHORT id = COMP_CNRMENU;

	  if (cmp->dcd.hwndLastMenu)
	    WinDestroyWindow(cmp->dcd.hwndLastMenu);
	  cmp->dcd.hwndLastMenu = (HWND) 0;
	  cmp->hwndCalling = WinWindowFromID(hwnd, SHORT1FROMMP(mp1));
	  if (pci) {
	    if (!pci || !*pci->pszFileName || *cmp->rightlist)
	      break;
	    id = COMP_MENU;
	    WinSendMsg(cmp->hwndCalling, CM_SETRECORDEMPHASIS,
		       MPFROMP(pci), MPFROM2SHORT(TRUE, CRA_CURSORED));
	  }
	  cmp->dcd.hwndLastMenu = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, id);
	  if (cmp->dcd.hwndLastMenu) {
	    if (id == COMP_CNRMENU) {
	      if (SHORT1FROMMP(mp1) == COMP_RIGHTDIR)
		WinSendMsg(cmp->dcd.hwndLastMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_SHOWSUBJECT, FALSE), MPVOID);
	      SetDetailsSwitches(cmp->dcd.hwndLastMenu, &cmp->dcd);
	      if (SHORT1FROMMP(mp1) == COMP_LEFTDIR)
		WinSendMsg(cmp->dcd.hwndLastMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_LOADLISTFILE, 0), MPVOID);
	      else if (*cmp->rightlist)
		WinSendMsg(cmp->dcd.hwndLastMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_SAVELISTFILE, 0), MPVOID);
	    }
	    PopupMenu(hwnd, hwnd, cmp->dcd.hwndLastMenu);
	  }
	}
	break;

      case CN_INITDRAG:
	cmp = INSTDATA(hwnd);
	if (*cmp->rightlist && SHORT1FROMMP(mp1) == COMP_RIGHTDIR)
	  break;
	DoFileDrag(WinWindowFromID(hwnd, SHORT1FROMMP(mp1)),
		   (HWND) 0, mp2, NULL, NULL, TRUE);
	break;

      case CN_BEGINEDIT:
      case CN_REALLOCPSZ:
	// fixme to be gone - field edits not allowed
	Runtime_Error(pszSrcFile, __LINE__,
		      "CN_BEGINEDIT/CN_REALLOCPSZ unexpected");
	break;

      case CN_EMPHASIS:
	{
	  PNOTIFYRECORDEMPHASIS pre = mp2;
	  PCNRITEM pci;

	  if (pre->fEmphasisMask & CRA_SELECTED) {
	    pci = (PCNRITEM) pre->pRecord;
	    if (pci) {
	      if (!pci || !*pci->pszFileName) {
		if (pci->rc.flRecordAttr & CRA_SELECTED)
		  WinSendDlgItemMsg(hwnd, SHORT1FROMMP(mp1),
				    CM_SETRECORDEMPHASIS,
				    MPFROMP(pci),
				    MPFROM2SHORT(FALSE, CRA_SELECTED));
	      }
	      else {

		CHAR s[81];

		cmp = INSTDATA(hwnd);
		if (pci->rc.flRecordAttr & CRA_SELECTED) {
		  if (SHORT1FROMMP(mp1) == COMP_LEFTDIR)
		    cmp->selleft++;
		  else
		    cmp->selright++;
		}
		else {
		  if (SHORT1FROMMP(mp1) == COMP_LEFTDIR) {
		    if (cmp->selleft)
		      cmp->selleft--;
		  }
		  else {
		    if (cmp->selright)
		      cmp->selright--;
		  }
		}
		if (SHORT1FROMMP(mp1) == COMP_LEFTDIR) {
		  // if (WinIsWindowEnabled(hwndLeft) || !(cmp->selleft % 50)) {
		  if (WinIsWindowEnabled(hwndLeft) || NeedGUIUpdate(0)) {
		    sprintf(s, " %d", cmp->selleft);
		    WinSetDlgItemText(hwnd, COMP_SELLEFT, s);
		  }
		}
		else {
		  // if (WinIsWindowEnabled(hwndRight) || !(cmp->selright % 50)) {
		  if (WinIsWindowEnabled(hwndRight) || NeedGUIUpdate(1)) {
		    sprintf(s, " %d", cmp->selright);
		    WinSetDlgItemText(hwnd, COMP_SELRIGHT, s);
		  }
		}
	      }
	    }
	  }
	}
	break;

      case CN_SCROLL:
	cmp = INSTDATA(hwnd);
	if (!cmp->forcescroll) {

	  PNOTIFYSCROLL pns = mp2;

	  if (pns->fScroll & CMA_VERTICAL) {
	    cmp->forcescroll = TRUE;
	    WinSendDlgItemMsg(hwnd, (SHORT1FROMMP(mp1) == COMP_LEFTDIR) ?
			      COMP_RIGHTDIR : COMP_LEFTDIR,
			      CM_SCROLLWINDOW, MPFROMSHORT(CMA_VERTICAL),
			      MPFROMLONG(pns->lScrollInc));
	    cmp->forcescroll = FALSE;
	  }
	}
	break;
      }
      break;				// COMP_RIGHTDIR
    }
    return 0;				// WM_CONTROL

  case UM_SETDIR:
    cmp = INSTDATA(hwnd);
    if (cmp) {

      COMPARE *forthread;
      CNRINFO cnri;

      cmp->includesubdirs = WinQueryButtonCheckstate(hwnd,
						     COMP_INCLUDESUBDIRS);
      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      cnri.pszCnrTitle = cmp->leftdir;
      cnri.flWindowAttr = CV_DETAIL | CV_MINI |
			  CA_CONTAINERTITLE | CA_TITLESEPARATOR |
			  CA_DETAILSVIEWTITLES | CA_OWNERDRAW;
      WinSendDlgItemMsg(hwnd, COMP_LEFTDIR, CM_SETCNRINFO, MPFROMP(&cnri),
			MPFROMLONG(CMA_CNRTITLE | CMA_FLWINDOWATTR));
      cnri.pszCnrTitle = cmp->rightdir;
      WinSendDlgItemMsg(hwnd, COMP_RIGHTDIR, CM_SETCNRINFO, MPFROMP(&cnri),
			MPFROMLONG(CMA_CNRTITLE | CMA_FLWINDOWATTR));
      WinCheckButton(hwnd, COMP_HIDENOTSELECTED, 0);
      cmp->filling = TRUE;
      forthread = xmalloc(sizeof(COMPARE), pszSrcFile, __LINE__);
      if (!forthread)
	WinDismissDlg(hwnd, 0);
      else {
	*forthread = *cmp;
	forthread->cmp = cmp;
	if (_beginthread(FillCnrsThread, NULL, 122880, (PVOID) forthread) ==
	    -1) {
	  Runtime_Error(pszSrcFile, __LINE__,
			GetPString(IDS_COULDNTSTARTTHREADTEXT));
	  WinDismissDlg(hwnd, 0);
	  free(forthread);
	}
	else {
	  WinEnableWindowUpdate(hwndLeft, FALSE);
	  WinEnableWindowUpdate(hwndRight, FALSE);
	  cmp->selleft = cmp->selright = 0;
	  WinSetDlgItemText(hwnd, COMP_SELLEFT, "0");
	  WinSetDlgItemText(hwnd, COMP_SELRIGHT, "0");
	  WinSetDlgItemText(hwnd, COMP_TOTALLEFT, "0");
	  WinSetDlgItemText(hwnd, COMP_TOTALRIGHT, "0");
	  WinSetDlgItemText(hwnd, COMP_NOTE,
			    GetPString(IDS_COMPHOLDREADDISKTEXT));
	  WinEnableWindow(hwndRight, FALSE);
	  WinEnableWindow(hwndLeft, FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DID_CANCEL), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_COLLECT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBOTH), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTONE), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTNEWER), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTOLDER), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBIGGER), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSMALLER), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBOTH), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTONE), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTNEWER), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTOLDER), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBIGGER), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTSMALLER), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTALL), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_INCLUDESUBDIRS), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_SETDIRS), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETELEFT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETERIGHT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYLEFT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVELEFT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYRIGHT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVERIGHT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAMECONTENT),
			  FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTIDENTICAL), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAME), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, IDM_INVERT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, COMP_FILTER), FALSE);
	}
      }
    }
    return 0;

  case UM_FILTER:
    cmp = INSTDATA(hwnd);
    if (cmp) {
      if (mp1) {
	DosEnterCritSec();
	SetMask((CHAR *) mp1, &cmp->dcd.mask);
	DosExitCritSec();
      }
      cmp->dcd.suspendview = 1;
      WinSendMsg(hwndLeft, CM_FILTER, MPFROMP(Filter),
		 MPFROMP(&cmp->dcd.mask));
      WinSendMsg(hwndRight, CM_FILTER, MPFROMP(Filter),
		 MPFROMP(&cmp->dcd.mask));
      cmp->dcd.suspendview = 0;
      if (*cmp->dcd.mask.szMask)
	WinSetDlgItemText(hwnd, COMP_NOTE,
			  GetPString(IDS_COMPREADYFILTEREDTEXT));
      else
	WinSetDlgItemText(hwnd, COMP_NOTE, GetPString(IDS_COMPREADYTEXT));
    }
    return 0;

  case UM_HIDENOTSELECTED:
    cmp = INSTDATA(hwnd);
    if (cmp) {
      USHORT wantHide = WinQueryButtonCheckstate(hwnd,
						 COMP_HIDENOTSELECTED);

      cmp->dcd.suspendview = 1;
      if (wantHide) {
	BOOL needRefresh = FALSE;
	HWND hwndl = WinWindowFromID(cmp->hwnd, COMP_LEFTDIR);
	HWND hwndr = WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR);
	PCNRITEM pcil = WinSendMsg(hwndl, CM_QUERYRECORD, MPVOID,
				   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
	PCNRITEM pcir = WinSendMsg(hwndr, CM_QUERYRECORD, MPVOID,
				   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));

	while (pcil && (INT) pcil != -1 && pcir && (INT) pcir != -1) {
	  if (~pcil->rc.flRecordAttr & CRA_SELECTED &&
	      ~pcir->rc.flRecordAttr & CRA_SELECTED) {
	    pcil->rc.flRecordAttr |= CRA_FILTERED;
	    pcir->rc.flRecordAttr |= CRA_FILTERED;
	    needRefresh = TRUE;
	  }
	  pcil = WinSendMsg(hwndl, CM_QUERYRECORD, MPFROMP(pcil),
			    MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
	  pcir = WinSendMsg(hwndr, CM_QUERYRECORD, MPFROMP(pcir),
			    MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
	} // while
	if (needRefresh) {
	  WinSendMsg(hwndl, CM_INVALIDATERECORD,
		     MPVOID, MPFROM2SHORT(0, CMA_REPOSITION));
	  WinSendMsg(hwndr, CM_INVALIDATERECORD,
		     MPVOID, MPFROM2SHORT(0, CMA_REPOSITION));
	}
      }
      else {
	WinSendMsg(hwndLeft, CM_FILTER, MPFROMP(Filter),
		   MPFROMP(&cmp->dcd.mask));
	WinSendMsg(hwndRight, CM_FILTER, MPFROMP(Filter),
		   MPFROMP(&cmp->dcd.mask));
      }
      cmp->dcd.suspendview = 0;
      if (*cmp->dcd.mask.szMask)
	WinSetDlgItemText(hwnd, COMP_NOTE,
			  GetPString(IDS_COMPREADYFILTEREDTEXT));
      else
	WinSetDlgItemText(hwnd, COMP_NOTE, GetPString(IDS_COMPREADYTEXT));
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_COMPARE:
      cmp = INSTDATA(hwnd);
      if (cmp) {

	PCNRITEM pci;
	CHAR ofile[CCHMAXPATH];

	pci = (PCNRITEM) WinSendMsg(cmp->hwndCalling,
				    CM_QUERYRECORDEMPHASIS,
				    MPFROMLONG(CMA_FIRST),
				    MPFROMSHORT(CRA_CURSORED));
	// 01 Aug 07 SHL
	if (pci && *pci->pszFileName) {
	  if (cmp->hwndCalling == hwndLeft)
	    strcpy(ofile, cmp->rightdir);
	  else
	    strcpy(ofile, cmp->leftdir);
	  if (ofile[strlen(ofile) - 1] != '\\')
	    strcat(ofile, "\\");
	  strcat(ofile, pci->pszDisplayName);
	  if (*compare) {
	    CHAR *fakelist[3];
	    fakelist[0] = pci->pszFileName;
	    fakelist[1] = ofile;
	    fakelist[2] = NULL;
	    ExecOnList(hwnd, compare,
                       WINDOWED | SEPARATEKEEP, NULL, fakelist, NULL,
                       pszSrcFile, __LINE__);
	  }
	  else {
	    FCOMPARE fc;
	    memset(&fc, 0, sizeof(fc));
	    fc.size = sizeof(fc);
	    fc.hwndParent = hwnd;
	    strcpy(fc.file1, pci->pszFileName);
	    strcpy(fc.file2, ofile);
	    WinDlgBox(HWND_DESKTOP, hwnd,
		      CFileDlgProc, FM3ModHandle, FCMP_FRAME, (PVOID) & fc);
	  }
	}
      }
      break;

    case COMP_FILTER:
    case IDM_FILTER:
      cmp = INSTDATA(hwnd);
      if (cmp) {

	BOOL empty = FALSE;
	PCNRITEM pci;
	CHAR *p;
	BOOL temp;

	if (!*cmp->dcd.mask.szMask) {
	  empty = TRUE;
	  temp = fSelectedAlways;
	  fSelectedAlways = TRUE;
	  pci = (PCNRITEM)CurrentRecord(hwnd);
	  fSelectedAlways = temp;
	  // 01 Aug 07 SHL
	  if (pci && ~pci->attrFile & FILE_DIRECTORY) {
	    p = strrchr(pci->pszFileName, '\\');
	    if (p) {
	      p++;
	      strcpy(cmp->dcd.mask.szMask, p);
	    }
	  }
	}
	cmp->dcd.mask.fNoAttribs = TRUE;
	cmp->dcd.mask.attrFile = ALLATTRS;
	cmp->dcd.mask.antiattr = 0;
	if (WinDlgBox(HWND_DESKTOP, hwnd, PickMaskDlgProc,
		      FM3ModHandle, MSK_FRAME, MPFROMP(&cmp->dcd.mask))) {
	  cmp->dcd.mask.attrFile = ALLATTRS;
	  cmp->dcd.mask.antiattr = 0;
	  WinSendMsg(hwnd, UM_FILTER, MPVOID, MPVOID);
	}
	else if (empty) {
	  *cmp->dcd.mask.szMask = 0;
	  cmp->dcd.mask.attrFile = ALLATTRS;
	  cmp->dcd.mask.antiattr = 0;
	}
      }
      break;

    case IDM_SHOWSUBJECT:
    case IDM_SHOWEAS:
    case IDM_SHOWSIZE:
    case IDM_SHOWLWDATE:
    case IDM_SHOWLWTIME:
    case IDM_SHOWLADATE:
    case IDM_SHOWLATIME:
    case IDM_SHOWCRDATE:
    case IDM_SHOWCRTIME:
    case IDM_SHOWATTR:
      cmp = INSTDATA(hwnd);
      if (cmp) {

	DIRCNRDATA dcd1;
	BOOL tempsubj;

	dcd1 = cmp->dcd;
	AdjustDetailsSwitches(hwndLeft,
			      (HWND) 0, SHORT1FROMMP(mp1),
			      cmp->leftdir, "DirCmp", &cmp->dcd, TRUE);
	tempsubj = cmp->dcd.detailssubject;
	cmp->dcd = dcd1;
	cmp->dcd.detailssubject = FALSE;
	AdjustDetailsSwitches(hwndRight,
			      cmp->dcd.hwndLastMenu, SHORT1FROMMP(mp1),
			      cmp->rightdir, "DirCmp", &cmp->dcd, TRUE);
	cmp->dcd.detailssubject = tempsubj;
      }
      break;

    case IDM_LOADLISTFILE:
      cmp = INSTDATA(hwnd);
      if (cmp) {

	CHAR fullname[CCHMAXPATH];

	strcpy(fullname, "*.PMD");
	if (insert_filename(HWND_DESKTOP, fullname, TRUE, FALSE) &&
	    *fullname && !strchr(fullname, '*') && !strchr(fullname, '?')) {
	  strcpy(cmp->rightlist, fullname);
	  PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	  PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	}
      }
      break;

    case IDM_SAVELISTFILE:
      cmp = INSTDATA(hwnd);
      if (cmp) {

	SNAPSTUFF *sf;
	CHAR fullname[CCHMAXPATH];

	strcpy(fullname, "*.PMD");
	if (export_filename(HWND_DESKTOP, fullname, 1) && *fullname &&
	    !strchr(fullname, '*') && !strchr(fullname, '?')) {
	  sf = xmallocz(sizeof(SNAPSTUFF), pszSrcFile, __LINE__);
	  if (sf) {
	    strcpy(sf->filename, fullname);
	    if (hwndLeft == cmp->hwndCalling)
	      strcpy(sf->dirname, cmp->leftdir);
	    else
	      strcpy(sf->dirname, cmp->rightdir);
	    sf->recurse = cmp->includesubdirs;
	    if (_beginthread(StartSnap, NULL, 65536, (PVOID) sf) == -1) {
	      Runtime_Error(pszSrcFile, __LINE__,
			    GetPString(IDS_COULDNTSTARTTHREADTEXT));
	      free(sf);
	    }
	  }
	}
      }
      break;

    case COMP_SETDIRS:
      cmp = INSTDATA(hwnd);
      if (cmp) {

	WALK2 wa;

	memset(&wa, 0, sizeof(wa));
	wa.size = sizeof(wa);
	strcpy(wa.szCurrentPath1, cmp->leftdir);
	strcpy(wa.szCurrentPath2, cmp->rightdir);
	if (WinDlgBox(HWND_DESKTOP, hwnd, WalkTwoCmpDlgProc,
		      FM3ModHandle, WALK2_FRAME,
		      MPFROMP(&wa)) &&
	    !IsFile(wa.szCurrentPath1) &&
	    !IsFile(wa.szCurrentPath2)) {
	  strcpy(cmp->leftdir, wa.szCurrentPath1);
	  strcpy(cmp->rightdir, wa.szCurrentPath2);
	  *cmp->rightlist = 0;
	  PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	  PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	}
      }
      break;

    case COMP_COPYLEFT:
    case COMP_MOVELEFT:
    case COMP_COPYRIGHT:
    case COMP_MOVERIGHT:
    case COMP_DELETELEFT:
    case COMP_DELETERIGHT:
      cmp = INSTDATA(hwnd);
      if (cmp) {

	COMPARE *forthread;

	cmp->filling = TRUE;
	forthread = xmalloc(sizeof(COMPARE), pszSrcFile, __LINE__);
	if (forthread) {
	  *forthread = *cmp;
	  forthread->cmp = cmp;
	  forthread->action = SHORT1FROMMP(mp1);
	  if (_beginthread(ActionCnrThread, NULL, 122880, (PVOID) forthread)
	      == -1) {
	    Runtime_Error(pszSrcFile, __LINE__,
			  GetPString(IDS_COULDNTSTARTTHREADTEXT));
	    free(forthread);
	  }
	  else {
	    WinEnableWindowUpdate(hwndLeft, FALSE);
	    WinEnableWindowUpdate(hwndRight, FALSE);
	    switch (SHORT1FROMMP(mp1)) {
	    case COMP_DELETELEFT:
	    case COMP_DELETERIGHT:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				GetPString(IDS_COMPHOLDDELETINGTEXT));
	      break;
	    case COMP_MOVELEFT:
	    case COMP_MOVERIGHT:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				GetPString(IDS_COMPHOLDMOVINGTEXT));
	      break;
	    case COMP_COPYLEFT:
	    case COMP_COPYRIGHT:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				GetPString(IDS_COMPHOLDCOPYINGTEXT));
	      break;
	    default:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				GetPString(IDS_COMPHOLDDUNNOTEXT));
	      break;
	    }
	    WinEnableWindow(hwndRight, FALSE);
	    WinEnableWindow(hwndLeft, FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, DID_CANCEL), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_COLLECT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBOTH), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTONE), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTNEWER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTOLDER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBIGGER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSMALLER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBOTH), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTONE), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTNEWER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTOLDER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBIGGER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTSMALLER),
			    FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTALL), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_INCLUDESUBDIRS),
			    FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_SETDIRS), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETELEFT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETERIGHT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYLEFT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVELEFT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYRIGHT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVERIGHT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAMECONTENT),
			    FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTIDENTICAL),
			    FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAME), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_INVERT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_FILTER), FALSE);
	  }
	}
      }
      break;

    case DID_OK:
      WinDismissDlg(hwnd, 0);
      break;
    case DID_CANCEL:
      WinDismissDlg(hwnd, 1);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_COMPARE, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case IDM_DESELECTALL:
    case IDM_SELECTNEWER:
    case IDM_SELECTOLDER:
    case IDM_SELECTBIGGER:
    case IDM_SELECTSMALLER:
    case IDM_DESELECTNEWER:
    case IDM_DESELECTOLDER:
    case IDM_DESELECTBIGGER:
    case IDM_DESELECTSMALLER:
    case IDM_DESELECTONE:
    case IDM_DESELECTBOTH:
    case IDM_SELECTBOTH:
    case IDM_SELECTONE:
    case IDM_SELECTSAMECONTENT:
    case IDM_SELECTIDENTICAL:		// Name, size and time
    case IDM_SELECTSAME:		// Name and size
    case IDM_INVERT:
      cmp = INSTDATA(hwnd);
      if (!cmp)
	Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      else {
	COMPARE *forthread;

	cmp->filling = TRUE;
	forthread = xmalloc(sizeof(COMPARE), pszSrcFile, __LINE__);
	if (forthread) {
	  *forthread = *cmp;
	  forthread->cmp = cmp;
	  forthread->action = SHORT1FROMMP(mp1);
	  if (_beginthread(SelectCnrsThread, NULL, 65536, (PVOID) forthread)
	      == -1) {
	    Runtime_Error(pszSrcFile, __LINE__,
			  GetPString(IDS_COULDNTSTARTTHREADTEXT));
	    free(forthread);
	  }
	  else {
	    WinEnableWindowUpdate(hwndLeft, FALSE);
	    WinEnableWindowUpdate(hwndRight, FALSE);
	    switch (SHORT1FROMMP(mp1)) {
	    case IDM_DESELECTALL:
	    case IDM_DESELECTNEWER:
	    case IDM_DESELECTOLDER:
	    case IDM_DESELECTBIGGER:
	    case IDM_DESELECTSMALLER:
	    case IDM_DESELECTONE:
	    case IDM_DESELECTBOTH:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				GetPString(IDS_COMPHOLDDESELTEXT));
	      break;
	    case IDM_INVERT:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				GetPString(IDS_COMPHOLDINVERTTEXT));
	      break;
	    default:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				GetPString(IDS_COMPHOLDSELTEXT));
	      break;
	    }
	    WinEnableWindow(hwndRight, FALSE);
	    WinEnableWindow(hwndLeft, FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, DID_CANCEL), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_COLLECT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBOTH), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTONE), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTNEWER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTOLDER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBIGGER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSMALLER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBOTH), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTONE), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTNEWER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTOLDER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBIGGER), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTSMALLER),
			    FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTALL), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_INCLUDESUBDIRS),
			    FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_SETDIRS), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETELEFT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETERIGHT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYLEFT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVELEFT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYRIGHT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVERIGHT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAMECONTENT),
			    FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTIDENTICAL),
			    FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAME), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, IDM_INVERT), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, COMP_FILTER), FALSE);
	  }
	}
      }
      break;

    case COMP_COLLECT:
      cmp = INSTDATA(hwnd);
      if (!cmp)
	Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      else {
	CHAR **listl;
	CHAR **listr = NULL;
	if (!Collector) {
	  SWP swp;
	  HWND hwndC;
	  if (!fAutoTile &&
	       !ParentIsDesktop(hwnd, cmp->hwndParent) &&
	       !fExternalCollector &&
	       !strcmp(realappname, FM3Str)) {
	    GetNextWindowPos(cmp->hwndParent, &swp, NULL, NULL);
	  }
	  hwndC = StartCollector(fExternalCollector ||
				 strcmp(realappname, FM3Str) ?
				   HWND_DESKTOP :
				   cmp->hwndParent,
				 4);
	  if (hwndC) {
	    if (!fAutoTile &&
		!ParentIsDesktop(hwnd, cmp->hwndParent) &&
		!fExternalCollector &&
		!strcmp(realappname, FM3Str)) {
	      WinSetWindowPos(hwndC, HWND_TOP,
			      swp.x, swp.y, swp.cx, swp.cy,
			      SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER);
	    }
	    else if (!ParentIsDesktop(hwnd, cmp->hwndParent) &&
		     fAutoTile &&
		     !strcmp(realappname, FM3Str)) {
	      TileChildren(cmp->hwndParent, TRUE);
	    }
	    DosSleep(32);		// 05 Aug 07 GKY 64
	    PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(COMP_COLLECT, 0), MPVOID);
	    break;
	  }
	}
	else
	  StartCollector(cmp->hwndParent, 4);

	temp = fSelectedAlways;
	fSelectedAlways = TRUE;
	listl = BuildList(hwndLeft);
	if (!*cmp->rightlist)
	  listr = BuildList(hwndRight);
	fSelectedAlways = temp;

	if (listl || listr) {
	  if (Collector) {
	    // 07 Aug 07 SHL Avoid collected from empty list
	    if (listl && listl[0] && *listl[0]) {
	      if (PostMsg(Collector, WM_COMMAND,
			  MPFROM2SHORT(IDM_COLLECTOR, 0), MPFROMP(listl)))
		listl = NULL;		// Collector will free
	    }
	    if (listr && listr[0] && *listr[0]) {
	      if (PostMsg(Collector, WM_COMMAND,
			  MPFROM2SHORT(IDM_COLLECTOR, 0), MPFROMP(listr)))
		listr = NULL;		// Collector will free
	    }
	    WinSetWindowPos(WinQueryWindow(WinQueryWindow(Collector,
							  QW_PARENT),
					   QW_PARENT),
			    HWND_TOP, 0, 0, 0, 0,
			    SWP_ACTIVATE);
	  }
	  FreeList(listl);
	  FreeList(listr);
	}
      }
      break;
    }
    return 0;

  case WM_CLOSE:
    WinDismissDlg(hwnd, 0);
    return 0;

  case WM_DESTROY:
    cmp = INSTDATA(hwnd);
    if (cmp) {
      if (cmp->dcd.hwndLastMenu)
	WinDestroyWindow(cmp->dcd.hwndLastMenu);
      if (cmp->dcd.hwndObject) {
	WinSetWindowPtr(cmp->dcd.hwndObject, QWL_USER, (PVOID) NULL);
	if (!PostMsg(cmp->dcd.hwndObject, WM_CLOSE, MPVOID, MPVOID))
	  WinSendMsg(cmp->dcd.hwndObject, WM_CLOSE, MPVOID, MPVOID);
      }
      free(cmp);
    }
    EmptyCnr(hwndLeft);
    EmptyCnr(hwndRight);
    DosPostEventSem(CompactSem);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(COMPAREDIR,FillCnrsThread,FillDirList,CompNames,BldFullPathName)
#pragma alloc_text(COMPAREDIR1,CompareDlgProc)
#pragma alloc_text(COMPAREDIR2,SelectCnrsThread,ActionCnrThread)
#pragma alloc_text(COMPAREFILE,CFileDlgProc,CompareFilesThread)
#pragma alloc_text(SNAPSHOT,SnapShot,StartSnap)

