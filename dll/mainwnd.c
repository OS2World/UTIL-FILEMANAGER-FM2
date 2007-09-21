
/***********************************************************************

  $Id$

  fm/2 main window

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2007 Steven H. Levine

  11 Jun 02 SHL Drop obsolete xor code
  16 Oct 02 SHL Handle large partitions
  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  23 May 05 SHL Use datamin.h
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  26 May 05 SHL Comments and localize code
  05 Jun 05 SHL Use QWL_USER
  06 Jun 05 SHL Rework MainWMCommand for VAC3.65 compat
  13 Aug 05 SHL Renames and comments
  08 Dec 05 SHL DriveProc: disable menu items if drive not ready
  17 Dec 05 SHL DriveProc: correct my stupid
  29 May 06 SHL IDM_EDITANYARCHIVER: sanitize code
  17 Jul 06 SHL Use Runtime_Error
  17 Aug 06 SHL Complain nicer if state name does not exist
  18 Feb 07 GKY More drive type and icon support
  08 Mar 07 SHL SaveDirCnrState: do not save state of NOPRESCAN volumes
  09 Mar 07 SHL RestoreDirCnrState/SaveDirCnrState: optimize and avoid overflows
  30 Mar 07 GKY Remove GetPString for window class names
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits
  06 Apr 07 GKY Add some error checking in drag/drop
  15 Apr 07 SHL mainwnd MAIN_SETUPLIST restore state not found reporting
  19 Apr 07 SHL Sync with AcceptOneDrop GetOneDrop mods
  20 Apr 07 SHL Avoid spurious add_udir error reports
  12 May 07 SHL Use dcd->ulItemsToUnHilite
  10 Jun 07 GKY Add CheckPmDrgLimit including IsFm2Window as part of work around PM drag limit
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  07 Aug 07 SHL Use BldQuotedFileName
  16 Aug 07 SHL Update IDM_SAVEDIRCNRSTATE logic for ticket# 109
  18 Aug 07 SHL Rework UM_FILLSETUPLIST for new setups storage
  19 Aug 07 SHL Move #pragma alloc_text to end of file for OpenWatcom
  19 Aug 07 SHL Rework SaveDirCnrState to return better error info
  30 Aug 07 SHL Add accelerator support to quicklist windows

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
// #define INCL_WINERRORS
#define INCL_SHLERRORS			// PMERR_NOT_IN_IDX
#define INCL_WINHELP
#define INCL_GPI
#define INCL_LONGLONG
#include <os2.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <process.h>			// _beginthread

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "tools.h"
#include "datamin.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static USHORT firsttool = 0;

static BOOL CloseDirCnrChildren(HWND hwndClient);
static BOOL RestoreDirCnrState(HWND hwndClient, PSZ pszStateName, BOOL noview);

static MRESULT EXPENTRY MainObjectWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
					  MPARAM mp2)
{
  switch (msg) {
  case UM_SETUP:
  case UM_SETUP2:
  case UM_SETUP3:
  case UM_SETUP4:
  case UM_SETUP5:
    /*
     * feed setup messages to main window
     */
    PostMsg(hwndMain, msg, mp1, mp2);
    return 0;

  case UM_SETUP6:
    /*
     * handle bubble help requests from drive bar buttons
     */
    {
      char dv[3], d;
      HWND hwndB = (HWND) mp1;
      USHORT id;

      id = WinQueryWindowUShort(hwndB, QWS_ID);
      *dv = 0;
      WinQueryWindowText(WinWindowFromID(WinQueryWindow(hwndB, QW_PARENT),
					 id + 50), sizeof(dv), dv);
      d = toupper(*dv);
      if (isalpha(d) && d > 'B' &&
	  !(driveflags[d - 'A'] & (DRIVE_CDROM | DRIVE_INVALID |
				   DRIVE_SLOW)) &&
	  (!hwndBubble ||
	   WinQueryWindowULong(hwndBubble, QWL_USER) != hwndB) &&
	  !WinQueryCapture(HWND_DESKTOP)) {

	FSALLOCATE fsa;
	CHAR s[90], szQty[38];
	ULONG ulPctFree;
	ULONGLONG ullFreeQty;

	if (!DosQueryFSInfo((d - 'A') + 1,
			    FSIL_ALLOC, &fsa, sizeof(FSALLOCATE))) {
	  ullFreeQty = (ULONGLONG) fsa.cUnitAvail *
	    (fsa.cSectorUnit * fsa.cbSector);
	  ulPctFree = (fsa.cUnit && fsa.cUnitAvail) ?
	    (fsa.cUnitAvail * 100) / fsa.cUnit : 0;
	  CommaFmtULL(szQty, sizeof(szQty), ullFreeQty, ' ');
	  sprintf(s, "%s (%lu%%) free", szQty, ulPctFree);
	}
	if ((!hwndBubble ||
	     WinQueryWindowULong(hwndBubble, QWL_USER) != hwndB) &&
	    !WinQueryCapture(HWND_DESKTOP))
	  WinSendMsg(hwndB, UM_SETUP6, MPFROMP(s), MPVOID);
      }
    }
    return 0;

  case UM_SETDIR:
    {
      CHAR s[8] = " :\\OS2";
      ULONG bd;

      if (DosQuerySysInfo(QSV_BOOT_DRIVE,
			  QSV_BOOT_DRIVE,
			  (PVOID) & bd, (ULONG) sizeof(ULONG)))
	bd = 3L;
      *s = (CHAR) bd + '@';
      WinSendMsg(hwndMain, UM_SETDIR, MPFROMP(s), MPFROMLONG(1));
      if (!mp1) {
	s[3] = 0;
	WinSendMsg(hwndMain, UM_SETDIR, MPFROMP(s), MPVOID);
      }
      PostMsg(MainObjectHwnd, UM_RESTORE, MPFROMLONG(1), MPFROMLONG(1));
    }
    return 0;

  case UM_RESTORE:
    if (mp2) {
      switch ((ULONG) mp2) {
      case 1:
	TileChildren(hwndMain, TRUE);
	break;
      case 2:
	CloseDirCnrChildren(hwndMain);
	break;
      }
    }
    else {
      fNoTileUpdate = TRUE;
      WinEnableWindow(WinQueryWindow(hwndMain, QW_PARENT), FALSE);
      RestoreDirCnrState(hwndMain, (char *)mp1, FALSE);
      WinEnableWindow(WinQueryWindow(hwndMain, QW_PARENT), TRUE);
      fNoTileUpdate = FALSE;
      if (mp1)
	free((char *)mp1);
      if (fAutoTile)
	TileChildren(hwndMain, TRUE);
    }
    return 0;

  case UM_NOTIFY:
    /*
     * bring up notify messages for various windows
     */
    if (mp1)
      return MRFROMLONG(DoNotify((char *)mp1));
    return 0;

  case WM_DESTROY:
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

VOID MakeMainObjWin(VOID * args)
{
  HAB hab2;
  HMQ hmq2;
  QMSG qmsg2;

  priority_bumped();
  hab2 = WinInitialize(0);
  if (hab2) {
    hmq2 = WinCreateMsgQueue(hab2, 128);
    if (hmq2) {
      DosError(FERR_DISABLEHARDERR);
      WinRegisterClass(hab2,
		       (PSZ) WC_OBJECTWINDOW,
		       MainObjectWndProc, 0, sizeof(PVOID));
      MainObjectHwnd = WinCreateWindow(HWND_OBJECT,
				       WC_OBJECTWINDOW,
				       (PSZ) NULL,
				       0,
				       0L,
				       0L,
				       0L,
				       0L,
				       0L, HWND_TOP, OBJ_FRAME, NULL, NULL);
      if (!MainObjectHwnd)
	Win_Error2(HWND_OBJECT, HWND_DESKTOP, pszSrcFile, __LINE__,
		   IDS_WINCREATEWINDOW);
      else {
	WinSetWindowPtr(MainObjectHwnd, QWL_USER, args);
	while (WinGetMsg(hab2, &qmsg2, (HWND) 0, 0, 0))
	  WinDispatchMsg(hab2, &qmsg2);
	WinDestroyWindow(MainObjectHwnd);
      }
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
}

static MRESULT EXPENTRY IdealButtonProc(HWND hwnd, ULONG msg, MPARAM mp1,
					MPARAM mp2)
{
  switch (msg) {
  case WM_MOUSEMOVE:
    BubbleHelp(hwnd, TRUE, FALSE, FALSE, GetPString(IDS_IDEALBUTTONHELP));
    break;
  }
  return PFNWPButton(hwnd, msg, mp1, mp2);
}

HWND TopWindow(HWND hwndParent, HWND exclude)
{
  HENUM henum;
  HWND hwndC = (HWND) 0;
  USHORT id;

  if (hwndParent) {
    henum = WinBeginEnumWindows(hwndMain);
    while ((hwndC = WinGetNextWindow(henum)) != NULLHANDLE) {
      if (hwndC != exclude) {
	id = WinQueryWindowUShort(hwndC, QWS_ID);
	if (id)
	  break;
      }
    }
    WinEndEnumWindows(henum);
  }
  return hwndC;
}

HWND TopWindowName(HWND hwndParent, HWND exclude, CHAR * ret)
{
  HENUM henum;
  HWND hwndC = (HWND) 0, hwndDir, hwndClient;
  USHORT id;
  PCNRITEM pci = NULL;

  if (ret) {
    *ret = 0;
    if (hwndParent) {
      henum = WinBeginEnumWindows(hwndMain);
      while ((hwndC = WinGetNextWindow(henum)) != NULLHANDLE) {
	// saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"Tree = %lu\rExclude = %lu\rFound = %lu",hwndTree,exclude,hwndC);
	if (hwndC != exclude && hwndC != hwndTree) {
	  id = WinQueryWindowUShort(hwndC, QWS_ID);
	  if (id) {
	    hwndClient = WinWindowFromID(hwndC, FID_CLIENT);
	    if (hwndClient) {
	      hwndDir = WinWindowFromID(hwndClient, DIR_CNR);
	      if (hwndDir) {
		if (fLookInDir) {
		  pci = (PCNRITEM) WinSendMsg(hwndDir,
					      CM_QUERYRECORDEMPHASIS,
					      MPFROMLONG(CMA_FIRST),
					      MPFROMSHORT(CRA_CURSORED));
		  if (pci && (INT) pci != -1)
		    break;
		}
		if (WinSendMsg(hwndClient,
			       UM_CONTAINERDIR, MPFROMP(ret), MPVOID)) {
		  MakeValidDir(ret);
		  // saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"Tree = %lu\rExclude = %lu\rFound = %lu\r\"%s\"",hwndTree,exclude,hwndC,ret);
		  WinEndEnumWindows(henum);
		  return hwndC;
		}
	      }
	    }
	  }
	}
      }
      WinEndEnumWindows(henum);
      if (!pci || (INT) pci == -1) {
	hwndC = hwndTree;
	pci = (PCNRITEM) WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,
								    FID_CLIENT),
						    TREE_CNR),
				    CM_QUERYRECORDEMPHASIS,
				    MPFROMLONG(CMA_FIRST),
				    MPFROMSHORT(CRA_CURSORED));
      }
      if (pci && (INT) pci != -1) {
	strcpy(ret, pci->pszFileName);
	MakeValidDir(ret);
      }
      else
	save_dir2(ret);
    }
  }
  return hwndC;
}

ULONG CountDirCnrs(HWND hwndParent)
{
  HENUM henum;
  HWND hwndF = (HWND) 0, hwndC, hwndDir;
  ULONG ret = 0;

  henum = WinBeginEnumWindows(hwndParent);
  while ((hwndF = WinGetNextWindow(henum)) != NULLHANDLE) {
    hwndC = WinWindowFromID(hwndF, FID_CLIENT);
    if (hwndC) {
      hwndDir = WinWindowFromID(hwndC, DIR_CNR);
      if (hwndDir)
	ret++;
    }
  }
  WinEndEnumWindows(henum);
  return ret;
}

HWND FindDirCnrByName(CHAR * directory, BOOL restore)
{
  HENUM henum;
  HWND hwndF = (HWND) 0, hwndC, hwndDir;
  CHAR retstr[CCHMAXPATH];

  if (hwndMain) {
    henum = WinBeginEnumWindows(hwndMain);
    while ((hwndF = WinGetNextWindow(henum)) != NULLHANDLE) {
      hwndC = WinWindowFromID(hwndF, FID_CLIENT);
      if (hwndC) {
	hwndDir = WinWindowFromID(hwndC, DIR_CNR);
	if (hwndDir) {
	  *retstr = 0;
	  WinSendMsg(hwndC, UM_CONTAINERDIR, MPFROMP(retstr), MPVOID);
	  if (*retstr && !stricmp(retstr, directory)) {
	    if (restore)
	      WinSetWindowPos(hwndF,
			      HWND_TOP,
			      0,
			      0,
			      0,
			      0,
			      SWP_RESTORE | SWP_SHOW |
			      SWP_ACTIVATE | SWP_ZORDER);
	    break;
	  }
	}
      }
    }
    WinEndEnumWindows(henum);
  }
  return hwndF;
}

static VOID SetToggleChecks(HWND hwndMenu)
{
  WinCheckMenuItem(hwndMenu, IDM_TEXTTOOLS, fTextTools);
  WinCheckMenuItem(hwndMenu, IDM_TOOLTITLES, fToolTitles);
  WinCheckMenuItem(hwndMenu, IDM_USERLIST, fUserComboBox);
  WinCheckMenuItem(hwndMenu, IDM_TOOLSUBMENU, fToolbar);
  WinCheckMenuItem(hwndMenu, IDM_AUTOVIEWSUBMENU, fAutoView);
  WinCheckMenuItem(hwndMenu, IDM_AUTOVIEWFILE, !fComments);
  WinCheckMenuItem(hwndMenu, IDM_AUTOVIEWCOMMENTS, fComments);
  WinCheckMenuItem(hwndMenu, IDM_MOREBUTTONS, fMoreButtons);
  WinCheckMenuItem(hwndMenu, IDM_DRIVEBAR, fDrivebar);
  WinCheckMenuItem(hwndMenu, IDM_AUTOTILE, fAutoTile);
  WinCheckMenuItem(hwndMenu, IDM_TILEBACKWARDS, fTileBackwards);
}

static VOID ResizeTools(HWND hwnd)
{
  register ULONG butx = 18L;
  INT attrib = SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER | SWP_NOREDRAW,
    noattrib;
  register TOOL *tool, *starttool;
  SWP *swp;
  register ULONG numtools, x;

  if (!fToolbar)
    return;
  noattrib = attrib;
  noattrib &= (~(SWP_SHOW | SWP_ZORDER));
  noattrib |= SWP_HIDE;
  /* count tools */
  tool = toolhead;
  for (numtools = 0L; tool; numtools++)
    tool = tool->next;
  /* allocate swp array for WinSetMultWindowPos */
  swp = xmallocz(sizeof(SWP) * (numtools + 2), pszSrcFile, __LINE__);
  if (swp) {
    for (x = 0; x < numtools + 2L; x++) {
      swp[x].hwndInsertBehind = HWND_TOP;
      swp[x].fl = attrib;
      swp[x].y = (fToolTitles) ? 14L : 3L;
      swp[x].cx = 32L;
      swp[x].cy = 32L;
    }
    swp[0].x = swp[1].x = 2L;
    swp[0].y = (fTextTools) ? 14L : 18L;
    swp[1].y = (fTextTools) ? 1L : 2L;
    swp[0].cx = swp[1].cx = 14L;
    swp[0].cy = swp[1].cy = 14L;
    swp[0].hwnd = WinWindowFromID(hwnd, IDM_TOOLLEFT);
    swp[1].hwnd = WinWindowFromID(hwnd, IDM_TOOLRIGHT);
    x = 2L;
    tool = find_tool(firsttool);
    if (!tool)
      tool = toolhead;
    starttool = tool;
    while (tool) {
      if (!(tool->flags & T_INVISIBLE)) {
	swp[x].x = butx;
	if (fTextTools || (tool->flags & T_TEXT)) {
	  butx += 55L;
	  swp[x].cx = 54L;
	  swp[x].cy = 24L;
	  swp[x].y = 3L;
	}
	else
	  butx += 33L;
	if (tool->flags & T_SEPARATOR)
	  butx += 12;
      }
      else
	swp[x].fl = noattrib;
      swp[x].hwnd = WinWindowFromID(hwnd, tool->id);
      x++;
      tool = tool->next;
    }
    tool = toolhead;
    while (tool && tool != starttool) {
      swp[x].x = butx;
      if (!(tool->flags & T_INVISIBLE)) {
	if (fTextTools || (tool->flags & T_TEXT)) {
	  butx += 55L;
	  swp[x].cx = 54L;
	  swp[x].cy = 24L;
	  swp[x].y = 3L;
	}
	else
	  butx += 33L;
	if (tool->flags & T_SEPARATOR)
	  butx += 12;
      }
      else
	swp[x].fl = noattrib;
      swp[x].hwnd = WinWindowFromID(hwnd, tool->id);
      x++;
      tool = tool->next;
    }
    WinSetMultWindowPos(WinQueryAnchorBlock(hwnd), swp, numtools + 2L);
    if (!fTextTools && fToolTitles) {
      for (x = 2L; x < numtools + 2L; x++) {
	if (fTextTools || !fToolTitles)
	  swp[x].fl = noattrib;
	else {
	  tool = find_tool(WinQueryWindowUShort(swp[x].hwnd, QWS_ID));
	  if (tool && (tool->flags & T_TEXT))
	    swp[x].fl = noattrib;
	  else {
	    swp[x].hwndInsertBehind = HWND_TOP;
	    swp[x].y = 1L;
	    swp[x].cy = 10L;
	  }
	}
	swp[x].hwnd = WinWindowFromID(hwnd,
				      WinQueryWindowUShort(swp[x].hwnd,
							   QWS_ID) + 25000);
      }
      WinSetMultWindowPos(WinQueryAnchorBlock(hwnd), &swp[2], numtools);
    }
    free(swp);
  }
  WinInvalidateRect(hwnd, NULL, TRUE);
}

static MRESULT EXPENTRY DropDownListProc(HWND hwnd, ULONG msg, MPARAM mp1,
					 MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) INSTDATA(hwnd);
  USHORT id;

  static HWND hwndMenu = (HWND)0;
  static BOOL emphasized = FALSE;

  switch (msg) {
  case WM_MOUSEMOVE:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    break;

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_MENUEND:
    if (hwndMenu == (HWND) mp2) {
      WinDestroyWindow(hwndMenu);
      hwndMenu = (HWND) 0;
    }
    break;

  case WM_FOCUSCHANGE:
    {
      HAB hab = WinQueryAnchorBlock(hwnd);
      HWND hwndParent = WinQueryWindow(hwnd, QW_PARENT);
      HWND hwndFrame = WinQueryWindow(hwndParent, QW_PARENT);
      static HACCEL haccelSaved = NULLHANDLE;
      static HACCEL haccelDriveList = NULLHANDLE;
      static HACCEL haccelSetupList = NULLHANDLE;
      static HACCEL haccelUserList = NULLHANDLE;
      static HACCEL haccelCmdList = NULLHANDLE;
      static HACCEL haccelButtonList = NULLHANDLE;
      // DbgMsg(pszSrcFile, __LINE__, "WM_FOCUSCHANGE %u", SHORT1FROMMP(mp2));
      id = WinQueryWindowUShort(hwndParent, QWS_ID);
      if (SHORT1FROMMP(mp2)) {
	// If getting focus 1st time - save original accelerator
	if (haccelSaved == NULLHANDLE) {
	  haccelSaved = WinQueryAccelTable(hab, hwndFrame);
	  if (haccelSaved == NULLHANDLE)
	    Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__, "WinQueryAccelTable");
	  // else
	    // DbgMsg(pszSrcFile, __LINE__, "WinQueryAccelTable SAVED %x", haccelSaved);
	}
	if (haccelSaved != NULLHANDLE) {
	  switch (id) {
	  case MAIN_DRIVELIST:
	    if (haccelDriveList == NULLHANDLE) {
		haccelDriveList = WinLoadAccelTable(hab, FM3ModHandle, MAIN_DRIVELIST);
		if (haccelDriveList == NULLHANDLE)
		  Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__, "WinLoadAccelTable");
	    }
	    if (haccelDriveList != NULLHANDLE) {
	      if (!WinSetAccelTable(hab, haccelDriveList, hwndFrame))
		Win_Error(hwndFrame, HWND_DESKTOP, pszSrcFile, __LINE__, "WinSetAccelTable");
	      // else
		// DbgMsg(pszSrcFile, __LINE__, "WinSetAccelTable MAIN_DRIVELIST %x %x", hwndFrame, haccelDriveList);
	    }
	    break;
	  case MAIN_SETUPLIST:
	    if (haccelSetupList == NULLHANDLE) {
		haccelSetupList = WinLoadAccelTable(hab, FM3ModHandle, MAIN_SETUPLIST);
		if (haccelSetupList == NULLHANDLE)
		  Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__, "WinLoadAccelTable");
	    }
	    if (haccelSetupList != NULLHANDLE) {
	      if (!WinSetAccelTable(hab, haccelSetupList, hwndFrame))
		Win_Error(hwndFrame, HWND_DESKTOP, pszSrcFile, __LINE__, "WinSetAccelTable");
	      // else
		// DbgMsg(pszSrcFile, __LINE__, "WinSetAccelTable MAIN_SETUPLIST %x %x", hwndFrame, haccelSetupList);
	    }
	    break;
	  case MAIN_CMDLIST:
	    if (haccelCmdList == NULLHANDLE) {
		haccelCmdList = WinLoadAccelTable(hab, FM3ModHandle, MAIN_CMDLIST);
		if (haccelCmdList == NULLHANDLE)
		  Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__, "WinLoadAccelTable");
	    }
	    if (haccelCmdList != NULLHANDLE) {
	      if (!WinSetAccelTable(hab, haccelCmdList, hwndFrame))
		Win_Error(hwndFrame, HWND_DESKTOP, pszSrcFile, __LINE__, "WinSetAccelTable");
	      // else
		// DbgMsg(pszSrcFile, __LINE__, "WinSetAccelTable MAIN_CMDLIST %x %x", hwndFrame, haccelCmdList);
	    }
	    break;
	  case MAIN_USERLIST:
	    if (haccelUserList == NULLHANDLE) {
		haccelUserList = WinLoadAccelTable(hab, FM3ModHandle, MAIN_USERLIST);
		if (haccelUserList == NULLHANDLE)
		  Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__, "WinLoadAccelTable");
	    }
	    if (haccelUserList != NULLHANDLE) {
	      if (!WinSetAccelTable(hab, haccelUserList, hwndFrame))
		Win_Error(hwndFrame, HWND_DESKTOP, pszSrcFile, __LINE__, "WinSetAccelTable");
	      // else
		// DbgMsg(pszSrcFile, __LINE__, "WinSetAccelTable MAIN_USERLIST %x %x", hwndFrame, haccelUserList);
	    }
	    break;
	  case MAIN_BUTTONLIST:
	    if (haccelButtonList == NULLHANDLE) {
		haccelButtonList = WinLoadAccelTable(hab, FM3ModHandle, MAIN_BUTTONLIST);
		if (haccelButtonList == NULLHANDLE)
		  Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__, "WinLoadAccelTable");
	    }
	    if (haccelButtonList != NULLHANDLE) {
	      if (!WinSetAccelTable(hab, haccelButtonList, hwndFrame))
		Win_Error(hwndFrame, HWND_DESKTOP, pszSrcFile, __LINE__, "WinSetAccelTable");
	      // else
		// DbgMsg(pszSrcFile, __LINE__, "WinSetAccelTable MAIN_BUTTONLIST %x %x", hwndFrame, haccelButtonList);
	    }
	    break;
	  } // switch
	}
      }
      else {
	// Losing focus
	switch (id) {
	case MAIN_DRIVELIST:
	case MAIN_SETUPLIST:
	case MAIN_CMDLIST:
	case MAIN_USERLIST:
	case MAIN_BUTTONLIST:
	  if (haccelSaved != NULLHANDLE) {
	    if (!WinSetAccelTable(hab, haccelSaved, hwndFrame))
	      Win_Error(hwndFrame, HWND_DESKTOP, pszSrcFile, __LINE__, "WinSetAccelTable");
	    // else
	      // DbgMsg(pszSrcFile, __LINE__, "WinSetAccelTable SAVED %x %x", hwndFrame, haccelSaved);
	  }
	  break;
	} // switch
      }
    }
    break; // WM_FOCUSCHANGE

  case WM_CONTEXTMENU:
    {
      MRESULT ret = MRFROMSHORT(TRUE);

      if (hwndMenu)
	WinDestroyWindow(hwndMenu);
      hwndMenu = (HWND) 0;
      id = WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_ID);
      switch (id) {
      case MAIN_CMDLIST:
	WinPostMsg(WinWindowFromID(WinQueryWindow(WinQueryWindow(hwnd,
								 QW_PARENT),
						  QW_PARENT),
				   FID_CLIENT),
		   WM_COMMAND, MPFROM2SHORT(IDM_EDITCOMMANDS, 0), MPVOID);
	break;
      case MAIN_USERLIST:
      case MAIN_SETUPLIST:
	hwndMenu = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, id);
	if (hwndMenu)
	  PopupMenu(hwnd,
		    WinWindowFromID(WinQueryWindow(WinQueryWindow(hwnd,
								  QW_PARENT),
						   QW_PARENT),
				    FID_CLIENT), hwndMenu);
	break;
      default:
	ret = FALSE;
	break;
      } // switch
      return ret;
    }

  case WM_CONTROL:
    if (hwndStatus2) {
      switch (SHORT1FROMMP(mp1)) {
      case CBID_EDIT:
	id = WinQueryWindowUShort(hwnd, QWS_ID);
	switch (SHORT2FROMMP(mp1)) {
	case EN_SETFOCUS:
	  switch (id) {
	  case MAIN_CMDLIST:
	    WinSetWindowText(hwndStatus2, GetPString(IDS_CMDLISTHELP));
	    break;
	  case MAIN_SETUPLIST:
	    WinSetWindowText(hwndStatus2, GetPString(IDS_SETUPLISTHELP));
	    break;
	  case MAIN_USERLIST:
	    WinSetWindowText(hwndStatus2, GetPString(IDS_USERLISTHELP));
	    break;
	  case MAIN_DRIVELIST:
	    WinSetWindowText(hwndStatus2, GetPString(IDS_DRIVELISTHELP));
	    break;
	  case MAIN_BUTTONLIST:
	    WinSetWindowText(hwndStatus2, GetPString(IDS_BUTTONLISTHELP));
	    break;
	  default:
	    break;
	  }
	  break;

	default:
	  break;
	}
      }
      break;

  default:
      break;
    }
    break;

  case WM_BEGINDRAG:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    // saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"%u %s %u",id,(id == CBID_EDIT) ? "TRUE" : "FALSE",WinQueryWindowUShort(WinQueryWindow(hwnd,QW_PARENT),QWS_ID) == MAIN_USERLIST);
    if (id == CBID_EDIT &&
	WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_ID) ==
	MAIN_USERLIST) {

      CHAR path[CCHMAXPATH];

      *path = 0;
      WinQueryWindowText(hwnd, CCHMAXPATH, path);
      bstrip(path);
      // saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"Dragging: %s",path);
      if (*path && !IsRoot(path))
	DragOne(hwnd, (HWND) 0, path, FALSE);
      return 0;
    }
    break;

  case DM_DRAGOVER:
    id = WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_ID);
    if (id == MAIN_USERLIST) {
      if (!emphasized) {
	emphasized = TRUE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
      if (AcceptOneDrop(hwnd, mp1, mp2))
	return MRFROM2SHORT(DOR_DROP, DO_MOVE);
      return MRFROM2SHORT(DOR_NEVERDROP, 0);
    }
    break;

  case DM_DRAGLEAVE:
    id = WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_ID);
    if (id == MAIN_USERLIST) {
      if (emphasized) {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
    }
    break;

  case DM_DROPHELP:
    id = WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_ID);
    if (id == MAIN_USERLIST) {
      DropHelp(mp1, mp2, hwnd, GetPString(IDS_USERLISTDROPHELP));
      return 0;
    }
    break;

  case DM_DROP:
    id = WinQueryWindowUShort(WinQueryWindow(hwnd, QW_PARENT), QWS_ID);
    if (id == MAIN_USERLIST) {

      char szFrom[CCHMAXPATH + 2];

      if (emphasized) {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
      if (GetOneDrop(hwnd, mp1, mp2, szFrom, sizeof(szFrom))) {
	MakeValidDir(szFrom);
	WinSetWindowText(hwnd, szFrom);
	PostMsg(WinWindowFromID(WinQueryWindow(WinQueryWindow(hwnd,
							      QW_PARENT),
					       QW_PARENT),
				FID_CLIENT),
		UM_COMMAND, MPFROM2SHORT(IDM_ADDTOUSERLIST, 0), MPVOID);
	return 0;
      }
    }
    break;

  case WM_DESTROY:
    if (hwndMenu)
      WinDestroyWindow(hwndMenu);
    hwndMenu = (HWND) 0;
    break;
  }

  return oldproc(hwnd, msg, mp1, mp2);
}

void BubbleHelp(HWND hwnd, BOOL other, BOOL drive, BOOL above, char *help)
{
  if (help && *help &&
      ((drive && fDrivebarHelp) ||
       (other && fOtherHelp) || (!other && !drive && fToolbarHelp))) {
    if ((!hwndBubble ||
	 WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd) &&
	!WinQueryCapture(HWND_DESKTOP))
      MakeBubble(hwnd, above, help);
  }
}

VOID MakeBubble(HWND hwnd, BOOL above, CHAR * help)
{
  if (!hwnd || !help || !*help)
    return;

  if (hwndBubble)
    WinDestroyWindow(hwndBubble);

  {
    HWND hwndActive;
    char ucClassname[8];

    hwndActive = WinQueryActiveWindow(HWND_DESKTOP);
    if (hwndActive) {
      /* don't bring up help if window isn't active */
      if (!WinIsChild(hwnd, hwndActive))
	return;
    }
    hwndActive = WinQueryFocus(HWND_DESKTOP);
    if (WinQueryClassName(hwndActive, sizeof(ucClassname), ucClassname)) {
      /* don't bring up help if a menu is active */
      if (!strcmp(ucClassname, "#4"))
	return;
    }
  }

  hwndBubble = WinCreateWindow(HWND_DESKTOP,
			       WC_BUBBLE,
			       help,
			       WS_CLIPSIBLINGS | SS_TEXT |
			       DT_CENTER | DT_VCENTER,
			       0,
			       0,
			       0,
			       0,
			       HWND_DESKTOP, HWND_TOP, MAIN_HELP, NULL, NULL);
  if (!hwndBubble)
    Win_Error2(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
	       IDS_WINCREATEWINDOW);
  else {
    HPS hps;
    POINTL aptl[TXTBOX_COUNT], ptl, tptl;
    LONG lxScreen, sx, sy, extra = 0, lyScreen;
    char *p, *pp, *wp;
    SWP swp;

    WinQueryWindowPos(hwnd, &swp);
    lyScreen = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
    lxScreen = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
    WinSetWindowULong(hwndBubble, QWL_USER, hwnd);
    SetPresParams(hwndBubble, NULL, NULL, NULL, GetPString(IDS_8HELVTEXT));
    hps = WinGetPS(hwndBubble);
    p = help;
    tptl.x = tptl.y = 0;
    while (p && *p) {
      wp = NULL;
      pp = strchr(p, '\r');
      if (pp) {
	wp = pp;
	*pp = 0;
	pp++;
      }
      GpiQueryTextBox(hps, strlen(p), p, TXTBOX_COUNT, aptl);
      tptl.x = max(aptl[TXTBOX_TOPRIGHT].x, tptl.x);
      if (tptl.y)
	tptl.y += extra;
      else
	extra = aptl[TXTBOX_TOPLEFT].y / 4;
      tptl.y += aptl[TXTBOX_TOPLEFT].y;
      if (wp)
	*wp = '\r';
      p = pp;
    }
    WinSetWindowULong(hwndBubble, QWL_USER + 4, extra);
    WinReleasePS(hps);
    ptl.x = ptl.y = 0;
    WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl, 1);
    if (above) {
      sy = ptl.y + swp.cy + 4;
      if (sy + tptl.y + 12 > lyScreen) {
	above = FALSE;
	sy = ptl.y - (tptl.y + 14);
      }
    }
    else
      sy = ptl.y - (tptl.y + 14);
    if (ptl.x > (lxScreen / 2))
      sx = (ptl.x - tptl.x) - 16;
    else
      sx = ptl.x + (54 * (above == FALSE)) + 2;
    if (sx < 0)
      sx = 0;
    if (sx + tptl.x + 14 > lxScreen)
      sx = lxScreen - (tptl.x + 14);
    if (sy < 0) {
      sy = ptl.y + swp.cy + 4;
      if (sy + tptl.y + 12 > lyScreen)
	sy = 0;
    }
    WinSetWindowPos(hwndBubble, HWND_TOP, sx, sy,
		    tptl.x + 14,
		    tptl.y + 12,
		    SWP_DEACTIVATE | SWP_SHOW | SWP_ZORDER |
		    SWP_MOVE | SWP_SIZE);
  }
}

MRESULT EXPENTRY BubbleProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, mp1, MPVOID);
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, (HWND) mp1);
    return 0;

  case WM_MOUSEMOVE:
    WinShowWindow(hwnd, FALSE);
    break;

  case UM_TIMER:
    {
      POINTL ptl;

      WinQueryPointerPos(HWND_DESKTOP, &ptl);
      if (WinWindowFromPoint(HWND_DESKTOP, &ptl, TRUE) !=
	  WinQueryWindowULong(hwnd, QWL_USER) || !WinIsWindowVisible(hwnd))
	WinDestroyWindow(hwnd);
    }
    return 0;

  case WM_PAINT:
    {
      HPS hps;
      SWP swp;
      POINTL ptl, aptl[TXTBOX_COUNT];
      CHAR *s, *p, *pp, *wp;
      ULONG extra, tlen, y;

      hps = WinBeginPaint(hwnd, (HPS) 0, NULL);
      if (hps) {
	WinQueryWindowPos(hwnd, &swp);
	GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, 0);
	GpiSetColor(hps, ((255 << 16) | (255 << 8) | 198));
	GpiSetBackMix(hps, BM_LEAVEALONE);
	GpiSetMix(hps, FM_OVERPAINT);
	ptl.x = ptl.y = 0;
	GpiMove(hps, &ptl);
	ptl.x = swp.cx - 1;
	ptl.y = swp.cy - 1;
	GpiBox(hps, DRO_OUTLINEFILL, &ptl, 0, 0);
	tlen = WinQueryWindowTextLength(hwnd);
	if (tlen) {
	  s = xmalloc(tlen + 2, pszSrcFile, __LINE__);
	  if (s) {
	    WinQueryWindowText(hwnd, tlen + 1, s);
	    if (*s) {
	      p = s;
	      y = swp.cy - 3;
	      extra = WinQueryWindowULong(hwnd, QWL_USER + 4);
	      GpiSetColor(hps, 0);
	      GpiSetMix(hps, FM_OVERPAINT);
	      while (p && *p) {
		wp = NULL;
		pp = strchr(p, '\r');
		if (pp) {
		  wp = pp;
		  *pp = 0;
		  pp++;
		}
		GpiQueryTextBox(hps, strlen(p), p, TXTBOX_COUNT, aptl);
		ptl.x = 7;
		y -= aptl[TXTBOX_TOPLEFT].y;
		if (p != s)
		  y -= extra;
		ptl.y = y;
		GpiCharStringAt(hps, &ptl, strlen(p), p);
		if (wp)
		  *wp = '\r';
		p = pp;
	      }
	    }
	    free(s);
	  }
	}
	if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE)) && swp.cx > 6 && swp.cy > 6) {
	  GpiSetColor(hps, CLR_WHITE);
	  ptl.x = 1;
	  ptl.y = 1;
	  GpiMove(hps, &ptl);
	  ptl.y = swp.cy - 2;
	  GpiLine(hps, &ptl);
	  ptl.x = swp.cx - 2;
	  GpiLine(hps, &ptl);
	  ptl.x = 2;
	  ptl.y = 2;
	  GpiMove(hps, &ptl);
	  ptl.y = swp.cy - 3;
	  GpiLine(hps, &ptl);
	  ptl.x = swp.cx - 3;
	  GpiLine(hps, &ptl);
	  GpiSetColor(hps, CLR_BROWN);
	  ptl.x = 1;
	  ptl.y = 1;
	  GpiMove(hps, &ptl);
	  ptl.x = swp.cx - 2;
	  GpiLine(hps, &ptl);
	  ptl.y = swp.cy - 2;
	  GpiLine(hps, &ptl);
	  ptl.x = 2;
	  ptl.y = 2;
	  GpiMove(hps, &ptl);
	  ptl.x = swp.cx - 3;
	  GpiLine(hps, &ptl);
	  ptl.y = swp.cy - 3;
	  GpiLine(hps, &ptl);
	}
	WinEndPaint(hps);
      }
    }
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    return 0;

  case WM_DESTROY:
    hwndBubble = (HWND) 0;
    break;
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY LEDProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_CREATE:
    {
      MRESULT mr = PFNWPStatic(hwnd, msg, mp1, mp2);
      HBITMAP hbmold = (HBITMAP) 0;
      HPS hps = (HPS) 0;

      switch (WinQueryWindowUShort(hwnd, QWS_ID)) {
      case MAIN_LED:
	hps = WinGetPS(hwnd);
	hbmold = (HBITMAP) WinSendMsg(hwnd, SM_QUERYHANDLE, MPVOID, MPVOID);
	if (!fBlueLED) {
	  hbmLEDon = GpiLoadBitmap(hps, 0, LEDON_BMP, 12, 12);
	  hbmLEDoff = GpiLoadBitmap(hps, 0, LEDOFF_BMP, 12, 12);
	}
	else {
	  hbmLEDon = GpiLoadBitmap(hps, 0, LEDON2_BMP, 12, 12);
	  hbmLEDoff = GpiLoadBitmap(hps, 0, LEDOFF2_BMP, 12, 12);
	}
	if (hbmLEDoff && hbmLEDon)
	  WinSendMsg(hwnd, SM_SETHANDLE, MPFROMLONG(hbmLEDoff), MPVOID);
	else {
	  if (hbmLEDoff)
	    GpiDeleteBitmap(hbmLEDoff);
	  if (hbmLEDon)
	    GpiDeleteBitmap(hbmLEDon);
	}
	if (hbmold &&
	    hbmLEDon &&
	    hbmLEDoff && hbmold != hbmLEDon && hbmold != hbmLEDoff)
	  GpiDeleteBitmap(hbmold);
	if (hps)
	  WinReleasePS(hps);
	break;
      default:
	SetPresParams(hwnd,
		      &RGBGREY,
		      &RGBBLACK, &RGBGREY, GetPString(IDS_6HELVTEXT));
	break;
      }
      return mr;
    }

  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, mp1, MPVOID);
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, (HWND) mp1);
    return 0;

  case WM_MOUSEMOVE:
    BubbleHelp(hwnd, TRUE, FALSE, FALSE, GetPString(IDS_LEDHELP));
    if (!fNoFinger) {
      WinSetPointer(HWND_DESKTOP, hptrFinger);
      return MRFROMLONG(TRUE);
    }
    break;

  case WM_BUTTON1CLICK:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT),
	    WM_COMMAND, MPFROM2SHORT(IDM_SHOWNOTEWND, 0), MPVOID);
    break;

  case WM_BUTTON2CLICK:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT),
	    WM_COMMAND, MPFROM2SHORT(IDM_HIDENOTEWND, 0), MPVOID);
    break;

  case WM_CHORD:
  case WM_BUTTON3CLICK:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT),
	    WM_COMMAND, MPFROM2SHORT(IDM_WINDOWDLG, 0), MPVOID);
    break;
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ChildButtonProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  USHORT id;
  TOOL *tool;

  static HWND hwndMenu = (HWND) 0;

  switch (msg) {
  case WM_BUTTON1DOWN:
  case WM_BUTTON2DOWN:
  case WM_BUTTON3DOWN:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    break;

  case WM_MOUSEMOVE:
    if (fToolbarHelp) {
      if ((!hwndBubble || WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd)
	  && !WinQueryCapture(HWND_DESKTOP)) {
	id = WinQueryWindowUShort(hwnd, QWS_ID);
	tool = find_tool(id);
	if (tool && tool->help && *tool->help) {

	  char s[128];

	  strcpy(s, tool->help);
	  if (tool->flags & T_DROPABLE)
	    strcat(s, GetPString(IDS_DROPONMETEXT));
	  MakeBubble(hwnd, FALSE, s);
	}
      }
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_TOOLBAR, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case IDM_HIDEANYTOOL:		/* hide any tool */
    case IDM_HIDETOOL:			/* hide tool */
      if (SHORT1FROMMP(mp1) == IDM_HIDETOOL)
	id = WinQueryWindowUShort(hwnd, QWS_ID);
      else
	id = (USHORT) WinDlgBox(HWND_DESKTOP, hwnd,
				PickToolProc, FM3ModHandle,
				PICKBTN_FRAME, GetPString(IDS_HIDETEXT));
      if (id) {
	tool = find_tool(id);
	if (tool) {
	  tool->flags |= T_INVISIBLE;
	  fToolsChanged = TRUE;
	}
      }
      break;

    case IDM_SHOWTOOLS:			/* show all tools */
      tool = toolhead;
      while (tool) {
	tool->flags &= (~T_INVISIBLE);
	tool = tool->next;
	fToolsChanged = TRUE;
      }
      break;

    case IDM_DELETEANYTOOL:		/* delete any button */
    case IDM_DELETETOOL:		/* delete button */
      if (SHORT1FROMMP(mp1) == IDM_DELETETOOL)
	id = WinQueryWindowUShort(hwnd, QWS_ID);
      else
	id =
	  (USHORT) WinDlgBox(HWND_DESKTOP, hwnd, PickToolProc, FM3ModHandle,
			     PICKBTN_FRAME, GetPString(IDS_DELETETEXT));
      if (id)
	PostMsg(WinQueryWindow(hwnd, QW_PARENT), UM_SETUP,
		MPFROM2SHORT(id, 0), MPVOID);
      return 0;

    case IDM_EDITANYTOOL:		/* edit any button */
    case IDM_EDITTOOL:			/* edit button */
      if (SHORT1FROMMP(mp1) == IDM_EDITTOOL)
	id = WinQueryWindowUShort(hwnd, QWS_ID);
      else
	id =
	  (USHORT) WinDlgBox(HWND_DESKTOP, hwnd, PickToolProc, FM3ModHandle,
			     PICKBTN_FRAME, GetPString(IDS_EDITTEXT));
      if (id) {
	tool = find_tool(id);
	if (tool) {
	  if (WinDlgBox(HWND_DESKTOP, hwnd, AddToolProc, FM3ModHandle,
			ADDBTN_FRAME, (PVOID) tool))
	    WinSendMsg(WinWindowFromID(WinQueryWindow(WinQueryWindow(hwnd,
								     QW_PARENT),
						      QW_PARENT), FID_CLIENT),
		       WM_COMMAND, MPFROM2SHORT(IDM_CREATETOOL, 0),
		       MPFROM2SHORT(id, 0));
	}
      }
      break;

    case IDM_ADDTOOL:			/* add tool */
      id = (USHORT) WinDlgBox(HWND_DESKTOP, hwnd, AddToolProc, FM3ModHandle,
			      ADDBTN_FRAME, MPVOID);
      if (id && id != (USHORT) - 1)
	WinSendMsg(WinWindowFromID(WinQueryWindow(WinQueryWindow(hwnd,
								 QW_PARENT),
						  QW_PARENT), FID_CLIENT),
		   WM_COMMAND, MPFROM2SHORT(IDM_CREATETOOL, 0),
		   MPFROM2SHORT(id, 0));
      break;

    case IDM_REORDERTOOLS:		/* reorder tools */
      WinDlgBox(HWND_DESKTOP,
		hwnd, ReOrderToolsProc, FM3ModHandle, RE_FRAME, MPVOID);
      break;

    case IDM_SAVETOOLS:
    case IDM_LOADTOOLS:
      if (WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    ToolIODlgProc,
		    FM3ModHandle,
		    SVBTN_FRAME,
		    (PVOID) (SHORT1FROMMP(mp1) == IDM_SAVETOOLS) ?
		    "TRUE" : NULL))
	BuildTools(hwndToolback, TRUE);
      break;
    }
    ResizeTools(WinQueryWindow(hwnd, QW_PARENT));
    return 0;

  case WM_MENUEND:
    if (hwndMenu == (HWND) mp2) {
      WinDestroyWindow(hwndMenu);
      hwndMenu = (HWND) 0;
    }
    break;

  case WM_CONTEXTMENU:
    DosEnterCritSec();
    if (!hwndMenu)
      hwndMenu = WinLoadMenu(hwnd, FM3ModHandle, ID_BUTTONMENU);
    DosExitCritSec();
    SetPresParams(hwndMenu, NULL, NULL, NULL, GetPString(IDS_10SYSPROTEXT));
    if (PopupMenu(hwnd, hwnd, hwndMenu))
      WinShowWindow(hwndMenu, TRUE);
    return MRFROMSHORT(TRUE);

  case DM_DRAGOVER:
    {
      PDRAGINFO pDInfo;			/* Pointer to DRAGINFO */

      pDInfo = (PDRAGINFO) mp1;		/* Get DRAGINFO pointer */
      DrgAccessDraginfo(pDInfo);	/* Access DRAGINFO */
      id = WinQueryWindowUShort(hwnd, QWS_ID);
      tool = find_tool(id);
      if (!tool) {
	DrgFreeDraginfo(pDInfo);
	return (MRFROM2SHORT(DOR_NEVERDROP, 0));	/* Drop not valid */
      }
      if (!(tool->flags & T_DROPABLE)) {
	DrgFreeDraginfo(pDInfo);
	return (MRFROM2SHORT(DOR_NEVERDROP, 0));	/* Drop not valid */
      }
      {
	PDRAGITEM pDItem;		/* Pointer to DRAGITEM */

	pDItem = DrgQueryDragitemPtr(pDInfo,	/* Access DRAGITEM */
				     0);	/* Index to DRAGITEM */
	if (DrgVerifyRMF(pDItem,	/* Check valid rendering */
			 DRM_OS2FILE,	/* mechanisms and data */
			 NULL)) {	/* formats */
	  if (!(tool->flags & T_EMPHASIZED)) {
	    tool->flags |= T_EMPHASIZED;
	    DrawTargetEmphasis(hwnd, ((tool->flags & T_EMPHASIZED) != 0));
	    DrgFreeDraginfo(pDInfo);
	  }
	  return (MRFROM2SHORT(DOR_DROP,	/* Return okay to drop */
			       DO_MOVE));	/* Move operation valid */
	}
	DrgFreeDraginfo(pDInfo);
      }
    }
    return (MRFROM2SHORT(DOR_NEVERDROP, 0));	/* Drop not valid */

  case DM_DROPHELP:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    tool = find_tool(id);
    PFNWPButton(hwnd, msg, mp1, mp2);
    DropHelp(mp1, mp2, hwnd, GetPString(IDS_TOOLDROPHELP));
    return 0;

  case DM_DRAGLEAVE:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    tool = find_tool(id);
    if (tool && (tool->flags & T_DROPABLE)) {
      if (tool->flags & T_EMPHASIZED) {
	tool->flags &= (~T_EMPHASIZED);
	DrawTargetEmphasis(hwnd, ((tool->flags & T_EMPHASIZED) != 0));
      }
    }
    break;

  case DM_DROP:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    tool = find_tool(id);
    if (tool && (tool->flags & T_DROPABLE) != 0) {
      LISTINFO *li;
      CNRDRAGINFO cdi;

      if (tool->flags & T_EMPHASIZED) {
	DrawTargetEmphasis(hwnd, ((tool->flags & T_EMPHASIZED) != 0));
	tool->flags &= (~T_EMPHASIZED);
      }
      memset(&cdi, 0, sizeof(cdi));
      cdi.pDragInfo = mp1;
      li = DoFileDrop(hwnd, NULL, FALSE, mp1, MPFROMP(&cdi));
      CheckPmDrgLimit(cdi.pDragInfo);
      if (li) {
	li->type = id;
	if (!li->list || !li->list[0])
	  FreeListInfo(li);
	else {
	  HWND hwndActive;

	  hwndActive = TopWindow(hwndMain, (HWND) 0);
	  if (hwndActive) {
	    if (!WinSendMsg(hwndActive, UM_COMMAND, MPFROMP(li), MPVOID))
	      FreeListInfo(li);
	  }
	  else
	    FreeListInfo(li);
	}
      }
    }
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    return 0;
  }
  return PFNWPButton(hwnd, msg, mp1, mp2);
}

VOID BuildTools(HWND hwndT, BOOL resize)
{
  TOOL *tool;
  ULONG ctrlxpos = 18L;
  CHAR s[33];
  HENUM henum;
  HWND hwndTool;

  henum = WinBeginEnumWindows(hwndT);
  while ((hwndTool = WinGetNextWindow(henum)) != NULLHANDLE)
    WinDestroyWindow(hwndTool);
  WinEndEnumWindows(henum);
  if (!fToolbar) {
    load_quicktools();
    load_tools(NULL);
    return;
  }
  if (!toolhead)
    load_tools(NULL);
  tool = toolhead;
  while (tool) {
    sprintf(s, "#%u", tool->id);
    hwndTool = (HWND) 0;
    if (!fTextTools) {
      if (!(tool->flags & T_MYICON)) {
	hwndTool = WinCreateWindow(hwndT,
				   WC_TOOLBUTTONS,
				   s,
				   BS_NOPOINTERFOCUS |
				   BS_BITMAP | BS_PUSHBUTTON,
				   ctrlxpos,
				   14,
				   32,
				   32, hwndT, HWND_TOP, tool->id, NULL, NULL);
      }
      if (!hwndTool) {
	HBITMAP hbm = LoadBitmapFromFileNum(tool->id);

	if (hbm) {
	  BTNCDATA btc;

	  memset(&btc, 0, sizeof(btc));
	  btc.cb = sizeof(btc);
	  btc.hImage = hbm;
	  hwndTool = WinCreateWindow(hwndT,
				     WC_TOOLBUTTONS,
				     NullStr,
				     BS_NOPOINTERFOCUS |
				     BS_BITMAP | BS_PUSHBUTTON,
				     ctrlxpos,
				     14,
				     32,
				     32,
				     hwndT, HWND_TOP, tool->id, &btc, NULL);
	  if (!hwndTool)
	    GpiDeleteBitmap(hbm);
	}
      }
      if (hwndTool)
	tool->flags &= (~T_TEXT);
    }
    if (!hwndTool) {
      hwndTool = WinCreateWindow(hwndT,
				 WC_TOOLBUTTONS,
				 (!tool->text && tool->id >= IDM_COMMANDSTART
				  && tool->id <
				  IDM_QUICKTOOLSTART) ? command_title(tool->
								      id -
								      IDM_COMMANDSTART)
				 : tool->text,
				 BS_NOPOINTERFOCUS | BS_PUSHBUTTON, ctrlxpos,
				 2, 54, 24, hwndT, HWND_TOP, tool->id, NULL,
				 NULL);
      if (!hwndTool)
	Win_Error2(hwndT, HWND_DESKTOP, pszSrcFile, __LINE__,
		   IDS_WINCREATEWINDOW);
      tool->flags |= T_TEXT;
    }
    if (fToolTitles && !fTextTools) {
      hwndTool = WinCreateWindow(hwndT,
				 WC_STATIC,
				 tool->text,
				 SS_TEXT | DT_LEFT | DT_VCENTER,
				 ctrlxpos,
				 1,
				 32,
				 10,
				 hwndT,
				 HWND_TOP, tool->id + 25000, NULL, NULL);
      if (!hwndTool)
	Win_Error2(hwndT, HWND_DESKTOP, pszSrcFile, __LINE__,
		   IDS_WINCREATEWINDOW);
      else {
	SetPresParams(hwndTool,
		      &RGBGREY,
		      &RGBBLACK, &RGBGREY, GetPString(IDS_2SYSTEMVIOTEXT));
      }
    }
    ctrlxpos += ((tool->flags & T_TEXT) ? 55L : 33L);
    SetPresParams(WinWindowFromID(hwndT, tool->id),
		  NULL, NULL, NULL, GetPString(IDS_8HELVTEXT));
    tool = tool->next;
  }					// while tool

  hwndTool = WinCreateWindow(hwndT,
			     WC_BUTTON,
			     "#6010",
			     BS_NOPOINTERFOCUS |
			     BS_BITMAP | BS_PUSHBUTTON,
			     1,
			     19,
			     14,
			     13, hwndT, HWND_TOP, IDM_TOOLLEFT, NULL, NULL);
  if (!hwndTool)
    Win_Error2(hwndT, HWND_DESKTOP, pszSrcFile, __LINE__,
	       IDS_WINCREATEWINDOW);
  hwndTool =
    WinCreateWindow(hwndT, WC_BUTTON, "#6011",
		    BS_NOPOINTERFOCUS | BS_BITMAP | BS_PUSHBUTTON, 1, 4, 14,
		    13, hwndT, HWND_TOP, IDM_TOOLRIGHT, NULL, NULL);
  if (!hwndTool)
    Win_Error2(hwndT, HWND_DESKTOP, pszSrcFile, __LINE__,
	       IDS_WINCREATEWINDOW);
  if (resize)
    ResizeTools(hwndT);
}

static MRESULT EXPENTRY CommandLineProc(HWND hwnd, ULONG msg, MPARAM mp1,
					MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);
  static BOOL lbup = FALSE;

  switch (msg) {
  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, hwnd);
    return 0;

  case WM_SETFOCUS:
    if (!mp2 && !lbup) {

      PID pid;
      TID tid;

      if (WinQueryWindowUShort((HWND) mp1, QWS_ID) == COMMAND_BUTTON)
	break;
      if (!WinQueryWindowProcess((HWND) mp1, &pid, &tid) || pid == mypid)
	WinDestroyWindow(hwnd);
    }
    break;

  case UM_RESCAN:
    {
      CHAR cl[1024];

      *cl = 0;
      lbup = TRUE;
      if (WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    CmdLine2DlgProc,
		    FM3ModHandle, EXEC2_FRAME, MPFROMP(cl))) {
	lstrip(cl);
	WinSetWindowText(hwnd, cl);
      }
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
      PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    }
    return 0;

  case UM_SETUP:
    lbup = FALSE;
    return 0;

  case WM_BUTTON1DBLCLK:
    PostMsg(hwnd, UM_OPENWINDOWFORME, MPVOID, MPVOID);
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case COMMAND_BUTTON:
      if (!lbup)
	PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      break;
    }
    return 0;

  case UM_OPENWINDOWFORME:
    {
      static char directory[CCHMAXPATH], cl[1000];
      char **list = NULL;
      ULONG len;
      HWND hwndCnr;

      *directory = *cl = 0;
      strcpy(cl, GetCmdSpec(FALSE));
      strcat(cl, " /C ");
      len = strlen(cl);
      WinQueryWindowText(hwnd, 1000 - len, cl + len);
      bstrip(cl + len);
      if (strlen(cl) > len) {
	WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
		   UM_SETUP, MPFROMP(cl + len), MPVOID);
	WinQueryWindowText(hwndStatus, CCHMAXPATH, directory);
	bstrip(directory);
	if (*directory && (IsRoot(directory) || !IsFile(directory))) {
	  if (!FM2Command(directory, cl + len)) {
	    hwndCnr = TopWindow(hwndMain, (HWND) 0);
	    if (hwndCnr) {
	      hwndCnr = WinWindowFromID(hwndCnr, FID_CLIENT);
	      if (hwndCnr) {
		hwndCnr = WinWindowFromID(hwndCnr, DIR_CNR);
		if (hwndCnr)
		  list = BuildList(hwndCnr);
	      }
	    }
	    WinSetActiveWindow(HWND_DESKTOP, hwndCnr);
	    if (add_cmdline(cl + len, FALSE) && fSaveMiniCmds)
	      save_cmdlines(FALSE);
	    ExecOnList(hwndCnr,
		       cl,
		       WINDOWED | ((fKeepCmdLine) ?
				   SEPARATEKEEP : SEPARATE),
		       directory, list, NULL);
	    if (list)
	      free(list);
	    WinDestroyWindow(hwnd);
	    break;
	  }
	}
      }
      WinSendMsg(hwnd, EM_SETSEL, MPFROM2SHORT(0, 1024), MPVOID);
    }
    return 0;

  case WM_CHAR:
    if (!lbup && !(SHORT1FROMMP(mp1) & KC_KEYUP)) {
      if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY) {
	if ((SHORT1FROMMP(mp2) & 255) == '\r')
	  PostMsg(hwnd, UM_OPENWINDOWFORME, MPVOID, MPVOID);
	else if ((SHORT1FROMMP(mp2) & 0xff) == 0x1b)
	  WinDestroyWindow(hwnd);
	else if (SHORT2FROMMP(mp2) == VK_UP || SHORT2FROMMP(mp2) == VK_DOWN)
	  PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      }
    }
    else if ((SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
	     ((SHORT2FROMMP(mp2) == VK_UP ||
	       SHORT2FROMMP(mp2) == VK_DOWN) ||
	      (SHORT1FROMMP(mp2) == '\x1b') || (SHORT1FROMMP(mp2) == '\r')))
      return 0;
    break;

  case WM_DESTROY:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT), UM_RESCAN, MPVOID, MPVOID);
    lbup = FALSE;
    break;
  }
  return oldproc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY DriveBackProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_CREATE:
    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    break;

  case UM_SETUP:
    {
      RGB2 rgb;

      memset(&rgb, 0, sizeof(rgb));
      rgb.bRed = (BYTE)128;
      SetPresParams(hwnd,
		    &RGBGREY, &rgb, &RGBGREY, GetPString(IDS_8HELVTEXT));
      SetTargetDir(hwnd, TRUE);
    }
    return 0;

  case WM_SETFOCUS:
  case UM_FOCUSME:
  case WM_BUTTON1DOWN:
  case WM_BUTTON1UP:
  case WM_BUTTON2DOWN:
  case WM_BUTTON2UP:
  case WM_BUTTON3DOWN:
  case WM_BUTTON3UP:
    return CommonTextButton(hwnd, msg, mp1, mp2);

  case WM_MOUSEMOVE:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case UM_CLICKED:
  case UM_CLICKED3:
    PostMsg(hwndMain, WM_COMMAND, MPFROM2SHORT(IDM_SETTARGET, 0), MPVOID);
    return 0;

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_CONTROLPOINTER:
    if (!fNoFinger &&
	(SHORT1FROMMP(mp1) >= IDM_DRIVEA &&
	 SHORT1FROMMP(mp1) < IDM_DRIVEA + 26))
      return MRFROMLONG(hptrFinger);
    break;

  case WM_COMMAND:
    {
      CHAR dv[4];

      *dv = 0;
      WinQueryWindowText(WinWindowFromID(hwnd, SHORT1FROMMP(mp1) + 50),
			 2, dv);
      if (isalpha(*dv)) {

	HWND hwndActive;

	dv[1] = ':';
	dv[2] = '\\';
	dv[3] = 0;
	hwndActive = TopWindow(hwnd, (HWND) 0);
	if (hwndActive)
	  WinSendMsg(WinWindowFromID(hwndActive, FID_CLIENT),
		     UM_DRIVECMD, MPFROMP(dv), MPVOID);
      }
    }
    return 0;

  case WM_PAINT:
    PaintRecessedWindow(hwnd, (HPS) 0, TRUE, FALSE);
    break;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    return 0;
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY DriveProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  USHORT id;
  CHAR szDrv[CCHMAXPATH];

  static BOOL emphasized = FALSE;
  static HWND hwndMenu = NULLHANDLE;
  static USHORT helpid = 0;

  switch (msg) {
  case WM_MOUSEMOVE:
    if (fDrivebarHelp &&
	(!hwndBubble ||
	 WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd) &&
	!WinQueryCapture(HWND_DESKTOP)) {
      id = WinQueryWindowUShort(hwnd, QWS_ID);
      if (helpid != id) {
	helpid = id;
	PostMsg(MainObjectHwnd, UM_SETUP6, MPFROMLONG((ULONG) hwnd), MPVOID);
      }
      else
	helpid = 0;
    }
    break;

  case UM_SETUP6:
    if (helpid == WinQueryWindowUShort(hwnd, QWS_ID)) {
      if ((char *)mp1 &&
	  (!hwndBubble ||
	   WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd) &&
	  !WinQueryCapture(HWND_DESKTOP)) {

	RECTL rcl;
	POINTL ptl;

	WinQueryPointerPos(HWND_DESKTOP, &ptl);
	WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
	WinQueryWindowRect(hwnd, &rcl);
	if (WinPtInRect(WinQueryAnchorBlock(hwnd), &rcl, &ptl))
	  BubbleHelp(hwnd, FALSE, TRUE, FALSE, (char *)mp1);
      }
    }
    return 0;

  case WM_MENUEND:
    if (hwndMenu == (HWND) mp2) {
      WinDestroyWindow(hwndMenu);
      hwndMenu = (HWND) 0;
    }
    break;

  case WM_CONTEXTMENU:
    if (hwndMenu)
      WinDestroyWindow(hwndMenu);
    hwndMenu = (HWND) 0;
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    *szDrv = 0;
    WinQueryWindowText(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				       id + 50), sizeof(szDrv), szDrv);
    if (isalpha(*szDrv)) {
      hwndMenu = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, MAIN_DRIVES);
      if (hwndMenu) {
	BOOL rdy;
	CHAR chDrv = *szDrv;
	UINT iDrv;

	strcpy(szDrv + 2, "\\");
	MakeValidDir(szDrv);
	// Disable menus if MakeValidDir changes drive letter fixme this section doesn't do anything see treecnt.c
	rdy = toupper(*szDrv) == toupper(chDrv);
	iDrv = toupper(*szDrv) - 'A';
	if (!rdy || ~driveflags[iDrv] & DRIVE_REMOTE)
	  WinEnableMenuItem(hwndMenu, IDM_DETACH, FALSE);

	if (!rdy || driveflags[iDrv] & DRIVE_NOTWRITEABLE) {
	  WinEnableMenuItem(hwndMenu, IDM_MKDIR, FALSE);
	  WinEnableMenuItem(hwndMenu, IDM_FORMAT, FALSE);
	  WinEnableMenuItem(hwndMenu, IDM_OPTIMIZE, FALSE);
	  WinEnableMenuItem(hwndMenu, IDM_UNDELETE, FALSE);
	}
	if (!rdy || ~driveflags[iDrv] & DRIVE_REMOVABLE) {
	  WinEnableMenuItem(hwndMenu, IDM_EJECT, FALSE);
	  WinEnableMenuItem(hwndMenu, IDM_LOCK, FALSE);
	  WinEnableMenuItem(hwndMenu, IDM_UNLOCK, FALSE);
	}
	if (!rdy) {
	  WinEnableMenuItem(hwndMenu, IDM_INFO, FALSE);
	  WinEnableMenuItem(hwndMenu, IDM_ARCHIVE, FALSE);
	  WinEnableMenuItem(hwndMenu, IDM_SIZES, FALSE);
	  WinEnableMenuItem(hwndMenu, IDM_SHOWALLFILES, FALSE);
	  WinEnableMenuItem(hwndMenu, IDM_CHKDSK, FALSE);
	}
	/* fixme to be gone?
	   if (!rdy || ~driveflags[iDrv] & DRIVE_CDROM) {
	   WinEnableMenuItem(hwndMenu, IDM_CLOSETRAY, FALSE);
	   }
	 */
	PopupMenu(hwnd, hwnd, hwndMenu);
      }
    }
    return MRFROMSHORT(TRUE);

  case WM_BUTTON2CLICK:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    if (!(shiftstate & KC_CTRL))
      break;

  case WM_CHORD:
  case WM_BUTTON3CLICK:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    *szDrv = 0;
    WinQueryWindowText(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				       id + 50), sizeof(szDrv), szDrv);
    if (isalpha(*szDrv)) {
      strcat(szDrv, "\\");
      if (!FindDirCnrByName(szDrv, TRUE))
	OpenDirCnr((HWND) 0, hwndMain, hwndTree, FALSE, szDrv);
    }
    break;

  case WM_COMMAND:
    PostMsg(hwnd, UM_COMMAND, mp1, mp2);
    return 0;

  case UM_COMMAND:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    *szDrv = 0;
    WinQueryWindowText(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				       id + 50), sizeof(szDrv), szDrv);
    if (isalpha(*szDrv)) {
      strcat(szDrv, "\\");
      CommonDriveCmd(hwnd, szDrv, SHORT1FROMMP(mp1));
    }
    return 0;

  case DM_DRAGOVER:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    *szDrv = 0;
    WinQueryWindowText(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				       id + 50), sizeof(szDrv), szDrv);
    if (isalpha(*szDrv) &&
	!(driveflags[toupper(*szDrv) - 'A'] & DRIVE_NOTWRITEABLE)) {
      if (!emphasized) {
	emphasized = TRUE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
      if (AcceptOneDrop(hwnd, mp1, mp2))
	return MRFROM2SHORT(DOR_DROP, DO_MOVE);
      return MRFROM2SHORT(DOR_NEVERDROP, 0);
    }
    break;

  case DM_DRAGLEAVE:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    *szDrv = 0;
    WinQueryWindowText(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				       id + 50), sizeof(szDrv), szDrv);
    if (isalpha(*szDrv) &&
	!(driveflags[toupper(*szDrv) - 'A'] & DRIVE_NOTWRITEABLE)) {
      if (emphasized) {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
    }
    break;

  case DM_DROPHELP:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    *szDrv = 0;
    WinQueryWindowText(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				       id + 50), sizeof(szDrv), szDrv);
    if (isalpha(*szDrv) &&
	!(driveflags[toupper(*szDrv) - 'A'] & DRIVE_NOTWRITEABLE)) {
      DropHelp(mp1, mp2, hwnd, GetPString(IDS_DRIVEDROPHELP));
      return 0;
    }
    break;

  case DM_DROP:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    *szDrv = 0;
    WinQueryWindowText(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				       id + 50), sizeof(szDrv), szDrv);
    if (isalpha(*szDrv) &&
	!(driveflags[toupper(*szDrv) - 'A'] & DRIVE_NOTWRITEABLE)) {

      CNRDRAGINFO cnd;
      LISTINFO *li;
      ULONG action = UM_ACTION;

      if (emphasized) {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
      memset(&cnd, 0, sizeof(cnd));
      cnd.pDragInfo = (PDRAGINFO) mp1;
      cnd.pRecord = NULL;
      li = DoFileDrop(hwnd,
		      NULL,
		      TRUE, MPFROM2SHORT(TREE_CNR, CN_DROP), MPFROMP(&cnd));
      CheckPmDrgLimit(cnd.pDragInfo);
      if (li) {
	strcpy(li->targetpath, szDrv);
	strcat(li->targetpath, "\\");
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
	  li->type = WinDlgBox(HWND_DESKTOP,
			       hwndMain,
			       DropListProc,
			       FM3ModHandle, DND_FRAME, MPFROMP(&cl));
	  if (li->type == DID_ERROR)
		  Win_Error(DND_FRAME, HWND_DESKTOP, pszSrcFile, __LINE__,
			    "Drag & Drop Dialog");
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
	else {
	  if (!WinDlgBox(HWND_DESKTOP,
			 hwndMain,
			 WalkDlgProc,
			 FM3ModHandle,
			 WALK_FRAME,
			 MPFROMP(li->targetpath)) || !*li->targetpath) {
	    FreeListInfo(li);
	    return 0;
	  }
	}
	switch (li->type) {
	case DND_LAUNCH:
	  strcat(li->targetpath, " %a");
	  ExecOnList(hwndMain,
		     li->targetpath, PROMPT | WINDOWED, NULL, li->list, NULL);
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
	    li->type = (li->type == DO_MOVE) ? IDM_FAKEEXTRACTM :
	      IDM_FAKEEXTRACT;
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
	else
	  WinSendMsg(hwndTree, UM_ACTION, MPFROMP(li), MPFROMLONG(action));
      }
      return 0;
    }
    break;

  case WM_DESTROY:
    if (hwndMenu)
      WinDestroyWindow(hwndMenu);
    hwndMenu = (HWND) 0;
    break;
  }
  return PFNWPButton(hwnd, msg, mp1, mp2);
}

VOID BuildDriveBarButtons(HWND hwndT)
{
  register ULONG x, y = 0;
  ULONG ulDriveNum, ulDriveMap, iconid;
  CHAR s[8];
  HENUM henum;
  HWND hwndB;

  henum = WinBeginEnumWindows(hwndT);

  while ((hwndB = WinGetNextWindow(henum)) != NULLHANDLE)
    WinDestroyWindow(hwndB);

  WinEndEnumWindows(henum);
  if (fDrivebar) {
    DosError(FERR_DISABLEHARDERR);
    DosQCurDisk(&ulDriveNum, &ulDriveMap);
    for (x = 0; x < 26; x++) {
      if ((ulDriveMap & (1L << x)) && !(driveflags[x] & DRIVE_IGNORE)) {
	if (x > 1) {
	  if (driveflags[x] & DRIVE_CDROM)
	    iconid = CDROM_ICON;
	  else
	    iconid = (driveflags[x] & DRIVE_REMOVABLE) ?
	      REMOVABLE_ICON :
		     (driveflags[x] & DRIVE_VIRTUAL) ?
		      VIRTUAL_ICON :
	      (driveflags[x] & DRIVE_REMOTE) ?
	      REMOTE_ICON :
		     (driveflags[x] & DRIVE_RAMDISK) ?
		      RAMDISK_ICON :
		     (driveflags[x] & DRIVE_ZIPSTREAM) ?
		      ZIPSTREAM_ICON :DRIVE_ICON;
	}
	else
	  iconid = FLOPPY_ICON;
	sprintf(s, "#%lu", iconid);
	hwndB = WinCreateWindow(hwndT,
				WC_DRIVEBUTTONS,
				s,
				BS_NOPOINTERFOCUS | BS_BITMAP | BS_PUSHBUTTON,
				0,
				0,
				28,
				18,
				hwndT, HWND_TOP, y + IDM_DRIVEA, NULL, NULL);
	if (!hwndB)
	  Win_Error2(hwndT, HWND_DESKTOP, pszSrcFile, __LINE__,
		     IDS_WINCREATEWINDOW);
	else {
	  WinSetWindowPos(hwndB, HWND_BOTTOM, 0, 0, 0, 0, SWP_ZORDER);
	  sprintf(s, "%c:", (CHAR) x + 'A');
	  hwndB = WinCreateWindow(hwndT,
				  WC_STATIC,
				  s,
				  SS_TEXT | DT_LEFT | DT_VCENTER,
				  0,
				  0,
				  10,
				  18,
				  hwndT,
				  HWND_TOP, y + IDM_DRIVEATEXT, NULL, NULL);
	  if (!hwndB)
	    Win_Error2(hwndT, HWND_DESKTOP, pszSrcFile, __LINE__,
		       IDS_WINCREATEWINDOW);
	  else {
	    SetPresParams(hwndB,
			  &RGBGREY,
			  &RGBBLACK, &RGBGREY, GetPString(IDS_6HELVTEXT));
	    WinSetWindowPos(hwndB, HWND_BOTTOM, 0, 0, 0, 0, SWP_ZORDER);
	  }
	  y++;
	}
      }
    }					// for
  }					// if drivebar
  PostMsg(WinQueryWindow(hwndT, QW_PARENT),
	  WM_UPDATEFRAME, MPFROMLONG(FCF_SIZEBORDER), MPVOID);
}

VOID ResizeDrives(HWND hwndT, long xwidth)
{
  register ULONG ctrlxpos = 2, ctrlypos = 0, ctrlxsize;
  HENUM henum;
  HWND hwndB;
  RECTL rcl;

  DriveLines = 0;
  if (!fDrivebar)
    return;
  if (!xwidth) {
    WinQueryWindowRect(hwndT, &rcl);
    xwidth = rcl.xRight - ((WinQuerySysValue(HWND_DESKTOP,
					     SV_CYSIZEBORDER) * 2) + 2);
  }
  henum = WinBeginEnumWindows(hwndT);
  while ((hwndB = WinGetNextWindow(henum)) != NULLHANDLE) {
    if (WinQueryWindowUShort(hwndB, QWS_ID) > IDM_DRIVEA + 27)
      ctrlxsize = 10;
    else
      ctrlxsize = 28;
    WinSetWindowPos(hwndB,
		    HWND_TOP,
		    ctrlxpos, ctrlypos, ctrlxsize, 18, SWP_MOVE | SWP_SHOW);
    ctrlxpos += (ctrlxsize + 2);
    if (ctrlxsize == 10) {
      if (ctrlxpos + (42 + ((fShowTarget && DriveLines == 0) ?
			    256 : 0)) > xwidth) {
	ctrlxpos = 2;
	ctrlypos += 18;
	DriveLines++;
      }
    }
  }
  if (ctrlxpos == 2 && DriveLines)
    DriveLines--;
}

MRESULT EXPENTRY StatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static HWND hwndE = (HWND) 0, hwndB = (HWND) 0;
  static CHAR lastcmd[1024] = "";

  switch (msg) {
  case WM_CREATE:
    {
      MRESULT mr = PFNWPStatic(hwnd, msg, mp1, mp2);

      SetPresParams(hwnd,
		    &RGBGREY,
		    &RGBBLACK, &RGBGREY, GetPString(IDS_8HELVBOLDTEXT));
      return mr;
    }

  case WM_PRESPARAMCHANGED:
    if (fRunning) {

      ULONG AttrFound, AttrValue[64], cbRetLen;

      cbRetLen = WinQueryPresParam(hwnd,
				   (ULONG) mp1,
				   0,
				   &AttrFound,
				   (ULONG) sizeof(AttrValue), &AttrValue, 0);
      if (cbRetLen) {
	switch (AttrFound) {
	case PP_FONTNAMESIZE:
	  PostMsg(WinQueryWindow(hwnd, QW_PARENT),
		  WM_UPDATEFRAME, MPFROMLONG(FCF_SIZEBORDER), MPVOID);
	  break;
	}
      }
    }
    break;

  case WM_CONTEXTMENU:
    PostMsg(hwndMain, UM_CONTEXTMENU, MPVOID, MPVOID);
    return MRFROMSHORT(TRUE);

  case WM_BEGINDRAG:
    if (hwndTree) {

      SWP swp;
      ULONG fl = SWP_ACTIVATE | SWP_SHOW | SWP_ZORDER;

      WinQueryWindowPos(hwndTree, &swp);
      if (!(swp.fl & SWP_MAXIMIZE))
	fl |= SWP_RESTORE;
      WinSetWindowPos(hwndTree, HWND_TOP, 0, 0, 0, 0, fl);
    }
    break;

  case UM_SETUP:
    if (mp1)
      strcpy(lastcmd, (CHAR *) mp1);
    return 0;

  case UM_RESCAN:
    if (hwndE && WinIsWindow(WinQueryAnchorBlock(hwnd), hwndE))
      WinDestroyWindow(hwndE);
    if (hwndB && WinIsWindow(WinQueryAnchorBlock(hwnd), hwndB))
      WinDestroyWindow(hwndB);
    hwndE = hwndB = (HWND) 0;
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case COMMAND_BUTTON:
      PostMsg(hwndE, msg, mp1, mp2);
      break;
    }
    return 0;

  case WM_MOUSEMOVE:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);
      char *s = NULL;

      if (fOtherHelp) {
	if ((!hwndBubble || WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd)
	    && !WinQueryCapture(HWND_DESKTOP)) {
	  switch (id) {
	  case IDM_ATTRS:
	    if (WinQueryWindowTextLength(hwnd))
	      s = GetPString(IDS_ATTRSBUTTONHELP);
	    break;
	  case IDM_INFO:
	    if (WinQueryWindowTextLength(hwnd))
	      s = GetPString(IDS_INFOBUTTONHELP);
	    break;
	  case IDM_RENAME:
	    if (WinQueryWindowTextLength(hwnd))
	      s = GetPString(IDS_NAMEBUTTONHELP);
	    break;
	  case MAIN_STATUS2:
	    if (!hwndE)
	      s = GetPString(IDS_STATUS2HELP);
	    break;
	  default:
	    break;
	  }
	  if (s)
	    MakeBubble(hwnd, FALSE, s);
	  else if (hwndBubble)
	    WinDestroyWindow(hwndBubble);
	}
      }
      switch (id) {
      case IDM_ATTRS:
      case IDM_INFO:
      case IDM_RENAME:
      case MAIN_STATUS2:
	return CommonTextProc(hwnd, msg, mp1, mp2);
      default:
	break;
      }
    }
    break;

  case WM_BUTTON2DOWN:
  case WM_BUTTON2UP:
  case UM_FOCUSME:
    return CommonTextButton(hwnd, msg, mp1, mp2);

  case WM_BUTTON1DOWN:
  case WM_BUTTON1UP:
  case WM_BUTTON3DOWN:
  case WM_BUTTON3UP:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id) {
      case IDM_ATTRS:
      case IDM_INFO:
      case IDM_RENAME:
      case MAIN_STATUS2:
	return CommonTextButton(hwnd, msg, mp1, mp2);
      default:
	PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
	break;
      }
    }
    break;

  case WM_SETFOCUS:
    if (mp2) {

      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      if (id == MAIN_STATUS2 && hwndE)
	WinSendMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      else
	return CommonTextButton(hwnd, msg, mp1, mp2);
    }
    break;

  case WM_BUTTON1CLICK:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      if (id == MAIN_STATUS) {
	if (SHORT2FROMMP(mp2) & KC_CTRL)
	  PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				  FID_CLIENT),
		  WM_COMMAND, MPFROM2SHORT(IDM_WINDOWDLG, 0), MPVOID);
	else if (hwndTree)
	  PostMsg(hwndTree, UM_TIMER, MPVOID, MPVOID);
      }
    }
    break;

  case UM_CLICKED:
  case UM_CLICKED3:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      if (id == MAIN_STATUS2 && !hwndE) {

	SWP swp;
	CHAR directory[CCHMAXPATH];
	PFNWP oldproce;

	*directory = 0;
	TopWindowName(hwndMain, (HWND) 0, directory);
	WinQueryWindowPos(hwnd, &swp);
	hwndB = WinCreateWindow(hwnd,
				WC_BUTTON,
				"+",
				WS_VISIBLE | BS_PUSHBUTTON |
				BS_NOPOINTERFOCUS,
				swp.cx - swp.cy,
				0,
				swp.cy,
				swp.cy,
				hwnd, HWND_TOP, COMMAND_BUTTON, NULL, NULL);
	if (!hwndB)
	  Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
	hwndE = WinCreateWindow(hwnd,
				WC_ENTRYFIELD,
				NULL,
				WS_VISIBLE | ES_AUTOSCROLL,
				0,
				0,
				swp.cx - swp.cy,
				swp.cy,
				hwnd, HWND_TOP, COMMAND_LINE, NULL, NULL);
	if (!hwndE)
	  Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
	if (!hwndE || !hwndB) {
	  PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	  return 0;
	}
	WinSendMsg(hwndE, EM_SETTEXTLIMIT, MPFROM2SHORT(1024, 0), MPVOID);
	WinSetWindowText(hwndStatus, directory);
	if (*lastcmd)
	  WinSetWindowText(hwndE, lastcmd);
	else
	  WinSetWindowText(hwndE, GetPString(IDS_HELPCMDTEXT));
	oldproce = WinSubclassWindow(hwndE, (PFNWP) CommandLineProc);
	if (oldproce)
	  WinSetWindowPtr(hwndE, QWL_USER, (PVOID) oldproce);
	PostMsg(hwndE, UM_FOCUSME, MPVOID, MPVOID);
	PostMsg(hwndE, EM_SETSEL, MPFROM2SHORT(0, 1024), MPVOID);
	return 0;
      }
      if (msg == UM_CLICKED3 || (SHORT2FROMMP(mp2) & KC_CTRL)) {
	switch (id) {
	case IDM_ATTRS:
	  id = IDM_SORTSIZE;
	  break;
	case IDM_INFO:
	  id = IDM_SORTLWDATE;
	  break;
	case IDM_RENAME:
	  id = IDM_SORTFILENAME;
	  break;
	}
      }
      PostMsg(WinQueryWindow(hwnd, QW_PARENT),
	      WM_COMMAND, MPFROM2SHORT(id, 0), MPVOID);
    }
    return 0;

  case WM_PAINT:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id) {
      case IDM_ATTRS:
      case IDM_INFO:
      case IDM_RENAME:
      case MAIN_STATUS2:
	PaintRecessedWindow(hwnd, (HPS) 0, TRUE, FALSE);
	break;
      default:
	PaintRecessedWindow(hwnd, (HPS) 0, FALSE, TRUE);
	break;
      }
      if (id == IDM_RENAME) {

	HPS hps;

	hps = WinBeginPaint(hwnd, (HPS) 0, NULL);
	if (hps) {
	  PaintSTextWindow(hwnd, hps);
	  WinEndPaint(hps);
	}
	return 0;
      }
    }
    break;

  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ToolBackProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_CREATE:
    hwndToolback = hwnd;
    SetPresParams(hwnd,
		  &RGBGREY, &RGBBLACK, &RGBGREY, GetPString(IDS_8HELVTEXT));
    break;

  case WM_MOUSEMOVE:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    break;

  case WM_CONTROLPOINTER:
    if (!fNoFinger && SHORT1FROMMP(mp1) < 25000)
      return MRFROMLONG(hptrFinger);
    break;

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_COMMAND:
  case UM_COMMAND:
    return WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				      FID_CLIENT), msg, mp1, mp2);

  case WM_PAINT:
    {
      HPS hps;
      RECTL rcl;

      hps = WinBeginPaint(hwnd, (HPS) 0, NULL);
      if (hps) {
	WinQueryWindowRect(hwnd, &rcl);
	WinFillRect(hps, &rcl, CLR_PALEGRAY);
	WinEndPaint(hps);
      }
      PaintRecessedWindow(hwnd, (HPS) 0, TRUE, FALSE);
    }
    break;

  case UM_SETUP:
    {
      USHORT id;
      TOOL *tool;

      id = SHORT1FROMMP(mp1);
      tool = find_tool(id);
      if (tool) {
	del_tool(tool);
	WinShowWindow(WinWindowFromID(hwnd, id), FALSE);
	if (fToolTitles)
	  WinShowWindow(WinWindowFromID(hwnd, id + 25000), FALSE);
	ResizeTools(hwnd);
      }
    }
    return 0;

  case WM_CHORD:
    {
      USHORT id;

      id = (USHORT) WinDlgBox(HWND_DESKTOP,
			      hwnd,
			      AddToolProc,
			      FM3ModHandle, ADDBTN_FRAME, MPVOID);
      if (id && id != (USHORT) - 1)
	WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				   FID_CLIENT),
		   WM_COMMAND,
		   MPFROM2SHORT(IDM_CREATETOOL, 0), MPFROM2SHORT(id, 0));
    }
    break;

  case WM_CONTEXTMENU:
    if (WinDlgBox(HWND_DESKTOP,
		  hwnd, ToolIODlgProc, FM3ModHandle, SVBTN_FRAME, MPVOID))
      BuildTools(hwnd, TRUE);
    return MRFROMSHORT(TRUE);

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    return 0;

  case WM_DESTROY:
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

static VOID AdjustSizeOfClient(PSWP pswp, PRECTL prectl)
{
  SWP swp;
  RECTL rectl;

  if (fFreeTree)
    return;
  if (pswp) {
    if (WinQueryWindowPos(hwndTree, &swp) &&
	!(swp.fl & (SWP_MINIMIZE | SWP_HIDE | SWP_MAXIMIZE))) {
      pswp->x = swp.cx;
      pswp->cx -= swp.cx;
    }
  }
  if (prectl) {
    if (WinQueryWindowPos(hwndTree, &swp) &&
	!(swp.fl & (SWP_MINIMIZE | SWP_HIDE | SWP_MAXIMIZE)) &&
	WinQueryWindowRect(hwndTree, &rectl)) {
      prectl->xLeft = rectl.xRight;
      prectl->xRight -= rectl.xRight;
    }
  }
}

VOID FillClient(HWND hwndClient, PSWP pswp, PRECTL prectl, BOOL avoidtree)
{
  ULONG adjust;

  adjust = WinQuerySysValue(HWND_DESKTOP, SV_CYICON);
  if (pswp)
    WinQueryWindowPos(hwndClient, pswp);
  if (prectl)
    WinQueryWindowRect(hwndClient, prectl);
  if (avoidtree && !fFreeTree)
    AdjustSizeOfClient(pswp, prectl);
  if (prectl)
    prectl->yBottom += adjust;
  if (pswp) {
    if (!avoidtree || fFreeTree)
      pswp->x = 0;
    pswp->y = adjust;
    if (pswp->cy >= adjust)
      pswp->cy -= adjust;
    else
      pswp->cy = 0;
  }
}

static VOID MoveChildrenAwayFromTree(HWND hwndClient)
{
  SWP swpC, swpT, swp;
  USHORT id;
  HWND hwndChild;
  HENUM henum;

  if (fFreeTree)
    return;
  WinQueryWindowPos(hwndClient, &swpC);
  if (swpC.fl & (SWP_MINIMIZE | SWP_HIDE))
    return;
  WinQueryWindowPos(hwndTree, &swpT);
  henum = WinBeginEnumWindows(hwndClient);
  while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    id = WinQueryWindowUShort(hwndChild, QWS_ID);
    if (!id || id == TREE_FRAME)
      continue;
    WinQueryWindowPos(hwndChild, &swp);
    if (!(swp.fl & (SWP_MAXIMIZE | SWP_HIDE | SWP_MINIMIZE))) {
      if (swp.x < swpT.cx) {
	swp.x = swpT.cx;
	if (swp.x + swp.cx > swpC.cx)
	  swp.cx = swpC.cx - swp.x;
	if (swp.cx > 24)
	  WinSetWindowPos(hwndChild, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
			  SWP_SIZE | SWP_MOVE | SWP_SHOW);
      }
    }
  }
  WinEndEnumWindows(henum);
}

static VOID ArrangeIcons(HWND hwndClient)
{
  HENUM henum;
  HWND hwndChild;
  SWP swp;

  henum = WinBeginEnumWindows(hwndClient);
  while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    WinQueryWindowPos(hwndChild, &swp);
    if (swp.fl & (SWP_MINIMIZE | SWP_HIDE)) {
      WinSetWindowUShort(hwndChild, QWS_XMINIMIZE, (USHORT) - 1);
      WinSetWindowUShort(hwndChild, QWS_YMINIMIZE, (USHORT) - 1);
      WinSetWindowPos(hwndChild, HWND_TOP, 0, 0, 0, 0,
		      SWP_MOVE | SWP_SHOW | SWP_FOCUSDEACTIVATE);
    }
  }
  WinEndEnumWindows(henum);
}

static VOID NextChild(HWND hwndClient, BOOL previous)
{
  HENUM henum;
  HWND hwndActive, hwndNext, hwndPrev = (HWND) 0;
  BOOL next = FALSE, once = FALSE;

  previous = !previous;

  hwndActive = WinQueryActiveWindow(hwndClient);
  if (!hwndActive)
    next = TRUE;
  henum = WinBeginEnumWindows(hwndClient);
  for (;;) {
    hwndNext = WinGetNextWindow(henum);
    if (hwndNext) {
      if (!WinQueryWindowUShort(hwndNext, QWS_ID))
	continue;
      if (next)
	break;
      if (hwndNext == hwndActive) {
	if (!previous && hwndPrev) {
	  hwndNext = hwndPrev;
	  break;
	}
	else if (previous)
	  next = TRUE;
      }
      hwndPrev = hwndNext;
    }
    else {
      if ((!next && previous) || once)
	break;
      else if (!previous) {
	hwndNext = hwndPrev;
	break;
      }
      else
	once = next = TRUE;
    }
  }
  WinEndEnumWindows(henum);

  if (hwndNext && hwndNext != hwndActive) {
    WinSetWindowPos(hwndNext, HWND_TOP, 0, 0, 0, 0,
		    SWP_ZORDER | SWP_ACTIVATE);
    WinSetWindowPos(hwndActive, ((previous) ? HWND_BOTTOM : hwndNext), 0, 0,
		    0, 0, SWP_ZORDER);
  }
}

BOOL CloseChildren(HWND hwndClient)
{
  HENUM henum;
  HWND hwndChild;
  BOOL ret = FALSE;

  fNoTileUpdate = TRUE;
  henum = WinBeginEnumWindows(hwndClient);
  while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    if (hwndChild != hwndTree) {
      WinSendMsg(WinWindowFromID(hwndChild, FID_CLIENT),
		 WM_SAVEAPPLICATION, MPVOID, MPVOID);
      if (WinSendMsg(WinWindowFromID(hwndChild, FID_CLIENT),
		     WM_CLOSE, MPVOID, MPVOID)) {
	ret = TRUE;
	break;
      }
    }
  }
  WinEndEnumWindows(henum);
  fNoTileUpdate = FALSE;
  return ret;
}

BOOL CloseDirCnrChildren(HWND hwndClient)
{
  /* returns TRUE if a directory container window was told to close */

  HENUM henum;
  HWND hwndChild, hwndDir, hwndTemp;
  BOOL ret = FALSE;

  fNoTileUpdate = TRUE;
  henum = WinBeginEnumWindows(hwndClient);
  while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    if (hwndChild != hwndTree) {
      hwndTemp = WinWindowFromID(hwndChild, FID_CLIENT);
      if (hwndTemp) {
	hwndDir = WinWindowFromID(hwndTemp, DIR_CNR);
	if (hwndDir) {
	  WinSendMsg(WinWindowFromID(hwndChild, FID_CLIENT),
		     WM_CLOSE, MPVOID, MPVOID);
	  ret = TRUE;
	}
      }
    }
  }
  WinEndEnumWindows(henum);
  fNoTileUpdate = FALSE;
  return ret;
}

/** Save directory container state
 * @param hwndClient Client window handle
 * @param pszStateName State name to save, NULL to save global state
 * @returns Number of directory container windows that were saved or -1 if error
 * @seealso RestoreDirCnrState
 */

#define STATE_NAME_MAX_BYTES	256

INT SaveDirCnrState(HWND hwndClient, PSZ pszStateName)
{
  HENUM henum;
  HWND hwndChild, hwndDir, hwndC;
  ULONG numsaves = 0, flWindowAttr;
  CHAR szPrefix[STATE_NAME_MAX_BYTES + 1];
  CHAR szKey[STATE_NAME_MAX_BYTES + 80];
  CHAR szDir[CCHMAXPATH];
  SWP swp;
  INT nSaved = 0;
  DIRCNRDATA *dcd;

  if (!pszStateName)
    strcpy(szPrefix, NullStr);
  else {
    if (strlen(pszStateName) > sizeof(szPrefix) - 2) {
      Runtime_Error(pszSrcFile, __LINE__, "SaveDirCnrState");
      return -1;
    }
    sprintf(szPrefix, "%s.", pszStateName);
  }

  henum = WinBeginEnumWindows(hwndClient);
  while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    if (hwndChild != hwndTree) {
      hwndC = WinWindowFromID(hwndChild, FID_CLIENT);
      if (hwndC) {
	hwndDir = WinWindowFromID(hwndC, DIR_CNR);
	if (hwndDir) {
	  WinQueryWindowPos(hwndChild, &swp);
	  *szDir = 0;
	  WinSendMsg(hwndC, UM_CONTAINERDIR, MPFROMP(szDir), MPVOID);
	  if (*szDir) {
	   if (driveflags[toupper(*szDir) - 'A'] & DRIVE_NOPRESCAN)
	     continue;
	    sprintf(szKey, "%sDirCnrPos.%lu", szPrefix, numsaves);
	    PrfWriteProfileData(fmprof, FM3Str, szKey, (PVOID) & swp,
				sizeof(SWP));
	    dcd =
	      WinQueryWindowPtr(WinWindowFromID(hwndC, DIR_CNR), QWL_USER);
	    if (dcd) {
	      sprintf(szKey, "%sDirCnrSort.%lu", szPrefix, numsaves);
	      PrfWriteProfileData(fmprof, FM3Str, szKey, (PVOID) & dcd->sortFlags,
				  sizeof(INT));
	      sprintf(szKey, "%sDirCnrFilter.%lu", szPrefix, numsaves);
	      PrfWriteProfileData(fmprof, FM3Str, szKey, (PVOID) & dcd->mask,
				  sizeof(MASK));
	      sprintf(szKey, "%sDirCnrView.%lu", szPrefix, numsaves);
	      flWindowAttr = dcd->flWindowAttr;
	      if (!fLeaveTree && (flWindowAttr & CV_TREE)) {
		flWindowAttr &= (~(CV_TREE | CV_ICON | CV_DETAIL | CV_TEXT));
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
	      }
	      PrfWriteProfileData(fmprof, FM3Str, szKey, (PVOID) & flWindowAttr,
				  sizeof(ULONG));
	    }
	    sprintf(szKey, "%sDirCnrDir.%lu", szPrefix, numsaves++);
	    PrfWriteProfileString(fmprof, FM3Str, szKey, szDir);
	    nSaved++;
	  }
	}
      }
    }
  } // while
  WinEndEnumWindows(henum);

  if (nSaved) {
    if (WinQueryWindowPos(hwndTree, &swp)) {
      sprintf(szKey, "%sLastTreePos", szPrefix);
      PrfWriteProfileData(fmprof, FM3Str, szKey, (PVOID) & swp, sizeof(SWP));
    }
    sprintf(szKey, "%sNumDirsLastTime", szPrefix);
    PrfWriteProfileData(fmprof, FM3Str, szKey, (PVOID) & numsaves, sizeof(ULONG));
    WinQueryWindowPos(WinQueryWindow(hwndClient, QW_PARENT), &swp);
    sprintf(szKey, "%sMySizeLastTime", szPrefix);
    PrfWriteProfileData(fmprof, FM3Str, szKey, (PVOID) & swp, sizeof(SWP));
  }

  return nSaved;
}

static VOID TransformSwp(PSWP pswp, double xtrans, double ytrans)
{
  SWP swp;
  HWND hwnd;

  if ((LONG) pswp->x < 0L) {
    pswp->cx -= abs(pswp->x);
    pswp->x = 0;
  }
  if ((LONG) pswp->y < 0L) {
    pswp->cy -= abs(pswp->y);
    pswp->y = 0;
  }
  pswp->x = (LONG) (((double)pswp->x * 100.0) / xtrans);
  pswp->cx = (LONG) (((double)pswp->cx * 100.0) / xtrans);
  pswp->y = (LONG) (((double)pswp->y * 100.0) / ytrans);
  pswp->cy = (LONG) (((double)pswp->cy * 100.0) / ytrans);
  if (pswp->hwnd) {
    hwnd = WinQueryWindow(pswp->hwnd, QW_PARENT);
    if (hwnd) {
      if (WinQueryWindowPos(hwnd, &swp)) {
	if (pswp->x > swp.cx)
	  pswp->x = (swp.cx > 24) ? swp.cx - 24 : swp.cx;
	if (pswp->y > swp.cy)
	  pswp->y = (swp.cy > 24) ? swp.cy - 24 : swp.cy;
	if (pswp->x + pswp->cx > swp.cx)
	  pswp->cx = swp.cx - pswp->x;
	if (pswp->y + pswp->cy > swp.cy)
	  pswp->cy = swp.cy - pswp->y;
      }
    }
  }
}

/** Restore directory container state
 * @param hwndClient Client window handle
 * @param pszStateName State name to restore, NULL to restore global state
 * @returns TRUE if one or more directory containers were opened
 * @seealso SaveDirCnrState
 */

static BOOL RestoreDirCnrState(HWND hwndClient, PSZ pszStateName, BOOL noview)
{
  CHAR szKey[STATE_NAME_MAX_BYTES + 80];
  CHAR szDir[CCHMAXPATH];
  CHAR szPrefix[STATE_NAME_MAX_BYTES + 2];
  HWND hwndDir, hwndC;
  SWP swp, swpO, swpN;
  ULONG size, numsaves = 0L, x;
  double xtrans, ytrans;
  BOOL fRestored = FALSE;
  DIRCNRDATA *dcd;

  if (!pszStateName)
    strcpy(szPrefix, NullStr);
  else {
    if (strlen(pszStateName) > sizeof(szPrefix) - 2) {
      Runtime_Error(pszSrcFile, __LINE__, "RestoreDirCnrState");
      return fRestored;
    }
    sprintf(szPrefix, "%s.", pszStateName);
  }

  size = sizeof(SWP);
  sprintf(szKey, "%sMySizeLastTime", szPrefix);
  if (!PrfQueryProfileData(fmprof,
			   FM3Str,
			   szKey,
			   (PVOID) & swpO,
			   &size) ||
      size != sizeof(SWP) || !swp.cx || !swp.cy)
  {
    WinQueryWindowPos(WinQueryWindow(hwndClient, QW_PARENT), &swpO);
  }
  if (!pszStateName || !strcmp(pszStateName, GetPString(IDS_FM2TEMPTEXT)))
    PrfWriteProfileData(fmprof, FM3Str, szKey, NULL, 0L);
  WinQueryWindowPos(WinQueryWindow(hwndClient, QW_PARENT), &swpN);
  if (swpN.fl & (SWP_MINIMIZE | SWP_HIDE))
    swpN = swpO;
  xtrans = ((double)swpO.cx * 100.0) / (double)swpN.cx;
  ytrans = ((double)swpO.cy * 100.0) / (double)swpN.cy;
  size = sizeof(SWP);
  sprintf(szKey, "%sLastTreePos", szPrefix);
  if (PrfQueryProfileData(fmprof, FM3Str, szKey, (PVOID) & swp, &size)) {
    if (!pszStateName || !strcmp(pszStateName, GetPString(IDS_FM2TEMPTEXT)))
      PrfWriteProfileData(fmprof, FM3Str, szKey, NULL, 0L);
    swp.hwnd = hwndTree;
    TransformSwp(&swp, xtrans, ytrans);
    if (!fFreeTree) {
      WinQueryWindowPos(hwndClient, &swpN);
      swp.x = 0;
      swp.y = (swpN.cy - swp.cy);
    }
    if (!(swp.fl & (SWP_MINIMIZE | SWP_HIDE))) {
      swp.fl &= (~SWP_ACTIVATE);
      WinSetWindowPos(hwndTree,
		      HWND_TOP,
		      swp.x,
		      swp.y,
		      swp.cx,
		      swp.cy,
		      swp.fl | SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER);
    }
    else {
      WinSetWindowPos(hwndTree,
		      HWND_TOP, 0, 0, 0, 0, SWP_MINIMIZE | SWP_SHOW);
      WinSetWindowUShort(hwndTree, QWS_XRESTORE, (USHORT) swp.x);
      WinSetWindowUShort(hwndTree, QWS_CXRESTORE, (USHORT) swp.cx);
      WinSetWindowUShort(hwndTree, QWS_YRESTORE, (USHORT) swp.y);
      WinSetWindowUShort(hwndTree, QWS_CYRESTORE, (USHORT) swp.cy);
    }
  }
  size = sizeof(ULONG);
  sprintf(szKey, "%sNumDirsLastTime", szPrefix);
  if (PrfQueryProfileData(fmprof,
			  FM3Str, szKey, (PVOID) & numsaves, &size) && numsaves) {
    if (!pszStateName || !strcmp(pszStateName, GetPString(IDS_FM2TEMPTEXT)))
      PrfWriteProfileData(fmprof, FM3Str, szKey, NULL, 0L);
    for (x = 0; x < numsaves; x++) {
      sprintf(szKey, "%sDirCnrPos.%lu", szPrefix, x);
      size = sizeof(SWP);
      if (PrfQueryProfileData(fmprof, FM3Str, szKey, (PVOID) & swp, &size)) {
	if (!pszStateName || !strcmp(pszStateName, GetPString(IDS_FM2TEMPTEXT)))
	  PrfWriteProfileData(fmprof, FM3Str, szKey, NULL, 0L);
	sprintf(szKey, "%sDirCnrDir.%lu", szPrefix, x);
	size = sizeof(szDir);
	if (PrfQueryProfileData(fmprof, FM3Str, szKey, (PVOID) szDir, &size)) {
	  if (!pszStateName || !strcmp(pszStateName, GetPString(IDS_FM2TEMPTEXT)))
	    PrfWriteProfileData(fmprof, FM3Str, szKey, NULL, 0L);
	  if (driveflags[toupper(*szDir) - 'A'] & DRIVE_NOPRESCAN) {
	    PrfWriteProfileData(fmprof, FM3Str, szKey, NULL, 0L);
	    continue;
	  }
	  hwndDir = (HWND) WinSendMsg(hwndClient,
				      UM_SETDIR,
				      MPFROMP(szDir), MPFROMLONG(1));
	  if (hwndDir) {
	    hwndC = WinWindowFromID(hwndDir, FID_CLIENT);
	    if (hwndC) {
	      dcd = WinQueryWindowPtr(WinWindowFromID(hwndC, DIR_CNR),
				      QWL_USER);
	      if (dcd) {
		size = sizeof(INT);
		sprintf(szKey, "%sDirCnrSort.%lu", szPrefix, x);
		if (PrfQueryProfileData(fmprof,
					FM3Str,
					szKey,
					(PVOID) & dcd->sortFlags,
					&size) && size == sizeof(INT)) {
		  if (!dcd->sortFlags)
		    dcd->sortFlags = SORT_PATHNAME;
		}
		if (!pszStateName || !strcmp(pszStateName, GetPString(IDS_FM2TEMPTEXT)))
		  PrfWriteProfileData(fmprof, FM3Str, szKey, NULL, 0L);
		size = sizeof(MASK);
		sprintf(szKey, "%sDirCnrFilter.%lu", szPrefix, x);
		if (PrfQueryProfileData(fmprof,
					FM3Str,
					szKey,
					(PVOID) & dcd->mask, &size) && size) {
		  if (*dcd->mask.szMask)
		    WinSendMsg(WinWindowFromID(hwndC, DIR_CNR),
			       UM_FILTER, MPFROMP(dcd->mask.szMask), MPVOID);
		}
		*(dcd->mask.prompt) = 0;
		if (!pszStateName || !strcmp(pszStateName, GetPString(IDS_FM2TEMPTEXT)))
		  PrfWriteProfileData(fmprof, FM3Str, szKey, NULL, 0L);
		size = sizeof(ULONG);
		sprintf(szKey, "%sDirCnrView.%lu", szPrefix, x);
		if (!noview) {
		  if (PrfQueryProfileData(fmprof,
					  FM3Str,
					  szKey,
					  (PVOID) & dcd->flWindowAttr,
					  &size) && size == sizeof(ULONG)) {

		    CNRINFO cnri;

		    memset(&cnri, 0, sizeof(CNRINFO));
		    cnri.cb = sizeof(CNRINFO);
		    if (WinSendMsg(WinWindowFromID(hwndC, DIR_CNR),
				   CM_QUERYCNRINFO,
				   MPFROMP(&cnri),
				   MPFROMLONG(sizeof(CNRINFO)))) {
		      cnri.flWindowAttr = dcd->flWindowAttr;
		      WinSendMsg(WinWindowFromID(hwndC, DIR_CNR),
				 CM_SETCNRINFO,
				 MPFROMP(&cnri),
				 MPFROMLONG(CMA_FLWINDOWATTR));
		    }
		  }
		}
		if (!pszStateName || !strcmp(pszStateName, GetPString(IDS_FM2TEMPTEXT)))
		  PrfWriteProfileData(fmprof, FM3Str, szKey, NULL, 0L);
	      }
	    }
	    fRestored = TRUE;
	    swp.hwnd = hwndDir;
	    TransformSwp(&swp, xtrans, ytrans);
	    if (!fAutoTile && !(swp.fl & (SWP_HIDE | SWP_MINIMIZE)))
	      WinSetWindowPos(hwndDir,
			      HWND_TOP,
			      swp.x,
			      swp.y,
			      swp.cx,
			      swp.cy,
			      swp.fl | SWP_MOVE |
			      SWP_SIZE | SWP_SHOW | SWP_ZORDER |
			      SWP_ACTIVATE);
	    else if (swp.fl & (SWP_HIDE | SWP_MINIMIZE)) {
	      WinSetWindowPos(hwndDir,
			      HWND_TOP, 0, 0, 0, 0, SWP_MINIMIZE | SWP_SHOW);
	      WinSetWindowUShort(hwndDir, QWS_XRESTORE, (USHORT) swp.x);
	      WinSetWindowUShort(hwndDir, QWS_CXRESTORE, (USHORT) swp.cx);
	      WinSetWindowUShort(hwndDir, QWS_YRESTORE, (USHORT) swp.y);
	      WinSetWindowUShort(hwndDir, QWS_CYRESTORE, (USHORT) swp.cy);
	    }
	    else
	      WinSetWindowPos(hwndDir,
			      HWND_TOP,
			      0, 0, 0, 0, SWP_ZORDER | SWP_ACTIVATE);
	  }
	}
      }
    } // for
  }
  return fRestored;
}

static ULONG CountChildren(HWND hwndClient, ULONG * ulNumMinChildren)
{
  HENUM henum;
  HWND hwndChild;
  SWP swp;
  register ULONG ulCnt = 0L;
  USHORT id;

  if (ulNumMinChildren)
    *ulNumMinChildren = 0L;
  henum = WinBeginEnumWindows(hwndClient);
  while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    id = WinQueryWindowUShort(hwndChild, QWS_ID);
    if (!id || (!fFreeTree && id == TREE_FRAME))
      continue;
    ulCnt++;
    if (ulNumMinChildren) {
      if (WinQueryWindowPos(hwndChild, &swp) && (swp.fl & SWP_MINIMIZE))
	(*ulNumMinChildren)++;
    }
  }
  WinEndEnumWindows(henum);
  return ulCnt;
}

VOID GetNextWindowPos(HWND hwndClient, PSWP pswp, ULONG * ulCntR,
		      ULONG * ulNumMinChildrenR)
{
  register ULONG ulCnt;
  ULONG ulNumMinChildren;
  RECTL Rectl;
  register ULONG ulXDiff, ulYDiff, ulWindowsPerStack;

  if (!ulCntR || !ulNumMinChildrenR)
    ulCnt = CountChildren(hwndClient, &ulNumMinChildren);
  else {
    ulCnt = *ulCntR;
    ulNumMinChildren = *ulNumMinChildrenR;
    if (ulCnt == (ULONG) - 1) {
      ulCnt = CountChildren(hwndClient, &ulNumMinChildren);
      /* return these values to the caller for later use */
      *ulCntR = ulCnt;
      *ulNumMinChildrenR = ulNumMinChildren;
    }
  }
  WinQueryWindowRect(hwndClient, &Rectl);
  AdjustSizeOfClient(NULL, &Rectl);
  if (!fFreeTree) {

    SWP swp;

    WinQueryWindowPos(hwndTree, &swp);
    if (ulNumMinChildren || (swp.fl & (SWP_HIDE | SWP_MINIMIZE)))
      Rectl.yBottom += WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 2;
  }
  else if (ulNumMinChildren)
    Rectl.yBottom += WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 2;

  ulXDiff = WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER) +
    WinQuerySysValue(HWND_DESKTOP, SV_CXMINMAXBUTTON) / 2;
  ulYDiff = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER) +
    WinQuerySysValue(HWND_DESKTOP, SV_CYMINMAXBUTTON);
  ulWindowsPerStack = (Rectl.yTop - Rectl.yBottom) / (3 * ulYDiff);
  pswp->cx = Rectl.xRight - (ulWindowsPerStack * ulXDiff);
  pswp->cy = (Rectl.yTop - Rectl.yBottom) - (ulWindowsPerStack * ulYDiff);
  ulWindowsPerStack++;
  pswp->x = Rectl.xLeft + ((ulCnt % ulWindowsPerStack) * ulXDiff);
  pswp->y = (Rectl.yTop - pswp->cy - ((ulCnt % ulWindowsPerStack) * ulYDiff));
}

static VOID CascadeChildren(HWND hwndClient)
{
  ULONG ulCnt = 0L, ulNumMinChildren;
  HWND hwndChild;
  HENUM henum;
  SWP swp;
  USHORT id;
  RECTL Rectl;

  WinQueryWindowPos(hwndClient, &swp);
  if (swp.fl & (SWP_HIDE | SWP_MINIMIZE))
    return;

  CountChildren(hwndClient, &ulNumMinChildren);
  if (!fFreeTree) {
    WinQueryWindowRect(hwndClient, &Rectl);
    AdjustSizeOfClient(NULL, &Rectl);
    WinQueryWindowPos(hwndTree, &swp);
    if (!(swp.fl & (SWP_MAXIMIZE | SWP_HIDE | SWP_MINIMIZE))) {
      if (swp.y + swp.cy < Rectl.yTop - Rectl.yBottom)
	swp.cy = (Rectl.yTop - Rectl.yBottom) - swp.y;
      if (swp.x != 0)
	swp.x = 0;
      if (swp.y < 0)
	swp.y = 0;
      if (swp.x + swp.cx > Rectl.xRight - Rectl.xLeft)
	swp.cx = Rectl.xRight - Rectl.xLeft;
      WinSetWindowPos(hwndTree, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
		      SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_RESTORE);
    }
  }
  henum = WinBeginEnumWindows(hwndClient);
  while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    id = WinQueryWindowUShort(hwndChild, QWS_ID);
    if (!id || (!fFreeTree && id == TREE_FRAME))
      continue;
    WinQueryWindowPos(hwndChild, &swp);
    if (!(swp.fl & (SWP_MINIMIZE | SWP_HIDE))) {
      GetNextWindowPos(hwndClient, &swp, &ulCnt, &ulNumMinChildren);
      WinSetWindowPos(hwndChild, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
		      SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_RESTORE |
		      SWP_ZORDER | SWP_ACTIVATE);
      ulCnt++;
    }
  }
  WinEndEnumWindows(henum);
}

VOID TileChildren(HWND hwndClient, BOOL absolute)
{
  register ULONG ulChildCnt, ulSquare, ulNumRows, ulNumCols, ulExtraCols,
    ulWidth, ulHeight;
  ULONG ulNumMinChildren;
  RECTL Rectl;
  HWND hwndChild;

  if (fNoTileUpdate || hwndClient == HWND_DESKTOP)
    return;
  {
    SWP swp;

    WinQueryWindowPos(hwndClient, &swp);
    if (swp.fl & (SWP_HIDE | SWP_MINIMIZE))
      return;
  }
  ulChildCnt = CountChildren(hwndClient, &ulNumMinChildren);
  ulChildCnt -= ulNumMinChildren;
  if (!ulChildCnt)
    return;

  fNoTileUpdate = TRUE;

  for (ulSquare = 2; ulSquare * ulSquare <= ulChildCnt; ulSquare++) {
    ;
  }
  if (!fTileBackwards) {
    ulNumCols = ulSquare - 1;
    ulNumRows = ulChildCnt / ulNumCols;
  }
  else {
    ulNumRows = ulSquare - 1;
    ulNumCols = ulChildCnt / ulNumRows;
  }
  ulExtraCols = ulChildCnt % ulNumCols;

  WinQueryWindowRect(hwndClient, &Rectl);

  if (!fFreeTree) {

    SWP swp;

    WinQueryWindowPos(hwndTree, &swp);
    if (!(swp.fl & (SWP_MAXIMIZE | SWP_HIDE | SWP_MINIMIZE))) {
      if (swp.y < 0)
	swp.y = 0;
      if (swp.y + swp.cy < Rectl.yTop - Rectl.yBottom)
	swp.cy = (Rectl.yTop - Rectl.yBottom) - swp.y;
      if (swp.x != 0)
	swp.x = 0;
      if (swp.x + swp.cx > Rectl.xRight - Rectl.xLeft)
	swp.cx = Rectl.xRight - Rectl.xLeft;
      WinSetWindowPos(hwndTree,
		      HWND_TOP,
		      swp.x,
		      swp.y,
		      swp.cx,
		      swp.cy, SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_RESTORE);
      WinQueryWindowPos(hwndTree, &swp);
    }
    if (ulNumMinChildren || (swp.fl & (SWP_HIDE | SWP_MINIMIZE)))
      Rectl.yBottom += WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 2;
  }
  else if (ulNumMinChildren)
    Rectl.yBottom += WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 2;

  AdjustSizeOfClient(NULL, &Rectl);

  if (Rectl.xRight > 0L && (Rectl.yBottom < Rectl.yTop)) {

    HENUM henum;

    henum = WinBeginEnumWindows(hwndClient);
    if ((hwndChild = WinGetNextWindow(henum)) != (HWND) 0) {

      ULONG ulCurRow, ulCurCol;
      SWP swp;
      USHORT id;

      ulHeight = (Rectl.yTop - Rectl.yBottom) / ulNumRows;

      for (ulCurRow = 0; ulCurRow < ulNumRows; ulCurRow++) {
	if ((ulNumRows - ulCurRow) <= ulExtraCols)
	  ulNumCols++;
	for (ulCurCol = 0; ulCurCol < ulNumCols; ulCurCol++) {
	  ulWidth = Rectl.xRight / ulNumCols;

	  while (hwndChild) {
	    id = WinQueryWindowUShort(hwndChild, QWS_ID);
	    if (id && (id != TREE_FRAME || fFreeTree)) {
	      WinQueryWindowPos(hwndChild, &swp);
	      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE)))
		break;
	    }
	    hwndChild = WinGetNextWindow(henum);
	  }

	  if (hwndChild) {
	    if (!absolute && (swp.fl & SWP_MAXIMIZE)) {
	      WinGetMaxPosition(hwndChild, &swp);
	      WinSetWindowPos(hwndChild,
			      HWND_TOP,
			      swp.x,
			      swp.y,
			      swp.cx, swp.cy, SWP_MOVE | SWP_SIZE | SWP_SHOW);
	      WinSetWindowUShort(hwndChild,
				 QWS_XRESTORE,
				 (USHORT) (ulWidth * ulCurCol) + Rectl.xLeft);
	      WinSetWindowUShort(hwndChild,
				 QWS_YRESTORE,
				 (USHORT) (Rectl.yTop -
					   (ulHeight * (ulCurRow + 1))));
	      WinSetWindowUShort(hwndChild, QWS_CXRESTORE, (USHORT) ulWidth);
	      WinSetWindowUShort(hwndChild, QWS_CYRESTORE, (USHORT) ulHeight);
	    }
	    else
	      WinSetWindowPos(hwndChild,
			      HWND_TOP,
			      (ulWidth * ulCurCol) + Rectl.xLeft,
			      Rectl.yTop - (ulHeight * (ulCurRow + 1)),
			      ulWidth,
			      ulHeight,
			      SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_RESTORE);
	    hwndChild = WinGetNextWindow(henum);
	  }
	}
	if ((ulNumRows - ulCurRow) <= ulExtraCols) {
	  ulNumCols--;
	  ulExtraCols--;
	}
      }
    }
    WinEndEnumWindows(henum);
  }

  fNoTileUpdate = FALSE;
}

static VOID ResizeChildren(HWND hwndClient, SHORT oldcx, SHORT oldcy,
			   SHORT newcx, SHORT newcy)
{
  /*
   * resize all children of the client to maintain their proportional
   * sizes and positions
   */

  if (!newcx || !newcy || !oldcx || !oldcy)
    return;
  {
    HENUM henum;
    HWND hwndChild;
    register LONG x, y, cx, cy, ucx, ucy, ux, uy;
    SWP swp;

    henum = WinBeginEnumWindows(hwndClient);
    while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
      if (!WinQueryWindowUShort(hwndChild, QWS_ID))
	continue;
      if (WinQueryWindowPos(hwndChild, &swp)) {
	if (swp.fl & (SWP_MINIMIZE | SWP_HIDE)) {
	  swp.x = WinQueryWindowUShort(hwndChild, QWS_XRESTORE);
	  swp.y = WinQueryWindowUShort(hwndChild, QWS_YRESTORE);
	  swp.cx = WinQueryWindowUShort(hwndChild, QWS_CXRESTORE);
	  swp.cy = WinQueryWindowUShort(hwndChild, QWS_CYRESTORE);
	}
	else if (swp.fl & SWP_MAXIMIZE) {
	  swp.x = WinQueryWindowUShort(hwndChild, QWS_XRESTORE);
	  swp.y = WinQueryWindowUShort(hwndChild, QWS_YRESTORE);
	  swp.cx = WinQueryWindowUShort(hwndChild, QWS_CXRESTORE);
	  swp.cy = WinQueryWindowUShort(hwndChild, QWS_CYRESTORE);
	}
	cx = (swp.cx) ? (LONG) (((double)oldcx * 100.0) / (double)swp.cx) : 0;
	cy = (swp.cy) ? (LONG) (((double)oldcy * 100.0) / (double)swp.cy) : 0;
	x = (swp.x) ? (LONG) (((double)oldcx * 100.0) / (double)swp.x) : 0;
	y = (swp.y) ? (LONG) (((double)oldcy * 100.0) / (double)swp.y) : 0;
	if (x < 0)
	  x = 0;
	if (y < 0)
	  y = 0;
	ux = (x) ? (LONG) (((double)newcx * 100.0) / (double)x) : 0;
	uy = (y) ? (LONG) (((double)newcy * 100.0) / (double)y) : 0;
	ucx = (cx) ? (LONG) (((double)newcx * 100.0) / (double)cx) : 0;
	ucy = (cy) ? (LONG) (((double)newcy * 100.0) / (double)cy) : 0;
	if (ux + ucx > newcx)
	  ucx = newcx - ux;
	if (uy + ucy > newcy)
	  ucy = newcy - uy;

	if (!(swp.fl & (SWP_MINIMIZE | SWP_HIDE | SWP_MAXIMIZE)))
	  WinSetWindowPos(hwndChild, HWND_TOP, ux, uy, ucx, ucy,
			  SWP_MOVE | SWP_SIZE | SWP_SHOW);
	else if (swp.fl & (SWP_HIDE | SWP_MINIMIZE)) {
	  WinSetWindowUShort(hwndChild, QWS_XMINIMIZE, (USHORT) - 1);
	  WinSetWindowUShort(hwndChild, QWS_YMINIMIZE, (USHORT) - 1);
	  WinSetWindowPos(hwndChild, HWND_TOP, 0, 0, 0, 0,
			  SWP_SIZE | SWP_MOVE | SWP_FOCUSDEACTIVATE);
	  WinSetWindowUShort(hwndChild, QWS_XRESTORE, ux);
	  WinSetWindowUShort(hwndChild, QWS_YRESTORE, uy);
	  WinSetWindowUShort(hwndChild, QWS_CXRESTORE, ucx);
	  WinSetWindowUShort(hwndChild, QWS_CYRESTORE, ucy);
	}
	else {
	  WinGetMaxPosition(hwndChild, &swp);
	  WinSetWindowPos(hwndChild, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
			  SWP_MOVE | SWP_SIZE | SWP_SHOW);
	  WinSetWindowUShort(hwndChild, QWS_XRESTORE, ux);
	  WinSetWindowUShort(hwndChild, QWS_YRESTORE, uy);
	  WinSetWindowUShort(hwndChild, QWS_CXRESTORE, ucx);
	  WinSetWindowUShort(hwndChild, QWS_CYRESTORE, ucy);
	}
      }
    }
    WinEndEnumWindows(henum);
  }
  if (!fFreeTree) {

    RECTL Rectl;
    SWP swp;

    WinQueryWindowRect(hwndClient, &Rectl);
    AdjustSizeOfClient(NULL, &Rectl);
    WinQueryWindowPos(hwndTree, &swp);
    if (!(swp.fl & (SWP_MAXIMIZE | SWP_HIDE | SWP_MINIMIZE))) {
      if (swp.y + swp.cy < Rectl.yTop - Rectl.yBottom)
	swp.cy = (Rectl.yTop - Rectl.yBottom) - swp.y;
      if (swp.x != 0)
	swp.x = 0;
      if (swp.y < 0)
	swp.y = 0;
      if (swp.x + swp.cx > Rectl.xRight - Rectl.xLeft)
	swp.cx = Rectl.xRight - Rectl.xLeft;
      WinSetWindowPos(hwndTree, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
		      SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_RESTORE);
    }
  }
}

static VOID MinResChildren(HWND hwndClient, ULONG cmd)
{
  HENUM henum;
  HWND hwndChild;

  {
    SWP swp;

    WinQueryWindowPos(hwndClient, &swp);
    if (swp.fl & (SWP_HIDE | SWP_MINIMIZE))
      return;
  }
  henum = WinBeginEnumWindows(hwndClient);
  while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
    if (!WinQueryWindowUShort(hwndChild, QWS_ID))
      continue;
    WinSetWindowPos(hwndChild, HWND_TOP, 0, 0, 0, 0, cmd);
  }
}

//=== ChildFrameButtonProc: subclass handler for WALKBUTTON and QUICKBUTTON windows ===

static MRESULT EXPENTRY ChildFrameButtonProc(HWND hwnd,
					     ULONG msg,
					     MPARAM mp1, MPARAM mp2)
{
  USHORT id;
  static BOOL emphasized = FALSE;

  switch (msg) {
  case WM_BUTTON1CLICK:
  case WM_CHORD:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    break;

  case WM_MOUSEMOVE:
    if (fOtherHelp) {
      if ((!hwndBubble || WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd)
	  && !WinQueryCapture(HWND_DESKTOP)) {
	id = WinQueryWindowUShort(hwnd, QWS_ID);
	switch (id) {
	case IDM_OPENWALK:
	  MakeBubble(hwnd, FALSE, GetPString(IDS_WALKBUTTONHELP));
	  break;
	case IDM_USERLIST:
	  MakeBubble(hwnd, FALSE, GetPString(IDS_QUICKBUTTONHELP));
	  break;
	}
      }
    }
    break;

  case WM_BUTTON3CLICK:
  case WM_BUTTON2CLICK:
    {
      USHORT cmd = 0;

      shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id) {
      case IDM_OPENWALK:
	switch (msg) {
	case WM_BUTTON2CLICK:
	  if ((shiftstate & (KC_ALT | KC_SHIFT | KC_CTRL)) ==
	      (KC_ALT | KC_SHIFT | KC_CTRL))
	    cmd = IDM_GREP;
	  else if ((shiftstate & (KC_ALT | KC_CTRL)) == (KC_ALT | KC_CTRL))
	    CascadeChildren(hwndMain);

#ifdef NEVER
	  else if ((shiftstate & (KC_ALT | KC_SHIFT)) == (KC_ALT | KC_SHIFT))
	    cmd = IDM_SYSINFO;
#endif

	  else if (shiftstate & KC_SHIFT)
	    cmd = IDM_WINDOWDLG;
	  else if (shiftstate & KC_CTRL)
	    cmd = IDM_SEEALL;
	  else if (shiftstate & KC_ALT)
	    TileChildren(hwndMain, TRUE);
	  else
	    cmd = IDM_WALKDIR;
	  break;
	case WM_BUTTON3CLICK:
	  TileChildren(hwndMain, TRUE);
	  break;
	}
	break;
      case IDM_USERLIST:
	switch (msg) {
	case WM_BUTTON2CLICK:
	  if ((shiftstate & (KC_ALT | KC_SHIFT | KC_CTRL)) ==
	      (KC_ALT | KC_SHIFT | KC_CTRL))
	    cmd = IDM_COLORPALETTE;
	  else if ((shiftstate & (KC_ALT | KC_CTRL)) == (KC_ALT | KC_CTRL))
	    cmd = IDM_HIDEMENU;
	  else if ((shiftstate & (KC_ALT | KC_SHIFT)) == (KC_ALT | KC_SHIFT))
	    cmd = IDM_NOTEBOOK;
	  else if (shiftstate & KC_SHIFT)
	    cmd = IDM_TOOLTITLES;
	  else if (shiftstate & KC_CTRL)
	    cmd = IDM_TEXTTOOLS;
	  else if (shiftstate & KC_ALT)
	    cmd = IDM_FONTPALETTE;
	  else
	    cmd = IDM_TOOLBAR;
	  break;
	case WM_BUTTON3CLICK:
	  cmd = IDM_DRIVEBAR;
	  break;
	}
	break;
      }					// switch id

      if (cmd) {
	PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT), FID_CLIENT),
		WM_COMMAND, MPFROM2SHORT(cmd, 0), MPVOID);
      }
    }
    break;

  case DM_DRAGOVER:
    id = WinQueryWindowUShort(hwnd, QWS_ID);
    if (id == IDM_OPENWALK) {
      if (!emphasized) {
	emphasized = TRUE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
      if (AcceptOneDrop(hwnd, mp1, mp2))
	return MRFROM2SHORT(DOR_DROP, DO_MOVE);
    }
    return MRFROM2SHORT(DOR_NEVERDROP, 0);

  case DM_DRAGLEAVE:
    if (emphasized) {
      emphasized = FALSE;
      DrawTargetEmphasis(hwnd, emphasized);
    }
    break;

  case DM_DROPHELP:
    DropHelp(mp1, mp2, hwnd, GetPString(IDS_OPENDROPHELP));
    return 0;

  case DM_DROP:
    {
      char szFrom[CCHMAXPATH + 2];

      if (emphasized) {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
      if (GetOneDrop(hwnd, mp1, mp2, szFrom, sizeof(szFrom))) {
	if (MakeValidDir(szFrom) && !FindDirCnrByName(szFrom, TRUE)) {
	  OpenDirCnr((HWND) 0, hwndMain, hwndTree, FALSE, szFrom);
	}
      }
    }
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    return 0;
  }
  return PFNWPButton(hwnd, msg, mp1, mp2);
}

static MRESULT EXPENTRY MainFrameWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
					 MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);
  static ULONG aheight = 0L;

  switch (msg) {
  case WM_ADJUSTWINDOWPOS:
    {
      SWP *pswp;

      pswp = (SWP *) mp1;
      if (fDataMin && !fAmClosing) {
	if (pswp->fl & (SWP_HIDE | SWP_MINIMIZE)) {

	  SWP swp;

	  WinQueryWindowPos(hwnd, &swp);
	  PostMsg(hwnd, UM_FOCUSME, MPFROMLONG(swp.fl), MPVOID);
	  HideNote();
	}
	else if (pswp->fl & (SWP_SHOW | SWP_RESTORE)) {
	  if (DataHwnd)
	    PostMsg(DataHwnd, WM_CLOSE, MPVOID, MPVOID);
	}
      }
      if (!fAmClosing) {
	if (pswp->fl & (SWP_HIDE | SWP_MINIMIZE))
	  HideNote();
      }
    }
    break;

  case UM_FOCUSME:
    CreateDataBar(HWND_DESKTOP, (ULONG) mp1);
    return 0;

  case WM_BUTTON1UP:
  case WM_BUTTON2UP:
  case WM_BUTTON3UP:
  case WM_MOUSEMOVE:
  case WM_CHORD:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    break;

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_CONTROL:
    return WinSendMsg(WinWindowFromID(hwnd, FID_CLIENT), UM_CONTROL, mp1,
		      mp2);

  case WM_COMMAND:
    return WinSendMsg(WinWindowFromID(hwnd, FID_CLIENT), msg, mp1, mp2);

  case WM_CALCFRAMERECT:
    {
      MRESULT mr;
      PRECTL prectl;
      LONG sheight = 20, bheight = 20;

      mr = oldproc(hwnd, msg, mp1, mp2);

      /*
       * Calculate the position of the client rectangle.
       * Otherwise, we'll see a lot of redraw when we move the
       * client during WM_FORMATFRAME.
       */

      if (mr && mp2) {
	prectl = (PRECTL) mp1;
	if (prectl->yBottom != prectl->yTop) {
	  {
	    HPS hps;
	    POINTL aptl[TXTBOX_COUNT];

	    hps = WinGetPS(hwndStatus);
	    if (hps) {
	      GpiQueryTextBox(hps, 6, "$`WjgT", TXTBOX_COUNT, aptl);
	      bheight = sheight = aptl[TXTBOX_TOPLEFT].y + 6;
	      WinReleasePS(hps);
	    }
	  }
	  prectl->yBottom += (sheight + 4);
	  prectl->yTop -= (sheight + 4);
	  if (fMoreButtons) {

	    HPS hps;
	    POINTL aptl[TXTBOX_COUNT];

	    hps = WinGetPS(hwndName);
	    if (hps) {
	      GpiQueryTextBox(hps, 6, "$`WjgT", TXTBOX_COUNT, aptl);
	      bheight = aptl[TXTBOX_TOPLEFT].y + 6;
	      WinReleasePS(hps);
	    }
	    prectl->yBottom += (bheight + 4);
	    prectl->yTop -= (bheight + 4);
	  }
	  if (fToolbar) {
	    if (!fTextTools)
	      prectl->yTop -= ((fToolTitles) ? 50 : 40);
	    else
	      prectl->yTop -= 32;
	  }
	  if (fDrivebar) {
	    ResizeDrives(WinWindowFromID(hwnd, MAIN_DRIVES),
			 ((prectl->xRight -
			   (WinQuerySysValue(HWND_DESKTOP,
					     SV_CYSIZEBORDER) * 2)) - 4));
	    prectl->yTop -= (16 * (DriveLines * 18));
	  }
	  if (fUserComboBox) {
	    if (!aheight) {

	      SWP swpTemp;

	      WinQueryWindowPos(WinWindowFromID(hwndDrivelist, CBID_EDIT),
				&swpTemp);
	      aheight = swpTemp.cy;
	    }
	    prectl->yTop -= (aheight + 6L);
	  }
	  if (fAutoView) {
	    AutoviewHeight = min(AutoviewHeight,
				 (prectl->yTop - prectl->yBottom) - 116);
	    AutoviewHeight = max(AutoviewHeight, 36);
	    prectl->yBottom += (AutoviewHeight + 6);
	  }
	}
      }
      return mr;
    }

  case WM_FORMATFRAME:
    {
      SHORT sCount, soldCount;
      PSWP pswp, pswpClient, pswpNew;
      SWP swpClient;
      LONG theight = 48L, dheight = 20L, width, sheight = 20, bheight = 20;

      sCount = (SHORT) oldproc(hwnd, msg, mp1, mp2);
      soldCount = sCount;

      /*
       * Reformat the frame to "squeeze" the client
       * and make room for status window sibling beneath
       * and toolbar above (if toolbar's on) and userlists
       * (if userlists are on).
       */

      pswp = (PSWP) mp1;
      {
	SHORT x;

	for (x = 0; x < soldCount; x++) {
	  if (WinQueryWindowUShort(pswp->hwnd, QWS_ID) == FID_CLIENT) {
	    pswpClient = pswp;
	    break;
	  }
	  pswp++;
	}
      }

      {
	HPS hps;
	POINTL aptl[TXTBOX_COUNT];

	hps = WinGetPS(hwndStatus);
	if (hps) {
	  GpiQueryTextBox(hps, 6, "$`WjgT", TXTBOX_COUNT, aptl);
	  bheight = sheight = aptl[TXTBOX_TOPLEFT].y + 6;
	  WinReleasePS(hps);
	}
	if (fMoreButtons) {
	  hps = WinGetPS(hwndName);
	  if (hps) {
	    GpiQueryTextBox(hps, 6, "$`WjgT", TXTBOX_COUNT, aptl);
	    bheight = aptl[TXTBOX_TOPLEFT].y + 6;
	    WinReleasePS(hps);
	  }
	}
      }
      pswpNew = (PSWP) mp1 + soldCount;
      *pswpNew = *pswpClient;
      swpClient = *pswpClient;
      pswpNew->hwnd = hwndStatus;
      pswpNew->hwndInsertBehind = HWND_BOTTOM;
      pswpNew->x = swpClient.x + 3;
      pswpNew->y = swpClient.y + 2;
      if (!fSplitStatus)
	width = swpClient.cx - (16 + (sheight * 2) + 4);
      else
	width = (swpClient.cx - (16 + (sheight * 2) + 4)) / 2;
      width = max(width, 10);
      if (fSplitStatus)
	pswpNew->cx = width - 6;
      else
	pswpNew->cx = width - 8;
      pswpNew->cy = sheight;
      pswpClient->y = pswpNew->y + pswpNew->cy + 3;
      pswpClient->cy = (swpClient.cy - pswpNew->cy) - 3;
      sCount++;

      if (fSplitStatus) {
	pswpNew = (PSWP) mp1 + (soldCount + 1);
	*pswpNew = *pswpClient;
	pswpNew->hwnd = hwndStatus2;
	pswpNew->hwndInsertBehind = HWND_BOTTOM;
	pswpNew->x = width + 8;
	pswpNew->y = swpClient.y + 2;
	pswpNew->cx = width - 6;
	pswpNew->cy = sheight;
	sCount++;
      }
      else {
	WinShowWindow(hwndStatus2, FALSE);
	WinSetWindowText(hwndStatus2, NullStr);
      }

      if (fToolbar) {
	if (fTextTools)
	  theight = 32L;
	else if (!fToolTitles)
	  theight = 40L;
	pswpNew = (PSWP) mp1 + (soldCount + 1 + (fSplitStatus != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = WinWindowFromID(hwnd, MAIN_TOOLS);
	pswpNew->hwndInsertBehind = HWND_BOTTOM;
	pswpNew->x = swpClient.x + 2;
	pswpNew->y = (swpClient.y + swpClient.cy) - (theight - 2);
	pswpNew->cx = swpClient.cx - 4;
	pswpNew->cy = theight - 4;
	pswpClient->cy -= theight;
	sCount++;
      }
      else
	WinShowWindow(WinWindowFromID(hwnd, MAIN_TOOLS), FALSE);

      if (fDrivebar) {
	ResizeDrives(WinWindowFromID(hwnd, MAIN_DRIVES), pswpClient->cx - 4);
	pswpNew = (PSWP) mp1 + (soldCount + 1 +
				(fSplitStatus != FALSE) +
				(fToolbar != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = WinWindowFromID(hwnd, MAIN_DRIVES);
	pswpNew->hwndInsertBehind = HWND_BOTTOM;
	pswpNew->x = swpClient.x + 2;
	dheight += ((dheight - 2) * DriveLines);
	pswpNew->y = (swpClient.y + swpClient.cy) - (dheight - 2);
	if (fToolbar)
	  pswpNew->y -= theight;
	pswpNew->cx = swpClient.cx - 4;
	pswpNew->cy = dheight - 4;
	pswpClient->cy -= dheight;
	sCount++;
      }
      else
	WinShowWindow(WinWindowFromID(hwnd, MAIN_DRIVES), FALSE);

      if (fAutoView) {
	pswpNew = (PSWP) mp1 + (soldCount + 1 +
				(fToolbar != FALSE) +
				(fDrivebar != FALSE) +
				(fSplitStatus != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = (fComments) ? hwndAutoMLE : hwndAutoview;
	pswpNew->x = pswpClient->x + 3;
	pswpNew->y = pswpClient->y + 3;
	if (fMoreButtons)
	  pswpNew->y += (bheight + 4);
	pswpNew->cx = pswpClient->cx - 6;
	AutoviewHeight = min(AutoviewHeight, pswpClient->cy - 116);
	AutoviewHeight = max(AutoviewHeight, 36);
	pswpNew->cy = AutoviewHeight;
	pswpClient->y += (AutoviewHeight + 6);
	pswpClient->cy -= (AutoviewHeight + 6);
	sCount++;
	WinShowWindow((fComments) ? hwndAutoview : hwndAutoMLE, FALSE);
      }
      else {
	WinShowWindow(hwndAutoview, FALSE);
	WinShowWindow(hwndAutoMLE, FALSE);
      }

      pswpNew = (PSWP) mp1 + (soldCount + 1 +
			      (fToolbar != FALSE) +
			      (fDrivebar != FALSE) +
			      (fSplitStatus != FALSE) + (fAutoView != FALSE));
      *pswpNew = *pswpClient;
      pswpNew->hwnd = WinWindowFromID(hwnd, IDM_OPENWALK);
      pswpNew->x = swpClient.cx - ((sheight * 2) + 4);
      pswpNew->y = swpClient.y;
      pswpNew->cx = sheight + 4;
      pswpNew->cy = sheight + 4;
      sCount++;
      pswpNew = (PSWP) mp1 + (soldCount + 2 +
			      (fToolbar != FALSE) +
			      (fDrivebar != FALSE) +
			      (fSplitStatus != FALSE) + (fAutoView != FALSE));
      *pswpNew = *pswpClient;
      pswpNew->hwnd = WinWindowFromID(hwnd, IDM_USERLIST);
      pswpNew->x = swpClient.cx - (sheight + 2);
      pswpNew->y = swpClient.y;
      pswpNew->cx = sheight + 4;
      pswpNew->cy = sheight + 4;
      sCount++;
      pswpNew = (PSWP) mp1 + (soldCount + 3 +
			      (fToolbar != FALSE) +
			      (fDrivebar != FALSE) +
			      (fSplitStatus != FALSE) + (fAutoView != FALSE));
      *pswpNew = *pswpClient;
      pswpNew->hwnd = WinWindowFromID(hwnd, MAIN_LED);
      pswpNew->x = swpClient.cx - ((sheight * 2) + 16);
      pswpNew->y = swpClient.y;
      pswpNew->cx = 12;
      pswpNew->cy = 12;
      sCount++;
      pswpNew = (PSWP) mp1 + (soldCount + 4 +
			      (fToolbar != FALSE) +
			      (fDrivebar != FALSE) +
			      (fSplitStatus != FALSE) + (fAutoView != FALSE));
      *pswpNew = *pswpClient;
      pswpNew->hwnd = WinWindowFromID(hwnd, MAIN_LEDHDR);
      pswpNew->x = swpClient.cx - ((sheight * 2) + 16);
      pswpNew->y = swpClient.y + 12;
      pswpNew->cx = 12;
      pswpNew->cy = sheight - 8;
      sCount++;
      if (fUserComboBox) {
	if (!aheight) {

	  SWP swpTemp;

	  WinQueryWindowPos(WinWindowFromID(hwndDrivelist, CBID_EDIT),
			    &swpTemp);
	  aheight = swpTemp.cy;
	}
	pswpNew = (PSWP) mp1 + (soldCount + 5 +
				(fToolbar != FALSE) +
				(fSplitStatus != FALSE) +
				(fDrivebar != FALSE) + (fAutoView != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = hwndDrivelist;
	pswpNew->x = swpClient.x;
	pswpNew->cx = 48;
	pswpClient->cy -= (aheight + 6L);
	pswpNew->y = pswpClient->y;
	pswpNew->cy = pswpClient->cy + (aheight + 5L);
	sCount++;
	pswpNew = (PSWP) mp1 + (soldCount + 6 +
				(fToolbar != FALSE) +
				(fDrivebar != FALSE) +
				(fSplitStatus != FALSE) +
				(fAutoView != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = hwndStatelist;
	pswpNew->x = swpClient.x + 48;
	pswpNew->cx = (swpClient.cx - 48) / 7;
	pswpNew->y = pswpClient->y;
	pswpNew->cy = pswpClient->cy + (aheight + 5L);
	sCount++;
	pswpNew = (PSWP) mp1 + (soldCount + 7 +
				(fToolbar != FALSE) +
				(fDrivebar != FALSE) +
				(fSplitStatus != FALSE) +
				(fAutoView != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = hwndCmdlist;
	pswpNew->x = swpClient.x + 48 + ((swpClient.cx - 48) / 7);
	pswpNew->cx = (swpClient.cx - 48) / 5 +
	  ((swpClient.cx - 48) / 5) - ((swpClient.cx - 48) / 7);
	pswpNew->y = pswpClient->y;
	pswpNew->cy = pswpClient->cy + (aheight + 5L);
	sCount++;
	pswpNew = (PSWP) mp1 + (soldCount + 8 +
				(fToolbar != FALSE) +
				(fDrivebar != FALSE) +
				(fSplitStatus != FALSE) +
				(fAutoView != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = hwndUserlist;
	pswpNew->x = swpClient.x + 48 + (((swpClient.cx - 48) / 5) * 2);
	pswpNew->cx = ((swpClient.x + swpClient.cx) - pswpNew->x) -
	  ((fToolbar) ? ((swpClient.cx - 48) / 7) : 0);
	pswpNew->y = pswpClient->y;
	pswpNew->cy = pswpClient->cy + (aheight + 5L);
	sCount++;
	if (fToolbar) {
	  pswpNew = (PSWP) mp1 + (soldCount + 9 +
				  (fToolbar != FALSE) +
				  (fDrivebar != FALSE) +
				  (fSplitStatus != FALSE) +
				  (fAutoView != FALSE));
	  *pswpNew = *pswpClient;
	  pswpNew->hwnd = hwndButtonlist;
	  pswpNew->x = swpClient.cx - ((swpClient.cx - 48) / 7) + 4;
	  pswpNew->cx = (swpClient.x + swpClient.cx) - pswpNew->x;
	  pswpNew->y = pswpClient->y;
	  pswpNew->cy = pswpClient->cy + (aheight + 5L);
	  sCount++;
	}
	else
	  WinShowWindow(hwndButtonlist, FALSE);
      }
      else {
	WinShowWindow(hwndUserlist, FALSE);
	WinShowWindow(hwndDrivelist, FALSE);
	WinShowWindow(hwndStatelist, FALSE);
	WinShowWindow(hwndButtonlist, FALSE);
	WinShowWindow(hwndCmdlist, FALSE);
      }
      {
	PSWP pswpTitlebar = (PSWP) 0, pswpMinbutton = (PSWP) 0;
	SHORT x;

	pswpNew = (PSWP) mp1 + (soldCount + 5 +
				(fToolbar != FALSE) +
				(fDrivebar != FALSE) +
				(fSplitStatus != FALSE) +
				(fAutoView != FALSE) +
				((fUserComboBox != FALSE) * 4) +
				(fUserComboBox != FALSE &&
				 fToolbar != FALSE));
	pswp = (PSWP) mp1;
	for (x = 0; x < soldCount; x++) {
	  if (!pswpTitlebar &&
	      WinQueryWindowUShort(pswp->hwnd, QWS_ID) == FID_TITLEBAR)
	    pswpTitlebar = pswp;
	  else if (!pswpMinbutton &&
		   WinQueryWindowUShort(pswp->hwnd, QWS_ID) == FID_MINMAX)
	    pswpMinbutton = pswp;
	  if (pswpTitlebar && pswpMinbutton)
	    break;
	  pswp++;
	}
	if (pswpMinbutton && pswpTitlebar) {
	  *pswpNew = *pswpMinbutton;
	  pswpNew->hwnd = WinWindowFromID(hwnd, IDM_IDEALSIZE);
	  pswpNew->cy = pswpMinbutton->cy + 3;
	  pswpNew->cx = min(pswpNew->cy, (pswpMinbutton->cx / 2) + 3);
	  pswpTitlebar->cx -= (pswpNew->cx - 1);
	  pswpNew->x = pswpTitlebar->x + (pswpTitlebar->cx);
	  pswpNew->y = pswpMinbutton->y - 1;
	  sCount++;
	}
	else
	  WinShowWindow(WinWindowFromID(hwnd, IDM_IDEALSIZE), FALSE);
      }

      if (fMoreButtons) {

	LONG lastx;

	pswpNew = (PSWP) mp1 + (soldCount + 6 +
				(fToolbar != FALSE) +
				(fDrivebar != FALSE) +
				(fSplitStatus != FALSE) +
				(fAutoView != FALSE) +
				((fUserComboBox != FALSE) * 4) +
				(fUserComboBox != FALSE &&
				 fToolbar != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = hwndName;
	pswpNew->x = swpClient.x + 3;
	pswpNew->y = swpClient.y + (sheight + 6);
	pswpNew->cx = ((swpClient.cx / 2) + (swpClient.cx / 5)) - 3;
	lastx = pswpNew->x + pswpNew->cx;
	pswpNew->cy = bheight;
	pswpClient->y += (bheight + 4);
	pswpClient->cy -= (bheight + 4);
	sCount++;
	pswpNew = (PSWP) mp1 + (soldCount + 7 +
				(fToolbar != FALSE) +
				(fDrivebar != FALSE) +
				(fSplitStatus != FALSE) +
				(fAutoView != FALSE) +
				((fUserComboBox != FALSE) * 4) +
				(fUserComboBox != FALSE &&
				 fToolbar != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = hwndDate;
	pswpNew->x = lastx + 3;
	pswpNew->y = swpClient.y + (sheight + 6);
	pswpNew->cx = (swpClient.cx / 6) + (swpClient.cx / 16) - 3;
	lastx = pswpNew->x + pswpNew->cx;
	pswpNew->cy = bheight;
	sCount++;
	pswpNew = (PSWP) mp1 + (soldCount + 8 +
				(fToolbar != FALSE) +
				(fDrivebar != FALSE) +
				(fSplitStatus != FALSE) +
				(fAutoView != FALSE) +
				((fUserComboBox != FALSE) * 4) +
				(fUserComboBox != FALSE &&
				 fToolbar != FALSE));
	*pswpNew = *pswpClient;
	pswpNew->hwnd = hwndAttr;
	pswpNew->x = lastx + 3;
	pswpNew->y = swpClient.y + (sheight + 6);
	pswpNew->cx = (swpClient.cx - pswpNew->x) - 1;
	pswpNew->cy = bheight;
	sCount++;
      }
      else {
	WinShowWindow(hwndAttr, FALSE);
	WinShowWindow(hwndName, FALSE);
	WinShowWindow(hwndDate, FALSE);
      }
      return MRFROMSHORT(sCount);
    }

  case WM_QUERYFRAMECTLCOUNT:
    {
      SHORT sCount;

      sCount = (SHORT) oldproc(hwnd, msg, mp1, mp2);

      sCount += 6;
      if (fSplitStatus)
	sCount++;
      if (fToolbar)
	sCount++;
      if (fUserComboBox) {
	sCount += 4;
	if (fToolbar)
	  sCount++;
      }
      if (fDrivebar)
	sCount++;
      if (fAutoView)
	sCount++;
      if (fMoreButtons)
	sCount += 3;
      return MRFROMSHORT(sCount);
    }

  case WM_CLOSE:
    WinSendMsg(WinWindowFromID(hwnd, FID_CLIENT), msg, mp1, mp2);
    return 0;
  }
  return oldproc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY MainWMCommand(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SetShiftState();
  switch (SHORT1FROMMP(mp1)) {

  case IDM_CONTEXTMENU:
    {
      HWND hwnd = WinQueryFocus(HWND_DESKTOP);
      // DbgMsg(pszSrcFile, __LINE__, "IDM_CONTEXTMENU %x", hwnd);
      if (hwnd != NULLHANDLE) {
	HWND hwndParent = WinQueryWindow(hwnd, QW_PARENT);
	USHORT id = WinQueryWindowUShort(hwndParent, QWS_ID);
	switch (id) {
	case MAIN_SETUPLIST:
	case MAIN_USERLIST:
	case MAIN_CMDLIST:
	  // DbgMsg(pszSrcFile, __LINE__, "WM_CONTEXTMENU");
	  WinPostMsg(hwnd, WM_CONTEXTMENU, 0, 0);
	}
      }
    }
    break;

  case IDM_SETTARGET:
    SetTargetDir(hwnd, FALSE);
    break;

  case IDM_TOAUTOMLE:
    if (fComments && fAutoView)
      WinSetFocus(HWND_DESKTOP, hwndAutoMLE);
    break;

  case IDM_HIDENOTEWND:
    HideNote();
    break;
  case IDM_SHOWNOTEWND:
    ShowNote();
    break;

  case IDM_COMPARE:
    {
      WALK2 wa;
      PCNRITEM pci;

      memset(&wa, 0, sizeof(wa));
      wa.size = sizeof(wa);
      pci =
	(PCNRITEM)
	WinSendMsg(WinWindowFromID
		   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
		   CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		   MPFROMSHORT(CRA_CURSORED));
      if (pci && (INT) pci != -1) {
	strcpy(wa.szCurrentPath1, pci->pszFileName);
	MakeValidDir(wa.szCurrentPath1);
      }
      else
	save_dir2(wa.szCurrentPath1);
      TopWindowName(hwndMain, (HWND) 0, wa.szCurrentPath2);
      if (!*wa.szCurrentPath2)
	strcpy(wa.szCurrentPath2, wa.szCurrentPath1);
      MakeValidDir(wa.szCurrentPath2);
      if (WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    WalkTwoCmpDlgProc,
		    FM3ModHandle,
		    WALK2_FRAME,
		    MPFROMP(&wa)) &&
	  !IsFile(wa.szCurrentPath1) && !IsFile(wa.szCurrentPath2)) {
	if (!*dircompare) {

	  COMPARE *cmp;

	  cmp = xmallocz(sizeof(COMPARE), pszSrcFile, __LINE__);
	  if (cmp) {
	    cmp->size = sizeof(COMPARE);
	    strcpy(cmp->leftdir, wa.szCurrentPath1);
	    strcpy(cmp->rightdir, wa.szCurrentPath2);
	    cmp->hwndParent = hwnd;
	    cmp->dcd.hwndParent = hwnd;
	    WinDlgBox(HWND_DESKTOP,
		      HWND_DESKTOP,
		      CompareDlgProc, FM3ModHandle, COMP_FRAME, MPFROMP(cmp));
	  }
	}
	else {

	  CHAR szPath1[CCHMAXPATH];
	  CHAR szPath2[CCHMAXPATH];
	  runemf2(SEPARATE,
		  HWND_DESKTOP, NULL, NULL,
		  "%s %s %s",
		  dircompare,
		  BldQuotedFileName(szPath1, wa.szCurrentPath1),
		  BldQuotedFileName(szPath2, wa.szCurrentPath2));
	  // CHAR d1[] = "\"";
	  // CHAR d2[] = "\"";
	  // if (!needs_quoting(wa.szCurrentPath1))
	  //   *d1 = 0;
	  // if (!needs_quoting(wa.szCurrentPath2))
	  //   *d2 = 0;
	  // runemf2(SEPARATE,
	  //	  HWND_DESKTOP, NULL, NULL,
	  //	  "%s %s%s%s %s%s%s",
	  //	  dircompare,
	  //	  d1, wa.szCurrentPath1, d1, d2, wa.szCurrentPath2, d2);
	}
      }
    }
    break;

  case IDM_EXIT:
  case IDM_KILLME:
    PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    break;

  case IDM_CLI:
    if (fSplitStatus &&
	hwndStatus2 &&
	!WinIsWindow(WinQueryAnchorBlock(hwnd),
		     WinWindowFromID(hwndStatus2, COMMAND_LINE)))
      PostMsg(hwndStatus2, UM_CLICKED, MPVOID, MPVOID);
    break;

  case IDM_ADDTOUSERLIST:
  case IDM_DELETEFROMUSERLIST:
    {
      CHAR temp[CCHMAXPATH], path[CCHMAXPATH];

      *temp = 0;
      WinQueryWindowText(hwndUserlist, CCHMAXPATH, temp);
      bstrip(temp);
      if (*temp &&
	  !DosQueryPathInfo(temp, FIL_QUERYFULLNAME, path, sizeof(path))) {
	if (SHORT1FROMMP(mp1) == IDM_ADDTOUSERLIST) {
	  add_udir(TRUE, path);
	  if (fUdirsChanged)
	    save_udirs();
	  WinSendMsg(hwnd, UM_FILLUSERLIST, MPVOID, MPVOID);
	}
	else {
	  if (!remove_udir(path))
	    Runtime_Error(pszSrcFile, __LINE__, "remove_udir");
	  else {
	    if (fUdirsChanged)
	      save_udirs();
	    WinSendMsg(hwnd, UM_FILLUSERLIST, MPVOID, MPVOID);
	  }
	}
      }
    }
    break;

  case IDM_SAVEDIRCNRSTATE:
  case IDM_DELETEDIRCNRSTATE:
    {
      CHAR szStateName[STATE_NAME_MAX_BYTES + 1];

      *szStateName = 0;
      WinQueryWindowText(hwndStatelist, STATE_NAME_MAX_BYTES, szStateName);
      bstrip(szStateName);
      // Ignore request if blank or attempting to using illegal name
      if (*szStateName && stricmp(szStateName, GetPString(IDS_STATETEXT))) {
	if (SHORT1FROMMP(mp1) == IDM_SAVEDIRCNRSTATE) {
	  // Save
	  INT nSaved = SaveDirCnrState(hwnd, szStateName);
	  if (nSaved > 0) {
	    INT ret = add_setup(szStateName);
	    if (ret == 0) {
	      WinSendMsg(hwndStatelist, LM_INSERTITEM,
			 MPFROM2SHORT(LIT_SORTASCENDING, 0), MPFROMP(szStateName));
	      save_setups();
	    }
	    else if (ret != 1) {
	      saymsg(MB_ENTER | MB_ICONASTERISK, hwnd,
		     GetPString(IDS_WARNINGTEXT),
		     "\"%s\" state name add failed", szStateName);	// 15 Apr 07 SHL failed
	      WinSetWindowText(hwndStatelist, GetPString(IDS_STATETEXT));
	    }
	  }
	  else {
	    saymsg(MB_ENTER | MB_ICONASTERISK,
		   hwnd,
		   GetPString(IDS_WARNINGTEXT),
		   nSaved == 0 ?
		     "Nothing to save" :
		     "State data save failed");
	    WinSetWindowText(hwndStatelist, GetPString(IDS_STATETEXT));
	  }
	}
	else {
	  // Delete
	  ULONG numsaves = 0, size, x;
	  CHAR s[STATE_NAME_MAX_BYTES + 80];

	  INT ret = remove_setup(szStateName);
	  if (ret == 1)
	    save_setups();
	  sprintf(s, "%s.NumDirsLastTime", szStateName);
	  size = sizeof(ULONG);
	  if (!PrfQueryProfileData(fmprof,
				   FM3Str,
				   s,
				   (PVOID)&numsaves,
				   &size)) {
	    saymsg(MB_ENTER | MB_ICONASTERISK, hwnd,
		   GetPString(IDS_WARNINGTEXT),
		   GetPString(IDS_DOESNTEXISTTEXT), szStateName);
	  }
	  else if (!size)
	    Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
	  else {
	    PrfWriteProfileData(fmprof, FM3Str, s, NULL, 0L);
	    for (x = 0; x < numsaves; x++) {
	      sprintf(s, "%s.DirCnrPos.%lu", szStateName, x);
	      PrfWriteProfileData(fmprof, FM3Str, s, NULL, 0);
	      sprintf(s, "%s.DirCnrDir.%lu", szStateName, x);
	      PrfWriteProfileData(fmprof, FM3Str, s, NULL, 0);
	      sprintf(s, "%s.DirCnrSort.%lu", szStateName, x);
	      PrfWriteProfileData(fmprof, FM3Str, s, NULL, 0);
	      sprintf(s, "%s.DirCnrFilter.%lu", szStateName, x);
	      PrfWriteProfileData(fmprof, FM3Str, s, NULL, 0);
	      sprintf(s, "%s.DirCnrView.%lu", szStateName, x);
	      PrfWriteProfileData(fmprof, FM3Str, s, NULL, 0);
	    }
	    sprintf(s, "%s.LastTreePos", szStateName);
	    PrfWriteProfileData(fmprof, FM3Str, s, NULL, 0);
	    sprintf(s, "%s.MySizeLastTime", szStateName);
	    PrfWriteProfileData(fmprof, FM3Str, s, NULL, 0);
	  }
	  PostMsg(hwnd, UM_FILLSETUPLIST, MPVOID, MPVOID);
	}
      }
    }
    break;

  case IDM_IDEALSIZE:
    {
      SWP swp, swpD;
      ULONG icz = WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 2L;
      ULONG bsz = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);

      WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &swp);
      if (swp.fl & SWP_MAXIMIZE) {
	WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_SYSCOMMAND,
		   MPFROM2SHORT(SC_RESTORE, 0), MPVOID);
	WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &swp);
      }
      WinGetMaxPosition(WinQueryWindow(hwnd, QW_PARENT), &swpD);
      swpD.x += bsz;
      swpD.cx -= (bsz * 2);
      swpD.y += icz;
      swpD.cy -= (icz + bsz);
      if (swp.x == swpD.x && swp.y == swpD.y &&
	  swp.cx == swpD.cx && swp.cy == swpD.cy &&
	  // fixme to be #defined someday
	  WinQueryWindowUShort(hwnd, QWL_USER + 10) &&
	  WinQueryWindowUShort(hwnd, QWL_USER + 14)) {
	swpD.x = WinQueryWindowUShort(hwnd, QWL_USER + 8);
	swpD.cx = WinQueryWindowUShort(hwnd, QWL_USER + 10);
	swpD.y = WinQueryWindowUShort(hwnd, QWL_USER + 12);
	swpD.cy = WinQueryWindowUShort(hwnd, QWL_USER + 14);
      }
      else {
	WinSetWindowUShort(hwnd, QWL_USER + 8, (USHORT) swp.x);
	WinSetWindowUShort(hwnd, QWL_USER + 10, (USHORT) swp.cx);
	WinSetWindowUShort(hwnd, QWL_USER + 12, (USHORT) swp.y);
	WinSetWindowUShort(hwnd, QWL_USER + 14, (USHORT) swp.cy);
      }
      WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), HWND_TOP,
		      swpD.x, swpD.y, swpD.cx, swpD.cy, SWP_MOVE | SWP_SIZE);
    }
    break;

  case IDM_BLINK:
    WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), HWND_TOP, 0, 0, 0, 0,
		    SWP_MINIMIZE);
    WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), HWND_TOP, 0, 0, 0, 0,
		    SWP_RESTORE | SWP_ZORDER);
    break;

  case DID_CANCEL:
    {
      HWND hwndTop = TopWindow(hwndMain, (HWND) 0);

      if (hwndTop)
	WinSetFocus(HWND_DESKTOP, hwndTop);
    }
    break;

  case IDM_NOTEBOOK:
    WinDlgBox(HWND_DESKTOP,
	      hwnd, CfgDlgProc, FM3ModHandle, CFG_FRAME, MPVOID);
    break;

  case IDM_VIEWHELPS:
  case IDM_VIEWINFS:
    WinDlgBox(HWND_DESKTOP,
	      HWND_DESKTOP,
	      ViewInfProc,
	      FM3ModHandle,
	      VINF_FRAME,
	      ((SHORT1FROMMP(mp1) == IDM_VIEWHELPS) ?
	       MPFROMP(NullStr) : MPVOID));
    break;

  case IDM_OPENWALK:
    {
      char newpath[CCHMAXPATH];

      *newpath = 0;
      TopWindowName(hwnd, (HWND) 0, newpath);
      if (WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    WalkAllDlgProc,
		    FM3ModHandle, WALK_FRAME, MPFROMP(newpath)) && *newpath)
	OpenDirCnr((HWND) 0, hwndMain, hwndTree, FALSE, newpath);
    }
    break;

  case IDM_WINDOWDLG:
    WindowList(hwnd);
    break;

  case IDM_HELPMOUSE:
  case IDM_HELPCONTEXT:
  case IDM_HELPHINTS:
  case IDM_HELPPIX:
  case IDM_HELPTUTOR:
  case IDM_HELPUSERLIST:
  case IDM_HELP:
  case IDM_HELPCONTENTS:
  case IDM_HELPKEYS:
  case IDM_HELPGENERAL:
    if (hwndHelp) {
      if (SHORT1FROMMP(mp2) == CMDSRC_MENU) {

	RECTL rcl;
	ULONG icz = WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 2L;

	WinQueryWindowRect(HWND_DESKTOP, &rcl);
	rcl.yBottom += icz;
	rcl.yTop -= icz;
	rcl.xLeft += icz;
	rcl.xRight -= icz;
	WinSendMsg(hwndHelp, HM_SET_COVERPAGE_SIZE, MPFROMP(&rcl), MPVOID);
      }
      else {

	RECTL rcl;

	WinQueryWindowRect(HWND_DESKTOP, &rcl);
	rcl.yBottom += 8;
	rcl.yTop = (rcl.yTop / 2) + (rcl.yTop / 7);
	rcl.xLeft = (rcl.xRight / 2) - (rcl.xRight / 7);
	rcl.xRight -= 8;
	WinSendMsg(hwndHelp, HM_SET_COVERPAGE_SIZE, MPFROMP(&rcl), MPVOID);
      }
      switch (SHORT1FROMMP(mp1)) {
      case IDM_HELPCONTEXT:
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_CONTEXT, 0), MPFROMSHORT(HM_RESOURCEID));
	break;

      case IDM_HELPMOUSE:
	if (hwndHelp)
	  WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_MOUSE, 0), MPFROMSHORT(HM_RESOURCEID));
	break;

      case IDM_HELPPIX:
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_BITMAP1, 0), MPFROMSHORT(HM_RESOURCEID));
	break;

      case IDM_HELPTUTOR:
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_TUTORIAL, 0),
		   MPFROMSHORT(HM_RESOURCEID));
	break;

      case IDM_HELPHINTS:
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_HINTS, 0), MPFROMSHORT(HM_RESOURCEID));
	break;

      case IDM_HELPGENERAL:
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_MAIN, 0), MPFROMSHORT(HM_RESOURCEID));
	break;
      case IDM_HELPKEYS:
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_KEYS, 0), MPFROMSHORT(HM_RESOURCEID));
	break;

      case IDM_HELP:
      case IDM_HELPCONTENTS:
	WinSendMsg(hwndHelp, HM_HELP_CONTENTS, MPVOID, MPVOID);
	break;

      case IDM_HELPUSERLIST:
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_USERLISTS, 0),
		   MPFROMSHORT(HM_RESOURCEID));
	break;
      }
    }
    break;

  case IDM_EDITANYARCHIVER:
    EditArchiverDefinition(hwnd);
    break;

  case IDM_ABOUT:
    WinDlgBox(HWND_DESKTOP, hwnd, AboutDlgProc, FM3ModHandle,
	      ABT_FRAME, MPVOID);
    break;

  case IDM_FONTPALETTE:
    OpenObject("<WP_FNTPAL>", Default, hwnd);
    break;

  case IDM_HICOLORPALETTE:
  case IDM_COLORPALETTE:
    {
      CHAR *palette = "<WP_CLRPAL>";
      ULONG version[2];

      if (!DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_MINOR,
			   (PVOID) & version, (ULONG) sizeof(version))) {
	if (version[0] > 20L || (version[0] == 20L && version[1] > 29L)) {
	  if (SHORT1FROMMP(mp1) == IDM_HICOLORPALETTE)
	    palette = "<WP_HIRESCLRPAL>";
	  else
	    palette = "<WP_LORESCLRPAL>";
	}
      }
      OpenObject(palette, Default, hwnd);
    }
    break;

  case IDM_SYSTEMSETUP:
    OpenObject("<WP_CONFIG>", Default, hwnd);
    break;

  case IDM_SCHEMEPALETTE:
    {
      HOBJECT hWPSObject;

      hWPSObject = WinQueryObject("<WP_SCHPAL>");
      if (hWPSObject != NULLHANDLE)
	WinSetObjectData(hWPSObject, "SCHEMES=Winter:PM_Winter,"
			 "Spring:PM_Spring,Summer:PM_Summer,"
			 "System:PM_System,Windows:PM_Windows;"
			 "OPEN=DEFAULT");
    }
    break;

  case IDM_SYSTEMCLOCK:
    OpenObject("<WP_CLOCK>", Default, hwnd);
    break;

#ifdef NEVER
  case IDM_SYSINFO:
    WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, SysInfoDlgProc, FM3ModHandle,
	      SYS_FRAME, NULL);
    break;
#endif

  case IDM_INSTANT:
    {
      CHAR path[CCHMAXPATH];
      PCNRITEM pci = (PCNRITEM) 0;

      if (hwndTree)
	pci = (PCNRITEM) WinSendMsg(hwndTree, CM_QUERYRECORDEMPHASIS,
				    MPFROMLONG(CMA_FIRST),
				    MPFROMSHORT(CRA_CURSORED));
      if (pci && (INT) pci != -1) {
	strcpy(path, pci->pszFileName);
	MakeValidDir(path);
      }
      else
	save_dir2(path);
      WinDlgBox(HWND_DESKTOP, hwnd, InstantDlgProc, FM3ModHandle,
		BAT_FRAME, MPFROMP(path));
    }
    break;

  case IDM_WINFULLSCREEN:
  case IDM_DOSCOMMANDLINE:
  case IDM_COMMANDLINE:
    {
      CHAR *env = GetCmdSpec(FALSE), path[CCHMAXPATH];
      INT type = SEPARATE | WINDOWED;

      *path = 0;
      TopWindowName(hwnd, (HWND) 0, path);
      if (SHORT1FROMMP(mp1) == IDM_DOSCOMMANDLINE)
	env = GetCmdSpec(TRUE);
      else if (SHORT1FROMMP(mp1) != IDM_COMMANDLINE) {
	env = "WINOS2.COM";
	type = SEPARATE | FULLSCREEN;
      }
      runemf2(type, hwnd, path, NULL, "%s", env);
    }
    break;

  case IDM_KILLPROC:
    WinDlgBox(HWND_DESKTOP, hwnd, KillDlgProc, FM3ModHandle,
	      KILL_FRAME, NULL);
    break;

  case IDM_AUTOVIEWCOMMENTS:
  case IDM_AUTOVIEWFILE:
    if (SHORT1FROMMP(mp1) == IDM_AUTOVIEWFILE)
      fComments = FALSE;
    else
      fComments = TRUE;
    PrfWriteProfileData(fmprof, FM3Str, "Comments", &fComments, sizeof(BOOL));
    WinSetWindowText((fComments) ? hwndAutoview : hwndAutoMLE, NullStr);
    goto AutoChange;

  case IDM_AUTOVIEW:
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER), SHORT1FROMMP(mp1),
		 &fAutoView, TRUE, "AutoView");
  AutoChange:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_UPDATEFRAME,
	    MPFROMLONG(FCF_SIZEBORDER), MPVOID);
    if (fAutoView) {

      CHAR s[CCHMAXPATH];
      HWND hwndDir;
      PCNRITEM pci;

      hwndDir = TopWindowName(hwnd, (HWND) 0, s);
      if (hwndDir) {
	hwndDir = WinWindowFromID(hwndDir, FID_CLIENT);
	if (hwndDir) {
	  hwndDir = WinWindowFromID(hwndDir, DIR_CNR);
	  if (hwndDir) {
	    pci = (PCNRITEM) WinSendMsg(hwndDir, CM_QUERYRECORDEMPHASIS,
					MPFROMLONG(CMA_FIRST),
					MPFROMSHORT(CRA_CURSORED));
	    if (pci && (INT) pci != -1 &&
		(!(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_SLOW)))
	      WinSendMsg(hwnd,
			 UM_LOADFILE,
			 MPFROMP(pci->pszFileName),
			 (SHORT1FROMMP(mp1) == IDM_AUTOVIEW) ?
			 MPVOID : MPFROMLONG(1));
	  }
	}
      }
    }
    break;

  case IDM_TEXTTOOLS:
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER), SHORT1FROMMP(mp1),
		 &fTextTools, TRUE, "TextTools");
    BuildTools(hwndToolback, TRUE);
    PostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_UPDATEFRAME,
	    MPFROMLONG(FCF_SIZEBORDER), MPVOID);
    break;

  case IDM_TOOLTITLES:
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER), SHORT1FROMMP(mp1),
		 &fToolTitles, TRUE, "ToolTitles");
    BuildTools(hwndToolback, TRUE);
    PostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_UPDATEFRAME,
	    MPFROMLONG(FCF_SIZEBORDER), MPVOID);
    break;

  case IDM_HIDEMENU:
    {
      HWND hwndMenu;

      hwndMenu = WinQueryWindowULong(hwnd, QWL_USER);
      MenuInvisible = (MenuInvisible) ? FALSE : TRUE;
      if (MenuInvisible) {
	WinSetParent(hwndMenu, HWND_OBJECT, FALSE);
	WinSetMenuItemText(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					   FID_SYSMENU), IDM_HIDEMENU,
			   GetPString(IDS_UNHIDEMENUTEXT));
      }
      else {
	WinSetParent(hwndMenu, WinQueryWindow(hwnd, QW_PARENT), FALSE);
	WinSetMenuItemText(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					   FID_SYSMENU), IDM_HIDEMENU,
			   GetPString(IDS_HIDEMENUTEXT));
      }
      PostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_UPDATEFRAME,
	      MPFROMLONG(FCF_MENU), MPVOID);
      PrfWriteProfileData(fmprof, FM3Str, "MenuInvisible",
			  &MenuInvisible, sizeof(BOOL));
    }
    break;

  case IDM_SEEALL:
  case IDM_GREP:
  case IDM_COLLECTOR:
    {
      HWND hwndC;
      SWP swp;
      BOOL already = FALSE;

      if (Collector)
	already = TRUE;
      if (!already && !fAutoTile && !fExternalCollector)
	GetNextWindowPos(hwnd, &swp, NULL, NULL);
      hwndC = StartCollector(fExternalCollector ? HWND_DESKTOP : hwnd, 4);
      if (hwndC) {
	if (!already && !fAutoTile && !fExternalCollector)
	  WinSetWindowPos(hwndC, HWND_TOP,
			  swp.x, swp.y, swp.cx, swp.cy,
			  SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER);
	else if (fAutoTile && !already)
	  TileChildren(hwnd, TRUE);
	WinSetWindowPos(hwndC, HWND_TOP, 0, 0, 0, 0, SWP_SHOW | SWP_RESTORE |
			SWP_ACTIVATE);
	if (SHORT1FROMMP(mp1) == IDM_GREP)
	  PostMsg(WinWindowFromID(hwndC, FID_CLIENT), WM_COMMAND,
		  MPFROM2SHORT(IDM_GREP, 0), MPVOID);
	if (SHORT1FROMMP(mp1) == IDM_SEEALL)
	  PostMsg(WinWindowFromID(hwndC, FID_CLIENT), WM_COMMAND,
		  MPFROM2SHORT(IDM_SEEALL, 0), MPVOID);
      }
    }
    break;

  case IDM_TOOLLEFT:
  case IDM_TOOLRIGHT:
    {
      TOOL *tool;

      if (!toolhead || !toolhead->next) {
	firsttool = (toolhead) ? toolhead->id : 0;
	break;
      }
      tool = find_tool(firsttool);
      if (!tool)
	tool = toolhead;
      if (SHORT1FROMMP(mp1) == IDM_TOOLRIGHT) {
	tool = prev_tool(tool, TRUE);
	firsttool = tool->id;
      }
      else {
	tool = next_tool(tool, TRUE);
	firsttool = tool->id;
      }
      ResizeTools(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				  MAIN_TOOLS));
    }
    break;

  case IDM_CREATETOOL:
    BuildTools(hwndToolback, TRUE);
    break;

  case IDM_TOOLBAR:
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER),
		 IDM_TOOLSUBMENU, &fToolbar, TRUE, "Toolbar");
    BuildTools(hwndToolback, TRUE);
    WinShowWindow(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				  MAIN_TOOLS), fToolbar);
    WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
	       WM_UPDATEFRAME, MPFROMLONG(FCF_SIZEBORDER), MPVOID);
    if (fDrivebar)
      WinInvalidateRect(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					MAIN_DRIVES), NULL, TRUE);
    break;

  case IDM_DRIVEBAR:
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER),
		 IDM_DRIVEBAR, &fDrivebar, TRUE, "Drivebar");
    WinShowWindow(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				  MAIN_DRIVES), fDrivebar);
    PostMsg(WinQueryWindow(hwnd, QW_PARENT),
	    WM_UPDATEFRAME, MPFROMLONG(FCF_SIZEBORDER), MPVOID);
    PostMsg(hwnd, UM_BUILDDRIVEBAR, MPVOID, MPVOID);
    break;

  case IDM_USERLIST:
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER),
		 SHORT1FROMMP(mp1), &fUserComboBox, TRUE, "UserComboBox");
    WinShowWindow(hwndUserlist, fUserComboBox);
    PostMsg(WinQueryWindow(hwnd, QW_PARENT),
	    WM_UPDATEFRAME, MPFROMLONG(FCF_SIZEBORDER), MPVOID);
    PostMsg(hwnd, UM_FILLUSERLIST, MPVOID, MPVOID);
    PostMsg(hwnd, UM_FILLSETUPLIST, MPVOID, MPVOID);
    PostMsg(hwnd, UM_FILLCMDLIST, MPVOID, MPVOID);
    PostMsg(hwnd, UM_FILLBUTTONLIST, MPVOID, MPVOID);
    break;

  case IDM_MOREBUTTONS:
    WinSetWindowText(hwndName, NullStr);
    WinSetWindowText(hwndDate, NullStr);
    WinSetWindowText(hwndAttr, NullStr);
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER),
		 SHORT1FROMMP(mp1), &fMoreButtons, TRUE, "MoreButtons");
    if (fMoreButtons) {

      HWND hwndTemp;

      hwndTemp = TopWindow(hwnd, (HWND) 0);

      if (hwndTemp) {
	WinSetFocus(HWND_DESKTOP, hwnd);
	WinSetFocus(HWND_DESKTOP, hwndTemp);
      }
    }
    PostMsg(WinQueryWindow(hwnd, QW_PARENT),
	    WM_UPDATEFRAME, MPFROMLONG(FCF_SIZEBORDER), MPVOID);
    break;

  case IDM_FREETREE:
    if (fFreeTree) {

      SWP swp, swpT;

      WinQueryWindowPos(hwndTree, &swpT);
      WinQueryWindowPos(hwnd, &swp);
      WinSetWindowPos(hwndTree, HWND_TOP, 0, swp.cy - swpT.cy, 0, 0,
		      SWP_MOVE);
    }
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER), SHORT1FROMMP(mp1),
		 &fFreeTree, TRUE, "FreeTree");
    if (fAutoTile)
      TileChildren(hwnd, TRUE);
    break;

  case IDM_AUTOTILE:
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER),
		 SHORT1FROMMP(mp1), &fAutoTile, TRUE, "AutoTile");
    if (fAutoTile)
      TileChildren(hwnd, TRUE);
    break;

  case IDM_TILEBACKWARDS:
    SetMenuCheck(WinQueryWindowULong(hwnd, QWL_USER),
		 SHORT1FROMMP(mp1), &fTileBackwards, TRUE, "TileBackwards");
    if (fAutoTile)
      TileChildren(hwnd, TRUE);
    break;

  case IDM_NEXTWINDOW:
  case IDM_PREVWINDOW:
    NextChild(hwnd, (SHORT1FROMMP(mp1) == IDM_PREVWINDOW));
    break;

  case IDM_CASCADE:
    CascadeChildren(hwnd);
    break;

  case IDM_TILE:
    TileChildren(hwnd, TRUE);
    break;

  case IDM_RESTORE:
    MinResChildren(hwnd, SWP_RESTORE);
    break;

  case IDM_MINIMIZE:
    MinResChildren(hwnd, SWP_MINIMIZE);
    break;

  case IDM_ARRANGEICONS:
    ArrangeIcons(hwnd);
    break;

  case IDM_INIVIEWER:
    StartIniEditor(hwnd, NULL, 4);
    break;

  case IDM_EDITASSOC:
    EditAssociations(hwnd);
    break;

  case IDM_EDITCOMMANDS:
    EditCommands(hwnd);
    PostMsg(hwnd, UM_FILLCMDLIST, MPVOID, MPVOID);
    break;

  default:
    if (!SwitchCommand((HWND) WinQueryWindowULong(hwnd, QWL_USER),
		       SHORT1FROMMP(mp1))) {
      if (SHORT1FROMMP(mp1) >= IDM_COMMANDSTART &&
	  SHORT1FROMMP(mp1) < IDM_QUICKTOOLSTART) {

	INT x;
	HWND hwndCnr;

	if (!cmdloaded)
	  load_commands();
	hwndCnr = TopWindow(hwnd, (HWND) 0);
	hwndCnr = (HWND) WinSendMsg(WinWindowFromID(hwndCnr, FID_CLIENT),
				    UM_CONTAINERHWND, MPVOID, MPVOID);
	if (!hwndCnr) {
	  Runtime_Error2(pszSrcFile, __LINE__, IDS_NOWINDOWTEXT);
	  break;
	}
	x = SHORT1FROMMP(mp1) - IDM_COMMANDSTART;
	if (x >= 0) {
	  x++;
	  RunCommand(hwndCnr, x);
	  if (fUnHilite) {

	    PCNRITEM pci;
	    DIRCNRDATA *dcd = NULL;

	    // 12 May 07 SHL fixme to understand? backwards maybe? looking for DIR_CNR?
	    if (WinQueryWindowUShort(hwndCnr, QWS_ID) != TREE_CNR)
	      dcd = INSTDATA(hwndCnr);
	    pci = (PCNRITEM) WinSendMsg(hwndCnr,
					CM_QUERYRECORDEMPHASIS,
					MPFROMLONG(CMA_FIRST),
					MPFROMSHORT(CRA_CURSORED));
	    if (pci && (INT) pci != -1 &&
		(pci->rc.flRecordAttr & CRA_SELECTED))
	    {
	      UnHilite(hwnd,
		       TRUE,
		       dcd ? &dcd->lastselection : NULL,
		       dcd ? dcd ->ulItemsToUnHilite : 0);
	    }
	  }
	}
      }
      else if (SHORT1FROMMP(mp1) >= IDM_QUICKTOOLSTART &&
	       SHORT1FROMMP(mp1) < IDM_QUICKTOOLSTART + 50) {
	if (!qtloaded)
	  load_quicktools();
	if (quicktool[SHORT1FROMMP(mp1) - IDM_QUICKTOOLSTART]) {
	  if (fToolsChanged)
	    save_tools(NULL);
	  if (!load_tools(quicktool[SHORT1FROMMP(mp1) - IDM_QUICKTOOLSTART]))
	    load_tools(NULL);
	  else {
	    strcpy(lasttoolbox,
		   quicktool[SHORT1FROMMP(mp1) - IDM_QUICKTOOLSTART]);
	    PrfWriteProfileString(fmprof, FM3Str, "LastToolBox", lasttoolbox);
	  }
	  BuildTools(hwndToolback, TRUE);
	}
      }
      else {

	HWND hwndActive;

	hwndActive = TopWindow(hwnd, (HWND) 0);
	if (hwndActive)
	  PostMsg(WinWindowFromID(hwndActive, FID_CLIENT),
		  WM_COMMAND, mp1, mp2);
      }
    }
    break;				// default
  } // switch mp1
  return 0;
}

static MRESULT EXPENTRY MainWMOnce(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2)
{
  TID tid;
  SWP swp;
  PFNWP oldproc;
  HWND hwndTmp;
  HWND hwndFrame;
  HWND hwndSysMenu, hwndSysSubMenu, hwndMenu;
  USHORT idSysMenu;
  MENUITEM mi, mit;
  ULONG size;
  BOOL temp = FALSE;

  switch (msg) {
  case WM_CREATE:
    WinQueryWindowProcess(hwnd, &mypid, &tid);
    hwndMain = hwnd;
    WinSetWindowUShort(hwnd, QWL_USER + 8, 0);
    WinSetWindowUShort(hwnd, QWL_USER + 10, 0);
    WinSetWindowUShort(hwnd, QWL_USER + 12, 0);
    WinSetWindowUShort(hwnd, QWL_USER + 16, 0);
    if (_beginthread(MakeMainObjWin, NULL, 245760, MPVOID) == -1) {
      Runtime_Error(pszSrcFile, __LINE__,
		    GetPString(IDS_COULDNTSTARTTHREADTEXT));
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      return 0;
    }
    else
      DosSleep(32);//05 Aug 07 GKY 64

    hwndFrame = WinQueryWindow(hwnd, QW_PARENT);

    /*
     * create frame children (not client children, frame children)
     */
    DosSleep(1);
    WinQueryWindowPos(hwndFrame, &swp);
    oldproc = WinSubclassWindow(hwndFrame, MainFrameWndProc);
    WinSetWindowPtr(hwndFrame, QWL_USER, (PVOID) oldproc);
    CommonCreateMainChildren(hwnd, &swp);

    if (!WinCreateWindow(hwndFrame,
			 WC_BUTTON,
			 "I",
			 WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS,
			 ((swp.cx -
			   WinQuerySysValue(HWND_DESKTOP,
					    SV_CXMINMAXBUTTON)) -
			  WinQuerySysValue(HWND_DESKTOP,
					   SV_CXMINMAXBUTTON) / 2) -
			 WinQuerySysValue(HWND_DESKTOP,
					  SV_CXSIZEBORDER),
			 (swp.cy - WinQuerySysValue(HWND_DESKTOP,
						    SV_CYMINMAXBUTTON)) -
			 WinQuerySysValue(HWND_DESKTOP,
					  SV_CYSIZEBORDER),
			 WinQuerySysValue(HWND_DESKTOP,
					  SV_CXMINMAXBUTTON) / 2,
			 WinQuerySysValue(HWND_DESKTOP,
					  SV_CYMINMAXBUTTON),
			 hwnd, HWND_TOP, IDM_IDEALSIZE, NULL, NULL)) {
      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
    }
    else {
      WinSubclassWindow(WinWindowFromID(hwndFrame, IDM_IDEALSIZE),
			IdealButtonProc);
      SetPresParams(WinWindowFromID(hwndFrame,
				    IDM_IDEALSIZE),
		    NULL, NULL, NULL, GetPString(IDS_10SYSTEMVIOTEXT));
    }

    hwndTmp = WinCreateWindow(hwndFrame,
			      WC_BUTTON,
			      "#1019",
			      WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS |
			      BS_BITMAP,
			      swp.cx - 46,
			      swp.y + 2,
			      24,
			      22, hwnd, HWND_TOP, IDM_OPENWALK, NULL, NULL);
    if (!hwndTmp)
      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

    hwndTmp = WinCreateWindow(hwndFrame,
			      WC_BUTTON,
			      "#3062",
			      WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS |
			      BS_BITMAP,
			      swp.cx - 22,
			      swp.y + 2,
			      24,
			      22, hwnd, HWND_TOP, IDM_USERLIST, NULL, NULL);
    if (!hwndTmp)
      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

    hwndUserlist = WinCreateWindow(hwndFrame,
				   WC_COMBOBOX,
				   (PSZ) NULL,
				   WS_VISIBLE | CBS_DROPDOWN |
				   LS_HORZSCROLL,
				   (swp.x +
				    WinQuerySysValue(HWND_DESKTOP,
						     SV_CXSIZEBORDER) + 48L),
				   (swp.cy -
				    WinQuerySysValue(HWND_DESKTOP,
						     SV_CYSIZEBORDER)) - 60,
				   ((swp.cx -
				     (WinQuerySysValue(HWND_DESKTOP,
						       SV_CXSIZEBORDER) *
				      2)) - 64L), 60L, hwndFrame, HWND_TOP,
				   MAIN_USERLIST, NULL, NULL);
    if (!hwndUserlist)
      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
    hwndCmdlist = WinCreateWindow(hwndFrame,
				  WC_COMBOBOX,
				  (PSZ) NULL,
				  WS_VISIBLE | CBS_DROPDOWN |
				  LS_HORZSCROLL,
				  (swp.x +
				   WinQuerySysValue(HWND_DESKTOP,
						    SV_CXSIZEBORDER) + 48L),
				  (swp.cy -
				   WinQuerySysValue(HWND_DESKTOP,
						    SV_CYSIZEBORDER)) - 60,
				  ((swp.cx -
				    (WinQuerySysValue(HWND_DESKTOP,
						      SV_CXSIZEBORDER) * 2)) -
				   64L), 60L, hwndFrame, HWND_TOP,
				  MAIN_CMDLIST, NULL, NULL);
    if (!hwndCmdlist)
      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
    WinSetWindowText(hwndCmdlist, GetPString(IDS_COMMANDSTEXT));
    hwndStatelist = WinCreateWindow(hwndFrame,
				    WC_COMBOBOX,
				    (PSZ) NULL,
				    WS_VISIBLE | CBS_DROPDOWN |
				    LS_HORZSCROLL,
				    (swp.x +
				     WinQuerySysValue(HWND_DESKTOP,
						      SV_CXSIZEBORDER) + 48L),
				    (swp.cy -
				     WinQuerySysValue(HWND_DESKTOP,
						      SV_CYSIZEBORDER)) - 60,
				    ((swp.cx -
				      (WinQuerySysValue(HWND_DESKTOP,
							SV_CXSIZEBORDER) *
				       2)) - 64L), 60L, hwndFrame, HWND_TOP,
				    MAIN_SETUPLIST, NULL, NULL);
    if (!hwndStatelist)
      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

    hwndDrivelist = WinCreateWindow(hwndFrame,
				    WC_COMBOBOX,
				    (PSZ) NULL,
				    WS_VISIBLE | CBS_DROPDOWN,
				    (swp.x +
				     WinQuerySysValue(HWND_DESKTOP,
						      SV_CXSIZEBORDER)),
				    (swp.cy -
				     WinQuerySysValue(HWND_DESKTOP,
						      SV_CYSIZEBORDER)) - 60,
				    48L,
				    60L,
				    hwndFrame,
				    HWND_TOP, MAIN_DRIVELIST, NULL, NULL);
    if (!hwndDrivelist)
      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
    SetPresParams(hwndDrivelist,
		  NULL, NULL, NULL, GetPString(IDS_10SYSTEMMONOTEXT));
    hwndButtonlist = WinCreateWindow(hwndFrame,
				     WC_COMBOBOX,
				     (PSZ) NULL,
				     WS_VISIBLE | CBS_DROPDOWN |
				     LS_HORZSCROLL,
				     (swp.cx -
				      WinQuerySysValue(HWND_DESKTOP,
						       SV_CXSIZEBORDER)) -
				     164L,
				     (swp.cy -
				      WinQuerySysValue(HWND_DESKTOP,
						       SV_CYSIZEBORDER)) - 60,
				     164L, 60L, hwndFrame, HWND_TOP,
				     MAIN_BUTTONLIST, NULL, NULL);
    if (!hwndButtonlist)
      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
    WinSendMsg(WinWindowFromID(hwndUserlist, CBID_EDIT),
	       EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendMsg(WinWindowFromID(hwndStatelist, CBID_EDIT),
	       EM_SETTEXTLIMIT, MPFROM2SHORT(STATE_NAME_MAX_BYTES, 0), MPVOID);
    WinSendMsg(WinWindowFromID(hwndDrivelist, CBID_EDIT),
	       EM_SETREADONLY, MPFROM2SHORT(TRUE, 0), MPVOID);
    WinSendMsg(WinWindowFromID(hwndButtonlist, CBID_EDIT),
	       EM_SETREADONLY, MPFROM2SHORT(TRUE, 0), MPVOID);
    WinSendMsg(WinWindowFromID(hwndCmdlist, CBID_EDIT),
	       EM_SETREADONLY, MPFROM2SHORT(TRUE, 0), MPVOID);

    oldproc = WinSubclassWindow(WinWindowFromID(hwndUserlist, CBID_EDIT),
				DropDownListProc);
    WinSetWindowPtr(WinWindowFromID(hwndUserlist, CBID_EDIT),
		    QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(WinWindowFromID(hwndCmdlist, CBID_EDIT),
				DropDownListProc);
    WinSetWindowPtr(WinWindowFromID(hwndCmdlist, CBID_EDIT),
		    QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(WinWindowFromID(hwndButtonlist, CBID_EDIT),
				DropDownListProc);
    WinSetWindowPtr(WinWindowFromID(hwndButtonlist, CBID_EDIT),
		    QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(WinWindowFromID(hwndStatelist, CBID_EDIT),
				DropDownListProc);
    WinSetWindowPtr(WinWindowFromID(hwndStatelist, CBID_EDIT),
		    QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(WinWindowFromID(hwndDrivelist, CBID_EDIT),
				DropDownListProc);
    WinSetWindowPtr(WinWindowFromID(hwndDrivelist, CBID_EDIT),
		    QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(hwndUserlist, DropDownListProc);
    WinSetWindowPtr(hwndUserlist, QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(hwndCmdlist, DropDownListProc);
    WinSetWindowPtr(hwndCmdlist, QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(hwndStatelist, DropDownListProc);
    WinSetWindowPtr(hwndStatelist, QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(hwndDrivelist, DropDownListProc);
    WinSetWindowPtr(hwndDrivelist, QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(hwndButtonlist, DropDownListProc);
    WinSetWindowPtr(hwndButtonlist, QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(WinWindowFromID(hwndFrame, IDM_USERLIST),
				ChildFrameButtonProc);
    WinSetWindowPtr(WinWindowFromID(hwndFrame, IDM_USERLIST),
		    QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(WinWindowFromID(hwndFrame, IDM_OPENWALK),
				ChildFrameButtonProc);
    WinSetWindowPtr(WinWindowFromID(hwndFrame, IDM_OPENWALK),
		    QWL_USER, (PVOID) oldproc);
    hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
    WinSendMsg(hwnd, UM_ADDTOMENU, MPVOID, MPVOID);
    SetToggleChecks(hwndMenu);
    SetConditionalCascade(hwndMenu, IDM_TOOLSUBMENU, IDM_TOOLBAR);
    SetConditionalCascade(hwndMenu, IDM_AUTOVIEWSUBMENU, IDM_AUTOVIEW);
    SetConditionalCascade(hwndMenu, IDM_TILEMENU, IDM_TILE);
    WinSetWindowULong(hwnd, QWL_USER, hwndMenu);
    memset(&mi, 0, sizeof(mi));
    memset(&mit, 0, sizeof(mit));
    hwndSysMenu = WinWindowFromID(hwndFrame, FID_SYSMENU);
    idSysMenu = SHORT1FROMMR(WinSendMsg(hwndSysMenu,
					MM_ITEMIDFROMPOSITION,
					MPVOID, MPVOID));
    WinSendMsg(hwndSysMenu,
	       MM_QUERYITEM, MPFROM2SHORT(idSysMenu, 0), MPFROMP(&mit));
    hwndSysSubMenu = mit.hwndSubMenu;
    mi.iPosition = MIT_END;
    mi.afStyle = MIS_SEPARATOR;
    mi.id = (USHORT) - 1;
    WinSendMsg(hwndSysSubMenu, MM_INSERTITEM, MPFROMP(&mi), MPFROMP(NULL));
    mi.afStyle = MIS_TEXT;
    mi.id = IDM_IDEALSIZE;
    WinSendMsg(hwndSysSubMenu,
	       MM_INSERTITEM,
	       MPFROMP(&mi), MPFROMP(GetPString(IDS_IDEALMENUTEXT)));
    mi.afStyle = MIS_TEXT;
    mi.id = IDM_HIDEMENU;
    WinSendMsg(hwndSysSubMenu,
	       MM_INSERTITEM,
	       MPFROMP(&mi), MPFROMP(GetPString(IDS_HIDEMENUTEXT)));
    SetSysMenu(hwndSysMenu);

    size = sizeof(BOOL);
    if (PrfQueryProfileData(fmprof,
			    FM3Str,
			    "MenuInvisible", &temp, &size) && size && temp)
      WinSendMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_HIDEMENU, 0), MPVOID);
    size = sizeof(BOOL);
    if (PrfQueryProfileData(fmprof,
			    FM3Str, "FreeTree", &temp, &size) && size && temp)
      WinSendMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_FREETREE, 0), MPVOID);
    size = sizeof(BOOL);
    if (PrfQueryProfileData(fmprof,
			    FM3Str,
			    "AutoTile", &temp, &size) && size && !temp)
      WinSendMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_AUTOTILE, 0), MPVOID);
    size = sizeof(BOOL);
    if (PrfQueryProfileData(fmprof,
			    FM3Str, "Toolbar", &temp, &size) && size && !temp)
      WinSendMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_TOOLBAR, 0), MPVOID);

    WinSetWindowText(WinWindowFromID(hwndFrame, FID_TITLEBAR), "FM/2");
    FixSwitchList(hwndFrame, NULL);
    break;				// WM_CREATE

  case UM_SETUP:
    /*
     * start up some initial children
     */
    load_tools(NULL);
    BuildTools(hwndToolback, TRUE);
    WinShowWindow(WinQueryWindow(hwnd, QW_PARENT), TRUE);
    PostMsg(MainObjectHwnd, UM_SETUP2, mp1, mp2);
    return 0;

  case UM_SETUP2:
    {
      SWP swp;
      ULONG size = sizeof(SWP);

      WinQueryWindowPos(hwnd, &swp);
      hwndTree = StartTreeCnr(hwnd, 0);
      if (!hwndTree)
	WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
      else {
	if (!fSaveState ||
	    !PrfQueryProfileData(fmprof,
				 FM3Str,
				 "LastTreePos",
				 &swp, &size) || size != sizeof(SWP)) {

	  INT ratio, height = 0;

	  if (!fNoTreeGap)
	    height = WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 2;
	  size = sizeof(ratio);
	  if (!PrfQueryProfileData(fmprof,
				   FM3Str,
				   "TreeWindowRatio",
				   (PVOID) & ratio,
				   &size) || size < sizeof(ratio))
	    ratio = 400;
	  WinSetWindowPos(hwndTree,
			  HWND_TOP,
			  0,
			  height,
			  (swp.cx * 100) / ratio,
			  swp.cy - height,
			  SWP_SHOW | SWP_SIZE | SWP_MOVE |
			  SWP_ACTIVATE | SWP_ZORDER);
	}
	else
	  WinSetWindowPos(hwndTree,
			  HWND_TOP,
			  swp.x,
			  swp.y,
			  swp.cx,
			  swp.cy,
			  swp.fl | SWP_MOVE | SWP_SIZE | SWP_SHOW |
			  SWP_ZORDER | SWP_ACTIVATE);
      }
      ResizeTools(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				  MAIN_TOOLS));
    }
    PostMsg(MainObjectHwnd, UM_SETUP3, mp1, mp2);
    return 0;

  case UM_SETUP3:
    /* start remaining child windows */
    if (!fNoSaveState && fSaveState)
      PostMsg(MainObjectHwnd, UM_RESTORE, MPVOID, MPVOID);
    PostMsg(MainObjectHwnd, UM_SETUP4, mp1, mp2);
    return 0;

  case UM_SETUP4:
    {
      INT argc = (INT) mp1, x;
      CHAR **argv = (CHAR **) mp2;

      for (x = 1; x < argc; x++) {
	if (*argv[x] == '/' || *argv[x] == ';')
	  continue;
	if (!IsFile(argv[x]) && !FindDirCnrByName(argv[x], FALSE))
	  OpenDirCnr((HWND) 0, hwndMain, hwndTree, TRUE, argv[x]);
      }
    }
    PostMsg(MainObjectHwnd, UM_SETUP5, MPVOID, MPVOID);
    return 0;

  case UM_SETUP5:
    if (fAutoTile)
      TileChildren(hwnd, TRUE);
    PostMsg(hwnd, UM_FILLUSERLIST, MPVOID, MPVOID);
    PostMsg(hwnd, UM_FILLSETUPLIST, MPVOID, MPVOID);
    PostMsg(hwnd, UM_FILLCMDLIST, MPVOID, MPVOID);
    PostMsg(hwnd, UM_FILLBUTTONLIST, MPVOID, MPVOID);
    {
      HWND hwndActive;

      hwndActive = TopWindow(hwnd, hwndTree);
      if (hwndActive)
	WinSetWindowPos(hwndActive, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE);
    }
    if (fStartMinimized || fReminimize)
      PostMsg(hwndTree, UM_MINIMIZE, MPVOID, MPVOID);
    else if (fStartMaximized)
      PostMsg(hwndTree, UM_MAXIMIZE, MPVOID, MPVOID);
    fRunning = TRUE;
    return 0;
  }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY MainWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_CREATE:
  case UM_SETUP:
  case UM_SETUP2:
  case UM_SETUP3:
  case UM_SETUP4:
  case UM_SETUP5:
    return MainWMOnce(hwnd, msg, mp1, mp2);

  case WM_CONTROLPOINTER:
    if (!fNoFinger &&
	(SHORT1FROMMP(mp1) == IDM_OPENWALK ||
	 SHORT1FROMMP(mp1) == IDM_USERLIST))
      return MRFROMLONG(hptrFinger);
    break;

  case UM_LOADFILE:
  case UM_THREADUSE:
  case UM_BUILDDRIVEBAR:
    return CommonMainWndProc(hwnd, msg, mp1, mp2);

  case WM_BUTTON1UP:
  case WM_BUTTON2UP:
  case WM_BUTTON3UP:
  case WM_MOUSEMOVE:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    break;

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_MENUEND:
    if ((HWND) mp2 == MainPopupMenu) {
      WinDestroyWindow(MainPopupMenu);
      MainPopupMenu = (HWND) 0;
    }
    break;

  case UM_CONTEXTMENU:
  case WM_CONTEXTMENU:
    if (CheckMenu(&MainPopupMenu, MAIN_POPUP)) {
      SetToggleChecks(MainPopupMenu);
      PopupMenu(hwnd, hwnd, MainPopupMenu);
    }
    if (msg == UM_CONTEXTMENU)
      return 0;
    return MRFROMSHORT(TRUE);

  case UM_SETUSERLISTNAME:
    if (mp1) {
      if (fUserComboBox)
	WinSetWindowText(WinWindowFromID(hwndUserlist, CBID_EDIT),
			 (CHAR *) mp1);
      if (add_udir(FALSE, (CHAR *) mp1)) {
	if (fUserComboBox && fAutoAddDirs) {
	  WinSendMsg(hwndUserlist, LM_INSERTITEM,
		     MPFROM2SHORT(LIT_SORTASCENDING, 0),
		     MPFROMP((CHAR *) mp1));
	}
      }
    }
    return 0;

  case UM_ARRANGEICONS:
    ArrangeIcons(hwnd);
    return 0;

  case WM_CHORD:
    PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_WINDOWDLG, 0), MPVOID);
    break;

  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case UM_FOCUSME:
    WinSetFocus(hwndTree, TRUE);
    return 0;

  case UM_RESCAN:
    TileChildren(hwnd, TRUE);
    return 0;

  case WM_SAVEAPPLICATION:
    {
      SWP swp;

      WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
	WinStoreWindowPos(FM2Str,
			  "MainWindowPos", WinQueryWindow(hwnd, QW_PARENT));
	if (!fNoSaveState && fSaveState)
	  SaveDirCnrState(hwnd, NULL);
      }
    }
    break;

  case MM_PORTHOLEINIT:
    switch (SHORT1FROMMP(mp1)) {
    case 0:
    case 1:
      {
	HWND hwndCurrent;
	ULONG wmsg;

	wmsg = (SHORT1FROMMP(mp1) == 0) ? UM_FILESMENU : UM_VIEWSMENU;
	hwndCurrent = TopWindow(hwnd, (HWND) 0);
	PortholeInit((HWND) WinSendMsg(WinWindowFromID(hwndCurrent,
						       FID_CLIENT), wmsg,
				       MPVOID, MPVOID), mp1, mp2);
      }
      break;
    }
    break;

  case WM_INITMENU:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_CONFIGMENU:
      SetToggleChecks((HWND) WinQueryWindowULong(hwnd, QWL_USER));
      break;

    case IDM_WINDOWSMENU:
      /*
       * add child windows of client
       * and switchlist entries to end of pulldown menu
       */
      {
	HWND hwndMenu, hwndSubMenu;
	MENUITEM mi;

	hwndMenu = WinQueryWindowULong(hwnd, QWL_USER);
	memset(&mi, 0, sizeof(mi));
	mi.iPosition = MIT_END;
	mi.afStyle = MIS_TEXT;
	if (!WinSendMsg(hwndMenu, MM_QUERYITEM,
			MPFROM2SHORT(IDM_WINDOWSMENU, TRUE), MPFROMP(&mi)))
	  break;
	hwndSubMenu = mi.hwndSubMenu;
	SetupWinList(hwndSubMenu, hwnd, WinQueryWindow(hwnd, QW_PARENT));
      }
      break;

    default:
      {
	HWND hwndCurrent;

	hwndCurrent = TopWindow(hwnd, (HWND) 0);
	if (hwndCurrent)
	  WinSendMsg(hwndCurrent, UM_INITMENU, mp1, mp2);
      }
      break;
    }
    break;

  case UM_ADDTOMENU:
    AddToMenu((CHAR *) mp1, WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					    FID_MENU));
    return 0;

  case UM_FILLCMDLIST:
    WinSendMsg(hwndCmdlist, LM_DELETEALL, MPVOID, MPVOID);
    if (!cmdloaded)
      load_commands();
    if (cmdhead) {

      LINKCMDS *info;

      info = cmdhead;
      while (info) {
	WinSendMsg(hwndCmdlist, LM_INSERTITEM,
		   MPFROM2SHORT(LIT_END, 0), MPFROMP(info->title));
	info = info->next;
      }
    }
    return 0;

  case UM_FILLSETUPLIST:
    fill_setups_list();
    return 0;

  case UM_FILLBUTTONLIST:
    WinSendMsg(hwndButtonlist, LM_DELETEALL, MPVOID, MPVOID);
    if (fUserComboBox) {

      BOOL foundit = FALSE, thisone;
      ULONG ulSearchCount;
      SHORT sSelect;
      FILEFINDBUF3 findbuf;
      HDIR hDir;
      CHAR *p;

      DosError(FERR_DISABLEHARDERR);
      hDir = HDIR_CREATE;
      ulSearchCount = 1;
      if (!xDosFindFirst("*.TLS", &hDir, FILE_READONLY | FILE_ARCHIVED,
			 &findbuf, sizeof(FILEFINDBUF3),
			 &ulSearchCount, FIL_STANDARD)) {
	do {
	  priority_bumped();
	  if (!foundit) {
	    thisone = FALSE;
	    p = strrchr(lasttoolbox, '\\');
	    if (!p)
	      p = lasttoolbox;
	    else
	      p++;
	    if (!stricmp(findbuf.achName, p))
	      thisone = TRUE;
	  }
	  p = strrchr(findbuf.achName, '.');
	  if (p)
	    *p = 0;
	  sSelect = (SHORT) WinSendMsg(hwndButtonlist, LM_INSERTITEM,
				       MPFROM2SHORT(LIT_SORTASCENDING, 0),
				       MPFROMP(findbuf.achName));
	  if (!foundit && thisone && sSelect >= 0) {
	    WinSendMsg(hwndButtonlist, LM_SELECTITEM,
		       MPFROM2SHORT(sSelect, 0), MPFROMLONG(TRUE));
	    foundit = TRUE;
	  }
	}
	while (!xDosFindNext(hDir, &findbuf, sizeof(FILEFINDBUF3),
			     &ulSearchCount));
	DosFindClose(hDir);
	priority_bumped();
      }
      WinSetWindowText(hwndButtonlist, GetPString(IDS_TOOLBOXTEXT));
    }
    return 0;

  case UM_FILLUSERLIST:
    WinSendMsg(hwndUserlist, LM_DELETEALL, MPVOID, MPVOID);
    if (fUserComboBox) {
      ULONG ulDriveNum;
      ULONG ulDriveMap;
      ULONG ulSearchCount;
      FILEFINDBUF3 findbuf;
      HDIR hDir;
      APIRET rc;
      LINKDIRS *info;
      LINKDIRS *temp;

      if (!loadedudirs)
	load_udirs();
      DosError(FERR_DISABLEHARDERR);
      DosQCurDisk(&ulDriveNum, &ulDriveMap);
      info = udirhead;
      while (info) {
	if (IsFullName(info->path) &&
	    !(driveflags[toupper(*info->path) - 'A'] &
	      (DRIVE_IGNORE | DRIVE_INVALID))) {
	  DosError(FERR_DISABLEHARDERR);
	  hDir = HDIR_CREATE;
	  ulSearchCount = 1;
	  if (!IsRoot(info->path))
	    rc = xDosFindFirst(info->path, &hDir, FILE_DIRECTORY |
			       MUST_HAVE_DIRECTORY | FILE_READONLY |
			       FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
			       &findbuf, sizeof(FILEFINDBUF3),
			       &ulSearchCount, FIL_STANDARD);
	  else {
	    rc = 0;
	    findbuf.attrFile = FILE_DIRECTORY;
	  }
	  priority_bumped();
	  if (!rc) {
	    if (!IsRoot(info->path))
	      DosFindClose(hDir);
	    if (findbuf.attrFile & FILE_DIRECTORY)
	      WinSendMsg(hwndUserlist, LM_INSERTITEM,
			 MPFROM2SHORT(LIT_SORTASCENDING, 0),
			 MPFROMP(info->path));
	    else {
	      temp = info->next;
	      remove_udir(info->path);
	      info = temp;
	      continue;
	    }
	  }
	  else if (!(ulDriveMap & (1L << (toupper(*info->path) - 'A')))) {
	    temp = info->next;
	    remove_udir(info->path);
	    info = temp;
	    continue;
	  }
	}
	info = info->next;
      }
      info = ldirhead;
      while (info) {
	if (IsFullName(info->path) &&
	    !(driveflags[toupper(*info->path) - 'A'] &
	      (DRIVE_IGNORE | DRIVE_INVALID))) {
	  DosError(FERR_DISABLEHARDERR);
	  hDir = HDIR_CREATE;
	  ulSearchCount = 1;
	  if (!IsRoot(info->path))
	    rc = xDosFindFirst(info->path, &hDir, FILE_DIRECTORY |
			       MUST_HAVE_DIRECTORY | FILE_READONLY |
			       FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
			       &findbuf, sizeof(FILEFINDBUF3),
			       &ulSearchCount, FIL_STANDARD);
	  else {
	    rc = 0;
	    findbuf.attrFile = FILE_DIRECTORY;
	  }
	  priority_bumped();
	  if (!rc) {
	    if (!IsRoot(info->path))
	      DosFindClose(hDir);
	    if (findbuf.attrFile & FILE_DIRECTORY)
	      WinSendMsg(hwndUserlist, LM_INSERTITEM,
			 MPFROM2SHORT(LIT_SORTASCENDING, 0),
			 MPFROMP(info->path));
	    else {
	      temp = info->next;
	      remove_udir(info->path);
	      info = temp;
	      continue;
	    }
	  }
	  else if (!(ulDriveMap & (1L << (toupper(*info->path) - 'A')))) {
	    temp = info->next;
	    remove_udir(info->path);
	    info = temp;
	    continue;
	  }
	}
	info = info->next;
      }
      WinSendMsg(hwndUserlist, LM_INSERTITEM,
		 MPFROM2SHORT(0, 0),
		 MPFROMP(GetPString(IDS_NEWDIRECTORYTEXT)));
      WinSetWindowText(hwndUserlist, GetPString(IDS_COMMONDIRTEXT));
    }
    return 0;

  case UM_SIZE:
    if (fAutoTile)
      TileChildren(hwnd, FALSE);
    else
      MoveChildrenAwayFromTree(hwnd);
    return 0;

  case WM_SIZE:
    ResizeChildren(hwnd, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1),
		   SHORT1FROMMP(mp2), SHORT2FROMMP(mp2));
    break;

  case WM_ERASEBACKGROUND:
    WinFillRect((HPS) mp1, (PRECTL) mp2, 0x00d0d0d0);
    return 0;

  case WM_PAINT:
    {
      HPS hps;
      RECTL rcl;

      hps = WinBeginPaint(hwnd, (HPS) 0, NULL);
      if (hps) {
	WinQueryWindowRect(hwnd, &rcl);
	WinFillRect(hps, &rcl, CLR_PALEGRAY);
	WinEndPaint(hps);
      }
    }
    break;

  case UM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case MAIN_CMDLIST:
    case MAIN_SETUPLIST:
    case MAIN_DRIVELIST:
    case MAIN_USERLIST:
    case MAIN_BUTTONLIST:
      switch (SHORT2FROMMP(mp1)) {
      case CBN_ENTER:
	{
	  HWND hwndUL = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					SHORT1FROMMP(mp1));
	  CHAR path[CCHMAXPATH];
	  ULONG ul;

	  switch (SHORT1FROMMP(mp1)) {
	  case MAIN_USERLIST:
	    ul = CCHMAXPATH;
	    break;
	  case MAIN_SETUPLIST:
	    ul = STATE_NAME_MAX_BYTES;
	    break;
	  default:
	    ul = 13;			// fixme to not be hardcoded
	  }

	  SetShiftState();
	  WinQueryWindowText(WinWindowFromID(hwndUL, CBID_EDIT), ul, path);
	  bstrip(path);
	  if (*path) {
	    if (SHORT1FROMMP(mp1) == MAIN_USERLIST) {
	      if (!strcmp(path, GetPString(IDS_NEWDIRECTORYTEXT))) {
		if (!LastDir ||
		    !WinSendMsg(WinQueryWindow(LastDir, QW_PARENT),
				UM_CONTAINERDIR, MPFROMP(path), MPVOID))
		  save_dir2(path);
		if (!PMMkDir(hwnd, path, TRUE)) {
		  WinSetWindowText(hwndUL, GetPString(IDS_COMMONDIRTEXT));
		  break;
		}
	      }
	      if (!IsFile(path) && !FindDirCnrByName(path, TRUE)) {

		HWND hwndDir;

		if ((fUserListSwitches &&
		     !(shiftstate & KC_SHIFT)) ||
		    (!fUserListSwitches && (shiftstate & KC_SHIFT))) {
		  hwndDir = FindDirCnr(hwnd);
		  if (hwndDir) {
		    WinSendMsg(LastDir, UM_SETDIR, MPFROMP(path), MPVOID);
		    break;
		  }
		}
		OpenDirCnr((HWND) 0, hwndMain, hwndTree, FALSE, path);
	      }
	    }
	    else if (SHORT1FROMMP(mp1) == MAIN_DRIVELIST) {
	      ShowTreeRec(WinWindowFromID(WinWindowFromID(hwndTree,
							  FID_CLIENT),
					  TREE_CNR), path, FALSE, TRUE);
	      WinSetFocus(HWND_DESKTOP, hwndTree);
	    }
	    else if (SHORT1FROMMP(mp1) == MAIN_BUTTONLIST) {
	      strcat(path, ".TLS");
	      load_tools(path);
	      PrfWriteProfileString(fmprof,
				    FM3Str, "LastToolBox", lasttoolbox);
	      BuildTools(hwndToolback, TRUE);
	      WinSetWindowText(hwndButtonlist, GetPString(IDS_TOOLBOXTEXT));
	    }
	    else if (SHORT1FROMMP(mp1) == MAIN_SETUPLIST) {

	      CHAR szKey[80];
	      ULONG size, numsaves = 0;

	      SetShiftState();
	      size = sizeof(ULONG);
	      sprintf(szKey, "%s.NumDirsLastTime", path);	// path is state name
	      if (!PrfQueryProfileData(fmprof,
				       FM3Str,
				       szKey,
				       (PVOID)&numsaves,
				       &size))
	      {
		if ((WinGetLastError(WinQueryAnchorBlock(hwnd)) & 0xffff) == PMERR_NOT_IN_IDX) {
		  saymsg(MB_ENTER | MB_ICONASTERISK, hwnd,
			 GetPString(IDS_WARNINGTEXT),
			 GetPString(IDS_DOESNTEXISTTEXT), path);
		}
		else {
		  Win_Error2(hwnd, hwnd, __FILE__, __LINE__,
			     IDS_PRFQUERYPROFILEDATA);
		}
	      }
	      else if (!numsaves)
		Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
	      else {
		if ((shiftstate & KC_SHIFT) == 0)
		  PostMsg(MainObjectHwnd, UM_RESTORE, MPVOID, MPFROMLONG(2L));
		{
		  char *temp;

		  temp = xstrdup(path, pszSrcFile, __LINE__);
		  if (!temp) {
		    if ((shiftstate & KC_SHIFT) != 0 || fAutoTile)
		      PostMsg(MainObjectHwnd, UM_RESTORE, MPVOID,
			      MPFROMLONG(1));
		  }
		  else {
		    if (!PostMsg
			(MainObjectHwnd, UM_RESTORE, MPFROMP(temp), MPVOID))
		      free(temp);
		  }
		}
	      }
	      // fixme to hold restored state name for a while
	      // WinSetWindowText(hwndStatelist, GetPString(IDS_STATETEXT));	// 15 Apr 07 SHL
	    }
	    else if (SHORT1FROMMP(mp1) == MAIN_CMDLIST) {

	      SHORT sSelect;

	      sSelect = (SHORT) WinSendMsg(hwndCmdlist,
					   LM_QUERYSELECTION,
					   MPFROMSHORT(LIT_FIRST), MPVOID);
	      if (sSelect >= 0)
		WinPostMsg(hwnd,
			   WM_COMMAND,
			   MPFROM2SHORT(IDM_COMMANDSTART + sSelect, 0),
			   MPVOID);
	      WinSetWindowText(hwndCmdlist, GetPString(IDS_COMMANDSTEXT));
	    }
	  }
	}
	break;

      default:
	break;
      }
      break;

    default:
      break;
    }
    return 0;

  case WM_HELP:
    WinSendMsg(hwndHelp, HM_HELP_CONTENTS, MPVOID, MPVOID);
    break;

  case UM_COMMAND:
  case WM_COMMAND:

    return MainWMCommand(hwnd, msg, mp1, mp2);

  case WM_CLOSE:
    WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
	       WM_SYSCOMMAND, MPFROM2SHORT(SC_RESTORE, 0), MPVOID);
    WinSendMsg(hwnd, WM_SAVEAPPLICATION, MPVOID, MPVOID);
    fAmClosing = TRUE;
    WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
	       WM_SYSCOMMAND, MPFROM2SHORT(SC_MINIMIZE, 0), MPVOID);
    if (CloseChildren(hwnd)) {
      fAmClosing = FALSE;
      if (fAutoTile)
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_TILE, 0), MPVOID);
      return 0;
    }
    if (hwndTree) {
      if (!PostMsg(hwndTree, WM_CLOSE, MPVOID, MPVOID))
	WinSendMsg(hwndTree, WM_CLOSE, MPVOID, MPVOID);
    }
    DosSleep(1);
    return 0;				// Hold off WM_QUIT

  case UM_CLOSE:
    HideNote();
    WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
    return 0;

  case UM_RESTORE:
    {
      char *temp;

      temp = xstrdup(GetPString(IDS_FM2TEMPTEXT), pszSrcFile, __LINE__);
      if (temp) {
	if (!PostMsg(MainObjectHwnd, UM_RESTORE, MPFROMP(temp), MPVOID))
	  free(temp);
      }
    }
    return 0;

  case UM_SETDIR:
    /* mp1 == name of directory to open */
    if (mp1)
      return MRFROMLONG(OpenDirCnr((HWND) 0,
				   hwndMain,
				   hwndTree, (BOOL) mp2, (char *)mp1));
    return 0;

  case WM_DESTROY:
    hwndMain = (HWND) 0;
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    break;
  }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(MISC8,SetToggleChecks,FindDirCnrByName,TopWindow)
#pragma alloc_text(MISC8,TopWindowName,CountDirCnrs)
#pragma alloc_text(MAINWND,AdjustSizeOfClient,FillClient,ChildButtonProc)
#pragma alloc_text(MAINWND,ToolBackProc,StatusProc)
#pragma alloc_text(MAINWND,MoveChildrenAwayFromTree,ArrangeIcons,NextChild)
#pragma alloc_text(MAINWND,ChildFrameButtonProc)
#pragma alloc_text(MAINWND2,CloseChildren,CountChildren,GetNextWindowPos)
#pragma alloc_text(MAINWND2,CascadeChildren,TileChildren,ResizeChildren)
#pragma alloc_text(MAINWND2,MinResChildren,MainFrameWndProc,MainWndProc)
#pragma alloc_text(MAINWND2,DropDownListProc)
#pragma alloc_text(MAINWND3,RestoreDirCnrState,SaveDirCnrState)
#pragma alloc_text(MAINWND3,CloseDirCnrChildren,TransformSwp)
#pragma alloc_text(MAINWND3,ResizeTools,BuildTools,CommandLineProc)
#pragma alloc_text(MAINWND4,DriveProc,DriveBackProc,BuildDriveBarButtons,ResizeDrives)
#pragma alloc_text(MAINWND4,LEDProc,IdealButtonProc)
#pragma alloc_text(MAINWND5,MainWMOnce)
#pragma alloc_text(MAINWND6,MainWMCommand)
#pragma alloc_text(BUBBLE,MakeBubble,BubbleProc,BubbleHelp)
#pragma alloc_text(MAINOBJ,MainObjectWndProc,MakeMainObjWin)

