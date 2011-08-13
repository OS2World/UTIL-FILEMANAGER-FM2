
/***********************************************************************

  $Id$

  Collector

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2011 Steven H. Levine

  15 Oct 02 MK Baseline
  10 Jan 04 SHL Avoid -1L byte counts
  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  24 May 05 SHL Rework Win_Error usage
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  25 May 05 SHL Rework for FillInRecordFromFFB
  05 Jun 05 SHL Use QWL_USER
  06 Jun 05 SHL Indent -i2
  06 Jun 05 SHL Make savedSortFlags static to avoid referencing garbage
  24 Oct 05 SHL Sanitize handle references
  24 Oct 05 SHL CollectorCnrWndProc: avoid excess writes to Status2 window
  10 Nov 05 SHL CollectorCnrWndProc: correct missing button window updates
  14 Jul 06 SHL Use Runtime_Error
  27 Jul 06 SHL Avoid shutdown hang - pre3 typo
  29 Jul 06 SHL Use xfgets_bstripcr
  15 Aug 06 SHL Don't write garbage to CollectorFilter INI entry
  15 Aug 06 SHL Rework SetMask args
  18 Aug 06 SHL CollectorCnrWndProc: avoid freeing NULL pointer
  31 Aug 06 SHL Disable Utilities->Seek and scan menu while busy
  31 Aug 06 SHL Correct stop scan context menu enable/disable
  30 Mar 07 GKY Remove GetPString for window class names
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits
  06 Apr 07 GKY Add some error checking in drag/drop
  19 Apr 07 SHL Use FreeDragInfoData.  Add more drag/drop error checks.
  12 May 07 SHL Use dcd->ulItemsToUnHilite
  10 Jun 07 GKY Add CheckPmDrgLimit including IsFm2Window as part of work around PM drag limit
  05 Jul 07 SHL CollectorCnrWndProc: just warn if busy
  02 Aug 07 SHL Sync with CNRITEM mods
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  26 Aug 07 GKY DosSleep(1) in loops changed to (0)
  22 Nov 07 GKY Use CopyPresParams to fix presparam inconsistencies in menus
  10 Jan 08 SHL Sync with CfgDlgProc mods
  10 Feb 08 GKY Implement bubble help for bitmap menu items
  15 Feb 08 SHL Sync with settings menu rework
  15 Feb 08 GKY Fix attempt to free container items that were never inserted
  15 Feb 08 GKY Fix "collect" so it updates recollected files and unhides them if needed
  29 Feb 08 GKY Use xfree where appropriate
  06 Jul 08 GKY Update delete/undelete to include move to and open XWP trashcan
  11 Jul 08 JBS Ticket 230: Simplified code and eliminated some local variables by incorporating
		all the details view settings (both the global variables and those in the
		DIRCNRDATA struct) into a new struct: DETAILS_SETTINGS.
  20 Jul 08 GKY Add save/append filename to clipboard.
		Change menu wording to make these easier to find
  25 Aug 08 GKY Check TMP directory space warn if lee than 5 MiB prevent archiver from opening if
		less than 10 KiB (It hangs and can't be closed)
  10 Dec 08 SHL Integrate exception handler support
  26 Dec 08 GKY Fixed DROPHELP to check for copy as default is action is DO_DEFAULT
  01 Jan 09 GKY Add Seek and Scan to drives & directory context menus pass drive/dir as search root
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale
  07 Feb 09 GKY Move repeated strings to PCSZs.
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  06 Jun 09 GKY Add option to show file system type or drive label in tree
  12 Jul 09 GKY Add szFSType to FillInRecordFromFSA use to bypass EA scan and size formatting
		for tree container
  13 Jul 09 GKY Fixed double free of memory buffer in UM_COLLECTFROMFILE
  15 Sep 09 SHL Use UM_GREP when passing pathname
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  23 Oct 10 GKY Add menu items for opening directory cnrs based on path of selected item
		including the option to use walk directories to select path
  28 May 11 GKY Fixed trap caused by passing a nonexistant pci to FillInRecordFromFFB in
		UM_COLLECT because pci is limited to 65535 files. (nRecord is a USHORT)
  29 May 11 SHL Rework UM_COLLECT >65K records logic to not require double loop
  29 May 11 SHL Tweak UM_COLLECT to bypass FindCnrRecord when container initially empty
  08 Aug 11 SHL Rework UM_COLLECT to avoid spurious container items free
  13 Aug 11 GKY Have file count and KIBs update at the same time

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <limits.h>			// USHRT_MAX
// #include <process.h>			// _beginthread

#define INCL_DOS			// QSV_MS_COUNT
#define INCL_WIN
#define INCL_DOSERRORS
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "dircnrs.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "grep.h"
#include "comp.h"
#include "arccnrs.h"			// StartArcCnr
#include "filldir.h"			// EmptyCnr...
#include "strutil.h"			// GetPString
#include "errutil.h"			// Runtime_Error
#include "tmrsvcs.h"			// ITIMER_DESC
#include "notebook.h"			// CfgDlgProc
#include "command.h"			// RunCommand
#include "worker.h"			// Action, MassAction
#include "notify.h"			// AddNote
#include "misc.h"		// AdjustCnrColsForPref, AdjustDetailsSwitches, CnrDirectEdit,
					// LoadDetailsSwitches, OpenEdit, QuickPopup, SayFilter
					// SaySort, SayView, SetCnrCols, SetDetailsSwitches
					// SetSortChecks, SetViewMenu, disable_menuitem, CheckMenu
					// CurrentRecord, DrawTargetEmphasis, IsFm2Window
#include "chklist.h"			// CenterOverWindow, DropListProc
#include "collect.h"
#include "common.h"			// CommonCnrProc, CommonCreateTextChildren, CommonFrameWndProc
					// CommonTextPaint
#include "select.h"			// DeselectAll, HideAll, RemoveAll, SelectAll, SelectList
#include "dirsize.h"			// DirSizeProc
#include "grep2.h"			// GrepDlgProc
#include "mainwnd.h"			// MakeBubble
#include "objwin.h"			// MakeObjWin
#include "saveclip.h"			// SaveListDlgProc
#include "findrec.h"			// ShowCnrRecord
#include "sortcnr.h"			// SortCollectorCnr
#include "seeall.h"			// StartSeeAll
#include "update.h"			// UpdateCnrList, UpdateCnrRecord
#include "droplist.h"			// CheckPmDrgLimit
#include "common.h"			// CommonTextButton, CommonTextProc
#include "presparm.h"			// CopyPresParams
#include "defview.h"			// DefaultViewKeys
#include "draglist.h"			// DoFileDrag, FreeDragInfoData
#include "systemf.h"			// ExecOnList
#include "filter.h"			// Filter
#include "findrec.h"			// FindCnrRecord
#include "shadow.h"			// OpenObject
#include "mkdir.h"			// PMMkDir
#include "valid.h"			// ParentIsDesktop
#include "viewer.h"			// StartMLEEditor
#include "newview.h"			// StartViewer
#include "undel.h"			// UndeleteDlgProc
#include "i18nutil.h"			// commafmt
#include "getnames.h"			// insert_filename
#include "select.h"			// InvertAll
#include "strips.h"			// bstrip
#include "wrappers.h"			// xDosFindFirst
#include "fortify.h"
#include "excputil.h"			// xbeginthread
#include "walkem.h"			// WalkAllDlgProc

// Data definitions
#pragma data_seg(GLOBAL1)
HWND CollectorCnrMenu;
HWND hwndStatus2;

#pragma data_seg(GLOBAL2)
INT CollectorsortFlags;

#pragma data_seg(DATA1)
static PSZ pszSrcFile = __FILE__;

MRESULT EXPENTRY CollectorFrameWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				       MPARAM mp2)
{
  return CommonFrameWndProc(COLLECTOR_CNR, hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CollectorTextProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2)
{
  DIRCNRDATA *dcd;

  static BOOL emphasized = FALSE;
  static HWND hwndButtonPopup = (HWND) 0;
  static ULONG timestamp = ULONG_MAX;
  static USHORT lastid = 0;

  switch (msg) {
  case WM_CREATE:
    return CommonTextProc(hwnd, msg, mp1, mp2);

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

	  if (hwndButtonPopup)
	    WinDestroyWindow(hwndButtonPopup);
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
	  if (hwndButtonPopup) {
	    WinSetWindowUShort(hwndButtonPopup, QWS_ID, id);
	    dcd = WinQueryWindowPtr(WinWindowFromID(WinQueryWindow(hwnd,
								   QW_PARENT),
						    COLLECTOR_CNR), QWL_USER);
	    if (id == DIR_VIEW) {
	      if (dcd) {
		SetViewMenu(hwndButtonPopup, dcd->flWindowAttr);
		SetDetailsSwitches(hwndButtonPopup, &dcd->ds);
		CopyPresParams(hwndButtonPopup, hwnd);
	      }

	      /* don't have tree view in collector */
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_TREEVIEW, FALSE), MPVOID);

	    }
	    else if (id == DIR_SORT) {
	      if (dcd)
		SetSortChecks(hwndButtonPopup, dcd->sortFlags);
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
	      PaintRecessedWindow(hwnd, NULLHANDLE, FALSE, FALSE);
	    }
	  }
	}
	break;
      default:
	PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				COLLECTOR_CNR),
		WM_CONTROL,
		MPFROM2SHORT(COLLECTOR_CNR, CN_CONTEXTMENU), MPVOID);
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
      case DIR_SELECTED:
      case DIR_VIEW:
      case DIR_SORT:
	PaintRecessedWindow(hwnd, NULLHANDLE, TRUE, FALSE);
	break;
      }
    }
    break;

  case WM_COMMAND:
    {
      DIRCNRDATA *dcd;
      MRESULT mr;

      mr = WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,
						     QW_PARENT),
				      COLLECTOR_CNR), msg, mp1, mp2);
      if (hwndButtonPopup &&
	  SHORT1FROMMP(mp1) > IDM_DETAILSTITLES &&
	  SHORT1FROMMP(mp1) < IDM_DETAILSSETUP) {
	dcd = WinQueryWindowPtr(WinWindowFromID(WinQueryWindow(hwnd,
							       QW_PARENT),
						COLLECTOR_CNR), QWL_USER);
	if (dcd)
	  SetDetailsSwitches(hwndButtonPopup, &dcd->ds);
      }
      return mr;
    }

  case WM_MOUSEMOVE:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);
      PCSZ s = NULL;

      if (fOtherHelp) {
	if ((!hwndBubble ||
	     WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd) &&
	    !WinQueryCapture(HWND_DESKTOP)) {
	  switch (id) {
	  case DIR_SELECTED:
	    s = GetPString(IDS_COLSELECTEDHELP);
	    break;
	  case DIR_TOTALS:
	    s = GetPString(IDS_COLTOTALSHELP);
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
	return CommonTextButton(hwnd, msg, mp1, mp2);
      }
    }
    break;

  case WM_BUTTON3UP:
  case WM_BUTTON1UP:
  case WM_BUTTON3DOWN:
  case WM_BUTTON1DOWN:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id) {
      case DIR_FILTER:
      case DIR_SORT:
      case DIR_VIEW:
      case DIR_SELECTED:
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
				COLLECTOR_CNR),
		WM_COMMAND, MPFROM2SHORT(cmd, 0), MPVOID);
    }
    return 0;

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
    else {
      if (emphasized) {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
    }
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
      }
      memset(&cnd, 0, sizeof(cnd));
      cnd.pDragInfo = (PDRAGINFO) mp1;
      cnd.pRecord = NULL;
      return WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_CONTROL,
			MPFROM2SHORT(COLLECTOR_CNR, dcmd), MPFROMP(&cnd));
    }
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CollectorClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
					MPARAM mp2)
{
  switch (msg) {
  case UM_CONTAINERHWND:
    return MRFROMLONG(WinWindowFromID(hwnd, COLLECTOR_CNR));

  case UM_VIEWSMENU:
    return MRFROMLONG(CheckMenu(hwnd, &CollectorCnrMenu, COLLECTORCNR_POPUP));

  case MM_PORTHOLEINIT:
  case WM_INITMENU:
  case UM_INITMENU:
  case UM_CONTAINER_FILLED:
  case UM_FILESMENU:
  case UM_UPDATERECORD:
  case UM_UPDATERECORDLIST:
    return WinSendMsg(WinWindowFromID(hwnd, COLLECTOR_CNR), msg, mp1, mp2);

  case WM_PSETFOCUS:
  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, COLLECTOR_CNR));
    break;

  case WM_PAINT:
    {
      HPS hps;
      RECTL rcl;

      hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
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
      WinSetWindowPos(WinWindowFromID(hwnd, COLLECTOR_CNR), HWND_TOP,
		      0, 0, cx, cy - 24, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_TOTALS), HWND_TOP,
		      2,
		      cy - 22,
		      (cx / 3) - 2, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_SELECTED), HWND_TOP,
		      2 + (cx / 3) + 2,
		      cy - 22,
		      (cx / 3) - 2, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      bx = (cx - (2 + (((cx / 3) + 2) * 2))) / 3;
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_VIEW), HWND_TOP,
		      2 + (((cx / 3) + 2) * 2),
		      cy - 22, bx - 4, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_SORT), HWND_TOP,
		      2 + (((cx / 3) + 2) * 2) + bx,
		      cy - 22, bx - 4, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_FILTER), HWND_TOP,
		      2 + (((cx / 3) + 2) * 2) + (bx * 2),
		      cy - 22, bx - 4, 20, SWP_SHOW | SWP_MOVE | SWP_SIZE);
    }
    CommonTextPaint(hwnd, NULLHANDLE);
    if (msg == UM_SIZE) {
      WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), HWND_TOP, 0, 0, 0, 0,
		      SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE);
      return 0;
    }
    break;

  case UM_COMMAND:
  case WM_COMMAND:
  case WM_CONTROL:
  case WM_CLOSE:
    return WinSendMsg(WinWindowFromID(hwnd, COLLECTOR_CNR), msg, mp1, mp2);
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CollectorObjWndProc(HWND hwnd, ULONG msg,
				     MPARAM mp1, MPARAM mp2)
{
  ULONG size;
  DIRCNRDATA *dcd;

  switch (msg) {
  case WM_CREATE:
    // 18 Jul 08 SHL fixme to doc why messsage gets lost
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
      li = DoFileDrop(dcd->hwndCnr, NULL, FALSE, MPVOID, MPFROMP(&cni));
      CheckPmDrgLimit(cni.pDragInfo);
      if (li) {
	li->type = fDefaultDeletePerm ? IDM_PERMDELETE : IDM_DELETE;
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
	UpdateCnrList(dcd->hwndCnr, list, numentries, FALSE, dcd);
    }
    return 0;

  case UM_SETUP:
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
#     ifdef FORTIFY
      Fortify_BecomeOwner(dcd);
#     endif
      /* set unique id */
      WinSetWindowUShort(hwnd,
			 QWS_ID,
			 COLLECTOROBJ_FRAME + (COLLECTOR_FRAME - dcd->id));
      dcd->hwndObject = hwnd;
      // 09 Feb 08 SHL fixme to be sure applet does not really need this
      // if (ParentIsDesktop(hwnd, dcd->hwndParent))
      //	DosSleep(100); //05 Aug 07 GKY 250
    }
    else
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    return 0;

  case UM_COMMAND:
    if (mp1) {
      LISTINFO *li = (LISTINFO *) mp1;

      switch (li->type) {
      case IDM_DOITYOURSELF:
      case IDM_APPENDTOCLIP:
      case IDM_APPENDTOCLIPFILENAME:
      case IDM_SAVETOCLIP:
      case IDM_ARCHIVE:
      case IDM_ARCHIVEM:
      case IDM_VIEW:
      case IDM_VIEWTEXT:
      case IDM_VIEWBINARY:
      case IDM_VIEWARCHIVE:
      case IDM_EDIT:
      case IDM_EDITTEXT:
      case IDM_EDITBINARY:
      case IDM_OBJECT:
      case IDM_SHADOW:
      case IDM_SHADOW2:
      case IDM_PRINT:
      case IDM_ATTRS:
      case IDM_DELETE:
      case IDM_PERMDELETE:
      case IDM_FAKEEXTRACT:
      case IDM_FAKEEXTRACTM:
      case IDM_MCIPLAY:
      case IDM_UPDATE:
	if (PostMsg(hwnd, UM_MASSACTION, mp1, mp2))
	  return (MRESULT) TRUE;
	break;
      default:
	if (PostMsg(hwnd, UM_ACTION, mp1, mp2))
	  return (MRESULT) TRUE;
      }
    }
    return 0;

  case UM_COLLECT:
    DosError(FERR_DISABLEHARDERR);
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
      LISTINFO *li = (LISTINFO *) mp1;
      INT x;
      FILEFINDBUF4L fb4;
      HDIR hdir;
      ULONG ulMaxFiles;
      CHAR fullname[CCHMAXPATH];

      if (!hwndStatus) {
	WinSetWindowText(WinWindowFromID(dcd->hwndClient, DIR_SELECTED),
			 (CHAR *) GetPString(IDS_COLLECTINGTEXT));
      }
      else {
	if (WinQueryFocus(HWND_DESKTOP) == dcd->hwndCnr)
	  WinSetWindowText(hwndStatus, (CHAR *) GetPString(IDS_COLLECTINGTEXT));
      }

      for (ulMaxFiles = 0; li->list[ulMaxFiles]; ulMaxFiles++) ;	// Count

      if (ulMaxFiles) {
	PCNRITEM pci = NULL;
	PCNRITEM pciFirst = NULL;
	PCNRITEM pciPrev;
	ULONG nm;
	ULONG ulRecsAtStart;
	ULONG ulRecsToInsert;
	ULONG ulRecsInserted = 0;
	ULONGLONG ullTotalBytes = 0;
	BOOL checkToInsert = FALSE;
	CNRINFO cnri;
	RECORDINSERT ri;
	ITIMER_DESC itdSleep = { 0 };	// 06 Feb 08 SHL

	InitITimer(&itdSleep, 500);	// Sleep every 500 mSec

	// Query initial count
	// 2011-05-29 SHL fixme to be utility
	memset(&cnri, 0, sizeof(CNRINFO));
	cnri.cb = sizeof(CNRINFO);
	if (WinSendMsg(dcd->hwndCnr, CM_QUERYCNRINFO, MPFROMP(&cnri),
		       MPFROMLONG(sizeof(CNRINFO)))) {
	  ulRecsAtStart = cnri.cRecords;
	}
	else {
	  Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		    "CM_QUERYCNRINFO" /* PCSZ_QUERYCNRINFO fixme */);
	  ulRecsAtStart = 0;
	}

	for (x = 0; li->list[x]; x++) {

	  // Allocate more container items if needed
	  if (!pci) {
	    ulRecsToInsert = ulMaxFiles - ulRecsInserted;	// Left to do
	    if (ulRecsToInsert > USHRT_MAX)
	      ulRecsToInsert = USHRT_MAX;		// Avoid USHORT overflows
	    pci = WinSendMsg(dcd->hwndCnr, CM_ALLOCRECORD,
			     MPFROMLONG(EXTRA_RECORD_BYTES),
			     MPFROMLONG(ulRecsToInsert));
	    pciFirst = pci;
	    pciPrev = NULL;
	    if (!pci) {
	      Runtime_Error(pszSrcFile, __LINE__, PCSZ_CM_ALLOCRECORD);
	      break;
	    }
	  } // if need allocate

	  nm = 1;
	  hdir = HDIR_CREATE;
	  DosError(FERR_DISABLEHARDERR);
	  // If started with records in container, check if updating existing record
	  if (ulRecsAtStart &&
	      FindCnrRecord(dcd->hwndCnr,
			    li->list[x],
			    NULL,
			    FALSE,
			    FALSE,
			    TRUE))
	  {
	    // Updating existing record
	    PCNRITEM pciUpd = UpdateCnrRecord(dcd->hwndCnr, li->list[x], FALSE, dcd);
	    if (!pciUpd) {
	      // file has disappeared
	      Runtime_Error(pszSrcFile, __LINE__, "pci NULL for list[%u]", x);
	    }
	    else {
	      // Update OK
	      if (Filter((PMINIRECORDCORE) pciUpd, (PVOID) & dcd->mask)) {
		pciUpd->rc.flRecordAttr &= ~CRA_FILTERED;	// Ensure visible
		// 2011-05-29 SHL fixme to check fail
		WinSendMsg(dcd->hwndCnr, CM_INVALIDATERECORD, MPVOID,
			   MPFROM2SHORT(0, CMA_REPOSITION | CMA_ERASE));
	      }
	    }
	    ulMaxFiles--;		// No insert needed
	    checkToInsert = TRUE;
	  }
	  // Add new entry maybe
	  else if (*li->list[x] &&
	      !DosQueryPathInfo(li->list[x], FIL_QUERYFULLNAME,
				fullname, sizeof(fullname)) &&
	      !IsRoot(fullname) &&
	      !xDosFindFirst(fullname,
			     &hdir,
			     FILE_NORMAL | FILE_DIRECTORY |
			     FILE_ARCHIVED | FILE_SYSTEM |
			     FILE_HIDDEN | FILE_READONLY,
			     &fb4, sizeof(fb4), &nm, FIL_QUERYEASIZEL))
	  {
	    // OK to add
	    DosFindClose(hdir);
	    priority_normal();
	    *fb4.achName = 0;
	    ullTotalBytes += FillInRecordFromFFB(dcd->hwndCnr,
						pci,
						fullname, &fb4, FALSE, dcd);
	    pciPrev = pci;
	    pci = (PCNRITEM) pci->rc.preccNextRecord;
	  }
	  else {
	    // DosQueryPathInfo etc. failed - try to recover
	    Runtime_Error(pszSrcFile, __LINE__, "DosQueryPathInfo failed for %s", fullname);
	    ulMaxFiles--;		// Nothing to insert
	    checkToInsert = TRUE;
	  }

	  if (checkToInsert) {
	    checkToInsert = FALSE;
	    // Remove extra records from chain
	    while (ulRecsInserted + ulRecsToInsert > ulMaxFiles) {
	      PCNRITEM pciNext = (PCNRITEM)pci->rc.preccNextRecord;
	      if (pciPrev)
		pciPrev->rc.preccNextRecord = (PMINIRECORDCORE)pciNext;
	      else
		pciFirst = pciNext;
	      pci->pszFileName = NullStr;	// Avoid spurious complaints
	      FreeCnrItem(dcd->hwndCnr, pci);
	      pci = pciNext;
	      ulRecsToInsert--;		// Remember gone
	    }
	  }

	  // Check if time to insert
	  if (!pci) {
	    // All allocated CNRITEMs filled
	    if (ulRecsToInsert) {
	      // Have CNRITEMs to insert
	      memset(&ri, 0, sizeof(RECORDINSERT));
	      ri.cb = sizeof(RECORDINSERT);
	      ri.pRecordOrder = (PRECORDCORE) CMA_END;
	      ri.pRecordParent = (PRECORDCORE) 0;
	      ri.zOrder = (ULONG) CMA_TOP;
	      ri.cRecordsInsert = ulRecsToInsert;
	      ri.fInvalidateRecord = TRUE;
	      WinSendMsg(dcd->hwndCnr,
			 CM_INSERTRECORD, MPFROMP(pciFirst), MPFROMP(&ri));
	      // 2011-05-29 SHL fixme to complain on failure
	      PostMsg(dcd->hwndCnr, UM_RESCAN, MPVOID, MPVOID);
              ulRecsInserted += ulRecsToInsert;
              dcd->ullTotalBytes = ullTotalBytes;
	      pciFirst = NULL;
	      ulRecsToInsert = 0;
	    }
	  } // if need CM_INSERTRECORD
	  SleepIfNeeded(&itdSleep, 1);	// 09 Feb 08 SHL
	} // for

	// Clean up in case stopped early by error
	if (pci)
	  Runtime_Error(pszSrcFile, __LINE__, "pci not NULL");

	if (pciFirst) {
	  Runtime_Error(pszSrcFile, __LINE__, "pciFirst not NULL");
	  // 2011-08-08 SHL fixme to initialze pszFileName to prevent spurious complaints
	  FreeCnrItemList(dcd->hwndCnr, pciFirst);
	}

      } // if have files

      if (dcd->flWindowAttr & CV_DETAIL)
	WinSendDlgItemMsg(hwnd,
			  COLLECTOR_CNR,
			  CM_INVALIDATERECORD,
			  MPVOID, MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
    } // if dcd
    return 0;

  case UM_COLLECTFROMFILE:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif

    if (dcd && mp1) {
      FILESTATUS4L fs4;
      PCNRITEM pci;
      RECORDINSERT ri;
      CHAR fullname[1024], *p;
      FILE *fp;
      ULONG errs = 0;
      BOOL first = FALSE;
      size_t c;
      CHAR *moder = "r";

#     ifdef FORTIFY
      Fortify_BecomeOwner(mp1);
#     endif

      fp = xfsopen((CHAR *)mp1, moder, SH_DENYNO, pszSrcFile, __LINE__, TRUE);
      if (fp) {
	while (!feof(fp)) {
	  // Avoid too much noise if collecting from binary file - oops
	  if (!fgets(fullname, sizeof(fullname), fp)) {
	    if (ferror(fp))
	      Runtime_Error(pszSrcFile, __LINE__, "fgets");
	    break;
	  }

	  c = strlen(fullname);
	  if (c + 1 >= sizeof(fullname))
	    errs++;
	  else if (!c || (fullname[c - 1] != '\n' && fullname[c - 1] != '\r'))
	    errs++;
	  else {
	    bstripcr(fullname);

	    if (*fullname == '\"') {
	      memmove(fullname, fullname + 1, strlen(fullname) + 1);
	      lstrip(fullname);
	      p = strchr(fullname, '\"');
	      if (p)
		*p = 0;
	      rstrip(fullname);
	    }
	    else {
	      p = strchr(fullname, ' ');
	      if (p)
		*p = 0;
	    }
	    // fullname now contains name of file to collect
	    DosError(FERR_DISABLEHARDERR);
	    if (FindCnrRecord(dcd->hwndCnr,
			      fullname,
			      NULL,
			      FALSE,
			      FALSE,
			      TRUE)) {
	      pci = UpdateCnrRecord(dcd->hwndCnr, fullname, FALSE, dcd);
	      if (Filter((PMINIRECORDCORE) pci, (PVOID) & dcd->mask)) {
		pci->rc.flRecordAttr &= ~CRA_FILTERED;
		WinSendMsg(dcd->hwndCnr, CM_INVALIDATERECORD, MPVOID,
			   MPFROM2SHORT(0, CMA_REPOSITION | CMA_ERASE));
	      }
	    }
	    else if (IsFullName(fullname) &&
		!IsRoot(fullname) &&
		!DosQueryPathInfo(fullname,
				  FIL_QUERYEASIZEL,
				  &fs4,
				  sizeof(fs4))) {
	      // collect it
	      pci = WinSendMsg(dcd->hwndCnr,
			       CM_ALLOCRECORD,
			       MPFROMLONG(EXTRA_RECORD_BYTES),
			       MPFROMLONG(1));
	      if (pci) {
		dcd->ullTotalBytes += FillInRecordFromFSA(dcd->hwndCnr, pci,
							  fullname,
							  &fs4, FALSE, NULL, dcd);
		memset(&ri, 0, sizeof(RECORDINSERT));
		ri.cb = sizeof(RECORDINSERT);
		ri.pRecordOrder = (PRECORDCORE) CMA_END;
		ri.pRecordParent = (PRECORDCORE) 0;
		ri.zOrder = (ULONG) CMA_TOP;
		ri.cRecordsInsert = 1;
		ri.fInvalidateRecord = TRUE;
		WinSendMsg(dcd->hwndCnr, CM_INSERTRECORD,
			   MPFROMP(pci), MPFROMP(&ri));
	      }
	    }
	    else
	      errs++;
	  }
	  if (errs > (first ? 0 : 50)) {
	    /* prevent runaway on bad file */
	    APIRET ret = saymsg(MB_YESNO, dcd->hwndCnr,
				GetPString(IDS_COLLECTNOLISTHDRTEXT),
				GetPString(IDS_COLLECTNOLISTTEXT),
				(CHAR *)mp1);

	    if (ret == MBID_NO)
	      break;
	    if (!first)
	      errs = 0;
	    else
	      first = FALSE;
	  }
	}				// while not eof
	fclose(fp);
      }
      free(mp1);
    }
#   ifdef FORTIFY
    Fortify_LeaveScope();
#   endif
    return 0;

  case UM_SELECT:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case IDM_SELECTLIST:
	{
	  CHAR filename[CCHMAXPATH], *p, *pp;

	  strcpy(filename, PCSZ_STARDOTLST);
	  size = CCHMAXPATH;
	  PrfQueryProfileData(fmprof, appname, "SaveToListName", filename,
			      &size);
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
	  if (WinDlgBox(HWND_DESKTOP, dcd->hwndCnr, PickMaskDlgProc,
			FM3ModHandle, MSK_FRAME, MPFROMP(&mask))) {
	    if (SHORT1FROMMP(mp1) == IDM_SELECTMASK)
	      SelectAll(dcd->hwndCnr, TRUE, TRUE, mask.szMask, mask.szText,
			FALSE);
	    else
	      DeselectAll(dcd->hwndCnr, TRUE, TRUE, mask.szMask, mask.szText,
			  FALSE);
	  }
	}

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
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif
    if (mp1) {
#     ifdef FORTIFY
      Fortify_BecomeOwner(mp1);
#     endif
      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (dcd) {
	WORKER *wk;
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
	    FreeListInfo((LISTINFO *) mp1);
	  }
	}
      }
    }
#   ifdef FORTIFY
    DosSleep(1);			// Let receiver take ownership
    Fortify_LeaveScope();
#   endif
    return 0;

  case UM_ACTION:
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif
    if (mp1) {
#     ifdef FORTIFY
      Fortify_BecomeOwner(mp1);
#     endif
      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (dcd) {
	WORKER *wk;
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
	    free(wk);
	    FreeListInfo((LISTINFO *) mp1);
	  }
	}
      }
    }
#   ifdef FORTIFY
    DosSleep(1);			// Let receiver take ownership
    Fortify_LeaveScope();
#   endif
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    break;

  case WM_DESTROY:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
      INT x;

      dcd->stopflag = 1;
      // Allow other threads to honor stop request
      for (x = 0; x < 100 && dcd->amextracted; x++)
	DosSleep(10);
      if (dcd->amextracted)
	Runtime_Error(pszSrcFile, __LINE__, "still busy");
      WinSendMsg(dcd->hwndCnr, UM_CLOSE, MPVOID, MPVOID);
      FreeList(dcd->lastselection);
      WinSetWindowPtr(dcd->hwndCnr, QWL_USER, NULL);	// 13 Apr 10 SHL Set NULL before freeing dcd
      free(dcd);
#     ifdef FORTIFY
      Fortify_LeaveScope();
#     endif
    }
    DosPostEventSem(CompactSem);
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CollectorCnrWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				     MPARAM mp2)
{
  DIRCNRDATA *dcd = INSTDATA(hwnd);
  ULONG size;

  static INT savedSortFlags;

  switch (msg) {
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
      case VK_DELETE:
	if ((shiftstate & KC_CTRL) == KC_CTRL)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_PERMDELETE, 0), MPVOID);
	else if ((shiftstate & KC_SHIFT) == KC_SHIFT)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_SAVETOCLIP, 0), MPVOID);
	else
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_DELETE, 0), MPVOID);
	break;
      }
    }
    if (shiftstate || fNoSearch)
      break;
    if (SHORT1FROMMP(mp1) & KC_CHAR) {
      ULONG thistime, len;
      SEARCHSTRING srch;
      PCNRITEM pci;

      if (!dcd)
	break;
      switch (SHORT1FROMMP(mp2)) {
      case '\x1b':
      case '\r':
      case '\n':
	dcd->lasttime = 0;
	*dcd->szCommonName = 0;
	break;
      default:
	thistime = WinQueryMsgTime(WinQueryAnchorBlock(hwnd));
	if (thistime > dcd->lasttime + 1250)
	  *dcd->szCommonName = 0;
	dcd->lasttime = thistime;
	if (SHORT1FROMMP(mp2) == ' ' && !dcd->szCommonName)
	  break;
      KbdRetry:
	len = strlen(dcd->szCommonName);
	if (len >= CCHMAXPATH - 1) {
	  *dcd->szCommonName = 0;
	  len = 0;
	}
	dcd->szCommonName[len] = toupper(SHORT1FROMMP(mp2));
	dcd->szCommonName[len + 1] = 0;
	memset(&srch, 0, sizeof(SEARCHSTRING));
	srch.cb = sizeof(SEARCHSTRING);
	srch.pszSearch = dcd->szCommonName;
	srch.fsPrefix = TRUE;
	srch.fsCaseSensitive = FALSE;
	srch.usView = CV_ICON;
	pci = WinSendMsg(hwnd, CM_SEARCHSTRING, MPFROMP(&srch),
			 MPFROMLONG(CMA_FIRST));
	if (pci && (INT) pci != -1) {
	  USHORT attrib = CRA_CURSORED;

	  /* make found item current item */
	  if (!stricmp(pci->pszFileName, dcd->szCommonName))
	    attrib |= CRA_SELECTED;
	  WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		     MPFROM2SHORT(TRUE, attrib));
	  /* make sure that record shows in viewport */
	  ShowCnrRecord(hwnd, (PMINIRECORDCORE) pci);
	  return (MRESULT) TRUE;
	}
	else {
	  if (SHORT1FROMMP(mp2) == ' ') {
	    dcd->szCommonName[len] = 0;
	    break;
	  }
	  *dcd->szCommonName = 0;
	  dcd->lasttime = 0;
	  if (len)			// retry as first letter if no match

	    goto KbdRetry;
	}
	break;
      }
    }
    break;

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
      if (WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnri),
		     MPFROMLONG(sizeof(CNRINFO)))) {
	if (cnri.flWindowAttr & CV_DETAIL)
	  PrfWriteProfileData(fmprof, appname, "CollectorCnrSplitBar",
			      (PVOID) & cnri.xVertSplitbar, sizeof(LONG));
      }
    }
    break;

  case WM_PRESPARAMCHANGED:
    PresParamChanged(hwnd, PCSZ_COLLECTOR, mp1, mp2);
    break;

  case UM_COMPARE:
    if (dcd && mp1 && mp2) {
      COMPARE *cmp;
      CHAR *leftdir = (CHAR *)mp1, *rightdir = (CHAR *)mp2;

      if (!IsFile(leftdir) && !IsFile(rightdir)) {
	cmp = xmallocz(sizeof(COMPARE), pszSrcFile, __LINE__);
	if (cmp) {
	  cmp->size = sizeof(COMPARE);
	  strcpy(cmp->leftdir, leftdir);
	  strcpy(cmp->rightdir, rightdir);
	  cmp->hwndParent = dcd->hwndParent;
	  cmp->dcd.hwndParent = dcd->hwndParent;
	  WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, CompareDlgProc,
		    FM3ModHandle, COMP_FRAME, MPFROMP(cmp));
	}
      }
    }
    return 0;

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
    /*
     * put name of our window on status line
     */
    if (dcd && hwndStatus && mp2) {
      PCNRITEM pci = NULL;

      if (fAutoView && hwndMain) {
	pci = WinSendMsg(hwnd,
			 CM_QUERYRECORDEMPHASIS,
			 MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	if (pci && (INT) pci != -1 &&
	    (!(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_SLOW)))
	  WinSendMsg(hwndMain, UM_LOADFILE, MPFROMP(pci->pszFileName), MPVOID);
	else
	  WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
      }
      if (dcd->amextracted)
	WinSetWindowText(hwndStatus2, (CHAR *) GetPString(IDS_INSEEKSCANTEXT));	// Say working
      WinSendMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    }
    break;

  case UM_RESCAN:
    if (dcd) {
      CNRINFO cnri;
      CHAR s[CCHMAXPATH + 69], tb[81], tf[81], szDate[DATE_BUF_BYTES] *p;
      PCNRITEM pci = NULL;

      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnri),
		 MPFROMLONG(sizeof(CNRINFO)));
      dcd->totalfiles = cnri.cRecords;
      commafmt(tf, sizeof(tf), dcd->totalfiles);
      CommaFmtULL(tb, sizeof(tb), dcd->ullTotalBytes, ' ');
      sprintf(s, "%s / %s", tf, tb);
      WinSetDlgItemText(dcd->hwndClient, DIR_TOTALS, s);

      commafmt(tf, sizeof(tf), dcd->selectedfiles);
      CommaFmtULL(tb, sizeof(tb), dcd->selectedbytes, ' ');
      sprintf(s, "%s / %s", tf, tb);
      WinSetDlgItemText(dcd->hwndClient, DIR_SELECTED, s);

      if (hwndStatus &&
	  dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent)) {
	if (hwndMain) {
	  pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1)
	    PostMsg(hwndMain, UM_LOADFILE, MPFROMP(pci->pszFileName), MPVOID);
	  else
	    PostMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	}
	if (!dcd->amextracted) {
	  if (!fMoreButtons) {
	    sprintf(s, " %s%s%s%s",
		    GetPString(IDS_COLLECTORTEXT),
		    *dcd->mask.szMask || dcd->mask.antiattr ||
		      dcd->mask.attrFile != ALLATTRS ? "  (" : NullStr,
		    *dcd->mask.szMask ?
		      dcd->mask.szMask :
		      dcd->mask.antiattr ||
		       dcd->mask.attrFile != ALLATTRS ?
			GetPString(IDS_ATTRTEXT) : NullStr,
		    *dcd->mask.szMask || dcd->mask.antiattr ||
		      dcd->mask.attrFile != ALLATTRS ?
		      ")" : NullStr);
	  }
	  else
	    strcpy(s, GetPString(IDS_COLLECTORTEXT));
	  WinSetWindowText(hwndStatus, s);
	}
	if (!pci)
	  pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	if (pci && (INT) pci != -1) {
	  BOOL fStatus2Used = FALSE;

	  if (fSplitStatus && hwndStatus2) {
	    if (pci->attrFile & FILE_DIRECTORY)
	      p = pci->pszFileName;
	    else {
	      if (!pci->pszFileName)
		Runtime_Error(pszSrcFile, __LINE__, "pci->pszFileName NULL for %p", pci);
	      p = strrchr(pci->pszFileName, '\\');
	      if (p) {
		if (*(p + 1))
		  p++;
		else
		  p = pci->pszFileName;
	      }
	      else
		p = pci->pszFileName;
	    }
	    CommaFmtULL(tb, sizeof(tb), pci->cbFile + pci->easize, ' ');
	    if (!fMoreButtons) {
	      DateFormat(szDate, pci->date);
	      sprintf(s, " %s  %s %02u%s%02u%s%02u  [%s]  %s",
		      tb, szDate, pci->time.hours, TimeSeparator, pci->time.minutes,
		      TimeSeparator, pci->time.seconds, pci->pszDispAttr, p);
	    }
	    else {
	      if (pci->cbFile + pci->easize > 1024)
		CommaFmtULL(tf, sizeof(tf), pci->cbFile + pci->easize, 'K');
	      else
		*tf = 0;
	      sprintf(s, GetPString(IDS_STATUSSIZETEXT),
		      tb, *tf ? " (" : NullStr, tf, *tf ? ")" : NullStr);
	    }
	    WinSetWindowText(hwndStatus2, s);
	    fStatus2Used = TRUE;
	  }
	  if (fMoreButtons) {
	    WinSetWindowText(hwndName, pci->pszFileName);
	    DateFormat(szDate, pci->date);
	    sprintf(s, "%s %02u%s%02u%s%02u",
		    szDate, pci->time.hours, TimeSeparator, pci->time.minutes,
		    TimeSeparator, pci->time.seconds);
	    WinSetWindowText(hwndDate, s);
	    WinSetWindowText(hwndAttr, pci->pszDispAttr);
	  }
	  if (dcd->amextracted && hwndStatus2 && !fStatus2Used)
	    WinSetWindowText(hwndStatus2, (CHAR *) GetPString(IDS_INSEEKSCANTEXT));	// Say working
	}
	else {
	  if (hwndStatus2) {
	    if (dcd->amextracted)
	      WinSetWindowText(hwndStatus2, (CHAR *) GetPString(IDS_INSEEKSCANTEXT));	// Say working
	    else
	      WinSetWindowText(hwndStatus2, NullStr);
	  }
	  if (fMoreButtons) {
	    WinSetWindowText(hwndName, NullStr);
	    WinSetWindowText(hwndDate, NullStr);
	    WinSetWindowText(hwndAttr, NullStr);
	  }
	}
      }
    }
    return 0;

  case UM_CONTAINER_FILLED:
    if (!fAlertBeepOff)
      DosBeep(1000, 50);		// Wake up user?
    WinSendMsg(hwnd,
	       CM_INVALIDATERECORD,
	       MPVOID, MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
    disable_menuitem(WinWindowFromID(WinQueryWindow(hwndMain, QW_PARENT),
				     FID_MENU), IDM_GREP, FALSE);
    disable_menuitem(TreeMenu, IDM_GREP, FALSE);
    disable_menuitem(DirMenu, IDM_GREP, FALSE);
    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    if (dcd) {
      dcd->stopflag = 0;
      dcd->amextracted = FALSE;		// Say not busy
      if (dcd->namecanchange) {
	if (!PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID))
	  WinSendMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      }
      else
	WinSetWindowPos(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				       QW_PARENT),
			HWND_TOP,
			0, 0, 0, 0, SWP_SHOW | SWP_RESTORE | SWP_ZORDER);
    }
    return 0;

  case UM_SETUP:
    if (dcd) {
      if (!dcd->hwndObject) {
	/* first time through -- set things up */

	CNRINFO cnri;

	RestorePresParams(hwnd, PCSZ_COLLECTOR);
	LoadDetailsSwitches(PCSZ_COLLECTOR, &dcd->ds, FALSE);

	dcd->amextracted = FALSE;	// Say not busy
	dcd->stopflag = 0;
	memset(&cnri, 0, sizeof(CNRINFO));
	cnri.cb = sizeof(CNRINFO);
	WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnri),
		   MPFROMLONG(sizeof(CNRINFO)));
	cnri.cyLineSpacing = 0;
	cnri.cxTreeIndent = 12;

	cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT | CV_DETAIL));
	cnri.flWindowAttr |= (CV_NAME | CA_DETAILSVIEWTITLES |
			      CV_MINI | CV_FLOW);
	cnri.pSortRecord = (PVOID) SortCollectorCnr;

	size = sizeof(ULONG);
	PrfQueryProfileData(fmprof, appname, "CollectorflWindowAttr",
			    (PVOID) & cnri.flWindowAttr, &size);
	size = sizeof(MASK);
	if (PrfQueryProfileSize(fmprof, appname, "CollectorFilter", &size) &&
	    size) {
	  PrfQueryProfileData(fmprof, appname, "CollectorFilter", &dcd->mask,
			      &size);
	  SetMask(NULL, &dcd->mask);
	}
	else {
	  dcd->mask.attrFile = (FILE_NORMAL | FILE_READONLY |
				FILE_DIRECTORY | FILE_HIDDEN |
				FILE_SYSTEM | FILE_ARCHIVED);
	  dcd->mask.antiattr = 0;
	}

	*(dcd->mask.prompt) = 0;

	cnri.flWindowAttr |= CV_FLOW;
	cnri.flWindowAttr &= (~(CA_MIXEDTARGETEMPH | CA_ORDEREDTARGETEMPH));
	dcd->flWindowAttr = cnri.flWindowAttr;
	WinSendMsg(hwnd, CM_SETCNRINFO, MPFROMP(&cnri),
		   MPFROMLONG(CMA_FLWINDOWATTR | CMA_LINESPACING |
			      CMA_CXTREEINDENT | CMA_PSORTRECORD));
	SetCnrCols(hwnd, FALSE);
	AdjustCnrColsForPref(hwnd, NULL, &dcd->ds, FALSE);

	/* fix splitbar for collector container */
	cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 32;
	size = sizeof(LONG);
	PrfQueryProfileData(fmprof, appname, "CollectorCnrSplitBar",
			    &cnri.xVertSplitbar, &size);
	if (cnri.xVertSplitbar <= 0)
	  cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 32;
	WinSendMsg(hwnd, CM_SETCNRINFO, MPFROMP(&cnri),
		   MPFROMLONG(CMA_XVERTSPLITBAR));

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
	  DosSleep(32);			// Let object window get started
      }
      SayFilter(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				DIR_FILTER), &dcd->mask, FALSE);
      SaySort(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
			      DIR_SORT), CollectorsortFlags, FALSE);
      SayView(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
			      DIR_VIEW), dcd->flWindowAttr);
    }
    else {
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      return 0;
    }
    return 0;

  case WM_MENUEND:
    if (dcd) {
      HWND hwndMenu = (HWND) mp2;

      if (hwndMenu == CollectorCnrMenu || hwndMenu == CollectorFileMenu ||
	  hwndMenu == CollectorDirMenu) {
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
      if (mp1 && !IsFile((CHAR *)mp1))
	OpenDirCnr(HWND_DESKTOP, hwndMain, dcd->hwndFrame, FALSE, (PSZ) mp1);
      else if (mp1 && IsFile(mp1) == 1 &&
	       CheckDriveSpaceAvail(ArcTempRoot, ullDATFileSpaceNeeded, ullTmpSpaceNeeded) != 2)
	StartArcCnr(HWND_DESKTOP,
		    dcd->hwndFrame, (CHAR *)mp1, 4, (ARC_TYPE *) mp2);
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
	  PortholeInit((HWND) WinSendMsg(dcd->hwndClient, wmsg, MPVOID,
					 MPVOID), mp1, mp2);
	}
	break;
      }
    }
    break;

  case UM_INITMENU:
  case WM_INITMENU:
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case IDM_VIEWSMENU:
	SetViewMenu((HWND) mp2, dcd->flWindowAttr);
	WinEnableMenuItem((HWND) mp2, IDM_RESELECT,
			  (dcd->lastselection != NULL));
	CopyPresParams((HWND) mp2, hwnd);
	break;

      case IDM_DETAILSSETUP:
	SetDetailsSwitches((HWND) mp2, &dcd->ds);
	break;

      case IDM_COMMANDSMENU:
	SetupCommandMenu((HWND) mp2, hwnd);
	break;

      case IDM_SORTSUBMENU:
	SetSortChecks((HWND) mp2, CollectorsortFlags);
	break;
      }
      dcd->hwndLastMenu = (HWND) mp2;
    }
    if (msg == WM_INITMENU)
      break;
    return 0;

  case UM_COLLECTFROMFILE:
    if (mp1) {
#     ifdef FORTIFY
      Fortify_EnterScope();
      Fortify_BecomeOwner(mp1);
#     endif
      if (!dcd) {
	Runtime_Error(pszSrcFile, __LINE__, NULL);
	free(mp1);
      }
      else {
	if (!PostMsg(dcd->hwndObject, UM_COLLECTFROMFILE, mp1, mp2)) {
	  Runtime_Error(pszSrcFile, __LINE__, PCSZ_POSTMSG);
	  free(mp1);
	}
      }
#     ifdef FORTIFY
      DosSleep(1);			// Let receiver take ownership
      Fortify_LeaveScope();
#     endif
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
      AddNote((CHAR *)mp2);
    return 0;

  case WM_COMMAND:
    DosError(FERR_DISABLEHARDERR);
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case IDM_SETTARGET:
	SetTargetDir(hwnd, FALSE);
	break;

      case IDM_CONTEXTMENU:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  PostMsg(hwnd, WM_CONTROL, MPFROM2SHORT(COLLECTOR_CNR,
						 CN_CONTEXTMENU),
		  MPFROMP(pci));
	}
	break;

      case IDM_SHOWALLFILES:
	{
	  PCNRITEM pci;

	  pci = WinSendMsg(hwnd,
			   CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1) {
	    static CHAR dirname[CCHMAXPATH];

	    strcpy(dirname, pci->pszFileName);
	    MakeValidDir(dirname);
	    StartSeeAll(HWND_DESKTOP, FALSE, dirname);
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
	QuickPopup(hwnd, dcd,
		   CheckMenu(hwnd, &CollectorCnrMenu, COLLECTORCNR_POPUP),
		   IDM_SELECTSUBMENU);
	break;

      case IDM_SHOWSORT:
	QuickPopup(hwnd, dcd,
		   CheckMenu(hwnd, &CollectorCnrMenu, COLLECTORCNR_POPUP),
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
	    DefaultViewKeys(hwnd, dcd->hwndFrame, dcd->hwndParent, &swp,
			    pci->pszFileName);
	  }
	}
	break;

      case IDM_SEEALL:
	StartSeeAll(HWND_DESKTOP, FALSE, NULL);
	break;

      case IDM_COLLECTSELECT:
	{
	  CHAR filename[CCHMAXPATH], *p, *pp;

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
	  if (insert_filename(hwnd, filename, FALSE, FALSE)) {
#	    ifdef FORTIFY
	    Fortify_EnterScope();
#	    endif
	    p = xstrdup(filename, pszSrcFile, __LINE__);
	    if (p) {
	      if (!PostMsg(hwnd, UM_COLLECTFROMFILE, MPFROMP(p), MPVOID))
		free(p);
	    }
#	    ifdef FORTIFY
	    DosSleep(1);		// Let receiver take ownership
	    Fortify_LeaveScope();
#	    endif
	  }
	}
	break;

      case IDM_COLLECTORVIEWSETTINGS:
	if (!ParentIsDesktop(dcd->hwndParent, dcd->hwndParent))
	  PostMsg(dcd->hwndParent, msg, MPFROMLONG(IDM_COLLECTORVIEWSETTINGS), mp2);
	else {
	  WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    CfgDlgProc,
		    FM3ModHandle,
		    CFG_FRAME,
		    MPFROMLONG(IDM_COLLECTORVIEWSETTINGS));
	}
	break;

      case IDM_RESELECT:
	SelectList(hwnd, FALSE, FALSE, FALSE, NULL, NULL, dcd->lastselection);
	break;

      case IDM_WALKDIR:
      case IDM_OPENDIRWINDOW:
      case IDM_OPENDIRICON:
      case IDM_OPENDIRDETAILS:
      case IDM_OPENDIRTREE:
	{
	  CHAR newpath[CCHMAXPATH];
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1) {
	    strcpy(newpath, pci->pszFileName);
	    MakeValidDir(newpath);
	  }
	  else
	    strcpy(newpath, pFM2SaveDirectory);
	  if (*newpath) {
	    switch (SHORT1FROMMP(mp1)) {
	    case IDM_WALKDIR:
	      WinDlgBox(HWND_DESKTOP, dcd->hwndParent, WalkAllDlgProc,
			FM3ModHandle, WALK_FRAME, MPFROMP(newpath));
	      break;
	    case IDM_OPENDIRWINDOW:
	      WinSendMsg(hwnd, UM_OPENWINDOWFORME, MPFROMP(newpath), MPVOID);
	      break;
	    case IDM_OPENDIRICON:
	      OpenObject(newpath, PCSZ_ICON, hwnd);
	      break;
	    case IDM_OPENDIRDETAILS:
	      OpenObject(newpath, Details, hwnd);
	      break;
	    case IDM_OPENDIRTREE:
	      OpenObject(newpath, PCSZ_TREE, hwnd);
	      break;
	    default:
	      break;
	    }
	  }
	}
	break;

      case IDM_HELP:
	if (hwndHelp)
	  WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_COLLECT, 0),
		     MPFROMSHORT(HM_RESOURCEID));
	break;

      case IDM_SORTNONE:
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
	savedSortFlags = CollectorsortFlags;
	CollectorsortFlags &= (SORT_REVERSE | SORT_DIRSFIRST | SORT_DIRSLAST);
      case IDM_SORTDIRSFIRST:
      case IDM_SORTDIRSLAST:
      case IDM_SORTREVERSE:
	switch (SHORT1FROMMP(mp1)) {
	case IDM_SORTSUBJECT:
	  CollectorsortFlags |= SORT_SUBJECT;
	  break;
	case IDM_SORTNONE:
	  CollectorsortFlags |= SORT_NOSORT;
	  break;
	case IDM_SORTSMARTNAME:
	  if (~savedSortFlags & SORT_FILENAME)
	    CollectorsortFlags |= SORT_FILENAME;
	  break;
	case IDM_SORTFILENAME:
	  CollectorsortFlags |= SORT_FILENAME;
	  break;
	case IDM_SORTSIZE:
	  CollectorsortFlags |= SORT_SIZE;
	  break;
	case IDM_SORTEASIZE:
	  CollectorsortFlags |= SORT_EASIZE;
	  break;
	case IDM_SORTFIRST:
	  CollectorsortFlags |= SORT_FIRSTEXTENSION;
	  break;
	case IDM_SORTLAST:
	  CollectorsortFlags |= SORT_LASTEXTENSION;
	  break;
	case IDM_SORTLWDATE:
	  CollectorsortFlags |= SORT_LWDATE;
	  break;
	case IDM_SORTLADATE:
	  CollectorsortFlags |= SORT_LADATE;
	  break;
	case IDM_SORTCRDATE:
	  CollectorsortFlags |= SORT_CRDATE;
	  break;
	case IDM_SORTDIRSFIRST:
	  if (CollectorsortFlags & SORT_DIRSFIRST)
	    CollectorsortFlags &= (~SORT_DIRSFIRST);
	  else {
	    CollectorsortFlags |= SORT_DIRSFIRST;
	    CollectorsortFlags &= (~SORT_DIRSLAST);
	  }
	  break;
	case IDM_SORTDIRSLAST:
	  if (CollectorsortFlags & SORT_DIRSLAST)
	    CollectorsortFlags &= (~SORT_DIRSLAST);
	  else {
	    CollectorsortFlags |= SORT_DIRSLAST;
	    CollectorsortFlags &= (~SORT_DIRSFIRST);
	  }
	  break;
	case IDM_SORTREVERSE:
	  if (CollectorsortFlags & SORT_REVERSE)
	    CollectorsortFlags &= (~SORT_REVERSE);
	  else
	    CollectorsortFlags |= SORT_REVERSE;
	  break;
	}
	PrfWriteProfileData(fmprof, appname, "CollectorSort",
			    &CollectorsortFlags, sizeof(INT));
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(SortCollectorCnr), MPVOID);
	SaySort(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				DIR_SORT), CollectorsortFlags, FALSE);
	break;

      case IDM_COLLECTFROMCLIP:
	{
	  LISTINFO *li;
#	  ifdef FORTIFY
	  Fortify_EnterScope();
#	  endif
	  li = xmallocz(sizeof(LISTINFO), pszSrcFile, __LINE__);
	  if (li) {
	    li->list = ListFromClipboard(hwnd);
	    if (!li->list || !li->list[0])
	      FreeListInfo(li);
	    else {
	      li->type = IDM_COLLECT;
	      if (!PostMsg(dcd->hwndObject, UM_COLLECT, MPFROMP(li), MPVOID))
		FreeListInfo(li);
	    }
	  }
#	  ifdef FORTIFY
	  DosSleep(1);			// Let receiver take ownership
	  Fortify_LeaveScope();
#	  endif
	}
	break;

      case IDM_REMOVE:
	if (fAutoView && hwndMain)
	  PostMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	dcd->suspendview = 1;
	RemoveAll(hwnd, &dcd->ullTotalBytes, &dcd->totalfiles);
	dcd->suspendview = 0;
	PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	break;

      case IDM_CLEARCNR:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) WinSendMsg(hwnd,
				      CM_QUERYRECORD,
				      MPVOID,
				      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
	  if (pci && (INT) pci != -1) {
	    RemoveCnrItems(hwnd, NULL, 0, CMA_FREE | CMA_INVALIDATE);
	    dcd->ullTotalBytes = dcd->selectedbytes = dcd->selectedfiles =
	      dcd->totalfiles = 0;
	    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	  }
	}
	break;

      case DID_CANCEL:
	if (dcd->amextracted)
	  dcd->stopflag = 1;		// Request cancel
	break;

      case IDM_COLLECTOR:
	if (mp2) {
	  LISTINFO *li;
#	  ifdef FORTIFY
	  Fortify_EnterScope();
#	  endif
	  li = xmallocz(sizeof(LISTINFO), pszSrcFile, __LINE__);
	  if (li) {
	    li->list = mp2;
	    if (!li->list || !li->list[0])
	      FreeListInfo(li);
	    else {
	      li->type = IDM_COLLECT;
	      if (!PostMsg(dcd->hwndObject, UM_COLLECT, MPFROMP(li), MPVOID))
		FreeListInfo(li);
	    }
	  }
	  else
	    FreeList(mp2);
#	  ifdef FORTIFY
	  DosSleep(1);			// Let receiver take ownership
	  Fortify_LeaveScope();
#	  endif
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

      case IDM_GREP:
      case UM_GREP:
	if (dcd->amextracted) {
	  saymsg(MB_OK | MB_ICONASTERISK,
		 hwnd,
		 GetPString(IDS_WARNINGTEXT),
		 "Collector busy - please try again later");
	}
	else {
	  GREPINFO *GrepInfo;

	  GrepInfo = xmallocz(sizeof(GREPINFO), pszSrcFile, __LINE__);
	  if (GrepInfo) {
	    GrepInfo->hwnd = &hwnd;
	    if (msg == UM_GREP && mp2)
	      GrepInfo->szGrepPath = mp2;
	    if (WinDlgBox(HWND_DESKTOP, hwnd, GrepDlgProc,
			  FM3ModHandle, GREP_FRAME, (PVOID) GrepInfo)) {
	      free(GrepInfo);
	      dcd->amextracted = TRUE;	// Say busy scanning
	      disable_menuitem(WinWindowFromID
			       (WinQueryWindow(hwndMain, QW_PARENT), FID_MENU),
			       IDM_GREP, TRUE);
	      disable_menuitem(TreeMenu, IDM_GREP, TRUE);
	      disable_menuitem(DirMenu, IDM_GREP, TRUE);
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	    else
	      free(GrepInfo);
	  }
	}
	break;

      case IDM_RESORT:
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(SortCollectorCnr), MPVOID);
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
			FM3ModHandle, MSK_FRAME, MPFROMP(&dcd->mask))) {
	    size = sizeof(MASK);
	    PrfWriteProfileData(fmprof, appname, "CollectorFilter",
				&dcd->mask, size);
	    dcd->suspendview = 1;
	    WinSendMsg(hwnd, CM_FILTER, MPFROMP(Filter), MPFROMP(&dcd->mask));
	    dcd->suspendview = 0;
	    if (fAutoView && hwndMain) {
	      pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
			       MPFROMLONG(CMA_FIRST),
			       MPFROMSHORT(CRA_CURSORED));
	      if (pci && (INT) pci != -1 &&
		  (!(driveflags[toupper(*pci->pszFileName) - 'A'] &
		     DRIVE_SLOW)))
		WinSendMsg(hwndMain, UM_LOADFILE, MPFROMP(pci->pszFileName),
			   MPVOID);
	      else
		WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	    }
	    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	  }
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
	PostMsg(dcd->hwndObject, UM_RESCAN, MPVOID, MPVOID);
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
	AdjustDetailsSwitches(hwnd, dcd->hwndLastMenu,
			      SHORT1FROMMP(mp1), NULL,
			      PCSZ_COLLECTOR, &dcd->ds, FALSE);
	break;

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
	  case IDM_ICON:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME));
	    cnri.flWindowAttr |= CV_ICON;
	    break;
	  case IDM_NAME:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME));
	    cnri.flWindowAttr |= CV_NAME;
	    break;
	  case IDM_TEXT:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME));
	    cnri.flWindowAttr |= CV_TEXT;
	    break;
	  case IDM_DETAILS:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME));
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
	  PrfWriteProfileData(fmprof, appname, "CollectorflWindowAttr",
			      &cnri.flWindowAttr, sizeof(ULONG));
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

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1)
	    WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, DirSizeProc, FM3ModHandle,
		      DSZ_FRAME, pci->pszFileName);
	}
	break;

      case IDM_MKDIR:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  PMMkDir(dcd->hwndParent, (pci && (INT) pci != -1) ?
		  pci->pszFileName : NULL, FALSE);
	}
	break;

      case IDM_DOITYOURSELF:
      case IDM_UPDATE:
      case IDM_COLLECTFROMFILE:
      case IDM_OPENWINDOW:
      case IDM_OPENSETTINGS:
      case IDM_OPENDEFAULT:
      case IDM_OPENICON:
      case IDM_OPENDETAILS:
      case IDM_OPENTREE:
      case IDM_OBJECT:
      case IDM_SHADOW:
      case IDM_SHADOW2:
      case IDM_DELETE:
      case IDM_PERMDELETE:
      case IDM_PRINT:
      case IDM_ATTRS:
      case IDM_INFO:
      case IDM_COPY:
      case IDM_MOVE:
      case IDM_WPSCOPY:
      case IDM_WPSMOVE:
      case IDM_COPYPRESERVE:
      case IDM_MOVEPRESERVE:
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
      case IDM_UUDECODE:
      case IDM_MERGE:
	{
	  LISTINFO *li;
	  ULONG action = UM_ACTION;
#	  ifdef FORTIFY
	  Fortify_EnterScope();
#	  endif
	  li = xmallocz(sizeof(LISTINFO), pszSrcFile, __LINE__);
	  if (li) {
	    li->type = SHORT1FROMMP(mp1);
	    li->hwnd = hwnd;
	    li->list = BuildList(hwnd);
	    if (li->list) {
	      switch (SHORT1FROMMP(mp1)) {
	      case IDM_DOITYOURSELF:
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
	      case IDM_OBJECT:
	      case IDM_VIEW:
	      case IDM_VIEWTEXT:
	      case IDM_VIEWBINARY:
	      case IDM_EDIT:
	      case IDM_EDITTEXT:
	      case IDM_EDITBINARY:
	      case IDM_MCIPLAY:
	      case IDM_UPDATE:
	      case IDM_INFO:
	      case IDM_EAS:
		action = UM_MASSACTION;
		break;
	      }
	      if (li->type == IDM_SHADOW || li->type == IDM_OBJECT ||
		  li->type == IDM_SHADOW2)
		*li->targetpath = 0;
	      if (!PostMsg(dcd->hwndObject, action, MPFROMP(li), MPVOID)) {
		Runtime_Error(pszSrcFile, __LINE__, PCSZ_POSTMSG);
		FreeListInfo(li);
	      }
	      else if (fUnHilite)
		UnHilite(hwnd, TRUE, &dcd->lastselection, dcd->ulItemsToUnHilite);
	    }
	    else
	      free(li);
	  }
#	  ifdef FORTIFY
	  Fortify_LeaveScope();
#	  endif
	}
	break;

      default:
	if (!cmdloaded)
	  load_commands();
	if (SHORT1FROMMP(mp1) >= IDM_COMMANDSTART &&
	    SHORT1FROMMP(mp1) < IDM_QUICKTOOLSTART) {
	  INT x;

	  x = SHORT1FROMMP(mp1);	// - IDM_COMMANDSTART;
	  if (x >= 0) {
	    //x++;
	    RunCommand(hwnd, x);
	    if (fUnHilite)
	      UnHilite(hwnd, TRUE, &dcd->lastselection, dcd->ulItemsToUnHilite);
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
	if (pci->attrFile & FILE_DIRECTORY)
	  menuHwnd = CheckMenu(hwnd, &CollectorDirMenu, COLLECTORDIR_POPUP);
	else
	  menuHwnd = CheckMenu(hwnd, &CollectorFileMenu, COLLECTORFILE_POPUP);
      }
      return MRFROMLONG(menuHwnd);
    }

  case WM_CONTROL:
    DosError(FERR_DISABLEHARDERR);
    if (dcd) {
      switch (SHORT2FROMMP(mp1)) {
      case CN_CONTEXTMENU:
	{
	  PCNRITEM pci = (PCNRITEM) mp2;

	  if (pci) {
	    WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		       MPFROM2SHORT(TRUE, CRA_CURSORED));
	    MarkAll(hwnd, FALSE, FALSE, TRUE);
	    if (pci->attrFile & FILE_DIRECTORY)
	      dcd->hwndLastMenu = CheckMenu(hwnd, &CollectorDirMenu,
					    COLLECTORDIR_POPUP);
	    else
	      dcd->hwndLastMenu = CheckMenu(hwnd, &CollectorFileMenu,
					    COLLECTORFILE_POPUP);
	  }
	  else {
	    dcd->hwndLastMenu = CheckMenu(hwnd, &CollectorCnrMenu,
					  COLLECTORCNR_POPUP);
	    if (dcd->hwndLastMenu && !dcd->cnremphasized) {
	      WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
			 MPFROM2SHORT(TRUE, CRA_SOURCE));
	      dcd->cnremphasized = TRUE;
	    }
	  }
	  if (dcd->hwndLastMenu) {
	    if (dcd->hwndLastMenu == CollectorCnrMenu) {
	      SetViewMenu(dcd->hwndLastMenu, dcd->flWindowAttr);
	      SetDetailsSwitches(dcd->hwndLastMenu, &dcd->ds);
	      CopyPresParams(dcd->hwndLastMenu, hwnd);
	      if (dcd->flWindowAttr & CV_MINI)
		WinCheckMenuItem(dcd->hwndLastMenu, IDM_MINIICONS, TRUE);
	      disable_menuitem(dcd->hwndLastMenu, DID_CANCEL,
			       !dcd->amextracted);
	      disable_menuitem(dcd->hwndLastMenu, IDM_GREP, dcd->amextracted);
	    }
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

      case CN_DROPHELP:
	if (mp2) {
	  PDRAGINFO pDInfo;
	  PCNRITEM pci;
	  ULONG numitems;
	  USHORT usOperation;

	  pci = (PCNRITEM) ((PCNRDRAGINFO) mp2)->pRecord;
	  pDInfo = ((PCNRDRAGINFO) mp2)->pDragInfo;
	  if (!DrgAccessDraginfo(pDInfo)) {
	    Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      PCSZ_DRGACCESSDRAGINFO);
	    return 0;
	  }
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
		 &"s"[numitems == 1],
		 (pci) ? NullStr : GetPString(IDS_NOTEXT),
		 (pci) ? NullStr : " ",
		 (pci) ? pci->pszFileName : NullStr,
		 (pci) ? " " : NullStr,
		 GetPString((usOperation == DO_COPY) ?
			    IDS_COPYTEXT :
			    (usOperation == DO_LINK) ?
			    IDS_LINKTEXT : IDS_MOVETEXT));
	}
	return 0;

      case CN_DRAGLEAVE:
	if (mp2) {
	  PDRAGINFO pDInfo;

	  // fixme to know why needed
	  pDInfo = ((PCNRDRAGINFO) mp2)->pDragInfo;
	  DrgAccessDraginfo(pDInfo);	/* Access DRAGINFO */
	  DrgFreeDraginfo(pDInfo);	/* Free DRAGINFO */
	}
	return 0;

      case CN_DRAGAFTER:
      case CN_DRAGOVER:
	if (mp2) {
	  PDRAGITEM pDItem;		/* Pointer to DRAGITEM */
	  PDRAGINFO pDInfo;		/* Pointer to DRAGINFO */
	  PCNRITEM pci;
	  USHORT uso;

	  pci = (PCNRITEM) ((PCNRDRAGINFO) mp2)->pRecord;
	  // if(SHORT1FROMMP(mp1) == CN_DRAGAFTER)
	  //    pci = NULL;
	  pDInfo = ((PCNRDRAGINFO) mp2)->pDragInfo;
	  if (!DrgAccessDraginfo(pDInfo)) {
	    Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      PCSZ_DRGACCESSDRAGINFO);
	    return (MRFROM2SHORT(DOR_NODROP, 0));	/* Drop not valid */
	  }
	  if (pci) {
	    if (pci->rc.flRecordAttr & CRA_SOURCE) {
	      DrgFreeDraginfo(pDInfo);
	      return (MRFROM2SHORT(DOR_NODROP, 0));
	    }
	    uso = pDInfo->usOperation;
	    if (uso == DO_DEFAULT)
	      uso = (fCopyDefault) ? DO_COPY : DO_MOVE;
	    if (!(pci->attrFile & FILE_DIRECTORY)) {
	      if (uso != DO_LINK && uso != DO_MOVE && uso != DO_COPY) {
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
	  pDItem = DrgQueryDragitemPtr(pDInfo,	/* Access DRAGITEM */
				       0);	/* Index to DRAGITEM */
	  if (DrgVerifyRMF(pDItem,	/* Check valid rendering */
			   (CHAR *) DRM_OS2FILE,	/* mechanisms and data */
			   NULL)) {
	    DrgFreeDraginfo(pDInfo);	/* Free DRAGINFO */
	    if (pci) {
	      if (driveflags[toupper(*pci->pszFileName) - 'A'] &
		  DRIVE_NOTWRITEABLE)
		return MRFROM2SHORT(DOR_DROP, DO_LINK);
	      if (toupper(*pci->pszFileName) < 'C')
		return MRFROM2SHORT(DOR_DROP, DO_COPY);
	      return MRFROM2SHORT(DOR_DROP,	/* Return okay to drop */
				  ((fCopyDefault) ? DO_COPY : DO_MOVE));
	    }
	    else
	      return MRFROM2SHORT(DOR_DROP,	/* Return okay to drop */
				  DO_COPY);
	  }
	  DrgFreeDraginfo(pDInfo);	/* Free DRAGINFO */
	}
	return (MRFROM2SHORT(DOR_NODROP, 0));	/* Drop not valid */

      case CN_INITDRAG:
	if (mp2) {
	  BOOL wasemphasized = FALSE;
	  PCNRDRAGINIT pcd = (PCNRDRAGINIT) mp2;
	  PCNRITEM pci;

	  if (pcd) {
	    pci = (PCNRITEM) pcd->pRecord;
	    if (pci) {
	      if (pci->rc.flRecordAttr & CRA_SELECTED)
		wasemphasized = TRUE;
	      if (IsRoot(pci->pszFileName))
		break;
	      if (hwndStatus2)
		WinSetWindowText(hwndStatus2,
				 (CHAR *) GetPString(IDS_DRAGFILEOBJTEXT));
	      if (DoFileDrag(hwnd, dcd->hwndObject, mp2, NULL, NULL, TRUE)) {
		if ((fUnHilite && wasemphasized) || dcd->ulItemsToUnHilite)
		  UnHilite(hwnd, TRUE, &dcd->lastselection, dcd->ulItemsToUnHilite);
	      }
	      if (hwndStatus2)
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	  }
	}
	return 0;

      case CN_DROP:
	if (mp2) {
	  LISTINFO *li;
	  ULONG action = UM_ACTION;

	  li = DoFileDrop(hwnd, NULL, TRUE, mp1, mp2);
	  CheckPmDrgLimit(((PCNRDRAGINFO)mp2)->pDragInfo);
	  if (li) {
	    if (!*li->targetpath) {
	      li->type = IDM_COLLECT;
	      action = UM_COLLECT;
	    }
	    else {
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
	      case DND_WILDCOPY:
		li->type = IDM_WILDCOPY;
		if (*li->targetpath && IsFile(li->targetpath) == 1) {
		  action = UM_MASSACTION;
		  li->type = IDM_ARCHIVE;
		}
		break;
	      case DND_COPY:
		li->type = IDM_COPY;
		if (*li->targetpath && IsFile(li->targetpath) == 1) {
		  action = UM_MASSACTION;
		  li->type = IDM_ARCHIVE;
		}
		break;
	      default:
		if (*li->arcname && li->info) {
		  action = UM_MASSACTION;
		  li->type =
		    (li->type ==
		     DO_MOVE) ? IDM_FAKEEXTRACTM : IDM_FAKEEXTRACT;
		}
		else if (*li->targetpath && IsFile(li->targetpath) == 1) {
		  action = UM_MASSACTION;
		  li->type =
		    (li->type == DO_MOVE) ? IDM_ARCHIVEM : IDM_ARCHIVE;
		}
		else
		  li->type = (li->type == DO_MOVE) ? IDM_MOVE : IDM_COPY;
		break;
	      } // switch
	    } // if !collect
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

      case CN_BEGINEDIT:
      case CN_REALLOCPSZ:
      case CN_ENDEDIT:
	{
	  MRESULT mre;

	  mre = CnrDirectEdit(hwnd, msg, mp1, mp2);
	  if (mre != (MRESULT) - 1)
	    return mre;
	}
	break;

      case CN_EMPHASIS:
	if (mp2) {
	  PNOTIFYRECORDEMPHASIS pre = mp2;
	  PCNRITEM pci;
	  CHAR s[CCHMAXPATH + 91], tb[81], tf[81], szDate[DATE_BUF_BYTES], *p;

	  pci = (PCNRITEM) ((pre) ? pre->pRecord : NULL);
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
	      CommaFmtULL(tb, sizeof(tb), dcd->selectedbytes, ' ');
	      sprintf(s, "%s / %s", tf, tb);
	      WinSetDlgItemText(dcd->hwndClient, DIR_SELECTED, s);
	    }
	  }
	  if (!dcd->suspendview &&
	      WinQueryActiveWindow(dcd->hwndParent) == dcd->hwndFrame) {
	    if (pre->fEmphasisMask & CRA_CURSORED) {
	      if (pci->rc.flRecordAttr & CRA_CURSORED) {
		if (fSplitStatus && hwndStatus2) {
		  if (pci->attrFile & FILE_DIRECTORY)
		    p = pci->pszFileName;
		  else {
		    p = strrchr(pci->pszFileName, '\\');
		    if (p) {
		      if (*(p + 1))
			p++;
		      else
			p = pci->pszFileName;
		    }
		    else
		      p = pci->pszFileName;
		  }
		  CommaFmtULL(tb, sizeof(tb), pci->cbFile + pci->easize, ' ');
		  if (!fMoreButtons) {
		    DateFormat(szDate, pci->date);
		    sprintf(s, " %s  %s %02u%s%02u%s%02u  [%s]  %s",
			    tb, szDate, pci->time.hours, TimeSeparator,
			    pci->time.minutes, TimeSeparator, pci->time.seconds,
			    pci->pszDispAttr, p);
		  }
		  else {
		    if (pci->cbFile + pci->easize > 1024)
		      CommaFmtULL(tf, sizeof(tf), pci->cbFile + pci->easize,
				  ' ');
		    else
		      *tf = 0;
		    sprintf(s, GetPString(IDS_STATUSSIZETEXT),
			    tb,
			    *tf ? " (" : NullStr, tf, *tf ? ")" : NullStr);
		  }
		  WinSetWindowText(hwndStatus2, s);
		}
		if (fMoreButtons) {
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
	  if (!dcd->suspendview && hwndMain &&
	      (pre->fEmphasisMask & CRA_CURSORED) &&
	      (pci->rc.flRecordAttr & CRA_CURSORED) &&
	      WinQueryActiveWindow(dcd->hwndParent) == dcd->hwndFrame)
	    WinSendMsg(hwndMain, UM_LOADFILE,
		       MPFROMP(((fComments
				 || (pci->attrFile & FILE_DIRECTORY) ==
				 0) ? pci->pszFileName : NULL)), MPVOID);
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
	    status = DosFindFirst(pci->pszFileName, &hDir,
				   FILE_NORMAL | FILE_DIRECTORY |
				   FILE_ARCHIVED | FILE_READONLY |
				   FILE_HIDDEN | FILE_SYSTEM,
				   &ffb, sizeof(ffb), &nm, FIL_STANDARD);
	    priority_bumped();
	    if (!status) {
	      DosFindClose(hDir);
	      if (ffb.attrFile & FILE_DIRECTORY) {
		if ((shiftstate & (KC_CTRL | KC_ALT)) == (KC_CTRL | KC_ALT))
		  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_SHOWALLFILES, 0),
			  MPVOID);
		else if ((shiftstate & (KC_CTRL | KC_SHIFT)) ==
			 (KC_CTRL | KC_SHIFT))
		  OpenObject(pci->pszFileName, Settings, dcd->hwndFrame);
		else if (shiftstate & KC_CTRL)
		  OpenObject(pci->pszFileName, Default, dcd->hwndFrame);
		else
		  OpenDirCnr(HWND_DESKTOP,
			     hwndMain,
			     dcd->hwndFrame, FALSE, pci->pszFileName);
	      }
	      else {
		SWP swp;

		WinSendMsg(hwnd,
			   CM_SETRECORDEMPHASIS,
			   MPFROMP(pci), MPFROM2SHORT(TRUE, CRA_INUSE));
		WinQueryWindowPos(dcd->hwndFrame, &swp);
		DefaultViewKeys(hwnd,
				dcd->hwndFrame,
				dcd->hwndParent, &swp, pci->pszFileName);
		WinSendMsg(hwnd,
			   CM_SETRECORDEMPHASIS,
			   MPFROMP(pci),
			   MPFROM2SHORT(FALSE, CRA_INUSE |
					((fUnHilite) ? CRA_SELECTED : 0)));
	      }
	    }
	    else
	      RemoveCnrItems(hwnd, pci, 1, CMA_FREE | CMA_INVALIDATE | CMA_ERASE);
	  }
	}
	break;
      }
    }
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

  case UM_CLOSE:
    WinDestroyWindow(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				    QW_PARENT));
    return 0;

  case UM_FOLDUP:
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      DosExit(EXIT_PROCESS, 1);
    return 0;

  case WM_CLOSE:
    if (dcd) {
      dcd->namecanchange = TRUE;
      dcd->stopflag = 1;
      if (dcd->amextracted)
	return 0;			// Can not close yet
    }
    WinSendMsg(hwnd, WM_SAVEAPPLICATION, MPVOID, MPVOID);
    if (dcd) {
      if (!dcd->dontclose && ParentIsDesktop(hwnd, dcd->hwndParent))
	PostMsg(hwnd, UM_FOLDUP, MPVOID, MPVOID);
      if (dcd->hwndObject) {
	DosSleep(32);			// Allow UM_FOLDUP to process
	if (!PostMsg(dcd->hwndObject, WM_CLOSE, MPVOID, MPVOID))
	  WinSendMsg(dcd->hwndObject, WM_CLOSE, MPVOID, MPVOID);
      }
    }
    else
      WinSendMsg(hwnd, UM_CLOSE, MPVOID, MPVOID);
    return 0;

  case WM_DESTROY:
    if (CollectorDirMenu)
      WinDestroyWindow(CollectorDirMenu);
    if (CollectorFileMenu)
      WinDestroyWindow(CollectorFileMenu);
    if (CollectorCnrMenu)
      WinDestroyWindow(CollectorCnrMenu);
    CollectorCnrMenu = CollectorFileMenu = CollectorDirMenu = (HWND) 0;
    Collector = (HWND) 0;
    EmptyCnr(hwnd);
    break;
  }
  if (dcd && dcd->oldproc){
      return dcd->oldproc(hwnd, msg, mp1, mp2);
  }
  else
      return PFNWPCnr(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CollectorMenuProc(HWND hwnd, ULONG msg, MPARAM mp1,
				     MPARAM mp2)
{
  PFNWP oldMenuProc = WinQueryWindowPtr(hwnd, QWL_USER);
  static short sLastMenuitem;

  switch (msg) {
    case WM_MOUSEMOVE: {
      if (fOtherHelp) {
	RECTL rectl;
	SHORT i, sCurrentMenuitem;
	SHORT MenuItems = 5;
	SHORT asMenuIDs[5] = {IDM_GREP,
	      IDM_SEEALL,
	      IDM_CLEARCNR,
	      IDM_REMOVE,
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
	} // for

	 switch (sCurrentMenuitem) {
	 case 0:
	   break;
	 case IDM_GREP:
	   szHelpString = GetPString(IDS_COLMENUSEEKSCANHELP);
	   break;
	 case IDM_SEEALL:
	   szHelpString = GetPString(IDS_COLMENUSEEALLHELP);
	   break;
	 case IDM_CLEARCNR:
	   szHelpString = GetPString(IDS_COLMENUCLEARCNRHELP);
	   break;
	 case IDM_REMOVE:
	   szHelpString = GetPString(IDS_COLMENUREMOVECNRHELP);
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

HWND StartCollector(HWND hwndParent, INT flags)
{
  HWND hwndFrame = (HWND) 0;
  HWND hwndClient;
  ULONG FrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
    FCF_SIZEBORDER | FCF_MINMAX | FCF_ICON | FCF_NOBYTEALIGN | FCF_ACCELTABLE;
  USHORT id;
  DIRCNRDATA *dcd;

  static USHORT idinc = 0;

  if (ParentIsDesktop(hwndParent, hwndParent))
    FrameFlags |= (FCF_TASKLIST | FCF_SHELLPOSITION | FCF_MENU);
  if (Collector) {
    WinSetWindowPos(WinQueryWindow(WinQueryWindow(Collector,
						  QW_PARENT),
				   QW_PARENT),
		    HWND_TOP, 0, 0, 0, 0, SWP_SHOW | SWP_RESTORE);
    return WinQueryWindow(WinQueryWindow(Collector, QW_PARENT), QW_PARENT);
  }
  hwndFrame = WinCreateStdWindow(hwndParent,
				 WS_VISIBLE,
				 &FrameFlags,
				 (CHAR *) WC_COLLECTOR,
				 NULL,
				 WS_VISIBLE | fwsAnimate,
				 FM3ModHandle, COLLECTOR_FRAME, &hwndClient);
  if (hwndFrame && hwndClient) {
    id = COLLECTOR_FRAME + idinc++;
    WinSetWindowUShort(hwndFrame, QWS_ID, id);
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif
    dcd = xmallocz(sizeof(DIRCNRDATA), pszSrcFile, __LINE__);
    if (!dcd) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      PostMsg(hwndClient, WM_CLOSE, MPVOID, MPVOID);
      hwndFrame = (HWND) 0;
    }
    else {
      dcd->size = sizeof(DIRCNRDATA);
      dcd->id = id;
      dcd->type = COLLECTOR_FRAME;
      dcd->hwndParent = (hwndParent) ? hwndParent : HWND_DESKTOP;
      dcd->hwndFrame = hwndFrame;
      dcd->hwndClient = hwndClient;
      if (flags & 4)
	dcd->dontclose = TRUE;
      {
	PFNWP oldproc;

	oldproc = WinSubclassWindow(hwndFrame, (PFNWP) CollectorFrameWndProc);
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
				     HWND_TOP,
				     (ULONG) COLLECTOR_CNR, NULL, NULL);
      if (!dcd->hwndCnr) {
	Win_Error(hwndClient, hwndClient, pszSrcFile, __LINE__,
		  PCSZ_WINCREATEWINDOW);
	PostMsg(hwndClient, WM_CLOSE, MPVOID, MPVOID);
	free(dcd);
	hwndFrame = (HWND) 0;
      }
      else {
	Collector = dcd->hwndCnr;
	WinSetWindowPtr(dcd->hwndCnr, QWL_USER, (PVOID) dcd);
	WinSetWindowText(hwndFrame, (CHAR *) GetPString(IDS_COLLECTORTITLETEXT));
	if (FrameFlags & FCF_MENU) {
	  PFNWP oldmenuproc;
	  HWND hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);

	  oldmenuproc = WinSubclassWindow(hwndMenu, (PFNWP) CollectorMenuProc);
	  WinSetWindowPtr(hwndMenu, QWL_USER, (PVOID) oldmenuproc);
	  if (!fToolbar) {
	    if (hwndMenu) {

	      WinSendMsg(hwndMenu,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SEEALL, FALSE), MPVOID);
	      WinSendMsg(hwndMenu,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_GREP, FALSE), MPVOID);
	      WinSendMsg(hwndMenu,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_CLEARCNR, FALSE), MPVOID);
	      WinSendMsg(hwndMenu,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_REMOVE, FALSE), MPVOID);
	    }
	  }
	}
	dcd->oldproc = WinSubclassWindow(dcd->hwndCnr,
					 (PFNWP) CollectorCnrWndProc);
	{
	  USHORT ids[] = { DIR_TOTALS,
			   DIR_SELECTED,
			   DIR_VIEW,
			   DIR_SORT,
			   DIR_FILTER,
			   0
	  };

	  CommonCreateTextChildren(dcd->hwndClient,
				   WC_COLSTATUS, ids);
	}
	if (FrameFlags & FCF_SHELLPOSITION)
	  PostMsg(hwndClient, UM_SIZE, MPVOID, MPVOID);
	if (!PostMsg(dcd->hwndCnr, UM_SETUP, MPVOID, MPVOID))
	  WinSendMsg(dcd->hwndCnr, UM_SETUP, MPVOID, MPVOID);
      }
    }
#   ifdef FORTIFY
    DosSleep(1);			// Let receiver take ownership
    Fortify_LeaveScope();
#   endif
  }
  return hwndFrame;
}

#pragma alloc_text(COLLECTOR,CollectorCnrWndProc,CollectorObjWndProc)
#pragma alloc_text(COLLECTOR,CollectorClientWndProc,CollectorTextProc)
#pragma alloc_text(COLLECTOR,CollectorFrameWndProc)
#pragma alloc_text(STARTUP,StartCollector)
