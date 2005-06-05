
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2004, 2005 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  05 Jun 05 SHL Use QWL_USER

***********************************************************************/

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <share.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA1)
#pragma alloc_text(WALKER,FillPathListBox,WalkDlgProc,TextSubProc)
#pragma alloc_text(WALKER,WalkAllDlgProc,WalkCopyDlgProc)
#pragma alloc_text(WALKER,WalkMoveDlgProc,WalkExtractDlgProc,WalkTargetDlgProc)
#pragma alloc_text(WALK2,WalkTwoDlgProc,WalkTwoCmpDlgProc,WalkTwoSetDlgProc)
#pragma alloc_text(UDIRS,add_udir,remove_udir,remove_ldir,load_udirs)
#pragma alloc_text(UDIRS,save_udirs,load_setup,save_setup,add_setup)
#pragma alloc_text(UDIRS,remove_setup)

typedef struct {
  USHORT size;
  USHORT changed;
  BOOL   nounwriteable;
  CHAR   szCurrentPath[CCHMAXPATH];
  CHAR  *szReturnPath;
} WALKER;

static CHAR  WalkFont[CCHMAXPATH] = "";
static ULONG WalkFontSize = sizeof(WalkFont);


VOID load_setups (VOID) {

  ULONG len = sizeof(lastsetups);

  memset(lastsetups,0,len);
  PrfQueryProfileData(fmprof,
                      FM3Str,
                      "LastSetups",
                      lastsetups,
                      &len);
  len = sizeof(INT);
  lastsetup = 0;
  PrfQueryProfileData(fmprof,
                      FM3Str,
                      "LastSetup",
                      &lastsetup,
                      &len);
  loadedsetups = TRUE;
}


VOID save_setups (VOID) {

  if(!loadedsetups)
    return;
  PrfWriteProfileData(fmprof,
                      FM3Str,
                      "LastSetups",
                      lastsetups,
                      (ULONG)sizeof(lastsetups));
  PrfWriteProfileData(fmprof,
                      FM3Str,
                      "LastSetup",
                      &lastsetup,
                      (ULONG)sizeof(INT));
}


BOOL add_setup (CHAR *name) {

  INT  x;

  if(!name || !*name)
    return FALSE;
  if(!loadedsetups)
    load_setups();
  for(x = 0;x < MAXNUMSETUPS;x++) {
    if(!stricmp(lastsetups[x],name))
      return FALSE;
  }
  lastsetup++;
  if(lastsetup >= MAXNUMSETUPS)
    lastsetup = 0;
  strcpy(lastsetups[lastsetup],name);
  return TRUE;
}


BOOL remove_setup (CHAR *name) {

  INT x,y;

  if(!name || !*name)
    return FALSE;
  if(!loadedsetups)
    load_setups();
  for(x = 0;x < MAXNUMSETUPS;x++) {
    if(!stricmp(lastsetups[x],name)) {
      *lastsetups[x] = 0;
      for(y = x + 1;y < MAXNUMSETUPS;y++)
        strcpy(lastsetups[y - 1],lastsetups[y]);
      *lastsetups[MAXNUMSETUPS - 1] = 0;
      if(lastsetup >= x)
        lastsetup--;
      return TRUE;
    }
  }
  return FALSE;
}


VOID load_udirs (VOID) {

  /* load linked list of user directories from USERDIRS.DAT file */

  FILE      *fp;
  LINKDIRS  *info,*last = NULL;
  CHAR       s[CCHMAXPATH + 24];

  loadedudirs = TRUE;
  fUdirsChanged = FALSE;
  save_dir2(s);
  if(s[strlen(s) - 1] != '\\')
    strcat(s,"\\");
  strcat(s,"USERDIRS.DAT");
  fp = _fsopen(s,"r",SH_DENYWR);
  if(fp) {
    while(!feof(fp)) {
      if(!fgets(s,CCHMAXPATH + 24,fp))
        break;
      s[CCHMAXPATH] = 0;
      bstripcr(s);
      if(*s && *s != ';') {
        info = malloc(sizeof(LINKDIRS));
        if(info) {
          info->path = strdup(s);
          if(info->path) {
            info->next = NULL;
            if(!udirhead)
              udirhead = info;
            else
              last->next = info;
            last = info;
          }
          else
            free(info);
        }
      }
    }
    fclose(fp);
  }
}


VOID save_udirs (VOID) {

  FILE     *fp;
  LINKDIRS *info;
  CHAR      s[CCHMAXPATH + 14];

  if(loadedudirs) {
    fUdirsChanged = FALSE;
    if(udirhead) {
      save_dir2(s);
      if(s[strlen(s) - 1] != '\\')
        strcat(s,"\\");
      strcat(s,"USERDIRS.DAT");
      fp = fopen(s,"w");
      if(fp) {
        fputs(GetPString(IDS_USERDEFDIRSTEXT),fp);
        info = udirhead;
        while(info) {
          fprintf(fp,
                  "%0.*s\n",
                  CCHMAXPATH,
                  info->path);
          info = info->next;
        }
        fclose(fp);
      }
    }
  }
}


BOOL add_udir (BOOL userdirs,CHAR *inpath) {

  CHAR      path[CCHMAXPATH];
  LINKDIRS *info,*last = NULL,*temp = NULL;

  if(inpath &&
     *inpath) {
    if(DosQueryPathInfo(inpath,
                        FIL_QUERYFULLNAME,
                        path,
                        sizeof(path)))
      strcpy(path,inpath);
    if(!userdirs &&
       IsRoot(path))
      return FALSE;
    if(IsFullName(path)) {
      if(!loadedudirs)
        load_udirs();
      info = (userdirs) ? udirhead : ldirhead;
      while(info) {
        if(!stricmp(info->path,path))
          return FALSE;
        last = info;
        info = info->next;
      }
      if(!userdirs) {
        info = udirhead;
        while(info) {
          if(!stricmp(info->path,path))
            return FALSE;
          info = info->next;
        }
      }
      else {  /* if adding manual directory, remove from auto list if present */
        info = ldirhead;
        while(info) {
          if(!stricmp(info->path,path)) {
            if(temp)
              temp->next = info->next;
            else
              ldirhead = info->next;
            free(info->path);
            free(info);
            break;
          }
          temp = info;
          info = info->next;
        }
      }
      info = malloc(sizeof(LINKDIRS));
      if(info) {
        info->path = strdup(path);
        if(info->path) {
          info->next = NULL;
          if(userdirs) {
            fUdirsChanged = TRUE;
            if(!udirhead)
              udirhead = info;
            else
              last->next = info;
          }
          else {
            if(!ldirhead)
              ldirhead = info;
            else
              last->next = info;
          }
          return TRUE;
        }
        else
          free(info);
      }
    }
  }
  return FALSE;
}


BOOL remove_udir (CHAR *path) {

  LINKDIRS *info,*last = NULL;

  if(path && *path) {
    if(!loadedudirs)
      load_udirs();
    info = udirhead;
    while(info) {
      if(!stricmp(info->path,path)) {
        if(last)
          last->next = info->next;
        else
          udirhead = info->next;
        free(info->path);
        free(info);
        fUdirsChanged = TRUE;
        return TRUE;
      }
      last = info;
      info = info->next;
    }
    info = ldirhead;
    while(info) {
      if(!stricmp(info->path,path)) {
        if(last)
          last->next = info->next;
        else
          ldirhead = info->next;
        free(info->path);
        free(info);
        return TRUE;
      }
      last = info;
      info = info->next;
    }
  }
  return FALSE;
}


BOOL remove_ldir (CHAR *path) {

  LINKDIRS *info,*last = NULL;

  if(path && *path) {
    info = ldirhead;
    while(info) {
      if(!stricmp(info->path,path)) {
        if(last)
          last->next = info->next;
        else
          ldirhead = info->next;
        free(info->path);
        free(info);
        return TRUE;
      }
      last = info;
      info = info->next;
    }
  }
  return FALSE;
}


VOID FillPathListBox (HWND hwnd,HWND hwnddrive,HWND hwnddir,CHAR *path,
                      BOOL nounwriteable) {

    /*
     * this function fills one or two list boxes with drive and directory
     * information showing all available drives and all directories off of
     * the directory represented by path.  This works independently of the
     * current directory.
     */

  CHAR         szDrive[] = " :",szTemp[1032];
  FILEFINDBUF3 findbuf;
  HDIR         hDir = HDIR_CREATE;
  SHORT        sDrive;
  ULONG        ulDriveNum,ulSearchCount = 1L,ulDriveMap;

  DosError(FERR_DISABLEHARDERR);
  DosQCurDisk(&ulDriveNum,&ulDriveMap);
  if(hwnddrive)
    WinSendMsg(hwnddrive,LM_DELETEALL,MPVOID,MPVOID);
  if(hwnddrive != hwnddir && hwnddir)
    WinSendMsg(hwnddir,LM_DELETEALL,MPVOID,MPVOID);

  if(hwnddrive) {
    for(sDrive = 0;sDrive < 26;sDrive++) {
      if(ulDriveMap & (1L << sDrive)) {
        *szDrive = (CHAR)(sDrive + 'A');
        if((!nounwriteable || !(driveflags[sDrive] & DRIVE_NOTWRITEABLE)) &&
           !(driveflags[sDrive] & (DRIVE_IGNORE | DRIVE_INVALID)))
          WinSendMsg(hwnddrive,LM_INSERTITEM,MPFROM2SHORT(LIT_END,0),
                     MPFROMP(szDrive));
      }
    }
    if(hwnddrive != hwnddir && path && isalpha(*path) && path[1] == ':') {
      *szDrive = toupper(*path);
      WinSetWindowText(hwnddrive,szDrive);
    }
  }

  if(hwnddir) {
    sprintf(szTemp,
            "%s%s*",
            path,
            (path[strlen(path) - 1] == '\\') ? "" : "\\");
    DosError(FERR_DISABLEHARDERR);
    if(!DosFindFirst(szTemp,
                     &hDir,
                     FILE_DIRECTORY | MUST_HAVE_DIRECTORY |
                     FILE_READONLY | FILE_ARCHIVED | FILE_SYSTEM |
                     FILE_HIDDEN,
                     &findbuf,
                     sizeof(FILEFINDBUF3),
                     &ulSearchCount,
                     FIL_STANDARD)) {
      do {
        if(findbuf.attrFile & FILE_DIRECTORY) {
          if(strcmp(findbuf.achName,"..") ||
             strlen(path) > 3 ||
             path[1] != ':') {
            if(findbuf.achName[0] != '.' ||
                findbuf.achName[1])
              WinSendMsg(hwnddir,
                         LM_INSERTITEM,
                         MPFROM2SHORT(LIT_SORTASCENDING,0),
                         MPFROMP(findbuf.achName));
          }
        }
        ulSearchCount = 1L;
      } while(!DosFindNext(hDir,
                           &findbuf,
                           sizeof(FILEFINDBUF3),
                           &ulSearchCount));
      DosFindClose(hDir);
    }
    DosError(FERR_DISABLEHARDERR);
  }
}


MRESULT EXPENTRY TextSubProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PFNWP oldproc = (PFNWP)WinQueryWindowPtr(hwnd,0);

  switch(msg) {
    case WM_CHAR:
      if(SHORT1FROMMP(mp1) & KC_KEYUP) {
        if((SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
           (SHORT1FROMMP(mp2) & 255) == '\r')
          PostMsg(WinQueryWindow(hwnd,QW_PARENT),WM_COMMAND,
                     MPFROM2SHORT(DID_OK,0),MPVOID);
      }
      break;
  }
  return oldproc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY WalkDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  WALKER      *wa;
  CHAR         szBuff[CCHMAXPATH + 1],szBuffer[CCHMAXPATH + 1],*p;
  SHORT        sSelect;
  static BOOL  okay;  /* avoid combobox selecting as filled */
  static CHAR  lastdir[CCHMAXPATH + 1];

  switch(msg) {
    case UM_SETUP2:
    case WM_INITDLG:
      okay = FALSE;
      *lastdir = 0;
      if(!mp2) {
        WinDismissDlg(hwnd,0);
        break;
      }
      wa = malloc(sizeof(WALKER));
      if(!wa) {
        DosBeep(50,100);
        WinDismissDlg(hwnd,0);
        break;
      }
      memset(wa,0,sizeof(WALKER));
      wa->size = (USHORT)sizeof(WALKER);
      WinSetWindowPtr(hwnd,0,(PVOID)wa);
      wa->szReturnPath = (CHAR *)mp2;
      {
        PFNWP oldproc;

        oldproc = WinSubclassWindow(WinWindowFromID(hwnd,WALK_PATH),
                                    (PFNWP)TextSubProc);
        if(oldproc)
          WinSetWindowPtr(WinWindowFromID(hwnd,WALK_PATH),
                          QWL_USER,
                          (PVOID)oldproc);
        WinSendDlgItemMsg(WinWindowFromID(hwnd,WALK_RECENT),
                          CBID_EDIT,
                          EM_SETTEXTLIMIT,
                          MPFROM2SHORT(CCHMAXPATH,0),
                          MPVOID);
        WinSendDlgItemMsg(WinWindowFromID(hwnd,WALK_RECENT),
                          CBID_EDIT,
                          EM_SETREADONLY,
                          MPFROM2SHORT(TRUE,0),
                          MPVOID);
      }
      PosOverOkay(hwnd);
      if(msg == UM_SETUP2)
        wa->nounwriteable = FALSE;
      else
        wa->nounwriteable = TRUE;
      if(!*wa->szReturnPath)
        save_dir2(wa->szCurrentPath);
      else {
        strcpy(wa->szCurrentPath,
               wa->szReturnPath);
        MakeFullName(wa->szCurrentPath);
      }
      if(wa->nounwriteable &&
         (driveflags[toupper(*wa->szCurrentPath) - 'A'] &
          DRIVE_NOTWRITEABLE)) {

        ULONG bd;

        strcpy(wa->szCurrentPath,"C:\\");
        if(DosQuerySysInfo(QSV_BOOT_DRIVE,
                           QSV_BOOT_DRIVE,
                           (PVOID)&bd,
                           (ULONG)sizeof(ULONG)))
          bd = 3L;
        *wa->szCurrentPath = (CHAR)bd + '@';
      }
      WinSendDlgItemMsg(hwnd,
                        WALK_PATH,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),
                        MPVOID);
      WinSetDlgItemText(hwnd,
                        WALK_PATH,
                        wa->szCurrentPath);
      if(!loadedudirs)
        load_udirs();
      {     /* fill user list box */
        ULONG        ulDriveNum,ulDriveMap;
        ULONG        ulSearchCount;
        FILEFINDBUF3 findbuf;
        HDIR         hDir;
        APIRET       rc;
        LINKDIRS    *info,*temp;

        DosError(FERR_DISABLEHARDERR);
        DosQCurDisk(&ulDriveNum,&ulDriveMap);
        info = udirhead;
        while(info) {
          if(IsFullName(info->path) &&
             !(driveflags[toupper(*info->path) - 'A'] &
               (DRIVE_IGNORE | DRIVE_INVALID))) {
            DosError(FERR_DISABLEHARDERR);
            hDir = HDIR_CREATE;
            ulSearchCount = 1L;
            if(!IsRoot(info->path))
              rc = DosFindFirst(info->path,&hDir,FILE_DIRECTORY |
                                MUST_HAVE_DIRECTORY | FILE_READONLY |
                                FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
                                &findbuf,sizeof(FILEFINDBUF3),
                                &ulSearchCount, FIL_STANDARD);
            else {
              rc = 0;
              findbuf.attrFile = FILE_DIRECTORY;
            }
            if(!rc) {
              if(!IsRoot(info->path))
                DosFindClose(hDir);
              if(findbuf.attrFile & FILE_DIRECTORY)
                WinSendDlgItemMsg(hwnd,WALK_USERLIST,LM_INSERTITEM,
                                  MPFROM2SHORT(LIT_SORTASCENDING,0),
                                  MPFROMP(info->path));
              else {
                temp = info->next;
                remove_udir(info->path);
                info = temp;
                continue;
              }
            }
            else if(!(ulDriveMap & (1L << (toupper(*info->path) - 'A')))) {
              temp = info->next;
              remove_udir(info->path);
              info = temp;
              continue;
            }
          }
          info = info->next;
        }
        info = ldirhead;
        while(info) {
          if(IsFullName(info->path) &&
             !(driveflags[toupper(*info->path) - 'A'] &
               (DRIVE_IGNORE | DRIVE_INVALID))) {
            DosError(FERR_DISABLEHARDERR);
            hDir = HDIR_CREATE;
            ulSearchCount = 1L;
            if(!IsRoot(info->path))
              rc = DosFindFirst(info->path,&hDir,FILE_DIRECTORY |
                                MUST_HAVE_DIRECTORY | FILE_READONLY |
                                FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
                                &findbuf,sizeof(FILEFINDBUF3),
                                &ulSearchCount, FIL_STANDARD);
            else {
              rc = 0;
              findbuf.attrFile = FILE_DIRECTORY;
            }
            if(!rc) {
              if(!IsRoot(info->path))
                DosFindClose(hDir);
              if(findbuf.attrFile & FILE_DIRECTORY)
                WinSendDlgItemMsg(hwnd,WALK_RECENT,LM_INSERTITEM,
                                  MPFROM2SHORT(LIT_SORTASCENDING,0),
                                  MPFROMP(info->path));
              else {
                temp = info->next;
                remove_ldir(info->path);
                info = temp;
                continue;
              }
              WinSetDlgItemText(hwnd,WALK_RECENT,
                                GetPString(IDS_WALKRECENTDIRSTEXT));
            }
            else if(!(ulDriveMap & (1L << (toupper(*info->path) - 'A')))) {
              temp = info->next;
              remove_ldir(info->path);
              info = temp;
              continue;
            }
          }
          info = info->next;
        }
      }
      FillPathListBox(hwnd,
                      WinWindowFromID(hwnd,WALK_DRIVELIST),
                      WinWindowFromID(hwnd,WALK_DIRLIST),
                      wa->szCurrentPath,wa->nounwriteable);
      if(!PostMsg(hwnd,
                  UM_SETUP4,
                  MPVOID,
                  MPVOID))
        okay = TRUE;
      {
        MRESULT ret;

        ret = WinDefDlgProc(hwnd,WM_INITDLG,mp1,mp2);
        WinSendMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
        WinInvalidateRect(WinWindowFromID(hwnd,WALK_PATH),NULL,TRUE);
        return ret;
      }

    case UM_SETUP4:
      okay = TRUE;
      return 0;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,UM_SETDIR,MPVOID,MPVOID);
      break;

    case UM_SETDIR:
      PaintRecessedWindow(WinWindowFromID(hwnd,WALK_HELP),(HPS)0,FALSE,TRUE);
      return 0;

    case WM_PRESPARAMCHANGED:
      {
        ULONG AttrFound,AttrValue[64],cbRetLen;

        cbRetLen = WinQueryPresParam(hwnd,(ULONG)mp1,0,&AttrFound,
                                     (ULONG)sizeof(AttrValue),
                                     &AttrValue,0);
        if(cbRetLen) {
          switch(AttrFound) {
            case PP_FONTNAMESIZE:
              PrfWriteProfileData(fmprof,
                                  appname,
                                  "WalkFont",
                                  (PVOID)AttrValue,
                                  cbRetLen);
              *WalkFont = 0;
              WalkFontSize = sizeof(WalkFont);
              WinInvalidateRect(WinWindowFromID(hwnd,WALK_PATH),
                                NULL,
                                TRUE);
              break;
          }
        }
      }
      break;

    case UM_SETUP3:
      save_udirs();
      if(hwndMain)
        PostMsg(hwndMain,
                UM_FILLUSERLIST,
                MPVOID,
                MPVOID);
      return 0;

    case UM_SETUP:
      {
        INT    x;
        USHORT id[] = {WALK_PATH,WALK_DIRLIST,WALK_USERLIST,
                       WALK_RECENT,0};

        if(*WalkFont ||
           (PrfQueryProfileData(fmprof,
                                appname,
                                "WalkFont",
                                (PVOID)WalkFont,
                                &WalkFontSize) &&
            WalkFontSize)) {
          for(x = 0;id[x];x++)
            WinSetPresParam(WinWindowFromID(hwnd,id[x]),
                            PP_FONTNAMESIZE,
                            WalkFontSize,
                            (PVOID)WalkFont);
        }
      }
      return 0;

    case UM_CONTROL:
    case WM_CONTROL:
      wa = WinQueryWindowPtr(hwnd,0);
      if(SHORT1FROMMP(mp1) == WALK_DRIVELIST ||
         SHORT1FROMMP(mp1) == WALK_DIRLIST ||
         SHORT1FROMMP(mp1) == WALK_USERLIST ||
         SHORT1FROMMP(mp1) == WALK_RECENT) {
        sSelect = (USHORT)WinSendDlgItemMsg(hwnd,
                                            SHORT1FROMMP(mp1),
                                            LM_QUERYSELECTION,
                                            MPVOID,MPVOID);
        *szBuffer = 0;
        if(sSelect >= 0)
          WinSendDlgItemMsg(hwnd,SHORT1FROMMP(mp1),LM_QUERYITEMTEXT,
                            MPFROM2SHORT(sSelect,CCHMAXPATH),
                            MPFROMP(szBuffer));
      }
      switch(SHORT1FROMMP(mp1)) {
        case WALK_PATH:
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,WALK_HELP,
                              GetPString(IDS_WALKCURRDIRTEXT));
          else if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,WALK_HELP,
                              GetPString(IDS_WALKDEFAULTHELPTEXT));
          break;

        case WALK_RECENT:
          if(okay && SHORT2FROMMP(mp1) == CBN_LBSELECT) {

            ULONG        ulSearchCount;
            FILEFINDBUF3 findbuf;
            HDIR         hDir;
            APIRET       rc;

//            *szBuffer = 0;
//            WinQueryDlgItemText(hwnd,WALK_RECENT,CCHMAXPATH,szBuffer);
            if(!*szBuffer)
              break;
            DosError(FERR_DISABLEHARDERR);
            hDir = HDIR_CREATE;
            ulSearchCount = 1L;
            if(!IsRoot(szBuffer)) {
              rc = DosFindFirst(szBuffer,&hDir,FILE_DIRECTORY |
                                MUST_HAVE_DIRECTORY | FILE_READONLY |
                                FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
                                &findbuf,sizeof(FILEFINDBUF3),
                                &ulSearchCount, FIL_STANDARD);
              if(!rc)
                DosFindClose(hDir);
            }
            else {
              findbuf.attrFile = FILE_DIRECTORY;
              rc = 0;
            }
            if(!rc && (findbuf.attrFile & FILE_DIRECTORY)) {
              strcpy(wa->szCurrentPath,szBuffer);
              WinSetDlgItemText(hwnd,WALK_PATH,wa->szCurrentPath);
              WinSetDlgItemText(hwnd,WALK_RECENT,wa->szCurrentPath);
              FillPathListBox(hwnd,
                              WinWindowFromID(hwnd,WALK_DRIVELIST),
                              WinWindowFromID(hwnd,WALK_DIRLIST),
                              wa->szCurrentPath,FALSE);
            }
            else
              DosBeep(50,100);
          }
          else if(SHORT2FROMMP(mp1) == CBN_ENTER)
            PostMsg(hwnd,WM_COMMAND,MPFROM2SHORT(DID_OK,0),MPVOID);
          else if(SHORT2FROMMP(mp1) == CBN_SHOWLIST)
            WinSetDlgItemText(hwnd,WALK_HELP,
                              GetPString(IDS_WALKRECENTDIRSHELPTEXT));
          break;

        case WALK_USERLIST:
          if(okay && *szBuffer && SHORT2FROMMP(mp1) == LN_SELECT) {

            ULONG        ulSearchCount;
            FILEFINDBUF3 findbuf;
            HDIR         hDir;
            APIRET       rc;

            DosError(FERR_DISABLEHARDERR);
            hDir = HDIR_CREATE;
            ulSearchCount = 1L;
            if(!IsRoot(szBuffer)) {
              rc = DosFindFirst(szBuffer,
                                &hDir,
                                FILE_DIRECTORY |
                                MUST_HAVE_DIRECTORY | FILE_READONLY |
                                FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
                                &findbuf,
                                sizeof(FILEFINDBUF3),
                                &ulSearchCount,
                                FIL_STANDARD);
              if(!rc)
                DosFindClose(hDir);
            }
            else {
              findbuf.attrFile = FILE_DIRECTORY;
              rc = 0;
            }
            if(!rc && (findbuf.attrFile & FILE_DIRECTORY)) {
              strcpy(wa->szCurrentPath,szBuffer);
              WinSetDlgItemText(hwnd,WALK_PATH,wa->szCurrentPath);
              FillPathListBox(hwnd,
                              WinWindowFromID(hwnd,WALK_DRIVELIST),
                              WinWindowFromID(hwnd,WALK_DIRLIST),
                              wa->szCurrentPath,FALSE);
            }
            else
              DosBeep(50,100);
          }
          else if(SHORT2FROMMP(mp1) == LN_ENTER)
            PostMsg(hwnd,
                    WM_COMMAND,
                    MPFROM2SHORT(DID_OK,0),
                    MPVOID);
          else if(SHORT2FROMMP(mp1) == LN_SETFOCUS)
            WinSetDlgItemText(hwnd,
                              WALK_HELP,
                              GetPString(IDS_WALKUSERDIRSHELPTEXT));
          else if(SHORT2FROMMP(mp1) == LN_KILLFOCUS)
            WinSetDlgItemText(hwnd,
                              WALK_HELP,
                              GetPString(IDS_WALKDEFAULTHELPTEXT));
          break;

        case WALK_DRIVELIST:
          if(okay && *szBuffer && SHORT2FROMMP(mp1) == LN_ENTER) {

            ULONG  ulDirLen = CCHMAXPATH;
            APIRET rc;

            rc = DosQCurDir(toupper(*szBuffer) - '@',
                            &szBuff[3],&ulDirLen);
            if(!rc) {
              strcpy(wa->szCurrentPath,"C:\\");
              *wa->szCurrentPath = toupper(*szBuffer);
              WinSetDlgItemText(hwnd,
                                WALK_PATH,
                                wa->szCurrentPath);
              FillPathListBox(hwnd,
                              WinWindowFromID(hwnd,WALK_DRIVELIST),
                              WinWindowFromID(hwnd,WALK_DIRLIST),
                              wa->szCurrentPath,FALSE);
            }
          }
          else if(SHORT2FROMMP(mp1) == LN_SETFOCUS)
            WinSetDlgItemText(hwnd,WALK_HELP,
                              GetPString(IDS_WALKDRIVELISTHELPTEXT));
          else if(SHORT2FROMMP(mp1) == LN_KILLFOCUS)
            WinSetDlgItemText(hwnd,WALK_HELP,
                              GetPString(IDS_WALKDEFAULTHELPTEXT));
          break;

        case WALK_DIRLIST:
          if(okay && SHORT2FROMMP(mp1) == LN_ENTER) {

            ULONG        ulSearchCount;
            FILEFINDBUF3 findbuf;
            HDIR         hDir;
            APIRET       rc;

            bstrip(szBuffer);
            if(*szBuffer) {
              strcpy(szBuff,wa->szCurrentPath);
              if(szBuff[strlen(szBuff) - 1] != '\\')
                strcat(szBuff,"\\");
              strcat(szBuff,
                     szBuffer);
              MakeFullName(szBuff);
              DosError(FERR_DISABLEHARDERR);
              hDir = HDIR_CREATE;
              ulSearchCount = 1L;
              if(!IsRoot(szBuff)) {
                rc = DosFindFirst(szBuff,
                                  &hDir,
                                  FILE_DIRECTORY |
                                  MUST_HAVE_DIRECTORY | FILE_READONLY |
                                  FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
                                  &findbuf,
                                  sizeof(FILEFINDBUF3),
                                  &ulSearchCount,
                                  FIL_STANDARD);
                if(!rc)
                  DosFindClose(hDir);
              }
              else {
                findbuf.attrFile = FILE_DIRECTORY;
                rc = 0;
              }
              if(!rc && (findbuf.attrFile & FILE_DIRECTORY)) {
                strcpy(wa->szCurrentPath,szBuff);
                WinSetDlgItemText(hwnd,WALK_PATH,wa->szCurrentPath);
                FillPathListBox(hwnd,
                                WinWindowFromID(hwnd,WALK_DRIVELIST),
                                WinWindowFromID(hwnd,WALK_DIRLIST),
                                wa->szCurrentPath,FALSE);
              }
            }
          }
          else if(SHORT2FROMMP(mp1) == LN_SETFOCUS)
            WinSetDlgItemText(hwnd,WALK_HELP,
                              GetPString(IDS_WALKDIRLISTHELPTEXT));
          else if(SHORT2FROMMP(mp1) == LN_KILLFOCUS)
            WinSetDlgItemText(hwnd,WALK_HELP,
                              GetPString(IDS_WALKDEFAULTHELPTEXT));
          break;
      }
      return 0;

    case WM_COMMAND:
      wa = WinQueryWindowPtr(hwnd,0);
      if(!wa)
        WinDismissDlg(hwnd,0);
      *szBuff = 0;
      WinQueryDlgItemText(hwnd,
                          WALK_PATH,
                          CCHMAXPATH,
                          szBuff);
      bstrip(szBuff);
      while((p = strchr(szBuff,'/')) != NULL)
        *p = '\\';
      while(strlen(szBuff) > 3 &&
            szBuff[strlen(szBuff) - 1] == '\\')
        szBuff[strlen(szBuff) - 1] = 0;
      MakeFullName(szBuff);
      if(*szBuff &&
         stricmp(szBuff,wa->szCurrentPath)) {
        if(!SetDir(WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),
                                  QW_OWNER),
                   hwnd,
                   szBuff,
                   0))
          strcpy(wa->szCurrentPath,
                 szBuff);
        else if(SHORT1FROMMP(mp1) != DID_CANCEL)
          return 0;
      }
      WinSetDlgItemText(hwnd,
                        WALK_PATH,
                        wa->szCurrentPath);
      switch(SHORT1FROMMP(mp1)) {
        case WALK_ADD:
          *szBuff = 0;
          WinQueryDlgItemText(hwnd,
                              WALK_PATH,
                              CCHMAXPATH,
                              szBuff);
          bstrip(szBuff);
          while((p = strchr(szBuff,'/')) != NULL)
            *p = '\\';
          if(*szBuff &&
             !IsFile(szBuff)) {
            MakeFullName(szBuff);
            if(add_udir(TRUE,
                        szBuff)) {
              WinSendDlgItemMsg(hwnd,
                                WALK_USERLIST,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_SORTASCENDING,0),
                                MPFROMP(szBuff));
              wa->changed = 1;
            }
            else
              DosBeep(50,100);
          }
          break;

        case WALK_DELETE:
          *szBuff = 0;
          WinQueryDlgItemText(hwnd,WALK_PATH,CCHMAXPATH,szBuff);
          bstrip(szBuff);
          while((p = strchr(szBuff,'/')) != NULL)
            *p = '\\';
          if(*szBuff &&
             !IsFile(szBuff)) {
            MakeFullName(szBuff);
            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                               WALK_USERLIST,
                                               LM_SEARCHSTRING,
                                               MPFROM2SHORT(0,LIT_FIRST),
                                               MPFROMP(szBuff));
            if(sSelect >= 0) {
              WinSendDlgItemMsg(hwnd,
                                WALK_USERLIST,
                                LM_DELETEITEM,
                                MPFROM2SHORT(sSelect,0),
                                MPVOID);
              remove_udir(szBuff);
              wa->changed = 1;
            }
          }
          break;

        case DID_OK:
          if(*wa->szCurrentPath) {
            strcpy(wa->szReturnPath,
                   wa->szCurrentPath);
            MakeValidDir(wa->szReturnPath);
            if(fAutoAddAllDirs)
              add_udir(FALSE,
                       wa->szReturnPath);
            if(fChangeTarget) {
              strcpy(targetdir,
                     wa->szReturnPath);
              PrfWriteProfileString(fmprof,
                                    appname,
                                    "Targetdir",
                                    targetdir);
            }
          }
          if(wa->changed)
            WinSendMsg(hwnd,
                       UM_SETUP3,
                       MPVOID,
                       MPVOID);
          WinDismissDlg(hwnd,1);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_WALKEM,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case DID_CANCEL:
          if(wa->changed)
            WinSendMsg(hwnd,
                       UM_SETUP3,
                       MPVOID,
                       MPVOID);
          free(wa);
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;

    case WM_CLOSE:
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY WalkAllDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      return WalkDlgProc(hwnd,
                         UM_SETUP2,
                         mp1,
                         mp2);
  }
  return WalkDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY WalkCopyDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      WinSetWindowText(hwnd,
                       GetPString(IDS_WALKCOPYDLGTEXT));
      return WalkDlgProc(hwnd,
                         UM_SETUP2,
                         mp1,
                         mp2);
  }
  return WalkDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY WalkMoveDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      WinSetWindowText(hwnd,
                       GetPString(IDS_WALKMOVEDLGTEXT));
      return WalkDlgProc(hwnd,
                         UM_SETUP2,
                         mp1,
                         mp2);
  }
  return WalkDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY WalkExtractDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,
                                     MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      WinSetWindowText(hwnd,
                       GetPString(IDS_WALKEXTRACTDLGTEXT));
      return WalkDlgProc(hwnd,
                         UM_SETUP2,
                         mp1,
                         mp2);
  }
  return WalkDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY WalkTargetDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,
                                    MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      {
        char s[CCHMAXPATH + 32];

        sprintf(s,
                GetPString(IDS_WALKTARGETDLGTEXT),
                (*targetdir) ?
                 NullStr :
                 " (",
                (*targetdir) ?
                 NullStr :
                 GetPString(IDS_NONE),
                (*targetdir) ?
                 NullStr :
                 ")");
        WinSetWindowText(hwnd,s);
      }
      return WalkDlgProc(hwnd,
                         UM_SETUP2,
                         mp1,
                         mp2);
  }
  return WalkDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY WalkTwoDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  WALK2       *wa;
  CHAR         szBuff[CCHMAXPATH + 1],szBuffer[CCHMAXPATH + 1],*p;
  SHORT        sSelect;
  static BOOL  okay;  /* avoid combobox selecting as filled */

  switch(msg) {
    case UM_SETUP2:
    case WM_INITDLG:
      okay = FALSE;
      if(!mp2) {
        WinDismissDlg(hwnd,0);
        break;
      }
      WinSetWindowPtr(hwnd,0,mp2);
      wa = mp2;
      {
        PFNWP oldproc;

        oldproc = WinSubclassWindow(WinWindowFromID(hwnd,WALK_PATH),
                                    (PFNWP)TextSubProc);
        if(oldproc)
          WinSetWindowPtr(WinWindowFromID(hwnd,WALK_PATH),
                          QWL_USER,
                          (PVOID)oldproc);
        oldproc = WinSubclassWindow(WinWindowFromID(hwnd,WALK2_PATH),
                                    (PFNWP)TextSubProc);
        if(oldproc)
          WinSetWindowPtr(WinWindowFromID(hwnd,WALK2_PATH),
                          QWL_USER,
                          (PVOID)oldproc);
      }
      if(!*wa->szCurrentPath1)
        save_dir2(wa->szCurrentPath1);
      MakeFullName(wa->szCurrentPath1);
      if(!*wa->szCurrentPath2)
        save_dir2(wa->szCurrentPath2);
      MakeFullName(wa->szCurrentPath2);
      WinSendDlgItemMsg(hwnd,
                        WALK_PATH,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),
                        MPVOID);
      WinSetDlgItemText(hwnd,WALK_PATH,wa->szCurrentPath1);
      WinSendDlgItemMsg(hwnd,WALK2_PATH,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),MPVOID);
      WinSetDlgItemText(hwnd,WALK2_PATH,wa->szCurrentPath2);
      FillPathListBox(hwnd,
                      WinWindowFromID(hwnd,WALK_DRIVELIST),
                      WinWindowFromID(hwnd,WALK_DIRLIST),
                      wa->szCurrentPath1,FALSE);
      FillPathListBox(hwnd,
                      WinWindowFromID(hwnd,WALK2_DRIVELIST),
                      WinWindowFromID(hwnd,WALK2_DIRLIST),
                      wa->szCurrentPath2,FALSE);
      if(!PostMsg(hwnd,UM_SETUP4,MPVOID,MPVOID))
        okay = TRUE;
      {
        MRESULT ret;

        ret = WinDefDlgProc(hwnd,WM_INITDLG,mp1,mp2);
        WinSendMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
        WinInvalidateRect(WinWindowFromID(hwnd,WALK_PATH),NULL,TRUE);
        WinInvalidateRect(WinWindowFromID(hwnd,WALK2_PATH),NULL,TRUE);
        return ret;
      }

    case UM_SETUP4:
      okay = TRUE;
      return 0;

    case WM_PRESPARAMCHANGED:
      {
        ULONG AttrFound,AttrValue[64],cbRetLen;

        cbRetLen = WinQueryPresParam(hwnd,(ULONG)mp1,0,&AttrFound,
                                     (ULONG)sizeof(AttrValue),
                                     &AttrValue,0);
        if(cbRetLen) {
          switch(AttrFound) {
            case PP_FONTNAMESIZE:
              PrfWriteProfileData(fmprof,
                                  appname,
                                  "WalkFont",
                                  (PVOID)AttrValue,
                                  cbRetLen);
              *WalkFont = 0;
              WalkFontSize = sizeof(WalkFont);
              WinInvalidateRect(WinWindowFromID(hwnd,WALK_PATH),
                                NULL,
                                TRUE);
              break;
          }
        }
      }
      break;

    case UM_SETUP:
      {
        INT     x;
        USHORT  id[] = {WALK_PATH,WALK_DIRLIST,
                        WALK2_PATH,WALK2_DIRLIST,0};

        if(*WalkFont ||
           (PrfQueryProfileData(fmprof,
                                appname,
                                "WalkFont",
                                (PVOID)WalkFont,
                                &WalkFontSize) &&
             WalkFontSize)) {
          for(x = 0;id[x];x++)
            WinSetPresParam(WinWindowFromID(hwnd,id[x]),
                            PP_FONTNAMESIZE,
                            WalkFontSize,
                            (PVOID)WalkFont);
        }
      }
      return 0;

    case UM_CONTROL:
    case WM_CONTROL:
      wa = WinQueryWindowPtr(hwnd,0);
      if(SHORT1FROMMP(mp1) == WALK_DRIVELIST ||
         SHORT1FROMMP(mp1) == WALK_DIRLIST ||
         SHORT1FROMMP(mp1) == WALK2_DRIVELIST ||
         SHORT1FROMMP(mp1) == WALK2_DIRLIST) {
        sSelect = (USHORT)WinSendDlgItemMsg(hwnd,
                                            SHORT1FROMMP(mp1),
                                            LM_QUERYSELECTION,
                                            MPVOID,MPVOID);
        *szBuffer = 0;
        if(sSelect >= 0)
          WinSendDlgItemMsg(hwnd,SHORT1FROMMP(mp1),LM_QUERYITEMTEXT,
                            MPFROM2SHORT(sSelect,CCHMAXPATH),
                            MPFROMP(szBuffer));
      }
      switch(SHORT1FROMMP(mp1)) {
        case WALK_DRIVELIST:
          if(okay && *szBuffer && SHORT2FROMMP(mp1) == LN_ENTER) {

            ULONG  ulDirLen = CCHMAXPATH;
            APIRET rc;

            rc = DosQCurDir(toupper(*szBuffer) - '@',
                            &szBuff[3],&ulDirLen);
            if(!rc) {
              strcpy(wa->szCurrentPath1,"C:\\");
              *wa->szCurrentPath1 = toupper(*szBuffer);
              WinSetDlgItemText(hwnd,
                                WALK_PATH,
                                wa->szCurrentPath1);
              FillPathListBox(hwnd,
                              WinWindowFromID(hwnd,WALK_DRIVELIST),
                              WinWindowFromID(hwnd,WALK_DIRLIST),
                              wa->szCurrentPath1,FALSE);
            }
          }
          break;

        case WALK_DIRLIST:
          if(okay && SHORT2FROMMP(mp1) == LN_ENTER) {

            ULONG        ulSearchCount;
            FILEFINDBUF3 findbuf;
            HDIR         hDir;
            APIRET       rc;

            bstrip(szBuffer);
            if(*szBuffer) {
              strcpy(szBuff,wa->szCurrentPath1);
              if(szBuff[strlen(szBuff) - 1] != '\\')
                strcat(szBuff,"\\");
              strcat(szBuff,szBuffer);
              MakeFullName(szBuff);
              DosError(FERR_DISABLEHARDERR);
              hDir = HDIR_CREATE;
              ulSearchCount = 1L;
              if(!IsRoot(szBuff)) {
                rc = DosFindFirst(szBuff,
                                  &hDir,
                                  FILE_DIRECTORY |
                                  MUST_HAVE_DIRECTORY | FILE_READONLY |
                                  FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
                                  &findbuf,
                                  sizeof(FILEFINDBUF3),
                                  &ulSearchCount,
                                  FIL_STANDARD);
                if(!rc)
                  DosFindClose(hDir);
              }
              else {
                findbuf.attrFile = FILE_DIRECTORY;
                rc = 0;
              }
              if(!rc && (findbuf.attrFile & FILE_DIRECTORY)) {
                strcpy(wa->szCurrentPath1,szBuff);
                WinSetDlgItemText(hwnd,WALK_PATH,wa->szCurrentPath1);
                FillPathListBox(hwnd,
                                WinWindowFromID(hwnd,WALK_DRIVELIST),
                                WinWindowFromID(hwnd,WALK_DIRLIST),
                                wa->szCurrentPath1,FALSE);
              }
            }
          }
          break;

        case WALK2_DRIVELIST:
          if(okay && *szBuffer && SHORT2FROMMP(mp1) == LN_ENTER) {

            ULONG  ulDirLen = CCHMAXPATH;
            APIRET rc;

            rc = DosQCurDir(toupper(*szBuffer) - '@',
                            &szBuff[3],&ulDirLen);
            if(!rc) {
              strcpy(wa->szCurrentPath2,"C:\\");
              *wa->szCurrentPath2 = toupper(*szBuffer);
              WinSetDlgItemText(hwnd,
                                WALK2_PATH,
                                wa->szCurrentPath2);
              FillPathListBox(hwnd,
                              WinWindowFromID(hwnd,WALK2_DRIVELIST),
                              WinWindowFromID(hwnd,WALK2_DIRLIST),
                              wa->szCurrentPath2,FALSE);
            }
          }
          break;

        case WALK2_DIRLIST:
          if(okay && SHORT2FROMMP(mp1) == LN_ENTER) {

            ULONG        ulSearchCount;
            FILEFINDBUF3 findbuf;
            HDIR         hDir;
            APIRET       rc;

            bstrip(szBuffer);
            if(*szBuffer) {
              strcpy(szBuff,wa->szCurrentPath2);
              if(szBuff[strlen(szBuff) - 1] != '\\')
                strcat(szBuff,"\\");
              strcat(szBuff,szBuffer);
              MakeFullName(szBuff);
              DosError(FERR_DISABLEHARDERR);
              hDir = HDIR_CREATE;
              ulSearchCount = 1L;
              if(!IsRoot(szBuff)) {
                rc = DosFindFirst(szBuff,
                                  &hDir,
                                  FILE_DIRECTORY |
                                  MUST_HAVE_DIRECTORY | FILE_READONLY |
                                  FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
                                  &findbuf,
                                  sizeof(FILEFINDBUF3),
                                  &ulSearchCount,
                                  FIL_STANDARD);
                if(!rc)
                  DosFindClose(hDir);
              }
              else {
                findbuf.attrFile = FILE_DIRECTORY;
                rc = 0;
              }
              if(!rc && (findbuf.attrFile & FILE_DIRECTORY)) {
                strcpy(wa->szCurrentPath2,szBuff);
                WinSetDlgItemText(hwnd,WALK2_PATH,wa->szCurrentPath2);
                FillPathListBox(hwnd,
                                WinWindowFromID(hwnd,WALK2_DRIVELIST),
                                WinWindowFromID(hwnd,WALK2_DIRLIST),
                                wa->szCurrentPath2,FALSE);
              }
            }
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      wa = WinQueryWindowPtr(hwnd,0);
      if(!wa)
        WinDismissDlg(hwnd,0);
      *szBuff = 0;
      WinQueryDlgItemText(hwnd,
                          WALK_PATH,
                          CCHMAXPATH,
                          szBuff);
      bstrip(szBuff);
      while((p = strchr(szBuff,'/')) != NULL)
        *p = '\\';
      while(strlen(szBuff) > 3 &&
            szBuff[strlen(szBuff) - 1] == '\\')
        szBuff[strlen(szBuff) - 1] = 0;
      MakeFullName(szBuff);
      if(*szBuff &&
         stricmp(szBuff,wa->szCurrentPath1)) {
        if(!SetDir(WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),
                                  QW_OWNER),
                   hwnd,
                   szBuff,
                   0))
          strcpy(wa->szCurrentPath1,szBuff);
        else if(SHORT1FROMMP(mp1) != DID_CANCEL)
          return 0;
      }
      WinSetDlgItemText(hwnd,
                        WALK_PATH,
                        wa->szCurrentPath1);
      *szBuff = 0;
      WinQueryDlgItemText(hwnd,
                          WALK2_PATH,
                          CCHMAXPATH,
                          szBuff);
      bstrip(szBuff);
      while((p = strchr(szBuff,'/')) != NULL)
        *p = '\\';
      while(strlen(szBuff) > 3 &&
            szBuff[strlen(szBuff) - 1] == '\\')
        szBuff[strlen(szBuff) - 1] = 0;
      MakeFullName(szBuff);
      if(*szBuff &&
         stricmp(szBuff,wa->szCurrentPath2)) {
        if(!SetDir(WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),
                                  QW_OWNER),
                   hwnd,
                   szBuff,
                   0))
          strcpy(wa->szCurrentPath2,szBuff);
        else if(SHORT1FROMMP(mp1) != DID_CANCEL)
          return 0;
      }
      WinSetDlgItemText(hwnd,
                        WALK2_PATH,
                        wa->szCurrentPath2);
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          WinDismissDlg(hwnd,1);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_WALKEM2,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;

    case WM_CLOSE:
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY WalkTwoCmpDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,
                                    MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      WinSetWindowText(hwnd,
                       GetPString(IDS_WALKCOMPAREDLGTEXT));
      return WalkTwoDlgProc(hwnd,
                            UM_SETUP2,
                            mp1,
                            mp2);
  }
  return WalkTwoDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY WalkTwoSetDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,
                                    MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      WinSetWindowText(hwnd,
                       GetPString(IDS_WALKSETDIRSDLGTEXT));
      return WalkTwoDlgProc(hwnd,
                            UM_SETUP2,
                            mp1,
                            mp2);
  }
  return WalkTwoDlgProc(hwnd,msg,mp1,mp2);
}

