#define INCL_DOS
#define INCL_WIN
#define INCL_GPI

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "tools.h"

typedef struct {
  USHORT  size;
  USHORT  dummy;
  HWND    hwndDir1;
  HWND    hwndDir2;
  HWND    hwndCurr;
  HWND    hwndLastDir;
  HWND    hwndMax;
} PERSON1DATA;

extern TOOL  *toolhead;

#pragma alloc_text(PERSON11,MainFrameWndProc2,MainWndProc2)
#pragma alloc_text(PERSON12,StartFM32,MainWMOnce2)
#pragma alloc_text(PERSON13,MainWMCommand2)


MRESULT EXPENTRY MainFrameWndProc2 (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PFNWP oldproc = (PFNWP)WinQueryWindowPtr(hwnd,0);

  switch(msg) {
    case WM_ADJUSTWINDOWPOS:
      {
        SWP *pswp;

        pswp = (SWP *)mp1;
        if(pswp->fl & (SWP_SHOW | SWP_RESTORE))
          PostMsg(WinWindowFromID(hwnd,FID_CLIENT),UM_SIZE,MPVOID,MPVOID);
        if(fDataMin && !fAmClosing) {
          if(pswp->fl & (SWP_HIDE | SWP_MINIMIZE)) {

            SWP swp;

            WinQueryWindowPos(hwnd,&swp);
            PostMsg(hwnd,UM_FOCUSME,MPFROMLONG(swp.fl),MPVOID);
            HideNote();
          }
          else if(pswp->fl & (SWP_SHOW | SWP_RESTORE)) {
            if(DataHwnd)
              PostMsg(DataHwnd,WM_CLOSE,MPVOID,MPVOID);
          }
        }
        if(!fAmClosing) {
          if(pswp->fl & (SWP_HIDE | SWP_MINIMIZE))
            HideNote();
        }
      }
      break;

    case UM_FOCUSME:
      CreateDataBar(HWND_DESKTOP,(ULONG)mp1);
      return 0;

    case WM_BUTTON1UP:
    case WM_BUTTON2UP:
    case WM_BUTTON3UP:
    case WM_MOUSEMOVE:
    case WM_CHORD:
     shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
     break;

    case WM_CHAR:
      shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
      break;

    case WM_CONTROL:
      return WinSendMsg(WinWindowFromID(hwnd,FID_CLIENT),UM_CONTROL,mp1,mp2);

    case WM_COMMAND:
      return WinSendMsg(WinWindowFromID(hwnd,FID_CLIENT),msg,mp1,mp2);

    case WM_CALCFRAMERECT:
      {
        MRESULT mr;
        PRECTL  prectl;
        LONG    sheight = 20,bheight = 20;

        mr = (oldproc) ? oldproc(hwnd,msg,mp1,mp2) :
                         PFNWPFrame(hwnd, msg, mp1, mp2);

        /*
         * Calculate the position of the client rectangle.
         * Otherwise,  we'll see a lot of redraw when we move the
         * client during WM_FORMATFRAME.
         */

        if(mr && mp2) {
          prectl = (PRECTL)mp1;
          if(prectl->yBottom != prectl->yTop) {
            {
              HPS    hps;
              POINTL aptl[TXTBOX_COUNT];

              hps = WinGetPS(hwndStatus);
              GpiQueryTextBox(hps,6,"$`WjgT",TXTBOX_COUNT,aptl);
              WinReleasePS(hps);
              sheight = aptl[TXTBOX_TOPLEFT].y + 6;
              hps = WinGetPS(hwndName);
              GpiQueryTextBox(hps,6,"$`WjgT",TXTBOX_COUNT,aptl);
              WinReleasePS(hps);
              bheight = aptl[TXTBOX_TOPLEFT].y + 6;
            }
            prectl->yBottom += (sheight + 4);
            prectl->yTop -= (sheight + 4);
            prectl->yBottom += (bheight + 4);
            prectl->yTop -= (bheight + 4);
            if(fToolbar) {
              if(!fTextTools)
                prectl->yTop -= ((fToolTitles) ? 50 : 40);
              else
                prectl->yTop -= 32;
            }
            ResizeDrives(WinWindowFromID(hwnd,MAIN_DRIVES),
                         ((prectl->xRight -
                          (WinQuerySysValue(HWND_DESKTOP,
                                            SV_CYSIZEBORDER) * 2))- 4));
            prectl->yTop -= (16 * (DriveLines * 18));
            prectl->yTop -= 2;
            if(fAutoView) {
              AutoviewHeight = min(AutoviewHeight,
                                   (prectl->yTop - prectl->yBottom) - 116);
              AutoviewHeight = max(AutoviewHeight,36);
              prectl->yBottom += (AutoviewHeight + 6);
            }
          }
        }
        return mr;
      }

    case WM_FORMATFRAME:
      {
        SHORT sCount,soldCount;
        PSWP  pswp,pswpClient,pswpNew;
        SWP   swpClient;
        ULONG theight = 48L,dheight = 20,bheight = 20,sheight = 20;
        LONG  width,lastx;

        sCount = (SHORT)((oldproc) ? oldproc(hwnd,msg,mp1,mp2) :
                                     PFNWPFrame(hwnd, msg, mp1, mp2));
        soldCount = sCount;
        /*
         * Reformat the frame to "squeeze" the client
         * and make room for status window sibling beneath
         * and toolbar above (if toolbar's on) and userlists
         * (if userlists are on).
         */

        pswp = (PSWP)mp1;
        {
          SHORT x;

          for(x = 0;x < sCount;x++) {
            if(WinQueryWindowUShort(pswp->hwnd,QWS_ID) == FID_CLIENT) {
              pswpClient = pswp;
              break;
            }
            pswp++;
          }
        }

        {
          HPS    hps;
          POINTL aptl[TXTBOX_COUNT];

          hps = WinGetPS(hwndStatus);
          GpiQueryTextBox(hps,6,"$`WjgT",TXTBOX_COUNT,aptl);
          WinReleasePS(hps);
          bheight = sheight = aptl[TXTBOX_TOPLEFT].y + 6;
          hps = WinGetPS(hwndName);
          GpiQueryTextBox(hps,6,"$`WjgT",TXTBOX_COUNT,aptl);
          WinReleasePS(hps);
          bheight = aptl[TXTBOX_TOPLEFT].y + 6;
        }

        pswpNew = (PSWP)mp1 + soldCount;
        *pswpNew = *pswpClient;
        swpClient = *pswpClient;
        pswpNew->hwnd = hwndStatus;
        pswpNew->x = swpClient.x + 3;
        pswpNew->y = swpClient.y + 2;
        width = (swpClient.cx / 2) - 6;
        width = max(width,10);
        pswpNew->cx = width - 6;
        pswpNew->cy = sheight;
        pswpClient->y = pswpNew->y + pswpNew->cy + 3;
        pswpClient->cy = (swpClient.cy - pswpNew->cy) - 3;
        sCount++;

        pswpNew = (PSWP)mp1 + (soldCount + 1);
        *pswpNew = *pswpClient;
        pswpNew->hwnd = hwndStatus2;
        pswpNew->x = width + 8;
        pswpNew->y = swpClient.y + 2;
        pswpNew->cx = width - 6;
        pswpNew->cy = sheight;
        sCount++;

        if(fToolbar) {
          if(fTextTools)
            theight = 32L;
          else if(!fToolTitles)
            theight = 40L;
          pswpNew = (PSWP)mp1 + (soldCount + 2);
          *pswpNew = *pswpClient;
          pswpNew->hwnd = WinWindowFromID(hwnd,MAIN_TOOLS);
          pswpNew->x = swpClient.x + 2;
          pswpNew->y = (swpClient.y + swpClient.cy) - (theight - 2);
          pswpNew->cx = swpClient.cx - 4;
          pswpNew->cy = theight - 4;
          pswpClient->cy -= theight;
          sCount++;
        }
        else
          WinShowWindow(WinWindowFromID(hwnd,MAIN_TOOLS),FALSE);

        ResizeDrives(WinWindowFromID(hwnd,MAIN_DRIVES),
                     pswpClient->cx - 4);
        pswpNew = (PSWP)mp1 + (soldCount + 2 + (fToolbar != FALSE));
        *pswpNew = *pswpClient;
        pswpNew->hwnd = WinWindowFromID(hwnd,MAIN_DRIVES);
        pswpNew->x = swpClient.x + 2;
        dheight += ((dheight - 2) * DriveLines);
        pswpNew->y = (swpClient.y + swpClient.cy) - (dheight - 2);
        if(fToolbar)
          pswpNew->y -= theight;
        pswpNew->cx = swpClient.cx - 4;
        pswpNew->cy = dheight - 4;
        pswpClient->cy -= dheight;
        pswpClient->cy -= 2;
        sCount++;

        if(fAutoView) {
          pswpNew = (PSWP)mp1 + (soldCount + 3 + (fToolbar != FALSE));
          *pswpNew = *pswpClient;
          pswpNew->hwnd = (fComments) ? hwndAutoMLE : hwndAutoview;
          pswpNew->x = pswpClient->x + 3;
          pswpNew->y = pswpClient->y + 3;
          if(fMoreButtons)
            pswpNew->y += (bheight + 4);
          pswpNew->cx = pswpClient->cx - 6;
          AutoviewHeight = min(AutoviewHeight,
                               pswpClient->cy - 116);
          AutoviewHeight = max(AutoviewHeight,36);
          pswpNew->cy = AutoviewHeight;
          pswpClient->y += (AutoviewHeight + 6);
          pswpClient->cy -= (AutoviewHeight + 6);
          sCount++;
          WinShowWindow((fComments) ? hwndAutoview : hwndAutoMLE,FALSE);
        }
        else {
          WinShowWindow(hwndAutoview,FALSE);
          WinShowWindow(hwndAutoMLE,FALSE);
        }

        pswpNew = (PSWP)mp1 + (soldCount + 3 + (fToolbar != FALSE) +
                                               (fAutoView != FALSE));
        *pswpNew = *pswpClient;
        pswpNew->hwnd = WinWindowFromID(hwnd,MAIN_LED);
        pswpNew->x = (swpClient.x + swpClient.cx) - 12;
        pswpNew->y = swpClient.y;
        pswpNew->cx = 12;
        pswpNew->cy = 12;
        sCount++;

        pswpNew = (PSWP)mp1 + (soldCount + 4 + (fToolbar != FALSE) +
                                               (fAutoView != FALSE));
        *pswpNew = *pswpClient;
        pswpNew->hwnd = WinWindowFromID(hwnd,MAIN_LEDHDR);
        pswpNew->x = (swpClient.x + swpClient.cx) - 12;
        pswpNew->y = swpClient.y + 12;
        pswpNew->cx = 12;
        pswpNew->cy = sheight - 8;
        sCount++;

        pswpNew = (PSWP)mp1 + (soldCount + 5 + (fToolbar != FALSE) +
                                               (fAutoView != FALSE));
        *pswpNew = *pswpClient;
        pswpNew->hwnd = hwndName;
        pswpNew->x = swpClient.x + 3;
        pswpNew->y = swpClient.y + (sheight + 6);
        pswpNew->cx = ((swpClient.cx / 2) + (swpClient.cx / 5)) - 3;
        lastx = pswpNew->x + pswpNew->cx;
        pswpNew->cy = bheight;
        pswpClient->y += (bheight + 4);
        pswpClient->cy -= (bheight + 4);
        sCount++;

        pswpNew = (PSWP)mp1 + (soldCount + 6 + (fToolbar != FALSE) +
                                               (fAutoView != FALSE));
        *pswpNew = *pswpClient;
        pswpNew->hwnd = hwndDate;
        pswpNew->x = lastx + 3;
        pswpNew->y = swpClient.y + (sheight + 6);
        pswpNew->cx = (swpClient.cx / 6) + (swpClient.cx / 16) - 3;
        lastx = pswpNew->x + pswpNew->cx;
        pswpNew->cy = bheight;
        sCount++;

        pswpNew = (PSWP)mp1 + (soldCount + 7 + (fToolbar != FALSE) +
                                               (fAutoView != FALSE));
        *pswpNew = *pswpClient;
        pswpNew->hwnd = hwndAttr;
        pswpNew->x = lastx + 3;
        pswpNew->y = swpClient.y + (sheight + 6);
        pswpNew->cx = (swpClient.cx - pswpNew->x) - 1;
        pswpNew->cy = bheight;
        sCount++;

        return MRFROMSHORT(sCount);
      }

    case WM_QUERYFRAMECTLCOUNT:
      {
        SHORT sCount;

        sCount = (SHORT) ((oldproc) ? oldproc(hwnd,msg,mp1,mp2) :
                                      PFNWPFrame(hwnd, msg, mp1, mp2));
        sCount += 8;
        if(fToolbar)
          sCount++;
        if(fAutoView)
          sCount++;
        return MRFROMSHORT(sCount);
      }

    case WM_CLOSE:
      WinSendMsg(WinWindowFromID(hwnd,FID_CLIENT),msg,mp1,mp2);
      return 0;
  }
  return (oldproc) ? oldproc(hwnd,msg,mp1,mp2) :
                     PFNWPFrame(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY MainWMCommand2 (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PERSON1DATA *pd;

  SetShiftState();
  switch(SHORT1FROMMP(mp1)) {
    case IDM_TOAUTOMLE:
    case IDM_CLI:
    case IDM_CREATETOOL:
    case IDM_ABOUT:
    case IDM_WINFULLSCREEN:
    case IDM_DOSCOMMANDLINE:
    case IDM_INIVIEWER:
    case IDM_EDITASSOC:
    case IDM_COMMANDLINE:
    case IDM_KILLPROC:
    case IDM_TOOLBAR:
    case IDM_TEXTTOOLS:
    case IDM_TOOLTITLES:
    case IDM_TOOLLEFT:
    case IDM_TOOLRIGHT:
    case IDM_AUTOVIEWCOMMENTS:
    case IDM_AUTOVIEWFILE:
    case IDM_AUTOVIEW:
//    case IDM_SYSINFO:
    case IDM_HIDENOTEWND:
    case IDM_SHOWNOTEWND:
    case IDM_INSTANT:
    case IDM_HELPCONTEXT:
    case IDM_HELPHINTS:
    case IDM_HELPPIX:
    case IDM_KILLME:
    case IDM_EXIT:
    case IDM_HELPTUTOR:
    case IDM_HELPCONTENTS:
    case IDM_HELPGENERAL:
    case IDM_HELPKEYS:
    case IDM_HELPMOUSE:
    case IDM_HELP:
      return MainWMCommand(hwnd,msg,mp1,mp2);

    case IDM_REMAP:
      WinDlgBox(HWND_DESKTOP,
                hwnd,
                RemapDlgProc,
                FM3ModHandle,
                MAP_FRAME,
                NULL);
      break;

    case IDM_TWODIRS:
      pd = WinQueryWindowPtr(hwnd,4);
      if(pd) {

        WALK2 wa;

        memset(&wa,0,sizeof(wa));
        wa.size = sizeof(wa);
        *wa.szCurrentPath1 = 0;
        WinSendMsg(pd->hwndDir1,
                   UM_CONTAINERDIR,
                   MPFROMP(wa.szCurrentPath1),
                   MPVOID);
        MakeValidDir(wa.szCurrentPath1);
        *wa.szCurrentPath2 = 0;
        WinSendMsg(pd->hwndDir2,
                   UM_CONTAINERDIR,
                   MPFROMP(wa.szCurrentPath2),
                   MPVOID);
        MakeValidDir(wa.szCurrentPath2);
        if(WinDlgBox(HWND_DESKTOP,
                     hwnd,
                     WalkTwoSetDlgProc,
                     FM3ModHandle,
                     WALK2_FRAME,
                     MPFROMP(&wa)) &&
           (IsRoot(wa.szCurrentPath1) ||
            !IsFile(wa.szCurrentPath1)) &&
           (IsRoot(wa.szCurrentPath2) ||
            !IsFile(wa.szCurrentPath2))) {
          WinSendMsg(WinWindowFromID(WinWindowFromID(pd->hwndDir1,
                                     FID_CLIENT),DIR_CNR),
                     UM_SETDIR,
                     MPFROMP(wa.szCurrentPath1),
                     MPVOID);
          WinSendMsg(WinWindowFromID(WinWindowFromID(pd->hwndDir2,
                                     FID_CLIENT),DIR_CNR),
                     UM_SETDIR,
                     MPFROMP(wa.szCurrentPath2),
                     MPVOID);
        }
      }
      break;

    case IDM_COMPARE:
      pd = WinQueryWindowPtr(hwnd,4);
      if(pd) {

        WALK2 wa;

        memset(&wa,0,sizeof(wa));
        wa.size = sizeof(wa);
        *wa.szCurrentPath1 = 0;
        WinSendMsg(pd->hwndDir1,
                   UM_CONTAINERDIR,
                   MPFROMP(wa.szCurrentPath1),
                   MPVOID);
        MakeValidDir(wa.szCurrentPath1);
        *wa.szCurrentPath2 = 0;
        WinSendMsg(pd->hwndDir2,
                   UM_CONTAINERDIR,
                   MPFROMP(wa.szCurrentPath2),
                   MPVOID);
        MakeValidDir(wa.szCurrentPath2);
        if(WinDlgBox(HWND_DESKTOP,
                     hwnd,
                     WalkTwoCmpDlgProc,
                     FM3ModHandle,
                     WALK2_FRAME,
                     MPFROMP(&wa)) &&
           (IsRoot(wa.szCurrentPath1) ||
            !IsFile(wa.szCurrentPath1)) &&
           (IsRoot(wa.szCurrentPath2) ||
            !IsFile(wa.szCurrentPath2))) {
          if(!*dircompare) {

            COMPARE *cmp;

            cmp = malloc(sizeof(COMPARE));
            if(cmp) {
              memset(cmp,0,sizeof(COMPARE));
              cmp->size = sizeof(COMPARE);
              strcpy(cmp->leftdir,wa.szCurrentPath1);
              strcpy(cmp->rightdir,wa.szCurrentPath2);
              cmp->hwndParent = hwnd;
              cmp->dcd.hwndParent = hwnd;
              WinDlgBox(HWND_DESKTOP,
                        HWND_DESKTOP,
                        CompareDlgProc,
                        FM3ModHandle,
                        COMP_FRAME,
                        MPFROMP(cmp));
            }
          }
          else {

            CHAR *d1 = "\"",*d2 = "\"";

            if(!needs_quoting(wa.szCurrentPath1))
              *d1 = 0;
            if(!needs_quoting(wa.szCurrentPath2))
              *d2 = 0;
            runemf2(SEPARATE,
                    HWND_DESKTOP,
                    NULL,
                    NULL,
                    "%s %s%s%s %s%s%s",
                    dircompare,
                    d1,
                    wa.szCurrentPath1,
                    d1,
                    d2,
                    wa.szCurrentPath2,
                    d2);
          }
        }
      }
      break;

    case IDM_VTREE:
      WinSendMsg(hwnd,
                 UM_SETUP2,
                 MPFROMLONG(1),
                 MPVOID);
      if(hwndTree) {
        WinShowWindow(hwndTree,FALSE);
        PostMsg(hwndTree,
                WM_CLOSE,
                MPVOID,
                MPVOID);
        hwndTree = (HWND)0;
      }
      else {

        ULONG size = sizeof(ULONG);

        hwndTree = StartTreeCnr(hwnd,3);
        PrfQueryProfileData(fmprof,
                            realappname,
                            "FM/4 TreeWidth",
                            (PVOID)&TreeWidth,
                            &size);
        TreeWidth = max(TreeWidth,80);
      }
      {
        BOOL dummy = (hwndTree != (HWND)0);
        PrfWriteProfileData(fmprof,
                            realappname,
                            "FM/4 TreeUp",
                            (PVOID)&dummy,
                            sizeof(dummy));
      }
      PostMsg(hwnd,
              UM_SIZE,
              MPVOID,
              MPVOID);
      break;

    case IDM_TILEBACKWARDS:
      WinSendMsg(hwnd,
                 UM_SETUP2,
                 MPFROMLONG(1),
                 MPVOID);
      fTileBackwards = (fTileBackwards) ? FALSE : TRUE;
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "TileBackwards",
                          (PVOID)&fTileBackwards,
                          sizeof(BOOL));
      PostMsg(hwnd,
              UM_SIZE,
              MPVOID,
              MPVOID);
      break;

    case IDM_NEXTWINDOW:
    case IDM_PREVWINDOW:
      pd = WinQueryWindowPtr(hwnd,4);
      if(pd) {

        HWND hwndFocus;

        if(hwndTree) {
          if(pd->hwndMax) {
            if(hwndTree == pd->hwndCurr)
              hwndFocus = pd->hwndMax;
            else
              hwndFocus = hwndTree;
          }
          else {
            if(hwndTree == pd->hwndCurr)
              hwndFocus = (SHORT1FROMMP(mp1) == IDM_PREVWINDOW) ?
                           pd->hwndDir1 : pd->hwndDir2;
            else if(pd->hwndDir1 == pd->hwndCurr)
              hwndFocus = (SHORT1FROMMP(mp1) == IDM_PREVWINDOW) ?
                           pd->hwndDir2 : hwndTree;
            else
              hwndFocus = (SHORT1FROMMP(mp1) == IDM_PREVWINDOW) ?
                           hwndTree : pd->hwndDir1;
          }
        }
        else {
          if(pd->hwndMax)
            hwndFocus = pd->hwndMax;
          else
            hwndFocus = (pd->hwndCurr == pd->hwndDir1) ?
                         pd->hwndDir2 : pd->hwndDir1;
        }
        WinSetFocus(HWND_DESKTOP,hwndFocus);
      }
      break;

    case IDM_NOTEBOOK:
      WinDlgBox(HWND_DESKTOP,
                hwnd,
                CfgDlgProc,
                FM3ModHandle,
                CFG_FRAME,
                MPFROMP(realappname));
      PostMsg(hwnd,
              UM_SIZE,
              MPVOID,
              MPVOID);
      break;

    case IDM_SEEALL:
    case IDM_GREP:
    case IDM_COLLECTOR:
      {
        HWND hwndC;

        hwndC = StartCollector(HWND_DESKTOP,4);
        if(hwndC) {
          WinSetWindowPos(hwndC,
                          HWND_TOP,
                          0,
                          0,
                          0,
                          0,
                          SWP_SHOW | SWP_RESTORE | SWP_ACTIVATE);
          if(SHORT1FROMMP(mp1) == IDM_GREP)
            PostMsg(WinWindowFromID(hwndC,FID_CLIENT),
                    WM_COMMAND,
                    MPFROM2SHORT(IDM_GREP,0),
                    MPVOID);
          if(SHORT1FROMMP(mp1) == IDM_SEEALL)
            PostMsg(WinWindowFromID(hwndC,FID_CLIENT),
                    WM_COMMAND,
                    MPFROM2SHORT(IDM_SEEALL,0),
                    MPVOID);
        }
      }
      break;

    case IDM_EDITCOMMANDS:
      EditCommands(hwnd);
      break;

    default:
      if(SHORT1FROMMP(mp1) >= IDM_SWITCHSTART &&
         SHORT1FROMMP(mp1) < IDM_SWITCHSTART + 499) {
        if(SHORT1FROMMP(mp1) - IDM_SWITCHSTART < numswitches)
          WinSwitchToProgram(switches[SHORT1FROMMP(mp1) -
                                      IDM_SWITCHSTART]);
        break;
      }
      else if(SHORT1FROMMP(mp1) >= IDM_COMMANDSTART &&
              SHORT1FROMMP(mp1) < IDM_QUICKTOOLSTART) {

        INT  x;
        HWND hwndCnr;

        if(!cmdloaded)
          load_commands();
        hwndCnr = TopWindow(hwnd,(HWND)0);
        hwndCnr = (HWND)WinSendMsg(WinWindowFromID(hwndCnr,FID_CLIENT),
                                   UM_CONTAINERHWND,
                                   MPVOID,
                                   MPVOID);
        if(!hwndCnr) {
          DosBeep(50,100);
          break;
        }
        x = SHORT1FROMMP(mp1) - IDM_COMMANDSTART;
        if(x >= 0) {
          x++;
          RunCommand(hwndCnr,x);
          if(fUnHilite) {

            PCNRITEM    pci;
            DIRCNRDATA *dcd = NULL;

            if(WinQueryWindowUShort(hwndCnr,QWS_ID) != TREE_CNR)
              dcd = INSTDATA(hwndCnr);
            pci = (PCNRITEM)WinSendMsg(hwndCnr,
                                       CM_QUERYRECORDEMPHASIS,
                                       MPFROMLONG(CMA_FIRST),
                                       MPFROMSHORT(CRA_CURSORED));
            if(pci && (INT)pci != -1 &&
               (pci->rc.flRecordAttr & CRA_SELECTED))
              UnHilite(hwnd,
                       TRUE,
                       ((dcd) ? &dcd->lastselection : NULL));
          }
        }
      }
      else if(SHORT1FROMMP(mp1) >= IDM_QUICKTOOLSTART &&
            SHORT1FROMMP(mp1) < IDM_QUICKTOOLSTART + 50) {
        if(!qtloaded)
          load_quicktools();
        if(quicktool[SHORT1FROMMP(mp1) - IDM_QUICKTOOLSTART]) {
          if(fToolsChanged)
            save_tools(NULL);
          if(!load_tools(quicktool[SHORT1FROMMP(mp1) - IDM_QUICKTOOLSTART]))
            load_tools(NULL);
          else {
            strcpy(lasttoolbox,quicktool[SHORT1FROMMP(mp1) - IDM_QUICKTOOLSTART]);
            PrfWriteProfileString(fmprof,
                                  FM3Str,
                                  "LastToolBox",
                                  lasttoolbox);
          }
          BuildTools(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                     MAIN_TOOLS),
                     TRUE);
        }
      }
      else {

        HWND hwndActive;

        hwndActive = TopWindow(hwnd,(HWND)0);
        if(hwndActive)
          PostMsg(WinWindowFromID(hwndActive,FID_CLIENT),
                  msg,
                  mp1,
                  mp2);
      }
      break;
  }
  return 0;
}


MRESULT EXPENTRY MainWMOnce2 (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PERSON1DATA *pd;

  switch(msg) {
    case WM_CREATE:
      {
        TID tid;

        WinQueryWindowProcess(hwnd,&mypid,&tid);
      }
      hwndMain = hwnd;
      WinSetWindowUShort(hwnd,8,0);
      WinSetWindowUShort(hwnd,10,0);
      WinSetWindowUShort(hwnd,12,0);
      WinSetWindowUShort(hwnd,16,0);
      if(_beginthread(MakeMainObjWin,
                      NULL,
                      245760,
                      MPVOID) == -1) {
        PostMsg(hwnd,
                WM_CLOSE,
                MPVOID,
                MPVOID);
        return 0;
      }
      else
        DosSleep(64);

      pd = malloc(sizeof(PERSON1DATA));
      if(!pd)
        WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      else {
        memset(pd,0,sizeof(PERSON1DATA));
        pd->size = sizeof(PERSON1DATA);
        WinSetWindowPtr(hwnd,4,(PVOID)pd);
      }
      {
        SWP   swp;
        PFNWP oldproc;

        /*
         * create frame children (not client children, frame children)
         */
        DosSleep(1L);
        WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),&swp);
        oldproc = WinSubclassWindow(WinQueryWindow(hwnd,QW_PARENT),
                                    (PFNWP)MainFrameWndProc2);
        if(oldproc)
          WinSetWindowPtr(WinQueryWindow(hwnd,QW_PARENT),
                          0,
                          (PVOID)oldproc);
        CommonCreateMainChildren(hwnd,&swp);

        {
          HWND hwndMenu;

          hwndMenu = WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                     FID_MENU);
          WinSetWindowULong(hwnd,
                            0,
                            hwndMenu);
          SetConditionalCascade(hwndMenu,
                                IDM_COMMANDSMENU,
                                IDM_DOITYOURSELF);
          SetConditionalCascade(hwndMenu,
                                IDM_TOOLSUBMENU,
                                IDM_TOOLBAR);
        }
      }
      WinSetWindowText(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                       FID_TITLEBAR),
                       "FM/2 Lite");
      FixSwitchList(WinQueryWindow(hwnd,QW_PARENT),
                    NULL);
      SetSysMenu(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                 FID_SYSMENU));
      break;

    case UM_SETUP:
      pd = WinQueryWindowPtr(hwnd,4);
      if(pd) {

        CHAR        s[CCHMAXPATH];
        ULONG       size;
        DIRCNRDATA *dcd;
        HWND        hwndC;
        BOOL        dummy = TRUE;

        size  = sizeof(BOOL);
        PrfQueryProfileData(fmprof,
                            realappname,
                            "FM/4 TreeUp",
                            (PVOID)&dummy,
                            &size);
        if(dummy) {
          size = sizeof(ULONG);
          hwndTree = StartTreeCnr(hwnd,3);
          PrfQueryProfileData(fmprof,
                              realappname,
                              "FM/4 TreeWidth",
                              (PVOID)&TreeWidth,
                              &size);
          TreeWidth = max(TreeWidth,80);
        }
        size = sizeof(BOOL);
        if(PrfQueryProfileData(fmprof,
                               FM3Str,
                               "Toolbar",
                               &dummy,
                               &size) &&
           size && !dummy)
          WinSendMsg(hwnd,
                     WM_COMMAND,
                     MPFROM2SHORT(IDM_TOOLBAR,0),
                     MPVOID);

        size = sizeof(s);
        *s = 0;
        if(PrfQueryProfileData(fmprof,
                               realappname,
                               "FM/4 Dir1",
                               s,
                               &size) &&
           *s)
          MakeValidDir(s);
        else
          save_dir(s);
        pd->hwndLastDir = pd->hwndCurr = pd->hwndDir1 =
          StartDirCnr(hwnd,s,(HWND)0,3);
        size = sizeof(s);
        *s = 0;
        if(PrfQueryProfileData(fmprof,
                               realappname,
                               "FM/4 Dir2",
                               s,
                               &size) &&
           *s)
          MakeValidDir(s);
        else
          save_dir(s);
        pd->hwndDir2 = StartDirCnr(hwnd,s,(HWND)0,3);
        WinSetFocus(HWND_DESKTOP,
                    pd->hwndCurr);

        hwndC = WinWindowFromID(pd->hwndDir1,FID_CLIENT);
        if(hwndC) {
          dcd = WinQueryWindowPtr(WinWindowFromID(hwndC,DIR_CNR),0);
          if(dcd) {
            size = sizeof(INT);
            if(PrfQueryProfileData(fmprof,
                                   realappname,
                                   "FM/4 Dir1.Sort",
                                   (PVOID)&dcd->sortFlags,
                                   &size) &&
               size == sizeof(INT)) {
              if(!dcd->sortFlags)
                dcd->sortFlags = SORT_PATHNAME;
            }
            size = sizeof(MASK);
            if(PrfQueryProfileData(fmprof,
                                   realappname,
                                   "FM/4 Dir1.Filter",
                                   (PVOID)&dcd->mask,
                                   &size) &&
               size) {
              if(*dcd->mask.szMask)
                WinSendMsg(WinWindowFromID(hwndC,DIR_CNR),
                           UM_FILTER,
                           MPFROMP(dcd->mask.szMask),
                           MPVOID);
            }
            *(dcd->mask.prompt) = 0;
            size = sizeof(ULONG);
            if(PrfQueryProfileData(fmprof,
                                   realappname,
                                   "FM/4 Dir1.View",
                                   (PVOID)&dcd->flWindowAttr,
                                   &size) &&
               size == sizeof(ULONG)) {

              CNRINFO cnri;

              memset(&cnri,0,sizeof(CNRINFO));
              cnri.cb = sizeof(CNRINFO);
              if(WinSendMsg(WinWindowFromID(hwndC,DIR_CNR),
                            CM_QUERYCNRINFO,
                            MPFROMP(&cnri),
                            MPFROMLONG(sizeof(CNRINFO)))) {
                cnri.flWindowAttr = dcd->flWindowAttr;
                WinSendMsg(WinWindowFromID(hwndC,DIR_CNR),
                           CM_SETCNRINFO,
                           MPFROMP(&cnri),
                           MPFROMLONG(CMA_FLWINDOWATTR));
              }
            }
          }
        }

        hwndC = WinWindowFromID(pd->hwndDir2,FID_CLIENT);
        if(hwndC) {
          dcd = WinQueryWindowPtr(WinWindowFromID(hwndC,DIR_CNR),0);
          if(dcd) {
            size = sizeof(INT);
            if(PrfQueryProfileData(fmprof,
                                   realappname,
                                   "FM/4 Dir2.Sort",
                                   (PVOID)&dcd->sortFlags,
                                   &size) &&
               size == sizeof(INT)) {
              if(!dcd->sortFlags)
                dcd->sortFlags = SORT_PATHNAME;
            }
            size = sizeof(MASK);
            if(PrfQueryProfileData(fmprof,
                                   realappname,
                                   "FM/4 Dir2.Filter",
                                   (PVOID)&dcd->mask,
                                   &size) &&
               size) {
              if(*dcd->mask.szMask)
                WinSendMsg(WinWindowFromID(hwndC,DIR_CNR),
                           UM_FILTER,
                           MPFROMP(dcd->mask.szMask),
                           MPVOID);
            }
            *(dcd->mask.prompt) = 0;
            size = sizeof(ULONG);
            if(PrfQueryProfileData(fmprof,
                                   realappname,
                                   "FM/4 Dir2.View",
                                   (PVOID)&dcd->flWindowAttr,
                                   &size) &&
               size == sizeof(ULONG)) {

              CNRINFO cnri;

              memset(&cnri,0,sizeof(CNRINFO));
              cnri.cb = sizeof(CNRINFO);
              if(WinSendMsg(WinWindowFromID(hwndC,DIR_CNR),
                            CM_QUERYCNRINFO,
                            MPFROMP(&cnri),
                            MPFROMLONG(sizeof(CNRINFO)))) {
                cnri.flWindowAttr = dcd->flWindowAttr;
                WinSendMsg(WinWindowFromID(hwndC,DIR_CNR),
                           CM_SETCNRINFO,
                           MPFROMP(&cnri),
                           MPFROMLONG(CMA_FLWINDOWATTR));
              }
            }
          }
        }
      }
      {
        ULONG which = 0,size = sizeof(ULONG);

        if(PrfQueryProfileData(fmprof,
                               realappname,
                               "FM/4 Max",
                               (PVOID)&which,
                               &size) &&
           size == sizeof(ULONG) &&
           which) {
          PostMsg(hwnd,
                  UM_MAXIMIZE,
                  MPFROMLONG(((which == 1) ?
                              pd->hwndDir1 :
                              pd->hwndDir2)),
                  MPVOID);
        }
      }
      PostMsg(hwnd,
              UM_SIZE,
              MPVOID,
              MPVOID);
      if(!hwndTree)
        PostMsg(hwnd,
                UM_BUILDDRIVES,
                MPVOID,
                MPVOID);
      load_tools(NULL);
      BuildTools(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                 MAIN_TOOLS),
                 TRUE);
      fRunning = TRUE;
      return 0;

    case WM_SAVEAPPLICATION:
      WinStoreWindowPos(FM2Str,
                        "MainWindowPos2",
                        WinQueryWindow(hwnd,QW_PARENT));
      pd = WinQueryWindowPtr(hwnd,4);
      if(pd) {

        CHAR        s[CCHMAXPATH];
        HWND        hwndC;
        DIRCNRDATA *dcd;
        ULONG       flWindowAttr;

        *s = 0;
        WinSendMsg(pd->hwndDir1,
                   UM_CONTAINERDIR,
                   MPFROMP(s),
                   MPVOID);
        if(*s) {
          MakeValidDir(s);
          PrfWriteProfileString(fmprof,
                                realappname,
                                "FM/4 Dir1",
                                s);
        }
        *s = 0;
        WinSendMsg(pd->hwndDir2,
                   UM_CONTAINERDIR,
                   MPFROMP(s),
                   MPVOID);
        if(*s) {
          MakeValidDir(s);
          PrfWriteProfileString(fmprof,
                                realappname,
                                "FM/4 Dir2",
                                s);
        }
        flWindowAttr = (pd->hwndMax == pd->hwndDir1) ?
                 1 :
                 (pd->hwndMax == pd->hwndDir2) ?
                  2 :
                  0;
        PrfWriteProfileData(fmprof,
                            realappname,
                            "FM/4 Max",
                            &flWindowAttr,
                            sizeof(flWindowAttr));
        hwndC = WinWindowFromID(pd->hwndDir1,FID_CLIENT);
        if(hwndC) {
          dcd = WinQueryWindowPtr(WinWindowFromID(hwndC,DIR_CNR),0);
          if(dcd) {
            flWindowAttr = dcd->flWindowAttr;
            if(!fLeaveTree && (flWindowAttr & CV_TREE)) {
              flWindowAttr &= (~(CV_TREE | CV_ICON | CV_DETAIL | CV_TEXT));
              if(dcd->lastattr) {
                if(dcd->lastattr & CV_TEXT)
                  flWindowAttr |= CV_TEXT;
                else if(dcd->lastattr & CV_DETAIL)
                  flWindowAttr |= CV_DETAIL;
                else if(dcd->lastattr & CV_ICON)
                  flWindowAttr |= CV_ICON;
                else
                  flWindowAttr |= CV_NAME;
              }
              else
                flWindowAttr |= CV_NAME;
            }
            PrfWriteProfileData(fmprof,
                                realappname,
                                "FM/4 Dir1.Sort",
                                (PVOID)&dcd->sortFlags,
                                sizeof(INT));
            PrfWriteProfileData(fmprof,
                                realappname,
                                "FM/4 Dir1.Filter",
                                (PVOID)&dcd->mask,
                                sizeof(MASK));
            PrfWriteProfileData(fmprof,
                                realappname,
                                "FM/4 Dir1.View",
                                (PVOID)&flWindowAttr,
                                sizeof(ULONG));
          }
        }

        hwndC = WinWindowFromID(pd->hwndDir2,FID_CLIENT);
        if(hwndC) {
          dcd = WinQueryWindowPtr(WinWindowFromID(hwndC,DIR_CNR),0);
          if(dcd) {
            flWindowAttr = dcd->flWindowAttr;
            if(!fLeaveTree && (flWindowAttr & CV_TREE)) {
              flWindowAttr &= (~(CV_TREE | CV_ICON | CV_DETAIL | CV_TEXT));
              if(dcd->lastattr) {
                if(dcd->lastattr & CV_TEXT)
                  flWindowAttr |= CV_TEXT;
                else if(dcd->lastattr & CV_DETAIL)
                  flWindowAttr |= CV_DETAIL;
                else if(dcd->lastattr & CV_ICON)
                  flWindowAttr |= CV_ICON;
                else
                  flWindowAttr |= CV_NAME;
              }
              else
                flWindowAttr |= CV_NAME;
            }
            PrfWriteProfileData(fmprof,
                                realappname,
                                "FM/4 Dir2.Sort",
                                (PVOID)&dcd->sortFlags,
                                sizeof(INT));
            PrfWriteProfileData(fmprof,
                                realappname,
                                "FM/4 Dir2.Filter",
                                (PVOID)&dcd->mask,
                                sizeof(MASK));
            PrfWriteProfileData(fmprof,
                                realappname,
                                "FM/4 Dir2.View",
                                (PVOID)&flWindowAttr,
                                sizeof(ULONG));
          }
        }
      }
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY MainWndProc2 (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PERSON1DATA *pd;

  switch(msg) {
    case WM_SAVEAPPLICATION:
    case UM_SETUP:
    case WM_CREATE:
      return MainWMOnce2(hwnd,msg,mp1,mp2);

    case UM_THREADUSE:
    case UM_LOADFILE:
    case UM_BUILDDRIVES:
      return CommonMainWndProc(hwnd,msg,mp1,mp2);

    case UM_SETUP2:
      pd = WinQueryWindowPtr(hwnd,4);
      if(pd) {
        if(mp1) {
          if(pd->hwndDir1 &&
             (!pd->hwndMax || pd->hwndMax == pd->hwndDir1))
            BoxWindow(pd->hwndDir1,
                      (HPS)0,
                      CLR_PALEGRAY);
          if(pd->hwndDir2 &&
             (!pd->hwndMax || pd->hwndMax == pd->hwndDir2))
            BoxWindow(pd->hwndDir2,
                      (HPS)0,
                      CLR_PALEGRAY);
          if(hwndTree)
            BoxWindow(hwndTree,
                      (HPS)0,
                      CLR_PALEGRAY);
        }
        else {
          if(hwndTree)
            BoxWindow(hwndTree,
                      (HPS)0,
                      (pd->hwndCurr == hwndTree) ?
                       CLR_RED :
                       CLR_WHITE);
          if(!pd->hwndMax ||
             pd->hwndMax == pd->hwndDir1)
            BoxWindow(pd->hwndDir1,
                      (HPS)0,
                      (pd->hwndDir1 == pd->hwndCurr) ?
                       CLR_RED :
                       (pd->hwndDir1 == pd->hwndLastDir) ?
                        CLR_DARKRED :
                        CLR_PALEGRAY);
          if(!pd->hwndMax ||
             pd->hwndMax == pd->hwndDir2)
            BoxWindow(pd->hwndDir2,
                      (HPS)0,
                      (pd->hwndDir2 == pd->hwndCurr) ?
                       CLR_RED :
                       (pd->hwndDir2 == pd->hwndLastDir) ?
                        CLR_DARKRED :
                        CLR_PALEGRAY);
        }
      }
      return 0;

    case WM_BUTTON1DOWN:
      shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
      if(hwndTree) {

        SWP swp;

        WinQueryWindowPos(hwndTree,&swp);
        if(SHORT1FROMMP(mp1) > (swp.x + swp.cx) - 3 &&
           SHORT1FROMMP(mp1) < (swp.x + swp.cx) + 3) {

          SWP       swpC;
          TRACKINFO track;

          WinQueryWindowPos(hwnd,&swpC);
          track.cxBorder = 4;
          track.cyBorder = 4;
          track.cxGrid = 1;
          track.cyGrid = 1;
          track.cxKeyboard = 8;
          track.cyKeyboard = 8;
          track.rclTrack.yBottom = 2;
          track.rclTrack.yTop = swpC.cy - 4;
          track.rclTrack.xLeft = 2;
          track.rclTrack.xRight = swp.x + swp.cx + 2;
          track.rclBoundary = track.rclTrack;
          track.rclBoundary.xRight = swpC.cx - 80;
          track.ptlMinTrackSize.x = 80;
          track.ptlMinTrackSize.y = swpC.cy - 4;
          track.ptlMaxTrackSize.x = swpC.cx - 80;
          track.ptlMaxTrackSize.y = swpC.cy - 4;
          track.fs = TF_RIGHT;
          if(WinTrackRect(hwnd,(HPS)0,&track)) {
            TreeWidth = track.rclTrack.xRight - track.rclTrack.xLeft;
            PrfWriteProfileData(fmprof,
                                realappname,
                                "FM/4 TreeWidth",
                                &TreeWidth,
                                sizeof(ULONG));
            WinSendMsg(hwnd,
                       UM_SETUP2,
                       MPFROMLONG(1),
                       MPVOID);
            PostMsg(hwnd,
                    UM_SIZE,
                    MPVOID,
                    MPVOID);
          }
          return (MRESULT)TRUE;
        }
      }
      break;

    case WM_MOUSEMOVE:
      shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
      if(hwndTree) {

        SWP swp;

        if(WinQueryWindowPos(hwndTree,&swp)) {
          if(SHORT1FROMMP(mp1) > (swp.x + swp.cx) - 3 &&
             SHORT1FROMMP(mp1) < (swp.x + swp.cx) + 3)
            WinSetPointer(HWND_DESKTOP,hptrEW);
          else
            WinSetPointer(HWND_DESKTOP,hptrArrow);
        }
        else
          WinSetPointer(HWND_DESKTOP,hptrArrow);
      }
      return (MRESULT)TRUE;

    case WM_BUTTON1UP:
    case WM_BUTTON2UP:
    case WM_BUTTON3UP:
    case WM_CHAR:
      shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
      break;

    case UM_ADVISEFOCUS:
      pd = WinQueryWindowPtr(hwnd,4);
      if(mp1 &&
         pd &&
         (HWND)mp1 != pd->hwndCurr) {
        if((HWND)mp1 != hwndTree)
          pd->hwndLastDir = (HWND)mp1;
        pd->hwndCurr = (HWND)mp1;
        WinSendMsg(hwnd,
                   UM_SETUP2,
                   MPVOID,
                   MPVOID);
      }
      return 0;

    case UM_MAXIMIZE:
      if(mp1) {
        pd = WinQueryWindowPtr(hwnd,4);
        if(pd) {
          WinSendMsg(hwnd,
                     UM_SETUP2,
                     MPFROMLONG(1),
                     MPVOID);
          if(pd->hwndMax != (HWND)mp1) {
            pd->hwndMax = (HWND)mp1;
            WinSetFocus(HWND_DESKTOP,
                        pd->hwndMax);
          }
          else
            pd->hwndMax = (HWND)0;
          PostMsg(hwnd,
                  UM_SIZE,
                  MPVOID,
                  MPVOID);
        }
      }
      return 0;

    case WM_INITMENU:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_FILESMENU:
        case IDM_VIEWSMENU:
        case IDM_DETAILSSETUP:
        case IDM_COMMANDSMENU:
        case IDM_SORTSUBMENU:
          pd = WinQueryWindowPtr(hwnd,4);
          if(pd)
            WinSendMsg(pd->hwndCurr,
                       UM_INITMENU,
                       mp1,
                       mp2);
          break;
        case IDM_CONFIGMENU:
          WinCheckMenuItem((HWND)mp2,
                           IDM_TOOLSUBMENU,
                           fToolbar);
          WinCheckMenuItem((HWND)mp2,
                           IDM_AUTOVIEW,
                           fAutoView);
          break;
        case IDM_TOOLSUBMENU:
          WinCheckMenuItem((HWND)mp2,
                           IDM_TOOLBAR,
                           fToolbar);
          WinCheckMenuItem((HWND)mp2,
                           IDM_TEXTTOOLS,
                           fTextTools);
          WinCheckMenuItem((HWND)mp2,
                           IDM_TOOLTITLES,
                           fToolTitles);
          break;
        case IDM_WINDOWSMENU:
          WinCheckMenuItem((HWND)mp2,
                           IDM_VTREE,
                           (hwndTree != (HWND)0));
          WinCheckMenuItem((HWND)mp2,
                           IDM_TILEBACKWARDS,
                           fTileBackwards);
          SetupWinList((HWND)mp2,
                       hwnd,
                       WinQueryWindow(hwnd,QW_PARENT));
          break;
      }
      break;

    case WM_SETFOCUS:
      if(mp2)
        PostMsg(hwnd,
                UM_FOCUSME,
                MPVOID,
                MPVOID);
      break;

    case UM_FOCUSME:
      pd = WinQueryWindowPtr(hwnd,4);
      if(pd)
        WinSetFocus(HWND_DESKTOP,
                    pd->hwndCurr);
      return 0;

    case UM_RESCAN:
      return 0;

    case UM_SIZE:
      {
        SWP swp;

        if(WinQueryWindowPos(hwnd,&swp)) {
          mp1 = MPFROM2SHORT(swp.cx,swp.cy);
          mp2 = MPFROM2SHORT(swp.cx,swp.cy);
        }
        else
          return 0;
      }
      /* intentional fallthru */
    case WM_SIZE:
      pd = WinQueryWindowPtr(hwnd,4);
      if(mp1 &&
         mp2 &&
         pd &&
         pd->hwndDir1 &&
         pd->hwndDir2) {
        if(hwndTree)
          WinSetWindowPos(hwndTree,
                          HWND_TOP,
                          2,
                          2,
                          TreeWidth - 4,
                          SHORT2FROMMP(mp2) - 4,
                          SWP_SHOW | SWP_MOVE | SWP_SIZE);
        else
          TreeWidth = 0;
        if(!pd->hwndMax) {
          if(fTileBackwards) {
            WinSetWindowPos(pd->hwndDir1,HWND_TOP,
                            (((SHORT1FROMMP(mp2) - TreeWidth) / 2) +
                              TreeWidth) + 2,
                            2,
                            ((SHORT1FROMMP(mp2) - TreeWidth) / 2) - 4,
                            SHORT2FROMMP(mp2) - 4,
                            SWP_SHOW | SWP_MOVE | SWP_SIZE);
            WinSetWindowPos(pd->hwndDir2,HWND_TOP,
                            TreeWidth + 2,
                            2,
                            ((SHORT1FROMMP(mp2) - TreeWidth) / 2) - 4,
                            SHORT2FROMMP(mp2) - 4,
                            SWP_SHOW | SWP_MOVE | SWP_SIZE);
          }
          else {
            WinSetWindowPos(pd->hwndDir1,HWND_TOP,
                            TreeWidth + 2,
                            2,
                            (SHORT1FROMMP(mp2) - TreeWidth) - 4,
                            (SHORT2FROMMP(mp2) / 2) - 4,
                            SWP_SHOW | SWP_MOVE | SWP_SIZE);
            WinSetWindowPos(pd->hwndDir2,HWND_TOP,
                            TreeWidth + 2,
                            (SHORT2FROMMP(mp2) / 2) + 2,
                            (SHORT1FROMMP(mp2) - TreeWidth) - 4,
                            (SHORT2FROMMP(mp2) / 2) - 4,
                            SWP_SHOW | SWP_MOVE | SWP_SIZE);
          }
        }
        else {

          HWND hwndOther;

          WinSetWindowPos(pd->hwndMax,HWND_TOP,
                          TreeWidth + 2,
                          2,
                          (SHORT1FROMMP(mp2) - TreeWidth) - 4,
                          SHORT2FROMMP(mp2) - 4,
                          SWP_SHOW | SWP_MOVE | SWP_SIZE);
          hwndOther = (pd->hwndMax == pd->hwndDir1) ?
                       pd->hwndDir2 : pd->hwndDir1;
          WinSetWindowPos(hwndOther,HWND_BOTTOM,0,0,0,0,SWP_HIDE);
        }
        WinSendMsg(hwnd,
                   UM_SETUP2,
                   MPVOID,
                   MPVOID);
      }
      if(msg == UM_SIZE)
        return 0;
      break;

    case WM_ERASEBACKGROUND:
      WinFillRect((HPS)mp1,
                  (PRECTL)mp2,
                  0x00d0d0d0);
      return 0;

    case WM_PAINT:
      {
        HPS    hps;
        RECTL  rcl;

        hps = WinBeginPaint(hwnd,
                            NULLHANDLE,
                            NULL);
        if(hps) {
          WinQueryWindowRect(hwnd,&rcl);
          WinFillRect(hps,
                      (PRECTL)&rcl,
                      CLR_PALEGRAY);
          WinEndPaint(hps);
          WinSendMsg(hwnd,
                     UM_SETUP2,
                     MPVOID,
                     MPVOID);
        }
      }
      break;

    case UM_COMMAND:
    case WM_COMMAND:
      return MainWMCommand2(hwnd,msg,mp1,mp2);

    case WM_CLOSE:
      fAmClosing = TRUE;
      WinSendMsg(hwnd,
                 WM_SAVEAPPLICATION,
                 MPVOID,
                 MPVOID);
      CloseChildren(hwnd);
      PostMsg(hwnd,
              UM_CLOSE,
              MPVOID,
              MPVOID);
      DosSleep(1L);
      return 0;

    case UM_CLOSE:
      WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      return 0;

    case WM_DESTROY:
      hwndMain = (HWND)0;
      pd = WinQueryWindowPtr(hwnd,4);
      if(pd)
        free(pd);
      if(!PostMsg((HWND)0,
                  WM_QUIT,
                  MPVOID,
                  MPVOID))
        WinSendMsg((HWND)0,
                   WM_QUIT,
                   MPVOID,
                   MPVOID);
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


HWND StartFM32 (HAB hab,INT argc,CHAR **argv) {

  HWND  hwndFrame,hwndClient;
  ULONG FrameFlags = FCF_TITLEBAR      | FCF_SYSMENU      |
                     FCF_SIZEBORDER    | FCF_MINMAX       |
                     FCF_ACCELTABLE    | FCF_MENU         |
                     FCF_ICON          | FCF_TASKLIST     |
                     FCF_NOBYTEALIGN;

  {
    INT x;

    for(x = 1;x < argc;x++) {
      if(*argv[x] == '+' && !argv[x][1])
        fLogFile = TRUE;
      if(*argv[x] == '-') {
        if(argv[x][1])
          strcpy(profile,&argv[x][1]);
      }
    }
  }
  {
    CHAR        inipath[CCHMAXPATH],fullpath[CCHMAXPATH];
    FILESTATUS3 fsa;

    if(PrfQueryProfileString(HINI_USERPROFILE,
                             FM2Str,
                             "Home",
                             NULL,
                             inipath,
                             sizeof(inipath))) {
      if(!DosQueryPathInfo(inipath,
                           FIL_STANDARD,
                           &fsa,
                           (ULONG)sizeof(fsa))) {
        if(fsa.attrFile & FILE_DIRECTORY) {
          if(DosQueryPathInfo(inipath,
                              FIL_QUERYFULLNAME,
                              fullpath,
                              sizeof(fullpath)))
            strcpy(fullpath,inipath);
          switch_to(fullpath);
        }
      }
    }
  }

  hwndFrame = WinCreateStdWindow(HWND_DESKTOP,
                                 WS_VISIBLE,
                                 &FrameFlags,
                                 GetPString(IDS_WCMAINWND2),
                                 NULL,
                                 WS_VISIBLE | WS_ANIMATE,
                                 FM3ModHandle,
                                 MAIN2_FRAME,
                                 &hwndClient);
  if(hwndFrame) {
    if(!WinRestoreWindowPos(FM2Str,
                            "MainWindowPos2",
                            hwndFrame)) {

      ULONG fl  = SWP_MOVE | SWP_SIZE;
      RECTL rcl;
      ULONG icz = WinQuerySysValue(HWND_DESKTOP,SV_CYICON) * 3L;
      ULONG bsz = WinQuerySysValue(HWND_DESKTOP,SV_CYSIZEBORDER);

      WinQueryWindowRect(HWND_DESKTOP,&rcl);
      rcl.yBottom += icz;
      rcl.yTop -= bsz;
      rcl.xLeft += bsz;
      rcl.xRight -= bsz;
      WinSetWindowPos(hwndFrame,
                      HWND_TOP,
                      rcl.xLeft,
                      rcl.yBottom,
                      rcl.xRight - rcl.xLeft,
                      rcl.yTop - rcl.yBottom,
                      fl);
    }
    if(fLogFile)
      LogFileHandle = _fsopen("FM2.LOG","a+",SH_DENYWR);
    if(hwndHelp)
      WinAssociateHelpInstance(hwndHelp,hwndFrame);
    PostMsg(hwndClient,
            UM_SETUP,
            MPFROMLONG(argc),
            MPFROMP(argv));
  }
  return hwndFrame;
}
