
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2005 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  05 Jun 05 SHL Use QWL_USER

***********************************************************************/

#define INCL_WIN
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA1)
#pragma alloc_text(FMEXTRACT,ExtractTextProc,ExtractDlgProc)


MRESULT EXPENTRY ExtractTextProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PFNWP        oldproc = (PFNWP)WinQueryWindowPtr(hwnd,0);
  static BOOL  emphasized = FALSE;

  switch(msg) {
    case DM_DRAGOVER:
      if(!emphasized) {
        emphasized = TRUE;
        DrawTargetEmphasis(hwnd,emphasized);
      }
      if(AcceptOneDrop(mp1,mp2))
        return MRFROM2SHORT(DOR_DROP,
                            DO_MOVE);
      return MRFROM2SHORT(DOR_NEVERDROP,0);

    case DM_DRAGLEAVE:
      if(emphasized) {
        emphasized = FALSE;
        DrawTargetEmphasis(hwnd,emphasized);
      }
      break;

    case DM_DROPHELP:
      DropHelp(mp1,mp2,hwnd,
               GetPString(IDS_EXTDROPHELPTEXT));
      return 0;

    case DM_DROP:
      {
        char szFrom[CCHMAXPATH + 2];

        if(emphasized) {
          emphasized = FALSE;
          DrawTargetEmphasis(hwnd,emphasized);
        }
        if(GetOneDrop(mp1,mp2,szFrom,sizeof(szFrom)))
          WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),WM_COMMAND,
                     MPFROM2SHORT(IDM_SWITCH,0),MPFROMP(szFrom));
      }
      return 0;
  }
  return (oldproc) ? oldproc(hwnd,msg,mp1,mp2) :
                     WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ExtractDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  EXTRDATA    *arcdata = NULL;

  switch(msg) {
    case WM_INITDLG:
      WinSetWindowPtr(hwnd,0,mp2);
      arcdata = (EXTRDATA *)mp2;
      {
        ULONG size = sizeof(BOOL);
        BOOL  fRemember = FALSE;
        PFNWP oldproc;

        oldproc = WinSubclassWindow(WinWindowFromID(hwnd,EXT_DIRECTORY),
                                    (PFNWP)ExtractTextProc);
        if(oldproc)
          WinSetWindowPtr(WinWindowFromID(hwnd,EXT_DIRECTORY),
	                  QWL_USER,
                          (PVOID)oldproc);
        PrfQueryProfileData(fmprof,FM3Str,"RememberExt",
                            (PVOID)&fRemember,&size);
        WinCheckButton(hwnd,EXT_REMEMBER,fRemember);
        WinSendDlgItemMsg(hwnd,EXT_DIRECTORY,EM_SETTEXTLIMIT,
                          MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
        WinSendDlgItemMsg(hwnd,EXT_COMMAND,EM_SETTEXTLIMIT,
                          MPFROM2SHORT(256,0),MPVOID);
        WinSendDlgItemMsg(hwnd,EXT_MASK,EM_SETTEXTLIMIT,
                          MPFROM2SHORT(256,0),MPVOID);
        WinSendDlgItemMsg(hwnd,EXT_FILENAME,EM_SETTEXTLIMIT,
                          MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
        if(arcdata->arcname && *arcdata->arcname)
          WinSetDlgItemText(hwnd,EXT_FILENAME,arcdata->arcname);
        else
          WinSetDlgItemText(hwnd,EXT_FILENAME,
                            GetPString(IDS_EXTVARIOUSTEXT));
        WinSendDlgItemMsg(hwnd,EXT_NORMAL,BM_SETCHECK,
                          MPFROM2SHORT(TRUE,0),MPVOID);
        WinSetDlgItemText(hwnd,EXT_COMMAND,arcdata->info->extract);
        if(fRemember) {

          CHAR textdir[CCHMAXPATH];

          size = sizeof(textdir);
          *textdir = 0;
          PrfQueryProfileData(fmprof,FM3Str,"Ext_ExtractDir",
                              (PVOID)textdir,&size);
          if(*textdir && !IsFile(textdir))
            strcpy(arcdata->extractdir,textdir);
          size = sizeof(textdir);
          *textdir = 0;
          PrfQueryProfileData(fmprof,FM3Str,"Ext_Mask",(PVOID)textdir,&size);
          WinSetDlgItemText(hwnd,EXT_MASK,textdir);
        }
        if(*extractpath && (!fRemember || !*arcdata->extractdir)) {
          if(arcdata->arcname && *arcdata->arcname &&
             !strcmp(extractpath,"*")) {

            CHAR *p;

            strcpy(arcdata->extractdir,arcdata->arcname);
            p = strrchr(arcdata->extractdir,'\\');
            if(p) {
              if(p < arcdata->extractdir + 3)
                p++;
              *p = 0;
            }
          }
          else
            strcpy(arcdata->extractdir,extractpath);
        }
        if(!*arcdata->extractdir) {
          if(*lastextractpath)
            strcpy(arcdata->extractdir,lastextractpath);
          else if(arcdata->arcname && *arcdata->arcname) {

            CHAR *p;

            strcpy(arcdata->extractdir,arcdata->arcname);
            p = strrchr(arcdata->extractdir,'\\');
            if(p) {
              if(p < arcdata->extractdir + 3)
                p++;
              *p = 0;
            }
          }
          if(!*arcdata->extractdir)
            save_dir2(arcdata->extractdir);
        }
        WinSetDlgItemText(hwnd,EXT_DIRECTORY,arcdata->extractdir);
        if(!arcdata->info->exwdirs)
          WinEnableWindow(WinWindowFromID(hwnd,EXT_WDIRS),FALSE);
        else if(fRemember) {
          size = sizeof(BOOL);
          fRemember = FALSE;
          PrfQueryProfileData(fmprof,FM3Str,"Ext_WDirs",
                              (PVOID)&fRemember,&size);
          if(fRemember)
            PostMsg(WinWindowFromID(hwnd,EXT_WDIRS),BM_CLICK,MPVOID,MPVOID);
        }
      }
      *arcdata->command = 0;
      PosOverOkay(hwnd);
      break;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,UM_SETDIR,MPVOID,MPVOID);
      break;

    case UM_SETDIR:
      PaintRecessedWindow(WinWindowFromID(hwnd,EXT_HELP),(HPS)0,FALSE,TRUE);
      return 0;

    case WM_CONTROL:
      arcdata = (EXTRDATA *)WinQueryWindowPtr(hwnd,0);
      switch(SHORT1FROMMP(mp1)) {
        case EXT_REMEMBER:
          {
            BOOL fRemember = WinQueryButtonCheckstate(hwnd,EXT_REMEMBER);

            PrfWriteProfileData(fmprof,FM3Str,"RememberExt",
                                (PVOID)&fRemember,sizeof(BOOL));
          }
          break;

        case EXT_FILENAME:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,EXT_HELP,
                              GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,EXT_HELP,
                              GetPString(IDS_ARCARCNAMEHELPTEXT));
          break;

        case EXT_DIRECTORY:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,EXT_HELP,GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,EXT_HELP,
                              GetPString(IDS_EXTEXTRACTDIRHELPTEXT));
          break;

        case EXT_COMMAND:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,EXT_HELP,
                              GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,EXT_HELP,
                              GetPString(IDS_ARCCMDHELPTEXT));
          break;

        case EXT_MASK:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,EXT_HELP,
                              GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,EXT_HELP,
                              GetPString(IDS_ARCMASKHELPTEXT));
          break;

        case EXT_NORMAL:
          if((BOOL)WinSendDlgItemMsg(hwnd,EXT_NORMAL,BM_QUERYCHECK,
                                     MPVOID,MPVOID))
            WinSetDlgItemText(hwnd,EXT_COMMAND,arcdata->info->extract);
          break;

        case EXT_WDIRS:
          if(arcdata->info->exwdirs) {
            if((BOOL)WinSendDlgItemMsg(hwnd,EXT_WDIRS,BM_QUERYCHECK,
                                       MPVOID,MPVOID))
              WinSetDlgItemText(hwnd,EXT_COMMAND,arcdata->info->exwdirs);
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      arcdata = (EXTRDATA *)WinQueryWindowPtr(hwnd,0);
      switch(SHORT1FROMMP(mp1)) {
        case IDM_SWITCH:
          if(mp2) {

            CHAR tdir[CCHMAXPATH];

            strcpy(tdir,(CHAR *)mp2);
            MakeValidDir(tdir);
            WinSetDlgItemText(hwnd,EXT_DIRECTORY,tdir);
          }
          break;

        case DID_CANCEL:
          arcdata->ret = 0;
          WinDismissDlg(hwnd,0);
          break;
        case DID_OK:
          {
            CHAR s[CCHMAXPATH + 1];
            BOOL fRemember;

            fRemember = WinQueryButtonCheckstate(hwnd,EXT_REMEMBER);
            *s = 0;
            WinQueryDlgItemText(hwnd,EXT_DIRECTORY,CCHMAXPATH,s);
            bstrip(s);
            if(*s) {
              if(!SetDir(WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),
                         QW_OWNER),hwnd,s,0)) {
                strcpy(arcdata->extractdir,s);
                WinSetDlgItemText(hwnd,EXT_DIRECTORY,s);
                if((!isalpha(*s) || s[1] != ':') && *s != '.')
                  saymsg(MB_ENTER,hwnd,
                         GetPString(IDS_WARNINGTEXT),
                         GetPString(IDS_SPECIFYDRIVETEXT));
              }
              else
                break;
              strcpy(lastextractpath,s);
              if(fRemember) {
                PrfWriteProfileString(fmprof,FM3Str,"Ext_ExtractDir",s);
                fRemember = WinQueryButtonCheckstate(hwnd,EXT_WDIRS);
                PrfWriteProfileData(fmprof,FM3Str,"Ext_WDirs",
                                    (PVOID)&fRemember,sizeof(BOOL));
                fRemember = TRUE;
              }
              *s = 0;
              WinQueryDlgItemText(hwnd,EXT_COMMAND,256,s);
              if(*s) {
                strcpy(arcdata->command,s);
                *s = 0;
                WinQueryDlgItemText(hwnd,EXT_MASK,256,s);
                *arcdata->masks = 0;
                strcpy(arcdata->masks,s);
                if(fRemember)
                  PrfWriteProfileString(fmprof,FM3Str,"Ext_Mask",s);
                arcdata->ret = 1;
                WinDismissDlg(hwnd,1);
                break;
              }
            }
          }
          DosBeep(50,100);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_EXTRACT,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case EXT_WALK:
          {
            CHAR temp[CCHMAXPATH + 1];

            strcpy(temp,arcdata->extractdir);
            if(WinDlgBox(HWND_DESKTOP,WinQueryWindow(WinQueryWindow(hwnd,
                         QW_PARENT),QW_OWNER),WalkExtractDlgProc,FM3ModHandle,
                         WALK_FRAME,(PVOID)temp)) {
              if(*temp && stricmp(temp,arcdata->extractdir)) {
                strcpy(arcdata->extractdir,temp);
              }
            }
            WinSetDlgItemText(hwnd,EXT_DIRECTORY,arcdata->extractdir);
          }
          break;

        case EXT_SEE:
          {
            CHAR     s[1001],*p;
            EXECARGS ex;

            WinQueryDlgItemText(hwnd,EXT_COMMAND,256,s);
            lstrip(s);
            if(*s) {
              p = strchr(s,' ');
              if(p)
                *p = 0;
              memset(&ex,0,sizeof(EXECARGS));
              ex.commandline = s;
              ex.flags = WINDOWED | SEPARATEKEEP | MAXIMIZED;
              *ex.path = 0;
              *ex.environment = 0;
              if(WinDlgBox(HWND_DESKTOP,
                           hwnd,
                           CmdLineDlgProc,
                           FM3ModHandle,
                           EXEC_FRAME,
                           MPFROMP(&ex)) &&
                 *s)
                runemf2(ex.flags,
                        hwnd,
                        NULL,
                        (*ex.environment) ? ex.environment : NULL ,
                        "%s",
                        s);
            }
            else
              DosBeep(50,100);
          }
          break;
      }
      return 0;

    case WM_CLOSE:
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}
