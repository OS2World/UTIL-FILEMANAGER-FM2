#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA1)
#pragma alloc_text(SEEFILES,DoADir,FillListBox,DrvsWndProc,SeeWndProc,EmptyListBox)

static HWND amup = (HWND)0;
static CHAR stopflag;
static CHAR drvsflags[26] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
static INT  Sorttype = SORT_FILENAME;


static VOID DoADir (HWND hwndList,CHAR *pathname,INT sorttype) {

  CHAR        *filename,*enddir,*temp,*build;
  FILEFINDBUF3 ffb;
  HDIR         hdir = HDIR_CREATE;
  ULONG        nm = 1L;
  SHORT        sSelect;

  filename = malloc(CCHMAXPATH);
  if(!filename)
    return;
  build = malloc(CCHMAXPATHCOMP + 34);
  if(!build) {
    free(filename);
    return;
  }
  memset(filename,0,CCHMAXPATH);
  strcpy(filename,pathname);
  enddir = &filename[strlen(filename) - 1];
  if(*enddir != '\\') {
    enddir++;
    *enddir = '\\';
  }
  enddir++;
  strcpy(enddir,"*");
  DosError(FERR_DISABLEHARDERR);
  if(!DosFindFirst(filename,
                   &hdir,
                   FILE_NORMAL | FILE_ARCHIVED | FILE_READONLY |
                   FILE_DIRECTORY,
                   &ffb,
                   sizeof(ffb),
                   &nm,
                   FIL_STANDARD)) {
    do {
      priority_normal();
      if(ffb.attrFile & FILE_DIRECTORY) {
        if(*ffb.achName != '.' ||
           (ffb.achName[1] && ffb.achName[1] != '.')) {
          strcpy(enddir,ffb.achName);
          DoADir(hwndList,
                 filename,
                 sorttype);
        }
      }
      else {
        *enddir = 0;
        switch(sorttype) {
          case SORT_SIZE:
            sprintf(build,
                    "%10lu  >%s",
                    ffb.cbFile,
                    ffb.achName);
            break;
          case SORT_LWDATE:
            sprintf(build,
                    "%02u:%02u:%02u %04u/%02u/%02u  >%s",
                    ffb.ftimeLastWrite.hours,
                    ffb.ftimeLastWrite.minutes,
                    ffb.ftimeLastWrite.twosecs * 2,
                    ffb.fdateLastWrite.year,
                    ffb.fdateLastWrite.month,
                    ffb.fdateLastWrite.day,
                    ffb.achName);
            break;
          default:
            strcpy(build,ffb.achName);
            break;
        }
        sSelect = (SHORT)WinSendMsg(hwndList,
                                    LM_INSERTITEM,
                                    MPFROMSHORT(LIT_SORTASCENDING),
                                    MPFROMP(build));
        if(sSelect >= 0) {
          temp = strdup(filename);
          if(temp) {
            WinSendMsg(hwndList,
                       LM_SETITEMHANDLE,
                       MPFROMSHORT(sSelect),
                       MPFROMLONG((LONG)temp));
            DosSleep(0L);
          }
          else {
            WinSendMsg(hwndList,LM_DELETEITEM,MPFROMSHORT(sSelect),MPVOID);
            saymsg(MB_ENTER | MB_ICONEXCLAMATION,
                   hwndList,
                   GetPString(IDS_ERRORTEXT),
                   GetPString(IDS_OUTOFMEMORY));
            DosBeep(250,100);
            stopflag = 1;
            break;
          }
        }
        else {
          WinSendMsg(hwndList,
                     LM_DELETEITEM,
                     MPFROMSHORT(sSelect),
                     MPVOID);
          stopflag = 1;
          saymsg(MB_ENTER | MB_ICONEXCLAMATION,
                 hwndList,
                 GetPString(IDS_PMLIMITTEXT),
                 GetPString(IDS_NOMOREFILESINLISTBOXTEXT));
          break;
        }
      }
      nm = 1L;
    } while(!stopflag &&
            !DosFindNext(hdir,
                         &ffb,
                         sizeof(ffb),
                         &nm));
    DosFindClose(hdir);
    priority_normal();
  }
  DosSleep(1L);
  free(filename);
  free(build);
}



static VOID FillListBox (VOID *args) {

  ULONG ulDriveNum,ulDriveMap,x;
  CHAR  startname[] = " :\\";
  HWND  hwnd,hwndList;
  HAB   hab2;
  HMQ   hmq2;

  hwnd = (HWND)args;
  hab2 = WinInitialize(0);
  if(hab2) {
    hmq2 = WinCreateMsgQueue(hab2,0);
    if(hmq2) {
      WinCancelShutdown(hmq2,TRUE);
      hwndList = WinWindowFromID(hwnd,SEEF_LISTBOX);
      priority_normal();
      DosError(FERR_DISABLEHARDERR);
      if(!DosQCurDisk(&ulDriveNum,&ulDriveMap)) {
        for(x = 2L;x < 26L && !stopflag;x++) {
          if((ulDriveMap & (1L << x)) && drvsflags[x]) {
            *startname = (CHAR)(x + 'A');
            DoADir(hwndList,startname,Sorttype);
            WinSetDlgItemText(hwnd,
                              SEEF_TOTAL,
                              NullStr);
            {
              SHORT sTotal;
              CHAR  s[33];

              sTotal = (SHORT)WinSendDlgItemMsg(hwnd,
                                                SEEF_LISTBOX,
                                                LM_QUERYITEMCOUNT,
                                                MPVOID,
                                                MPVOID);
              if(sTotal >= 0) {
                sprintf(s,
                        "%u",
                        sTotal);
                WinSetDlgItemText(hwnd,
                                  SEEF_TOTAL,
                                  s);
              }
            }
          }
        }
      }
      priority_bumped();
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
  PostMsg(hwnd,UM_CONTAINER_FILLED,MPVOID,MPVOID);
}


static VOID EmptyListBox (VOID *args) {

  HWND  hwnd,hwndList;
  HAB   hab2;
  HMQ   hmq2;

  hwnd = (HWND)args;
  hab2 = WinInitialize(0);
  if(hab2) {
    hmq2 = WinCreateMsgQueue(hab2,0);
    if(hmq2) {
      WinCancelShutdown(hmq2,TRUE);
      hwndList = WinWindowFromID(hwnd,SEEF_LISTBOX);
      priority_normal();
      DosError(FERR_DISABLEHARDERR);
      {
        SHORT sTotal,sSelect = 0;
        CHAR *dir;

        DosSleep(128L);
        sTotal = (SHORT)WinSendMsg(hwndList,LM_QUERYITEMCOUNT,MPVOID,MPVOID);
        while(sSelect < sTotal) {
          dir = (CHAR *)WinSendMsg(hwndList,LM_QUERYITEMHANDLE,
                                   MPFROM2SHORT(sSelect,0),MPVOID);
          if(dir)
            free(dir);
          sSelect++;
          DosSleep(0L);
        }
        DosSleep(128L);
        sTotal = (SHORT)WinSendMsg(hwndList,LM_QUERYITEMCOUNT,MPVOID,MPVOID);
        while(sSelect < sTotal) {
          dir = (CHAR *)WinSendMsg(hwndList,LM_QUERYITEMHANDLE,
                                   MPFROM2SHORT(sSelect,0),MPVOID);
          if(dir)
            free(dir);
          sSelect++;
          DosSleep(0L);
        }
      }
      priority_bumped();
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
  DosSleep(1L);
  PostMsg(hwnd,WM_CLOSE,MPVOID,MPVOID);
}


MRESULT EXPENTRY DrvsWndProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      {
        ULONG ulDriveNum,ulDriveMap,x;
        CHAR  startname[] = " :";
        SHORT sSelect;

        DosError(FERR_DISABLEHARDERR);
        if(!DosQCurDisk(&ulDriveNum,&ulDriveMap)) {
          for(x = 2L;x < 26L && !stopflag;x++) {
            if(!(driveflags[x] & (DRIVE_IGNORE | DRIVE_INVALID))) {
              if(ulDriveMap & (1L << x)) {
                *startname = (CHAR)(x + 'A');
                sSelect = (SHORT)WinSendDlgItemMsg(hwnd,DRVS_LISTBOX,
                                                   LM_INSERTITEM,
                                                   MPFROM2SHORT(LIT_END,0),
                                                   MPFROMP(startname));
                if(sSelect >= 0 && drvsflags[x])
                  WinSendDlgItemMsg(hwnd,DRVS_LISTBOX,LM_SELECTITEM,
                                    MPFROM2SHORT(sSelect,0),MPFROMLONG(TRUE));
              }
            }
          }
        }
      }
      WinCheckButton(hwnd,DRVS_BYNAME,TRUE);
      break;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          {
            INT   x;
            SHORT sSelect;
            CHAR  filename[3];

            Sorttype = SORT_FILENAME;
            if(WinQueryButtonCheckstate(hwnd,DRVS_BYSIZE))
              Sorttype = SORT_SIZE;
            else if(WinQueryButtonCheckstate(hwnd,DRVS_BYDATE))
              Sorttype = SORT_LWDATE;
            memset(drvsflags,0,sizeof(drvsflags));
            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,DRVS_LISTBOX,
                                               LM_QUERYSELECTION,
                                               MPFROM2SHORT(LIT_FIRST,0),
                                               MPVOID);
            while(sSelect >= 0) {
              *filename = 0;
              if(WinSendDlgItemMsg(hwnd,DRVS_LISTBOX,LM_QUERYITEMTEXT,
                                   MPFROM2SHORT(sSelect,2),
                                   MPFROMP(filename)) && *filename)
                drvsflags[*filename - 'A'] = 1;
              sSelect = (SHORT)WinSendDlgItemMsg(hwnd,DRVS_LISTBOX,
                                                 LM_QUERYSELECTION,
                                                 MPFROM2SHORT(sSelect,0),
                                                 MPVOID);
            }
            for(x = 2L;x < 26L;x++) {
              if(drvsflags[x]) {
                WinDismissDlg(hwnd,0);
                return 0;
              }
            }
          }
          WinDismissDlg(hwnd,1);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_DRVSWND,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,1);
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY SeeWndProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static HWND hwndParent;

  switch(msg) {
    case WM_INITDLG:
      if(amup || !mp2) {
        DosBeep(250,100);
        WinSetWindowPos(amup,HWND_TOP,0,0,0,0,SWP_SHOW | SWP_RESTORE |
                        SWP_ZORDER | SWP_ACTIVATE);
        WinDismissDlg(hwnd,0);
        break;
      }
      hwndParent = *(HWND *)mp2;
      amup = hwnd;
      stopflag = 0;
      if(WinDlgBox(HWND_DESKTOP,(hwndMain) ? hwndParent : hwndMain,
                   DrvsWndProc,FM3ModHandle,DRVS_FRAME,MPVOID)) {
        amup = (HWND)0;
        WinDismissDlg(hwnd,0);
      }
      else {
        if(_beginthread(FillListBox,NULL,524288,(PVOID)hwnd) == -1) {
          amup = (HWND)0;
          WinDismissDlg(hwnd,0);
        }
      }
      break;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,UM_SETDIR,MPVOID,MPVOID);
      break;

    case UM_SETDIR:
      PaintRecessedWindow(WinWindowFromID(hwnd,SEEF_DIR),(HPS)0,FALSE,TRUE);
      return 0;

    case UM_CONTAINER_FILLED:
      WinSetDlgItemText(hwnd,
                        SEEF_TOTAL,
                        NullStr);
      {
        SHORT sTotal;
        CHAR  s[33];

        sTotal = (SHORT)WinSendDlgItemMsg(hwnd,
                                          SEEF_LISTBOX,
                                          LM_QUERYITEMCOUNT,
                                          MPVOID,
                                          MPVOID);
        if(sTotal >= 0) {
          sprintf(s,
                  "%u",
                  sTotal);
          WinSetDlgItemText(hwnd,
                            SEEF_TOTAL,
                            s);
        }
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case SEEF_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case LN_ENTER:
            case LN_SELECT:
              {
                CHAR *dir,filename[CCHMAXPATH],*p,*f;
                SHORT sSelect;

                sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                                   SEEF_LISTBOX,
                                                   LM_QUERYSELECTION,
                                                   MPFROM2SHORT(LIT_FIRST,0),
                                                   MPVOID);
                if(sSelect >= 0) {
                  dir = WinSendDlgItemMsg(hwnd,
                                          SEEF_LISTBOX,
                                          LM_QUERYITEMHANDLE,
                                          MPFROM2SHORT(sSelect,0),
                                          MPVOID);
                  if(dir) {
                    switch(SHORT2FROMMP(mp1)) {
                      case LN_SELECT:
                        WinSetDlgItemText(hwnd,SEEF_DIR,dir);
                        break;
                      case LN_ENTER:
                        *filename = 0;
                        if(WinSendDlgItemMsg(hwnd,SEEF_LISTBOX,
                                             LM_QUERYITEMTEXT,
                                             MPFROM2SHORT(sSelect,CCHMAXPATH),
                                             MPFROMP(filename)) &&
                           *filename) {
                          f = strchr(filename,'>');
                          if(!f)
                            f = filename;
                          else
                            f++;
                          p = malloc(strlen(dir) + strlen(f) + 1);
                          if(p) {
                            sprintf(p,
                                    "%s%s",
                                    dir,
                                    f);
                            if(!PostMsg(hwnd,
                                        WM_COMMAND,
                                        MPFROM2SHORT(IDM_COLLECT,0),
                                        MPFROMP(p)))
                              free(p);
                          }
                        }
                        break;
                    }
                  }
                }
              }
              break;
          }
          return 0;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_COLLECT:
          if(!mp2)
            break;
          if(!Collector)
            WinSendMsg(hwndMain,WM_COMMAND,MPFROM2SHORT(IDM_COLLECTOR,0),
                       MPVOID);
          else
            WinSetWindowPos(WinQueryWindow(WinQueryWindow(Collector,QW_PARENT),
                            QW_PARENT),HWND_TOP,0,0,0,0,SWP_SHOW |
                            SWP_RESTORE | SWP_ZORDER);
          if(!PostMsg(hwnd,WM_COMMAND,MPFROM2SHORT(IDM_COLLECTOR,0),mp2))
            free(mp2);
          break;

        case IDM_COLLECTOR:
          if(!mp2)
            break;
          DosSleep(64L);
          {
            CHAR **list;

            list = malloc(sizeof(CHAR *) * 2);
            if(list) {
              list[0] = mp2;
              list[1] = NULL;
              if(Collector) {
                if(!PostMsg(Collector,WM_COMMAND,
                               MPFROM2SHORT(IDM_COLLECTOR,0),
                               MPFROMP(list)))
                  FreeList(list);
              }
              else
                FreeList(list);
            }
            else
              free(mp2);
          }
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_SEEWND,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case DID_OK:
        case DID_CANCEL:
          stopflag = 1;
          PostMsg(hwnd,
                  UM_CLOSE,
                  MPVOID,
                  MPVOID);
          break;
      }
      return 0;

    case UM_CLOSE:
      WinSetWindowPos(hwnd,
                      HWND_BOTTOM,
                      0,
                      0,
                      0,
                      0,
                      SWP_SIZE | SWP_MOVE |
                      SWP_MINIMIZE | SWP_HIDE) | SWP_ZORDER;
      _beginthread(EmptyListBox,
                   NULL,
                   32768,
                   (PVOID)hwnd);
      return 0;

    case WM_CLOSE:
      amup = (HWND)0;
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

