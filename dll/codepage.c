
/***********************************************************************

  $Id$

  Select code page

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2010 Steven H.Levine

  14 Jul 06 SHL Use Runtime_Error
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  11 Jan 09 GKY Moved codepage names to a character array here from the string file.
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.

***********************************************************************/

#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "notebook.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "strutil.h"			// GetPString
#include "errutil.h"			// Runtime_Error
#include "codepage.h"
#include "misc.h"			// PostMsg

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;
static CHAR *CodePage[23] = {"437 USA",
"850 Multilingual",
"852 Latin 2",
"857 Turkish",
"860 Portuguese",
"861 Iceland",
"863 French-Canadian",
"865 Nordic",
"866 Russian OS/2",
"878 Russian KOI8-R",
"932 Japan",
"934 Korea",
"936 China",
"938 Taiwan",
"942 Japan SAA",
"944 Korea SAA",
"946 China SAA",
"948 Taiwan SAA",
"949 Korea KS",
"950 Taiwan (Big 5)",
"1004 DTP/Win",
"1251 Russian Win",
"1381 China GB"};

MRESULT EXPENTRY PickCodePageDlgBox(HWND hwnd, ULONG msg, MPARAM mp1,
				    MPARAM mp2)
{

  SHORT sSelect;

  switch (msg) {
  case WM_INITDLG:
    WinShowWindow(WinWindowFromID(hwnd, PICK_SAVEPOS), FALSE);
    {
      ULONG cp;
      char *p;

      cp = WinQueryCp(WinQueryWindowULong(hwnd, QWL_HMQ));
      for (sSelect = 0; sSelect < 23; sSelect++) {
        p = CodePage[sSelect];
	WinSendDlgItemMsg(hwnd,
			  PICK_LISTBOX,
			  LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(p));
	if (atoi(p) == cp) {
	  WinSendDlgItemMsg(hwnd,
			    PICK_LISTBOX,
			    LM_SETTOPINDEX, MPFROM2SHORT(sSelect, 0), MPVOID);
	  WinSendDlgItemMsg(hwnd,
			    PICK_LISTBOX,
			    LM_SELECTITEM,
			    MPFROM2SHORT(sSelect, 0), MPFROMSHORT(TRUE));
	}
      }
    }
    WinSendDlgItemMsg(hwnd,
		      PICK_INPUT,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(256, 0), MPVOID);
    WinSetWindowText(hwnd, (CHAR *) GetPString(IDS_PICKCODEPAGETEXT));
    PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
    break;

  case UM_STRETCH:
    {
      SWP swp, swpL, swpE;
      LONG titl, szbx, szby;

      WinQueryWindowPos(hwnd, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE)) &&
	  (swp.x != SHORT1FROMMP(mp1) || swp.cx != SHORT2FROMMP(mp1) ||
	   swp.y != SHORT1FROMMP(mp2) || swp.cy != SHORT2FROMMP(mp2))) {
	szbx = SysVal(SV_CXSIZEBORDER);
	szby = SysVal(SV_CYSIZEBORDER);
	titl = SysVal(SV_CYTITLEBAR);
	WinQueryWindowPos(WinWindowFromID(hwnd, PICK_LISTBOX), &swpL);
	WinQueryWindowPos(WinWindowFromID(hwnd, PICK_INPUT), &swpE);
	WinSetWindowPos(WinWindowFromID(hwnd, PICK_LISTBOX), HWND_TOP,
			szbx,
			swpL.y,
			swp.cx - (szbx * 2L),
			((((swp.cy - swpL.y) - swpE.cy) - 8) - titl) -
			(szby * 2L), SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, PICK_INPUT), HWND_TOP,
			szbx + 2,
			swpL.y + (((((swp.cy - swpL.y) - swpE.cy) - 8) -
				   titl) - (szby * 2L)) + 4,
			0L, 0L, SWP_MOVE);
	WinInvalidateRect(WinWindowFromID(hwnd, PICK_INPUT), NULL, FALSE);
      }
    }
    return 0;

  case WM_ADJUSTWINDOWPOS:
    {
      SWP swp;

      WinQueryWindowPos(hwnd, &swp);
      PostMsg(hwnd,
	      UM_STRETCH,
	      MPFROM2SHORT((SHORT) swp.x, (SHORT) swp.cx),
	      MPFROM2SHORT((SHORT) swp.y, (SHORT) swp.cy));
    }
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case PICK_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_SELECT:
	sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
					     PICK_LISTBOX,
					     LM_QUERYSELECTION,
					     MPFROMSHORT(LIT_FIRST), MPVOID);
	if (sSelect >= 0)
	  WinSetDlgItemText(hwnd, PICK_INPUT, CodePage[sSelect]);
	break;
      case LN_ENTER:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      {
	INT x;
	CHAR s[257], *p;

	*s = 0;
	WinQueryDlgItemText(hwnd, PICK_INPUT, 257, s);
	if (!*s)
	  Runtime_Error(pszSrcFile, __LINE__, "no input");
	else {
          for (x = 0; x < 23; x++) {
            p = CodePage[x];
	    if (!stricmp(s, p)) {
	      WinDismissDlg(hwnd, atoi(p));
	      break;
	    }
	  }
	}
      }
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_CODEPAGE, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;

    case PICK_SAVEPOS:
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;

  case WM_CLOSE:
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

INT PickCodepage(HWND hwnd)
{

  INT cp;

  cp = (INT) WinDlgBox(HWND_DESKTOP,
		       hwnd,
		       PickCodePageDlgBox, FM3ModHandle, PICK_FRAME, MPVOID);
  if (cp > 0) {

    HMQ hmq;
    ULONG cps[50], len, x;

    if (cp == 1004) {
      hmq = WinQueryWindowULong(hwnd, QWL_HMQ);
      WinSetCp(hmq, cp);
    }
    else if (!DosQueryCp(sizeof(cps), cps, &len)) {
      for (x = 0; x < len / sizeof(ULONG); x++) {
	if (cps[x] == (ULONG) cp) {
	  hmq = WinQueryWindowULong(hwnd, QWL_HMQ);
	  WinSetCp(hmq, cp);
	  break;
	}
      }
    }
    DosSetProcessCp(cp);
  }
  else
    cp = -1;
  return cp;
}

#pragma alloc_text(FMCODEPAGE,PickCodePageDlgBox,PickCodepage)
