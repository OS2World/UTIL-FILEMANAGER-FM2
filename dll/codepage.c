#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA1)

#pragma alloc_text(FMCODEPAGE,PickCodePageDlgBox,PickCodepage)

MRESULT EXPENTRY PickCodePageDlgBox (HWND hwnd,ULONG msg,MPARAM mp1,
                                     MPARAM mp2) {

  SHORT sSelect;

  switch(msg) {
    case WM_INITDLG:
      WinShowWindow(WinWindowFromID(hwnd,PICK_SAVEPOS),
                    FALSE);
      {
        ULONG cp;
        char *p;

        cp = WinQueryCp(WinQueryWindowULong(hwnd,
                        QWL_HMQ));
        for(sSelect = 0;
            (p = GetPString(IDS_CODEPAGES1 + sSelect)) != NULL;
            sSelect++) {
          if(!strcmp(p,"0"))
            break;
          WinSendDlgItemMsg(hwnd,
                            PICK_LISTBOX,
                            LM_INSERTITEM,
                            MPFROMSHORT(LIT_END),
                            MPFROMP(p));
          if(atoi(p) == cp) {
            WinSendDlgItemMsg(hwnd,
                              PICK_LISTBOX,
                              LM_SETTOPINDEX,
                              MPFROM2SHORT(sSelect,0),
                              MPVOID);
            WinSendDlgItemMsg(hwnd,
                              PICK_LISTBOX,
                              LM_SELECTITEM,
                              MPFROM2SHORT(sSelect,0),
                              MPFROMSHORT(TRUE));
          }
        }
      }
      WinSendDlgItemMsg(hwnd,
                        PICK_INPUT,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(256,0),
                        MPVOID);
      WinSetWindowText(hwnd,
                       GetPString(IDS_PICKCODEPAGETEXT));
      PostMsg(hwnd,
              UM_STRETCH,
              MPVOID,
              MPVOID);
      break;

    case UM_STRETCH:
      {
        SWP  swp,swpL,swpE;
        LONG titl,szbx,szby;

        WinQueryWindowPos(hwnd,&swp);
        if(!(swp.fl & (SWP_HIDE | SWP_MINIMIZE)) &&
           (swp.x != SHORT1FROMMP(mp1) || swp.cx != SHORT2FROMMP(mp1) ||
            swp.y != SHORT1FROMMP(mp2) || swp.cy != SHORT2FROMMP(mp2))) {
          szbx = SysVal(SV_CXSIZEBORDER);
          szby = SysVal(SV_CYSIZEBORDER);
          titl = SysVal(SV_CYTITLEBAR);
          WinQueryWindowPos(WinWindowFromID(hwnd,PICK_LISTBOX),&swpL);
          WinQueryWindowPos(WinWindowFromID(hwnd,PICK_INPUT),&swpE);
          WinSetWindowPos(WinWindowFromID(hwnd,PICK_LISTBOX),HWND_TOP,
                          szbx,
                          swpL.y,
                          swp.cx - (szbx * 2L),
                          ((((swp.cy - swpL.y) - swpE.cy) - 8) - titl) -
                           (szby * 2L),
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,PICK_INPUT),HWND_TOP,
                          szbx + 2,
                          swpL.y + (((((swp.cy - swpL.y) - swpE.cy) - 8) -
                                    titl) - (szby * 2L)) + 4,
                          0L,0L,
                          SWP_MOVE);
          WinInvalidateRect(WinWindowFromID(hwnd,PICK_INPUT),
                            NULL,
                            FALSE);
        }
      }
      return 0;

    case WM_ADJUSTWINDOWPOS:
      {
        SWP    swp;

        WinQueryWindowPos(hwnd,&swp);
        PostMsg(hwnd,
                UM_STRETCH,
                MPFROM2SHORT((SHORT)swp.x,(SHORT)swp.cx),
                MPFROM2SHORT((SHORT)swp.y,(SHORT)swp.cy));
      }
      break;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case PICK_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case LN_SELECT:
              sSelect = (USHORT)WinSendDlgItemMsg(hwnd,
                                                  PICK_LISTBOX,
                                                  LM_QUERYSELECTION,
                                                  MPFROMSHORT(LIT_FIRST),
                                                  MPVOID);
              if(sSelect >= 0)
                WinSetDlgItemText(hwnd,
                                  PICK_INPUT,
                                  GetPString(IDS_CODEPAGES1 + sSelect));
              break;
            case LN_ENTER:
              PostMsg(hwnd,
                      WM_COMMAND,
                      MPFROM2SHORT(DID_OK,0),
                      MPVOID);
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          {
            INT  x;
            CHAR s[257],*p;

            *s = 0;
            WinQueryDlgItemText(hwnd,
                                PICK_INPUT,
                                257,
                                s);
            if(*s) {
              for(x = 0;
                  (p = GetPString(IDS_CODEPAGES1 + x)) != NULL;
                  x++) {
                if(!stricmp(s,p)) {
                  WinDismissDlg(hwnd,atoi(p));
                  break;
                }
              }
            }
            else
              DosBeep(50,100);
          }
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CODEPAGE,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case PICK_SAVEPOS:
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;

    case WM_CLOSE:
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


INT PickCodepage (HWND hwnd) {

  INT cp;

  cp = (INT)WinDlgBox(HWND_DESKTOP,
                      hwnd,
                      PickCodePageDlgBox,
                      FM3ModHandle,
                      PICK_FRAME,
                      MPVOID);
  if(cp > 0) {

    HMQ   hmq;
    ULONG cps[50],len,x;

/*
    numcps = WinQueryCpList(WinQueryAnchorBlock(hwnd),50,cps);
    if(numcps) {
      for(x = 0;x < numcps;x++) {
        if(cps[x] == (ULONG)cp) {
          hmq = WinQueryWindowULong(hwnd,QWL_HMQ);
          WinSetCp(hmq,cp);
          break;
        }
      }
    }
*/
    if(cp == 1004) {
      hmq = WinQueryWindowULong(hwnd,QWL_HMQ);
      WinSetCp(hmq,cp);
    }
    else if(!DosQueryCp(sizeof(cps),cps,&len)) {
      for(x = 0;x < len / sizeof(ULONG);x++) {
        if(cps[x] == (ULONG)cp) {
          hmq = WinQueryWindowULong(hwnd,QWL_HMQ);
          WinSetCp(hmq,cp);
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

