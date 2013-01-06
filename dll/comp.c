
/***********************************************************************

  $Id$

  Compare directories

  Copyright (c) 1993-02 M. Kimes
  Copyright (c) 2003, 2012 Steven H. Levine

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
  04 Jan 08 SHL Avoid traps if CM_ALLOCRECORD returns less that requested
  05 Jan 08 SHL Use WM_TIMER for progress messaging
  05 Jan 08 SHL Use ITIMER_DESC for hogging control
  12 Jan 08 SHL Correct select count display regression
  12 Jan 08 SHL Localize SpecialSelect here and rename
  12 Jan 08 SHL Use SleepIfNeeded
  12 Jan 08 SHL Reduce/eliminate more DosSleep calls
  16 Jan 08 SHL Update total/select counts with WM_TIMER only
  17 Jan 08 SHL Change hide not selected button to 3 state
  18 Jan 08 SHL Honor filters in actions
  20 Jan 08 GKY Compare dialog now saves and restores size and position
  29 Feb 08 GKY Use xfree where appropriate
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  16 Mar 08 GKY Prevent trap caused by files that exceed maxpath length
  11 Jul 08 JBS Ticket 230: Simplified code and eliminated some local variables by incorporating
		all the details view settings (both the global variables and those in the
		DIRCNRDATA struct) into a new struct: DETAILS_SETTINGS.
  08 Sep 08 SHL Avoid aliased pszLongName pointer in ActionCnrThread IDM_MOVE
  10 Dec 08 SHL Integrate exception handler support
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis.
  11 Jan 09 GKY Replace font names in the string file with global set at compile in init.c
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)
  08 Mar 09 GKY Additional strings move to PCSZs in init.c & String Table
  15 Mar 09 GKY Use WriteDetailsSwitchs to save detail switch changes to the ini file.
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.
  13 Jul 09 SHL Sync with renames
  26 Sep 09 SHL Don't hide if nothing selected
  27 Sep 09 SHL Support AND'ed selections
  27 Sep 09 SHL Rework CompSelect for size and speed
  27 Sep 09 SHL Allow fast cancel
  27 Sep 09 SHL Drop unused reset logic
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  23 Oct 10 GKY Add ForwardslashToBackslash function to streamline code
  29 May 11 SHL Rework >65K records logic - prior fix was not quite right
  12 Jun 11 GKY Added SleepIfNeeded in the container fill loop
  02 Jan 12 GKY Added pszFmtFileSize to container info to fix loss of file sizes on move and copy.
  12 Aug 12 GKY Add ability to change and save PresParam
  12 Aug 12 GKY Fix loading of a list file in the right compare container
  12 Aug 12 GKY Allow for selection of include subdirectories or a list file on initial startup of compare dirs
  05 Jan 13 GKY Fix snapshot file to actually load and save (with/without subdirectories) properly.
  05 Jan 13 GKY Toggle of include subdirectories leaves snapshot file loaded.
  05 Jan 13 GKY Added an indicator (textbox) that a list (snapshot) file is loaded.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <io.h>
#include <ctype.h>
#include <limits.h>			// USHRT_MAX

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSERRORS
#define INCL_GPI
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "mainwnd2.h"			// Data declaration(s)
#include "inis.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "newview.h"			// Data declarations
#include "fm3dlg.h"
#include "fm3str.h"
#include "pathutil.h"			// BldFullPathName
#include "filldir.h"			// EmptyCnr...
#include "makelist.h"			// AddToFileList...
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "tmrsvcs.h"			// IsITimerExpired
#include "comp.h"
#include "misc.h"			// AddToListboxBottom, AdjustCnrColRO, AdjustCnrColVis,
					// AdjustCnrColsForPref, CurrentRecord,
					// AdjustDetailsSwitches, LoadDetailsSwitches, SetCnrCols
					// SetDetailsSwitches
#include "select.h"			// Deselect, Deselect, InvertAll
#include "mkdir.h"			// MassMkdir
#include "valid.h"			// TestCDates
#include "walkem.h"			// WalkTwoCmpDlgProc
#include "common.h"			// DecrThreadUsage, IncrThreadUsage
#include "defview.h"			// DefaultViewKeys
#include "draglist.h"			// DoFileDrag
#include "systemf.h"			// ExecOnList
#include "filter.h"			// Filter
#include "mainwnd.h"			// GetNextWindowPos
#include "shadow.h"			// OpenObject
#include "chklist.h"			// PopupMenu
#include "presparm.h"			// SetPresParams
#include "collect.h"			// StartCollector
#include "subj.h"			// Subject
#include "copyf.h"			// docopyf
#include "getnames.h"			// export_filename
#include "wrappers.h"			// xDosFindNext
#include "notebook.h"			// External compare/dircompare
#include "i18nutil.h"			// CommaFmtULL
#include "fortify.h"			// 06 May 08 SHL added
#include "excputil.h"			// xbeginthread
#include "info.h"			// driveflags

typedef struct
{
  CHAR filename[CCHMAXPATH];
  CHAR dirname[CCHMAXPATH];
  BOOL recurse;
}
SNAPSTUFF;

// Data definitions
static PSZ pszSrcFile = __FILE__;

#pragma data_seg(GLOBAL1)
BOOL fSelectedAlways;

/**
 * Write directory tree to snapshot file; recurse if requested
 */

static VOID SnapShot(char *path, FILE *fp, BOOL recurse)
{
  PFILEFINDBUF4L pffb;
  char *mask, *enddir;
  HDIR hdir = HDIR_CREATE;
  ULONG ulFindCnt;
  CHAR szCmmaFmtFileSize[81], szDate[DATE_BUF_BYTES];

  // 13 Aug 07 SHL fimxe to use FileToGet
  pffb = xmalloc(sizeof(FILEFINDBUF4L), pszSrcFile, __LINE__);
  if (pffb) {
    mask = xmalloc(CCHMAXPATH, pszSrcFile, __LINE__);
    if (mask) {
      BldFullPathName(mask, path, "*");
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
	  if (!(pffb->attrFile & FILE_DIRECTORY)) {
            ulltoa(pffb->cbFile, szCmmaFmtFileSize, 10);
	    FDateFormat(szDate, pffb->fdateLastWrite);
	    fprintf(fp,
		    "\"%s\",%u,%s,%s,%02u%s%02u%s%02u,%lu,%lu,N\n",
		    mask,
		    enddir - mask,
		    szCmmaFmtFileSize,
		    szDate,
		    pffb->ftimeLastWrite.hours,
		    TimeSeparator,
		    pffb->ftimeLastWrite.minutes,
		    TimeSeparator,
		    pffb->ftimeLastWrite.twosecs,
		    pffb->attrFile,
		    pffb->cbList > 4 ? pffb->cbList / 2 : 0);
	  }
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

/**
 * Write snapshot file thread
 * Write directory tree to snapshot file
 */

static VOID StartSnapThread(VOID *pargs)
{
  SNAPSTUFF *sf = (SNAPSTUFF *)pargs;
  FILE *fp;
  CHAR *modew = "w";

  if (sf) {
    if (*sf->dirname && *sf->filename) {
      priority_normal();
      ForwardslashToBackslash(sf->dirname);
      AddBackslashToPath(sf->dirname);
      fp = xfopen(sf->filename, modew, pszSrcFile, __LINE__, FALSE);
      if (fp) {
        fprintf(fp, "\"%s\"\n", sf->dirname);
        //DbgMsg(pszSrcFile, __LINE__, "recurse %i", sf->recurse);
	SnapShot(sf->dirname, fp, sf->recurse);
	fclose(fp);
      }
    }
    free(sf);
  }
}

/**
 * Compare files thread
 * Scan files and update container select flags
 */

static VOID CompareFilesThread(VOID *args)
{
  FCOMPARE fc;
  HAB hab2;
  HMQ hmq2;
  FILE *fp1, *fp2;
  ULONG len1, len2;
  ULONG offset = 0;
  LONG numread1, numread2;
  CHAR s[1024], ss[1024], *p1, *p2;
  CHAR *moderb = "rb";

  if (args) {
    fc = *(FCOMPARE *)args;
    hab2 = WinInitialize(0);
    if (hab2) {
#     ifdef FORTIFY
      Fortify_EnterScope();
#      endif
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
	fp1 = xfsopen(fc.file1, moderb, SH_DENYNO, pszSrcFile, __LINE__, TRUE);
	if (!fp1) {
	  sprintf(s, GetPString(IDS_COMPCANTOPENTEXT), fc.file1);
	  AddToListboxBottom(fc.hwndList, s);
	  WinSetWindowText(fc.hwndHelp, (CHAR *) GetPString(IDS_ERRORTEXT));
	}
	else {
	  fp2 = xfsopen(fc.file2, moderb, SH_DENYNO, pszSrcFile, __LINE__, TRUE);
	  if (!fp2) {
	    sprintf(s, GetPString(IDS_COMPCANTOPENTEXT), fc.file2);
	    AddToListboxBottom(fc.hwndList, s);
	    WinSetWindowText(fc.hwndHelp, (CHAR *) GetPString(IDS_ERRORTEXT));
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
			       (CHAR *) GetPString(IDS_COMPDONTMATCHTEXT));
	    }
	    else {
	      WinSetWindowText(fc.hwndHelp,
			       (CHAR *) GetPString(IDS_COMPCOMPARINGTEXT));
	      while (WinIsWindow(hab2, fc.hwndList)) {
		numread1 = fread(s, 1, 1024, fp1);
		numread2 = fread(ss, 1, 1024, fp2);
		if (numread1 != numread2 || feof(fp1) != feof(fp2)) {
		  sprintf(s, GetPString(IDS_COMPREADERRORTEXT),
			  offset, offset);
		  AddToListboxBottom(fc.hwndList, s);
		  WinSetWindowText(fc.hwndHelp, (CHAR *) GetPString(IDS_ERRORTEXT));
		  break;
		}
		else if (!numread1 && feof(fp1) && feof(fp2)) {
		  AddToListboxBottom(fc.hwndList,
				     GetPString(IDS_COMPFILESMATCHTEXT));
		  if (!stricmp(fc.file1, fc.file2))
		    AddToListboxBottom(fc.hwndList,
				       GetPString(IDS_COMPWONDERWHYTEXT));
		  WinSetWindowText(fc.hwndHelp,
				   (CHAR *) GetPString(IDS_COMPCOMPLETETEXT));
		  break;
		}
		else if (numread1 <= 0 || numread2 <= 0) {
		  if (offset == len1)
		    break;
		  else {
		    sprintf(s, GetPString(IDS_COMPMATCHREADERRORTEXT),
			    offset, offset);
		    WinSetWindowText(fc.hwndHelp,
				     (CHAR *) GetPString(IDS_COMPODDERRORTEXT));
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
				       (CHAR *) GetPString(IDS_COMPDONTMATCHTEXT));
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
#     ifdef FORTIFY
      Fortify_LeaveScope();
#      endif
    }
  }
}

/**
 * Select directories to compare dialog procedure
 */

MRESULT EXPENTRY CFileDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  FCOMPARE *fc;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2)
      WinDismissDlg(hwnd, 0);
    else {
      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      fc = (FCOMPARE *)mp2;
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
      if (xbeginthread(CompareFilesThread,
		       65536,
		       fc,
		       pszSrcFile,
		       __LINE__) == -1)
      {
	WinDismissDlg(hwnd, 0);
      }
    }
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, FCMP_HELP),
			(HPS)0, FALSE, TRUE);
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
    DosSleep(50);			// Let others die first
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/**
 * Action Thread
 * Do requested action on container contents
 */

static VOID ActionCnrThread(VOID *args)
{
  COMPARE *cmp = (COMPARE *)args;
  HAB hab;
  HMQ hmq;
  HWND hwndCnrS, hwndCnrD;
  PCNRITEM pciS, pciD, pciNextS, pciNextD;
  CHAR szNewName[CCHMAXPATH], szDirName[CCHMAXPATH], *p;
  APIRET rc;
  ITIMER_DESC itdSleep = { 0 };

  if (!cmp) {
    Runtime_Error(pszSrcFile, __LINE__, NULL);
    return;
  }

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize(0);
  if (hab) {
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
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

      pciS = WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPVOID,
		       MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
      pciD = WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPVOID,
			MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));

      InitITimer(&itdSleep, 500);	// Sleep every 500 mSec

      while (pciS && (INT)pciS != -1 && pciD && (INT)pciD != -1) {

	if (cmp->cmp->stop)
	  break;			// 27 Sep 09 SHL

	pciNextS = WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPFROMP(pciS),
			  MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
	pciNextD = WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPFROMP(pciD),
			   MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));

	// Process file if selected and not filtered
	if (*pciS->pszFileName &&
	     pciS->rc.flRecordAttr & CRA_SELECTED &&
	     ~pciS->rc.flRecordAttr & CRA_FILTERED)
	{
	  // Source name not blank
	  switch (cmp->action) {
	  case IDM_DELETE:
	    if (!unlinkf(pciS->pszFileName)) {
	      WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciS),
			 MPFROM2SHORT(FALSE, CRA_SELECTED));

	      if (!*pciD->pszFileName) {
		// Other side is blank - remove from both sides
		RemoveCnrItems(hwndCnrS, pciS, 1, CMA_FREE | CMA_INVALIDATE);
		if (pciD->rc.flRecordAttr & CRA_SELECTED)
		  WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciD),
			     MPFROM2SHORT(FALSE, CRA_SELECTED));
		RemoveCnrItems(hwndCnrD, pciD, 1, CMA_FREE | CMA_INVALIDATE);
	      }
	      else {
		// Other side is not blank - blank just this side
		FreeCnrItemData(pciS);
		// 29 Aug 08 SHL Point pci fields at NullStr to sync with FreeCnrItemData mods
		pciS->pszFileName = NullStr;
		pciS->pszDisplayName = pciS->pszFileName;
		pciS->rc.pszIcon = pciS->pszFileName;
		pciS->flags = 0;	// Just on one side
		WinSendMsg(hwndCnrS, CM_INVALIDATERECORD, MPFROMP(&pciS),
			   MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
		pciD->flags = 0;	// Just on one side
		if (pciD->pszSubject != NullStr) {
		  xfree(pciD->pszSubject, pszSrcFile, __LINE__);
		  pciD->pszSubject = NullStr;
		}
	      }
	      if (hwndCnrS == WinWindowFromID(cmp->hwnd, COMP_LEFTDIR))
		cmp->cmp->totalleft--;
	      else
		cmp->cmp->totalright--;
	    }
	    break;

	  case IDM_MOVE:
	    {
	      BOOL fResetVerify = FALSE;

	      if (hwndCnrS == WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR))
		BldFullPathName(szNewName, cmp->leftdir, pciS->pszDisplayName);
	      else
		BldFullPathName(szNewName, cmp->rightdir, pciS->pszDisplayName);
	      // Make directory if required
	      strcpy(szDirName, szNewName);
	      p = strrchr(szDirName, '\\');
	      if (fVerify && (driveflags[toupper(*szNewName) - 'A'] & DRIVE_WRITEVERIFYOFF ||
			      driveflags[toupper(*pciS->pszFileName) - 'A'] & DRIVE_WRITEVERIFYOFF)) {
		DosSetVerify(FALSE);
		fResetVerify = TRUE;
	      }
	      if (p) {
		if (p > szDirName + 2)
		  p++;
		*p = 0;
		if (IsFile(szDirName) == -1)
		  MassMkdir(hwndMain, szDirName);
	      }
	      rc = docopyf(MOVE, pciS->pszFileName, szNewName);
	      if (fResetVerify) {
		DosSetVerify(fVerify);
		fResetVerify = FALSE;
	      }
	      if (!rc && stricmp(pciS->pszFileName, szNewName)) {
		WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciS),
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
		pciD->pszLongName = pciS->pszLongName;
		pciS->pszLongName = NullStr;	// 07 Sep 08 SHL avoid aliased pointer
		if (pciD->pszSubject != NullStr) {
		  xfree(pciD->pszSubject, pszSrcFile, __LINE__);
		  pciD->pszSubject = NullStr;
		}
		pciD->attrFile = pciS->attrFile;
		pciD->pszDispAttr = pciS->pszDispAttr;
		pciD->flags = 0;		// Just on one side
		pciD->date = pciS->date;
		pciD->time = pciS->time;
		pciD->ladate = pciS->ladate;
		pciD->latime = pciS->latime;
		pciD->crdate = pciS->crdate;
		pciD->crtime = pciS->crtime;
                pciD->pszFmtFileSize = pciS->pszFmtFileSize;
		pciD->cbFile = pciS->cbFile;
		pciD->easize = pciS->easize;

		if (pciS->pszFileName != NullStr) {
		  xfree(pciS->pszFileName, pszSrcFile, __LINE__);
		  pciS->pszFileName = NullStr;
		  pciS->pszDisplayName = pciS->pszFileName;
		  pciS->rc.pszIcon = pciS->pszFileName;
		}
		if (pciS->pszSubject != NullStr) {
		  xfree(pciS->pszSubject, pszSrcFile, __LINE__);
		  pciS->pszSubject = NullStr;
		}
		pciS->flags = 0;		// Just on one side

		WinSendMsg(hwndCnrS, CM_INVALIDATERECORD, MPFROMP(&pciS),
			   MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));

		WinSendMsg(hwndCnrD, CM_INVALIDATERECORD, MPFROMP(&pciD),
			   MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));

		if (hwndCnrS == WinWindowFromID(cmp->hwnd, COMP_LEFTDIR))
		  cmp->cmp->totalleft--;
		else
		  cmp->cmp->totalright--;
	      }
	      else if (rc) {
		rc = Dos_Error(MB_ENTERCANCEL,
			       rc,
			       HWND_DESKTOP,
			       pszSrcFile,
			       __LINE__,
			       GetPString(IDS_COMPMOVEFAILEDTEXT),
			       pciS->pszFileName, szNewName);
		if (rc == MBID_CANCEL)	// Cause loop to break
		  pciNextS = NULL;
	      }
	      break;
	    }

	  case IDM_COPY:
	    {
	      BOOL fResetVerify = FALSE;

	      if (hwndCnrS == WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR))
		BldFullPathName(szNewName, cmp->leftdir, pciS->pszDisplayName);
	      else
		BldFullPathName(szNewName, cmp->rightdir, pciS->pszDisplayName);
	      // Make directory if required
	      strcpy(szDirName, szNewName);
	      p = strrchr(szDirName, '\\');
	      if (fVerify && (driveflags[toupper(*szNewName) - 'A'] & DRIVE_WRITEVERIFYOFF ||
				driveflags[toupper(*pciS->pszFileName) - 'A'] & DRIVE_WRITEVERIFYOFF)) {
		DosSetVerify(FALSE);
		fResetVerify = TRUE;
	      }
	      if (p) {
		if (p > szDirName + 2)
		  p++;
		*p = 0;
		if (IsFile(szDirName) == -1)
		  MassMkdir(hwndMain, szDirName);
	      }
	      rc = docopyf(COPY, pciS->pszFileName, szNewName);
	      if (fResetVerify) {
		DosSetVerify(fVerify);
		fResetVerify = FALSE;
	      }
	      if (rc) {
		rc = Dos_Error(MB_ENTERCANCEL,
			       rc,
			       HWND_DESKTOP,
			       pszSrcFile,
			       __LINE__,
			       GetPString(IDS_COMPCOPYFAILEDTEXT),
			       pciS->pszFileName, szNewName);
		if (rc == MBID_CANCEL)
		  pciNextS = NULL;	// Cause loop to break
	      }
	      else {
		WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciS),
			   MPFROM2SHORT(FALSE, CRA_SELECTED));
		if (pciD->rc.flRecordAttr & CRA_SELECTED)
		  WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciD),
			     MPFROM2SHORT(FALSE, CRA_SELECTED));
		if (~pciD->flags & CNRITEM_EXISTS) {
		  if (hwndCnrD == WinWindowFromID(cmp->hwnd, COMP_LEFTDIR))
		    cmp->cmp->totalleft++;
		  else
		    cmp->cmp->totalright++;
		}
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
		pciD->attrFile = pciS->attrFile;
		pciD->pszDispAttr = pciS->pszDispAttr;
		pciD->flags = CNRITEM_EXISTS;	// Now on both sides
		pciD->date = pciS->date;
		pciD->time = pciS->time;
		pciD->ladate = pciS->ladate;
		pciD->latime = pciS->latime;
		pciD->crdate = pciS->crdate;
                pciD->crtime = pciS->crtime;
                pciD->pszFmtFileSize = pciS->pszFmtFileSize;
		pciD->cbFile = pciS->cbFile;
		pciD->easize = pciS->easize;

		// Forget status until we regenerate it
		if (pciS->pszSubject != NullStr) {
		  xfree(pciS->pszSubject, pszSrcFile, __LINE__);
		  pciS->pszSubject = NullStr;
		}
		pciS->flags = CNRITEM_EXISTS;	// Now on both sides

		WinSendMsg(hwndCnrS, CM_INVALIDATERECORD, MPFROMP(&pciS),
			   MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
		WinSendMsg(hwndCnrD, CM_INVALIDATERECORD, MPFROMP(&pciD),
			   MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
	      }
	      break;
	    }

	  default:
	    break;
	  } // switch

	} // if have name

	pciS = pciNextS;
	pciD = pciNextD;

	SleepIfNeeded(&itdSleep, 0);
      }	// while
      WinPostMsg(cmp->hwnd, WM_TIMER, MPFROMLONG(ID_COMP_TIMER), 0);	// Force update
    Abort:
      WinDestroyMsgQueue(hmq);
    }
    PostMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPFROMLONG(1), MPVOID);
    DecrThreadUsage();
    free(cmp);
    WinTerminate(hab);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
  }
  else
    xfree(cmp, pszSrcFile, __LINE__);
}

static VOID CompSelect(HWND hwndCnrS, HWND hwndCnrD, HWND hwnd, INT action, USHORT shiftstate, BOOL *stop);

/**
 * Update container selection flags thread
 */

static VOID SelectCnrsThread(VOID *args)
{
  COMPARE *cmp = (COMPARE *)args;
  HAB hab;
  HMQ hmq;

  if (!cmp) {
    Runtime_Error(pszSrcFile, __LINE__, NULL);
    return;
  }

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize(0);
  if (hab) {
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
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
	CompSelect(WinWindowFromID(cmp->hwnd, COMP_LEFTDIR),
		   WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR),
		   cmp->hwnd,
		   cmp->action,
		   cmp->shiftstate,
		   &cmp->cmp->stop);
	break;
      }
      if (!PostMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPFROMLONG(1L), MPVOID))
	WinSendMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPFROMLONG(1L), MPVOID);
      WinDestroyMsgQueue(hmq);
    }
    DecrThreadUsage();
    free(cmp);
    WinTerminate(hab);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
  }
  else
    free(cmp);
}

/**
 * Set item selections for CompSelect
 */

static VOID CompSelectSetSelects(PCNRITEM pciS, PCNRITEM pciD, BOOL matchS, BOOL matchD, BOOL wantAnd);

static VOID CompSelectSetSelects(PCNRITEM pciS, PCNRITEM pciD, BOOL matchS, BOOL matchD, BOOL wantAnd)
{
  ULONG oldSel;
  ULONG newSel;

  oldSel = pciS->rc.flRecordAttr & CRA_SELECTED;
  newSel = matchS ? (wantAnd ? oldSel : CRA_SELECTED) : (wantAnd ? 0 : oldSel);
  if ((pciS->rc.flRecordAttr & CRA_SELECTED) != newSel)
    WinSendMsg(pciS->hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pciS),
	       MPFROM2SHORT(newSel ? TRUE : FALSE, CRA_SELECTED));

  oldSel = pciD->rc.flRecordAttr & CRA_SELECTED;
  newSel = matchD ? (wantAnd ? oldSel : CRA_SELECTED) : (wantAnd ? 0 : oldSel);
  if ((pciD->rc.flRecordAttr & CRA_SELECTED) != newSel) {
    WinSendMsg(pciD->hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pciD),
	       MPFROM2SHORT(newSel ? TRUE : FALSE, CRA_SELECTED));
  }
}

/**
 * Clear item selections for CompSelect
 */

static BOOL CompSelectClearSelects(PCNRITEM pciS, PCNRITEM pciD, BOOL matchS, BOOL matchD);

static BOOL CompSelectClearSelects(PCNRITEM pciS, PCNRITEM pciD, BOOL matchS, BOOL matchD)
{
  BOOL changed;

  if ((pciS->rc.flRecordAttr & CRA_SELECTED) && matchS) {
    WinSendMsg(pciS->hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pciS),
	       MPFROM2SHORT(FALSE, CRA_SELECTED));
    changed = TRUE;
  }
  else
    changed = FALSE;

  if ((pciD->rc.flRecordAttr & CRA_SELECTED) && matchD) {
    WinSendMsg(pciD->hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pciD),
	       MPFROM2SHORT(FALSE, CRA_SELECTED));
    changed = TRUE;
  }

  return changed;
}

/**
 * Do select actions for compare directories containers
 * @param action is select mode
 */

static VOID CompSelect(HWND hwndCnrS, HWND hwndCnrD, HWND hwnd, INT action, USHORT shiftstate, BOOL *stop)
{
  PCNRITEM pciS;
  PCNRITEM pciD;
  PCNRITEM *pciSa = NULL;
  PCNRITEM *pciDa = NULL;
  CNRINFO cnri;
  BOOL slow = FALSE;
  UINT x;
  UINT numD;
  UINT numS;
  ITIMER_DESC itdSleep = { 0 };
  BOOL fUpdateHideButton = FALSE;
  BOOL wantAnd = (shiftstate & (KC_SHIFT | KC_ALT | KC_CTRL)) == KC_CTRL;
  BOOL matched;

  if (!hwndCnrS || !hwndCnrD) {
    Runtime_Error(pszSrcFile, __LINE__, "hwndCnrS %p hwndCnrD %p", hwndCnrS, hwndCnrD);
    return;
  }

  memset(&cnri, 0, sizeof(CNRINFO));
  cnri.cb = sizeof(CNRINFO);
  WinSendMsg(hwndCnrD, CM_QUERYCNRINFO, MPFROMP(&cnri),
	     MPFROMLONG(sizeof(CNRINFO)));
  numD = cnri.cRecords;
  memset(&cnri, 0, sizeof(CNRINFO));
  cnri.cb = sizeof(CNRINFO);
  WinSendMsg(hwndCnrS, CM_QUERYCNRINFO, MPFROMP(&cnri),
	     MPFROMLONG(sizeof(CNRINFO)));
  numS = cnri.cRecords;
  if (!numD || numS != numD) {
    Runtime_Error(pszSrcFile, __LINE__, "numD %u != numS %u", numD, numS);
    return;
  }

  pciDa = xmalloc(sizeof(PCNRITEM) * numD, pszSrcFile, __LINE__);
  if (!pciDa)
    return;

  pciSa = xmalloc(sizeof(PCNRITEM) * numS, pszSrcFile, __LINE__);
  if (!pciSa) {
    free(pciDa);
    return;
  }

  InitITimer(&itdSleep, 500);		// Sleep every 500 mSec

Restart:

  memset(pciDa, 0, sizeof(PCNRITEM) * numD);
  memset(pciSa, 0, sizeof(PCNRITEM) * numS);

  pciD = (PCNRITEM)WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPVOID,
			      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  x = 0;
  while (pciD && (INT)pciD != -1 && x < numD) {
    pciDa[x] = pciD;
    x++;
    if (!slow)
      pciD = (PCNRITEM) pciD->rc.preccNextRecord;
    else
      pciD = (PCNRITEM) WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPFROMP(pciD),
				   MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    SleepIfNeeded(&itdSleep, 0);
  } // while

  if (numD != x) {
    // Something out of sync - fixme to document why slow logic needed
    if (!slow) {
      slow = TRUE;
      goto Restart;
    }
    free(pciDa);
    free(pciSa);
    Runtime_Error(pszSrcFile, __LINE__, "numD %u != x %lu", numD, x);
    return;
  }

  pciS = (PCNRITEM) WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPVOID,
			       MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  x = 0;
  while (pciS && (INT)pciS != -1 && x < numS) {
    pciSa[x] = pciS;
    x++;
    if (!slow)
      pciS = (PCNRITEM) pciS->rc.preccNextRecord;
    else
      pciS = (PCNRITEM) WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPFROMP(pciS),
				   MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    SleepIfNeeded(&itdSleep, 0);
  } // while

  if (numS != x) {
    if (!slow) {
      slow = TRUE;
      goto Restart;
    }
    free(pciSa);
    free(pciDa);
    Runtime_Error(pszSrcFile, __LINE__, "numS (%lu) != x (%lu)", numS, x);
    return;
  }

  switch (action) {
  case IDM_SELECTIDENTICAL:
    // Same Date/size
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	matched = pciS->flags & CNRITEM_EXISTS &&
		  ~pciS->flags & CNRITEM_SMALLER &&
		  ~pciS->flags & CNRITEM_LARGER &&
		  ~pciS->flags & CNRITEM_NEWER &&
		  ~pciS->flags & CNRITEM_OLDER;
	CompSelectSetSelects(pciS, pciDa[x], matched, matched, wantAnd);
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_SELECTSAME:
    // Same Size
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	matched = pciS->flags & CNRITEM_EXISTS &&
		  ~pciS->flags & CNRITEM_SMALLER &&
		  ~pciS->flags & CNRITEM_LARGER;
	CompSelectSetSelects(pciS, pciDa[x], matched, matched, wantAnd);
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_SELECTSAMECONTENT:
    for (x = 0; x < numS && !*stop; x++) {

      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	matched = FALSE;
	pciD = pciDa[x];
	if (pciS->flags & CNRITEM_EXISTS) {
	  FILE *fp1 = NULL;
	  FILE *fp2 = NULL;
	  UINT errLineNo = 0;
	  UINT compErrno = 0;
	  CHAR buf1[1024];
	  CHAR buf2[1024];
	  HAB hab = WinQueryAnchorBlock(hwndCnrS);
	  CHAR *moderb = "rb";

	  if (!*pciS->pszFileName ||
	      !*pciD->pszFileName) {
	    Runtime_Error(pszSrcFile, __LINE__,
			  "CNRITEM_EXISTS set with null file name for index %u", x);
	    break;
	  }

	  fp1 = xfsopen(pciS->pszFileName, moderb, SH_DENYNO, pszSrcFile, __LINE__, TRUE);
	  if (!fp1) {
	    errLineNo = __LINE__;
	    compErrno = errno;
	  }
	  else {
	    fp2 = xfsopen(pciD->pszFileName, moderb, SH_DENYNO, pszSrcFile, __LINE__, TRUE);
	    if (!fp2) {
	      errLineNo = __LINE__;
	      compErrno = errno;
	    }
	    else {
	      size_t len1 = filelength(fileno(fp1));
	      size_t len2 = filelength(fileno(fp2));
	      if (len1 == len2) {
		setbuf(fp1, NULL);
		setbuf(fp2, NULL);
		while (WinIsWindow(hab, hwndCnrS)) {
		  size_t numread1 = fread(buf1, 1, 1024, fp1);
		  size_t numread2 = fread(buf2, 1, 1024, fp2);

		  if (!numread1 || !numread2 || numread1 != numread2) {
		    if (ferror(fp1) || ferror(fp2)) {
		      errLineNo = __LINE__;
		      compErrno = errno;
		    }
		    else if (feof(fp1) && feof(fp2))
		      matched = TRUE;
		    break;
		  }
		  else if (memcmp(buf1, buf2, numread1))
		    break;
		}	// while
	      } // same len
	    } // if open ok
	  } // if open ok
	  if (fp1)
	    fclose(fp1);
	  if (fp2)
	    fclose(fp2);

	  if (errLineNo) {
	    Runtime_Error(pszSrcFile, errLineNo,
			  "error %d while comparing", compErrno);
	  }
	} // if exists
	CompSelectSetSelects(pciS, pciD, matched, matched, wantAnd);
      } // if not filtered
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_SELECTBOTH:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	matched = pciS->flags & CNRITEM_EXISTS;
	CompSelectSetSelects(pciS, pciD, matched, matched, wantAnd);
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_SELECTONE:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	CompSelectSetSelects(pciS,
			     pciD,
			     ~pciS->flags & CNRITEM_EXISTS && *pciS->pszFileName,
			     ~pciD->flags & CNRITEM_EXISTS && *pciD->pszFileName,
			     wantAnd);
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_SELECTBIGGER:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	CompSelectSetSelects(pciS,
			     pciD,
			     pciS->flags & CNRITEM_LARGER,
			     pciD->flags & CNRITEM_LARGER,
			     wantAnd);
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_SELECTSMALLER:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	CompSelectSetSelects(pciS,
			     pciD,
			     pciS->flags & CNRITEM_SMALLER,
			     pciD->flags & CNRITEM_SMALLER,
			     wantAnd);
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_SELECTNEWER:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	CompSelectSetSelects(pciS,
			     pciD,
			     pciS->flags & CNRITEM_NEWER,
			     pciD->flags & CNRITEM_NEWER,
			     wantAnd);
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_SELECTOLDER:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	CompSelectSetSelects(pciS,
			     pciD,
			     pciS->flags & CNRITEM_OLDER,
			     pciD->flags & CNRITEM_OLDER,
			     wantAnd);
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_DESELECTBOTH:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	matched = pciS->flags & CNRITEM_EXISTS;
	if (CompSelectClearSelects(pciS, pciDa[x], matched, matched))
	  fUpdateHideButton = TRUE;
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_DESELECTONE:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	if (CompSelectClearSelects(pciS,
				   pciD,
				   ~pciS->flags & CNRITEM_EXISTS && *pciS->pszFileName,
				   ~pciD->flags & CNRITEM_EXISTS && *pciD->pszFileName)) {
	  fUpdateHideButton = TRUE;
	}
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_DESELECTBIGGER:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	if (CompSelectClearSelects(pciS,
				   pciD,
				   pciS->flags & CNRITEM_LARGER,
				   pciD->flags & CNRITEM_LARGER)) {
	  fUpdateHideButton = TRUE;
	}
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_DESELECTSMALLER:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	if (CompSelectClearSelects(pciS,
				   pciD,
				   pciS->flags & CNRITEM_SMALLER,
				   pciD->flags & CNRITEM_SMALLER)) {
	  fUpdateHideButton = TRUE;
	}
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_DESELECTNEWER:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	if (CompSelectClearSelects(pciS,
				   pciD,
				   pciS->flags & CNRITEM_NEWER,
				   pciD->flags & CNRITEM_NEWER)) {
	  fUpdateHideButton = TRUE;
	}
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  case IDM_DESELECTOLDER:
    for (x = 0; x < numS; x++) {
      pciS = pciSa[x];
      if (~pciS->rc.flRecordAttr & CRA_FILTERED) {
	pciD = pciDa[x];
	if (CompSelectClearSelects(pciS,
				   pciD,
				   pciS->flags & CNRITEM_OLDER,
				   pciD->flags & CNRITEM_OLDER)) {
	  fUpdateHideButton = TRUE;
	}
      }
      SleepIfNeeded(&itdSleep, 0);
    } // for
    break;

  default:
    break;
  } // switch action

  free(pciSa);
  free(pciDa);

  if (fUpdateHideButton) {
    if (WinQueryButtonCheckstate(hwnd,COMP_HIDENOTSELECTED) == 1)
      WinCheckButton(hwnd, COMP_HIDENOTSELECTED, 2);
  }

  WinPostMsg(hwnd, WM_TIMER, MPFROMLONG(ID_COMP_TIMER), 0);	// Force update
  DosPostEventSem(CompactSem);
}

/**
 * Build FILELIST given pathname
 */

static VOID FillDirList(CHAR *str, UINT skiplen, BOOL recurse,
			FILELIST ***list, UINT *pnumfiles, UINT *pnumalloc)
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
  static BOOL fDone;
  ITIMER_DESC itdSleep = { 0 };		// 30 May 11 GKY

  if (!str || !*str) {
    Runtime_Error(pszSrcFile, __LINE__, NULL);
    return;
  }

  maskstr = xmalloc(CCHMAXPATH + 100, pszSrcFile, __LINE__);
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
    InitITimer(&itdSleep, 500);
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
	    FillDirList(maskstr, skiplen, recurse, list, pnumfiles, pnumalloc);
	  }
	}
	else {
	  if (fForceUpper)
	    strupr(pffbFile->achName);
	  else if (fForceLower)
	    strlwr(pffbFile->achName);
	  memcpy(enddir, pffbFile->achName, pffbFile->cchName + 1);
	  if (strlen(maskstr) > CCHMAXPATH) {
	    // Complain if pathnames exceeds max
	    DosFindClose(hDir);
	    free(pffbArray);
	    free(maskstr);
	    if (!fDone) {
	      fDone = TRUE;
	      saymsg(MB_OK | MB_ICONASTERISK,
		     HWND_DESKTOP,
		     GetPString(IDS_WARNINGTEXT),
		     GetPString(IDS_LENGTHEXCEEDSMAXPATHTEXT));
	    }
	    return;
	  }
	  if (AddToFileList(maskstr + skiplen,
			    pffbFile, list, pnumfiles, pnumalloc)) {
	    goto Abort;
	  }
	}
	pffbFile = (PFILEFINDBUF4L)((PBYTE)pffbFile + pffbFile->oNextEntryOffset);
      } // for
      DosError(FERR_DISABLEHARDERR);
      ulFindCnt = FilesToGet;
      rc = xDosFindNext(hDir, pffbArray, ulBufBytes, &ulFindCnt, FIL_QUERYEASIZEL);
      SleepIfNeeded(&itdSleep, 1);
    } while (!rc);

Abort:

    DosFindClose(hDir);
    DosSleep(0);
  }

  if (rc && rc != ERROR_NO_MORE_FILES) {
    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      GetPString(IDS_CANTFINDDIRTEXT), maskstr);
  }

  xfree(maskstr, pszSrcFile, __LINE__);
  xfree(pffbArray, pszSrcFile, __LINE__);
}

#define GetHwndLeft(h)	(WinWindowFromID(h,COMP_LEFTDIR))
#define GetHwndRight(h)	(WinWindowFromID(h,COMP_RIGHTDIR))

/**
 * Compare names for qsort
 */

static int CompNames(const void *n1, const void *n2)
{
  FILELIST *fl1 = *(FILELIST **)n1;
  FILELIST *fl2 = *(FILELIST **)n2;

  return stricmp(fl1->fname, fl2->fname);
}

/**
 * Fill left and right containers
 */

static VOID FillCnrsThread(VOID *args)
{
  COMPARE *cmp = (COMPARE *)args;
  HAB hab;
  HMQ hmq;
  BOOL notified = FALSE;
  ITIMER_DESC itdSleep = { 0 };

  CHAR szBuf[CCHMAXPATH];
  CNRINFO cnri;

# ifdef FORTIFY
  // 10 May 08 SHL fixme to suppress W111
  Fortify_EnterScope();
#  endif

  if (!cmp) {
    Runtime_Error(pszSrcFile, __LINE__, NULL);
#   ifdef FORTIFY
    // 10 May 08 SHL fixme to suppress W111
    Fortify_LeaveScope();
#    endif
    return;				// 10 Dec 08 SHL was _endthread
  }

  DosError(FERR_DISABLEHARDERR);

  InitITimer(&itdSleep, 500);		// Sleep every 500 mSec

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
      UINT l;
      UINT r;
      // UINT cntr;
      FILELIST **filesl = NULL;
      FILELIST **filesr = NULL;
      UINT numallocl = 0;
      UINT numallocr = 0;
      UINT lenl;			// Directory prefix length
      UINT lenr;
      ULONG ulRecsNeeded;
      CHAR *pch;
      HWND hwndLeft;
      HWND hwndRight;

      WinCancelShutdown(hmq, TRUE);
      IncrThreadUsage();

      hwndLeft = GetHwndLeft(cmp->hwnd);
      hwndRight = GetHwndRight(cmp->hwnd);
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
      cmp->cmp->totalleft = 0;
      cmp->cmp->totalright = 0;

      // Build list of all files in left directory
      if (fForceLower)
	strlwr(cmp->leftdir);
      else if (fForceUpper)
	strupr(cmp->leftdir);
      FillDirList(cmp->leftdir, lenl, cmp->includesubdirs,
		  &filesl, &cmp->cmp->totalleft, &numallocl);

      if (filesl)
	qsort(filesl, cmp->cmp->totalleft, sizeof(CHAR *), CompNames);

      // Build list of all files in right directory
      //DbgMsg(pszSrcFile, __LINE__, "list file %s", cmp->rightlist);
      if (!*cmp->rightlist) {
	if (fForceLower)
	  strlwr(cmp->rightdir);
	else if (fForceUpper)
	  strupr(cmp->rightdir);
	FillDirList(cmp->rightdir, lenr, cmp->includesubdirs,
                    &filesr, &cmp->cmp->totalright, &numallocr);
      }
      else {
	// Use snapshot file
	FILE *fp;
	FILEFINDBUF4L fb4;
	CHAR str[CCHMAXPATH * 2], *p;
        CHAR *moder = "r";

	memset(&fb4, 0, sizeof(fb4));
	fp = xfopen(cmp->rightlist, moder, pszSrcFile, __LINE__, FALSE);
        if (fp) {
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
          WinSetDlgItemText(cmp->hwnd, COMP_LISTLOADED, "List File Loaded");
	  cnri.pszCnrTitle = cmp->rightdir;
	  if (!WinSendMsg(hwndRight, CM_SETCNRINFO,
		     MPFROMP(&cnri), MPFROMLONG(CMA_CNRTITLE))) {
            Win_Error(hwndRight, cmp->hwnd, pszSrcFile, __LINE__, "CM_SETCNRINFO");
            WinSetDlgItemText(cmp->hwnd, COMP_LISTLOADED, "");
	  }

	  if (*cmp->rightdir) {
	    lenr = strlen(cmp->rightdir);
	    if (cmp->rightdir[strlen(cmp->rightdir) - 1] != '\\')
              lenr++;
            //DbgMsg(pszSrcFile, __LINE__, "end of file %i", feof(fp));
            while (!feof(fp)) {
	      if (!xfgets_bstripcr
                  (str, sizeof(str), fp, pszSrcFile, __LINE__)) {
                break;
              }
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
			fb4.cbFile = atoll(p);
			p = strchr(p, ',');
                        if (p) {
                          p++;
                          if (ulDateFmt == 2 || ulDateFmt == 3)
                            fb4.fdateLastWrite.year = atol(p) - 1980;
                          if (ulDateFmt == 1)
                            fb4.fdateLastWrite.day = atol(p);
                          else
                            fb4.fdateLastWrite.month = atol(p);
			  p = strchr(p, DateSeparator[0]);
                          if (p) {
                            p++;
                            if (ulDateFmt == 2 || ulDateFmt == 3)
                              fb4.fdateLastWrite.month = atol(p);
                            else
                              fb4.fdateLastWrite.day = atol(p);
			    p = strchr(p, DateSeparator[0]);
                            if (p) {
                              p++;
                              if (ulDateFmt == 2)
                                fb4.fdateLastWrite.day = atol(p);
                              if (ulDateFmt == 3)
                                fb4.fdateLastWrite.month = atol(p);
                              else
                                fb4.fdateLastWrite.year = atol(p) - 1980;
			      p = strchr(p, ',');
                              if (p) {
				p++;
				fb4.ftimeLastWrite.hours = atol(p);
                                p = strchr(p, TimeSeparator[0]);
                                if (p) {
				  p++;
				  fb4.ftimeLastWrite.minutes = atol(p);
				  p = strchr(p, TimeSeparator[0]);
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
							  &cmp->cmp->totalright,
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
        qsort(filesr, cmp->cmp->totalright, sizeof(CHAR *), CompNames);

      // We now have two lists of files, both sorted.
      // Count total number of container entries required on each side
      l = 0;
      r = 0;
      ulRecsNeeded = 0;
      while ((filesl && filesl[l]) || (filesr && filesr[r])) {

	if (cmp->stop)
	  break;			// Cancel requested

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

	ulRecsNeeded++;			// Count how many entries req'd

      }	// while counting

      if (cmp->stop)
	ulRecsNeeded = 0;

      // Insert records into the containers

      if (ulRecsNeeded) {

	PCNRITEM pcilFirst;
	PCNRITEM pcirFirst;
	PCNRITEM pcil = NULL;
	PCNRITEM pcir = NULL;
	INT ret;
	ULONG ulRecsAllocated = 0;
	ULONG insertedl;
	ULONG insertedr;

	l = 0;
	r = 0;

	// Use send to get message on screen quickly
	WinSendMsg(cmp->hwnd, UM_CONTAINERHWND, MPVOID, MPVOID);

	cmp->cmp->totalleft = 0;
	cmp->cmp->totalright = 0;

	while ((filesl && filesl[l]) || (filesr && filesr[r])) {

	  ULONG ulRecsToInsert;	// limited to USHRT_MAX

	  if (cmp->stop)
	    break;

	  // Check alloc needed
	  if (!pcil || !pcir) {
	    if (pcil != pcir) {
	      // 2011-05-29 SHL	fixme to GetPString
	      Runtime_Error(pszSrcFile, __LINE__, "pcil and pcir out of sync");
	      cmp->stop = TRUE;
	      break;
	    }
	    ulRecsToInsert = ulRecsNeeded - ulRecsAllocated;
	    if (ulRecsToInsert > USHRT_MAX)
	      ulRecsToInsert = USHRT_MAX;

	    pcilFirst = WinSendMsg(hwndLeft,
				   CM_ALLOCRECORD,
				   MPFROMLONG(EXTRA_RECORD_BYTES),
				   MPFROMLONG(ulRecsToInsert));
	    if (!pcilFirst) {
	      Win_Error(hwndLeft, cmp->hwnd, pszSrcFile, __LINE__, PCSZ_CM_ALLOCRECORD);
	      cmp->stop = TRUE;
	      break;
	    }
	    pcirFirst = WinSendMsg(hwndRight, CM_ALLOCRECORD,
				   MPFROMLONG(EXTRA_RECORD_BYTES),
				   MPFROMLONG(ulRecsToInsert));
	    if (!pcirFirst) {
	      Win_Error(hwndRight, cmp->hwnd, pszSrcFile, __LINE__, PCSZ_CM_ALLOCRECORD);
	      FreeCnrItemList(hwndLeft, pcilFirst);
	      pcilFirst = NULL;
	      cmp->stop = TRUE;
	      break;
	    }
	    pcil = pcilFirst;
	    pcir = pcirFirst;
	    insertedl = 0;
	    insertedr = 0;
	    ulRecsAllocated += ulRecsToInsert;
	  } // if need alloc

	  pcir->hwndCnr = hwndRight;
	  pcir->rc.hptrIcon = (HPOINTER)0;
	  pcil->hwndCnr = hwndLeft;
	  pcil->rc.hptrIcon = (HPOINTER)0;

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
	    cmp->cmp->totalleft++;
	    insertedl++;
	    BldFullPathName(szBuf, cmp->leftdir, filesl[l]->fname);
	    pcil->pszFileName = xstrdup(szBuf, pszSrcFile, __LINE__);
	    pcil->pszDisplayName = pcil->pszFileName + lenl;
	    pcil->attrFile = filesl[l]->attrFile;
	    pcil->pszDispAttr = FileAttrToString(pcil->attrFile);
	    pcil->cbFile = filesl[l]->cbFile;
	    // 12 Jan 08 SHL fixme to use cached size here too
	    CommaFmtULL(szBuf, sizeof(szBuf), pcil->cbFile, ' ');
	    pcil->pszFmtFileSize = xstrdup(szBuf, pszSrcFile, __LINE__);
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
	      if (!Filter((PMINIRECORDCORE)pcil, (PVOID)&cmp->dcd.mask)) {
		pcil->rc.flRecordAttr |= CRA_FILTERED;
		pcir->rc.flRecordAttr |= CRA_FILTERED;
	      }
	    }
	  } // if on left

	  if (x >= 0) {
	    // File appears on right side
	    cmp->cmp->totalright++;
	    insertedr++;
	    BldFullPathName(szBuf, cmp->rightdir, filesr[r]->fname);
	    pcir->pszFileName = xstrdup(szBuf, pszSrcFile, __LINE__);	// 31 Jul 07 SHL
	    pcir->pszDisplayName = pcir->pszFileName + lenr;
	    pcir->attrFile = filesr[r]->attrFile;
	    // pcir->rc.hptrIcon = hptrFile;
	    pcir->pszDispAttr = FileAttrToString(pcir->attrFile);
	    pcir->cbFile = filesr[r]->cbFile;
	    // 12 Jan 08 SHL fixme to used cached size here too
	    CommaFmtULL(szBuf, sizeof(szBuf), pcir->cbFile, ' ');
	    pcir->pszFmtFileSize = xstrdup(szBuf, pszSrcFile, __LINE__);
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
	    if (ret == 1) {
	      pcil->flags |= CNRITEM_NEWER;
	      pcir->flags |= CNRITEM_OLDER;
	      if (pch != szBuf) {
		strcpy(pch, ", ");
		pch += 2;
	      }
	      strcpy(pch, GetPString(IDS_NEWERTEXT));
	      pch += 5;
	    }
	    else if (ret == -1)	{
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

	  if (x <= 0)
	    free(filesl[l++]);		// Done with item on left

	  if (x >= 0)
	    free(filesr[r++]);		// Done with item on right

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
	  if (!pcir->pszSubject)
	    pcir->pszSubject = NullStr;

	  if (!pcil->pszDispAttr)
	    pcil->pszDispAttr = NullStr;
	  if (!pcir->pszDispAttr)
	    pcir->pszDispAttr = NullStr;

	  // Avoid hogging systems
	  SleepIfNeeded(&itdSleep, 0);

	  pcil = (PCNRITEM)pcil->rc.preccNextRecord;
	  pcir = (PCNRITEM)pcir->rc.preccNextRecord;

	  if (pcil == NULL || pcir == NULL) {
	    RECORDINSERT ri;
	    if (pcil != pcir) {
	      Runtime_Error(pszSrcFile, __LINE__, "pcil and pcir out of sync");
	      cmp->stop = TRUE;
	      break;
	    }
	    // Say inserting
	    WinSendMsg(cmp->hwnd, UM_CONTAINERDIR, MPVOID, MPVOID);

	    // Insert left side
	    memset(&ri, 0, sizeof(RECORDINSERT));
	    ri.cb = sizeof(RECORDINSERT);
	    ri.pRecordOrder = (PRECORDCORE)CMA_END;
	    ri.pRecordParent = (PRECORDCORE)NULL;
	    ri.zOrder = (ULONG)CMA_TOP;
	    ri.cRecordsInsert = ulRecsToInsert;
	    ri.fInvalidateRecord = FALSE;

	    if (!WinSendMsg(hwndLeft, CM_INSERTRECORD,
			    MPFROMP(pcilFirst), MPFROMP(&ri))) {
	      Win_Error(hwndLeft, cmp->hwnd, pszSrcFile, __LINE__, "CM_INSERTRECORD");
	      FreeCnrItemList(hwndLeft, pcilFirst);
	      cmp->cmp->totalleft -= insertedl;
	    }
	    pcilFirst = NULL;

	    // Insert right side
	    memset(&ri, 0, sizeof(RECORDINSERT));
	    ri.cb = sizeof(RECORDINSERT);
	    ri.pRecordOrder = (PRECORDCORE)CMA_END;
	    ri.pRecordParent = (PRECORDCORE)NULL;
	    ri.zOrder = (ULONG)CMA_TOP;
	    ri.cRecordsInsert = ulRecsToInsert;
	    ri.fInvalidateRecord = FALSE;

	    if (!WinSendMsg(hwndRight, CM_INSERTRECORD,
			    MPFROMP(pcirFirst), MPFROMP(&ri))) {
	      Win_Error(hwndRight, cmp->hwnd, pszSrcFile, __LINE__, "CM_INSERTRECORD");
	      // 2011-05-29 SHL	fixme?
	      RemoveCnrItems(hwndLeft, NULL, 0, CMA_FREE | CMA_INVALIDATE);
	      FreeCnrItemList(hwndRight, pcirFirst);
	      cmp->cmp->totalright -= insertedr;
	    }
	    pcirFirst = NULL;
	  } // if need insert

	} // while filling left or right

	// If stopped early clean up

	if (cmp->stop) {
	  // Free up container records that we did not insert in container
	  if (pcilFirst)
	    FreeCnrItemList(hwndLeft, pcilFirst);

	  // Free up items we did not insert in container
	  if (filesl) {
	    for(; filesl[l]; l++) {
	      free(filesl[l]);
	    }
	  }

	  if (pcirFirst)
	    FreeCnrItemList(hwndRight, pcirFirst);

	  if (filesr) {
	    for (; filesr[r]; r++) {
	      free(filesr[r]);
	    }
	  }
	} // if insufficient resources

	xfree(filesl, pszSrcFile, __LINE__);	// Free header - have already freed elements
	filesl = NULL;
	  xfree(filesr, pszSrcFile, __LINE__);
	filesr = NULL;

      }	// if ulRecsNeeded

      Deselect(hwndLeft);
      Deselect(hwndRight);

      // Request window update
      if (!PostMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID))
	WinSendMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
      notified = TRUE;

      if (filesl)
	FreeList((CHAR **)filesl);	// Must have failed to create container
      if (filesr)
	FreeList((CHAR **)filesr);

      WinDestroyMsgQueue(hmq);
    } // if have queue
    if (!notified)
      PostMsg(cmp->hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
    DecrThreadUsage();
    WinTerminate(hab);
  }
  free(cmp);
  DosPostEventSem(CompactSem);

# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif

}

/**
 * Find matching item in other container
 * @return PCNRITEM or NULL
 */

static PCNRITEM FindMatchingItem(PCNRITEM pciFindS, HWND hwndCnrS, HWND hwndCnrD);

static PCNRITEM FindMatchingItem(PCNRITEM pciFindS, HWND hwndCnrS, HWND hwndCnrD)
{

  PCNRITEM pciS;
  PCNRITEM pciD;

  pciS = WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPVOID,
		   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  pciD = WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPVOID,
		    MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  while (pciS && (INT)pciS != -1 && pciD && (INT)pciD != -1) {

    if (pciS == pciFindS)
      break;

    pciS = WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPFROMP(pciS),
		      MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    pciD = WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPFROMP(pciD),
		       MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
  } // while

  if (pciS != pciFindS)
    pciD = NULL;

  return pciD;
}

/**
 * Set button/window enables
 */

static VOID SetButtonEnables(COMPARE* cmp, BOOL fEnable);

static VOID SetButtonEnables(COMPARE* cmp, BOOL fEnable)
{
  HWND hwnd = cmp->hwnd;
  HWND hwndLeft = GetHwndLeft(hwnd);
  HWND hwndRight = GetHwndRight(hwnd);

  if (!fEnable) {
    // Disable before
    WinEnableWindowUpdate(hwndLeft, fEnable);
    WinEnableWindowUpdate(hwndRight, fEnable);
  }
  WinEnableWindow(hwndLeft, fEnable);
  WinEnableWindow(hwndRight, fEnable);
  if (fEnable) {
    // Enable after
    WinEnableWindowUpdate(hwndLeft, fEnable);
    WinEnableWindowUpdate(hwndRight, fEnable);
  }

  WinEnableWindow(WinWindowFromID(hwnd, DID_OK), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, COMP_COLLECT), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBOTH), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTONE), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTNEWER), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTOLDER), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTBIGGER), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSMALLER), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBOTH), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTONE), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTNEWER), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTOLDER), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTBIGGER), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTSMALLER), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_DESELECTALL), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAMECONTENT), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTIDENTICAL), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_SELECTSAME), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, IDM_INVERT), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, COMP_SETDIRS), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETELEFT), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, COMP_FILTER), fEnable);
  if (!fEnable || !*cmp->rightlist ) {
    WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYLEFT), fEnable);
    WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVELEFT), fEnable);
    WinEnableWindow(WinWindowFromID(hwnd, COMP_DELETERIGHT), fEnable);
    WinEnableWindow(WinWindowFromID(hwnd, COMP_COPYRIGHT), fEnable);
    WinEnableWindow(WinWindowFromID(hwnd, COMP_MOVERIGHT), fEnable);
  }
  WinEnableWindow(WinWindowFromID(hwnd, COMP_INCLUDESUBDIRS), fEnable);
  WinEnableWindow(WinWindowFromID(hwnd, COMP_HIDENOTSELECTED), fEnable);
}

/**
 * Compare directories dialog procedure
 */

MRESULT EXPENTRY CompareDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  COMPARE *cmp;
  BOOL temp;
  CHAR szPathName[CCHMAXPATH];
  CHAR s[81];

  static HPOINTER hptr;

  switch (msg) {
  case WM_INITDLG:
    cmp = (COMPARE *)mp2;
    if (!cmp) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      WinDismissDlg(hwnd, 0);
    }
    else {
      if (!hptr)
	hptr = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, COMPARE_ICON);
      WinDefDlgProc(hwnd, WM_SETICON, MPFROMLONG(hptr), MPVOID);
      cmp->hwnd = hwnd;
      WinSetWindowPtr(hwnd, QWL_USER, (PVOID)cmp);
      {
	SWP swp;
	ULONG size = sizeof(SWP);

	PrfQueryProfileData(fmprof, FM3Str, "CompDir.Position", (PVOID) &swp, &size);
	swp.fl &= ~SWP_SIZE;		// 04 Feb 09 SHL ignore saved size
	WinSetWindowPos(hwnd,
			HWND_TOP,
			swp.x,
			swp.y,
			swp.cx,
			swp.cy,
			swp.fl);
      }
      SetCnrCols(GetHwndLeft(hwnd), TRUE);
      SetCnrCols(GetHwndRight(hwnd), TRUE);
      if (cmp->listfile) {
        CHAR fullname[CCHMAXPATH];

	strcpy(fullname, PCSZ_STARDOTPMD);
	if (insert_filename(HWND_DESKTOP, fullname, TRUE, FALSE) &&
	    *fullname && !strchr(fullname, '*') && !strchr(fullname, '?'))
          strcpy(cmp->rightlist, fullname);
      }
      WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
      PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
      {
        USHORT ids[] = {COMP_FRAME, COMP_LEFTDIR, COMP_RIGHTDIR, COMP_COLLECT,
                        COMP_VIEW, COMP_NOTE, COMP_TOTALLEFT, COMP_SELLEFT, COMP_TOTALRIGHT,
                        COMP_SELRIGHT, COMP_CNRMENU, COMP_DIRMENU, COMP_MENU,
                        COMP_INCLUDESUBDIRS, COMP_SETDIRS, COMP_COPYLEFT, COMP_MOVELEFT,
                        COMP_DELETELEFT, COMP_COPYRIGHT, COMP_MOVERIGHT, COMP_DELETERIGHT,
                        COMP_TOTALLEFTHDR, COMP_SELLEFTHDR, COMP_TOTALRIGHTHDR,
                        COMP_SELRIGHTHDR, COMP_FILTER, COMP_HIDENOTSELECTED, 0};
	UINT x;
        CHAR s[24];

	for (x = 0; ids[x]; x++) {
          sprintf(s, "CompDir%i", ids[x]);
          RestorePresParams(WinWindowFromID(hwnd, ids[x]), s);
	}
      }
      WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_COMP_TIMER, 500);
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
			    (HPS)0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, COMP_SELLEFT),
			    (HPS)0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, COMP_TOTALRIGHT),
			    (HPS)0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, COMP_SELRIGHT),
			    (HPS)0, FALSE, FALSE);
	PaintRecessedWindow(GetHwndLeft(hwnd), (HPS)0,
			    (hwndActive == GetHwndLeft(hwnd)), TRUE);
	PaintRecessedWindow(GetHwndRight(hwnd), (HPS)0,
			    (hwndActive == GetHwndRight(hwnd)), TRUE);
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
      if (!cmp)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	cmp->dcd.size = sizeof(DIRCNRDATA);
	cmp->dcd.type = DIR_FRAME;
	cmp->dcd.hwndFrame = hwnd;
	cmp->dcd.hwndClient = hwnd;
	cmp->dcd.mask.attrFile = (FILE_DIRECTORY | FILE_ARCHIVED |
				  FILE_READONLY | FILE_SYSTEM | FILE_HIDDEN);
	LoadDetailsSwitches(PCSZ_DIRCMP, &cmp->dcd.ds, FALSE);
	cmp->dcd.ds.detailslongname = FALSE;
	cmp->dcd.ds.detailsicon = FALSE;	// TRUE;
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
      AdjustCnrColRO(GetHwndLeft(hwnd), GetPString(IDS_FILENAMECOLTEXT), TRUE, FALSE);
      AdjustCnrColRO(GetHwndLeft(hwnd), GetPString(IDS_LONGNAMECOLTEXT), TRUE, FALSE);
      AdjustCnrColRO(GetHwndRight(hwnd), GetPString(IDS_FILENAMECOLTEXT), TRUE, FALSE);
      AdjustCnrColRO(GetHwndRight(hwnd), GetPString(IDS_LONGNAMECOLTEXT), TRUE, FALSE);
      AdjustCnrColsForPref(GetHwndLeft(hwnd), cmp->leftdir, &cmp->dcd.ds, TRUE);
      tempsubj = cmp->dcd.ds.detailssubject;
      cmp->dcd.ds.detailssubject = FALSE;
      AdjustCnrColsForPref(GetHwndRight(hwnd), cmp->rightdir, &cmp->dcd.ds, TRUE);
      if (*cmp->rightlist) {
	AdjustCnrColVis(GetHwndRight(hwnd), GetPString(IDS_LADATECOLTEXT), FALSE,
			FALSE);
	AdjustCnrColVis(GetHwndRight(hwnd), GetPString(IDS_LATIMECOLTEXT), FALSE,
			FALSE);
	AdjustCnrColVis(GetHwndRight(hwnd), GetPString(IDS_CRDATECOLTEXT), FALSE,
			FALSE);
	AdjustCnrColVis(GetHwndRight(hwnd), GetPString(IDS_CRTIMECOLTEXT), FALSE,
			FALSE);
      }
      cmp->dcd.ds.detailssubject = tempsubj;
    }
    return 0;

  case WM_DRAWITEM:
    if (mp2) {
      POWNERITEM pown = (POWNERITEM)mp2;
      PCNRDRAWITEMINFO pcown;
      PCNRITEM pci;

      pcown = (PCNRDRAWITEMINFO)pown->hItem;
      if (pcown) {
	pci = (PCNRITEM)pcown->pRecord;
	// 01 Aug 07 SHL if field null or blank, we draw
	// fixme to document why - probably to optimize and bypass draw?
	if (pci && (INT)pci != -1 && !*pci->pszFileName)
	  return MRFROMLONG(TRUE);
      }
    }
    return 0;

  case UM_CONTAINERHWND:
    // Building list
    WinSetDlgItemText(hwnd, COMP_NOTE, (CHAR *) GetPString(IDS_COMPHOLDBLDLISTTEXT));
    return 0;

  case UM_CONTAINERDIR:
    // Filling container
    WinSetDlgItemText(hwnd, COMP_NOTE, (CHAR *) GetPString(IDS_COMPHOLDFILLCNRTEXT));
    return 0;

  case WM_TIMER:
    // Show current totals
    cmp = INSTDATA(hwnd);
    if (!cmp) {
      Runtime_Error(pszSrcFile, __LINE__, "pCompare NULL");
      WinDismissDlg(hwnd, 0);
    }
    else {
      if (cmp->uOldTotalLeft != cmp->totalleft) {
	cmp->uOldTotalLeft = cmp->totalleft;
	sprintf(s, " %d", cmp->totalleft);
	WinSetDlgItemText(hwnd, COMP_TOTALLEFT, s);
      }
      if (cmp->uOldTotalRight != cmp->totalright) {
	cmp->uOldTotalRight = cmp->totalright;
	sprintf(s, " %d", cmp->totalright);
	WinSetDlgItemText(hwnd, COMP_TOTALRIGHT, s);
      }
      if (cmp->uOldSelLeft != cmp->selleft) {
	cmp->uOldSelLeft = cmp->selleft;
	sprintf(s, " %d", cmp->selleft);
	WinSetDlgItemText(hwnd, COMP_SELLEFT, s);
      }
      if (cmp->uOldSelRight != cmp->selright) {
	cmp->uOldSelRight = cmp->selright;
	sprintf(s, " %d", cmp->selright);
	WinSetDlgItemText(hwnd, COMP_SELRIGHT, s);
      }
    }
    break;

  case UM_CONTAINER_FILLED:
    cmp = INSTDATA(hwnd);
    if (!cmp) {
      Runtime_Error(pszSrcFile, __LINE__, "pCompare NULL");
      WinDismissDlg(hwnd, 0);
    }
    else if (cmp->stop) {
      // Delayed cancel
      WinDismissDlg(hwnd, 1);
    }
    else {
      cmp->filling = FALSE;
      SetButtonEnables(cmp, TRUE);
      WinPostMsg(hwnd, WM_TIMER, MPFROMLONG(ID_COMP_TIMER), 0);	// Force counts to update
      if (*cmp->dcd.mask.szMask) {
	sprintf(s, GetPString(IDS_COMPREADYFILTEREDTEXT), cmp->dcd.mask.szMask);
	WinSetDlgItemText(hwnd, COMP_NOTE, s);
      }
      else
	WinSetDlgItemText(hwnd, COMP_NOTE, (CHAR *) GetPString(IDS_COMPREADYTEXT));
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
      if ((HWND)mp2 == cmp->dcd.hwndLastMenu) {
	MarkAll(GetHwndLeft(hwnd), TRUE, FALSE, TRUE);
	MarkAll(GetHwndRight(hwnd), TRUE, FALSE, TRUE);
	WinDestroyWindow(cmp->dcd.hwndLastMenu);
	cmp->dcd.hwndLastMenu = (HWND)0;
      }
    }
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case COMP_INCLUDESUBDIRS:
      switch (SHORT2FROMMP(mp1)) {
      case BN_CLICKED:
	cmp = INSTDATA(hwnd);
	//if (cmp)
	//  *cmp->rightlist = 0;
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
			    (HPS)0, FALSE, TRUE);
	break;

      case CN_SETFOCUS:
	PaintRecessedWindow(WinWindowFromID(hwnd, SHORT1FROMMP(mp1)),
			    (HPS)0, TRUE, TRUE);
	break;

      case CN_ENTER:
	if (mp2) {

	  PCNRITEM pci = (PCNRITEM)((PNOTIFYRECORDENTER)mp2)->pRecord;
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
		       MPFROM2SHORT(FALSE,
				    CRA_INUSE |	(fUnHilite ? CRA_SELECTED : 0)));
	  }
	}
	break;

      case CN_CONTEXTMENU:
	cmp = INSTDATA(hwnd);
	if (cmp) {
	  PCNRITEM pci = (PCNRITEM)mp2;
	  USHORT id = COMP_CNRMENU;		// Assume container menu

	  if (cmp->dcd.hwndLastMenu)
	    WinDestroyWindow(cmp->dcd.hwndLastMenu);
	  cmp->dcd.hwndLastMenu = (HWND)0;
	  cmp->hwndCalling = WinWindowFromID(hwnd, SHORT1FROMMP(mp1));
	  if (pci) {
	    if (!pci || !*pci->pszFileName || *cmp->rightlist)
	      break;
	    id = COMP_MENU;		// Want container item menu
	    WinSendMsg(cmp->hwndCalling, CM_SETRECORDEMPHASIS,
		       MPFROMP(pci), MPFROM2SHORT(TRUE, CRA_CURSORED));
	  }
	  cmp->dcd.hwndLastMenu = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, id);
	  if (cmp->dcd.hwndLastMenu) {
	    switch (id) {
	    case COMP_CNRMENU:
	      if (SHORT1FROMMP(mp1) == COMP_RIGHTDIR)
		WinSendMsg(cmp->dcd.hwndLastMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_SHOWSUBJECT, FALSE), MPVOID);
	      SetDetailsSwitches(cmp->dcd.hwndLastMenu, &cmp->dcd.ds);
	      if (SHORT1FROMMP(mp1) == COMP_LEFTDIR)
		WinSendMsg(cmp->dcd.hwndLastMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_LOADLISTFILE, 0), MPVOID);
	      else if (*cmp->rightlist)
		WinSendMsg(cmp->dcd.hwndLastMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_SAVELISTFILE, 0), MPVOID);
	      break;
	    case COMP_MENU:
	      // Can't compare what's not there
	      if (~pci->flags & CNRITEM_EXISTS) {
		WinSendMsg(cmp->dcd.hwndLastMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_COMPARE, 0), MPVOID);
	      }
	      break;
	    } // switch
	    PopupMenu(hwnd, hwnd, cmp->dcd.hwndLastMenu);
	  }
	}
	break;

      case CN_INITDRAG:
	cmp = INSTDATA(hwnd);
	if (*cmp->rightlist && SHORT1FROMMP(mp1) == COMP_RIGHTDIR)
	  break;
	DoFileDrag(WinWindowFromID(hwnd, SHORT1FROMMP(mp1)),
		   (HWND)0, mp2, NULL, NULL, TRUE);
	break;

      case CN_BEGINEDIT:
      case CN_REALLOCPSZ:
	// fixme to be gone - field edits not allowed?
	Runtime_Error(pszSrcFile, __LINE__,
		      "CN_BEGINEDIT/CN_REALLOCPSZ unexpected");
	break;

      case CN_EMPHASIS:
	{
	  PNOTIFYRECORDEMPHASIS pnre = mp2;
	  BOOL fSelected;
	  if (pnre->fEmphasisMask & CRA_SELECTED) {
	    // Select toggled
	    PCNRITEM pci = (PCNRITEM)pnre->pRecord;
	    if (pci) {
	      if (!*pci->pszFileName) {
		// User clicked on empty slot - just clear
		if (pci->rc.flRecordAttr & CRA_SELECTED)
		  WinSendDlgItemMsg(hwnd, SHORT1FROMMP(mp1),
				    CM_SETRECORDEMPHASIS,
				    MPFROMP(pci),
				    MPFROM2SHORT(FALSE, CRA_SELECTED));
	      }
	      else {
		BOOL fUpdateHideButton = FALSE;
		cmp = INSTDATA(hwnd);
		if (SHORT1FROMMP(mp1) == COMP_LEFTDIR) {
		  fSelected = pci->rc.flRecordAttr & CRA_SELECTED;
		  cmp->selleft += fSelected ? 1 : -1;
		  if (!fSelected)
		    fUpdateHideButton = TRUE;
		}
		else if (SHORT1FROMMP(mp1) == COMP_RIGHTDIR) {
		  fSelected = pci->rc.flRecordAttr & CRA_SELECTED;
		  cmp->selright += fSelected ? 1 : -1;
		  if (!fSelected)
		    fUpdateHideButton = TRUE;
		}
		else {
		  Runtime_Error(pszSrcFile, __LINE__,
				"mp1 %u unexpected", SHORT1FROMMP(mp1));
		}
		if (fUpdateHideButton) {
		  ULONG state = WinQueryButtonCheckstate(hwnd,COMP_HIDENOTSELECTED);
		  if (state == 1) {
		    WinCheckButton(hwnd, COMP_HIDENOTSELECTED, 2);
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
	    // Scroll other window to match
	    WinSendDlgItemMsg(hwnd,
			      SHORT1FROMMP(mp1) == COMP_LEFTDIR ?
				COMP_RIGHTDIR : COMP_LEFTDIR,
			      CM_SCROLLWINDOW,
			      MPFROMSHORT(CMA_VERTICAL),
			      MPFROMLONG(pns->lScrollInc));
	    cmp->forcescroll = FALSE;
	  }
	}
	break;
      } // switch COMP_LEFTDIR mp1
      break;				// COMP_LEFTDIR / COMP_RIGHTDIR
    } // switch WM_CONTROL mp1
    return 0;				// WM_CONTROL

  case UM_SETDIR:
    cmp = INSTDATA(hwnd);
    if (cmp) {
      COMPARE *forthread;
      CNRINFO cnri;
      if (cmp->includesubdirs)
        WinCheckButton(hwnd, COMP_INCLUDESUBDIRS, TRUE);
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
      WinSetDlgItemText(hwnd, COMP_LISTLOADED, "");
      cnri.pszCnrTitle = cmp->rightdir;
      WinSendDlgItemMsg(hwnd, COMP_RIGHTDIR, CM_SETCNRINFO, MPFROMP(&cnri),
			MPFROMLONG(CMA_CNRTITLE | CMA_FLWINDOWATTR));
      WinCheckButton(hwnd, COMP_HIDENOTSELECTED, 0);
      cmp->filling = TRUE;
#     ifdef FORTIFY
      Fortify_EnterScope();
#      endif
      forthread = xmalloc(sizeof(COMPARE), pszSrcFile, __LINE__);
      if (!forthread)
	WinDismissDlg(hwnd, 0);
      else {
	*forthread = *cmp;
	forthread->cmp = cmp;
	if (xbeginthread(FillCnrsThread,
			 122880,
			 forthread,
			 pszSrcFile,
			 __LINE__) == -1)
	{
	  WinDismissDlg(hwnd, 0);
	  free(forthread);
#	  ifdef FORTIFY
	  Fortify_LeaveScope();
#	   endif
	}
	else {
	  WinSetDlgItemText(hwnd, COMP_NOTE,
			    (CHAR *) GetPString(IDS_COMPHOLDREADDISKTEXT));
          SetButtonEnables(cmp, FALSE);
          cmp->includesubdirs = FALSE;
	  cmp->selleft = 0;
	  cmp->selright = 0;
	}
      }
    }
    return 0;

  case UM_FILTER:
    cmp = INSTDATA(hwnd);
    if (cmp) {
      if (mp1)
	SetMask((CHAR *)mp1, &cmp->dcd.mask);

      WinSetDlgItemText(hwnd, COMP_NOTE,
			(CHAR *) GetPString(IDS_COMPHOLDFILTERINGTEXT));
      // cmp->dcd.suspendview = 1;	// 12 Jan 08 SHL appears not to be used here
      priority_idle();			// Don't hog resources
      WinSendMsg(GetHwndLeft(hwnd), CM_FILTER, MPFROMP(Filter),
		 MPFROMP(&cmp->dcd.mask));
      WinSendMsg(GetHwndRight(hwnd), CM_FILTER, MPFROMP(Filter),
		 MPFROMP(&cmp->dcd.mask));
      priority_normal();
      // cmp->dcd.suspendview = 0;	// 12 Jan 08 SHL appears not to be used here
      if (*cmp->dcd.mask.szMask) {
	sprintf(s,
		GetPString(IDS_COMPREADYFILTEREDTEXT),
		cmp->dcd.mask.szMask);
	WinSetDlgItemText(hwnd, COMP_NOTE, s);
      }
      else
	WinSetDlgItemText(hwnd, COMP_NOTE, (CHAR *) GetPString(IDS_COMPREADYTEXT));
    }
    return 0;

  case UM_HIDENOTSELECTED:
    cmp = INSTDATA(hwnd);
    if (cmp) {
      ULONG wasHidden = WinQueryButtonCheckstate(hwnd,
						 COMP_HIDENOTSELECTED);
      ULONG nowHidden = wasHidden != 1 && (cmp->selleft || cmp->selright);

      // cmp->dcd.suspendview = 1;	// 12 Jan 08 SHL appears not to be used here
      // 26 Sep 09 SHL Unhide if nothing selected
      if (nowHidden) {
	// Hide items not selected on both sides
	BOOL needRefresh = FALSE;
	HWND hwndl = WinWindowFromID(cmp->hwnd, COMP_LEFTDIR);
	HWND hwndr = WinWindowFromID(cmp->hwnd, COMP_RIGHTDIR);
	PCNRITEM pcil = WinSendMsg(hwndl, CM_QUERYRECORD, MPVOID,
				   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
	PCNRITEM pcir = WinSendMsg(hwndr, CM_QUERYRECORD, MPVOID,
				   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));

	while (pcil && (INT)pcil != -1 && pcir && (INT)pcir != -1) {
	  if (~pcil->rc.flRecordAttr & CRA_SELECTED &&
	      ~pcir->rc.flRecordAttr & CRA_SELECTED) {
	    if (~pcil->rc.flRecordAttr & CRA_FILTERED) {
	      needRefresh = TRUE;
	      pcil->rc.flRecordAttr |= CRA_FILTERED;
	    }
	    if (~pcir->rc.flRecordAttr & CRA_FILTERED) {
	      pcir->rc.flRecordAttr |= CRA_FILTERED;
	      needRefresh = TRUE;
	    }
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
	// Unhide
	WinSendMsg(GetHwndLeft(hwnd), CM_FILTER, MPFROMP(Filter),
		   MPFROMP(&cmp->dcd.mask));
	WinSendMsg(GetHwndRight(hwnd), CM_FILTER, MPFROMP(Filter),
		   MPFROMP(&cmp->dcd.mask));
      }
      // cmp->dcd.suspendview = 0;	// 12 Jan 08 SHL appears not to be used here
      if (*cmp->dcd.mask.szMask) {
	sprintf(s,
		GetPString(IDS_COMPREADYFILTEREDTEXT),
		cmp->dcd.mask.szMask);
	WinSetDlgItemText(hwnd, COMP_NOTE, s);
      }
      else
	WinSetDlgItemText(hwnd, COMP_NOTE, (CHAR *) GetPString(IDS_COMPREADYTEXT));
      WinCheckButton(hwnd, COMP_HIDENOTSELECTED, nowHidden);
    }
    return 0;

  case WM_COMMAND:
    // 29 Apr 09 SHL fixme to support more context menu items - IDM_VIEW, IDM_EDIT etc.
    switch (SHORT1FROMMP(mp1)) {
    case IDM_COMPARE:
      cmp = INSTDATA(hwnd);
      if (cmp) {
	PCNRITEM pci;
	pci = (PCNRITEM)WinSendMsg(cmp->hwndCalling,
				   CM_QUERYRECORDEMPHASIS,
				   MPFROMLONG(CMA_FIRST),
				   MPFROMSHORT(CRA_CURSORED));
	if (pci && *pci->pszFileName) {
	  if (cmp->hwndCalling == GetHwndLeft(hwnd))
	    strcpy(szPathName, cmp->rightdir);
	  else
	    strcpy(szPathName, cmp->leftdir);
	  AddBackslashToPath(szPathName);
	  strcat(szPathName, pci->pszDisplayName);
	  if (*compare) {
	    CHAR *fakelist[3];
	    fakelist[0] = pci->pszFileName;
	    fakelist[1] = szPathName;
	    fakelist[2] = NULL;
	    ExecOnList(hwnd, compare, WINDOWED | SEPARATEKEEP,
		       NULL, NULL, fakelist, NULL,
		       pszSrcFile, __LINE__);
	  }
	  else {
	    FCOMPARE fc;
	    memset(&fc, 0, sizeof(fc));
	    fc.size = sizeof(fc);
	    fc.hwndParent = hwnd;
	    strcpy(fc.file1, pci->pszFileName);
	    strcpy(fc.file2, szPathName);
	    WinDlgBox(HWND_DESKTOP, hwnd,
		      CFileDlgProc, FM3ModHandle, FCMP_FRAME, (PVOID)&fc);
	  }
	}
      }
      break;

    case IDM_DELETE:
      cmp = INSTDATA(hwnd);
      if (!cmp)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	PCNRITEM pciS;
	PCNRITEM pciD;
	HWND hwndCnrS;
	HWND hwndCnrD;
	pciS = (PCNRITEM)WinSendMsg(cmp->hwndCalling,
				   CM_QUERYRECORDEMPHASIS,
				   MPFROMLONG(CMA_FIRST),
				   MPFROMSHORT(CRA_CURSORED));
	if (!pciS)
	  Runtime_Error(pszSrcFile, __LINE__, NULL);
	else {
	  // Find matching record
	  if (cmp->hwndCalling == GetHwndLeft(hwnd)) {
	    hwndCnrS = GetHwndLeft(hwnd);
	    hwndCnrD = GetHwndRight(hwnd);
	  }
	  else {
	    hwndCnrS = GetHwndRight(hwnd);
	    hwndCnrD = GetHwndLeft(hwnd);
	  }

	  pciD = FindMatchingItem(pciS, hwndCnrS, hwndCnrD);

	  if (!pciD)
	    Runtime_Error(pszSrcFile, __LINE__, NULL);
	  else if (!pciS->pszFileName)
	    Runtime_Error(pszSrcFile, __LINE__, NULL);
	  else {
	    if (!unlinkf(pciS->pszFileName)) {
	      WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciS),
			 MPFROM2SHORT(FALSE, CRA_SELECTED));

	      if (!*pciD->pszFileName) {
		// Other side is blank - remove from both sides
		RemoveCnrItems(hwndCnrS, pciS, 1, CMA_FREE | CMA_INVALIDATE);
		if (pciD->rc.flRecordAttr & CRA_SELECTED)
		  WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciD),
			     MPFROM2SHORT(FALSE, CRA_SELECTED));
		RemoveCnrItems(hwndCnrD, pciD, 1, CMA_FREE | CMA_INVALIDATE);
	      }
	      else {
		// Other side is not blank - just blank this side
		FreeCnrItemData(pciS);
		pciS->pszFileName = NullStr;
		pciS->pszDisplayName = pciS->pszFileName;
		pciS->rc.pszIcon = pciS->pszFileName;
		pciS->flags = 0;	// Just on one side
		WinSendMsg(hwndCnrS, CM_INVALIDATERECORD, MPFROMP(&pciS),
			   MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
		pciD->flags = 0;	// Just on one side
		if (pciD->pszSubject != NullStr) {
		  xfree(pciD->pszSubject, pszSrcFile, __LINE__);
		  pciD->pszSubject = NullStr;
		}
	      }
	      if (hwndCnrS == GetHwndLeft(hwnd))
		cmp->totalleft--;
	      else
		cmp->totalright--;
	      // Force displayed counts to update
	      WinPostMsg(hwnd, WM_TIMER, MPFROMLONG(ID_COMP_TIMER), 0);
	    }
	  }
	}
      }
      break; // IDM_DELETE

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
	AdjustDetailsSwitches(GetHwndLeft(hwnd),
			      (HWND)0, SHORT1FROMMP(mp1),
			      cmp->leftdir, PCSZ_DIRCMP, &cmp->dcd.ds, TRUE);
	tempsubj = cmp->dcd.ds.detailssubject;
	cmp->dcd = dcd1;
	cmp->dcd.ds.detailssubject = FALSE;
	AdjustDetailsSwitches(GetHwndRight(hwnd),
			      cmp->dcd.hwndLastMenu, SHORT1FROMMP(mp1),
			      cmp->rightdir, PCSZ_DIRCMP, &cmp->dcd.ds, TRUE);
	cmp->dcd.ds.detailssubject = tempsubj;
	WriteDetailsSwitches(PCSZ_DIRCMP, &cmp->dcd.ds, TRUE);
      }
      break;

    case IDM_LOADLISTFILE:
      cmp = INSTDATA(hwnd);
      if (cmp) {
	CHAR fullname[CCHMAXPATH];

	strcpy(fullname, PCSZ_STARDOTPMD);
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

	strcpy(fullname, PCSZ_STARDOTPMD);
	if (export_filename(HWND_DESKTOP, fullname, 1) && *fullname &&
	    !strchr(fullname, '*') && !strchr(fullname, '?')) {
	  sf = xmallocz(sizeof(SNAPSTUFF), pszSrcFile, __LINE__);
	  if (sf) {
	    strcpy(sf->filename, fullname);
	    if (GetHwndLeft(hwnd) == cmp->hwndCalling)
	      strcpy(sf->dirname, cmp->leftdir);
	    else
              strcpy(sf->dirname, cmp->rightdir);
            cmp->includesubdirs = WinQueryButtonCheckstate(hwnd,
                                                           COMP_INCLUDESUBDIRS);
            sf->recurse = cmp->includesubdirs;
            //DbgMsg(pszSrcFile, __LINE__, "recurse %i %i", sf->recurse, cmp->includesubdirs);
	    if (xbeginthread(StartSnapThread,
			     65536,
			     sf,
			     pszSrcFile,
			     __LINE__) == -1)
	    {
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
        wa.includesubdirs = WinQueryButtonCheckstate(hwnd,
	  					     COMP_INCLUDESUBDIRS);
	if (WinDlgBox(HWND_DESKTOP,
		      hwnd,
		      WalkTwoCmpDlgProc,
		      FM3ModHandle,
		      WALK2_FRAME,
		      MPFROMP(&wa)) &&
	    !IsFile(wa.szCurrentPath1) &&
	    !IsFile(wa.szCurrentPath2)) {
	  strcpy(cmp->leftdir, wa.szCurrentPath1);
          strcpy(cmp->rightdir, wa.szCurrentPath2);
          cmp->includesubdirs = wa.includesubdirs;
          if (!cmp->includesubdirs)
            WinCheckButton(hwnd, COMP_INCLUDESUBDIRS, FALSE);
          cmp->listfile = wa.listfile;
          if (cmp->listfile) {
            CHAR fullname[CCHMAXPATH];
    
            strcpy(fullname, PCSZ_STARDOTPMD);
            if (insert_filename(HWND_DESKTOP, fullname, TRUE, FALSE) &&
                *fullname && !strchr(fullname, '*') && !strchr(fullname, '?'))
              strcpy(cmp->rightlist, fullname);
          }
          else
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
	  if (xbeginthread(ActionCnrThread,
			   122880,
			   forthread,
			   pszSrcFile,
			   __LINE__) == -1)
	  {
	    free(forthread);
	  }
	  else {
	    WinEnableWindowUpdate(GetHwndLeft(hwnd), FALSE);
	    WinEnableWindowUpdate(GetHwndRight(hwnd), FALSE);
	    switch (SHORT1FROMMP(mp1)) {
	    case COMP_DELETELEFT:
	    case COMP_DELETERIGHT:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				(CHAR *) GetPString(IDS_COMPHOLDDELETINGTEXT));
	      break;
	    case COMP_MOVELEFT:
	    case COMP_MOVERIGHT:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				(CHAR *) GetPString(IDS_COMPHOLDMOVINGTEXT));
	      break;
	    case COMP_COPYLEFT:
	    case COMP_COPYRIGHT:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				(CHAR *) GetPString(IDS_COMPHOLDCOPYINGTEXT));
	      break;
	    default:
	      Runtime_Error(pszSrcFile, __LINE__, "mp1 %u unexpected", SHORT1FROMMP(mp1));
	    }
	    SetButtonEnables(cmp, FALSE);
	  }
	}
      }
      break;

    case DID_OK:
      {
	SWP swp;
	ULONG size = sizeof(SWP);
        USHORT ids[] = {COMP_FRAME, COMP_LEFTDIR, COMP_RIGHTDIR, COMP_COLLECT,
                        COMP_VIEW, COMP_NOTE, COMP_TOTALLEFT, COMP_SELLEFT, COMP_TOTALRIGHT,
                        COMP_SELRIGHT, COMP_CNRMENU, COMP_DIRMENU, COMP_MENU,
                        COMP_INCLUDESUBDIRS, COMP_SETDIRS, COMP_COPYLEFT, COMP_MOVELEFT,
                        COMP_DELETELEFT, COMP_COPYRIGHT, COMP_MOVERIGHT, COMP_DELETERIGHT,
                        COMP_TOTALLEFTHDR, COMP_SELLEFTHDR, COMP_TOTALRIGHTHDR,
                        COMP_SELRIGHTHDR, COMP_FILTER, COMP_HIDENOTSELECTED, 0};
        UINT x;
        CHAR s[24];
	WinQueryWindowPos(hwnd, &swp);
	PrfWriteProfileData(fmprof, FM3Str, "CompDir.Position", (PVOID) &swp,
                            size);
	for (x = 0; ids[x]; x++) {
          //fixme to allow user to change presparams 1-10-09 GKY
          sprintf(s, "CompDir%i", ids[x]);
          SavePresParams(WinWindowFromID(hwnd, ids[x]), s);
        }
      }
      WinDismissDlg(hwnd, 0);
      break;
    case DID_CANCEL:
      cmp = INSTDATA(hwnd);
      if (!cmp)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	cmp->stop = TRUE;			// In case thread running
	if (cmp->filling)
	  break;				// UM_CONTAINER_FILLED will dismiss
      }
      {
	SWP swp;
	ULONG size = sizeof(SWP);
	WinQueryWindowPos(hwnd, &swp);
	PrfWriteProfileData(fmprof, FM3Str, "CompDir.Position", (PVOID) &swp,
			    size);
      }
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
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	COMPARE *forthread;

	cmp->filling = TRUE;
	forthread = xmalloc(sizeof(COMPARE), pszSrcFile, __LINE__);
	if (forthread) {
	  *forthread = *cmp;
	  forthread->cmp = cmp;
	  forthread->action = SHORT1FROMMP(mp1);
	  SetShiftState();
	  forthread->shiftstate = shiftstate;	// For and'ed actions

	  if (xbeginthread(SelectCnrsThread,
			   65536,
			   forthread,
			   pszSrcFile,
			   __LINE__) == -1)
	  {
	    free(forthread);
	  }
	  else {
	    WinEnableWindowUpdate(GetHwndLeft(hwnd), FALSE);
	    WinEnableWindowUpdate(GetHwndRight(hwnd), FALSE);
	    switch (SHORT1FROMMP(mp1)) {
	    case IDM_DESELECTALL:
	    case IDM_DESELECTNEWER:
	    case IDM_DESELECTOLDER:
	    case IDM_DESELECTBIGGER:
	    case IDM_DESELECTSMALLER:
	    case IDM_DESELECTONE:
	    case IDM_DESELECTBOTH:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				(CHAR *) GetPString(IDS_COMPHOLDDESELTEXT));
	      break;
	    case IDM_INVERT:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				(CHAR *) GetPString(IDS_COMPHOLDINVERTTEXT));
	      break;
	    default:
	      WinSetDlgItemText(hwnd, COMP_NOTE,
				(CHAR *) GetPString(IDS_COMPHOLDSELTEXT));
	      break;
	    }
	    SetButtonEnables(cmp, FALSE);
	  }
	}
      }
      break;

    case COMP_COLLECT:
      cmp = INSTDATA(hwnd);
      if (!cmp)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
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
	    DosSleep(1);		// 12 Jan 08 SHL Let screen update
	    PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(COMP_COLLECT, 0), MPVOID);
	    break;
	  }
	}
	else
	  StartCollector(cmp->hwndParent, 4);

	temp = fSelectedAlways;
	fSelectedAlways = TRUE;
	listl = BuildList(GetHwndLeft(hwnd));
	if (!*cmp->rightlist)
	  listr = BuildList(GetHwndRight(hwnd));
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
    } // switch mp1
    return 0;

  case WM_CLOSE:
    cmp = INSTDATA(hwnd);
    if (!cmp)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else {
      cmp->stop = TRUE;			// In case thread running
      if (cmp->filling)
	break;				// UM_CONTAINER_FILLED will dismiss
    }
    WinDismissDlg(hwnd, 0);
    return 0;

  case WM_DESTROY:
    cmp = INSTDATA(hwnd);
    if (cmp) {
      // 17 Jan 08 SHL fixme to know if stop really needed?
      WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_COMP_TIMER);
      if (cmp->dcd.hwndLastMenu)
	WinDestroyWindow(cmp->dcd.hwndLastMenu);
      if (cmp->dcd.hwndObject) {
	WinSetWindowPtr(cmp->dcd.hwndObject, QWL_USER, (PVOID)NULL);
	if (!PostMsg(cmp->dcd.hwndObject, WM_CLOSE, MPVOID, MPVOID))
	  WinSendMsg(cmp->dcd.hwndObject, WM_CLOSE, MPVOID, MPVOID);
      }
      free(cmp);
    }
    EmptyCnr(GetHwndLeft(hwnd));
    EmptyCnr(GetHwndRight(hwnd));
    DosPostEventSem(CompactSem);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#undef GetHwndLeft
#undef GetHwndRight

#pragma alloc_text(COMPAREDIR,FillCnrsThread,FillDirList,CompNames,BldFullPathName)
#pragma alloc_text(COMPAREDIR1,CompareDlgProc)
#pragma alloc_text(COMPAREDIR2,SelectCnrsThread,ActionCnrThread)
#pragma alloc_text(COMPAREFILE,CFileDlgProc,CompareFilesThread)
#pragma alloc_text(SNAPSHOT,SnapShot,StartSnapThread)
#pragma alloc_text(COMPSELECT,CompSelect,CompSelectSetSelects,CompSelectClearSelects)

