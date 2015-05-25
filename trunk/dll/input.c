
/***********************************************************************

  $Id$

  Input dialog procecedure

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2010 Steven H. Levine

  28 May 05 SHL Use saymsg
  14 Jul 06 SHL Use Runtime_Error
  22 Mar 07 GKY Use QWL_USER
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "input.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "fm3dll.h"

static PSZ pszSrcFile = __FILE__;

MRESULT EXPENTRY InputDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  // mp2 points at a structure of type STRINGINPARMS
  STRINGINPARMS *psip;
  PCSZ psz;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      WinDismissDlg(hwnd, 0);
      break;
    }
    WinSetWindowPtr(hwnd, 0, (PVOID) mp2);
    psip = (STRINGINPARMS *) mp2;
    if (!WinSendDlgItemMsg(hwnd, STR_INPUT, EM_SETTEXTLIMIT,
			   MPFROM2SHORT(psip->inputlen, 0), MPVOID)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__, "setlimit failed");
      WinDismissDlg(hwnd, 0);
      break;
    }
    if (psip->prompt && *psip->prompt)
      WinSetDlgItemText(hwnd, STR_PROMPT, (CHAR *) psip->prompt);
    if (psip->ret && *psip->ret) {
      WinSetDlgItemText(hwnd, STR_INPUT, psip->ret);
      WinSendDlgItemMsg(hwnd, STR_INPUT, EM_SETSEL,
			MPFROM2SHORT(0, strlen(psip->ret)), MPVOID);
    }
    *psip->ret = 0;
    if (psip->title && *psip->title)
      WinSetWindowText(hwnd, (CHAR *) psip->title);
    break;

  case WM_CONTROL:			// don't care
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      psip = WinQueryWindowPtr(hwnd, QWL_USER);
      WinQueryDlgItemText(hwnd, STR_INPUT, psip->inputlen, psip->ret);
      WinDismissDlg(hwnd, 1);
      break;

    case IDM_HELP:
      psip = WinQueryWindowPtr(hwnd, QWL_USER);
      psz = psip->help && *psip->help ?
	psip->help : GetPString(IDS_ENTERTEXTHELPTEXT);

      saymsg(MB_ENTER | MB_ICONASTERISK, hwnd, GetPString(IDS_HELPTEXT), psz);
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(FMINPUT,InputDlgProc)
