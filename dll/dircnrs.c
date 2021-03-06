
/***********************************************************************

  $Id$

  Directory containers

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2015 Steven H. Levine

  16 Oct 02 SHL Handle large partitions
  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  24 May 05 SHL Rework Win_Error usage
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  26 May 05 SHL More large file formatting updates
  05 Jun 05 SHL Use QWL_USER
  10 Nov 05 SHL Comments
  13 Jul 06 SHL Use Runtime_Error
  26 Jul 06 SHL Use chop_at_crnl
  15 Aug 06 SHL Rework warning message text
  07 Jan 07 GKY Move error strings etc. to string file
  30 Mar 07 GKY Remove GetPString for window class names
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits
  06 Apr 07 GKY Add some error checking in drag/drop
  19 Apr 07 SHL Use FreeDragInfoData.  Add more drag/drop error checking.
  12 May 07 SHL Use dcd->ulItemsToUnHilite; sync with UnHilite arg mods
  10 Jun 07 GKY Add CheckPmDrgLimit including IsFm2Window as part of work around PM drag limit
  02 Aug 07 SHL Sync with CNRITEM mods
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  26 Aug 07 GKY DosSleep(1) in loops changed to (0)
  22 Nov 07 GKY Use CopyPresParams to fix presparam inconsistencies in menus
  10 Jan 08 SHL Sync with CfgDlgProc mods
  19 Jan 08 JBS Ticket 150: fix/improve save and restore of dir cnr state at FM/2 close/reopen
  15 Feb 08 SHL Sync with settings menu rework
  22 Feb 08 JBS Ticket 230: Fix/improve various code related to state or presparam values in the INI file.
  11 May 08 GKY Avoid using stale dcd after free
  11 May 08 SHL Add stale dcd sanity checks
  21 Jun 08 GKY Fix columns to honor preferences on new container open.
  22 Jun 08 GKY Included free_... functions for fortify checking
  06 Jul 08 GKY Update delete/undelete to include move to and open XWP trashcan
  11 Jul 08 JBS Ticket 230: Simplified code and eliminated some local variables by incorporating
		all the details view settings (both the global variables and those in the
		DIRCNRDATA struct) into a new struct: DETAILS_SETTINGS.
  20 Jul 08 GKY Add save/append filename to clipboard.
		Change menu wording to make these easier to find
  02 Aug 08 GKY Always pass temp variable point to treecnr UM_SHOWME to avoid
		freeing dcd->directory early
  25 Aug 08 GKY Check TMP directory space warn if lee than 5 MiB prevent archiver from opening if
		less than 10 KiB (It hangs and can't be closed)
  29 Nov 08 GKY Remove or replace with a mutex semaphore DosEnterCriSec where appropriate.
  10 Dec 08 SHL Integrate exception handler support
  26 Dec 08 GKY Fixed DROPHELP to check for copy as default is action is DO_DEFAULT
  01 Jan 09 GKY Add Seek and Scan to drives & directory context menus pass drive/dir as search root
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  07 Feb 09 GKY Move repeated strings to PCSZs.
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  12 Mar 09 SHL Use common SearchContainer
  14 Mar 09 GKY Prevent execution of UM_SHOWME while drive scan is occuring
  29 Mar 09 SHL Keep more keys away from PM if extended search in progress
  29 Mar 09 SHL Increase extended search timeout to 3 seconds
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.
  22 Jul 09 GKY Code changes to use semaphores to serialize drive scanning
  22 Jul 09 SHL Cleanup of SETFOCUS code
  14 Sep 09 SHL Drop experimental code
  15 Sep 09 SHL Show rescan progress while filling container
  13 Dec 09 GKY Fixed separate paramenters. Please note that appname should be used in
                profile calls for user settings that work and are setable in more than one
                miniapp; FM3Str should be used for setting only relavent to FM/2 or that
                aren't user settable; realappname should be used for setting applicable to
                one or more miniapp but not to FM/2
  17 Jan 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10).
                Mostly cast CHAR CONSTANT * as CHAR *.
  28 May 10 GKY Yet another attempt to prevent duplicate directory names in the tree by
                suppressing SHOW_ME during initial drive scan.
  20 Nov 10 GKY Rework scanning code to remove redundant scans, prevent double directory
                entries in the tree container, fix related semaphore performance using
                combination of event and mutex semaphores
  04 Aug 12 GKY Changes to use Unlock to unlock files if Unlock.exe is in path both from menu/toolbar and as part of
                copy, move and delete operations
  22 Feb 14 GKY Fix warn readonly yes don't ask to work when recursing directories.
  02 Mar 14 GKY Speed up intial drive scans Ticket 528
  26 Jun 14 SHL Rework DirObjWndProc UM_RESCAN to avoid hanging FM/2 Lite when tree hidden
  30 Aug 14 GKY Add semaphore hmtxFiltering to prevent freeing dcd while filtering. Prevents
                a trap when FM2 is shutdown while directory containers are still populating
  02 May 15 GKY Changes to allow a JAVA executable object to be created using "Real object"
                menu item on a jar file.
  02 Aug 15 GKY Remove unneed SubbyScan code and improve suppression of blank lines and
                duplicate subdirectory name caused by running Stubby in worker threads.
  07 Aug 15 SHL Rework to use AddFleshWorkRequest rather than direct calls to Stubby/Flesh/Unflesh
  26 Sep 15 GKY Add code so case UM_TOPDIR actually exists put it in the object window so it
                can call WaitFleshWorkListEmpty while avoiding thread 1
  27 Sep 15 GKY DosSleep times in WaitFleshWorkListEmpty set by caller

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSERRORS
#define INCL_LONGLONG
#define INCL_WINWORKPLACE

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "mainwnd2.h"			// Data declaration(s)
#include "grep.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "treecnr.h"			// Data declaration(s)
#include "dircnrs.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "arccnrs.h"			// StartArcCnr
#include "comp.h"			// COMPARE
#include "filldir.h"			// EmptyCnr...
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "notebook.h"			// CfgDlgProc
#include "command.h"			// RunCommand
#include "worker.h"			// Action, MassAction
#include "misc.h"			// GetTidForThread, AdjustCnrColsForFSType, AdjustCnrColsForPref
					// AdjustDetailsSwitches, CnrDirectEdit, OpenEdit, QuickPopup
					// SayFilter, SaySort, SayView, SetCnrCols, SetDetailsSwitches
					// SetSortChecks, SetViewMenu, SwitchCommand, CheckMenu
					// CurrentRecord, DrawTargetEmphasis, IsFm2Window
#include "chklist.h"			// CenterOverWindow, DropListProc
#include "common.h"			// CommonCnrProc, CommonCreateTextChildren, CommonFrameWndProc
					// CommonTextPaint, CommonTextButton, CommonTextProc
#include "mainwnd.h"			// CountDirCnrs, GetNextWindowPos, MakeBubble, TopWindow
#include "select.h"			// DeselectAll, HideAll, InvertAll, SelectAll, SelectList
					// SpecialSelect2
#include "dirsize.h"			// DirSizeProc
#include "flesh.h"			// AddFleshWorkRequest
#include "valid.h"			// IsValidDir
#include "objwin.h"			// MakeObjWin
#include "notify.h"			// NotifyError
#include "objcnr.h"			// ObjCnrDlgProc
#include "draglist.h"			// DoFileDrag, FreeDragInfoData, PickUp
#include "saveclip.h"			// SaveListDlgProc
#include "findrec.h"			// ShowCnrRecord
#include "sortcnr.h"			// SortDirCnr
#include "seeall.h"			// StartSeeAll
#include "update.h"			// UpdateCnrList, UpdateCnrRecord
#include "walkem.h"			// add_udir
#include "strips.h"			// chop_at_crnl
#include "droplist.h"			// AcceptOneDrop, CheckPmDrgLimit, DropHelp, GetOneDrop
#include "presparm.h"			// CopyPresParams
#include "defview.h"			// DefaultViewKeys
#include "systemf.h"			// ExecOnList
#include "filter.h"			// Filter
#include "findrec.h"			// FindCnrRecord
#include "input.h"			// InputDlgProc
#include "shadow.h"			// OpenObject
#include "mkdir.h"			// PMMkDir
#include "collect.h"			// StartCollector
#include "viewer.h"			// StartMLEEditor
#include "newview.h"			// StartViewer
#include "undel.h"			// UndeleteDlgProc
#include "i18nutil.h"			// commafmt
#include "getnames.h"			// insert_filename
#include "wrappers.h"			// xfree
#include "fortify.h"
#include "excputil.h"			// 06 May 08 SHL added
#include "pathutil.h"			// AddBackslashToPath
#include "copyf.h"			// ignorereadonly
#if 0
#define  __PMPRINTF__
#include "PMPRINTF.H"
#endif

// Data definitions
#pragma data_seg(GLOBAL1)
HWND DirCnrMenu;
HWND hwndAttr;
HWND hwndDate;

#pragma data_seg(GLOBAL2)
INT sortFlags;

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

MRESULT EXPENTRY DirFrameWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  return CommonFrameWndProc(DIR_CNR, hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY DirTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static BOOL emphasized = FALSE;
  static HWND hwndButtonPopup = (HWND) 0;
  static USHORT lastid = 0;
  static ULONG timestamp = ULONG_MAX;

  switch (msg) {
  case WM_CREATE:
    return CommonTextProc(hwnd, msg, mp1, mp2);

  case WM_COMMAND:
    {
      DIRCNRDATA *dcd;
      MRESULT mr;

      mr = WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,
						     QW_PARENT),
				      DIR_CNR), msg, mp1, mp2);
      if (hwndButtonPopup &&
	  SHORT1FROMMP(mp1) > IDM_DETAILSTITLES &&
	  SHORT1FROMMP(mp1) < IDM_DETAILSSETUP) {
	dcd = WinQueryWindowPtr(WinWindowFromID(WinQueryWindow(hwnd,
							       QW_PARENT),
						DIR_CNR), QWL_USER);
	if (dcd)
	  SetDetailsSwitches(hwndButtonPopup, &dcd->ds);
      }
      return mr;
    }

  case UM_CONTEXTMENU:
  case WM_CONTEXTMENU:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id) {
      case DIR_FOLDERICON:
	if (fNoFoldMenu) {
	  PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				  DIR_CNR),
		  WM_COMMAND, MPFROM2SHORT(IDM_PREVIOUS, 0), mp2);
	  break;
	}
	// else intentional fallthru
      case DIR_SELECTED:
      case DIR_VIEW:
      case DIR_SORT:
	{
	  POINTL ptl = { 0, 0 };
	  SWP swp;
	  DIRCNRDATA *dcd;

	  if (hwndButtonPopup)
	    WinDestroyWindow(hwndButtonPopup);
	  if (id == DIR_SELECTED && msg == WM_CONTEXTMENU)
	    id = DIR_MAX;
	  if (id == lastid) {

	    ULONG check;

	    DosQuerySysInfo(QSV_MS_COUNT,
			    QSV_MS_COUNT, &check, sizeof(check));
	    if (check < timestamp + 500) {
	      lastid = 0;
	      goto MenuAbort;
	    }
	  }
	  hwndButtonPopup = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, id);
	  CopyPresParams(hwndButtonPopup, hwnd);
	  if (hwndButtonPopup) {
	    WinSetWindowUShort(hwndButtonPopup, QWS_ID, id);
	    dcd = WinQueryWindowPtr(WinWindowFromID(WinQueryWindow(hwnd,
								   QW_PARENT),
						    DIR_CNR), QWL_USER);
	    if (id == DIR_SORT) {	// don't have sort pathname in dirs
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SORTNAME, FALSE), MPVOID);
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SORTNONE, FALSE), MPVOID);
	      if (dcd)
		SetSortChecks(hwndButtonPopup, dcd->sortFlags);
	    }
	    else if (id == DIR_VIEW) {
	      if (dcd) {
		SetViewMenu(hwndButtonPopup, dcd->flWindowAttr);
		SetDetailsSwitches(hwndButtonPopup, &dcd->ds);
	      }
	    }
	    else if (id == DIR_MAX) {

	      int x;
	      BOOL enable;
	      USHORT ids[] = { IDM_SELECTBOTH,
		IDM_SELECTMORE,
		IDM_SELECTONE,
		IDM_SELECTNEWER,
		IDM_SELECTOLDER,
		IDM_SELECTBIGGER,
		IDM_SELECTSMALLER,
		IDM_DESELECTBOTH,
		IDM_DESELECTMORE,
		IDM_DESELECTONE,
		IDM_DESELECTNEWER,
		IDM_DESELECTOLDER,
		IDM_DESELECTBIGGER,
		IDM_DESELECTSMALLER,
		0
	      };

	      enable = (CountDirCnrs(dcd->hwndParent) > 1);
	      for (x = 0; ids[x]; x++)
		WinEnableMenuItem(hwndButtonPopup, ids[x], enable);
	    }
	    else if (id == DIR_SELECTED) {
	      if (dcd)
		WinEnableMenuItem(hwndButtonPopup,
				  IDM_RESELECT, (dcd->lastselection != NULL));
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
	PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT), DIR_CNR),
		WM_CONTROL, MPFROM2SHORT(DIR_CNR, CN_CONTEXTMENU), MPVOID);
	break;
      }
    }					// case WM_CONTENT_MENU

  MenuAbort:

    if (msg == UM_CONTEXTMENU)
      return 0;
    break;

  case WM_MENUEND:
    if (hwndButtonPopup == (HWND) mp2) {
      lastid = WinQueryWindowUShort((HWND) mp2, QWS_ID);
      WinDestroyWindow(hwndButtonPopup);
      hwndButtonPopup = (HWND) 0;
      DosQuerySysInfo(QSV_MS_COUNT,
		      QSV_MS_COUNT, &timestamp, sizeof(timestamp));
      switch (lastid) {
      case DIR_VIEW:
      case DIR_SORT:
      case DIR_FOLDERICON:
      case DIR_SELECTED:
      case DIR_MAX:
	PaintRecessedWindow(hwnd, (HPS) 0, TRUE, FALSE);
	break;
      }
    }
    break;

  case WM_BUTTON3DOWN:
  case WM_BUTTON1DOWN:
  case WM_BUTTON3UP:
  case WM_BUTTON1UP:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id) {
      case DIR_FILTER:
      case DIR_VIEW:
      case DIR_SORT:
      case DIR_SELECTED:
      case DIR_FOLDERICON:
      case DIR_MAX:
	return CommonTextButton(hwnd, msg, mp1, mp2);
      }
    }
    break;

  case WM_BUTTON1DBLCLK:
    {
      NOTIFYRECORDENTER nr;

      if (WinQueryWindowUShort(hwnd, QWS_ID) != DIR_FOLDERICON) {
	memset(&nr, 0, sizeof(NOTIFYRECORDENTER));
	nr.hwndCnr = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT), DIR_CNR);
	WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
		   WM_CONTROL, MPFROM2SHORT(DIR_CNR, CN_ENTER), MPFROMP(&nr));
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
	    s = GetPString(IDS_DIRCNRTOTALHELP);
	    break;
	  case DIR_SELECTED:
	    s = GetPString(IDS_DIRCNRSELECTEDHELP);
	    break;
	  case DIR_VIEW:
	    s = GetPString(IDS_DIRCNRVIEWHELP);
	    break;
	  case DIR_SORT:
	    s = GetPString(IDS_DIRCNRSORTHELP);
	    break;
	  case DIR_FILTER:
	    s = GetPString(IDS_DIRCNRFILTERHELP);
	    break;
	  case DIR_MAX:
	    s = GetPString(IDS_DIRCNRMAXHELP);
	    break;
	  case DIR_FOLDERICON:
	    s = GetPString(IDS_DIRCNRFOLDERHELP);
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
      case DIR_MAX:
      case DIR_FOLDERICON:
      case DIR_FILTER:
      case DIR_SORT:
      case DIR_VIEW:
      case DIR_SELECTED:
	return CommonTextButton(hwnd, msg, mp1, mp2);
      }
    }
    break;

  case WM_CHORD:
  case WM_BUTTON3CLICK:
  case WM_BUTTON1CLICK:
  case UM_CLICKED:
  case UM_CLICKED3:
    {
      USHORT id, cmd = 0;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      if (msg == UM_CLICKED || msg == UM_CLICKED3) {
	switch (id) {
	case DIR_MAX:
	  cmd = IDM_MAXIMIZE;
	  break;
	case DIR_VIEW:
	case DIR_SELECTED:
	case DIR_SORT:
	  PostMsg(hwnd, UM_CONTEXTMENU, MPVOID, MPVOID);
	  break;
	case DIR_FILTER:
	  cmd = IDM_FILTER;
	  break;
	default:
	  break;
	}
      }
      else if (id == DIR_FOLDERICON) {
	if ((msg == WM_BUTTON1CLICK && (SHORT2FROMMP(mp2) & KC_CTRL)))
	  cmd = IDM_PREVIOUS;
	else if (msg == WM_BUTTON3CLICK || msg == WM_CHORD)
	  cmd = IDM_RESCAN;
	else if (msg == WM_BUTTON1CLICK && (SHORT2FROMMP(mp2) & KC_SHIFT))
	  cmd = IDM_WALKDIR;
	else if (msg == WM_BUTTON1CLICK && (SHORT2FROMMP(mp2) & KC_ALT))
	  cmd = IDM_WINDOWDLG;
	else
	  cmd = IDM_PARENT;
      }
      if (cmd)
	PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				DIR_CNR),
		WM_COMMAND, MPFROM2SHORT(cmd, 0), MPVOID);
    }
    if (msg == UM_CLICKED || msg == UM_CLICKED3)
      return 0;
    break;

  case DM_DROP:
  case DM_DRAGOVER:
  case DM_DRAGLEAVE:
  case DM_DROPHELP:
  case WM_BEGINDRAG:
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
	return (MRFROM2SHORT(DOR_NODROP, 0));	// Drop not valid
      case DM_DROPHELP:
	DropHelp(mp1, mp2, hwnd, GetPString(IDS_DIRCNRFOLDERDROPHELP));
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
				       DIR_CNR),
		       WM_COMMAND, MPFROM2SHORT(IDM_SWITCH, 0),
		       MPFROMP(szFrom));
	}
	return 0;
      default:
	return PFNWPStatic(hwnd, msg, mp1, mp2);
      }
    case DIR_MAX:
      if (msg == WM_BEGINDRAG)
	return PFNWPStatic(hwnd, msg, mp1, mp2);
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
	memset(&cnd, 0, sizeof(cnd));
	cnd.pDragInfo = (PDRAGINFO) mp1;
	cnd.pRecord = NULL;
	return WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
			  WM_CONTROL,
			  MPFROM2SHORT(DIR_CNR, dcmd), MPFROMP(&cnd));
      }
    }
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY DirClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2)
{
  switch (msg) {
  case UM_CONTAINERDIR:
    if (mp1) {

      DIRCNRDATA *dcd;

      *(CHAR *)mp1 = 0;
      dcd = WinQueryWindowPtr(WinWindowFromID(hwnd, DIR_CNR), QWL_USER);
      if (dcd)
	strcpy((CHAR *)mp1, dcd->directory);
      return MRFROMLONG(TRUE);
    }
    return 0;

  case UM_CONTAINERHWND:
    return MRFROMLONG(WinWindowFromID(hwnd, DIR_CNR));

  case UM_VIEWSMENU:
    return MRFROMLONG(CheckMenu(hwnd, &DirCnrMenu, DIRCNR_POPUP));

  case UM_DRIVECMD:
  case WM_INITMENU:
  case UM_FILTER:
  case UM_INITMENU:
  case MM_PORTHOLEINIT:
  case UM_COMMAND:
  case UM_FILESMENU:
  case UM_UPDATERECORD:
  case UM_UPDATERECORDLIST:
    return WinSendMsg(WinWindowFromID(hwnd, DIR_CNR), msg, mp1, mp2);

  case WM_PSETFOCUS:
  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, DIR_CNR));
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
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_CNR), HWND_TOP,
		      0, 0, cx, cy - 24, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      if (WinWindowFromID(hwnd, DIR_MAX) != (HWND) 0) {
	WinSetWindowPos(WinWindowFromID(hwnd, DIR_MAX), HWND_TOP,
			cx - 22,
			cy - 22, 20, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
	cx -= 24;
      }
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

  case WM_COMMAND:
  case WM_CONTROL:
  case WM_CLOSE:
    return WinSendMsg(WinWindowFromID(hwnd, DIR_CNR), msg, mp1, mp2);
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY DirObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *dcd;
  CHAR tf[64];
  CHAR tb[64];
  CHAR s[CCHMAXPATH * 2];

  switch (msg) {
  case WM_CREATE:
    DbgMsg(pszSrcFile, __LINE__, "WM_CREATE mp1 %p mp2 %p", mp1, mp2);	// 18 Jul 08 SHL fixme
    break;

  case DM_PRINTOBJECT:
    return MRFROMLONG(DRR_TARGET);

  case DM_DISCARDOBJECT:
    dcd = INSTDATA(hwnd);
    if (fFM2Deletes && dcd) {
      LISTINFO *li;
      CNRDRAGINFO cni;
      cni.pRecord = NULL;
      cni.pDragInfo = (PDRAGINFO) mp1;
      li =
	DoFileDrop(dcd->hwndCnr, dcd->directory, FALSE, MPVOID,
		   MPFROMP(&cni));
      CheckPmDrgLimit(cni.pDragInfo);
      if (li) {
	li->type = (fDefaultDeletePerm) ? IDM_PERMDELETE : IDM_DELETE;
	if (!PostMsg(hwnd, UM_MASSACTION, MPFROMP(li), MPVOID))
	  FreeListInfo(li);
	else
	  return MRFROMLONG(DRR_SOURCE);
      }
    }
    return MRFROMLONG(DRR_TARGET);

  case UM_UPDATERECORDLIST:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd && mp1) {

      INT numentries = 0;
      CHAR **list = (CHAR **) mp1;

      while (list[numentries])
	numentries++;
      if (numentries)
	UpdateCnrList(dcd->hwndCnr, list, numentries, TRUE, dcd);
    }
    return 0;

  case UM_SETUP:
#   ifdef FORTIFY
    // DbgMsg(pszSrcFile, __LINE__, "UM_SETUP hwnd %p TID %u", hwnd, GetTidForThread());	// 18 Jul 08 SHL fixme
    Fortify_EnterScope();
#   endif
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
#     ifdef FORTIFY
      Fortify_BecomeOwner(dcd);		// We free dcd
      if (GetTidForThread())
	Fortify_ChangeScope(dcd, -1);
#     endif
      // set unique id
      WinSetWindowUShort(hwnd, QWS_ID, DIROBJ_FRAME + (DIR_FRAME - dcd->id));
      dcd->hwndObject = hwnd;
      if (ParentIsDesktop(hwnd, dcd->hwndParent))
	DosSleep(100);
    }
    else
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
#   ifdef FORTIFY
    // TID 1 will free data
    if (GetTidForThread())
      Fortify_LeaveScope();
#   endif
    return 0;

  case UM_RESCAN2:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd && dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent)) {
      FSALLOCATE fsa;
      CHAR szFree[64];
      DosError(FERR_DISABLEHARDERR);
      if (!DosQueryFSInfo(toupper(*dcd->directory) - '@',
			  FSIL_ALLOC, &fsa, sizeof(FSALLOCATE))) {
	CommaFmtULL(tb, sizeof(tb),
		    (ULONGLONG) fsa.cUnitAvail * (fsa.cSectorUnit *
						  fsa.cbSector), 'K');
	sprintf(szFree, "  {%s %s}", tb, GetPString(IDS_FREETEXT));
      }
      else
        *szFree = 0;
      DosRequestMutexSem(hmtxFiltering, SEM_INDEFINITE_WAIT);
      commafmt(tf, sizeof(tf), dcd->totalfiles);
      CommaFmtULL(tb, sizeof(tb), dcd->ullTotalBytes, ' ');
      DosReleaseMutexSem(hmtxFiltering);
      if (!fMoreButtons) {
	sprintf(s, " [%s / %s]%s%s%s%s %s",
		tf, tb, szFree,
		(*dcd->mask.szMask || dcd->mask.antiattr ||
		 dcd->mask.attrFile != ALLATTRS) ? "  (" : NullStr,
		(*dcd->mask.szMask) ? dcd->mask.szMask :
		(dcd->mask.antiattr ||
		 dcd->mask.attrFile != ALLATTRS) ?
		GetPString(IDS_ALLTEXT) : NullStr,
		(*dcd->mask.szMask || dcd->mask.antiattr ||
		 dcd->mask.attrFile != ALLATTRS) ? ")" : NullStr,
		dcd->directory);
      }
      else {
	sprintf(s, " [%s / %s]%s %s", tf, tb, szFree, dcd->directory);
      }
      if (dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent))
	WinSetWindowText(hwndStatus, s);
    }
    return 0;

  case UM_FLESH:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {

      PCNRITEM pci, pciC;

      pci = WinSendMsg(dcd->hwndCnr,
		       CM_QUERYRECORD,
		       MPVOID, MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
      while (pci && (INT) pci != -1) {
        if (!pci->pszFileName || !strcmp(pci->pszFileName, NullStr)) {
          Runtime_Error(pszSrcFile, __LINE__, "pci->pszFileName NULL for %p", pci);
          return 0;
        }
	if (pci->attrFile & FILE_DIRECTORY) {
	  pciC = WinSendMsg(dcd->hwndCnr,
			    CM_QUERYRECORD,
			    MPFROMP(pci),
			    MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
          if (!pciC) {
            AddFleshWorkRequest(dcd->hwndCnr, pci, eStubby);
          }
	}
	pci = WinSendMsg(dcd->hwndCnr,
			 CM_QUERYRECORD,
			 MPFROMP(pci), MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
      }
      dcd->firsttree = TRUE;
    }
    return 0;

  case UM_TOPDIR:
    if (mp1) {
      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (dcd) {
        PCNRITEM pci = (PCNRITEM) mp1;
        WaitFleshWorkListEmpty(pci->pszFileName, 240L);
        ShowCnrRecord(dcd->hwndCnr, (PMINIRECORDCORE) pci);
      }
    }
    return 0;

  case UM_RESCAN:
    // populate container
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
      DosRequestMutexSem(hmtxFM2Globals, SEM_INDEFINITE_WAIT);
      if (dcd->stopflag)
	dcd->stopflag--;
      if (dcd->stopflag) {
	DosReleaseMutexSem(hmtxFM2Globals);
	return 0;
      }
      DosReleaseMutexSem(hmtxFM2Globals);
      if (mp1) {
	strcpy(dcd->previous, dcd->directory);
	strcpy(dcd->directory, (CHAR *)mp1);
      }
      MakeValidDir(dcd->directory);
      {
	sprintf(s,
		"%s%s%s",
		(ParentIsDesktop(dcd->hwndFrame, (HWND) 0)) ?
		"VDir" :
		NullStr,
		(ParentIsDesktop(dcd->hwndFrame, (HWND) 0)) ?
		(!dcd->dontclose) ?
		" Master: " : ": " : NullStr, dcd->directory);
	WinSetWindowText(dcd->hwndFrame, s);
	WinSetWindowText(WinWindowFromID(dcd->hwndFrame, FID_TITLEBAR), s);
      }
      RemoveCnrItems(dcd->hwndCnr, NULL, 0, CMA_FREE | CMA_INVALIDATE | CMA_ERASE);
      AdjustCnrColsForFSType(dcd->hwndCnr, dcd->directory, &dcd->ds, FALSE);
      DosRequestMutexSem(hmtxFiltering, SEM_INDEFINITE_WAIT);
      dcd->ullTotalBytes = dcd->totalfiles = dcd->selectedfiles = dcd->selectedbytes = 0;
      DosReleaseMutexSem(hmtxFiltering);
      WinSetDlgItemText(dcd->hwndClient, DIR_TOTALS, "0 / 0k");
      WinSetDlgItemText(dcd->hwndClient, DIR_SELECTED, "0 / 0k");
      if (hwndStatus &&
	  dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent)) {
	WinSetWindowText(hwndStatus, (CHAR *) GetPString(IDS_PLEASEWAITSCANNINGTEXT));
	if (hwndMain)
	  WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
      }
      // 2014-06-26 SHL FM/2 Lite may not have drive tree yet
      if (hwndTree) {
	// 2015-08-12 SHL Optimze to update drive tree for only last saved state
	if (fSwitchTreeOnDirChg) {
	  // Keep drive tree in sync with directory container
          PSZ pszTempDir;
	  pszTempDir = xstrdup(dcd->directory, pszSrcFile, __LINE__);
	  if (pszTempDir) {
	    if (hwndMain) {
	      if (TopWindow(hwndMain, (HWND)0) == dcd->hwndFrame) {
		if (!PostMsg(hwndTree, UM_SHOWME, MPFROMP(pszTempDir), MPVOID))
		  free(pszTempDir);
	      }
	    }
	    else {
	      if (!PostMsg(hwndTree, UM_SHOWME, MPFROMP(pszTempDir), MPVOID))
		free(pszTempDir);
	    }
	  }
	} // fSwitchTreeOnDirChg
      } // if hwndTree
      dcd->firsttree = FALSE;
      WinStartTimer(WinQueryAnchorBlock(hwnd), dcd->hwndCnr, ID_DIRCNR_TIMER, 500);
      // fixme to check errors
      FillDirCnr(dcd->hwndCnr, dcd->directory, dcd, &dcd->ullTotalBytes);
      WinStopTimer(WinQueryAnchorBlock(hwnd), dcd->hwndCnr, ID_DIRCNR_TIMER);
      PostMsg(dcd->hwndCnr, UM_RESCAN, MPVOID, MPVOID);
      if (mp2 && !fLeaveTree && (dcd->flWindowAttr & CV_TREE)) {
	ULONG flWindowAttr = dcd->flWindowAttr;
	CNRINFO cnri;
	flWindowAttr &=	~(CV_NAME | CV_TREE | CV_ICON | CV_DETAIL | CV_TEXT);
	if (dcd->lastattr) {
	  if (dcd->lastattr & CV_TEXT)
	    flWindowAttr |= CV_TEXT;
	  else if (dcd->lastattr & CV_DETAIL)
	    flWindowAttr |= CV_DETAIL;
	  else if (dcd->lastattr & CV_ICON)
	    flWindowAttr |= CV_ICON;
	  else
	    flWindowAttr |= CV_NAME;
	}
	else
	  flWindowAttr |= CV_NAME;
	flWindowAttr |= CV_FLOW;
	memset(&cnri, 0, sizeof(CNRINFO));
	cnri.cb = sizeof(CNRINFO);
	if (WinSendMsg(dcd->hwndCnr, CM_QUERYCNRINFO, MPFROMP(&cnri),
		       MPFROMLONG(sizeof(CNRINFO)))) {
	  dcd->flWindowAttr = cnri.flWindowAttr = flWindowAttr;
	  WinSendMsg(dcd->hwndCnr, CM_SETCNRINFO,
		     MPFROMP(&cnri), MPFROMLONG(CMA_FLWINDOWATTR));
	  SayView(WinWindowFromID(dcd->hwndClient,
				  DIR_VIEW), dcd->flWindowAttr);
	}
      }
      if (dcd->flWindowAttr & CV_TREE)
	PostMsg(dcd->hwndObject, UM_FLESH, MPVOID, MPVOID);
      if (*dcd->previous) {
	if (strlen(dcd->previous) > strlen(dcd->directory) &&
	    !strnicmp(dcd->directory, dcd->previous,
		      strlen(dcd->directory)))
	{
	  PCNRITEM pci = FindCnrRecord(dcd->hwndCnr,
				       dcd->previous,
				       NULL, TRUE, FALSE, TRUE);
	  if (pci && (INT) pci != -1) {
	    // make found item current (cursored) item
	    WinSendMsg(dcd->hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		       MPFROM2SHORT(TRUE, CRA_CURSORED));
	    // make sure that record shows in viewport
	    ShowCnrRecord(dcd->hwndCnr, (PMINIRECORDCORE) pci);
	  }
	}
      }
    }
    return 0;

  case UM_COMMAND:
    if (mp1) {

      LISTINFO *li = (LISTINFO *) mp1;

      switch (li->type) {
      case IDM_DOITYOURSELF:
      case IDM_APPENDTOCLIP:
      case IDM_APPENDTOCLIPFILENAME:
      case IDM_SAVETOCLIP:
      case IDM_SAVETOCLIPFILENAME:
      case IDM_ARCHIVE:
      case IDM_ARCHIVEM:
      case IDM_VIEWTEXT:
      case IDM_VIEWBINARY:
      case IDM_VIEWARCHIVE:
      case IDM_VIEW:
      case IDM_EDITTEXT:
      case IDM_EDITBINARY:
      case IDM_EDIT:
      case IDM_OBJECT:
      case IDM_SHADOW:
      case IDM_SHADOW2:
      case IDM_JAVAEXE:
      case IDM_PRINT:
      case IDM_ATTRS:
      case IDM_DELETE:
      case IDM_PERMDELETE:
      case IDM_MCIPLAY:
      case IDM_UPDATE:
	if (li->type == IDM_DELETE)
	  ignorereadonly = FALSE;
	if (PostMsg(hwnd, UM_MASSACTION, mp1, mp2))
	  return (MRESULT) TRUE;
	break;
      default:
	if (PostMsg(hwnd, UM_ACTION, mp1, mp2))
	  return (MRESULT) TRUE;
      }
    }
    return 0;

  case UM_SELECT:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case IDM_SELECTBOTH:
      case IDM_SELECTONE:
      case IDM_SELECTMORE:
      case IDM_SELECTNEWER:
      case IDM_SELECTOLDER:
      case IDM_SELECTBIGGER:
      case IDM_SELECTSMALLER:
      case IDM_DESELECTBOTH:
      case IDM_DESELECTONE:
      case IDM_DESELECTMORE:
      case IDM_DESELECTNEWER:
      case IDM_DESELECTOLDER:
      case IDM_DESELECTBIGGER:
      case IDM_DESELECTSMALLER:
	SpecialSelect2(dcd->hwndParent, SHORT1FROMMP(mp1));
	break;
      case IDM_SELECTLIST:
	{
	  CHAR filename[CCHMAXPATH], *p, *pp;
	  ULONG size;

	  strcpy(filename, PCSZ_STARDOTLST);
	  size = CCHMAXPATH;
	  PrfQueryProfileData(fmprof, appname, "SaveToListName",
			      filename, &size);
	  pp = strrchr(filename, '\\');
	  if (!pp)
	    pp = filename;
	  p = strrchr(pp, '.');
	  if (p && *(p + 1) && p > pp + 1) {
	    if (pp > filename)
	      pp++;
	    *pp = '*';
	    pp++;
	    if (p > pp)
	      memmove(pp, p, strlen(p) + 1);
	  }
	  if (insert_filename(hwnd, filename, FALSE, FALSE))
	    SelectList(dcd->hwndCnr, TRUE, FALSE, FALSE, NULL, filename,
		       NULL);
	}
	break;
      case IDM_SELECTALL:
	SelectAll(dcd->hwndCnr, TRUE, TRUE, NULL, NULL, FALSE);
	break;
      case IDM_DESELECTALL:
	DeselectAll(dcd->hwndCnr, TRUE, TRUE, NULL, NULL, FALSE);
	break;
      case IDM_SELECTALLFILES:
	SelectAll(dcd->hwndCnr, TRUE, FALSE, NULL, NULL, FALSE);
	break;
      case IDM_DESELECTALLFILES:
	DeselectAll(dcd->hwndCnr, TRUE, FALSE, NULL, NULL, FALSE);
	break;
      case IDM_SELECTALLDIRS:
	SelectAll(dcd->hwndCnr, FALSE, TRUE, NULL, NULL, FALSE);
	break;
      case IDM_DESELECTALLDIRS:
	DeselectAll(dcd->hwndCnr, FALSE, TRUE, NULL, NULL, FALSE);
	break;
      case IDM_DESELECTMASK:
      case IDM_SELECTMASK:
	{
	  MASK mask;
	  PCNRITEM pci = (PCNRITEM) mp2;

	  memset(&mask, 0, sizeof(MASK));
	  mask.fNoAttribs = TRUE;
	  mask.fNoDirs = TRUE;
	  mask.fText = TRUE;
	  strcpy(mask.prompt,
		 GetPString((SHORT1FROMMP(mp1) == IDM_SELECTMASK) ?
			    IDS_SELECTFILTERTEXT : IDS_DESELECTFILTERTEXT));
	  if (pci && (INT) pci != -1)
	    strcpy(mask.szMask, pci->pszFileName);
	  if (WinDlgBox(HWND_DESKTOP,
			dcd->hwndCnr,
			PickMaskDlgProc,
			FM3ModHandle, MSK_FRAME, MPFROMP(&mask))) {
	    if (SHORT1FROMMP(mp1) == IDM_SELECTMASK)
	      SelectAll(dcd->hwndCnr,
			TRUE, TRUE, mask.szMask, mask.szText, FALSE);
	    else
	      DeselectAll(dcd->hwndCnr,
			  TRUE, TRUE, mask.szMask, mask.szText, FALSE);
	  }
	}
	break;

      case IDM_DESELECTCLIP:
      case IDM_SELECTCLIP:
	{
	  CHAR **list;

	  list = ListFromClipboard(hwnd);
	  if (list) {
	    SelectList(dcd->hwndCnr, TRUE, FALSE,
		       (SHORT1FROMMP(mp1) == IDM_DESELECTCLIP),
		       NULL, NULL, list);
	    FreeList(list);
	  }
	}
	break;

      case IDM_INVERT:
	InvertAll(dcd->hwndCnr);
	break;
      }
    }
    return 0;

  case UM_MASSACTION:
    if (mp1) {
      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (dcd) {
	WORKER *wk;
#	ifdef FORTIFY
	Fortify_EnterScope();
#	endif
	wk = xmallocz(sizeof(WORKER), pszSrcFile, __LINE__);
	if (!wk)
	  FreeListInfo((LISTINFO *) mp1);
	else {
	  wk->size = sizeof(WORKER);
	  wk->hwndCnr = dcd->hwndCnr;
	  wk->hwndParent = dcd->hwndParent;
	  wk->hwndFrame = dcd->hwndFrame;
	  wk->hwndClient = dcd->hwndClient;
	  wk->li = (LISTINFO *) mp1;
	  strcpy(wk->directory, dcd->directory);
	  if (xbeginthread(MassAction,
			   122880,
			   wk,
			   pszSrcFile,
			   __LINE__) == -1)
	  {
	    free(wk);
	    FreeListInfo((LISTINFO *)mp1);
	  }
	}
#	ifdef FORTIFY
	DosSleep(1);		 // Allow MassAction to take ownership
	Fortify_LeaveScope();
#	endif
      }
    }
    return 0;

  case UM_ACTION:
    if (mp1) {

      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (dcd) {

	WORKER *wk;
#	ifdef FORTIFY
	Fortify_EnterScope();
#	endif
	wk = xmallocz(sizeof(WORKER), pszSrcFile, __LINE__);
	if (!wk)
	  FreeListInfo((LISTINFO *) mp1);
	else {
	  wk->size = sizeof(WORKER);
	  wk->hwndCnr = dcd->hwndCnr;
	  wk->hwndParent = dcd->hwndParent;
	  wk->hwndFrame = dcd->hwndFrame;
	  wk->hwndClient = dcd->hwndClient;
	  wk->li = (LISTINFO *) mp1;
	  strcpy(wk->directory, dcd->directory);

	  if (xbeginthread(Action,
			   122880,
			   wk,
			   pszSrcFile,
			   __LINE__) == -1)
	  {
	    Runtime_Error(pszSrcFile, __LINE__,
			  GetPString(IDS_COULDNTSTARTTHREADTEXT));
	    free(wk);
	    FreeListInfo((LISTINFO *) mp1);
	  }
	}
#	ifdef FORTIFY
	Fortify_LeaveScope();
#	endif
      }
    }
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    break;

  case WM_DESTROY:
#   ifdef FORTIFY
    DbgMsg(pszSrcFile, __LINE__, "WM_DESTROY hwnd %p TID %u", hwnd, GetTidForThread());	// 18 Jul 08 SHL fixme
#   endif
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!dcd)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else {
      if (dcd->hwndRestore)
	WinSetWindowPos(dcd->hwndRestore,
			HWND_TOP,
			0,
			0,
			0,
			0,
	                SWP_RESTORE | SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER);
      FreeList(dcd->lastselection);
      WinSetWindowPtr(dcd->hwndCnr, QWL_USER, NULL);	// 13 Apr 10 SHL Set NULL before freeing dcd
      DosRequestMutexSem(hmtxFiltering, SEM_INDEFINITE_WAIT);
      xfree(dcd, pszSrcFile, __LINE__);
      dcd = NULL;
      DosReleaseMutexSem(hmtxFiltering);
      DosPostEventSem(CompactSem);
    }
#   ifdef FORTIFY
    Fortify_LeaveScope();
#   endif
    // 22 Jul 08 SHL fixme to understand
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY DirCnrWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *dcd = INSTDATA(hwnd);
  CHAR tf[64];
  CHAR tb[64];
  CHAR s[CCHMAXPATH];

  switch (msg) {
  case WM_CREATE:
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif
    break;

  case DM_PRINTOBJECT:
    return MRFROMLONG(DRR_TARGET);

  case DM_DISCARDOBJECT:
    if (dcd)
      return WinSendMsg(dcd->hwndObject, msg, mp1, mp2);
    else
      return MRFROMLONG(DRR_TARGET);

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (SHORT1FROMMP(mp1) & KC_KEYUP)
      return (MRESULT) TRUE;
    if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY) {
      switch (SHORT2FROMMP(mp2)) {
      case VK_INSERT:
	if ((shiftstate & KC_CTRL) == KC_CTRL)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_MKDIR, 0), MPVOID);
	// Alt-Insert - create file
	else if ((shiftstate & KC_ALT) == KC_ALT)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_CREATE, 0), MPVOID);
	break;
      case VK_PAGEUP:
	if ((shiftstate & KC_CTRL) == KC_CTRL)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_PARENT, 0), MPVOID);
	break;
      case VK_PAGEDOWN:
	if ((shiftstate & KC_CTRL) == KC_CTRL)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_PREVIOUS, 0), MPVOID);
	break;
      case VK_HOME:
	if ((shiftstate & KC_CTRL) == KC_CTRL && dcd) {
	  PSZ p;
	  strcpy(s, dcd->directory);
	  p = strchr(s, '\\');
	  if (p) {
	    p++;
	    *p = 0;
	    WinSendMsg(hwnd, UM_SETDIR, MPFROMP(s), MPVOID);
	  }
	}
	break;
      case VK_DELETE:
	if ((shiftstate & KC_CTRL) == KC_CTRL)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_PERMDELETE, 0), MPVOID);
	else if ((shiftstate & KC_SHIFT) == KC_SHIFT)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_SAVETOCLIP, 0), MPVOID);
	else
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_DELETE, 0), MPVOID);
	break;
      } // switch
    }

    if (SearchContainer(hwnd, msg, mp1, mp2))
      return (MRESULT)TRUE;		// Avoid default handler
    break;				// Let default handler see key too

  case WM_MOUSEMOVE:
  case WM_BUTTON1UP:
  case WM_BUTTON2UP:
  case WM_BUTTON3UP:
  case WM_CHORD:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    break;

  case WM_BUTTON1MOTIONEND:
    {
      CNRINFO cnri;

      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      if (WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnri),
		     MPFROMLONG(sizeof(CNRINFO)))) {
	if (cnri.flWindowAttr & CV_DETAIL)
	  PrfWriteProfileData(fmprof, appname, "CnrSplitBar",
			      (PVOID) & cnri.xVertSplitbar, sizeof(LONG));
      }
    }
    break;

  case UM_COMPARE:
    if (dcd && mp1 && mp2) {

      COMPARE *cmp;
      CHAR *leftdir = (CHAR *)mp1, *rightdir = (CHAR *)mp2;

      if (!IsFile(leftdir) && !IsFile(rightdir)) {
#	ifdef FORTIFY
	Fortify_EnterScope();
#	endif
	cmp = xmallocz(sizeof(COMPARE), pszSrcFile, __LINE__);
	if (cmp) {
	  cmp->size = sizeof(COMPARE);
	  strcpy(cmp->leftdir, leftdir);
	  strcpy(cmp->rightdir, rightdir);
	  cmp->hwndParent = dcd->hwndParent;
	  cmp->dcd.hwndParent = dcd->hwndParent;
	  WinDlgBox(HWND_DESKTOP,
		    HWND_DESKTOP,
		    CompareDlgProc, FM3ModHandle, COMP_FRAME, MPFROMP(cmp));
	}
#	ifdef FORTIFY
	Fortify_LeaveScope();
#	endif
      }
    }
    return 0;

   case WM_PRESPARAMCHANGED:
     PresParamChanged(hwnd, PCSZ_DIRCNR, mp1, mp2);
     break;

  case UM_UPDATERECORDLIST:
    if (dcd && mp1)
      WinSendMsg(dcd->hwndObject, msg, mp1, mp2);
    return 0;

  case UM_UPDATERECORD:
    if (dcd && mp1) {

      CHAR *filename;

      filename = mp1;
      if (filename)
	UpdateCnrRecord(hwnd, filename, TRUE, dcd);
    }
    return 0;

  case WM_SETFOCUS:
    // put name of our window (directory name) on status line
    if (mp2) {
      // Getting focus
      if (dcd && hwndStatus) {
	// put name of our window (directory name) on status line
	PCNRITEM pci = NULL;
	if (fAutoView && hwndMain) {
	  pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
			   MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1 &&
	      (!(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_SLOW)))
	    WinSendMsg(hwndMain, UM_LOADFILE, MPFROMP(pci->pszFileName), MPVOID);
	  else
	    WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	}
	if (*dcd->directory) {
	  if (hwndMain)
	    WinSendMsg(hwndMain,
		       UM_SETUSERLISTNAME, MPFROMP(dcd->directory), MPVOID);
	  else
	    add_udir(FALSE, dcd->directory);
	}

      if (hwndMain)
	PostMsg(hwndMain, UM_ADVISEFOCUS, MPFROMLONG(dcd->hwndFrame), MPVOID);
    }

      LastDir = hwnd;
      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      // 2015-08-13 SHL
      if (fSwitchTreeOnFocus && hwndTree && dcd && *dcd->directory) {
	PSZ pszTempDir = xstrdup(dcd->directory, pszSrcFile, __LINE__);
	if (pszTempDir) {
	  if (!PostMsg(hwndTree, UM_SHOWME, MPFROMP(pszTempDir), MPVOID))
	    free(pszTempDir);		// Failed
	}
      }
    }
    break;

  case UM_SETDIR:
    if (dcd && mp1) {

      CHAR fullname[CCHMAXPATH];

      DosError(FERR_DISABLEHARDERR);
      if (!DosQueryPathInfo((CHAR *)mp1,
			    FIL_QUERYFULLNAME, fullname, sizeof(fullname))) {
	if (stricmp(dcd->directory, fullname)) {
	  strcpy(dcd->previous, dcd->directory);
	  strcpy(dcd->directory, fullname);
	  dcd->stopflag++;
	  if (!PostMsg(dcd->hwndObject, UM_RESCAN, MPVOID, MPFROMLONG(1L))) {
	    strcpy(dcd->directory, dcd->previous);
	    dcd->stopflag--;
	  }
	  else if (*dcd->directory) {
	    if (hwndMain)
	      WinSendMsg(hwndMain,
			 UM_SETUSERLISTNAME, MPFROMP(dcd->directory), MPVOID);
	    else
	      add_udir(FALSE, dcd->directory);
	  }
	}
      }
    }
    break;

  case WM_TIMER:
    // Started/stopped by DirObjWndPro
    if (dcd) {
      commafmt(tb, sizeof(tb), dcd->totalfiles);
      CommaFmtULL(tf, sizeof(tf), dcd->ullTotalBytes, 'K');
      sprintf(s, "%s / %s", tb, tf);
      WinSetDlgItemText(dcd->hwndClient, DIR_TOTALS, s);
    }
    break; // WM_TIMER

  case UM_RESCAN:
    if (dcd) {

      CNRINFO cnri;
      CHAR szDate[DATE_BUF_BYTES];
      PCNRITEM pci;

      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendMsg(hwnd,
		 CM_QUERYCNRINFO,
		 MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
      cnri.pszCnrTitle = dcd->directory;
      WinSendMsg(hwnd,
	         CM_SETCNRINFO, MPFROMP(&cnri), MPFROMLONG(CMA_CNRTITLE));
      DosRequestMutexSem(hmtxFiltering, SEM_INDEFINITE_WAIT);
      dcd->totalfiles = cnri.cRecords;
      commafmt(tb, sizeof(tb), dcd->totalfiles);
      CommaFmtULL(tf, sizeof(tf), dcd->ullTotalBytes, 'K');
      DosReleaseMutexSem(hmtxFiltering);
      sprintf(s, "%s / %s", tb, tf);
      WinSetDlgItemText(dcd->hwndClient, DIR_TOTALS, s);
      commafmt(tb, sizeof(tb), dcd->selectedfiles);
      CommaFmtULL(tf, sizeof(tf), dcd->selectedbytes, 'K');
      sprintf(s, "%s / %s", tb, tf);
      WinSetDlgItemText(dcd->hwndClient, DIR_SELECTED, s);
      if (hwndStatus &&
	  dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent)) {
	PostMsg(dcd->hwndObject, UM_RESCAN2, MPVOID, MPVOID);
	if ((fSplitStatus && hwndStatus2) || fMoreButtons) {
	  pci = WinSendMsg(hwnd,
			   CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1) {
	    if (fSplitStatus && hwndStatus2) {
	      CommaFmtULL(tb, sizeof(tb), pci->cbFile + pci->easize, ' ');
	      if (!fMoreButtons) {
		DateFormat(szDate, pci->date);
		sprintf(s, " %s  %s %02u%s%02u%s%02u  [%s]  %s",
			tb,
			szDate,
			pci->time.hours,
			TimeSeparator,
			pci->time.minutes,
			TimeSeparator,
			pci->time.seconds,
			pci->pszDispAttr, pci->pszFileName);
	      }
	      else {
		*tf = 0;
		if (pci->cbFile + pci->easize > 1024) {
		  CommaFmtULL(tf, sizeof(tf), pci->cbFile + pci->easize, 'K');
		}
		sprintf(s,
			GetPString(IDS_STATUSSIZETEXT),
			tb, *tf ? " (" : NullStr, tf, *tf ? ")" : NullStr);
	      }
	      WinSetWindowText(hwndStatus2, s);
	    }
	    else
	      WinSetWindowText(hwndStatus2, NullStr);
	    if (fMoreButtons) {
	      WinSetWindowText(hwndName, pci->pszFileName);
	      DateFormat(szDate, pci->date);
	      sprintf(s, "%s %02u%s%02u%s%02u",
		      szDate,
		      pci->time.hours, TimeSeparator,
		      pci->time.minutes, TimeSeparator,
		      pci->time.seconds);
	      WinSetWindowText(hwndDate, s);
	      WinSetWindowText(hwndAttr, pci->pszDispAttr);
	    }
	  }
	  else {
	    WinSetWindowText(hwndStatus2, NullStr);
	    WinSetWindowText(hwndName, NullStr);
	    WinSetWindowText(hwndDate, NullStr);
	    WinSetWindowText(hwndAttr, NullStr);
	  }
	}
      }
    }
    return 0;

  case UM_SORTRECORD:
    if (dcd) {

      CNRINFO cnri;

      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendMsg(hwnd,
		 CM_QUERYCNRINFO,
	         MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
      cnri.pSortRecord = (PVOID) SortDirCnr;
      WinSendMsg(hwnd,
		 CM_SETCNRINFO, MPFROMP(&cnri), MPFROMLONG(CMA_PSORTRECORD));
      WinSendMsg(hwnd,
		 CM_SORTRECORD,
		 MPFROMP(SortDirCnr), MPFROMLONG(dcd->sortFlags));
    }
    return 0;

  case UM_SETUP:
    if (dcd) {
      if (!dcd->hwndObject) {
	// first time through -- set things up

	CNRINFO cnri;

	memset(&cnri, 0, sizeof(CNRINFO));
	cnri.cb = sizeof(CNRINFO);
	WinSendMsg(hwnd,
		   CM_QUERYCNRINFO,
		   MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
	cnri.cyLineSpacing = 0;
	cnri.cxTreeIndent = 12L;

	cnri.flWindowAttr &= (~(CV_TREE | CV_ICON | CV_DETAIL | CV_TEXT));
	cnri.flWindowAttr |= (CV_NAME | CA_DETAILSVIEWTITLES | CV_MINI |
	                      CV_FLOW);
	cnri.pSortRecord = (PVOID) SortDirCnr;

	{
	  ULONG size = sizeof(ULONG);

	  PrfQueryProfileData(fmprof, appname, "DirflWindowAttr",
			      (PVOID) & cnri.flWindowAttr, &size);
	  size = sizeof(MASK);
	  if (!*dcd->mask.szMask &&
	      !dcd->mask.attrFile && !dcd->mask.antiattr) {
	    if (PrfQueryProfileSize(fmprof, appname, "DirFilter", &size) && size) {
	      PrfQueryProfileData(fmprof, appname, "DirFilter", &dcd->mask, &size);
	      SetMask(dcd->mask.szMask, &dcd->mask);
	    }
	    else
	      dcd->mask.attrFile = (FILE_READONLY | FILE_NORMAL |
				    FILE_ARCHIVED | FILE_DIRECTORY |
				    FILE_HIDDEN | FILE_SYSTEM);
	  }
	  *(dcd->mask.prompt) = 0;
	}
	if (dcd->flWindowAttr)
	  cnri.flWindowAttr = dcd->flWindowAttr;
	else
	  dcd->flWindowAttr = cnri.flWindowAttr;
	cnri.flWindowAttr &= (~(CA_MIXEDTARGETEMPH | CA_ORDEREDTARGETEMPH |
				CA_TITLEREADONLY | CA_TITLESEPARATOR));
	cnri.flWindowAttr |= CV_FLOW;
	dcd->flWindowAttr |= CV_FLOW;
	if (WinWindowFromID(dcd->hwndFrame, FID_TITLEBAR))
	  cnri.flWindowAttr &= (~CA_CONTAINERTITLE);
	else
	  cnri.flWindowAttr |= CA_CONTAINERTITLE;
	if (!dcd->sortFlags)
	  dcd->sortFlags = sortFlags;
	WinSendMsg(hwnd,
		   CM_SETCNRINFO,
		   MPFROMP(&cnri),
		   MPFROMLONG(CMA_FLWINDOWATTR | CMA_LINESPACING |
			      CMA_CXTREEINDENT | CMA_PSORTRECORD));
	SetCnrCols(hwnd, FALSE);
	if (xbeginthread(MakeObjWin,
			 245760,
			 dcd,
			 pszSrcFile,
			 __LINE__) == -1)
	{
	  Runtime_Error(pszSrcFile, __LINE__,
			GetPString(IDS_COULDNTSTARTTHREADTEXT));
	  PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	  return 0;
	}
	else
	  DosSleep(32); 
	WinEnableMenuItem(DirCnrMenu, IDM_FINDINTREE, (hwndTree != (HWND) 0));
      }
	// 2014-06-11 SHL fm/2 lite can get here before drive scan completes - may not be true anymore
	PostMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);
    }
    else {
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      return 0;
    }
    return 0;

  case UM_SETUP2:
    if (dcd) {
      AdjustCnrColsForPref(hwnd, NULL, &dcd->ds, FALSE);
      SayFilter(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
		DIR_FILTER), &dcd->mask, FALSE);
      SaySort(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
	      DIR_SORT), dcd->sortFlags, FALSE);
      SayView(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
			      DIR_VIEW), dcd->flWindowAttr);
    } else
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    return 0;

  case WM_MENUEND:
    if (dcd) {

      HWND hwndMenu = (HWND) mp2;

      if (hwndMenu == DirCnrMenu ||
	  hwndMenu == FileMenu ||
	  hwndMenu == DirMenu) {
	MarkAll(hwnd, TRUE, FALSE, TRUE);
	if (dcd->cnremphasized) {
	  WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
		     MPFROM2SHORT(FALSE, CRA_SOURCE));
	  dcd->cnremphasized = FALSE;
	}
      }
    }
    break;

  case UM_OPENWINDOWFORME:
    if (dcd) {
      if (mp1 && !IsFile((CHAR *)mp1)) {
	OpenDirCnr(hwnd, dcd->hwndParent, dcd->hwndFrame, FALSE, (char *)mp1);
      }
      else if (mp1 && IsFile(mp1) == 1 &&
	       CheckDriveSpaceAvail(ArcTempRoot, ullDATFileSpaceNeeded, ullTmpSpaceNeeded) != 2) {
	StartArcCnr(HWND_DESKTOP,
		    dcd->hwndFrame, (CHAR *)mp1, 4, (ARC_TYPE *) mp2);
      }
    }
    return 0;

  case MM_PORTHOLEINIT:
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case 0:
      case 1:
	{
	  ULONG wmsg;

	  wmsg = (SHORT1FROMMP(mp1) == 0) ? UM_FILESMENU : UM_VIEWSMENU;
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
	CopyPresParams((HWND) mp2, hwndMainMenu);
	if (isalpha(*dcd->directory)) {
	  if (driveflags[toupper(*dcd->directory) - 'A'] & DRIVE_NOTWRITEABLE) {
	    WinEnableMenuItem((HWND) mp2, IDM_MOVEMENU, FALSE);
	    WinEnableMenuItem((HWND) mp2, IDM_RENAME, FALSE);
	    WinEnableMenuItem((HWND) mp2, IDM_MKDIR, FALSE);
	    WinEnableMenuItem((HWND) mp2, IDM_UNDELETE, FALSE);
	    WinEnableMenuItem((HWND) mp2, IDM_DELETESUBMENU, FALSE);
	    WinEnableMenuItem((HWND) mp2, IDM_DELETE, FALSE);
	    WinEnableMenuItem((HWND) mp2, IDM_EDIT, FALSE);
	    WinEnableMenuItem((HWND) mp2, IDM_EDITTEXT, FALSE);
	    WinEnableMenuItem((HWND) mp2, IDM_EDITBINARY, FALSE);
	    WinEnableMenuItem((HWND) mp2, IDM_ATTRS, FALSE);
	  }
	  else {
	    WinEnableMenuItem((HWND) mp2, IDM_MOVEMENU, TRUE);
	    WinEnableMenuItem((HWND) mp2, IDM_RENAME, TRUE);
	    WinEnableMenuItem((HWND) mp2, IDM_MKDIR, TRUE);
	    WinEnableMenuItem((HWND) mp2, IDM_UNDELETE, TRUE);
	    WinEnableMenuItem((HWND) mp2, IDM_DELETESUBMENU, TRUE);
	    WinEnableMenuItem((HWND) mp2, IDM_DELETE, TRUE);
	    WinEnableMenuItem((HWND) mp2, IDM_EDIT, TRUE);
	    WinEnableMenuItem((HWND) mp2, IDM_EDITTEXT, TRUE);
	    WinEnableMenuItem((HWND) mp2, IDM_EDITBINARY, TRUE);
	    WinEnableMenuItem((HWND) mp2, IDM_ATTRS, TRUE);
	  }
	WinEnableMenuItem((HWND) mp2, IDM_UNLOCKFILE, fUnlock);
	}
	break;

      case IDM_VIEWSMENU:
	SetViewMenu((HWND) mp2, dcd->flWindowAttr);
	CopyPresParams((HWND) mp2, hwndMainMenu);
	WinEnableMenuItem((HWND) mp2, IDM_RESELECT,
			  (dcd->lastselection != NULL));
	if (isalpha(*dcd->directory)) {
	  if (driveflags[toupper(*dcd->directory) - 'A'] & DRIVE_NOTWRITEABLE)
	    WinEnableMenuItem((HWND) mp2, IDM_MKDIR, FALSE);
	  else
	    WinEnableMenuItem((HWND) mp2, IDM_MKDIR, TRUE);
	}
	WinEnableMenuItem((HWND) mp2,
			  IDM_SELECTCOMPAREMENU,
		          (CountDirCnrs(dcd->hwndParent) > 1));
	break;

      case IDM_DETAILSSETUP:
	SetDetailsSwitches((HWND) mp2, &dcd->ds);
	break;

      case IDM_COMMANDSMENU:
	SetupCommandMenu((HWND) mp2, hwnd);
	break;

      case IDM_SORTSUBMENU:
	SetSortChecks((HWND) mp2, dcd->sortFlags);
	break;

      case IDM_WINDOWSMENU:
	SetupWinList((HWND) mp2,
		     (hwndMain) ? hwndMain : (HWND) 0, dcd->hwndFrame);
	break;
      }
      dcd->hwndLastMenu = (HWND) mp2;
    }
    if (msg == WM_INITMENU)
      break;
    return 0;

  case UM_FILTER:
    if (dcd) {

      PCNRITEM pci;

      if (mp1)
	SetMask((CHAR *)mp1, &dcd->mask);

      dcd->suspendview = 1;
      WinSendMsg(hwnd, CM_FILTER, MPFROMP(Filter), MPFROMP(&dcd->mask));
      dcd->suspendview = 0;
      if (fAutoView && hwndMain) {
	pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
			 MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	if (pci && (INT) pci != -1 &&
	    (!(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_SLOW)))
	  WinSendMsg(hwndMain, UM_LOADFILE, MPFROMP(pci->pszFileName), MPVOID);
	else
	  WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
      }
      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    }
    return 0;

  case UM_COMMAND:
    if (mp1) {
      if (dcd) {
	if (!PostMsg(dcd->hwndObject, UM_COMMAND, mp1, mp2)) {
	  Runtime_Error(pszSrcFile, __LINE__, PCSZ_POSTMSG);
	  FreeListInfo((LISTINFO *) mp1);
	}
	else
	  return (MRESULT) TRUE;
      }
      else
	FreeListInfo((LISTINFO *) mp1);
    }
    return 0;

  case UM_NOTIFY:
    if (mp2)
      Notify((CHAR *)mp2);
    return 0;

  case UM_DRIVECMD:
    if (mp1)
      WinSendMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_SWITCH, 0), mp1);
    return 0;

  case WM_COMMAND:
    DosError(FERR_DISABLEHARDERR);
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case IDM_SETTARGET:
	SetTargetDir(hwnd, FALSE, NULL);
	break;

      case IDM_CREATE:
	{
	  STRINGINPARMS sip;
	  CHAR filename[CCHMAXPATHCOMP];

	  memset(&sip, 0, sizeof(sip));
	  sip.help = GetPString(IDS_CREATETEXT);
	  sip.prompt = GetPString(IDS_CREATEPROMPTTEXT);
	  sip.inputlen = CCHMAXPATHCOMP - (strlen(dcd->directory) - 1);
	  strcpy(filename, "NEWFILE.TXT");
	  sip.ret = filename;
	  sip.title = GetPString(IDS_CREATETITLETEXT);
	  if (WinDlgBox(HWND_DESKTOP, hwnd, InputDlgProc, FM3ModHandle,
			STR_FRAME, &sip)) {
	    bstrip(sip.ret);
	    if (*sip.ret) {
	      CHAR newfile[CCHMAXPATH];
	      FILE *fp;
	      INT test;
	      PCNRITEM pci;

	      strcpy(newfile, dcd->directory);
	      AddBackslashToPath(newfile);
#	      if 0
	      if (newfile[strlen(newfile) - 1] != '\\')
		strcat(newfile, "\\");
#	      endif
	      strcat(newfile, sip.ret);
	      test = IsFile(newfile);
	      if (test != 1) {
		CHAR *modew = "w";

		fp = xfopen(newfile, modew, pszSrcFile, __LINE__, TRUE);
	      }
	      if (test != 1 && !fp) {
		saymsg(MB_ENTER,
		       hwnd,
		       GetPString(IDS_ERRORTEXT),
		       GetPString(IDS_CREATEERRORTEXT), newfile);
	      }
	      else {
		if (fp) {
		  WinSendMsg(hwnd, UM_UPDATERECORD, MPFROMP(newfile), MPVOID);
		  fclose(fp);
		}
		if (*editor) {

		  CHAR *dummy[2];

		  dummy[0] = newfile;
		  dummy[1] = NULL;
		  ExecOnList(hwnd,
		             editor, WINDOWED | SEPARATE, NULL, NULL,
		             dummy, NULL, pszSrcFile, __LINE__);
		}
		else
		  StartMLEEditor(dcd->hwndParent, 4, newfile, dcd->hwndFrame);
		pci = FindCnrRecord(hwnd, newfile, NULL, TRUE, FALSE, TRUE);
		if (pci && (INT) pci != -1)
		  // make sure that record shows in viewport
		  ShowCnrRecord(hwnd, (PMINIRECORDCORE) pci);
	      }
	    }
	  }
	}
	break;

      case IDM_CONTEXTMENU:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  PostMsg(hwnd, WM_CONTROL, MPFROM2SHORT(DIR_CNR, CN_CONTEXTMENU),
		  MPFROMP(pci));
	}
	break;

      case IDM_MAXIMIZE:
	PostMsg(hwndMain, UM_MAXIMIZE, MPFROMLONG(dcd->hwndFrame), MPVOID);
	break;

      case IDM_SHOWALLFILESCNR:
	StartSeeAll(HWND_DESKTOP, FALSE, dcd->directory);
	break;

      case IDM_SHOWALLFILES:
	{
	  PCNRITEM pci;

	  pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1) {

	    static CHAR dirname[CCHMAXPATH];

	    strcpy(dirname, pci->pszFileName);
	    MakeValidDir(dirname);
	    StartSeeAll(HWND_DESKTOP, FALSE, dirname);
	  }
	}
	break;

      case IDM_FINDINTREE:
	if (hwndTree) {
	  PSZ pszTempDir = xstrdup(dcd->directory, pszSrcFile, __LINE__);

	  if (pszTempDir) {
	    if (!PostMsg(hwndTree, UM_SHOWME, MPFROMP(pszTempDir),
			    MPFROMLONG(1L)))
	      free(pszTempDir);
	  }
	}
	break;

      case IDM_BEGINEDIT:
	OpenEdit(hwnd);
	break;

      case IDM_ENDEDIT:
	WinSendMsg(hwnd, CM_CLOSEEDIT, MPVOID, MPVOID);
	break;

      case IDM_SHOWSELECT:
	QuickPopup(hwnd,
		   dcd,
		   CheckMenu(hwnd, &DirCnrMenu, DIRCNR_POPUP), IDM_SELECTSUBMENU);
	break;

      case IDM_SHOWSORT:
	QuickPopup(hwnd, dcd, CheckMenu(hwnd, &DirCnrMenu, DIRCNR_POPUP),
		   IDM_SORTSUBMENU);
	break;

      case IDM_VIEWORARC:
	{
	  SWP swp;
	  PCNRITEM pci;

	  pci = (PCNRITEM) WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
				      MPFROMLONG(CMA_FIRST),
				      MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1) {
	    WinQueryWindowPos(dcd->hwndFrame, &swp);
	    DefaultViewKeys(hwnd,
			    dcd->hwndFrame,
			    dcd->hwndParent, &swp, pci->pszFileName);
	  }
	}
	break;

      case IDM_DIRCNRSETTINGS:
	if (!ParentIsDesktop(dcd->hwndParent, dcd->hwndParent))
	  PostMsg(dcd->hwndParent, msg, MPFROMLONG(IDM_DIRCNRSETTINGS), mp2);
	else {
	  WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    CfgDlgProc,
		    FM3ModHandle,
		    CFG_FRAME,
		    MPFROMLONG(IDM_DIRCNRSETTINGS));
	}
	break;

      case IDM_QTREE:
      case IDM_TREE:
	{
	  CHAR newpath[CCHMAXPATH];
	  APIRET rc;
	  PCNRITEM pci;

	  if (SHORT1FROMMP(mp1) == IDM_TREE) {
	    pci = (PCNRITEM) CurrentRecord(hwnd);
	    if (pci && (INT) pci != -1)
	      strcpy(newpath, pci->pszFileName);
	    else
	      strcpy(newpath, dcd->directory);
	  }
	  else
	    strcpy(newpath, dcd->directory);
	  MakeValidDir(newpath);
	  rc = WinDlgBox(HWND_DESKTOP, dcd->hwndClient, ObjCnrDlgProc,
			 FM3ModHandle, QTREE_FRAME, MPFROMP(newpath));
	  if (rc)
	    WinSendMsg(hwnd, UM_SETDIR, MPFROMP(newpath), MPVOID);
	}
	break;

      case IDM_RESELECT:
	SelectList(hwnd, TRUE, FALSE, FALSE, NULL, NULL, dcd->lastselection);
	break;

      case IDM_HELP:
	if (hwndHelp) {
	  if (!ParentIsDesktop(dcd->hwndFrame, dcd->hwndParent))
	    PostMsg(dcd->hwndParent, UM_COMMAND, mp1, mp2);
	  else
	    WinSendMsg(hwndHelp, HM_HELP_CONTENTS, MPVOID, MPVOID);
	}
	break;

      case IDM_WINDOWDLG:
	if (!ParentIsDesktop(dcd->hwndFrame, dcd->hwndParent))
	  PostMsg(dcd->hwndParent, UM_COMMAND,
		  MPFROM2SHORT(IDM_WINDOWDLG, 0), MPVOID);
	break;

      case IDM_SORTSMARTNAME:
      case IDM_SORTNAME:
      case IDM_SORTFILENAME:
      case IDM_SORTSIZE:
      case IDM_SORTEASIZE:
      case IDM_SORTFIRST:
      case IDM_SORTLAST:
      case IDM_SORTLWDATE:
      case IDM_SORTLADATE:
      case IDM_SORTCRDATE:
      case IDM_SORTSUBJECT:
	dcd->sortFlags &= (SORT_REVERSE | SORT_DIRSFIRST | SORT_DIRSLAST);
      case IDM_SORTDIRSFIRST:
      case IDM_SORTDIRSLAST:
      case IDM_SORTREVERSE:
	switch (SHORT1FROMMP(mp1)) {
	case IDM_SORTSUBJECT:
	  dcd->sortFlags |= SORT_SUBJECT;
	  break;
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
	case IDM_SORTLADATE:
	  dcd->sortFlags |= SORT_LADATE;
	  break;
	case IDM_SORTCRDATE:
	  dcd->sortFlags |= SORT_CRDATE;
	  break;
	case IDM_SORTDIRSFIRST:
	  if (dcd->sortFlags & SORT_DIRSFIRST)
	    dcd->sortFlags &= (~SORT_DIRSFIRST);
	  else {
	    dcd->sortFlags |= SORT_DIRSFIRST;
	    dcd->sortFlags &= (~SORT_DIRSLAST);
	  }
	  break;
	case IDM_SORTDIRSLAST:
	  if (dcd->sortFlags & SORT_DIRSLAST)
	    dcd->sortFlags &= (~SORT_DIRSLAST);
	  else {
	    dcd->sortFlags |= SORT_DIRSLAST;
	    dcd->sortFlags &= (~SORT_DIRSFIRST);
	  }
	  break;
	case IDM_SORTREVERSE:
	  if (dcd->sortFlags & SORT_REVERSE)
	    dcd->sortFlags &= (~SORT_REVERSE);
	  else
	    dcd->sortFlags |= SORT_REVERSE;
	  break;
	}
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(SortDirCnr),
		   MPFROMLONG(dcd->sortFlags));
	SaySort(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				DIR_SORT), dcd->sortFlags, FALSE);
	break;

      case IDM_COLLECT:
      case IDM_GREP:
	if (!Collector) {

	  HWND hwndC;
	  SWP swp;

	  if (!ParentIsDesktop(hwnd, dcd->hwndParent) && !fAutoTile &&
	      (!fExternalCollector && !strcmp(realappname, FM3Str)))
	    GetNextWindowPos(dcd->hwndParent, &swp, NULL, NULL);
	  hwndC = StartCollector((fExternalCollector ||
				  strcmp(realappname, FM3Str)) ?
				 HWND_DESKTOP : dcd->hwndParent, 4);
	  if (hwndC) {
	    if (!ParentIsDesktop(hwnd, dcd->hwndParent) && !fAutoTile &&
		(!fExternalCollector && !strcmp(realappname, FM3Str)))
	      WinSetWindowPos(hwndC, HWND_TOP, swp.x, swp.y,
			      swp.cx, swp.cy, SWP_MOVE | SWP_SIZE |
			      SWP_SHOW | SWP_ZORDER);
	    else if (!ParentIsDesktop(hwnd, dcd->hwndParent) && fAutoTile &&
		     !strcmp(realappname, FM3Str))
	      TileChildren(dcd->hwndParent, TRUE);
	    WinSetWindowPos(hwndC, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE);
	    DosSleep(100);
	  }
	}
	else
	  StartCollector(dcd->hwndParent, 4);
	if (SHORT1FROMMP(mp1) == IDM_GREP) {
	  PCNRITEM pci = NULL;

	  pci = WinSendMsg(hwnd,
			   CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1)
	    PostMsg(Collector, WM_COMMAND,
		    MPFROM2SHORT(IDM_GREP, 0), MPFROMP(pci->pszFileName));
	  else
	    PostMsg(Collector, WM_COMMAND,
		    MPFROM2SHORT(IDM_GREP, 0), MPVOID);
	}
	else
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_COLLECTOR, 0), MPVOID);
	break;

      case IDM_COLLECTOR:
	DosSleep(32); 
	{
	  CHAR **list;

	  list = BuildList(hwnd);
	  if (list) {
	    if (Collector) {
	      if (!PostMsg(Collector,
			   WM_COMMAND,
			   MPFROM2SHORT(IDM_COLLECTOR, 0), MPFROMP(list)))
		FreeList(list);
	      else if (fUnHilite)
		UnHilite(hwnd, TRUE, &dcd->lastselection, 0);
	    }
	    else
	      FreeList(list);
	  }
	}
	break;

      case IDM_UNDELETE:
	{
	  PCNRITEM pci;
	  CHAR path[CCHMAXPATH];
	  HOBJECT hObject;
	  HWND hwndDesktop;

	  hObject = WinQueryObject("<XWP_TRASHCAN>");
	  if (hObject != NULLHANDLE && fTrashCan) {
	    hwndDesktop = WinQueryDesktopWindow((HAB) 0, NULLHANDLE);
	    WinSetFocus(HWND_DESKTOP, hwndDesktop);
	    WinOpenObject(hObject, 0, TRUE);
	  }
	  else {
	    pci = (PCNRITEM) CurrentRecord(hwnd);
	    if (pci && (INT) pci != -1) {
	      strcpy(path, pci->pszFileName);
	      MakeValidDir(path);
	      WinDlgBox(HWND_DESKTOP, hwnd, UndeleteDlgProc, FM3ModHandle,
			UNDEL_FRAME, MPFROMP(path));
	    }
	  }
	}
	break;

      case IDM_UNDELETESPEC:
	{
	  HOBJECT hObject;
	  HWND hwndDesktop;

	  hObject = WinQueryObject("<XWP_TRASHCAN>");
	  if (hObject != NULLHANDLE && fTrashCan) {
	    hwndDesktop = WinQueryDesktopWindow((HAB) 0, NULLHANDLE);
	    WinSetFocus(HWND_DESKTOP, hwndDesktop);
	    WinOpenObject(hObject, 0, TRUE);
	  }
	else
	  WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    UndeleteDlgProc,
		    FM3ModHandle, UNDEL_FRAME, MPFROMP(dcd->directory));
	}
	break;

      case IDM_RESORT:
#	    if 0
	    WinSendMsg(hwnd,
		       CM_SORTRECORD,
		       MPFROMP(SortDirCnr),
		       MPFROMLONG((fSyncUpdates) ? sortFlags : dcd->sortFlags));
#endif
	WinSendMsg(hwnd,
		   CM_SORTRECORD,
		   MPFROMP(SortDirCnr), MPFROMLONG(dcd->sortFlags));
	break;

      case IDM_FILTER:
	{
	  BOOL empty = FALSE;
	  PCNRITEM pci;
	  CHAR *p;

	  if (!*dcd->mask.szMask) {
	    empty = TRUE;
	    pci = (PCNRITEM) CurrentRecord(hwnd);
	    if (pci && !(pci->attrFile & FILE_DIRECTORY)) {
	      p = strrchr(pci->pszFileName, '\\');
	      if (p) {
		p++;
		strcpy(dcd->mask.szMask, p);
	      }
	    }
	  }
	  *(dcd->mask.prompt) = 0;

	  if (WinDlgBox(HWND_DESKTOP, hwnd, PickMaskDlgProc,
			FM3ModHandle, MSK_FRAME, MPFROMP(&dcd->mask)))
	    WinSendMsg(hwnd, UM_FILTER, MPVOID, MPVOID);
	  else if (empty)
	    *dcd->mask.szMask = 0;
	  SayFilter(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				    DIR_FILTER), &dcd->mask, FALSE);
	}
	break;

      case IDM_UNHIDEALL:
	WinSendMsg(hwnd, CM_FILTER, MPFROMP(Filter), MPFROMP(&dcd->mask));
	break;

      case IDM_HIDEALL:
	if (fAutoView && hwndMain)
	  PostMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	dcd->suspendview = 1;
	HideAll(hwnd);
	dcd->suspendview = 0;
	PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	break;

      case IDM_SELECTBOTH:
      case IDM_SELECTONE:
      case IDM_SELECTMORE:
      case IDM_SELECTNEWER:
      case IDM_SELECTOLDER:
      case IDM_SELECTBIGGER:
      case IDM_SELECTSMALLER:
      case IDM_DESELECTBOTH:
      case IDM_DESELECTONE:
      case IDM_DESELECTMORE:
      case IDM_DESELECTNEWER:
      case IDM_DESELECTOLDER:
      case IDM_DESELECTBIGGER:
      case IDM_DESELECTSMALLER:
	if (ParentIsDesktop(hwnd, dcd->hwndParent)) {
	  Runtime_Error(pszSrcFile, __LINE__, "ParentIsDesktop unexpected");
	  break;
	}
      case IDM_SELECTLIST:
      case IDM_SELECTALL:
      case IDM_DESELECTALL:
      case IDM_SELECTALLFILES:
      case IDM_DESELECTALLFILES:
      case IDM_SELECTALLDIRS:
      case IDM_DESELECTALLDIRS:
      case IDM_SELECTMASK:
      case IDM_DESELECTMASK:
      case IDM_INVERT:
      case IDM_SELECTCLIP:
      case IDM_DESELECTCLIP:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if ((INT) pci == -1)
	    pci = NULL;
	  if (SHORT1FROMMP(mp1) == IDM_HIDEALL) {
	    if (pci) {
	      if (!(pci->rc.flRecordAttr & CRA_SELECTED))
		pci->rc.flRecordAttr |= CRA_FILTERED;
	      WinSendMsg(hwnd, CM_INVALIDATERECORD, MPFROMP(&pci),
			 MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
	      break;
	    }
	  }
	  PostMsg(dcd->hwndObject, UM_SELECT, mp1, MPFROMP(pci));
	}
	break;

      case IDM_RESCAN:
	dcd->stopflag++;
	if (!PostMsg(dcd->hwndObject, UM_RESCAN, MPVOID, MPVOID)) {
	  dcd->stopflag--;
	}
	break;

      case IDM_SHOWLNAMES:
      case IDM_SHOWSUBJECT:
      case IDM_SHOWEAS:
      case IDM_SHOWSIZE:
      case IDM_SHOWICON:
      case IDM_SHOWLWDATE:
      case IDM_SHOWLWTIME:
      case IDM_SHOWLADATE:
      case IDM_SHOWLATIME:
      case IDM_SHOWCRDATE:
      case IDM_SHOWCRTIME:
      case IDM_SHOWATTR:
	AdjustDetailsSwitches(hwnd,
			      dcd->hwndLastMenu,
			      SHORT1FROMMP(mp1),
			      dcd->directory, NULL, &dcd->ds, FALSE);
	break;

      case IDM_TREEVIEW:
      case IDM_ICON:
      case IDM_TEXT:
      case IDM_DETAILS:
      case IDM_NAME:
      case IDM_MINIICONS:
      case IDM_DETAILSTITLES:
	{
	  CNRINFO cnri;

	  memset(&cnri, 0, sizeof(CNRINFO));
	  cnri.cb = sizeof(CNRINFO);
	  WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnri),
		     MPFROMLONG(sizeof(CNRINFO)));
	  switch (SHORT1FROMMP(mp1)) {
	  case IDM_TREEVIEW:
	    if (!(cnri.flWindowAttr & CV_TREE))
	      dcd->lastattr = cnri.flWindowAttr;
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME | CA_TREELINE));
	    cnri.flWindowAttr |= CA_TREELINE | CV_TREE | CV_ICON;
	    if (!dcd->firsttree)
	      PostMsg(dcd->hwndObject, UM_FLESH, MPVOID, MPVOID);
	    break;
	  case IDM_ICON:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME | CA_TREELINE));
	    cnri.flWindowAttr |= CV_ICON;
	    break;
	  case IDM_NAME:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME | CA_TREELINE));
	    cnri.flWindowAttr |= CV_NAME;
	    break;
	  case IDM_TEXT:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME | CA_TREELINE));
	    cnri.flWindowAttr |= CV_TEXT;
	    break;
	  case IDM_DETAILS:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME | CA_TREELINE));
	    cnri.flWindowAttr |= CV_DETAIL;
	    break;
	  case IDM_MINIICONS:
	    if (cnri.flWindowAttr & CV_MINI)
	      cnri.flWindowAttr &= (~CV_MINI);
	    else
	      cnri.flWindowAttr |= CV_MINI;
	    break;
	  case IDM_DETAILSTITLES:
	    if (cnri.flWindowAttr & CA_DETAILSVIEWTITLES)
	      cnri.flWindowAttr &= (~CA_DETAILSVIEWTITLES);
	    else
	      cnri.flWindowAttr |= CA_DETAILSVIEWTITLES;
	    break;
	  }
	  cnri.flWindowAttr &= (~(CA_ORDEREDTARGETEMPH | CA_MIXEDTARGETEMPH));
	  cnri.flWindowAttr |= CV_FLOW;
	  dcd->flWindowAttr = cnri.flWindowAttr;
	  WinSendMsg(hwnd, CM_SETCNRINFO, MPFROMP(&cnri),
		     MPFROMLONG(CMA_FLWINDOWATTR));
	  WinSendMsg(hwnd, CM_INVALIDATERECORD, MPVOID,
		     MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
	  SayView(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				  DIR_VIEW), dcd->flWindowAttr);
	}
	break;

      case IDM_SAVETOLIST:
	WinDlgBox(HWND_DESKTOP, hwnd, SaveListDlgProc, FM3ModHandle,
		  SAV_FRAME, MPFROMP(&hwnd));
	break;

      case IDM_SIZES:
	{
	  PCNRITEM pci;
	  CHAR path[CCHMAXPATH];

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1)
	    strcpy(path, pci->pszFileName);
	  else
	    strcpy(path, dcd->directory);
	  MakeValidDir(path);
	  WinDlgBox(HWND_DESKTOP,
		    HWND_DESKTOP, DirSizeProc, FM3ModHandle, DSZ_FRAME, path);
	}
	break;

      case IDM_MKDIR:
	{
	  PCNRITEM pci;
	  BOOL saved;

	  saved = fSelectedAlways;
	  fSelectedAlways = FALSE;
	  pci = (PCNRITEM)CurrentRecord(hwnd);
	  // 01 Oct 07 SHL Make below selected directory or in current directory
	  PMMkDir(dcd->hwndParent,
		  pci && (INT)pci != -1 ? pci->pszFileName : dcd->directory,
		  FALSE);
	  fSelectedAlways = saved;
	}
	break;

      case IDM_SWITCH:
	if (mp2) {
	  strcpy(dcd->previous, dcd->directory);
	  strcpy(dcd->directory, (CHAR *)mp2);
	  dcd->stopflag++;
	  if (!PostMsg(dcd->hwndObject, UM_RESCAN, MPVOID, MPFROMLONG(1L))) {
	    strcpy(dcd->directory, dcd->previous);
	    dcd->stopflag--;
	  }
	  else if (*dcd->directory) {
	    if (hwndMain)
	      WinSendMsg(hwndMain,
			 UM_SETUSERLISTNAME, MPFROMP(dcd->directory), MPVOID);
	    else
	      add_udir(FALSE, dcd->directory);
	  }
	}
	break;

      case IDM_PARENT:
	{
	  CHAR tempname1[CCHMAXPATH], tempname2[CCHMAXPATH];

	  strcpy(tempname1, dcd->directory);
	  AddBackslashToPath(tempname1);
	  strcat(tempname1, "..");
	  DosError(FERR_DISABLEHARDERR);
	  if (!DosQueryPathInfo(tempname1,
				FIL_QUERYFULLNAME,
				tempname2, sizeof(tempname2))) {
	    if (stricmp(dcd->directory, tempname2)) {
	      strcpy(dcd->previous, dcd->directory);
	      strcpy(dcd->directory, tempname2);
	      dcd->stopflag++;
	      if (!PostMsg(dcd->hwndObject,
			   UM_RESCAN, MPVOID, MPFROMLONG(1L))) {
		strcpy(dcd->directory, dcd->previous);
		dcd->stopflag--;
	      }
	      else if (*dcd->directory) {
		if (hwndMain)
		  WinSendMsg(hwndMain,
			     UM_SETUSERLISTNAME,
			     MPFROMP(dcd->directory), MPVOID);
		else
		  add_udir(FALSE, dcd->directory);
	      }
	    }
	  }
	}
	break;

      case IDM_PREVIOUS:
	if (*dcd->previous && stricmp(dcd->directory, dcd->previous)) {

	  CHAR tempname[CCHMAXPATH];

	  if (IsValidDir(dcd->previous)) {
	    strcpy(tempname, dcd->directory);
	    strcpy(dcd->directory, dcd->previous);
	    strcpy(dcd->previous, tempname);
	    dcd->stopflag++;
	    if (!PostMsg(dcd->hwndObject, UM_RESCAN, MPVOID, MPFROMLONG(1L))) {
	      strcpy(dcd->directory, dcd->previous);
	      dcd->stopflag--;
	    }
	    else if (*dcd->directory) {
	      if (hwndMain)
		WinSendMsg(hwndMain,
			   UM_SETUSERLISTNAME,
			   MPFROMP(dcd->directory), MPVOID);
	      else
		add_udir(FALSE, dcd->directory);
	    }
	  }
	  else
	    *dcd->previous = 0;
	}
	break;

      case IDM_WALKDIR:
	{
	  CHAR newdir[CCHMAXPATH];

	  strcpy(newdir, dcd->directory);
	  if (!WinDlgBox(HWND_DESKTOP,
			 dcd->hwndParent,
			 WalkAllDlgProc,
			 FM3ModHandle,
			 WALK_FRAME, MPFROMP(newdir)) || !*newdir)
	    break;
	  if (stricmp(newdir, dcd->directory)) {
	    strcpy(dcd->previous, dcd->directory);
	    strcpy(dcd->directory, newdir);
	    dcd->stopflag++;
	    if (!PostMsg(dcd->hwndObject, UM_RESCAN, MPVOID, MPFROMLONG(1L))) {
	      strcpy(dcd->directory, dcd->previous);
	      dcd->stopflag--;
	    }
	    else if (*dcd->directory) {
	      if (hwndMain)
		WinSendMsg(hwndMain,
			   UM_SETUSERLISTNAME,
			   MPFROMP(dcd->directory), MPVOID);
	      else
		add_udir(FALSE, dcd->directory);
	    }
	  }
	}
	break;

      case IDM_OPENICONME:
	OpenObject(dcd->directory, PCSZ_ICON, dcd->hwndFrame);
	break;
      case IDM_OPENDETAILSME:
	OpenObject(dcd->directory, Details, dcd->hwndFrame);
	break;
      case IDM_OPENTREEME:
	OpenObject(dcd->directory, PCSZ_TREE, dcd->hwndFrame);
	break;
      case IDM_OPENSETTINGSME:
	OpenObject(dcd->directory, Settings, dcd->hwndFrame);
	break;

      case IDM_DOITYOURSELF:
      case IDM_UPDATE:
      case IDM_OPENWINDOW:
      case IDM_OPENSETTINGS:
      case IDM_OPENDEFAULT:
      case IDM_OPENICON:
      case IDM_OPENDETAILS:
      case IDM_OPENTREE:
      case IDM_OBJECT:
      case IDM_SHADOW:
      case IDM_SHADOW2:
      case IDM_JAVAEXE:
      case IDM_DELETE:
      case IDM_PERMDELETE:
      case IDM_PRINT:
      case IDM_ATTRS:
      case IDM_INFO:
      case IDM_COPY:
      case IDM_MOVE:
      case IDM_WPSMOVE:
      case IDM_WPSCOPY:
      case IDM_WILDCOPY:
      case IDM_WILDMOVE:
      case IDM_RENAME:
      case IDM_COMPARE:
      case IDM_EAS:
      case IDM_SUBJECT:
      case IDM_VIEW:
      case IDM_VIEWTEXT:
      case IDM_VIEWBINARY:
      case IDM_VIEWARCHIVE:
      case IDM_EDIT:
      case IDM_EDITTEXT:
      case IDM_EDITBINARY:
      case IDM_SAVETOCLIP:
      case IDM_SAVETOCLIPFILENAME:
      case IDM_APPENDTOCLIP:
      case IDM_APPENDTOCLIPFILENAME:
      case IDM_ARCHIVE:
      case IDM_ARCHIVEM:
      case IDM_EXTRACT:
      case IDM_MCIPLAY:
      case IDM_COLLECTFROMFILE:
      case IDM_UUDECODE:
      case IDM_UNLOCKFILE:
      case IDM_MERGE:
	{
	  LISTINFO *li;
	  ULONG action = UM_ACTION;
	  li = xmallocz(sizeof(LISTINFO), pszSrcFile, __LINE__);
	  if (li) {
	    li->type = SHORT1FROMMP(mp1);
	    li->hwnd = hwnd;
	    li->list = BuildList(hwnd);
	    if (li->type == IDM_DELETE)
	      ignorereadonly = FALSE;
	    switch (SHORT1FROMMP(mp1)) {
	    case IDM_WILDMOVE:
	    case IDM_WILDCOPY:
	    case IDM_MOVE:
	    case IDM_COPY:
	    case IDM_WPSMOVE:
	    case IDM_WPSCOPY:
	      break;
	    default:
	      strcpy(li->targetpath, dcd->directory);
	      break;
	    }
	    if (li->list) {
	      if (SHORT1FROMMP(mp1) == IDM_COLLECTFROMFILE) {
		if (!Collector) {

		  HWND hwndC;
		  SWP swp;

		  if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
		      !fAutoTile &&
		      (!fExternalCollector && !strcmp(realappname, FM3Str)))
		    GetNextWindowPos(dcd->hwndParent, &swp, NULL, NULL);
		  hwndC = StartCollector((fExternalCollector ||
					  strcmp(realappname, FM3Str)) ?
					 HWND_DESKTOP : dcd->hwndParent, 4);
		  if (hwndC) {
		    if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
			!fAutoTile && (!fExternalCollector &&
				       !strcmp(realappname, FM3Str)))
		      WinSetWindowPos(hwndC, HWND_TOP, swp.x, swp.y,
				      swp.cx, swp.cy, SWP_MOVE | SWP_SIZE |
				      SWP_SHOW | SWP_ZORDER);
		    else if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
			     fAutoTile && !strcmp(realappname, FM3Str))
		      TileChildren(dcd->hwndParent, TRUE);
		    WinSetWindowPos(hwndC, HWND_TOP, 0, 0, 0, 0,
				    SWP_ACTIVATE);
		    DosSleep(100);
		  }
		}
		else
		  StartCollector(dcd->hwndParent, 4);
	      }
	      switch (SHORT1FROMMP(mp1)) {
	      case IDM_APPENDTOCLIP:
	      case IDM_APPENDTOCLIPFILENAME:
	      case IDM_SAVETOCLIP:
	      case IDM_SAVETOCLIPFILENAME:
	      case IDM_ARCHIVE:
	      case IDM_ARCHIVEM:
	      case IDM_DELETE:
	      case IDM_PERMDELETE:
	      case IDM_ATTRS:
	      case IDM_PRINT:
	      case IDM_SHADOW:
	      case IDM_SHADOW2:
	      case IDM_JAVAEXE:
	      case IDM_OBJECT:
	      case IDM_VIEW:
	      case IDM_VIEWTEXT:
	      case IDM_VIEWBINARY:
	      case IDM_EDIT:
	      case IDM_EDITTEXT:
	      case IDM_EDITBINARY:
	      case IDM_MCIPLAY:
	      case IDM_UPDATE:
	      case IDM_DOITYOURSELF:
	      case IDM_INFO:
	      case IDM_EAS:
		action = UM_MASSACTION;
		break;
	      }
	      if (li->type == IDM_DELETE)
		ignorereadonly = FALSE;
	      if (SHORT1FROMMP(mp1) == IDM_OBJECT ||
		  SHORT1FROMMP(mp1) == IDM_SHADOW ||
		  SHORT1FROMMP(mp1) == IDM_SHADOW2)
		*li->targetpath = 0;
	      if (!PostMsg(dcd->hwndObject, action, MPFROMP(li), MPVOID)) {
		Runtime_Error(pszSrcFile, __LINE__, PCSZ_POSTMSG);
		FreeListInfo(li);
	      }
	      else if (fUnHilite)
		UnHilite(hwnd, TRUE, &dcd->lastselection, 0);
	    }
	    else
	      free(li);
	  }
	}
	break;

      case IDM_DRIVESMENU:
	if (!hwndMain)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_WALKDIR, 0), MPVOID);
	break;

      default:
	if (SwitchCommand(dcd->hwndLastMenu, SHORT1FROMMP(mp1)))
	  return 0;
	else {
	  if (SHORT1FROMMP(mp1) >= IDM_COMMANDSTART &&
	      SHORT1FROMMP(mp1) < IDM_QUICKTOOLSTART) {

	    register INT x;

	    if (!cmdloaded)
	      load_commands();
	    x = SHORT1FROMMP(mp1);
	    if (x >= 0) {
	      RunCommand(hwnd, x);
	      if (fUnHilite)
		UnHilite(hwnd, TRUE, &dcd->lastselection, 0);
	    }
	  }
	}
	break;
      }
    }
    return 0;

  case UM_FIXCNRMLE:
  case UM_FIXEDITNAME:
    return CommonCnrProc(hwnd, msg, mp1, mp2);

  case UM_FILESMENU:
    {
      PCNRITEM pci;
      HWND menuHwnd = (HWND) 0;

      pci = (PCNRITEM) CurrentRecord(hwnd);
      if (pci && (INT) pci != -1) {
	if (pci->attrFile & FILE_DIRECTORY) {
	  menuHwnd = CheckMenu(hwndMainMenu, &DirMenu, DIR_POPUP);
	}
	else
	  menuHwnd = CheckMenu(hwndMainMenu, &FileMenu, FILE_POPUP);
      }
      return MRFROMLONG(menuHwnd);
    }

  case WM_CONTROL:
    DosError(FERR_DISABLEHARDERR);
    if (dcd) {
      switch (SHORT2FROMMP(mp1)) {
      case CN_COLLAPSETREE:
      case CN_EXPANDTREE:
	{
	  PCNRITEM pci = (PCNRITEM) mp2;

	  if (pci && (INT) pci != -1 && !(pci->flags & RECFLAGS_ENV)) {
	    if (driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_REMOVABLE) {
	      struct
	      {
		ULONG serial;
		CHAR volumelength;
		CHAR volumelabel[CCHMAXPATH];
	      }
	      volser;
	      APIRET rc;

	      memset(&volser, 0, sizeof(volser));
	      DosError(FERR_DISABLEHARDERR);
	      // fixme?
	      rc = DosQueryFSInfo(toupper(*pci->pszFileName) - '@',
				  FSIL_VOLSER, &volser, sizeof(volser));
	      if (rc) {
		Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
			  GetPString(IDS_CANTFINDDIRTEXT),
			  pci->pszFileName);
		if (!fErrorBeepOff)
		  DosBeep(250,100);
		driveserial[toupper(*pci->pszFileName) - 'A'] = -1;
		AddFleshWorkRequest(hwnd, pci, eUnFlesh);
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	      }
	      else {
		if (SHORT2FROMMP(mp1) == CN_COLLAPSETREE &&
		    !volser.serial ||
		    driveserial[toupper(*pci->pszFileName) - 'A'] !=
		    volser.serial)
		  AddFleshWorkRequest(hwnd, pci, eUnFlesh);
		if (SHORT2FROMMP(mp1) != CN_COLLAPSETREE ||
		    (!volser.serial ||
		     driveserial[toupper(*pci->pszFileName) - 'A'] !=
		     volser.serial)) {
                  // 2015-08-07 SHL FIXME to wait for Flesh to finish before PostMsg
                  // 2015-09-26 GKY Waits (WaitFleshWorkListEmpty) in object window
		  if (SHORT2FROMMP(mp1) == CN_EXPANDTREE && AddFleshWorkRequest(hwnd, pci, eFlesh) &&
		      !dcd->suspendview && fTopDir) {
		    PostMsg(dcd->hwndObject, UM_TOPDIR, MPFROMP(pci), MPVOID);
		  }
		}
		driveserial[toupper(*pci->pszFileName) - 'A'] = volser.serial;
	      }
	    }
            else if (SHORT2FROMMP(mp1) == CN_EXPANDTREE) {
              if (AddFleshWorkRequest(hwnd, pci, eFlesh) && !dcd->suspendview &&
                  fTopDir) {
                // 2015-08-07 SHL FIXME to wait for Flesh to finish before PostMsg
                // 2015-09-26 GKY Waits (WaitFleshWorkListEmpty) in object window
		PostMsg(dcd->hwndObject, UM_TOPDIR, MPFROMP(pci), MPVOID);
	      }
	    }
	    if (SHORT2FROMMP(mp1) == CN_EXPANDTREE && !dcd->suspendview)
	      WinSendMsg(hwnd, UM_FILTER, MPVOID, MPVOID);

	  }
	}
	break;

      case CN_CONTEXTMENU:
	{
	  PCNRITEM pci = (PCNRITEM) mp2;

	  if (pci) {
	    WinSendMsg(hwnd,
		       CM_SETRECORDEMPHASIS,
		       MPFROMP(pci), MPFROM2SHORT(TRUE, CRA_CURSORED));
	    MarkAll(hwnd, FALSE, FALSE, TRUE);
	    if (pci->attrFile & FILE_DIRECTORY)
	      dcd->hwndLastMenu = CheckMenu(hwndMainMenu, &DirMenu, DIR_POPUP);
	    else
	      dcd->hwndLastMenu = CheckMenu(hwndMainMenu, &FileMenu, FILE_POPUP);
	  }
	  else {
	    dcd->hwndLastMenu = CheckMenu(hwnd, &DirCnrMenu, DIRCNR_POPUP);
	    if (dcd->hwndLastMenu && !dcd->cnremphasized) {
	      WinSendMsg(hwnd,
			 CM_SETRECORDEMPHASIS,
			 MPVOID, MPFROM2SHORT(TRUE, CRA_SOURCE));
	      dcd->cnremphasized = TRUE;
	    }
	  }
	  if (dcd->hwndLastMenu) {
	    if (dcd->hwndLastMenu == DirCnrMenu) {
	      if (dcd->flWindowAttr & CV_MINI)
		WinCheckMenuItem(dcd->hwndLastMenu, IDM_MINIICONS, TRUE);
	    }
	    if (dcd->hwndLastMenu == DirMenu)
	      WinEnableMenuItem(DirMenu, IDM_TREE, TRUE);
	    if (!PopupMenu(hwnd, hwnd, dcd->hwndLastMenu)) {
	      if (dcd->cnremphasized) {
		WinSendMsg(hwnd,
			   CM_SETRECORDEMPHASIS,
			   MPVOID, MPFROM2SHORT(FALSE, CRA_SOURCE));
		dcd->cnremphasized = TRUE;
	      }
	      MarkAll(hwnd, TRUE, FALSE, TRUE);
	    }
	  }
	}
	break;

      case CN_DROPHELP:
	if (mp2) {

	  PDRAGINFO pDInfo;
	  PCNRITEM pci;
	  ULONG numitems;
	  USHORT usOperation;

	  pci = (PCNRITEM) ((PCNRDRAGINFO) mp2)->pRecord;
	  pDInfo = (PDRAGINFO) ((PCNRDRAGINFO) mp2)->pDragInfo;
	  if (!DrgAccessDraginfo(pDInfo)) {
	    Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      GetPString(IDS_DROPERRORTEXT));
	  }
	  else {
	    numitems = DrgQueryDragitemCount(pDInfo);
	    usOperation = pDInfo->usOperation;
	    if (usOperation == DO_DEFAULT)
	      usOperation = fCopyDefault ? DO_COPY : DO_MOVE;
	    FreeDragInfoData(hwnd, pDInfo);
	    saymsg(MB_ENTER | MB_ICONASTERISK,
		   hwnd,
		   GetPString(IDS_DROPHELPHDRTEXT),
		   GetPString(IDS_DROPHELPTEXT),
		   numitems,
		   &"s"[numitems == 1L],
		   pci ? NullStr : GetPString(IDS_NOTEXT),
		   pci ? NullStr : " ",
		   pci ? pci->pszFileName : NullStr,
		   pci ? " " : NullStr,
		   GetPString((usOperation == DO_MOVE) ?
			      IDS_MOVETEXT :
			      (usOperation == DO_LINK) ?
			      IDS_LINKTEXT : IDS_COPYTEXT));
	  }
	}
	return 0;

      case CN_DRAGLEAVE:
	return 0;

      case CN_DRAGAFTER:
      case CN_DRAGOVER:
	if (mp2) {

	  PDRAGITEM pDItem;	// Pointer to DRAGITEM
	  PDRAGINFO pDInfo;	// Pointer to DRAGINFO
	  PCNRITEM pci;
	  USHORT uso;

	  pci = (PCNRITEM) ((PCNRDRAGINFO) mp2)->pRecord;
	  pDInfo = ((PCNRDRAGINFO) mp2)->pDragInfo;
	  if (!DrgAccessDraginfo(pDInfo)) {
	    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		      PCSZ_DRGACCESSDRAGINFO);
	      return (MRFROM2SHORT(DOR_NEVERDROP, 0));
	  }
	  if (*dcd->directory &&
	     (driveflags[toupper(*dcd->directory) - 'A'] &
	      DRIVE_NOTWRITEABLE)) {
	    DrgFreeDraginfo(pDInfo);
	    return MRFROM2SHORT(DOR_DROP,	// Return okay to link
				DO_LINK);	// (compare) only
	  }
	  if (pci) {
	    if (pci->rc.flRecordAttr & CRA_SOURCE) {
	      DrgFreeDraginfo(pDInfo);
	      return (MRFROM2SHORT(DOR_NODROP, 0));
	    }
	    uso = pDInfo->usOperation;
	    if (uso == DO_DEFAULT)
	      uso = fCopyDefault ? DO_COPY : DO_MOVE;
	    if (!(pci->attrFile & FILE_DIRECTORY)) {
	      if (uso != DO_LINK && uso != DO_COPY && uso != DO_MOVE) {
		DrgFreeDraginfo(pDInfo);
		return MRFROM2SHORT(DOR_NODROP, 0);
	      }
	      if (uso != DO_LINK &&
		  !(driveflags[toupper(*pci->pszFileName) - 'A'] &
		    DRIVE_NOTWRITEABLE)) {

		ARC_TYPE *info = NULL;

		if (!fQuickArcFind &&
		    !(driveflags[toupper(*pci->pszFileName) - 'A'] &
		      DRIVE_SLOW))
		  info = find_type(pci->pszFileName, NULL);
		else
		  info = quick_find_type(pci->pszFileName, NULL);
		if (!info || ((uso == DO_MOVE && !info->move) ||
			      (uso == DO_COPY && !info->create))) {
		  DrgFreeDraginfo(pDInfo);
		  return MRFROM2SHORT(DOR_NODROP, 0);
		}
	      }
	    }
	  }

	  /**
	   * Access DRAGITEM index to DRAGITEM
	   * Check valid rendering mechanisms and data
	   */
	  pDItem = DrgQueryDragitemPtr(pDInfo, 0);
	  if (DrgVerifyRMF(pDItem, (CHAR *) DRM_OS2FILE, NULL) ||
	      ((!pci || (pci->attrFile & FILE_DIRECTORY)) &&
	       DrgVerifyRMF(pDItem, (CHAR *) DRM_FM2ARCMEMBER, (CHAR *) DRF_FM2ARCHIVE))) {
	    DrgFreeDraginfo(pDInfo);
	    if (driveflags[toupper(*dcd->directory) - 'A'] &
		DRIVE_NOTWRITEABLE)
	      return MRFROM2SHORT(DOR_DROP, DO_LINK);
	    if (toupper(*dcd->directory) < 'C')
	      return MRFROM2SHORT(DOR_DROP, DO_COPY);
	    return MRFROM2SHORT(DOR_DROP,	// Return okay to drop
				((fCopyDefault) ? DO_COPY : DO_MOVE));
	  }
	  DrgFreeDraginfo(pDInfo);	// Free DRAGINFO
	}
	return MRFROM2SHORT(DOR_NODROP, 0);	// Drop not valid

      case CN_INITDRAG:
	{
	  BOOL wasemphasized = FALSE;
	  PCNRDRAGINIT pcd = (PCNRDRAGINIT) mp2;
	  PCNRITEM pci;

	  if (pcd) {
	    pci = (PCNRITEM) pcd->pRecord;
	    if (pci) {
	      if ((INT) pci == -1)
		pci = NULL;
	      else if (pci->rc.flRecordAttr & CRA_SELECTED)
		wasemphasized = TRUE;
	    }
	    else if (!*dcd->directory) {
	      Runtime_Error(pszSrcFile, __LINE__, NULL);
	      break;
	    }
	    else if (IsRoot(dcd->directory)) {
		saymsg(MB_ENTER, hwnd, GetPString(IDS_ERRORTEXT),
		       GetPString(IDS_CANTDRAGROOTDIR));
	      break;
	    }
	    if (hwndStatus2) {
	      if (pci)
		WinSetWindowText(hwndStatus2, (CHAR *) GetPString(IDS_DRAGFILEOBJTEXT));
	      else
		WinSetWindowText(hwndStatus2, (CHAR *) GetPString(IDS_DRAGDIRTEXT));
	    }
	    if (DoFileDrag(hwnd,
			   dcd->hwndObject,
			   mp2,
			   NULL,
			   pci ? NULL : dcd->directory,
			   pci ? TRUE : FALSE)) {
	      if ((pci && fUnHilite && wasemphasized) || dcd->ulItemsToUnHilite) {
		UnHilite(hwnd, TRUE, &dcd->lastselection, dcd->ulItemsToUnHilite);
	      }
	    }
	    if (hwndStatus2) {
	      WinSetFocus(HWND_DESKTOP, hwnd);
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	  }
	}
	return 0;

      case CN_DROP:
	if (mp2) {

	  LISTINFO *li;
	  ULONG action = UM_ACTION;

	  li = DoFileDrop(hwnd, dcd->directory, TRUE, mp1, mp2);
	  CheckPmDrgLimit(((PCNRDRAGINFO)mp2)->pDragInfo);
	  if (li) {
	    if (li->list && li->list[0] && IsRoot(li->list[0]))
	      li->type = DO_LINK;
	    else if (fDragndropDlg && (!*li->arcname || !li->info)) {

	      CHECKLIST cl;

	      memset(&cl, 0, sizeof(cl));
	      cl.size = sizeof(cl);
	      cl.flags = li->type;
	      cl.list = li->list;
	      cl.cmd = li->type;
	      cl.prompt = li->targetpath;
	      li->type = WinDlgBox(HWND_DESKTOP, dcd->hwndParent,
				   DropListProc, FM3ModHandle,
				   DND_FRAME, MPFROMP(&cl));
	      if (li->type == DID_ERROR)
		  Win_Error(DND_FRAME, HWND_DESKTOP, pszSrcFile, __LINE__,
			    GetPString(IDS_DRAGDROPDIALOGTEXT));
	      if (!li->type) {
		FreeListInfo(li);
		return 0;
	      }
	      li->list = cl.list;
	      if (!li->list || !li->list[0]) {
		FreeListInfo(li);
		return 0;
	      }
	    }
	    switch (li->type) {
	    case DND_LAUNCH:
	      strcat(li->targetpath, " %a");
	      ExecOnList(dcd->hwndParent, li->targetpath,
			 PROMPT | WINDOWED, NULL, NULL, li->list, NULL,
			 pszSrcFile, __LINE__);
	      FreeList(li->list);
	      li->list = NULL;
	      break;
	    case DO_LINK:
	      if (fLinkSetsIcon) {
		li->type = IDM_SETICON;
		action = UM_MASSACTION;
	      }
	      else
		li->type = IDM_COMPARE;
	      break;
	    case DND_EXTRACT:
	      if (*li->targetpath && !IsFile(li->targetpath))
		li->type = IDM_EXTRACT;
	      break;
	    case DND_MOVE:
	      li->type = IDM_MOVE;
	      if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = IDM_ARCHIVEM;
	      }
	      break;
	    case DND_WILDMOVE:
	      li->type = IDM_WILDMOVE;
	      if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = IDM_ARCHIVEM;
	      }
	      break;
	    case DND_OBJECT:
	      li->type = IDM_OBJECT;
	      action = UM_MASSACTION;
	      break;
	    case DND_SHADOW:
	      li->type = IDM_SHADOW;
	      action = UM_MASSACTION;
	      break;
	    case DND_COMPARE:
	      li->type = IDM_COMPARE;
	      break;
	    case DND_SETICON:
	      action = UM_MASSACTION;
	      li->type = IDM_SETICON;
	      break;
	    case DND_COPY:
	      li->type = IDM_COPY;
	      if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = IDM_ARCHIVE;
	      }
	      break;
	    case DND_WILDCOPY:
	      li->type = IDM_WILDCOPY;
	      if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = IDM_ARCHIVE;
	      }
	      break;
	    default:
	      if (*li->arcname && li->info) {
		action = UM_MASSACTION;
		li->type =
		  (li->type == DO_MOVE) ? IDM_FAKEEXTRACTM : IDM_FAKEEXTRACT;
	      }
	      else if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = (li->type == DO_MOVE) ? IDM_ARCHIVEM : IDM_ARCHIVE;
	      }
	      else
		li->type = (li->type == DO_MOVE) ? IDM_MOVE : IDM_COPY;
	      break;
	    }
	    if (!li->list || !li->list[0])
	      FreeListInfo(li);
	    else if (!PostMsg(dcd->hwndObject, action, MPFROMP(li), MPVOID))
	      FreeListInfo(li);
	    else {

	      USHORT usop = 0;

	      switch (li->type) {
	      case IDM_COPY:
	      case IDM_WILDCOPY:
		usop = DO_COPY;
		break;
	      case IDM_MOVE:
	      case IDM_WILDMOVE:
	      case IDM_ARCHIVEM:
		usop = DO_MOVE;
		break;
	      }
	      if (usop)
		return MRFROM2SHORT(DOR_DROP, usop);
	    }
	  }
	}
	return 0;

      case CN_ENDEDIT:
      case CN_BEGINEDIT:
	{
	  PFIELDINFO pfi = ((PCNREDITDATA) mp2)->pFieldInfo;
	  PCNRITEM pci = (PCNRITEM) ((PCNREDITDATA) mp2)->pRecord;

	  if (pfi || pci) {

	    MRESULT mre;

	    mre = CnrDirectEdit(hwnd, msg, mp1, mp2);
	    if (mre != (MRESULT) - 1)
	      return mre;
	  }
	  else if (!pfi && !pci)
	    PostMsg(hwnd, UM_FIXCNRMLE, MPFROMLONG(CCHMAXPATH), MPVOID);
	}
	return 0;

      case CN_REALLOCPSZ:
	{
	  PFIELDINFO pfi = ((PCNREDITDATA) mp2)->pFieldInfo;
	  PCNRITEM pci = (PCNRITEM) ((PCNREDITDATA) mp2)->pRecord;
	  HWND hwndMLE;
	  CHAR testname[CCHMAXPATH];

	  if (!pci && !pfi) {
	    hwndMLE = WinWindowFromID(hwnd, CID_MLE);
	    WinQueryWindowText(hwndMLE, sizeof(s), s);
	    chop_at_crnl(s);
	    bstrip(s);
	    if (*s) {
	      if (!DosQueryPathInfo(s,
				    FIL_QUERYFULLNAME,
				    testname, sizeof(testname))) {
		if (!SetDir(dcd->hwndParent, hwnd, testname, 1)) {
		  PostMsg(hwnd, UM_SETDIR, MPFROMP(testname), MPVOID);
		}
	      }
	    }
	  }
	  else {

	    MRESULT mre;

	    mre = CnrDirectEdit(hwnd, msg, mp1, mp2);
	    if (mre != (MRESULT) - 1)
	      return mre;
	  }
	}
	return 0;

      case CN_EMPHASIS:
	if (!mp2)
	  Runtime_Error(pszSrcFile, __LINE__, "mp2 NULL");
	else {
	  PNOTIFYRECORDEMPHASIS pre = mp2;
	  PCNRITEM pci;
	  CHAR s[CCHMAXPATHCOMP + 91];

	  pci = (PCNRITEM) (pre ? pre->pRecord : NULL);
	  if (!pci) {
	    if (hwndStatus2)
	      WinSetWindowText(hwndStatus2, NullStr);
	    if (fMoreButtons) {
	      WinSetWindowText(hwndName, NullStr);
	      WinSetWindowText(hwndDate, NullStr);
	      WinSetWindowText(hwndAttr, NullStr);
	    }
	    if (hwndMain)
	      WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	    break;
	  }
	  if (pre->fEmphasisMask & CRA_SELECTED) {
	    if (pci->rc.flRecordAttr & CRA_SELECTED) {
	      dcd->selectedbytes += (pci->cbFile + pci->easize);
	      dcd->selectedfiles++;
	    }
	    else if (dcd->selectedfiles) {
	      dcd->selectedbytes -= (pci->cbFile + pci->easize);
	      dcd->selectedfiles--;
	    }
	    if (!dcd->suspendview) {
	      commafmt(tf, sizeof(tf), dcd->selectedfiles);
	      CommaFmtULL(tb, sizeof(tb), dcd->selectedbytes, 'K');
	      sprintf(s, "%s / %s", tf, tb);
	      WinSetDlgItemText(dcd->hwndClient, DIR_SELECTED, s);
	    }
	  }
	  if (!dcd->suspendview && hwndMain && pci->pszFileName != NullStr &&
	      (pre->fEmphasisMask & CRA_CURSORED) &&
	      (pci->rc.flRecordAttr & CRA_CURSORED) &&
	      WinQueryActiveWindow(dcd->hwndParent) == dcd->hwndFrame) {
	    if (driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_SLOW)
	      WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	    else
	      WinSendMsg(hwndMain,
			 UM_LOADFILE, MPFROMP(pci->pszFileName), MPVOID);
	  }
	  if (!dcd->suspendview &&
	      WinQueryActiveWindow(dcd->hwndParent) == dcd->hwndFrame) {
	    if (pre->fEmphasisMask & CRA_CURSORED) {
	      if (pci->rc.flRecordAttr & CRA_CURSORED) {
		if (fSplitStatus && hwndStatus2) {
		  CommaFmtULL(tb, sizeof(tb), pci->cbFile + pci->easize, ' ');
		  if (!fMoreButtons) {
		    CHAR date[11];

		    DateFormat(date, pci->date);
		    sprintf(s, " %s  %s %02u%s%02u%s%02u  [%s]  %s",
			    tb, date, pci->time.hours, TimeSeparator,
			    pci->time.minutes, TimeSeparator, pci->time.seconds,
			    pci->pszDispAttr, pci->pszFileName);
		  }
		  else {
		    *tf = 0;
		    if (pci->cbFile + pci->easize > 1024) {
		      CommaFmtULL(tf, sizeof(tf),
				  pci->cbFile + pci->easize, 'K');
		    }
		    sprintf(s, GetPString(IDS_STATUSSIZETEXT),
			    tb,
			    *tf ? " (" : NullStr, tf, *tf ? ")" : NullStr);
		  }
		  WinSetWindowText(hwndStatus2, s);
		}
		if (fMoreButtons) {
		  CHAR szDate[DATE_BUF_BYTES];

		  WinSetWindowText(hwndName, pci->pszFileName);
		  DateFormat(szDate, pci->date);
		  sprintf(s, "%s %02u%s%02u%s%02u",
			  szDate, pci->time.hours, TimeSeparator, pci->time.minutes,
			  TimeSeparator, pci->time.seconds);
		  WinSetWindowText(hwndDate, s);
		  WinSetWindowText(hwndAttr, pci->pszDispAttr);
		}
	      }
	    }
	  }
	}
	break;

      case CN_ENTER:
	if (mp2) {

	  PCNRITEM pci = (PCNRITEM) ((PNOTIFYRECORDENTER) mp2)->pRecord;
	  FILEFINDBUF3 ffb;
	  HDIR hDir = HDIR_CREATE;
	  ULONG nm = 1;
	  APIRET status = 0;

	  SetShiftState();
	  if (pci) {
	    if (pci->rc.flRecordAttr & CRA_INUSE)
	      break;
	    DosError(FERR_DISABLEHARDERR);
	    status = DosFindFirst(pci->pszFileName,
				   &hDir,
				   FILE_NORMAL | FILE_DIRECTORY |
				   FILE_ARCHIVED | FILE_READONLY |
				   FILE_HIDDEN | FILE_SYSTEM,
				   &ffb, sizeof(ffb), &nm, FIL_STANDARD);
	    priority_bumped();
	    if (!status) {
	      DosFindClose(hDir);
	      if (ffb.attrFile & FILE_DIRECTORY) {
		if ((shiftstate & (KC_CTRL | KC_ALT)) == (KC_CTRL | KC_ALT))
		  PostMsg(hwnd,
			  WM_COMMAND,
			  MPFROM2SHORT(IDM_SHOWALLFILES, 0), MPVOID);
		else if ((shiftstate & (KC_CTRL | KC_SHIFT)) ==
			 (KC_CTRL | KC_SHIFT))
		  OpenObject(pci->pszFileName, Settings, dcd->hwndFrame);
		else if (shiftstate & KC_CTRL)
		  OpenObject(pci->pszFileName, Default, dcd->hwndFrame);
		else if (shiftstate & KC_SHIFT) {

		  HWND hwndDir;

		  hwndDir = OpenDirCnr((HWND) 0,
				       dcd->hwndParent,
				       dcd->hwndFrame,
				       FALSE, pci->pszFileName);
		  if (hwndDir) {
		    if (fMinOnOpen)
		      WinSetWindowPos(dcd->hwndFrame,
				      HWND_BOTTOM,
				      0, 0, 0, 0, SWP_MINIMIZE | SWP_ZORDER);
		    if (fAutoTile)
		      TileChildren(dcd->hwndParent, TRUE);
		    WinSetWindowPos(hwndDir,
				    HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE);
		  }
		}
		else {
		  strcpy(dcd->previous, dcd->directory);
		  strcpy(dcd->directory, pci->pszFileName);
		  dcd->stopflag++;
		  if (!PostMsg(dcd->hwndObject,
			       UM_RESCAN, MPVOID, MPFROMLONG(1))) {
		    dcd->stopflag--;
		  }
		  else if (*dcd->directory) {
		    if (hwndMain)
		      WinSendMsg(hwndMain,
				 UM_SETUSERLISTNAME,
				 MPFROMP(dcd->directory), MPVOID);
		    else
		      add_udir(FALSE, dcd->directory);
		  }
		}
	      }
	      else {

		SWP swp;

		WinQueryWindowPos(dcd->hwndFrame, &swp);
		WinSendMsg(hwnd,
			   CM_SETRECORDEMPHASIS,
			   MPFROMP(pci), MPFROM2SHORT(TRUE, CRA_INUSE));
		DefaultViewKeys(hwnd,
				dcd->hwndFrame,
				dcd->hwndParent, &swp, pci->pszFileName);
		WinSendMsg(hwnd,
			   CM_SETRECORDEMPHASIS,
			   MPFROMP(pci),
			   MPFROM2SHORT(FALSE,
					CRA_INUSE |
					((fUnHilite) ? CRA_SELECTED : 0)));
	      }
	    }
	    else {
	      if (!*dcd->directory || IsValidDir(dcd->directory)) {
		NotifyError(pci->pszFileName, status);
		RemoveCnrItems(hwnd, pci, 1, CMA_FREE | CMA_INVALIDATE | CMA_ERASE);
		if (hwndStatus)
		  WinSetWindowText(hwndStatus,
				   (CHAR *) GetPString(IDS_RESCANSUGGESTEDTEXT));
	      }
	      else {
		dcd->stopflag++;
		if (!PostMsg(dcd->hwndObject,
			     UM_RESCAN, MPVOID, MPFROMLONG(1L))) {
		  dcd->stopflag--;
		}
		else if (*dcd->directory) {
		  if (hwndMain)
		    WinSendMsg(hwndMain,
			       UM_SETUSERLISTNAME,
			       MPFROMP(dcd->directory), MPVOID);
		  else
		    add_udir(FALSE, dcd->directory);
		}
	      }
	    }
	  }
	  else if (*dcd->directory)
	    OpenObject(dcd->directory, Default, hwnd);
	} // CN_ENTER
	break;
      } // switch mp1
      break;
    } // if dcd
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
      xfree((CHAR *)mp2, pszSrcFile, __LINE__);
      return MRFROMLONG(hwnd);
    }
    return 0;

  case WM_SAVEAPPLICATION:
    if (dcd && ParentIsDesktop(hwnd, dcd->hwndParent)) {

      SWP swp;

      WinQueryWindowPos(dcd->hwndFrame, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE | SWP_MAXIMIZE)))
	PrfWriteProfileData(fmprof, appname, "VDirSizePos", &swp, sizeof(swp));
    }
    break;

  case WM_CLOSE:
    WinSendMsg(hwnd, WM_SAVEAPPLICATION, MPVOID, MPVOID);
    if (LastDir == hwnd)
	LastDir = (HWND) 0;
    DosRequestMutexSem(hmtxFiltering, SEM_INDEFINITE_WAIT);
    if (dcd) {
      dcd->stopflag++;
      if (!dcd->dontclose && ParentIsDesktop(dcd->hwndFrame, (HWND) 0))
	PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
      if (dcd->hwndObject) {
	  // Ensure object window destroy does not attempt duplicate clean up
	  WinSetWindowPtr(dcd->hwndObject, QWL_USER, NULL);
	  if (!PostMsg(dcd->hwndObject, WM_CLOSE, MPVOID, MPVOID))
	    Win_Error(dcd->hwndObject, hwnd, pszSrcFile, __LINE__, "hwndObject WinPostMsg failed");
      }
      if (dcd->hwndRestore)
	WinSetWindowPos(dcd->hwndRestore,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_RESTORE | SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER);
      FreeList(dcd->lastselection);
      WinSetWindowPtr(hwnd, QWL_USER, NULL);
      DosPostEventSem(CompactSem);
    }
    DosReleaseMutexSem(hmtxFiltering);
    WinDestroyWindow(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				    QW_PARENT));
    return 0;

  case WM_DESTROY:
#   ifdef FORTIFY
    DbgMsg(pszSrcFile, __LINE__, "WM_DESTROY hwnd %p TID %u", hwnd, GetTidForThread());	// 18 Jul 08 SHL fixme
#   endif
    if (DirMenu)
      WinDestroyWindow(DirMenu);
    if (DirCnrMenu)
      WinDestroyWindow(DirCnrMenu);
    if (FileMenu)
      WinDestroyWindow(FileMenu);
    DirMenu = DirCnrMenu = FileMenu = (HWND) 0;
    EmptyCnr(hwnd);
    DosRequestMutexSem(hmtxFiltering, SEM_INDEFINITE_WAIT);
    xfree(dcd, pszSrcFile, __LINE__);
    dcd = NULL;
    DosReleaseMutexSem(hmtxFiltering);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#   endif
    break;
  } // switch

  if (dcd && dcd->oldproc) {
#   ifdef FORTIFY    // 11 May 08 SHL fixme debug fortify
    if ((ULONG)dcd->oldproc == 0xa9a9a9a9)
      DbgMsg(pszSrcFile, __LINE__, "calling oldproc after dcd free msg %x mp1 %x mp2 %x",
	     msg, mp1, mp2);
#   endif
    return dcd->oldproc(hwnd, msg, mp1, mp2);
  }
  else
      return PFNWPCnr(hwnd, msg, mp1, mp2);
}

/**
 * Search container for matching text
 * @return TRUE if key completely handled here
 */

MRESULT EXPENTRY SearchContainer(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *dcd = INSTDATA(hwnd);
  ULONG thistime;
  UINT len;
  SEARCHSTRING srch;
  PCNRITEM pci;
  USHORT key;

  if (!dcd)
    return FALSE;

  // Just to be safe, caller has probably already checked
  if (SHORT1FROMMP(mp1) & KC_KEYUP)
    return FALSE;			// Let PM process key

  // Just to be safe, caller has probably already cached
  shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));

  // If not plain character or suppressed
  // Shift toggles configured setting
  if (shiftstate & (KC_ALT | KC_CTRL) || (shiftstate & KC_SHIFT ? !fNoSearch : fNoSearch))
    return FALSE;			// Let PM process key

  if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY) {
    key = SHORT2FROMMP(mp2);
    if (key == VK_BACKSPACE)
      key = '\x8';
    else if (key == VK_ESC)
      key = '\x1b';
    else
      return FALSE;				// Let PM process key
  }
  else if (SHORT1FROMMP(mp1) & KC_CHAR)
    key = SHORT1FROMMP(mp2);
  else
    return FALSE;

  thistime = WinQueryMsgTime(WinQueryAnchorBlock(hwnd));
  if (thistime > dcd->lasttime + 3000)
    *dcd->szCommonName = 0;		// 3 seconds since last character
  dcd->lasttime = thistime;

  switch (key) {
  case '\x1b':			// Esc
    *dcd->szCommonName = 0;
    break;
  default:
    if (key == ' ' && !*dcd->szCommonName)
      break;			// Let PM process space to allow select toggle
    len = strlen(dcd->szCommonName);
    if (key == '\x8') {
      // Backspace
      if (len) {
	len--;
	dcd->szCommonName[len] = 0;	// Chop
      }
    }
    else {
      if (len >= CCHMAXPATH - 1) {
	if (!fErrorBeepOff)
	  DosBeep(250,100);
	  // WinAlarm(hwnd,WA_WARNING);
      }
      else {
	dcd->szCommonName[len] = toupper(key);
	dcd->szCommonName[len + 1] = 0;
      }
    }
    // Case insensitive search
    memset(&srch, 0, sizeof(SEARCHSTRING));
    srch.cb = (ULONG) sizeof(SEARCHSTRING);
    srch.pszSearch = (PSZ) dcd->szCommonName;
    srch.fsPrefix = TRUE;
    srch.fsCaseSensitive = FALSE;
    srch.usView = CV_ICON;
    pci = WinSendMsg(hwnd, CM_SEARCHSTRING, MPFROMP(&srch),
		     MPFROMLONG(CMA_FIRST));
    if (pci && (INT) pci != -1) {
      // Got match make found item current item
      USHORT attrib = CRA_CURSORED;
      // 29 Mar 09 SHL fixme to clear other object select if not extended select
      if (!stricmp(pci->pszDisplayName, dcd->szCommonName))
	attrib |= CRA_SELECTED;
      WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		 MPFROM2SHORT(TRUE, attrib));
      // make sure that record shows in viewport
      ShowCnrRecord(hwnd, (PMINIRECORDCORE) pci);
      return (MRESULT)TRUE;
    }
    else {
      if (key == ' ')
	return FALSE;			// Let PM process space to toggle select
      // No match
      // Erase non-matching last character, len is not yet incremented
      dcd->szCommonName[len] = 0;
      if (len == 0 && key != '\\') {
	// Let's see if user forgot leading backslash
	if (SearchContainer(hwnd, msg, mp1, MPFROM2SHORT('\\', 0))) {
	  if (SearchContainer(hwnd, msg, mp1, mp2))
	    return (MRESULT)TRUE;	// Grab key from PM
	}
      }
      if (*dcd->szCommonName)
	return (MRESULT)TRUE;		// Have partial match, grab key from PM
#if 0 // 15 Mar 09 SHL fixme to not hang
      if (!fErrorBeepOff)
	DosBeep(250,100);
	// WinAlarm(hwnd,WA_WARNING);
#endif
    }
  } // switch

  return FALSE;				// Let PM process key
}

HWND StartDirCnr(HWND hwndParent, CHAR * directory, HWND hwndRestore,
		 ULONG flags)
{
  /**
   * bitmapped flags:
   * 0x00000001 = don't close app when window closes
   * 0x00000002 = no frame controls
   */

  HWND hwndFrame = (HWND) 0, hwndClient;
  ULONG FrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
    FCF_SIZEBORDER | FCF_MINMAX | FCF_ICON | FCF_NOBYTEALIGN | FCF_ACCELTABLE;
  USHORT id;
  static USHORT idinc = 0;
  DIRCNRDATA *dcd;
  static BOOL first = FALSE;

  if (flags & 2)
    FrameFlags &= (~(FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
		     FCF_MINMAX | FCF_ICON));
  if (!idinc)
    idinc = (rand() % 100);
  if (!hwndParent)
    hwndParent = HWND_DESKTOP;
  if (ParentIsDesktop(hwndParent, hwndParent))
    FrameFlags |= (FCF_TASKLIST | FCF_MENU);
  if (!hwndMain && !first) {
    if (DirCnrMenu) {
      MENUITEM mi;
      memset(&mi, 0, sizeof(mi));
      WinSendMsg(DirCnrMenu,
		 MM_DELETEITEM, MPFROM2SHORT(IDM_DRIVESMENU, FALSE), MPVOID);
      mi.iPosition = MIT_END;
      mi.afStyle = MIS_TEXT;
      mi.id = IDM_DRIVESMENU;
      WinSendMsg(DirCnrMenu,
		 MM_INSERTITEM,
		 MPFROMP(&mi), MPFROMP(GetPString(IDS_DRIVESMENUTEXT)));
    }
    first = TRUE;
  }
  if (directory) {
    hwndFrame = WinCreateStdWindow(hwndParent,
				   WS_VISIBLE,
				   &FrameFlags,
				   (CHAR *) WC_DIRCONTAINER,
				   NULL,
				   WS_VISIBLE | fwsAnimate,
				   FM3ModHandle, DIR_FRAME, &hwndClient);
    if (hwndFrame && hwndClient) {
      id = DIR_FRAME + idinc++;
      if (idinc > 99)
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
	dcd->size = sizeof(DIRCNRDATA);
	dcd->id = id;
	dcd->type = DIR_FRAME;
	dcd->hwndParent = (hwndParent) ? hwndParent : HWND_DESKTOP;
	dcd->hwndFrame = hwndFrame;
	dcd->hwndClient = hwndClient;
	dcd->hwndRestore = hwndRestore;
	dcd->dontclose = ((flags & 1) != 0);
	dcd->ds.detailslongname = dsDirCnrDefault.detailslongname;
	dcd->ds.detailssubject = dsDirCnrDefault.detailssubject;
	dcd->ds.detailsea = dsDirCnrDefault.detailsea;
	dcd->ds.detailssize = dsDirCnrDefault.detailssize;
	dcd->ds.detailsicon = dsDirCnrDefault.detailsicon;
	dcd->ds.detailsattr = dsDirCnrDefault.detailsattr;
	dcd->ds.detailscrdate = dsDirCnrDefault.detailscrdate;
	dcd->ds.detailscrtime = dsDirCnrDefault.detailscrtime;
	dcd->ds.detailslwdate = dsDirCnrDefault.detailslwdate;
	dcd->ds.detailslwtime = dsDirCnrDefault.detailslwtime;
	dcd->ds.detailsladate = dsDirCnrDefault.detailsladate;
	dcd->ds.detailslatime = dsDirCnrDefault.detailslatime;
	strcpy(dcd->directory, directory);
	add_udir(FALSE, directory);
	{
	  PFNWP oldproc;

	  oldproc = WinSubclassWindow(hwndFrame, (PFNWP) DirFrameWndProc);
	  WinSetWindowPtr(hwndFrame, QWL_USER, (PVOID) oldproc);
	}
	dcd->hwndCnr = WinCreateWindow(hwndClient,
				       WC_CONTAINER,
				       NULL,
				       CCS_AUTOPOSITION | CCS_MINIICONS |
				       CCS_MINIRECORDCORE | ulCnrType, 
				       0,
				       0,
				       0,
				       0,
				       hwndClient,
				       HWND_TOP, (ULONG) DIR_CNR, NULL, NULL);
	if (!dcd->hwndCnr) {
	  Win_Error(hwndClient, hwndClient, pszSrcFile, __LINE__,
		    PCSZ_WINCREATEWINDOW);
	  PostMsg(hwndClient, WM_CLOSE, MPVOID, MPVOID);
	  DosRequestMutexSem(hmtxFiltering, SEM_INDEFINITE_WAIT);
	  free(dcd);
	  DosReleaseMutexSem(hmtxFiltering);
	  hwndFrame = (HWND) 0;
	}
	else {
#	  ifdef FORTIFY
	  Fortify_ChangeScope(dcd, -1);
#	  endif
	  RestorePresParams(dcd->hwndCnr, PCSZ_DIRCNR);
	  WinSetWindowPtr(dcd->hwndCnr, QWL_USER, (PVOID) dcd);
	  dcd->oldproc = WinSubclassWindow(dcd->hwndCnr,
					   (PFNWP) DirCnrWndProc);
	  {
	    USHORT ids[] = { DIR_TOTALS, DIR_SELECTED, DIR_VIEW, DIR_SORT,
	      DIR_FILTER, DIR_FOLDERICON, DIR_MAX, 0
	    };

	    if (!(flags & 2))
	      ids[6] = 0;
	    CommonCreateTextChildren(dcd->hwndClient,
				     WC_DIRSTATUS, ids);
	  }
	  if (!PostMsg(dcd->hwndCnr, UM_SETUP, MPVOID, MPVOID))
	    WinSendMsg(dcd->hwndCnr, UM_SETUP, MPVOID, MPVOID);
	  if (FrameFlags & FCF_TASKLIST) {

	    SWP swp, swpD;
	    ULONG size = sizeof(swp);
	    LONG cxScreen, cyScreen;

	    WinQueryTaskSizePos(WinQueryAnchorBlock(hwndFrame), 0, &swp);
	    if (PrfQueryProfileData(fmprof, appname, "VDirSizePos", &swpD, &size)) {
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
	  WinShowWindow(dcd->hwndCnr, TRUE);
	}
      }
#     ifdef FORTIFY
      Fortify_LeaveScope();
#     endif
    }
  }
  return hwndFrame;
}

#pragma alloc_text(DIRCNRS,DirCnrWndProc,DirObjWndProc,DirClientWndProc)
#pragma alloc_text(DIRCNRS,DirTextProc,DirFrameWndProc)
#pragma alloc_text(STARTUP,StartDirCnr)
