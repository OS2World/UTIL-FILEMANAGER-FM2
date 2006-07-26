
/***********************************************************************

  $Id$

  Input dialog procecedure

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2006 Steven H. Levine

  28 May 05 SHL Use saymsg
  14 Jul 06 SHL Use Runtime_Error

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma alloc_text(FMINPUT,InputDlgProc)

MRESULT EXPENTRY InputDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  // mp2 points at a structure of type STRINGINPARMS
  STRINGINPARMS *psip;
  PSZ psz;

  switch(msg)
  {
    case WM_INITDLG:
      if (!mp2)
      {
        Runtime_Error(pszSrcFile, __LINE__, "no data");
	WinDismissDlg(hwnd,0);
	break;
      }
      WinSetWindowPtr(hwnd,0,(PVOID)mp2);
      psip = (STRINGINPARMS *)mp2;
      if (!WinSendDlgItemMsg(hwnd,STR_INPUT,EM_SETTEXTLIMIT,
			     MPFROM2SHORT(psip->inputlen,0),MPVOID))
      {
        Win_Error(hwnd,hwnd,__FILE__,__LINE__,
                  "setlimit failed");
	WinDismissDlg(hwnd,0);
	break;
      }
      if (psip->prompt && *psip->prompt)
	WinSetDlgItemText(hwnd,STR_PROMPT,psip->prompt);
      if (psip->ret && *psip->ret)
      {
	WinSetDlgItemText(hwnd,STR_INPUT,psip->ret);
	WinSendDlgItemMsg(hwnd,STR_INPUT,EM_SETSEL,
			  MPFROM2SHORT(0,strlen(psip->ret)),MPVOID);
      }
      *psip->ret = 0;
      if (psip->title && *psip->title)
	WinSetWindowText(hwnd,psip->title);
      break;

    case WM_CONTROL:			// don't care
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1))
      {
	case DID_OK:
	  psip = WinQueryWindowPtr(hwnd,0);
	  WinQueryDlgItemText(hwnd,STR_INPUT,psip->inputlen,psip->ret);
	  WinDismissDlg(hwnd,1);
	  break;

	case IDM_HELP:
	  psip = WinQueryWindowPtr(hwnd,0);
	  psz = psip->help && *psip->help ?
		psip->help : GetPString(IDS_ENTERTEXTHELPTEXT);

	  saymsg(MB_ENTER | MB_ICONASTERISK,
		 hwnd,
		 GetPString(IDS_HELPTEXT),
		 psz);
	  break;

	case DID_CANCEL:
	  WinDismissDlg(hwnd,0);
	  break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

