/**************************************************************************/
/* Custom menu support routines for FM/2                                  */
/* copyright (c) 1996 by M. Kimes -- all rights reserved                  */
/**************************************************************************/

#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>
#include "fm3dll.h"
#include "menu.h"

#pragma data_seg(DATA2)
#pragma alloc_text(MENU,tokenize,FreeMenuList,AddToMenu)

MENU *menuhead = NULL;


INT tokenize (CHAR *str,INT max,CHAR **tokens) {

  INT   x = 0;
  CHAR *p;

  if(str && max && tokens) {
    p = str;
    for(;;) {
      p = skip_delim(p," \t");
      if(!p)
        break;
      tokens[x++] = p;
      if(x == max)
        break;
      p = to_delim(p," \t");
      if(!p)
        break;
      *p = 0;
      p++;
// saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"\"%s\"\r\r%d",tokens[x - 1],x);
      if(!*p)
        break;
    }
  }
  return x;
}


VOID FreeMenuList (MENU *head) {

  MENU *info,*next;

  info = head;
  while(info) {
    next = info->next;
    if(info->text)
      free(info->text);
    free(info);
    info = next;
  }
}


BOOL AddToMenu (CHAR *filename,HWND hwndMenu) {

  FILE *fp;
  CHAR  s[256];
  CHAR *tokens[3];
  INT   lines = 0;
  MENU *info,*last = NULL;
  BOOL  ret = FALSE;

  if(!hwndMenu)
    return ret;
  if(!filename)
    filename = "FM3MENU.DAT";
  fp = _fsopen(filename,"r",SH_DENYWR);
  if(fp) {
    while(!feof(fp)) {
      if(!fgets(s,256,fp))
        break;
      lines++;
      stripcr(s);
      lstrip(rstrip(s));
      if(!*s || *s == ';')
        continue;
      if(tokenize(s,3,tokens) == 3 && (USHORT)atoi(tokens[1])) {
// saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"%s\r\r%s\r\r%s",tokens[0],tokens[1],tokens[2]);
        info = malloc(sizeof(MENU));
        if(info) {
          memset(info,0,sizeof(MENU));
          info->size = sizeof(MENU);
          info->text = strdup(tokens[2]);
          if(info->text) {
            if(!stricmp(tokens[0],"MENUITEM"))
              info->cmd = atoi(tokens[1]);
            else if(!stricmp(tokens[0],"SEPARATOR"))
              info->type = SEPARATOR;
            else { /* error! */
              free(info->text);
              free(info);
              info = NULL;
            }
            if(info) {
              if(!menuhead)
                menuhead = info;
              else
                last->next = info;
              info->next = NULL;
              last = info;
            }
          }
          else
            free(info);
        }
      }
      else {  /* error! */
// saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"Tokenization failed");
      }
    }
    fclose(fp);
    if(menuhead) {

      MENUITEM mi;

      memset(&mi,0,sizeof(mi));
      info = menuhead;
      WinEnableWindow(hwndMenu,FALSE);
      while(info) {
        mi.iPosition = MIT_END;
        mi.id = info->cmd;
        mi.afStyle = (info->type == SEPARATOR) ? MIS_BREAKSEPARATOR : MIS_TEXT;
        if(WinSendMsg(hwndMenu, MM_INSERTITEM, MPFROMP(&mi),
                      MPFROMP(info->text)))
          ret = TRUE;
        info = info->next;
      }
      WinEnableWindow(hwndMenu,TRUE);
      FreeMenuList(menuhead);
      menuhead = NULL;
    }
  }
// else saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"Couldn't open %s",filename);
  return ret;
}
