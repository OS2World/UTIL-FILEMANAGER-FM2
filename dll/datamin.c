
/***********************************************************************

  $Id$

  Minimized data bar

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2002 Steven H.Levine

  Revisions	14 Sep 02 SHL - Handle large partitions
		16 Oct 02 SHL - Handle large partitions better

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "procstat.h"

#pragma data_seg(DATA2)
#pragma alloc_text(DATAMIN,DataDlgProc,MiniTimeProc)

APIRET16 APIENTRY16 Dos16MemAvail (PULONG pulAvailMem);

long MINI_X = 208,MINI_Y = 16;


MRESULT EXPENTRY MiniTimeProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_BUTTON1CLICK:
      {
        USHORT id = WinQueryWindowUShort(hwnd,QWS_ID);

        if(id >= MINI_DRIVEA)
          WinInvalidateRect(hwnd,NULL,FALSE);
        else if(id == MINI_TIME)
          PostMsg(WinQueryWindow(hwnd,QW_PARENT),
                  UM_SETUP6,
                  MPVOID,
                  MPVOID);
        else if(id == MINI_PROC)
          WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),
                     WM_SYSCOMMAND,
                     MPFROM2SHORT(SC_TASKMANAGER,0),
                     MPVOID);
      }
      break;

    case WM_BUTTON1DBLCLK:
      {
        USHORT id = WinQueryWindowUShort(hwnd,QWS_ID);

        if(id >= MINI_DRIVEA && !hwndMain) {

          CHAR s[] = " :\\";

          *s = (CHAR)(id - MINI_DRIVEA) + 'A';
          OpenDirCnr((HWND)0,
                     HWND_DESKTOP,
                     (HWND)0,
                     FALSE,
                     s);
          return MRFROMLONG(1L);
        }
        else if(id == MINI_TIME) {
          OpenObject("<WP_CLOCK>",
                     (SHORT2FROMMP(mp2) & KC_SHIFT) ?
                      Default : Settings,
                     hwnd);
          return MRFROMLONG(1L);
        }

#ifdef NEVER
        else if(id == MINI_MEM) {
          WinDlgBox(HWND_DESKTOP,
                    HWND_DESKTOP,
                    SysInfoDlgProc,
                    FM3ModHandle,
                    SYS_FRAME,
                    NULL);
          return MRFROMLONG(1L);
        }
#endif

        else if(id == MINI_PROC || id == MINI_MEM) {
          WinDlgBox(HWND_DESKTOP,
                    hwnd,
                    KillDlgProc,
                    FM3ModHandle,
                    KILL_FRAME,
                    NULL);
          return MRFROMLONG(1L);
        }
        else if(id == MINI_SWAP && *SwapperDat) {

          char s[5];

          strncpy(s,SwapperDat,4);
          s[3] = 0;
          WinDlgBox(HWND_DESKTOP,
                    hwndMain,
                    UndeleteDlgProc,
                    FM3ModHandle,
                    UNDEL_FRAME,
                    MPFROMP(s));
          return MRFROMLONG(1L);
        }
      }
      break;

    case WM_BUTTON1MOTIONSTART:
      PostMsg(WinQueryWindow(hwnd,QW_PARENT),
              UM_BUTTON1MOTIONSTART,
              MPVOID,
              MPVOID);
      break;

    case WM_CONTEXTMENU:
      PostMsg(WinQueryWindow(hwnd,QW_PARENT),
              UM_CONTEXTMENU,
              MPVOID,
              MPVOID);
      break;

    case WM_PAINT:
      {
        MRESULT mr;
        USHORT  id;

        id = WinQueryWindowUShort(hwnd,QWS_ID);
        if(id >= MINI_DRIVEA) {

          HPS hps;

          hps = WinBeginPaint(hwnd,(HPS)0,NULL);
          if(hps) {
            mr = WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),
                            UM_PAINT,
                            MPFROM2SHORT(id,0),
                            MPFROMLONG(hps));
            WinEndPaint(hps);
          }
        }
        else
          mr = PFNWPStatic(hwnd,msg,mp1,mp2);
        return mr;
      }
  }
  return PFNWPStatic(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY DataProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static ULONG counter;
  static BOOL  NoFloat,noqproc = FALSE,Positioned;
  static HWND  hwndMenu = (HWND)0;

  switch(msg) {
    case WM_CREATE:
      if(DataHwnd) {
        WinSetWindowPos(DataHwnd,
                        HWND_TOP,
                        0,
                        0,
                        0,
                        0,
                        SWP_ZORDER | SWP_SHOW);
        return MRFROMLONG(1L);
      }
      DataHwnd = WinQueryWindow(hwnd,QW_PARENT);
      NoFloat = FALSE;
      Positioned = FALSE;
      SetPresParams(hwnd,
                    &RGBGREY,
                    &RGBBLACK,
                    &RGBBLACK,
                    GetPString(IDS_8HELVTEXT));
      {
        int    c;
        long   x = 3;
        USHORT ids[] = {MINI_TIME,MINI_MEM,MINI_SWAP,MINI_PROC,0};
        POINTL aptl[TXTBOX_COUNT];
        HPS    hps;

        hps = WinGetPS(hwnd);
        if(hps) {
          GpiQueryTextBox(hps,
                          34,
                          "  -=03:08:22  SMW  1998/08/02=-  ",
                          TXTBOX_COUNT,
                          aptl);
          WinReleasePS(hps);
          MINI_X = aptl[TXTBOX_TOPRIGHT].x + 6;
          MINI_Y = aptl[TXTBOX_TOPRIGHT].y + 6;
        }
        for(c = 0;ids[c];c++) {
          WinCreateWindow(hwnd,
                          GetPString(IDS_WCMINITIME),
                          NullStr,
                          SS_TEXT | DT_CENTER | DT_VCENTER | WS_VISIBLE,
                          x,
                          3,
                          MINI_X,
                          MINI_Y,
                          hwnd,
                          HWND_TOP,
                          ids[c],
                          NULL,
                          NULL);
          x += (MINI_X + 4);
        }
      }
      if(!hwndMain) {

        SWCNTRL swctl;

        memset(&swctl,0,sizeof(swctl));
        swctl.hwnd = WinQueryWindow(hwnd,QW_PARENT);
        swctl.uchVisibility = SWL_VISIBLE;
        swctl.fbJump = (fDataToFore) ? SWL_NOTJUMPABLE : SWL_JUMPABLE;
        swctl.bProgType = PROG_PM;
        strcpy(swctl.szSwtitle,
               GetPString(IDS_DATABARTITLETEXT));
        WinCreateSwitchEntry(WinQueryAnchorBlock(hwnd),
                             &swctl);
      }
      PostMsg(hwnd,
              UM_SETUP,
              MPVOID,
              MPVOID);
      return 0;

    case WM_MENUEND:
      NoFloat = FALSE;
      if(hwndMenu == (HWND)mp2) {
        WinDestroyWindow(hwndMenu);
        hwndMenu = (HWND)0;
      }
      break;

    case UM_RESTORE:
      WinSetWindowPtr(hwnd,0,mp1);
      return 0;

    case UM_SETUP:
      {
        long  x,y;
        SWP   swp,swpD;
        int   c;
        ULONG size = sizeof(SWP),numdrives = 0,
              drivestyle = (DRIVE_REMOVABLE | DRIVE_INVALID | DRIVE_IGNORE |
                            DRIVE_ZIPSTREAM),
              ulDriveNum,ulDriveMap;

        if(!fDataInclRemote)
          drivestyle |= DRIVE_REMOTE;
        if(fDataShowDrives) {
          DosError(FERR_DISABLEHARDERR);
          DosQCurDisk(&ulDriveNum,&ulDriveMap);
          x = 3;
          y = MINI_Y + 4;
          for(c = 2;c < 26;c++) {
            if((ulDriveMap & (1L << c)) && !(driveflags[c] & drivestyle)) {
              WinCreateWindow(hwnd,
                              GetPString(IDS_WCMINITIME),
                              NullStr,
                              SS_TEXT | DT_CENTER | DT_VCENTER | WS_VISIBLE,
                              x,
                              y,
                              MINI_X,
                              MINI_Y,
                              hwnd,
                              HWND_TOP,
                              MINI_DRIVEA + c,
                              NULL,
                              NULL);
              numdrives++;
              x += (MINI_X + 4);
              if((numdrives % 4) == 0) {
                y += (MINI_Y + 4);
                x = 3;
              }
            }
          }
        }
        x = (MINI_X * 4) + 18;
        y = (MINI_Y + 4) + ((numdrives / 4) * (MINI_Y + 4)) +
            (((numdrives % 4) != 0) * (MINI_Y + 4));
        if(!Positioned) {
          if(PrfQueryProfileData(fmprof,
                                 appname,
                                 "DataMinPos",
                                 &swp,
                                 &size)) {
            WinQueryWindowPos(HWND_DESKTOP,&swpD);
            if(swp.x > swpD.cx - 16)
              swp.x = swpD.cx - 16;
            if(swp.y > swpD.cy - 16)
              swp.y = swpD.cy - 16;
            WinSetWindowPos(WinQueryWindow(hwnd,QW_PARENT),
                            HWND_TOP,
                            swp.x,
                            swp.y,
                            x,
                            y,
                            SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ZORDER);
          }
          else
            WinSetWindowPos(WinQueryWindow(hwnd,QW_PARENT),
                            HWND_TOP,
                            0,
                            0,
                            x,
                            y,
                            SWP_SHOW | SWP_SIZE | SWP_MOVE | SWP_ZORDER);
          Positioned = TRUE;
        }
        else
          WinSetWindowPos(WinQueryWindow(hwnd,QW_PARENT),
                          HWND_TOP,
                          0,
                          0,
                          x,
                          y,
                          SWP_SHOW | SWP_SIZE | SWP_ZORDER);
        WinShowWindow(WinQueryWindow(hwnd,QW_PARENT),
                                     TRUE);
      }
      counter = 0;
      PostMsg(hwnd,
              UM_TIMER,
              MPVOID,
              MPVOID);
      return 0;

    case WM_BUTTON1DBLCLK:
      if(hwndMain)
        PostMsg(hwnd,
                WM_CLOSE,
                MPVOID,
                MPVOID);
      break;

    case UM_CONTEXTMENU:
    case WM_CONTEXTMENU:
      if(!hwndMenu)
        hwndMenu = WinLoadMenu(HWND_DESKTOP,
                               FM3ModHandle,
                               MINI_FRAME);
      if(hwndMenu) {
        WinCheckMenuItem(hwndMenu,
                         MINI_FLOAT,
                         fDataToFore);
        WinCheckMenuItem(hwndMenu,
                         MINI_SHOW,
                         fDataShowDrives);
        WinCheckMenuItem(hwndMenu,
                         MINI_BORING,
                         fDullMin);
        WinCheckMenuItem(hwndMenu,
                         MINI_INCLREMOTE,
                         fDataInclRemote);
        NoFloat = TRUE;
        if(!PopupMenu(hwnd,
                      hwnd,
                      hwndMenu))
          NoFloat = FALSE;
      }
      if(msg == UM_CONTEXTMENU)
        return 0;
      break;

    case WM_BUTTON2DBLCLK:
      if(!(SHORT2FROMMP(mp2) & KC_SHIFT)) {
        PostMsg(hwnd,
                WM_COMMAND,
                MPFROM2SHORT(MINI_FLOAT,0),
                MPVOID);
        break;
      }
      /* else intentional fallthru */
    case WM_CHORD:
    case WM_BUTTON3DBLCLK:
      PostMsg(hwnd,
              WM_COMMAND,
              MPFROM2SHORT(MINI_SHOW,0),
              MPVOID);
      break;

    case UM_BUTTON1MOTIONSTART:
    case WM_BUTTON1MOTIONSTART:
      {
        TRACKINFO TrackInfo;
        SWP       Position;

        memset(&TrackInfo,0,sizeof(TrackInfo));
        TrackInfo.cxBorder   = 1 ;
        TrackInfo.cyBorder   = 1 ;
        TrackInfo.cxGrid     = 1 ;
        TrackInfo.cyGrid     = 1 ;
        TrackInfo.cxKeyboard = 8 ;
        TrackInfo.cyKeyboard = 8 ;
        WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),&Position);
        TrackInfo.rclTrack.xLeft   = Position.x ;
        TrackInfo.rclTrack.xRight  = Position.x + Position.cx ;
        TrackInfo.rclTrack.yBottom = Position.y ;
        TrackInfo.rclTrack.yTop    = Position.y + Position.cy ;
        WinQueryWindowPos(HWND_DESKTOP,&Position);
        TrackInfo.rclBoundary.xLeft   = Position.x ;
        TrackInfo.rclBoundary.xRight  = Position.x + Position.cx ;
        TrackInfo.rclBoundary.yBottom = Position.y ;
        TrackInfo.rclBoundary.yTop    = Position.y + Position.cy ;
        TrackInfo.ptlMinTrackSize.x = 0 ;
        TrackInfo.ptlMinTrackSize.y = 0 ;
        TrackInfo.ptlMaxTrackSize.x = Position.cx ;
        TrackInfo.ptlMaxTrackSize.y = Position.cy ;
        TrackInfo.fs = TF_MOVE | TF_STANDARD | TF_ALLINBOUNDARY ;
        if(WinTrackRect(HWND_DESKTOP,(HPS)0,&TrackInfo)) {
          WinSetWindowPos(WinQueryWindow(hwnd,QW_PARENT),
                          HWND_TOP,TrackInfo.rclTrack.xLeft,
                          TrackInfo.rclTrack.yBottom,0,0,SWP_MOVE);
          WinSendMsg(hwnd,WM_SAVEAPPLICATION,MPVOID,MPVOID);
        }
      }
      break;

    case WM_HELP:
      PostMsg(hwnd,WM_COMMAND,MPFROM2SHORT(IDM_HELP,0),MPVOID);
      break;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_COMMANDLINE:
        case IDM_DOSCOMMANDLINE:
        case IDM_WINFULLSCREEN:
          {
            CHAR    *env = GetCmdSpec(FALSE),path[CCHMAXPATH];
            INT      type = SEPARATE | WINDOWED;

            *path = 0;
            TopWindowName(hwnd,
                          (HWND)0,
                          path);
            if(SHORT1FROMMP(mp1) == IDM_DOSCOMMANDLINE)
              env = GetCmdSpec(TRUE);
            else if(SHORT1FROMMP(mp1) != IDM_COMMANDLINE) {
              env = "WINOS2.COM";
              type = SEPARATE | FULLSCREEN;
            }
            runemf2(type,
                    hwnd,
                    path,
                    NULL,
                    "%s",
                    env);
          }
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_DATABAR,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case MINI_CLOSE:
          PostMsg(hwnd,
                  WM_CLOSE,
                  MPVOID,
                  MPVOID);
          break;

        case MINI_BORING:
          fDullMin = (fDullMin) ? FALSE : TRUE;
          PrfWriteProfileData(fmprof,
                              FM3Str,
                              "DullDatabar",
                              &fDullMin,
                              sizeof(BOOL));
          WinInvalidateRect(hwnd,
                            NULL,
                            TRUE);
          break;

        case MINI_INCLREMOTE:
        case MINI_SHOW:
          if(SHORT1FROMMP(mp1) == MINI_SHOW) {
            fDataShowDrives = (fDataShowDrives) ? FALSE : TRUE;
            PrfWriteProfileData(fmprof,
                                appname,
                                "DataShowDrives",
                                &fDataShowDrives,
                                sizeof(BOOL));
          }
          else {
            fDataInclRemote = (fDataInclRemote) ? FALSE : TRUE;
            PrfWriteProfileData(fmprof,
                                appname,
                                "DataInclRemote",
                                &fDataInclRemote,
                                sizeof(BOOL));
          }
          {
            HENUM   henum;
            HWND    hwndTemp;
            USHORT  id;

            henum = WinBeginEnumWindows(hwnd);
            while((hwndTemp = WinGetNextWindow(henum)) != NULLHANDLE) {
              id = WinQueryWindowUShort(hwndTemp,QWS_ID);
              if(id >= MINI_DRIVEA)
                WinDestroyWindow(hwndTemp);
            }
            WinEndEnumWindows(henum);
          }
          PostMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
          break;

        case MINI_FLOAT:
          fDataToFore = (fDataToFore) ? FALSE : TRUE;
          PrfWriteProfileData(fmprof,
                              appname,
                              "DataToFore",
                              &fDataToFore,
                              sizeof(BOOL));
          if(!hwndMain) {

            SWCNTRL swcntrl;
            HSWITCH hswitch;

            hswitch = (HSWITCH)WinQuerySwitchHandle(hwnd,(PID)0);
            if(hswitch) {
              memset(&swcntrl,0,sizeof(SWCNTRL));
              if(!WinQuerySwitchEntry(hswitch,&swcntrl)) {
                swcntrl.fbJump = (fDataToFore) ?
                                  SWL_NOTJUMPABLE :
                                  SWL_JUMPABLE;
                WinChangeSwitchEntry(hswitch,
                                     &swcntrl);
              }
            }
          }
          break;
      }
      return 0;

    case WM_SIZE:
      WinSetWindowPos(hwnd,
                      HWND_TOP,
                      0,
                      0,
                      SHORT1FROMMP(mp2),
                      SHORT2FROMMP(mp2),
                      SWP_MOVE | SWP_SIZE);
      break;

    case WM_PAINT:
      {
        HPS     hps;
        POINTL  ptl;
        SWP     swp;
        RECTL   rcl;

        hps = WinBeginPaint(hwnd,
                            (HPS)0,
                            &rcl);
        if(hps) {
          WinFillRect(hps,
                      (PRECTL)&rcl,
                      CLR_PALEGRAY);
          GpiSetMix(hps,FM_OVERPAINT);
          GpiSetColor(hps,CLR_WHITE);
          WinQueryWindowPos(hwnd,&swp);
          ptl.x = 0;
          ptl.y = 0;
          GpiMove(hps,&ptl);
          ptl.y = swp.cy - 1;
          GpiLine(hps,&ptl);
          ptl.x = swp.cx - 1;
          GpiLine(hps,&ptl);
          GpiSetColor(hps,CLR_DARKGRAY);
          ptl.y = 0;
          GpiLine(hps,&ptl);
          ptl.x = 0;
          GpiLine(hps,&ptl);
          {
            HENUM   henum;
            HWND    hwndTemp;

            henum = WinBeginEnumWindows(hwnd);
            while((hwndTemp = WinGetNextWindow(henum)) != NULLHANDLE) {
              PaintRecessedWindow(hwndTemp,
                                  hps,
                                  (WinQueryWindowUShort(hwndTemp,QWS_ID)
                                   != MINI_TIME),
                                  FALSE);
            }
            WinEndEnumWindows(henum);
          }
          WinEndPaint(hps);
        }
      }
      return 0;

    case UM_PAINT:
      {
        CHAR        s[90],szFreeQty[38],*pszFreeUM,path[] = " :",
                   *szBuf = NULL,*FSystem = NULL;
        float       fltFreeQty;
        ULONG       percentfree,wasx,size;
        HPS         hps = (HPS)mp2;
        FSALLOCATE  fsa;
        HWND        hwndTemp;
        USHORT      id;
        SWP         swp;
        POINTL      ptl;

        id = SHORT1FROMMP(mp1);
        if(id >= MINI_DRIVEA) {
          hwndTemp = WinWindowFromID(hwnd,id);
          if(!hwndTemp)
            return 0;
          if(!WinQueryWindowPos(hwndTemp,&swp))
            return 0;
          *szFreeQty = 0;
          DosError(FERR_DISABLEHARDERR);
          if(!DosQueryFSInfo((id - MINI_DRIVEA) + 1,
                             FSIL_ALLOC,
                             &fsa,
                             sizeof(FSALLOCATE))) {
            fltFreeQty = (float)fsa.cUnitAvail * (fsa.cSectorUnit * fsa.cbSector);
            if (fltFreeQty >= (1024 * 1024)) {
              fltFreeQty /= (1024 * 1024);
              pszFreeUM = "mb";
            }
            else if (fltFreeQty >= 1024) {
              fltFreeQty /= 1024;
              pszFreeUM = "kb";
            }
            else
	    {
              pszFreeUM = "b";
	    }
            percentfree = (fsa.cUnit && fsa.cUnitAvail) ?
                            (fsa.cUnitAvail * 100) / fsa.cUnit : 0;
            commafmt(szFreeQty,sizeof(szFreeQty),(ULONG)fltFreeQty);
            *path = (CHAR)(id - MINI_DRIVEA) + 'A';
            if(!DosAllocMem((PVOID)&szBuf,
                            4096,
                            PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE)) {
              *szBuf = 0;
              size = 4096;
              DosError(FERR_DISABLEHARDERR);
              if(!DosQueryFSAttach(path,
                                   0,
                                   FSAIL_QUERYNAME,
                                   (PFSQBUFFER2)szBuf,
                                   &size)) {
                FSystem = ((PFSQBUFFER2)szBuf)->szName +
                           ((PFSQBUFFER2)szBuf)->cbName + 1;
                FSystem[15] = 0;
              }
            }
            else
              szBuf = NULL;
            sprintf(s,
                    " %s %s%s (%lu%%) %s%s%s%s",
                    path,
                    szFreeQty,
                    pszFreeUM,
                    percentfree,
                    GetPString(IDS_FREETEXT),
                    (FSystem) ? " [" : NullStr,
                    FSystem,
                    (FSystem) ? "]" : NullStr);
            if(szBuf)
              DosFreeMem(szBuf);
            if(!hps)
              hps = WinGetPS(hwndTemp);
            if(hps) {
              if(!fDullMin) {
                ptl.x = 0;
                ptl.y = 0;
                GpiMove(hps,
                        &ptl);
                GpiSetColor(hps,
                            CLR_BLACK);
                ptl.x = swp.cx - 1;
                ptl.y = swp.cy - 1;
                GpiBox(hps,
                       DRO_OUTLINE,
                       &ptl,
                       0,
                       0);
                ptl.x = 1;
                ptl.y = 1;
                if(percentfree) {
                  GpiMove(hps,&ptl);
                  GpiSetColor(hps,
                              (percentfree < 11) ? CLR_DARKRED :
                               (percentfree < 26) ? CLR_DARKBLUE :
                                                    CLR_DARKGREEN);
                  ptl.y = swp.cy - 2;
                  ptl.x = ((swp.cx - 2) * percentfree) / 100;
                  wasx = ptl.x;
                  GpiBox(hps,
                         DRO_OUTLINEFILL,
                         &ptl,
                         0,
                         0);
                  GpiSetColor(hps,
                              (percentfree < 11) ? CLR_RED :
                               (percentfree < 26) ? CLR_BLUE : CLR_GREEN);
                  ptl.x = wasx;
                  ptl.y = swp.cy - 2;
                  GpiMove(hps,
                          &ptl);
                  ptl.x = 1;
                  GpiLine(hps,
                          &ptl);
                  ptl.y = 2;
                  ptl.x = 1;
                  GpiLine(hps,
                          &ptl);
                  ptl.x = wasx;
                }
                if(percentfree < 99) {
                  GpiSetColor(hps,
                              CLR_DARKGRAY);
                  wasx = ptl.x;
                  ptl.y = 2;
                  GpiMove(hps,&ptl);
                  ptl.y = swp.cy - 2;
                  ptl.x = swp.cx - 2;
                  GpiBox(hps,
                         DRO_OUTLINEFILL,
                         &ptl,
                         0,
                         0);
                  ptl.x = wasx;
                  GpiMove(hps,
                          &ptl);
                  GpiSetColor(hps,
                              CLR_PALEGRAY);
                  ptl.x = swp.cx - 3;
                  GpiLine(hps,&ptl);
                  ptl.x = wasx;
                  ptl.y = 1;
                  GpiMove(hps,&ptl);
                  GpiSetColor(hps,
                              CLR_BLACK);
                  ptl.x = swp.cx - 2;
                  GpiLine(hps,&ptl);
                  ptl.y = swp.cy - 3;
                  GpiLine(hps,&ptl);
                }
                GpiSetColor(hps,
                            CLR_WHITE);
              }
              else {
                GpiSetColor(hps,
                            CLR_PALEGRAY);
                ptl.x = 0;
                ptl.y = 0;
                GpiMove(hps,
                        &ptl);
                ptl.x = swp.cx - 1;
                ptl.y = swp.cy - 1;
                GpiBox(hps,
                       DRO_OUTLINEFILL,
                       &ptl,
                       0,
                       0);
                GpiSetColor(hps,
                            (percentfree < 11) ? CLR_DARKRED :
                                                 CLR_DARKBLUE);
              }
              GpiSetBackMix(hps,
                            BM_LEAVEALONE);
              GpiSetMix(hps,
                        FM_OVERPAINT);
              {
                POINTL aptl[TXTBOX_COUNT];

                GpiQueryTextBox(hps,
                                strlen(s),
                                s,
                                TXTBOX_COUNT,
                                aptl);
                ptl.y = ((swp.cy / 2) -
                          ((aptl[TXTBOX_TOPRIGHT].y +
                           aptl[TXTBOX_BOTTOMLEFT].y) / 2));
                ptl.y++;
                ptl.x = (swp.cx / 2) - (aptl[TXTBOX_TOPRIGHT].x / 2);
                if(ptl.x < 2)
                  ptl.x = 2;
                GpiCharStringAt(hps,
                                &ptl,
                                strlen(s),
                                s);
              }
              if(!mp2)
                WinReleasePS(hps);
            }
          }
        }
      }
      return 0;

    case UM_TIMER:
      {
        CHAR          s[134];
        DATETIME      dt;

        if(fDataToFore && !NoFloat)
          WinSetWindowPos(WinQueryWindow(hwnd,QW_PARENT),
                          HWND_TOP,0,0,0,0,SWP_ZORDER);
        if(counter && (counter % 19) && (counter % 20)) {
          if(!DosGetDateTime(&dt)) {
            sprintf(s,
                    " %02hu:%02hu:%02hu  %s %04u/%02u/%02u",
                    dt.hours,
                    dt.minutes,
                    dt.seconds,
                    GetPString(IDS_SUNDAY + dt.weekday),
                    dt.year,
                    dt.month,
                    dt.day);
            WinSetDlgItemText(hwnd,
                              MINI_TIME,
                              s);
          }
        }
        else if(!counter || !(counter % 19))
          PostMsg(hwnd,UM_SETUP6,MPVOID,MPVOID);
        if(!(counter % 4)) {
          PostMsg(hwnd,UM_SETUP3,MPVOID,MPVOID);
          if(!(counter % 10)) {
            PostMsg(hwnd,UM_SETUP5,MPVOID,MPVOID);
            if(!(counter % 20)) {
              PostMsg(hwnd,UM_SETUP2,MPVOID,MPVOID);
              PostMsg(hwnd,UM_SETUP4,MPVOID,MPVOID);
            }
          }
        }
      }
      counter++;
      return 0;

    case UM_SETUP2:
      {
        CHAR		s[134],szFileQty[38],szFreeQty[38],*pszFileUM,*pszFreeUM;
        FILEFINDBUF3	ffb;
        ULONG 		nm = 1L;
	float		fltFreeQty;
        HDIR		hdir = HDIR_CREATE;
        FSALLOCATE	fsa;

        if(*SwapperDat) {
          DosError(FERR_DISABLEHARDERR);
          if(!DosFindFirst(SwapperDat,&hdir,FILE_NORMAL | FILE_HIDDEN |
                           FILE_SYSTEM | FILE_ARCHIVED | FILE_READONLY,
                           &ffb,sizeof(ffb),&nm,FIL_STANDARD)) {
            priority_bumped();
            DosFindClose(hdir);
            *szFileQty = *szFreeQty = 0;
            DosError(FERR_DISABLEHARDERR);
            if(!DosQueryFSInfo(toupper(*SwapperDat) - '@',FSIL_ALLOC,
                               &fsa,sizeof(FSALLOCATE))) {
              fltFreeQty = fsa.cUnitAvail * (fsa.cSectorUnit * fsa.cbSector);
              if(fltFreeQty > 1024 * 1024) {
                pszFreeUM = "mb";
                fltFreeQty /= (1024 * 1024);
              }
              else if(fltFreeQty > 1024) {
                pszFreeUM = "kb";
                fltFreeQty /= 1024;
              }
              else {
                pszFreeUM = "b";
              }
              commafmt(szFreeQty,sizeof(szFreeQty),(ULONG)fltFreeQty);
            }
            if(ffb.cbFile > 1024 * 1024) {
              pszFileUM = "mb";
              commafmt(szFileQty,sizeof(szFileQty),ffb.cbFile / (1024 * 1024));
            }
            else if(ffb.cbFile > 1024) {
              pszFileUM = "kb";
              commafmt(szFileQty,sizeof(szFileQty),ffb.cbFile / 1024);
            }
            else {
              pszFileUM = "b";
              commafmt(szFileQty,sizeof(szFileQty),ffb.cbFile);
            }
            sprintf(s," %s %s%s%s%s%s",
                    GetPString(IDS_SWAPTITLETEXT),
                    szFileQty,
                    pszFileUM,
                    (*szFreeQty) ? "/" : NullStr,
                    szFreeQty,
                    (*szFreeQty) ? pszFreeUM : NullStr);
            WinSetDlgItemText(hwnd,
                              MINI_SWAP,
                              s);
          }
        }
      }
      return 0;

    case UM_SETUP3:
      {
        CHAR  s[134],tm[38],szQty[38],*pszUM,*tmk;
        ULONG amem = 0;

        if(!DosQuerySysInfo(QSV_TOTAVAILMEM,QSV_TOTAVAILMEM,
                            (PVOID)&amem,(ULONG)sizeof(amem))) {
          *tm = *szQty = 0;
          if(amem > 1024 * 1024) {
            tmk = "mb";
            commafmt(tm,sizeof(tm),amem / (1024 * 1024));
          }
          else if(amem > 1024) {
            tmk = "kb";
            commafmt(tm,sizeof(tm),amem / 1024);
          }
          else {
            tmk = "b";
            commafmt(tm,sizeof(tm),amem);
          }
          if(!Dos16MemAvail(&amem)) {
            if(amem > 1024 * 1024) {
              pszUM = "mb";
              commafmt(szQty,sizeof(szQty),amem / (1024 * 1024));
            }
            else if(amem > 1024) {
              pszUM = "kb";
              commafmt(szQty,sizeof(szQty),amem / 1024);
            }
            else {
              pszUM = "b";
              commafmt(szQty,sizeof(szQty),amem);
            }
          }
          sprintf(s," %s%s%s%s%s%s",
                  GetPString(IDS_MEMTITLETEXT),
                  szQty,
                  (*szQty) ? pszUM : NullStr,
                  (*szQty) ? "/" : NullStr,
                  tm,
                  tmk);
          WinSetDlgItemText(hwnd,
                            MINI_MEM,
                            s);
        }
      }
      return 0;

    case UM_SETUP4:
      {
        HWND        hwndTemp;
        HENUM       henum;
        USHORT      id;

        henum = WinBeginEnumWindows(hwnd);
        while((hwndTemp = WinGetNextWindow(henum)) != NULLHANDLE) {
          id = WinQueryWindowUShort(hwndTemp,QWS_ID);
          if(id >= MINI_DRIVEA)
            WinInvalidateRect(hwndTemp,NULL,FALSE);
        }
        WinEndEnumWindows(henum);
      }
      return 0;

    case UM_SETUP5:
      {
        CHAR s[134],tm[38],szQty[38];

        if(fUseQProcStat && !noqproc) {

          PROCESSINFO  *ppi;
          BUFFHEADER   *pbh = NULL;
          MODINFO      *pmi;
          ULONG         numprocs = 0,numthreads = 0;


          if(!DosAllocMem((PVOID)&pbh,USHRT_MAX + 4096,
                          PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE)) {
            if(!DosQProcStatus(pbh,USHRT_MAX)) {
              ppi = pbh->ppi;
              while(ppi->ulEndIndicator != PROCESS_END_INDICATOR ) {
                pmi = pbh->pmi;
                while(pmi && ppi->hModRef != pmi->hMod)
                  pmi = pmi->pNext;
                if(pmi) {
                  numprocs++;
                  numthreads += ppi->usThreadCount;
                }
                ppi = (PPROCESSINFO)(ppi->ptiFirst + ppi->usThreadCount);
              }
              *szQty = *tm = 0;
              commafmt(szQty,sizeof(szQty),numprocs);
              commafmt(tm,sizeof(tm),numthreads);
              sprintf(s,
                      " %s%s  %s%s",
                      GetPString(IDS_PROCSTITLETEXT),
                      szQty,
                      GetPString(IDS_THRDSTITLETEXT),
                      tm);
              WinSetDlgItemText(hwnd,
                                MINI_PROC,
                                s);
            }
            else
              noqproc = TRUE;
            DosFreeMem(pbh);
          }
        }
        else {
          *szQty = 0;
          commafmt(szQty,sizeof(szQty),
                   WinQuerySwitchList(WinQueryAnchorBlock(hwnd),(PSWBLOCK)0,0));
          sprintf(s,
                  " %s%s",
                  GetPString(IDS_TASKSTITLETEXT),
                  szQty);
          WinSetDlgItemText(hwnd,
                            MINI_PROC,
                            s);
        }
      }
      return 0;

    case UM_SETUP6:
      {
        ULONG val = 0,numdays,nummins;
        CHAR  s[128];

        if(!DosQuerySysInfo(QSV_MS_COUNT,
                            QSV_MS_COUNT,
                            (PVOID)&val,
                            (ULONG)sizeof(val))) {
          val /= 60000L;
          numdays = val / (60L * 24L);
          strcpy(s,GetPString(IDS_ELAPSEDTITLETEXT));
          if(numdays)
            sprintf(s + strlen(s),
                    " %lu %s%s, ",
                    numdays,
                    GetPString(IDS_DAYTEXT),
                    &"s"[numdays == 1L]);
          nummins = val % (60L * 24L);
          sprintf(s + strlen(s),
                  " %lu:%02lu",
                  nummins / 60,
                  nummins % 60);
          WinSetDlgItemText(hwnd,
                            MINI_TIME,
                            s);
        }
      }
      return 0;

    case WM_SAVEAPPLICATION:
      {
        SWP swp;

        WinQueryWindowPos(WinQueryWindow(hwnd,QW_PARENT),&swp);
        PrfWriteProfileData(fmprof,
                            appname,
                            "DataMinPos",
                            &swp,
                            sizeof(SWP));
      }
      break;

    case WM_CLOSE:
      WinSendMsg(hwnd,
                 WM_SAVEAPPLICATION,
                 MPVOID,
                 MPVOID);
      WinDestroyWindow(WinQueryWindow(hwnd,QW_PARENT));
      return 0;

    case WM_DESTROY:
      if(DataHwnd == WinQueryWindow(hwnd,QW_PARENT)) {
        DataHwnd = (HWND)0;
        if(hwndMenu)
          WinDestroyWindow(hwndMenu);
        hwndMenu = (HWND)0;
      }
      if(hwndMain) {

        SWP   swp;
        ULONG fl = SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE,ofl;

        ofl = WinQueryWindowULong(hwnd,0);
        WinQueryWindowPos(WinQueryWindow(hwndMain,QW_PARENT),&swp);
        if(swp.fl & SWP_MINIMIZE)
          fl |= ((ofl & SWP_MAXIMIZE) ? SWP_MAXIMIZE : SWP_RESTORE);
        WinSetWindowPos(WinQueryWindow(hwndMain,QW_PARENT),
                        HWND_TOP,
                        0,
                        0,
                        0,
                        0,
                        fl);
      }
      else if(!PostMsg((HWND)0,
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


HWND CreateDataBar (HWND hwndParent,ULONG fl) {

  HWND  hwndClient = (HWND)0;
  ULONG FrameFlags = 0;

  if(WinCreateStdWindow(hwndParent,
                        WS_VISIBLE,
                        &FrameFlags,
                        GetPString(IDS_WCDATABAR),
                        NULL,
                        WS_VISIBLE,
                        0,
                        MINI_FRAME,
                        &hwndClient)) {
    WinSendMsg(hwndClient,
               UM_RESTORE,
               MPFROMLONG(fl),
               MPVOID);
  }
  return hwndClient;
}
