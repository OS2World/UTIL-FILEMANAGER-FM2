
/***********************************************************************

  $Id$

  archiver.bb2 loader and utilities

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  Revisions	01 Aug 04 SHL Rework lstrip/rstrip usage

***********************************************************************/

#define INCL_WIN
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <share.h>
#include <ctype.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

BOOL loadedarcs = FALSE;

#pragma alloc_text(MISC9,quick_find_type,find_type)


ARC_TYPE * quick_find_type (CHAR *filespec,ARC_TYPE *topsig) {

  ARC_TYPE *info,*found = NULL;
  CHAR     *p;

  if(!loadedarcs)
    load_archivers();
  p = strrchr(filespec,'.');
  if(p) {
    p++;
    info = (topsig) ? topsig : arcsighead;
    while(info) {
      if(info->ext &&
         *(info->ext) &&
         !stricmp(p,info->ext)) {
        found = find_type(filespec,topsig);
        break;
      }
      info = info->next;
    }
  }
  return found;
}


ARC_TYPE * find_type (CHAR *filespec,ARC_TYPE *topsig) {

  HFILE       handle;
  ULONG       action,len,l;
  ARC_TYPE   *info;
  CHAR        *p,buffer[80];   /* Read buffer for the signatures. */

  if(!loadedarcs)
    load_archivers();
  if(topsig == NULL)
    topsig = arcsighead;
  DosError(FERR_DISABLEHARDERR);
  if(DosOpen(filespec,
             &handle,
             &action,
             0L,
             0L,
             OPEN_ACTION_FAIL_IF_NEW |
              OPEN_ACTION_OPEN_IF_EXISTS,
             OPEN_FLAGS_FAIL_ON_ERROR |
              OPEN_FLAGS_NOINHERIT |
              OPEN_FLAGS_RANDOMSEQUENTIAL |
              OPEN_SHARE_DENYNONE |
              OPEN_ACCESS_READONLY,
             0L))
    return NULL;
  info = topsig;                   /* start of signatures */
  while(info) {
    if(!info->signature ||
       !*info->signature) {        /* no signature -- work on extension only */
      p = strrchr(filespec,'.');
      if(p) {
        p++;
        if(info->ext &&
           *(info->ext) &&
           !stricmp(p,info->ext))
          break;
      }
    }
    l = strlen(info->signature);   /* Get the signature length. */
    l = min(l,79);
    if(!DosChgFilePtr(handle,
                      abs(info->file_offset),
                      (info->file_offset >= 0L) ?
                       FILE_BEGIN :
                       FILE_END,
                      &len)) {
      if(!DosRead(handle,
                  buffer,
                  l,
                  &len) &&
         len == l) {
        if(!memcmp(info->signature,
                   buffer,
                   l))
          break;
      }
    }
    info = info->next;
  }
  DosClose(handle);                    /* Either way, we're done for now */
  return info;                         /* return signature, if any */
}

#pragma alloc_text(AVL,load_archivers)

INT load_archivers (VOID) {

  FILE           *handle;
  CHAR           s[257],*p;
  ARC_TYPE       *info = NULL,*last = NULL;
  INT            numlines = NUMLINES,x;

  loadedarcs = TRUE;
  DosEnterCritSec();
  p = searchpath(GetPString(IDS_ARCHIVERBB2));
  if(!p || !*p) {
    DosExitCritSec();
    return -1;
  }
  handle = _fsopen(p,"r",SH_DENYWR);
  DosExitCritSec();
  if(!handle)
    return -2;
  strcpy(archiverbb2,p);
  if(!fgets(s,256,handle)) {
    fclose(handle);
    return -3;
  }
  p = strchr(s,';');
  if(p)
    *p = 0;
  bstripcr(s);
  if(*s)
    numlines = atoi(s);
  if(!*s || numlines < NUMLINES)
    return -3;
  while(!feof(handle)) {
    if(!fgets(s,256,handle))
      break;
    p = strchr(s,';');
    if(p)
      *p = 0;				// Chop comment
    bstripcr(s);
    if(*s) {
      info = malloc(sizeof(ARC_TYPE));
      if(!info)
        break;
      memset(info,0,sizeof(ARC_TYPE));
      if(*s)
        info->id = strdup(s);
      else
        info->id = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      if(*s)
        info->ext = strdup(s);
      else
        info->ext = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      info->file_offset = atol(s);
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      if(*s)
        info->list = strdup(s);
      else
        info->list = NULL;
      if(!info->list)
        break;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      if(*s)
        info->extract = strdup(s);
      else
        info->extract = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      if(*s)
        info->exwdirs = strdup(s);
      else
        info->exwdirs = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      if(*s)
        info->test = strdup(s);
      else
        info->test = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      if(*s)
        info->create = strdup(s);
      else
        info->create = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;				// Chop comment
      bstripcr(s);
      if(*s)
        info->createwdirs = strdup(s);
      else
        info->createwdirs = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      if(*s)
        info->createrecurse = strdup(s);
      else
        info->createrecurse = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      if(*s)
        info->move = strdup(s);
      else
        info->move = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      if(*s)
        info->movewdirs = strdup(s);
      else
        info->movewdirs = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      bstripcr(s);
      info->delete = strdup(s);
      if(!fgets(s,256,handle))
        break;
      stripcr(s);
      literal(s);
      if(*s) {
        info->signature = strdup(s);
        if(!info->signature)
          break;
      }
      else
        info->signature = NULL;
      if(!fgets(s,256,handle))
        break;
      stripcr(s);
      info->startlist = strdup(s);
      if(!fgets(s,256,handle))
        break;
      stripcr(s);
      if(*s)
        info->endlist = strdup(s);
      else
        info->endlist = NULL;
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      info->osizepos = atoi(s);
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      info->nsizepos = atoi(s);
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      info->fdpos = atoi(s);
      p = strchr(s,',');
      if(p) {
        p++;
        info->datetype = atoi(p);
      }
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      info->fdflds = atoi(s);
      if(!fgets(s,256,handle))
        break;
      p = strchr(s,';');
      if(p)
        *p = 0;
      info->fnpos = atoi(s);
      p = strchr(s,',');
      if(p) {
        p++;
        info->nameislast = (BOOL)(*p && atol(p) == 0) ? FALSE : TRUE;
        p = strchr(p,',');
        if(p) {
          p++;
          info->nameisnext = (BOOL)(*p && atol(p) == 0) ? FALSE : TRUE;
          p = strchr(p,',');
          if(p) {
            p++;
            info->nameisfirst = (BOOL)(*p && atol(p) == 0) ? FALSE : TRUE;
          }
        }
      }
      for(x = NUMLINES;x < numlines;x++) {
        if(!fgets(s,256,handle))
          break;
      }
      info->next = NULL;
      if(!arcsighead) {
        arcsighead = last = info;
        info->prev = NULL;
      }
      else {
        last->next = info;
        info->prev = last;
        last = info;
      }
      if(info->extract &&
         !*info->extract) {
        free(info->extract);
        info->extract = NULL;
      }
    }
    info = NULL;
  }
  fclose(handle);
  if(info) {
    if(info->id)        free(info->id);
    if(info->ext)       free(info->ext);
    if(info->list)      free(info->list);
    if(info->extract)   free(info->extract);
    if(info->create)    free(info->create);
    if(info->move)      free(info->move);
    if(info->delete)    free(info->delete);
    if(info->signature) free(info->signature);
    if(info->startlist) free(info->startlist);
    if(info->endlist)   free(info->endlist);
    if(info->exwdirs)   free(info->exwdirs);
    if(info->test)      free(info->test);
    if(info->createrecurse)
                        free(info->createrecurse);
    if(info->createwdirs)
                        free(info->createwdirs);
    if(info->movewdirs) free(info->movewdirs);
  }
  if(!arcsighead)
    return -4;
  return 0;
}


#pragma alloc_text(FMARCHIVE,SBoxDlgProc)

MRESULT EXPENTRY SBoxDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  /* dlg proc that allows selecting an archiver entry */

  ARC_TYPE        **info,*temp,*test;
  SHORT           sSelect,x;
  CHAR            text[256];

  switch(msg) {
    case WM_INITDLG:
      if(!loadedarcs)
        load_archivers();
      if(!(ARC_TYPE **)mp2) {
        DosBeep(100,100);
        WinDismissDlg(hwnd,0);
        break;
      }
      info = (ARC_TYPE **)mp2;
      if(*info)
        *info = arcsighead;
      WinSetWindowPtr(hwnd,0L,(PVOID)info);
      temp = arcsighead;
      WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_DELETEALL,MPVOID,MPVOID);
      /* this loop fills the listbox */
      {
        BOOL found = FALSE;

        while(temp) {
          /*
           * this inner loop tests for a dupe signature entry and assures
           * that only the entry at the top of the list gets used for
           * conversion; editing any is okay
           */
          if(*info) {
            test = arcsighead;
            while(test && test != temp) {
              if(!strcmp(test->signature,temp->signature))
                goto ContinueHere;
              test = test->next;
            }
          }

          if(!*info || (temp->id && temp->extract && temp->create)) {
            sSelect = (SHORT)WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_INSERTITEM,
                                               MPFROM2SHORT(LIT_END,0),
                                               MPFROMP((temp->id) ?
                                                       temp->id : "?"));
            if(!found && *szDefArc && temp->id && !strcmp(szDefArc,temp->id)) {
              WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_SELECTITEM,
                                MPFROMSHORT(sSelect),MPFROMSHORT(TRUE));
              found = TRUE;
            }
          }
          else {
            if(!temp->id || !*temp->id)
              WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                MPFROMP(GetPString(IDS_UNKNOWNUNUSABLETEXT)));
            else {
              CHAR s[81];

              sprintf(s,"%0.12s %s",
                      temp->id,
                      GetPString(IDS_UNUSABLETEXT));
              WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END,0),
                                MPFROMP(s));
            }
          }

ContinueHere:

          temp = temp->next;
        }
        if(found)
          PosOverOkay(hwnd);
      }
      break;

    case WM_COMMAND:
      info = (ARC_TYPE **)WinQueryWindowPtr(hwnd,0L);
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                             ASEL_LISTBOX,
                                             LM_QUERYSELECTION,
                                             MPFROMSHORT(LIT_FIRST),
                                             MPVOID);
          if(sSelect >= 0) {
            temp = arcsighead;
            if(*info) {
              *text = 0;
              WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_QUERYITEMTEXT,
                                MPFROM2SHORT(sSelect,255),MPFROMP(text));
              if(*text) {
                while(temp) {
                  if(temp->id) {
                    if(!strcmp(text,temp->id))
                      break;
                  }
                  temp = temp->next;
                }
              }
              else
                temp = NULL;
            }
            else {
              x = 0;
              while(temp) {
                if(x >= sSelect)
                  break;
                x++;
                temp = temp->next;
              }
            }
            if(temp && (!*info || (temp->id && temp->extract &&
                                   temp->create))) {
              *info = temp;
            }
            else {
              WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_SELECTITEM,
                                MPFROMSHORT(LIT_NONE),FALSE);
              DosBeep(100,100);
              temp = NULL;
              return 0;
            }
          }
          else {
            DosBeep(100,100);
            return 0;
          }
          WinDismissDlg(hwnd,TRUE);
          return 0;

        case DID_CANCEL:
          *info = NULL;
          PostMsg(hwnd,WM_CLOSE,MPVOID,MPVOID);
          break;

        default:
          break;
      }
      return 0;

    case WM_CONTROL:
      if(SHORT1FROMMP(mp1) == ASEL_LISTBOX && SHORT2FROMMP(mp1) == LN_ENTER)
        PostMsg(hwnd,WM_COMMAND,MPFROM2SHORT(DID_OK,0),MPVOID);
      return 0;

    case WM_CLOSE:
      WinDismissDlg(hwnd,FALSE);
      return 0;

    default:
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


/*
02-08-96  23:55               1
 8 Feb 96 23:55:32            2
 8 Feb 96  11:55p             3
96-02-08 23:55:32             4
*/

#pragma alloc_text(ARCCNRS,ArcDateTime)

BOOL ArcDateTime (CHAR *dt,INT type,CDATE *cdate,CTIME *ctime) {

  INT   x;
  BOOL  ret = FALSE;
  CHAR *p,*pp,*pd;

  if(dt && cdate && ctime) {
    memset(cdate,0,sizeof(CDATE));
    memset(ctime,0,sizeof(CTIME));
    if(type) {
      p = dt;
      while(*p && *p == ' ')
        p++;
      pd = dt;
      switch(type) {
        case 1:
          cdate->month = atoi(pd);
          p = to_delim(pd,"-/.");
          if(p) {
            p++;
            cdate->day = atoi(p);
            pd = p;
            p = to_delim(pd,"-/.");
            if(p) {
              p++;
              cdate->year = atoi(p);
              if(cdate->year > 80 && cdate->year < 1900)
                cdate->year += 1900;
              else if(cdate->year < 1900)
                cdate->year += 2000;
              ret = TRUE;
              p = strchr(p,' ');
              if(p) {
                while(*p && *p == ' ')
                  p++;
                ctime->hours = atoi(p);
                p = to_delim(pd,":.");
                if(p) {
                  p++;
                  ctime->minutes = atoi(p);
                  p = to_delim(pd,":.");
                  if(p) {
                    p++;
                    ctime->seconds = atoi(p);
                  }
                }
              }
            }
          }
          break;

        case 2:
          cdate->day = atoi(p);
          p = strchr(p,' ');
          if(p) {
            p++;
            for(x = 0;x < 12;x++) {
              if(!strnicmp(p,GetPString(IDS_JANUARY + x),3))
                break;
            }
            if(x < 12) {
              cdate->month = x;
              p = strchr(p,' ');
              if(p) {
                p++;
                cdate->year = atoi(p);
                if(cdate->year > 80 && cdate->year < 1900)
                  cdate->year += 1900;
                else if(cdate->year < 1900)
                  cdate->year += 2000;
                ret = TRUE;
                p = strchr(p,' ');
                if(p) {
                  while(*p && *p == ' ')
                    p++;
                  ctime->hours = atoi(p);
                  p = to_delim(pd,":.");
                  if(p) {
                    p++;
                    ctime->minutes = atoi(p);
                    p = to_delim(pd,":.");
                    if(p) {
                      p++;
                      ctime->seconds = atoi(p);
                    }
                  }
                }
              }
            }
          }
          break;

        case 3:
          cdate->day = atoi(p);
          p = strchr(p,' ');
          if(p) {
            p++;
            for(x = 0;x < 12;x++) {
              if(!strnicmp(p,GetPString(IDS_JANUARY + x),3))
                break;
            }
            if(x < 12) {
              cdate->month = x;
              p = strchr(p,' ');
              if(p) {
                p++;
                cdate->year = atoi(p);
                if(cdate->year > 80 && cdate->year < 1900)
                  cdate->year += 1900;
                else if(cdate->year < 1900)
                  cdate->year += 2000;
                ret = TRUE;
                p = strchr(p,' ');
                if(p) {
                  while(*p && *p == ' ')
                    p++;
                  ctime->hours = atoi(p);
                  p = to_delim(pd,":.");
                  if(p) {
                    p++;
                    pp = p;
                    ctime->minutes = atoi(p);
                    p = to_delim(pd,":.");
                    if(p) {
                      p++;
                      ctime->seconds = atoi(p);
                      p += 2;
                      if(toupper(*p) == 'P')
                        ctime->hours += 12;
                    }
                    else  {
                      p = pp;
                      p += 2;
                      if(toupper(*p) == 'P')
                        ctime->hours += 12;
                    }
                  }
                }
              }
            }
          }
          break;

        case 4:
          cdate->year = atoi(p);
          if(cdate->year > 80 && cdate->year < 1900)
            cdate->year += 1900;
          else if(cdate->year < 1900)
            cdate->year += 2000;
          p = to_delim(pd,"-/.");
          if(p) {
            p++;
            cdate->month = atoi(p);
            pd = p;
            p = to_delim(pd,"-/.");
            if(p) {
              p++;
              cdate->day = atoi(p);
              ret = TRUE;
              p = strchr(p,' ');
              if(p) {
                while(*p && *p == ' ')
                  p++;
                ctime->hours = atoi(p);
                p = to_delim(pd,":.");
                if(p) {
                  p++;
                  ctime->minutes = atoi(p);
                  p = to_delim(pd,":.");
                  if(p) {
                    p++;
                    ctime->seconds = atoi(p);
                  }
                }
              }
            }
          }
          break;

        case 5:
          cdate->day = atoi(pd);
          p = to_delim(pd,"-/.");
          if(p) {
            p++;
            cdate->month = atoi(p);
            pd = p;
            p = to_delim(pd,"-/.");
            if(p) {
              p++;
              cdate->year = atoi(p);
              if(cdate->year > 80 && cdate->year < 1900)
                cdate->year += 1900;
              else if(cdate->year < 1900)
                cdate->year += 2000;
              ret = TRUE;
              p = strchr(p,' ');
              if(p) {
                while(*p && *p == ' ')
                  p++;
                ctime->hours = atoi(p);
                p = to_delim(pd,":.");
                if(p) {
                  p++;
                  ctime->minutes = atoi(p);
                  p = to_delim(pd,":.");
                  if(p) {
                    p++;
                    ctime->seconds = atoi(p);
                  }
                }
              }
            }
          }
          break;

        default:
          break;
      }
    }
  }
  return ret;
}
