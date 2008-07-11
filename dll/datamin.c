
/***********************************************************************

  $Id$

  Minimized data bar

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2006 Steven H. Levine

  14 Sep 02 SHL Handle large partitions
  16 Oct 02 SHL Handle large partitions better
  23 May 05 SHL Use QWL_USER
  23 May 05 SHL Avoid delays for inaccessible drives
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  06 Jun 05 SHL Drop unused code
  22 Jul 06 SHL Check more run time errors
  02 Jan 07 GKY Changed drive information string formating to accomodate 6 char FS names
  07 Jan 07 GKY Move error strings etc. to string file
  30 Mar 07 GKY Remove GetPString for window class names
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  02 Sep 07 GKY Replaced DosQProcStatus with DosQuerySysState to fix trap in thunk code

***********************************************************************/

#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <process.h>			// _beginthread

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG

#include "fm3dlg.h"
#include "fm3str.h"
#include "procstat.h"
#include "datamin.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "fm3dll.h"
#include "fortify.h"

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

APIRET16 APIENTRY16 Dos16MemAvail(PULONG pulAvailMem);

static volatile HEV G_hevDataMin = NULLHANDLE;
static volatile HWND G_hwndSingle = NULLHANDLE;

static VOID dataminThread(VOID * pv);

long MINI_X = 208, MINI_Y = 16;

//=== MiniTimeProc - time, swap and drive status mini windows procedure ===

MRESULT EXPENTRY MiniTimeProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  APIRET rc;

  switch (msg) {
  case WM_CREATE:
    {
      PVOID pv = xmalloc(sizeof(tDataMin), pszSrcFile, __LINE__);
      if (pv)
        WinSetWindowPtr(hwnd, QWL_DATAMIN_PTR, pv);
    }
    break;

  case WM_BUTTON1CLICK:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);

      if (id >= MINI_DRIVEA) {
	if (G_hevDataMin != NULLHANDLE) {
	  G_hwndSingle = hwnd;
	  rc = DosPostEventSem(G_hevDataMin);
	  if (rc) {
              Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
                        GetPString(IDS_POSTSEMFAILED));
	  }
	}
      }
      else if (id == MINI_TIME)
	PostMsg(WinQueryWindow(hwnd, QW_PARENT), UM_SETUP6,	// Up time
		MPVOID, MPVOID);
      else if (id == MINI_PROC)
	WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
		   WM_SYSCOMMAND, MPFROM2SHORT(SC_TASKMANAGER, 0), MPVOID);
    }
    break;

  case WM_BUTTON1DBLCLK:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);

      if (id >= MINI_DRIVEA && !hwndMain) {
	CHAR s[] = " :\\";

	*s = (CHAR) (id - MINI_DRIVEA) + 'A';
	OpenDirCnr((HWND) 0, HWND_DESKTOP, (HWND) 0, FALSE, s);
	return MRFROMLONG(1L);
      }
      else if (id == MINI_TIME) {
	OpenObject("<WP_CLOCK>",
		   (SHORT2FROMMP(mp2) & KC_SHIFT) ? Default : Settings, hwnd);
	return MRFROMLONG(1L);
      }

#ifdef NEVER
      else if (id == MINI_MEM) {
	WinDlgBox(HWND_DESKTOP,
		  HWND_DESKTOP,
		  SysInfoDlgProc, FM3ModHandle, SYS_FRAME, NULL);
	return MRFROMLONG(1L);
      }
#endif

      else if (id == MINI_PROC || id == MINI_MEM) {
	WinDlgBox(HWND_DESKTOP,
		  hwnd, KillDlgProc, FM3ModHandle, KILL_FRAME, NULL);
	return MRFROMLONG(1L);
      }
      else if (id == MINI_SWAP && *SwapperDat) {

	char s[5];

	strncpy(s, SwapperDat, 4);
	s[3] = 0;
	WinDlgBox(HWND_DESKTOP,
		  hwndMain,
		  UndeleteDlgProc, FM3ModHandle, UNDEL_FRAME, MPFROMP(s));
	return MRFROMLONG(1L);
      }
    }
    break;

  case WM_BUTTON1MOTIONSTART:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT),
	    UM_BUTTON1MOTIONSTART, MPVOID, MPVOID);
    break;

  case WM_CONTEXTMENU:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT), UM_CONTEXTMENU, MPVOID, MPVOID);
    break;

  case WM_PAINT:
    {
      MRESULT mr = 0;
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      if (id >= MINI_DRIVEA) {
	HPS hps = WinBeginPaint(hwnd, (HPS) 0, NULL);

	if (hps) {
	  mr = WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
			  UM_PAINT, MPFROM2SHORT(id, 0), MPFROMLONG(hps));
	  WinEndPaint(hps);
	  return mr;			// Bypass default paint
	}
      }
    }
    break;

  case WM_DESTROY:
    {
      PVOID pv = WinQueryWindowPtr(hwnd, QWL_DATAMIN_PTR);

      xfree(pv, pszSrcFile, __LINE__);
    }
    break;
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);

}					// MiniTimeProc

//=== DataProc - databar client window procedure ===

MRESULT EXPENTRY DataProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  APIRET rc;

  static ULONG counter;
  static BOOL NoFloat, noqproc = FALSE, Positioned;
  static HWND hwndMenu = (HWND) 0;

  switch (msg) {
  case WM_CREATE:
    if (DataHwnd) {
      WinSetWindowPos(DataHwnd, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER | SWP_SHOW);
      return MRFROMLONG(1L);
    }
    DataHwnd = WinQueryWindow(hwnd, QW_PARENT);
    NoFloat = FALSE;
    Positioned = FALSE;
    SetPresParams(hwnd,
		  &RGBGREY, &RGBBLACK, &RGBBLACK, GetPString(IDS_8HELVTEXT));
    {
      int c;
      long x = 3;
      USHORT ids[] = { MINI_TIME, MINI_MEM, MINI_SWAP, MINI_PROC, 0 };
      POINTL aptl[TXTBOX_COUNT];
      HPS hps;

      hps = WinGetPS(hwnd);
      if (hps) {
	GpiQueryTextBox(hps,
			34,
			"  -=03:08:22  SMW  1998/08/02=-  ",
			TXTBOX_COUNT, aptl);
	WinReleasePS(hps);
	MINI_X = aptl[TXTBOX_TOPRIGHT].x + 6;
	MINI_Y = aptl[TXTBOX_TOPRIGHT].y + 6;
      }
      for (c = 0; ids[c]; c++) {
	if (!WinCreateWindow(hwnd,
			     WC_MINITIME,
			     NullStr,
			     SS_TEXT | DT_CENTER | DT_VCENTER | WS_VISIBLE,
			     x,
			     3,
			     MINI_X,
			     MINI_Y, hwnd, HWND_TOP, ids[c], NULL, NULL)) {
	  Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
	}
	x += (MINI_X + 4);
      }
    }
    if (!hwndMain) {
      SWCNTRL swctl;

      memset(&swctl, 0, sizeof(swctl));
      swctl.hwnd = WinQueryWindow(hwnd, QW_PARENT);
      swctl.uchVisibility = SWL_VISIBLE;
      swctl.fbJump = (fDataToFore) ? SWL_NOTJUMPABLE : SWL_JUMPABLE;
      swctl.bProgType = PROG_PM;
      strcpy(swctl.szSwtitle, GetPString(IDS_DATABARTITLETEXT));
      WinCreateSwitchEntry(WinQueryAnchorBlock(hwnd), &swctl);
    }
    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    return 0;

  case WM_MENUEND:
    NoFloat = FALSE;
    if (hwndMenu == (HWND) mp2) {
      WinDestroyWindow(hwndMenu);
      hwndMenu = (HWND) 0;
    }
    break;

  case UM_RESTORE:
    WinSetWindowPtr(hwnd, QWL_USER, mp1);
    return 0;

  case UM_SETUP:
    {
      long x, y;
      SWP swp, swpD;
      int c;
      ULONG size = sizeof(SWP);
      ULONG numdrives = 0;
      ULONG drivestyle = (DRIVE_REMOVABLE | DRIVE_INVALID |
			  DRIVE_IGNORE | DRIVE_ZIPSTREAM | DRIVE_NOSTATS);
      ULONG ulDriveNum, ulDriveMap;

      if (!fDataInclRemote)
	drivestyle |= DRIVE_REMOTE || DRIVE_VIRTUAL || DRIVE_RAMDISK;
      if (fDataShowDrives) {
	DosError(FERR_DISABLEHARDERR);
	DosQCurDisk(&ulDriveNum, &ulDriveMap);
	x = 3;
	y = MINI_Y + 4;
	// Drive status windows
	for (c = 2; c < 26; c++) {
	  if ((ulDriveMap & (1L << c)) && !(driveflags[c] & drivestyle)) {
	    if (!WinCreateWindow(hwnd,
				 WC_MINITIME,
				 NullStr,
				 SS_TEXT | DT_CENTER | DT_VCENTER |
				 WS_VISIBLE, x, y, MINI_X, MINI_Y, hwnd,
				 HWND_TOP, MINI_DRIVEA + c, NULL, NULL)) {
	      Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__,
			 IDS_WINCREATEWINDOW);
	    }
	    numdrives++;
	    x += (MINI_X + 4);
	    if ((numdrives % 4) == 0) {
	      y += (MINI_Y + 4);
	      x = 3;
	    }
	  }
	}
      }
      x = (MINI_X * 4) + 18;
      y = (MINI_Y + 4) + ((numdrives / 4) * (MINI_Y + 4)) +
	(((numdrives % 4) != 0) * (MINI_Y + 4));
      if (!Positioned) {
	if (PrfQueryProfileData(fmprof, appname, "DataMinPos", &swp, &size)) {
	  WinQueryWindowPos(HWND_DESKTOP, &swpD);
	  if (swp.x > swpD.cx - 16)
	    swp.x = swpD.cx - 16;
	  if (swp.y > swpD.cy - 16)
	    swp.y = swpD.cy - 16;
	  WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT),
			  HWND_TOP,
			  swp.x,
			  swp.y,
			  x, y, SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ZORDER);
	}
	else
	  WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT),
			  HWND_TOP,
			  0,
			  0,
			  x, y, SWP_SHOW | SWP_SIZE | SWP_MOVE | SWP_ZORDER);
	Positioned = TRUE;
      }
      else
	WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT),
			HWND_TOP,
			0, 0, x, y, SWP_SHOW | SWP_SIZE | SWP_ZORDER);
      WinShowWindow(WinQueryWindow(hwnd, QW_PARENT), TRUE);
      if (numdrives) {
	if (_beginthread(dataminThread, NULL, 32768, (PVOID) hwnd) == -1) {
	  Runtime_Error(pszSrcFile, __LINE__,
			GetPString(IDS_COULDNTSTARTTHREADTEXT));
	  PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	}
      }
      counter = 0;
      PostMsg(hwnd, UM_TIMER, MPVOID, MPVOID);
    }
    return 0;				// UM_SETUP

  case WM_BUTTON1DBLCLK:
    if (hwndMain)
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    break;

  case UM_CONTEXTMENU:
  case WM_CONTEXTMENU:
    if (!hwndMenu)
      hwndMenu = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, MINI_FRAME);
    if (hwndMenu) {
      WinCheckMenuItem(hwndMenu, MINI_FLOAT, fDataToFore);
      WinCheckMenuItem(hwndMenu, MINI_SHOW, fDataShowDrives);
      WinCheckMenuItem(hwndMenu, MINI_BORING, fDullMin);
      WinCheckMenuItem(hwndMenu, MINI_INCLREMOTE, fDataInclRemote);
      NoFloat = TRUE;
      if (!PopupMenu(hwnd, hwnd, hwndMenu))
	NoFloat = FALSE;
    }
    if (msg == UM_CONTEXTMENU)
      return 0;
    break;

  case WM_BUTTON2DBLCLK:
    if (!(SHORT2FROMMP(mp2) & KC_SHIFT)) {
      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(MINI_FLOAT, 0), MPVOID);
      break;
    }
    /* else intentional fallthru */
  case WM_CHORD:
  case WM_BUTTON3DBLCLK:
    PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(MINI_SHOW, 0), MPVOID);
    break;

  case UM_BUTTON1MOTIONSTART:
  case WM_BUTTON1MOTIONSTART:
    {
      TRACKINFO TrackInfo;
      SWP Position;

      memset(&TrackInfo, 0, sizeof(TrackInfo));
      TrackInfo.cxBorder = 1;
      TrackInfo.cyBorder = 1;
      TrackInfo.cxGrid = 1;
      TrackInfo.cyGrid = 1;
      TrackInfo.cxKeyboard = 8;
      TrackInfo.cyKeyboard = 8;
      WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &Position);
      TrackInfo.rclTrack.xLeft = Position.x;
      TrackInfo.rclTrack.xRight = Position.x + Position.cx;
      TrackInfo.rclTrack.yBottom = Position.y;
      TrackInfo.rclTrack.yTop = Position.y + Position.cy;
      WinQueryWindowPos(HWND_DESKTOP, &Position);
      TrackInfo.rclBoundary.xLeft = Position.x;
      TrackInfo.rclBoundary.xRight = Position.x + Position.cx;
      TrackInfo.rclBoundary.yBottom = Position.y;
      TrackInfo.rclBoundary.yTop = Position.y + Position.cy;
      TrackInfo.ptlMinTrackSize.x = 0;
      TrackInfo.ptlMinTrackSize.y = 0;
      TrackInfo.ptlMaxTrackSize.x = Position.cx;
      TrackInfo.ptlMaxTrackSize.y = Position.cy;
      TrackInfo.fs = TF_MOVE | TF_STANDARD | TF_ALLINBOUNDARY;
      if (WinTrackRect(HWND_DESKTOP, (HPS) 0, &TrackInfo)) {
	WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT),
			HWND_TOP, TrackInfo.rclTrack.xLeft,
			TrackInfo.rclTrack.yBottom, 0, 0, SWP_MOVE);
	WinSendMsg(hwnd, WM_SAVEAPPLICATION, MPVOID, MPVOID);
      }
    }
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_HELP, 0), MPVOID);
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_COMMANDLINE:
    case IDM_DOSCOMMANDLINE:
    case IDM_WINFULLSCREEN:
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
        runemf2(type, hwnd, pszSrcFile, __LINE__,
                path, NULL, "%s", env);
      }
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_DATABAR, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case MINI_CLOSE:
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      break;

    case MINI_BORING:
      fDullMin = (fDullMin) ? FALSE : TRUE;
      PrfWriteProfileData(fmprof,
			  FM3Str, "DullDatabar", &fDullMin, sizeof(BOOL));
      if (G_hevDataMin != NULLHANDLE) {
	rc = DosPostEventSem(G_hevDataMin);
	if (rc) {
            Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
                      GetPString(IDS_POSTSEMFAILED));
	}
      }

      break;

    case MINI_INCLREMOTE:
    case MINI_SHOW:
      if (SHORT1FROMMP(mp1) == MINI_SHOW) {
	fDataShowDrives = (fDataShowDrives) ? FALSE : TRUE;
	PrfWriteProfileData(fmprof,
			    appname,
			    "DataShowDrives", &fDataShowDrives, sizeof(BOOL));
      }
      else {
	fDataInclRemote = (fDataInclRemote) ? FALSE : TRUE;
	PrfWriteProfileData(fmprof,
			    appname,
			    "DataInclRemote", &fDataInclRemote, sizeof(BOOL));
      }
      {
	HENUM henum;
	HWND hwndChild;
	USHORT id;

	henum = WinBeginEnumWindows(hwnd);
	while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
	  id = WinQueryWindowUShort(hwndChild, QWS_ID);
	  if (id >= MINI_DRIVEA)
	    WinDestroyWindow(hwndChild);
	}
	WinEndEnumWindows(henum);
      }
      PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      break;

    case MINI_FLOAT:
      fDataToFore = (fDataToFore) ? FALSE : TRUE;
      PrfWriteProfileData(fmprof,
			  appname, "DataToFore", &fDataToFore, sizeof(BOOL));
      if (!hwndMain) {

	SWCNTRL swcntrl;
	HSWITCH hswitch;

	hswitch = (HSWITCH) WinQuerySwitchHandle(hwnd, (PID) 0);
	if (hswitch) {
	  memset(&swcntrl, 0, sizeof(SWCNTRL));
	  if (!WinQuerySwitchEntry(hswitch, &swcntrl)) {
	    swcntrl.fbJump = (fDataToFore) ? SWL_NOTJUMPABLE : SWL_JUMPABLE;
	    WinChangeSwitchEntry(hswitch, &swcntrl);
	  }
	}
      }
      break;
    }
    return 0;

  case WM_SIZE:
    WinSetWindowPos(hwnd,
		    HWND_TOP,
		    0,
		    0,
		    SHORT1FROMMP(mp2),
		    SHORT2FROMMP(mp2), SWP_MOVE | SWP_SIZE);
    break;

  case WM_PAINT:
    {
      HPS hps;
      POINTL ptl;
      SWP swp;
      RECTL rcl;

      hps = WinBeginPaint(hwnd, (HPS) 0, &rcl);
      if (hps) {
	WinFillRect(hps, (PRECTL) & rcl, CLR_PALEGRAY);
	GpiSetMix(hps, FM_OVERPAINT);
	GpiSetColor(hps, CLR_WHITE);
	WinQueryWindowPos(hwnd, &swp);
	ptl.x = 0;
	ptl.y = 0;
	GpiMove(hps, &ptl);
	ptl.y = swp.cy - 1;
	GpiLine(hps, &ptl);
	ptl.x = swp.cx - 1;
	GpiLine(hps, &ptl);
	GpiSetColor(hps, CLR_DARKGRAY);
	ptl.y = 0;
	GpiLine(hps, &ptl);
	ptl.x = 0;
	GpiLine(hps, &ptl);
	{
	  HENUM henum;
	  HWND hwndTemp;

	  henum = WinBeginEnumWindows(hwnd);
	  while ((hwndTemp = WinGetNextWindow(henum)) != NULLHANDLE) {
	    PaintRecessedWindow(hwndTemp,
				hps, (WinQueryWindowUShort(hwndTemp, QWS_ID)
				      != MINI_TIME), FALSE);
	  }
	  WinEndEnumWindows(henum);
	}
	WinEndPaint(hps);
      }
    }
    return 0;

  case UM_PAINT:
    {
      CHAR s[90];
      CHAR szFreeQty[38];
      CHAR szDrvLtr[] = " :";
      CHAR *pszFSystem;
      ULONGLONG ullFreeQty;
      ULONG ulPercentFree;
      ULONG wasx;
      HPS hps = (HPS) mp2;
      HWND hwndChild;
      USHORT id;
      SWP swp;
      POINTL ptl;
      tDataMin *pDM;

      id = SHORT1FROMMP(mp1);
      if (id >= MINI_DRIVEA) {
	hwndChild = WinWindowFromID(hwnd, id);
	if (!hwndChild)
	  return 0;
	if (!WinQueryWindowPos(hwndChild, &swp))
	  return 0;
	pDM = WinQueryWindowPtr(hwndChild, QWL_DATAMIN_PTR);
	if (!pDM || pDM->qfsi_rc) {
	  ullFreeQty = 0;
	  ulPercentFree = 0;
	}
	else {
	  ullFreeQty = (ULONGLONG) pDM->fsa.cUnitAvail *
	    (pDM->fsa.cSectorUnit * pDM->fsa.cbSector);

	  ulPercentFree = (pDM->fsa.cUnit && pDM->fsa.cUnitAvail) ?
	    (pDM->fsa.cUnitAvail * 100) / pDM->fsa.cUnit : 0;
	}

	CommaFmtULL(szFreeQty, sizeof(szFreeQty), ullFreeQty, ' ');
	*szDrvLtr = (CHAR) (id - MINI_DRIVEA) + 'A';

	if (!pDM || pDM->qfsi_rc || pDM->qfsa_rc)
	  pszFSystem = "N/A";
	else {
	  pszFSystem = (PCHAR)(pDM->fsqb2.szName) + pDM->fsqb2.cbName + 1;
	  pszFSystem[15] = 0;
	}
	sprintf(s,
		"%s %13s %lu%%-%s %6s ",
		szDrvLtr,
		szFreeQty,
		ulPercentFree, GetPString(IDS_FREETEXT), pszFSystem);
	if (!hps)
	  hps = WinGetPS(hwndChild);
	if (hps) {
	  if (!fDullMin) {
	    ptl.x = 0;
	    ptl.y = 0;
	    GpiMove(hps, &ptl);
	    GpiSetColor(hps, CLR_BLACK);
	    ptl.x = swp.cx - 1;
	    ptl.y = swp.cy - 1;
	    GpiBox(hps, DRO_OUTLINE, &ptl, 0, 0);
	    ptl.x = 1;
	    ptl.y = 1;
	    if (ulPercentFree) {
	      GpiMove(hps, &ptl);
	      GpiSetColor(hps,
			  (ulPercentFree < 11) ? CLR_DARKRED :
			  (ulPercentFree < 26) ? CLR_DARKBLUE :
			  CLR_DARKGREEN);
	      ptl.y = swp.cy - 2;
	      ptl.x = ((swp.cx - 2) * ulPercentFree) / 100;
	      wasx = ptl.x;
	      GpiBox(hps, DRO_OUTLINEFILL, &ptl, 0, 0);
	      GpiSetColor(hps,
			  (ulPercentFree < 11) ? CLR_RED :
			  (ulPercentFree < 26) ? CLR_BLUE : CLR_GREEN);
	      ptl.x = wasx;
	      ptl.y = swp.cy - 2;
	      GpiMove(hps, &ptl);
	      ptl.x = 1;
	      GpiLine(hps, &ptl);
	      ptl.y = 2;
	      ptl.x = 1;
	      GpiLine(hps, &ptl);
	      ptl.x = wasx;
	    }
	    if (ulPercentFree < 99) {
	      GpiSetColor(hps, CLR_DARKGRAY);
	      wasx = ptl.x;
	      ptl.y = 2;
	      GpiMove(hps, &ptl);
	      ptl.y = swp.cy - 2;
	      ptl.x = swp.cx - 2;
	      GpiBox(hps, DRO_OUTLINEFILL, &ptl, 0, 0);
	      ptl.x = wasx;
	      GpiMove(hps, &ptl);
	      GpiSetColor(hps, CLR_PALEGRAY);
	      ptl.x = swp.cx - 3;
	      GpiLine(hps, &ptl);
	      ptl.x = wasx;
	      ptl.y = 1;
	      GpiMove(hps, &ptl);
	      GpiSetColor(hps, CLR_BLACK);
	      ptl.x = swp.cx - 2;
	      GpiLine(hps, &ptl);
	      ptl.y = swp.cy - 3;
	      GpiLine(hps, &ptl);
	    }
	    GpiSetColor(hps, CLR_WHITE);
	  }
	  else {
	    GpiSetColor(hps, CLR_PALEGRAY);
	    ptl.x = 0;
	    ptl.y = 0;
	    GpiMove(hps, &ptl);
	    ptl.x = swp.cx - 1;
	    ptl.y = swp.cy - 1;
	    GpiBox(hps, DRO_OUTLINEFILL, &ptl, 0, 0);
	    GpiSetColor(hps,
			(ulPercentFree < 11) ? CLR_DARKRED : CLR_DARKBLUE);
	  }
	  GpiSetBackMix(hps, BM_LEAVEALONE);
	  GpiSetMix(hps, FM_OVERPAINT);
	  {
	    POINTL aptl[TXTBOX_COUNT];

	    GpiQueryTextBox(hps, strlen(s), s, TXTBOX_COUNT, aptl);
	    ptl.y = ((swp.cy / 2) -
		     ((aptl[TXTBOX_TOPRIGHT].y +
		       aptl[TXTBOX_BOTTOMLEFT].y) / 2));
	    ptl.y++;
	    ptl.x = (swp.cx / 2) - (aptl[TXTBOX_TOPRIGHT].x / 2);
	    if (ptl.x < 2)
	      ptl.x = 2;
	    GpiCharStringAt(hps, &ptl, strlen(s), s);
	  }
	  if (!mp2)
	    WinReleasePS(hps);
	}
      }					// if drive window
    }
    return 0;

  case UM_TIMER:
    {
      CHAR s[134];
      DATETIME dt;

      if (fDataToFore && !NoFloat)
	WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT),
			HWND_TOP, 0, 0, 0, 0, SWP_ZORDER);
      if (counter && (counter % 19) && (counter % 20)) {
	if (!DosGetDateTime(&dt)) {
	  sprintf(s,
		  " %02hu:%02hu:%02hu  %s %04u/%02u/%02u",
		  dt.hours,
		  dt.minutes,
		  dt.seconds,
		  GetPString(IDS_SUNDAY + dt.weekday),
		  dt.year, dt.month, dt.day);
	  WinSetDlgItemText(hwnd, MINI_TIME, s);
	}
      }
      else if (!counter || !(counter % 19))
	PostMsg(hwnd, UM_SETUP6, MPVOID, MPVOID);	// Uptime
      if (!(counter % 4)) {
	PostMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);	// Memory utilization
	if (!(counter % 10)) {
	  PostMsg(hwnd, UM_SETUP5, MPVOID, MPVOID);	// Process status
	  if (!(counter % 20)) {
	    PostMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);	// Swapper
	  }
	}
      }
    }
    counter++;
    return 0;

  case UM_SETUP2:
    {
      CHAR s[134], szFileQty[38], szFreeQty[38];
      FILEFINDBUF3L ffb;
      ULONG nm = 1;
      ULONGLONG ullFreeQty;
      HDIR hdir = HDIR_CREATE;
      FSALLOCATE fsa;

      if (*SwapperDat) {
	DosError(FERR_DISABLEHARDERR);
	if (!xDosFindFirst(SwapperDat, &hdir, FILE_NORMAL | FILE_HIDDEN |
			   FILE_SYSTEM | FILE_ARCHIVED | FILE_READONLY,
			   &ffb, sizeof(ffb), &nm, FIL_STANDARDL)) {
	  priority_bumped();
	  DosFindClose(hdir);
	  DosError(FERR_DISABLEHARDERR);
	  if (!DosQueryFSInfo(toupper(*SwapperDat) - '@', FSIL_ALLOC,
			      &fsa, sizeof(FSALLOCATE))) {
	    ullFreeQty =
	      (ULONGLONG) fsa.cUnitAvail * (fsa.cSectorUnit * fsa.cbSector);
	    CommaFmtULL(szFreeQty, sizeof(szFreeQty), ullFreeQty, ' ');
	  }
	  else
	    *szFreeQty = 0;

	  CommaFmtULL(szFileQty, sizeof(szFileQty), ffb.cbFile, ' ');
	  sprintf(s, " %s %s%s%s",
		  GetPString(IDS_SWAPTITLETEXT),
		  szFileQty, *szFreeQty ? "/" : NullStr, szFreeQty);
	  WinSetDlgItemText(hwnd, MINI_SWAP, s);
	}
      }
    }
    return 0;

  case UM_SETUP3:			// Memory utilization
    {
      CHAR s[134], tm[38], szQty[38];
      ULONG amem = 0;

      if (!DosQuerySysInfo(QSV_TOTAVAILMEM, QSV_TOTAVAILMEM,
			   (PVOID) & amem, (ULONG) sizeof(amem))) {
	CommaFmtUL(tm, sizeof(tm), amem, 'M');
	if (!Dos16MemAvail(&amem))
	  CommaFmtUL(szQty, sizeof(szQty), amem, 'M');
	else
	  *szQty = 0;
	sprintf(s, " %s%s%s%s",
		GetPString(IDS_MEMTITLETEXT),
		szQty, (*szQty) ? "/" : NullStr, tm);
	WinSetDlgItemText(hwnd, MINI_MEM, s);
      }
    }
    return 0;

  case UM_SETUP5:			// Process status
    {
     CHAR s[134], tm[38], szQty[38];

     if (fUseQProcStat && !noqproc) {

	PROCESSINFO *ppi;
	BUFFHEADER *pbh = NULL;
	MODINFO *pmi;
	ULONG numprocs = 0, numthreads = 0;
	APIRET rc;

	rc = DosAllocMem((PVOID)&pbh, USHRT_MAX + 4096,
			 PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
	if (rc)
	  Dos_Error(MB_CANCEL, rc, hwnd, pszSrcFile, __LINE__,
		    GetPString(IDS_OUTOFMEMORY));
	else {
	  if (DosQProcStatus((ULONG *)pbh, USHRT_MAX))
	    noqproc = TRUE;
	  else {
	    ppi = pbh->ppi;
	    while (ppi->ulEndIndicator != PROCESS_END_INDICATOR) {
	      pmi = pbh->pmi;
	      while (pmi && ppi->hModRef != pmi->hMod)
		pmi = pmi->pNext;
	      if (pmi) {
		numprocs++;
		numthreads += ppi->usThreadCount;
	      }
	      ppi = (PPROCESSINFO) (ppi->ptiFirst + ppi->usThreadCount);
	    }
	    commafmt(szQty, sizeof(szQty), numprocs);
	    commafmt(tm, sizeof(tm), numthreads);
	    sprintf(s,
		    " %s%s  %s%s",
		    GetPString(IDS_PROCSTITLETEXT),
		    szQty, GetPString(IDS_THRDSTITLETEXT), tm);
	    WinSetDlgItemText(hwnd, MINI_PROC, s);
	  }
	  DosFreeMem(pbh);
	}
      }
      else if (fUseQSysState && !noqproc) {

	QSPREC *ppi;
	QSPTRREC *pbh = NULL;
	QSLREC *pmi;
	ULONG numprocs = 0, numthreads = 0;
	APIRET rc;

	rc = DosAllocMem((PVOID) & pbh, USHRT_MAX + 4096,
			 PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
	if (rc)
	  Dos_Error(MB_CANCEL, rc, hwnd, pszSrcFile, __LINE__,
		    GetPString(IDS_OUTOFMEMORY));
	else { //2 Sep 07 GKY 0x05 = process & Mod data only
	  if (DosQuerySysState(QS_PROCESS | QS_MTE, 0, 0, 0, pbh, USHRT_MAX))
	    noqproc = TRUE;
	  else {
	    ppi = pbh->pProcRec;
	    while (ppi->RecType == 1) {
	      pmi = pbh->pLibRec;
	      while (pmi && ppi->hMte != pmi->hmte)
		pmi = pmi->pNextRec;
	      if (pmi) {
		numprocs++;
		numthreads += ppi->cTCB;
	      }
	      ppi = (QSPREC *) (ppi->pThrdRec + ppi->cTCB);
	    }
	    commafmt(szQty, sizeof(szQty), numprocs);
	    commafmt(tm, sizeof(tm), numthreads);
	    sprintf(s,
		    " %s%s  %s%s",
		    GetPString(IDS_PROCSTITLETEXT),
		    szQty, GetPString(IDS_THRDSTITLETEXT), tm);
	    WinSetDlgItemText(hwnd, MINI_PROC, s);
	  }
	  DosFreeMem(pbh);
	}
      }
      else {
	commafmt(szQty, sizeof(szQty),
		 WinQuerySwitchList(WinQueryAnchorBlock(hwnd), (PSWBLOCK) 0,
				    0));
	sprintf(s, " %s%s", GetPString(IDS_TASKSTITLETEXT), szQty);
	WinSetDlgItemText(hwnd, MINI_PROC, s);
      }
    }
    return 0;

  case UM_SETUP6:			// Uptime
    {
      ULONG val = 0, numdays, nummins;
      CHAR s[128];

      if (!DosQuerySysInfo(QSV_MS_COUNT,
			   QSV_MS_COUNT,
			   (PVOID) & val, (ULONG) sizeof(val))) {
	val /= 60000L;
	numdays = val / (60L * 24L);
	strcpy(s, GetPString(IDS_ELAPSEDTITLETEXT));
	if (numdays)
	  sprintf(s + strlen(s),
		  " %lu %s%s, ",
		  numdays, GetPString(IDS_DAYTEXT), &"s"[numdays == 1L]);
	nummins = val % (60L * 24L);
	sprintf(s + strlen(s), " %lu:%02lu", nummins / 60, nummins % 60);
	WinSetDlgItemText(hwnd, MINI_TIME, s);
      }
    }
    return 0;

  case WM_SAVEAPPLICATION:
    {
      SWP swp;

      WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &swp);
      PrfWriteProfileData(fmprof, appname, "DataMinPos", &swp, sizeof(SWP));
    }
    break;

  case WM_CLOSE:
    WinSendMsg(hwnd, WM_SAVEAPPLICATION, MPVOID, MPVOID);
    WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
    return 0;

  case WM_DESTROY:
    if (DataHwnd == WinQueryWindow(hwnd, QW_PARENT)) {
      DataHwnd = (HWND) 0;
      if (hwndMenu)
	WinDestroyWindow(hwndMenu);
      hwndMenu = (HWND) 0;
    }
    if (hwndMain) {

      SWP swp;
      ULONG fl = SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE, ofl;

      ofl = WinQueryWindowULong(hwnd, QWL_USER);
      WinQueryWindowPos(WinQueryWindow(hwndMain, QW_PARENT), &swp);
      if (swp.fl & SWP_MINIMIZE)
	fl |= ((ofl & SWP_MAXIMIZE) ? SWP_MAXIMIZE : SWP_RESTORE);
      WinSetWindowPos(WinQueryWindow(hwndMain, QW_PARENT),
		      HWND_TOP, 0, 0, 0, 0, fl);
    }
    else if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);

}					// DataProc

//=== CreateDataBar - create databar windows ===

HWND CreateDataBar(HWND hwndParent, ULONG fl)
{
  HWND hwndClient = (HWND) 0;
  ULONG FrameFlags = 0;

  if (WinCreateStdWindow(hwndParent,
			 WS_VISIBLE,
			 &FrameFlags,
			 WC_DATABAR,
			 NULL, WS_VISIBLE, 0, MINI_FRAME, &hwndClient)) {
    WinSendMsg(hwndClient, UM_RESTORE, MPFROMLONG(fl), MPVOID);
  }
  return hwndClient;

}					// CreateDataBar

//=== dataminThread - drive status thread ===

static VOID dataminThread(VOID * pv)
{
  HAB hab = NULLHANDLE;
  HMQ hmq = NULLHANDLE;
  HWND hwndParent = (HWND) pv;
  HWND hwnd;
  HENUM henum;
  BOOL busy = TRUE;
  APIRET rc;
  USHORT id;

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif
  if (G_hevDataMin == NULLHANDLE) {
    // Create just once for any thread that might use it
    // Kernel will clean up on exit
    rc = DosCreateEventSem(NULL, (PHEV) & G_hevDataMin, 0L, FALSE);
    if (rc) {
        Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
                  GetPString(IDS_CREATESEMFAILED));
      busy = FALSE;
    }
  }

  // fixme to report errors
  hab = WinInitialize(0);
  if (hab == NULLHANDLE)
    busy = FALSE;
  else {
    hmq = WinCreateMsgQueue(hab, 0);
    if (hmq == NULLHANDLE)
      busy = FALSE;
    else
      WinCancelShutdown(hmq, TRUE);
  }

  while (busy) {
    HWND hwndSingle = G_hwndSingle;

    G_hwndSingle = NULLHANDLE;

    busy = FALSE;

    if (!WinIsWindow(hab, hwndParent))
      break;

    henum = WinBeginEnumWindows(hwndParent);
    while (henum && (hwnd = WinGetNextWindow(henum)) != NULLHANDLE) {
      if (!WinIsWindow(hab, hwnd))
	continue;
      if (hwndSingle && hwndSingle != hwnd)
	continue;
      id = WinQueryWindowUShort(hwnd, QWS_ID);
      if (id > MINI_DRIVEA) {
	ULONG dskNum = id - MINI_DRIVEA + 1;
	tDataMin *pDM = WinQueryWindowPtr(hwnd, QWL_DATAMIN_PTR);
	SWP swp;
	CHAR szPath[] = " :";

	if (!pDM)
	  continue;
	busy = TRUE;
	if (!WinQueryWindowPos(hwnd, &swp))
	  continue;

	DosError(FERR_DISABLEHARDERR);
	pDM->qfsi_rc = DosQueryFSInfo(dskNum,
				      FSIL_ALLOC,
				      &pDM->fsa, sizeof(FSALLOCATE));

	if (!pDM->qfsi_rc) {
	  *szPath = (CHAR) dskNum + 'A' - 1;
	  pDM->qfsa_cb = sizeof(FSQBUFFER2) + 256;	// se tDataMin
	  DosError(FERR_DISABLEHARDERR);
	  pDM->qfsa_rc = DosQueryFSAttach(szPath, 0,	/* Ordinal */
					  FSAIL_QUERYNAME,
					  &pDM->fsqb2, &pDM->qfsa_cb);
	}
	WinInvalidateRect(hwnd, NULL, FALSE);
      }					// if drive window
    }					// while henum
    WinEndEnumWindows(henum);

    if (busy) {
      ULONG clPosted;

      rc = DosWaitEventSem(G_hevDataMin, 20000L);
      if (rc && rc != ERROR_TIMEOUT) {
          Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
                    GetPString(IDS_POSTSEMFAILED));
      }

      rc = DosResetEventSem(G_hevDataMin, &clPosted);
      if (rc && rc != ERROR_ALREADY_RESET) {
          Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
                    GetPString(IDS_POSTSEMFAILED));
      }
    }

  }					// while

  if (hmq != NULLHANDLE)
    WinDestroyMsgQueue(hmq);

  if (hab != NULLHANDLE)
    WinTerminate(hab);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
}					// dataminThread

#pragma alloc_text(DATAMIN,DataDlgProc,MiniTimeProc)
