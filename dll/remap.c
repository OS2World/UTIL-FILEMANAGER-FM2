
/***********************************************************************

  $Id$

  Copyright (c) 1993, 1998 M. Kimes
  Copyright (c) 2004, 2006 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  06 Aug 05 SHL Renames
  22 Jul 06 SHL Check more run time errors
  29 Jul 06 SHL Use xfgets
  31 Aug 06 SHL Use _fsopen to avoid noise complaints

***********************************************************************/

#define INCL_WIN
#define INCL_DOS
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

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(FMREMAP,RemapDlgProc,load_resources,save_resources)
#pragma alloc_text(FMREMAP,add_resource,remove_resource,free_resources)

typedef struct APPNOTIFY {
  HAPP              happ;
  BOOL              attach;
  BOOL              failedonce;
  CHAR              uncname[CCHMAXPATH];
  CHAR              device;
  struct APPNOTIFY *next;
  struct APPNOTIFY *prev;
} APPNOTIFY;

typedef struct LINKRES {
  CHAR           *res;
  struct LINKRES *next;
} LINKRES;

static LINKRES *reshead    = NULL;
static BOOL     loadedres = FALSE;

#define MAXNUMRES 200


VOID load_resources (VOID)
{
  /* load linked list of resources from RESOURCE.DAT file */

  FILE      *fp;
  LINKRES   *info,*last = NULL;
  CHAR       s[CCHMAXPATH + 14];
  INT        x = 0;

  loadedres = TRUE;
  save_dir2(s);
  if(s[strlen(s) - 1] != '\\')
    strcat(s,"\\");
  strcat(s,"RESOURCE.DAT");
  fp = _fsopen(s,"r",SH_DENYWR);
  if (fp) {
    while (x < MAXNUMRES && !feof(fp)) {
      if (!xfgets_bstripcr(s,sizeof(s),fp,pszSrcFile,__LINE__))
        break;
      if (*s && *s != ';') {
        info = xmalloc(sizeof(LINKRES),pszSrcFile,__LINE__);
        if (info) {
          info->res = xstrdup(s,pszSrcFile,__LINE__);
          if (!info->res)
            free(info);
	  else {
            x++;
            info->next = NULL;
            if(!reshead)
              reshead = info;
            else
              last->next = info;
            last = info;
          }
        }
      }
    }
    fclose(fp);
  }
}


VOID save_resources (VOID)
{
  /* save linked list of resources to RESOURCE.DAT file */

  LINKRES *info;
  FILE    *fp;
  CHAR     s[CCHMAXPATH + 14];

  if(!loadedres)
    return;
  save_dir2(s);
  if(s[strlen(s) - 1] != '\\')
    strcat(s,"\\");
  strcat(s,"RESOURCE.DAT");
  if(reshead) {
    fp = xfopen(s,"w",pszSrcFile,__LINE__);
    if (fp) {
      fputs(GetPString(IDS_REMOTEFILETEXT),fp);
      info = reshead;
      while (info) {
        fprintf(fp,
                "%0.*s\n",
                CCHMAXPATH,
                info->res);
        info = info->next;
      }
      fclose(fp);
    }
  }
  else
    unlink(s);
}


BOOL add_resource (CHAR *res)
{
  LINKRES *info,*last = NULL;
  INT      x = 0;

  if(!res || !*res)
    return FALSE;
  if(!loadedres)
    load_resources();
  info = reshead;
  while(info) {
    if(!stricmp(info->res,res))
      return FALSE;
    last = info;
    info = info->next;
    x++;
  }
  info = xmalloc(sizeof(LINKRES),pszSrcFile,__LINE__);
  if(info) {
    info->res = xstrdup(res,pszSrcFile,__LINE__);
    if (!info->res)
      free(info);
    else {
      info->next = NULL;
      if(!reshead)
        reshead = info;
      else
        last->next = info;
      if(x > MAXNUMRES) {
        info = reshead;
        reshead = reshead->next;
        free(info);
      }
      return TRUE;
    }
  }
  return FALSE;
}


BOOL remove_resource (CHAR *res)
{
  LINKRES *info,*last = NULL;

  if(!res || !*res)
    return FALSE;
  if(!loadedres)
    load_resources();
  info = reshead;
  while(info) {
    if(!stricmp(info->res,res)) {
      if(last)
        last->next = info->next;
      else
        reshead = info->next;
      free(info->res);
      free(info);
      return TRUE;
    }
    last = info;
    info = info->next;
  }
  return FALSE;
}


VOID free_resources (VOID)
{
  LINKRES *info,*next;

  info = reshead;
  while(info) {
    next = info->next;
    free(info->res);
    free(info);
    info = next;
  }
  reshead = NULL;
  DosPostEventSem(CompactSem);
}


MRESULT EXPENTRY RemapDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  static BOOL fRemapped;
  static APPNOTIFY *apphead = NULL,*apptail = NULL;

  switch(msg) {
    case WM_INITDLG:
      WinSendDlgItemMsg(hwnd,
                        MAP_ATTACHTO,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(CCHMAXPATH,0),
                        MPVOID);
      fRemapped = FALSE;
      if(!loadedres)
        load_resources();
      {
        LINKRES *info;

        info = reshead;
        while(info) {
          WinSendDlgItemMsg(hwnd,
                            MAP_ATTACHTO,
                            LM_INSERTITEM,
                            MPFROM2SHORT(LIT_END,0),
                            MPFROMP(info->res));
          info = info->next;
        }
      }
      {
        ULONG ulDriveMap,ulDriveNum,x,ulType;
        CHAR  s[3] = " :";

        DosError(FERR_DISABLEHARDERR);
        if(!DosQCurDisk(&ulDriveNum,&ulDriveMap)) {
          for(x = 0;x < 26;x++) {
            if(!(driveflags[x] & DRIVE_IGNORE)) {
              *s = (CHAR)x + 'A';
              if(!(ulDriveMap & (1L << x)))
                WinSendDlgItemMsg(hwnd,
                                  MAP_ATTACHLIST,
                                  LM_INSERTITEM,
                                  MPFROM2SHORT(LIT_END,0),
                                  MPFROMP(s));
              else {
                CheckDrive((CHAR)x + 'A',NULL,&ulType);
                if(ulType & DRIVE_REMOTE)
                  WinSendDlgItemMsg(hwnd,
                                    MAP_DETACHLIST,
                                    LM_INSERTITEM,
                                    MPFROM2SHORT(LIT_END,0),
                                    MPFROMP(s));
              }
            }
          }
        }
        else
          WinDismissDlg(hwnd,0);
      }
      break;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case MAP_ATTACHLIST:
          switch(SHORT2FROMMP(mp1)) {
            case LN_ENTER:
              PostMsg(hwnd,
                      WM_COMMAND,
                      MPFROM2SHORT(MAP_ATTACH,0),
                      MPVOID);
              break;
          }
          break;
        case MAP_DETACHLIST:
          switch(SHORT2FROMMP(mp1)) {
            case LN_ENTER:
              PostMsg(hwnd,
                      WM_COMMAND,
                      MPFROM2SHORT(MAP_DETACH,0),
                      MPVOID);
              break;
            case LN_SELECT:
              {
                SHORT x;
                CHAR  d[3];

                WinSetDlgItemText(hwnd,
                                  MAP_ATTACHTO,
                                  NullStr);
                x = (SHORT)WinSendDlgItemMsg(hwnd,
                                             MAP_DETACHLIST,
                                             LM_QUERYSELECTION,
                                             MPFROMSHORT(LIT_FIRST),
                                             MPVOID);
                if(x >= 0) {
                  *d = 0;
                  WinSendDlgItemMsg(hwnd,
                                    MAP_DETACHLIST,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(x,sizeof(d)),
                                    MPFROMP(d));
                  if(*d) {

                    CHAR        buf[2048];
                    ULONG       len;
                    APIRET      rc;
                    FSQBUFFER2 *p2;

                    memset(buf,0,sizeof(buf));
                    len = sizeof(buf);
                    p2 = (FSQBUFFER2 *)buf;
                    DosError(FERR_DISABLEHARDERR);
                    rc = DosQueryFSAttach(d,
                                          0,
                                          FSAIL_QUERYNAME,
                                          p2,
                                          &len);
                    if(!rc) {

                      CHAR *p;

                      p  = (char *)p2->szName;
                      p += p2->cbName + 1;
                      p += p2->cbFSDName + 1;
                      if(p2->cbFSAData)
                        WinSetDlgItemText(hwnd,
                                          MAP_ATTACHTO,
                                          p);
                      else
                        WinSetDlgItemText(hwnd,
                                          MAP_ATTACHTO,
                                          GetPString(IDS_UNKNOWNBRKTTEXT));
                    }
                    else
                      WinSetDlgItemText(hwnd,
                                        MAP_ATTACHTO,
                                        GetPString(IDS_UNKNOWNBRKTTEXT));
                  }
                }
              }
              break;
          }
          break;
      }
      break;

    case WM_APPTERMINATENOTIFY:
      {
        APPNOTIFY *info;
        SHORT      x,c;
        CHAR       d[3];
        HWND       hwndList;

        if(!mp2)
          fRemapped = TRUE;
        info = apphead;
GetRidOfIt:
        while(info) {
          if(info->happ == (HAPP)mp1) {
/* Note:  if this next line is removed, FM/2 will start the attach/detach
 * request again, once for each request, to see if it might succeed and to
 * ensure the request is seen by the user in case interaction is required.
 */
info->failedonce = TRUE;
            hwndList = WinWindowFromID(hwnd,
                                       (info->attach) ?
                                        MAP_ATTACHLIST :
                                        MAP_DETACHLIST);
            if(!mp2 || (ULONG)mp2 == 1041 || info->failedonce) {
              if(info->prev)
                info->prev->next = info->next;
              if(info->next)
                info->next->prev = info->prev;
              if(apphead == info)
                apphead = info->next;
              if(apptail == info)
                apptail = info->prev;
            }
            if(!mp2) {
              if(*info->uncname &&
                 stricmp(info->uncname,GetPString(IDS_UNKNOWNBRKTTEXT)) &&
                 add_resource(info->uncname)) {
                save_resources();
                WinSendDlgItemMsg(hwnd,
                                  MAP_ATTACHTO,
                                  LM_INSERTITEM,
                                  MPFROM2SHORT(LIT_END,0),
                                  MPFROMP(info->uncname));
              }
              c = (SHORT)WinSendMsg(hwndList,
                                    LM_QUERYITEMCOUNT,
                                    MPVOID,
                                    MPVOID);
              if(c > 0) {
                for(x = 0;x < c;x++) {
                  *d = 0;
                  WinSendMsg(hwndList,
                             LM_QUERYITEMTEXT,
                             MPFROM2SHORT(x,sizeof(d)),
                             MPFROMP(d));
                  if(*d == info->device) {
                    WinSendMsg(hwndList,
                               LM_DELETEITEM,
                               MPFROMSHORT(x),
                               MPVOID);
                    hwndList = WinWindowFromID(hwnd,
                                               (info->attach) ?
                                                MAP_DETACHLIST :
                                                MAP_ATTACHLIST);
                    d[1] = ':';
                    d[2] = 0;
                    WinSendMsg(hwndList,
                               LM_INSERTITEM,
                               MPFROM2SHORT(LIT_SORTASCENDING,0),
                               MPFROMP(d));
                    break;
                  }
                }
              }
            }
            else if((ULONG)mp2 != 1041 && !info->failedonce) {

              PROGDETAILS pgd;
              CHAR        params[368],*p;
              HAPP        happ;

              *d = info->device;
              d[1] = ':';
              d[2] = 0;
              p = GetCmdSpec(FALSE);
              memset(&pgd,0,sizeof(pgd));
              pgd.Length = sizeof(pgd);
              pgd.progt.progc = PROG_WINDOWABLEVIO;
              pgd.progt.fbVisible = SHE_VISIBLE;
              pgd.pszTitle = (info->attach) ? GetPString(IDS_ATTACHREQTEXT) :
                                              GetPString(IDS_DETACHREQTEXT);
              pgd.pszExecutable = p;
              pgd.pszParameters = params;
              pgd.pszStartupDir = NULL;
              pgd.pszIcon = NULL;
              pgd.pszEnvironment = NULL;
              pgd.swpInitial.hwndInsertBehind = HWND_TOP;
              pgd.swpInitial.hwnd = hwnd;
              pgd.swpInitial.fl = SWP_SHOW | SWP_ACTIVATE;
              if(info->attach)
                sprintf(params,"/C NET USE %s \"%s\"",d,info->uncname);
              else
                sprintf(params,"/C NET USE %s /D",d);
              info->failedonce = TRUE;
              happ = WinStartApp(hwnd,&pgd,pgd.pszParameters,
                                 NULL,SAF_MAXIMIZED);
              if(!happ)
                goto GetRidOfIt;
              info->happ = happ;
              break;
            }
            else if((ULONG)mp2 == 1041)
              saymsg(MB_CANCEL | MB_ICONEXCLAMATION,hwnd,
                     GetPString(IDS_ERRORTEXT),
                     "%s",
                     GetPString(IDS_CANTSTARTNETUSETEXT));
            if(!mp2 || (ULONG)mp2 == 1041 || info->failedonce)
              free(info);
            break;
          }
          info = info->next;
        }
      }
      break;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case MAP_DELETE:
          {
            SHORT x;
            CHAR  resource[CCHMAXPATH];

            x = (SHORT)WinSendDlgItemMsg(hwnd,
                                         MAP_ATTACHTO,
                                         LM_QUERYSELECTION,
                                         MPFROMSHORT(LIT_FIRST),
                                         MPVOID);
            if(x >= 0) {
              *resource = 0;
              WinSendDlgItemMsg(hwnd,
                                MAP_ATTACHTO,
                                LM_QUERYITEMTEXT,
                                MPFROM2SHORT(x,sizeof(resource)),
                                MPFROMP(resource));
              bstrip(resource);
              if(*resource) {
                if(remove_resource(resource)) {
                  save_resources();
                  WinSendDlgItemMsg(hwnd,
                                    MAP_ATTACHTO,
                                    LM_DELETEITEM,
                                    MPFROMSHORT(x),
                                    MPVOID);
                  if(x)
                    x--;
                  WinSendDlgItemMsg(hwnd,
                                    MAP_ATTACHTO,
                                    LM_SELECTITEM,
                                    MPFROMSHORT(x),
                                    MPFROMSHORT(TRUE));
                  if(!(SHORT)WinSendDlgItemMsg(hwnd,
                                               MAP_ATTACHTO,
                                               LM_QUERYITEMCOUNT,
                                               MPVOID,
                                               MPVOID))
                    WinSetDlgItemText(hwnd,
                                      MAP_ATTACHTO,
                                      NullStr);
                }
              }
            }
          }
          break;

        case MAP_CLEAR:
          free_resources();
          save_resources();
          WinSendDlgItemMsg(hwnd,
                            MAP_ATTACHTO,
                            LM_DELETEALL,
                            MPVOID,
                            MPVOID);
          WinSetDlgItemText(hwnd,
                            MAP_ATTACHTO,
                            NullStr);
          break;

        case MAP_INFO:
        case MAP_DETACH:
          {
            CHAR   d[3],s[CCHMAXPATH];
            SHORT  x;

            *s = 0;
            WinQueryDlgItemText(hwnd,
                                MAP_ATTACHTO,
                                sizeof(s),
                                s);
            bstrip(s);
            x = (SHORT)WinSendDlgItemMsg(hwnd,
                                         MAP_DETACHLIST,
                                         LM_QUERYSELECTION,
                                         MPFROMSHORT(LIT_FIRST),
                                         MPVOID);
            if(x >= 0) {
              *d = 0;
              WinSendDlgItemMsg(hwnd,
                                MAP_DETACHLIST,
                                LM_QUERYITEMTEXT,
                                MPFROM2SHORT(x,sizeof(d)),
                                MPFROMP(d));
              if(*d) {
                switch(SHORT1FROMMP(mp1)) {
                  case MAP_DETACH:
                    {
                      PROGDETAILS pgd;
                      CHAR        params[368],*p;
                      HAPP        happ;

                      p = GetCmdSpec(FALSE);
                      memset(&pgd,0,sizeof(pgd));
                      pgd.Length = sizeof(pgd);
                      pgd.progt.progc = PROG_WINDOWABLEVIO;
                      pgd.progt.fbVisible = SHE_VISIBLE;
                      pgd.pszTitle = GetPString(IDS_DETACHREQTEXT);
                      pgd.pszExecutable = p;
                      pgd.pszParameters = params;
                      pgd.pszStartupDir = NULL;
                      pgd.pszIcon = NULL;
                      pgd.pszEnvironment = NULL;
                      pgd.swpInitial.hwndInsertBehind = HWND_TOP;
                      pgd.swpInitial.hwnd = hwnd;
                      pgd.swpInitial.fl = SWP_SHOW | SWP_ACTIVATE;
                      sprintf(params,"/C NET USE %s /D",d);
                      happ = WinStartApp(hwnd,
                                         &pgd,
                                         pgd.pszParameters,
                                         NULL,
                                         SAF_MAXIMIZED);
                      if(happ) {

                        APPNOTIFY *info;

                        WinSetDlgItemText(hwnd,
                                          MAP_ATTACHTO,
                                          NullStr);
                        info = xmallocz(sizeof(APPNOTIFY),pszSrcFile,__LINE__);
                        if (info) {
                          info->happ = happ;
                          info->attach = FALSE;
                          info->failedonce = FALSE;
                          strcpy(info->uncname,s);
                          info->device = *d;
                          if(!apphead)
                            apphead = info;
                          else {
                            apptail->next = info;
                            info->prev = apptail;
                          }
                          apptail = info;
                        }
                      }
                      else
                        saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                               hwnd,
                               GetPString(IDS_ERRORTEXT),
                               GetPString(IDS_CANTSTARTTEXT),
                               p,
                               params);
                    }
#ifdef NEVER				// fixme to be gone?
                    DosError(FERR_DISABLEHARDERR);
                    rc = DosFSAttach(d,
                                     s,
                                     d,
                                     strlen(d) + 1,
                                     FS_DETACH);
                    if (rc) {
                      Dos_Error(MB_CANCEL,
                                rc,
                                hwnd,
                                pszSrcFile,
                                __LINE__,
                                GetPString(IDS_DETACHFAILEDTEXT),
                                d,
                                s);
		    }
		    else {
                      fRemapped = TRUE;
                      WinSendDlgItemMsg(hwnd,
                                        MAP_DETACHLIST,
                                        LM_DELETEITEM,
                                        MPFROMSHORT(x),
                                        MPVOID);
                      WinSendDlgItemMsg(hwnd,
                                        MAP_ATTACHLIST,
                                        LM_INSERTITEM,
                                        MPFROM2SHORT(LIT_SORTASCENDING,0),
                                        MPFROMP(d));
                    }
#endif					// fixme to be gone?
                    break;

                  case MAP_INFO:
                    runemf2(SEPARATEKEEP | WINDOWED | MAXIMIZED,
                            hwnd,
                            NULL,
                            NULL,
                            "%s /C NET USE %s",
                            GetCmdSpec(FALSE),
                            d);
                    break;
                }
              }
            }
          }
          break;

        case MAP_ATTACH:
          {
            CHAR   d[3],s[CCHMAXPATH];
            SHORT  x;

            *s = 0;
            WinQueryDlgItemText(hwnd,
                                MAP_ATTACHTO,
                                sizeof(s),
                                s);
            bstrip(s);
            if(*s) {
              x = (SHORT)WinSendDlgItemMsg(hwnd,
                                           MAP_ATTACHLIST,
                                           LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST),
                                           MPVOID);
              if(x >= 0) {
                *d = 0;
                WinSendDlgItemMsg(hwnd,
                                  MAP_ATTACHLIST,
                                  LM_QUERYITEMTEXT,
                                  MPFROM2SHORT(x,sizeof(d)),
                                  MPFROMP(d));
                if(*d) {

                  PROGDETAILS pgd;
                  CHAR        params[368],*p;
                  HAPP        happ;

                  p = GetCmdSpec(FALSE);
                  memset(&pgd,0,sizeof(pgd));
                  pgd.Length = sizeof(pgd);
                  pgd.progt.progc = PROG_WINDOWABLEVIO;
                  pgd.progt.fbVisible = SHE_VISIBLE;
                  pgd.pszTitle = GetPString(IDS_ATTACHREQTEXT);
                  pgd.pszExecutable = p;
                  pgd.pszParameters = params;
                  pgd.pszStartupDir = NULL;
                  pgd.pszIcon = NULL;
                  pgd.pszEnvironment = NULL;
                  pgd.swpInitial.hwndInsertBehind = HWND_TOP;
                  pgd.swpInitial.hwnd = hwnd;
                  pgd.swpInitial.fl = SWP_SHOW | SWP_ACTIVATE;
                  sprintf(params,"/C NET USE %s \"%s\"",d,s);
                  happ = WinStartApp(hwnd,
                                     &pgd,
                                     pgd.pszParameters,
                                     NULL,
                                     SAF_MAXIMIZED);
                  if(happ) {

                    APPNOTIFY *info;

                    info = xmallocz(sizeof(APPNOTIFY),pszSrcFile,__LINE__);
                    if (info) {
                      info->happ = happ;
                      info->attach = TRUE;
                      info->failedonce = FALSE;
                      strcpy(info->uncname,s);
                      info->device = *d;
                      if(!apphead)
                        apphead = info;
                      else {
                        apptail->next = info;
                        info->prev = apptail;
                      }
                      apptail = info;
                    }
                  }
                  else
                    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
                           hwnd,
                           GetPString(IDS_ERRORTEXT),
                           GetPString(IDS_CANTSTARTTEXT),
                           p,
                           params);
#ifdef NEVER				// fixme to be gone?
                  DosError(FERR_DISABLEHARDERR);
                  rc = DosFSAttach(d,s,s,strlen(s) + 1,FS_ATTACH);
                  if (rc) {
                    Dos_Error(MB_CANCEL,
                              rc,
                              hwnd,
                              pszSrcFile,
                              __LINE__,
                              GetPString(IDS_ATTACHFAILEDTEXT),
                              s,
                              d);
		  }
		  else {
                    fRemapped = TRUE;
                    WinSendDlgItemMsg(hwnd,
                                      MAP_ATTACHLIST,
                                      LM_DELETEITEM,
                                      MPFROMSHORT(x),
                                      MPVOID);
                    WinSendDlgItemMsg(hwnd,
                                      MAP_DETACHLIST,
                                      LM_INSERTITEM,
                                      MPFROM2SHORT(LIT_SORTASCENDING,0),
                                      MPFROMP(d));
                  }
#endif					// fixme to be gone?
                }
              }
            }
          }
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_REMAP,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case DID_CANCEL:
          if(fRemapped) {
            if(hwndTree)
              PostMsg(hwndTree,
                      WM_COMMAND,
                      MPFROM2SHORT(IDM_RESCAN,0),
                      MPVOID);
            else
              FillInDriveFlags(NULL);
            if(hwndMain)
              PostMsg(hwndMain,
                      UM_BUILDDRIVEBAR,
                      MPVOID,
                      MPVOID);
          }
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;

    case WM_DESTROY:
      if(apphead) {

        APPNOTIFY *info,*next;

        info = apphead;
        while(info) {
          next = info->next;
          free(info);
          info = next;
        }
        apphead = apptail = NULL;
        saymsg(MB_YESNOCANCEL,
               HWND_DESKTOP,
               GetPString(IDS_NOTICETITLETEXT),
               "%s",
               GetPString(IDS_REMAPNOTICETEXT));
      }
      free_resources();
      loadedres = FALSE;
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

