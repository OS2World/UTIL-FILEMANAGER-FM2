
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  Archive containers

  Revisions	31 Jul 04 SHL - ArcReviewDlgProc: correct nameis... decodes

***********************************************************************/

#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "version.h"
#include "fm3str.h"

#pragma data_seg(DATA1)
#pragma alloc_text(AVV,EditArchiverData,assign,reassign_from_window)
#pragma alloc_text(AVV,get_int_from_window,get_int2_from_window)
#pragma alloc_text(AVV,get_long_from_window,get_int3_from_window)
#pragma alloc_text(AVV,get_int4_from_window)


APIRET EditArchiverData (HWND hwnd,DIRCNRDATA *arc) {

  ARCDUMP ad;
  FILE   *fp;
  HFILE   oldstdout,newstdout;
  APIRET  ret;
  CHAR    arctemp[33];

  if(!arc)
    return 0;
  if(!arc->info) {
    if(!WinDlgBox(HWND_DESKTOP,
                  hwnd,
                  SBoxDlgProc,
                  FM3ModHandle,
                  ASEL_FRAME,
                  (PVOID)&arc->info) ||
                  !arc->info) /* we blew it */
      return 0;
  }
  if(*arc->arcname) {
    sprintf(arctemp,
            "%s.%03x",
            ArcTempRoot,
            (clock() & 4095L));
    DosForceDelete(arctemp);
    fp = fopen(arctemp,"w");
    if(fp) {
      newstdout = -1;
      if(DosDupHandle(fileno(stdout),&newstdout))
        DosBeep(50,100);
      oldstdout = fileno(stdout);
      DosDupHandle(fileno(fp),&oldstdout);
      runemf2(SEPARATE | INVISIBLE | FULLSCREEN | BACKGROUND | WAIT,
             HWND_DESKTOP,
             NullStr,
             NULL,
             "%s %s%s%s",
             arc->info->list,
             (strchr(arc->arcname,' ')) ? "\"" : NullStr,
             arc->arcname,
             (strchr(arc->arcname,' ')) ? "\"" : NullStr);
      oldstdout = fileno(stdout);
      DosDupHandle(newstdout,&oldstdout);
      DosClose(newstdout);
      fclose(fp);
    }
  }
  memset(&ad,0,sizeof(ARCDUMP));
  ad.info = arc->info;
  if(*arc->arcname) {
    strcpy(ad.listname,arctemp);
    strcpy(ad.arcname,arc->arcname);
  }
  ret = WinDlgBox(HWND_DESKTOP,
                  hwnd,
                  ArcReviewDlgProc,
                  FM3ModHandle,
                  AD_FRAME,
                  MPFROMP(&ad));
  if(*arc->arcname)
    DosForceDelete(arctemp);
  return ret;
}


CHAR * assign (CHAR *a,CHAR *b) {

  if(a)
    free(a);
  if(b &&
     *b)
    a = strdup(b);
  else
    a = NULL;
  return a;
}


CHAR * reassign_from_window (HWND hwnd,USHORT id,char *a) {

  char s[257] = "";

  WinQueryDlgItemText(hwnd,
                      id,
                      255,
                      s);
  a = assign(a,s);
  return a;
}


INT get_int_from_window (HWND hwnd,USHORT id) {

  char s[257] = "";

  WinQueryDlgItemText(hwnd,id,255,s);
  return atoi(s);
}


INT get_int2_from_window (HWND hwnd,USHORT id) {

  char s[257] = "",*p;

  WinQueryDlgItemText(hwnd,id,255,s);
  p = strchr(s,',');
  if(p)
    p++;
  return (p) ? atoi(p) : 0;
}


INT get_int3_from_window (HWND hwnd,USHORT id) {

  char s[257] = "",*p;

  WinQueryDlgItemText(hwnd,id,255,s);
  p = strchr(s,',');
  if(p) {
    p++;
    p = strchr(p,',');
    if(p)
      p++;
  }
  return (p) ? atoi(p) : 0;
}


INT get_int4_from_window (HWND hwnd,USHORT id) {

  char s[257] = "",*p;

  WinQueryDlgItemText(hwnd,id,255,s);
  p = strchr(s,',');
  if(p) {
    p++;
    p = strchr(p,',');
    if(p) {
      p++;
      p = strchr(p,',');
      if(p)
        p++;
    }
  }
  return (p) ? atoi(p) : 0;
}


LONG get_long_from_window (HWND hwnd,USHORT id) {

  char s[257] = "";

  WinQueryDlgItemText(hwnd,id,255,s);
  return atol(s);
}


#pragma alloc_text (AVV2,nonull,rewrite_archiverbb2,checkfile,checkfile2)

CHAR * nonull (CHAR *a) {

  if(!a)
    return NullStr;
  return a;
}


VOID rewrite_archiverbb2 (CHAR *archiverbb2) {

  FILE        *fp;
  INT         counter = 0;
  ARC_TYPE    *info;
  static CHAR s[258];
  CHAR        *p;

  if(!arcsighead) {
    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
           HWND_DESKTOP,
           GetPString(IDS_SAYWHATTEXT),
           GetPString(IDS_NOINFOTOSAVETEXT));
    return;
  }
  if(!archiverbb2 || !*archiverbb2) {
    saymsg(MB_ENTER | MB_ICONASTERISK,
           HWND_DESKTOP,
           GetPString(IDS_NOTETEXT),
           GetPString(IDS_SAVEARCBB2TEXT));
    archiverbb2 = GetPString(IDS_ARCHIVERBB2);
  }
  p = strrchr(archiverbb2,'.');   /* save a backup */
  if(p && !stricmp(p,".BB2")) {
    strcpy(p,".BAK");
    DosDelete(archiverbb2);
    strcpy(s,archiverbb2);
    strcpy(p,".BB2");
    DosMove(archiverbb2,s);
  }
  fp = fopen(archiverbb2,"w");
  if(fp) {
    fprintf(fp,"%u\n",NUMLINES);
    fprintf(fp,
            ";\n; %s file written by FM/2 v%d.%02d\n;\n",
            GetPString(IDS_ARCHIVERBB2),
            VERMAJOR,
            VERMINOR);
    fputs(GetPString(IDS_ARCHIVERBB2TEXT),fp);
    info = arcsighead;
    while(info) {
      fprintf(fp,
              GetPString(IDS_ENTRYCNTRTEXT),
              ++counter);
      if(info->id)
        fprintf(fp,
                " (%s)\n;\n",
                info->id);
      fprintf(fp,
              "%s\n%s\n%ld\n%s\n",
              nonull(info->id),
              nonull(info->ext),
              info->file_offset,
              nonull(info->list));
      fprintf(fp,
              "%s\n%s\n%s\n%s\n%s\n%s\n",
              nonull(info->extract),
              nonull(info->exwdirs),
              nonull(info->test),
              nonull(info->create),
              nonull(info->createwdirs),
              nonull(info->createrecurse));
      fprintf(fp,
              "%s\n%s\n%s\n",
              nonull(info->move),
              nonull(info->movewdirs),
              nonull(info->delete));
      fprintf(fp,
              "%s\n%s\n%s\n%d\n%d\n%d,%d\n%d\n%d,%lu,%lu,%lu\n",
              fixup(info->signature,
                    s,
                    255,
                    strlen(info->signature)),
              nonull(info->startlist),
              nonull(info->endlist),
              info->osizepos,
              info->nsizepos,
              info->fdpos,
              info->datetype,
              info->fdflds,
              info->fnpos,
              info->nameislast,
              info->nameisnext,
              info->nameisfirst);
      fprintf(fp,";\n");
      info = info->next;
    }
    fclose(fp);
  }
}


CHAR * checkfile (CHAR *file,INT *error) {

  CHAR  *p,*pp = NULL;
  INT   ret;
  ULONG apptype;

  if(!file || !*file) {
    *error = 3;
    return NULL;
  }
  pp = strchr(file,' ');
  if(pp)
    *pp = 0;
  p = searchpath(file);
  if(!p || !*p)
    *error = 1;
  else {
    ret = (INT)DosQAppType(p,&apptype);
    if(ret)
      *error = -1;
    else {
      apptype &= (~FAPPTYP_32BIT);
      if(!apptype ||
         (apptype == FAPPTYP_NOTWINDOWCOMPAT) ||
         (apptype == FAPPTYP_WINDOWCOMPAT) ||
         (apptype & FAPPTYP_BOUND) ||
         (apptype & FAPPTYP_WINDOWAPI) ||
         (apptype & FAPPTYP_DOS)) {
        *error = 0;
      }
      else
        *error = 2;
    }
  }
  if(pp)
    *pp = ' ';
  return p;
}


ULONG checkfile2 (CHAR *file,INT *error) {

  CHAR  *p,*pp = NULL;
  INT   ret;
  ULONG apptype = 0L;

  if(!file || !*file) {
    *error = 3;
    return apptype;
  }
  pp = strchr(file,' ');
  if(pp)
    *pp = 0;
  p = searchpath(file);
  if(!p || !*p)
    *error = 1;
  else {
    ret = (INT)DosQAppType(p,&apptype);
    if(ret)
      *error = -1;
    else {
      apptype &= (~FAPPTYP_32BIT);
      if(!apptype ||
         (apptype == FAPPTYP_NOTWINDOWCOMPAT) ||
         (apptype == FAPPTYP_WINDOWCOMPAT) ||
         (apptype & FAPPTYP_BOUND) ||
         (apptype & FAPPTYP_WINDOWAPI) ||
         (apptype & FAPPTYP_DOS)) {
        *error = 0;
      }
      else
        *error = 2;
    }
  }
  if(pp)
    *pp = ' ';
  return apptype;
}

#pragma alloc_text (AVV3,checksayfile,check_archiver,ArcReviewDlgProc)

CHAR *checksayfile (HWND hwnd,CHAR *file,INT *error) {

  CHAR *p,*pp = NULL;

  p = checkfile(file,error);
  if(*error) {
    if(p)
      pp = strchr(p,' ');
    if(pp)
      *pp = 0;
    switch(*error) {
      case 1:
        saymsg(MB_ENTER | MB_ICONEXCLAMATION,
               hwnd,
               GetPString(IDS_ERRORCHKARCTEXT),
               GetPString(IDS_PROGNOTFOUNDTEXT),
               (p && *p) ? p : file);
        break;

      case 2:
        saymsg(MB_ENTER | MB_ICONEXCLAMATION,
               hwnd,
               GetPString(IDS_ERRORCHKARCTEXT),
               GetPString(IDS_PROGNOTEXECTEXT),
               (p && *p) ? p : file);
        break;

      case -1:
        saymsg(MB_ENTER | MB_ICONEXCLAMATION,
               hwnd,
               GetPString(IDS_ERRORCHKARCTEXT),
               GetPString(IDS_OS2CHOKECHKTEXT),
               (p && *p) ? p : file);
        break;

      default:
        break;
    }
    if(pp)
      *pp = ' ';
  }
  return p;
}


INT check_archiver (HWND hwnd,ARC_TYPE *info) {

  BOOL fStart = FALSE,fEnd = FALSE,fNpos = FALSE;
  INT  List = 0,Create = 0,Extract = 0;
  static CHAR *aerrors[3];

  aerrors[0] = GetPString(IDS_STARTLISTEMPTYTEXT);
  aerrors[1] = GetPString(IDS_ENDLISTEMPTYTEXT);
  aerrors[2] = GetPString(IDS_BOGUSNAMETEXT);
  if(!info->startlist || !*info->startlist)
    fStart = TRUE;
  if(!info->endlist || !*info->endlist)
    fEnd = TRUE;
  if(info->fnpos > 50 || info->fnpos < -1)
    fNpos = TRUE;
  checkfile(info->list,&List);
  checkfile(info->create,&Create);
  checkfile(info->extract,&Extract);
  if(!fStart && !fEnd && !fNpos && !List && !Create && !Extract)
    return 1;
  saymsg(MB_ENTER | MB_ICONASTERISK,
         hwnd,
         GetPString(IDS_WARNINGSTEXT),
         GetPString(IDS_AVVCHK1TEXT),
         (fStart) ? aerrors[0] : NullStr,
         (fEnd) ? aerrors[1] : NullStr,
         (fNpos) ? aerrors[2] : NullStr,
         (List == 1) ?
          GetPString(IDS_AVVCHK2TEXT) :
          (List == -1) ?
           GetPString(IDS_AVVCHK3TEXT) :
           (List == 2) ?
            GetPString(IDS_AVVCHK4TEXT) :
            (List == 3) ?
             GetPString(IDS_AVVCHK5TEXT) :
             NullStr,
            (Create == 1) ?
             GetPString(IDS_AVVCHK6TEXT) :
             (Create == -1) ?
              GetPString(IDS_AVVCHK7TEXT) :
              (Create == 2) ?
               GetPString(IDS_AVVCHK8TEXT) :
               (Create == 3) ?
                GetPString(IDS_AVVCHK9TEXT) :
                NullStr,
               (Extract == 1) ?
                GetPString(IDS_AVVCHK10TEXT) :
                (Extract == -1) ?
                 GetPString(IDS_AVVCHK11TEXT) :
                 (Extract == 2) ?
                  GetPString(IDS_AVVCHK12TEXT) :
                  (Extract == 3) ?
                   GetPString(IDS_AVVCHK13TEXT) :
                   NullStr);
  if(List || Extract)
    return 0;
  return 1;
}


MRESULT EXPENTRY ArcReviewDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  ARCDUMP        *admp;
  static CHAR    s[258];
  SHORT          sSelect;

  if(msg != WM_INITDLG)
    admp = (ARCDUMP *)WinQueryWindowPtr(hwnd,0);

  switch(msg) {
    case WM_INITDLG:
      admp = (ARCDUMP *)mp2;
      if(!admp || !admp->info) {
        WinDismissDlg(hwnd,0);
        return 0;
      }
      WinSetWindowPtr(hwnd,0,mp2);
      {
        WinSendDlgItemMsg(hwnd,
                          AD_LISTBOX,
                          LM_DELETEALL,
                          MPVOID,
                          MPVOID);
        for(sSelect = AD_ID;sSelect < AD_ADDWPATHS + 1;sSelect++)
          WinSendDlgItemMsg(hwnd,
                            sSelect,
                            EM_SETTEXTLIMIT,
                            MPFROM2SHORT(255,0),
                            MPVOID);
        if(admp->info->id)
          WinSetDlgItemText(hwnd,
                            AD_ID,
                            admp->info->id);
        if(admp->info->ext)
          WinSetDlgItemText(hwnd,
                            AD_EXT,
                            admp->info->ext);
        sprintf(s,
                "%ld",
                admp->info->file_offset);
        WinSetDlgItemText(hwnd,
                          AD_SIGPOS,
                          s);
        if(admp->info->signature)
          WinSetDlgItemText(hwnd,
                            AD_SIG,
                            fixup(admp->info->signature,
                                  s,
                                  255,
                                  strlen(admp->info->signature)));
        if(admp->info->startlist)
          WinSetDlgItemText(hwnd,
                            AD_STARTLIST,
                            admp->info->startlist);
        if(admp->info->endlist)
          WinSetDlgItemText(hwnd,
                            AD_ENDLIST,
                            admp->info->endlist);
        if(admp->info->list)
          WinSetDlgItemText(hwnd,
                            AD_LIST,
                            admp->info->list);
        sprintf(s,
                "%d,%d,%d,%d",
                admp->info->fnpos,
                admp->info->nameislast,
                admp->info->nameisnext,
                admp->info->nameisfirst);
        WinSetDlgItemText(hwnd,AD_FNAMEPOS,s);
        sprintf(s,"%d",admp->info->osizepos);
        WinSetDlgItemText(hwnd,AD_OLDSZ,s);
        sprintf(s,"%d",admp->info->nsizepos);
        WinSetDlgItemText(hwnd,AD_NEWSZ,s);
        sprintf(s,"%d,%d",admp->info->fdpos,admp->info->datetype);
        WinSetDlgItemText(hwnd,AD_DATEPOS,s);
        sprintf(s,"%d",admp->info->fdflds);
        WinSetDlgItemText(hwnd,AD_NUMDATEFLDS,s);
        if(admp->info->extract)
          WinSetDlgItemText(hwnd,AD_EXTRACT,admp->info->extract);
        if(admp->info->exwdirs)
          WinSetDlgItemText(hwnd,AD_WDIRS,admp->info->exwdirs);
        if(admp->info->test)
          WinSetDlgItemText(hwnd,AD_TEST,admp->info->test);
        if(admp->info->create)
          WinSetDlgItemText(hwnd,AD_ADD,admp->info->create);
        if(admp->info->move)
          WinSetDlgItemText(hwnd,AD_MOVE,admp->info->move);
        if(admp->info->delete)
          WinSetDlgItemText(hwnd,AD_DELETE,admp->info->delete);
        if(admp->info->createrecurse)
          WinSetDlgItemText(hwnd,AD_ADDRECURSE,admp->info->createrecurse);
        if(admp->info->createwdirs)
          WinSetDlgItemText(hwnd,AD_ADDWPATHS,admp->info->createwdirs);
        if(admp->info->movewdirs)
          WinSetDlgItemText(hwnd,AD_MOVEWPATHS,admp->info->movewdirs);
      }
      PostMsg(hwnd,
              UM_SETUP,
              MPVOID,
              MPVOID);
      break;


    case UM_SETUP:
      if(admp->listname && *admp->listname) {

        FILE *fp;

        fp = fopen(admp->listname,"r");
        if(!fp) {
          WinSendDlgItemMsg(hwnd,
                            AD_LISTBOX,
                            LM_INSERTITEM,
                            MPFROM2SHORT(LIT_END,0),
                            MPFROMP(GetPString(IDS_CANTOPENFILETEXT)));
        }
        else {
          while(!feof(fp)) {
            if(!fgets(s,256,fp)) break;
            stripcr(s);
            WinSendDlgItemMsg(hwnd,
                              AD_LISTBOX,
                              LM_INSERTITEM,
                              MPFROM2SHORT(LIT_END,0),
                              MPFROMP(s));
          }
          fclose(fp);
        }
      }
      else {
          WinSendDlgItemMsg(hwnd,
                            AD_LISTBOX,
                            LM_INSERTITEM,
                            MPFROM2SHORT(LIT_END,0),
                            MPFROMP(GetPString(IDS_NOTAPPLICABLETEXT)));
      }
      check_archiver(hwnd,admp->info);
      return 0;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,
              UM_SETDIR,
              MPVOID,
              MPVOID);
      break;

    case UM_SETDIR:
      PaintRecessedWindow(WinWindowFromID(hwnd,AD_HELP),
                          (HPS)0,
                          FALSE,
                          TRUE);
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case AD_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case LN_ENTER:
              for(sSelect = 0;sSelect < 10;sSelect++)
                WinSetDlgItemText(hwnd,AD_FLD1 + sSelect,NullStr);
              if(admp->listname) {
                sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                                   AD_LISTBOX,
                                                   LM_QUERYSELECTION,
                                                   MPVOID,
                                                   MPVOID);
                WinSendDlgItemMsg(hwnd,
                                  AD_LISTBOX,
                                  LM_QUERYITEMTEXT,
                                  MPFROM2SHORT(sSelect,255),
                                  MPFROMP(s));
                if(*s) {

                  char *p,*pp;

                  p = s;
                  for(sSelect = 0;sSelect < 10;sSelect++) {
                    pp = p;
                    while(*pp == ' ' || *pp == '\t')
                      pp++;
                    if(!*pp)
                      break;
                    p = pp;
                    while(*p && (*p != ' ' && *p != '\t'))
                      p++;
                    if(*p)
                      *p++ = 0;
                    WinSetDlgItemText(hwnd,
                                      AD_FLD1 + sSelect,
                                      pp);
                  }
                }
                else
                  DosBeep(50,100);
              }
              else
                DosBeep(50,100);
              break;

            case LN_KILLFOCUS:
              WinSetDlgItemText(hwnd,
                                AD_HELP,
                                NullStr);
              break;

            case LN_SETFOCUS:
              WinSetDlgItemText(hwnd,
                                AD_HELP,
                                GetPString(IDS_TEXTARCPRODUCEDTEXT));
              break;
          }
          break;

        case AD_ID:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADIDTEXT));
          break;

        case AD_ADD:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADADDTEXT));
          break;

        case AD_MOVE:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADMOVETEXT));
          break;

        case AD_EXT:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADEXTTEXT));
          break;

        case AD_EXTRACT:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADEXTRACTTEXT));
          break;

        case AD_WDIRS:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADWDIRSTEXT));
          break;

        case AD_SIG:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADSIGTEXT));
          break;

        case AD_LIST:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADLISTTEXT));
          break;

        case AD_TEST:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADTESTTEXT));
          break;

        case AD_ADDWPATHS:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADADDWPATHSTEXT));
          break;

        case AD_MOVEWPATHS:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADMOVEWPATHSTEXT));
          break;

        case AD_ADDRECURSE:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADADDRECURSETEXT));
          break;

        case AD_DELETE:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADDELETETEXT));
          break;

        case AD_SIGPOS:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADSIGPOSTEXT));
          break;

        case AD_FNAMEPOS:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADFNAMEPOSTEXT));
          break;

        case AD_OLDSZ:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADOLDSZTEXT));
          break;

        case AD_NUMDATEFLDS:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADNUMDATEFLDSTEXT));
          break;

        case AD_DATEPOS:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADDATEPOSTEXT));
          break;

        case AD_NEWSZ:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADNEWSZTEXT));
          break;

        case AD_STARTLIST:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADSTARTLISTTEXT));
          break;

        case AD_ENDLIST:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,NullStr);
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,AD_HELP,
                              GetPString(IDS_ADENDLISTTEXT));
          break;

      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case AD_SEEEXTRACTOR:
        case AD_SEEADDER:
          {
            static CHAR tempargs[1026];

            *tempargs = 0;
            if(SHORT1FROMMP(mp1) == AD_SEEADDER)
              WinQueryDlgItemText(hwnd,
                                  AD_ADD,
                                  255,
                                  tempargs);
            else
              WinQueryDlgItemText(hwnd,
                                  AD_EXTRACT,
                                  255,
                                  tempargs);
            if(!*tempargs)
              saymsg(MB_CANCEL,
                     hwnd,
                     GetPString(IDS_BZZZTTEXT),
                     GetPString(IDS_NEEDENTRYTEXT));
            else {

              CHAR *p;

              p = lstrip(tempargs);
              p = strchr(tempargs,' ');
              if(p)
                *p = 0;
              {
                EXECARGS ex;

                ex.flags = SEPARATEKEEP | WINDOWED | MAXIMIZED;
                ex.commandline = tempargs;
                *ex.path = 0;
                *ex.environment = 0;
                if(WinDlgBox(HWND_DESKTOP,
                             hwnd,
                             CmdLineDlgProc,
                             FM3ModHandle,
                             EXEC_FRAME,
                             &ex) == 1)
                  runemf2(ex.flags,
                          hwnd,
                          NULL,
                          (*ex.environment) ? ex.environment : NULL,
                          "%s",
                          tempargs);
              }
            }
          }
          return 0;

        case DID_OK:
          admp->info->startlist = reassign_from_window(hwnd,AD_STARTLIST,admp->info->startlist);
          admp->info->endlist = reassign_from_window(hwnd,AD_ENDLIST,admp->info->endlist);
          admp->info->id = reassign_from_window(hwnd,AD_ID,admp->info->id);
          admp->info->create = reassign_from_window(hwnd,AD_ADD,admp->info->create);
          admp->info->createwdirs = reassign_from_window(hwnd,AD_ADDWPATHS,admp->info->createwdirs);
          admp->info->createrecurse = reassign_from_window(hwnd,AD_ADDRECURSE,admp->info->createrecurse);
          admp->info->movewdirs = reassign_from_window(hwnd,AD_MOVEWPATHS,admp->info->movewdirs);
          admp->info->move = reassign_from_window(hwnd,AD_MOVE,admp->info->move);
          admp->info->delete = reassign_from_window(hwnd,AD_DELETE,admp->info->delete);
          admp->info->test = reassign_from_window(hwnd,AD_TEST,admp->info->test);
          admp->info->extract = reassign_from_window(hwnd,AD_EXTRACT,admp->info->extract);
          admp->info->exwdirs = reassign_from_window(hwnd,AD_WDIRS,admp->info->exwdirs);
          admp->info->ext = reassign_from_window(hwnd,AD_EXT,admp->info->ext);
          literal(reassign_from_window(hwnd,
                                       AD_SIG,
                                       admp->info->signature));
          admp->info->list = reassign_from_window(hwnd,
                                                  AD_LIST,
                                                  admp->info->list);
          admp->info->file_offset = get_long_from_window(hwnd,AD_SIGPOS);
          admp->info->osizepos = get_int_from_window(hwnd,AD_OLDSZ);
          admp->info->nsizepos = get_int_from_window(hwnd,AD_NEWSZ);
          admp->info->fdpos = get_int_from_window(hwnd,AD_DATEPOS);
          admp->info->datetype = get_int2_from_window(hwnd,AD_DATEPOS);
          admp->info->fdflds = get_int_from_window(hwnd,AD_NUMDATEFLDS);
          admp->info->fnpos = get_int_from_window(hwnd,AD_FNAMEPOS);
          admp->info->nameislast = (get_int2_from_window(hwnd,AD_FNAMEPOS)) ? TRUE : FALSE;
          admp->info->nameisnext = (get_int3_from_window(hwnd,AD_FNAMEPOS)) ? TRUE : FALSE;
          admp->info->nameisfirst = (get_int4_from_window(hwnd,AD_FNAMEPOS)) ? TRUE : FALSE;
          {
            INT ret;

            ret = check_archiver(hwnd,admp->info);
            if(saymsg(MB_YESNO,
                      hwnd,
                      GetPString(IDS_ADCHANGESINMEMTEXT),
                      GetPString(IDS_ADREWRITETEXT),
                      (!ret) ?
                       GetPString(IDS_NOTRECOMMENDTEXT) :
                       NullStr) ==
               MBID_YES) {

              CHAR *ab2;

              ab2 = searchpath(GetPString(IDS_ARCHIVERBB2));
              rewrite_archiverbb2(ab2);
            }
          }
          WinDismissDlg(hwnd,TRUE);
          return 0;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_EDITARC,0),
                       MPFROMSHORT(HM_RESOURCEID));
            break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,FALSE);
          return 0;

        case AD_TOSTART:
          if(admp->listname) {
            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                               AD_LISTBOX,
                                               LM_QUERYSELECTION,
                                               MPVOID,
                                               MPVOID);
            WinSendDlgItemMsg(hwnd,
                              AD_LISTBOX,
                              LM_QUERYITEMTEXT,
                              MPFROM2SHORT(sSelect,255),
                              MPFROMP(s));
            if(*s)
              WinSetDlgItemText(hwnd,
                                AD_STARTLIST,
                                s);
            else
BooBoo:
              saymsg(MB_ENTER,
                     hwnd,
                     GetPString(IDS_OOPSTEXT),
                     GetPString(IDS_SELECTFROMLISTTEXT));
          }
          else
            DosBeep(50,100);
          return 0;

        case AD_TOEND:
          if(admp->listname) {
            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                               AD_LISTBOX,
                                               LM_QUERYSELECTION,
                                               MPVOID,
                                               MPVOID);
            WinSendDlgItemMsg(hwnd,
                              AD_LISTBOX,
                              LM_QUERYITEMTEXT,
                              MPFROM2SHORT(sSelect,255),
                              MPFROMP(s));
            if(*s)
              WinSetDlgItemText(hwnd,
                                AD_ENDLIST,
                                s);
            else
              goto BooBoo;
          }
          else
            DosBeep(50,100);
          return 0;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

