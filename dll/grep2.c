
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  Revisions	01 Aug 04 SHL - Rework lstrip/rstrip usage

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSERRORS

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <share.h>
#include <limits.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "grep.h"

#pragma data_seg(DATA1)
#pragma alloc_text(GREP,GrepDlgProc,EnvDlgProc)


MRESULT EXPENTRY EnvDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static CHAR lastenv[CCHMAXPATH] = "DPATH";

  switch(msg) {
    case WM_INITDLG:
      if(mp2) {
        WinSetWindowPtr(hwnd,0,mp2);
        *(CHAR *)mp2 = 0;
        {
          char *p,*pp,temp[CCHMAXPATH];

          p = GetPString(IDS_ENVVARNAMES);
          while(*p == ' ')
            p++;
          while(*p) {
            *temp = 0;
            pp = temp;
            while(*p && *p != ' ')
              *pp++ = *p++;
            *pp = 0;
            while(*p == ' ')
              p++;
            if(*temp)
              WinSendDlgItemMsg(hwnd,
                                ENV_LISTBOX,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                MPFROMP(temp));
          }
        }
        WinSendDlgItemMsg(hwnd,
                          ENV_NAME,
                          EM_SETTEXTLIMIT,
                          MPFROM2SHORT(CCHMAXPATH,0),
                          MPVOID);
        WinSetDlgItemText(hwnd,
                          ENV_NAME,
                          lastenv);
        WinSendDlgItemMsg(hwnd,
                          ENV_NAME,
                          EM_SETSEL,
                          MPFROM2SHORT(0,CCHMAXPATH),
                          MPVOID);
      }
      else
        WinDismissDlg(hwnd,0);
      break;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case ENV_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case LN_SELECT:
              {
                SHORT sSelect;
                CHAR  s[CCHMAXPATH];

                sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                                   ENV_LISTBOX,
                                                   LM_QUERYSELECTION,
                                                   MPFROMSHORT(LIT_FIRST),
                                                   MPVOID);
                if(sSelect >= 0) {
                  *s = 0;
                  WinSendDlgItemMsg(hwnd,
                                    ENV_LISTBOX,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(sSelect,CCHMAXPATH),
                                    MPFROMP(s));
                  bstrip(s);
                  if(*s)
                    WinSetDlgItemText(hwnd,
                                      ENV_NAME,
                                      s);
                }
              }
              break;
            case LN_ENTER:
              PostMsg(hwnd,
                      WM_COMMAND,
                      MPFROM2SHORT(DID_OK,0),
                      MPVOID);
              break;
          }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
        case DID_OK:
          {
            CHAR *p = WinQueryWindowPtr(hwnd,0);

            if(p) {
              WinQueryDlgItemText(hwnd,
                                  ENV_NAME,
                                  CCHMAXPATH,
                                  p);
              bstrip(p);
              if(*p) {
                strcpy(lastenv,p);
                WinDismissDlg(hwnd,1);
              }
              else {
                DosBeep(250,100);
                WinSetFocus(HWND_DESKTOP,
                            WinWindowFromID(hwnd,ENV_NAME));
              }
            }
          }
          break;
        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_ENV,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY GrepDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  static CHAR  lastmask[8192] = "*",lasttext[4096] = "";
  static BOOL  recurse = TRUE,sensitive = FALSE,absolute = FALSE,
               sayfiles = FALSE,searchEAs = TRUE,searchFiles = TRUE,
               changed = FALSE,findifany = TRUE;
  static UINT  newer = 0L,older = 0L;
  static ULONG greater = 0L,lesser = 0L;
  HWND         hwndCollect,hwndMLE = WinWindowFromID(hwnd,GREP_SEARCH);

  switch(msg) {
    case WM_INITDLG:
      if(!mp2) {
        WinDismissDlg(hwnd,0);
        break;
      }
      WinSetWindowULong(hwnd,0,*(HWND *)mp2);
      WinSendDlgItemMsg(hwnd,
                        GREP_MASK,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(8192,0),
                        MPVOID);
      MLEsetlimit(hwndMLE,4096);
      MLEsetformat(hwndMLE,MLFIE_NOTRANS);
      WinSendDlgItemMsg(hwnd,
                        GREP_NEWER,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(34,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        GREP_OLDER,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(34,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        GREP_GREATER,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(34,0),
                        MPVOID);
      WinSendDlgItemMsg(hwnd,
                        GREP_LESSER,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(34,0),
                        MPVOID);
      WinSetDlgItemText(hwnd,
                        GREP_MASK,
                        lastmask);
      WinSendDlgItemMsg(hwnd,
                        GREP_MASK,
                        EM_SETSEL,
                        MPFROM2SHORT(0,8192),
                        MPVOID);
      WinSetWindowText(hwndMLE,lasttext);
      if(*lasttext) {
        MLEsetcurpos(hwndMLE,0);
        MLEsetcurposa(hwndMLE,4096);
        if(!searchEAs)
          searchFiles = TRUE;
      }
      WinCheckButton(hwnd,GREP_RECURSE,recurse);
      WinCheckButton(hwnd,GREP_ABSOLUTE,absolute);
      WinCheckButton(hwnd,GREP_CASE,sensitive);
      WinCheckButton(hwnd,GREP_SAYFILES,sayfiles);
      WinCheckButton(hwnd,GREP_SEARCHEAS,searchEAs);
      WinCheckButton(hwnd,GREP_SEARCHFILES,searchFiles);
      WinCheckButton(hwnd,GREP_FINDIFANY,findifany);
      {
        CHAR s[35];

        sprintf(s,"%lu",greater);
        WinSetDlgItemText(hwnd,GREP_GREATER,s);
        sprintf(s,"%lu",lesser);
        WinSetDlgItemText(hwnd,GREP_LESSER,s);
        sprintf(s,"%u",newer);
        WinSetDlgItemText(hwnd,GREP_NEWER,s);
        sprintf(s,"%u",older);
        WinSetDlgItemText(hwnd,GREP_OLDER,s);
      }
      WinEnableWindow(WinWindowFromID(hwnd,GREP_IGNOREEXTDUPES),FALSE);
      WinEnableWindow(WinWindowFromID(hwnd,GREP_CRCDUPES),FALSE);
      WinEnableWindow(WinWindowFromID(hwnd,GREP_NOSIZEDUPES),FALSE);
      {
        FILE        *fp;
        static CHAR  s[8192 + 14];

        save_dir2(s);
        if(s[strlen(s) - 1] != '\\')
          strcat(s,"\\");
        strcat(s,"GREPMASK.DAT");
        fp = _fsopen(s,"r",SH_DENYWR);
        if(fp) {
          while(!feof(fp)) {
            if(!fgets(s,8192 + 4,fp))
              break;
            bstripcr(s);
            if(*s && *s != ';')
              WinSendDlgItemMsg(hwnd,
                                GREP_LISTBOX,
                                LM_INSERTITEM,
                                MPFROM2SHORT(LIT_SORTASCENDING,0),
                                MPFROMP(s));
          }
          fclose(fp);
        }
      }
      FillPathListBox(hwnd,
                      WinWindowFromID(hwnd,GREP_DRIVELIST),
                      (HWND)0,
                      NULL,
                      FALSE);
      break;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,
              UM_SETDIR,
              MPVOID,
              MPVOID);
      break;

    case UM_SETDIR:
      PaintRecessedWindow(WinWindowFromID(hwnd,GREP_HELP),
                          (HPS)0,
                          FALSE,
                          TRUE);
      return 0;

    case UM_FOCUSME:
      /* set focus to window hwnd in mp1 */
      if(mp1)
        WinSetFocus(HWND_DESKTOP,(HWND)mp1);
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case GREP_DRIVELIST:
          switch(SHORT2FROMMP(mp1)) {
            case LN_KILLFOCUS:
              WinSetDlgItemText(hwnd,
                                GREP_HELP,
                                GetPString(IDS_ARCDEFAULTHELPTEXT));
              break;
            case LN_SETFOCUS:
              WinSetDlgItemText(hwnd,
                                GREP_HELP,
                                GetPString(IDS_2CLICKADDDRVMASKTEXT));
              break;
            case LN_ENTER:
              {
                SHORT sSelect;
                static CHAR s[8192],simple[8192],*p;
                LONG  len;

                WinQueryDlgItemText(hwnd,
                                    GREP_MASK,
                                    8192,
                                    s);
                bstrip(s);
                p = strrchr(s,'\\');
                if(p)
                  strcpy(simple,p);
                else if(*s) {
                  strcpy(simple,"\\");
                  strcat(simple,s);
                  *s = 0;
                }
                else
                  strcpy(simple,"\\*");
                if(simple[strlen(simple) - 1] == ';')
                  simple[strlen(simple) - 1] = 0;
                len = strlen(simple) + 1;
                if(strlen(s) > 8192 - len) {
                  DosBeep(250,100);
                  WinSetDlgItemText(hwnd,
                                    GREP_MASK,
                                    s);
                  break;
                }

                sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                                   GREP_DRIVELIST,
                                                   LM_QUERYSELECTION,
                                                   MPFROMSHORT(LIT_FIRST),
                                                   MPVOID);
                if(sSelect >= 0) {
                  if(*s && s[strlen(s) - 1] != ';')
                    strcat(s,";");
                  WinSendDlgItemMsg(hwnd,
                                    GREP_DRIVELIST,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(sSelect,
                                                 (8192 - strlen(s)) - len),
                                    MPFROMP(&s[strlen(s)]));
                  rstrip(s);
                  if(*s) {
                    strcat(s,simple);
                    WinSetDlgItemText(hwnd,
                                      GREP_MASK,
                                      s);
                    WinSendDlgItemMsg(hwnd,
                                      GREP_MASK,
                                      EM_SETSEL,
                                      MPFROM2SHORT(strlen(s) - (len + 1),
                                                   strlen(s)),
                                      MPVOID);
                    PostMsg(hwnd,
                            UM_FOCUSME,
                            MPFROMLONG(WinWindowFromID(hwnd,GREP_MASK)),
                            MPVOID);
                  }
                }
              }
              break;
          }
          break;
        case GREP_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case LN_KILLFOCUS:
              WinSetDlgItemText(hwnd,
                                GREP_HELP,
                                GetPString(IDS_ARCDEFAULTHELPTEXT));
              break;
            case LN_SETFOCUS:
              WinSetDlgItemText(hwnd,
                                GREP_HELP,
                                GetPString(IDS_ADDSELDELMASKTEXT));
              break;
            case LN_ENTER:
            case LN_SELECT:
              if((SHORT2FROMMP(mp1) == LN_ENTER &&
                 !WinQueryButtonCheckstate(hwnd,GREP_APPEND)) ||
                 (SHORT2FROMMP(mp1) == LN_SELECT &&
                 WinQueryButtonCheckstate(hwnd,GREP_APPEND)))
                break;
              {
                SHORT       sSelect;
                static CHAR s[8192];

                *s = 0;
                sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                                   GREP_LISTBOX,
                                                   LM_QUERYSELECTION,
                                                   MPFROMSHORT(LIT_FIRST),
                                                   MPVOID);
                if(sSelect >= 0) {
                  if(WinQueryButtonCheckstate(hwnd,GREP_APPEND)) {
                    WinQueryDlgItemText(hwnd,
                                        GREP_MASK,
                                        8192,
                                        s);
                    bstrip(s);
                    if(*s && strlen(s) < 8190 && s[strlen(s) - 1] != ';')
                      strcat(s,";");
                  }
                  WinSendDlgItemMsg(hwnd,
                                    GREP_LISTBOX,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(sSelect,8192 - strlen(s)),
                                    MPFROMP(s + strlen(s)));
                  bstrip(s);
                  if(*s)
                    WinSetDlgItemText(hwnd,
                                      GREP_MASK,
                                      s);
                }
              }
              break;
          }
          break;
        case GREP_MASK:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_MASKSFINDTEXT));
          break;
        case GREP_SEARCH:
          if(SHORT2FROMMP(mp1) == MLN_KILLFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == MLN_SETFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_TEXTFINDTEXT));
          break;
        case GREP_GREATER:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_MINSIZEFINDTEXT));
          break;
        case GREP_LESSER:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_MAXSIZEFINDTEXT));
          break;
        case GREP_NEWER:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_MAXAGEFINDTEXT));
          break;
        case GREP_OLDER:
          if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_ARCDEFAULTHELPTEXT));
          if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
            WinSetDlgItemText(hwnd,
                              GREP_HELP,
                              GetPString(IDS_MINAGEFINDTEXT));
          break;
        case GREP_FINDDUPES:
          {
            BOOL finddupes = WinQueryButtonCheckstate(hwnd,GREP_FINDDUPES);

            WinEnableWindow(WinWindowFromID(hwnd,GREP_SEARCH),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_ABSOLUTE),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_CASE),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_CRCDUPES),finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_NOSIZEDUPES),finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_IGNOREEXTDUPES),finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_SEARCHFILES),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_SEARCHEAS),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_GREATER),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_LESSER),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_NEWER),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_OLDER),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_FINDIFANY),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_GK),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_LK),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_NM),!finddupes);
            WinEnableWindow(WinWindowFromID(hwnd,GREP_OM),!finddupes);
            if(finddupes)
              WinCheckButton(hwnd,GREP_RECURSE,TRUE);
          }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case GREP_ENV:
          {
            CHAR          path[CCHMAXPATH];
            CHAR         *p,*t;
            static CHAR   s[8192],simple[8192],env[8192];
            LONG          len;

            *path = 0;
            if (!WinDlgBox(HWND_DESKTOP,
                           hwnd,
                           EnvDlgProc,
                           FM3ModHandle,
                           ENV_FRAME,
                           path)) {
              break;
	    }
            bstrip(path);
            if (!*path)
              break;
            if(!stricmp(path,"LIBPATH"))
              LoadLibPath(env,8192);
            else {
              p = getenv(path);
              if(!p)
                break;
              strcpy(env,p);
            }
            bstrip(env);
            if(!*env)
              break;
            WinQueryDlgItemText(hwnd,
                                GREP_MASK,
                                8192,
                                s);
            bstrip(s);
            if(strlen(s) > 8192 - 5) {
              DosBeep(50,100);
              break;
            }
            p = strrchr(s,'\\');
            if(p)
              strcpy(simple,p + 1);
            else if(*s)
              strcpy(simple,s);
            else
              strcpy(simple,"*");
            if(!p)
              *s = 0;
            if(simple[strlen(simple) - 1] == ';')
              simple[strlen(simple) - 1] = 0;
            len = strlen(simple) + 1;
            p = env;
            while(p && *p) {
              strncpy(path,p,CCHMAXPATH - 1);
              path[CCHMAXPATH - 1] = 0;
              t = strchr(path,';');
              if(t)
                *t = 0;
              bstrip(path);
              if(isalpha(*path) && path[1] == ':' && path[2] == '\\') {
                if(strlen(s) > (8192 - len) - (strlen(path) + 1)) {
                  WinSetDlgItemText(hwnd,
                                    GREP_MASK,
                                    s);
                  break;
                }
                if(!*s || (*s && s[strlen(s) - 1] != ';')) {
                  if(*s)
                    strcat(s,";");
                  strcat(s,path);
                  len += strlen(path);
                  if(s[strlen(s) - 1] != '\\') {
                    len++;
                    strcat(s,"\\");
                  }
                  rstrip(s);
                  if(*s) {
                    strcat(s,simple);
                    WinSetDlgItemText(hwnd,
                                      GREP_MASK,
                                      s);
                    WinSendDlgItemMsg(hwnd,
                                      GREP_MASK,
                                      EM_SETSEL,
                                      MPFROM2SHORT(strlen(s) - (len - 1),
                                                   strlen(s)),
                                      MPVOID);
                  }
                }
              }
              p = strchr(p,';');
              if(p)
                p++;
            }
          }
          break;

        case GREP_WALK:
          {
            CHAR        path[CCHMAXPATH],*p;
            static CHAR s[8192],simple[8192];
            LONG  len;

            WinQueryDlgItemText(hwnd,
                                GREP_MASK,
                                8192,
                                s);
            bstrip(s);
            if(strlen(s) > 8192 - 5) {
              DosBeep(50,100);
              break;
            }
            *path = 0;
            if(WinDlgBox(HWND_DESKTOP,
                         hwnd,
                         WalkAllDlgProc,
                         FM3ModHandle,
                         WALK_FRAME,
                         MPFROMP(path)) &&
               *path) {
              p = strrchr(s,'\\');
              if(p)
                strcpy(simple,p + 1);
              else if(*s)
                strcpy(simple,s);
              else
                strcpy(simple,"*");
              if(!p)
                *s = 0;
              if(simple[strlen(simple) - 1] == ';')
                simple[strlen(simple) - 1] = 0;
              len = strlen(simple) + 1;
              if(strlen(s) > (8192 - len) - (strlen(path) + 1)) {
                DosBeep(250,100);
                WinSetDlgItemText(hwnd,
                                  GREP_MASK,
                                  s);
                break;
              }
              if(!*s || (*s && s[strlen(s) - 1] != ';')) {
                if(*s)
                  strcat(s,";");
                strcat(s,path);
                len += strlen(path);
                if(s[strlen(s) - 1] != '\\') {
                  len++;
                  strcat(s,"\\");
                }
                rstrip(s);
                if(*s) {
                  strcat(s,simple);
                  WinSetDlgItemText(hwnd,
                                    GREP_MASK,
                                    s);
                  WinSendDlgItemMsg(hwnd,
                                    GREP_MASK,
                                    EM_SETSEL,
                                    MPFROM2SHORT(strlen(s) - (len - 1),
                                                 strlen(s)),
                                    MPVOID);
                }
              }
            }
          }
          break;

        case GREP_ADD:
          {
            static CHAR s[8192];
            SHORT       sSelect;

            *s = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_MASK,
                                8192,
                                s);
            bstrip(s);
            if(*s) {
              sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                                 GREP_LISTBOX,
                                                 LM_SEARCHSTRING,
                                                 MPFROM2SHORT(0,LIT_FIRST),
                                                 MPFROMP(s));
              if(sSelect < 0) {
                WinSendDlgItemMsg(hwnd,
                                  GREP_LISTBOX,
                                  LM_INSERTITEM,
                                  MPFROM2SHORT(LIT_SORTASCENDING,0),
                                  MPFROMP(s));
                changed = TRUE;
              }
            }
          }
          break;

        case GREP_DELETE:
          {
            SHORT sSelect;

            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                               GREP_LISTBOX,
                                               LM_QUERYSELECTION,
                                               MPFROMSHORT(LIT_FIRST),
                                               MPVOID);
            if(sSelect >= 0) {
              WinSendDlgItemMsg(hwnd,
                                GREP_LISTBOX,
                                LM_DELETEITEM,
                                MPFROM2SHORT(sSelect,0),
                                MPVOID);
              changed = TRUE;
            }
          }
          break;

        case GREP_OM:
          {
            CHAR  str[81];
            UINT  temp;

            *str = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_OLDER,
                                34,
                                str);
            temp = atoi(str) * 30L;
            sprintf(str,"%u",temp);
            WinSetDlgItemText(hwnd,
                              GREP_OLDER,
                              str);
          }
          break;

        case GREP_NM:
          {
            CHAR  str[81];
            UINT  temp;

            *str = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_NEWER,
                                34,
                                str);
            temp = atoi(str) * 30L;
            sprintf(str,"%u",temp);
            WinSetDlgItemText(hwnd,
                              GREP_NEWER,
                              str);
          }
          break;

        case GREP_GK:
          {
            CHAR  str[81];
            ULONG temp;

            *str = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_GREATER,
                                34,
                                str);
            temp = atol(str) * 1024L;
            sprintf(str,"%lu",temp);
            WinSetDlgItemText(hwnd,GREP_GREATER,str);
          }
          break;

        case GREP_LK:
          {
            CHAR  str[81];
            ULONG temp;

            *str = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_LESSER,
                                34,
                                str);
            temp = atol(str) * 1024L;
            sprintf(str,"%lu",temp);
            WinSetDlgItemText(hwnd,
                              GREP_LESSER,
                              str);
          }
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_GREP,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case GREP_LOCALHDS:
        case GREP_REMOTEHDS:
        case GREP_ALLHDS:
          {
            static CHAR  str[8192],new[8192];
            CHAR        *p,*szDrive = " :\\";
            ULONG        ulDriveNum,ulDriveMap;
            INT          x;
            BOOL         incl;

            *str = *new = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_MASK,
                                8192,
                                str);
            str[8192 - 1] = 0;
            p = strchr(str,';');
            if(p)
              *p = 0;
            p = strrchr(str,'\\');
            if(!p)
              p = strrchr(str,'/');
            if(!p)
              p = strrchr(str,':');
            if(p)
              strcpy(str,p + 1);
            if(!*str)
              strcpy(str,"*");
            *new = 0;
            DosError(FERR_DISABLEHARDERR);
            DosQCurDisk(&ulDriveNum,
                        &ulDriveMap);
            for(x = 2;x < 26;x++) {
              if(ulDriveMap & (1L << x)) {
                incl = FALSE;
                switch(SHORT1FROMMP(mp1)) {
                  case GREP_ALLHDS:
                    if(!(driveflags[x] & (DRIVE_REMOVABLE | DRIVE_IGNORE)))
                      incl = TRUE;
                    break;
                  case GREP_LOCALHDS:
                    if(!(driveflags[x] &
                       (DRIVE_REMOVABLE | DRIVE_IGNORE | DRIVE_REMOTE)))
                      incl = TRUE;
                    break;
                  case GREP_REMOTEHDS:
                    if(!(driveflags[x] &
                       (DRIVE_REMOVABLE | DRIVE_IGNORE)) &&
                       (driveflags[x] & DRIVE_REMOTE))
                      incl = TRUE;
                    break;
                }
              }
              if(incl) {
                if(strlen(new) + strlen(str) + 5 < 8192 - 1) {
                  if(*new)
                    strcat(new,";");
                  *szDrive = x + 'A';
                  strcat(new,szDrive);
                  strcat(new,str);
                }
              }
            }
            if(*new)
              WinSetDlgItemText(hwnd,
                                GREP_MASK,
                                new);
          }
          break;

        case DID_OK:
          hwndCollect = WinQueryWindowULong(hwnd,0);
          if(!hwndCollect)
            DosBeep(50,100);
          else {

            static GREP g;
            CHAR       *str;

            str = malloc(8192 + 512);
            if(!str) {
              DosBeep(50,100);
              break;
            }
            memset(&g,0,sizeof(GREP));
            *str = 0;
            g.size = sizeof(GREP);
            if(WinQueryButtonCheckstate(hwnd,GREP_RECURSE))
              recurse = TRUE;
            else
              recurse = FALSE;
            if(WinQueryButtonCheckstate(hwnd,GREP_ABSOLUTE))
              absolute = TRUE;
            else
              absolute = FALSE;
            if(WinQueryButtonCheckstate(hwnd,GREP_CASE))
              sensitive = TRUE;
            else
              sensitive = FALSE;
            if(WinQueryButtonCheckstate(hwnd,GREP_SAYFILES))
              sayfiles = TRUE;
            else
              sayfiles = FALSE;
            if(WinQueryButtonCheckstate(hwnd,GREP_SEARCHEAS))
              searchEAs = TRUE;
            else
              searchEAs = FALSE;
            if(WinQueryButtonCheckstate(hwnd,GREP_SEARCHFILES))
              searchFiles = TRUE;
            else
              searchFiles = FALSE;
            if(WinQueryButtonCheckstate(hwnd,GREP_FINDIFANY))
              findifany = TRUE;
            else
              findifany = FALSE;
            if(WinQueryButtonCheckstate(hwnd,GREP_FINDDUPES))
              g.finddupes = TRUE;
            else
              g.finddupes = FALSE;
            if(g.finddupes) {
              if(WinQueryButtonCheckstate(hwnd,GREP_CRCDUPES))
                g.CRCdupes = TRUE;
              else
                g.CRCdupes = FALSE;
              if(WinQueryButtonCheckstate(hwnd,GREP_NOSIZEDUPES))
                g.nosizedupes = TRUE;
              else
                g.nosizedupes = FALSE;
              if(WinQueryButtonCheckstate(hwnd,GREP_IGNOREEXTDUPES))
                g.ignoreextdupes = TRUE;
              else
                g.ignoreextdupes = FALSE;
            }
            *str = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_MASK,
                                8192,
                                str);
            bstrip(str);
            if(!*str) {
              DosBeep(50,100);
              WinSetFocus(HWND_DESKTOP,
                          WinWindowFromID(hwnd,GREP_MASK));
              free(str);
              break;
            }
            strcpy(g.tosearch,str);
            strcpy(lastmask,str);
            *str = 0;
            WinQueryWindowText(hwndMLE,
                               4096,
                               str);
            strcpy(lasttext,str);
            {
              CHAR *p,*pp;
              ULONG matched = 0;

              pp = g.searchPattern;
              p = str;
              while(*p) {
                if(*p == '\r') {
                  p++;
                  continue;
                }
                if(*p == '\n') {
                  if(*(p + 1))
                    matched++;
                  *pp = 0;
                }
                else
                  *pp = *p;
                pp++;
                p++;
              }
              if(*g.searchPattern)
                matched++;
              *pp = 0;
              pp++;
              *pp = 0;
              g.numlines = matched;
              g.matched = malloc(g.numlines);
              if(!g.matched)
                g.numlines = 0;
            }
            *str = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_GREATER,
                                34,
                                str);
            greater = atol(str);
            *str = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_LESSER,
                                34,
                                str);
            lesser = atol(str);
            *str = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_NEWER,
                                34,
                                str);
            newer = atoi(str);
            *str = 0;
            WinQueryDlgItemText(hwnd,
                                GREP_OLDER,
                                34,
                                str);
            older = atoi(str);
            if(older || newer) {

              FDATE     fdate;
              FTIME     ftime;
              struct tm tm;
              time_t    t;

              t = time(NULL);
              tm = *localtime(&t);
              fdate.day = tm.tm_mday;
              fdate.month = tm.tm_mon + 1;
              fdate.year = tm.tm_year - 80;
              ftime.hours = tm.tm_hour;
              ftime.minutes = tm.tm_min;
              ftime.twosecs = tm.tm_sec / 2;
              if(older) {
                g.olderthan = SecsSince1980(&fdate,&ftime);
                g.olderthan -= (older * (24L * 60L * 60L));
              }
              if(newer) {
                g.newerthan = SecsSince1980(&fdate,&ftime);
                g.newerthan -= (newer * (24L * 60L * 60L));
              }
            }
            if(!newer)
              g.newerthan = 0;
            if(!older)
              g.olderthan = 0;
            g.greaterthan = greater;
            g.lessthan = lesser;
            g.absFlag = absolute;
            g.caseFlag = sensitive;
            g.dirFlag = recurse;
            g.sayfiles = sayfiles;
            g.searchEAs = searchEAs;
            g.searchFiles = searchFiles;
            g.findifany = findifany;
            g.hwndFiles = hwndCollect;
            g.hwnd = WinQueryWindow(hwndCollect,QW_PARENT);
            g.hwndCurFile = WinWindowFromID(g.hwnd,DIR_SELECTED);
            g.attrFile = ((DIRCNRDATA *)INSTDATA(hwndCollect))->mask.attrFile;
            g.antiattr = ((DIRCNRDATA *)INSTDATA(hwndCollect))->mask.antiattr;
            g.stopflag = &((DIRCNRDATA *)INSTDATA(hwndCollect))->stopflag;
            if(_beginthread(dogrep,NULL,524280,(PVOID)&g) == -1) {
              free(str);
              DosBeep(50,100);
              WinDismissDlg(hwnd,0);
              break;
            }
            else
              DosSleep(128L);
            free(str);
          }
          if(changed) {

            FILE       *fp;
            static CHAR s[8192 + 14];
            SHORT       sSelect,x;

            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                               GREP_LISTBOX,
                                               LM_QUERYITEMCOUNT,
                                               MPVOID,
                                               MPVOID);
            if(sSelect > 0) {
              save_dir2(s);
              if(s[strlen(s) - 1] != '\\')
                strcat(s,"\\");
              strcat(s,"GREPMASK.DAT");
              fp = fopen(s,"w");
              if(fp) {
                fputs(GetPString(IDS_GREPFILETEXT),fp);
                for(x = 0;x < sSelect;x++) {
                  *s = 0;
                  WinSendDlgItemMsg(hwnd,
                                    GREP_LISTBOX,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(x,8192),
                                    MPFROMP(s));
                  bstrip(s);
                  if(*s)
                    fprintf(fp,"%s\n",s);
                }
                fclose(fp);
              }
            }
          }
          WinDismissDlg(hwnd,1);
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

