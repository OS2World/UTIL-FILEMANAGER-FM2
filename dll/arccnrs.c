
/***********************************************************************

  $Id$

  Archive containers

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2013 Steven H. Levine

  11 Jun 02 SHL Ensure archive name not garbage
  22 May 03 SHL ArcObjWndProc: fix UM_RESCAN now that we understand it
  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  25 May 05 SHL Rename comnam to szCommonName and fix typo
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  05 Jun 05 SHL Drop obsolete, localize
  05 Jun 05 SHL Correct last sort logic
  05 Jun 05 SHL Use QWL_USER
  22 Jun 05 SHL ArcSort: correct typo in last sort fix
  13 Aug 05 SHL FillArcCnr: optimize
  08 Dec 05 SHL FillArcCnr: allow list start and end markers to be empty (i.e. tar)
  08 Dec 05 SHL ArcCnrWndProc: suppress IDM_EXTRACT if no simple extract (i.e. tar)
  30 Dec 05 SHL ArcCnrWndProc: correct date/time column display setup
  29 May 06 SHL Comments
  14 Jul 06 SHL Use Runtime_Error
  26 Jul 06 SHL Correct SelectAll usage
  29 Jul 06 SHL Use xfgets_bstripcr
  31 Jul 06 SHL Lower priority for archives with more than 1000 entries
  02 Aug 06 SHL Add logic to stop processing large archives
  23 Aug 06 SHL Integrate John Small's switch list title logic
  03 Nov 06 SHL Renames
  14 Mar 07 SHL ArcObjWndProc/UM_ENTER: delay before starting viewer
  30 Mar 07 GKY Remove GetPString for window class names
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeISH limit
  06 Apr 07 GKY Add some error checking in drag/drop
  20 Apr 07 SHL Sync with NumItemsToUnhilite mods
  21 Apr 07 GKY Find FM2Utils by path or utils directory
  12 May 07 SHL Use dcd->ulItemsToUnHilite; sync with UnHilite arg mods
  10 Jun 07 GKY Add CheckPmDrgLimit including IsFm2Window as part of work around PM drag limit
  16 Jun 07 SHL Use DosQueryAppType not DosQAppType
  02 Aug 07 SHL Sync with ARCITEM mods
  06 Aug 07 SHL Use BldFullPathName and BldQuotedFileName
  06 Aug 07 SHL Move BldQuotedFileName and BldQuotedFullPathNamehere
		to be near primary caller
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  26 Aug 07 GKY DosSleep(1) in loops changed to (0)
  22 Nov 07 GKY Use CopyPresParams to fix presparam inconsistencies in menus
  30 Dec 07 GKY Use TestCDates for sort by date
  10 Jan 08 SHL Sync with CfgDlgProc mods
  10 Feb 08 GKY Implement bubble help for bitmap menu items
  15 Feb 08 SHL Sync with settings menu rework
  29 Feb 08 GKY Use xfree where appropriate
  14 Jul 08 JBS Ticket 126: Add support for WPS open default & open settings in arccnrs
  16 Jul 08 GKY Fix trap on viewing multiple files from an archive (misplaced free)
  16 Jul 08 GKY Use TMP directory for temp files if present. Use MakeTempName
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory
  25 Aug 08 GKY Check TMP directory space warn if lee than 5 MiB prevent archiver from opening if
		less than 10 KiB (It hangs and can't be closed)
  08 Nov 08 GKY Add WaitChildThread to fix hang caused by viewer trying to open a file before
		the archiver process closes. (Ticket 58)
  23 Nov 08 JBS Ticket 284: Support archivers with no Start or End of list strings
  29 Nov 08 GKY Add flag to tell CheckListProc file is in an archive so it won't try to open it.
  29 Nov 08 GKY Remove or replace with a mutex semaphore DosEnterCriSec where appropriate.
  30 Nov 08 GKY Add the option of creating a subdirectory from the arcname
		for the extract path to arc container.
  02 Dec 08 JBS Ticket 284: Changed string indicating no Start/End of list strings.
  03 Dec 08 GKY Subdirectory from the arcname for the extract path only created if an "extract"
		menu option is selected.
  10 Dec 08 SHL Integrate exception handler support
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)
  12 Mar 09 SHL Use common SearchContainer
  13 Dec 09 GKY Fixed separate paramenters. Please note that appname should be used in
                profile calls for user settings that work and are setable in more than one
                miniapp; FM3Str should be used for setting only relavent to FM/2 or that
                aren't user settable; realappname should be used for setting applicable to
                one or more miniapp but not to FM/2
  17 Jan 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  15 Apr 10 JBS Ticket 422: Stop hang when open archive gets deleted or moved
  23 Oct 10 GKY Add ForwardslashToBackslash function to streamline code
  20 Nov 10 GKY Check that pTmpDir IsValid and recreate if not found; Fixes hangs caused
                by temp file creation failures.
  13 Aug 11 GKY Change to Doxygen comment format
  30 Jul 13 GKY Changes to allow 7z archiver to work with AV.
  05 Aug 13 GKY Changes to allow Lzip to work with AV
  11 Aug 13 GKY Removed code that attempted to use the archive name as the extract directory
                It was never fully implemented and doesn't make sense for the container.
  09 Feb 14 GKY Fix trap on opening a file without an extention
  22 Feb 14 GKY Fix warn readonly yes don't ask to work when recursing directories.
  28 Apr 14 JBS Ticket #522: Ensure use of wrapper functions where needed
  01 Mar 14 GKY Fixed error checking in FillArcCnr only to report missing archivers after
                all entries have been tried. Added a check b/gzip exes for TAR.B/GZ archives.
                Use the test archive string from the first working archive description.
                Enhance the error message. Ticket 502
  01 Mar 14 GKY Fix a trap caused by selecting "print" from the arccontainer menu. Ticket 525
  01 Mar 14 GKY Fix the problem with copying text from the test archive window by launching
                it in a command shell (i.e. comspec /k archiver -t archive) Ticket 503
  02 Mar 14 GKY Fixed typo that reversed the function of the saymsg dialog g/bzip check.
                Added option to suppress message regarding missing bzip2.exe
                or gzip.exe on TAR.B/GZ archives.
  06 Apr 14 GKY Removed all BZ/GZ checks
  28 Jun 14 GKY Fix errors identified with CPPCheck; Fix retry to create workdir code
  12 Aug 15 JBS Ticket #522: Ensure no "highmem-unsafe" functions are called directly.
                Calls to unsafe Dos... functions have been changed to call the wrapped xDos... functions.

***********************************************************************/

#include <stdlib.h>			// free..
#include <string.h>
#include <ctype.h>
#include <direct.h>	        	// rmdir
#include <share.h>			// SH_DENYWR
#include <limits.h>			// ULONG_MAX

#if 0
#include <malloc.h>			// _heapchk
#endif

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "arccnrs.h"
#include "makelist.h"			// Typedef
#include "colors.h"			// Typedef
#include "mainwnd2.h"			// Data declaration(s)
#include "grep.h"			// Data declaration(s)
#include "dircnrs.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "pathutil.h"			// BldFullPathName
#include "filldir.h"			// EmptyCnr...
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "notebook.h"			// CfgDlgProc
#include "worker.h"		// Action, MassAction
#include "avv.h"			// ArcReviewDlgProc, rewrite_archiverbb2
#include "chklist.h"			// CenterOverWindow, CheckListProc
#include "common.h"			// CommonCreateTextChildren, CommonFrameWndProc, CommonTextPaint
				// CommonTextButton
#include "draglist.h"			// DoFileDrag, DragOne
#include "valid.h"			// GetDesktopName, TestCDates
#include "mainwnd.h"			// GetNextWindowPos, MakeBubble, TopWindowName
#include "objwin.h"			// MakeObjWin
#include "shadow.h"			// MakeShadows
#include "objcnr.h"			// ObjCnrDlgProc
#include "printer.h"			// PrintListThread
#include "srchpath.h"			// RunFM2Util
#include "misc.h"			// Broadcast, CheckMenu, CurrentRecord, SayFilter, SaySort
				// DrawTargetEmphasis, IsFm2Window
#include "select.h"			// SelectAll, SelectList
#include "findrec.h"			// ShowCnrRecord
#include "walkem.h"			// WalkExtractDlgProc
#include "droplist.h"			// AcceptOneDrop, CheckPmDrgLimit, DropHelp, GetOneDrop
#include "archive.h"			// ArchiveDlgProc
#include "common.h"			// CommonTextProc
#include "presparm.h"			// CopyPresParams
#include "defview.h"			// DefaultViewKeys
#include "systemf.h"			// ExecOnList
#include "filter.h"			// PickMaskDlgProc
#include "avl.h"			// SBoxDlgProc
#include "mkdir.h"			// SetDir
#include "collect.h"			// StartCollector
#include "viewer.h"			// StartMLEEditor
#include "newview.h"			// StartViewer
#include "i18nutil.h"			// commafmt
#include "copyf.h"			// unlinkf
#include "literal.h"			// wildcard
#include "wrappers.h"			// xrealloc
#include "misc.h"		// AdjustCnrColVis, QuickPopup, SetSortChecks, SwitchCommand
#include "select.h"			// DeselectAll, InvertAll
#include "strips.h"			// bstrip
#include "dirs.h"			// save_dir2
#include "fortify.h"
#include "excputil.h"			// 06 May 08 SHL added

#define ARCFLAGS_REALDIR    0x00000001
#define ARCFLAGS_PSEUDODIR  0x00000002
#define CON_COLS                6
#define EXTRA_ARCRECORD_BYTES   (sizeof(ARCITEM) - sizeof(MINIRECORDCORE))
#define NO_START_OF_ARCHIVER_LIST_STRING "None"
#define NO_END_OF_ARCHIVER_LIST_STRING   NO_START_OF_ARCHIVER_LIST_STRING

#pragma data_seg(DATA1)
static INT DefArcSortFlags;

// Data definitions
static PSZ pszSrcFile = __FILE__;
#pragma data_seg(GLOBAL1)
HWND ArcCnrMenu;
HWND ArcMenu;
CHAR ArcTempRoot[CCHMAXPATH];
BOOL fArcStuffVisible;
BOOL fFileNameCnrPath;

#pragma data_seg(GLOBAL2)
CHAR lastextractpath[CCHMAXPATH];
ULONGLONG ullDATFileSpaceNeeded = 10000;

typedef struct {

  HWND   hwndCnr;                //hwnd you want the message posted to
  HWND   hwndClient;             //hwnd calling this thread; NULL will work
  ULONG  RunFlags;               //runemf2 flags see systemf.h
  ULONG  msg;                    //Message to post
  UINT   uiLineNumber;
  PCSZ   pszSrcFile;
  CHAR   filename[CCHMAXPATH];   //file passed as MP1 message parameter (file selected)
  CHAR   *pszDirectory;          //Execution directory
  CHAR   *pszEnvironment;        //Enviroment -- NULL passes current
  CHAR   *pszCmdLine;             //Use sprintf to format multipart command line into single string
  CHAR   formatstring[40];       //Usally "%s"
}
WAITCHILD;

/** Creates a thread to wait for a child process to complete then posts a message and closes
 *  This function should only be used for runemf2 calls that include the WAIT flag
 */
VOID WaitChildThread(VOID * arg)
{
  WAITCHILD *WaitChild;
  HAB thab;
  CHAR *filename;
  INT ret;

  DosError(FERR_DISABLEHARDERR);

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif

  WaitChild = (WAITCHILD *) arg;
  if (WaitChild) {
    filename = xstrdup(WaitChild->filename, pszSrcFile, __LINE__);
    thab = WinInitialize(0);
    if (thab) {
      IncrThreadUsage();
      priority_normal();
      ret = runemf2(WaitChild->RunFlags, WaitChild->hwndClient,
		    WaitChild->pszSrcFile, WaitChild->uiLineNumber,
		    WaitChild->pszDirectory, WaitChild->pszEnvironment,
		    WaitChild->formatstring, WaitChild->pszCmdLine);
      if (ret != -1) {
	if (IsFile(WaitChild->filename) == 1)
	  PostMsg(WaitChild->hwndCnr, WaitChild->msg, MPFROMP(filename), MPVOID);
      }
      DecrThreadUsage();
      WinTerminate(thab);
    }
    xfree(WaitChild->pszDirectory, pszSrcFile, __LINE__);
    xfree(WaitChild->pszEnvironment, pszSrcFile, __LINE__);
    xfree(WaitChild->pszCmdLine, pszSrcFile, __LINE__);
    free(WaitChild);
  } // if WaitChild
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

static MRESULT EXPENTRY ArcErrProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2)
{
  ARCDUMP *ad;
  CHAR szQuotedArcName[CCHMAXPATH];

  switch (msg) {
  case WM_INITDLG:
    if (!mp2)
      WinDismissDlg(hwnd, 0);
    else {
      ad = (ARCDUMP *) mp2;
      WinSetWindowPtr(hwnd, QWL_USER, ad);
      if (ad->errmsg)
	WinSetDlgItemText(hwnd, ARCERR_TEXT, ad->errmsg);
      if (!ad->info->test)
	WinEnableWindow(WinWindowFromID(hwnd, ARCERR_TEST), FALSE);
      if (ad->listname) {
	MLEsetlimit(WinWindowFromID(hwnd, ARCERR_MLE), -1L);
	MLEsetformat(WinWindowFromID(hwnd, ARCERR_MLE), MLFIE_NOTRANS);
	MLEsetcurpos(WinWindowFromID(hwnd, ARCERR_MLE),
		     MLEgetlen(WinWindowFromID(hwnd, ARCERR_MLE)));
	MLEinsert(WinWindowFromID(hwnd, ARCERR_MLE),
		  GetPString(IDS_ARCHIVERREPORTTEXT));
	MLEinsertfile(WinWindowFromID(hwnd, ARCERR_MLE), ad->listname);
      }
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp) {
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_ARCERR, 0), MPFROMSHORT(HM_RESOURCEID));
      }
      break;

    case DID_OK:
      ad = WinQueryWindowPtr(hwnd, QWL_USER);
      WinDlgBox(HWND_DESKTOP, hwnd, ArcReviewDlgProc, FM3ModHandle,
		AD_FRAME, MPFROMP(ad));
      WinDismissDlg(hwnd, 0);
      break;

    case ARCERR_VIEW:
      ad = WinQueryWindowPtr(hwnd, QWL_USER);
      {
	CHAR *list[2];

	list[0] = ad->arcname;
	list[1] = NULL;
	if (TestBinary(ad->arcname)) {
	  if (*binview)
	    ExecOnList((HWND) 0, binview, WINDOWED | SEPARATE, NULL, NULL, list,
		       NULL, pszSrcFile, __LINE__);
	  else
	    StartMLEEditor(HWND_DESKTOP, 16 + 4 + 1, ad->arcname, hwnd);
	}
	else {
	  if (*viewer) {
	    ExecOnList((HWND) 0, viewer, WINDOWED | SEPARATE |
			    (fViewChild ? CHILD : 0), NULL,
			     NULL, list, NULL, pszSrcFile, __LINE__);
	  }
	  else
	    StartMLEEditor(HWND_DESKTOP, 8 + 4 + 1, ad->arcname, hwnd);
	}
      }
      break;

    case ARCERR_TEST: {
      CHAR *comspec;

      ad = WinQueryWindowPtr(hwnd, QWL_USER);
      comspec = getenv( "COMSPEC" );
      runemf2(SEPARATEKEEP | WINDOWED | MAXIMIZED,
	      hwnd, pszSrcFile, __LINE__, NULL, NULL,
	      "%s /k %s %s",
	      comspec, ad->info->test,
	      BldQuotedFileName(szQuotedArcName, ad->arcname));
      break;
    }
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static SHORT APIENTRY ArcSort(PMINIRECORDCORE pmrc1, PMINIRECORDCORE pmrc2,
			      PVOID pStorage)
{
  PARCITEM pai1 = (PARCITEM) pmrc1;
  PARCITEM pai2 = (PARCITEM) pmrc2;
  DIRCNRDATA *pdcd = (DIRCNRDATA *) pStorage;
  SHORT ret = 0;
  CHAR *pext, *ppext;
  INT sortFlags;

  if (!pdcd) {
    HWND hwndCnr = pai1->hwndCnr;

    pdcd = WinQueryWindowPtr(hwndCnr, QWL_USER);
    if (!pdcd) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      return ret;
    }
  }

  sortFlags = pdcd->sortFlags;		// Optimize

  if (sortFlags) {
    switch (sortFlags & (~SORT_REVERSE)) {
    case SORT_FIRSTEXTENSION:
      pext = strchr(pai1->pszFileName, '.');
      ppext = strchr(pai2->pszFileName, '.');
      if (!pext)
	pext = NullStr;
      if (!ppext)
	ppext = NullStr;
      ret = stricmp(pext, ppext);
      break;

    case SORT_LASTEXTENSION:
      pext = strrchr(pai1->pszFileName, '.');
      ppext = strrchr(pai2->pszFileName, '.');
      if (!pext)
	pext = NullStr;
      if (!ppext)
	ppext = NullStr;
      ret = stricmp(pext, ppext);
      break;

    case SORT_LWDATE:
      ret = TestCDates(&pai1->date, &pai1->time,
		       &pai2->date, &pai2->time);
      break;

    case SORT_SIZE:
      ret =
	(pai1->cbFile < pai2->cbFile) ? 1 : (pai1->cbFile ==
					     pai2->cbFile) ? 0 : -1;
      if (!ret)
	ret =
	  (pai1->cbComp < pai2->cbComp) ? 1 : (pai1->cbComp ==
					       pai2->cbComp) ? 0 : -1;
      break;

    case SORT_EASIZE:
      ret =
	(pai1->cbComp < pai2->cbComp) ? 1 : (pai1->cbComp ==
					     pai2->cbComp) ? 0 : -1;
      if (!ret)
	ret =
	  (pai1->cbFile < pai2->cbFile) ? 1 : (pai1->cbFile ==
					       pai2->cbFile) ? 0 : -1;
      break;
    }
    if (!ret)
      ret = (SHORT) stricmp(pai1->pszFileName, pai2->pszFileName);
    if (ret && (sortFlags & SORT_REVERSE))
      ret = ret > 0 ? -1 : 1;
    return ret;
  }
  return (SHORT) stricmp(pai1->pszFileName, pai2->pszFileName);
}

static INT APIENTRY ArcFilter(PMINIRECORDCORE rmini, PVOID arg)
{
  DIRCNRDATA *dcd = (DIRCNRDATA *) arg;
  PARCITEM r;
  register INT x;
  INT ret = FALSE;

  if (dcd && *dcd->mask.szMask) {
    r = (PARCITEM) rmini;
    if (dcd->mask.pszMasks[1]) {
      for (x = 0; dcd->mask.pszMasks[x]; x++) {
	if (*dcd->mask.pszMasks[x]) {
	  if (*dcd->mask.pszMasks[x] != '/') {
	    if (wildcard(r->pszFileName, dcd->mask.pszMasks[x], TRUE))
	      ret = TRUE;
	  }
	  else {
	    if (wildcard(r->pszFileName, dcd->mask.pszMasks[x] + 1, TRUE)) {
	      ret = FALSE;
	      break;
	    }
	  }
	}
      }
    }
    else {
      if (wildcard(r->pszFileName, dcd->mask.szMask, TRUE))
	ret = TRUE;
    }
  }
  else
    ret = TRUE;
  return ret;
}

static MRESULT EXPENTRY ArcFrameWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
					MPARAM mp2)
{
  return CommonFrameWndProc(ARC_CNR, hwnd, msg, mp1, mp2);
}

static BOOL IsArcThere(HWND hwnd, CHAR * arcname)
{
  if (arcname) {
    if (IsFile(arcname) != 1) {
      saymsg(MB_CANCEL, hwnd,
	     GetPString(IDS_SAYWHATTEXT),
	     GetPString(IDS_ARCNOTTHERETEXT), arcname);
      return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}

/**
 * Free storage associated with archive container item
 * Caller is responsible for correcting pointers
 */

static VOID FreeArcItemData(PARCITEM pai)
{
  PSZ psz;

  if (pai->pszFileName && pai->pszFileName != NullStr) {
    psz = pai->pszFileName;
    pai->pszFileName = NULL;		// 08 Jul 08 SHL was NulStr
    free(psz);
  }
}

/**
 * Remove item(s) from archive container and free associated storage if requested
 * @param paiFirst points to first item to remove or NULL to remove all
 * @param usCnt is remove count or 0 to remove all
 */

static VOID RemoveArcItems(HWND hwnd, PARCITEM paiFirst, USHORT usCnt, USHORT usFlags)
{
  INT remaining = usCnt;
  PARCITEM pai;

  if ((usCnt && !paiFirst) || (!usCnt && paiFirst))
    Runtime_Error(pszSrcFile, __LINE__, "paiFirst %p usCnt %u mismatch", paiFirst, usCnt);
  else {
    // Free our buffers if free requested
    if (usFlags & CMA_FREE) {
      if (paiFirst)
	pai = paiFirst;
      else {
	pai = (PARCITEM)WinSendMsg(hwnd, CM_QUERYRECORD, MPVOID,
				   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
	if ((INT)pai == -1) {
	  Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,"CM_QUERYRECORD");
	  remaining = -1;
	  pai = NULL;
	}
      }
      while (pai) {
	FreeArcItemData(pai);
	pai = (PARCITEM)pai->rc.preccNextRecord;
	if (remaining && --remaining == 0)
	  break;
      }
    }
  }
  if (remaining != - 1) {
    remaining = (INT)WinSendMsg(hwnd, CM_REMOVERECORD, MPFROMP(&paiFirst),
                                MPFROM2SHORT(usCnt, usFlags));
    if (remaining == -1) {
      Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,
                "CM_REMOVERECORD hwnd %x pai %p cnt %u",
                hwnd, paiFirst, usCnt);
    }
  }
}

/**
 * Empty all records from an archive container and
 * free associated storage and free up field infos
 */

static VOID EmptyArcCnr(HWND hwnd)
{
#if 0 // fixme to be gone or to be configurable
  {
    int state = _heapchk();
    if (state != _HEAPOK)
      Runtime_Error(pszSrcFile, __LINE__, "heap corrupted %d", state);
    else
      DbgMsg(pszSrcFile, __LINE__, "_memavl %u", _memavl());
  }
#endif

  // Remove all ARCITEM records
  RemoveArcItems(hwnd, NULL, 0, CMA_FREE);

  // Use common code to remove rest
  EmptyCnr(hwnd);
}

//== FillArcCnr() generate archive content list and fill container window ==

static INT FillArcCnr(HWND hwndCnr, CHAR * arcname, ARC_TYPE ** arcinfo,
		      ULONGLONG * pullTotalBytes, volatile PCHAR pStopFlag)
{
  FILE *fp;
  HFILE oldstdout;
  HFILE newstdout;
  CHAR lonename[CCHMAXPATH + 2],
       *nsize, *osize, *fdate, *fname, *p, *pp, *arctemp;
  CHAR s[CCHMAXPATH * 2];
  CHAR TestStr[CCHMAXPATH * 2];
  BOOL gotstart;
  BOOL gotend;
  BOOL wasquote;
  BOOL nomove = FALSE;			// fixme to be gone?
  BOOL notest;
  INT highest = 0, fieldnum, counter = 0, numarcfiles = 0;
  PARCITEM lastpai;
  ARC_TYPE *info;
  ARC_TYPE *tinfo;
  ULONG apptype;
  APIRET rc;
  CHAR *mode;
  ULONG cnter = 0;

  if (!arcname || !arcinfo)
    return 0;

  info = *arcinfo;
  if (!info)
    info = find_type(arcname, NULL);
  arctemp = xmallocz(CCHMAXPATH, pszSrcFile, __LINE__);
  if (CheckDriveSpaceAvail(ArcTempRoot, ullDATFileSpaceNeeded, ullTmpSpaceNeeded) == 1)
    saymsg(MB_OK,
	   HWND_DESKTOP,
	   NullStr,
	   GetPString(IDS_ARCTMPDRIVESPACELIMITED),
	   ArcTempRoot);
  MakeTempName(arctemp, ArcTempRoot, 2);


ReTry:

  tinfo = NULL;
  numarcfiles = counter = highest = 0;
  gotstart = gotend = FALSE;
  lastpai = NULL;
  *pullTotalBytes = 0;
  if (!info || !info->list)
    Runtime_Error(pszSrcFile, __LINE__, NULL);
  else {
    RemoveArcItems(hwndCnr, NULL, 0, CMA_FREE | CMA_INVALIDATE | CMA_ERASE);
    *arcinfo = info;
    highest = info->osizepos;
    if (info->nsizepos > highest)
      highest = info->nsizepos;
    if (info->fdpos > highest)
      highest = info->fdpos;
    if (info->fnpos > highest)
      highest = info->fnpos;
    if (highest > 50) {
      saymsg(MB_ENTER | MB_ICONEXCLAMATION, HWND_DESKTOP,
	     GetPString(IDS_SHAMETEXT), GetPString(IDS_BUNGEDUPTEXT));
    }
    if (info->fnpos == -1)
      highest = 32767;

    DosError(FERR_DISABLEHARDERR);
    xDosForceDelete(arctemp);
    DosError(FERR_DISABLEHARDERR);

    strcpy(s, info->list);
    p = strchr(s, ' ');
    if (p)
      *p = 0;
    DosError(FERR_DISABLEHARDERR);
    if (!xDosQueryAppType(s, &apptype) &&
	(apptype & FAPPTYP_DOS ||
	 apptype & FAPPTYP_WINDOWSREAL ||
	 apptype & FAPPTYP_WINDOWSPROT ||
	 apptype & FAPPTYP_WINDOWSPROT31)) {
      p = GetCmdSpec(TRUE);
      runemf2(SEPARATE | INVISIBLE | MINIMIZED | BACKGROUND | WAIT,
	      hwndCnr, pszSrcFile, __LINE__, NULL, "DOS_BACKGROUND_EXECUTION=1",
	      "%s /C %s %s >%s",
	      p,			// shell
	      info->list,		// list command
	      BldQuotedFileName(s, arcname),
	      arctemp);
    }
    else {
      mode = "w";
      fp = xfopen(arctemp, mode, pszSrcFile, __LINE__, FALSE);
      if (!fp) {
	xfree(arctemp, pszSrcFile, __LINE__);
	return 0;
      }
      else {
	newstdout = -1;
	DosError(FERR_DISABLEHARDERR);
	rc = xDosDupHandle(fileno(stdout), &newstdout);
	if (rc) {
	  Dos_Error(MB_CANCEL, rc, hwndCnr, pszSrcFile, __LINE__,
		    PCSZ_DOSDUPHANDLE);
	  xfree(arctemp, pszSrcFile, __LINE__);
	  return 0;
	}
	else {
	  oldstdout = fileno(stdout);
	  DosError(FERR_DISABLEHARDERR);
	  rc = xDosDupHandle(fileno(fp), &oldstdout);
	  if (rc) {
	    Dos_Error(MB_CANCEL, rc, hwndCnr, pszSrcFile, __LINE__,
		      PCSZ_DOSDUPHANDLE);
	    xfree(arctemp, pszSrcFile, __LINE__);
	    return 0;
	  }
          else {
            rc = 0;
 	    rc = SearchPathForFile(PCSZ_PATH, s, NULL);
 	    if (!rc) {
 	      cnter ++;
              runemf2(SEPARATE | INVISIBLE | MINIMIZED | BACKGROUND | WAIT,
                      hwndCnr, pszSrcFile, __LINE__, NULL, NULL,
                      "%s %s",
                      info->list,
                      BldQuotedFileName(s, arcname));
              if (cnter == 1) {
                if (info->test)
                  strcpy(TestStr, info->test);
                else {
                  strcpy(TestStr, "");
                  notest = TRUE;
                }
              }
              else if (notest && info->test) {
                strcpy(TestStr, info->test);
                notest = FALSE;
              }
 	    }
	    oldstdout = fileno(stdout);
	    DosError(FERR_DISABLEHARDERR);
	    xDosDupHandle(newstdout, &oldstdout);
	    DosClose(newstdout);
	    fclose(fp);
	  }
	}
      }
    }

    DosError(FERR_DISABLEHARDERR);
    mode = "r";
    fp = xfsopen(arctemp, mode, SH_DENYWR, pszSrcFile, __LINE__, TRUE);

    if (fp) {
      gotstart = !info->startlist ||		// If list has no start marker
		 !*info->startlist ||
		 (stricmp(info->startlist, NO_START_OF_ARCHIVER_LIST_STRING) == 0);

      while (!feof(fp) && !gotend && !*pStopFlag) {
	if (!xfgets_bstripcr(s, sizeof(s), fp, pszSrcFile, __LINE__))
	  break;
	if (!gotstart) {
	  if (!strcmp(s, info->startlist))
	    gotstart = TRUE;
	}
	else if (info->endlist && !strcmp(s, info->endlist))
	  gotend = TRUE;
	else {
	  // add to container
	  fname = NULL;
	  bstrip(s);
	  if (info->nameisfirst) {
	    strncpy(lonename, s, CCHMAXPATH + 2);
	    lonename[CCHMAXPATH + 1] = 0;
	    fname = lonename;
	    if (!xfgets_bstripcr(s, sizeof(s), fp, pszSrcFile, __LINE__))
	      break;
	    if (*fname == '\"') {
	      memmove(fname, fname + 1, strlen(fname) + 1);
	      p = strchr(fname, '\"');
	      if (p)
		*p = 0;
	    }
	  }
	  nsize = NULL;
	  osize = fdate = NullStr;
	  p = s;
	  for (fieldnum = 0; fieldnum <= highest; fieldnum++) {
            pp = p;
	    while (*pp && (*pp == ' ' || *pp == '\t'))	// skip leading
	      pp++;
            if (!*pp) {
              if (fieldnum == info->fnpos && (!strcmp(strupr(info->ext), "7Z") ||
                                             !strcmp(strupr(info->signature), "7Z")))
                fname = nsize;// GKY 7-30-13 Work around for missing nsize field for some members of archive
              break;
            }
	    wasquote = FALSE;
            p = pp;
	    while (*p && (wasquote ||
			  ((fieldnum != info->fnpos || !info->nameislast) ?
			   (*p != ' ' && *p != '\t') : TRUE))) {
	      if (*p == '\"') {
		if (!wasquote) {
		  wasquote = TRUE;
		  memmove(p, p + 1, strlen(p));
		  while (*p == ' ' || *p == '\t')
		    p++;
		}
		else {
		  memmove(p, p + 1, strlen(p));
		  break;
		}
	      }
              else if (*p)
		p++;
            }
	    if (*p) {
	      *p = 0;
	      p++;
            }
	    if (fieldnum == info->nsizepos)
	      nsize = pp;
	    else if (fieldnum == info->osizepos)
	      osize = pp;
	    else if (fieldnum == info->fdpos) {
	      fdate = pp;
	      if (info->fdflds > 1 && info->fdflds < 24) {
		INT y;

		if (*p) {
		  p--;
		  *p = ' ';
		  for (y = 0; y < info->fdflds - 1; y++) {
		    while (*p && (*p == ' ' || *p == '\t'))
		      p++;
		    while (*p && (*p != ' ' && *p != '\t'))
		      p++;
		    fieldnum++;
		  }
		  if (*p) {
		    *p = 0;
		    p++;
		  }
		}
	      }
	    }
            else if (fieldnum == info->fnpos) {
	      fname = pp;
	      if (pp && *pp == '*' && !*(pp + 1))	// workaround for LH.EXE
		fname = NULL;
	      if (info->nameislast)
		break;
	    }
	    else if ((!p || !*p) && info->fnpos == -1) {
	      fname = pp;
	      break;
	    }
	  } // for fldnum
	  if (info->nameisnext) {
	    if (!xfgets_bstripcr
		(lonename, sizeof(lonename), fp, pszSrcFile, __LINE__))
	      break;
	    fname = lonename;
	  }
	  // fixme to complain?
	  if (fname && *fname) {

	    RECORDINSERT ri;
	    PARCITEM pai;

	    pai = WinSendMsg(hwndCnr,
			     CM_ALLOCRECORD,
			     MPFROMLONG(EXTRA_ARCRECORD_BYTES),
			     MPFROMLONG(1L));
	    if (!pai) {
	      Runtime_Error(pszSrcFile, __LINE__, PCSZ_CM_ALLOCRECORD);
	      break;
	    }
	    else {
	      memset(pai, 0, sizeof(ARCITEM));
	      pai->hwndCnr = hwndCnr;
	      if (*fname == '*') {
		fname++;
		pai->flags = ARCFLAGS_REALDIR;
	      }
	      if (fname[strlen(fname) - 1] == '\\' ||
		  fname[strlen(fname) - 1] == '/')
		pai->flags = ARCFLAGS_REALDIR;
	      pai->pszFileName = xstrdup(fname,pszSrcFile, __LINE__);
#	      ifdef FORTIFY
	      // Will be freed by WM_DESTROY
	      Fortify_SetOwner(pai->pszFileName, 1);
	      // Fortify_ChangeScope(pai->pszFileName, -1);
#	      endif

	      pai->pszDisplayName = pai->pszFileName;
	      pai->rc.pszIcon = pai->pszDisplayName;
	      if (fdate)
		strcpy(pai->szDate, fdate);
	      // pai->pszFileName = pai->pszFileName;
	      pai->rc.pszIcon = pai->pszFileName;
	      pai->rc.hptrIcon = (pai->flags & ARCFLAGS_REALDIR) != 0 ?
		hptrDir : hptrFile;
	      pai->pszDate = pai->szDate;
	      if (osize)
		pai->cbFile = atol(osize);
	      if (nsize)
		pai->cbComp = atol(nsize);
	      if (info->datetype && fdate && *fdate)
	        ArcDateTime(fdate, info->datetype, &pai->date, &pai->time);
	      memset(&ri, 0, sizeof(RECORDINSERT));
	      ri.cb = sizeof(RECORDINSERT);
	      ri.pRecordOrder = (PRECORDCORE) CMA_END;
	      ri.pRecordParent = (PRECORDCORE) NULL;
	      ri.zOrder = (USHORT) CMA_TOP;
	      ri.cRecordsInsert = 1L;
	      ri.fInvalidateRecord = FALSE;
	      if (WinSendMsg(hwndCnr,
			     CM_INSERTRECORD, MPFROMP(pai), MPFROMP(&ri))) {
		*pullTotalBytes += pai->cbFile;
	      }
	      numarcfiles++;
	      if (!(++counter % 50)) {
		if (!lastpai)
		  lastpai = pai;
		WinSendMsg(hwndCnr,
			   CM_INVALIDATERECORD,
			   lastpai,
			   MPFROM2SHORT(10, CMA_ERASE | CMA_REPOSITION));
		lastpai = pai;
	      }
	      // Avoid hogging system for large archive
	      if (numarcfiles == 100)
		priority_idle();
	    }
	  }
	}
      }					// while !eof

      fclose(fp);

      if (*pStopFlag)
	numarcfiles = 0;		// Request close
      else if (!numarcfiles || !gotstart
               || (!gotend && info->endlist && *info->endlist &&
                   (stricmp(info->endlist, NO_END_OF_ARCHIVER_LIST_STRING)))) {
	// Oops
	ARCDUMP ad;
	CHAR errstr[CCHMAXPATH + 256];

	// Try for alternate archiver
	tinfo = info;
	do {
	  tinfo = tinfo->next;
	  if (tinfo)
	    tinfo = find_type(arcname, tinfo);
	  if (tinfo) {
	    DosError(FERR_DISABLEHARDERR);
	    xDosForceDelete(arctemp);
	    info = tinfo;
	    goto ReTry;
	  }
        } while (tinfo);
        if (!fAlertBeepOff)
          DosBeep(750, 50);		// wake up user

        if (cnter > 0) {
          CHAR Temp[CCHMAXPATH + 2];

          sprintf(errstr, GetPString(IDS_ARCERRORINFOTEXT),
                  arcname,
                  !gotstart ? GetPString(IDS_NOGOTSTARTTEXT) : NullStr,
                  !numarcfiles ? GetPString(IDS_NOARCFILESFOUNDTEXT) : NullStr,
                  !gotend ? GetPString(IDS_NOENDOFLISTTEXT) : NullStr,
                  !notest ? NullStr : GetPString(IDS_ARCNOTEST));
          memset(&ad, 0, sizeof(ARCDUMP));
          ad.info = info;
          strcpy(ad.listname, arctemp);
          strcpy(ad.arcname, arcname);
          if (!notest) {
            strcpy(Temp, info->test);
            info->test = xstrdup(TestStr, pszSrcFile, __LINE__);
          }
          else if (rc) {
            strcpy(Temp, info->test);
            info->test = NULL;
          }
          ad.errmsg = errstr;
          WinDlgBox(HWND_DESKTOP,
                    hwndCnr,
                    ArcErrProc, FM3ModHandle, ARCERR_FRAME, MPFROMP(&ad));
          if (!notest || rc)
            info->test = xstrdup(Temp, pszSrcFile, __LINE__);
        }
        else
 	  saymsg(MB_OK, HWND_DESKTOP, GetPString(IDS_ARCMISSINGEXE),
                 GetPString(IDS_ARCMISSINGEXEVERBOSE));
      }
      else if (!nomove && tinfo) {
	// if we got a false hit, move working hit to top
	tinfo = info->next;
	info->next = arcsighead;
	arcsighead->prev = info;
	if (tinfo)
	  tinfo->next->prev = info->prev;
	info->prev->next = tinfo;
	info->prev = NULL;
	arcsighead = info;
	rewrite_archiverbb2(NULL);	// Rewrite with warning
      }
    }					// if opened

    DosError(FERR_DISABLEHARDERR);
    xDosForceDelete(arctemp);
    xfree(arctemp, pszSrcFile, __LINE__);
  }

  if (numarcfiles)
    priority_normal();

  return numarcfiles;
} // FillArcCnr

MRESULT EXPENTRY ArcTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static BOOL emphasized = FALSE;
  static HWND hwndButtonPopup = (HWND) 0;
  static ULONG timestamp = ULONG_MAX;
  static USHORT lastid = 0;

  switch (msg) {
  case WM_CREATE:
    return CommonTextProc(hwnd, msg, mp1, mp2);

  case WM_COMMAND:
    return WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				      ARC_CNR), msg, mp1, mp2);

  case UM_CONTEXTMENU:
  case WM_CONTEXTMENU:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id) {
      case DIR_SELECTED:
      case DIR_VIEW:
      case DIR_SORT:
	{
	  POINTL ptl = { 0, 0 };
	  SWP swp;
	  DIRCNRDATA *dcd;

	  if (hwndButtonPopup)
	    WinDestroyWindow(hwndButtonPopup);
	  if (id == DIR_SELECTED)
	    id = DIR_RESTORE;
	  if (id == lastid) {

	    ULONG check;

	    DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &check,
			    sizeof(check));
	    if (check < timestamp + 500) {
	      lastid = 0;
	      goto MenuAbort;
	    }
	  }
	  hwndButtonPopup = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, id);
	  if (hwndButtonPopup) {
	    WinSetWindowUShort(hwndButtonPopup, QWS_ID, id);
	    dcd = WinQueryWindowPtr(WinWindowFromID(WinQueryWindow(hwnd,
								   QW_PARENT),
						    ARC_CNR), QWL_USER);
	    if (id == DIR_SORT) {
	      if (dcd)
		SetSortChecks(hwndButtonPopup, dcd->sortFlags);
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SORTNONE, FALSE), MPVOID);
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SORTNAME, FALSE), MPVOID);
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SORTLADATE, FALSE), MPVOID);
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SORTCRDATE, FALSE), MPVOID);
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SORTDIRSFIRST, FALSE), MPVOID);
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SORTDIRSLAST, FALSE), MPVOID);
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SORTSUBJECT, FALSE), MPVOID);
	      WinSendMsg(hwndButtonPopup,
			 MM_SETITEMTEXT,
			 MPFROM2SHORT(IDM_SORTEASIZE, 0),
			 MPFROMP(GetPString(IDS_COMPRESSEDSIZEMENUTEXT)));
	      WinSendMsg(hwndButtonPopup,
			 MM_SETITEMTEXT,
			 MPFROM2SHORT(IDM_SORTLWDATE, 0),
			 MPFROMP(GetPString(IDS_DATEMENUTEXT)));
	    }
	    ptl.x = 0;
	    if (WinPopupMenu(HWND_OBJECT,
			     HWND_OBJECT,
			     hwndButtonPopup, -32767, -32767, 0, 0)) {
	      WinQueryWindowPos(hwndButtonPopup, &swp);
	      ptl.y = -(swp.cy + 2);
	    }
	    else {
	      WinQueryWindowPos(hwnd, &swp);
	      ptl.y = swp.cy + 2;
	    }
	    if (WinPopupMenu(hwnd,
			     hwnd,
			     hwndButtonPopup,
			     ptl.x,
			     ptl.y,
			     0,
			     PU_HCONSTRAIN | PU_VCONSTRAIN |
			     PU_KEYBOARD | PU_MOUSEBUTTON1)) {
	      CenterOverWindow(hwndButtonPopup);
	      PaintRecessedWindow(hwnd, (HPS) 0, FALSE, FALSE);
	    }
	  }
	}
	break;
      default:
	PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				ARC_CNR),
		WM_CONTROL, MPFROM2SHORT(ARC_CNR, CN_CONTEXTMENU), MPVOID);
	break;
      }
    }
  MenuAbort:
    if (msg == UM_CONTEXTMENU)
      return 0;
    break;

  case WM_MENUEND:
    if (hwndButtonPopup == (HWND) mp2) {
      lastid = WinQueryWindowUShort((HWND) mp2, QWS_ID);
      WinDestroyWindow(hwndButtonPopup);
      hwndButtonPopup = (HWND) 0;
      DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &timestamp,
		      sizeof(timestamp));
      switch (lastid) {
      case DIR_VIEW:
      case DIR_SORT:
      case DIR_RESTORE:
      case DIR_SELECTED:
	PaintRecessedWindow(hwnd, (HPS) 0, TRUE, FALSE);
	break;
      }
    }
    break;

  case WM_MOUSEMOVE:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);
      PCSZ s = NULL;

      if (fOtherHelp) {
	if ((!hwndBubble ||
	     WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd) &&
	    !WinQueryCapture(HWND_DESKTOP)) {
	  switch (id) {
	  case DIR_TOTALS:
	    s = GetPString(IDS_ARCCNRTOTALSHELPTEXT);
	    break;
	  case DIR_SELECTED:
	    s = GetPString(IDS_ARCCNRSELECTEDHELPTEXT);
	    break;
	  case DIR_VIEW:
	    s = GetPString(IDS_ARCCNRVIEWHELPTEXT);
	    break;
	  case DIR_SORT:
	    s = GetPString(IDS_DIRCNRSORTHELP);
	    break;
	  case DIR_FILTER:
	    s = GetPString(IDS_DIRCNRFILTERHELP);
	    break;
	  case DIR_FOLDERICON:
	    s = GetPString(IDS_ARCCNRFOLDERHELPTEXT);
	    break;
	  default:
	    break;
	  }
	  if (s)
	    MakeBubble(hwnd, TRUE, s);
	  else if (hwndBubble)
	    WinDestroyWindow(hwndBubble);
	}
      }
      switch (id) {
      case DIR_FILTER:
      case DIR_SORT:
      case DIR_VIEW:
      case DIR_SELECTED:
      case DIR_FOLDERICON:
	return CommonTextButton(hwnd, msg, mp1, mp2);
      }
    }
    break;

  case WM_BUTTON3UP:
  case WM_BUTTON1UP:
  case WM_BUTTON1DOWN:
  case WM_BUTTON3DOWN:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id) {
      case DIR_FILTER:
      case DIR_SORT:
      case DIR_VIEW:
      case DIR_SELECTED:
      case DIR_FOLDERICON:
	return CommonTextButton(hwnd, msg, mp1, mp2);
      }
    }
    break;

  case UM_CLICKED:
  case UM_CLICKED3:
    {
      USHORT id, cmd = 0;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id) {
      case DIR_FOLDERICON:
	switch (msg) {
	case WM_BUTTON3CLICK:
	case WM_CHORD:
	  cmd = IDM_RESCAN;
	  break;
	default:
	  if ((SHORT2FROMMP(mp2) & KC_ALT) != 0)
	    cmd = IDM_WINDOWDLG;
	  else
	    cmd = IDM_WALKDIR;
	  break;
	}
	break;
      case DIR_VIEW:
      case DIR_SORT:
      case DIR_SELECTED:
	PostMsg(hwnd, UM_CONTEXTMENU, MPVOID, MPVOID);
	break;
      case DIR_FILTER:
	cmd = IDM_FILTER;
	break;
      default:
	break;
      }
      if (cmd)
	PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				ARC_CNR),
		WM_COMMAND, MPFROM2SHORT(cmd, 0), MPVOID);
    }
    return 0;

  case WM_BEGINDRAG:
  case DM_DROP:
  case DM_DRAGOVER:
  case DM_DRAGLEAVE:
  case DM_DROPHELP:
    if (msg == DM_DRAGOVER) {
      if (!emphasized) {
	emphasized = TRUE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
    }
    else if (msg != WM_BEGINDRAG) {
      if (emphasized) {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
    }
    switch (WinQueryWindowUShort(hwnd, QWS_ID)) {
    case DIR_FOLDERICON:
      switch (msg) {
      case DM_DRAGOVER:
	if (AcceptOneDrop(hwnd, mp1, mp2))
	  return MRFROM2SHORT(DOR_DROP, DO_MOVE);
	return MRFROM2SHORT(DOR_NODROP, 0);	// Drop not valid
      case DM_DROPHELP:
	DropHelp(mp1, mp2, hwnd, GetPString(IDS_ARCCNRFOLDERDROPHELPTEXT));
	return 0;
      case DM_DROP:
	{
	  char szFrom[CCHMAXPATH + 2];

	  if (emphasized) {
	    emphasized = FALSE;
	    DrawTargetEmphasis(hwnd, emphasized);
	  }
	  if (GetOneDrop(hwnd, mp1, mp2, szFrom, sizeof(szFrom)))
	    WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				       ARC_CNR),
		       WM_COMMAND,
		       MPFROM2SHORT(IDM_SWITCH, 0), MPFROMP(szFrom));
	}
	return 0;
      default:
	return PFNWPStatic(hwnd, msg, mp1, mp2);
      }
    default:
      {
	CNRDRAGINFO cnd;
	USHORT dcmd;

	switch (msg) {
	case DM_DROP:
	  dcmd = CN_DROP;
	  break;
	case DM_DRAGOVER:
	  dcmd = CN_DRAGOVER;
	  break;
	case DM_DRAGLEAVE:
	  dcmd = CN_DRAGLEAVE;
	  break;
	case DM_DROPHELP:
	  dcmd = CN_DROPHELP;
	  break;
	case WM_BEGINDRAG:
	  dcmd = CN_INITDRAG;
	  break;
	}
	cnd.pDragInfo = (PDRAGINFO) mp1;
	cnd.pRecord = NULL;
	return WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
			  WM_CONTROL,
			  MPFROM2SHORT(ARC_CNR, dcmd), MPFROMP(&cnd));
      }
    }
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ArcClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2)
{

  switch (msg) {
  case UM_CONTAINERHWND:
    return MRFROMLONG(WinWindowFromID(hwnd, ARC_CNR));

  case UM_VIEWSMENU:
    // fixme to disble menu items as needed
    return MRFROMLONG(CheckMenu(hwnd, &ArcCnrMenu, ARCCNR_POPUP));

  case UM_FILESMENU:
    // fixme to disble menu items as needed
    return MRFROMLONG(CheckMenu(hwnd, &ArcMenu, ARC_POPUP));

  case MM_PORTHOLEINIT:
  case WM_INITMENU:
  case UM_INITMENU:
  case UM_COMMAND:
  case UM_LOADFILE:
  case UM_UPDATERECORD:
  case UM_UPDATERECORDLIST:
  case WM_COMMAND:
  case WM_CONTROL:
  case WM_CLOSE:
    return WinSendMsg(WinWindowFromID(hwnd, ARC_CNR), msg, mp1, mp2);

  case WM_PSETFOCUS:
  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, ARC_CNR));
    break;

  case WM_PAINT:
    {
      HPS hps;
      RECTL rcl;

      hps = WinBeginPaint(hwnd, (HPS) 0, NULL);
      if (hps) {
	WinQueryWindowRect(hwnd, &rcl);
	WinFillRect(hps, &rcl, CLR_PALEGRAY);
	CommonTextPaint(hwnd, hps);
	WinEndPaint(hps);
      }
    }
    break;

  case UM_SIZE:
  case WM_SIZE:
    if (msg == UM_SIZE) {

      SWP swp;

      WinQueryWindowPos(hwnd, &swp);
      mp1 = MPFROM2SHORT(swp.cx, swp.cy);
      mp2 = MPFROM2SHORT(swp.cx, swp.cy);
    }
    {
      USHORT cx, cy, bx;

      cx = SHORT1FROMMP(mp2);
      cy = SHORT2FROMMP(mp2);
      WinSetWindowPos(WinWindowFromID(hwnd, ARC_CNR), HWND_TOP,
		      0,
		      22, cx, cy - (24 + 22), SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, ARC_EXTRACTDIR), HWND_TOP,
		      0, 0, cx, 22, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_FOLDERICON), HWND_TOP,
		      2, cy - 22, 24, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_TOTALS), HWND_TOP,
		      29,
		      cy - 22,
		      (cx / 3) - 2, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_SELECTED), HWND_TOP,
		      29 + (cx / 3) + 2,
		      cy - 22,
		      (cx / 3) - 2, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      bx = (cx - (29 + (((cx / 3) + 2) * 2))) / 3;
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_VIEW), HWND_TOP,
		      29 + (((cx / 3) + 2) * 2),
		      cy - 22, bx - 4, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_SORT), HWND_TOP,
		      29 + (((cx / 3) + 2) * 2) + bx,
		      cy - 22, bx - 4, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_FILTER), HWND_TOP,
		      29 + (((cx / 3) + 2) * 2) + (bx * 2),
		      cy - 22, bx - 4, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
    }
    CommonTextPaint(hwnd, (HPS) 0);
    if (msg == UM_SIZE) {
      WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), HWND_TOP, 0, 0, 0, 0,
		      SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE);
      return 0;
    }
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ArcObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *dcd;
  PSZ psz;
  CHAR szQuotedArcName[CCHMAXPATH];
  CHAR szQuotedMemberName[CCHMAXPATH];

  switch (msg) {
  case WM_CREATE:
    DbgMsg(pszSrcFile, __LINE__, "WM_CREATE mp1 %p mp2 %p", mp1, mp2);	// 18 Jul 08 SHL fixme
    break;

  case DM_PRINTOBJECT:
  case DM_DISCARDOBJECT:
    dcd = INSTDATA(hwnd);
    if (dcd) {

      LISTINFO *li;
      CNRDRAGINFO cni;

      cni.pRecord = NULL;
      cni.pDragInfo = (PDRAGINFO) mp1;
      li = DoFileDrop(dcd->hwndCnr,
		      dcd->directory, FALSE, MPVOID, MPFROMP(&cni));
      CheckPmDrgLimit(cni.pDragInfo);
      if (li) {
	li->type = (msg == DM_DISCARDOBJECT) ? IDM_DELETE : IDM_PRINT;
	if (!li->list ||
	    !li->list[0] || !PostMsg(hwnd, UM_ACTION, MPFROMP(li), MPVOID))
	  FreeListInfo(li);
	else
	  return MRFROMLONG(DRR_SOURCE);
      }
    }
    return MRFROMLONG(DRR_TARGET);

  case DM_RENDERPREPARE:
    return (MRESULT) TRUE;

  case DM_RENDER:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd && dcd->info && dcd->info->extract && dcd->arcname) {

      PDRAGTRANSFER pdt = (PDRAGTRANSFER) mp1;
      CHAR filename[CCHMAXPATH];
      ULONG len;

      if (pdt->hwndClient && pdt->pditem && pdt->hstrSelectedRMF &&
	  pdt->hstrRenderToName) {
	if (pdt->usOperation == DO_COPY || pdt->usOperation == DO_MOVE) {
	  *filename = 0;
	  len = DrgQueryStrName(pdt->hstrSelectedRMF, CCHMAXPATH, filename);
	  filename[len] = 0;
	  if (strnicmp(filename, "OS2FILE,", 8)) {
	    *filename = 0;
	    len = DrgQueryStrName(pdt->hstrRenderToName, CCHMAXPATH, filename);
	    filename[len] = 0;
	    if (len && *filename) {
	      psz = xstrdup(filename, pszSrcFile, __LINE__);
	      if (psz) {
		PostMsg(hwnd, UM_RENDER, MPFROMP(pdt), MPFROMP(psz));
		return (MRESULT) TRUE;
	      }
	    }
	  }
	}
	pdt->fsReply = DMFL_RENDERRETRY;
      }
    }
    return (MRESULT) FALSE;

  case UM_RENDER:
    {
      PDRAGTRANSFER pdt = (PDRAGTRANSFER) mp1;
      USHORT usRes = DMFL_RENDERFAIL;

      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (dcd && dcd->info && dcd->info->extract && dcd->arcname) {

	CHAR *filename = (CHAR *) mp2, *p;
	ULONG len;
	CHAR membername[CCHMAXPATH], construct[CCHMAXPATH * 2];

	*membername = 0;
	len = DrgQueryStrName(pdt->pditem->hstrSourceName,
			      CCHMAXPATH, membername);
	membername[len] = 0;
	if (*membername && len && filename) {
	  unlinkf(filename);
	  strcpy(construct, filename);
	  p = strrchr(filename, '\\');
	  if (!p)
	    *construct = 0;
	  else {
	    if (p == filename || *(p - 1) == ':')
	      p++;
	    *p = 0;
	  }
	  runemf2(SEPARATE | WINDOWED | ASYNCHRONOUS | WAIT |
		  (fArcStuffVisible ? 0 : BACKGROUND | MINIMIZED),
		  dcd->hwndClient, pszSrcFile, __LINE__, construct, NULL,
		  "%s %s %s",
		  dcd->info->extract,
		  BldQuotedFileName(szQuotedArcName, dcd->arcname),
		  BldQuotedFileName(szQuotedMemberName, membername));
	  BldFullPathName(construct, construct, membername);
	  if (IsFile(construct) != -1) {
	    rename(construct, filename);
	    unlinkf(construct);
	    if (IsFile(filename) != -1)
	      usRes = DMFL_RENDEROK;
	  }
	}
      }
      xfree((CHAR *)mp2, pszSrcFile, __LINE__);
      PostMsg(pdt->hwndClient, DM_RENDERCOMPLETE, MPFROMP(pdt),
	      MPFROM2SHORT(usRes, 0));
    }
    return 0;

  case UM_SETUP:
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!dcd) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    }
    else {
#     ifdef FORTIFY
      Fortify_BecomeOwner(dcd);
#     endif
      // set unique id
      WinSetWindowUShort(hwnd, QWS_ID, ARCOBJ_FRAME + (ARC_FRAME - dcd->id));
      dcd->hwndObject = hwnd;		// pass back hwnd
      if (ParentIsDesktop(hwnd, dcd->hwndParent))
	DosSleep(100); //05 Aug 07 GKY 250 // Avoid race?
    }
    return 0;

  case UM_RESCAN:
    /**
     * populate container
     */
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
      if (mp1)
	strcpy(dcd->arcname, (CHAR *) mp1);	// Update name on request
      dcd->ullTotalBytes = dcd->totalfiles =
	dcd->selectedfiles = dcd->selectedbytes = 0;
      WinSetDlgItemText(dcd->hwndClient, DIR_TOTALS, "0");
      WinSetDlgItemText(dcd->hwndClient, DIR_SELECTED, "0 / 0k");
      dcd->totalfiles = FillArcCnr(dcd->hwndCnr,
				   dcd->arcname,
				   &dcd->info,
				   &dcd->ullTotalBytes, &dcd->stopflag);
      if (!dcd->totalfiles)
	PostMsg(dcd->hwndCnr, WM_CLOSE, MPVOID, MPVOID);
      else {
	dcd->arcfilled = TRUE;
	if (!PostMsg(dcd->hwndCnr, UM_RESCAN, MPVOID, MPVOID))
	  WinSendMsg(dcd->hwndCnr, UM_RESCAN, MPVOID, MPVOID);
	PostMsg(dcd->hwndCnr, UM_SETUP2, MPVOID, MPVOID);
	WinSendMsg(dcd->hwndCnr,
		   CM_INVALIDATERECORD,
		   MPVOID, MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
      }
    }
    return 0;

  case UM_SELECT:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case IDM_SELECTALL:
      case IDM_SELECTALLFILES:
	SelectAll(dcd->hwndCnr, TRUE, TRUE, NULL, NULL, TRUE);
	break;
      case IDM_DESELECTALL:
      case IDM_DESELECTALLFILES:
	DeselectAll(dcd->hwndCnr, TRUE, TRUE, NULL, NULL, TRUE);
	break;
      case IDM_DESELECTMASK:
      case IDM_SELECTMASK:
	{
	  MASK mask;
	  PARCITEM pci = (PARCITEM) mp2;

	  memset(&mask, 0, sizeof(MASK));
	  mask.fNoAttribs = TRUE;
	  mask.fNoDirs = TRUE;
	  mask.fText = TRUE;
	  strcpy(mask.prompt,
		 GetPString((SHORT1FROMMP(mp1) == IDM_SELECTMASK) ?
			    IDS_SELECTFILTERTEXT : IDS_DESELECTFILTERTEXT));
	  if (pci && (INT) pci != -1)
	    strcpy(mask.szMask, pci->pszFileName);
	  if (WinDlgBox(HWND_DESKTOP, dcd->hwndCnr, PickMaskDlgProc,
			FM3ModHandle, MSK_FRAME, MPFROMP(&mask))) {
	    if (SHORT1FROMMP(mp1) == IDM_SELECTMASK)
	      SelectAll(dcd->hwndCnr, TRUE, TRUE, mask.szMask, NULL, FALSE);
	    else
	      DeselectAll(dcd->hwndCnr, TRUE, TRUE, mask.szMask, NULL, FALSE);
	  }
        }
        break;

      case IDM_INVERT:
	InvertAll(dcd->hwndCnr);
	break;
      }
    }
    return 0;

  case UM_ENTER:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {

      CHAR *s = (CHAR *) mp1, *p, *pp; // filename[CCHMAXPATH];
      WAITCHILD *WaitChild;

      WaitChild = xmallocz(sizeof(WAITCHILD), pszSrcFile, __LINE__);
      if (!WaitChild)
	return 0;
      WaitChild->pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
      if (s) {
	if (!dcd->info->extract) {
	  Runtime_Error(pszSrcFile, __LINE__, "no extract");
	  free(s);
	  return 0;
	}
	sprintf(WaitChild->pszCmdLine, "%s %s %s",
		dcd->info->exwdirs ? dcd->info->exwdirs :
		dcd->info->extract,
		BldQuotedFileName(szQuotedArcName, dcd->arcname),
		BldQuotedFileName(szQuotedMemberName, s));
	if (!dcd->info->exwdirs) {
	  p = s;
	  p = strrchr(s, '\\');
	  pp = strrchr(s, '/');
	  if (p && pp)
	    p = max(p, pp);
	  else if (!p)
	    p = pp;
	  if (p)
	    memmove(s, p + 1, strlen(p + 1));
	}
        sprintf(WaitChild->filename, "%s\\%s", dcd->workdir, s);
        ForwardslashToBackslash(WaitChild->filename);
	free(s);
	WaitChild->RunFlags = SEPARATE | ASYNCHRONOUS | WAIT |
			      (fArcStuffVisible ? 0 : BACKGROUND);
	WaitChild->hwndClient = dcd->hwndClient;
	WaitChild->msg = UM_ENTER;
	WaitChild->uiLineNumber = __LINE__;
	WaitChild->pszSrcFile = pszSrcFile;
	WaitChild->pszDirectory = xstrdup(dcd->workdir, pszSrcFile, __LINE__);
	WaitChild->pszEnvironment = NULL;
	strcpy(WaitChild->formatstring, "%s");
	WaitChild->hwndCnr = dcd->hwndCnr;
	xbeginthread(WaitChildThread,
		     65536,
		     WaitChild,
		     pszSrcFile,
		     __LINE__);
      }
    }
    return 0;

  case UM_COMMAND:
    if (mp1) {
      if (PostMsg(hwnd, UM_ACTION, mp1, mp2))
	return (MRESULT) TRUE;
    }
    return 0;

  case UM_ACTION:
    DosError(FERR_DISABLEHARDERR);
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {

      LISTINFO *li = (LISTINFO *) mp1;
      register INT x;

      if (li && li->list && li->list[0]) {
	// printf("%x/r", li->type); fflush(stdout);	// 24 Sep 08 SHL
	switch (li->type) {
	case IDM_ARCHIVE:
	case IDM_ARCHIVEM:
	  {
	    DIRCNRDATA ad;
	    CHAR szBuffer[1025], *p;

	    if (!li->list[1] && !stricmp(li->list[0], dcd->arcname)) {
	      Runtime_Error(pszSrcFile, __LINE__, "arc to self");
	      break;
	    }
	    ad = *dcd;
	    ad.namecanchange = 0;
	    ad.fmoving = (li->type == IDM_ARCHIVEM);
            if (!WinDlgBox(HWND_DESKTOP, dcd->hwndClient, ArchiveDlgProc,
                           FM3ModHandle, ARCH_FRAME, (PVOID) & ad) || !*ad.arcname ||
                !*ad.command)	// we blew it
	      break;
	    // build the sucker
	    strcpy(szBuffer, ad.command);
	    strcat(szBuffer, " ");

	    BldQuotedFileName(szBuffer + strlen(szBuffer), ad.arcname);

	    p = &szBuffer[strlen(szBuffer)];	// Remeber where archiver name ends

	    if (ad.mask.szMask) {
	      strcat(szBuffer, " ");

	      BldQuotedFileName(szBuffer + strlen(szBuffer), ad.mask.szMask);
	    }
	    strcat(szBuffer, " ");
	    x = 0;

	    // Run commands avoiding command line overflow
	    while (li->list[x]) {

	      if (IsFile(li->list[x]))
		BldQuotedFileName(szBuffer + strlen(szBuffer), li->list[x]);
	      else
		 BldQuotedFullPathName(szBuffer + strlen(szBuffer), li->list[x], "*");

	      x++;
	      if (!li->list[x] || strlen(szBuffer) +
		  strlen(li->list[x]) + 5 > MaxComLineStrg) {
		runemf2(SEPARATE | WINDOWED | ASYNCHRONOUS |
			(fArcStuffVisible ? 0 : BACKGROUND | MINIMIZED),
			hwnd, pszSrcFile, __LINE__,
			NULL, NULL, "%s", szBuffer);
		*p = 0;
	      }
	      strcat(szBuffer, " ");
	    } // while

	    PostMsg(dcd->hwndCnr, UM_RESCAN, MPFROMSHORT(1), MPVOID);
	    Broadcast(WinQueryAnchorBlock(hwnd),
		      hwndMain, UM_UPDATERECORD, MPFROMP(ad.arcname), MPVOID);
	    Broadcast(WinQueryAnchorBlock(hwnd),
		      hwndMain,
		      UM_UPDATERECORDLIST, MPFROMP(li->list), MPVOID);
	  }
	  break;

	case IDM_REFRESH:
	case IDM_DELETE:
	  {
	    CHAR *endofit;
	    PSZ pszCmdLine;
	    INT z;
	    CHECKLIST ck;
	    CHAR prompt[CCHMAXPATH + 257];

	    if (!dcd->info->delete)
	      break;
	    memset(&ck, 0, sizeof(ck));
	    ck.size = sizeof(ck);
	    ck.list = li->list;
	    ck.cmd = li->type;
	    ck.prompt = prompt;
	    ck.flags = CHECK_ARCHIVE;
	    sprintf(prompt, GetPString(IDS_ARCCNRDELREFTEXT),
		    (li->type == IDM_DELETE) ?
		    GetPString(IDS_DELETELOWERTEXT) :
		    GetPString(IDS_REFRESHLOWERTEXT),
		    &"s"[li->list[1] == NULL],
		    dcd->arcname,
		    (li->type == IDM_DELETE) ?
		    GetPString(IDS_DELETELOWERTEXT) :
		    GetPString(IDS_REFRESHLOWERTEXT));
	    if (!WinDlgBox(HWND_DESKTOP, hwnd, CheckListProc,
			   FM3ModHandle, CHECK_FRAME, MPFROMP(&ck)))
	      break;
	    li->list = ck.list;
	    if (!li->list || !li->list[0])
	      break;
	    pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
	    if (!pszCmdLine)
	      break;
	    strcpy(pszCmdLine, li->type == IDM_DELETE ?
			 dcd->info->delete :
			 dcd->info->create);
	    strcat(pszCmdLine, " ");
	    BldQuotedFileName(pszCmdLine + strlen(pszCmdLine), dcd->arcname);
	    endofit = &pszCmdLine[strlen(pszCmdLine)];
	    z = 0;
	    do {
	      for (x = z; li->list[x] &&
		strlen(pszCmdLine) + strlen(li->list[x]) < 999; x++) {
		strcat(pszCmdLine, " ");
		BldQuotedFileName(pszCmdLine + strlen(pszCmdLine), li->list[x]);
	      }
	      z = x;
	      runemf2(SEPARATE | WINDOWED | ASYNCHRONOUS | WAIT |
		      (fArcStuffVisible ? 0 : BACKGROUND | MINIMIZED),
		      hwnd, pszSrcFile, __LINE__, NullStr, NULL, "%s", pszCmdLine);
	      *endofit = 0;
	      free(pszCmdLine);
	    } while (li->list[x]);
	    PostMsg(dcd->hwndCnr, UM_RESCAN, MPFROMSHORT(1), MPVOID);
	    Broadcast(WinQueryAnchorBlock(hwnd),
		      hwndMain,
		      UM_UPDATERECORD, MPFROMP(dcd->arcname), MPVOID);
	  }
	  break;

	case IDM_PRINT:
	case IDM_VIRUSSCAN:
	case IDM_VIEW:
	case IDM_MCIPLAY:
	case IDM_VIEWARCHIVE:
	case IDM_VIEWTEXT:
	case IDM_VIEWBINARY:
	case IDM_EDIT:
	case IDM_EDITTEXT:
	case IDM_EDITBINARY:
	case IDM_EXEC:
	case IDM_EXTRACTWDIRS:
	case IDM_EXTRACT:
	case IDM_OPENDEFAULT:
	case IDM_OPENSETTINGS:
	  {
	    CHAR *endofit, *ptr;
	    PSZ pszCmdLine;
	    INT z;
	    if ((li->type == IDM_EXTRACT && !li->info->extract) ||
		((li->type == IDM_VIEW || li->type == IDM_VIEWTEXT ||
		  li->type == IDM_VIEWBINARY || li->type == IDM_EDIT ||
		  li->type == IDM_VIEWARCHIVE || li->type == IDM_EDITTEXT ||
		  li->type == IDM_EDITBINARY || li->type == IDM_MCIPLAY) &&
		 (!li->info->extract && !li->info->exwdirs)) ||
		(li->type != IDM_EXTRACT && li->type != IDM_EDIT &&
		 li->type != IDM_VIEW && li->type != IDM_VIEWTEXT &&
		 li->type != IDM_VIEWBINARY &&
		 li->type != IDM_VIEWARCHIVE &&
		 li->type != IDM_EDITTEXT &&
		 li->type != IDM_EDITBINARY &&
		 li->type != IDM_MCIPLAY && !li->info->exwdirs)) {
	      Runtime_Error(pszSrcFile, __LINE__, "no cmd for request");
	      break;
	    }
	    if (li->type == IDM_EXTRACT || li->type == IDM_EXTRACTWDIRS) {

	      CHAR fullname[CCHMAXPATH * 2];
	      CHAR **exfiles = NULL;
	      UINT numfiles = 0, numalloc = 0;

	      if (li->targetpath && fFileNameCnrPath &&
		  stricmp(lastextractpath, li->targetpath)) {
		strcpy(lastextractpath, li->targetpath);
		SetDir(dcd->hwndParent, hwnd, li->targetpath, 1);
	      }
	      for (x = 0; li->list[x]; x++) {
		BldFullPathName(fullname, li->targetpath, li->list[x]);
		//Check if file already exists on disk warn if it does.
		if (IsFile(fullname) != -1) {
		  AddToList(li->list[x], &exfiles, &numfiles, &numalloc);
		  li->list = RemoveFromList(li->list, li->list[x]);
		  if (!li->list)
		    break;
		  x--;
		}
	      }
	      if (exfiles && numfiles) {

		CHECKLIST ckl;
		CHAR prompt[(CCHMAXPATH * 2) + 256];

		memset(&ckl, 0, sizeof(ckl));
		ckl.size = sizeof(ckl);
		ckl.list = exfiles;
		ckl.prompt = prompt;
		ckl.cmd = li->type;
		ckl.flags = CHECK_ARCHIVE;
		sprintf(prompt,
			GetPString(IDS_REPLACEWARNTEXT),
			&"s"[numfiles == 1],
			li->arcname, &"s"[numfiles != 1], li->targetpath);
		if (!WinDlgBox(HWND_DESKTOP, hwnd, CheckListProc,
			       FM3ModHandle, CHECK_FRAME, MPFROMP(&ckl))) {
		  if (ckl.list)
		    FreeList(ckl.list);
		  break;
		}
		else if (ckl.list)
		  li->list = CombineLists(li->list, ckl.list);
	      } // end check and warn
	    }
	    if (!li->list || !li->list[0])
	      break;
	    pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
	    if (!pszCmdLine)
	      break;
	    strcpy(pszCmdLine,
		   (li->type == IDM_EXTRACT ||
		    ((li->type == IDM_VIEW ||
		      li->type == IDM_VIEWTEXT ||
		      li->type == IDM_VIEWBINARY ||
		      li->type == IDM_VIEWARCHIVE ||
		      li->type == IDM_PRINT ||
		      li->type == IDM_EDIT ||
		      li->type == IDM_EDITTEXT ||
		      li->type == IDM_OPENDEFAULT ||
		      li->type == IDM_OPENSETTINGS ||
		      (li->type == IDM_EDITBINARY ||		// JBS No way for this () to be true??
		       li->type == IDM_MCIPLAY)) ||
		     !li->info->exwdirs)) ?
		     li->info->extract :
		     li->info->exwdirs);
	     strcat(pszCmdLine, " ");
	     BldQuotedFileName(pszCmdLine + strlen(pszCmdLine), li->arcname);
	     endofit = &pszCmdLine[strlen(pszCmdLine)];
	     z = 0;
	     do {
	       for (x = z; li->list[x] &&
		   strlen(pszCmdLine) + strlen(li->list[x]) < MaxComLineStrg - 1 ; x++) {
		strcat(pszCmdLine, " ");
		BldQuotedFileName(pszCmdLine + strlen(pszCmdLine), li->list[x]);
		ptr = li->list[x];
		while (*ptr) {
		  if (*ptr == '/')
		    *ptr = '\\';
		  ptr++;
		}
	      }
	      z = x;
	      runemf2(SEPARATE | WINDOWED | WAIT |
		      (fArcStuffVisible ? 0 : BACKGROUND | MINIMIZED),
		      hwnd, pszSrcFile, __LINE__,
		      li->targetpath, NULL, "%s", pszCmdLine);
	      *endofit = 0;
	    } while (li->list[x]);
	    if (li->type == IDM_EXTRACT || li->type == IDM_EXTRACTWDIRS) {
	      // update windows
	      for (x = 0; li->list[x]; x++) {

		CHAR *temp, *p;

                temp = li->list[x];
                ForwardslashToBackslash(temp);
		p = xmalloc(strlen(temp) + strlen(li->targetpath) + 2,
			    pszSrcFile, __LINE__);
		if (p) {
		  BldFullPathName(p, li->targetpath, temp);
		  li->list[x] = p;
		  free(temp);
		}
	      }
	      if (fFolderAfterExtract) {

		CHAR objectpath[CCHMAXPATH], *p;
		APIRET rc;

		GetDesktopName(objectpath, sizeof(objectpath));
		rc = WinDlgBox(HWND_DESKTOP, dcd->hwndParent, ObjCnrDlgProc,
			       FM3ModHandle, OBJCNR_FRAME,
			       MPFROMP(objectpath));
		if (rc) {
		  if (rc > 1)
		    strcpy(objectpath, "<WP_DESKTOP>");
		  p = NULL;
		  if (li->arcname) {
		    p = strrchr(li->arcname, '\\');
		    if (p)
		      p++;
		  }
		  MakeShadows(dcd->hwndParent, li->list, 2, objectpath, p);
		}
	      }
	      Broadcast(WinQueryAnchorBlock(hwnd),
			hwndMain,
			UM_UPDATERECORDLIST, MPFROMP(li->list), MPVOID);
	    }
	    else if (li->type == IDM_EXEC)
	      ExecOnList(hwnd,
			 li->runfile,
                         WINDOWED | SEPARATEKEEP | PROMPT,
                         li->targetpath, NULL, NULL, GetPString(IDS_EXECARCFILETITLETEXT),
			 pszSrcFile, __LINE__);
	    else if (li->type == IDM_VIRUSSCAN)
	      ExecOnList(hwnd, virus, PROMPT | WINDOWED | SEPARATEKEEP,
			 li->targetpath, NULL, NULL,
			 GetPString(IDS_VIRUSSCANARCHIVETITLETEXT),
			 pszSrcFile, __LINE__);
	    else if (li->type == IDM_VIEW || li->type == IDM_VIEWTEXT ||
		     li->type == IDM_VIEWBINARY || li->type == IDM_EDIT ||
		     li->type == IDM_EDITTEXT ||
		     li->type == IDM_VIEWARCHIVE ||
		     li->type == IDM_EDITBINARY ||
		     li->type == IDM_OPENDEFAULT ||
		     li->type == IDM_OPENSETTINGS ||
		     li->type == IDM_MCIPLAY || li->type == IDM_PRINT) {

	      CHAR *temp, *p;
	      for (x = 0; li->list[x]; x++) {
		if (!li->info->exwdirs) {
		  temp = li->list[x];
		  p = strrchr(li->list[x], '\\');
		  if (p) {
		    p++;
		    li->list[x] = xstrdup(p, pszSrcFile, __LINE__);
		    if (!li->list[x])
		      li->list[x] = temp;
		    else {
		      xfree(temp, pszSrcFile, __LINE__);
		    }
		  }
		}
		BldFullPathName(pszCmdLine, li->targetpath, li->list[x]);
		temp = li->list[x];
		li->list[x] = xstrdup(pszCmdLine, pszSrcFile, __LINE__);
		if (!li->list[x])
		  li->list[x] = temp;
		else
		  xfree(temp, pszSrcFile, __LINE__);
	      }
	      free(pszCmdLine);
	      if (li->type == IDM_VIEW || li->type == IDM_EDIT) {

		BOOL isit = TestBinary(li->list[0]);

		if (isit) {
		  if (li->type == IDM_VIEW)
		    li->type = IDM_VIEWBINARY;
		  else
		    li->type = IDM_EDITBINARY;
		}
		else {
		  if (li->type == IDM_VIEW)
		    li->type = IDM_VIEWTEXT;
		  else
		    li->type = IDM_EDITTEXT;
		}
	      }
	      if (li->type == IDM_MCIPLAY) {

		FILE *fp;
                CHAR szTempFile[CCHMAXPATH];
                CHAR *modew = "w";

                if (pTmpDir && !IsValidDir(pTmpDir))
                  DosCreateDir(pTmpDir, 0);
		BldFullPathName(szTempFile, pTmpDir, PCSZ_FM2PLAYTEMP);
		fp = xfopen(szTempFile, modew, pszSrcFile, __LINE__, FALSE);
		if (fp) {
		  fprintf(fp, "%s", ";AV/2-built FM2Play listfile\n");
		  for (x = 0; li->list[x]; x++)
		    fprintf(fp, "%s\n", li->list[x]);
		  fprintf(fp, ";end\n");
		  fclose(fp);
		  strrev(szTempFile);
		  strcat(szTempFile, "@/");
		  strrev(szTempFile);
		  RunFM2Util(PCSZ_FM2PLAYEXE, szTempFile);
		}
	      }
	      else if (li->type == IDM_PRINT) {
		strcpy(li->targetpath, printer);
		// 10 Dec 08 SHL correct error handling - looked backward
		if (xbeginthread(PrintListThread,
				 65536,
				 li,
				 pszSrcFile,
				 __LINE__) != -1)
		{
		  li = NULL;		// Ensure not freed here
		}

	      }
	      else if (li->type == IDM_VIEWARCHIVE) {

		ARC_TYPE *info;

		for (x = 0; li->list[x]; x++) {
		  if (IsFile(li->list[x]) == 1) {
		    info = NULL;	// Do not hide dups - fixme to know why?
		    if (WinDlgBox(HWND_DESKTOP, HWND_DESKTOP,
				  SBoxDlgProc, FM3ModHandle, ASEL_FRAME,
				  (PVOID) & info) && info) {
		      StartArcCnr(HWND_DESKTOP,
				  HWND_DESKTOP, li->list[x], 4, info);
		    }
		  }
		}
	      }
	      else if ((li->type == IDM_VIEWTEXT && *viewer) ||
		       (li->type == IDM_VIEWBINARY && *binview) ||
		       (li->type == IDM_EDITTEXT && *editor) ||
		       (li->type == IDM_EDITBINARY && *bined)) {
		ExecOnList(hwnd, ((li->type == IDM_VIEWTEXT) ? viewer :
				  (li->type == IDM_VIEWBINARY) ? binview :
				  (li->type == IDM_EDITTEXT) ? editor :
				  bined),
                           WINDOWED | SEPARATE, NULL, NULL, 
                           li->list,
			   NULL, pszSrcFile, __LINE__);
	      }
	      else if (li->type == IDM_OPENDEFAULT ||
		       li->type == IDM_OPENSETTINGS) {
		WORKER *wk;
#		ifdef FORTIFY
		Fortify_EnterScope();
#		endif
		wk = xmallocz(sizeof(WORKER), pszSrcFile, __LINE__);
		if (!wk)
		  FreeListInfo(li);
		else {
		  wk->size = sizeof(WORKER);
		  wk->hwndCnr = dcd->hwndCnr;
		  wk->hwndParent = dcd->hwndParent;
		  wk->hwndFrame = dcd->hwndFrame;
		  wk->hwndClient = dcd->hwndClient;
		  wk->li = li;
		  strcpy(wk->directory, dcd->directory);
		  if (xbeginthread(Action,
				   122880,
				   wk,
				   pszSrcFile,
				   __LINE__) == -1)
		  {
		    free(wk);
		    FreeListInfo((LISTINFO *) mp1);
		  }
		}
#		ifdef FORTIFY
		Fortify_LeaveScope();
#		endif
	      }
	      else {
		if (li->hwnd) {

		  ULONG viewtype;

		  for (x = 0; li->list[x]; x++) {
		    if (x == 0) {
		      if (li->type == IDM_VIEWBINARY ||
			  li->type == IDM_EDITBINARY)
			viewtype = 16;
		      else
			viewtype = 8;
		    }
		    else
		      viewtype = 0;
#		    ifdef FORTIFY
		    Fortify_EnterScope();
#		    endif
		    temp = xstrdup(li->list[x], pszSrcFile, __LINE__);
		    if (temp) {
		      if (!PostMsg(WinQueryWindow(li->hwnd, QW_PARENT),
				   UM_LOADFILE,
				   MPFROMLONG(4L +
					      (li->type == IDM_VIEWTEXT ||
					       li->type == IDM_VIEWBINARY) +
					      viewtype), MPFROMP(temp)))
			free(temp);
		    }
#		    ifdef FORTIFY
		    DosSleep(1);	// Allow MassAction to take ownership
		    Fortify_LeaveScope();
#		    endif
		  }
		}
	      }
	    }
	  }
	  break;

	case IDM_FIND:
	  {
	    UINT numfiles = 0, numalloced = 0;
	    CHAR **list2 = NULL, fullname[CCHMAXPATH * 2], *p;

            for (x = 0; li->list[x]; x++) {
              ForwardslashToBackslash(li->list[x]);
	      BldFullPathName(fullname, dcd->directory, li->list[x]);
	      if (IsFile(fullname) != -1)
		if (AddToList(fullname, &list2, &numfiles, &numalloced))
		  break;
	      if (strchr(li->list[x], '\\')) {
		p = strrchr(li->list[x], '\\');
		if (p) {
		  p++;
		  if (*p) {
		    BldFullPathName(fullname, dcd->directory, p);
		    if (IsFile(fullname) != -1)
		      if (AddToList(fullname, &list2, &numfiles, &numalloced))
			break;
		  }
		}
	      }
	    }
	    if (!numfiles || !list2)
	      Runtime_Error(pszSrcFile, __LINE__, "no files or list");
	    else {
	      WinSendMsg(dcd->hwndCnr, WM_COMMAND,
			 MPFROM2SHORT(IDM_COLLECTOR, 0), MPVOID);
	      DosSleep(10); 
	      if (Collector) {
		if (!PostMsg(Collector, WM_COMMAND,
			     MPFROM2SHORT(IDM_COLLECTOR, 0), MPFROMP(list2)))
		  FreeList(list2);
	      }
	      else
		FreeList(list2);
	    }
	  }
	  break;
	} // switch
      }
      if (li && li->type != IDM_OPENDEFAULT && li->type != IDM_OPENSETTINGS)
      {
	FreeListInfo(li);
      }
    }
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    break;

  case WM_DESTROY:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
      if (*dcd->workdir) {
	DosSleep(16);
	wipeallf(TRUE, "%s\\*", dcd->workdir);
	if (rmdir(dcd->workdir)) {
	  DosSleep(100);
	  wipeallf(TRUE, "%s\\*", dcd->workdir);
	  rmdir(dcd->workdir);
	}
      }
      FreeList(dcd->lastselection);
      WinSendMsg(dcd->hwndCnr, UM_CLOSE, MPVOID, MPVOID);
      WinSetWindowPtr(dcd->hwndCnr, QWL_USER, NULL);	// 13 Apr 10 SHL Set NULL before freeing dcd
      free(dcd);
#     ifdef FORTIFY
      Fortify_LeaveScope();
#     endif
    }
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    break;
  }					// switch
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

static MRESULT EXPENTRY ArcCnrWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				      MPARAM mp2)
{
  DIRCNRDATA *dcd = INSTDATA(hwnd);
  CHAR szQuotedArcName[CCHMAXPATH];

  switch (msg) {
  case DM_PRINTOBJECT:
  case DM_DISCARDOBJECT:
    if (dcd)
      return WinSendMsg(dcd->hwndObject, msg, mp1, mp2);
    else
      return MRFROMLONG(DRR_TARGET);

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (SHORT1FROMMP(mp1) & KC_KEYUP)
      return (MRESULT)TRUE;
    if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY) {
      switch (SHORT2FROMMP(mp2)) {
      case VK_DELETE:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_DELETE, 0), MPVOID);
	break;
      }
    }

    if (SearchContainer(hwnd, msg, mp1, mp2))
    	return (MRESULT)TRUE;		// Avoid default handler
    break;				// Let default handler see key too

  case WM_MOUSEMOVE:
  case WM_BUTTON1UP:
  case WM_BUTTON2UP:
  case WM_BUTTON3UP:
  case WM_CHORD:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_BUTTON1MOTIONEND:
    {
      CNRINFO cnri;

      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      if (WinSendMsg(hwnd,
		     CM_QUERYCNRINFO,
		     MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)))) {
	if (cnri.flWindowAttr & CV_DETAIL)
	  PrfWriteProfileData(fmprof,
			      appname,
			      "ArcCnrSplitBar",
			      (PVOID) & cnri.xVertSplitbar, sizeof(LONG));
      }
    }
    break;

  case WM_PRESPARAMCHANGED:
    PresParamChanged(hwnd, PCSZ_ARCCNR, mp1, mp2);
    break;

  case UM_UPDATERECORD:
  case UM_UPDATERECORDLIST:
    if (dcd && !IsArcThere(hwnd, dcd->arcname))
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    return 0;

  case WM_SETFOCUS:
    /**
     * put name of our window (archive name) on status line
     */
    if (dcd && hwndStatus && mp2)
      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    break;

  case UM_SETUP2:
    if (dcd && dcd->info) {
      if (dcd->info->fdpos == -1 || !dcd->info->datetype)
	dcd->sortFlags &= (~SORT_LWDATE);
      if (dcd->info->nsizepos == -1)
	dcd->sortFlags &= (~SORT_EASIZE);
      if (dcd->info->osizepos == -1)
	dcd->sortFlags &= (~SORT_SIZE);
      AdjustCnrColVis(hwnd,
		      GetPString(IDS_OLDSIZECOLTEXT),
		      dcd->info->osizepos != -1, FALSE);
      AdjustCnrColVis(hwnd,
		      GetPString(IDS_NEWSIZECOLTEXT),
		      dcd->info->nsizepos != -1, FALSE);
      // Display unsullied date/time string if type 0
      AdjustCnrColVis(hwnd,
		      GetPString(IDS_DATETIMECOLTEXT),
		      dcd->info->fdpos != -1 && !dcd->info->datetype, FALSE);
      // Display parsed date/time columns if type specified
      AdjustCnrColVis(hwnd,
        	      GetPString(IDS_TIMECOLTEXT),
        	      dcd->info->fdpos != -1 && dcd->info->datetype, FALSE);
      AdjustCnrColVis(hwnd,
        	      GetPString(IDS_DATECOLTEXT),
        	      dcd->info->fdpos != -1 && dcd->info->datetype, FALSE);
      WinSendMsg(hwnd, CM_INVALIDATEDETAILFIELDINFO, MPVOID, MPVOID);
    }
    return 0;

  case UM_RESCAN:
    if (dcd) {
      CNRINFO cnri;
      CHAR s[CCHMAXPATH * 2], tb[81], tf[81];
      PARCITEM pci;

      if (mp1) {
	PostMsg(dcd->hwndObject, UM_RESCAN, MPVOID, MPVOID);
	return 0;
      }
      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendMsg(hwnd,
		 CM_QUERYCNRINFO,
		 MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
      dcd->totalfiles = cnri.cRecords;
      commafmt(tf, sizeof(tf), dcd->selectedfiles);
      if (dcd->ullTotalBytes)
	CommaFmtULL(tb, sizeof(tb), dcd->selectedbytes, 'K');
      else
	*tb = 0;
      sprintf(s, "%s%s%s", tf, *tb ? " / " : NullStr, tb);
      WinSetDlgItemText(dcd->hwndClient, DIR_SELECTED, s);
      commafmt(tf, sizeof(tf), dcd->totalfiles);
      if (dcd->ullTotalBytes)
	CommaFmtULL(tb, sizeof(tb), dcd->ullTotalBytes, 'K');
      else
	*tb = 0;
      sprintf(s, "%s%s%s", tf, *tb ? " / " : NullStr, tb);
      WinSetDlgItemText(dcd->hwndClient, DIR_TOTALS, s);
      if (hwndStatus &&
	  dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent)) {
	sprintf(s, " [%s%s%s]%s%s%s  %s",
		tf,
		*tb ? " / " : NullStr,
		tb,
		*dcd->mask.szMask ? " (" : NullStr,
		*dcd->mask.szMask ? dcd->mask.szMask : NullStr,
		*dcd->mask.szMask ? ")" : NullStr, dcd->arcname);
	WinSetWindowText(hwndStatus, s);
	if (!ParentIsDesktop(hwnd, dcd->hwndParent)) {
	  pci = WinSendMsg(hwnd,
			   CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1) {
	    if (fSplitStatus && hwndStatus2) {
	      if (dcd->ullTotalBytes)
		CommaFmtULL(tb, sizeof(tb), pci->cbFile, ' ');
	      else
		*tb = 0;
	      sprintf(s, "%s%s%s%s",
		      *tb ? " " : NullStr,
		      tb, *tb ? "  " : NullStr, pci->pszFileName);
	      WinSetWindowText(hwndStatus2, s);
	    }
	    if (fMoreButtons)
	      WinSetWindowText(hwndName, pci->pszFileName);
	  }
	  else {
	    WinSetWindowText(hwndStatus2, NullStr);
	    WinSetWindowText(hwndName, NullStr);
	  }
	  WinSetWindowText(hwndDate, NullStr);
	  WinSetWindowText(hwndAttr, NullStr);
	}
      }
      if ((dcd->arcfilled && !dcd->totalfiles) ||
	  !IsArcThere(hwnd, dcd->arcname))
	PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    }
    return 0;

  case UM_SETUP:
    if (!dcd) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      return 0;
    }
    else {
      if (!dcd->hwndObject) {
	/**
	 * first time through -- set things up
	 */
	{
	  CHAR *p;
	  ULONG z, was;
	  APIRET rc;

	  rc = DosCreateDir(dcd->workdir, 0);
	  if (rc) {
	    if (rc == ERROR_ACCESS_DENIED) {
	      p = strrchr(dcd->workdir, '.');
	      if (p) {          /* jbs: What if there is no "."? Give up? */
		p++;
		was = strtoul(p, NULL, 16);
		for (z = 0; z < 99; z++) {
		  was++;
		  sprintf(p, "%03x", was);
		  rc = DosCreateDir(dcd->workdir, 0);
		  if (!rc || rc != ERROR_ACCESS_DENIED)
		    break;
		}
	      }
	    }
            if (rc) {
	      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
              return 0;
            }
	  }
	}
	RestorePresParams(hwnd, PCSZ_ARCCNR);
	dcd->mask.fNoAttribs = TRUE;
	dcd->mask.fNoDirs = TRUE;
	*dcd->mask.prompt = 0;
	{
	  PFIELDINFO pfi, pfiLastLeftCol;
	  ULONG numcols = CON_COLS;
	  CNRINFO cnri;
	  ULONG size;

	  pfi = WinSendMsg(hwnd,
			   CM_ALLOCDETAILFIELDINFO,
			   MPFROMLONG(numcols), NULL);
	  if (pfi) {

	    PFIELDINFO pfiFirst;
	    FIELDINFOINSERT fii;

	    pfiFirst = pfi;
	    pfi->flData = CFA_STRING | CFA_LEFT | CFA_FIREADONLY;
	    pfi->flTitle = CFA_CENTER;
	    pfi->pTitleData = (PSZ)GetPString(IDS_FILENAMECOLTEXT);
	    pfi->offStruct = FIELDOFFSET(ARCITEM, pszDisplayName);
	    pfiLastLeftCol = pfi;
	    pfi = pfi->pNextFieldInfo;
	    pfi->flData =
	      CFA_ULONG | CFA_RIGHT | CFA_SEPARATOR | CFA_FIREADONLY;
	    pfi->flTitle = CFA_CENTER;
	    pfi->pTitleData = (PSZ)GetPString(IDS_OLDSIZECOLTEXT);
	    pfi->offStruct = FIELDOFFSET(ARCITEM, cbFile);
	    pfi = pfi->pNextFieldInfo;
	    pfi->flData =
	      CFA_ULONG | CFA_RIGHT | CFA_SEPARATOR | CFA_FIREADONLY;
	    pfi->flTitle = CFA_CENTER;
	    pfi->pTitleData = (PSZ)GetPString(IDS_NEWSIZECOLTEXT);
	    pfi->offStruct = FIELDOFFSET(ARCITEM, cbComp);
	    pfi = pfi->pNextFieldInfo;
	    pfi->flData =
	      CFA_STRING | CFA_CENTER | CFA_SEPARATOR | CFA_FIREADONLY;
	    pfi->flTitle = CFA_CENTER | CFA_FITITLEREADONLY;
	    pfi->pTitleData = (PSZ)GetPString(IDS_DATETIMECOLTEXT);
	    pfi->offStruct = FIELDOFFSET(ARCITEM, pszDate);
	    pfi = pfi->pNextFieldInfo;
	    pfi->flData = CFA_DATE | CFA_RIGHT | CFA_FIREADONLY;
	    pfi->flTitle = CFA_CENTER;
	    pfi->pTitleData = (PSZ)GetPString(IDS_DATECOLTEXT);
	    pfi->offStruct = FIELDOFFSET(ARCITEM, date);
	    pfi = pfi->pNextFieldInfo;
	    pfi->flData = CFA_TIME | CFA_RIGHT | CFA_FIREADONLY;
	    pfi->flTitle = CFA_CENTER | CFA_FITITLEREADONLY;
	    pfi->pTitleData = (PSZ)GetPString(IDS_TIMECOLTEXT);
	    pfi->offStruct = FIELDOFFSET(ARCITEM, time);
	    memset(&fii, 0, sizeof(FIELDINFOINSERT));
	    fii.cb = sizeof(FIELDINFOINSERT);
	    fii.pFieldInfoOrder = (PFIELDINFO) CMA_FIRST;
	    fii.cFieldInfoInsert = (SHORT) numcols;
	    fii.fInvalidateFieldInfo = TRUE;
	    WinSendMsg(hwnd,
		       CM_INSERTDETAILFIELDINFO,
		       MPFROMP(pfiFirst), MPFROMP(&fii));
	    PostMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);

	    memset(&cnri, 0, sizeof(cnri));
	    cnri.cb = sizeof(CNRINFO);
	    cnri.pFieldInfoLast = pfiLastLeftCol;
	    cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET + 32;

	    size = sizeof(LONG);
	    PrfQueryProfileData(fmprof, appname, "ArcCnrSplitBar",
				&cnri.xVertSplitbar, &size);
	    if (cnri.xVertSplitbar <= 0)
	      cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET + 32;

	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT | CV_NAME));
	    cnri.flWindowAttr |= (CV_DETAIL | CA_DETAILSVIEWTITLES | CV_FLOW);
	    cnri.flWindowAttr &= (~(CA_ORDEREDTARGETEMPH |
				    CA_MIXEDTARGETEMPH));
	    cnri.pSortRecord = (PVOID) ArcSort;
	    WinSendMsg(hwnd,
		       CM_SETCNRINFO,
		       MPFROMP(&cnri),
		       MPFROMLONG(CMA_PFIELDINFOLAST |
				  CMA_XVERTSPLITBAR |
				  CMA_PSORTRECORD | CMA_FLWINDOWATTR));
	  }
	}
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(ArcSort), MPFROMP(dcd));
	if (xbeginthread(MakeObjWin,
			 245760,
			 dcd,
			 pszSrcFile,
			 __LINE__) == -1)
	{
	  PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	  return 0;
	}
	else
	  DosSleep(1);
	SayFilter(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				  DIR_FILTER), &dcd->mask, TRUE);
	SaySort(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				DIR_SORT), dcd->sortFlags, TRUE);
	DefArcSortFlags = dcd->sortFlags;	// Remember for new windows
      }
    }
    return 0;

  case UM_SETDIR:
    if (dcd) {

      CHAR s[CCHMAXPATH], *p;
      ULONG ret = 0;

      WinQueryDlgItemText(dcd->hwndClient, ARC_EXTRACTDIR, CCHMAXPATH, s);
      bstrip(s);
      MakeFullName(s);
      if (*s) {
	while ((p = strchr(s, '/')) != NULL)
	  *p = '\\';
	while (strlen(s) > 3 && s[strlen(s) - 1] == '\\')
	  s[strlen(s) - 1] = 0;
	if (stricmp(s, dcd->directory)) {
	  if (IsFullName(s)) {
	    if (driveflags[toupper(*s) - 'A'] &
		(DRIVE_NOTWRITEABLE | DRIVE_IGNORE | DRIVE_INVALID)) {
	      Runtime_Error(pszSrcFile, __LINE__, "drive %s bad", s);
	      WinSetDlgItemText(dcd->hwndClient,
				ARC_EXTRACTDIR, dcd->directory);
	      return 0;
	    }
	  }
	  if (!SetDir(dcd->hwndParent, hwnd, s, 0)) {
	    if (stricmp(dcd->directory, s)) {
	      DosRequestMutexSem(hmtxFM2Globals, SEM_INDEFINITE_WAIT);
	      strcpy(lastextractpath, s);
	      DosReleaseMutexSem(hmtxFM2Globals);
	    }
	    strcpy(dcd->directory, s);
	    if ((!isalpha(*s) || s[1] != ':') && *s != '.')
	      saymsg(MB_ENTER | MB_ICONASTERISK,
		     hwnd,
		     GetPString(IDS_WARNINGTEXT),
		     GetPString(IDS_SPECIFYDRIVETEXT));
	  }
	  else
	    ret = 1;
	}
      }
      WinSetDlgItemText(dcd->hwndClient, ARC_EXTRACTDIR, dcd->directory);
      return (MRESULT) ret;
    }
    return 0;

  case UM_ENTER:
    if (WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID)) {
      free(mp1);
      return 0;
    }
    SetShiftState();
    if (dcd && (CHAR *) mp1) {

      SWP swp;
      CHAR *filename = mp1;
      if (IsFile(filename) != 1) {
	free(mp1);
	return 0;
      }
      WinQueryWindowPos(dcd->hwndFrame, &swp);
      DefaultViewKeys(hwnd, dcd->hwndFrame, dcd->hwndParent, &swp, filename);
      if (fUnHilite)
	UnHilite(hwnd, FALSE, &dcd->lastselection, 0);
    }
    free(mp1);
    return 0;

  case WM_MENUEND:
    if (dcd) {

      HWND hwndMenu = (HWND) mp2;

      if (hwndMenu == ArcCnrMenu || hwndMenu == ArcMenu) {
	MarkAll(hwnd, TRUE, FALSE, TRUE);
	if (dcd->cnremphasized) {
	  WinSendMsg(hwnd,
		     CM_SETRECORDEMPHASIS,
		     MPVOID, MPFROM2SHORT(FALSE, CRA_SOURCE));
	  dcd->cnremphasized = FALSE;
	}
      }
    }
    break;

  case MM_PORTHOLEINIT:
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case 0:
      case 1:
	{
	  ULONG wmsg;

	  wmsg = SHORT1FROMMP(mp1) == 0 ? UM_FILESMENU : UM_VIEWSMENU;
	  PortholeInit((HWND) WinSendMsg(dcd->hwndClient,
					 wmsg, MPVOID, MPVOID), mp1, mp2);
	}
	break;
      }
    }
    break;

  case UM_INITMENU:
  case WM_INITMENU:
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case IDM_FILESMENU:
	if (dcd->info) {
	  WinEnableMenuItem((HWND) mp2,
			    IDM_DELETE, dcd->info->delete != NULL);
          WinEnableMenuItem((HWND) mp2, IDM_TEST, dcd->info->test != NULL);
          WinEnableMenuItem((HWND) mp2, IDM_EXTRACT,
                            dcd->info->extract != NULL && !strstr(dcd->info->ext, "TAR"));
          WinEnableMenuItem((HWND) mp2, IDM_ARCEXTRACT,
                            dcd->info->extract != NULL && !strstr(dcd->info->ext, "TAR"));
	  WinEnableMenuItem((HWND) mp2,
			    IDM_EXTRACTWDIRS, dcd->info->exwdirs != NULL);
	  WinEnableMenuItem((HWND) mp2,
			    IDM_ARCEXTRACTWDIRS, dcd->info->exwdirs != NULL);
	  WinEnableMenuItem((HWND) mp2,
			    IDM_ARCEXTRACTWDIRSEXIT,
			    dcd->info->exwdirs != NULL);
	}
	break;

      case IDM_VIEWSMENU:
        WinEnableMenuItem((HWND) mp2, IDM_TEST, dcd->info->test != NULL);
        WinEnableMenuItem((HWND) mp2, IDM_ARCEXTRACT,
                            dcd->info->extract != NULL && !strstr(dcd->info->ext, "TAR"));
	WinCheckMenuItem((HWND) mp2,
			 IDM_MINIICONS, (dcd->flWindowAttr & CV_MINI) != 0);
	WinEnableMenuItem((HWND) mp2,
			  IDM_RESELECT, (dcd->lastselection != NULL));
	break;

      case IDM_COMMANDSMENU:
	SetupCommandMenu((HWND) mp2, hwnd);
	break;

      case IDM_SORTSUBMENU:
	SetSortChecks((HWND) mp2, dcd->sortFlags);
	break;

      case IDM_WINDOWSMENU:
	/**
	 * add switchlist entries to end of pulldown menu
	 */
	SetupWinList((HWND)mp2,
		     hwndMain ? hwndMain : (HWND)0, dcd->hwndFrame);
	break;
      }
      dcd->hwndLastMenu = (HWND) mp2;
    }
    if (msg == WM_INITMENU)
      break;
    return 0;

  case UM_LOADFILE:
    if (dcd && mp2) {

      HWND hwnd;

      if ((INT)mp1 == 5 || (INT)mp1 == 13 || (INT)mp1 == 21)
	hwnd = StartViewer(HWND_DESKTOP, (INT)mp1,
			   (CHAR *)mp2, dcd->hwndFrame);
      else
	hwnd = StartMLEEditor(dcd->hwndParent,
			      (INT)mp1, (CHAR *)mp2, dcd->hwndFrame);
      free((CHAR *)mp2);
      return MRFROMLONG(hwnd);
    }
    return 0;

  case UM_COMMAND:
    if (mp1) {
      if (dcd) {
	if (!PostMsg(dcd->hwndObject, UM_COMMAND, mp1, mp2)) {
	  Runtime_Error(pszSrcFile, __LINE__, "post");
	  FreeListInfo((LISTINFO *) mp1);
	}
	else
	  return (MRESULT) TRUE;
      }
      else
	FreeListInfo((LISTINFO *) mp1);
    }
    return 0;

  case UM_OPENWINDOWFORME:
    if (dcd) {
      if (mp1 && !IsFile((CHAR *) mp1)) {
	OpenDirCnr((HWND) 0, hwndMain, dcd->hwndFrame, FALSE, (char *)mp1);
      }
      else if (mp1 && IsFile(mp1) == 1 &&
	       CheckDriveSpaceAvail(ArcTempRoot, ullDATFileSpaceNeeded, ullTmpSpaceNeeded) != 2) {
	StartArcCnr(HWND_DESKTOP,
		    dcd->hwndFrame, (CHAR *) mp1, 4, (ARC_TYPE *) mp2);
      }
    }
    return 0;

  case WM_COMMAND:
    DosError(FERR_DISABLEHARDERR);
    if (dcd) {
      if (SwitchCommand(dcd->hwndLastMenu, SHORT1FROMMP(mp1)))
	return 0;
      if (WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID))
	return 0;
      if (!IsArcThere(hwnd, dcd->arcname)) {
	PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	return 0;
      }
      switch (SHORT1FROMMP(mp1)) {
      case IDM_TREEVIEW:

	break;

      case IDM_CONTEXTMENU:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  PostMsg(hwnd,
		  WM_CONTROL,
		  MPFROM2SHORT(ARC_CNR, CN_CONTEXTMENU), MPFROMP(pci));
	}
	break;

      case IDM_NEXTWINDOW:
      case IDM_PREVWINDOW:
	{
	  HWND hwndActive;

	  hwndActive = WinQueryFocus(HWND_DESKTOP);
	  WinSetFocus(HWND_DESKTOP,
		      hwndActive == hwnd ?
			WinWindowFromID(dcd->hwndClient, ARC_EXTRACTDIR) :
			hwnd);
	}
	break;

      case IDM_FOLDERAFTEREXTRACT:
	fFolderAfterExtract = fFolderAfterExtract ? FALSE : TRUE;
	PrfWriteProfileData(fmprof, appname, "FolderAfterExtract",
			    &fFolderAfterExtract, sizeof(BOOL));
	break;

      case IDM_SHOWSELECT:
	QuickPopup(hwnd, dcd, CheckMenu(hwnd, &ArcCnrMenu, ARCCNR_POPUP),
		   IDM_SELECTSUBMENU);
	break;

      case IDM_SHOWSORT:
	QuickPopup(hwnd, dcd, CheckMenu(hwnd, &ArcCnrMenu, ARCCNR_POPUP),
		   IDM_SORTSUBMENU);
	break;

      case IDM_ARCHIVERSETTINGS:
	if (!ParentIsDesktop(dcd->hwndParent, dcd->hwndParent))
	  PostMsg(dcd->hwndParent, msg, MPFROMLONG(IDM_ARCHIVERSETTINGS), mp2);
	else {
	  WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    CfgDlgProc,
		    FM3ModHandle,
		    CFG_FRAME,
		    MPFROMLONG(IDM_ARCHIVERSETTINGS));
	}
	break;

      case IDM_RESCAN:
	dcd->ullTotalBytes = dcd->totalfiles =
	  dcd->selectedfiles = dcd->selectedbytes = 0;
	WinSetDlgItemText(dcd->hwndClient, DIR_TOTALS, "0");
	WinSetDlgItemText(dcd->hwndClient, DIR_SELECTED, "0 / 0k");
	dcd->totalfiles = FillArcCnr(dcd->hwndCnr,
				     dcd->arcname,
				     &dcd->info,
				     &dcd->ullTotalBytes, &dcd->stopflag);
	PostMsg(dcd->hwndCnr, UM_RESCAN, MPVOID, MPVOID);
	PostMsg(dcd->hwndCnr, UM_SETUP2, MPVOID, MPVOID);
	WinSendMsg(dcd->hwndCnr,
		   CM_INVALIDATERECORD,
		   MPVOID, MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
	break;

      case IDM_RESELECT:
	SelectList(hwnd, TRUE, FALSE, FALSE, NULL, NULL, dcd->lastselection);
	break;

      case IDM_HELP:
	if (hwndHelp)
	  WinSendMsg(hwndHelp,
		     HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_ARCLIST, 0),
		     MPFROMSHORT(HM_RESOURCEID));
	break;

      case IDM_WINDOWDLG:
	if (!ParentIsDesktop(dcd->hwndParent, dcd->hwndFrame))
	  PostMsg(dcd->hwndParent,
		  UM_COMMAND, MPFROM2SHORT(IDM_WINDOWDLG, 0), MPVOID);
	break;

      case IDM_SELECTALL:
      case IDM_SELECTALLFILES:
      case IDM_DESELECTALL:
      case IDM_DESELECTALLFILES:
      case IDM_SELECTMASK:
      case IDM_DESELECTMASK:
      case IDM_INVERT:
	{
	  PARCITEM pci;

	  pci = (PARCITEM) WinSendMsg(hwnd,
				      CM_QUERYRECORDEMPHASIS,
				      MPFROMLONG(CMA_FIRST),
				      MPFROMSHORT(CRA_CURSORED));
	  if ((INT) pci == -1)
	    pci = NULL;
	  if (SHORT1FROMMP(mp1) == IDM_HIDEALL) {
	    if (pci) {
	      if (!(pci->rc.flRecordAttr & CRA_SELECTED))
		pci->rc.flRecordAttr |= CRA_FILTERED;
	      WinSendMsg(hwnd,
			 CM_INVALIDATERECORD,
			 MPFROMP(&pci),
			 MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
	      break;
	    }
	  }
	  PostMsg(dcd->hwndObject, UM_SELECT, mp1, MPFROMP(pci));
	}
	break;

      case IDM_SORTSMARTNAME:
      case IDM_SORTNAME:
      case IDM_SORTFILENAME:
      case IDM_SORTSIZE:
      case IDM_SORTEASIZE:
      case IDM_SORTFIRST:
      case IDM_SORTLAST:
      case IDM_SORTLWDATE:
	dcd->sortFlags &= SORT_REVERSE;
	// intentional fallthru
      case IDM_SORTREVERSE:
	switch (SHORT1FROMMP(mp1)) {
	case IDM_SORTSMARTNAME:
	case IDM_SORTFILENAME:
	  dcd->sortFlags |= SORT_FILENAME;
	  break;
	case IDM_SORTSIZE:
	  dcd->sortFlags |= SORT_SIZE;
	  break;
	case IDM_SORTEASIZE:
	  dcd->sortFlags |= SORT_EASIZE;
	  break;
	case IDM_SORTFIRST:
	  dcd->sortFlags |= SORT_FIRSTEXTENSION;
	  break;
	case IDM_SORTLAST:
	  dcd->sortFlags |= SORT_LASTEXTENSION;
	  break;
	case IDM_SORTLWDATE:
	  dcd->sortFlags |= SORT_LWDATE;
	  break;
	case IDM_SORTREVERSE:
	  if (dcd->sortFlags & SORT_REVERSE)
	    dcd->sortFlags &= (~SORT_REVERSE);
	  else
	    dcd->sortFlags |= SORT_REVERSE;
	  break;
	}
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(ArcSort), MPFROMP(dcd));
	SaySort(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				DIR_SORT), dcd->sortFlags, TRUE);
	DefArcSortFlags = dcd->sortFlags;	// Remember for new windows
	break;

      case IDM_COLLECTOR:
	if (!Collector) {
	  HWND hwndC;
	  SWP swp;

	  if (ParentIsDesktop(hwnd, dcd->hwndParent) && !fAutoTile &&
	      (!fExternalCollector && !strcmp(realappname, FM3Str)))
	    GetNextWindowPos(dcd->hwndParent, &swp, NULL, NULL);
	  hwndC = StartCollector(fExternalCollector ||
				   strcmp(realappname, FM3Str) ?
				     HWND_DESKTOP : dcd->hwndParent, 4);
	  if (hwndC) {
	    if (!ParentIsDesktop(hwnd, dcd->hwndParent) && !fAutoTile &&
		(!fExternalCollector && !strcmp(realappname, FM3Str)))
	      WinSetWindowPos(hwndC,
			      HWND_TOP,
			      swp.x,
			      swp.y,
			      swp.cx,
			      swp.cy,
			      SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER);
	    else if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
		     fAutoTile && !strcmp(realappname, FM3Str)) {
	      TileChildren(dcd->hwndParent, TRUE);
	    }
	    WinSetWindowPos(hwndC, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE);
	    DosSleep(100); 
	  }
	}
	else
	  StartCollector(dcd->hwndParent, 4);
	break;

      case IDM_ARCEXTRACTEXIT:
      case IDM_ARCEXTRACT:
	if (dcd->directory && fFileNameCnrPath &&
	    stricmp(lastextractpath, dcd->directory)) {
          strcpy(lastextractpath, dcd->directory);
	  SetDir(dcd->hwndParent, hwnd, dcd->directory, 1);
	}
	if (dcd->info->extract)
	  runemf2(SEPARATE | WINDOWED | ASYNCHRONOUS |
		  (fArcStuffVisible ? 0 : BACKGROUND | MINIMIZED),
		  hwnd, pszSrcFile, __LINE__,
		  dcd->directory, NULL, "%s %s", dcd->info->extract,
		  BldQuotedFileName(szQuotedArcName, dcd->arcname));
	if (SHORT1FROMMP(mp1) == IDM_ARCEXTRACTEXIT)
	  PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	break;

      case IDM_ARCEXTRACTWDIRSEXIT:
      case IDM_ARCEXTRACTWDIRS:
	if (dcd->directory && fFileNameCnrPath &&
	    stricmp(lastextractpath, dcd->directory)) {
	  strcpy(lastextractpath, dcd->directory);
	  SetDir(dcd->hwndParent, hwnd, dcd->directory, 1);
	}
	if (dcd->info->exwdirs)
	  runemf2(SEPARATE | WINDOWED | ASYNCHRONOUS |
		  (fArcStuffVisible ? 0 : BACKGROUND | MINIMIZED),
		  hwnd, pszSrcFile, __LINE__,
		  dcd->directory, NULL, "%s %s",
		  dcd->info->exwdirs,
		  BldQuotedFileName(szQuotedArcName, dcd->arcname));
	if (SHORT1FROMMP(mp1) == IDM_ARCEXTRACTWDIRSEXIT)
	  PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	break;

      case IDM_RESORT:
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(ArcSort), MPFROMP(dcd));
	break;

      case IDM_FILTER:
	{
	  BOOL empty = FALSE;
	  PARCITEM pci;

	  if (!*dcd->mask.szMask) {
	    empty = TRUE;
	    pci = (PARCITEM) CurrentRecord(hwnd);
	    if (pci && (INT) pci != -1 && strchr(pci->pszFileName, '.'))
	      strcpy(dcd->mask.szMask, pci->pszFileName);
	  }

	  if (WinDlgBox(HWND_DESKTOP, hwnd, PickMaskDlgProc,
			FM3ModHandle, MSK_FRAME, MPFROMP(&dcd->mask))) {
	    WinSendMsg(hwnd, CM_FILTER, MPFROMP(ArcFilter), MPFROMP(dcd));
	    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	  }
	  else if (empty)
	    *dcd->mask.szMask = 0;
	  SayFilter(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				    DIR_FILTER), &dcd->mask, TRUE);
	}
	break;

      case IDM_SWITCH:
	if (mp2) {
	  if (stricmp(dcd->directory, (CHAR *) mp2)) {
	    DosRequestMutexSem(hmtxFM2Globals, SEM_INDEFINITE_WAIT);
	    strcpy(lastextractpath, (CHAR *) mp2);
	    MakeValidDir(lastextractpath);
	    DosReleaseMutexSem(hmtxFM2Globals);
	  }
	  strcpy(dcd->directory, (CHAR *) mp2);
	  MakeValidDir(dcd->directory);
	  WinSetWindowText(dcd->hwndExtract, dcd->directory);
	}
	break;

      case IDM_WALKDIR:
	{
	  CHAR newdir[CCHMAXPATH];

	  strcpy(newdir, dcd->directory);
	  if (!WinDlgBox(HWND_DESKTOP, dcd->hwndParent, WalkExtractDlgProc,
			 FM3ModHandle, WALK_FRAME,
			 MPFROMP(newdir)) || !*newdir)
	    break;
	  if (stricmp(newdir, dcd->directory)) {
	    strcpy(dcd->directory, newdir);
	    if (stricmp(lastextractpath, newdir))
	      strcpy(lastextractpath, newdir);
	    WinSetWindowText(dcd->hwndExtract, dcd->directory);
	  }
	}
	break;

      case IDM_TEST:
	if (dcd->info->test)
	  runemf2(SEPARATEKEEP | WINDOWED | MAXIMIZED,
		  hwnd, pszSrcFile, __LINE__, NULL, NULL,
		  "%s %s",dcd->info->test,
		  BldQuotedFileName(szQuotedArcName, dcd->arcname));
	break;

      case IDM_REFRESH:
      case IDM_DELETE:
      case IDM_PRINT:
      case IDM_VIEW:
      case IDM_VIEWTEXT:
      case IDM_VIEWBINARY:
      case IDM_VIEWARCHIVE:
      case IDM_EDIT:
      case IDM_EDITTEXT:
      case IDM_EDITBINARY:
      case IDM_EXTRACT:
      case IDM_EXTRACTWDIRS:
      case IDM_FIND:
      case IDM_EXEC:
      case IDM_VIRUSSCAN:
      case IDM_OPENDEFAULT:
      case IDM_OPENSETTINGS:
      case IDM_MCIPLAY:
	{
	  LISTINFO *li;
#	  ifdef FORTIFY
	  Fortify_EnterScope();
#	  endif
	  li = xmallocz(sizeof(LISTINFO), pszSrcFile, __LINE__);
	  if (li) {
	    li->type = SHORT1FROMMP(mp1);
	    li->hwnd = hwnd;
            li->list = BuildArcList(hwnd);
            if (li->type == IDM_DELETE)
              ignorereadonly = FALSE;
	    if (li->type == IDM_REFRESH) {

	      CHAR s[CCHMAXPATH], *p;
	      INT x, y;

	      for (x = 0; li->list && li->list[x]; x++) {
		BldFullPathName(s, dcd->workdir, li->list[x]);
		if (IsFile(s) != 1) {
		  free(li->list[x]);
		  li->list[x] = NULL;
		  for (y = x; li->list[y]; y++)
		    li->list[y] = li->list[y + 1];
		  li->list =
		    xrealloc(li->list, y * sizeof(CHAR *), pszSrcFile,
			     __LINE__);
		  x--;
		}
		else {
		  p = xstrdup(s, pszSrcFile, __LINE__);
		  if (p) {
		    free(li->list[x]);
		    li->list[x] = p;
		  }
		}
	      }				// for
	    }
	    strcpy(li->arcname, dcd->arcname);
	    li->info = dcd->info;
	    {
	      PARCITEM pai;

	      if (SHORT1FROMMP(mp1) != IDM_EXEC)
		pai = (PARCITEM) CurrentRecord(hwnd);
	      else
		pai = (PARCITEM) WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
					    MPFROMLONG(CMA_FIRST),
					    MPFROMSHORT(CRA_CURSORED));
	      if (pai && (INT) pai != -1)
		strcpy(li->runfile, pai->pszFileName);
	      else
		strcpy(li->runfile, li->list[0]);
	    }
	    switch (SHORT1FROMMP(mp1)) {
	    case IDM_VIEW:
	    case IDM_VIEWTEXT:
	    case IDM_VIEWBINARY:
	    case IDM_VIEWARCHIVE:
	    case IDM_EDIT:
	    case IDM_EDITTEXT:
	    case IDM_EDITBINARY:
	    case IDM_EXEC:
	    case IDM_PRINT:
	    case IDM_VIRUSSCAN:
	    case IDM_OPENDEFAULT:
	    case IDM_OPENSETTINGS:
	    case IDM_MCIPLAY:
	      strcpy(li->targetpath, dcd->workdir);
	      break;
	    default:
	      strcpy(li->targetpath, dcd->directory);
	      break;
	    }
	    if (li->list) {
	      if (!PostMsg(dcd->hwndObject, UM_ACTION, MPFROMP(li), MPVOID)) {
		Runtime_Error(pszSrcFile, __LINE__, "post");
		FreeListInfo(li);
	      }
	      else if (fUnHilite && SHORT1FROMMP(mp1) != IDM_EDIT)
		UnHilite(hwnd, TRUE, &dcd->lastselection, 0);
	    }
	    else {
	      free(li);
	    }
	  }
#	  ifdef FORTIFY
	  Fortify_LeaveScope();
#	  endif
	}
	break;
      }
    }
    return 0;

  case WM_CONTROL:
    DosError(FERR_DISABLEHARDERR);
    if (dcd) {
      switch (SHORT2FROMMP(mp1)) {
      case CN_BEGINEDIT:
	PostMsg(hwnd, CM_CLOSEEDIT, MPVOID, MPVOID);
	break;

      case CN_ENDEDIT:
	if (!((PCNREDITDATA) mp2)->pRecord) {

	  PFIELDINFO pfi = ((PCNREDITDATA) mp2)->pFieldInfo;
	  USHORT cmd = 0;

	  if (!pfi || pfi->offStruct == FIELDOFFSET(ARCITEM, pszDisplayName))
	    cmd = IDM_SORTSMARTNAME;
	  else if (pfi->offStruct == FIELDOFFSET(ARCITEM, cbFile))
	    cmd = IDM_SORTSIZE;
	  else if (pfi->offStruct == FIELDOFFSET(ARCITEM, cbComp))
	    cmd = IDM_SORTEASIZE;
	  else if (pfi->offStruct == FIELDOFFSET(ARCITEM, date))
	    cmd = IDM_SORTLWDATE;
	  else if (pfi->offStruct == FIELDOFFSET(ARCITEM, time))
	    cmd = IDM_SORTLWDATE;
	  if (cmd)
	    PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(cmd, 0), MPVOID);
	}
	break;

      case CN_DROPHELP:
	saymsg(MB_ENTER, hwnd,
	       GetPString(IDS_DROPHELPHDRTEXT),
	       GetPString(IDS_ARCCNRDROPHELPTEXT), dcd->arcname);
	return 0;

      case CN_DRAGLEAVE:
	if (mp2) {

	  PDRAGINFO pDInfo;

	  pDInfo = ((PCNRDRAGINFO) mp2)->pDragInfo;
	  DrgAccessDraginfo(pDInfo);	// Access DRAGINFO
	  DrgFreeDraginfo(pDInfo);	// Free DRAGINFO
	}
	return 0;

      case CN_DRAGAFTER:
      case CN_DRAGOVER:
	if (mp2) {

	  PDRAGITEM pDItem;		// Pointer to DRAGITEM
	  PDRAGINFO pDInfo;		// Pointer to DRAGINFO
	  PARCITEM pci;

	  pci = (PARCITEM) ((PCNRDRAGINFO) mp2)->pRecord;
	  if (SHORT1FROMMP(mp1) == CN_DRAGAFTER)
	    pci = NULL;
	  pDInfo = ((PCNRDRAGINFO) mp2)->pDragInfo;
	  DrgAccessDraginfo(pDInfo);	// Access DRAGINFO
	  if (*dcd->arcname) {
	    if ((driveflags[toupper(*dcd->arcname) - 'A'] &
		 DRIVE_NOTWRITEABLE) || !dcd->info || !dcd->info->create) {
	      DrgFreeDraginfo(pDInfo);
	      return MRFROM2SHORT(DOR_NEVERDROP, 0);
	    }
	  }
	  if (pci && (INT) pci != -1) {
	    DrgFreeDraginfo(pDInfo);
	    return MRFROM2SHORT(DOR_NODROP, 0);
	  }
	  pDItem = DrgQueryDragitemPtr(pDInfo,	        // Access DRAGITEM
				       0);	        // Index to DRAGITEM
	  if (DrgVerifyRMF(pDItem,	                // Check valid rendering
			   (CHAR *) DRM_OS2FILE,	// mechanisms and data
			   NULL) && !(pDItem->fsControl & DC_PREPARE)) {
	    DrgFreeDraginfo(pDInfo);	// Free DRAGINFO
	    return MRFROM2SHORT(DOR_DROP,	// Return okay to drop
				fCopyDefault ? DO_COPY : DO_MOVE);
	  }
	  DrgFreeDraginfo(pDInfo);	// Free DRAGINFO
	}
	return (MRFROM2SHORT(DOR_NEVERDROP, 0));	// Drop not valid

      case CN_INITDRAG:
	if (mp2) {

	  BOOL wasemphasized = FALSE;
	  PCNRDRAGINIT pcd = (PCNRDRAGINIT) mp2;
	  PARCITEM pci;

	  if (pcd) {
	    pci = (PARCITEM) pcd->pRecord;
	    if (pci && (INT) pci != -1) {
	      if (pci->rc.flRecordAttr & CRA_SELECTED)
		wasemphasized = TRUE;
	      if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
		  fSplitStatus && hwndStatus2)
		WinSetWindowText(hwndStatus2, (CHAR *) GetPString(IDS_DRAGARCMEMTEXT));
	      if (DoFileDrag(hwnd,
			     dcd->hwndObject,
			     mp2, dcd->arcname, NULL, TRUE)) {
		if ((fUnHilite && wasemphasized) || dcd->ulItemsToUnHilite)
		  UnHilite(hwnd, TRUE, &dcd->lastselection, dcd->ulItemsToUnHilite);
	      }
	      if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
		  fSplitStatus && hwndStatus2) {
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	      }
	    }
	    else {
	      if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
		  fSplitStatus && hwndStatus2)
		WinSetWindowText(hwndStatus2,
				 (CHAR *) GetPString(IDS_DRAGARCFILETEXT));
	      DragOne(hwnd, dcd->hwndObject, dcd->arcname, FALSE);
	      if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
		  fSplitStatus && hwndStatus2)
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	  }
	}
	return 0;

      case CN_DROP:
	if (mp2) {

	  LISTINFO *li;

	  li = DoFileDrop(hwnd, dcd->arcname, FALSE, mp1, mp2);
	  CheckPmDrgLimit(((PCNRDRAGINFO)mp2)->pDragInfo);
	  if (li) {
	    li->type = li->type == DO_MOVE ? IDM_ARCHIVEM : IDM_ARCHIVE;
	    strcpy(li->targetpath, dcd->arcname);
	    if (!li->list ||
		!li->list[0] ||
		!PostMsg(dcd->hwndObject, UM_ACTION, MPFROMP(li), MPVOID))
	      FreeListInfo(li);
	  }
	}
	return 0;

      case CN_CONTEXTMENU:
	{
	  PARCITEM pci = (PARCITEM) mp2;

	  if (pci && (INT) pci != -1) {
	    WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		       MPFROM2SHORT(TRUE, CRA_CURSORED));
	    MarkAll(hwnd, FALSE, FALSE, TRUE);
	    dcd->hwndLastMenu = CheckMenu(hwnd, &ArcMenu, ARC_POPUP);
	  }
	  else {
	    dcd->hwndLastMenu = CheckMenu(hwnd, &ArcCnrMenu, ARCCNR_POPUP);
	    if (dcd->hwndLastMenu && !dcd->cnremphasized) {
	      WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
			 MPFROM2SHORT(TRUE, CRA_SOURCE));
	      dcd->cnremphasized = TRUE;
	    }
	  }
	  if (dcd->hwndLastMenu) {
	    if (dcd->hwndLastMenu == ArcCnrMenu) {
	      if (dcd->flWindowAttr & CV_MINI)
		WinCheckMenuItem(dcd->hwndLastMenu, IDM_MINIICONS, TRUE);
	    }
	    WinCheckMenuItem(dcd->hwndLastMenu, IDM_FOLDERAFTEREXTRACT,
			     fFolderAfterExtract);
	    if (!PopupMenu(hwnd, hwnd, dcd->hwndLastMenu)) {
	      if (dcd->cnremphasized) {
		WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
			   MPFROM2SHORT(FALSE, CRA_SOURCE));
		dcd->cnremphasized = TRUE;
	      }
	      MarkAll(hwnd, TRUE, FALSE, TRUE);
	    }
	  }
	}
	break;

      case CN_EMPHASIS:
	if (mp2) {

	  PNOTIFYRECORDEMPHASIS pre = mp2;
	  PARCITEM pci;
	  CHAR s[CCHMAXPATHCOMP + 91], tf[81], tb[81];

	  pci = (PARCITEM)(pre ? pre->pRecord : NULL);
	  if (!pci) {
	    if (!ParentIsDesktop(hwnd, dcd->hwndParent)) {
	      if (hwndStatus2)
		WinSetWindowText(hwndStatus2, NullStr);
	      if (fMoreButtons)
		WinSetWindowText(hwndName, NullStr);
	    }
	    break;
	  }
	  if (pre->fEmphasisMask & CRA_SELECTED) {
	    if (pci->rc.flRecordAttr & CRA_SELECTED) {
	      dcd->selectedbytes += pci->cbFile;
	      dcd->selectedfiles++;
	    }
	    else if (dcd->selectedfiles) {
	      dcd->selectedbytes -= pci->cbFile;
	      dcd->selectedfiles--;
	    }
	    commafmt(tf, sizeof(tf), dcd->selectedfiles);
	    if (dcd->ullTotalBytes)
	      CommaFmtULL(tb, sizeof(tb), dcd->selectedbytes, ' ');
	    else
	      *tb = 0;
	    sprintf(s, "%s%s%s", tf, *tb ? " / " : NullStr, tb);
	    WinSetDlgItemText(dcd->hwndClient, DIR_SELECTED, s);
	  }
	  else if (WinQueryActiveWindow(dcd->hwndParent) ==
		   dcd->hwndFrame &&
		   !ParentIsDesktop(hwnd, dcd->hwndParent)) {
	    if (pre->fEmphasisMask & CRA_CURSORED) {
	      if (pci->rc.flRecordAttr & CRA_CURSORED) {
		if (fSplitStatus && hwndStatus2) {
		  if (dcd->ullTotalBytes)
		    CommaFmtULL(tb, sizeof(tb), pci->cbFile, ' ');
		  else
		    *tb = 0;
		  sprintf(s, "%s%s%s%s",
			  *tb ? " " : NullStr,
			  tb, *tb ? "  " : NullStr, pci->pszFileName);
		  WinSetWindowText(hwndStatus2, s);
		}
		if (fMoreButtons)
		  WinSetWindowText(hwndName, pci->pszFileName);
	      }
	    }
	  }
	}
	break;

      case CN_ENTER:
	if (mp2) {

	  PARCITEM pci = (PARCITEM) ((PNOTIFYRECORDENTER) mp2)->pRecord;

	  if (pci && (INT) pci != -1) {

	    CHAR *s;

	    if ((pci->rc.flRecordAttr & CRA_INUSE) ||
		(pci->flags & (ARCFLAGS_REALDIR | ARCFLAGS_PSEUDODIR)))
	      break;
	    s = xstrdup(pci->pszFileName, pszSrcFile, __LINE__);
	    if (s) {
	      if (!PostMsg(dcd->hwndObject, UM_ENTER, MPFROMP(s), MPVOID)) {
		Runtime_Error(pszSrcFile, __LINE__, "post");
		free(s);
	      }
	    }
	  }
	}
	break;
      }
    }
    return 0;

  case UM_FOLDUP:
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      DosExit(EXIT_PROCESS, 1);
    return 0;

  case UM_CLOSE:
    WinDestroyWindow(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				    QW_PARENT));
    return 0;

  case WM_SAVEAPPLICATION:
    if (dcd && ParentIsDesktop(hwnd, dcd->hwndParent)) {
      SWP swp;

      WinQueryWindowPos(dcd->hwndFrame, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE | SWP_MAXIMIZE)))
	PrfWriteProfileData(fmprof, appname, "AV2SizePos", &swp, sizeof(swp));
    }
    break;

  case WM_CLOSE:
    WinSendMsg(hwnd, WM_SAVEAPPLICATION, MPVOID, MPVOID);
    if (dcd)
      dcd->stopflag++;
    if (dcd && dcd->hwndObject) {
      if (!PostMsg(dcd->hwndObject, WM_CLOSE, MPVOID, MPVOID))
	WinSendMsg(dcd->hwndObject, WM_CLOSE, MPVOID, MPVOID);
    }
    // In case object window frees dcd
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!dcd ||
	(!dcd->dontclose &&
	 !dcd->amextracted && ParentIsDesktop(hwnd, dcd->hwndParent))) {
      if (!PostMsg(hwnd, UM_FOLDUP, MPVOID, MPVOID))
	WinSendMsg(hwnd, UM_FOLDUP, MPVOID, MPVOID);
    }
    return 0;

  case WM_DESTROY:
    if (ArcMenu)
      WinDestroyWindow(ArcMenu);
    if (ArcCnrMenu)
      WinDestroyWindow(ArcCnrMenu);
    ArcMenu = ArcCnrMenu = (HWND) 0;
    EmptyArcCnr(hwnd);
#     ifdef FORTIFY
    Fortify_LeaveScope();
#     endif
    break;
  }
  if (dcd && dcd->oldproc){
      return dcd->oldproc(hwnd, msg, mp1, mp2);
  }
  else
      return PFNWPCnr(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ArcCnrMenuProc(HWND hwnd, ULONG msg, MPARAM mp1,
				     MPARAM mp2)
{
  PFNWP oldMenuProc = WinQueryWindowPtr(hwnd, QWL_USER);
  static short  sLastMenuitem;

  switch (msg) {
    case WM_MOUSEMOVE: {
      if (fOtherHelp) {
	RECTL rectl;
	SHORT i, sCurrentMenuitem;
	SHORT MenuItems = 10;
	SHORT asMenuIDs[10] = {IDM_VIEW,
	      IDM_DELETE,
	      IDM_EXEC,
	      IDM_EXTRACT,
	      IDM_TEST,
	      IDM_VIRUSSCAN,
	      IDM_RESCAN,
	      IDM_WALKDIR,
	      IDM_FILTER,
	      0};
	PCSZ szHelpString = NULL;


	for (i=0; i<MenuItems; i++) {
	  sCurrentMenuitem = asMenuIDs[i];
	  oldMenuProc(hwnd,MM_QUERYITEMRECT,
		      MPFROM2SHORT(asMenuIDs[i], FALSE),
		      &rectl);

	if (MOUSEMSG(&msg)->x > rectl.xLeft &&
	    MOUSEMSG(&msg)->x < rectl.xRight &&
	    MOUSEMSG(&msg)->y > rectl.yBottom &&
	    MOUSEMSG(&msg)->y < rectl.yTop)
	   break;
	}                      // for


	 switch (sCurrentMenuitem) {
	 case 0:
	   break;
	 case IDM_VIEW:
	   szHelpString = GetPString(IDS_ARCCNRVIEWMENUHELP);
	   break;
	 case IDM_DELETE:
	   szHelpString = GetPString(IDS_ARCCNRDELETEMENUHELP);
	   break;
	 case IDM_EXEC:
	   szHelpString = GetPString(IDS_ARCCNREXECMENUHELP);
	   break;
	 case IDM_EXTRACT:
	   szHelpString = GetPString(IDS_ARCCNREXTRACTMENUHELP);
	   break;
	 case IDM_TEST:
	   szHelpString = GetPString(IDS_ARCCNRTESTMENUHELP);
	   break;
	 case IDM_VIRUSSCAN:
	   szHelpString = GetPString(IDS_ARCCNRVIRUSMENUHELP);
	   break;
	 case IDM_RESCAN:
	   szHelpString = GetPString(IDS_ARCCNRRESCANMENUHELP);
	   break;
	 case IDM_WALKDIR:
	   szHelpString = GetPString(IDS_ARCCNRWALKDIRMENUHELP);
	   break;
	 case IDM_FILTER:
	   szHelpString = GetPString(IDS_ARCCNRFILTERMENUHELP);
	   break;
	 default:
	   break;
	 }

	if (sLastMenuitem != sCurrentMenuitem && szHelpString) {
	  sLastMenuitem = sCurrentMenuitem;
	  MakeBubble(hwnd, TRUE, szHelpString);
	}
	else if (hwndBubble && !sCurrentMenuitem){
	  sLastMenuitem = sCurrentMenuitem;
	  WinDestroyWindow(hwndBubble);
	}
      }
    }
  }
    return oldMenuProc(hwnd, msg, mp1, mp2);
}

HWND StartArcCnr(HWND hwndParent, HWND hwndCaller, CHAR * arcname, INT flags,
		 ARC_TYPE * sinfo)
{
  /**
   * bitmapped flags:
   *  1 = am extracted from another archive
   *  4 = don't kill proc on close
   */

  HWND hwndFrame = (HWND) 0, hwndClient;
  ULONG FrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
    FCF_SIZEBORDER | FCF_MINMAX | FCF_ICON | FCF_NOBYTEALIGN | FCF_ACCELTABLE;
  USHORT id;
  DIRCNRDATA *dcd;
  ARC_TYPE *info = sinfo;
  CHAR title[MAXNAMEL + 1] = "AV/2 - ";
  CHAR fullname[CCHMAXPATH + 8], *p, temp;
  static USHORT idinc = 0;

  if (!idinc)
    idinc = (rand() % 256);
  if (ParentIsDesktop(hwndParent, hwndParent))
    FrameFlags |= (FCF_TASKLIST | FCF_MENU);
  if (arcname) {
    DosError(FERR_DISABLEHARDERR);
    if (DosQueryPathInfo(arcname,
			 FIL_QUERYFULLNAME, fullname, sizeof(fullname)))
      strcpy(fullname, arcname);
    ForwardslashToBackslash(fullname);
    if (!info)
      info = find_type(fullname, arcsighead);
    if (!info)
      return hwndFrame;
    if (strlen(title) + strlen(fullname) > MAXNAMEL) {
      p = title + strlen(title);
      strncpy(p, fullname, MAXNAMEL / 2 - 5);
      strcpy(p + MAXNAMEL / 2 - 5, "...");
      strcat(title, fullname + strlen(fullname) - (MAXNAMEL / 2 - 5));
    }
    else {
      strcat(title, fullname);
    }
    hwndFrame = WinCreateStdWindow(hwndParent,
				   WS_VISIBLE,
				   &FrameFlags,
				   (CHAR *) WC_ARCCONTAINER,
				   title,
				   WS_VISIBLE | fwsAnimate,
				   FM3ModHandle, ARC_FRAME, &hwndClient);
    if (hwndFrame && hwndClient) {
      id = ARC_FRAME + idinc++;
      if (idinc > 512)
	idinc = 0;
      WinSetWindowUShort(hwndFrame, QWS_ID, id);
#     ifdef FORTIFY
      Fortify_EnterScope();
#     endif
      dcd = xmallocz(sizeof(DIRCNRDATA), pszSrcFile, __LINE__);
      if (!dcd) {
	PostMsg(hwndClient, WM_CLOSE, MPVOID, MPVOID);
	hwndFrame = (HWND) 0;
      }
      else {
#	ifdef FORTIFY
	// Will be freed by WM_DESTROY
	Fortify_ChangeScope(dcd, -1);
#	endif
	dcd->size = sizeof(DIRCNRDATA);
	dcd->id = id;
	dcd->type = ARC_FRAME;
        if (pTmpDir && !IsValidDir(pTmpDir))
          DosCreateDir(pTmpDir, 0);
	MakeTempName(dcd->workdir, ArcTempRoot, 2);
	strcpy(dcd->arcname, fullname);
	if (*extractpath) {
	  if (!strcmp(extractpath, "*")) {
	    p = strrchr(fullname, '\\');
	    if (p) {
	      if (p < fullname + 3)
		p++;
	      temp = *p;
	      *p = 0;
	      strcpy(dcd->directory, fullname);
	      *p = temp;
	    }
	  }
	  else
	    strcpy(dcd->directory, extractpath);
        }
	if (!*dcd->directory && *lastextractpath) {
	  DosRequestMutexSem(hmtxFM2Globals, SEM_INDEFINITE_WAIT);
	  strcpy(dcd->directory, lastextractpath);
	  DosReleaseMutexSem(hmtxFM2Globals);
	}
	if (!*dcd->directory) {
	  if (!ParentIsDesktop(hwndParent, hwndParent))
	    TopWindowName(hwndParent, hwndCaller, dcd->directory);
	  if (!*dcd->directory) {
	    p = strrchr(fullname, '\\');
	    if (p) {
	      if (p < fullname + 3)
		p++;
	      *p = 0;
	      strcpy(dcd->directory, fullname);
	    }
	  }
	}
	if (!*dcd->directory ||
	    (IsFile(dcd->directory) == 1) ||
	    (isalpha(*dcd->directory) &&
	     (driveflags[toupper(*dcd->directory) - 'A'] &
	      DRIVE_NOTWRITEABLE)))
	  strcpy(dcd->directory, pFM2SaveDirectory);
	dcd->hwndParent = hwndParent ? hwndParent : HWND_DESKTOP;
	dcd->hwndFrame = hwndFrame;
	dcd->hwndClient = hwndClient;
	dcd->amextracted = (flags & 1) != 0;
	dcd->dontclose = (flags & 4) != 0;
	dcd->info = info;
	dcd->sortFlags = DefArcSortFlags;
	{
	  PFNWP oldproc;

	  oldproc = WinSubclassWindow(hwndFrame, (PFNWP) ArcFrameWndProc);
	  WinSetWindowPtr(hwndFrame, QWL_USER, (PVOID) oldproc);
	}
	dcd->hwndCnr = WinCreateWindow(hwndClient,
				       WC_CONTAINER,
				       NULL,
				       CCS_AUTOPOSITION | CCS_MINIICONS |
				       CCS_MINIRECORDCORE | ulCnrType |
				       WS_VISIBLE,
				       0,
				       0,
				       0,
				       0,
				       hwndClient,
				       HWND_TOP, (ULONG) ARC_CNR, NULL, NULL);
	if (!dcd->hwndCnr) {
	  Win_Error(hwndClient, hwndClient, pszSrcFile, __LINE__,
		    PCSZ_WINCREATEWINDOW);
	  PostMsg(hwndClient, WM_CLOSE, MPVOID, MPVOID);
	  free(dcd);
	  hwndFrame = (HWND) 0;
	}
	else {
	  WinSetWindowPtr(dcd->hwndCnr, QWL_USER, (PVOID) dcd);
	  dcd->oldproc = WinSubclassWindow(dcd->hwndCnr,
					   (PFNWP) ArcCnrWndProc);
	  {
	    USHORT ids[] = { DIR_TOTALS, DIR_SELECTED, DIR_VIEW, DIR_SORT,
	      DIR_FILTER, DIR_FOLDERICON, 0
	    };

	    CommonCreateTextChildren(dcd->hwndClient,
				     WC_ARCSTATUS, ids);
	  }
	  WinEnableWindow(WinWindowFromID(dcd->hwndClient, DIR_VIEW), FALSE);
	  dcd->hwndExtract = WinCreateWindow(dcd->hwndClient,
					     WC_ENTRYFIELD,
					     NULL,
					     ES_AUTOSCROLL,
					     0,
					     0,
					     0,
					     0,
					     dcd->hwndClient,
					     HWND_TOP,
					     ARC_EXTRACTDIR, NULL, NULL);
	  WinSendMsg(dcd->hwndExtract,
		     EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
	  WinSetWindowText(dcd->hwndExtract, dcd->directory);
	  if (!PostMsg(dcd->hwndCnr, UM_SETUP, MPVOID, MPVOID))
	    WinSendMsg(dcd->hwndCnr, UM_SETUP, MPVOID, MPVOID);
	  if (FrameFlags & FCF_MENU) {
	    PFNWP oldmenuproc;
	    HWND hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);

	    oldmenuproc = WinSubclassWindow(hwndMenu, (PFNWP) ArcCnrMenuProc);
	    WinSetWindowPtr(hwndMenu, QWL_USER, (PVOID) oldmenuproc);
	    if (!fToolbar) {

	      if (hwndMenu) {
		WinSendMsg(hwndMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_VIEW, FALSE), MPVOID);
		WinSendMsg(hwndMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_EXEC, FALSE), MPVOID);
		WinSendMsg(hwndMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_RESCAN, FALSE), MPVOID);
		WinSendMsg(hwndMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_DELETE, FALSE), MPVOID);
		WinSendMsg(hwndMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_EXTRACT, FALSE), MPVOID);
		WinSendMsg(hwndMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_TEST, FALSE), MPVOID);
		WinSendMsg(hwndMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_VIRUSSCAN, FALSE), MPVOID);
		WinSendMsg(hwndMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_WALKDIR, FALSE), MPVOID);
		WinSendMsg(hwndMenu, MM_DELETEITEM,
			   MPFROM2SHORT(IDM_FILTER, FALSE), MPVOID);
	      }
	    }
	  }
	  if (FrameFlags & FCF_TASKLIST) {

	    SWP swp, swpD;
	    ULONG size = sizeof(swp);
	    LONG cxScreen, cyScreen;

	    WinQueryTaskSizePos(WinQueryAnchorBlock(hwndFrame), 0, &swp);
	    if (PrfQueryProfileData(fmprof, appname, "AV2SizePos", &swpD, &size)) {
	      cxScreen = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
	      cyScreen = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
	      if (swp.x + swpD.cx > cxScreen)
		swp.x = cxScreen - swpD.cx;
	      if (swp.y + swpD.cy > cyScreen)
		swp.y = cyScreen - swpD.cy;
	      swp.cx = swpD.cx;
	      swp.cy = swpD.cy;
	    }
	    WinSetWindowPos(hwndFrame,
			    HWND_TOP,
			    swp.x,
			    swp.y,
			    swp.cx,
			    swp.cy,
			    SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ZORDER |
			    SWP_ACTIVATE);
	  }
	}
      }
#     ifdef FORTIFY
      Fortify_LeaveScope();
#     endif
    }
  }
  return hwndFrame;
}

#pragma alloc_text(ARCCNRS,ArcCnrWndProc,ArcObjWndProc,ArcClientWndProc,BldQuotedFullPathName)
#pragma alloc_text(ARCCNRS,ArcTextProc,FillArcCnr,ArcFilter,BldQuotedFileName)
#pragma alloc_text(ARCCNRS,ArcSort,ArcFrameWndProc,IsArcThere,ArcErrProc)
#pragma alloc_text(STARTUP,StartArcCnr)
