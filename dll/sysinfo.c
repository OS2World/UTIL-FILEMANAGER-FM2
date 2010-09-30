
/***********************************************************************

  $Id$

  System Info Display

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2002, 2008 Steven H.Levine

  16 Oct 02 SHL Baseline
  08 Feb 03 SHL Enable display
  01 Aug 04 SHL RunRmview: avoid buffer overflow
  26 Jul 06 SHL Report open errors
  29 Jul 06 SHL Use xfgets
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  16 JUL 08 GKY Use TMP directory for temp files
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "sysinfo.h"
#include "printer.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "pathutil.h"                   // BldFullPathName
#include "copyf.h"			// unlinkf
#include "wrappers.h"			// xfgets
#include "systemf.h"			// runemf2
#include "misc.h"			// PostMsg
#include "strips.h"			// bstrip

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

VOID RunRmview(VOID * arg)
{
  HWND hwnd = (HWND) arg;
  CHAR s[2048], *p, szTempFile[CCHMAXPATH];
  HAB thab;
  HMQ thmq;
  FILE *fp;
  HFILE oldstdout, newstdout;
  CHAR *mode = "w";

  DosError(FERR_DISABLEHARDERR);

  thab = WinInitialize(0);
  thmq = WinCreateMsgQueue(thab, 0);
  WinCancelShutdown(thmq, TRUE);
  if (thab && thmq) {
    if (!WinIsWindow(thab, hwnd))
      goto Abort;
    BldFullPathName(szTempFile, pTmpDir, "$RMVIEW.#$#");
    unlinkf(szTempFile);
    fp = xfopen(szTempFile, mode, pszSrcFile, __LINE__, FALSE);
    if (!fp)
      goto Abort;
    else {
      newstdout = -1;
      if (DosDupHandle(fileno(stdout), &newstdout)) {
	fclose(fp);
	goto Abort;
      }
      oldstdout = fileno(stdout);
      DosDupHandle(fileno(fp), &oldstdout);
      runemf2(SEPARATE | INVISIBLE | FULLSCREEN | BACKGROUND | WAIT,
              hwnd, pszSrcFile, __LINE__,
              NULL, NULL, "%s", szTempFile);
      oldstdout = fileno(stdout);
      DosDupHandle(newstdout, &oldstdout);
      DosClose(newstdout);
      fclose(fp);
    }
    if (!WinIsWindow(thab, hwnd))
      goto Abort;
    mode = "r";
    fp = xfopen(szTempFile, mode, pszSrcFile, __LINE__, FALSE);
    if (fp) {
      xfgets(s, sizeof(s), fp, pszSrcFile, __LINE__);
      xfgets(s, sizeof(s), fp, pszSrcFile, __LINE__);
      if (!feof(fp) && WinIsWindow(thab, hwnd))
	WinSendDlgItemMsg(hwnd, SYS_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0),
			  MPFROMP(" -= RMView Physical Info =-"));
      while (!feof(fp)) {
	strset(s, 0);
	if (!xfgets(s, sizeof(s), fp, pszSrcFile, __LINE__))
	  break;
	stripcr(s);
	rstrip(s);
	p = s;
	while (*p == '\r' || *p == '\n')
	  p++;
	if (!WinIsWindow(thab, hwnd))
	  break;
	WinSendDlgItemMsg(hwnd, SYS_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0), MPFROMP(p));
      }
      fclose(fp);
    }
  Abort:
    WinDestroyMsgQueue(thmq);
    WinTerminate(thab);
  }
  if (szTempFile)
    DosForceDelete(szTempFile);
}

MRESULT EXPENTRY SysInfoDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static HWND me = (HWND) 0;
  static LONG ypos = 0;
  static HPOINTER hptrIcon = (HPOINTER) 0;

  switch (msg) {
  case WM_INITDLG:
    if (me) {
      WinSetWindowPos(me, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE |
		      SWP_SHOW | SWP_RESTORE | SWP_ZORDER);
      WinDismissDlg(hwnd, 0);
      break;
    }
    hptrIcon = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, SYS_FRAME);
    WinDefDlgProc(hwnd, WM_SETICON, MPFROMLONG(hptrIcon), MPVOID);
    {
      static CHAR *names[] = { "Max. Path Length",	/* 1  */
	"Max. Text Sessions",	/* 2  */
	"Max. PM Sessions",	/* 3  */
	"Max. VDM Sessions",	/* 4  */
	"Boot Drive",		/* 5  */
	"Dynamic Priority",	/* 6  */
	"Max. Wait",		/* 7  */
	"Min. Timeslice",	/* 8  */
	"Max. Timeslice",	/* 9  */
	"Page Size",		/* 10 */
	"Version Major",	/* 11 */
	"Version Minor",	/* 12 */
	"Version Revision",	/* 13 */
	"Millisecs Up",		/* 14 */
	"Unixtime Low",		/* 15 */
	"Unixtime High",	/* 16 */
	"Physical Memory",	/* 17 */
	"Resident Memory",	/* 18 */
	"Total Available Memory",	/* 19 */
	"Max. Private Memory",	/* 20 */
	"Max. Shared Memory",	/* 21 */
	"Timer Interval",	/* 22 */
	"Max. Path Comp. Length",	/* 23 */
	"Foreground FS Session ID",	/* 24 */
	"Foreground Process PID",	/* 25 */
	NULL
      };
      static CHAR *pnames[] = { "Swap button",
	"Dblclk time",
	"CX dblclk",
	"CY dblclk",
	"CX sizeborder",
	"CY sizeborder",
	"Alarm",
	"",
	"",
	"Cursor rate",
	"First scroll rate",
	"Scroll rate",
	"Numbered lists",
	"Warning freq",
	"Note freq",
	"Error freq",
	"Warning duration",
	"Note duration",
	"Error duration",
	"",
	"CX screen",
	"CY screen",
	"CX vscroll",
	"CY hscroll",
	"CY vscroll arrow",
	"CX hscroll arrow",
	"CX border",
	"CY border",
	"CX dlgframe",
	"CY dlgframe",
	"CY titlebar",
	"CY vslider",
	"CX hslider",
	"CX minmaxbutton",
	"CY minmaxbutton",
	"CY menu",
	"CX fullscreen",
	"CY fullscreen",
	"CX icon",
	"CY icon",
	"CX pointer",
	"CY pointer",
	"Debug",
	"# Mouse buttons",
	"Pointer level",
	"Cursor level",
	"Track rect level",
	"# timers",
	"Mouse present",
	"CX bytealign",
	"CY bytealign",
	"",
	"",
	"",
	"",
	"",
	"Not reserved",
	"Extra key beep",
	"Set lights",
	"Insert mode",
	"",
	"",
	"",
	"",
	"Menu rolldown delay",
	"Menu rollup delay",
	"Alt mnemonic",
	"Tasklist mouse access",
	"CX icon text width",
	"# Icon text lines",
	"Chord time",
	"CX chord",
	"CY chord",
	"CX motion",
	"CY motion",
	"Begin drag",
	"End drag",
	"Single select",
	"Open",
	"Context menu",
	"Context help",
	"Text edit",
	"Begin select",
	"End select",
	"Begin drag kb",
	"End drag kb",
	"Select kb",
	"Open kb",
	"Context menu kb",
	"Context help kb",
	"Text edit kb",
	"Begin select kb",
	"End select kb",
	"Animation",
	"Animation speed",
	"Mono icons",
	"Kbd id",
	"Print screen",
	NULL
      };
      static CHAR *dnames[] = { "# printers",
	"# RS232 ports",
	"# diskette drives",
	"Coprocessor present",
	"PC submodel",
	"PC model",
	"Display",
	NULL
      };
      ULONG vals[26], val, x;
      CHAR s[134], dev;

      if (DosQuerySysInfo(QSV_MAX_PATH_LENGTH, QSV_MAX_COMP_LENGTH + 2,
			  (PVOID) vals, (ULONG) sizeof(vals))) {
	WinDismissDlg(hwnd, 0);
	break;
      }
      me = hwnd;
      WinSendDlgItemMsg(hwnd, SYS_LISTBOX, LM_INSERTITEM,
			MPFROMLONG(LIT_END), MPFROMP(" -= Base OS Info =-"));
      for (x = 0; names[x]; x++) {
	switch (x) {
	case 0:
	case 9:
	case 22:
	  sprintf(s, "%-28.28s%lu bytes", names[x], vals[x]);
	  break;
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	  sprintf(s, "%-28.28s%lu bytes (%lu mb)", names[x], vals[x],
		  vals[x] / (1024 * 1024));
	  break;
	case 4:
	  sprintf(s, "%-28.28s%c:", names[x], (CHAR) vals[x] + '@');
	  break;
	case 5:
	  sprintf(s, "%-28.28s%s", names[x], (vals[x]) ? "On" : "Off");
	  break;
	case 6:
	  sprintf(s, "%-28.28s%lu seconds", names[x], vals[x]);
	  break;
	case 7:
	case 8:
	  sprintf(s, "%-28.28s%lu milliseconds", names[x], vals[x]);
	  break;
	case 13:
	  {
	    ULONG numdays, nummins;

	    sprintf(s, "%-28.28s%lu (", names[x], vals[x]);
	    vals[x] /= 60000;
	    numdays = vals[x] / (60 * 24);
	    if (numdays)
	      sprintf(s + strlen(s), "%lu day%s, ", numdays,
		      &"s"[numdays == 1]);
	    nummins = vals[x] % (60 * 24);
	    sprintf(s + strlen(s), "%luh:%02lum)", nummins / 60,
		    nummins % 60);
	  }
	  break;
	case 21:
	  sprintf(s, "%-28.28s%lu.%01lu milliseconds", names[x],
		  vals[x] / 10, vals[x] % 10);
	  break;
	default:
	  sprintf(s, "%-28.28s%lu", names[x], vals[x]);
	  break;
	}
	if (*s)
	  WinSendDlgItemMsg(hwnd, SYS_LISTBOX, LM_INSERTITEM,
			    MPFROMLONG(LIT_END), MPFROMP(s));
      }
      WinSendDlgItemMsg(hwnd, SYS_LISTBOX, LM_INSERTITEM,
			MPFROMLONG(LIT_END), MPFROMP(" -= PM Info =-"));
      for (x = 0; pnames[x]; x++) {
	if (*pnames[x]) {
	  val = WinQuerySysValue(HWND_DESKTOP, x);
	  switch (x) {
	  case SV_CXBYTEALIGN:
	  case SV_CYBYTEALIGN:
	  case SV_CYMOTIONSTART:
	  case SV_CXMOTIONSTART:
	  case SV_CYDBLCLK:
	  case SV_CXDBLCLK:
	  case SV_CYPOINTER:
	  case SV_CXPOINTER:
	  case SV_CYICON:
	  case SV_CXICON:
	  case SV_CXFULLSCREEN:
	  case SV_CYFULLSCREEN:
	  case SV_CYMENU:
	  case SV_CYMINMAXBUTTON:
	  case SV_CXMINMAXBUTTON:
	  case SV_CXHSLIDER:
	  case SV_CYVSLIDER:
	  case SV_CXDLGFRAME:
	  case SV_CYDLGFRAME:
	  case SV_CXSIZEBORDER:
	  case SV_CYSIZEBORDER:
	  case SV_CXBORDER:
	  case SV_CYBORDER:
	  case SV_CYTITLEBAR:
	  case SV_CXHSCROLLARROW:
	  case SV_CYVSCROLLARROW:
	  case SV_CXVSCROLL:
	  case SV_CYHSCROLL:
	  case SV_CYSCREEN:
	  case SV_CXSCREEN:
	  case SV_CXICONTEXTWIDTH:
	  case SV_CXCHORD:
	  case SV_CYCHORD:
	    sprintf(s, "%-28.28s%lu pels", pnames[x], val);
	    break;
	  case SV_CONTEXTMENU:
	  case SV_OPEN:
	  case SV_TEXTEDIT:
	  case SV_BEGINSELECT:
	  case SV_ENDSELECT:
	  case SV_BEGINDRAG:
	  case SV_ENDDRAG:
	  case SV_CONTEXTHELP:
	  case SV_SINGLESELECT:
	    sprintf(s, "%-28.28sWM: %04xh KC: %04xh", pnames[x],
		    LOUSHORT(val), HIUSHORT(val));
	    break;
	  case SV_TASKLISTMOUSEACCESS:
	    sprintf(s, "%-28.28sWM: %04xh", pnames[x], LOUSHORT(val));
	    break;
	  case SV_CONTEXTMENUKB:
	  case SV_TEXTEDITKB:
	  case SV_BEGINDRAGKB:
	  case SV_ENDDRAGKB:
	  case SV_SELECTKB:
	  case SV_OPENKB:
	  case SV_CONTEXTHELPKB:
	  case SV_BEGINSELECTKB:
	  case SV_ENDSELECTKB:
	    sprintf(s, "%-28.28sVK: %04xh KC: %04xh", pnames[x],
		    LOUSHORT(val), HIUSHORT(val));
	    break;
	  case SV_CHORDTIME:
	  case SV_DBLCLKTIME:
	  case SV_CURSORRATE:
	  case SV_FIRSTSCROLLRATE:
	  case SV_SCROLLRATE:
	  case SV_MENUROLLDOWNDELAY:
	  case SV_MENUROLLUPDELAY:
	  case SV_ANIMATIONSPEED:
	  case SV_WARNINGDURATION:
	  case SV_NOTEDURATION:
	  case SV_ERRORDURATION:
	    sprintf(s, "%-28.28s%lu milliseconds", pnames[x], val);
	    break;
	  case SV_MOUSEPRESENT:
	    sprintf(s, "%-28.28s%s", pnames[x], (val) ? "True" : "False");
	    break;
	  case SV_ALARM:
	  case SV_ANIMATION:
	  case SV_MONOICONS:
	  case SV_PRINTSCREEN:
	  case SV_SETLIGHTS:
	  case SV_INSERTMODE:
	  case SV_SWAPBUTTON:
	  case SV_DEBUG:
	    sprintf(s, "%-28.28s%s", pnames[x], (val) ? "On" : "Off");
	    break;
	  default:
	    sprintf(s, "%-28.28s%lu", pnames[x], val);
	    break;
	  }
	  WinSendDlgItemMsg(hwnd, SYS_LISTBOX, LM_INSERTITEM,
			    MPFROMLONG(LIT_END), MPFROMP(s));
	}
      }
      WinSendDlgItemMsg(hwnd, SYS_LISTBOX, LM_INSERTITEM,
			MPFROMLONG(LIT_END), MPFROMP(" -= Hardware info =-"));
      for (x = 0; dnames[x]; x++) {
	if (*dnames[x]) {
	  if (!DosDevConfig(&dev, x)) {
	    switch (x) {
	    case DEVINFO_COPROCESSOR:
	      sprintf(s, "%-28.28s%s", dnames[x], (dev) ? "True" : "False");
	      break;
	    case DEVINFO_ADAPTER:
	      sprintf(s, "%-28.28s%sMonochrome or printer", dnames[x],
		      (dev) ? "Not " : NullStr);
	      break;
	    default:
	      sprintf(s, "%-28.28s%lu", dnames[x], dev);
	      break;
	    }
	  }
	  WinSendDlgItemMsg(hwnd, SYS_LISTBOX, LM_INSERTITEM,
			    MPFROMLONG(LIT_END), MPFROMP(s));
	}
      }
    }
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
    break;

  case UM_STRETCH:
    {
      SWP swpC, swp;

      WinQueryWindowPos(hwnd, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
	WinQueryWindowPos(WinWindowFromID(hwnd, SYS_LISTBOX), &swpC);
	if (!ypos)
	  ypos = swpC.y;
	WinSetWindowPos(WinWindowFromID(hwnd, SYS_LISTBOX), HWND_TOP,
			SysVal(SV_CXSIZEBORDER),
			ypos,
			swp.cx - (SysVal(SV_CXSIZEBORDER) * 2),
			(swp.cy - ypos) - (SysVal(SV_CYTITLEBAR) +
					   SysVal(SV_CYSIZEBORDER)),
			SWP_MOVE | SWP_SIZE);
      }
    }
    return 0;

  case WM_COMMAND:
    WinDismissDlg(hwnd, 0);
    return 0;

  case WM_DESTROY:
    if (me == hwnd) {
      me = (HWND) 0;
      if (hptrIcon)
	WinDestroyPointer(hptrIcon);
      hptrIcon = (HPOINTER) 0;
    }
    break;
  }

  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(SYSINFO,SysInfoDlgProc,RunRmview)
