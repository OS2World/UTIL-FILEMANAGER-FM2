#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"

#pragma data_seg(DATA1)
#pragma alloc_text(INSTANT,InstantDlgProc)

#define hwndMLE            WinWindowFromID(hwnd,BAT_MLE)

static INT batches = 0;


MRESULT EXPENTRY InstantDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  CHAR  *path;
  APIRET temp;
  static CHAR *bat = NULL;
  static HWND  myhwnd = (HWND)0;

  switch(msg) {
    case WM_INITDLG:
      if(myhwnd) {
        DosBeep(250,100);
        WinSendMsg(myhwnd,WM_SYSCOMMAND,MPFROM2SHORT(SC_RESTORE,0),MPVOID);
        WinSetActiveWindow(HWND_DESKTOP,myhwnd);
        WinDismissDlg(hwnd,0);
        break;
      }
      if(!mp2) {
        DosBeep(50,100);
        WinDismissDlg(hwnd,0);
        break;
      }
      WinSetWindowPtr(hwnd,0,mp2);
      path = (CHAR *)mp2;
      {
        CHAR s[CCHMAXPATH + 81];

        sprintf(s,
                GetPString(IDS_INSTANTTITLETEXT),
                path);
        WinSetWindowText(hwnd,s);
      }
      WinSendMsg(hwndMLE,
                 MLM_SETTEXTLIMIT,
                 MPFROMLONG((LONG)(10240L)),
                 MPVOID);
      WinSendMsg(hwndMLE,
                 MLM_FORMAT,
                 MPFROM2SHORT(MLFIE_NOTRANS,0),
                 MPVOID);
      if(bat) {

        ULONG tlen = strlen(bat);
        IPT   iptOffset = 0L;

        WinSendMsg(hwndMLE,MLM_SETIMPORTEXPORT,
                   MPFROMP(bat),
                   MPFROMLONG(12287L));
        WinSendMsg(hwndMLE,MLM_IMPORT,MPFROMP(&iptOffset),MPFROMP(tlen));
        DosFreeMem(bat);
        bat = NULL;
      }
      break;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          path = (CHAR *)WinQueryWindowPtr(hwnd,0);
          {
            CHAR  s[CCHMAXPATH + 1];
            FILE *fp;
            IPT   iptOffset = 0L;
            LONG  len,tlen,mem;
            CHAR *rexx = "";

            mem = MLEgetlen(hwndMLE);
            if(mem) {
              if(DosAllocMem((PVOID)&bat,mem,
                               PAG_COMMIT | PAG_READ | PAG_WRITE) ||
                 !bat) {
                DosBeep(50,100);
                WinDismissDlg(hwnd,0);
                break;
              }
              tlen = (LONG)WinSendMsg(hwndMLE,MLM_QUERYTEXTLENGTH,MPVOID,MPVOID);
              if(tlen) {
                WinSendMsg(hwndMLE,MLM_SETIMPORTEXPORT,
                           MPFROMP(bat),
                           MPFROMLONG(mem));
                len = (LONG)WinSendMsg(hwndMLE,MLM_EXPORT,
                                       MPFROMP(&iptOffset),
                                       MPFROMP(&tlen));
                bat[len] = 0;
                lstrip(bat);
                while(strlen(bat) && bat[strlen(bat) - 1] == '\n' ||
                      bat[strlen(bat) - 1] == ' ') {
                  stripcr(bat);
                  rstrip(bat);
                  stripcr(bat);
                  rstrip(bat);
                }
                if(*bat) {
                  sprintf(s,"%s%sFMTMP%d.CMD",path,
                          (path[strlen(path) - 1] == '\\') ? "" : "\\",
                          batches++);
                  fp = fopen(s,"w");
                  if(fp) {
                    if(!strncmp(bat,"/*",2)) {
                      rexx = "'";
                      fprintf(fp,
                              "%s\n",
                              GetPString(IDS_REXXCOMMENT));
                    }
                    fprintf(fp,"%s%c:%s\n",rexx,toupper(*path),rexx);
                    fprintf(fp,"%sCD \"%s%s\"%s\n",rexx,path,
                            (strlen(path) < 3) ? "\\" : "",rexx);
                    fprintf(fp,"%s",bat);
                    fprintf(fp,"\n%sDEL \"%s\"%s\n",rexx,s,rexx);
                    fclose(fp);
                    runemf2(WINDOWED | SEPARATE,
                            hwnd,
                            path,
                            NULL,
                            "%s /C \"%s\"",
                            GetCmdSpec(FALSE),
                            s);
                  }
                  else
                    DosBeep(500,100);
                }
                else
                  DosBeep(50,100);
              }
              else
                DosBeep(50,100);
            }
          }
          WinDismissDlg(hwnd,0);
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;

        case IDM_HELP:
          path = WinQueryWindowPtr(hwnd,0);
          temp = saymsg(MB_YESNOCANCEL,
                        hwnd,
                        GetPString(IDS_INSTANTHELPTITLETEXT),
                        GetPString(IDS_INSTANTHELPTEXT),
                        path,(strlen(path) < 3) ? "\\" : "",path,
                        (path[strlen(path) - 1] == '\\') ? "" : "\\",
                        batches);
          if(temp ==  MBID_YES)
            runemf2(WINDOWED | INVISIBLE | BACKGROUND,
                    hwnd,
                    NULL,
                    NULL,
                    "%s /C HELP BATCH",
                    GetCmdSpec(FALSE));
          else if(temp == MBID_CANCEL)
            WinDismissDlg(hwnd,0);
          break;
      }
      return 0;

    case WM_CLOSE:
    case WM_DESTROY:
      myhwnd = (HWND)0;
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

