
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2002 Steven H.Levine

  Archive containers

  Revisions	11 Jun 02 SHL - Ensure archive name not garbage

***********************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <direct.h>
#include <share.h>
#include <limits.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"

#pragma data_seg(DATA1)
#pragma alloc_text(ARCCNRS,ArcCnrWndProc,ArcObjWndProc,ArcClientWndProc)
#pragma alloc_text(ARCCNRS,ArcTextProc,FillArcCnr,ArcFilter)
#pragma alloc_text(ARCCNRS,ArcSort,ArcFrameWndProc,IsArcThere,ArcErrProc)
#pragma alloc_text(STARTUP,StartArcCnr)

INT arcsortFlags = 0;


MRESULT EXPENTRY ArcErrProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  ARCDUMP *ad;

  switch(msg) {
    case WM_INITDLG:
      if(!mp2)
        WinDismissDlg(hwnd,0);
      else {
        ad = (ARCDUMP *)mp2;
        WinSetWindowPtr(hwnd,0,ad);
        if(ad->errmsg)
          WinSetDlgItemText(hwnd,
                            ARCERR_TEXT,
                            ad->errmsg);
        if(!ad->info->test)
          WinEnableWindow(WinWindowFromID(hwnd,ARCERR_TEST),FALSE);
        if(ad->listname) {
          MLEsetlimit(WinWindowFromID(hwnd,ARCERR_MLE),-1L);
          MLEsetformat(WinWindowFromID(hwnd,ARCERR_MLE),MLFIE_NOTRANS);
          MLEsetcurpos(WinWindowFromID(hwnd,ARCERR_MLE),
                       MLEgetlen(WinWindowFromID(hwnd,ARCERR_MLE)));
          MLEinsert(WinWindowFromID(hwnd,ARCERR_MLE),
                    GetPString(IDS_ARCHIVERREPORTTEXT));
          MLEinsertfile(WinWindowFromID(hwnd,ARCERR_MLE),ad->listname);
        }
      }
      break;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_ARCERR,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case DID_OK:
          ad = WinQueryWindowPtr(hwnd,0);
          WinDlgBox(HWND_DESKTOP,hwnd,ArcReviewDlgProc,FM3ModHandle,
                    AD_FRAME,MPFROMP(ad));
          WinDismissDlg(hwnd,0);
          break;

        case ARCERR_VIEW:
          ad = WinQueryWindowPtr(hwnd,0);
          {
            CHAR *list[2];

            list[0] = ad->arcname;
            list[1] = NULL;
            if(TestBinary(ad->arcname)) {
              if(*binview)
                ExecOnList((HWND)0,binview,WINDOWED | SEPARATE,NULL,list,NULL);
              else
                StartMLEEditor(HWND_DESKTOP,16 + 4 + 1,ad->arcname,hwnd);
            }
            else {
              if(*viewer)
                ExecOnList((HWND)0,viewer,WINDOWED | SEPARATE |
                           ((fViewChild) ? CHILD : 0),
                           NULL,list,NULL);
              else
                StartMLEEditor(HWND_DESKTOP,8 + 4 + 1,ad->arcname,hwnd);
            }
          }
          break;

        case ARCERR_TEST:
          ad = WinQueryWindowPtr(hwnd,0);
          runemf2(SEPARATEKEEP | WINDOWED | MAXIMIZED,
                  hwnd,NULL,NULL,"%s %s%s%s",ad->info->test,
                  ((needs_quoting(ad->arcname)) ? "\"" : NullStr),
                  ad->arcname,
                  ((needs_quoting(ad->arcname)) ? "\"" : NullStr));
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


SHORT APIENTRY ArcSort (PMINIRECORDCORE p1,PMINIRECORDCORE p2,PVOID pStorage) {

  PARCITEM    p = (PARCITEM)p1,pp = (PARCITEM)p2;
  DIRCNRDATA *dcd = (DIRCNRDATA *)pStorage;
  SHORT       ret = 0;
  CHAR       *pext,*ppext;

  if(dcd)
    arcsortFlags = dcd->sortFlags;
  if(arcsortFlags) {
    switch(arcsortFlags & (~SORT_REVERSE)) {
      case SORT_FIRSTEXTENSION:
        pext = strchr(p->szFileName,'.');
        ppext = strchr(pp->szFileName,'.');
        if(!pext)
          pext = NullStr;
        if(!ppext)
          ppext = NullStr;
        ret = stricmp(pext,ppext);
        break;

      case SORT_LASTEXTENSION:
        pext = strrchr(p->szFileName,'.');
        ppext = strrchr(pp->szFileName,'.');
        if(!pext)
          pext = NullStr;
        if(!ppext)
          ppext = NullStr;
        ret = stricmp(pext,ppext);
        break;

      case SORT_LWDATE:
        ret = (p->date.year < pp->date.year) ? 1 :
              (p->date.year > pp->date.year) ? -1 :
              (p->date.month < pp->date.month) ? 1 :
              (p->date.month > pp->date.month) ? -1 :
              (p->date.day < pp->date.day) ? 1 :
              (p->date.day > pp->date.day) ? -1 :
              (p->time.hours < pp->time.hours) ? 1 :
              (p->time.hours > pp->time.hours) ? -1 :
              (p->time.minutes < pp->time.minutes) ? 1 :
              (p->time.minutes > pp->time.minutes) ? -1 :
              (p->time.seconds < pp->time.seconds) ? 1 :
              (p->time.seconds > pp->time.seconds) ? -1 : 0;
        break;

      case SORT_SIZE:
        ret = (p->cbFile < pp->cbFile) ? 1 : (p->cbFile == pp->cbFile) ?
              0 : -1;
        if(!ret)
          ret = (p->cbComp < pp->cbComp) ? 1 : (p->cbComp == pp->cbComp) ?
                0 : -1;
        break;

      case SORT_EASIZE:
        ret = (p->cbComp < pp->cbComp) ? 1 : (p->cbComp == pp->cbComp) ?
              0 : -1;
        if(!ret)
          ret = (p->cbFile < pp->cbFile) ? 1 : (p->cbFile == pp->cbFile) ?
                0 : -1;
        break;
    }
    if(!ret)
      ret = (SHORT)stricmp(p->szFileName,pp->szFileName);
    if(ret && (arcsortFlags & SORT_REVERSE))
      ret = (ret > 0) ? -1 : 1;
    return ret;
  }
  return (SHORT)stricmp(p->szFileName,pp->szFileName);
}


INT APIENTRY ArcFilter (PMINIRECORDCORE rmini,PVOID arg) {

  DIRCNRDATA   *dcd = (DIRCNRDATA *)arg;
  PARCITEM      r;
  register INT  x;
  INT           ret = FALSE;

  if(dcd && *dcd->mask.szMask) {
    r = (PARCITEM)rmini;
    if(dcd->mask.pszMasks[1]) {
      for(x = 0;dcd->mask.pszMasks[x];x++) {
        if(*dcd->mask.pszMasks[x]) {
          if(*dcd->mask.pszMasks[x] != '/') {
            if(wildcard(r->szFileName,dcd->mask.pszMasks[x],FALSE))
              ret = TRUE;
          }
          else {
            if(wildcard(r->szFileName,dcd->mask.pszMasks[x] + 1,FALSE)) {
              ret = FALSE;
              break;
            }
          }
        }
      }
    }
    else {
      if(wildcard(r->szFileName,dcd->mask.szMask,FALSE))
        ret = TRUE;
    }
  }
  else
    ret = TRUE;
  return ret;
}


MRESULT EXPENTRY ArcFrameWndProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  return CommonFrameWndProc(ARC_CNR,hwnd,msg,mp1,mp2);
}


BOOL IsArcThere (HWND hwnd, CHAR *arcname) {

  if(arcname) {
    if(IsFile(arcname) != 1) {
      saymsg(MB_CANCEL,hwnd,
             GetPString(IDS_SAYWHATTEXT),
             GetPString(IDS_ARCNOTTHERETEXT),
             arcname);
      return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}


INT FillArcCnr (HWND hwndCnr,CHAR *arcname,ARC_TYPE **arcinfo,
                ULONG *totalbytes)
{

  FILE         *fp;
  HFILE         oldstdout,newstdout;
  CHAR          s[CCHMAXPATH * 2],lonename[CCHMAXPATH + 2],
               *nsize,*osize,*fdate,*fname,*p,*pp,arctemp[33];
  BOOL          gotstart = FALSE,gotend = FALSE,wasquote,nomove = FALSE;
  INT           highest = 0,x,counter = 0,numarcfiles = 0;
  PARCITEM      lastpai = NULL;
  ARC_TYPE     *info,*tinfo = NULL;
  ULONG         apptype;

  if(!arcname || !arcinfo)
    return 0;
  info = *arcinfo;
  if(!info)
    info = find_type(arcname,arcsighead);
    for(x = 0;x < 99;x++) {
      sprintf(arctemp,"%s.%03x",ArcTempRoot,(clock() & 4095L));
      if(IsFile(arctemp) == 1)
        DosSleep(rand() % 100);
      else
        break;
    }
ReTry:

#ifdef DEBUG
  if (info && info->id)
    WinSetWindowText(WinQueryWindow(WinQueryWindow(hwndCnr,QW_PARENT),QW_PARENT),info->id);
#endif

  tinfo = NULL;
  numarcfiles = counter = highest = 0;
  gotstart = gotend = FALSE;
  lastpai = NULL;
  *totalbytes = 0L;
  if(info && info->list) {
    WinSendMsg(hwndCnr,
               CM_REMOVERECORD,
               MPVOID,
               MPFROM2SHORT(0,
                            CMA_FREE | CMA_INVALIDATE | CMA_ERASE));
    *arcinfo = info;
    highest = info->osizepos;
    if(info->nsizepos > highest)
      highest = info->nsizepos;
    if(info->fdpos > highest)
      highest = info->fdpos;
    if(info->fnpos > highest)
      highest = info->fnpos;
    if(highest > 50)
      saymsg(MB_ENTER | MB_ICONEXCLAMATION,HWND_DESKTOP,
             GetPString(IDS_SHAMETEXT),
             "%s",
             GetPString(IDS_BUNGEDUPTEXT));
    if(info->fnpos == -1)
      highest = 32767;
    DosError(FERR_DISABLEHARDERR);
    DosForceDelete(arctemp);
    DosError(FERR_DISABLEHARDERR);
    strcpy(s,info->list);
    p = strchr(s,' ');
    if(p)
      *p = 0;
    DosError(FERR_DISABLEHARDERR);
    if(!DosQAppType(s,&apptype) &&
       ((apptype & FAPPTYP_DOS) ||
        (apptype & FAPPTYP_WINDOWSREAL) ||
        (apptype & FAPPTYP_WINDOWSPROT) ||
        (apptype & 0x1000))) {
      p = GetCmdSpec(TRUE);
      runemf2(SEPARATE | INVISIBLE | MINIMIZED | BACKGROUND | WAIT,
              hwndCnr,
              NULL,
              "DOS_BACKGROUND_EXECUTION=1",
              "%s /C %s %s%s%s > %s",
              p,
              info->list,
              (needs_quoting(arcname)) ? "\"" : NullStr,
              arcname,
              (needs_quoting(arcname)) ? "\"" : NullStr,
              arctemp);
    }
    else {
      fp = fopen(arctemp,"w");
      if(fp) {
        newstdout = -1;
        DosError(FERR_DISABLEHARDERR);
        if(!DosDupHandle(fileno(stdout),&newstdout)) {
          oldstdout = fileno(stdout);
          DosError(FERR_DISABLEHARDERR);
          if(!DosDupHandle(fileno(fp),&oldstdout)) {
            runemf2(SEPARATE | INVISIBLE | FULLSCREEN | BACKGROUND | WAIT,
                    hwndCnr,NULL,NULL,"%s %s%s%s",info->list,
                    (needs_quoting(arcname)) ? "\"" : NullStr,arcname,
                    (needs_quoting(arcname)) ? "\"" : NullStr);
            oldstdout = fileno(stdout);
            DosError(FERR_DISABLEHARDERR);
            DosDupHandle(newstdout,&oldstdout);
            DosClose(newstdout);
            fclose(fp);
          }
          else {
            DosBeep(50,100);
            return 0;
          }
        }
        else {
          DosBeep(50,100);
          return 0;
        }
      }
      else {
        DosBeep(50,100);
        return 0;
      }
    }
    DosError(FERR_DISABLEHARDERR);
    fp = _fsopen(arctemp,"r",SH_DENYWR);
    if(fp) {
      while(!feof(fp) && !gotend) {
        if(!fgets(s,CCHMAXPATH * 2,fp))
          break;
        s[(CCHMAXPATH * 2) - 1] = 0;
        stripcr(s);
        if(!gotstart) {
          if(!strcmp(s,info->startlist))
          gotstart = TRUE;
        }
        else if(info->endlist &&
                !strcmp(s,info->endlist))
          gotend = TRUE;
        else {    /* add to container */
          fname = NULL;
          lstrip(rstrip(s));
          if(info->nameisfirst) {
            strncpy(lonename,s,CCHMAXPATH + 2);
            lonename[CCHMAXPATH + 1] = 0;
            fname = lonename;
            if(!fgets(s,CCHMAXPATH * 2,fp))
              break;
            s[(CCHMAXPATH * 2) - 1] = 0;
            stripcr(s);
            lstrip(rstrip(s));
            if(*fname == '\"') {
              memmove(fname,fname + 1,strlen(fname) + 1);
              p = strchr(fname,'\"');
              if(p)
                *p = 0;
            }
          }
          nsize = NULL;
          osize = fdate = NullStr;
          p = s;
          for(x = 0;x <= highest;x++) {
            pp = p;
            while(*pp && (*pp == ' ' || *pp == '\t')) /* skip leading */
              pp++;
            if(!*pp)
              break;
            wasquote = FALSE;
            p = pp;
            while(*p && (wasquote ||
                         ((x != info->fnpos || !info->nameislast) ?
                          (*p != ' ' && *p != '\t') : TRUE))) {
              if(*p == '\"') {
                if(!wasquote) {
                  wasquote = TRUE;
                  memmove(p,p + 1,strlen(p));
                  while(*p == ' ' || *p == '\t')
                   p++;
                }
                else {
                  memmove(p,p + 1,strlen(p));
                  break;
                }
              }
              else if(*p)
                p++;
            }
            if(*p) {
              *p = 0;
              p++;
            }
            if(x == info->nsizepos)
              nsize = pp;
            else if(x == info->osizepos)
              osize = pp;
            else if(x == info->fdpos) {
              fdate = pp;
              if(info->fdflds > 1 &&
                 info->fdflds < 24) {

                INT y;

                if(*p) {
                  p--;
                  *p = ' ';
                  for(y = 0;y < info->fdflds - 1;y++) {
                    while(*p && (*p == ' ' || *p == '\t'))
                      p++;
                    while(*p && (*p != ' ' && *p != '\t'))
                      p++;
                    x++;
                  }
                  if(*p) {
                    *p = 0;
                    p++;
                  }
                }
              }
            }
            else if(x == info->fnpos) {
              fname = pp;
              if(pp && *pp == '*' && !*(pp + 1))  /* workaround for LH.EXE */
                fname = NULL;
              if(info->nameislast)
                break;
            }
            else if((!p || !*p) && info->fnpos == -1) {
              fname = pp;
              break;
            }
          }
          if(info->nameisnext) {
            if(!fgets(lonename,CCHMAXPATH + 2,fp))
              break;
            lonename[CCHMAXPATH + 1] = 0;
            stripcr(lonename);
            lstrip(rstrip(lonename));
            fname = lonename;
          }
          if(fname && *fname) {

            RECORDINSERT ri;
            PARCITEM     pai;

#ifdef DEBUG
            saymsg(MB_ENTER,hwndCnr,DEBUG_STRING,
                   "fname: %s\r\rpp: %s\r\rp: %s\r\rlonename: %s\r\rhighest: %ld\r\rx: %ld\r\rfdate: %s",
                   (fname) ? fname : "NULL",
                   (pp) ? pp : "NULL",
                   (p) ? p : "NULL",
                   lonename,
                   highest,
                   x,
                   (fdate) ? fdate : "NULL");
#endif

            pai = WinSendMsg(hwndCnr,
                             CM_ALLOCRECORD,
                             MPFROMLONG(EXTRA_ARCRECORD_BYTES),
                             MPFROMLONG(1L));
            if(pai) {
              memset(pai,0,sizeof(ARCITEM));
              pai->hwndCnr = hwndCnr;
              if(*fname == '*') {
                fname++;
                pai->flags = ARCFLAGS_REALDIR;
              }
              if(fname[strlen(fname) - 1] == '\\' ||
                 fname[strlen(fname) - 1] == '/')
                pai->flags = ARCFLAGS_REALDIR;
              strcpy(pai->szFileName,fname);
              if(fdate)
                strcpy(pai->szDate,fdate);
              pai->pszFileName = pai->szFileName;
              pai->rc.pszIcon = pai->pszFileName;
              pai->rc.hptrIcon = ((pai->flags & ARCFLAGS_REALDIR) != 0) ?
                                  hptrDir :
                                  hptrFile;
              pai->pszDate = pai->szDate;
              if(osize)
                pai->cbFile = atol(osize);
              if(nsize)
                pai->cbComp = atol(nsize);
              if(info->datetype && fdate && *fdate)
                ArcDateTime(fdate,
                            info->datetype,
                            &pai->date,
                            &pai->time);
              memset(&ri,0,sizeof(RECORDINSERT));
              ri.cb                 = sizeof(RECORDINSERT);
              ri.pRecordOrder       = (PRECORDCORE) CMA_END;
              ri.pRecordParent      = (PRECORDCORE)NULL;
              ri.zOrder             = (USHORT)CMA_TOP;
              ri.cRecordsInsert     = 1L;
              ri.fInvalidateRecord  = FALSE;
              if(WinSendMsg(hwndCnr,
                            CM_INSERTRECORD,
                            MPFROMP(pai),
                            MPFROMP(&ri)))
                *totalbytes += pai->cbFile;
              if(!lastpai)
                lastpai = pai;
              numarcfiles++;
              if(!(++counter % 50)) {
                WinSendMsg(hwndCnr,
                           CM_INVALIDATERECORD,
                           lastpai,
                           MPFROM2SHORT(10,
                                        CMA_ERASE | CMA_REPOSITION));
                lastpai = pai;
              }
            }
          }
        }
      }
      fclose(fp);
      if(!numarcfiles || !gotstart || (!gotend && info->endlist)) {

        ARCDUMP ad;

        tinfo = info;
        do {
          tinfo = tinfo->next;
          if(tinfo)
            tinfo = find_type(arcname,tinfo);
          if(tinfo) {
            DosError(FERR_DISABLEHARDERR);
            DosForceDelete(arctemp);
            info = tinfo;
            goto ReTry;
          }
        } while(tinfo);
        DosBeep(750,50);
        {
          CHAR errstr[CCHMAXPATH + 256];

          sprintf(errstr,GetPString(IDS_ARCERRORINFOTEXT),
                  arcname,
                  (!gotstart) ? GetPString(IDS_NOGOTSTARTTEXT) : NullStr,
                  (!numarcfiles) ? GetPString(IDS_NOARCFILESFOUNDTEXT) : NullStr,
                  (!gotend) ? GetPString(IDS_NOENDOFLISTTEXT) : NullStr);
          memset(&ad,0,sizeof(ARCDUMP));
          ad.info = info;
          strcpy(ad.listname,arctemp);
          strcpy(ad.arcname,arcname);
          ad.errmsg = errstr;
          WinDlgBox(HWND_DESKTOP,
                    hwndCnr,
                    ArcErrProc,
                    FM3ModHandle,
                    ARCERR_FRAME,
                    MPFROMP(&ad));
        }
      }
      else {
        if(!nomove && tinfo) {  /* if we got a false hit, move working hit to top */
          tinfo = info->next;
          info->next = arcsighead;
          arcsighead->prev = info;
          if(tinfo)
            tinfo->next->prev = info->prev;
          info->prev->next = tinfo;
          info->prev = NULL;
          arcsighead = info;
          rewrite_archiverbb2(NULL);
        }
      }
    }
    DosError(FERR_DISABLEHARDERR);
    DosForceDelete(arctemp);
  }
  else
    DosBeep(50,100);

  return numarcfiles;
}


MRESULT EXPENTRY ArcTextProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static BOOL   emphasized      = FALSE;
  static HWND   hwndButtonPopup = (HWND)0;
  static ULONG  timestamp       = ULONG_MAX;
  static USHORT lastid          = 0;

  switch(msg) {
    case WM_CREATE:
      return CommonTextProc(hwnd,msg,mp1,mp2);

    case WM_COMMAND:
      return WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                        ARC_CNR),
                        msg,
                        mp1,
                        mp2);

    case UM_CONTEXTMENU:
    case WM_CONTEXTMENU:
      {
        USHORT id;

        id = WinQueryWindowUShort(hwnd,QWS_ID);
        switch(id) {
          case DIR_SELECTED:
          case DIR_VIEW:
          case DIR_SORT:
            {
              POINTL      ptl = {0,0};
              SWP         swp;
              DIRCNRDATA *dcd;

              if(hwndButtonPopup)
                WinDestroyWindow(hwndButtonPopup);
              if(id == DIR_SELECTED)
                id = DIR_RESTORE;
              if(id == lastid) {

                ULONG check;

                DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&check,sizeof(check));
                if(check < timestamp + 500) {
                  lastid = 0;
                  goto MenuAbort;
                }
              }
              hwndButtonPopup = WinLoadMenu(HWND_DESKTOP,
                                            FM3ModHandle,
                                            id);
              if(hwndButtonPopup) {
                WinSetWindowUShort(hwndButtonPopup,
                                   QWS_ID,
                                   id);
                dcd = WinQueryWindowPtr(WinWindowFromID(WinQueryWindow(hwnd,
                                                        QW_PARENT),
                                        ARC_CNR),
                                        0);
                if(id == DIR_SORT) {
                  if(dcd)
                    SetSortChecks(hwndButtonPopup,
                                  dcd->sortFlags);
                  WinSendMsg(hwndButtonPopup,
                             MM_DELETEITEM,
                             MPFROM2SHORT(IDM_SORTNONE,
                                          FALSE),
                             MPVOID);
                  WinSendMsg(hwndButtonPopup,
                             MM_DELETEITEM,
                             MPFROM2SHORT(IDM_SORTNAME,
                                          FALSE),
                             MPVOID);
                  WinSendMsg(hwndButtonPopup,
                             MM_DELETEITEM,
                             MPFROM2SHORT(IDM_SORTLADATE,
                                          FALSE),
                             MPVOID);
                  WinSendMsg(hwndButtonPopup,
                             MM_DELETEITEM,
                             MPFROM2SHORT(IDM_SORTCRDATE,
                                          FALSE),
                             MPVOID);
                  WinSendMsg(hwndButtonPopup,
                             MM_DELETEITEM,
                             MPFROM2SHORT(IDM_SORTDIRSFIRST,
                                          FALSE),
                             MPVOID);
                  WinSendMsg(hwndButtonPopup,
                             MM_DELETEITEM,
                             MPFROM2SHORT(IDM_SORTDIRSLAST,
                                          FALSE),
                             MPVOID);
                  WinSendMsg(hwndButtonPopup,
                             MM_DELETEITEM,
                             MPFROM2SHORT(IDM_SORTSUBJECT,
                                          FALSE),
                             MPVOID);
                  WinSendMsg(hwndButtonPopup,
                             MM_SETITEMTEXT,
                             MPFROM2SHORT(IDM_SORTEASIZE,0),
                             MPFROMP(GetPString(IDS_COMPRESSEDSIZEMENUTEXT)));
                  WinSendMsg(hwndButtonPopup,
                             MM_SETITEMTEXT,
                             MPFROM2SHORT(IDM_SORTLWDATE,0),
                             MPFROMP(GetPString(IDS_DATEMENUTEXT)));
                }
                ptl.x = 0;
                if(WinPopupMenu(HWND_OBJECT,
                                HWND_OBJECT,
                                hwndButtonPopup,
                                -32767,
                                -32767,
                                0,
                                0)) {
                  WinQueryWindowPos(hwndButtonPopup,
                                    &swp);
                  ptl.y = -(swp.cy + 2);
                }
                else {
                  WinQueryWindowPos(hwnd,
                                    &swp);
                  ptl.y = swp.cy + 2;
                }
                if(WinPopupMenu(hwnd,
                                hwnd,
                                hwndButtonPopup,
                                ptl.x,
                                ptl.y,
                                0,
                                PU_HCONSTRAIN | PU_VCONSTRAIN |
                                PU_KEYBOARD   | PU_MOUSEBUTTON1)) {
                  CenterOverWindow(hwndButtonPopup);
                  PaintRecessedWindow(hwnd,
                                      (HPS)0,
                                      FALSE,
                                      FALSE);
                }
              }
            }
            break;
          default:
            PostMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                    ARC_CNR),
                    WM_CONTROL,
                    MPFROM2SHORT(ARC_CNR,
                                 CN_CONTEXTMENU),
                    MPVOID);
            break;
        }
      }
MenuAbort:
      if(msg == UM_CONTEXTMENU)
        return 0;
      break;

    case WM_MENUEND:
      if(hwndButtonPopup == (HWND)mp2) {
        lastid = WinQueryWindowUShort((HWND)mp2,QWS_ID);
        WinDestroyWindow(hwndButtonPopup);
        hwndButtonPopup = (HWND)0;
        DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&timestamp,sizeof(timestamp));
        switch(lastid) {
          case DIR_VIEW:
          case DIR_SORT:
          case DIR_RESTORE:
          case DIR_SELECTED:
            PaintRecessedWindow(hwnd,(HPS)0,TRUE,FALSE);
            break;
        }
      }
      break;

    case WM_MOUSEMOVE:
      {
        USHORT id   = WinQueryWindowUShort(hwnd,QWS_ID);
        char  *s    = NULL;

        if(fOtherHelp) {
          if((!hwndBubble ||
              WinQueryWindowULong(hwndBubble,0) != hwnd) &&
             !WinQueryCapture(HWND_DESKTOP)) {
            switch(id) {
              case DIR_TOTALS:
                s = GetPString(IDS_ARCCNRTOTALSHELPTEXT);
                break;
              case DIR_SELECTED:
                s = GetPString(IDS_ARCCNRSELECTEDHELPTEXT);
                break;
              case DIR_VIEW:
                s = GetPString(IDS_ARCCNRVIEWHELPTEXT);
                break;
              case DIR_SORT:
                s = GetPString(IDS_DIRCNRSORTHELP);
                break;
              case DIR_FILTER:
                s = GetPString(IDS_DIRCNRFILTERHELP);
                break;
              case DIR_FOLDERICON:
                s = GetPString(IDS_ARCCNRFOLDERHELPTEXT);
                break;
              default:
                break;
            }
            if(s)
              MakeBubble(hwnd,TRUE,s);
            else if(hwndBubble)
              WinDestroyWindow(hwndBubble);
          }
        }
        switch(id) {
          case DIR_FILTER:
          case DIR_SORT:
          case DIR_VIEW:
          case DIR_SELECTED:
          case DIR_FOLDERICON:
            return CommonTextButton(hwnd,msg,mp1,mp2);
        }
      }
      break;

    case WM_BUTTON3UP:
    case WM_BUTTON1UP:
    case WM_BUTTON1DOWN:
    case WM_BUTTON3DOWN:
      {
        USHORT id;

        id = WinQueryWindowUShort(hwnd,QWS_ID);
        switch(id) {
          case DIR_FILTER:
          case DIR_SORT:
          case DIR_VIEW:
          case DIR_SELECTED:
          case DIR_FOLDERICON:
            return CommonTextButton(hwnd,msg,mp1,mp2);
        }
      }
      break;

    case UM_CLICKED:
    case UM_CLICKED3:
      {
        USHORT id,cmd = 0;

        id = WinQueryWindowUShort(hwnd,QWS_ID);
        switch(id) {
          case DIR_FOLDERICON:
            switch(msg) {
              case WM_BUTTON3CLICK:
              case WM_CHORD:
                cmd = IDM_RESCAN;
                break;
              default:
                if((SHORT2FROMMP(mp2) & KC_ALT) != 0)
                  cmd = IDM_WINDOWDLG;
                else
                  cmd = IDM_WALKDIR;
                break;
            }
            break;
          case DIR_VIEW:
          case DIR_SORT:
          case DIR_SELECTED:
            PostMsg(hwnd,
                    UM_CONTEXTMENU,
                    MPVOID,
                    MPVOID);
            break;
          case DIR_FILTER:
            cmd = IDM_FILTER;
            break;
          default:
            break;
        }
        if(cmd)
          PostMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                  ARC_CNR),
                  WM_COMMAND,
                  MPFROM2SHORT(cmd,0),
                  MPVOID);
      }
      return 0;

    case WM_BEGINDRAG:
    case DM_DROP:
    case DM_DRAGOVER:
    case DM_DRAGLEAVE:
    case DM_DROPHELP:
      if(msg == DM_DRAGOVER) {
        if(!emphasized) {
          emphasized = TRUE;
          DrawTargetEmphasis(hwnd,emphasized);
        }
      }
      else if(msg != WM_BEGINDRAG) {
        if(emphasized) {
          emphasized = FALSE;
          DrawTargetEmphasis(hwnd,emphasized);
        }
      }
      switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
        case DIR_FOLDERICON:
          switch(msg) {
            case DM_DRAGOVER:
              if(AcceptOneDrop(mp1,mp2))
                return MRFROM2SHORT(DOR_DROP,
                                    DO_MOVE);
              return MRFROM2SHORT(DOR_NODROP,0);         /* Drop not valid        */
            case DM_DROPHELP:
              DropHelp(mp1,
                       mp2,
                       hwnd,
                       GetPString(IDS_ARCCNRFOLDERDROPHELPTEXT));
              return 0;
            case DM_DROP:
              {
                char szFrom[CCHMAXPATH + 2];

                if(emphasized) {
                  emphasized = FALSE;
                  DrawTargetEmphasis(hwnd,emphasized);
                }
                if(GetOneDrop(mp1,mp2,szFrom,sizeof(szFrom)))
                  WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                             ARC_CNR),
                             WM_COMMAND,
                             MPFROM2SHORT(IDM_SWITCH,0),
                             MPFROMP(szFrom));
              }
              return 0;
            default:
              return PFNWPStatic(hwnd,msg,mp1,mp2);
          }
        default:
          {
            CNRDRAGINFO cnd;
            USHORT      dcmd;

            switch(msg) {
              case DM_DROP:
                dcmd = CN_DROP;
                break;
              case DM_DRAGOVER:
                dcmd = CN_DRAGOVER;
                break;
              case DM_DRAGLEAVE:
                dcmd = CN_DRAGLEAVE;
                break;
              case DM_DROPHELP:
                dcmd = CN_DROPHELP;
                break;
              case WM_BEGINDRAG:
                dcmd = CN_INITDRAG;
                break;
            }
            cnd.pDragInfo = (PDRAGINFO)mp1;
            cnd.pRecord = NULL;
            return WinSendMsg(WinQueryWindow(hwnd,QW_PARENT),
                              WM_CONTROL,
                              MPFROM2SHORT(ARC_CNR,dcmd),
                              MPFROMP(&cnd));
          }
      }
  }
  return PFNWPStatic(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ArcClientWndProc (HWND hwnd,ULONG msg,MPARAM mp1,
                                   MPARAM mp2) {

  switch(msg) {
    case UM_CONTAINERHWND:
      return MRFROMLONG(WinWindowFromID(hwnd,ARC_CNR));

    case UM_VIEWSMENU:
      return MRFROMLONG(CheckMenu(&ArcCnrMenu,ARCCNR_POPUP));

    case UM_FILESMENU:
      return MRFROMLONG(CheckMenu(&ArcMenu,ARC_POPUP));

    case MM_PORTHOLEINIT:
    case WM_INITMENU:
    case UM_INITMENU:
    case UM_COMMAND:
    case UM_LOADFILE:
    case UM_UPDATERECORD:
    case UM_UPDATERECORDLIST:
    case WM_COMMAND:
    case WM_CONTROL:
    case WM_CLOSE:
      return WinSendMsg(WinWindowFromID(hwnd,ARC_CNR),msg,mp1,mp2);

    case WM_PSETFOCUS:
    case WM_SETFOCUS:
      if(mp2)
        PostMsg(hwnd,UM_FOCUSME,MPVOID,MPVOID);
      break;

    case UM_FOCUSME:
      WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,ARC_CNR));
      break;

    case WM_PAINT:
      {
        HPS    hps;
        RECTL  rcl;

        hps = WinBeginPaint(hwnd,(HPS)0,NULL);
        if(hps) {
          WinQueryWindowRect(hwnd,&rcl);
          WinFillRect(hps,&rcl,CLR_PALEGRAY);
          CommonTextPaint(hwnd,hps);
          WinEndPaint(hps);
        }
      }
      break;

    case UM_SIZE:
    case WM_SIZE:
      if(msg == UM_SIZE) {

        SWP     swp;

        WinQueryWindowPos(hwnd,&swp);
        mp1 = MPFROM2SHORT(swp.cx,swp.cy);
        mp2 = MPFROM2SHORT(swp.cx,swp.cy);
      }
      {
        USHORT  cx,cy,bx;

        cx = SHORT1FROMMP(mp2);
        cy = SHORT2FROMMP(mp2);
        WinSetWindowPos(WinWindowFromID(hwnd,ARC_CNR),HWND_TOP,
                        0,
                        22,
                        cx,
                        cy - (24 + 22),
                        SWP_SHOW | SWP_MOVE | SWP_SIZE);
        WinSetWindowPos(WinWindowFromID(hwnd,ARC_EXTRACTDIR),HWND_TOP,
                        0,
                        0,
                        cx,
                        22,
                        SWP_SHOW | SWP_MOVE | SWP_SIZE);
        WinSetWindowPos(WinWindowFromID(hwnd,DIR_FOLDERICON),HWND_TOP,
                        2,
                        cy - 22,
                        24,
                        20,
                        SWP_SHOW | SWP_MOVE | SWP_SIZE);
        WinSetWindowPos(WinWindowFromID(hwnd,DIR_TOTALS),HWND_TOP,
                        29,
                        cy - 22,
                        (cx / 3) - 2,
                        20,
                        SWP_SHOW | SWP_MOVE | SWP_SIZE);
        WinSetWindowPos(WinWindowFromID(hwnd,DIR_SELECTED),HWND_TOP,
                        29 + (cx / 3) + 2,
                        cy - 22,
                        (cx / 3) - 2,
                        20,
                        SWP_SHOW | SWP_MOVE | SWP_SIZE);
        bx = (cx - (29 + (((cx / 3) + 2) * 2))) / 3;
        WinSetWindowPos(WinWindowFromID(hwnd,DIR_VIEW),HWND_TOP,
                        29 + (((cx / 3) + 2) * 2),
                        cy - 22,
                        bx - 4,
                        20,
                        SWP_SHOW | SWP_MOVE | SWP_SIZE);
        WinSetWindowPos(WinWindowFromID(hwnd,DIR_SORT),HWND_TOP,
                        29 + (((cx / 3) + 2) * 2) + bx,
                        cy - 22,
                        bx - 4,
                        20,
                        SWP_SHOW | SWP_MOVE | SWP_SIZE);
        WinSetWindowPos(WinWindowFromID(hwnd,DIR_FILTER),HWND_TOP,
                        29 + (((cx / 3) + 2) * 2) + (bx * 2),
                        cy - 22,
                        bx - 4,
                        20,
                        SWP_SHOW | SWP_MOVE | SWP_SIZE);
      }
      CommonTextPaint(hwnd,(HPS)0);
      if(msg == UM_SIZE) {
        WinSetWindowPos(WinQueryWindow(hwnd,QW_PARENT),HWND_TOP,0,0,0,0,
                        SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE);
        return 0;
      }
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ArcObjWndProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  DIRCNRDATA *dcd;

  switch(msg) {
    case WM_CREATE:
      break;

    case DM_PRINTOBJECT:
    case DM_DISCARDOBJECT:
      dcd = INSTDATA(hwnd);
      if(dcd) {

        LISTINFO    *li;
        CNRDRAGINFO cni;

        cni.pRecord = NULL;
        cni.pDragInfo = (PDRAGINFO)mp1;
        li = DoFileDrop(dcd->hwndCnr,
                        dcd->directory,
                        FALSE,
                        MPVOID,
                        MPFROMP(&cni));
        if(li) {
          li->type = (msg == DM_DISCARDOBJECT) ?
                      IDM_DELETE :
                      IDM_PRINT;
          if(!li->list ||
             !li->list[0] ||
             !PostMsg(hwnd,
                      UM_ACTION,
                      MPFROMP(li),
                      MPVOID))
            FreeListInfo(li);
          else
            return MRFROMLONG(DRR_SOURCE);
        }
      }
      return MRFROMLONG(DRR_TARGET);

    case DM_RENDERPREPARE:
      return (MRESULT)TRUE;

    case DM_RENDER:
      dcd = WinQueryWindowPtr(hwnd,0);
      if(dcd && dcd->info && dcd->info->extract && dcd->arcname) {

        PDRAGTRANSFER  pdt = (PDRAGTRANSFER)mp1;
        CHAR           filename[CCHMAXPATH];
        ULONG          len;

        if(pdt->hwndClient && pdt->pditem && pdt->hstrSelectedRMF &&
           pdt->hstrRenderToName) {

          if(pdt->usOperation != DO_COPY && pdt->usOperation != DO_MOVE) {
            pdt->fsReply = DMFL_RENDERRETRY;
            return (MRESULT)FALSE;
          }
          *filename = 0;
          len = DrgQueryStrName(pdt->hstrSelectedRMF,
                                CCHMAXPATH,filename);
          filename[len] = 0;
          if(!strnicmp(filename,"OS2FILE,",8)) {
// saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"RMF = \"%s\"",filename);
            pdt->fsReply = DMFL_RENDERRETRY;
            return (MRESULT)FALSE;
          }
          *filename = 0;
          len = DrgQueryStrName(pdt->hstrRenderToName,
                                CCHMAXPATH,filename);
          filename[len] = 0;
          if(len && *filename) {
            PostMsg(hwnd,UM_RENDER,MPFROMP(pdt),MPFROMP(strdup(filename)));
            return (MRESULT)TRUE;
          }
          else {
// saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"No render-to name given.");
            pdt->fsReply = DMFL_RENDERRETRY;
            return (MRESULT)FALSE;
          }
        }
      }
      return(MRESULT)FALSE;

    case UM_RENDER:
      {
        PDRAGTRANSFER  pdt = (PDRAGTRANSFER)mp1;
        USHORT         usRes = DMFL_RENDERFAIL;

        dcd = WinQueryWindowPtr(hwnd,0);
        if(dcd && dcd->info && dcd->info->extract && dcd->arcname) {

          CHAR          *filename = (CHAR *)mp2,*p;
          ULONG          len;
          CHAR           membername[CCHMAXPATH],construct[CCHMAXPATH * 2];

          *membername = 0;
          len = DrgQueryStrName(pdt->pditem->hstrSourceName,
                                CCHMAXPATH,membername);
          membername[len] = 0;
          if(*membername && len && filename) {
            unlinkf("%s",filename);
            strcpy(construct,filename);
            p = strrchr(filename,'\\');
            if(!p)
              *construct = 0;
            else {
              if(p == filename || *(p - 1) == ':')
                p++;
              *p = 0;
            }
// saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"%s %s %s\r[%s]",dcd->info->extract,dcd->arcname,membername,construct);
            runemf2(SEPARATE | WINDOWED | WAIT |
                    ((fArcStuffVisible) ? 0 : (BACKGROUND | MINIMIZED)) |
                    WAIT,dcd->hwndClient,construct,NULL,"%s %s%s%s %s%s%s",
                    dcd->info->extract,
                    (needs_quoting(dcd->arcname)) ? "\"" : NullStr,
                    dcd->arcname,
                    (needs_quoting(dcd->arcname)) ? "\"" : NullStr,
                    (needs_quoting(membername)) ? "\"" : NullStr,
                    membername,
                    (needs_quoting(membername)) ? "\"" : NullStr);
            if(*construct && construct[strlen(construct) - 1] != '\\')
              strcat(construct,"\\");
            strcat(construct,membername);
            if(IsFile(construct) != -1) {
              rename(construct,filename);
              unlinkf("%s",construct);
              if(IsFile(filename) != -1)
                usRes = DMFL_RENDEROK;
            }
          }
        }
        if(mp2)
          free((CHAR *)mp2);
        PostMsg(pdt->hwndClient,DM_RENDERCOMPLETE,MPFROMP(pdt),
                   MPFROM2SHORT(usRes,0));
      }
      return 0;

    case UM_SETUP:
      dcd = WinQueryWindowPtr(hwnd,0);
      if(dcd) {
        /* set unique id */
        WinSetWindowUShort(hwnd,QWS_ID,ARCOBJ_FRAME + (ARC_FRAME - dcd->id));
        dcd->hwndObject = hwnd;
        if(ParentIsDesktop(hwnd,dcd->hwndParent))
          DosSleep(250L);
      }
      else
        PostMsg(hwnd,WM_CLOSE,MPVOID,MPVOID);
      return 0;

    case UM_RESCAN:
      /*
       * populate container
       */
      dcd = WinQueryWindowPtr(hwnd,0);
      if(dcd) {
        char *s;			// SHL
        if(mp1)
	{
          strcpy(dcd->arcname,(CHAR *)mp1);
	  s = dcd->arcname;			// SHL
	}
	else
	  s = "** Unknown **";			// SHL
        WinSetWindowText(dcd->hwndFrame,"AV/2");
        WinSetWindowText(WinWindowFromID(dcd->hwndFrame,FID_TITLEBAR),s);
        dcd->totalbytes = dcd->totalfiles =
          dcd->selectedfiles = dcd->selectedbytes = 0;
        WinSetDlgItemText(dcd->hwndClient,DIR_TOTALS,"0");
        WinSetDlgItemText(dcd->hwndClient,DIR_SELECTED,"0 / 0k");
        dcd->totalfiles = FillArcCnr(dcd->hwndCnr,
                                     dcd->arcname,
                                     &dcd->info,
                                     &dcd->totalbytes);
        if(!dcd->totalfiles)
          PostMsg(dcd->hwndCnr,
                  WM_CLOSE,
                  MPVOID,
                  MPVOID);
        else {
          dcd->arcfilled = TRUE;
          if(!PostMsg(dcd->hwndCnr,
                      UM_RESCAN,
                      MPVOID,
                      MPVOID))
            WinSendMsg(dcd->hwndCnr,
                       UM_RESCAN,
                       MPVOID,
                       MPVOID);
          PostMsg(dcd->hwndCnr,
                  UM_SETUP2,
                  MPVOID,
                  MPVOID);
          WinSendMsg(dcd->hwndCnr,
                     CM_INVALIDATERECORD,
                     MPVOID,
                     MPFROM2SHORT(0,CMA_ERASE | CMA_REPOSITION));
        }
      }
      return 0;

    case UM_SELECT:
      dcd = WinQueryWindowPtr(hwnd,0);
      if(dcd) {
        switch(SHORT1FROMMP(mp1)) {
          case IDM_SELECTALL:
          case IDM_SELECTALLFILES:
            SelectAll(dcd->hwndCnr,TRUE,TRUE,NULL,NULL,FALSE);
            break;
          case IDM_DESELECTALL:
          case IDM_DESELECTALLFILES:
            DeselectAll(dcd->hwndCnr,TRUE,TRUE,NULL,NULL,FALSE);
            break;
          case IDM_DESELECTMASK:
          case IDM_SELECTMASK:
            {
              MASK     mask;
              PARCITEM pci = (PARCITEM)mp2;

              memset(&mask,0,sizeof(MASK));
              mask.fNoAttribs = TRUE;
              mask.fNoDirs = TRUE;
              strcpy(mask.prompt,
                     GetPString((SHORT1FROMMP(mp1) == IDM_SELECTMASK) ?
                                IDS_SELECTFILTERTEXT :
                                IDS_DESELECTFILTERTEXT));
              if(pci && (INT)pci != -1)
                strcpy(mask.szMask,pci->szFileName);
              if(WinDlgBox(HWND_DESKTOP,dcd->hwndCnr,PickMaskDlgProc,
                           FM3ModHandle,MSK_FRAME,MPFROMP(&mask))) {
                if(SHORT1FROMMP(mp1) == IDM_SELECTMASK)
                  SelectAll(dcd->hwndCnr,TRUE,TRUE,mask.szMask,NULL,FALSE);
                else
                  DeselectAll(dcd->hwndCnr,TRUE,TRUE,mask.szMask,NULL,FALSE);
              }
            }

          case IDM_INVERT:
            InvertAll(dcd->hwndCnr);
            break;
        }
      }
      return 0;

    case UM_ENTER:
      dcd = WinQueryWindowPtr(hwnd,0);
      if(dcd) {

        CHAR   *s = (CHAR *)mp1,*p,*pp,filename[CCHMAXPATH];

        if(s) {
          if(!dcd->info->extract) {
            DosBeep(50,100);
            free(s);
            return 0;
          }
          runemf2(SEPARATE | WINDOWED |
                  ((fArcStuffVisible) ? 0 : (BACKGROUND | MINIMIZED)) |
                  WAIT,dcd->hwndClient,dcd->workdir,NULL,"%s %s%s%s %s%s%s",
                  ((dcd->info->exwdirs) ? dcd->info->exwdirs :
                    dcd->info->extract),
                    (needs_quoting(dcd->arcname)) ? "\"" : NullStr,
                    dcd->arcname,
                    (needs_quoting(dcd->arcname)) ? "\"" : NullStr,
                    (needs_quoting(s)) ? "\"" : NullStr,
                    s,
                    (needs_quoting(s)) ? "\"" : NullStr);

          if(!dcd->info->exwdirs) {
            p = s;
            p = strrchr(s,'\\');
            pp = strrchr(s,'/');
            if(p && pp)
              p = max(p,pp);
            else if(!p)
              p = pp;
            if(p)
              memmove(s,p + 1,strlen(p + 1));
          }
          sprintf(filename,"%s\\%s",dcd->workdir,s);
          p = filename;
          while(*p) {
            if(*p == '/')
              *p = '\\';
            p++;
          }
          free(s);
          if(IsFile(filename) == 1)
            WinSendMsg(dcd->hwndCnr,UM_ENTER,MPFROMP(filename),MPVOID);
        }
      }
      return 0;

    case UM_COMMAND:
      if(mp1) {
        if(PostMsg(hwnd,UM_ACTION,mp1,mp2))
          return (MRESULT)TRUE;
      }
      return 0;

    case UM_ACTION:
      DosError(FERR_DISABLEHARDERR);
      dcd = WinQueryWindowPtr(hwnd,0);
      if(dcd) {

        LISTINFO    *li = (LISTINFO *)mp1;
        register INT x;

        if(li && li->list && li->list[0]) {
          switch(li->type) {
            case IDM_ARCHIVE:
            case IDM_ARCHIVEM:
              {
                DIRCNRDATA  ad;
                CHAR        szBuffer[1025],*p;

                if(!li->list[1] && !stricmp(li->list[0],dcd->arcname)) {
                  DosBeep(250,100);
                  break;
                }
                ad = *dcd;
                ad.namecanchange = 0;
                ad.fmoving = (li->type == IDM_ARCHIVEM);
                if(!WinDlgBox(HWND_DESKTOP,dcd->hwndClient,
                              ArchiveDlgProc,FM3ModHandle,
                              ARCH_FRAME,(PVOID)&ad) ||
                              !*ad.arcname ||
                              !*ad.command) /* we blew it */
                  break;
                /* build the sucker */
                strcpy(szBuffer,ad.command);
                strcat(szBuffer," ");
                if(needs_quoting(ad.arcname))
                  strcat(szBuffer,"\"");
                strcat(szBuffer,ad.arcname);
                if(needs_quoting(ad.arcname))
                  strcat(szBuffer,"\"");
                p = &szBuffer[strlen(szBuffer)];
                if(ad.mask.szMask) {
                  strcat(szBuffer," ");
                  if(needs_quoting(ad.mask.szMask))
                    strcat(szBuffer,"\"");
                  strcat(szBuffer,ad.mask.szMask);
                  if(needs_quoting(ad.mask.szMask))
                    strcat(szBuffer,"\"");
                }
                strcat(szBuffer," ");
                x = 0;
                while(li->list[x]) {
                  if(needs_quoting(li->list[x]))
                    strcat(szBuffer,"\"");
                  strcat(szBuffer,li->list[x]);
                  if(!IsFile(li->list[x])) {
                    if(szBuffer[strlen(szBuffer) - 1] != '\\')
                      strcat(szBuffer,"\\");
                    strcat(szBuffer,"*");
                  }
                  if(needs_quoting(li->list[x]))
                    strcat(szBuffer,"\"");
                  x++;
                  if(!li->list[x] || strlen(szBuffer) +
                     strlen(li->list[x]) + 5 > 1024) {
                    runemf2(SEPARATE | WINDOWED |
                            ((fArcStuffVisible) ? 0 :
                              (BACKGROUND | MINIMIZED)) |
                            WAIT,hwnd,NULL,NULL,"%s",szBuffer);
                    *p = 0;
                  }
                  strcat(szBuffer," ");
                }
                PostMsg(dcd->hwndCnr,
                        UM_RESCAN,
                        MPFROMSHORT(1),
                        MPVOID);
                Broadcast(WinQueryAnchorBlock(hwnd),
                          hwndMain,
                          UM_UPDATERECORD,
                          MPFROMP(ad.arcname),
                          MPVOID);
                Broadcast(WinQueryAnchorBlock(hwnd),
                          hwndMain,
                          UM_UPDATERECORDLIST,
                          MPFROMP(li->list),
                          MPVOID);
              }
              break;

            case IDM_REFRESH:
            case IDM_DELETE:
              {
                CHAR      cl[1001],*endofit;
                INT       z;
                CHECKLIST ck;
                CHAR      prompt[CCHMAXPATH + 257];

                if(!dcd->info->delete)
                  break;
                memset(&ck,0,sizeof(ck));
                ck.size = sizeof(ck);
                ck.list = li->list;
                ck.cmd  = li->type;
                ck.prompt = prompt;
                sprintf(prompt,GetPString(IDS_ARCCNRDELREFTEXT),
                        (li->type == IDM_DELETE) ?
                         GetPString(IDS_DELETELOWERTEXT) :
                         GetPString(IDS_REFRESHLOWERTEXT),
                        &"s"[li->list[1] == NULL],
                        dcd->arcname,
                        (li->type == IDM_DELETE) ?
                         GetPString(IDS_DELETELOWERTEXT) :
                         GetPString(IDS_REFRESHLOWERTEXT));
                if(!WinDlgBox(HWND_DESKTOP,hwnd,CheckListProc,
                              FM3ModHandle,
                              CHECK_FRAME,MPFROMP(&ck)))
                  break;
                li->list = ck.list;
                if(!li->list || !li->list[0])
                  break;
                if(li->type == IDM_DELETE)
                  sprintf(cl,"%s %s%s%s",dcd->info->delete,
                          (needs_quoting(dcd->arcname)) ? "\"" : NullStr,
                          dcd->arcname,
                          (needs_quoting(dcd->arcname)) ? "\"" : NullStr);
                else
                  sprintf(cl,"%s %s%s%s",dcd->info->create,
                          (needs_quoting(dcd->arcname)) ? "\"" : NullStr,
                          dcd->arcname,
                          (needs_quoting(dcd->arcname)) ? "\"" : NullStr);
                endofit = &cl[strlen(cl)];
                z = 0;
                do {
                  for(x = z;li->list[x] &&
                       strlen(cl) + strlen(li->list[x]) < 999;x++) {
                    strcat(cl," ");
                    if(needs_quoting(li->list[x]))
                      strcat(cl,"\"");
                    strcat(cl,li->list[x]);
                    if(needs_quoting(li->list[x]))
                      strcat(cl,"\"");
                  }
                  z = x;
                  runemf2(SEPARATE | WINDOWED |
                          ((fArcStuffVisible) ? 0 : (BACKGROUND | MINIMIZED)) |
                          WAIT,hwnd,NullStr,NULL,"%s",cl);
                  *endofit = 0;
                } while(li->list[x]);
                PostMsg(dcd->hwndCnr,
                        UM_RESCAN,
                        MPFROMSHORT(1),
                        MPVOID);
                Broadcast(WinQueryAnchorBlock(hwnd),
                          hwndMain,
                          UM_UPDATERECORD,
                          MPFROMP(dcd->arcname),
                          MPVOID);
              }
              break;

            case IDM_PRINT:
            case IDM_VIRUSSCAN:
            case IDM_VIEW:
            case IDM_MCIPLAY:
            case IDM_VIEWARCHIVE:
            case IDM_VIEWTEXT:
            case IDM_VIEWBINARY:
            case IDM_EDIT:
            case IDM_EDITTEXT:
            case IDM_EDITBINARY:
            case IDM_EXEC:
            case IDM_EXTRACTWDIRS:
            case IDM_EXTRACT:
              {
                CHAR cl[1001],*endofit,*ptr;
                INT  z;

                if((li->type == IDM_EXTRACT && !li->info->extract) ||
                   ((li->type == IDM_VIEW || li->type == IDM_VIEWTEXT ||
                    li->type == IDM_VIEWBINARY || li->type == IDM_EDIT ||
                    li->type == IDM_VIEWARCHIVE || li->type == IDM_EDITTEXT ||
                    li->type == IDM_EDITBINARY || li->type == IDM_MCIPLAY) &&
                   (!li->info->extract && !li->info->exwdirs)) ||
                   (li->type != IDM_EXTRACT && li->type != IDM_EDIT &&
                    li->type != IDM_VIEW && li->type != IDM_VIEWTEXT &&
                    li->type != IDM_VIEWBINARY &&
                    li->type != IDM_VIEWARCHIVE &&
                    li->type != IDM_EDITTEXT &&
                    li->type != IDM_EDITBINARY &&
                    li->type != IDM_MCIPLAY &&
                    !li->info->exwdirs)) {
                  DosBeep(250,100);
                  break;
                }
                if(li->type == IDM_EXTRACT || li->type == IDM_EXTRACTWDIRS) {

                  CHAR        fullname[CCHMAXPATH * 2];
                  CHAR        **exfiles = NULL;
                  INT         numfiles = 0,numalloc = 0;

                  for(x = 0;li->list[x];x++) {
                    sprintf(fullname,"%s%s%s",
                            li->targetpath,
                            (li->targetpath[strlen(li->targetpath) - 1] == '\\') ?
                            NullStr : "\\",
                            li->list[x]);
                    if(IsFile(fullname) != -1) {
                      AddToList(li->list[x],&exfiles,&numfiles,
                                &numalloc);
                      li->list = RemoveFromList(li->list,li->list[x]);
                      if(!li->list)
                        break;
                      x--;
                    }
                  }
                  if(exfiles && numfiles) {

                    CHECKLIST ckl;
                    CHAR      prompt[(CCHMAXPATH * 2) + 256];

                    memset(&ckl,0,sizeof(ckl));
                    ckl.size = sizeof(ckl);
                    ckl.list = exfiles;
                    ckl.prompt = prompt;
                    ckl.cmd = li->type;
                    sprintf(prompt,
                            GetPString(IDS_REPLACEWARNTEXT),
                            &"s"[numfiles == 1],
                            li->arcname,
                            &"s"[numfiles != 1],
                            li->targetpath);
                    if(!WinDlgBox(HWND_DESKTOP,hwnd,CheckListProc,
                                  FM3ModHandle,
                                  CHECK_FRAME,MPFROMP(&ckl))) {
                      if(ckl.list)
                        FreeList(ckl.list);
                      break;
                    }
                    else if(ckl.list)
                      li->list = CombineLists(li->list,ckl.list);
                  }
                }
                if(!li->list || !li->list[0])
                  break;
                sprintf(cl,"%s %s%s%s",(li->type == IDM_EXTRACT ||
                        ((li->type == IDM_VIEW || li->type == IDM_VIEWTEXT ||
                          li->type == IDM_VIEWBINARY ||
                          li->type == IDM_VIEWARCHIVE ||
                          li->type == IDM_PRINT || li->type == IDM_EDIT ||
                          li->type == IDM_EDITTEXT ||
                          li->type == IDM_EDITBINARY &&
                          li->type == IDM_MCIPLAY) &&
                          !li->info->exwdirs)) ?
                        li->info->extract : li->info->exwdirs,
                        (needs_quoting(li->arcname)) ? "\"" : NullStr,
                        li->arcname,
                        (needs_quoting(li->arcname)) ? "\"" : NullStr);
                endofit = &cl[strlen(cl)];
                z = 0;
                do {
                  for(x = z;li->list[x] &&
                       strlen(cl) + strlen(li->list[x]) < 999;x++) {
                    strcat(cl," ");
                    if(needs_quoting(li->list[x]))
                      strcat(cl,"\"");
                    strcat(cl,li->list[x]);
                    if(needs_quoting(li->list[x]))
                      strcat(cl,"\"");
                    ptr = li->list[x];
                    while(*ptr) {
                      if(*ptr == '/')
                        *ptr = '\\';
                      ptr++;
                    }
                  }
                  z = x;
                  runemf2(SEPARATE | WINDOWED |
                          ((fArcStuffVisible) ? 0 : (BACKGROUND | MINIMIZED)) |
                          WAIT,hwnd,li->targetpath,NULL,"%s",cl);
                  *endofit = 0;
                } while(li->list[x]);
                if(li->type == IDM_EXTRACT || li->type == IDM_EXTRACTWDIRS) {
                  /* update windows */
                  for(x = 0;li->list[x];x++) {

                    CHAR *temp,*p;

                    temp = li->list[x];
                    p = temp;
                    while(*p) {
                      if(*p == '/')
                        *p = '\\';
                      p++;
                    }
                    p = malloc(strlen(temp) + strlen(li->targetpath) + 2);
                    if(p) {
                      strcpy(p,li->targetpath);
                      if(p[strlen(p) - 1] != '\\')
                        strcat(p,"\\");
                      strcat(p,temp);
                      li->list[x] = p;
                      free(temp);
                    }
                  }
                  if(fFolderAfterExtract) {

                    CHAR   objectpath[CCHMAXPATH],*p;
                    APIRET rc;

                    GetDesktopName(objectpath,sizeof(objectpath));
                    rc = WinDlgBox(HWND_DESKTOP,dcd->hwndParent,ObjCnrDlgProc,
                                   FM3ModHandle,OBJCNR_FRAME,
                                   MPFROMP(objectpath));
                    if(rc) {
                      if(rc > 1)
                        strcpy(objectpath,"<WP_DESKTOP>");
                      p = NULL;
                      if(li->arcname) {
                        p = strrchr(li->arcname,'\\');
                        if(p)
                          p++;
                      }
                      MakeShadows(dcd->hwndParent,li->list,2,objectpath,p);
                    }
                  }
                  Broadcast(WinQueryAnchorBlock(hwnd),
                            hwndMain,
                            UM_UPDATERECORDLIST,
                            MPFROMP(li->list),
                            MPVOID);
                }
                else if(li->type == IDM_EXEC)
                  ExecOnList(hwnd,
                             li->runfile,
                             WINDOWED | SEPARATEKEEP | PROMPT,
                             li->targetpath,
                             NULL,
                             GetPString(IDS_EXECARCFILETITLETEXT));
                else if(li->type == IDM_VIRUSSCAN)
                  ExecOnList(hwnd,virus,PROMPT | WINDOWED | SEPARATEKEEP,
                             li->targetpath,NULL,
                             GetPString(IDS_VIRUSSCANARCHIVETITLETEXT));
                else if(li->type == IDM_VIEW || li->type == IDM_VIEWTEXT ||
                        li->type == IDM_VIEWBINARY || li->type == IDM_EDIT ||
                        li->type == IDM_EDITTEXT ||
                        li->type == IDM_VIEWARCHIVE ||
                        li->type == IDM_EDITBINARY ||
                        li->type == IDM_MCIPLAY ||
                        li->type == IDM_PRINT) {

                  CHAR *temp,*p;

                  for(x = 0;li->list[x];x++) {
                    if(!li->info->exwdirs) {
                      temp = li->list[x];
                      p = strrchr(li->list[x],'\\');
                      if(p) {
                        p++;
                        li->list[x] = strdup(p);
                        if(li->list[x])
                          free(temp);
                        else
                          li->list[x] = temp;
                      }
                    }
                    sprintf(cl,"%s%s%s",li->targetpath,
                            (li->targetpath[strlen(li->targetpath) - 1] == '\\') ?
                            NullStr : "\\",li->list[x]);
                    temp = li->list[x];
                    li->list[x] = strdup(cl);
                    if(li->list[x])
                      free(temp);
                    else
                      li->list[x] = temp;
                  }
                  if(li->type == IDM_VIEW || li->type == IDM_EDIT) {

                    BOOL isit = TestBinary(li->list[0]);

                    if(isit) {
                      if(li->type == IDM_VIEW)
                        li->type = IDM_VIEWBINARY;
                      else
                        li->type = IDM_EDITBINARY;
                    }
                    else {
                      if(li->type == IDM_VIEW)
                        li->type = IDM_VIEWTEXT;
                      else
                        li->type = IDM_EDITTEXT;
                    }
                  }
                  if(li->type == IDM_MCIPLAY) {

                    FILE *fp;

                    fp = fopen("$FM2PLAY.$$$","w");
                    if(fp) {
                      fprintf(fp,"%s",";AV/2-built FM2Play listfile\n");
                      for(x = 0;li->list[x];x++)
                        fprintf(fp,"%s\n",li->list[x]);
                      fprintf(fp,";end\n");
                      fclose(fp);
                      runemf2(SEPARATE | WINDOWED,HWND_DESKTOP,NULL,
                              NULL,"%sFM2PLAY.EXE /@$FM2PLAY.$$$",
                              (fAddUtils) ? "UTILS\\" : NullStr);
                    }
                  }
                  else if(li->type == IDM_PRINT) {
                    strcpy(li->targetpath,printer);
                    if(_beginthread(PrintList,
                                    NULL,
                                    65536,
                                    (PVOID)li) != -1)
                      li = NULL;
                  }
                  else if(li->type == IDM_VIEWARCHIVE) {

                    ARC_TYPE *info;

                    for(x = 0;li->list[x];x++) {
                      if(IsFile(li->list[x]) == 1) {
                        info = NULL;
                        if(WinDlgBox(HWND_DESKTOP,HWND_DESKTOP,
                                     SBoxDlgProc,FM3ModHandle,ASEL_FRAME,
                                     (PVOID)&info) && info)
                          StartArcCnr(HWND_DESKTOP,
                                      HWND_DESKTOP,
                                      li->list[x],
                                      4,
                                      info);
                      }
                    }
                  }
                  else if((li->type == IDM_VIEWTEXT && *viewer) ||
                          (li->type == IDM_VIEWBINARY && *binview) ||
                          (li->type == IDM_EDITTEXT && *editor) ||
                          (li->type == IDM_EDITBINARY && *bined)) {
                    DosSleep(100L);
                    ExecOnList(hwnd,((li->type == IDM_VIEWTEXT) ? viewer :
                                     (li->type == IDM_VIEWBINARY) ? binview :
                                     (li->type == IDM_EDITTEXT) ? editor :
                                     bined),
                               WINDOWED | SEPARATE,li->targetpath,li->list,
                               NULL);
                  }
                  else {
                    if(li->hwnd) {

                      ULONG viewtype;

                      for(x = 0;li->list[x];x++) {
                        if(x == 0) {
                          if(li->type == IDM_VIEWBINARY ||
                             li->type == IDM_EDITBINARY)
                            viewtype = 16;
                          else
                            viewtype = 8;
                        }
                        else
                          viewtype = 0;
                        temp = strdup(li->list[x]);
                        if(temp) {
                          if(!PostMsg(WinQueryWindow(li->hwnd,QW_PARENT),
                                         UM_LOADFILE,
                                         MPFROMLONG(4L +
                                         (li->type == IDM_VIEWTEXT ||
                                          li->type == IDM_VIEWBINARY) +
                                          viewtype),
                                         MPFROMP(temp)))
                            free(temp);
                        }
                      }
                    }
                  }
                }
              }
              break;

            case IDM_FIND:
              {
                INT         numfiles = 0,numalloced = 0;
                CHAR      **list2 = NULL,fullname[CCHMAXPATH * 2],*p;

                for(x = 0;li->list[x];x++) {
                  p = li->list[x];
                  while(*p) {
                    if(*p == '/')
                      *p = '\\';
                    p++;
                  }
                  sprintf(fullname,"%s%s%s",dcd->directory,
                          (dcd->directory[strlen(dcd->directory) - 1] == '\\') ?
                          NullStr : "\\",li->list[x]);
                  if(IsFile(fullname) != -1)
                    if(AddToList(fullname,&list2,&numfiles,&numalloced))
                      break;
                  if(strchr(li->list[x],'\\')) {
                    p = strrchr(li->list[x],'\\');
                    if(p) {
                      p++;
                      if(*p) {
                        sprintf(fullname,"%s%s%s",dcd->directory,
                                (dcd->directory[strlen(dcd->directory) - 1] == '\\') ?
                                NullStr : "\\",p);
                        if(IsFile(fullname) != -1)
                          if(AddToList(fullname,&list2,&numfiles,&numalloced))
                            break;
                      }
                    }
                  }
                }
                if(numfiles && list2) {
                  WinSendMsg(dcd->hwndCnr,WM_COMMAND,
                             MPFROM2SHORT(IDM_COLLECTOR,0),MPVOID);
                  DosSleep(128L);
                  if(Collector) {
                    if(!PostMsg(Collector,WM_COMMAND,
                                   MPFROM2SHORT(IDM_COLLECTOR,0),
                                   MPFROMP(list2)))
                      FreeList(list2);
                  }
                  else
                    FreeList(list2);
                }
                else
                  DosBeep(250,100);
              }
              break;
          }
        }
        FreeListInfo(li);
      }
      return 0;

    case WM_CLOSE:
      WinDestroyWindow(hwnd);
      break;

    case WM_DESTROY:
      dcd = WinQueryWindowPtr(hwnd,0);
      if(dcd) {
        if(*dcd->workdir) {
          DosSleep(33L);
          wipeallf("%s\\*",dcd->workdir);
          if(rmdir(dcd->workdir)) {
            DosSleep(256L);
            wipeallf("%s\\*",dcd->workdir);
            rmdir(dcd->workdir);
          }
        }
        FreeList(dcd->lastselection);
        WinSendMsg(dcd->hwndCnr,UM_CLOSE,MPVOID,MPVOID);
        free(dcd);
        WinSetWindowPtr(dcd->hwndCnr,0,NULL);
      }
      if(!PostMsg((HWND)0,WM_QUIT,MPVOID,MPVOID))
        WinSendMsg((HWND)0,WM_QUIT,MPVOID,MPVOID);
      break;
  }
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ArcCnrWndProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  DIRCNRDATA *dcd = WinQueryWindowPtr(hwnd,0);

  switch(msg) {
    case DM_PRINTOBJECT:
    case DM_DISCARDOBJECT:
      if(dcd)
        return WinSendMsg(dcd->hwndObject,
                          msg,
                          mp1,
                          mp2);
      else
        return MRFROMLONG(DRR_TARGET);

    case WM_CHAR:
      shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
      if(SHORT1FROMMP(mp1) & KC_KEYUP)
        return (MRESULT)TRUE;
      if(SHORT1FROMMP(mp1) & KC_VIRTUALKEY) {
        switch(SHORT2FROMMP(mp2)) {
          case VK_DELETE:
            PostMsg(hwnd,
                    WM_COMMAND,
                    MPFROM2SHORT(IDM_DELETE,0),
                    MPVOID);
            break;
        }
      }
      if(shiftstate || fNoSearch)
        break;
      if(SHORT1FROMMP(mp1) & KC_CHAR) {

        ULONG        thistime,len;
        SEARCHSTRING srch;
        PCNRITEM     pci;

        if(!dcd)
          break;
        switch(SHORT1FROMMP(mp2)) {
          case '\x1b':
          case '\r':
          case '\n':
            dcd->lasttime = 0;
            *dcd->comnam = 0;
            break;
          default:
            thistime = WinQueryMsgTime(WinQueryAnchorBlock(hwnd));
            if(thistime > dcd->lasttime + 1250)
              *dcd->comnam = 0;
            dcd->lasttime = thistime;
            if(SHORT1FROMMP(mp2) == ' ' && !dcd->comnam)
              break;
KbdRetry:
            len = strlen(dcd->comnam);
            if(len >= CCHMAXPATH - 1) {
              *dcd->comnam = 0;
              len = 0;
            }
            dcd->comnam[len] = toupper(SHORT1FROMMP(mp2));
            dcd->comnam[len + 1] = 0;
            memset(&srch,0,sizeof(SEARCHSTRING));
            srch.cb = (ULONG)sizeof(SEARCHSTRING);
            srch.pszSearch = (PSZ)dcd->comnam;
            srch.fsPrefix = TRUE;
            srch.fsCaseSensitive = FALSE;
            srch.usView = CV_ICON;
            pci = WinSendMsg(hwnd,
                             CM_SEARCHSTRING,
                             MPFROMP(&srch),
                             MPFROMLONG(CMA_FIRST));
            if(pci && (INT)pci != -1) {

              USHORT  attrib = CRA_CURSORED;

              /* make found item current item */
              if(!stricmp(pci->pszFileName,dcd->comnam))
                attrib |= CRA_SELECTED;
              WinSendMsg(hwnd,
                         CM_SETRECORDEMPHASIS,
                         MPFROMP(pci),
                         MPFROM2SHORT(TRUE,attrib));
              /* make sure that record shows in viewport */
              ShowCnrRecord(hwnd,(PMINIRECORDCORE)pci);
              return (MRESULT)TRUE;
            }
            else {
              if(SHORT1FROMMP(mp2) == ' ') {
                dcd->comnam[len] = 0;
                break;
              }
              *dcd->comnam = 0;
              dcd->lasttime = 0;
              if(len)           // retry as first letter if no match
                goto KbdRetry;
            }
            break;
        }
      }
      break;

    case WM_MOUSEMOVE:
    case WM_BUTTON1UP:
    case WM_BUTTON2UP:
    case WM_BUTTON3UP:
    case WM_CHORD:
      shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
      break;

    case WM_BUTTON1MOTIONEND:
      {
        CNRINFO cnri;

        memset(&cnri,0,sizeof(CNRINFO));
        cnri.cb = sizeof(CNRINFO);
        if(WinSendMsg(hwnd,
                      CM_QUERYCNRINFO,
                      MPFROMP(&cnri),
                      MPFROMLONG(sizeof(CNRINFO)))) {
          if(cnri.flWindowAttr & CV_DETAIL)
            PrfWriteProfileData(fmprof,
                                appname,
                                "ArcCnrSplitBar",
                                (PVOID)&cnri.xVertSplitbar,
                                sizeof(LONG));
        }
      }
      break;

    case WM_PRESPARAMCHANGED:
      PresParamChanged(hwnd,"ArcCnr",mp1,mp2);
      break;

    case UM_UPDATERECORD:
    case UM_UPDATERECORDLIST:
      if(dcd &&
         !IsArcThere(hwnd,dcd->arcname))
        PostMsg(hwnd,
                WM_CLOSE,
                MPVOID,
                MPVOID);
      return 0;

    case WM_SETFOCUS:
      /*
       * put name of our window (archive name) on status line
       */
      if(dcd &&
         hwndStatus &&
         mp2)
        WinSendMsg(hwnd,
                   UM_RESCAN,
                   MPVOID,
                   MPVOID);
      break;

    case UM_SETUP2:
      if(dcd &&
         dcd->info) {
        if(dcd->info->fdpos == -1 ||
           !dcd->info->datetype)
          dcd->sortFlags &= (~SORT_LWDATE);
        if(dcd->info->nsizepos == -1)
          dcd->sortFlags &= (~SORT_EASIZE);
        if(dcd->info->osizepos == -1)
          dcd->sortFlags &= (~SORT_SIZE);
        AdjustCnrColVis(hwnd,
                        GetPString(IDS_OLDSIZECOLTEXT),
                        (dcd->info->osizepos != -1),
                        FALSE);
        AdjustCnrColVis(hwnd,
                        GetPString(IDS_NEWSIZECOLTEXT),
                        (dcd->info->nsizepos != -1),
                        FALSE);
        AdjustCnrColVis(hwnd,
                        GetPString(IDS_DATETIMECOLTEXT),
                        (dcd->info->fdpos != -1 && !dcd->info->datetype),
                        FALSE);
        AdjustCnrColVis(hwnd,
                        GetPString(IDS_TIMECOLTEXT),
                        (dcd->info->fdpos != -1 || dcd->info->datetype),
                        FALSE);
        AdjustCnrColVis(hwnd,
                        GetPString(IDS_DATECOLTEXT),
                        (dcd->info->fdpos != -1 || dcd->info->datetype),
                        FALSE);
        WinSendMsg(hwnd,
                   CM_INVALIDATEDETAILFIELDINFO,
                   MPVOID,
                   MPVOID);
      }
      return 0;

    case UM_RESCAN:
      if(dcd) {

        CNRINFO  cnri;
        CHAR     s[CCHMAXPATH * 2],tb[81],tf[81],*tbk;
        PARCITEM pci;

        if(mp1) {
          PostMsg(dcd->hwndObject,
                  UM_RESCAN,
                  MPVOID,
                  MPVOID);
          return 0;
        }
        memset(&cnri,0,sizeof(CNRINFO));
        cnri.cb = sizeof(CNRINFO);
        WinSendMsg(hwnd,
                   CM_QUERYCNRINFO,
                   MPFROMP(&cnri),
                   MPFROMLONG(sizeof(CNRINFO)));
        dcd->totalfiles = cnri.cRecords;
        commafmt(tf,sizeof(tf),dcd->selectedfiles);
        *tb = 0;
        if(dcd->totalbytes) {
          if(dcd->selectedbytes > 1024) {
            tbk = "k";
            commafmt(tb,sizeof(tb),dcd->selectedbytes / 1024L);
          }
          else {
            tbk = "b";
            commafmt(tb,sizeof(tb),dcd->selectedbytes);
          }
        }
        sprintf(s,"%s%s%s%s",tf,(*tb) ? " / " : NullStr,tb,
                (*tb) ? tbk : NullStr);
        WinSetDlgItemText(dcd->hwndClient,
                          DIR_SELECTED,
                          s);
        commafmt(tf,sizeof(tf),dcd->totalfiles);
        *tb = 0;
        if(dcd->totalbytes) {
          if(dcd->totalbytes > 1024) {
            tbk = "k";
            commafmt(tb,sizeof(tb),dcd->totalbytes / 1024L);
          }
          else {
            tbk = "b";
            commafmt(tb,sizeof(tb),dcd->totalbytes);
          }
        }
        sprintf(s,"%s%s%s%s",tf,(*tb) ? " / " : NullStr,tb,
                (*tb) ? tbk : NullStr);
        WinSetDlgItemText(dcd->hwndClient,
                          DIR_TOTALS,
                          s);
        if(hwndStatus &&
           dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent)) {
          sprintf(s," [%s%s%s%s]%s%s%s  %s",tf,(*tb) ? " / " : NullStr,
                  tb,(*tb) ? tbk : NullStr,
                  (*dcd->mask.szMask) ? " (" : NullStr,
                  (*dcd->mask.szMask) ? dcd->mask.szMask : NullStr,
                  (*dcd->mask.szMask) ? ")" : NullStr,dcd->arcname);
          WinSetWindowText(hwndStatus,s);
          if(!ParentIsDesktop(hwnd,dcd->hwndParent)) {
            pci = WinSendMsg(hwnd,
                             CM_QUERYRECORDEMPHASIS,
                             MPFROMLONG(CMA_FIRST),
                             MPFROMSHORT(CRA_CURSORED));
            if(pci && (INT)pci != -1) {
              if(fSplitStatus && hwndStatus2) {
                *tb = 0;
                if(dcd->totalbytes)
                  commafmt(tb,sizeof(tb),pci->cbFile);
                sprintf(s,"%s%s%s%s",(*tb) ? " " : NullStr,tb,
                        (*tb) ? "b  " : NullStr,
                        pci->szFileName);
                WinSetWindowText(hwndStatus2,s);
              }
              if(fMoreButtons)
                WinSetWindowText(hwndName,
                                 pci->szFileName);
            }
            else {
              WinSetWindowText(hwndStatus2,NullStr);
              WinSetWindowText(hwndName,NullStr);
            }
            WinSetWindowText(hwndDate,NullStr);
            WinSetWindowText(hwndAttr,NullStr);
          }
        }
        if((dcd->arcfilled &&
            !dcd->totalfiles) ||
           !IsArcThere(hwnd,dcd->arcname))
          PostMsg(hwnd,
                  WM_CLOSE,
                  MPVOID,
                  MPVOID);
      }
      return 0;

    case UM_SETUP:
      if(dcd) {
        if(!dcd->hwndObject) {
          /*
           * first time through -- set things up
           */
          {
            APIRET rc;
            CHAR  *p,*pp;
            ULONG  z,was;

            rc = DosCreateDir(dcd->workdir,0L);
            if(rc) {
              if(rc == ERROR_ACCESS_DENIED) {
                p = strrchr(dcd->workdir,'.');
                if(p) {
                  p++;
                  pp = p;
                  was = strtoul(p,&pp,16);
                  for(z = 0;z < 99;z++) {
                    was++;
                    sprintf(p,"%03x");
                    rc = DosCreateDir(dcd->workdir,0L);
                    if(!rc || rc != ERROR_ACCESS_DENIED)
                      break;
                  }
                }
              }
              if(rc)
                PostMsg(hwnd,WM_CLOSE,MPVOID,MPVOID);
              return 0;
            }
          }
          RestorePresParams(hwnd,"ArcCnr");
          dcd->mask.fNoAttribs = TRUE;
          dcd->mask.fNoDirs = TRUE;
          *dcd->mask.prompt = 0;
          {
            PFIELDINFO  pfi, pfiLastLeftCol;
            ULONG       numcols = CON_COLS;

            pfi = WinSendMsg(hwnd,
                             CM_ALLOCDETAILFIELDINFO,
                             MPFROMLONG(numcols),
                             NULL);
            if(pfi) {

              PFIELDINFO      pfiFirst;
              FIELDINFOINSERT fii;

              pfiFirst = pfi;
              pfi->flData     = CFA_STRING | CFA_LEFT | CFA_FIREADONLY;
              pfi->flTitle    = CFA_CENTER;
              pfi->pTitleData = GetPString(IDS_FILENAMECOLTEXT);
              pfi->offStruct  = FIELDOFFSET(ARCITEM, pszFileName);
              pfiLastLeftCol = pfi;
              pfi             = pfi->pNextFieldInfo;
              pfi->flData     = CFA_ULONG | CFA_RIGHT | CFA_SEPARATOR | CFA_FIREADONLY;
              pfi->flTitle    = CFA_CENTER;
              pfi->pTitleData = GetPString(IDS_OLDSIZECOLTEXT);
              pfi->offStruct  = FIELDOFFSET(ARCITEM, cbFile);
              pfi             = pfi->pNextFieldInfo;
              pfi->flData     = CFA_ULONG | CFA_RIGHT | CFA_SEPARATOR | CFA_FIREADONLY;
              pfi->flTitle    = CFA_CENTER;
              pfi->pTitleData = GetPString(IDS_NEWSIZECOLTEXT);
              pfi->offStruct  = FIELDOFFSET( ARCITEM, cbComp );
              pfi             = pfi->pNextFieldInfo;
              pfi->flData     = CFA_STRING | CFA_CENTER | CFA_SEPARATOR | CFA_FIREADONLY;
              pfi->flTitle    = CFA_CENTER | CFA_FITITLEREADONLY;
              pfi->pTitleData = GetPString(IDS_DATETIMECOLTEXT);
              pfi->offStruct  = FIELDOFFSET(ARCITEM, pszDate);
              pfi             = pfi->pNextFieldInfo;
              pfi->flData     = CFA_DATE | CFA_RIGHT | CFA_FIREADONLY;
              pfi->flTitle    = CFA_CENTER;
              pfi->pTitleData = GetPString(IDS_DATECOLTEXT);
              pfi->offStruct  = FIELDOFFSET(ARCITEM, date);
              pfi             = pfi->pNextFieldInfo;
              pfi->flData     = CFA_TIME | CFA_RIGHT | CFA_FIREADONLY;
              pfi->flTitle    = CFA_CENTER | CFA_FITITLEREADONLY;
              pfi->pTitleData = GetPString(IDS_TIMECOLTEXT);
              pfi->offStruct  = FIELDOFFSET( ARCITEM, time );
              memset( &fii, 0, sizeof( FIELDINFOINSERT ) );
              fii.cb                   = sizeof( FIELDINFOINSERT );
              fii.pFieldInfoOrder      = (PFIELDINFO) CMA_FIRST;
              fii.cFieldInfoInsert     = (SHORT) numcols;
              fii.fInvalidateFieldInfo = TRUE;
              WinSendMsg(hwnd,
                         CM_INSERTDETAILFIELDINFO,
                         MPFROMP(pfiFirst),
                         MPFROMP(&fii));
              PostMsg(hwnd,
                      UM_SETUP2,
                      MPVOID,
                      MPVOID);
              {
                CNRINFO cnri;

                memset(&cnri,0,sizeof(cnri));
                cnri.cb             = sizeof( CNRINFO );
                cnri.pFieldInfoLast = pfiLastLeftCol;
                cnri.xVertSplitbar  = DIR_SPLITBAR_OFFSET + 32;
                {
                  ULONG size;

                  size = sizeof(LONG);
                  PrfQueryProfileData(fmprof,appname,"ArcCnrSplitBar",
                                      &cnri.xVertSplitbar,&size);
                  if(cnri.xVertSplitbar <= 0)
                    cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET + 32;
                }
                cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
                                      CV_NAME));
                cnri.flWindowAttr |= (CV_DETAIL | CA_DETAILSVIEWTITLES |
                                      CV_FLOW);
                cnri.flWindowAttr &= (~(CA_ORDEREDTARGETEMPH |
                                        CA_MIXEDTARGETEMPH));
                cnri.pSortRecord = (PVOID)ArcSort;
                WinSendMsg(hwnd,
                           CM_SETCNRINFO,
                           MPFROMP(&cnri),
                           MPFROMLONG(CMA_PFIELDINFOLAST |
                                      CMA_XVERTSPLITBAR  |
                                      CMA_PSORTRECORD    |
                                      CMA_FLWINDOWATTR));
              }
            }
          }
          WinSendMsg(hwnd,
                     CM_SORTRECORD,
                     MPFROMP(ArcSort),
                     MPFROMP(dcd));
          if(_beginthread(MakeObjWin,
                          NULL,
                          245760,
                          (PVOID)dcd) == -1) {
            PostMsg(hwnd,
                    WM_CLOSE,
                    MPVOID,
                    MPVOID);
            return 0;
          }
          else
            DosSleep(1L);
          SayFilter(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                    DIR_FILTER),
                    &dcd->mask,
                    TRUE);
          SaySort(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                  DIR_SORT),
                  dcd->sortFlags,
                  TRUE);
        }
      }
      else {
        PostMsg(hwnd,
                WM_CLOSE,
                MPVOID,
                MPVOID);
        return 0;
      }
      return 0;

    case UM_SETDIR:
      if(dcd) {

        CHAR        s[CCHMAXPATH],*p;
        ULONG       ret = 0;

        WinQueryDlgItemText(dcd->hwndClient,
                            ARC_EXTRACTDIR,
                            CCHMAXPATH,
                            s);
        lstrip(rstrip(s));
        MakeFullName(s);
        if(*s) {
          while((p = strchr(s,'/')) != NULL)
            *p = '\\';
          while(strlen(s) > 3 && s[strlen(s) - 1] == '\\')
            s[strlen(s) - 1] = 0;
          if(stricmp(s,dcd->directory)) {
            if(IsFullName(s)) {
              if(driveflags[toupper(*s) - 'A'] &
                 (DRIVE_NOTWRITEABLE | DRIVE_IGNORE | DRIVE_INVALID)) {
                DosBeep(250,100);
                WinSetDlgItemText(dcd->hwndClient,
                                  ARC_EXTRACTDIR,
                                  dcd->directory);
                return 0;
              }
            }
            if(!SetDir(dcd->hwndParent,hwnd,s,0)) {
              if(stricmp(dcd->directory,s)) {
                DosEnterCritSec();
                 strcpy(lastextractpath,s);
                DosExitCritSec();
              }
              strcpy(dcd->directory,s);
              if((!isalpha(*s) || s[1] != ':') && *s != '.')
                saymsg(MB_ENTER | MB_ICONASTERISK,
                       hwnd,
                       GetPString(IDS_WARNINGTEXT),
                       GetPString(IDS_SPECIFYDRIVETEXT));
            }
            else
              ret = 1;
          }
        }
        WinSetDlgItemText(dcd->hwndClient,
                          ARC_EXTRACTDIR,
                          dcd->directory);
        return (MRESULT)ret;
      }
      return 0;

    case UM_ENTER:
      if(WinSendMsg(hwnd,
                    UM_SETDIR,
                    MPVOID,
                    MPVOID))
        return 0;
      SetShiftState();
      if(dcd && (CHAR *)mp1) {

        SWP   swp;
        CHAR *filename = mp1;

        if(IsFile(filename) != 1)
          return 0;
        WinQueryWindowPos(dcd->hwndFrame,&swp);
        DefaultViewKeys(hwnd,
                        dcd->hwndFrame,
                        dcd->hwndParent,
                        &swp,
                        filename);
        if(fUnHilite)
          UnHilite(hwnd,
                   FALSE,
                   &dcd->lastselection);
      }
      return 0;

    case WM_MENUEND:
      if(dcd) {

        HWND hwndMenu = (HWND)mp2;

        if(hwndMenu == ArcCnrMenu ||
           hwndMenu == ArcMenu) {
          MarkAll(hwnd,
                  TRUE,
                  FALSE,
                  TRUE);
          if(dcd->cnremphasized) {
            WinSendMsg(hwnd,
                       CM_SETRECORDEMPHASIS,
                       MPVOID,
                       MPFROM2SHORT(FALSE,CRA_SOURCE));
            dcd->cnremphasized = FALSE;
          }
        }
      }
      break;

    case MM_PORTHOLEINIT:
      if(dcd) {
        switch(SHORT1FROMMP(mp1)) {
          case 0:
          case 1:
            {
              ULONG wmsg;

              wmsg = (SHORT1FROMMP(mp1) == 0) ? UM_FILESMENU : UM_VIEWSMENU;
              PortholeInit((HWND)WinSendMsg(dcd->hwndClient,
                                            wmsg,
                                            MPVOID,
                                            MPVOID),
                            mp1,
                            mp2);
            }
            break;
        }
      }
      break;

    case UM_INITMENU:
    case WM_INITMENU:
      if(dcd) {
        switch(SHORT1FROMMP(mp1)) {
          case IDM_FILESMENU:
            if(dcd->info) {
              WinEnableMenuItem((HWND)mp2,
                                IDM_DELETE,
                                (dcd->info->delete != NULL));
              WinEnableMenuItem((HWND)mp2,
                                IDM_TEST,
                                (dcd->info->test != NULL));
              WinEnableMenuItem((HWND)mp2,
                                IDM_EXTRACTWDIRS,
                                (dcd->info->exwdirs != NULL));
              WinEnableMenuItem((HWND)mp2,
                                IDM_ARCEXTRACTWDIRS,
                                (dcd->info->exwdirs != NULL));
              WinEnableMenuItem((HWND)mp2,
                                IDM_ARCEXTRACTWDIRSEXIT,
                                (dcd->info->exwdirs != NULL));
            }
            break;

          case IDM_VIEWSMENU:
            WinCheckMenuItem((HWND)mp2,
                             IDM_MINIICONS,
                             ((dcd->flWindowAttr & CV_MINI) != 0));
            WinEnableMenuItem((HWND)mp2,
                              IDM_RESELECT,
                              (dcd->lastselection != NULL));
            break;

          case IDM_COMMANDSMENU:
            SetupCommandMenu((HWND)mp2,
                             hwnd);
            break;

          case IDM_SORTSUBMENU:
            SetSortChecks((HWND)mp2,
                          dcd->sortFlags);
            break;

          case IDM_WINDOWSMENU:
            /*
             * add switchlist entries to end of pulldown menu
             */
            SetupWinList((HWND)mp2,
                         (hwndMain) ? hwndMain : (HWND)0,
                         dcd->hwndFrame);
            break;
        }
        dcd->hwndLastMenu = (HWND)mp2;
      }
      if(msg == WM_INITMENU)
        break;
      return 0;

    case UM_LOADFILE:
      if(dcd && mp2) {

        HWND ret;

        ret = StartMLEEditor(dcd->hwndParent,
                             (INT)mp1,
                             (CHAR *)mp2,
                             dcd->hwndFrame);
        free((CHAR *)mp2);
        return MRFROMLONG(ret);
      }
      return 0;

    case UM_COMMAND:
      if(mp1) {
        if(dcd) {
          if(!PostMsg(dcd->hwndObject,
                      UM_COMMAND,
                      mp1,
                      mp2)) {
            FreeListInfo((LISTINFO *)mp1);
            DosBeep(50,100);
          }
          else
            return (MRESULT)TRUE;
        }
        else
          FreeListInfo((LISTINFO *)mp1);
      }
      return 0;

    case UM_OPENWINDOWFORME:
      if(dcd) {
        if(mp1 &&
           !IsFile((CHAR *)mp1))
          OpenDirCnr((HWND)0,
                     hwndMain,
                     dcd->hwndFrame,
                     FALSE,
                     (char *)mp1);
        else if(mp1 &&
                IsFile(mp1) == 1)
          StartArcCnr(HWND_DESKTOP,
                      dcd->hwndFrame,
                      (CHAR *)mp1,
                      4,
                      (ARC_TYPE *)mp2);
      }
      return 0;

    case WM_COMMAND:
      DosError(FERR_DISABLEHARDERR);
      if(dcd) {
        if(SwitchCommand(dcd->hwndLastMenu,
                         SHORT1FROMMP(mp1)))
          return 0;
        if(WinSendMsg(hwnd,
                      UM_SETDIR,
                      MPVOID,
                      MPVOID))
          return 0;
        if(!IsArcThere(hwnd,
                       dcd->arcname)) {
          PostMsg(hwnd,
                  WM_CLOSE,
                  MPVOID,
                  MPVOID);
          return 0;
        }
        switch(SHORT1FROMMP(mp1)) {
          case IDM_TREEVIEW:

            break;

          case IDM_CONTEXTMENU:
            {
              PCNRITEM pci;

              pci = (PCNRITEM)CurrentRecord(hwnd);
              PostMsg(hwnd,
                      WM_CONTROL,
                      MPFROM2SHORT(ARC_CNR,
                                   CN_CONTEXTMENU),
                      MPFROMP(pci));
            }
            break;

          case IDM_NEXTWINDOW:
          case IDM_PREVWINDOW:
            {
              HWND hwndActive;

              hwndActive = WinQueryFocus(HWND_DESKTOP);
              WinSetFocus(HWND_DESKTOP,
                          ((hwndActive == hwnd) ?
                           WinWindowFromID(dcd->hwndClient,ARC_EXTRACTDIR) :
                           hwnd));
            }
            break;

          case IDM_FOLDERAFTEREXTRACT:
            fFolderAfterExtract = (fFolderAfterExtract) ? FALSE : TRUE;
            PrfWriteProfileData(fmprof,appname,"FolderAfterExtract",
                                &fFolderAfterExtract,sizeof(BOOL));
            break;

          case IDM_SHOWSELECT:
            QuickPopup(hwnd,dcd,CheckMenu(&ArcCnrMenu,ARCCNR_POPUP),
                       IDM_SELECTSUBMENU);
            break;

          case IDM_SHOWSORT:
            QuickPopup(hwnd,dcd,CheckMenu(&ArcCnrMenu,ARCCNR_POPUP),
                       IDM_SORTSUBMENU);
            break;

          case IDM_NOTEBOOK:
            if(!ParentIsDesktop(dcd->hwndParent,dcd->hwndParent))
              PostMsg(dcd->hwndParent,
                      msg,
                      mp1,
                      mp2);
            else
              WinDlgBox(HWND_DESKTOP,
                        hwnd,
                        CfgDlgProc,
                        FM3ModHandle,
                        CFG_FRAME,
                        (PVOID)"Archive");
            break;

          case IDM_RESCAN:
            dcd->totalbytes = dcd->totalfiles =
              dcd->selectedfiles = dcd->selectedbytes = 0;
            WinSetDlgItemText(dcd->hwndClient,DIR_TOTALS,"0");
            WinSetDlgItemText(dcd->hwndClient,DIR_SELECTED,"0 / 0k");
            dcd->totalfiles = FillArcCnr(dcd->hwndCnr,
                                         dcd->arcname,
                                         &dcd->info,
                                         &dcd->totalbytes);
            PostMsg(dcd->hwndCnr,
                    UM_RESCAN,
                    MPVOID,
                    MPVOID);
            PostMsg(dcd->hwndCnr,
                    UM_SETUP2,
                    MPVOID,
                    MPVOID);
            WinSendMsg(dcd->hwndCnr,
                       CM_INVALIDATERECORD,
                       MPVOID,
                       MPFROM2SHORT(0,
                                    CMA_ERASE | CMA_REPOSITION));
            break;

          case IDM_RESELECT:
            SelectList(hwnd,
                       TRUE,
                       FALSE,
                       FALSE,
                       NULL,
                       NULL,
                       dcd->lastselection);
            break;

          case IDM_HELP:
            if(hwndHelp)
              WinSendMsg(hwndHelp,
                         HM_DISPLAY_HELP,
                         MPFROM2SHORT(HELP_ARCLIST,0),
                         MPFROMSHORT(HM_RESOURCEID));
            break;

          case IDM_WINDOWDLG:
            if(!ParentIsDesktop(dcd->hwndParent,dcd->hwndFrame))
              PostMsg(dcd->hwndParent,
                      UM_COMMAND,
                      MPFROM2SHORT(IDM_WINDOWDLG,0),
                      MPVOID);
            break;

          case IDM_SELECTALL:
          case IDM_SELECTALLFILES:
          case IDM_DESELECTALL:
          case IDM_DESELECTALLFILES:
          case IDM_SELECTMASK:
          case IDM_DESELECTMASK:
          case IDM_INVERT:
            {
              PARCITEM pci;

              pci = (PARCITEM)WinSendMsg(hwnd,
                                         CM_QUERYRECORDEMPHASIS,
                                         MPFROMLONG(CMA_FIRST),
                                         MPFROMSHORT(CRA_CURSORED));
              if((INT)pci == -1)
                pci = NULL;
              if(SHORT1FROMMP(mp1) == IDM_HIDEALL) {
                if(pci) {
                  if(!(pci->rc.flRecordAttr & CRA_SELECTED))
                    pci->rc.flRecordAttr |= CRA_FILTERED;
                    WinSendMsg(hwnd,
                               CM_INVALIDATERECORD,
                               MPFROMP(&pci),
                               MPFROM2SHORT(1,
                                            CMA_ERASE | CMA_REPOSITION));
                    break;
                }
              }
              PostMsg(dcd->hwndObject,
                      UM_SELECT,
                      mp1,
                      MPFROMP(pci));
            }
            break;

          case IDM_SORTSMARTNAME:
          case IDM_SORTNAME:
          case IDM_SORTFILENAME:
          case IDM_SORTSIZE:
          case IDM_SORTEASIZE:
          case IDM_SORTFIRST:
          case IDM_SORTLAST:
          case IDM_SORTLWDATE:
            dcd->sortFlags &= SORT_REVERSE;
            /* intentional fallthru */
          case IDM_SORTREVERSE:
            switch(SHORT1FROMMP(mp1)) {
              case IDM_SORTSMARTNAME:
              case IDM_SORTFILENAME:
                dcd->sortFlags |= SORT_FILENAME;
                break;
              case IDM_SORTSIZE:
                dcd->sortFlags |= SORT_SIZE;
                break;
              case IDM_SORTEASIZE:
                dcd->sortFlags |= SORT_EASIZE;
                break;
              case IDM_SORTFIRST:
                dcd->sortFlags |= SORT_FIRSTEXTENSION;
                break;
              case IDM_SORTLAST:
                dcd->sortFlags |= SORT_LASTEXTENSION;
                break;
              case IDM_SORTLWDATE:
                dcd->sortFlags |= SORT_LWDATE;
                break;
              case IDM_SORTREVERSE:
                if(dcd->sortFlags & SORT_REVERSE)
                  dcd->sortFlags &= (~SORT_REVERSE);
                else
                  dcd->sortFlags |= SORT_REVERSE;
                break;
            }
            WinSendMsg(hwnd,
                       CM_SORTRECORD,
                       MPFROMP(ArcSort),
                       MPFROMP(dcd));
            SaySort(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                                    DIR_SORT),
                    dcd->sortFlags,
                    TRUE);
            break;

          case IDM_COLLECTOR:
            if(!Collector) {

              HWND hwndC;
              SWP  swp;

              if(ParentIsDesktop(hwnd,dcd->hwndParent) && !fAutoTile &&
                 (!fExternalCollector && !strcmp(realappname,FM3Str)))
                GetNextWindowPos(dcd->hwndParent,&swp,NULL,NULL);
              hwndC = StartCollector((fExternalCollector ||
                                      strcmp(realappname,FM3Str)) ?
                                     HWND_DESKTOP : dcd->hwndParent,4);
              if(hwndC) {
                if(!ParentIsDesktop(hwnd,dcd->hwndParent) && !fAutoTile &&
                   (!fExternalCollector && !strcmp(realappname,FM3Str)))
                  WinSetWindowPos(hwndC,
                                  HWND_TOP,
                                  swp.x,
                                  swp.y,
                                  swp.cx,
                                  swp.cy,
                                  SWP_MOVE | SWP_SIZE |
                                  SWP_SHOW | SWP_ZORDER);
                else if(!ParentIsDesktop(hwnd,dcd->hwndParent) &&
                        fAutoTile &&
                        !strcmp(realappname,FM3Str))
                  TileChildren(dcd->hwndParent,TRUE);
                WinSetWindowPos(hwndC,
                                HWND_TOP,
                                0,
                                0,
                                0,
                                0,
                                SWP_ACTIVATE);
                DosSleep(128L);
              }
            }
            else
              StartCollector(dcd->hwndParent,4);
            break;

          case IDM_ARCEXTRACTEXIT:
          case IDM_ARCEXTRACT:
            if(dcd->info->extract)
              runemf2(SEPARATE | WINDOWED |
                      ((fArcStuffVisible) ? 0 : (BACKGROUND | MINIMIZED)),
                      hwnd,dcd->directory,NULL,"%s %s%s%s",
                      dcd->info->extract,
                      (needs_quoting(dcd->arcname)) ? "\"" : NullStr,
                      dcd->arcname,
                      (needs_quoting(dcd->arcname)) ? "\"" : NullStr);
            if(SHORT1FROMMP(mp1) == IDM_ARCEXTRACTEXIT)
              PostMsg(hwnd,
                      WM_CLOSE,
                      MPVOID,
                      MPVOID);
            break;

          case IDM_ARCEXTRACTWDIRSEXIT:
          case IDM_ARCEXTRACTWDIRS:
            if(dcd->info->exwdirs)
              runemf2(SEPARATE | WINDOWED |
                      ((fArcStuffVisible) ? 0 : (BACKGROUND | MINIMIZED)) ,
                      hwnd,dcd->directory,NULL,"%s %s%s%s",
                      dcd->info->exwdirs,
                      (needs_quoting(dcd->arcname)) ? "\"" : NullStr,
                      dcd->arcname,
                      (needs_quoting(dcd->arcname)) ? "\"" : NullStr);
            if(SHORT1FROMMP(mp1) == IDM_ARCEXTRACTWDIRSEXIT)
              PostMsg(hwnd,
                      WM_CLOSE,
                      MPVOID,
                      MPVOID);
            break;

          case IDM_RESORT:
            WinSendMsg(hwnd,
                       CM_SORTRECORD,
                       MPFROMP(ArcSort),
                       MPFROMP(dcd));
            break;

          case IDM_FILTER:
            {
              BOOL     empty = FALSE;
              PARCITEM pci;

              if(!*dcd->mask.szMask) {
                empty = TRUE;
                pci = (PARCITEM)CurrentRecord(hwnd);
                if(pci && strchr(pci->szFileName,'.'))
                  strcpy(dcd->mask.szMask,pci->szFileName);
              }

              if(WinDlgBox(HWND_DESKTOP,hwnd,PickMaskDlgProc,
                           FM3ModHandle,MSK_FRAME,MPFROMP(&dcd->mask))) {
                WinSendMsg(hwnd,CM_FILTER,MPFROMP(ArcFilter),
                           MPFROMP(dcd));
                PostMsg(hwnd,UM_RESCAN,MPVOID,MPVOID);
              }
              else if(empty)
                *dcd->mask.szMask = 0;
              SayFilter(WinWindowFromID(WinQueryWindow(hwnd,QW_PARENT),
                        DIR_FILTER),&dcd->mask,TRUE);
            }
            break;

          case IDM_SWITCH:
            if(mp2) {
              if(stricmp(dcd->directory,(CHAR *)mp2)) {
                DosEnterCritSec();
                 strcpy(lastextractpath,(CHAR *)mp2);
                 MakeValidDir(lastextractpath);
                DosExitCritSec();
              }
              strcpy(dcd->directory,(CHAR *)mp2);
              MakeValidDir(dcd->directory);
              WinSetWindowText(dcd->hwndExtract,dcd->directory);
            }
            break;

          case IDM_WALKDIR:
            {
              CHAR newdir[CCHMAXPATH];

              strcpy(newdir,dcd->directory);
              if(!WinDlgBox(HWND_DESKTOP,dcd->hwndParent,WalkExtractDlgProc,
                            FM3ModHandle,WALK_FRAME,
                            MPFROMP(newdir)) || !*newdir)
                break;
              if(stricmp(newdir,dcd->directory)) {
                strcpy(dcd->directory,newdir);
                if(stricmp(lastextractpath,newdir))
                  strcpy(lastextractpath,newdir);
                WinSetWindowText(dcd->hwndExtract,dcd->directory);
              }
            }
            break;

          case IDM_TEST:
            if(dcd->info->test)
              runemf2(SEPARATEKEEP | WINDOWED | MAXIMIZED,
                      hwnd,NULL,NULL,"%s %s%s%s",dcd->info->test,
                      (needs_quoting(dcd->arcname)) ? "\"" : NullStr,
                      dcd->arcname,
                      (needs_quoting(dcd->arcname)) ? "\"" : NullStr);
            break;

          case IDM_REFRESH:
          case IDM_DELETE:
          case IDM_PRINT:
          case IDM_VIEW:
          case IDM_VIEWTEXT:
          case IDM_VIEWBINARY:
          case IDM_VIEWARCHIVE:
          case IDM_EDIT:
          case IDM_EDITTEXT:
          case IDM_EDITBINARY:
          case IDM_EXTRACT:
          case IDM_EXTRACTWDIRS:
          case IDM_FIND:
          case IDM_EXEC:
          case IDM_VIRUSSCAN:
            {
              LISTINFO *li;

              li = malloc(sizeof(LISTINFO));
              if(li) {
                memset(li,0,sizeof(LISTINFO));
                li->type = SHORT1FROMMP(mp1);
                li->hwnd = hwnd;
                li->list = BuildArcList(hwnd);
                if(li->type == IDM_REFRESH) {

                  CHAR s[CCHMAXPATH],*p;
                  INT  x,y;

                  for(x = 0;li->list && li->list[x];x++) {
                    sprintf(s,"%s%s%s",dcd->workdir,
                            (dcd->workdir[strlen(dcd->workdir) - 1] == '\\') ?
                            NullStr : "\\",li->list[x]);
                    if(IsFile(s) != 1) {
                      free(li->list[x]);
                      li->list[x] = NULL;
                      for(y = x;li->list[y];y++)
                        li->list[y] = li->list[y + 1];
                      li->list = realloc(li->list,y * sizeof(CHAR *));
                      x--;
                    }
                    else {
                      p = strdup(s);
                      if(p) {
                        free(li->list[x]);
                        li->list[x] = p;
                      }
                    }
                  }
                }
                strcpy(li->arcname,dcd->arcname);
                li->info = dcd->info;
                {
                  PARCITEM pai;

                  if(SHORT1FROMMP(mp1) != IDM_EXEC)
                    pai = (PARCITEM)CurrentRecord(hwnd);
                  else
                    pai = (PARCITEM)WinSendMsg(hwnd,CM_QUERYRECORDEMPHASIS,
                                               MPFROMLONG(CMA_FIRST),
                                               MPFROMSHORT(CRA_CURSORED));
                  if(pai && (INT)pai != -1)
                    strcpy(li->runfile,pai->szFileName);
                  else
                    strcpy(li->runfile,li->list[0]);
                }
                switch(SHORT1FROMMP(mp1)) {
                  case IDM_VIEW:
                  case IDM_VIEWTEXT:
                  case IDM_VIEWBINARY:
                  case IDM_VIEWARCHIVE:
                  case IDM_EDIT:
                  case IDM_EDITTEXT:
                  case IDM_EDITBINARY:
                  case IDM_EXEC:
                  case IDM_PRINT:
                  case IDM_VIRUSSCAN:
                    strcpy(li->targetpath,dcd->workdir);
                    break;
                  default:
                    strcpy(li->targetpath,dcd->directory);
                    break;
                }
                if(li->list) {
                  if(!PostMsg(dcd->hwndObject,UM_ACTION,MPFROMP(li),
                                 MPVOID)) {
                    FreeListInfo(li);
                    DosBeep(50,100);
                  }
                  else if(fUnHilite && SHORT1FROMMP(mp1) != IDM_EDIT)
                    UnHilite(hwnd,TRUE,&dcd->lastselection);
                }
                else
                  free(li);
              }
            }
            break;
        }
      }
      return 0;

    case WM_CONTROL:
      DosError(FERR_DISABLEHARDERR);
      if(dcd) {
        switch(SHORT2FROMMP(mp1)) {
          case CN_BEGINEDIT:
            PostMsg(hwnd,
                    CM_CLOSEEDIT,
                    MPVOID,
                    MPVOID);
            break;

          case CN_ENDEDIT:
            if(!((PCNREDITDATA)mp2)->pRecord) {

              PFIELDINFO pfi = ((PCNREDITDATA)mp2)->pFieldInfo;
              USHORT     cmd = 0;

              if(!pfi || pfi->offStruct == FIELDOFFSET(ARCITEM,pszFileName))
                cmd = IDM_SORTSMARTNAME;
              else if(pfi->offStruct == FIELDOFFSET(ARCITEM,cbFile))
                cmd = IDM_SORTSIZE;
              else if(pfi->offStruct == FIELDOFFSET(ARCITEM,cbComp))
                cmd = IDM_SORTEASIZE;
              else if(pfi->offStruct == FIELDOFFSET(ARCITEM,date))
                cmd = IDM_SORTLWDATE;
              else if(pfi->offStruct == FIELDOFFSET(ARCITEM,time))
                cmd = IDM_SORTLWDATE;
              if(cmd)
                PostMsg(hwnd,
                        WM_COMMAND,
                        MPFROM2SHORT(cmd,0),
                        MPVOID);
            }
            break;

          case CN_DROPHELP:
            saymsg(MB_ENTER,hwnd,
                   GetPString(IDS_DROPHELPHDRTEXT),
                   GetPString(IDS_ARCCNRDROPHELPTEXT),
                   dcd->arcname);
            return 0;

          case CN_DRAGLEAVE:
            if(mp2) {

              PDRAGINFO pDInfo;

              pDInfo = ((PCNRDRAGINFO)mp2)->pDragInfo;
              DrgAccessDraginfo(pDInfo);             /* Access DRAGINFO       */
              DrgFreeDraginfo(pDInfo);               /* Free DRAGINFO         */
            }
            return 0;

          case CN_DRAGAFTER:
          case CN_DRAGOVER:
            if(mp2) {

              PDRAGITEM pDItem;                      /* Pointer to DRAGITEM   */
              PDRAGINFO pDInfo;                      /* Pointer to DRAGINFO   */
              PARCITEM  pci;

              pci = (PARCITEM)((PCNRDRAGINFO)mp2)->pRecord;
              if(SHORT1FROMMP(mp1) == CN_DRAGAFTER)
                pci = NULL;
              pDInfo = ((PCNRDRAGINFO)mp2)->pDragInfo;
              DrgAccessDraginfo(pDInfo);             /* Access DRAGINFO       */
              if(*dcd->arcname) {
                if((driveflags[toupper(*dcd->arcname) - 'A'] & DRIVE_NOTWRITEABLE) ||
                   !dcd->info ||
                   !dcd->info->create) {
                  DrgFreeDraginfo(pDInfo);
                  return MRFROM2SHORT(DOR_NEVERDROP,0);
                }
              }
              if(pci) {
                DrgFreeDraginfo(pDInfo);
                return MRFROM2SHORT(DOR_NODROP,0);
              }
              pDItem = DrgQueryDragitemPtr(pDInfo,   /* Access DRAGITEM       */
                                           0);       /* Index to DRAGITEM     */
              if(DrgVerifyRMF(pDItem,                /* Check valid rendering */
                              DRM_OS2FILE,           /* mechanisms and data   */
                              NULL) &&
                 !(pDItem->fsControl & DC_PREPARE)) {
                DrgFreeDraginfo(pDInfo);             /* Free DRAGINFO         */
                return(MRFROM2SHORT(DOR_DROP,        /* Return okay to drop   */
                                    ((fCopyDefault) ?
                                     DO_COPY : DO_MOVE)));
              }
              DrgFreeDraginfo(pDInfo);               /* Free DRAGINFO         */
            }
            return(MRFROM2SHORT(DOR_NEVERDROP,0)); /* Drop not valid        */

          case CN_INITDRAG:
            if(mp2) {

              BOOL         wasemphasized = FALSE;
              PCNRDRAGINIT pcd = (PCNRDRAGINIT)mp2;
              PARCITEM  pci;

              if(pcd) {
                pci = (PARCITEM)pcd->pRecord;
                if(pci) {
                  if(pci->rc.flRecordAttr & CRA_SELECTED)
                    wasemphasized = TRUE;
                  if(!ParentIsDesktop(hwnd,dcd->hwndParent) &&
                     fSplitStatus && hwndStatus2)
                    WinSetWindowText(hwndStatus2,
                                     GetPString(IDS_DRAGARCMEMTEXT));
                  if(DoFileDrag(hwnd,
                                dcd->hwndObject,
                                mp2,
                                dcd->arcname,
                                NULL,
                                TRUE)) {
                    if(fUnHilite && wasemphasized)
                      UnHilite(hwnd,TRUE,&dcd->lastselection);
                  }
                  if(!ParentIsDesktop(hwnd,dcd->hwndParent) &&
                     fSplitStatus && hwndStatus2)
                    PostMsg(hwnd,UM_RESCAN,MPVOID,MPVOID);
                }
                else {
                  if(!ParentIsDesktop(hwnd,dcd->hwndParent) &&
                     fSplitStatus && hwndStatus2)
                    WinSetWindowText(hwndStatus2,
                                     GetPString(IDS_DRAGARCFILETEXT));
                  DragOne(hwnd,
                          dcd->hwndObject,
                          dcd->arcname,
                          FALSE);
                  if(!ParentIsDesktop(hwnd,dcd->hwndParent) &&
                     fSplitStatus && hwndStatus2)
                    PostMsg(hwnd,UM_RESCAN,MPVOID,MPVOID);
                }
              }
            }
            return 0;

          case CN_DROP:
            if(mp2) {

              LISTINFO *li;

DosBeep(500,100);
              li = DoFileDrop(hwnd,
                              dcd->arcname,
                              FALSE,
                              mp1,
                              mp2);
DosBeep(50,100);
              if(li) {
                li->type = (li->type == DO_MOVE) ?
                            IDM_ARCHIVEM :
                            IDM_ARCHIVE;
                strcpy(li->targetpath,dcd->arcname);
                if(!li->list ||
                   !li->list[0] ||
                   !PostMsg(dcd->hwndObject,
                            UM_ACTION,
                            MPFROMP(li),
                            MPVOID))
                  FreeListInfo(li);
              }
            }
            return 0;

          case CN_CONTEXTMENU:
            {
              PARCITEM pci = (PARCITEM)mp2;

              if(pci) {
                WinSendMsg(hwnd,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                           MPFROM2SHORT(TRUE,CRA_CURSORED));
                MarkAll(hwnd,FALSE,FALSE,TRUE);
                dcd->hwndLastMenu = CheckMenu(&ArcMenu,ARC_POPUP);
              }
              else {
                dcd->hwndLastMenu = CheckMenu(&ArcCnrMenu,ARCCNR_POPUP);
                if(dcd->hwndLastMenu && !dcd->cnremphasized) {
                  WinSendMsg(hwnd,CM_SETRECORDEMPHASIS,MPVOID,
                             MPFROM2SHORT(TRUE,CRA_SOURCE));
                  dcd->cnremphasized = TRUE;
                }
              }
              if(dcd->hwndLastMenu) {
                if(dcd->hwndLastMenu == ArcCnrMenu) {
                  if(dcd->flWindowAttr & CV_MINI)
                    WinCheckMenuItem(dcd->hwndLastMenu,IDM_MINIICONS,TRUE);
                }
                WinCheckMenuItem(dcd->hwndLastMenu,IDM_FOLDERAFTEREXTRACT,
                                 fFolderAfterExtract);
                if(!PopupMenu(hwnd,hwnd,dcd->hwndLastMenu)) {
                  if(dcd->cnremphasized) {
                    WinSendMsg(hwnd,CM_SETRECORDEMPHASIS,MPVOID,
                               MPFROM2SHORT(FALSE,CRA_SOURCE));
                    dcd->cnremphasized = TRUE;
                  }
                  MarkAll(hwnd,TRUE,FALSE,TRUE);
                }
              }
            }
            break;

          case CN_EMPHASIS:
            if(mp2) {

              PNOTIFYRECORDEMPHASIS pre = mp2;
              PARCITEM              pci;
              CHAR                  s[CCHMAXPATHCOMP + 91],tf[81],tb[81],*tbk;

              pci = (PARCITEM)((pre) ? pre->pRecord : NULL);
              if(!pci) {
                if(!ParentIsDesktop(hwnd,dcd->hwndParent)) {
                  if(hwndStatus2)
                    WinSetWindowText(hwndStatus2,NullStr);
                  if(fMoreButtons)
                    WinSetWindowText(hwndName,NullStr);
                }
                break;
              }
              if(pre->fEmphasisMask & CRA_SELECTED) {
                if(pci->rc.flRecordAttr & CRA_SELECTED) {
                  dcd->selectedbytes += pci->cbFile;
                  dcd->selectedfiles++;
                }
                else if(dcd->selectedfiles) {
                  dcd->selectedbytes -= pci->cbFile;
                  dcd->selectedfiles--;
                }
                commafmt(tf,sizeof(tf),dcd->selectedfiles);
                *tb = 0;
                if(dcd->totalbytes) {
                  if(dcd->selectedbytes > 1024) {
                    tbk = "k";
                    commafmt(tb,sizeof(tb),dcd->selectedbytes / 1024L);
                  }
                  else {
                    tbk = "b";
                    commafmt(tb,sizeof(tb),dcd->selectedbytes);
                  }
                }
                sprintf(s,"%s%s%s%s",tf,(*tb) ? " / " : NullStr,tb,
                        (*tb) ? tbk : NullStr);
                WinSetDlgItemText(dcd->hwndClient,DIR_SELECTED,s);
              }
              else if(WinQueryActiveWindow(dcd->hwndParent) ==
                      dcd->hwndFrame &&
                      !ParentIsDesktop(hwnd,dcd->hwndParent)) {
                if(pre->fEmphasisMask & CRA_CURSORED) {
                  if(pci->rc.flRecordAttr & CRA_CURSORED) {
                    if(fSplitStatus && hwndStatus2) {
                      *tb = 0;
                      if(dcd->totalbytes)
                        commafmt(tb,sizeof(tb),pci->cbFile);
                      sprintf(s,"%s%s%s%s",(*tb) ? " " : NullStr,tb,
                              (*tb) ? "b  " : NullStr,
                              pci->szFileName);
                      WinSetWindowText(hwndStatus2,s);
                    }
                    if(fMoreButtons)
                      WinSetWindowText(hwndName,pci->szFileName);
                  }
                }
              }
            }
            break;

          case CN_ENTER:
            if(mp2) {

              PARCITEM pci = (PARCITEM)((PNOTIFYRECORDENTER)mp2)->pRecord;

              if(pci) {

                CHAR *s;

                if((pci->rc.flRecordAttr & CRA_INUSE) ||
                   (pci->flags & (ARCFLAGS_REALDIR | ARCFLAGS_PSEUDODIR)))
                  break;
                s = strdup(pci->szFileName);
                if(s) {
                  if(!PostMsg(dcd->hwndObject,UM_ENTER,
                                 MPFROMP(s),MPVOID)) {
                    free(s);
                    DosBeep(50,100);
                  }
                }
                else
                  DosBeep(50,100);
              }
            }
            break;
        }
      }
      return 0;

    case UM_FOLDUP:
      if(!PostMsg((HWND)0,
                  WM_QUIT,
                  MPVOID,
                  MPVOID))
        DosExit(EXIT_PROCESS,1);
      return 0;

    case UM_CLOSE:
      WinDestroyWindow(WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),
                       QW_PARENT));
      return 0;

    case WM_SAVEAPPLICATION:
      if(dcd &&
         ParentIsDesktop(hwnd,dcd->hwndParent)) {

        SWP swp;

        WinQueryWindowPos(dcd->hwndFrame,&swp);
        if(!(swp.fl & (SWP_HIDE | SWP_MINIMIZE | SWP_MAXIMIZE)))
          PrfWriteProfileData(fmprof,
                              appname,
                              "AV2SizePos",
                              &swp,
                              sizeof(swp));
      }
      break;

    case WM_CLOSE:
      WinSendMsg(hwnd,
                 WM_SAVEAPPLICATION,
                 MPVOID,
                 MPVOID);
      if(dcd)
        dcd->stopflag++;
      if(dcd && dcd->hwndObject) {
        if(!PostMsg(dcd->hwndObject,
                    WM_CLOSE,
                    MPVOID,
                    MPVOID))
          WinSendMsg(dcd->hwndObject,
                     WM_CLOSE,
                     MPVOID,
                     MPVOID);
      }
      dcd = WinQueryWindowPtr(hwnd,0);
      if(!dcd ||
         (!dcd->dontclose &&
          !dcd->amextracted &&
          ParentIsDesktop(hwnd,dcd->hwndParent))) {
        if(!PostMsg(hwnd,
                    UM_FOLDUP,
                    MPVOID,
                    MPVOID))
          WinSendMsg(hwnd,
                     UM_FOLDUP,
                     MPVOID,
                     MPVOID);
      }
      return 0;

    case WM_DESTROY:
      if(ArcMenu)
        WinDestroyWindow(ArcMenu);
      if(ArcCnrMenu)
        WinDestroyWindow(ArcCnrMenu);
      ArcMenu = ArcCnrMenu = (HWND)0;
      EmptyCnr(hwnd);
      break;
  }
  return (dcd && dcd->oldproc) ? dcd->oldproc(hwnd,msg,mp1,mp2) :
                                 PFNWPCnr(hwnd,msg,mp1,mp2);
}


HWND StartArcCnr (HWND hwndParent,HWND hwndCaller,CHAR *arcname,INT flags,
                  ARC_TYPE *sinfo) {

  /*
   * bitmapped flags:
   *  1 = am extracted from another archive
   *  4 = don't kill proc on close
   */

  HWND          hwndFrame = (HWND)0,hwndClient;
  ULONG         FrameFlags = FCF_TITLEBAR   | FCF_SYSMENU     |
                             FCF_SIZEBORDER | FCF_MINMAX      |
                             FCF_ICON       | FCF_NOBYTEALIGN |
                             FCF_ACCELTABLE;
  USHORT        id;
  DIRCNRDATA   *dcd;
  ARC_TYPE     *info = sinfo;
  CHAR          fullname[CCHMAXPATH],*p,temp;
  static USHORT idinc = 0;

  if(!idinc)
    idinc = (rand() % 256);
  if(ParentIsDesktop(hwndParent,hwndParent))
    FrameFlags |= (FCF_TASKLIST | FCF_MENU);
  if(arcname) {
    DosError(FERR_DISABLEHARDERR);
    if(DosQueryPathInfo(arcname,
                        FIL_QUERYFULLNAME,
                        fullname,
                        sizeof(fullname)))
      strcpy(fullname,arcname);
    p = fullname;
    while(*p) {
      if(*p == '/')
        *p = '\\';
      p++;
    }
    if(!info)
      info = find_type(fullname,
                       arcsighead);
    if(!info)
      return hwndFrame;
    hwndFrame = WinCreateStdWindow(hwndParent,
                                   WS_VISIBLE,
                                   &FrameFlags,
                                   GetPString(IDS_WCARCCONTAINER),
                                   NULL,
                                   WS_VISIBLE | fwsAnimate,
                                   FM3ModHandle,
                                   ARC_FRAME,
                                   &hwndClient);
    if(hwndFrame && hwndClient) {
      id = ARC_FRAME + idinc++;
      if(idinc > 512)
        idinc = 0;
      WinSetWindowUShort(hwndFrame,QWS_ID,id);
      dcd = malloc(sizeof(DIRCNRDATA));
      if(dcd) {
        memset(dcd,0,sizeof(DIRCNRDATA));
        dcd->size = sizeof(DIRCNRDATA);
        dcd->id = id;
        dcd->type = ARC_FRAME;
        save_dir2(dcd->workdir);
        if(dcd->workdir[strlen(dcd->workdir) - 1] != '\\')
          strcat(dcd->workdir,"\\");
        sprintf(dcd->workdir + strlen(dcd->workdir),"%s.%03x",
                ArcTempRoot,(clock() & 4095));
        strcpy(dcd->arcname,fullname);
        if(*extractpath) {
          if(!strcmp(extractpath,"*")) {
            p = strrchr(fullname,'\\');
            if(p) {
              if(p < fullname + 3)
                p++;
              temp = *p;
              *p = 0;
              strcpy(dcd->directory,fullname);
              *p = temp;
            }
          }
          else
            strcpy(dcd->directory,extractpath);
        }
        if(!*dcd->directory &&
           *lastextractpath) {
          DosEnterCritSec();
           strcpy(dcd->directory,lastextractpath);
          DosExitCritSec();
        }
        if(!*dcd->directory) {
          if(!ParentIsDesktop(hwndParent,hwndParent))
            TopWindowName(hwndParent,
                          hwndCaller,
                          dcd->directory);
          if(!*dcd->directory) {
            p = strrchr(fullname,'\\');
            if(p) {
              if(p < fullname + 3)
                p++;
              *p = 0;
              strcpy(dcd->directory,fullname);
            }
          }
        }
        if(!*dcd->directory ||
           IsFile(dcd->directory) ||
           (isalpha(*dcd->directory) &&
            (driveflags[toupper(*dcd->directory) - 'A'] & DRIVE_NOTWRITEABLE)))
          save_dir2(dcd->directory);
        dcd->hwndParent = (hwndParent) ? hwndParent :
                                         HWND_DESKTOP;
        dcd->hwndFrame  = hwndFrame;
        dcd->hwndClient = hwndClient;
        dcd->amextracted = ((flags & 1) != 0);
        dcd->dontclose = ((flags & 4) != 0);
        dcd->info = info;
        {
          PFNWP oldproc;

          oldproc = WinSubclassWindow(hwndFrame,
                                      (PFNWP)ArcFrameWndProc);
          WinSetWindowPtr(hwndFrame,
                          0,
                          (PVOID)oldproc);
        }
        dcd->hwndCnr = WinCreateWindow(hwndClient,
                                       WC_CONTAINER,
                                       NULL,
                                       CCS_AUTOPOSITION | CCS_MINIICONS |
                                       CCS_MINIRECORDCORE | ulCnrType |
                                       WS_VISIBLE,
                                       0,
                                       0,
                                       0,
                                       0,
                                       hwndClient,
                                       HWND_TOP,
                                       (ULONG)ARC_CNR,
                                       NULL,
                                       NULL);
        if(dcd->hwndCnr) {
          WinSetWindowPtr(dcd->hwndCnr,
                          0,
                          (PVOID)dcd);
          {
            CHAR s[CCHMAXPATH + 8];

            sprintf(s,
                    "AV/2: %s",
                    dcd->arcname);
            WinSetWindowText(hwndFrame,s);
            WinSetWindowText(WinWindowFromID(hwndFrame,
                                             FID_TITLEBAR),
                             s);
          }
          dcd->oldproc = WinSubclassWindow(dcd->hwndCnr,
                                           (PFNWP)ArcCnrWndProc);
          {
            USHORT ids[] = {DIR_TOTALS,DIR_SELECTED,DIR_VIEW,DIR_SORT,
                            DIR_FILTER,DIR_FOLDERICON,0};

            CommonCreateTextChildren(dcd->hwndClient,
                                     GetPString(IDS_WCARCSTATUS),
                                     ids);
          }
          WinEnableWindow(WinWindowFromID(dcd->hwndClient,DIR_VIEW),
                          FALSE);
          dcd->hwndExtract = WinCreateWindow(dcd->hwndClient,
                                             WC_ENTRYFIELD,
                                             NULL,
                                             ES_AUTOSCROLL,
                                             0,
                                             0,
                                             0,
                                             0,
                                             dcd->hwndClient,
                                             HWND_TOP,
                                             ARC_EXTRACTDIR,
                                             NULL,
                                             NULL);
          WinSendMsg(dcd->hwndExtract,
                     EM_SETTEXTLIMIT,
                     MPFROM2SHORT(CCHMAXPATH,0),
                     MPVOID);
          WinSetWindowText(dcd->hwndExtract,
                           dcd->directory);
          if(!PostMsg(dcd->hwndCnr,
                      UM_SETUP,
                      MPVOID,
                      MPVOID))
            WinSendMsg(dcd->hwndCnr,
                       UM_SETUP,
                       MPVOID,
                       MPVOID);
          if(FrameFlags & FCF_MENU) {
            if(!fToolbar) {

              HWND hwndMenu = WinWindowFromID(hwndFrame,FID_MENU);

              if(hwndMenu) {
                WinSendMsg(hwndMenu,MM_DELETEITEM,
                           MPFROM2SHORT(IDM_VIEW,FALSE),
                           MPVOID);
                WinSendMsg(hwndMenu,MM_DELETEITEM,
                           MPFROM2SHORT(IDM_EXEC,FALSE),
                           MPVOID);
                WinSendMsg(hwndMenu,MM_DELETEITEM,
                           MPFROM2SHORT(IDM_RESCAN,FALSE),
                           MPVOID);
                WinSendMsg(hwndMenu,MM_DELETEITEM,
                           MPFROM2SHORT(IDM_DELETE,FALSE),
                           MPVOID);
                WinSendMsg(hwndMenu,MM_DELETEITEM,
                           MPFROM2SHORT(IDM_EXTRACT,FALSE),
                           MPVOID);
                WinSendMsg(hwndMenu,MM_DELETEITEM,
                           MPFROM2SHORT(IDM_TEST,FALSE),
                           MPVOID);
                WinSendMsg(hwndMenu,MM_DELETEITEM,
                           MPFROM2SHORT(IDM_VIRUSSCAN,FALSE),
                           MPVOID);
                WinSendMsg(hwndMenu,MM_DELETEITEM,
                           MPFROM2SHORT(IDM_WALKDIR,FALSE),
                           MPVOID);
                WinSendMsg(hwndMenu,MM_DELETEITEM,
                           MPFROM2SHORT(IDM_FILTER,FALSE),
                           MPVOID);
              }
            }
          }
          if(FrameFlags & FCF_TASKLIST) {

            SWP   swp,swpD;
            ULONG size = sizeof(swp);
            LONG  cxScreen,cyScreen;

            WinQueryTaskSizePos(WinQueryAnchorBlock(hwndFrame),0,&swp);
            if(PrfQueryProfileData(fmprof,
                                   appname,
                                   "AV2SizePos",
                                   &swpD,
                                   &size)) {
              cxScreen = WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);
              cyScreen = WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN);
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
        }
        else {
          PostMsg(hwndClient,
                  WM_CLOSE,
                  MPVOID,
                  MPVOID);
          free(dcd);
          hwndFrame = (HWND)0;
        }
      }
      else {
        PostMsg(hwndClient,
                WM_CLOSE,
                MPVOID,
                MPVOID);
        hwndFrame = (HWND)0;
      }
    }
  }
  return hwndFrame;
}

