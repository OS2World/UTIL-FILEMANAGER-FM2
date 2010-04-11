/***********************************************************************

  $Id$

  Thread notes window and popup notifications over status line

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2010 Steven H.Levine

  17 Jul 06 SHL Use Win_Error
  22 Jul 06 SHL Check more run time errors
  30 Mar 07 GKY Remove GetPString for window class names
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  16 Apr 08 SHL Comment and clean up logic
  10 Dec 08 SHL Integrate exception handler support
  11 Jan 09 GKY Replace font names in the string file with global set at compile in init.c
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  13 Jul 09 SHL Sync with renames
  16 Jul 09 SHL Stop leaking hptrIcon
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>			// _threadid

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "collect.h"			// Data declaration(s)
#include "grep.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "notify.h"
#include "presparm.h"			// SetPresParams
#include "mainwnd.h"			// Data declaration(s)
#include "wrappers.h"			// xmalloc
#include "misc.h"			// PostMsg
#include "fortify.h"
#include "excputil.h"			// xbeginthread

#pragma data_seg(DATA1)

// Data definitions
static PSZ pszSrcFile = __FILE__;
static volatile HWND hwndNotify;	// 16 Apr 08 SHL
static volatile PCSZ pszCachedNote;	// 16 Jul 09 SHL

#pragma data_seg(GLOBAL1)
BOOL fThreadNotes;

static VOID StartNotes(PCSZ pszNote);

/**
 * Notification message window procedure
 * Displays timed message over status line
 */

MRESULT EXPENTRY NotifyWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static ULONG showing = 0;

  switch (msg) {
  case WM_CREATE:
    showing++;
    {
      MRESULT rc = PFNWPStatic(hwnd, msg, mp1, mp2);

      if (!WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_NOTIFY_TIMER, 5000)) {
	Win_Error(hwnd, hwnd, pszSrcFile, __LINE__, "WinStartTimer");
	WinDestroyWindow(hwnd);
      }
      else {

	RGB2 rgb2F, rgb2;

	memset(&rgb2F, 0, sizeof(RGB2));
	rgb2F.bRed = (BYTE)65;
	rgb2.bRed = rgb2.bGreen = rgb2.bBlue = (BYTE)255;
	rgb2.fcOptions = 0;
	//fixme to allow user to change presparams 1-10-09 GKY
	SetPresParams(hwnd, &rgb2, &rgb2F, &rgb2, FNT_8HELVETICA);
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
    WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_NOTIFY_TIMER);
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
 * Process UM_NOTIFY message to display timed message over status line
 */

HWND DoNotify(PCSZ str)
{
  char *p;
  HWND hwnd = (HWND)0, hwndP;
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
	p = (PSZ)str;
      else {
	strcpy(p + 1, str);
	*p = ' ';
      }
    }
    else
      p = (PSZ)str;

    hwnd = WinCreateWindow(hwndP,
			   (CHAR *) WC_ERRORWND,
			   p,
			   SS_TEXT | DT_LEFT | DT_VCENTER | WS_VISIBLE,
			   x, y, cx, cy, hwndP, HWND_TOP, id++, NULL, NULL);
    if (!hwnd)
      Win_Error(hwndP, hwndP, pszSrcFile, __LINE__,
		PCSZ_WINCREATEWINDOW);

    if (p != str)
      free(p);
    if (id > NOTE_MAX)
      id = NOTE_FRAME;			// Wrap
  }

  AddNote(str);				// Add thread notes window

  return hwnd;
}

/**
 * Display timed notification window over status line
 */

HWND Notify(PCSZ str)
{
  return (HWND)WinSendMsg(MainObjectHwnd, UM_NOTIFY, MPFROMP(str), MPVOID);
}

/**
 * Notify on error
 * Format message and pass to Notify
 */

VOID NotifyError(PCSZ filename, APIRET status)
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
  if (!fErrorBeepOff) {
    DosBeep(250, 10);
    DosBeep(500, 10);
    DosBeep(250, 10);
    DosBeep(500, 10);
  }
  Notify(errortext);
}

/**
 * Thread notes window dialog procedure
 */

MRESULT EXPENTRY NoteWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static HPOINTER hptrIcon;

  switch (msg) {
  case WM_INITDLG:
    if (hwndNotify != (HWND)0) {
      // Already have notes dialog - pass message on
      if (mp2) {
	AddNote((PCSZ)mp2);		// 16 Jul 09 SHL was direct LM_INSERTITEM
	xfree((PSZ)mp2, pszSrcFile, __LINE__);
      }
      WinDismissDlg(hwnd, 0);
      break;
    }
    hwndNotify = hwnd;
    // Remember showing for restart
    fThreadNotes = TRUE;
    PrfWriteProfileData(fmprof,
			FM3Str,
			"ThreadNotes",
			&fThreadNotes,
			sizeof(BOOL));
    fThreadNotes = FALSE;		// Optimize shutdown

    // 16 Jul 09 SHL Added
    if (pszCachedNote) {
      PCSZ p = pszCachedNote;
      pszCachedNote = NULL;
      AddNote(p);
      xfree((PSZ)p, pszSrcFile, __LINE__);
    }

    if (mp2) {
      AddNote((PCSZ)mp2);		// 16 Jul 09 SHL was direct LM_INSERTITEM
      xfree((PSZ)mp2, pszSrcFile, __LINE__);
    }

    // Grab focus
    PostMsg(hwnd,
	    UM_FOCUSME,
	    MPFROMLONG(WinQueryActiveWindow(HWND_DESKTOP)),
	    MPVOID);

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
    if (pszCachedNote)
      DbgMsg(pszSrcFile, __LINE__, "pszCachedNote %p unexpected", pszCachedNote);	// 18 Jul 08 SHL fixme to be Runtime_Error
    else {
      // Cache last item for next open
      SHORT ndx = (SHORT)WinSendDlgItemMsg(hwnd, NOTE_LISTBOX,
					   LM_QUERYITEMCOUNT, MPVOID, MPVOID);
      if (ndx != LIT_NONE) {
	SHORT len;
	ndx--;
	len = (SHORT)WinSendDlgItemMsg(hwnd, NOTE_LISTBOX,
				       LM_QUERYITEMTEXTLENGTH,
				       MPFROMSHORT(ndx), MPVOID);
	if (len != LIT_ERROR) {
	  PSZ p;
	  len++;
	  p = xmalloc(len, pszSrcFile, __LINE__);
	  if (p) {
	    SHORT len2 = (SHORT)WinSendDlgItemMsg(hwnd, NOTE_LISTBOX,
					   LM_QUERYITEMTEXT,
					   MPFROM2SHORT(ndx, len), MPFROMP(p));
	    len--;
	    if (len2 != len) {
	      DbgMsg(pszSrcFile, __LINE__, "len %u unexpected - should be %u", len2, len);	// 18 Jul 08 SHL fixme to be Runtime_Error
	      xfree((PSZ)p, pszSrcFile, __LINE__);
	    }
	    else
	      pszCachedNote = p;
	  }
	}
      }
    }
    WinDismissDlg(hwnd, 0);
    return 0;

  case WM_DESTROY:
    if (hwndNotify == hwnd) {
      fThreadNotes = FALSE;		// Remember not open
      PrfWriteProfileData(fmprof,
			  FM3Str, "ThreadNotes", &fThreadNotes, sizeof(BOOL));
      hwndNotify = (HWND)0;
    }
    if (hptrIcon) {
      WinDestroyPointer(hptrIcon);
      hptrIcon = (HPOINTER)0;		// 16 Jul 09 SHL
    }
    if (!PostMsg((HWND)0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND)0, WM_QUIT, MPVOID, MPVOID);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/**
 * Thread notes dialog window thread
 */

static VOID NoteThread(VOID *args)
{
  HAB hab = WinInitialize(0);
# ifdef FORTIFY
  Fortify_EnterScope();
#  endif
  if (hab) {
    HMQ hmq = WinCreateMsgQueue(hab, 0);
    if (hmq) {
      if (!hwndNotify)
	WinDlgBox(HWND_DESKTOP,
		  HWND_DESKTOP,
		  NoteWndProc, FM3ModHandle, NOTE_FRAME, args);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

/**
 * Start thread notes dialog window thread
 */

static VOID StartNotes(PCSZ note)
{
  if (!hwndNotify) {
    if (xbeginthread(NoteThread,
		     65536,
		     (VOID*)note,
		     pszSrcFile,
		     __LINE__) != -1)
    {
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
 * Open window if was open and first messages after restart
 * Cache last note until window opened
 */

VOID AddNote(PCSZ note)
{
  PSZ s;
  PCSZ p;
  BOOL once = FALSE;

  // Cache last note until window opened
  // 16 Jul 09 SHL fixme to avoid FORTIFY complaints
  if (!fThreadNotes && !hwndNotify && note) {
    p = note + strspn(note, " \t");	// Skip leading white
    if (*p) {
      if (pszCachedNote)
	xfree((PSZ)pszCachedNote, pszSrcFile, __LINE__);
      pszCachedNote = xstrdup(p, pszSrcFile, __LINE__);
    }
    return;
  }

  if ((fThreadNotes || hwndNotify) && note) {
    p = note + strspn(note, " \t");	// Skip leading white
    if (*p) {
      // If have cached note, output it first
      if (pszCachedNote) {
	PCSZ psz = pszCachedNote;
	pszCachedNote = NULL;
	AddNote(psz);
	free((VOID*)psz);
      }
      if (!hwndNotify) {
	fThreadNotes = FALSE;		// 16 Jul 09 SHL fixme to be gone?
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
	      PostMsg(hwndNotify, UM_NOTIFY, MPVOID, MPVOID);
	      break;
	    }
	    PostMsg(hwndNotify, UM_CONTAINER_FILLED, MPVOID, MPVOID);
	    once = TRUE;
	  }
	  free(s);
	}
      }
    }
  }
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

#pragma alloc_text(NOTIFY,Notify,NotifyWndProc,StartNotify,NotifyError)
#pragma alloc_text(NOTIFY2,AddNote,NoteThread,NoteWndProc)
#pragma alloc_text(NOTIFY3,StartNotes,EndNote,HideNote,ShowNote)
