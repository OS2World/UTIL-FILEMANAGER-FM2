
/***********************************************************************

  $Id$

  Kill a process

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2006 Steven H. Levine

  24 May 05 SHL Rework Win_Error usage
  14 Jul 06 SHL Use Runtime_Error

***********************************************************************/

#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <process.h>
#include <limits.h>

#include "procstat.h"
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(KILLPROC,FillKillList,FillKillList2,GetDosPgmName,KillDlgProc)

CHAR *GetDosPgmName (PID pid,CHAR *string)
{
  HSWITCH hs;
  SWCNTRL swctl;
  PCH     pch;

  *string = 0;
  hs = WinQuerySwitchHandle(0,pid);
  if(hs) {
    WinQuerySwitchEntry(hs,&swctl);
    pch = swctl.szSwtitle;
    while(*pch) {
      if(*pch < 0x10)
        if(pch != swctl.szSwtitle && *(pch - 1) == 0x20)
          memmove(pch,pch + 1,strlen(pch));
        else {
          *pch = 0x20;
          pch++;
        }
      else
        pch++;
    }
    strcpy(string,swctl.szSwtitle);
  }
  if(!*string)
    strcpy(string,GetPString(IDS_UNKNOWNDOSPROCTEXT));
  return string;
}


VOID FillKillList2 (VOID *arg)
{
  HWND          hwnd = *(HWND *)arg;
  CHAR          s[1036];
  HAB           thab;
  HMQ           thmq;
  INT           rc;
  PROCESSINFO  *ppi;
  BUFFHEADER   *pbh;
  MODINFO      *pmi;

  thab = WinInitialize(0);
  thmq = WinCreateMsgQueue(thab,0);
  WinCancelShutdown(thmq,TRUE);

  WinSendDlgItemMsg(hwnd,KILL_LISTBOX,LM_DELETEALL,MPVOID,MPVOID);
  rc = DosAllocMem((PVOID)&pbh,USHRT_MAX + 4096,
                    PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
  if (rc)
    Dos_Error(MB_CANCEL,rc,HWND_DESKTOP,pszSrcFile,__LINE__,GetPString(IDS_OUTOFMEMORY));
  else {
    rc = DosQProcStatus(pbh,USHRT_MAX);
    if (!rc) {
      ppi = pbh->ppi;
      while(ppi->ulEndIndicator != PROCESS_END_INDICATOR ) {
        if(ppi->pid != mypid) {
          pmi = pbh->pmi;
          while(pmi && ppi->hModRef != pmi->hMod)
            pmi = pmi->pNext;
          if(pmi) {
            sprintf(s,"%04x ",ppi->pid);
            if(!stricmp(pmi->szModName,"SYSINIT"))
              GetDosPgmName(ppi->pid,s + strlen(s));
            else {
              if(*pmi->szModName)
                strcat(s,pmi->szModName);
              else
                strcat(s,GetPString(IDS_UNKNOWNPROCTEXT));
            }
            if (WinIsWindow(thab,hwnd)) {
              WinSendDlgItemMsg(hwnd,KILL_LISTBOX,LM_INSERTITEM,
                                MPFROM2SHORT(LIT_SORTASCENDING,0),
                                             MPFROMP(s));
	    }
            else
              break;
          }
        }
        ppi = (PPROCESSINFO)(ppi->ptiFirst + ppi->usThreadCount);
      } // while
    }
    DosFreeMem(pbh);
  }

  if(WinIsWindow(thab,hwnd))
    PostMsg(hwnd,UM_CONTAINER_FILLED,MPVOID,MPVOID);
  WinDestroyMsgQueue(thmq);
  WinTerminate(thab);
}


VOID FillKillList (VOID *arg)
{
  HWND  hwnd = *(HWND *)arg;
  CHAR  s[1036],progname[1027],*p;
  HAB   thab;
  HMQ   thmq;
  FILE *fp;
  BOOL  foundstart = FALSE;
  INT   rc;
  CHAR *startstring = "Process and Thread Information";
  CHAR *endstring = "System Semaphore Information";
  PID   pid;
  HFILE oldstdout,newstdout;

  DosError(FERR_DISABLEHARDERR);

  thab = WinInitialize(0);
  thmq = WinCreateMsgQueue(thab,0);
  WinCancelShutdown(thmq,TRUE);

  WinSendDlgItemMsg(hwnd,
                    KILL_LISTBOX,
                    LM_DELETEALL,
                    MPVOID,
                    MPVOID);
  strcpy(s,"$PSTAT#$.#$#");
  unlinkf("%s",s);
  fp = fopen(s,"w");
  if(!fp) {
    Win_Error(NULLHANDLE,HWND_DESKTOP,__FILE__,__LINE__,
              GetPString(IDS_REDIRECTERRORTEXT));
    goto Abort;
  }
  else {
    newstdout = -1;
    rc = DosDupHandle(fileno(stdout),&newstdout);
    if (rc)
      Dos_Error(MB_CANCEL,rc,hwnd,__FILE__,__LINE__,"DosDupHandle");
    oldstdout = fileno(stdout);
    DosDupHandle(fileno(fp),&oldstdout);
    rc = runemf2(SEPARATE | INVISIBLE | FULLSCREEN | BACKGROUND | WAIT,
                 hwnd,
                 NULL,
                 NULL,
                 "%s",
                 "PSTAT.EXE /C");
    oldstdout = fileno(stdout);
    DosDupHandle(newstdout,&oldstdout);
    DosClose(newstdout);
    fclose(fp);
    if(rc == -1) {
      saymsg(MB_CANCEL,
             hwnd,
             GetPString(IDS_ARGHTEXT),
             GetPString(IDS_CANTRUNPSTATTEXT));
      goto Abort;
    }
  }
  fp = fopen(s,"r");
  if(fp) {
    while(!feof(fp)) {
      strset(s,0);
      if(!fgets(s,1025,fp))
        break;
      if(!foundstart) {
        if(*s == ' ' && strstr(s,startstring))
          foundstart = TRUE;
      }
      else {
        if(*s == ' ' && strstr(s,endstring))
          break;
        if(*s == ' ' && s[5] == ' ' && isxdigit(s[1]) &&
           isxdigit(s[2]) && isxdigit(s[3]) && isxdigit(s[4])) {
          p = &s[1];
          pid = strtol(&s[1],&p,16);
          if(pid && pid != mypid) {
            strcpy(progname,&s[30]);
            p = strchr(progname,' ');
            if(p)
              *p = 0;
            if(!stristr(progname,"\\PSTAT.EXE")) {
              sprintf(s,"%04x %s",pid,progname);
              WinSendDlgItemMsg(hwnd,
                                KILL_LISTBOX,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_SORTASCENDING,0),
                                MPFROMP(s));
            }
          }
        }
      }
    }
    fclose(fp);
  }
Abort:
  DosForceDelete("$PSTAT#$.#$#");
  PostMsg(hwnd,
          UM_CONTAINER_FILLED,
          MPVOID,
          MPVOID);
  WinDestroyMsgQueue(thmq);
  WinTerminate(thab);
}


MRESULT EXPENTRY KillDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  SHORT           sSelect;
  PID             pid;
  static BOOL     listdone;
  static HPOINTER hptrIcon = (HPOINTER)0;

  switch(msg) {
    case WM_INITDLG:
      hptrIcon = WinLoadPointer(HWND_DESKTOP,
                                FM3ModHandle,
                                KILL_FRAME);
      WinDefDlgProc(hwnd,
                    WM_SETICON,
                    MPFROMLONG(hptrIcon),
                    MPVOID);
      WinCheckButton(hwnd,
                     KILL_CHECKBOX,
                     fUseQProcStat);
      PostMsg(hwnd,
              WM_COMMAND,
              MPFROM2SHORT(KILL_RESCAN,0),
              MPVOID);
      break;

    case UM_CONTAINER_FILLED:
      listdone = TRUE;
      if((SHORT)WinSendDlgItemMsg(hwnd,
                                  KILL_LISTBOX,
                                  LM_QUERYITEMCOUNT,
                                  MPVOID,
                                  MPVOID) == 0) {
        if(!fUseQProcStat)
          saymsg(MB_CANCEL,
                 hwnd,
                 GetPString(IDS_ICHOKEDTEXT),
                 GetPString(IDS_ISPSTATTHERETEXT));
        else
          saymsg(MB_CANCEL,
                 hwnd,
                 GetPString(IDS_ICHOKEDTEXT),
                 GetPString(IDS_DOSQPROCSTATFAILEDTEXT));
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case KILL_CHECKBOX:
          fUseQProcStat = WinQueryButtonCheckstate(hwnd,
                                                   KILL_CHECKBOX);
          PrfWriteProfileData(fmprof,
                              FM3Str,
                              "UseQProcStat",
                              &fUseQProcStat,
                              sizeof(BOOL));
          PostMsg(hwnd,
                  WM_COMMAND,
                  MPFROM2SHORT(KILL_RESCAN,0),
                  MPVOID);
          break;

        case KILL_LISTBOX:
          switch(SHORT2FROMMP(mp2)) {
            case LN_ENTER:
              WinSendDlgItemMsg(hwnd,
                                DID_OK,
                                BM_CLICK,
                                MPFROMSHORT(TRUE),
                                MPVOID);
              break;
          }
          break;

        default:
          break;
      }
      return 0;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,
              UM_STRETCH,
              MPVOID,
              MPVOID);
      break;

    case UM_STRETCH:
      {
        SWP swpC,swp,swpH;

        WinQueryWindowPos(hwnd,&swp);
        if(!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
          WinQueryWindowPos(WinWindowFromID(hwnd,KILL_LISTBOX),&swpC);
          WinQueryWindowPos(WinWindowFromID(hwnd,KILL_HDR),&swpH);
          WinSetWindowPos(WinWindowFromID(hwnd,KILL_LISTBOX),HWND_TOP,
                          SysVal(SV_CXSIZEBORDER),
                          swpC.y,
                          swp.cx - (SysVal(SV_CXSIZEBORDER) * 2),
                          ((swp.cy - swpC.y) - (SysVal(SV_CYTITLEBAR) +
                                               SysVal(SV_CYSIZEBORDER)) -
                           (swpH.cy + 8)),
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,KILL_HDR),HWND_TOP,
                          SysVal(SV_CXSIZEBORDER) + 4,
                          swpC.y + ((swp.cy - swpC.y) -
                                    (SysVal(SV_CYTITLEBAR) +
                                     SysVal(SV_CYSIZEBORDER)) -
                                    (swpH.cy + 4)),
                          swpH.cx,
                          swpH.cy,
                          SWP_MOVE);
        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case KILL_RESCAN:
          listdone = FALSE;
          if(fUseQProcStat) {
            if(_beginthread(FillKillList2,
                            NULL,
                            65536,
                            (PVOID)&hwnd) != -1)
              DosSleep(250L);
            else
              WinDismissDlg(hwnd,0);
          }
          else {
            if(_beginthread(FillKillList,
                            NULL,
                            65536,
                            (PVOID)&hwnd) != -1)
              DosSleep(250L);
            else
              WinDismissDlg(hwnd,0);
          }
          break;

        case KILL_SHOW:
        case DID_OK:
          sSelect = (USHORT)WinSendDlgItemMsg(hwnd,
                                              KILL_LISTBOX,
                                              LM_QUERYSELECTION,
                                              MPFROMSHORT(LIT_FIRST),
                                              MPVOID);
          if(sSelect >= 0) {

            CHAR   s[31],*p;
            APIRET error;

            *s = 0;
            WinSendDlgItemMsg(hwnd,
                              KILL_LISTBOX,
                              LM_QUERYITEMTEXT,
                              MPFROM2SHORT(sSelect,30),
                              MPFROMP(s));
            if(*s) {
              p = s;
              pid = strtol(s,&p,16);
              if(pid) {
                if(SHORT1FROMMP(mp1) == DID_OK) {
                  error = DosKillProcess(DKP_PROCESS,pid);
                  if(error && error != ERROR_INVALID_PROCID) {
                    Dos_Error(MB_CANCEL,
                              error,
                              hwnd,
                              __FILE__,
                              __LINE__,
                              GetPString(IDS_DOSKILLFAILEDTEXT));
		  }
                  else
                    WinSendDlgItemMsg(hwnd,
                                      KILL_LISTBOX,
                                      LM_DELETEITEM,
                                      MPFROM2SHORT(sSelect,0),
                                      MPVOID);
                }
                else if(!ShowSession(hwnd,pid))
                  Notify(GetPString(IDS_SORRYCANTSHOWTEXT));
              }
            }
          }
          break;

        case DID_CANCEL:
          if(!listdone)
            Runtime_Error(pszSrcFile, __LINE__, "busy");
	  else
            WinDismissDlg(hwnd,0);
          break;

        case IDM_HELP:
          saymsg(MB_ENTER | MB_ICONASTERISK,
                 hwnd,
                 GetPString(IDS_KILLPROCHELPTITLETEXT),
                 GetPString(IDS_KILLPROCHELPTEXT));
          break;
      }
      return 0;

    case WM_CLOSE:
      if(!listdone) {
        Runtime_Error(pszSrcFile, __LINE__, "busy");
        return 0;
      }
      break;

    case WM_DESTROY:
      if(hptrIcon)
        WinDestroyPointer(hptrIcon);
      hptrIcon = (HPOINTER)0;
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

