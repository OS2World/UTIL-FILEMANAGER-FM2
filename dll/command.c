
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  Revisions	01 Aug 04 SHL Rework lstrip/rstrip usage

***********************************************************************/

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
#include "tools.h"

typedef struct {
  CHAR  cl[1001];
  INT   flags;
  CHAR  title[34];
} COMMAND;

#pragma data_seg(DATA1)
#pragma alloc_text(COMMAND,command_title,free_commands,add_command,kill_command)
#pragma alloc_text(COMMAND,CommandDlgProc,EditCommands,ReOrderProc,CommandTextProc)


MRESULT EXPENTRY CommandTextProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  PFNWP       oldproc = (PFNWP)WinQueryWindowPtr(hwnd,0);
  static BOOL emphasized = FALSE;

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
      DropHelp(mp1,
               mp2,
               hwnd,
               GetPString(IDS_DROPCMDHELPTEXT));
      return 0;

    case DM_DROP:
      {
        char szFrom[CCHMAXPATH + 5];

        if(emphasized) {
          emphasized = FALSE;
          DrawTargetEmphasis(hwnd,emphasized);
        }
        if(GetOneDrop(mp1,mp2,szFrom,CCHMAXPATH)) {
          strcat(szFrom," %a");
          WinSetWindowText(hwnd,szFrom);
        }
      }
      return 0;
  }
  return (oldproc) ? oldproc(hwnd,msg,mp1,mp2) :
                     WinDefWindowProc(hwnd,msg,mp1,mp2);
}


MRESULT EXPENTRY ReOrderProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  switch(msg) {
    case WM_INITDLG:
      if(!cmdhead) {
        WinDismissDlg(hwnd,0);
        break;
      }
      {
        LINKCMDS *info;
        SHORT     x;

        info = cmdhead;
        while(info) {
          x = (SHORT)WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,LM_INSERTITEM,
                                       MPFROMSHORT(LIT_END),
                                       MPFROMP(info->title));
          if(x >= 0)
            WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,LM_SETITEMHANDLE,
                              MPFROMSHORT(x),MPFROMP(info));
          else {
            DosBeep(50,100);
            WinDismissDlg(hwnd,0);
          }
          info = info->next;
        }
      }
      break;

    case WM_CONTROL:
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;

        case DID_OK:
          {
            LINKCMDS *temphead = NULL,*info,*last = NULL,*temptail = NULL;
            SHORT     sSelect,numitems;

            sSelect = 0;
            numitems = (SHORT)WinSendDlgItemMsg(hwnd,RE_REMOVELISTBOX,
                                                LM_QUERYITEMCOUNT,
                                                MPVOID,MPVOID);
            while(numitems) {
              info = (LINKCMDS *)WinSendDlgItemMsg(hwnd,RE_REMOVELISTBOX,
                                                   LM_QUERYITEMHANDLE,
                                                   MPFROMSHORT(sSelect++),
                                                   MPVOID);
              if(info) {
                if(!temphead) {
                  temphead = info;
                  info->prev = NULL;
                }
                else {
                  last->next = info;
                  info->prev = last;
                }
                temptail = info;
                last = info;
                info->next = NULL;
              }
              numitems--;
            }
            sSelect = 0;
            numitems = (SHORT)WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,
                                                LM_QUERYITEMCOUNT,
                                                MPVOID,MPVOID);
            while(numitems) {
              info = (LINKCMDS *)WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,
                                                   LM_QUERYITEMHANDLE,
                                                   MPFROMSHORT(sSelect++),
                                                   MPVOID);
              if(info) {
                if(!temphead) {
                  temphead = info;
                  info->prev = NULL;
                }
                else {
                  last->next = info;
                  info->prev = last;
                }
                temptail = info;
                last = info;
                info->next = NULL;
              }
              numitems--;
            }
            cmdhead = temphead;
            cmdtail = temptail;
          }
          WinDismissDlg(hwnd,1);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_REORDERCOMMANDS,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case RE_ADD:
          {
            SHORT     sSelect,x;
            LINKCMDS *info;

            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,
                                               LM_QUERYSELECTION,
                                               MPFROMSHORT(LIT_FIRST),
                                               MPVOID);
            while(sSelect >= 0) {
              info = (LINKCMDS *)WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,
                                                   LM_QUERYITEMHANDLE,
                                                   MPFROMSHORT(sSelect),
                                                   MPVOID);
              if(info) {
                x = (SHORT)WinSendDlgItemMsg(hwnd,RE_REMOVELISTBOX,
                                             LM_INSERTITEM,
                                             MPFROM2SHORT(LIT_END,0),
                                             MPFROMP(info->title));
                if(x >= 0) {
                  WinSendDlgItemMsg(hwnd,RE_REMOVELISTBOX,
                                    LM_SETITEMHANDLE,
                                    MPFROMSHORT(x),
                                    MPFROMP(info));
                  WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,LM_DELETEITEM,
                                    MPFROMSHORT(sSelect),MPVOID);
                }
              }
              else
                WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,LM_SELECTITEM,
                                  MPFROMSHORT(sSelect),MPFROMSHORT(FALSE));
              sSelect = (USHORT)WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,
                                                  LM_QUERYSELECTION,
                                                  MPFROMSHORT(LIT_FIRST),
                                                  MPVOID);
            }
          }
          break;

        case RE_REMOVE:
          {
            SHORT     sSelect,x;
            LINKCMDS *info;

            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,RE_REMOVELISTBOX,
                                               LM_QUERYSELECTION,
                                               MPFROMSHORT(LIT_FIRST),
                                               MPVOID);
            while(sSelect >= 0) {
              info = (LINKCMDS *)WinSendDlgItemMsg(hwnd,RE_REMOVELISTBOX,
                                                   LM_QUERYITEMHANDLE,
                                                   MPFROMSHORT(sSelect),
                                                   MPVOID);
              if(info) {
                x = (SHORT)WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,
                                             LM_INSERTITEM,
                                             MPFROM2SHORT(LIT_END,0),
                                             MPFROMP(info->title));
                if(x >= 0) {
                  WinSendDlgItemMsg(hwnd,RE_ADDLISTBOX,
                                    LM_SETITEMHANDLE,
                                    MPFROMSHORT(x),
                                    MPFROMP(info));
                  WinSendDlgItemMsg(hwnd,RE_REMOVELISTBOX,LM_DELETEITEM,
                                    MPFROMSHORT(sSelect),MPVOID);
                }
              }
              else
                WinSendDlgItemMsg(hwnd,RE_REMOVELISTBOX,LM_SELECTITEM,
                                  MPFROMSHORT(sSelect),MPFROMSHORT(FALSE));
              sSelect = (USHORT)WinSendDlgItemMsg(hwnd,RE_REMOVELISTBOX,
                                                  LM_QUERYSELECTION,
                                                  MPFROMSHORT(LIT_FIRST),
                                                  MPVOID);
            }
          }
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


CHAR *command_title (INT cx) {

  static CHAR  duh[] = "???";
  LINKCMDS    *info;
  INT          x = 0;

  if(!cmdloaded)
    load_commands();
  info = cmdhead;
  while(info) {
    if(x == cx)
      return info->title;
    info = info->next;
  }
  return duh;
}


VOID free_commands (VOID) {

  LINKCMDS *info,*next;

  info = cmdhead;
  while(info) {
    next = info->next;
    free(info->title);
    free(info->cl);
    free(info);
    info = next;
  }
  cmdhead = cmdtail = NULL;
}


VOID load_commands (VOID) {

  FILE     *fp;
  LINKCMDS *info;
  CHAR      cl[1024];
  CHAR      title[100];
  CHAR      flags[72];

  if(cmdhead)
    free_commands();
  cmdloaded = TRUE;
  save_dir2(cl);
  if(cl[strlen(cl) - 1] != '\\')
    strcat(cl,"\\");
  strcat(cl,"COMMANDS.DAT");
  fp = _fsopen(cl,"r",SH_DENYWR);
  if(fp) {
    while(!feof(fp)) {
      if(!fgets(title,100,fp))
        break;
      title[34] = 0;
      bstripcr(title);
      if(!*title || *title == ';')
        continue;
      if(!fgets(cl,1024,fp) ||
         !fgets(flags,72,fp))
        break;                       /* error! */
      cl[1000] = 0;
      flags[34] = 0;
      bstripcr(cl);
      bstripcr(flags);
      if(!*cl)
        continue;
      info = malloc(sizeof(LINKCMDS));
      if(info) {
        memset(info,0,sizeof(LINKCMDS));
        info->cl = strdup(cl);
        info->title = strdup(title);
        info->flags = atol(flags);
        if(!info->cl || !info->title) {
          if(info->cl)
            free(info->cl);
          if(info->title)
            free(info->title);
          free(info);
          break;
        }
        if(!cmdhead)
          cmdhead = info;
        else {
          cmdtail->next = info;
          info->prev = cmdtail;
        }
        cmdtail = info;
      }
    }
    fclose(fp);
  }
}


VOID save_commands (VOID) {

  LINKCMDS *info;
  FILE     *fp;
  CHAR      s[CCHMAXPATH + 14];

  if(!cmdloaded || !cmdhead)
    return;
  info = cmdhead;
  save_dir2(s);
  if(s[strlen(s) - 1] != '\\')
    strcat(s,"\\");
  strcat(s,"COMMANDS.DAT");
  fp = fopen(s,"w");
  if(fp) {
    fputs(GetPString(IDS_COMMANDFILETEXT),fp);
    info = cmdhead;
    while(info) {
      fprintf(fp,
              ";\n%0.34s\n%0.1000s\n%lu\n",
              info->title,
              info->cl,
              info->flags);
      info = info->next;
    }
    fclose(fp);
  }
}


LINKCMDS * add_command (COMMAND *addme) {

  LINKCMDS *info;

  if(addme && *addme->cl && *addme->title) {
    info = cmdhead;
    while(info) {
      if(!stricmp(info->title,addme->title))
        return NULL;
      info = info->next;
    }
    if(!info) {
      info = malloc(sizeof(LINKCMDS));
      if(info) {
        memset(info,0,sizeof(LINKCMDS));
        info->cl = strdup(addme->cl);
        info->title = strdup(addme->title);
        if(addme->flags)
          info->flags = addme->flags;
        if(!info->cl || !info->title) {
          if(info->cl)
            free(info->cl);
          if(info->title)
            free(info->title);
          free(info);
        }
        else {
          if(!cmdhead)                    /* only item in list */
            cmdhead = cmdtail = info;
          else {                          /* place at tail */
            cmdtail->next = info;
            info->prev = cmdtail;
            cmdtail = info;
          }
          return info;
        }
      }
    }
  }
  return NULL;
}


BOOL kill_command (CHAR *killme) {

  LINKCMDS *info;

  if(killme && *killme) {
    info = cmdhead;
    while(info) {
      if(!stricmp(info->title,killme)) {
        if(info == cmdhead) {
          cmdhead = info->next;
          if(info == cmdtail)
            cmdtail = info->prev;
        }
        else {
          if(info->next)
            (info->next)->prev = info->prev;
          if(info->prev)
            (info->prev)->next = info->next;
          if(info == cmdtail)
            cmdtail = info->prev;
        }
        free(info->cl);
        free(info->title);
        free(info);
        return TRUE;
      }
      info = info->next;
    }
  }
  return FALSE;
}


MRESULT EXPENTRY CommandDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  SHORT     x;
  LINKCMDS *info;

  switch(msg) {
    case WM_INITDLG:
      WinSendDlgItemMsg(hwnd,CMD_LISTBOX,LM_DELETEALL,MPVOID,MPVOID);
      WinSendDlgItemMsg(hwnd,CMD_CL,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(1000,0),MPVOID);
      WinSendDlgItemMsg(hwnd,CMD_TITLE,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(33,0),MPVOID);
      WinSetDlgItemText(hwnd,CMD_CL,NullStr);
      WinSetDlgItemText(hwnd,CMD_TITLE,NullStr);
      WinCheckButton(hwnd,CMD_DEFAULT,TRUE);
      WinCheckButton(hwnd,CMD_PROMPT,FALSE);
      WinCheckButton(hwnd,CMD_ONCE,FALSE);
      info = cmdhead;
      while(info) {
        x = (SHORT)WinSendDlgItemMsg(hwnd,CMD_LISTBOX,LM_INSERTITEM,
                                     MPFROM2SHORT(LIT_END,0),
                                     MPFROMP(info->title));
        if(x >= 0)
          WinSendDlgItemMsg(hwnd,CMD_LISTBOX,LM_SETITEMHANDLE,
                            MPFROMSHORT(x),MPFROMP(info));
        info = info->next;
      }
      WinSendDlgItemMsg(hwnd,CMD_LISTBOX,LM_SELECTITEM,
                        MPFROMSHORT(0),MPFROMSHORT(TRUE));
      {
        PFNWP oldproc;

        oldproc = WinSubclassWindow(WinWindowFromID(hwnd,CMD_CL),
                                    (PFNWP)CommandTextProc);
        if(oldproc)
          WinSetWindowPtr(WinWindowFromID(hwnd,CMD_CL),0,(PVOID)oldproc);
      }
      break;

    case WM_CONTROL:
      if(SHORT1FROMMP(mp1) == CMD_LISTBOX) {
        switch(SHORT2FROMMP(mp1)) {
          case LN_ENTER:
          case LN_SELECT:
            x = (SHORT)WinSendDlgItemMsg(hwnd,CMD_LISTBOX,
                                         LM_QUERYSELECTION,
                                         MPFROMSHORT(LIT_FIRST),
                                         MPVOID);
            if(x >= 0) {
              info = (LINKCMDS *)WinSendDlgItemMsg(hwnd,CMD_LISTBOX,
                                                   LM_QUERYITEMHANDLE,
                                                   MPFROMSHORT(x),
                                                   MPVOID);
              if(!info) {
                DosBeep(50,100);
                break;
              }
              WinSetDlgItemText(hwnd,CMD_CL,info->cl);
              if(!(info->flags & 1023))
                WinCheckButton(hwnd,CMD_DEFAULT,TRUE);
              else {
                if(info->flags & FULLSCREEN)
                  WinCheckButton(hwnd,CMD_FULLSCREEN,TRUE);
                else if(info->flags & MINIMIZED)
                  WinCheckButton(hwnd,CMD_MINIMIZED,TRUE);
                else if(info->flags & MAXIMIZED)
                  WinCheckButton(hwnd,CMD_MAXIMIZED,TRUE);
                else if(info->flags & INVISIBLE)
                  WinCheckButton(hwnd,CMD_INVISIBLE,TRUE);
              }
              WinCheckButton(hwnd,CMD_PROMPT,((info->flags & PROMPT) != 0));
              WinCheckButton(hwnd,CMD_KEEP,((info->flags & KEEP) != 0));
              WinCheckButton(hwnd,CMD_ONCE,((info->flags & ONCE) != 0));
              WinSetDlgItemText(hwnd,CMD_TITLE,info->title);
              {
                CHAR  env[1002];
                ULONG size;

                *env = 0;
                size = sizeof(env) - 1;
                if(PrfQueryProfileData(fmprof,FM3Str,info->cl,env,&size) &&
                   *env)
                  WinSetDlgItemText(hwnd,CMD_ENVIRON,env);
                else
                  WinSetDlgItemText(hwnd,CMD_ENVIRON,NullStr);
              }
            }
            break;
        }
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case CMD_FIND:
          {
            CHAR filename[CCHMAXPATH + 9];

            *filename = 0;
            if(insert_filename(hwnd,filename,2,FALSE) && *filename) {
              strcat(filename," %a");
              WinSetDlgItemText(hwnd,CMD_CL,filename);
            }
          }
          break;

        case CMD_REORDER:
          if(!cmdhead || !cmdhead->next) {
            DosBeep(250,100);
            break;
          }
          if(WinDlgBox(HWND_DESKTOP,hwnd,ReOrderProc,FM3ModHandle,
                       RE_FRAME,MPVOID)) {
            WinSendDlgItemMsg(hwnd,
                              CMD_LISTBOX,
                              LM_DELETEALL,
                              MPVOID,
                              MPVOID);
            WinSetDlgItemText(hwnd,
                              CMD_CL,
                              NullStr);
            WinSetDlgItemText(hwnd,
                              CMD_TITLE,
                              NullStr);
            info = cmdhead;
            while(info) {
              x = (SHORT)WinSendDlgItemMsg(hwnd,
                                           CMD_LISTBOX,
                                           LM_INSERTITEM,
                                           MPFROM2SHORT(LIT_END,0),
                                           MPFROMP(info->title));
              if(x >= 0)
                WinSendDlgItemMsg(hwnd,
                                  CMD_LISTBOX,
                                  LM_SETITEMHANDLE,
                                  MPFROMSHORT(x),
                                  MPFROMP(info));
              info = info->next;
            }
            WinSendDlgItemMsg(hwnd,
                              CMD_LISTBOX,
                              LM_SELECTITEM,
                              MPFROMSHORT(0),
                              MPFROMSHORT(TRUE));
            WinCheckButton(hwnd,
                           CMD_DEFAULT,
                           TRUE);
            WinCheckButton(hwnd,
                           CMD_PROMPT,
                           FALSE);
            WinCheckButton(hwnd,
                           CMD_ONCE,
                           FALSE);
            save_commands();
          }
          break;

        case DID_OK:
          x = (SHORT)WinSendDlgItemMsg(hwnd,
                                       CMD_LISTBOX,
                                       LM_QUERYSELECTION,
                                       MPFROMSHORT(LIT_FIRST),
                                       MPVOID);
          WinDismissDlg(hwnd,0);
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,
                       HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_COMMAND,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case CMD_ADD:
          {
            COMMAND temp;

            memset(&temp,0,sizeof(COMMAND));
            WinQueryDlgItemText(hwnd,
                                CMD_CL,
                                sizeof(temp.cl),
                                temp.cl);
            bstrip(temp.cl);
            WinQueryDlgItemText(hwnd,CMD_TITLE,sizeof(temp.title),temp.title);
            if(WinQueryButtonCheckstate(hwnd,CMD_DEFAULT))
              temp.flags = 0;
            else if(WinQueryButtonCheckstate(hwnd,CMD_FULLSCREEN))
              temp.flags = FULLSCREEN;
            else if(WinQueryButtonCheckstate(hwnd,CMD_MINIMIZED))
              temp.flags = MINIMIZED;
            else if(WinQueryButtonCheckstate(hwnd,CMD_MAXIMIZED))
              temp.flags = MAXIMIZED;
            else if(WinQueryButtonCheckstate(hwnd,CMD_INVISIBLE))
              temp.flags = INVISIBLE;
            if(WinQueryButtonCheckstate(hwnd,CMD_KEEP))
              temp.flags |= KEEP;
            if(WinQueryButtonCheckstate(hwnd,CMD_PROMPT))
              temp.flags |= PROMPT;
            if(WinQueryButtonCheckstate(hwnd,CMD_ONCE))
              temp.flags |= ONCE;
            info = add_command(&temp);
            if(info) {
              {
                CHAR env[1002];

                *env = 0;
                WinQueryDlgItemText(hwnd,
                                    CMD_ENVIRON,
                                    1000,
                                    env);
                bstripcr(env);
                if(*env)
                  PrfWriteProfileString(fmprof,
                                        FM3Str,
                                        temp.cl,
                                        env);
              }
              x = (SHORT)WinSendDlgItemMsg(hwnd,
                                           CMD_LISTBOX,
                                           LM_INSERTITEM,
                                           MPFROM2SHORT(LIT_END,0),
                                           MPFROMP(temp.title));
              if(x >= 0) {
                WinSendDlgItemMsg(hwnd,
                                  CMD_LISTBOX,
                                  LM_SETITEMHANDLE,
                                  MPFROMSHORT(x),
                                  MPFROMP(info));
                WinSendDlgItemMsg(hwnd,
                                  CMD_LISTBOX,
                                  LM_SELECTITEM,
                                  MPFROMSHORT(x),
                                  MPFROMSHORT(TRUE));
                save_commands();
              }
            }
            else
              DosBeep(50,200);
          }
          break;

        case CMD_DELETE:
          {
            CHAR temp[34];

            WinQueryDlgItemText(hwnd,CMD_TITLE,34,temp);
            bstrip(temp);
            if(kill_command(temp)) {
              x = (SHORT)WinSendDlgItemMsg(hwnd,
                                           CMD_LISTBOX,
                                           LM_QUERYSELECTION,
                                           MPFROMSHORT(LIT_FIRST),
                                           MPVOID);
              if(x >= 0) {
                WinSendDlgItemMsg(hwnd,
                                  CMD_LISTBOX,
                                  LM_DELETEITEM,
                                  MPFROMSHORT(x),
                                  MPVOID);
                WinSendDlgItemMsg(hwnd,
                                  CMD_LISTBOX,
                                  LM_SELECTITEM,
                                  MPFROMSHORT(LIT_NONE),
                                  MPFROMSHORT(FALSE));
              }
              save_commands();
            }
            else
              DosBeep(50,200);
          }
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


VOID RunCommand (HWND hwnd,INT cx) {

  static CHAR  stop = 0;
  INT          x;
  CHAR       **list;
  LINKCMDS    *info;

  list = BuildList(hwnd);
  if(!list || !list[0])
    return;
  x = 0;
  info = cmdhead;
  while(info) {
    x++;
    if(x == cx)
      break;
    info = info->next;
  }
  if(info) {

    INT   flags;
    BOOL  dieafter = FALSE;

    x--;
    flags = info->flags;
    if(!(flags & FULLSCREEN))
      flags |= WINDOWED;
    if(flags & KEEP)
      flags |= SEPARATEKEEP;
    else
      flags |= SEPARATE;
    flags &= (~KEEP);
    if(flags & DIEAFTER)
      dieafter = TRUE;
    flags &= (~DIEAFTER);
    if((flags & ONCE) && list && list[0]) {

      CHAR  *fakelist[2];
      INT    cntr;

      flags &= (~ONCE);
      for(cntr = 0;list[cntr];cntr++) {
        *fakelist = list[cntr];
        fakelist[1] = NULL;
        ExecOnList(hwnd,
                   info->cl,
                   flags,
                   NULL,
                   fakelist,
                   GetPString(IDS_EXECCMDTITLETEXT));
      }
    }
    else
      ExecOnList(hwnd,
                 info->cl,
                 flags,
                 NULL,
                 list,
                 GetPString(IDS_EXECCMDTITLETEXT));
  }
  FreeList(list);
  DosPostEventSem(CompactSem);
}


VOID EditCommands (HWND hwnd) {

  static CHAR stop = 0;

  if(stop)
    return;
  stop++;
  if(!cmdloaded)
    load_commands();
  WinDlgBox(HWND_DESKTOP,
            hwnd,
            CommandDlgProc,
            FM3ModHandle,
            CMD_FRAME,
            MPFROMP(&hwnd));
  stop = 0;
}

