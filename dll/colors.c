
/***********************************************************************

  $Id$

  Set colors

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2010 Steven H. Levine

  14 Jul 06 SHL Use Runtime_Error
  22 Mar 07 GKY Use QWL_USER
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "fm3dlg.h"
#include "strutil.h"			// GetPString
#include "errutil.h"			// Runtime_Error
#include "colors.h"
#include "misc.h"			// PostMsg

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

MRESULT EXPENTRY ColorDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  COLORS *co;

  switch (msg) {
  case WM_INITDLG:
    if (mp2) {

      register SHORT x;

      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      co = (COLORS *) mp2;
      if (co->prompt)
	WinSetWindowText(hwnd, (CHAR *) GetPString(co->prompt));
      for (x = 0; x < co->numcolors; x++)
	WinSendDlgItemMsg(hwnd,
			  COLOR_LISTBOX,
			  LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0),
			  MPFROMP(GetPString(co->descriptions + x)));
      PostMsg(WinWindowFromID(hwnd,
			      COLOR_LISTBOX),
	      LM_SELECTITEM, MPFROMSHORT(0), MPFROMLONG(TRUE));
      PostMsg(hwnd,
	      WM_CONTROL, MPFROM2SHORT(COLOR_LISTBOX, LN_SELECT), MPVOID);
    }
    else
      WinDismissDlg(hwnd, 0);
    break;

  case WM_PAINT:
    PostMsg(hwnd, UM_PAINT, MPVOID, MPVOID);
    break;

  case UM_PAINT:
    {
      HPS hps;
      USHORT ids[] = { COLOR_WHITE,
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_RED,
	COLOR_PINK,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_YELLOW,
	COLOR_DARKGRAY,
	COLOR_DARKBLUE,
	COLOR_DARKRED,
	COLOR_DARKPINK,
	COLOR_DARKGREEN,
	COLOR_DARKCYAN,
	COLOR_BROWN,
	COLOR_PALEGRAY,
	0
      };
      LONG colors[] = { CLR_WHITE,
	CLR_BLACK,
	CLR_BLUE,
	CLR_RED,
	CLR_PINK,
	CLR_GREEN,
	CLR_CYAN,
	CLR_YELLOW,
	CLR_DARKGRAY,
	CLR_DARKBLUE,
	CLR_DARKRED,
	CLR_DARKPINK,
	CLR_DARKGREEN,
	CLR_DARKCYAN,
	CLR_BROWN,
	CLR_PALEGRAY
      };
      INT x;
      SWP swp;
      POINTL ptl;

      hps = WinGetPS(hwnd);
      if (hps) {
	for (x = 0; ids[x]; x++) {
	  WinQueryWindowPos(WinWindowFromID(hwnd, ids[x]), &swp);
	  ptl.x = swp.x - 12;
	  ptl.y = swp.y + swp.cy;
	  GpiMove(hps, &ptl);
	  GpiSetColor(hps, colors[x]);
	  ptl.x += 8;
	  ptl.y -= swp.cy;
	  GpiBox(hps, DRO_FILL, &ptl, 0, 0);
	  GpiSetColor(hps, (colors[x] == CLR_BLACK) ? CLR_WHITE : CLR_BLACK);
	  GpiBox(hps, DRO_OUTLINE, &ptl, 0, 0);
	}
	WinReleasePS(hwnd);
      }
    }
    return 0;

  case WM_CONTROL:
    co = (COLORS *) WinQueryWindowPtr(hwnd, QWL_USER);
    switch (SHORT1FROMMP(mp1)) {
    case COLOR_WHITE:
    case COLOR_BLACK:
    case COLOR_BLUE:
    case COLOR_RED:
    case COLOR_PINK:
    case COLOR_GREEN:
    case COLOR_CYAN:
    case COLOR_YELLOW:
    case COLOR_DARKGRAY:
    case COLOR_DARKBLUE:
    case COLOR_DARKRED:
    case COLOR_DARKPINK:
    case COLOR_DARKGREEN:
    case COLOR_DARKCYAN:
    case COLOR_BROWN:
    case COLOR_PALEGRAY:
      if (WinQueryButtonCheckstate(hwnd, SHORT1FROMMP(mp1))) {
	co->colors[co->currentcolor] = (LONG) (SHORT1FROMMP(mp1) -
					       COLOR_FIRST);
      }
      break;

    case COLOR_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_SELECT:
	{
	  SHORT sSelect;

	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      COLOR_LISTBOX,
					      LM_QUERYSELECTION,
					      MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (sSelect < 0)
	    Runtime_Error(pszSrcFile, __LINE__, "LM_QUERYSELECTION");
	  else {
	    co->currentcolor = (USHORT) sSelect;
	    WinCheckButton(hwnd, COLOR_FIRST +
			   co->colors[co->currentcolor], TRUE);
	  }
	}
	break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    co = (COLORS *) WinQueryWindowPtr(hwnd, QWL_USER);
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      memcpy(co->colors, co->origs, sizeof(LONG) * co->numcolors);
      WinDismissDlg(hwnd, 0);
      break;
    case DID_OK:
      WinDismissDlg(hwnd, 1);
      break;
    case IDM_UNDO:
      memcpy(co->colors, co->origs, sizeof(LONG) * co->numcolors);
      PostMsg(hwnd,
	      WM_CONTROL, MPFROM2SHORT(COLOR_LISTBOX, LN_SELECT), MPVOID);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(COLORS,ColorDlgProc)
