
/***********************************************************************

  $Id$

  Configuration notebook

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2006 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  04 Jun 05 SHL Support Cancel button; make Esc key more consistent
  29 May 06 SHL Comments
  17 Jul 06 SHL Use Runtime_Error
  15 Aug 06 SHL Rework SetMask args

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(NOTEBOOK,CfgTDlgProc,CfgTSDlgProc,CfgMDlgProc)
#pragma alloc_text(NOTEBOOK2,CfgADlgProc,CfgSDlgProc,CfgVDlgProc)
#pragma alloc_text(NOTEBOOK3,CfgDDlgProc,Cfg5DlgProc,Cfg6DlgProc)
#pragma alloc_text(NOTEBOOK4,Cfg7DlgProc,Cfg8DlgProc,CfgCDlgProc)
#pragma alloc_text(NOTEBOOK5,CfgGDlgProc,CfgDlgProc,CfgBDlgProc)

typedef struct {
  USHORT frameid;
  ULONG  title;
  PFNWP  proc;
  HWND   hwnd;
  ULONG  helpid;
  ULONG  pageID;
} NOTEPAGES;

static HWND hwndNotebook;


MRESULT EXPENTRY CfgADlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      WinSendDlgItemMsg(hwnd,CFGA_VIRUS,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinSendDlgItemMsg(hwnd,CFGA_EXTRACTPATH,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinEnableWindow(WinWindowFromID(hwnd,CFGA_FIND),FALSE);
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinCheckButton(hwnd,CFGA_ARCSTUFFVISIBLE,fArcStuffVisible);
      WinCheckButton(hwnd,CFGA_FOLDERAFTEREXTRACT,fFolderAfterExtract);
      WinCheckButton(hwnd,CFGA_QUICKARCFIND,fQuickArcFind);
      WinCheckButton(hwnd,CFGA_DEFARC,(*szDefArc != 0));
      WinSetDlgItemText(hwnd,CFGA_DEFARCNAME,szDefArc);
      WinSetDlgItemText(hwnd,CFGA_VIRUS,virus);
      WinSetDlgItemText(hwnd,CFGA_EXTRACTPATH,extractpath);
      break;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFGA_VIRUS:
        case CFGA_EXTRACTPATH:
          switch(SHORT2FROMMP(mp1)) {
            case EN_KILLFOCUS:
              WinEnableWindow(WinWindowFromID(hwnd,CFGA_FIND),FALSE);
              break;
            case EN_SETFOCUS:
              WinEnableWindow(WinWindowFromID(hwnd,CFGA_FIND),TRUE);
              break;
          }
          break;
        case CFGA_DEFARC:
          switch(SHORT2FROMMP(mp1)) {
            case BN_CLICKED:
              if (WinQueryButtonCheckstate(hwnd,CFGA_DEFARC)) {

                ARC_TYPE *pat = arcsighead;	// Hide dups

                if (!WinDlgBox(HWND_DESKTOP,hwnd,
                               SBoxDlgProc,FM3ModHandle,ASEL_FRAME,
                               (PVOID)&pat) ||
		    !pat || !pat->id || !*pat->id) {
                  DosBeep(250,100);	// Complain
                  WinCheckButton(hwnd,CFGA_DEFARC,FALSE);
                }
                else
                  WinSetDlgItemText(hwnd,CFGA_DEFARCNAME,pat->id);
              }
              break;
            default:
              break;
          }
          break;

        default:
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFGA,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case CFGA_FIND:
          {
            CHAR   filename[CCHMAXPATH + 9];
            USHORT id;
            HWND   hwndFocus;

            strcpy(filename,"*.EXE");
            hwndFocus = WinQueryFocus(HWND_DESKTOP);
            if(hwndFocus) {
              id = WinQueryWindowUShort(hwndFocus,QWS_ID);
              switch(id) {
                case CFGA_VIRUS:
                  if(insert_filename(hwnd,filename,2,FALSE) && *filename)
                    WinSetDlgItemText(hwnd,id,filename);
                  break;
                case CFGA_EXTRACTPATH:
                  strcpy(filename,extractpath);
                  if(WinDlgBox(HWND_DESKTOP,hwndNotebook,
                               WalkExtractDlgProc,FM3ModHandle,WALK_FRAME,
                               MPFROMP(filename)) &&
                     *filename)
                    WinSetDlgItemText(hwnd,id,filename);
                  break;
                default:
                  Runtime_Error(pszSrcFile, __LINE__, "bad case");
                  break;
              }
            }
          }
          break;
      }
      return 0;

    case WM_CLOSE:
      fQuickArcFind = WinQueryButtonCheckstate(hwnd,CFGA_QUICKARCFIND);
      PrfWriteProfileData(fmprof,
                          appname,
                          "QuickArcFind",
                          &fQuickArcFind,
                          sizeof(BOOL));
      fArcStuffVisible = WinQueryButtonCheckstate(hwnd,CFGA_ARCSTUFFVISIBLE);
      PrfWriteProfileData(fmprof,
                          appname,
                          "ArcStuffVisible",
                          &fArcStuffVisible,
                          sizeof(BOOL));
      fFolderAfterExtract = WinQueryButtonCheckstate(hwnd,
                                                     CFGA_FOLDERAFTEREXTRACT);
      PrfWriteProfileData(fmprof,
                          appname,
                          "FolderAfterExtract",
                          &fFolderAfterExtract,
                          sizeof(BOOL));
      if(WinQueryButtonCheckstate(hwnd,CFGA_DEFARC)) {

        CHAR temp[CCHMAXPATH];

        *temp = 0;
        WinQueryDlgItemText(hwnd,CFGA_DEFARCNAME,CCHMAXPATH,temp);
        strcpy(szDefArc,temp);
      }
      else
        *szDefArc = 0;
      PrfWriteProfileString(fmprof,appname,"DefArc",szDefArc);
      WinQueryDlgItemText(hwnd,CFGA_VIRUS,CCHMAXPATH,virus);
      virus[CCHMAXPATH - 1] = 0;
      bstrip(virus);
      if(!*virus)
        strcpy(virus,"OS2SCAN.EXE %p /SUB /A");
      WinQueryDlgItemText(hwnd,
                          CFGA_EXTRACTPATH,
                          CCHMAXPATH,
                          extractpath);
      extractpath[CCHMAXPATH - 1] = 0;
      bstrip(extractpath);
      if(*extractpath) {
        if(strcmp(extractpath,"*")) {

          MakeFullName(extractpath);
          if(IsFile(extractpath)) {
            Runtime_Error(pszSrcFile, __LINE__, "%s not a directory", extractpath);
            *extractpath = 0;
          }
        }
      }
      PrfWriteProfileString(fmprof,
                            appname,
                            "Virus",
                            virus);
      PrfWriteProfileString(fmprof,
                            appname,
                            "ExtractPath",
                            extractpath);
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CfgSDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      WinSendDlgItemMsg(hwnd,CFGS_FILESTOGET,SPBM_SETTEXTLIMIT,
                        MPFROMSHORT(4L),MPVOID);
      WinSendDlgItemMsg(hwnd,CFGS_FILESTOGET,SPBM_OVERRIDESETLIMITS,
                        MPFROMLONG(2048L),MPFROMLONG(16L));
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinCheckButton(hwnd,CFGS_NOICONSFILES,(fNoIconsFiles == FALSE));
      WinCheckButton(hwnd,CFGS_NOICONSDIRS,(fNoIconsDirs == FALSE));
      WinCheckButton(hwnd,CFGS_LOADSUBJECTS,fLoadSubject);
      WinCheckButton(hwnd,CFGS_LOADLONGNAMES,fLoadLongnames);
      WinCheckButton(hwnd,CFGS_FORCELOWER,fForceLower);
      WinCheckButton(hwnd,CFGS_FORCEUPPER,fForceUpper);
      WinCheckButton(hwnd,CFGS_NOREMOVABLESCAN,fNoRemovableScan);
      WinCheckButton(hwnd,CFGS_REMOTEBUG,fRemoteBug);
      WinSendDlgItemMsg(hwnd,CFGS_FILESTOGET,SPBM_SETCURRENTVALUE,
                        MPFROMLONG(FilesToGet),MPVOID);
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFGS_FORCEUPPER:
        case CFGS_FORCELOWER:
          {
            BOOL temp;

            temp = WinQueryButtonCheckstate(hwnd,SHORT1FROMMP(mp1));
            if(temp) {
              switch(SHORT1FROMMP(mp1)) {
                case CFGS_FORCEUPPER:
                  WinCheckButton(hwnd,CFGS_FORCELOWER,FALSE);
                  break;
                case CFGS_FORCELOWER:
                  WinCheckButton(hwnd,CFGS_FORCEUPPER,FALSE);
                  break;
              }
            }
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFGS,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      fLoadLongnames = WinQueryButtonCheckstate(hwnd,CFGS_LOADLONGNAMES);
      PrfWriteProfileData(fmprof,appname,"LoadLongname",&fLoadLongnames,sizeof(BOOL));
      fLoadSubject = WinQueryButtonCheckstate(hwnd,CFGS_LOADSUBJECTS);
      PrfWriteProfileData(fmprof,appname,"LoadSubject",&fLoadSubject,sizeof(BOOL));
      fRemoteBug = WinQueryButtonCheckstate(hwnd,CFGS_REMOTEBUG);
      PrfWriteProfileData(fmprof,appname,"RemoteBug",&fRemoteBug,sizeof(BOOL));
      fNoRemovableScan = WinQueryButtonCheckstate(hwnd,CFGS_NOREMOVABLESCAN);
      PrfWriteProfileData(fmprof,FM3Str,"NoRemovableScan",&fNoRemovableScan,
                          sizeof(BOOL));
      fNoIconsFiles = WinQueryButtonCheckstate(hwnd,CFGS_NOICONSFILES);
      fNoIconsFiles = (fNoIconsFiles) ? FALSE : TRUE;
      PrfWriteProfileData(fmprof,appname,"NoIconsFiles",
                          &fNoIconsFiles,sizeof(BOOL));
      fNoIconsDirs = WinQueryButtonCheckstate(hwnd,CFGS_NOICONSDIRS);
      fNoIconsDirs = (fNoIconsDirs) ? FALSE : TRUE;
      PrfWriteProfileData(fmprof,appname,"NoIconsDirs",
                          &fNoIconsDirs,sizeof(BOOL));
      fForceUpper = WinQueryButtonCheckstate(hwnd,CFGS_FORCEUPPER);
      PrfWriteProfileData(fmprof,appname,"ForceUpper",
                          &fForceUpper,sizeof(BOOL));
      fForceLower = WinQueryButtonCheckstate(hwnd,CFGS_FORCELOWER);
      PrfWriteProfileData(fmprof,appname,"ForceLower",
                          &fForceLower,sizeof(BOOL));
      {
        ULONG temp = 0;

        WinSendDlgItemMsg(hwnd,CFGS_FILESTOGET,SPBM_QUERYVALUE,
                          MPFROMP(&temp),MPFROM2SHORT(0,SPBQ_DONOTUPDATE));
        if(temp < 16L)
          temp = 16L;
        else if(temp > 2048L)
          temp = 2048L;
        FilesToGet = temp;
        PrfWriteProfileData(fmprof,
                            appname,
                            "FilesToGet",
                            &FilesToGet,
                            sizeof(ULONG));
      }
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CfgVDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      WinSendDlgItemMsg(hwnd,CFGV_VIEWER,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinSendDlgItemMsg(hwnd,CFGV_EDITOR,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinSendDlgItemMsg(hwnd,CFGV_BINVIEW,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinSendDlgItemMsg(hwnd,CFGV_BINED,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinSendDlgItemMsg(hwnd,CFGV_FTPRUN,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinSendDlgItemMsg(hwnd,CFGV_HTTPRUN,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinEnableWindow(WinWindowFromID(hwnd,CFGV_FIND),FALSE);
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinSetDlgItemText(hwnd,CFGV_VIEWER,viewer);
      WinSetDlgItemText(hwnd,CFGV_EDITOR,editor);
      WinSetDlgItemText(hwnd,CFGV_BINVIEW,binview);
      WinSetDlgItemText(hwnd,CFGV_BINED,bined);
      WinSetDlgItemText(hwnd,CFGV_FTPRUN,ftprun);
      WinSetDlgItemText(hwnd,CFGV_HTTPRUN,httprun);
      WinCheckButton(hwnd,CFGV_USENEWVIEWER,fUseNewViewer);
      WinCheckButton(hwnd,CFGV_GUESSTYPE,fGuessType);
      WinCheckButton(hwnd,CFGV_VIEWCHILD,fViewChild);
      WinCheckButton(hwnd,CFGV_CHECKMM,fCheckMM);
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFGV_VIEWER:
        case CFGV_EDITOR:
        case CFGV_BINVIEW:
        case CFGV_BINED:
        case CFGV_HTTPRUN:
        case CFGV_FTPRUN:
          switch(SHORT2FROMMP(mp1)) {
            case EN_KILLFOCUS:
              WinEnableWindow(WinWindowFromID(hwnd,CFGV_FIND),FALSE);
              break;
            case EN_SETFOCUS:
              WinEnableWindow(WinWindowFromID(hwnd,CFGV_FIND),TRUE);
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFGV,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case CFGV_FIND:
          {
            CHAR   filename[CCHMAXPATH + 9];
            USHORT id;
            HWND   hwndFocus;

            strcpy(filename,"*.EXE");
            hwndFocus = WinQueryFocus(HWND_DESKTOP);
            if(hwndFocus) {
              id = WinQueryWindowUShort(hwndFocus,QWS_ID);
              switch(id) {
                case CFGV_BINVIEW:
                case CFGV_BINED:
                case CFGV_VIEWER:
                case CFGV_EDITOR:
                  if(insert_filename(hwnd,filename,2,FALSE) && *filename) {
                    strcat(filename," %a");
                    WinSetDlgItemText(hwnd,id,filename);
                  }
                  break;
                case CFGV_HTTPRUN:
                case CFGV_FTPRUN:
                  if(insert_filename(hwnd,filename,2,FALSE) && *filename)
                    WinSetDlgItemText(hwnd,id,filename);
                  break;
                default:
                  Runtime_Error(pszSrcFile, __LINE__, "bad case %d", id);
                  break;
              }
            }
          }
          break;
      }
      return 0;

    case WM_CLOSE:
      WinQueryDlgItemText(hwnd,CFGV_VIEWER,CCHMAXPATH,viewer);
      viewer[CCHMAXPATH - 1] = 0;
      bstrip(viewer);
      WinQueryDlgItemText(hwnd,CFGV_EDITOR,CCHMAXPATH,editor);
      editor[CCHMAXPATH - 1] = 0;
      bstrip(editor);
      WinQueryDlgItemText(hwnd,CFGV_BINVIEW,CCHMAXPATH,binview);
      binview[CCHMAXPATH - 1] = 0;
      bstrip(binview);
      WinQueryDlgItemText(hwnd,CFGV_BINED,CCHMAXPATH,bined);
      bined[CCHMAXPATH - 1] = 0;
      bstrip(bined);
      WinQueryDlgItemText(hwnd,CFGV_FTPRUN,CCHMAXPATH,ftprun);
      ftprun[CCHMAXPATH - 1] = 0;
      bstrip(ftprun);
      WinQueryDlgItemText(hwnd,CFGV_HTTPRUN,CCHMAXPATH,httprun);
      httprun[CCHMAXPATH - 1] = 0;
      bstrip(httprun);
      PrfWriteProfileString(fmprof,appname,"Viewer",viewer);
      PrfWriteProfileString(fmprof,appname,"Editor",editor);
      PrfWriteProfileString(fmprof,appname,"BinView",binview);
      PrfWriteProfileString(fmprof,appname,"BinEd",bined);
      PrfWriteProfileString(fmprof,appname,"FTPRun",ftprun);
      PrfWriteProfileString(fmprof,appname,"HTTPRun",httprun);
      fUseNewViewer = WinQueryButtonCheckstate(hwnd,CFGV_USENEWVIEWER);
      PrfWriteProfileData(fmprof,appname,"UseNewViewer",&fUseNewViewer,
                          sizeof(BOOL));
      fGuessType = WinQueryButtonCheckstate(hwnd,CFGV_GUESSTYPE);
      PrfWriteProfileData(fmprof,appname,"GuessType",&fGuessType,sizeof(BOOL));
      fViewChild = WinQueryButtonCheckstate(hwnd,CFGV_VIEWCHILD);
      PrfWriteProfileData(fmprof,appname,"ViewChild",&fViewChild,sizeof(BOOL));
      fCheckMM = WinQueryButtonCheckstate(hwnd,CFGV_CHECKMM);
      PrfWriteProfileData(fmprof,appname,"CheckMM",&fCheckMM,sizeof(BOOL));
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CfgBDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,
              UM_UNDO,
              MPVOID,
              MPVOID);
      break;

    case UM_UNDO:
      WinCheckButton(hwnd,
                     CFGB_TOOLBARHELP,
                     fToolbarHelp);
      WinCheckButton(hwnd,
                     CFGB_DRIVEBARHELP,
                     fDrivebarHelp);
      WinCheckButton(hwnd,
                     CFGB_OTHERHELP,
                     fOtherHelp);
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,
                  UM_UNDO,
                  MPVOID,
                  MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),
                  msg,
                  mp1,
                  mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFGB,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      fToolbarHelp = WinQueryButtonCheckstate(hwnd,
                                              CFGB_TOOLBARHELP);
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "ToolbarHelp",
                          &fToolbarHelp,
                          sizeof(BOOL));
      fDrivebarHelp = WinQueryButtonCheckstate(hwnd,
                                               CFGB_DRIVEBARHELP);
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "DrivebarHelp",
                          &fDrivebarHelp,
                          sizeof(BOOL));
      fOtherHelp = WinQueryButtonCheckstate(hwnd,
                                            CFGB_OTHERHELP);
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "OtherHelp",
                          &fOtherHelp,
                          sizeof(BOOL));
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CfgTSDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  static MASK mask;

  switch(msg) {
    case WM_INITDLG:
      WinSendDlgItemMsg(hwnd,
                        CFG5_FILTER,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),
                        MPVOID);
      PostMsg(hwnd,
              UM_UNDO,
              MPVOID,
              MPVOID);
      break;

    case UM_FOCUSME:
      WinSetFocus(HWND_DESKTOP,
                  WinWindowFromID(hwnd,IDM_HELP));
      return 0;

    case UM_UNDO:
      {
        ULONG flWindowAttr = 0,size = sizeof(ULONG);

        if(!PrfQueryProfileData(fmprof,
                                appname,
                                "TreeflWindowAttr",
                                (PVOID)&flWindowAttr,
                                &size))
          flWindowAttr |= (CV_TREE | CA_TREELINE | CV_ICON | CV_MINI | CV_FLOW);
        WinCheckButton(hwnd,CFG5_ICON,((flWindowAttr & CV_ICON) != FALSE));
        WinCheckButton(hwnd,CFG5_MINIICONS,((flWindowAttr & CV_MINI) != FALSE));
        memset(&mask,0,sizeof(mask));
        mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
                        FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
        mask.fIsTree = TRUE;
        size = sizeof(MASK);
        if(PrfQueryProfileData(fmprof,appname,"TreeFilter",&mask,&size)) {
          SetMask(NULL,&mask);
        }
        if(!mask.attrFile)
          mask.attrFile = (FILE_READONLY | FILE_NORMAL |
                           FILE_ARCHIVED | FILE_DIRECTORY |
                           FILE_HIDDEN   | FILE_SYSTEM);
        strcpy(mask.prompt,
               GetPString(IDS_TREEFILTERTITLETEXT));
        WinSetDlgItemText(hwnd,CFG5_FILTER,mask.szMask);
        WinCheckButton(hwnd,CFG6_SORTFIRST,FALSE);
        WinCheckButton(hwnd,CFG6_SORTLAST,FALSE);
        WinCheckButton(hwnd,CFG6_SORTSIZE,FALSE);
        WinCheckButton(hwnd,CFG6_SORTEASIZE,FALSE);
        WinCheckButton(hwnd,CFG6_SORTLWDATE,FALSE);
        WinCheckButton(hwnd,CFG6_SORTLADATE,FALSE);
        WinCheckButton(hwnd,CFG6_SORTCRDATE,FALSE);
        WinCheckButton(hwnd,CFG6_SORTNAME,FALSE);
        WinCheckButton(hwnd,CFG6_SORTREVERSE,FALSE);
        if(TreesortFlags & SORT_FIRSTEXTENSION)
          WinCheckButton(hwnd,CFG6_SORTFIRST,TRUE);
        else if(TreesortFlags & SORT_LASTEXTENSION)
          WinCheckButton(hwnd,CFG6_SORTLAST,TRUE);
        else if(TreesortFlags & SORT_SIZE)
          WinCheckButton(hwnd,CFG6_SORTSIZE,TRUE);
        else if(TreesortFlags & SORT_EASIZE)
          WinCheckButton(hwnd,CFG6_SORTEASIZE,TRUE);
        else if(TreesortFlags & SORT_LWDATE)
          WinCheckButton(hwnd,CFG6_SORTLWDATE,TRUE);
        else if(TreesortFlags & SORT_LADATE)
          WinCheckButton(hwnd,CFG6_SORTLADATE,TRUE);
        else if(TreesortFlags & SORT_CRDATE)
          WinCheckButton(hwnd,CFG6_SORTCRDATE,TRUE);
        else if(TreesortFlags & SORT_FILENAME)
          WinCheckButton(hwnd,CFG6_SORTFILENAME,TRUE);
        else
          WinCheckButton(hwnd,CFG6_SORTNAME,TRUE);
        if(TreesortFlags & SORT_REVERSE)
          WinCheckButton(hwnd,CFG6_SORTREVERSE,TRUE);
      }
      return 0;

    case UM_SETUP5:
      if(WinDlgBox(HWND_DESKTOP,hwndNotebook,PickMaskDlgProc,
                   FM3ModHandle,MSK_FRAME,MPFROMP(&mask))) {
        SetMask(NULL,&mask);
        WinSetDlgItemText(hwnd,CFG5_FILTER,mask.szMask);
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFG5_FILTER:
          switch(SHORT2FROMMP(mp1)) {
            case EN_SETFOCUS:
              PostMsg(hwnd,
                      UM_FOCUSME,
                      MPVOID,
                      MPVOID);
              PostMsg(hwnd,
                      UM_SETUP5,
                      MPVOID,
                      MPVOID);
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_TREEVIEW,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      {
        ULONG flWindowAttr = 0;

        if(WinQueryButtonCheckstate(hwnd,CFG5_ICON))
          flWindowAttr |= CV_ICON;
        else
          flWindowAttr |= CV_TEXT;
        if(WinQueryButtonCheckstate(hwnd,CFG5_MINIICONS))
          flWindowAttr |= CV_MINI;
        flWindowAttr |= (CV_TREE | CV_FLOW | CA_TREELINE);
        PrfWriteProfileData(fmprof,
                            appname,
                            "TreeflWindowAttr",
                            &flWindowAttr,
                            sizeof(ULONG));
        if(hwndTree) {

          CNRINFO cnri;

          memset(&cnri,0,sizeof(cnri));
          cnri.cb = sizeof(cnri);
          WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                     CM_QUERYCNRINFO,MPFROMP(&cnri),MPFROMLONG(sizeof(cnri)));
          cnri.flWindowAttr = flWindowAttr;
          WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                     CM_SETCNRINFO,MPFROMP(&cnri),MPFROMLONG(CMA_FLWINDOWATTR));
        }
      }
      TreesortFlags = 0;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTFILENAME))
        TreesortFlags |= SORT_FILENAME;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTSIZE))
        TreesortFlags |= SORT_SIZE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTEASIZE))
        TreesortFlags |= SORT_EASIZE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTFIRST))
        TreesortFlags |= SORT_FIRSTEXTENSION;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTLAST))
        TreesortFlags |= SORT_LASTEXTENSION;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTLWDATE))
        TreesortFlags |= SORT_LWDATE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTLADATE))
        TreesortFlags |= SORT_LADATE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTCRDATE))
        TreesortFlags |= SORT_CRDATE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTDIRSFIRST))
        TreesortFlags |= SORT_DIRSFIRST;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTDIRSLAST))
        TreesortFlags |= SORT_DIRSLAST;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTREVERSE))
        TreesortFlags |= SORT_REVERSE;
      PrfWriteProfileData(fmprof,appname,"TreeSort",&TreesortFlags,sizeof(INT));
      if(hwndTree)
        PostMsg(hwndTree,
                WM_COMMAND,
                MPFROM2SHORT(IDM_RESORT,0),
                MPVOID);
      *mask.prompt = 0;
      PrfWriteProfileData(fmprof,appname,"TreeFilter",&mask,sizeof(MASK));
      if(hwndTree) {

        DIRCNRDATA *dcd;

        dcd = WinQueryWindowPtr(WinWindowFromID(WinWindowFromID(hwndTree,
                                                FID_CLIENT),TREE_CNR),
				QWL_USER);
        if(dcd && dcd->size == sizeof(DIRCNRDATA)) {
          dcd->mask = mask;
          PostMsg(hwndTree,UM_FILTER,MPVOID,MPVOID);
        }
      }
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CfgTDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinCheckButton(hwnd,CFGT_FOLLOWTREE,fFollowTree);
      WinCheckButton(hwnd,CFGT_TOPDIR,fTopDir);
      WinCheckButton(hwnd,CFGT_DCOPENS,fDCOpens);
      WinCheckButton(hwnd,CFGT_VTREEOPENSWPS,fVTreeOpensWPS);
      WinCheckButton(hwnd,CFGT_COLLAPSEFIRST,fCollapseFirst);
      WinCheckButton(hwnd,CFGT_SWITCHTREEONFOCUS,fSwitchTreeOnFocus);
      WinCheckButton(hwnd,CFGT_SWITCHTREE,fSwitchTree);
      WinCheckButton(hwnd,CFGT_SWITCHTREEEXPAND,fSwitchTreeExpand);
      WinCheckButton(hwnd,CFGT_SHOWENV,fShowEnv);
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFGT,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      fVTreeOpensWPS = WinQueryButtonCheckstate(hwnd,CFGT_VTREEOPENSWPS);
      PrfWriteProfileData(fmprof,FM3Str,"VTreeOpensWPS",&fVTreeOpensWPS,
                          sizeof(BOOL));
      fCollapseFirst = WinQueryButtonCheckstate(hwnd,CFGT_COLLAPSEFIRST);
      PrfWriteProfileData(fmprof,appname,"CollapseFirst",&fCollapseFirst,
                          sizeof(BOOL));
      fSwitchTreeOnFocus = WinQueryButtonCheckstate(hwnd,
                                                    CFGT_SWITCHTREEONFOCUS);
      PrfWriteProfileData(fmprof,appname,"SwitchTreeOnFocus",
                          &fSwitchTreeOnFocus,sizeof(BOOL));
      fSwitchTreeExpand = WinQueryButtonCheckstate(hwnd,CFGT_SWITCHTREEEXPAND);
      PrfWriteProfileData(fmprof,appname,"SwitchTreeExpand",&fSwitchTreeExpand,
                          sizeof(BOOL));
      fSwitchTree = WinQueryButtonCheckstate(hwnd,CFGT_SWITCHTREE);
      PrfWriteProfileData(fmprof,appname,"SwitchTree",&fSwitchTree,
                          sizeof(BOOL));
      fFollowTree = WinQueryButtonCheckstate(hwnd,CFGT_FOLLOWTREE);
      PrfWriteProfileData(fmprof,appname,"FollowTree",&fFollowTree,
                          sizeof(BOOL));
      fTopDir = WinQueryButtonCheckstate(hwnd,CFGT_TOPDIR);
      PrfWriteProfileData(fmprof,appname,"TopDir",(PVOID)&fTopDir,
                          sizeof(BOOL));
      fDCOpens = WinQueryButtonCheckstate(hwnd,CFGT_DCOPENS);
      PrfWriteProfileData(fmprof,FM3Str,"DoubleClickOpens",&fDCOpens,
                          sizeof(BOOL));
      if(hwndTree &&
        fShowEnv != WinQueryButtonCheckstate(hwnd,CFGT_SHOWENV))
        PostMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                WM_COMMAND,
                MPFROM2SHORT(IDM_RESCAN,0),
                MPVOID);
      fShowEnv = WinQueryButtonCheckstate(hwnd,CFGT_SHOWENV);
      PrfWriteProfileData(fmprof,appname,"ShowEnv",&fShowEnv,
                          sizeof(BOOL));
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CfgGDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinCheckButton(hwnd,CFGG_CONFIRMDELETE,fConfirmDelete);
      WinCheckButton(hwnd,CFGG_VERIFYWRITES,fVerify);
      WinCheckButton(hwnd,CFGG_LINKSETSICON,fLinkSetsIcon);
      WinCheckButton(hwnd,CFGG_DONTMOVEMOUSE,fDontMoveMouse);
      WinCheckButton(hwnd,CFGG_DEFAULTCOPY,fCopyDefault);
      WinCheckButton(hwnd,CFGG_IDLECOPY,fRealIdle);
      WinCheckButton(hwnd,CFGG_DNDDLG,fDragndropDlg);
      WinCheckButton(hwnd,CFGG_DEFAULTDELETEPERM,fDefaultDeletePerm);
      {
        long th;

        th = (fNoFinger) ? 2 :
              (fNoDead) ? 1 : 0;
        WinCheckButton(hwnd,CFGG_NODEAD,th);
      }
      WinCheckButton(hwnd,CFGG_BORING,fDullMin);
      WinCheckButton(hwnd,CFGG_CUSTOMFILEDLG,fCustomFileDlg);
      WinCheckButton(hwnd,CFGG_FM2DELETES,fFM2Deletes);
      WinCheckButton(hwnd,CFGG_CONFIRMTARGET,fConfirmTarget);
      WinSetDlgItemText(hwnd,CFGG_TARGETDIR,targetdir);
      return 0;

    case UM_FOCUSME:
      WinSetFocus(HWND_DESKTOP,
                  WinWindowFromID(hwnd,IDM_HELP));
      return 0;

    case UM_SETUP5:
      SetTargetDir(hwnd,FALSE);
      WinSetDlgItemText(hwnd,
                        CFGG_TARGETDIR,
                        targetdir);
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFGG_TARGETDIR:
          switch(SHORT2FROMMP(mp1)) {
            case EN_SETFOCUS:
              PostMsg(hwnd,
                      UM_FOCUSME,
                      MPVOID,
                      MPVOID);
              PostMsg(hwnd,
                      UM_SETUP5,
                      MPVOID,
                      MPVOID);
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFGG,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      {
        long test;

        test = WinQueryButtonCheckstate(hwnd,
                                        CFGG_NODEAD);
        fNoDead = (test == 1);
        fNoFinger = (test == 2);
        PrfWriteProfileData(fmprof,
                            FM3Str,
                            "NoDead",
                            &fNoDead,
                            sizeof(BOOL));
        PrfWriteProfileData(fmprof,
                            FM3Str,
                            "NoFinger",
                            &fNoFinger,
                            sizeof(BOOL));
        WinDestroyPointer(hptrFinger);
        if(!fNoDead)
          hptrFinger  = WinLoadPointer(HWND_DESKTOP,
                                       FM3ModHandle,
                                       FINGER_ICON);
        else
          hptrFinger  = WinLoadPointer(HWND_DESKTOP,
                                       FM3ModHandle,
                                       FINGER2_ICON);
      }
      fLinkSetsIcon = WinQueryButtonCheckstate(hwnd,
                                               CFGG_LINKSETSICON);
      PrfWriteProfileData(fmprof,
                          appname,
                          "LinkSetsIcon",
                          &fLinkSetsIcon,
                          sizeof(BOOL));
      fCustomFileDlg = WinQueryButtonCheckstate(hwnd,
                                                CFGG_CUSTOMFILEDLG);
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "CustomFileDlg",
                          &fCustomFileDlg,
                          sizeof(BOOL));
      fDullMin = WinQueryButtonCheckstate(hwnd,
                                          CFGG_BORING);
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "DullDatabar",
                          &fDullMin,
                          sizeof(BOOL));
      fConfirmDelete = WinQueryButtonCheckstate(hwnd,
                                                CFGG_CONFIRMDELETE);
      PrfWriteProfileData(fmprof,
                          appname,
                          "ConfirmDelete",
                          &fConfirmDelete,
                          sizeof(BOOL));
      fDontMoveMouse = WinQueryButtonCheckstate(hwnd,
                                                CFGG_DONTMOVEMOUSE);
      PrfWriteProfileData(fmprof,
                          appname,
                          "DontMoveMouse",
                          &fDontMoveMouse,
                          sizeof(BOOL));
      fCopyDefault = WinQueryButtonCheckstate(hwnd,CFGG_DEFAULTCOPY);
      PrfWriteProfileData(fmprof,appname,"DefaultCopy",
                          &fCopyDefault,sizeof(BOOL));
      fRealIdle = WinQueryButtonCheckstate(hwnd,CFGG_IDLECOPY);
      PrfWriteProfileData(fmprof,appname,"IdleCopy",
                          &fRealIdle,sizeof(BOOL));
      fDragndropDlg = WinQueryButtonCheckstate(hwnd,CFGG_DNDDLG);
      PrfWriteProfileData(fmprof,appname,"Drag&DropDlg",
                          &fDragndropDlg,sizeof(BOOL));
      fVerify = WinQueryButtonCheckstate(hwnd,CFGG_VERIFYWRITES);
      PrfWriteProfileData(fmprof,appname,"VerifyWrites",
                          &fVerify,sizeof(BOOL));
      DosSetVerify(fVerify);
      fDefaultDeletePerm = WinQueryButtonCheckstate(hwnd,
                                                    CFGG_DEFAULTDELETEPERM);
      PrfWriteProfileData(fmprof,appname,"DefaultDeletePerm",
                          &fDefaultDeletePerm,sizeof(BOOL));
      fFM2Deletes = WinQueryButtonCheckstate(hwnd,CFGG_FM2DELETES);
      PrfWriteProfileData(fmprof,FM3Str,"FM2Deletes",
                          &fFM2Deletes,sizeof(BOOL));
      fConfirmTarget = WinQueryButtonCheckstate(hwnd,CFGG_CONFIRMTARGET);
      PrfWriteProfileData(fmprof,appname,"ConfirmTarget",
                          &fConfirmTarget,sizeof(BOOL));
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CfgCDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      WinSendDlgItemMsg(hwnd,CFGC_COMPARE,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinSendDlgItemMsg(hwnd,CFGC_DIRCOMPARE,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinEnableWindow(WinWindowFromID(hwnd,CFGC_FIND),FALSE);
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinSetDlgItemText(hwnd,CFGC_COMPARE,compare);
      WinSetDlgItemText(hwnd,CFGC_DIRCOMPARE,dircompare);
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFGC_COMPARE:
        case CFGC_DIRCOMPARE:
          switch(SHORT2FROMMP(mp1)) {
            case EN_KILLFOCUS:
              WinEnableWindow(WinWindowFromID(hwnd,CFGC_FIND),FALSE);
              break;
            case EN_SETFOCUS:
              WinEnableWindow(WinWindowFromID(hwnd,CFGC_FIND),TRUE);
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFGC,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case CFGC_FIND:
          {
            CHAR   filename[CCHMAXPATH + 9];
            USHORT id;
            HWND   hwndFocus;

            strcpy(filename,"*.EXE");
            hwndFocus = WinQueryFocus(HWND_DESKTOP);
            if(hwndFocus) {
              id = WinQueryWindowUShort(hwndFocus,QWS_ID);
              switch(id) {
                case CFGC_COMPARE:
                case CFGC_DIRCOMPARE:
                  if(insert_filename(hwnd,filename,2,FALSE) && *filename) {
                    strcat(filename," %a");
                    WinSetDlgItemText(hwnd,id,filename);
                  }
                  break;
                default:
                  Runtime_Error(pszSrcFile, __LINE__, "bad case %d", id);
                  break;
              }
            }
          }
          break;
      }
      return 0;

    case WM_CLOSE:
      WinQueryDlgItemText(hwnd,
                          CFGC_DIRCOMPARE,
                          CCHMAXPATH,
                          dircompare);
      dircompare[CCHMAXPATH - 1] = 0;
      bstrip(dircompare);
      PrfWriteProfileString(fmprof,
                            appname,
                            "DirCompare",
                            dircompare);
      WinQueryDlgItemText(hwnd,
                          CFGC_COMPARE,
                          CCHMAXPATH,
                          compare);
      compare[CCHMAXPATH - 1] = 0;
      bstrip(compare);
      PrfWriteProfileString(fmprof,
                            appname,
                            "Compare",
                            compare);
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CfgDDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinCheckButton(hwnd,CFGD_UNHILITE,fUnHilite);
      WinCheckButton(hwnd,CFGD_SYNCUPDATES,fSyncUpdates);
      WinCheckButton(hwnd,CFGD_LOOKINDIR,fLookInDir);
      WinCheckButton(hwnd,CFGD_MINONOPEN,fMinOnOpen);
      WinCheckButton(hwnd,CFGD_SELECTEDALWAYS,fSelectedAlways);
      WinCheckButton(hwnd,CFGD_NOSEARCH,fNoSearch);
      WinCheckButton(hwnd,CFGD_EXTENDEDSEL,
                     ((ulCnrType & CCS_EXTENDSEL) != 0));
      WinCheckButton(hwnd,CFGD_MULTIPLESEL,
                     ((ulCnrType & CCS_MULTIPLESEL) != 0));
      WinCheckButton(hwnd,CFGD_LEAVETREE,fLeaveTree);
      WinCheckButton(hwnd,CFGD_NOFOLDMENU,fNoFoldMenu);
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFGD,0),
                       MPFROMSHORT(HM_RESOURCEID));
            break;
      }
      return 0;

    case WM_CLOSE:
      ulCnrType = 0;
      if(WinQueryButtonCheckstate(hwnd,CFGD_EXTENDEDSEL))
        ulCnrType |= CCS_EXTENDSEL;
      if(WinQueryButtonCheckstate(hwnd,CFGD_MULTIPLESEL))
        ulCnrType |= CCS_MULTIPLESEL;
      PrfWriteProfileData(fmprof,appname,"ContainerType",
                          (PVOID)&ulCnrType,sizeof(BOOL));
      fMinOnOpen = WinQueryButtonCheckstate(hwnd,CFGD_MINONOPEN);
      PrfWriteProfileData(fmprof,FM3Str,"MinDirOnOpen",&fMinOnOpen,
                          sizeof(BOOL));
      fLeaveTree = WinQueryButtonCheckstate(hwnd,CFGD_LEAVETREE);
      PrfWriteProfileData(fmprof,appname,"LeaveTree",&fLeaveTree,sizeof(BOOL));
      fNoFoldMenu = WinQueryButtonCheckstate(hwnd,CFGD_NOFOLDMENU);
      PrfWriteProfileData(fmprof,appname,"NoFoldMenu",&fNoFoldMenu,
                          sizeof(BOOL));
      fSelectedAlways = WinQueryButtonCheckstate(hwnd,CFGD_SELECTEDALWAYS);
      PrfWriteProfileData(fmprof,appname,"SelectedAlways",&fSelectedAlways,
                          sizeof(BOOL));
      fNoSearch = WinQueryButtonCheckstate(hwnd,CFGD_NOSEARCH);
      PrfWriteProfileData(fmprof,appname,"NoSearch",&fNoSearch,sizeof(BOOL));
      fLookInDir = WinQueryButtonCheckstate(hwnd,CFGD_LOOKINDIR);
      PrfWriteProfileData(fmprof,FM3Str,"LookInDir",(PVOID)&fLookInDir,
                          sizeof(BOOL));
      fUnHilite = WinQueryButtonCheckstate(hwnd,CFGD_UNHILITE);
      PrfWriteProfileData(fmprof,appname,"UnHilite",
                          &fUnHilite,sizeof(BOOL));
      {
        BOOL dummy = WinQueryButtonCheckstate(hwnd,CFGD_SYNCUPDATES);

        if(dummy != fSyncUpdates) {
          fSyncUpdates = dummy;
          if(hwndMain && !strcmp(realappname,FM3Str)) {
            if(SaveDirCnrState(hwndMain,GetPString(IDS_FM2TEMPTEXT))) {
              PostMsg(MainObjectHwnd,UM_RESTORE,MPVOID,MPFROMLONG(2L));
              PostMsg(hwndMain,UM_RESTORE,MPVOID,MPVOID);
            }
          }
        }
      }
      PrfWriteProfileData(fmprof,appname,"SyncUpdates",
                          &fSyncUpdates,sizeof(BOOL));
      if(!(ulCnrType & (CCS_EXTENDSEL | CCS_MULTIPLESEL)))
        saymsg(MB_ENTER | MB_ICONEXCLAMATION,
               HWND_DESKTOP,
               GetPString(IDS_WARNINGTEXT),
               GetPString(IDS_SELECTTYPEERRORTEXT));
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CfgMDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinCheckButton(hwnd,CFGM_EXTERNALINIS,fExternalINIs);
      WinCheckButton(hwnd,CFGM_EXTERNALARCBOXES,fExternalArcboxes);
      WinCheckButton(hwnd,CFGM_EXTERNALVIEWER,fExternalViewer);
      WinCheckButton(hwnd,CFGM_EXTERNALCOLLECTOR,fExternalCollector);
      WinCheckButton(hwnd,CFGM_SAVESTATE,fSaveState);
      WinCheckButton(hwnd,CFGM_AUTOTILE,(fAutoTile));
      WinCheckButton(hwnd,CFGM_FREETREE,(fFreeTree));
      WinCheckButton(hwnd,CFGM_SPLITSTATUS,(fSplitStatus));
      WinCheckButton(hwnd,CFGM_NOTREEGAP,(fNoTreeGap));
      WinCheckButton(hwnd,CFGM_STARTMIN,(fStartMinimized));
      WinCheckButton(hwnd,CFGM_STARTMAX,(fStartMaximized));
      WinCheckButton(hwnd,CFGM_DATAMIN,(fDataMin));
      WinCheckButton(hwnd,CFGM_TILEBACKWARDS,(fTileBackwards));
      {
        long th;

        th = (fAutoAddDirs && fAutoAddAllDirs) ? 2 :
              (fAutoAddDirs) ? 1 : 0;
        WinCheckButton(hwnd,CFGM_RECENTDIRS,th);
        WinSendMsg(hwnd,
                   UM_RESTORE,
                   MPFROM2SHORT(CFGM_RECENTDIRS,0),
                   MPVOID);
      }
      WinCheckButton(hwnd,CFGM_USERLISTSWITCHES,fUserListSwitches);
      WinCheckButton(hwnd,CFGM_WSANIMATE,(fwsAnimate != 0));
      WinCheckButton(hwnd,CFGM_SEPARATEPARMS,fSeparateParms);
      WinCheckButton(hwnd,CFGM_BLUELED,fBlueLED);
      WinCheckButton(hwnd,CFGM_SHOWTARGET,fShowTarget);
      WinEnableWindow(WinWindowFromID(hwnd,CFGM_STARTMAX),!(fStartMinimized));
      WinEnableWindow(WinWindowFromID(hwnd,CFGM_STARTMIN),!(fStartMaximized));
      return 0;

    case UM_RESTORE:
      {
        long test;
        BOOL th,oh;
        char s[80];

        test = WinQueryButtonCheckstate(hwnd,
                                        SHORT1FROMMP(mp1));
        th = (test != 0);
        oh = (test == 1);
        *s = 0;
        switch(SHORT1FROMMP(mp1)) {
          case CFGM_RECENTDIRS:
            sprintf(s,
                    GetPString(IDS_RECENTHELPWHICHTEXT),
                    (!oh && th) ?
                     GetPString(IDS_RECENTONLYTEXT) :
                      (oh && th) ?
                       GetPString(IDS_ALLONLYTEXT) :
                       GetPString(IDS_NONE));
            break;
        }
        if(*s)
          WinSetDlgItemText(hwnd,
                            SHORT1FROMMP(mp1),
                            s);
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFGM_RECENTDIRS:
          WinSendMsg(hwnd,UM_RESTORE,mp1,MPVOID);
          break;
        case CFGM_STARTMIN:
          if(WinQueryButtonCheckstate(hwnd,CFGM_STARTMIN)) {
            WinCheckButton(hwnd,CFGM_STARTMAX,FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,CFGM_STARTMAX),FALSE);
          }
          else
            WinEnableWindow(WinWindowFromID(hwnd,CFGM_STARTMAX),TRUE);
          break;
        case CFGM_STARTMAX:
          if(WinQueryButtonCheckstate(hwnd,CFGM_STARTMAX)) {
            WinCheckButton(hwnd,CFGM_STARTMIN,FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,CFGM_STARTMIN),FALSE);
          }
          else
            WinEnableWindow(WinWindowFromID(hwnd,CFGM_STARTMIN),TRUE);
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFGM,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      if(hwndMain && !strcmp(realappname,FM3Str)) {
        if(fFreeTree != WinQueryButtonCheckstate(hwnd,CFGM_FREETREE))
          PostMsg(hwndMain,WM_COMMAND,
                     MPFROM2SHORT(IDM_FREETREE,0),MPVOID);
        if(fAutoTile != WinQueryButtonCheckstate(hwnd,CFGM_AUTOTILE))
          PostMsg(hwndMain,WM_COMMAND,
                     MPFROM2SHORT(IDM_AUTOTILE,0),MPVOID);
        if(fSplitStatus != WinQueryButtonCheckstate(hwnd,CFGM_SPLITSTATUS)) {
          fSplitStatus = (fSplitStatus) ? FALSE : TRUE;
          PostMsg(hwndMain,WM_COMMAND,
                     MPFROM2SHORT(IDM_BLINK,0),MPVOID);
          PrfWriteProfileData(fmprof,FM3Str,"SplitStatus",&fSplitStatus,
                              sizeof(BOOL));
        }
      }
      fUserListSwitches = WinQueryButtonCheckstate(hwnd,CFGM_USERLISTSWITCHES);
      PrfWriteProfileData(fmprof,FM3Str,"UserListSwitches",
                          (PVOID)&fUserListSwitches,sizeof(BOOL));
      fExternalINIs = WinQueryButtonCheckstate(hwnd,CFGM_EXTERNALINIS);
      PrfWriteProfileData(fmprof,FM3Str,"ExternalINIs",(PVOID)&fExternalINIs,
                          sizeof(BOOL));
      fExternalArcboxes = WinQueryButtonCheckstate(hwnd,CFGM_EXTERNALARCBOXES);
      PrfWriteProfileData(fmprof,FM3Str,"ExternalArcboxes",
                         (PVOID)&fExternalArcboxes,sizeof(BOOL));
      fExternalCollector = WinQueryButtonCheckstate(hwnd,CFGM_EXTERNALCOLLECTOR);
      PrfWriteProfileData(fmprof,FM3Str,"ExternalCollector",
                          (PVOID)&fExternalCollector,sizeof(BOOL));
      fExternalViewer = WinQueryButtonCheckstate(hwnd,CFGM_EXTERNALVIEWER);
      PrfWriteProfileData(fmprof,FM3Str,"ExternalViewer",
                          (PVOID)&fExternalViewer,sizeof(BOOL));
      {
        long test;

        test = WinQueryButtonCheckstate(hwnd,CFGM_RECENTDIRS);
        fAutoAddDirs = (test != 0);
        fAutoAddAllDirs = (test == 2);
      }
      PrfWriteProfileData(fmprof,
                          appname,
                          "AutoAddDirs",
                          (PVOID)&fAutoAddDirs,
                          sizeof(BOOL));
      PrfWriteProfileData(fmprof,
                          appname,
                          "AutoAddAllDirs",
                          (PVOID)&fAutoAddAllDirs,
                          sizeof(BOOL));
      fwsAnimate = WinQueryButtonCheckstate(hwnd,CFGM_WSANIMATE);
      if(fwsAnimate)
        fwsAnimate = WS_ANIMATE;
      PrfWriteProfileData(fmprof,
                          appname,
                          "WS_ANIMATE",
                          (PVOID)&fwsAnimate,
                          sizeof(ULONG));
      fSaveState = WinQueryButtonCheckstate(hwnd,CFGM_SAVESTATE);
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "SaveState",
                          (PVOID)&fSaveState,
                          sizeof(BOOL));
      fStartMinimized = WinQueryButtonCheckstate(hwnd,CFGM_STARTMIN);
      PrfWriteProfileData(fmprof,
                          appname,
                          "StartMinimized",
                          (PVOID)&fStartMinimized,
                          sizeof(BOOL));
      fStartMaximized = WinQueryButtonCheckstate(hwnd,CFGM_STARTMAX);
      PrfWriteProfileData(fmprof,
                          appname,
                          "StartMaximized",
                          (PVOID)&fStartMaximized,
                          sizeof(BOOL));
      fDataMin = WinQueryButtonCheckstate(hwnd,CFGM_DATAMIN);
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "DataMin",
                          (PVOID)&fDataMin,
                          sizeof(BOOL));
      fTileBackwards = WinQueryButtonCheckstate(hwnd,CFGM_TILEBACKWARDS);
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "TileBackwards",
                          (PVOID)&fTileBackwards,
                          sizeof(BOOL));
      fNoTreeGap = WinQueryButtonCheckstate(hwnd,CFGM_NOTREEGAP);
      PrfWriteProfileData(fmprof,
                          FM3Str,
                          "NoTreeGap",
                          (PVOID)&fNoTreeGap,
                          sizeof(BOOL));
      fBlueLED = WinQueryButtonCheckstate(hwnd,CFGM_BLUELED);
      PrfWriteProfileData(fmprof,
                          appname,
                          "BlueLED",
                          (PVOID)&fBlueLED,
                          sizeof(BOOL));
      {
        BOOL dummy;

        dummy = WinQueryButtonCheckstate(hwnd,CFGM_SHOWTARGET);
        if(dummy != fShowTarget) {
          fShowTarget = dummy;
          PrfWriteProfileData(fmprof,
                              appname,
                              "ShowTarget",
                              (PVOID)&fShowTarget,
                              sizeof(BOOL));
          if(hwndMain)
            PostMsg(WinQueryWindow(hwndMain,QW_PARENT),
                    WM_UPDATEFRAME,
                    MPFROMLONG(FCF_SIZEBORDER),
                    MPVOID);
          SetTargetDir(hwnd,TRUE);
        }
        dummy = WinQueryButtonCheckstate(hwnd,CFGM_SEPARATEPARMS);
        if(dummy != fSeparateParms) {
          fSeparateParms = dummy;
          PrfWriteProfileData(fmprof,
                              FM3Str,
                              "SeparateParms",
                              (PVOID)&fSeparateParms,
                              sizeof(BOOL));
          WinSendMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),
                     UM_UNDO,
                     MPVOID,
                     MPVOID);
        }
      }
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY Cfg5DlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  static MASK mask;

  switch(msg) {
    case WM_INITDLG:
      WinSendDlgItemMsg(hwnd,
                        CFG5_FILTER,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),
                        MPVOID);
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_FOCUSME:
      WinSetFocus(HWND_DESKTOP,
                  WinWindowFromID(hwnd,CFG5_MINIICONS));
      return 0;

    case UM_UNDO:
      {
        ULONG flWindowAttr = 0,size = sizeof(ULONG);

        if(!PrfQueryProfileData(fmprof,
                                appname,
                                "DirflWindowAttr",
                                &flWindowAttr,
                                &size))
          flWindowAttr = (CV_NAME | CV_MINI | CA_DETAILSVIEWTITLES |
                          CV_FLOW);
        if(flWindowAttr & CV_ICON)
          WinCheckButton(hwnd,CFG5_ICON,TRUE);
        if(flWindowAttr & CV_NAME)
          WinCheckButton(hwnd,CFG5_NAME,TRUE);
        if(flWindowAttr & CV_TEXT)
          WinCheckButton(hwnd,CFG5_TEXT,TRUE);
        if(flWindowAttr & CV_DETAIL)
          WinCheckButton(hwnd,CFG5_DETAIL,TRUE);
        if(flWindowAttr & CV_MINI)
          WinCheckButton(hwnd,CFG5_MINIICONS,TRUE);
        if(flWindowAttr & CA_DETAILSVIEWTITLES)
          WinCheckButton(hwnd,CFG5_SHOWTITLES,TRUE);
        WinCheckButton(hwnd,CFG5_SHOWLNAMES,detailslongname);
        WinCheckButton(hwnd,CFG5_SHOWSUBJECT,detailssubject);
        WinCheckButton(hwnd,CFG5_SHOWEAS,detailsea);
        WinCheckButton(hwnd,CFG5_SHOWSIZE,detailssize);
        WinCheckButton(hwnd,CFG5_SHOWICON,detailsicon);
        WinCheckButton(hwnd,CFG5_SHOWLWDATE,detailslwdate);
        WinCheckButton(hwnd,CFG5_SHOWLWTIME,detailslwtime);
        WinCheckButton(hwnd,CFG5_SHOWLADATE,detailsladate);
        WinCheckButton(hwnd,CFG5_SHOWLATIME,detailslatime);
        WinCheckButton(hwnd,CFG5_SHOWCRDATE,detailscrdate);
        WinCheckButton(hwnd,CFG5_SHOWCRTIME,detailscrtime);
        WinCheckButton(hwnd,CFG5_SHOWATTR,detailsattr);
        memset(&mask,0,sizeof(mask));
        mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
                        FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
        size = sizeof(MASK);
        if(PrfQueryProfileData(fmprof,
                               appname,
                               "DirFilter",
                               &mask,
                               &size))
          SetMask(NULL,&mask);
        if(!mask.attrFile)
          mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
                          FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
        strcpy(mask.prompt,
               GetPString(IDS_DEFDIRFILTERTITLETEXT));
        WinSetDlgItemText(hwnd,
                          CFG5_FILTER,
                          mask.szMask);
      }
      return 0;

    case UM_SETUP5:
      if(WinDlgBox(HWND_DESKTOP,hwndNotebook,PickMaskDlgProc,
                   FM3ModHandle,MSK_FRAME,MPFROMP(&mask))) {
        SetMask(NULL,&mask);
        WinSetDlgItemText(hwnd,CFG5_FILTER,mask.szMask);
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFG5_FILTER:
          switch(SHORT2FROMMP(mp1)) {
            case EN_SETFOCUS:
              PostMsg(hwnd,
                      UM_FOCUSME,
                      MPVOID,
                      MPVOID);
              PostMsg(hwnd,
                      UM_SETUP5,
                      MPVOID,
                      MPVOID);
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFG5,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      {
        ULONG flWindowAttr = 0;

        if(WinQueryButtonCheckstate(hwnd,CFG5_ICON))
          flWindowAttr |= CV_ICON;
        if(WinQueryButtonCheckstate(hwnd,CFG5_NAME))
          flWindowAttr |= CV_NAME;
        if(WinQueryButtonCheckstate(hwnd,CFG5_TEXT))
          flWindowAttr |= CV_TEXT;
        if(WinQueryButtonCheckstate(hwnd,CFG5_DETAIL))
          flWindowAttr |= CV_DETAIL;
        if(WinQueryButtonCheckstate(hwnd,CFG5_MINIICONS))
          flWindowAttr |= CV_MINI;
        if(WinQueryButtonCheckstate(hwnd,CFG5_SHOWTITLES))
          flWindowAttr |= CA_DETAILSVIEWTITLES;
        flWindowAttr |= CV_FLOW;
        PrfWriteProfileData(fmprof,
                            appname,
                            "DirflWindowAttr",
                            &flWindowAttr,
                            sizeof(ULONG));
      }
      detailslongname = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLNAMES);
      PrfWriteProfileData(fmprof,appname,"DetailsLongname",
                          &detailslongname,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLongname",
                          &detailslongname,sizeof(BOOL));
      detailssubject = WinQueryButtonCheckstate(hwnd,CFG5_SHOWSUBJECT);
      PrfWriteProfileData(fmprof,appname,"DetailsSubject",
                          &detailssubject,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsSubject",
                          &detailssubject,sizeof(BOOL));
      detailsea = WinQueryButtonCheckstate(hwnd,CFG5_SHOWEAS);
      PrfWriteProfileData(fmprof,appname,"DetailsEA",
                          &detailsea,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsEA",
                          &detailsea,sizeof(BOOL));
      detailssize = WinQueryButtonCheckstate(hwnd,CFG5_SHOWSIZE);
      PrfWriteProfileData(fmprof,appname,"DetailsSize",
                          &detailssize,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsSize",
                          &detailssize,sizeof(BOOL));
      detailsicon = WinQueryButtonCheckstate(hwnd,CFG5_SHOWICON);
      PrfWriteProfileData(fmprof,appname,"DetailsIcon",
                          &detailsicon,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsIcon",
                          &detailsicon,sizeof(BOOL));
      detailslwdate = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLWDATE);
      PrfWriteProfileData(fmprof,appname,"DetailsLWDate",
                          &detailslwdate,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLWDate",
                          &detailslwdate,sizeof(BOOL));
      detailslwtime = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLWTIME);
      PrfWriteProfileData(fmprof,appname,"DetailsLWTime",
                          &detailslwtime,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLWTime",
                          &detailslwtime,sizeof(BOOL));
      detailsladate = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLADATE);
      PrfWriteProfileData(fmprof,appname,"DetailsLADate",
                          &detailsladate,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLADate",
                          &detailsladate,sizeof(BOOL));
      detailslatime = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLATIME);
      PrfWriteProfileData(fmprof,appname,"DetailsLATime",
                          &detailslatime,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLATime",
                          &detailslatime,sizeof(BOOL));
      detailscrdate = WinQueryButtonCheckstate(hwnd,CFG5_SHOWCRDATE);
      PrfWriteProfileData(fmprof,appname,"DetailsCRDate",
                          &detailscrdate,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsCRDate",
                          &detailscrdate,sizeof(BOOL));
      detailscrtime = WinQueryButtonCheckstate(hwnd,CFG5_SHOWCRTIME);
      PrfWriteProfileData(fmprof,appname,"DetailsCRTime",
                          &detailscrtime,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsCRTime",
                          &detailscrtime,sizeof(BOOL));
      detailsattr = WinQueryButtonCheckstate(hwnd,CFG5_SHOWATTR);
      PrfWriteProfileData(fmprof,appname,"DetailsAttr",
                          &detailsattr,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsAttr",
                          &detailsattr,sizeof(BOOL));
      *mask.prompt = 0;
      PrfWriteProfileData(fmprof,
                          appname,
                          "DirFilter",
                          &mask,
                          sizeof(MASK));
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY Cfg6DlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinCheckButton(hwnd,CFG6_SORTFIRST,FALSE);
      WinCheckButton(hwnd,CFG6_SORTLAST,FALSE);
      WinCheckButton(hwnd,CFG6_SORTSIZE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTEASIZE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTLWDATE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTLADATE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTCRDATE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTNAME,FALSE);
      WinCheckButton(hwnd,CFG6_SORTDIRSFIRST,FALSE);
      WinCheckButton(hwnd,CFG6_SORTDIRSLAST,FALSE);
      WinCheckButton(hwnd,CFG6_SORTREVERSE,FALSE);
      if(sortFlags & SORT_FIRSTEXTENSION)
        WinCheckButton(hwnd,CFG6_SORTFIRST,TRUE);
      else if(sortFlags & SORT_LASTEXTENSION)
        WinCheckButton(hwnd,CFG6_SORTLAST,TRUE);
      else if(sortFlags & SORT_SIZE)
        WinCheckButton(hwnd,CFG6_SORTSIZE,TRUE);
      else if(sortFlags & SORT_EASIZE)
        WinCheckButton(hwnd,CFG6_SORTEASIZE,TRUE);
      else if(sortFlags & SORT_LWDATE)
        WinCheckButton(hwnd,CFG6_SORTLWDATE,TRUE);
      else if(sortFlags & SORT_LADATE)
        WinCheckButton(hwnd,CFG6_SORTLADATE,TRUE);
      else if(sortFlags & SORT_CRDATE)
        WinCheckButton(hwnd,CFG6_SORTCRDATE,TRUE);
      else if(sortFlags & SORT_FILENAME)
        WinCheckButton(hwnd,CFG6_SORTFILENAME,TRUE);
      else
        WinCheckButton(hwnd,CFG6_SORTNAME,TRUE);
      if(sortFlags & SORT_DIRSFIRST)
        WinCheckButton(hwnd,CFG6_SORTDIRSFIRST,TRUE);
      else if(sortFlags & SORT_DIRSLAST)
        WinCheckButton(hwnd,CFG6_SORTDIRSLAST,TRUE);
      if(sortFlags & SORT_REVERSE)
        WinCheckButton(hwnd,CFG6_SORTREVERSE,TRUE);
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFG6_SORTDIRSFIRST:
        case CFG6_SORTDIRSLAST:
          {
            BOOL temp;

            temp = WinQueryButtonCheckstate(hwnd,SHORT1FROMMP(mp1));
            if(temp) {
              switch(SHORT1FROMMP(mp1)) {
                case CFG6_SORTDIRSFIRST:
                  WinCheckButton(hwnd,CFG6_SORTDIRSLAST,FALSE);
                  break;
                case CFG6_SORTDIRSLAST:
                  WinCheckButton(hwnd,CFG6_SORTDIRSFIRST,FALSE);
                  break;
              }
            }
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFG6,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      sortFlags = 0;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTFILENAME))
        sortFlags |= SORT_FILENAME;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTSIZE))
        sortFlags |= SORT_SIZE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTEASIZE))
        sortFlags |= SORT_EASIZE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTFIRST))
        sortFlags |= SORT_FIRSTEXTENSION;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTLAST))
        sortFlags |= SORT_LASTEXTENSION;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTLWDATE))
        sortFlags |= SORT_LWDATE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTLADATE))
        sortFlags |= SORT_LADATE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTCRDATE))
        sortFlags |= SORT_CRDATE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTDIRSFIRST))
        sortFlags |= SORT_DIRSFIRST;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTDIRSLAST))
        sortFlags |= SORT_DIRSLAST;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTREVERSE))
        sortFlags |= SORT_REVERSE;
      PrfWriteProfileData(fmprof,
                          appname,
                          "Sort",
                          &sortFlags,
                          sizeof(INT));
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY Cfg7DlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  static MASK mask;

  switch(msg) {
    case WM_INITDLG:
      WinSendDlgItemMsg(hwnd,CFG5_FILTER,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_FOCUSME:
      WinSetFocus(HWND_DESKTOP,
                  WinWindowFromID(hwnd,CFG5_MINIICONS));
      return 0;

    case UM_UNDO:
      WinCheckButton(hwnd,CFG5_EXTERNALCOLLECTOR,fExternalCollector);
      {
        ULONG flWindowAttr = 0,size = sizeof(ULONG);

        if(!PrfQueryProfileData(fmprof,
                                appname,
                                "CollectorflWindowAttr",
                                &flWindowAttr,
                                &size))
          flWindowAttr = (CV_NAME | CA_DETAILSVIEWTITLES | CV_MINI | CV_FLOW);
        if(flWindowAttr & CV_ICON)
          WinCheckButton(hwnd,CFG5_ICON,TRUE);
        if(flWindowAttr & CV_NAME)
          WinCheckButton(hwnd,CFG5_NAME,TRUE);
        if(flWindowAttr & CV_TEXT)
          WinCheckButton(hwnd,CFG5_TEXT,TRUE);
        if(flWindowAttr & CV_DETAIL)
          WinCheckButton(hwnd,CFG5_DETAIL,TRUE);
        if(flWindowAttr & CV_MINI)
          WinCheckButton(hwnd,CFG5_MINIICONS,TRUE);
        if(flWindowAttr & CA_DETAILSVIEWTITLES)
          WinCheckButton(hwnd,CFG5_SHOWTITLES,TRUE);
        memset(&mask,0,sizeof(mask));
        mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
                        FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
        size = sizeof(MASK);
        if(PrfQueryProfileData(fmprof,
                               appname,
                               "CollectorFilter",
                               &mask,
                               &size)) {
          SetMask(NULL,&mask);
        }
        if(!mask.attrFile)
          mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
                          FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
        strcpy(mask.prompt,
               GetPString(IDS_DEFCOLFILTERTITLETEXT));
        WinSetDlgItemText(hwnd,CFG5_FILTER,mask.szMask);
      }
      {
        DIRCNRDATA dcd;

        memset(&dcd,0,sizeof(dcd));
        LoadDetailsSwitches("Collector",
                            &dcd);
        WinCheckButton(hwnd,CFG5_SHOWLNAMES,dcd.detailslongname);
        WinCheckButton(hwnd,CFG5_SHOWSUBJECT,dcd.detailssubject);
        WinCheckButton(hwnd,CFG5_SHOWEAS,dcd.detailsea);
        WinCheckButton(hwnd,CFG5_SHOWSIZE,dcd.detailssize);
        WinCheckButton(hwnd,CFG5_SHOWICON,dcd.detailsicon);
        WinCheckButton(hwnd,CFG5_SHOWLWDATE,dcd.detailslwdate);
        WinCheckButton(hwnd,CFG5_SHOWLWTIME,dcd.detailslwtime);
        WinCheckButton(hwnd,CFG5_SHOWLADATE,dcd.detailsladate);
        WinCheckButton(hwnd,CFG5_SHOWLATIME,dcd.detailslatime);
        WinCheckButton(hwnd,CFG5_SHOWCRDATE,dcd.detailscrdate);
        WinCheckButton(hwnd,CFG5_SHOWCRTIME,dcd.detailscrtime);
        WinCheckButton(hwnd,CFG5_SHOWATTR,dcd.detailsattr);
      }
      return 0;

    case UM_SETUP5:
      if(WinDlgBox(HWND_DESKTOP,hwndNotebook,PickMaskDlgProc,
                   FM3ModHandle,MSK_FRAME,MPFROMP(&mask))) {
        SetMask(NULL,&mask);
        WinSetDlgItemText(hwnd,CFG5_FILTER,mask.szMask);
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFG5_FILTER:
          switch(SHORT2FROMMP(mp1)) {
            case EN_SETFOCUS:
              PostMsg(hwnd,
                      UM_FOCUSME,
                      MPVOID,
                      MPVOID);
              PostMsg(hwnd,
                      UM_SETUP5,
                      MPVOID,
                      MPVOID);
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFG7,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      fExternalCollector = WinQueryButtonCheckstate(hwnd,
                                                    CFG5_EXTERNALCOLLECTOR);
      PrfWriteProfileData(fmprof,FM3Str,"ExternalCollector",
                          &fExternalCollector,sizeof(BOOL));
      {
        ULONG flWindowAttr = 0;

        if(WinQueryButtonCheckstate(hwnd,CFG5_ICON))
          flWindowAttr |= CV_ICON;
        if(WinQueryButtonCheckstate(hwnd,CFG5_NAME))
          flWindowAttr |= CV_NAME;
        if(WinQueryButtonCheckstate(hwnd,CFG5_TEXT))
          flWindowAttr |= CV_TEXT;
        if(WinQueryButtonCheckstate(hwnd,CFG5_DETAIL))
          flWindowAttr |= CV_DETAIL;
        if(WinQueryButtonCheckstate(hwnd,CFG5_MINIICONS))
          flWindowAttr |= CV_MINI;
        if(WinQueryButtonCheckstate(hwnd,CFG5_SHOWTITLES))
          flWindowAttr |= CA_DETAILSVIEWTITLES;
        flWindowAttr |= CV_FLOW;
        PrfWriteProfileData(fmprof,appname,"CollectorflWindowAttr",
                            &flWindowAttr,sizeof(ULONG));
      }
      {
        DIRCNRDATA dcd;

        memset(&dcd,0,sizeof(dcd));
        dcd.detailslongname = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLNAMES);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsLongname",
                            &dcd.detailslongname,sizeof(BOOL));
        dcd.detailssubject = WinQueryButtonCheckstate(hwnd,CFG5_SHOWSUBJECT);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsSubject",
                            &dcd.detailssubject,sizeof(BOOL));
        dcd.detailsea = WinQueryButtonCheckstate(hwnd,CFG5_SHOWEAS);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsEA",
                            &dcd.detailsea,sizeof(BOOL));
        dcd.detailssize = WinQueryButtonCheckstate(hwnd,CFG5_SHOWSIZE);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsSize",
                            &dcd.detailssize,sizeof(BOOL));
        dcd.detailsicon = WinQueryButtonCheckstate(hwnd,CFG5_SHOWICON);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsIcon",
                            &dcd.detailsicon,sizeof(BOOL));
        dcd.detailslwdate = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLWDATE);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsLWDate",
                            &dcd.detailslwdate,sizeof(BOOL));
        dcd.detailslwtime = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLWTIME);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsLWTime",
                            &dcd.detailslwtime,sizeof(BOOL));
        dcd.detailsladate = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLADATE);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsLADate",
                            &dcd.detailsladate,sizeof(BOOL));
        dcd.detailslatime = WinQueryButtonCheckstate(hwnd,CFG5_SHOWLATIME);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsLATime",
                            &dcd.detailslatime,sizeof(BOOL));
        dcd.detailscrdate = WinQueryButtonCheckstate(hwnd,CFG5_SHOWCRDATE);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsCRDate",
                            &dcd.detailscrdate,sizeof(BOOL));
        dcd.detailscrtime = WinQueryButtonCheckstate(hwnd,CFG5_SHOWCRTIME);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsCRTime",
                            &dcd.detailscrtime,sizeof(BOOL));
        dcd.detailsattr = WinQueryButtonCheckstate(hwnd,CFG5_SHOWATTR);
        PrfWriteProfileData(fmprof,appname,"Collector.DetailsAttr",
                            &dcd.detailsattr,sizeof(BOOL));
        *mask.prompt = 0;
        PrfWriteProfileData(fmprof,
                            appname,
                            "CollectorFilter",
                            &mask,
                            sizeof(MASK));
      }
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY Cfg8DlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
      break;

    case UM_UNDO:
      WinCheckButton(hwnd,CFG6_SORTFIRST,FALSE);
      WinCheckButton(hwnd,CFG6_SORTLAST,FALSE);
      WinCheckButton(hwnd,CFG6_SORTSIZE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTEASIZE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTLWDATE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTLADATE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTCRDATE,FALSE);
      WinCheckButton(hwnd,CFG6_SORTNAME,FALSE);
      WinCheckButton(hwnd,CFG6_SORTDIRSFIRST,FALSE);
      WinCheckButton(hwnd,CFG6_SORTDIRSLAST,FALSE);
      WinCheckButton(hwnd,CFG6_SORTREVERSE,FALSE);
      if(CollectorsortFlags & SORT_FIRSTEXTENSION)
        WinCheckButton(hwnd,CFG6_SORTFIRST,TRUE);
      else if(CollectorsortFlags & SORT_LASTEXTENSION)
        WinCheckButton(hwnd,CFG6_SORTLAST,TRUE);
      else if(CollectorsortFlags & SORT_SIZE)
        WinCheckButton(hwnd,CFG6_SORTSIZE,TRUE);
      else if(CollectorsortFlags & SORT_EASIZE)
        WinCheckButton(hwnd,CFG6_SORTEASIZE,TRUE);
      else if(CollectorsortFlags & SORT_LWDATE)
        WinCheckButton(hwnd,CFG6_SORTLWDATE,TRUE);
      else if(CollectorsortFlags & SORT_LADATE)
        WinCheckButton(hwnd,CFG6_SORTLADATE,TRUE);
      else if(CollectorsortFlags & SORT_CRDATE)
        WinCheckButton(hwnd,CFG6_SORTCRDATE,TRUE);
      else if(CollectorsortFlags & SORT_FILENAME)
        WinCheckButton(hwnd,CFG6_SORTFILENAME,TRUE);
      else
        WinCheckButton(hwnd,CFG6_SORTNAME,TRUE);
      if(CollectorsortFlags & SORT_DIRSFIRST)
        WinCheckButton(hwnd,CFG6_SORTDIRSFIRST,TRUE);
      else if(CollectorsortFlags & SORT_DIRSLAST)
        WinCheckButton(hwnd,CFG6_SORTDIRSLAST,TRUE);
      if(CollectorsortFlags & SORT_REVERSE)
        WinCheckButton(hwnd,CFG6_SORTREVERSE,TRUE);
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFG6_SORTDIRSFIRST:
        case CFG6_SORTDIRSLAST:
          {
            BOOL temp;

            temp = WinQueryButtonCheckstate(hwnd,SHORT1FROMMP(mp1));
            if(temp) {
              switch(SHORT1FROMMP(mp1)) {
                case CFG6_SORTDIRSFIRST:
                  WinCheckButton(hwnd,CFG6_SORTDIRSLAST,FALSE);
                  break;
                case CFG6_SORTDIRSLAST:
                  WinCheckButton(hwnd,CFG6_SORTDIRSFIRST,FALSE);
                  break;
              }
            }
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_UNDO:
          PostMsg(hwnd,UM_UNDO,MPVOID,MPVOID);
          break;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFG8,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_CLOSE:
      CollectorsortFlags = 0;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTFILENAME))
        CollectorsortFlags |= SORT_FILENAME;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTSIZE))
        CollectorsortFlags |= SORT_SIZE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTEASIZE))
        CollectorsortFlags |= SORT_EASIZE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTFIRST))
        CollectorsortFlags |= SORT_FIRSTEXTENSION;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTLAST))
        CollectorsortFlags |= SORT_LASTEXTENSION;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTLWDATE))
        CollectorsortFlags |= SORT_LWDATE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTLADATE))
        CollectorsortFlags |= SORT_LADATE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTCRDATE))
        CollectorsortFlags |= SORT_CRDATE;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTDIRSFIRST))
        CollectorsortFlags |= SORT_DIRSFIRST;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTDIRSLAST))
        CollectorsortFlags |= SORT_DIRSLAST;
      if(WinQueryButtonCheckstate(hwnd,CFG6_SORTREVERSE))
        CollectorsortFlags |= SORT_REVERSE;
      PrfWriteProfileData(fmprof,
                          appname,
                          "CollectorSort",
                          &CollectorsortFlags,
                          sizeof(INT));
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY Cfg9DlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch(msg) {
    case WM_INITDLG:
      break;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CFG9,0),
                       MPFROMSHORT(HM_RESOURCEID));
          return 0;
        case CFG9_MAXIMUMUI:
          if(hwndMain) {
            if(MenuInvisible)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_HIDEMENU,0),MPVOID);
            if(!fAutoView)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_AUTOVIEW,0),MPVOID);
            if(!fDrivebar)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_DRIVEBAR,0),MPVOID);
            if(!fToolbar)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_TOOLBAR,0),MPVOID);
            if(!fMoreButtons)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_MOREBUTTONS,0),MPVOID);
            if(!fUserComboBox)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_USERLIST,0),MPVOID);
          }
          return 0;
        case CFG9_MINIMUMUI:
          if(hwndMain) {
            if(!MenuInvisible)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_HIDEMENU,0),MPVOID);
            if(fAutoView)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_AUTOVIEW,0),MPVOID);
            if(fToolbar)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_TOOLBAR,0),MPVOID);
            if(fMoreButtons)
              WinSendMsg(hwndMain,WM_COMMAND,MPFROM2SHORT(IDM_MOREBUTTONS,0),
                         MPVOID);
            if(fUserComboBox)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_USERLIST,0),MPVOID);
            saymsg(MB_ENTER | MB_ICONASTERISK,
                   hwnd,
                   GetPString(IDS_DONTFORGETTEXT),
                   GetPString(IDS_UNHIDEMENUWARNTEXT));
          }
          return 0;
        case CFG9_MAXINFOPRETTY:
          fLoadSubject        = TRUE;
          fLoadLongnames      = TRUE;
          fNoIconsFiles       = FALSE;
          fNoIconsDirs        = FALSE;
          fForceUpper         = FALSE;
          fForceLower         = FALSE;
          fArcStuffVisible    = TRUE;
          fSplitStatus        = TRUE;
          fDragndropDlg       = TRUE;
          {
            ULONG flWindowAttr;

            flWindowAttr      = CV_DETAIL | CV_FLOW | CA_DETAILSVIEWTITLES;
            PrfWriteProfileData(fmprof,
                                appname,
                                "DirflWindowAttr",
                                &flWindowAttr,
                                sizeof(ULONG));
            PrfWriteProfileData(fmprof,
                                appname,
                                "CollectorflWindowAttr",
                                &flWindowAttr,
                                sizeof(ULONG));
            PrfWriteProfileData(fmprof,
                                appname,
                                "DirCnr.Fontnamesize",
                                NULL,
                                0);
            PrfWriteProfileData(fmprof,
                                appname,
                                "Collector.Fontnamesize",
                                NULL,
                                0);
          }
          detailslongname     = TRUE;
          detailssubject      = TRUE;
          detailsea           = TRUE;
          detailssize         = TRUE;
          detailsicon         = TRUE;
          detailslwdate       = TRUE;
          detailslwtime       = TRUE;
          detailsladate       = TRUE;
          detailslatime       = TRUE;
          detailscrdate       = TRUE;
          detailscrtime       = TRUE;
          detailsattr         = TRUE;
          if(hwndTree) {

            CNRINFO cnri;
            ULONG   flWindowAttr = CV_TREE | CV_ICON |
                                   CV_FLOW | CA_TREELINE;

            memset(&cnri,0,sizeof(cnri));
            cnri.cb = sizeof(cnri);
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_QUERYCNRINFO,MPFROMP(&cnri),MPFROMLONG(sizeof(cnri)));
            cnri.flWindowAttr = flWindowAttr;
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_SETCNRINFO,MPFROMP(&cnri),MPFROMLONG(CMA_FLWINDOWATTR));
          }
          break;

        case CFG9_MAXINFOPLAIN:
          fLoadSubject        = TRUE;
          fLoadLongnames      = TRUE;
          fNoIconsFiles       = TRUE;
          fNoIconsDirs        = TRUE;
          fForceUpper         = FALSE;
          fForceLower         = FALSE;
          fArcStuffVisible    = TRUE;
          fSplitStatus        = TRUE;
          fDragndropDlg       = TRUE;
          {
            ULONG flWindowAttr;

            flWindowAttr      = CV_DETAIL | CV_FLOW | CV_MINI;
            PrfWriteProfileData(fmprof,
                                appname,
                                "DirflWindowAttr",
                                &flWindowAttr,
                                sizeof(ULONG));
            PrfWriteProfileData(fmprof,
                                appname,
                                "CollectorflWindowAttr",
                                &flWindowAttr,
                                sizeof(ULONG));
            PrfWriteProfileData(fmprof,
                                appname,
                                "DirCnr.Fontnamesize",
                                GetPString(IDS_8HELVTEXT),
                                strlen(GetPString(IDS_8HELVTEXT)) + 1);
            PrfWriteProfileData(fmprof,
                                appname,
                                "Collector.Fontnamesize",
                                GetPString(IDS_8HELVTEXT),
                                strlen(GetPString(IDS_8HELVTEXT)) + 1);
          }
          detailslongname     = TRUE;
          detailssubject      = TRUE;
          detailsea           = TRUE;
          detailssize         = TRUE;
          detailsicon         = TRUE;
          detailslwdate       = TRUE;
          detailslwtime       = TRUE;
          detailsladate       = TRUE;
          detailslatime       = TRUE;
          detailscrdate       = TRUE;
          detailscrtime       = TRUE;
          detailsattr         = TRUE;
          if(hwndTree) {

            CNRINFO cnri;
            ULONG   flWindowAttr = CV_TREE | CV_MINI | CV_TEXT |
                                   CV_FLOW | CA_TREELINE;

            memset(&cnri,0,sizeof(cnri));
            cnri.cb = sizeof(cnri);
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_QUERYCNRINFO,MPFROMP(&cnri),MPFROMLONG(sizeof(cnri)));
            cnri.flWindowAttr = flWindowAttr;
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_SETCNRINFO,MPFROMP(&cnri),MPFROMLONG(CMA_FLWINDOWATTR));
          }
          break;
        case CFG9_MAXFILENAMES:
          if(hwndMain && fAutoView)
            WinSendMsg(hwndMain,WM_COMMAND,
                       MPFROM2SHORT(IDM_AUTOVIEW,0),MPVOID);
          fForceUpper         = FALSE;
          fForceLower         = TRUE;
          fExternalViewer     = TRUE;
          fExternalArcboxes   = TRUE;
          fExternalCollector  = TRUE;
          fExternalINIs       = TRUE;
          fLoadSubject        = FALSE;
          fLoadLongnames      = FALSE;
          fNoIconsFiles       = TRUE;
          fNoIconsDirs        = TRUE;
          {
            ULONG flWindowAttr;

            flWindowAttr      = CV_TEXT | CV_FLOW;
            PrfWriteProfileData(fmprof,appname,"DirflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
            PrfWriteProfileData(fmprof,appname,"CollectorflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
            PrfWriteProfileData(fmprof,appname,"DirCnr.Fontnamesize",
                                GetPString(IDS_8HELVTEXT),
                                strlen(GetPString(IDS_8HELVTEXT)) + 1);
            PrfWriteProfileData(fmprof,appname,"Collector.Fontnamesize",
                                GetPString(IDS_8HELVTEXT),
                                strlen(GetPString(IDS_8HELVTEXT)) + 1);
          }
          if(hwndTree) {

            CNRINFO cnri;
            ULONG   flWindowAttr = CV_TREE | CV_TEXT |
                                   CV_FLOW | CA_TREELINE;

            memset(&cnri,0,sizeof(cnri));
            cnri.cb = sizeof(cnri);
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_QUERYCNRINFO,MPFROMP(&cnri),MPFROMLONG(sizeof(cnri)));
            cnri.flWindowAttr = flWindowAttr;
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_SETCNRINFO,MPFROMP(&cnri),MPFROMLONG(CMA_FLWINDOWATTR));
          }
          break;
        case CFG9_MAXSPEED:
          fLoadSubject        = FALSE;
          fLoadLongnames      = FALSE;
          fVerify             = FALSE;
          DosSetVerify(FALSE);
          FilesToGet          = 512;
          fQuickArcFind       = TRUE;
          fMinOnOpen          = TRUE;
          fRealIdle           = FALSE;
          fNoIconsFiles       = TRUE;
          fNoIconsDirs        = TRUE;
          fSyncUpdates        = FALSE;
          fArcStuffVisible    = FALSE;
          fForceUpper         = FALSE;
          fForceLower         = FALSE;
          detailslongname     = FALSE;
          detailssubject      = FALSE;
          break;

        case CFG9_HECTOR:
          fSwitchTree = TRUE;
          fSwitchTreeOnFocus = FALSE;
          fSwitchTreeExpand = TRUE;
          fCollapseFirst = TRUE;
          fSelectedAlways     = FALSE;
          fTileBackwards      = FALSE;
          fExternalViewer     = FALSE;
          fExternalArcboxes   = TRUE;
          fExternalCollector  = FALSE;
          fExternalINIs       = TRUE;
          fCopyDefault        = FALSE;
          fFollowTree         = FALSE;
          fLoadSubject        = FALSE;
          fLoadLongnames      = FALSE;
          fDontMoveMouse      = FALSE;
          fUnHilite           = TRUE;
          fUserListSwitches   = TRUE;
          fDCOpens            = FALSE;
          fLinkSetsIcon       = FALSE;
          fConfirmDelete      = TRUE;
          fSyncUpdates        = FALSE;
          fRealIdle           = FALSE;
          fNoIconsFiles       = FALSE;
          fNoIconsDirs        = TRUE;
          fFolderAfterExtract = FALSE;
          fVerify             = TRUE;
          DosSetVerify(TRUE);
          fForceUpper         = FALSE;
          fForceLower         = TRUE;
          fArcStuffVisible    = FALSE;
          fVTreeOpensWPS      = FALSE;
          fRemoteBug          = FALSE;
          fDragndropDlg       = TRUE;
          fMinOnOpen          = FALSE;
          fQuickArcFind       = TRUE;
          fNoRemovableScan    = TRUE;
          FilesToGet          = 128;
          fFreeTree           = FALSE;
          fSplitStatus        = TRUE;
          fAutoTile           = TRUE;
          fSaveState          = TRUE;
          fStartMinimized     = FALSE;
          fStartMaximized     = FALSE;
          fDataMin            = FALSE;
          ulCnrType           = CCS_EXTENDSEL | CCS_MULTIPLESEL;
          fNoTreeGap          = TRUE;
          {
            ULONG flWindowAttr;

            flWindowAttr      = (CV_NAME | CV_MINI | CV_FLOW |
                                 CA_DETAILSVIEWTITLES);
            PrfWriteProfileData(fmprof,appname,"DirflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
            PrfWriteProfileData(fmprof,appname,"CollectorflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
          }
          detailslongname     = FALSE;
          detailssubject      = FALSE;
          detailsea           = TRUE;
          detailssize         = TRUE;
          detailsicon         = TRUE;
          detailslwdate       = TRUE;
          detailslwtime       = TRUE;
          detailsladate       = FALSE;
          detailslatime       = FALSE;
          detailscrdate       = FALSE;
          detailscrtime       = FALSE;
          detailsattr         = TRUE;
          sortFlags           = SORT_FILENAME | SORT_DIRSFIRST;
          CollectorsortFlags  = SORT_FILENAME | SORT_DIRSFIRST;
          if(hwndMain) {

            SWP swp;

            if(WinQueryWindowPos(hwndMain,&swp)) {
              WinSetWindowPos(hwndTree,HWND_TOP,0,0,
                              swp.cx / 5,swp.cy,
                              SWP_MOVE | SWP_SIZE);
            }
          }
          if(hwndMain) {
            if(MenuInvisible)
              WinSendMsg(hwndMain,WM_COMMAND,
                      MPFROM2SHORT(IDM_HIDEMENU,0),MPVOID);
            if(fAutoView)
              WinSendMsg(hwndMain,WM_COMMAND,
                      MPFROM2SHORT(IDM_AUTOVIEW,0),MPVOID);
            if(fToolbar)
              WinSendMsg(hwndMain,WM_COMMAND,
                      MPFROM2SHORT(IDM_TOOLBAR,0),MPVOID);
            if(!fDrivebar)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_DRIVEBAR,0),MPVOID);
            if(!fMoreButtons)
              WinSendMsg(hwndMain,WM_COMMAND,MPFROM2SHORT(IDM_MOREBUTTONS,0),
                      MPVOID);
            if(!fUserComboBox)
              WinSendMsg(hwndMain,WM_COMMAND,
                      MPFROM2SHORT(IDM_USERLIST,0),MPVOID);
          }
          if(hwndTree) {

            CNRINFO cnri;
            ULONG   flWindowAttr = (CV_TREE | CV_TEXT | CV_MINI |
                                    CV_FLOW | CA_TREELINE);

            memset(&cnri,0,sizeof(cnri));
            cnri.cb = sizeof(cnri);
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_QUERYCNRINFO,MPFROMP(&cnri),MPFROMLONG(sizeof(cnri)));
            cnri.flWindowAttr = flWindowAttr;
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_SETCNRINFO,MPFROMP(&cnri),MPFROMLONG(CMA_FLWINDOWATTR));
          }
          break;

        case CFG9_DEFAULT:
          fSwitchTree         = FALSE;
          fSwitchTreeOnFocus  = FALSE;
          fSwitchTreeExpand   = FALSE;
          fCollapseFirst      = FALSE;
          fSelectedAlways     = FALSE;
          fTileBackwards      = FALSE;
          fExternalViewer     = FALSE;
          fExternalArcboxes   = FALSE;
          fExternalCollector  = FALSE;
          fExternalINIs       = FALSE;
          fCopyDefault        = FALSE;
          fFollowTree         = FALSE;
          fLoadSubject        = TRUE;
          fLoadLongnames      = TRUE;
          fDontMoveMouse      = FALSE;
          fUnHilite           = TRUE;
          fUserListSwitches   = FALSE;
          fDCOpens            = FALSE;
          fLinkSetsIcon       = FALSE;
          fConfirmDelete      = TRUE;
          fSyncUpdates        = FALSE;
          fRealIdle           = FALSE;
          fNoIconsFiles       = FALSE;
          fNoIconsDirs        = FALSE;
          fFolderAfterExtract = FALSE;
          fVerify             = TRUE;
          fNoSearch           = TRUE;
          DosSetVerify(TRUE);
          fForceUpper         = FALSE;
          fForceLower         = TRUE;
          fArcStuffVisible    = TRUE;
          fVTreeOpensWPS      = FALSE;
          fRemoteBug          = TRUE;
          fDragndropDlg       = TRUE;
          fMinOnOpen          = FALSE;
          fQuickArcFind       = TRUE;
          fNoRemovableScan    = FALSE;
          FilesToGet          = 128;
          fFreeTree           = FALSE;
          fSplitStatus        = TRUE;
          fAutoTile           = TRUE;
          fSaveState          = TRUE;
          fStartMinimized     = FALSE;
          fStartMaximized     = FALSE;
          fDataMin            = TRUE;
          ulCnrType           = CCS_EXTENDSEL;
          fNoTreeGap          = FALSE;
          {
            ULONG flWindowAttr;

            flWindowAttr      = (CV_NAME | CV_MINI | CV_FLOW |
                                 CA_DETAILSVIEWTITLES);
            PrfWriteProfileData(fmprof,appname,"DirflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
            PrfWriteProfileData(fmprof,appname,"CollectorflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
          }
          detailslongname     = FALSE;
          detailssubject      = FALSE;
          detailsea           = TRUE;
          detailssize         = TRUE;
          detailsicon         = TRUE;
          detailslwdate       = TRUE;
          detailslwtime       = TRUE;
          detailsladate       = FALSE;
          detailslatime       = FALSE;
          detailscrdate       = FALSE;
          detailscrtime       = FALSE;
          detailsattr         = TRUE;
          sortFlags           = SORT_FILENAME | SORT_DIRSFIRST;
          CollectorsortFlags  = SORT_FILENAME | SORT_DIRSFIRST;
          if(hwndMain) {

            SWP swp;

            if(WinQueryWindowPos(hwndMain,&swp)) {
              WinSetWindowPos(hwndTree,HWND_TOP,0,0,
                              swp.cx / 5,
                              swp.cy -
                                (WinQuerySysValue(HWND_DESKTOP,SV_CYICON) * 2),
                              SWP_MOVE | SWP_SIZE);
            }
          }
          if(hwndTree) {

            CNRINFO cnri;
            ULONG   flWindowAttr = (CV_TREE | CV_TEXT |
                                    CV_FLOW | CA_TREELINE |
                                    CV_MINI);

            memset(&cnri,0,sizeof(cnri));
            cnri.cb = sizeof(cnri);
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_QUERYCNRINFO,MPFROMP(&cnri),MPFROMLONG(sizeof(cnri)));
            cnri.flWindowAttr = flWindowAttr;
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_SETCNRINFO,MPFROMP(&cnri),MPFROMLONG(CMA_FLWINDOWATTR));
          }
          break;

        case CFG9_WINDOZETHINK:
          fTileBackwards = FALSE;
          ulCnrType      = CCS_MULTIPLESEL;
          fFollowTree = TRUE;
          fTopDir = FALSE;
          fSwitchTree = TRUE;
          fSwitchTreeOnFocus = FALSE;
          fSwitchTreeExpand = TRUE;
          fCollapseFirst = TRUE;
          fDCOpens = FALSE;
          {
            ULONG flWindowAttr;

            flWindowAttr      = CV_NAME | CV_FLOW | CA_DETAILSVIEWTITLES;
            PrfWriteProfileData(fmprof,appname,"DirflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
            PrfWriteProfileData(fmprof,appname,"CollectorflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
          }
          fLinkSetsIcon = FALSE;
          fFreeTree = FALSE;
          fNoTreeGap = TRUE;
          fExternalArcboxes = TRUE;
          fExternalViewer = TRUE;
          fExternalCollector = TRUE;
          fExternalINIs = TRUE;
          fUserListSwitches = TRUE;
          WinSendMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),UM_UNDO,MPVOID,MPVOID);
          if(hwndTree) {

            CNRINFO cnri;
            ULONG   flWindowAttr = CV_TREE | CV_MINI | CV_ICON |
                                   CV_FLOW | CA_TREELINE;

            memset(&cnri,0,sizeof(cnri));
            cnri.cb = sizeof(cnri);
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_QUERYCNRINFO,MPFROMP(&cnri),MPFROMLONG(sizeof(cnri)));
            cnri.flWindowAttr = flWindowAttr;
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_SETCNRINFO,MPFROMP(&cnri),MPFROMLONG(CMA_FLWINDOWATTR));
          }
          if(hwndMain) {
            if(fAutoView)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_AUTOVIEW,0),MPVOID);
            if(!fDrivebar)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_DRIVEBAR,0),MPVOID);
            {
              SWP swp;

              if(WinQueryWindowPos(hwndMain,&swp)) {
                WinSetWindowPos(hwndTree,HWND_TOP,0,0,
                                swp.cx / 5,swp.cy,
                                SWP_MOVE | SWP_SIZE);
              }
            }
            PostMsg(MainObjectHwnd,UM_RESTORE,MPVOID,MPFROMLONG(2L));
            PostMsg(MainObjectHwnd,UM_SETDIR,MPFROMLONG(1L),MPVOID);
          }
          return 0;

        case CFG9_DOSTHINK:
          fTileBackwards = TRUE;
          ulCnrType      = CCS_MULTIPLESEL;
          fSwitchTree = TRUE;
          fSwitchTreeOnFocus = FALSE;
          fSwitchTreeExpand = TRUE;
          fCollapseFirst = TRUE;
          fFollowTree = TRUE;
          fTopDir = FALSE;
          if(hwndMain) {
            if(!fTextTools)
              WinSendMsg(hwndMain,WM_COMMAND,
                      MPFROM2SHORT(IDM_TEXTTOOLS,0),MPVOID);
          }
          {
            ULONG flWindowAttr;

            flWindowAttr      = CV_TEXT | CV_FLOW | CA_DETAILSVIEWTITLES;
            PrfWriteProfileData(fmprof,appname,"DirflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
            PrfWriteProfileData(fmprof,appname,"CollectorflWindowAttr",
                                &flWindowAttr,sizeof(ULONG));
          }
          if(hwndTree) {

            CNRINFO cnri;
            ULONG   flWindowAttr = CV_TREE | CV_TEXT |
                                   CV_FLOW | CA_TREELINE;

            memset(&cnri,0,sizeof(cnri));
            cnri.cb = sizeof(cnri);
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_QUERYCNRINFO,MPFROMP(&cnri),MPFROMLONG(sizeof(cnri)));
            cnri.flWindowAttr = flWindowAttr;
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),TREE_CNR),
                       CM_SETCNRINFO,MPFROMP(&cnri),MPFROMLONG(CMA_FLWINDOWATTR));
          }
          /* intentional fallthru */
        case CFG9_1X:
          if(SHORT1FROMMP(mp1) == CFG9_1X) {
            fTileBackwards = FALSE;
            ulCnrType      = CCS_MULTIPLESEL | CCS_EXTENDSEL;
            fSwitchTree = FALSE;
            fSwitchTreeOnFocus = FALSE;
            fSwitchTreeExpand = FALSE;
            fCollapseFirst = FALSE;
            fFollowTree = FALSE;
            fNoSearch = TRUE;
          }
          fAutoTile = TRUE;
          fSaveState = TRUE;
          fDCOpens = FALSE;
          fLinkSetsIcon = FALSE;
          fFreeTree = FALSE;
          fNoTreeGap = TRUE;
          fExternalArcboxes = TRUE;
          fExternalViewer = TRUE;
          fExternalCollector = TRUE;
          fExternalINIs = TRUE;
          fUserListSwitches = TRUE;
          WinSendMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),UM_UNDO,MPVOID,MPVOID);
          if(hwndTree) {

            CNRINFO cnri;
            ULONG   flWindowAttr = CV_TREE | CV_ICON |
                                   CV_FLOW | CA_TREELINE;

            memset(&cnri,0,sizeof(cnri));
            cnri.cb = sizeof(cnri);
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),
                       TREE_CNR),
                       CM_QUERYCNRINFO,
                       MPFROMP(&cnri),
                       MPFROMLONG(sizeof(cnri)));
            cnri.flWindowAttr = flWindowAttr;
            WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree,FID_CLIENT),
                       TREE_CNR),
                       CM_SETCNRINFO,
                       MPFROMP(&cnri),
                       MPFROMLONG(CMA_FLWINDOWATTR));
          }
          if(hwndMain) {
            if(fAutoView)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_AUTOVIEW,0),MPVOID);
            if(fUserComboBox)
              WinSendMsg(hwndMain,WM_COMMAND,
                         MPFROM2SHORT(IDM_USERLIST,0),MPVOID);
            {
              SWP swp;

              if(WinQueryWindowPos(hwndMain,&swp)) {
                WinSetWindowPos(hwndTree,HWND_TOP,0,0,
                                swp.cx / 5,swp.cy,
                                SWP_MOVE | SWP_SIZE);
              }
            }
            PostMsg(MainObjectHwnd,UM_RESTORE,MPVOID,MPFROMLONG(2L));
            PostMsg(MainObjectHwnd,UM_SETDIR,MPVOID,MPVOID);
          }
          return 0;

        case DID_CANCEL:
          WinSendMsg(hwnd,UM_UNDO,MPVOID,MPVOID);

        case DID_OK:
          PostMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),msg,mp1,mp2);
          return 0;

        default:
          return 0;
      }
      PrfWriteProfileData(fmprof,appname,"DetailsLongname",
                          &detailslongname,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLongname",
                          &detailslongname,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsSubject",
                          &detailssubject,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsSubject",
                          &detailssubject,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsEA",
                          &detailsea,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsEA",
                          &detailsea,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsSize",
                          &detailssize,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsSize",
                          &detailssize,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsIcon",
                          &detailsicon,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsIcon",
                          &detailsicon,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsLWDate",
                          &detailslwdate,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLWDate",
                          &detailslwdate,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsLWTime",
                          &detailslwtime,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLWTime",
                          &detailslwtime,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsLADate",
                          &detailsladate,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLADate",
                          &detailsladate,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsLATime",
                          &detailslatime,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsLATime",
                          &detailslatime,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsCRDate",
                          &detailscrdate,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsCRDate",
                          &detailscrdate,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsCRTime",
                          &detailscrtime,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsCRTime",
                          &detailscrtime,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DetailsAttr",
                          &detailsattr,sizeof(BOOL));
      PrfWriteProfileData(fmprof,appname,"DirCnr.DetailsAttr",
                          &detailsattr,sizeof(BOOL));
      if(hwndMain) {
        if(SaveDirCnrState(hwndMain,
                           GetPString(IDS_FM2TEMPTEXT))) {
          PostMsg(MainObjectHwnd,
                  UM_RESTORE,
                  MPVOID,
                  MPFROMLONG(2L));
          PostMsg(hwndMain,
                  UM_RESTORE,
                  MPVOID,
                  MPVOID);
        }
      }
      WinSendMsg((HWND)WinQueryWindowULong(hwnd,QWL_USER),
                 UM_UNDO,
                 MPVOID,
                 MPVOID);
      return 0;

    case WM_CLOSE:
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


NOTEPAGES np[] = {CFGD_FRAME,
                   IDS_NOTEDIRCNRS1TEXT,
                   CfgDDlgProc,
                   0,
                   0,
                   0,
                  CFG5_FRAME,
                   IDS_NOTEDIRVIEW1TEXT,
                   Cfg5DlgProc,
                   0,
                   0,
                   0,
                  CFG6_FRAME,
                   IDS_NOTEDIRSORT1TEXT,
                   Cfg6DlgProc,
                   0,
                   0,
                   0,
                  CFG5_FRAME,
                   IDS_NOTECOLVIEW1TEXT,
                   Cfg7DlgProc,
                   0,
                   0,
                   0,
                  CFG6_FRAME,
                   IDS_NOTECOLSORT1TEXT,
                   Cfg8DlgProc,
                   0,
                   0,
                   0,
                  CFGA_FRAME,
                   IDS_NOTEARCHIVER1TEXT,
                   CfgADlgProc,
                   0,
                   0,
                   0,
                  CFGT_FRAME,
                   IDS_NOTETREE1TEXT,
                   CfgTDlgProc,
                   0,
                   0,
                   0,
                  CFGTS_FRAME,
                   IDS_NOTETREESORT1TEXT,
                   CfgTSDlgProc,
                   0,
                   0,
                   0,
                  CFGV_FRAME,
                   IDS_NOTEVIEWERS1TEXT,
                   CfgVDlgProc,
                   0,
                   0,
                   0,
                  CFGC_FRAME,
                   IDS_NOTECOMPARE1TEXT,
                   CfgCDlgProc,
                   0,
                   0,
                   0,
                  CFGM_FRAME,
                   IDS_NOTEMONOLITHIC1TEXT,
                   CfgMDlgProc,
                   0,
                   0,
                   0,
                  CFGG_FRAME,
                   IDS_NOTEGENERAL1TEXT,
                   CfgGDlgProc,
                   0,
                   0,
                   0,
                  CFGS_FRAME,
                   IDS_NOTESCANNING1TEXT,
                   CfgSDlgProc,
                   0,
                   0,
                   0,
                  CFGB_FRAME,
                   IDS_NOTEBUBBLE1TEXT,
                   CfgBDlgProc,
                   0,
                   0,
                   0,
                  CFG9_FRAME,
                   IDS_NOTEQUICK1TEXT,
                   Cfg9DlgProc,
                   0,
                   0,
                   0,
                  0,
                   0,
                   NULL,
                   0,
                   0,
                   0};


MRESULT EXPENTRY CfgDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  HWND   hwndTemp;
  USHORT attrib = BKA_FIRST;
  INT    x;
  ULONG  pageID;

  switch(msg)
  {
    case WM_INITDLG:
      hwndNotebook = hwnd;
      if(mp2) {
        if(!strcmp((CHAR *)mp2,"FM/4")) {
          x = 0;
          while(np[x].frameid && np[x].frameid != CFG9_FRAME)
            x++;
          np[x].frameid = 0;
        }
      }
      PostMsg(hwnd,
              UM_SETUP,
              MPVOID,
              MPVOID);
      WinSendDlgItemMsg(hwnd,
                        CFG_NOTEBOOK,
                        BKM_SETDIMENSIONS,
                        MPFROM2SHORT(82,24),
                        MPFROMLONG(BKA_MAJORTAB));
      WinSendDlgItemMsg(hwnd,
                        CFG_NOTEBOOK,
                        BKM_SETDIMENSIONS,
                        MPFROM2SHORT(20,20),
                        MPFROMLONG(BKA_PAGEBUTTON));
      WinSendDlgItemMsg(hwnd,
                        CFG_NOTEBOOK,
                        BKM_SETDIMENSIONS,
                        MPFROM2SHORT(0,0),
                        MPFROMLONG(BKA_MINORTAB));
      for(x = 0;np[x].frameid;x++) {
        hwndTemp = WinLoadDlg(HWND_DESKTOP,
                              HWND_DESKTOP,
                              np[x].proc,
                              FM3ModHandle,
                              np[x].frameid,
                              MPVOID);
        if(hwndTemp) {
          WinSetWindowULong(hwndTemp,
                            QWL_USER,
                            (ULONG)hwnd);
          np[x].hwnd = hwndTemp;
          np[x].pageID = (ULONG)WinSendDlgItemMsg(hwnd,
                                                  CFG_NOTEBOOK,
                                                  BKM_INSERTPAGE,
                                                  MPFROMLONG(BKA_FIRST),
                                                  MPFROM2SHORT(BKA_AUTOPAGESIZE |
                                                               BKA_STATUSTEXTON |
                                                               BKA_MAJOR,
                                                               attrib));
          attrib = BKA_LAST;
          WinSendDlgItemMsg(hwnd,
                            CFG_NOTEBOOK,
                            BKM_SETPAGEWINDOWHWND,
                            MPFROMLONG(np[x].pageID),
                            MPFROMLONG(np[x].hwnd));
          WinSendDlgItemMsg(hwnd,
                            CFG_NOTEBOOK,
                            BKM_SETTABTEXT,
                            MPFROMLONG(np[x].pageID),
                            MPFROMP(GetPString(np[x].title)));
          WinSendDlgItemMsg(hwnd,
                            CFG_NOTEBOOK,
                            BKM_SETSTATUSLINETEXT,
                            MPFROMLONG(np[x].pageID),
                            MPFROMP(GetPString(np[x].title + 1)));
        }
      }
      /* see if we've been asked to display quick cfg page */
      if(!mp2 ||
         strcmp((CHAR *)mp2,"First Time") ||
         !x ||
         !np[x - 1].hwnd ||
         !np[x - 1].pageID) {
        PostMsg(WinWindowFromID(hwnd,CFG_NOTEBOOK),
                BKM_TURNTOPAGE,
                MPFROMLONG(np[0].pageID),
                MPVOID);
      }
      else {
        PostMsg(MainObjectHwnd,
                UM_SETDIR,
                MPFROMLONG(1L),
                MPVOID);
        PostMsg(WinWindowFromID(hwnd,CFG_NOTEBOOK),
                BKM_TURNTOPAGE,
                MPFROMLONG(np[x - 1].pageID),
                MPVOID);
        PostMsg(hwnd,
                UM_FOCUSME,
                MPFROMLONG(np[x - 1].hwnd),
                MPVOID);
        PostMsg(np[x - 1].hwnd,
                WM_COMMAND,
                MPFROM2SHORT(IDM_HELP,0),
                MPVOID);
      }
      break;

    case UM_FOCUSME:
      if(mp1)
        WinSetActiveWindow(HWND_DESKTOP,
                           (HWND)mp1);
      break;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case CFG_NOTEBOOK:
          switch(SHORT2FROMMP(mp1)) {
            case BKN_PAGESELECTED:
              if(mp2) {

                PAGESELECTNOTIFY *psn = mp2;

                WinSendDlgItemMsg(hwnd,
                                  CFG_NOTEBOOK,
                                  BKM_QUERYPAGEWINDOWHWND,
                                  MPFROMLONG(psn->ulPageIdNew),
                                  MPVOID);
              }
              break;
          }
          break;
      }
      return 0;

    case UM_SETUP:
      WinSetActiveWindow(HWND_DESKTOP,
                         WinQueryWindow(hwnd,QW_OWNER));
      WinSetActiveWindow(HWND_DESKTOP,
                         hwnd);
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          WinDismissDlg(hwnd,1);
          break;

        case DID_CANCEL:
	  // Tell current page to undo itself in case changed and still alive
          pageID = (ULONG)WinSendDlgItemMsg(hwnd,
                                            CFG_NOTEBOOK,
                                            BKM_QUERYPAGEID,
                                            MPFROMLONG(0),
                                            MPFROM2SHORT(BKA_TOP,0));
          hwndTemp = (HWND)WinSendDlgItemMsg(hwnd,
                                             CFG_NOTEBOOK,
                                             BKM_QUERYPAGEWINDOWHWND,
                                             MPFROMLONG(pageID),
                                             MPVOID);
          if (hwndTemp)
            WinSendMsg(hwndTemp,UM_UNDO,MPVOID,MPVOID);

          WinDismissDlg(hwnd,0);
          break;

        case IDM_HELP:  /* relay message to appropriate page's window */
          pageID = (ULONG)WinSendDlgItemMsg(hwnd,
                                            CFG_NOTEBOOK,
                                            BKM_QUERYPAGEID,
                                            MPFROMLONG(0),
                                            MPFROM2SHORT(BKA_TOP,0));
          hwndTemp = (HWND)WinSendDlgItemMsg(hwnd,
                                             CFG_NOTEBOOK,
                                             BKM_QUERYPAGEWINDOWHWND,
                                             MPFROMLONG(pageID),
                                             MPVOID);
          if(hwndTemp)
            PostMsg(hwndTemp,
                    WM_COMMAND,
                    MPFROM2SHORT(IDM_HELP,0),
                    MPVOID);
          break;
      }
      return 0;

    case UM_UNDO:
      for(x = 0;np[x].frameid;x++) {
        if(np[x].hwnd)
          WinSendMsg(np[x].hwnd,
                     UM_UNDO,
                     MPVOID,
                     MPVOID);
      }
      break;

    case WM_DESTROY:
      if(np[0].frameid) {
        for(x = 1;np[x].frameid;x++) {
          if(np[x].hwnd) {
            WinSendMsg(np[x].hwnd,
                       WM_CLOSE,
                       MPVOID,
                       MPVOID);
            np[x].hwnd = (HWND)0;
            np[x].pageID = 0;
          }
        }
        WinSendMsg(np[0].hwnd,
                   WM_CLOSE,
                   MPVOID,
                   MPVOID);
        np[0].hwnd = (HWND)0;
        np[0].pageID = 0;
      }
      hwndNotebook = (HWND)0;
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

