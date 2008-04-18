
/***********************************************************************

  $Id$

  Thread notes window and popup notification status line

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2008 Steven H.Levine

  17 Jul 06 SHL Use Win_Error
  22 Jul 06 SHL Check more run time errors
  30 Mar 07 GKY Remove GetPString for window class names
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Feb 08 GKY Use xfree where appropriate
  16 Apr 08 SHL Comment and clean up logic

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>			// _threadid
#include <process.h>			// _beginthread

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "fm3dll.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static volatile HWND hwndNotify;	// 16 Apr 08 SHL

/**
 * Popup notification message window procedure
 * Display timed message over status line
 */

MRESULT EXPENTRY NotifyWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static ULONG showing = 0;

  switch (msg) {
  case WM_CREATE:
    showing++;
    {
      MRESULT rc = PFNWPStatic(hwnd, msg, mp1, mp2);

      if (!WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_TIMER2, 5000)) {
	Win_Error(hwnd, hwnd, pszSrcFile, __LINE__, "WinStartTimer");
	WinDestroyWindow(hwnd);
      }
      else {

	RGB2 rgb2F, rgb2;

	memset(&rgb2F, 0, sizeof(RGB2));
	rgb2F.bRed = (BYTE)65;
	rgb2.bRed = rgb2.bGreen = rgb2.bBlue = (BYTE)255;
	rgb2.fcOptions = 0;
	SetPresParams(hwnd, &rgb2, &rgb2F, &rgb2, GetPString(IDS_8HELVTEXT));
	if (hwndMain) {
	  if (hwndStatus)
	    WinShowWindow(hwndStatus, FALSE);
	  if (hwndStatus2)
	    WinShowWindow(hwndStatus2, FALSE);
	}
	PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      }
      return rc;
    }

  case UM_SETUP:
    WinSetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER | SWP_SHOW);
    WinInvalidateRect(hwnd, NULL, FALSE);
    return 0;

  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_REPLACEFOCUS, mp1, MPVOID);
    break;

  case UM_REPLACEFOCUS:
    if (mp1)
      WinSetFocus(HWND_DESKTOP, (HWND) mp1);
    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    return 0;

  case WM_PAINT:
    {
      SWP swp;
      POINTL ptl;

      MRESULT mr = PFNWPStatic(hwnd, msg, mp1, mp2);
      HPS hps = WinGetPS(hwnd);
      if (hps) {
	if (WinQueryWindowPos(hwnd, &swp)) {
	  ptl.x = 0;
	  ptl.y = 0;
	  GpiMove(hps, &ptl);
	  GpiSetColor(hps, CLR_RED);
	  ptl.x = swp.cx - 1;
	  ptl.y = swp.cy - 1;
	  GpiBox(hps, DRO_OUTLINE, &ptl, 2, 2);
	}
	WinReleasePS(hwnd);
      }
      return mr;
    }

  case WM_BUTTON1DOWN:
  case WM_BUTTON2DOWN:
  case WM_BUTTON3DOWN:
  case WM_TIMER:
  case WM_CLOSE:
    WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_TIMER2);
    WinDestroyWindow(hwnd);
    return 0;

  case WM_DESTROY:
    showing--;
    if (!showing && hwndMain) {
      if (hwndStatus)
	WinShowWindow(hwndStatus, TRUE);
      if (hwndStatus2)
	WinShowWindow(hwndStatus2, TRUE);
    }
    break;
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

/**
 * Display timed notification window over status line
 */

HWND DoNotify(char *str)
{
  char *p;
  HWND hwnd = (HWND) 0, hwndP;
  LONG x, y, cx, cy;
  SWP swp, swpS, swpS2;
  static USHORT id = NOTE_FRAME;

  if (str && *str) {
    // figure out what size the window should be and where it should be
    hwndP = hwndMain ? WinQueryWindow(hwndMain, QW_PARENT) : HWND_DESKTOP;
    WinQueryWindowPos(hwndP, &swp);
    if (hwndStatus)
      WinQueryWindowPos(hwndStatus, &swpS);
    if (hwndStatus2)
      WinQueryWindowPos(hwndStatus2, &swpS2);
    x = hwndMain ? (hwndStatus ? swpS.x - 1 :
		      WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER)) : 0;
    y = hwndMain ? (hwndStatus ? swpS.y - 1 :
		      WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER)) : 0;
    if (hwndMain && hwndStatus) {
      if (hwndStatus2)
	cx = swpS2.cx + swpS.cx + 8;
      else
	cx = swpS.cx + 6;
    }
    else
      cx = (swp.cx - ((x * 2) + 4));
    cy = hwndMain ? (hwndStatus ? swpS.cy + 2 : 28) : 28;

    // pretty-up the note by putting on a leading space
    if (*str != ' ') {
      p = xmalloc(strlen(str) + 2, pszSrcFile, __LINE__);
      if (!p)
	p = str;
      else {
	strcpy(p + 1, str);
	*p = ' ';
      }
    }
    else
      p = str;

    hwnd = WinCreateWindow(hwndP,
			   WC_ERRORWND,
			   p,
			   SS_TEXT | DT_LEFT | DT_VCENTER | WS_VISIBLE,
			   x, y, cx, cy, hwndP, HWND_TOP, id++, NULL, NULL);
    if (!hwndP)
      Win_Error2(hwndP, hwndP, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);

    if (p != str)
      xfree(p);
    if (id > NOTE_MAX)
      id = NOTE_FRAME;
  }

  AddNote(str);

  return hwnd;
}

/**
 * Add message to thread notes window
 */

HWND Notify(char *str)
{
  return (HWND)WinSendMsg(MainObjectHwnd, UM_NOTIFY, MPFROMP(str), MPVOID);
}

/**
 * Add error message to thread notes window
 */

VOID NotifyError(CHAR * filename, APIRET status)
{
  CHAR errortext[512];

  if (!filename)
    return;
  sprintf(errortext, GetPString(IDS_ERRORACCESSTEXT), status, filename);
  if (toupper(*filename) > 'B') {
    if (status == 21)
      strcat(errortext, GetPString(IDS_EMPTYREMOVETEXT));
    else if (status == 27)
      strcat(errortext, GetPString(IDS_CDMUSICTEXT));
    else if (status == 19)
      strcat(errortext, GetPString(IDS_WRITEPROTECTTEXT));
    else if (status == 108)
      strcat(errortext, GetPString(IDS_DRIVELOCKEDTEXT));
  }
  else {
    if (status == 21)
      strcat(errortext, GetPString(IDS_EMPTYFLOPPYTEXT));
    else if (status == 27)
      strcat(errortext, GetPString(IDS_UNFORMATEDTEXT));
    else if (status == 107)
      sprintf(&errortext[strlen(errortext)],
	      GetPString(IDS_PHANTOMTEXT), toupper(*filename));
    else if (status == 19)
      strcat(errortext, GetPString(IDS_DISKWRITEPROTEXTTEXT));
    else if (status == 108)
      strcat(errortext, GetPString(IDS_DISKLOCKEDTEXT));
  }
  DosBeep(250, 10);
  DosBeep(500, 10);
  DosBeep(250, 10);
  DosBeep(500, 10);
  Notify(errortext);
}

/**
 * Thread notes dialog window dialog procedure
 */

MRESULT EXPENTRY NoteWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static HPOINTER hptrIcon = (HPOINTER) 0;

  switch (msg) {
  case WM_INITDLG:
    if (hwndNotify != (HWND)0) {
      // Already have notes dialog - pass message on
      if (mp2) {
	WinSendDlgItemMsg(hwndNotify,
			  NOTE_LISTBOX,
			  LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), mp2);
	PostMsg(hwndNotify, UM_NOTIFY, MPVOID, MPVOID);
	xfree((CHAR *) mp2);
      }
      WinDismissDlg(hwnd, 0);
      break;
    }
    hwndNotify = hwnd;
    fThreadNotes = FALSE;
    // Remember showing
    {
      BOOL dummy = TRUE;
      PrfWriteProfileData(fmprof,
			  FM3Str, "ThreadNotes", &dummy, sizeof(BOOL));
    }
    if (mp2) {
      WinSendDlgItemMsg(hwnd,
			NOTE_LISTBOX,
			LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0), mp2);
      xfree((CHAR *) mp2);
    }

    {
      // Return focus
      HWND hwndActive = WinQueryActiveWindow(HWND_DESKTOP);
      PostMsg(hwnd, UM_FOCUSME, MPFROMLONG(hwndActive), MPVOID);
    }

    hptrIcon = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, NOTE_FRAME);
    if (hptrIcon)
      WinDefDlgProc(hwnd, WM_SETICON, MPFROMLONG(hptrIcon), MPFROMLONG(0L));
    break;

  case UM_FOCUSME:
    {
      ULONG size = sizeof(SWP),
	fl = SWP_ZORDER | SWP_FOCUSDEACTIVATE | SWP_SHOW;
      SWP swp;

      if (PrfQueryProfileData(fmprof,
			      FM3Str, "NoteWndSwp", (PVOID) & swp, &size)) {
	if (swp.fl & (SWP_HIDE | SWP_MINIMIZE)) {
	  fl |= SWP_MINIMIZE;
	  fl &= (~SWP_SHOW);
	}
	else
	  fl |= (SWP_MOVE | SWP_SIZE);
      }
      WinSetWindowPos(hwnd, HWND_BOTTOM, swp.x, swp.y, swp.cx, swp.cy, fl);
      if (fl & SWP_MINIMIZE) {
	WinSetWindowUShort(hwnd, QWS_XRESTORE, (USHORT) swp.x);
	WinSetWindowUShort(hwnd, QWS_CXRESTORE, (USHORT) swp.cx);
	WinSetWindowUShort(hwnd, QWS_YRESTORE, (USHORT) swp.y);
	WinSetWindowUShort(hwnd, QWS_CYRESTORE, (USHORT) swp.cy);
      }
    }
    if (mp1)
      WinSetActiveWindow(HWND_DESKTOP, (HWND) mp1);
    return 0;

  case UM_STRETCH:
    {
      SWP swp;
      LONG titl, szbx, szby;

      WinQueryWindowPos(hwnd, &swp);
      if (!(swp.fl & (SWP_MINIMIZE | SWP_HIDE))) {
	szbx = SysVal(SV_CXSIZEBORDER);
	szby = SysVal(SV_CYSIZEBORDER);
	titl = SysVal(SV_CYTITLEBAR);
	WinSetWindowPos(WinWindowFromID(hwnd, NOTE_LISTBOX),
			HWND_TOP,
			szbx,
			szby,
			swp.cx - (szbx * 2L),
			(swp.cy - titl) - (szby * 2L), SWP_MOVE | SWP_SIZE);
      }
      if (!(swp.fl & SWP_MAXIMIZE)) {
	if (swp.fl & (SWP_MINIMIZE | SWP_HIDE)) {
	  swp.x = WinQueryWindowUShort(hwnd, QWS_XRESTORE);
	  swp.y = WinQueryWindowUShort(hwnd, QWS_YRESTORE);
	  swp.cx = WinQueryWindowUShort(hwnd, QWS_CXRESTORE);
	  swp.cy = WinQueryWindowUShort(hwnd, QWS_CYRESTORE);
	}
	PrfWriteProfileData(fmprof,
			    FM3Str, "NoteWndSwp", (PVOID) & swp, sizeof(SWP));
      }
    }
    return 0;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
    break;

  case UM_CONTAINER_FILLED:
    {
      SHORT y;
      SHORT x = (SHORT)WinSendDlgItemMsg(hwnd,
					 NOTE_LISTBOX,
					 LM_QUERYITEMCOUNT, MPVOID, MPVOID);
      if (x > 60) {
	for (y = 0; y < x - 50; y++) {
	  WinSendDlgItemMsg(hwnd,
			    NOTE_LISTBOX,
			    LM_DELETEITEM, MPFROMSHORT(y), MPVOID);
	}
      }
    }
    return 0;

  case UM_NOTIFY:
    {
      SHORT x = (SHORT) WinSendDlgItemMsg(hwnd,
					  NOTE_LISTBOX,
					  LM_QUERYITEMCOUNT, MPVOID, MPVOID);
      if (x > 0)
	WinSendDlgItemMsg(hwnd,
			  NOTE_LISTBOX,
			  LM_SETTOPINDEX, MPFROMSHORT(x), MPVOID);
    }
    return 0;

  case UM_SHOWME:
    WinSetWindowPos(hwnd,
		    HWND_TOP,
		    0,
		    0,
		    0, 0, SWP_SHOW | SWP_RESTORE | SWP_ZORDER | SWP_ACTIVATE);
    return 0;

  case WM_COMMAND:
    return 0;

  case WM_CLOSE:
    WinDismissDlg(hwnd, 0);
    return 0;

  case WM_DESTROY:
    if (hwndNotify == hwnd) {
      fThreadNotes = FALSE;
      PrfWriteProfileData(fmprof,
			  FM3Str, "ThreadNotes", &fThreadNotes, sizeof(BOOL));
      hwndNotify = (HWND) 0;
    }
    if (hptrIcon)
      WinDestroyPointer(hptrIcon);
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/**
 * Thread notes dialog window thread
 */

static VOID NoteThread(VOID * args)
{
  HAB hab = WinInitialize(0);

  if (hab) {
    HMQ hmq = WinCreateMsgQueue(hab, 0);
    if (hmq) {
      if (!hwndNotify)
	WinDlgBox(HWND_DESKTOP,
		  HWND_DESKTOP,
		  NoteWndProc, FM3ModHandle, NOTE_FRAME, (CHAR *)args);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
}

/**
 * Start thread notes dialog window thread
 */

VOID StartNotes(CHAR * note)
{
  if (!hwndNotify) {
    if (_beginthread(NoteThread, NULL, 65536, (PVOID) note) == -1)
      Runtime_Error2(pszSrcFile, __LINE__, IDS_COULDNTSTARTTHREADTEXT);
    else {
      USHORT i;
      for (i = 0; !hwndNotify && i < 10; i++)
	DosSleep(10);
      if (!hwndNotify)
	Runtime_Error(pszSrcFile, __LINE__, "Can not create Notify window");
    }
  }
}

/**
 * Add note to thread notes window or popup status window
 */

BOOL AddNote(CHAR * note)
{
  CHAR *s, *p;
  BOOL once = FALSE, ret = FALSE;

  if ((fThreadNotes || hwndNotify) && note && *note) {
    p = note;
    while (*p == ' ')
      p++;
    if (*p) {
      if (!hwndNotify) {
	fThreadNotes = FALSE;
	StartNotes(NULL);
      }
      if (hwndNotify) {
	s = xmalloc(strlen(p) + 14, pszSrcFile, __LINE__);
	if (s) {
	  sprintf(s, "%08lx  %s", _threadid, p);
	  while (!once) {
	    if ((SHORT) WinSendDlgItemMsg(hwndNotify,
					  NOTE_LISTBOX,
					  LM_INSERTITEM,
					  MPFROM2SHORT(LIT_END, 0),
					  MPFROMP(s)) >= 0) {
	      ret = TRUE;
	      PostMsg(hwndNotify, UM_NOTIFY, MPVOID, MPVOID);
	      break;
	    }
	    PostMsg(hwndNotify, UM_CONTAINER_FILLED, MPVOID, MPVOID);
	    once = TRUE;
	  }
	}
	xfree(s);
      }
    }
  }
  return ret;
}

/**
 * Close thread notes window
 */

VOID EndNote(VOID)
{
  if (hwndNotify)
    if (!PostMsg(hwndNotify, WM_CLOSE, MPVOID, MPVOID))
      WinSendMsg(hwndNotify, WM_CLOSE, MPVOID, MPVOID);
}

/**
 * Pop up thread notes window
 */

VOID ShowNote(VOID)
{
  if (!hwndNotify)
    StartNotes(NULL);
  if (hwndNotify)
    PostMsg(hwndNotify, UM_SHOWME, MPVOID, MPVOID);
}

/**
 * Hide thread notes window
 */

VOID HideNote(VOID)
{
  if (hwndNotify)
    WinSetWindowPos(hwndNotify,
		    HWND_BOTTOM,
		    0,
		    0, 0, 0, SWP_MINIMIZE | SWP_ZORDER | SWP_FOCUSDEACTIVATE);
}

#pragma alloc_text(NOTIFY,Notify,NotifyWndProc,StartNotify)
#pragma alloc_text(NOTIFY,NotifyThread,NotifyError)
#pragma alloc_text(NOTIFY2,AddNote,NoteThread,NoteWndProc)
#pragma alloc_text(NOTIFY3,StartNotes,EndNote,HideNote,ShowNote)
