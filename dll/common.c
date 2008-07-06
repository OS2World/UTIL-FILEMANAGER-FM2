
/***********************************************************************

  $Id$

  Common window functions

  Copyright (c) 1993, 1998 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  13 Aug 05 SHL Renames
  22 Jul 06 SHL Check more run time errors
  15 Aug 06 SHL Use Dos_Error
  03 Nov 06 SHL Rework thread usage count logic
  07 Jan 07 GKY Move error strings etc. to string file
  07 Jan 07 GKY Updated Helv font to vector font Helvetica
  22 Mar 07 GKY Use QWL_USER
  30 Mar 07 GKY Remove GetPString for window class names
  02 Aug 07 SHL Sync with CNRITEM mods
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  26 Aug 07 SHL Change to DosSleep(0)
  29 Feb 08 GKY Use xfree where appropriate
  06 Jul 08 GKY Update delete/undelete to include move to and open XWP trashcan

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "filldir.h"			// RemoveCnrItems
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "fm3dll.h"
#include "fortify.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

MRESULT EXPENTRY CommonFrameWndProc(USHORT id,
				    HWND hwnd,
				    ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg) {
  case UM_TIMER:
  case UM_ACTION:
  case UM_SHOWME:
  case UM_OPENWINDOWFORME:
  case UM_MINIMIZE:
  case UM_MAXIMIZE:
  case UM_CONTAINERDIR:
  case UM_FILTER:
  case UM_INITMENU:
  case UM_COMMAND:
  case UM_UPDATERECORD:
  case UM_UPDATERECORDLIST:
  case WM_COMMAND:
  case MM_PORTHOLEINIT:
  case WM_INITMENU:
  case UM_CONTAINER_FILLED:
  case UM_FILESMENU:
    return WinSendMsg(WinWindowFromID(hwnd, FID_CLIENT), msg, mp1, mp2);

  case UM_CONTEXTMENU:
    PostMsg(WinWindowFromID(hwnd, FID_CLIENT), msg, mp1, mp2);
    return 0;

  case UM_CONTAINERHWND:
    return MRFROMLONG(WinWindowFromID(hwnd, id));

  case DM_DROP:
  case DM_DRAGOVER:
  case DM_DRAGLEAVE:
  case DM_DROPHELP:
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
      cnd.pDragInfo = (PDRAGINFO) mp1;
      cnd.pRecord = NULL;
      return WinSendMsg(WinWindowFromID(hwnd, FID_CLIENT),
			WM_CONTROL, MPFROM2SHORT(id, dcmd), MPFROMP(&cnd));
    }

  case UM_RESCAN:
    if (fAutoTile &&
	!fAmClosing && !fNoTileUpdate && !ParentIsDesktop(hwnd, (HWND) 0))
      PostMsg(WinQueryWindow(hwnd, QW_PARENT), UM_RESCAN, MPVOID, MPVOID);
    return 0;

  case WM_MINMAXFRAME:
    {
      PSWP pswp = (PSWP) mp1;
      MRESULT rc;

      rc = oldproc(hwnd, msg, mp1, mp2);
      if (pswp->fl & (SWP_MINIMIZE | SWP_HIDE | SWP_RESTORE))
	WinSendMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      return rc;
    }

  case WM_DESTROY:
    WinSendMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    break;
  }
  return oldproc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CommonTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_CREATE:
    {
      MRESULT rc;

      rc = PFNWPStatic(hwnd, msg, mp1, mp2);
      switch (WinQueryWindowUShort(hwnd, QWS_ID))
      case DIR_SORT:
      case DIR_VIEW:
      case DIR_FILTER:

      SetPresParams(hwnd, &RGBGREY, &RGBBLACK, &RGBBLACK, GetPString(IDS_8HELVTEXT));
      return rc;
    }
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

void CommonTextPaint(HWND hwnd, HPS hps)
{
  int x;
  USHORT ids[] = { DIR_FOLDERICON, DIR_TOTALS, DIR_SELECTED, DIR_VIEW,
    DIR_FILTER, DIR_SORT, DIR_MAX, 0
  };
  BOOL bools[] = { TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE };

  for (x = 0; ids[x]; x++) {
    if (WinWindowFromID(hwnd, ids[x]) != (HWND) 0)
      PaintRecessedWindow(WinWindowFromID(hwnd, ids[x]), hps,
			  bools[x], FALSE);
  }
}

void CommonCreateTextChildren(HWND hwnd, char *class, USHORT * ids)
{
  int x;
  CHAR s[33];
  ULONG attrs;
  HWND hwndTmp;

  if (!hwnd || !class || !ids) {
    Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
    return;
  }

  for (x = 0; ids[x]; x++) {
    *s = 0;
    switch (ids[x]) {
    case DIR_SORT:
    case DIR_FILTER:
    case DIR_VIEW:
      attrs = SS_TEXT | DT_LEFT | DT_VCENTER;
      break;
    case DIR_FOLDERICON:
    case DIR_MAX:
      attrs = SS_BITMAP;
      sprintf(s, "#%d", ids[x]);
      break;
    default:
      attrs = SS_TEXT | DT_CENTER | DT_VCENTER;
      break;
    }
    hwndTmp = WinCreateWindow(hwnd, class, s, attrs, 0, 0, 0, 0, hwnd,
			      HWND_TOP, ids[x], NULL, NULL);
    if (!hwndTmp)
      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
  } // for
}

void CommonDriveCmd(HWND hwnd, char *drive, USHORT cmd)
{
  char dv[CCHMAXPATH];

  if (!drive || !*drive)
    return;
  strcpy(dv, drive);
  MakeValidDir(dv);

  switch (cmd) {
  case IDM_MKDIR:
    PMMkDir(hwnd, dv, FALSE);
    break;
  case IDM_INFO:
    WinDlgBox(HWND_DESKTOP,
	      HWND_DESKTOP,
	      DrvInfoProc, FM3ModHandle, INFO_FRAME, (PVOID) dv);
    break;
  case IDM_DRVFLAGS:
    strcpy(dv, drive);			// Must use original drive letter
    if (WinDlgBox(HWND_DESKTOP,
		  hwnd,
		  SetDrvProc,
		  FM3ModHandle, DVS_FRAME, MPFROMP(dv)) && hwndTree)
      PostMsg(hwndTree, WM_COMMAND, MPFROM2SHORT(IDM_UPDATE, 0), MPVOID);
    break;
  case IDM_SIZES:
    WinDlgBox(HWND_DESKTOP,
	      HWND_DESKTOP, DirSizeProc, FM3ModHandle, DSZ_FRAME, dv);
    break;
  case IDM_SHOWALLFILES:
    StartSeeAll(HWND_DESKTOP, FALSE, dv);
    break;
  case IDM_UNDELETE:
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
                FM3ModHandle, UNDEL_FRAME, MPFROMP(dv));
    }
    break;
  case IDM_CHKDSK:
    runemf2(SEPARATE | WINDOWED,
	    hwnd, pszSrcFile, __LINE__, NULL, NULL,
	    "PMCHKDSK.EXE %c:", toupper(*dv));
    break;
  case IDM_OPTIMIZE:
    {
      CHAR FileSystem[CCHMAXPATH];
      ULONG type;

      CheckDrive(*dv, FileSystem, &type);
      if (*FileSystem) {
	strcat(FileSystem, "OPT.CMD");
	runemf2(SEPARATE | WINDOWED,
		hwnd, pszSrcFile, __LINE__,
		NULL,
		NULL,
		"%s /C %s %c:", GetCmdSpec(FALSE), FileSystem, toupper(*dv));
      }
    }
    break;
  case IDM_FORMAT:
    runemf2(SEPARATE | WINDOWED,
	    hwnd, pszSrcFile, __LINE__, NULL, NULL,
	    "PMFORMAT.EXE %c:", toupper(*dv));
    break;

#if 0					// fixme to be gone?

  case IDM_CLOSETRAY:
    if (driveflags[*dv - 'A'] & DRIVE_CDROM) {

      BYTE parm[4] = { "CD01" };
      ULONGLONG dlen;
      ULONG plen;
      HFILE hfile;
      APIRET rc;

      dv[2] = 0;
      rc = DosOpen(dv,
		   &hfile,
		   &dlen,
		   0,
		   0,
		   OPEN_ACTION_OPEN_IF_EXISTS |
		   OPEN_ACTION_CREATE_IF_NEW,
		   OPEN_FLAGS_DASD |
		   OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE, NULL);
      if (rc)
	Dos_Error(MB_CANCEL, rc, hwnd, pszSrcFile, __LINE__, "DosOpen");
      else {
	dlen = 0;
	plen = sizeof(parm);
	rc = DosDevIOCtl(hfile,
			 0x80,
			 0x45, &parm, sizeof(parm), &plen, NULL, 0, &dlen);
	DosClose(hfile);
	if (rc)
	  Dos_Error(MB_CANCEL, rc, hwnd, pszSrcFile, __LINE__, "DosDevIOCtl");
      }
    }
    break;
#endif // fixme to be gone?

  case IDM_LOCK:
  case IDM_UNLOCK:
  case IDM_EJECT:
    {
      BYTE parm[2];
      ULONG plen = sizeof(parm), dlen = 0L;

      switch (SHORT1FROMMP(cmd)) {
      case IDM_LOCK:
	parm[0] = 1;
	break;
      case IDM_UNLOCK:
	parm[0] = 0;
	break;
      case IDM_EJECT:
	parm[0] = 2;
	break;
      }
      parm[1] = (BYTE) (*dv - 'A');
      DosError(FERR_DISABLEHARDERR);
      DosDevIOCtl(-1L,
		  8L, 0x40L, &parm, sizeof(parm), &plen, NULL, 0L, &dlen);
    }
    break;
  }
}

void CommonCreateMainChildren(HWND hwnd, SWP * swp)
{
  HWND hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
  HWND hwndTmp;
  PFNWP oldproc;

  // Create a children of frame window
  // Left status window
  hwndStatus = WinCreateWindow(hwndFrame,
			       WC_STATUS,
			       "Status",
			       WS_VISIBLE | SS_TEXT | DT_LEFT |
			       DT_VCENTER,
			       swp->x + 4 +
			       WinQuerySysValue(HWND_DESKTOP,
						SV_CXSIZEBORDER),
			       swp->y + 4 +
			       WinQuerySysValue(HWND_DESKTOP,
						SV_CYSIZEBORDER),
			       ((swp->cx / 2) - 8) -
			       (WinQuerySysValue(HWND_DESKTOP,
						 SV_CXSIZEBORDER) * 2),
			       20,
			       hwndFrame, HWND_TOP, MAIN_STATUS, NULL, NULL);
  if (!hwndStatus)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  // Right status window
  hwndStatus2 = WinCreateWindow(hwndFrame,
				WC_STATUS,
				"Status2",
				WS_VISIBLE | SS_TEXT | DT_LEFT |
				DT_VCENTER,
				((swp->cx / 2) + 4) +
				WinQuerySysValue(HWND_DESKTOP,
						 SV_CXSIZEBORDER),
				((swp->cy / 2) + 4) +
				WinQuerySysValue(HWND_DESKTOP,
						 SV_CYSIZEBORDER),
				((swp->cx / 2) - 8) -
				(WinQuerySysValue(HWND_DESKTOP,
						  SV_CXSIZEBORDER) * 2),
				20,
				hwndFrame,
				HWND_TOP, MAIN_STATUS2, NULL, NULL);
  if (!hwndStatus2)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  hwndAttr = WinCreateWindow(hwndFrame,
			     WC_STATUS,
			     "Attr",
			     WS_VISIBLE | SS_TEXT | DT_CENTER |
			     DT_VCENTER,
			     swp->x + 4 +
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CXSIZEBORDER),
			     swp->y + 4 + 24 +
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CYSIZEBORDER),
			     ((swp->cx / 4) - 8) -
			     (WinQuerySysValue(HWND_DESKTOP,
					       SV_CXSIZEBORDER) * 2),
			     20, hwndFrame, HWND_TOP, IDM_ATTRS, NULL, NULL);
  if (!hwndAttr)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  hwndDate = WinCreateWindow(hwndFrame,
			     WC_STATUS,
			     "Date",
			     WS_VISIBLE | SS_TEXT | DT_CENTER |
			     DT_VCENTER,
			     ((swp->x / 4) * 2) + 4 +
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CXSIZEBORDER),
			     swp->y + 4 + 24 +
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CYSIZEBORDER),
			     ((swp->cx / 4) - 8) -
			     (WinQuerySysValue(HWND_DESKTOP,
					       SV_CXSIZEBORDER) * 2),
			     20, hwndFrame, HWND_TOP, IDM_INFO, NULL, NULL);
  if (!hwndDate)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  hwndName = WinCreateWindow(hwndFrame,
			     WC_STATUS,
			     "Name",
			     WS_VISIBLE | SS_TEXT | DT_LEFT |
			     DT_VCENTER,
			     ((swp->x / 4) * 3) + 4 +
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CXSIZEBORDER),
			     swp->y + 4 +
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CYSIZEBORDER),
			     ((swp->cx / 4) - 8) -
			     (WinQuerySysValue(HWND_DESKTOP,
					       SV_CXSIZEBORDER) * 2),
			     20, hwndFrame, HWND_TOP, IDM_RENAME, NULL, NULL);
  if (!hwndName)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  hwndTmp = WinCreateWindow(hwndFrame,
			    WC_TOOLBACK,
			    NullStr,
			    WS_VISIBLE | SS_TEXT | DT_CENTER | DT_VCENTER,
			    swp->x +
			    WinQuerySysValue(HWND_DESKTOP,
					     SV_CXSIZEBORDER),
			    (swp->cy -
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CYSIZEBORDER)) - 30,
			    swp->cx -
			    (WinQuerySysValue(HWND_DESKTOP,
					      SV_CXSIZEBORDER) * 2),
			    30, hwndFrame, HWND_TOP, MAIN_TOOLS, NULL, NULL);
  if (!hwndTmp)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  hwndBack = WinCreateWindow(hwndFrame,
			     WC_DRIVEBACK,
			     NullStr,
			     WS_VISIBLE | SS_TEXT | DT_RIGHT | DT_BOTTOM,
			     swp->x +
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CXSIZEBORDER),
			     (swp->cy -
			      WinQuerySysValue(HWND_DESKTOP,
					       SV_CYSIZEBORDER)) - 30,
			     swp->cx -
			     (WinQuerySysValue(HWND_DESKTOP,
					       SV_CXSIZEBORDER) * 2),
			     30,
			     hwndFrame, HWND_TOP, MAIN_DRIVES, NULL, NULL);

  if (!hwndBack)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  hwndLED = WinCreateWindow(hwndFrame,
			    WC_LED,
			    "#920",
			    WS_VISIBLE | SS_BITMAP,
			    swp->cx - 58,
			    swp->y,
			    12,
			    12, hwndFrame, HWND_TOP, MAIN_LED, NULL, NULL);
  if (!hwndLED)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  hwndLEDHdr = WinCreateWindow(hwndFrame,
			       WC_LED,
			       "0",
			       WS_VISIBLE | SS_TEXT | DT_VCENTER |
			       DT_CENTER,
			       swp->cx - 58,
			       swp->y + 12,
			       12,
			       12,
			       hwndFrame, HWND_TOP, MAIN_LEDHDR, NULL, NULL);
  if (!hwndLEDHdr)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  hwndAutoview = WinCreateWindow(hwndFrame,
				 WC_AUTOVIEW,
				 NullStr,
				 WS_VISIBLE | SS_TEXT | DT_LEFT |
				 DT_TOP | DT_WORDBREAK,
				 swp->x + 4 +
				 WinQuerySysValue(HWND_DESKTOP,
						  SV_CXSIZEBORDER),
				 swp->y + 4 +
				 WinQuerySysValue(HWND_DESKTOP,
						  SV_CYSIZEBORDER) + 16,
				 (swp->cx - 8) -
				 (WinQuerySysValue(HWND_DESKTOP,
						   SV_CXSIZEBORDER) * 2),
				 48,
				 hwndFrame,
				 HWND_TOP, MAIN_AUTOVIEW, NULL, NULL);
  if (!hwndAutoview)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  hwndAutoMLE = WinCreateWindow(hwndFrame,
				// GetPString(IDS_WCAUTOVIEW),
				WC_MLE,
				NullStr,
				WS_VISIBLE | MLS_HSCROLL |
				MLS_VSCROLL | MLS_BORDER,
				swp->x + 4 +
				WinQuerySysValue(HWND_DESKTOP,
						 SV_CXSIZEBORDER),
				swp->y + 4 +
				WinQuerySysValue(HWND_DESKTOP,
						 SV_CYSIZEBORDER) + 16,
				(swp->cx - 8) -
				(WinQuerySysValue(HWND_DESKTOP,
						  SV_CXSIZEBORDER) * 2),
				48,
				hwndFrame,
				HWND_TOP, MAIN_AUTOVIEWMLE, NULL, NULL);
  if (!hwndAutoMLE)
    Win_Error2(hwndFrame, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

  oldproc = WinSubclassWindow(hwndAutoMLE, AutoViewProc);
  WinSetWindowPtr(hwndAutoMLE, QWL_USER, (PVOID) oldproc);
  PostMsg(hwndAutoMLE, UM_SETUP, MPVOID, MPVOID);
}

MRESULT EXPENTRY CommonMainWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2)
{
  switch (msg) {
  case UM_THREADUSE:
    if (hbmLEDon && hbmLEDoff) {
      static LONG threaduse;
      CHAR ts[33];

      if (mp1) {
	threaduse++;
	if (threaduse == 1)
	  WinSendMsg(hwndLED, SM_SETHANDLE, MPFROMLONG(hbmLEDon), MPVOID);
      }
      else {
	threaduse--;
	if (threaduse <= 0) {
	  threaduse = 0;
	  WinSendMsg(hwndLED, SM_SETHANDLE, MPFROMLONG(hbmLEDoff), MPVOID);
	}
      }
      ltoa(threaduse, ts, 10);
      WinSetWindowText(hwndLEDHdr, ts);
      DosSleep(0);
    }
    return 0;

  case UM_LOADFILE:
    {
      CHAR *s = NULL;
      static CHAR lastfile[CCHMAXPATH] = "";

      if (!fAutoView) {
	if (*lastfile)
	  PostMsg((fComments) ? hwndAutoMLE : hwndAutoview,
		  UM_LOADFILE, MPVOID, MPVOID);
	*lastfile = 0;
      }
      else {
	if (mp1) {
	  if (!mp2 && !stricmp(lastfile, (CHAR *)mp1))
	    return 0;
	  strcpy(lastfile, (CHAR *)mp1);
	  s = xstrdup(lastfile, pszSrcFile, __LINE__);
	}
	else
	  *lastfile = 0;
	if (!PostMsg((fComments) ? hwndAutoMLE : hwndAutoview,
		     UM_LOADFILE, MPFROMP(s), MPVOID)) {
	  xfree(s, pszSrcFile, __LINE__);
	}
      }
    }
    return 0;

  case UM_BUILDDRIVEBAR:
    BuildDriveBarButtons(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					 MAIN_DRIVES));
    return 0;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CommonTextButton(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2)
{
  static HWND hwndLast = (HWND) 0;
  static ULONG lastclick = 0;

  switch (msg) {
  case WM_MOUSEMOVE:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (!fNoFinger) {
      WinSetPointer(HWND_DESKTOP, hptrFinger);
      return MRFROMLONG(TRUE);
    }
    break;

  case WM_BUTTON1DOWN:
  case WM_BUTTON3DOWN:
    if (hwndBubble)
      WinDestroyWindow(hwndBubble);
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    WinSetCapture(HWND_DESKTOP, hwnd);
    PaintRecessedWindow(hwnd, (HPS) 0, FALSE, FALSE);
    break;

  case WM_BUTTON2UP:
  case WM_BUTTON2DOWN:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (msg == WM_BUTTON2UP && hwndLast) {
      PostMsg(hwnd, UM_FOCUSME, MPFROMP(hwndLast), MPVOID);
      hwndLast = (HWND) 0;
    }
    break;

  case WM_BUTTON1UP:
  case WM_BUTTON3UP:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    WinSetCapture(HWND_DESKTOP, (HWND) 0);
    PaintRecessedWindow(hwnd, (HPS) 0, TRUE, FALSE);
    {
      ULONG check;

      DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &check, sizeof(check));
      if (check > lastclick + 500) {

	RECTL rcl;
	POINTS pts;

	pts.x = SHORT1FROMMP(mp1);
	pts.y = SHORT2FROMMP(mp1);
	WinQueryWindowRect(hwnd, &rcl);
	if (pts.x > 0 && pts.y > 0 && pts.x < rcl.xRight && pts.y < rcl.yTop)
	  PostMsg(hwnd, UM_CLICKED + (msg == WM_BUTTON3UP), mp1, mp2);
      }
      lastclick = check;
    }
    PostMsg(hwnd, UM_FOCUSME, MPFROMP(hwndLast), MPVOID);
    hwndLast = (HWND) 0;
    break;

  case WM_SETFOCUS:
    if (mp2)
      hwndLast = (HWND) mp1;
    break;

  case UM_FOCUSME:
    if (mp1) {

      PID pid;
      TID tid;

      if (WinIsWindow(WinQueryAnchorBlock(hwnd), (HWND) mp1) &&
	  WinQueryWindowProcess((HWND) mp1, &pid, &tid) && pid == mypid) {
	WinSetFocus(HWND_DESKTOP, (HWND) mp1);
	return 0;
      }
    }
    {
      HWND htemp = (HWND) 0;

      if (hwndMain)
	htemp = TopWindow(hwndMain, (HWND) 0);
      if (!htemp)
	htemp = hwndTree;
      if (htemp)
	WinSetFocus(HWND_DESKTOP, htemp);
      else
	WinSetFocus(HWND_DESKTOP, WinQueryWindow(hwnd, QW_PARENT));
    }
    return 0;
  }

  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CommonCnrProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *dcd = WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg) {
  case UM_FIXCNRMLE:
    if (dcd) {

      SWP swp, swpC;

      if (WinQueryWindowPos(WinWindowFromID(hwnd, CID_MLE), &swp)) {
	if (WinQueryWindowPos(hwnd, &swpC)) {
	  swpC.cx -= (WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL) + 4);
	  swpC.cx -= swp.x;
	  WinSetWindowPos(WinWindowFromID(hwnd, CID_MLE),
			  HWND_TOP, 0, 0, swpC.cx, swp.cy + 2, SWP_SIZE);
	}
      }
      if (mp1)
	WinSendMsg(WinWindowFromID(hwnd, CID_MLE),
		   MLM_SETTEXTLIMIT, mp1, MPVOID);
    }
    return 0;

  case UM_FIXEDITNAME:
    if (dcd) {
      if (mp1 && (INT) mp1 != -1) {

	CHAR *p;

	WinSendMsg(hwnd, UM_FIXCNRMLE, MPFROMLONG(CCHMAXPATH), MPVOID);
	MLEdelete(WinWindowFromID(hwnd, CID_MLE),
		  0, strlen((CHAR *)mp1) + 1);
	WinSetDlgItemText(hwnd, CID_MLE, (CHAR *)mp1);
	p = strrchr((CHAR *)mp1, '\\');
	if (p) {
	  p++;
	  MLEsetcurposa(WinWindowFromID(hwnd, CID_MLE), (p - (CHAR *)mp1));
	  MLEsetcurposc(WinWindowFromID(hwnd, CID_MLE), strlen((CHAR *)mp1));
	}
	else
	  MLEsetcurpos(WinWindowFromID(hwnd, CID_MLE), strlen((CHAR *)mp1));
      }
      else if (mp2) {
	if ((INT) mp1 == -1) {
	  PCNRITEM pci = (PCNRITEM) mp2;
	  RemoveCnrItems(hwnd, pci, 1, CMA_FREE | CMA_INVALIDATE);
	}
	else {
	  Broadcast(WinQueryAnchorBlock(hwnd),
		    dcd->hwndParent, UM_UPDATERECORD, mp2, MPVOID);
	  free(mp2);
	}
      }
    }
    return 0;
  }
  return PFNWPCnr(hwnd, msg, mp1, mp2);
}

HWND OpenDirCnr(HWND hwnd, HWND hwndParent, HWND hwndRestore,
		BOOL noautotile, char *directory)
{
  SWP swp;
  HWND hwndDir;

  if (ParentIsDesktop(hwnd, hwndParent) || *(ULONG *) realappname != FM3UL)
    StartDirCnr(HWND_DESKTOP, directory, hwndRestore, 1);
  else if (*(ULONG *) realappname == FM3UL) {
    if (!fAutoTile)
      GetNextWindowPos(hwndParent, &swp, NULL, NULL);
    hwndDir = StartDirCnr(hwndParent, directory, hwndRestore, 1);
    if (hwndDir) {
      if (!fAutoTile)
	WinSetWindowPos(hwndDir,
			HWND_TOP,
			swp.x,
			swp.y,
			swp.cx,
			swp.cy, SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER);
      else if (!noautotile)
	TileChildren(hwndParent, TRUE);
      WinSetWindowPos(hwndDir,
		      HWND_TOP,
		      0, 0, 0, 0, SWP_RESTORE | SWP_SHOW | SWP_ACTIVATE);
    }
  }
  else {

    char s[256];

    assign_ignores(s);
    runemf2(WINDOWED | SEPARATE,
	    hwnd, pszSrcFile, __LINE__,
	    NULL,
	    NULL,
	    "VDIR.EXE %s%s\"%s%s\"",
	    (*s) ? s : NullStr, (*s) ? " " : NullStr,
	    directory,
	    (directory[strlen(directory) - 1] == '\\') ? "\\" : NullStr);
  }
  return hwndDir;
}

//= IncrThreadUsage() Increment thread usage counter ==

VOID IncrThreadUsage(VOID)
{
  if (hwndMain)
    WinPostMsg(hwndMain, UM_THREADUSE, MPFROMLONG(1), MPVOID);
}

//= DecrThreadUsage() Decrement thread usage counter ==

VOID DecrThreadUsage(VOID)
{
  if (hwndMain)
    WinPostMsg(hwndMain, UM_THREADUSE, MPVOID, MPVOID);
}

#pragma alloc_text(COMMON,CommonFrameWndProc,CommonTextProc,CommonTextPaint)
#pragma alloc_text(COMMON1,CommonCreateTextChildren,CommonCreateMainChildren)
#pragma alloc_text(COMMON2,CommonDriveCmd,CommonTextButton)
#pragma alloc_text(COMMON3,CommonMainWndProc,IncrThreadUsage,DecrThreadUsage)
#pragma alloc_text(COMMON4,CommonCnrProc)
#pragma alloc_text(COMMON5,OpenDirCnr)
