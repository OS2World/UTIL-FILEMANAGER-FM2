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


MRESULT EXPENTRY InputDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  /*
   * mp2 should point at a structure of type STRINGINPARMS
   */

  STRINGINPARMS *ret;

  switch(msg) {
    case WM_INITDLG:
      if(!mp2) {
        WinDismissDlg(hwnd,0);
        break;
      }
      WinSetWindowPtr(hwnd,0,(PVOID)mp2);
      ret = (STRINGINPARMS *)mp2;
      if(!(BOOL)WinSendDlgItemMsg(hwnd,STR_INPUT,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(ret->inputlen,0),MPVOID)) {
        DosBeep(50,100);
        WinDismissDlg(hwnd,0);
        break;
      }
      if(ret->prompt && *ret->prompt)
        WinSetDlgItemText(hwnd,STR_PROMPT,ret->prompt);
      if(ret->ret && *ret->ret) {
        WinSetDlgItemText(hwnd,STR_INPUT,ret->ret);
        WinSendDlgItemMsg(hwnd,STR_INPUT,EM_SETSEL,
                          MPFROM2SHORT(0,strlen(ret->ret)),MPVOID);
      }
      *ret->ret = 0;
      if(ret->title && *ret->title)
        WinSetWindowText(hwnd,ret->title);
      break;

    case WM_CONTROL:    /* don't care */
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          ret = WinQueryWindowPtr(hwnd,0);
          WinQueryDlgItemText(hwnd,STR_INPUT,ret->inputlen,ret->ret);
          WinDismissDlg(hwnd,1);
          break;

        case IDM_HELP:
          ret = WinQueryWindowPtr(hwnd,0);
          if(ret->help && *ret->help)
            WinMessageBox(HWND_DESKTOP,hwnd,ret->help,
                          GetPString(IDS_HELPTEXT),
                          0,MB_ENTER | MB_ICONASTERISK | MB_MOVEABLE);
          else
            WinMessageBox(HWND_DESKTOP,hwnd,
                          GetPString(IDS_ENTERTEXTHELPTEXT),
                          GetPString(IDS_HELPTEXT),
                          0,MB_ENTER | MB_ICONASTERISK | MB_MOVEABLE);
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

