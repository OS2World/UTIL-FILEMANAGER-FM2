
/***********************************************************************

  $Id$

  Custom menu support routines for FM/2

  Copyright (c) 1996-98 M. Kimes
  Copyright (c) 2004, 2006 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  22 Jul 06 SHL Check more run time errors
  29 Jul 06 SHL Use xfgets_bstripcr
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Feb 08 GKY Use xfree where appropriate

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>

#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "menu.h"
#include "errutil.h"			// Dos_Error...
#include "fm3dll.h"
#include "fortify.h"

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

MENU *menuhead = NULL;

INT tokenize(CHAR * str, INT max, CHAR ** tokens)
{
  INT x = 0;
  CHAR *p;

  if (str && max && tokens) {
    p = str;
    for (;;) {
      p = skip_delim(p, " \t");
      if (!p)
	break;
      tokens[x++] = p;
      if (x == max)
	break;
      p = to_delim(p, " \t");
      if (!p)
	break;
      *p = 0;
      p++;
      // saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"\"%s\"\r\r%d",tokens[x - 1],x);
      if (!*p)
	break;
    }
  }
  return x;
}

VOID FreeMenuList(MENU * head)
{
  MENU *info, *next;

  info = head;
  while (info) {
    next = info->next;
    xfree(info->text, pszSrcFile, __LINE__);
    free(info);
    info = next;
  }
}

BOOL AddToMenu(CHAR * filename, HWND hwndMenu)
{
  FILE *fp;
  CHAR s[256];
  CHAR *tokens[3];
  INT lines = 0;
  MENU *info, *last = NULL;
  BOOL ret = FALSE;

  // fixme to complain?
  if (!hwndMenu) {
    Runtime_Error(pszSrcFile, __LINE__, "no data");
    return ret;
  }
  if (!filename)
    filename = "FM3MENU.DAT";
  fp = _fsopen(filename, "r", SH_DENYWR);
  if (!fp) {
    // else saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"Couldn't open %s",filename);
  }
  else {
    while (!feof(fp)) {
      if (!xfgets_bstripcr(s, sizeof(s), fp, pszSrcFile, __LINE__))
	break;
      lines++;
      if (!*s || *s == ';')
	continue;
      if (tokenize(s, 3, tokens) == 3 && (USHORT) atoi(tokens[1])) {
	info = xmallocz(sizeof(MENU), pszSrcFile, __LINE__);
	if (info) {
	  info->size = sizeof(MENU);
	  info->text = xstrdup(tokens[2], pszSrcFile, __LINE__);
	  if (!info->text)
	    free(info);
	  else {
	    if (!stricmp(tokens[0], "MENUITEM"))
	      info->cmd = atoi(tokens[1]);
	    else if (!stricmp(tokens[0], "SEPARATOR"))
	      info->type = SEPARATOR;
	    else {
	      /* error! */
	      xfree(info->text, pszSrcFile, __LINE__);
	      free(info);
	      info = NULL;
	    }
	    if (info) {
	      if (!menuhead)
		menuhead = info;
	      else
		last->next = info;
	      info->next = NULL;
	      last = info;
	    }
	  }
	}
      }
      else {
	// fixme to complain?
	// saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"Tokenization failed");
      }
    }
    fclose(fp);

    if (menuhead) {
      MENUITEM mi;

      memset(&mi, 0, sizeof(mi));
      info = menuhead;
      WinEnableWindow(hwndMenu, FALSE);
      while (info) {
	mi.iPosition = MIT_END;
	mi.id = info->cmd;
	mi.afStyle =
	  (info->type == SEPARATOR) ? MIS_BREAKSEPARATOR : MIS_TEXT;
	if (WinSendMsg
	    (hwndMenu, MM_INSERTITEM, MPFROMP(&mi), MPFROMP(info->text)))
	  ret = TRUE;
	info = info->next;
      }
      WinEnableWindow(hwndMenu, TRUE);
      FreeMenuList(menuhead);
      menuhead = NULL;
    }
  }
  return ret;
}

#pragma alloc_text(MENU,tokenize,FreeMenuList,AddToMenu)
