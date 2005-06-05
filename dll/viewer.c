
/***********************************************************************

  $Id$

  MLE text editor/viewer

  Copyright (c) 1993-97 M. Kimes
  Copyright (c) 2005 Steven H. Levine

  23 May 05 SHL Use QWL_USER

***********************************************************************/

#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"

#pragma data_seg(DATA1)
#pragma alloc_text(VIEWER,MLEEditorProc,MLESubProc)
#pragma alloc_text(STARTUP,StartMLEEditor)

#define hwndMLE WinWindowFromID(hwnd,MLE_MLE)

#define COLORS_MAX                   2

#define COLORS_FOREGROUND            0
#define COLORS_BACKGROUND            1

static LONG   Colors[COLORS_MAX] = {COLR_BLACK,COLR_PALEGRAY};
static BOOL   Firsttime = TRUE;


HWND StartMLEEditor (HWND hwndClient,INT flags,CHAR *filename,
                     HWND hwndRestore) {

  /*
   * create an editor window
   * bitmapped flags:
   *  1 =  readonly
   *  2 =  don't position window for non-desktop client
   *  4 =  don't "kill app" when closed
   *  8 =  as text
   *  16 = as binary
   */

  HWND        hwndFrame,hwnd;
  XMLEWNDPTR *vw;
  ULONG    flFrameFlags = FCF_SYSMENU    | FCF_SIZEBORDER | FCF_ICON |
                          FCF_TITLEBAR   | FCF_MINMAX     | FCF_MENU |
                          FCF_ACCELTABLE | FCF_NOBYTEALIGN;

  if(fExternalViewer || strcmp(realappname,FM3Str))
    hwndClient = HWND_DESKTOP;
  if((flags & 1) && fUseNewViewer)
    return StartViewer(hwndClient,(USHORT)flags,filename,hwndRestore);

  vw = malloc(sizeof(XMLEWNDPTR));
  if(!vw) {
    DosBeep(50,100);
    return (HWND)0;
  }
  memset(vw,0,sizeof(XMLEWNDPTR));
  vw->size = sizeof(XMLEWNDPTR);
  if(flags & 1) {
    if(flags & 8)
      vw->hex = 2;
    else if(flags & 16)
      vw->hex = 1;
    else if(!fGuessType)
      vw->hex = 2;
  }
  else
    vw->hex = 2;
  vw->hwndParent = (hwndClient) ? hwndClient : HWND_DESKTOP;
  vw->srch.size = sizeof(SRCHPTR);
  if(flags & 4)
    vw->dontclose = TRUE;
  if(filename)
    strcpy(vw->exportfilename,filename);
  vw->hwndRestore = hwndRestore;
  if(ParentIsDesktop(hwndClient,hwndClient))
    flFrameFlags |= FCF_TASKLIST;
  hwndFrame = WinCreateStdWindow(hwndClient,
                                 WS_VISIBLE,
                                 &flFrameFlags,
                                 GetPString(IDS_WCMLEEDITOR),
                                 NullStr,
                                 WS_VISIBLE | fwsAnimate,
                                 FM3ModHandle,
                                 MLE_FRAME,
                                 &hwnd);
  if(hwndFrame) {
    vw->hwndFrame = hwndFrame;
    if(!ParentIsDesktop(hwndFrame,hwndClient) && !(flags & 2)) {

      SWP swp;

      FillClient(hwndClient,&swp,NULL,FALSE);
      WinSetWindowPos(hwndFrame,
                      HWND_TOP,
                      swp.x,
                      swp.y,
                      swp.cx,
                      swp.cy,
                      SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ACTIVATE |
                      SWP_ZORDER);
    }
    else if(flFrameFlags & FCF_TASKLIST) {

      SWP   swp,swpD;
      ULONG size = sizeof(swp);
      LONG  cxScreen,cyScreen;

      WinQueryTaskSizePos(WinQueryAnchorBlock(hwndFrame),
                          0,
                          &swp);
      if(PrfQueryProfileData(fmprof,
                             appname,
                             "ViewSizePos",
                             &swpD,
                             &size)) {
        cxScreen = WinQuerySysValue(HWND_DESKTOP,
                                    SV_CXSCREEN);
        cyScreen = WinQuerySysValue(HWND_DESKTOP,
                                    SV_CYSCREEN);
        if(swp.x + swpD.cx > cxScreen)
          swp.x = cxScreen - swpD.cx;
        if(swp.y + swpD.cy > cyScreen)
          swp.y = cyScreen - swpD.cy;
        swp.cx = swpD.cx;
        swp.cy = swpD.cy;
      }
      WinSetWindowPos(hwndFrame,
                      HWND_TOP,
                      swp.x,
                      swp.y,
                      swp.cx,
                      swp.cy,
                      SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ZORDER |
                      SWP_ACTIVATE);
    }
    MLEsetreadonly(hwndMLE,((flags & 1) != 0));
    WinSetWindowPtr(hwnd,QWL_USER,(PVOID)vw);
    if(!PostMsg(hwnd,
                UM_SETUP,
                MPVOID,
                MPFROMLONG(hwndClient)))
      WinSendMsg(hwnd,
                 UM_SETUP,
                 MPVOID,
                 MPFROMLONG(hwndClient));
  }
  return hwndFrame;
}


MRESULT EXPENTRY MLESubProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PFNWP       oldproc = (PFNWP)WinQueryWindowPtr(hwnd,QWL_USER);
  XMLEWNDPTR *vw;

  switch(msg) {
    case WM_SETFOCUS:
      if(mp2)
        PostMsg(WinQueryWindow(hwnd,QW_PARENT),UM_SELECT,mp1,mp2);
      break;

    case WM_CHAR:
      vw = WinQueryWindowPtr(WinQueryWindow(hwnd,QW_PARENT),QWL_USER);
      if(vw && vw->size == sizeof(XMLEWNDPTR) && vw->hex == 1) {
        if(!MLEgetreadonly(hwnd)) {
          WinSetSysValue(HWND_DESKTOP,SV_INSERTMODE,FALSE);
          if(vw->fWrap) {
            vw->fWrap = FALSE;
            MLEsetwrap(hwnd,FALSE);
          }
          MLEanctocur(hwnd);
          {
            static ULONG  badoff[] = {11,14,17,20,23,26,29,32,35,38,41,44,
                                      47,0};
            ULONG         line,pos,offset,len,x;
            MRESULT       mr;

            mr = oldproc(hwnd,msg,mp1,mp2);

            len = MLEcurlinelen(hwnd);
            line = MLEcurline(hwnd);
            pos = MLEcurpos(hwnd);
            offset = len - MLElinelenleft(hwnd,pos);
            if(offset < 9) {
              if(!line || offset == 8 || line == MLEnumlines(hwnd) - 1)
                MLEsetcurpos(hwnd,pos + (9 - offset));
              else
                MLEsetcurpos(hwnd,MLEstartofline(hwnd,line + 1) + 9);
            }
            else if(offset >= 71) {
              if(vw->lastpos == offset - 1) {
                if(line < MLEnumlines(hwnd) - 1)
                  MLEsetcurpos(hwnd,MLEstartofline(hwnd,line + 2) + 55);
                else
                  MLEsetcurpos(hwnd,MLEstartofline(hwnd,line + 1) + 55);
              }
              else
                MLEsetcurpos(hwnd,MLEstartofline(hwnd,line) + 70);
            }
            else if(offset == 53) {
              if(line < MLEnumlines(hwnd) - 1)
                MLEsetcurpos(hwnd,MLEstartofline(hwnd,line + 1) + 9);
              else
                MLEsetcurpos(hwnd,MLEstartofline(hwnd,line) + 9);
            }
            else if(offset == 54) {
              if(line < MLEnumlines(hwnd) - 1)
                MLEsetcurpos(hwnd,MLEstartofline(hwnd,line + 1) + 70);
              else
                MLEsetcurpos(hwnd,MLEstartofline(hwnd,line) + 70);
            }
            else {
              for(x = 0;badoff[x];x++) {
                if(offset == badoff[x]) {
                  if(vw->lastpos < pos)
                    MLEsetcurpos(hwnd,pos + 1);
                  else
                    MLEsetcurpos(hwnd,pos - 1);
                }
              }
            }
            {
              CHAR s[80];

              sprintf(s,
                      GetPString(IDS_VIEWPOSSTUFFTEXT),
                      len,
                      line,
                      pos,
                      offset);
              if(hwndStatus)
                WinSetWindowText(hwndStatus,s);
            }
            vw->lastpos = MLEcurpos(hwnd);
            return mr;
          }
        }
      }
      break;

    case WM_MENUEND:
      vw = WinQueryWindowPtr(WinQueryWindow(hwnd,QW_PARENT),QWL_USER);
      if(vw && vw->size == sizeof(XMLEWNDPTR)) {
        if(vw->hwndPopupMenu == (HWND)mp2) {
          WinDestroyWindow(vw->hwndPopupMenu);
          vw->hwndPopupMenu = (HWND)0;
        }
      }
      break;

    case WM_CONTEXTMENU:
      vw = WinQueryWindowPtr(WinQueryWindow(hwnd,QW_PARENT),QWL_USER);
      if(vw && vw->size == sizeof(XMLEWNDPTR)) {
        if(!vw->hwndPopupMenu)
          vw->hwndPopupMenu = WinLoadMenu(HWND_DESKTOP,FM3ModHandle,MLE_POPUP);
        if(vw->hwndPopupMenu) {
          if(MLEgetreadonly(hwnd)) {
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_UNDO,FALSE);
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_CUTCLIP,FALSE);
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_PASTECLIP,FALSE);
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_CLEAR,FALSE);
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_BLOCKMENU,FALSE);
          }
          else {
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_UNDO,TRUE);
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_CUTCLIP,TRUE);
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_PASTECLIP,TRUE);
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_CLEAR,TRUE);
            WinEnableMenuItem(vw->hwndPopupMenu,MLE_BLOCKMENU,TRUE);
          }
          if(!*ftprun)
            WinSendMsg(vw->hwndPopupMenu,MM_DELETEITEM,
                       MPFROM2SHORT(MLE_VIEWFTP,FALSE),MPVOID);
          if(!*httprun)
            WinSendMsg(vw->hwndPopupMenu,MM_DELETEITEM,
                       MPFROM2SHORT(MLE_VIEWHTTP,FALSE),MPVOID);
          PopupMenu(WinQueryWindow(hwnd,QW_PARENT),
                    WinQueryWindow(hwnd,QW_PARENT),
                    vw->hwndPopupMenu);
        }
      }
      break;
  }
  return oldproc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY MLEEditorProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  XMLEWNDPTR *vw;

  if(msg != WM_CREATE &&
     msg != UM_SETUP)
    vw = (XMLEWNDPTR *)WinQueryWindowPtr(hwnd,QWL_USER);
  else
    vw = NULL;

  switch(msg) {
    case WM_CREATE:
      /* create MLE window */
      WinCreateWindow(hwnd,
                      WC_MLE,
                      (PSZ)NULL,
                      MLS_HSCROLL | MLS_VSCROLL | MLS_BORDER |
                      WS_VISIBLE,
                      0L,
                      0L,
                      0L,
                      0L,
                      hwnd,
                      HWND_TOP,
                      MLE_MLE,
                      MPVOID,
                      MPVOID);
      {
        PFNWP oldproc;

        oldproc = WinSubclassWindow(WinWindowFromID(hwnd,MLE_MLE),
                                    (PFNWP)MLESubProc);
        if(oldproc)
          WinSetWindowPtr(WinWindowFromID(hwnd,MLE_MLE),
                          QWL_USER,
                          (PVOID)oldproc);
      }
      break;

    case WM_INITMENU:
      switch(SHORT1FROMMP(mp1)) {
        case MLE_EDITMENU:
          {
            ULONG ret;
            CHAR  lbl[162];

            strcpy(lbl,GetPString(IDS_UNDOTEXT));
            ret = (ULONG)WinSendMsg(hwndMLE,MLM_QUERYUNDO,MPVOID,MPVOID);
            switch(HIUSHORT(ret)) {
              case WM_CHAR:
              case MLM_CUT:
              case MLM_CLEAR:
              case MLM_PASTE:
                WinEnableMenuItem((HWND)mp2,MLE_UNDO,TRUE);
                if(!LOUSHORT(ret))
                  strcpy(lbl,GetPString(IDS_REDOTEXT));
                switch(HIUSHORT(ret)) {
                  case WM_CHAR:
                    strcat(lbl,GetPString(IDS_KEYSTROKEMENUTEXT));
                    break;
                  case MLM_CUT:
                    strcat(lbl,GetPString(IDS_CUTTOCLIPMENUTEXT));
                    break;
                  case MLM_CLEAR:
                    strcat(lbl,GetPString(IDS_CLEARSELMENUTEXT));
                    break;
                  case MLM_PASTE:
                    strcat(lbl,GetPString(IDS_PASTEFROMCLIPMENUTEXT));
                    break;
                }
                WinSetMenuItemText((HWND)mp2,MLE_UNDO,lbl);
                break;
              default:
                WinEnableMenuItem((HWND)mp2,MLE_UNDO,FALSE);
                break;
            }
          }
          break;
      }
      break;

    case UM_SETUP2:
      vw = WinQueryWindowPtr(hwnd,QWL_USER);
      if(vw) {

        CHAR s[CCHMAXPATH + 8];

        sprintf(s,
                "%s: %s",
                FM2Str,
                (*vw->exportfilename) ?
                 vw->exportfilename :
                 GetPString(IDS_UNTITLEDTEXT));
        WinSetWindowText(WinQueryWindow(hwnd,
                                        QW_PARENT),
                         s);
        if(WinQueryWindow(hwnd,QW_PARENT) ==
           WinQueryActiveWindow(WinQueryWindow(WinQueryWindow(hwnd,
                                                              QW_PARENT),
                                QW_PARENT)) &&
           !ParentIsDesktop(WinQueryWindow(hwnd,QW_PARENT),(HWND)0)) {
          if(hwndStatus2)
            WinSetWindowText(hwndStatus2,
                             (*vw->exportfilename) ?
                              vw->exportfilename :
                              GetPString(IDS_UNTITLEDTEXT));
          if(fMoreButtons) {
            WinSetWindowText(hwndName,
                             (*vw->exportfilename) ?
                              vw->exportfilename :
                              GetPString(IDS_UNTITLEDTEXT));
            WinSetWindowText(hwndDate,NullStr);
            WinSetWindowText(hwndAttr,NullStr);
          }
          if(hwndStatus)
            WinSetWindowText(hwndStatus,
                             GetPString(IDS_INTERNALVIEWEREDITORTITLETEXT));
        }
      }
      return 0;

    case UM_SETUP:
      vw = WinQueryWindowPtr(hwnd,QWL_USER);
      if(vw) {
        vw->hab = WinQueryAnchorBlock(hwnd);
        WinSendMsg(hwnd,UM_SETUP2,MPVOID,MPVOID);
        /* set up initial MLE conditions */
        vw->srch.hwndmle = hwndMLE;
        MLEsetcurpos(hwndMLE,0L);
        MLEclearall(hwndMLE);
        MLEsetlimit(hwndMLE,-1L);
        MLEsetformat(hwndMLE,MLFIE_NOTRANS);
        WinSetSysValue(HWND_DESKTOP,SV_INSERTMODE,TRUE);
        vw->fWrap = TRUE;
        vw->fStripTrail = TRUE;
        vw->fStripTrailLines = TRUE;
        vw->srch.fInsensitive = TRUE;
        vw->ExpandTabs = 4;
        vw->TabStops = 32;
        {
          ULONG size;

          size = sizeof(BOOL);
          PrfQueryProfileData(fmprof,
                              FM3Str,
                              "MLEWrap",
                              (PVOID)&vw->fWrap,
                              &size);
          size = sizeof(BOOL);
          PrfQueryProfileData(fmprof,
                              FM3Str,
                              "MLEstriptrail",
                              (PVOID)&vw->fStripTrail,
                              &size);
          size = sizeof(BOOL);
          PrfQueryProfileData(fmprof,
                              FM3Str,
                              "MLEstriptraillines",
                              (PVOID)&vw->fStripTrailLines,
                              &size);
          size = sizeof(BOOL);
          PrfQueryProfileData(fmprof,
                              FM3Str,
                              "MLEInsensitve",
                              (PVOID)&vw->srch.fInsensitive,
                              &size);
          size = sizeof(INT);
          PrfQueryProfileData(fmprof,
                              FM3Str,
                              "MLEExpandTabs",
                              (PVOID)&vw->ExpandTabs,
                              &size);
          size = sizeof(INT);
          PrfQueryProfileData(fmprof,
                              FM3Str,
                              "MLETabStops",
                              (PVOID)&vw->TabStops,
                              &size);
        }
        vw->accel = WinQueryAccelTable(vw->hab,
                                       WinQueryWindow(hwnd,QW_PARENT));
        vw->hwndMenu = WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                       FID_MENU);
        vw->ch = FALSE;
        MLEsetchanged(hwndMLE,FALSE);
        {
          MRESULT ret = 0;

          if(vw->hwndMenu) {
            SetMenuCheck(vw->hwndMenu,
                         MLE_TOGWRAP,
                         &vw->fWrap,
                         FALSE,
                         NULL);
            SetMenuCheck(vw->hwndMenu,
                         MLE_STRIPTRAILBLANKS,
                         &vw->fStripTrail,
                         FALSE,
                         NULL);
            SetMenuCheck(vw->hwndMenu,
                         MLE_STRIPTRAILLINES,
                         &vw->fStripTrailLines,
                         FALSE,
                         NULL);
            {
              BOOL tempbool = (vw->ExpandTabs != 0);

              SetMenuCheck(vw->hwndMenu,
                           MLE_EXPANDTABS,
                           &tempbool,
                           FALSE,
                           NULL);
            }
            SetMenuCheck(vw->hwndMenu,
                         MLE_SENSITIVE,
                         &vw->srch.fInsensitive,
                         FALSE,
                         NULL);
          }
          MLEsetwrap(hwndMLE,vw->fWrap);
          {
            ULONG CpList[2];
            ULONG CpSize;

            if(!DosQueryCp(sizeof(CpList),
                           CpList,
                           &CpSize) &&
               CpSize)
              vw->cp = CpList[0];
          }
          {
            ULONG size = sizeof(FATTRS),cps[50],len,x;
            HMQ hmq;

            if(!PrfQueryProfileData(fmprof,
                                    FM3Str,
                                    "MLEFont",
                                    &vw->fattrs,
               &size) || size != sizeof(FATTRS)) {
              vw->fattrs.usRecordLength = sizeof(FATTRS);
              vw->fattrs.lMaxBaselineExt = 16;
              vw->fattrs.lAveCharWidth   = 8;
              strcpy(vw->fattrs.szFacename,
                     GetPString(IDS_SYSMONOTEXT));
              vw->fattrs.usCodePage = (USHORT)vw->cp;
            }
            else
              vw->cp = vw->fattrs.usCodePage;
            if(!DosQueryCp(sizeof(cps),
                           cps,
                           &len)) {
              for(x = 0;x < len / sizeof(ULONG);x++) {
                if(cps[x] == (ULONG)vw->cp) {
                  hmq = WinQueryWindowULong(hwnd,
                                            QWL_HMQ);
                  WinSetCp(hmq,vw->cp);
                  break;
                }
              }
            }
            DosSetProcessCp(vw->cp);
            WinSendMsg(hwndMLE,
                       MLM_SETFONT,
                       MPFROMP(&vw->fattrs),
                       MPVOID);
          }
          if(Firsttime) {

            ULONG size;

            Firsttime = FALSE;
            size = sizeof(ULONG);
            PrfQueryProfileData(fmprof,
                                FM3Str,
                                "MLEBackgroundcolor",
                                &Colors[COLORS_BACKGROUND],
                                &size);
            size = sizeof(ULONG);
            PrfQueryProfileData(fmprof,
                                FM3Str,
                                "MLEForegroundcolor",
                                &Colors[COLORS_FOREGROUND],
                                &size);
          }
          WinSendMsg(hwndMLE,
                     MLM_SETBACKCOLOR,
                     MPFROMLONG(standardcolors[Colors[COLORS_BACKGROUND]]),
                     MPVOID);
          WinSendMsg(hwndMLE,
                     MLM_SETTEXTCOLOR,
                     MPFROMLONG(standardcolors[Colors[COLORS_FOREGROUND]]),
                     MPVOID);
          if(*vw->exportfilename)
            if(MLEbackgroundload(hwnd,
                                 UM_CONTAINER_FILLED,
                                 hwndMLE,
                                 vw->exportfilename,vw->hex) !=
               -1) {
              vw->busy = TRUE;
              WinEnableWindow(vw->hwndMenu,
                              FALSE);
            }
          if(vw->busy || MLEgetreadonly(hwndMLE)) {
            disable_menuitem(vw->hwndMenu,
                             MLE_FILEMENU,
                             FALSE);
            disable_menuitem(vw->hwndMenu,
                             MLE_CUTCLIP,
                             FALSE);
            disable_menuitem(vw->hwndMenu,
                             MLE_PASTECLIP,
                             FALSE);
            disable_menuitem(vw->hwndMenu,
                             MLE_CLEAR,
                             FALSE);
            disable_menuitem(vw->hwndMenu,
                             MLE_CUTLINE,
                             FALSE);
            disable_menuitem(vw->hwndMenu,
                             MLE_BLOCKMENU,
                             FALSE);
          }
          return ret;
        }
      }
      return 0;

    case UM_CONTAINER_FILLED: /* file was loaded */
      WinEnableWindow(vw->hwndMenu,
                      TRUE);
      vw->busy = FALSE;
      if(vw->killme) {
        PostMsg(hwnd,
                WM_CLOSE,
                MPVOID,
                MPVOID);
        return 0;
      }
      if(!MLEgetreadonly(hwndMLE)) {
        disable_menuitem(vw->hwndMenu,
                         MLE_FILEMENU,
                         TRUE);
        disable_menuitem(vw->hwndMenu,
                         MLE_CUTCLIP,
                         TRUE);
        disable_menuitem(vw->hwndMenu,
                         MLE_PASTECLIP,
                         TRUE);
        disable_menuitem(vw->hwndMenu,
                         MLE_CLEAR,
                         TRUE);
        disable_menuitem(vw->hwndMenu,
                         MLE_CUTLINE,
                         TRUE);
        disable_menuitem(vw->hwndMenu,
                         MLE_BLOCKMENU,
                         TRUE);
      }
      if(mp1) {
        if(mp2) {
          vw->ch = FALSE;
          strcpy(vw->exportfilename,(CHAR *)mp2);
        }
      }
      else {
        vw->ch = FALSE;
        *vw->exportfilename = 0;
      }
      WinSendMsg(hwnd,UM_SETUP2,MPVOID,MPVOID);
      return 0;

    case WM_SIZE:
      WinSetWindowPos(hwndMLE,
                      HWND_TOP,
                      0,
                      0,
                      SHORT1FROMMP(mp2),
                      SHORT2FROMMP(mp2),
                      SWP_MOVE | SWP_SIZE);
      break;

    case UM_SELECT:
    case WM_SETFOCUS:       /* sling focus to MLE */
      if(mp2) {
        if(hwndMain && fAutoView)
          PostMsg(hwndMain,
                  UM_LOADFILE,
                  MPVOID,
                  MPVOID);
        WinSendMsg(hwnd,UM_SETUP2,MPVOID,MPVOID);
        if(msg != UM_SELECT)
          PostMsg(hwnd,
                  UM_FOCUSME,
                  MPVOID,
                  MPVOID);
      }
      if(msg == WM_SETFOCUS)
        break;
      return 0;

    case UM_FOCUSME:
      WinSetFocus(HWND_DESKTOP, hwndMLE);
      return 0;

    case WM_ERASEBACKGROUND:
      WinFillRect((HPS)mp1,(PRECTL)mp2,0x00d0d0d0);
      return 0;

    case WM_PAINT:
      {
        RECTL rcl;
        HPS hps;

        hps = WinBeginPaint(hwnd,(HPS)0,NULL);
        WinQueryWindowRect(hwnd,&rcl);
        WinFillRect(hps,&rcl,CLR_PALEGRAY);
        WinEndPaint(hps);
      }
      break;

    case UM_LOADFILE:
      if((CHAR *)mp1) {
//        switch_to(mp1);
        if(MLEbackgroundload(hwnd,
                             UM_CONTAINER_FILLED,
                             hwndMLE,
                             (CHAR *)mp1,
                             vw->hex) !=
           -1) {
          vw->busy = TRUE;
          WinEnableWindow(vw->hwndMenu,
                          FALSE);
        }
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case MLE_MLE:
          switch(SHORT2FROMMP(mp1)) {
            case MLN_CHANGE:
              if(!vw->ch)
                vw->ch = TRUE;
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      if(vw->busy && SHORT1FROMMP(mp1) != MLE_QUIT) {
        DosBeep(50,100);
        return 0;
      }
      switch(SHORT1FROMMP(mp1)) {
/*
        case MLE_PREVIEW:
          preview_text(hwndMLE);
          break;
*/
        case MLE_VIEWFTP:
          MLEinternet(hwndMLE,TRUE);
          break;

        case MLE_VIEWHTTP:
          MLEinternet(hwndMLE,FALSE);
          break;

        case IDM_NEXTWINDOW:
          WinSendMsg(hwndMLE,
                     WM_CHAR,
                     MPFROM2SHORT((KC_CHAR | KC_VIRTUALKEY),
                                  MAKEUSHORT(1,9)),
                     MPFROM2SHORT(9,VK_TAB));
          break;

        case IDM_COLORPALETTE:
          {
            COLORS co;
            LONG   temp[COLORS_MAX];

            memset(&co,0,sizeof(co));
            co.size = sizeof(co);
            co.numcolors = COLORS_MAX;
            co.colors = Colors;
            co.descriptions = IDS_EDCOLORS1TEXT;
            co.origs = temp;
            co.prompt = IDS_EDCOLORSPROMPTTEXT;
            memcpy(temp,
                   Colors,
                   sizeof(LONG) * COLORS_MAX);
            if(WinDlgBox(HWND_DESKTOP,
                         hwnd,
                         ColorDlgProc,
                         FM3ModHandle,
                         COLOR_FRAME,
                         (PVOID)&co)) {
              WinSendMsg(hwndMLE,
                         MLM_SETTEXTCOLOR,
                         MPFROMLONG(standardcolors[Colors[COLORS_FOREGROUND]]),
                         MPVOID);
              PrfWriteProfileData(fmprof,
                                  FM3Str,
                                  "MLEForegroundcolor",
                                  &Colors[COLORS_FOREGROUND],
                                  sizeof(LONG));
              WinSendMsg(hwndMLE,
                         MLM_SETBACKCOLOR,
                         MPFROMLONG(standardcolors[Colors[COLORS_BACKGROUND]]),
                         MPVOID);
              PrfWriteProfileData(fmprof,
                                  FM3Str,
                                  "MLEBackgroundcolor",
                                  &Colors[COLORS_BACKGROUND],
                                  sizeof(LONG));
            }
          }
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_EDITOR,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case MLE_CODEPAGE:
          vw->cp = PickCodepage(hwnd);
          vw->fattrs.usCodePage = (USHORT)vw->cp;
          WinSendMsg(hwndMLE,
                     MLM_SETFONT,
                     MPFROMP(&vw->fattrs),
                     MPVOID);
          PrfWriteProfileData(fmprof,
                              FM3Str,
                              "MLEFont",
                              &vw->fattrs,
                              sizeof(FATTRS));
          break;

        case MLE_NEWFILE:
          if(!MLEgetreadonly(hwndMLE)) {
            if(vw->ch) {

              APIRET temp;

              temp = saymsg(MB_YESNOCANCEL | MB_ICONEXCLAMATION,
                            hwnd,
                            NullStr,
                            GetPString(IDS_SAVECHANGESTEXT));
              if(temp == MBID_CANCEL)
                break;
              if(temp == MBID_YES)
                WinSendMsg(hwnd,
                           WM_COMMAND,
                           MPFROM2SHORT(MLE_EXPORTFILE,0),
                           MPVOID);
            }
            MLEclearall(hwndMLE);
            *vw->exportfilename = 0;
            vw->ch = FALSE;
            MLEsetchanged(hwndMLE,
                          FALSE);
            WinSendMsg(hwnd,UM_SETUP2,MPVOID,MPVOID);
          }
          break;

        case MLE_TOGGLEREADONLY:
          if(!vw->busy && vw->hex != 1) {  /* I dunno why I gotta reset the colors... */

            BOOL ro;
            LONG fColor,bColor;

            fColor = (LONG)WinSendMsg(hwndMLE,
                                      MLM_QUERYTEXTCOLOR,
                                      MPVOID,
                                      MPVOID);
            bColor = (LONG)WinSendMsg(hwndMLE,
                                      MLM_QUERYBACKCOLOR,
                                      MPVOID,
                                      MPVOID);
            ro = MLEgetreadonly(hwndMLE);
            ro = (ro) ? FALSE : TRUE;
            MLEsetreadonly(hwndMLE,ro);
            disable_menuitem(vw->hwndMenu,
                             MLE_FILEMENU,
                             !ro);
            disable_menuitem(vw->hwndMenu,
                             MLE_CUTCLIP,
                             !ro);
            disable_menuitem(vw->hwndMenu,
                             MLE_PASTECLIP,
                             !ro);
            disable_menuitem(vw->hwndMenu,
                             MLE_CLEAR,
                             !ro);
            disable_menuitem(vw->hwndMenu,
                             MLE_CUTLINE,
                             !ro);
            disable_menuitem(vw->hwndMenu,
                             MLE_BLOCKMENU,
                             !ro);
            WinSendMsg(hwndMLE,
                       MLM_SETTEXTCOLOR,
                       MPFROMLONG(fColor),
                       MPVOID);
            WinSendMsg(hwndMLE,
                       MLM_SETBACKCOLOR,
                       MPFROMLONG(bColor),
                       MPVOID);
          }
          else
            DosBeep(50,100);
          break;

        case IDM_RENAME:
        case MLE_SETEXPORTFILE:
          if(vw && !MLEgetreadonly(hwndMLE)) {

            CHAR filename[1027];

            strcpy(filename,vw->exportfilename);
            if(export_filename(hwnd,filename,1)) {
              if(stricmp(filename,vw->exportfilename)) {
                vw->ch = TRUE;
                MLEsetchanged(hwndMLE,TRUE);
                strcpy(vw->exportfilename,filename);
                WinSendMsg(hwnd,UM_SETUP2,MPVOID,MPVOID);
              }
            }
          }
          break;

        case MLE_EXPORTFILE:
          if(!MLEgetreadonly(hwndMLE)) {

            LONG oldsize;

            if(!*vw->exportfilename ||
               strchr(vw->exportfilename,'?') ||
               strchr(vw->exportfilename,'*')) {
              WinSendMsg(hwnd,
                         WM_COMMAND,
                         MPFROM2SHORT(MLE_SETEXPORTFILE,0),
                         MPVOID);
              if(!*vw->exportfilename ||
                 strchr(vw->exportfilename,'?') ||
                 strchr(vw->exportfilename,'*'))
                break;
            }
            { /* zero file length instead of unlink (protects EAs from loss) */
              FILE *fp;

              fp = fopen(vw->exportfilename,"r+");
              if(fp) {
                oldsize = filelength(fileno(fp));
                DosSetFileSize(fileno(fp),0L);
                fclose(fp);
              }
            }
            if(!MLEexportfile(hwndMLE,
                              vw->exportfilename,
                              vw->ExpandTabs,
                              vw->fStripTrailLines,
                              vw->fStripTrail)) {
              FILE *fp;

              DosBeep(50,100);
              fp = fopen(vw->exportfilename,"r+");
              if(fp) {
                DosSetFileSize(fileno(fp),oldsize);
                fclose(fp);
              }
            }
            else {
              vw->ch = FALSE;
              MLEsetchanged(hwndMLE,FALSE);
            }
          }
          break;

        case IDM_EDIT:
        case IDM_VIEW:
        case MLE_LOADFILE:
          if(vw->ch && !MLEgetreadonly(hwndMLE)) {

            APIRET temp;

            temp = saymsg(MB_YESNO,
                          hwnd,
                          NullStr,
                          GetPString(IDS_LOADOVERTEXT));
            if(temp != MBID_YES)
              break;
          }
          /* intentional fallthru */
        case MLE_INSERTFILE:
          if(!MLEgetreadonly(hwndMLE)) {

            CHAR filename[1027];

            strcpy(filename,vw->importfilename);
            if(insert_filename(hwnd,
                               filename,
                               (SHORT1FROMMP(mp1) == MLE_INSERTFILE) ?
                                FALSE :
                                TRUE,
                               FALSE)) {
              strcpy(vw->importfilename,filename);
              if(SHORT1FROMMP(mp1) == MLE_INSERTFILE)
                MLEinsertfile(hwndMLE,filename);
              else {
//                switch_to(filename);
                if(MLEbackgroundload(hwnd,
                                     UM_CONTAINER_FILLED,
                                     hwndMLE,
                                     filename,
                                     vw->hex) !=
                   -1) {
                  vw->busy = TRUE;
                  WinEnableWindow(vw->hwndMenu,
                                  FALSE);
                }
              }
            }
          }
          break;

        case MLE_STRIPTRAILBLANKS:
          SetMenuCheck(vw->hwndMenu,
                       MLE_STRIPTRAILBLANKS,
                       &vw->fStripTrail,
                       TRUE,
                       "MLEstriptrail");
          break;

        case MLE_STRIPTRAILLINES:
          SetMenuCheck(vw->hwndMenu,
                       MLE_STRIPTRAILLINES,
                       &vw->fStripTrailLines,
                       TRUE,
                       "MLEstriptraillines");
          break;

        case MLE_TAB:
          {
            STRINGINPARMS sip;
            CHAR s[35];

            sip.help = GetPString(IDS_TABSTOPHELPTEXT);
            sip.ret = s;
            sprintf(s,
                    "%u",
                    vw->TabStops);
            sip.prompt = GetPString(IDS_TABSTOPPROMPTTEXT);
            sip.inputlen = 34;
            sip.title = GetPString(IDS_TABSTOPTITLETEXT);
            WinDlgBox(HWND_DESKTOP,
                      hwnd,
                      InputDlgProc,
                      FM3ModHandle,
                      STR_FRAME,
                      &sip);
            if(*s && atoi(s) > -1 && atoi(s) < 512) {
              vw->TabStops = atoi(s);
              WinSendMsg(hwndMLE,
                         MLM_SETTABSTOP,
                         MPFROMLONG(vw->TabStops),
                         MPVOID);
              PrfWriteProfileData(fmprof,
                                  FM3Str,
                                  "MLETabStops",
                                  &vw->TabStops,
                                  sizeof(INT));
            }
          }
          break;

        case MLE_EXPANDTABS:
          {
            BOOL          tempbool;
            STRINGINPARMS sip;
            CHAR          s[35];

            sip.help = GetPString(IDS_EXTABHELPTEXT);
            sip.ret = s;
            sprintf(s,
                    "%u",
                    vw->ExpandTabs);
            sip.prompt = GetPString(IDS_EXTABPROMPTTEXT);
            sip.inputlen = 34;
            sip.title = GetPString(IDS_EXTABTITLETEXT);
            WinDlgBox(HWND_DESKTOP,
                      hwnd,
                      InputDlgProc,
                      FM3ModHandle,
                      STR_FRAME,
                      &sip);
            if(*s && atoi(s) > -1 && atoi(s) < 33) {
              vw->ExpandTabs = atoi(s);
              tempbool = (vw->ExpandTabs != 0);
              SetMenuCheck(vw->hwndMenu,
                           MLE_EXPANDTABS,
                           &tempbool,
                           FALSE,
                           NULL);
              PrfWriteProfileData(fmprof,
                                  FM3Str,
                                  "MLEExpandTabs",
                                  &vw->ExpandTabs,
                                  sizeof(INT));
            }
          }
          break;

        case MLE_APPENDCLIP:
          MLEdoblock(hwndMLE,
                     APPENDCLIP,
                     NULL);
          break;

        case MLE_WRITEBLOCK:
          {
            CHAR filename[1027];

            strcpy(filename,vw->exportfilename);
            if(export_filename(hwnd,filename,1))
              MLEdoblock(hwndMLE,
                         WRITE,
                         filename);
          }
          break;

        case MLE_FORMAT:
          if(!MLEgetreadonly(hwndMLE))
            MLEdoblock(hwndMLE,
                       FORMAT,
                       NULL);
          break;

        case MLE_XOR:
          if(!MLEgetreadonly(hwndMLE))
            MLEdoblock(hwndMLE,
                       XOR,
                       NULL);
          break;

        case MLE_ROT13:
          if(!MLEgetreadonly(hwndMLE))
            MLEdoblock(hwndMLE,
                       ROT13,
                       NULL);
          break;

        case MLE_UPPERCASE:
          if(!MLEgetreadonly(hwndMLE))
            MLEdoblock(hwndMLE,
                       UPPERCASE,
                       NULL);
          break;

        case MLE_LOWERCASE:
          if(!MLEgetreadonly(hwndMLE))
            MLEdoblock(hwndMLE,
                       LOWERCASE,
                       NULL);
          break;

        case MLE_TOGGLECASE:
          if(!MLEgetreadonly(hwndMLE))
            MLEdoblock(hwndMLE,
                       TOGGLECASE,
                       NULL);
          break;

        case MLE_JUMP:
          {
            ULONG         numlines,linenum;
            CHAR          s[35],ss[133];
            STRINGINPARMS sip;

            sip.help = GetPString(IDS_NVLINEJUMPTEXT);
            sip.ret = s;
            *s = 0;
            sip.prompt = ss;
            sip.inputlen = 34;
            sip.title = GetPString(IDS_NVLINEJUMPTITLETEXT);
            numlines = MLEnumlines(hwndMLE);
            if(numlines) {
              sprintf(sip.prompt,
                      GetPString(IDS_NVJUMPTEXT),
                      GetPString(IDS_LINETEXT),
                      1,
                      numlines);
              WinDlgBox(HWND_DESKTOP,
                        hwnd,
                        InputDlgProc,
                        FM3ModHandle,
                        STR_FRAME,
                        &sip);
              if(*s) {
                linenum = atol(s);
                if(linenum > 0 && linenum <= numlines) {
                  MLEsettopline(hwndMLE,
                                MLEstartofline(hwndMLE,
                                               linenum));
                  MLEsetcurpos(hwndMLE,
                               MLEstartofline(hwndMLE,
                                              linenum));
                }
              }
            }
            else
              DosBeep(50,100);
          }
          break;

        case MLE_CUTLINE:          /* delete current line */
          if(!MLEgetreadonly(hwndMLE))
            MLEdeletecurline(hwndMLE);
          break;

        case IDM_DELETE:
        case MLE_CLEAR:
          if(!MLEgetreadonly(hwndMLE))
            MLEclear(hwndMLE);
          break;

        case DID_CANCEL:            /* escape */
          if(MLEgetreadonly(hwndMLE))
            PostMsg(hwnd,
                    WM_CLOSE,
                    MPVOID,
                    MPVOID);
          else
            PostMsg(hwnd,
                    WM_COMMAND,
                    MPFROM2SHORT(MLE_DESELECTALL,0),
                    MPVOID);
          break;

        case MLE_QUIT:             /* outtahere */
          MLEsetchanged(hwndMLE,
                        FALSE);
          vw->ch = FALSE;
          /* intentional fallthru */
        case MLE_END:
          PostMsg(hwnd,
                  WM_CLOSE,
                  MPVOID,
                  MPVOID);
          break;

        case MLE_SETFONT:          /* select a new font */
          SetMLEFont(hwndMLE,
                     &vw->fattrs,
                     0);
          PrfWriteProfileData(fmprof,
                              FM3Str,
                              "MLEFont",
                              &vw->fattrs,
                              sizeof(FATTRS));
          break;

        case MLE_SELECTALL:
          MLEselectall(hwndMLE);
          break;

        case MLE_DESELECTALL:
          MLEsetcurpos(hwndMLE,
                       MLEcurpos(hwndMLE));
          break;

        case MLE_UNDO:
          if(!MLEgetreadonly(hwndMLE))
            MLEundo(hwndMLE);
          break;

        case IDM_SAVETOCLIP:
        case MLE_COPYCLIP:
          MLEcopy(hwndMLE);
          break;

        case MLE_CUTCLIP:
          if(!MLEgetreadonly(hwndMLE))
            MLEcut(hwndMLE);
          break;

        case IDM_COLLECTFROMCLIP:
        case MLE_PASTECLIP:
          if(!MLEgetreadonly(hwndMLE))
            MLEpaste(hwndMLE);
          break;

        case MLE_SENSITIVE:
          SetMenuCheck(vw->hwndMenu,
                       MLE_SENSITIVE,
                       &vw->srch.fInsensitive,
                       TRUE,
                       "MLEInsensitive");
          break;

        case MLE_FINDFIRST:         /* search */
          if(MLEfindfirst(hwnd,&vw->srch))
            PostMsg(hwnd,
                    WM_COMMAND,
                    MPFROM2SHORT(MLE_FINDNEXT,0),
                    MPVOID);
          break;

        case IDM_GREP:
        case MLE_FINDNEXT:
          {
            INT temp;

            temp = MLEfindnext(hwnd,&vw->srch);
            if(temp < 0)
              PostMsg(hwnd,
                      WM_COMMAND,
                      MPFROM2SHORT(MLE_FINDFIRST,0),
                      MPVOID);
            else if(temp > 0)
              PostMsg(hwnd,
                      WM_COMMAND,
                      MPFROM2SHORT(MLE_FINDNEXT,0),
                      MPVOID);
          }
          break;

        case MLE_TOGWRAP:     /* toggle wrap mode */
          if(vw->hex != 1) {
            SetMenuCheck(vw->hwndMenu,
                         MLE_TOGWRAP,
                         &vw->fWrap,
                         TRUE,
                         "MLEWrap");
            MLEsetwrap(hwndMLE,
                       vw->fWrap);
          }
          break;

        case IDM_ABOUT:
        case MLE_ABOUT:
          saymsg(MB_ENTER | MB_ICONASTERISK,
                 hwnd,
                 GetPString(IDS_VIEWABOUTTITLETEXT),
                 GetPString(IDS_VIEWABOUTTEXT));
          break;
      }
      return 0;

    case WM_SAVEAPPLICATION:
      if(vw &&
         ParentIsDesktop(hwnd,vw->hwndParent)) {

        SWP swp;

        WinQueryWindowPos(vw->hwndFrame,
                          &swp);
        if(!(swp.fl & (SWP_HIDE | SWP_MINIMIZE | SWP_MAXIMIZE)))
          PrfWriteProfileData(fmprof,
                              appname,
                              "ViewSizePos",
                              &swp,
                              sizeof(swp));
      }
      break;

    case WM_CLOSE:    /* outtahere */
      WinSendMsg(hwnd,
                 WM_SAVEAPPLICATION,
                 MPVOID,
                 MPVOID);
      if(vw) {
        if(vw->busy) {
          vw->killme = TRUE;
          return 0;
        }
        if(vw->ch &&
           !MLEgetreadonly(hwndMLE)) {

          APIRET temp;

          temp = saymsg(MB_YESNOCANCEL | MB_ICONEXCLAMATION,
                        hwnd,
                        NullStr,
                        GetPString(IDS_SAVECHANGESTEXT));
          if(temp == MBID_CANCEL)
            return 0;
          if(temp == MBID_YES)
            WinSendMsg(hwnd,
                       WM_COMMAND,
                       MPFROM2SHORT(MLE_EXPORTFILE,0),
                       MPVOID);
        }
      }
      WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      return 0;

    case WM_DESTROY:
       DosPostEventSem(CompactSem);
      {
        BOOL dontclose = FALSE;

        if(vw) {
          if(vw->hwndPopupMenu)
            WinDestroyWindow(vw->hwndPopupMenu);
          if(vw->accel)
            WinDestroyAccelTable(vw->accel);
          dontclose = vw->dontclose;
          WinSetWindowPtr(hwnd,QWL_USER,NULL);
          if(vw->hwndRestore) {

            ULONG fl = SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER;
            SWP   swp;

            if(WinQueryWindowPos(vw->hwndRestore,
                                 &swp)) {
              if(!(swp.fl & SWP_MAXIMIZE))
                fl |= SWP_RESTORE;
              WinSetWindowPos(vw->hwndRestore,
                              HWND_TOP,
                              0,
                              0,
                              0,
                              0,
                              fl);
            }
          }
          free(vw);
        }
        if(!dontclose &&
           ParentIsDesktop(hwnd,WinQueryWindow(WinQueryWindow(hwnd,
                           QW_PARENT),QW_PARENT))) {
          if(!PostMsg((HWND)0,
                      WM_QUIT,
                      MPVOID,
                      MPVOID))
            DosExit(EXIT_PROCESS,1);
        }
      }
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}

