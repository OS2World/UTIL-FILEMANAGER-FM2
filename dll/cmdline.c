#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"

#pragma alloc_text(CMDLINE1,CmdLineDlgProc,CmdListSubProc,CmdLine2DlgProc,CmdBtnSubProc)
#pragma alloc_text(CMDLINE2,save_cmdlines,load_cmdlines,add_cmdline,remove_cmdline,free_cmdlines)

#define MAXNUMCLS 250

typedef struct LINKCLS {
  CHAR           *cl;
  struct LINKCLS *next;
} LINKCLS;

static LINKCLS *clbig = NULL,*clsmall = NULL;
static BOOL     loadedbig = FALSE,loadedsmall = FALSE;


VOID load_cmdlines (BOOL big) {

  /* load linked list of cmdlines from CMDLINES.DAT file */

  FILE      *fp;
  LINKCLS   *info,*last = NULL,*clhead;
  CHAR       s[1024];
  INT        x = 0;

  clhead = (big) ? clbig : clsmall;
  if(big)
    loadedbig = TRUE;
  else
    loadedsmall = TRUE;
  save_dir2(s);
  if(s[strlen(s) - 1] != '\\')
    strcat(s,"\\");
  strcat(s,(big) ? "CMDLINES.DAT" : "CMDMINI.DAT");
  fp = _fsopen(s,"r",SH_DENYWR);
  if(fp) {
    while(x < MAXNUMCLS && !feof(fp)) {
      if(!fgets(s,sizeof(s),fp))
        break;
      s[sizeof(s) - 1] = 0;
      stripcr(s);
      lstrip(rstrip(s));
      if(*s && *s != ';') {
        info = malloc(sizeof(LINKCLS));
        if(info) {
          x++;
          info->cl = strdup(s);
          if(info->cl) {
            info->next = NULL;
            if(!clhead)
              clhead = info;
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
  if(big)
    clbig = clhead;
  else
    clsmall = clhead;
}


VOID save_cmdlines (BOOL big) {

  /* save linked list of cmdlines to CMDLINES.DAT file */

  LINKCLS *info,*clhead;
  FILE    *fp;
  CHAR     s[CCHMAXPATH + 14];

  clhead = (big) ? clbig : clsmall;
  if((big && !loadedbig) || (!big && !loadedsmall))
    return;
  save_dir2(s);
  if(s[strlen(s) - 1] != '\\')
    strcat(s,"\\");
  strcat(s,(big) ? "CMDLINES.DAT" : "CMDMINI.DAT");
  if(clhead) {
    fp = fopen(s,"w");
    if(fp) {
      fputs(GetPString(IDS_COMMANDFILE2TEXT),fp);
      info = clhead;
      while(info) {
        fprintf(fp,"%0.*s\n",1000,info->cl);
        info = info->next;
      }
      fclose(fp);
    }
  }
  else
    unlink(s);
  if(big)
    clbig = clhead;
  else
    clsmall = clhead;
}


BOOL add_cmdline (CHAR *cl,BOOL big) {

  LINKCLS *info,*last = NULL,*clhead;
  INT      x = 0;

  if(!cl || !*cl)
    return FALSE;
  clhead = (big) ? clbig : clsmall;
  if((big && !loadedbig) || (!big && !loadedsmall))
    load_cmdlines(big);
  info = clhead;
  while(info) {
    if(!stricmp(info->cl,cl))
      return FALSE;
    last = info;
    info = info->next;
    x++;
  }
  info = malloc(sizeof(LINKCLS));
  if(info) {
    info->cl = strdup(cl);
    if(info->cl) {
      info->next = NULL;
      if(!clhead)
        clhead = info;
      else
        last->next = info;
      if(x > MAXNUMCLS) {
        info = clhead;
        clhead = clhead->next;
        free(info);
      }
      if(big)
        clbig = clhead;
      else
        clsmall = clhead;
      return TRUE;
    }
    else
      free(info);
  }
  return FALSE;
}


BOOL remove_cmdline (CHAR *cl,BOOL big) {

  LINKCLS *info,*last = NULL,*clhead;

  if(!cl || !*cl)
    return FALSE;
  if((big && !loadedbig) || (!big && !loadedsmall))
    load_cmdlines(big);
  clhead = (big) ? clbig : clsmall;
  info = clhead;
  while(info) {
    if(!stricmp(info->cl,cl)) {
      if(last)
        last->next = info->next;
      else
        clhead = info->next;
      free(info->cl);
      free(info);
      if(big)
        clbig = clhead;
      else
        clsmall = clhead;
      return TRUE;
    }
    last = info;
    info = info->next;
  }
  return FALSE;
}


VOID free_cmdlines (BOOL big) {

  LINKCLS *info,*next,*clhead;

  clhead = (big) ? clbig : clsmall;
  info = clhead;
  while(info) {
    next = info->next;
    free(info->cl);
    free(info);
    info = next;
  }
  clhead = NULL;
  if(big)
    clbig = clhead;
  else
    clsmall = clhead;
  DosPostEventSem(CompactSem);
}


MRESULT EXPENTRY CmdBtnSubProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_MOUSEMOVE:
      {
        ULONG strid = 0;

        switch(WinQueryWindowUShort(hwnd,QWS_ID)) {
          case EXEC2_CLR:
            strid = IDS_CMDWIPEALLHELPTEXT;
            break;
          case EXEC2_DEL:
            strid = IDS_CMDDELHILITEHELPTEXT;
            break;
          case IDM_HELP:
            strid = IDS_CMDHELPHELPTEXT;
            break;
          case DID_CANCEL:
            strid = IDS_CMDCLOSEBOXHELPTEXT;
            break;
          case EXEC2_OPEN:
            strid = IDS_CMDOPENHELPTEXT;
            break;
          case EXEC2_CLOSE:
            strid = IDS_CMDCLOSEHELPTEXT;
            break;
          case EXEC2_FILTER:
            strid = IDS_CMDFILTERHELPTEXT;
            break;
          case EXEC2_KEEP:
            strid = IDS_CMDKEEPHELPTEXT;
            break;
          case EXEC2_SAVE:
            strid = IDS_CMDSAVEHELPTEXT;
            break;
        }
        if(strid)
          BubbleHelp(hwnd,
                     TRUE,
                     FALSE,
                     FALSE,
                     GetPString(strid));
      }
      break;
  }
  return PFNWPButton(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CmdListSubProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PFNWP oldproc = (PFNWP)WinQueryWindowPtr(hwnd,0);

  switch(msg) {
    case WM_MOUSEMOVE:
      if(hwndBubble)
        WinDestroyWindow(hwndBubble);
      break;

    case WM_CHAR:
      if(SHORT1FROMMP(mp1) & KC_KEYUP) {
        if((SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
           (SHORT2FROMMP(mp2)) == VK_DELETE) {
          if((SHORT1FROMMP(mp1) & KC_CTRL) != KC_CTRL)
            PostMsg(WinQueryWindow(hwnd,QW_PARENT),WM_COMMAND,
                    MPFROM2SHORT(EXEC2_DEL,0),MPVOID);
          else
            PostMsg(WinQueryWindow(hwnd,QW_PARENT),WM_COMMAND,
                    MPFROM2SHORT(EXEC2_CLR,0),MPVOID);
        }
      }
      break;
  }
  return oldproc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CmdLineDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  EXECARGS *ex;

  switch(msg) {
    case WM_INITDLG:
      ex = (EXECARGS *)mp2;
      WinSetWindowPtr(hwnd,0,(PVOID)ex);
      if(!ex || !ex->commandline) {
        WinDismissDlg(hwnd,0);
        break;
      }
      if(*ex->title)
        WinSetWindowText(hwnd,ex->title);
      WinShowWindow(WinWindowFromID(hwnd,EXEC2_DEL),FALSE);
      WinCheckButton(hwnd,EXEC_SAVECMD,fSaveBigCmds);
      WinSendDlgItemMsg(hwnd,EXEC_CL,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(1000,0),MPVOID);
      WinSetDlgItemText(hwnd,EXEC_CL,ex->commandline);
      WinSetWindowPtr(WinWindowFromID(hwnd,EXEC_LISTBOX),0,
                      (PVOID)WinSubclassWindow(WinWindowFromID(hwnd,
                                               EXEC_LISTBOX),
                                               (PFNWP)CmdListSubProc));
      if(*ex->commandline == ' ')
        WinSendDlgItemMsg(hwnd,EXEC_CL,EM_SETSEL,MPFROM2SHORT(0,0),MPVOID);
      WinSendDlgItemMsg(hwnd,EXEC_CL,EM_SETINSERTMODE,
                        MPFROMSHORT(TRUE),MPVOID);
      if(ex->flags & MINIMIZED)
        WinCheckButton(hwnd,EXEC_MINIMIZED,TRUE);
      else if(ex->flags & MAXIMIZED)
        WinCheckButton(hwnd,EXEC_MAXIMIZED,TRUE);
      else if(ex->flags & FULLSCREEN)
        WinCheckButton(hwnd,EXEC_FULLSCREEN,TRUE);
      else if(ex->flags & INVISIBLE)
        WinCheckButton(hwnd,EXEC_INVISIBLE,TRUE);
      else
        WinCheckButton(hwnd,EXEC_DEFAULT,TRUE);
      if((ex->flags & SEPARATEKEEP) == SEPARATEKEEP)
        WinCheckButton(hwnd,EXEC_KEEP,TRUE);
      else
        WinCheckButton(hwnd,EXEC_KEEP,FALSE);
      MLEsetformat(WinWindowFromID(hwnd,EXEC_ENVIRON),MLFIE_NOTRANS);
      MLEsetlimit(WinWindowFromID(hwnd,EXEC_ENVIRON),1000);
      WinSetDlgItemText(hwnd,EXEC_ENVIRON,ex->environment);
//      WinEnableWindow(WinWindowFromID(hwnd,EXEC_ENVIRON),FALSE);
      WinSendMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
      PosOverOkay(hwnd);
      break;

    case UM_SETUP:
      {
        ULONG apptype = 0L;
        CHAR  executable[CCHMAXPATH],commandline[1001],*p;

        ex = INSTDATA(hwnd);
        if(!ex)
          return 0;
        WinSetDlgItemText(hwnd,EXEC_WARNING,NullStr);
        WinSetDlgItemText(hwnd,EXEC_WARNING2,NullStr);
        *commandline = 0;
        WinQueryDlgItemText(hwnd,EXEC_CL,1000,commandline);
        if(!*commandline)
          return 0;
        if(*ex->path) {
          strcpy(executable,ex->path);
          if(executable[strlen(executable) - 1] != '\\')
            strcat(executable,"\\");
        }
        else
          *executable = 0;
        strncat(executable,commandline,CCHMAXPATH - strlen(executable));
        executable[CCHMAXPATH - 1] = 0;
        p = strchr(executable,' ');
        if(p)
          *p = 0;
        if(IsFile(executable) == -1) {
          strncpy(executable,commandline,CCHMAXPATH);
          executable[CCHMAXPATH - 1] = 0;
          p = strchr(executable,' ');
          if(p)
            *p = 0;
          if(IsFile(executable) == -1) {
            p = searchpath(executable);
            if(*p)
              strcpy(executable,p);
            else {
              WinSetDlgItemText(hwnd,
                                EXEC_WARNING2,
                                GetPString(IDS_CANTFINDFILETEXT));
              break;
            }
          }
        }
        if(DosQAppType(executable,&apptype) ||
           (apptype && !(apptype &
                         (FAPPTYP_NOTWINDOWCOMPAT |
                          FAPPTYP_WINDOWCOMPAT |
                          FAPPTYP_WINDOWAPI |
                          FAPPTYP_BOUND |
                          FAPPTYP_DOS |
                          FAPPTYP_WINDOWSREAL |
                          FAPPTYP_WINDOWSPROT |
                          FAPPTYP_32BIT |
                          0x1000)))) {
          p = strchr(executable,'.');
          if(p) {
            if(!stricmp(p,".BAT") ||
               !stricmp(p,".CMD")) {
              WinSetDlgItemText(hwnd,
                                EXEC_WARNING2,
                                GetPString(IDS_RUNTHRUCMDEXETEXT));
              break;
            }
          }
          WinSetDlgItemText(hwnd,
                            EXEC_WARNING2,
                            GetPString(IDS_NOTDIRECTEXETEXT));
        }
        else if(apptype) {  /* acceptable */

          CHAR s[257];

          *s = 0;
          WinSetDlgItemText(hwnd,
                            EXEC_WARNING,
                            GetPString(IDS_ACCEPTABLEAPPTEXT));
          if(apptype & FAPPTYP_DOS)
            strcat(s,GetPString(IDS_DOSTEXT));
          if(apptype & FAPPTYP_WINDOWSREAL) {
            if(*s)
              strcat(s," ");
            strcat(s,GetPString(IDS_WINDOWSREALTEXT));
          }
          if(apptype & FAPPTYP_WINDOWSPROT) {
            if(*s)
              strcat(s," ");
            strcat(s,GetPString(IDS_WINDOWSPROTTEXT));
          }
          if(apptype & 0x1000) {
            if(*s)
              strcat(s," ");
            strcat(s,GetPString(IDS_WINDOWSENHTEXT));
          }
          if((apptype & FAPPTYP_WINDOWAPI) == FAPPTYP_WINDOWAPI) {
            if(*s)
              strcat(s," ");
            strcat(s,GetPString(IDS_PMTEXT));
            WinCheckButton(hwnd,
                           EXEC_FULLSCREEN,
                           FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,EXEC_FULLSCREEN),
                            FALSE);
          }
          if(apptype & FAPPTYP_BOUND) {
            if(*s)
              strcat(s," ");
            strcat(s,GetPString(IDS_BOUNDTEXT));
          }
          if((apptype & FAPPTYP_WINDOWCOMPAT) &&
             !(apptype & FAPPTYP_NOTWINDOWCOMPAT)) {
            if(*s)
              strcat(s," ");
            strcat(s,GetPString(IDS_VIOTEXT));
          }
          if((apptype & FAPPTYP_NOTWINDOWCOMPAT) &&
             !(apptype & FAPPTYP_WINDOWCOMPAT)) {
            if(*s)
              strcat(s," ");
            strcat(s,GetPString(IDS_FULLSCREENTEXT));
          }
          if(apptype & FAPPTYP_32BIT) {
            if(*s)
              strcat(s," ");
            strcat(s,GetPString(IDS_32BITTEXT));
          }
          WinSetDlgItemText(hwnd,EXEC_WARNING2,s);
          if((apptype & (FAPPTYP_DOS | FAPPTYP_WINDOWSREAL |
                        FAPPTYP_WINDOWSPROT | 0x1000)) ||
                        ((apptype & FAPPTYP_WINDOWAPI) == FAPPTYP_WINDOWAPI)) {
            WinCheckButton(hwnd,
                           EXEC_KEEP,
                           FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,
                                            EXEC_KEEP),
                            FALSE);
          }
          if(apptype & (FAPPTYP_WINDOWSREAL | FAPPTYP_WINDOWSPROT | 0x1000))
            WinCheckButton(hwnd,
                           EXEC_FULLSCREEN,
                           TRUE);
        }
        else
          WinSetDlgItemText(hwnd,
                            EXEC_WARNING2,
                            GetPString(IDS_UNKNOWNDEFAULTTEXT));
      }
      return 0;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,
              UM_SETDIR,
              MPVOID,
              MPVOID);
      break;

    case UM_SETDIR:
      PaintRecessedWindow(WinWindowFromID(hwnd,
                                          EXEC_WARNING2),
                          (HPS)0,
                          FALSE,
                          FALSE);
      return 0;

    case UM_RESCAN:
      WinSendDlgItemMsg(hwnd,
                        EXEC_LISTBOX,
                        LM_DELETEALL,
                        MPVOID,
                        MPVOID);
      if(!loadedbig)
        load_cmdlines(TRUE);
      {
        LINKCLS *info;

        info = clbig;
        while(info) {
          WinSendDlgItemMsg(hwnd,
                            EXEC_LISTBOX,
                            LM_INSERTITEM,
                            MPFROM2SHORT(LIT_END,0),
                            MPFROMP(info->cl));
          info = info->next;
        }
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case EXEC_SAVECMD:
          {
            fSaveBigCmds = (fSaveBigCmds) ? FALSE : TRUE;
            WinCheckButton(hwnd,
                           EXEC_SAVECMD,
                           fSaveBigCmds);
            PrfWriteProfileData(fmprof,
                                appname,
                                "SaveBigCmds",
                                &fSaveBigCmds,
                                sizeof(BOOL));
          }
          break;
        case EXEC_CL:
          switch(SHORT2FROMMP(mp1)) {
            case EN_KILLFOCUS:
              ex = INSTDATA(hwnd);
              if(!ex || !ex->dropped)
                WinSendMsg(hwnd,
                           UM_SETUP,
                           MPVOID,
                           MPVOID);
              break;
          }
          break;
        case EXEC_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case LN_KILLFOCUS:
              ex = INSTDATA(hwnd);
              if(ex) {
                if(ex->dropped)
                  PostMsg(hwnd,
                          WM_COMMAND,
                          MPFROM2SHORT(EXEC_DROP,0),
                          MPVOID);
              }
              break;
            case LN_ENTER:
              {
                SHORT x;
                CHAR  cmdline[1001];

                x = (SHORT)WinSendDlgItemMsg(hwnd,
                                             EXEC_LISTBOX,
                                             LM_QUERYSELECTION,
                                             MPFROMSHORT(LIT_FIRST),
                                             MPVOID);
                if(x >= 0) {
                  *cmdline = 0;
                  WinSendDlgItemMsg(hwnd,
                                    EXEC_LISTBOX,
                                    LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(x,
                                                 sizeof(cmdline)),
                                    MPFROMP(cmdline));
                  lstrip(rstrip(cmdline));
                  if(*cmdline) {
                    WinSetDlgItemText(hwnd,
                                      EXEC_CL,
                                      cmdline);
                    PostMsg(hwnd,
                            WM_COMMAND,
                            MPFROM2SHORT(EXEC_DROP,0),
                            MPVOID);
                    PostMsg(hwnd,
                            UM_SETUP,
                            MPVOID,
                            MPVOID);
                  }
                }
              }
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case EXEC2_CLR:
          free_cmdlines(TRUE);
          save_cmdlines(TRUE);
          WinSendDlgItemMsg(hwnd,
                            EXEC_LISTBOX,
                            LM_DELETEALL,
                            MPVOID,
                            MPVOID);
          break;
        case EXEC2_DEL:
          {
            SHORT x;
            CHAR  cmdline[1001];

            x = (SHORT)WinSendDlgItemMsg(hwnd,
                                         EXEC_LISTBOX,
                                         LM_QUERYSELECTION,
                                         MPFROMSHORT(LIT_FIRST),
                                         MPVOID);
            if(x >= 0) {
              *cmdline = 0;
              WinSendDlgItemMsg(hwnd,
                                EXEC_LISTBOX,
                                LM_QUERYITEMTEXT,
                                MPFROM2SHORT(x,sizeof(cmdline)),
                                MPFROMP(cmdline));
              lstrip(rstrip(cmdline));
              if(*cmdline) {
                if(remove_cmdline(cmdline,TRUE) && fSaveBigCmds) {
                  save_cmdlines(TRUE);
                  WinSendDlgItemMsg(hwnd,
                                    EXEC_LISTBOX,
                                    LM_DELETEITEM,
                                    MPFROMSHORT(x),
                                    MPVOID);
                  if(x)
                    x--;
                  WinSendDlgItemMsg(hwnd,
                                    EXEC_LISTBOX,
                                    LM_SELECTITEM,
                                    MPFROMSHORT(x),
                                    MPFROMSHORT(TRUE));
                }
              }
            }
          }
          break;
        case EXEC_DROP:
          ex = INSTDATA(hwnd);
          if(ex) {
            if(!ex->dropped) {
              *ex->tempprompt = 0;
              WinQueryDlgItemText(hwnd,
                                  EXEC_WARNING2,
                                  sizeof(ex->tempprompt),
                                  ex->tempprompt);
              WinSetDlgItemText(hwnd,
                                EXEC_WARNING2,
                                GetPString(IDS_CMDSELCMDHELPTEXT));
              ex->dropped = TRUE;
              if((SHORT)WinSendDlgItemMsg(hwnd,
                                          EXEC_LISTBOX,
                                          LM_QUERYITEMCOUNT,
                                          MPVOID,
                                          MPVOID) == 0)
                PostMsg(hwnd,
                        UM_RESCAN,
                        MPVOID,
                        MPVOID);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_CL),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_WARNING),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_DEFAULT),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_MINIMIZED),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_MAXIMIZED),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_FULLSCREEN),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_INVISIBLE),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_KEEP),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_ENVIRON),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,DID_OK),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,IDM_HELP),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_ABORT),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,DID_CANCEL),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_LISTBOX),TRUE);
              WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,EXEC_LISTBOX));
              WinSetDlgItemText(hwnd,EXEC_DROP,"^");
              WinShowWindow(WinWindowFromID(hwnd,EXEC2_DEL),TRUE);
            }
            else {
              ex->dropped = FALSE;
              WinSetDlgItemText(hwnd,EXEC_WARNING2,ex->tempprompt);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_LISTBOX),FALSE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_CL),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_WARNING),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_DEFAULT),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_MINIMIZED),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_MAXIMIZED),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_FULLSCREEN),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_INVISIBLE),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_KEEP),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_ENVIRON),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,DID_OK),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,IDM_HELP),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,EXEC_ABORT),TRUE);
              WinShowWindow(WinWindowFromID(hwnd,DID_CANCEL),TRUE);
              WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,EXEC_CL));
              WinSetDlgItemText(hwnd,EXEC_DROP,"v");
              WinShowWindow(WinWindowFromID(hwnd,EXEC2_DEL),FALSE);
            }
          }
          break;

        case DID_OK:
          ex = INSTDATA(hwnd);
          WinQueryDlgItemText(hwnd,EXEC_CL,1000,ex->commandline);
          ex->flags = 0;
          if(WinQueryButtonCheckstate(hwnd,EXEC_MINIMIZED))
            ex->flags |= MINIMIZED;
          if(WinQueryButtonCheckstate(hwnd,EXEC_MAXIMIZED))
            ex->flags |= MAXIMIZED;
          if(WinQueryButtonCheckstate(hwnd,EXEC_FULLSCREEN))
            ex->flags |= FULLSCREEN;
          else
            ex->flags |= WINDOWED;
          if(WinQueryButtonCheckstate(hwnd,EXEC_INVISIBLE))
            ex->flags |= INVISIBLE;
          if(WinQueryButtonCheckstate(hwnd,EXEC_KEEP))
            ex->flags |= SEPARATEKEEP;
          else
            ex->flags |= SEPARATE;
          strset(ex->environment,0);
          WinQueryDlgItemText(hwnd,EXEC_ENVIRON,1000,ex->environment);
          if(add_cmdline(ex->commandline,TRUE) && fSaveBigCmds)
            save_cmdlines(TRUE);
          WinDismissDlg(hwnd,1);
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;

        case EXEC_ABORT:
          WinDismissDlg(hwnd,2);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_CMDLINE,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;

    case WM_DESTROY:
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY CmdLine2DlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      if(!mp2) {
        WinDismissDlg(hwnd,0);
        break;
      }
      WinSetWindowPtr(hwnd,0,mp2);
      {
        int    x;
        USHORT ids[] = {IDM_HELP,EXEC2_CLR,EXEC2_DEL,EXEC2_KEEP,EXEC2_SAVE,
                        EXEC2_OPEN,EXEC2_CLOSE,EXEC2_FILTER,DID_CANCEL,0};

        WinSetWindowPtr(WinWindowFromID(hwnd,EXEC2_LISTBOX),0,
                        (PVOID)WinSubclassWindow(WinWindowFromID(hwnd,
                                                 EXEC2_LISTBOX),
                                                 (PFNWP)CmdListSubProc));
        for(x = 0;ids[x];x++)
          WinSetWindowPtr(WinWindowFromID(hwnd,ids[x]),0,
                          (PVOID)WinSubclassWindow(WinWindowFromID(hwnd,
                                                   ids[x]),
                                                   (PFNWP)CmdBtnSubProc));
      }
      PostMsg(hwnd,UM_RESCAN,MPVOID,MPVOID);
      break;

    case UM_RESCAN:
      WinSendDlgItemMsg(hwnd,EXEC2_LISTBOX,LM_DELETEALL,MPVOID,MPVOID);
      if(!loadedsmall)
        load_cmdlines(FALSE);
      {
        LINKCLS *info;

        info = clsmall;
        while(info) {
          WinSendDlgItemMsg(hwnd,EXEC2_LISTBOX,LM_INSERTITEM,
                            MPFROM2SHORT(LIT_END,0),MPFROMP(info->cl));
          info = info->next;
        }
      }
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case EXEC2_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case LN_ENTER:
              {
                SHORT x;
                CHAR *cmdline = WinQueryWindowPtr(hwnd,0);

                x = (SHORT)WinSendDlgItemMsg(hwnd,EXEC2_LISTBOX,
                                             LM_QUERYSELECTION,
                                             MPFROMSHORT(LIT_FIRST),
                                             MPVOID);
                if(x >= 0) {
                  *cmdline = 0;
                  WinSendDlgItemMsg(hwnd,EXEC2_LISTBOX,LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(x,1000),
                                    MPFROMP(cmdline));
                  lstrip(rstrip(cmdline));
                  if(*cmdline)
                    WinDismissDlg(hwnd,1);
                }
              }
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case EXEC2_KEEP:
        case EXEC2_SAVE:
        case EXEC2_FILTER:
        case EXEC2_CLOSE:
        case EXEC2_OPEN:
        case IDM_HELP:
          {
            char *cmdline = WinQueryWindowPtr(hwnd,0);
            ULONG strid = 0;

            if(cmdline) {
              switch(SHORT1FROMMP(mp1)) {
                case EXEC2_OPEN:
                  strid = IDS_OPENCMDTEXT;
                  break;
                case EXEC2_CLOSE:
                  strid = IDS_CLOSECMDTEXT;
                  break;
                case EXEC2_FILTER:
                  strid = IDS_FILTERCMDTEXT;
                  break;
                case EXEC2_SAVE:
                  strid = (fSaveMiniCmds) ? IDS_NOSAVECMDTEXT : IDS_SAVECMDTEXT;
                  break;
                case EXEC2_KEEP:
                  strid = (fKeepCmdLine) ? IDS_NOKEEPCMDTEXT : IDS_KEEPCMDTEXT;
                  break;
                case IDM_HELP:
                  strid = IDS_HELPCMDTEXT;
                  break;
              }
              if(strid) {
                strcpy(cmdline,GetPString(strid));
                WinDismissDlg(hwnd,1);
              }
            }
            break;
          }
        case EXEC2_CLR:
          free_cmdlines(FALSE);
          save_cmdlines(FALSE);
          WinSendDlgItemMsg(hwnd,
                            EXEC2_LISTBOX,
                            LM_DELETEALL,
                            MPVOID,
                            MPVOID);
          break;
        case EXEC2_DEL:
          {
            SHORT x;
            CHAR  cmdline[1001];

            x = (SHORT)WinSendDlgItemMsg(hwnd,
                                         EXEC2_LISTBOX,
                                         LM_QUERYSELECTION,
                                         MPFROMSHORT(LIT_FIRST),
                                         MPVOID);
            if(x >= 0) {
              *cmdline = 0;
              WinSendDlgItemMsg(hwnd,
                                EXEC2_LISTBOX,
                                LM_QUERYITEMTEXT,
                                MPFROM2SHORT(x,sizeof(cmdline)),
                                MPFROMP(cmdline));
              lstrip(rstrip(cmdline));
              if(*cmdline) {
                if(remove_cmdline(cmdline,FALSE) && fSaveMiniCmds) {
                  save_cmdlines(FALSE);
                  WinSendDlgItemMsg(hwnd,
                                    EXEC2_LISTBOX,
                                    LM_DELETEITEM,
                                    MPFROMSHORT(x),
                                    MPVOID);
                  if(x)
                    x--;
                  WinSendDlgItemMsg(hwnd,
                                    EXEC2_LISTBOX,
                                    LM_SELECTITEM,
                                    MPFROMSHORT(x),
                                    MPFROMSHORT(TRUE));
                }
              }
            }
          }
          break;
        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;

    case WM_DESTROY:
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

