
/***********************************************************************

  $Id$

  About dialogs

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H. Levine

  Revisions
  01 Nov 04 SHL Rename SKULL? defines to avoid rc issues
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat


***********************************************************************/

#define INCL_DOS
#define INCL_WIN

#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "version.h"

#pragma data_seg(DATA1)

MRESULT EXPENTRY AuthorDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SWP swp;
  static HAB hab = 0;
  BOOL no = FALSE;
  static BOOL pause = FALSE;
  static INT width, direction, ticktock, counter = 0;
  static HPOINTER stick1, stick2, stick3, stick4, stick12, stick22, stick32,
    stick42, stick5, stick52;

  switch (msg) {
  case WM_INITDLG:
    if (hab) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    if (mp2)
      pause = TRUE;
    else
      pause = FALSE;
    AboutBox = hwnd;
    hab = WinQueryAnchorBlock(hwnd);
    WinQueryWindowPos(hwnd, &swp);
    width = swp.cx;
    {
      CHAR s[81];

      sprintf(s,
	      "%s %d.%02d%s",
	      GetPString(IDS_VERSIONTEXT), VERMAJOR, VERMINOR, VERREALMINOR);
      WinSetDlgItemText(hwnd, ABT_VERSION, s);
    }
    if (!strcmp(realappname, "FM/4"))
      WinSetDlgItemText(hwnd, ABT_PROGNAME, GetPString(IDS_FM2LITETEXT));
    if (!pause) {
      stick1 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK1);
      stick2 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK2);
      stick3 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK3);
      stick4 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK4);
      stick5 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK5);
      stick12 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK12);
      stick22 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK22);
      stick32 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK32);
      stick42 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK42);
      stick52 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ID_STICK52);
      direction = 2;
      ticktock = 0;
      WinStartTimer(hab, hwnd, ID_TIMER, 164);
      PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    }
    break;

  case UM_SETUP:
    if (counter++ > 1) {

      HPOINTER hbm, sk0, sk1, sk2, sk3, sk4, sk5, sk6, sk7, sk8, sk9;

      hbm = (HPOINTER) WinSendDlgItemMsg(hwnd,
					 ABT_ICON,
					 SM_QUERYHANDLE, MPVOID, MPVOID);
      if (hbm) {
	sk0 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL0_ICON);
	sk1 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL1_ICON);
	sk2 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL2_ICON);
	sk3 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL3_ICON);
	sk4 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL4_ICON);
	sk5 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL5_ICON);
	sk6 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL6_ICON);
	sk7 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL7_ICON);
	sk8 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL8_ICON);
	sk9 = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SKULL9_ICON);
	if (sk0 && sk1 && sk2 && sk3 && sk4 && sk5 && sk6 && sk7 &&
	    sk8 && sk9) {
	  WinShowWindow(WinWindowFromID(hwnd, ABT_ICON), FALSE);
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk0), MPVOID);
	  WinShowWindow(WinWindowFromID(hwnd, ABT_ICON), TRUE);
	  DosSleep(16);
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk1), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk2), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk3), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk4), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk5), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk6), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk7), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk8), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk9), MPVOID);
	  DosSleep(100);//05 Aug 07 GKY 257
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk8), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk7), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk6), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk5), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk4), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk3), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk2), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk1), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(sk0), MPVOID);
	  DosSleep(16);//05 Aug 07 GKY 33
	  WinShowWindow(WinWindowFromID(hwnd, ABT_ICON), FALSE);
	  WinSendDlgItemMsg(hwnd, ABT_ICON, SM_SETHANDLE,
			    MPFROMLONG(hbm), MPVOID);
	  WinShowWindow(WinWindowFromID(hwnd, ABT_ICON), TRUE);
	}
	if (sk0)
	  WinDestroyPointer(sk0);
	if (sk1)
	  WinDestroyPointer(sk1);
	if (sk2)
	  WinDestroyPointer(sk2);
	if (sk3)
	  WinDestroyPointer(sk3);
	if (sk4)
	  WinDestroyPointer(sk4);
	if (sk5)
	  WinDestroyPointer(sk5);
	if (sk6)
	  WinDestroyPointer(sk6);
	if (sk7)
	  WinDestroyPointer(sk7);
	if (sk8)
	  WinDestroyPointer(sk8);
	if (sk9)
	  WinDestroyPointer(sk9);
      }
    }
    if (counter > 4)
      WinDlgBox(HWND_DESKTOP,
		hwnd, AuthorDlgProc, FM3ModHandle, AUTHOR_FRAME, NULL);
    return 0;

  case UM_SETDIR:
    WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);
    DosSleep(250);//05 Aug 07 GKY 2500
    return 0;

  case WM_TIMER:
    WinQueryWindowPos(WinWindowFromID(hwnd, ABT_STICK1), &swp);
    if (swp.x + (swp.cx * 3) > width - 3) {
      direction = -2;
      WinShowWindow(WinWindowFromID(hwnd, ABT_STICK1), FALSE);
      WinSendDlgItemMsg(hwnd,
			ABT_STICK1, SM_SETHANDLE, MPFROMLONG(stick3), MPVOID);
      WinShowWindow(WinWindowFromID(hwnd, ABT_STICK1), TRUE);
      WinShowWindow(WinWindowFromID(hwnd, ABT_STICK2), FALSE);
      WinSendDlgItemMsg(hwnd,
			ABT_STICK2,
			SM_SETHANDLE, MPFROMLONG(stick32), MPVOID);
      WinShowWindow(WinWindowFromID(hwnd, ABT_STICK2), TRUE);
      no = TRUE;
    }
    else if (swp.x < 3) {
      direction = 2;
      WinShowWindow(WinWindowFromID(hwnd, ABT_STICK2), FALSE);
      WinSendDlgItemMsg(hwnd, ABT_STICK2,
			SM_SETHANDLE, MPFROMLONG(stick32), MPVOID);
      WinShowWindow(WinWindowFromID(hwnd, ABT_STICK2), TRUE);
      WinShowWindow(WinWindowFromID(hwnd, ABT_STICK1), FALSE);
      WinSendDlgItemMsg(hwnd,
			ABT_STICK1, SM_SETHANDLE, MPFROMLONG(stick3), MPVOID);
      WinShowWindow(WinWindowFromID(hwnd, ABT_STICK1), TRUE);
      no = TRUE;
    }
    swp.x += direction;
    WinSetWindowPos(WinWindowFromID(hwnd, ABT_STICK1),
		    HWND_TOP, swp.x, swp.y, swp.cx, swp.cy, SWP_MOVE);
    WinSetWindowPos(WinWindowFromID(hwnd, ABT_ICON),
		    HWND_TOP,
		    swp.x + swp.cx, swp.y, swp.cx, swp.cy, SWP_MOVE);
    WinSetWindowPos(WinWindowFromID(hwnd, ABT_STICK2),
		    HWND_TOP,
		    swp.x + (swp.cx * 2), swp.y, swp.cx, swp.cy, SWP_MOVE);
    if (!no) {
      if (direction > 0) {
	WinShowWindow(WinWindowFromID(hwnd, ABT_STICK1), FALSE);
	WinSendDlgItemMsg(hwnd,
			  ABT_STICK1,
			  SM_SETHANDLE,
			  MPFROMLONG((ticktock) ? stick1 : stick2), MPVOID);
	WinShowWindow(WinWindowFromID(hwnd, ABT_STICK1), TRUE);
	WinShowWindow(WinWindowFromID(hwnd, ABT_STICK2), FALSE);
	WinSendDlgItemMsg(hwnd,
			  ABT_STICK2,
			  SM_SETHANDLE,
			  MPFROMLONG((ticktock) ? stick42 : stick52), MPVOID);
	WinShowWindow(WinWindowFromID(hwnd, ABT_STICK2), TRUE);
      }
      else {
	WinShowWindow(WinWindowFromID(hwnd, ABT_STICK2), FALSE);
	WinSendDlgItemMsg(hwnd,
			  ABT_STICK2,
			  SM_SETHANDLE,
			  MPFROMLONG((ticktock) ? stick12 : stick22), MPVOID);
	WinShowWindow(WinWindowFromID(hwnd, ABT_STICK2), TRUE);
	WinShowWindow(WinWindowFromID(hwnd, ABT_STICK1), FALSE);
	WinSendDlgItemMsg(hwnd,
			  ABT_STICK1,
			  SM_SETHANDLE,
			  MPFROMLONG((ticktock) ? stick4 : stick5), MPVOID);
	WinShowWindow(WinWindowFromID(hwnd, ABT_STICK1), TRUE);
      }
    }
    ticktock = (ticktock) ? 0 : 1;
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case ABT_VERSION:
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    return 0;

  case WM_DESTROY:
    if (hab) {
      if (!pause) {
	WinStopTimer(hab, hwnd, ID_TIMER);
	WinDestroyPointer(stick1);
	WinDestroyPointer(stick2);
	WinDestroyPointer(stick3);
	WinDestroyPointer(stick4);
	WinDestroyPointer(stick5);
	WinDestroyPointer(stick12);
	WinDestroyPointer(stick22);
	WinDestroyPointer(stick32);
	WinDestroyPointer(stick42);
	WinDestroyPointer(stick52);
      }
      hab = 0;
    }
    AboutBox = (HWND) 0;
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(ABOUT,AboutDlgProc,AuthorDlgProc)
