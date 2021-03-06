
/**************************************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2010 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  14 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets, xfgets_bstripcr
  10 Sep 06 GKY Add Move to last, Okay adds if new, Replace Current in Listbox Dialog
  19 Oct 06 GKY Rework replace logic
  18 Feb 07 GKY Move error messages etc to string file
  19 Apr 07 SHL Sync with AcceptOneDrop GetOneDrop mods
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  06 Jan 08 GKY Use NormalizeCmdLine to check program strings on entry
  29 Feb 08 GKY Changes to enable user settable command line length
  29 Feb 08 GKY Use xfree where appropriate
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory and use BldFullPathName
  24 Aug 08 GKY Warn full drive on save of .DAT file; prevent loss of existing file
  15 Nov 09 GKY Add check for attempt to open zero byte file (avoids MMPM trying to play them)
  21 Dec 09 GKY Added CheckExecutibleFlags to streamline code in command.c assoc.c & cmdline.c
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10).
                Mostly cast CHAR CONSTANT * as CHAR *.
  01 May 10 GKY Add ENVIRONMENT_SIZE variable to standardize this size everywhere.
  01 May 10 GKY Changes to move environment storage to INI file
  03 Jul 11 GKY Fixed failure to save associations after reordering item to the top or bottom.
  13 Aug 11 GKY Change to Doxygen comment format

**************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_PM
#define INCL_WINHOOKS
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "arccnrs.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "defview.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "pathutil.h"			// BldQuotedFileName
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "assoc.h"
#include "droplist.h"			// AcceptOneDrop, DropHelp, GetOneDrop
#include "misc.h"			// DrawTargetEmphasis
#include "systemf.h"			// ExecOnList
#include "shadow.h"			// OpenObject
#include "getnames.h"			// insert_filename
#include "literal.h"			// literal
#include "wrappers.h"			// xfgets
#include "strips.h"			// bstrip
#include "stristr.h"			// stristr
#include "dirs.h"			// save_dir2
#include "fortify.h"

#pragma data_seg(DATA1)

typedef struct
{
  LONG offset;
  ULONG flags;
  PSZ pszCmdLine;
  CHAR mask[CCHMAXPATH];
  CHAR sig[CCHMAXPATH];
  CHAR env[ENVIRONMENT_SIZE];
}
ASSOC;

typedef struct LINKASSOC
{
  CHAR *mask;
  PSZ pszCmdLine;
  CHAR *sig;
  CHAR *env;
  LONG offset;
  ULONG flags;
  struct LINKASSOC *prev;
  struct LINKASSOC *next;
}
LINKASSOC;

static LINKASSOC *asshead = NULL, *asstail = NULL;
static BOOL assloaded = FALSE, replace = FALSE;

static PSZ pszSrcFile = __FILE__;

static VOID load_associations(VOID);
static VOID save_associations(VOID);

MRESULT EXPENTRY AssocTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);
  static BOOL emphasized = FALSE;

  switch (msg) {
  case DM_DRAGOVER:
    if (!emphasized) {
      emphasized = TRUE;
      DrawTargetEmphasis(hwnd, emphasized);
    }
    if (AcceptOneDrop(hwnd, mp1, mp2))
      return MRFROM2SHORT(DOR_DROP, DO_MOVE);
    return MRFROM2SHORT(DOR_NEVERDROP, 0);

  case DM_DRAGLEAVE:
    if (emphasized) {
      emphasized = FALSE;
      DrawTargetEmphasis(hwnd, emphasized);
    }
    break;

  case DM_DROPHELP:
    DropHelp(mp1, mp2, hwnd, GetPString(IDS_ASSOCDROPHELPTEXT));
    return 0;

  case DM_DROP:
    {
      char szFrom[CCHMAXPATH + 5];

      if (emphasized) {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
      if (GetOneDrop(hwnd, mp1, mp2, szFrom, CCHMAXPATH)) {
	strcat(szFrom, " %a");
	WinSetWindowText(hwnd, szFrom);
      }
    }
    return 0;
  }
  return (oldproc) ? oldproc(hwnd, msg, mp1, mp2) :
    WinDefWindowProc(hwnd, msg, mp1, mp2);
}

VOID free_associations(VOID)
{
  LINKASSOC *info, *next;

  info = asshead;
  while (info) {
    next = info->next;
    xfree(info->mask, pszSrcFile, __LINE__);
    xfree(info->pszCmdLine, pszSrcFile, __LINE__);
    xfree(info->sig, pszSrcFile, __LINE__);
    free(info);
    info = next;
  }
  asshead = asstail = NULL;
}

VOID load_associations(VOID)
{
  FILE *fp;
  LINKASSOC *info;
  PSZ pszCmdLine;
  CHAR mask[CCHMAXPATH + 24];
  CHAR sig[CCHMAXPATH + 24];
  CHAR offset[72];
  CHAR flags[72];
  CHAR env[ENVIRONMENT_SIZE];
  CHAR key[CCHMAXPATH];
  CHAR *moder = "r";

  if (asshead)
    free_associations();
  assloaded = TRUE;
  BldFullPathName(mask, pFM2SaveDirectory, PCSZ_ASSOCDAT);
  fp = xfsopen(mask, moder, SH_DENYWR, pszSrcFile, __LINE__, TRUE);
  pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
  if (!pszCmdLine) {
    if (fp)
      fclose(fp); //already complained
  }
  if (fp) {
    while (!feof(fp)) {
      if (!xfgets(mask, sizeof(mask), fp, pszSrcFile, __LINE__))	// fixme why +24?
	break;
      mask[CCHMAXPATH] = 0;
      bstripcr(mask);
      if (!*mask || *mask == ';')
	continue;
      if (!xfgets(pszCmdLine, MaxComLineStrg, fp, pszSrcFile, __LINE__) ||
	  !xfgets(sig, CCHMAXPATH + 24, fp, pszSrcFile, __LINE__) ||
	  !xfgets(offset, sizeof(offset), fp, pszSrcFile, __LINE__) ||
	  !xfgets(flags, sizeof(flags), fp, pszSrcFile, __LINE__))
	break;				// error!
      pszCmdLine[MaxComLineStrg - 1] = 0;
      bstripcr(pszCmdLine);
      sig[CCHMAXPATH] = 0;
      bstripcr(sig);
      offset[34] = 0;
      bstripcr(offset);
      flags[34] = 0;
      bstripcr(flags);
      if (!*pszCmdLine)
        continue;
      sprintf(key, "ASSOC.%senv", pszCmdLine);
      PrfQueryProfileString(fmprof, FM3Str, key, NullStr, env, sizeof(env));
      info = xmallocz(sizeof(LINKASSOC), pszSrcFile, __LINE__);
      if (info) {
	info->pszCmdLine = xstrdup(pszCmdLine, pszSrcFile, __LINE__);
	info->mask = xstrdup(mask, pszSrcFile, __LINE__);
	if (*sig)
	  info->sig = xstrdup(sig, pszSrcFile, __LINE__);
	info->offset = atol(offset);
        info->flags = atol(flags);
        if (env != NullStr)
          info->env = xstrdup(env, pszSrcFile, __LINE__);
	if (!info->pszCmdLine || !info->mask) {
	  xfree(info->pszCmdLine, pszSrcFile, __LINE__);
          xfree(info->mask, pszSrcFile, __LINE__);
          xfree(info->env, pszSrcFile, __LINE__);
	  free(info);
	  break;
	}
	if (!asshead)
	  asshead = info;
	else {
	  asstail->next = info;
	  info->prev = asstail;
	}
	asstail = info;
      }
    }
    xfree(pszCmdLine, pszSrcFile, __LINE__);
    fclose(fp);
  }
}

VOID display_associations(HWND hwnd, ASSOC *temp, LINKASSOC *info)
{
  PSZ pszDisplayStr;
  SHORT x;

  pszDisplayStr = xmallocz((CCHMAXPATH * 2) + MaxComLineStrg + 6,
			   pszSrcFile, __LINE__);
  if (pszDisplayStr) {
    sprintf(pszDisplayStr, "%-12s \x1a %-24s %s%s%s", temp->mask,
	    temp->pszCmdLine, (*temp->sig) ?
	    "[" : NullStr, (*temp->sig) ? temp->sig : NullStr,
	    (*temp->sig) ? "]" : NullStr);
    x = (SHORT) WinSendDlgItemMsg(hwnd,
				  ASS_LISTBOX,
				  LM_INSERTITEM,
				  MPFROM2SHORT(LIT_END, 0), MPFROMP(pszDisplayStr));
    if (x >= 0) {
      WinSendDlgItemMsg(hwnd,
			ASS_LISTBOX,
			LM_SETITEMHANDLE,
			MPFROMSHORT(x), MPFROMP(info));
      WinSendDlgItemMsg(hwnd,
			ASS_LISTBOX,
			LM_SELECTITEM,
			MPFROMSHORT(x), MPFROMSHORT(TRUE));
    }
    free(pszDisplayStr);
  }
}

VOID save_associations(VOID)
{
  LINKASSOC *info;
  FILE *fp;
  CHAR s[CCHMAXPATH + 14];
  CHAR *modew = "w";

  if (!assloaded || !asshead)
    return;
  info = asshead;
#ifdef NEVER
  while (info) {
    next = info->next;
    if (!strcmp("*", info->mask)) {
      if (info != asshead) {		// already top record
	if (info->prev)
	  (info->prev)->next = info->next;
	if (info->next)
	  (info->next)->prev = info->prev;
	if (info == asstail)
	  asstail = info->prev;
	info->next = asshead->next;
	info->prev = NULL;
	asshead = info;
      }
    }
    info = next;
  }
#endif
  BldFullPathName(s, pFM2SaveDirectory, PCSZ_ASSOCDAT);
  if (CheckDriveSpaceAvail(s, ullDATFileSpaceNeeded, 1) == 2)
    return; //already gave error msg
  fp = xfopen(s, modew, pszSrcFile, __LINE__, FALSE);
  if (fp) {
    fputs(GetPString(IDS_ASSOCFILETEXT), fp);
    info = asshead;
    while (info) {
      fprintf(fp,
	      ";\n%0.*s\n%0.*s\n%0.*s\n%lu\n%lu\n",
	      CCHMAXPATH,
	      info->mask,
	      MaxComLineStrg,
	      info->pszCmdLine,
	      CCHMAXPATH,
              (info->sig) ? info->sig : NullStr, info->offset, info->flags);
      if (info->env[0] != 0) {
        CHAR key[CCHMAXPATH];

        sprintf(key, "ASSOC.%senv", info->pszCmdLine);
        PrfWriteProfileString(fmprof, FM3Str, key, info->env);
      }
      else {
        CHAR key[CCHMAXPATH];

        sprintf(key, "ASSOC.%senv", info->pszCmdLine);
        PrfWriteProfileString(fmprof, FM3Str, key, NULL);
      }

      info = info->next;
    }
    fclose(fp);
  }
}

LINKASSOC *add_association(ASSOC * addme)
{
  LINKASSOC *info;

  if (addme && *addme->pszCmdLine && *addme->mask) {
    info = asshead;
    while (info) {
      if ((!replace) && (!stricmp(info->mask, addme->mask) &&
			 ((!info->sig && !*addme->sig) || (!replace) &&
			  (info->sig && !strcmp(addme->sig, info->sig)))))
	return NULL;
      info = info->next;
    }
    if (!info) {
      info = xmallocz(sizeof(LINKASSOC), pszSrcFile, __LINE__);
      if (info) {
	info->pszCmdLine = xstrdup(addme->pszCmdLine, pszSrcFile, __LINE__);
	info->mask = xstrdup(addme->mask, pszSrcFile, __LINE__);
	if (*addme->sig)
	  info->sig = xstrdup(addme->sig, pszSrcFile, __LINE__);
	if (addme->offset)
	  info->offset = addme->offset;
	if (addme->flags)
          info->flags = addme->flags;
        if (addme->env != NullStr)
          info->env = xstrdup(addme->env, pszSrcFile, __LINE__);
	if (!info->pszCmdLine || !info->mask) {
	  xfree(info->pszCmdLine, pszSrcFile, __LINE__);
	  xfree(info->mask, pszSrcFile, __LINE__);
	  free(info);
	}
	else {
	  if (!asshead)			// only item in list
	    asshead = asstail = info;
	  else {
	    if (asstail) {		// place at tail
	      asstail->next = info;
	      info->prev = asstail;
	    }
	    asstail = info;
	  }
	  return info;
	}
      }
    }
  }
  return NULL;
}

BOOL kill_association(ASSOC * killme)
{
  LINKASSOC *info;

  if (killme && *killme->mask) {
    info = asshead;
    while (info) {
      if (!stricmp(info->mask, killme->mask) &&
	  info->offset == killme->offset &&
	  (((!info->sig || !*info->sig) && !*killme->sig) ||
	   (info->sig && !strcmp(killme->sig, info->sig)))) {
	if (info == asshead) {
	  asshead = info->next;
	  if (info == asstail)
	    asstail = info->prev;
	}
	else {
	  if (info->next)
	    (info->next)->prev = info->prev;
	  if (info->prev)
	    (info->prev)->next = info->next;
	  if (info == asstail)
	    asstail = info->prev;
	}
	xfree(info->pszCmdLine, pszSrcFile, __LINE__);
	xfree(info->mask, pszSrcFile, __LINE__);
	xfree(info->sig, pszSrcFile, __LINE__);
	free(info);
	return TRUE;
      }
      info = info->next;
    }
  }
  return FALSE;
}

INT ExecAssociation(HWND hwnd, CHAR * datafile)
{
  CHAR *file;
  CHAR sig[CCHMAXPATH];
  CHAR sigl[CCHMAXPATH];
  CHAR mask[CCHMAXPATH], *p;
  FILE *fp;
  BOOL didmatch;
  BOOL exclude;
  BOOL checked = FALSE;
  ULONG offset;
  LINKASSOC *info;
  CHAR *moderb = "rb";

  if (!assloaded)
    load_associations();
  if (!asshead)
    return -1;
  if (!datafile || !*datafile)
    return -1;
  file = strrchr(datafile, '\\');
  if (!file)
    file = strrchr(datafile, ':');
  if (file)
    file++;
  else
    file = datafile;
  info = asshead;
  while (info) {
    strcpy(mask, info->mask);
    p = strtok(mask, ";");
    while (p) {
      if (*p == '/') {
	p++;
	exclude = TRUE;
      }
      else
	exclude = FALSE;
	didmatch = wildcard((strchr(p, '\\') ||
			     strchr(p, ':')) ? datafile : file, p, FALSE);
      if (exclude && didmatch)
        didmatch = FALSE;
      fp = xfsopen(datafile, moderb, SH_DENYNO, pszSrcFile, __LINE__, TRUE);
      if (fp) {
        if (!checked) {
          APIRET temp;
  
          temp = fread(sig, 1, 2, fp);
          if (temp != 2) {
            temp = saymsg(MB_YESNO, hwnd, NullStr, GetPString(IDS_ZEROBYTETEXT));
            if (temp == MBID_YES) {
              DefaultView(hwnd, (HWND) 0, hwndMain, NULL, 8, datafile);
            }
            fclose(fp);
            return 0;
          }
          else
            checked = TRUE;
        }
        if (didmatch) {
          if (info->sig && *info->sig) {
            strcpy(sigl, info->sig);
            literal(sigl);
            
              if (info->offset < 0L) {
                fseek(fp, 0L, SEEK_END);
                offset = ftell(fp) + info->offset;
              }
              else
                offset = info->offset;
              fseek(fp, offset, SEEK_SET);
              if (fread(sig,
                        1,
                        strlen(sigl),
                        fp) != strlen(sigl) || strncmp(sigl, sig, strlen(sigl)))
                didmatch = FALSE;
              
          }
        }
        fclose(fp);
      }
      if (didmatch) {			// got a match; do it...

	CHAR *list[2];
	INT flags, rc;
	BOOL dieafter = FALSE;

	if (fAmAV2) {
	  if (stristr(info->pszCmdLine, "AV2.EXE") ||
	      stristr(info->pszCmdLine, "AV2.CMD") || stristr(info->pszCmdLine, "<>"))
	    return -1;
	}
	if (!strcmp(info->pszCmdLine, "<>")) {
	  OpenObject(datafile, Default, hwnd);
	  return 0;
	}
	list[0] = datafile;
	list[1] = NULL;
	flags = info->flags;
	if (!(flags & FULLSCREEN))
	  flags |= WINDOWED;
	if (flags & KEEP)
	  flags |= SEPARATEKEEP;
	else
	  flags |= SEPARATE;
	flags &= (~KEEP);
	if (flags & DIEAFTER)
	  dieafter = TRUE;
        flags &= (~DIEAFTER);
	rc = ExecOnList(hwnd,
			info->pszCmdLine,
			flags, NULL,
                        info->env != NullStr ? info->env : NULL,
                        list,
                        GetPString(IDS_EXECASSOCTITLETEXT),
			pszSrcFile, __LINE__);
	if (rc != -1 && dieafter)
	  PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
	return rc;
      }
      p = strtok(0, ";");
    }
    info = info->next;
  }
  return -1;
}

MRESULT EXPENTRY AssocDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  LINKASSOC *info;
  SHORT x, y;

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, ASS_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
    WinSendDlgItemMsg(hwnd, ASS_MASK, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, ASS_CL, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg - 1, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, ASS_ENVIRON, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(ENVIRONMENT_SIZE - 1, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, ASS_SIG, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, ASS_OFFSET, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(34, 0), MPVOID);
    WinSetDlgItemText(hwnd, ASS_MASK, NullStr);
    WinSetDlgItemText(hwnd, ASS_CL, NullStr);
    WinSetDlgItemText(hwnd, ASS_SIG, NullStr);
    WinSetDlgItemText(hwnd, ASS_OFFSET, "0");
    WinCheckButton(hwnd, ASS_DEFAULT, TRUE);
    WinCheckButton(hwnd, ASS_PROMPT, FALSE);
    WinCheckButton(hwnd, ASS_KEEP, FALSE);
    WinCheckButton(hwnd, ASS_DIEAFTER, FALSE);
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    {
      PFNWP oldproc;

      oldproc = WinSubclassWindow(WinWindowFromID(hwnd, ASS_CL),
				  (PFNWP) AssocTextProc);
      if (oldproc)
	WinSetWindowPtr(WinWindowFromID(hwnd, ASS_CL), QWL_USER,
			(PVOID) oldproc);
    }
    break;

  case UM_UNDO:
    {
      PSZ pszDisplayStr;

      pszDisplayStr = xmallocz((CCHMAXPATH * 2) + MaxComLineStrg + 6,
			       pszSrcFile, __LINE__);
      if (pszDisplayStr) {
	WinSendDlgItemMsg(hwnd, ASS_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
	info = asshead;
	while (info) {
	  sprintf(pszDisplayStr,
		  "%-12s \x1a %-24s %s%s%s",
		  info->mask,
		  info->pszCmdLine,
		  (info->sig && *info->sig) ?
		  "[" : NullStr,
		  (info->sig && *info->sig) ? info->sig : NullStr,
		  (info->sig && *info->sig) ? "]" : NullStr);
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					ASS_LISTBOX,
					LM_INSERTITEM,
					MPFROM2SHORT(LIT_END, 0), MPFROMP(pszDisplayStr));
	  if (x >= 0)
	    WinSendDlgItemMsg(hwnd,
			      ASS_LISTBOX,
			      LM_SETITEMHANDLE, MPFROMSHORT(x), MPFROMP(info));
	  info = info->next;
	}
	WinSendDlgItemMsg(hwnd,
			  ASS_LISTBOX,
			  LM_SELECTITEM, MPFROMSHORT(0), MPFROMSHORT(TRUE));
	xfree(pszDisplayStr, pszSrcFile, __LINE__);
      }
    }
    return 0;

  case WM_CONTROL:
    if (SHORT1FROMMP(mp1) == ASS_LISTBOX) {
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
      case LN_SELECT:
	x = (SHORT) WinSendDlgItemMsg(hwnd,
				      ASS_LISTBOX,
				      LM_QUERYSELECTION,
				      MPFROMSHORT(LIT_FIRST), MPVOID);
	if (x >= 0) {

	  CHAR s[36];

	  info = (LINKASSOC *) WinSendDlgItemMsg(hwnd,
						 ASS_LISTBOX,
						 LM_QUERYITEMHANDLE,
						 MPFROMSHORT(x), MPVOID);
	  if (!info) {
	    Runtime_Error(pszSrcFile, __LINE__, "Query item handle failed");
	    break;
	  }
	  WinSetDlgItemText(hwnd, ASS_MASK, info->mask);
	  WinSetDlgItemText(hwnd, ASS_CL, info->pszCmdLine);
	  WinSetDlgItemText(hwnd, ASS_SIG,
			    (info->sig && *info->sig) ? info->sig : NullStr);
	  sprintf(s, "%ld", info->offset);
	  WinSetDlgItemText(hwnd, ASS_OFFSET, s);
	  if (!(info->flags & 1023))
	    WinCheckButton(hwnd, ASS_DEFAULT, TRUE);
	  else {
	    if (info->flags & FULLSCREEN)
	      WinCheckButton(hwnd, ASS_FULLSCREEN, TRUE);
	    else if (info->flags & MINIMIZED)
	      WinCheckButton(hwnd, ASS_MINIMIZED, TRUE);
	    else if (info->flags & MAXIMIZED)
	      WinCheckButton(hwnd, ASS_MAXIMIZED, TRUE);
	    else if (info->flags & INVISIBLE)
	      WinCheckButton(hwnd, ASS_INVISIBLE, TRUE);
	  }
	  WinCheckButton(hwnd, ASS_KEEP, ((info->flags & KEEP) != 0));
	  WinCheckButton(hwnd, ASS_DIEAFTER, ((info->flags & DIEAFTER) != 0));
	  WinCheckButton(hwnd, ASS_PROMPT, ((info->flags & PROMPT) != 0));
	  {
	    CHAR env[ENVIRONMENT_SIZE];
            //ULONG size;
            CHAR key[CCHMAXPATH];

	    *env = 0;
            //size = sizeof(env) - 1;
            sprintf(key, "ASSOC.%senv", info->pszCmdLine);
            if (PrfQueryProfileString(fmprof, FM3Str, key, NullStr, env, sizeof(env)) &&
                env != NullStr)
	      WinSetDlgItemText(hwnd, ASS_ENVIRON, env);
	    else
	      WinSetDlgItemText(hwnd, ASS_ENVIRON, NullStr);
	  }
	}
	break;
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case ASS_TOP:
      x = (SHORT) WinSendDlgItemMsg(hwnd, ASS_LISTBOX,
				    LM_QUERYSELECTION,
				    MPFROMSHORT(LIT_FIRST), MPVOID);
      if (x >= 0) {
	info = (LINKASSOC *) WinSendDlgItemMsg(hwnd, ASS_LISTBOX,
					       LM_QUERYITEMHANDLE,
					       MPFROMSHORT(x), MPVOID);
	if (info) {
	  if (info != asshead) {
	    if (info->prev)
	      info->prev->next = info->next;
	    if (info->next)
	      info->next->prev = info->prev;
	    if (info == asstail)
	      asstail = info->prev;
	    info->prev = NULL;
	    info->next = asshead;
	    asshead->prev = info;
	    asshead = info;
            WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
            save_associations();
	  }
	}
      }
      break;

    case ASS_BOTTOM:
      x = (SHORT) WinSendDlgItemMsg(hwnd, ASS_LISTBOX,
				    LM_QUERYSELECTION,
				    MPFROMSHORT(LIT_FIRST), MPVOID);
      if (x >= 0) {
	info = (LINKASSOC *) WinSendDlgItemMsg(hwnd, ASS_LISTBOX,
					       LM_QUERYITEMHANDLE,
					       MPFROMSHORT(x), MPVOID);
	if (info) {
	  if (info != asstail) {
	    if (info->next)
	      info->next->prev = info->prev;
	    if (info->prev)
	      info->prev->next = info->next;
	    if (info == asshead)
	      asshead = info->next;
	    info->next = NULL;
	    info->prev = asstail;
	    asstail->next = info;
	    asstail = info;
            WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
            save_associations();
	  }
	}
      }
      break;
    case ASS_FIND:
      {
	CHAR filename[CCHMAXPATH + 9], szfilename[CCHMAXPATH + 9];

	*filename = 0;
	if (insert_filename(hwnd, filename, 2, FALSE) && *filename) {
	  BldQuotedFileName(szfilename, filename);
	  strcat(szfilename, " %a");
	  WinSetDlgItemText(hwnd, ASS_CL, szfilename);
	}
      }
      break;

    case DID_OK:
      {
	ASSOC temp;
	CHAR dummy[34];
	PSZ pszWorkBuf;
	replace = FALSE;

	memset(&temp, 0, sizeof(ASSOC));
	temp.pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
	if (!temp.pszCmdLine)
	  break; //already complained

	{
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					ASS_LISTBOX,
					LM_QUERYSELECTION, MPVOID, MPVOID);
	  if (x == LIT_NONE)
	    x = (SHORT) WinSendDlgItemMsg(hwnd,
					  ASS_LISTBOX,
					  LM_SELECTITEM,
					  MPFROMSHORT(0), MPFROMSHORT(TRUE));
	}
	pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
	if (!pszWorkBuf) {
	  free(temp.pszCmdLine);
	  break; //already complained
	}
	WinQueryDlgItemText(hwnd, ASS_MASK, sizeof(temp.mask), temp.mask);
	WinQueryDlgItemText(hwnd, ASS_CL, MaxComLineStrg, temp.pszCmdLine);
	if (strcmp(temp.pszCmdLine, "<>")) {
	  NormalizeCmdLine(pszWorkBuf, temp.pszCmdLine);
	  memcpy(temp.pszCmdLine, pszWorkBuf, strlen(pszWorkBuf) + 1);
	}
	free(pszWorkBuf);
	WinQueryDlgItemText(hwnd, ASS_SIG, sizeof(temp.sig), temp.sig);
	rstrip(temp.sig);
	if (*temp.sig) {
	  WinQueryDlgItemText(hwnd, ASS_OFFSET, sizeof(dummy), dummy);
	  temp.offset = atol(dummy);
	}
	bstrip(temp.mask);
        bstrip(temp.pszCmdLine);
        temp.flags = CheckExecutibleFlags(hwnd, 1);
	if (fCancelAction){
	  fCancelAction = FALSE;
	  free(temp.pszCmdLine);
	  break;
	}
	else
	info = add_association(&temp);
	if (!info)
	  WinDismissDlg(hwnd, 1);	
	else {
	  display_associations(hwnd, &temp, info);
	  save_associations();
	}
	free(temp.pszCmdLine);
      }
      WinDismissDlg(hwnd, 1);
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_ASSOC, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case ASS_ADD:
      {
	ASSOC temp;
	CHAR dummy[34];
	PSZ pszWorkBuf;
	replace = FALSE;

	memset(&temp, 0, sizeof(ASSOC));
	temp.pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
	if (!temp.pszCmdLine)
	  break; //already complained
	pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
	if (!pszWorkBuf) {
	  free(temp.pszCmdLine);
	  break; //already complained
	}
	WinQueryDlgItemText(hwnd, ASS_MASK, sizeof(temp.mask), temp.mask);
	WinQueryDlgItemText(hwnd, ASS_CL, MaxComLineStrg, temp.pszCmdLine);
	if (strcmp(temp.pszCmdLine, "<>")) {
	  NormalizeCmdLine(pszWorkBuf, temp.pszCmdLine);
	  memcpy(temp.pszCmdLine, pszWorkBuf, strlen(pszWorkBuf) + 1);
	}
	free(pszWorkBuf);
	WinQueryDlgItemText(hwnd, ASS_SIG, sizeof(temp.sig), temp.sig);
	rstrip(temp.sig);
	if (*temp.sig) {
	  WinQueryDlgItemText(hwnd, ASS_OFFSET, sizeof(dummy), dummy);
	  temp.offset = atol(dummy);
	}
	bstrip(temp.mask);
        bstrip(temp.pszCmdLine);
        temp.flags = CheckExecutibleFlags(hwnd, 1);
	if (fCancelAction){
	  fCancelAction = FALSE;
	  free(temp.pszCmdLine);
	  break;
	}
	else
	  info = add_association(&temp);
	//Add will fail if mask is not changed
	if (info) {
	  display_associations(hwnd, &temp, info);
	  save_associations();
	}
	free(temp.pszCmdLine);
      }
      break;

    case ASS_DELETE:
      {
	ASSOC temp;
        CHAR dummy[34];
        CHAR key[CCHMAXPATH];

	memset(&temp, 0, sizeof(ASSOC));
	temp.pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
	if (!temp.pszCmdLine)
	  break; //already complained
	WinQueryDlgItemText(hwnd, ASS_MASK, sizeof(temp.mask), temp.mask);
	WinQueryDlgItemText(hwnd, ASS_SIG, sizeof(temp.sig), temp.sig);
	rstrip(temp.sig);
	if (*temp.sig) {
	  WinQueryDlgItemText(hwnd, ASS_OFFSET, sizeof(dummy), dummy);
	  temp.offset = atol(dummy);
	}
        sprintf(key, "ASSOC.%senv", temp.pszCmdLine);
	PrfWriteProfileString(fmprof, FM3Str, key, NULL);
	if (kill_association(&temp)) {
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					ASS_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (x >= 0) {
	    WinSendDlgItemMsg(hwnd,
			      ASS_LISTBOX,
			      LM_DELETEITEM, MPFROMSHORT(x), MPVOID);
	    WinSendDlgItemMsg(hwnd, ASS_LISTBOX, LM_SELECTITEM,
			      MPFROMSHORT(LIT_NONE), MPFROMSHORT(FALSE));
	  }
	  save_associations();
	}
	free(temp.pszCmdLine);
      }

      break;
    case ASS_REPLACE:

      {
	ASSOC temp;
        CHAR dummy[34];
	PSZ pszWorkBuf;
	replace = TRUE;

	memset(&temp, 0, sizeof(ASSOC));
	temp.pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
	if (!temp.pszCmdLine)
	  break; //already complained
	y = (SHORT) WinSendDlgItemMsg(hwnd,
				      ASS_LISTBOX,
				      LM_QUERYSELECTION,
				      MPFROMSHORT(LIT_CURSOR), MPVOID);
	pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
	if (!pszWorkBuf) {
	  free(temp.pszCmdLine);
	  break; //already complained
	}
	WinQueryDlgItemText(hwnd, ASS_MASK, sizeof(temp.mask), temp.mask);
	WinQueryDlgItemText(hwnd, ASS_CL, MaxComLineStrg, temp.pszCmdLine);
	if (strcmp(temp.pszCmdLine, "<>")) {
	  NormalizeCmdLine(pszWorkBuf, temp.pszCmdLine);
	  memcpy(temp.pszCmdLine, pszWorkBuf, strlen(pszWorkBuf) + 1);
	}
	free(pszWorkBuf);
	WinQueryDlgItemText(hwnd, ASS_SIG, sizeof(temp.sig), temp.sig);
	rstrip(temp.sig);
	if (*temp.sig) {
	  WinQueryDlgItemText(hwnd, ASS_OFFSET, sizeof(dummy), dummy);
	  temp.offset = atol(dummy);
	}
	bstrip(temp.mask);
        bstrip(temp.pszCmdLine);
        WinQueryDlgItemText(hwnd, ASS_ENVIRON, sizeof(temp.env), temp.env);
        bstrip(temp.env);
        temp.flags = CheckExecutibleFlags(hwnd, 1);
	if (fCancelAction){
	  fCancelAction = FALSE;
	  free(temp.pszCmdLine);
	  break;
	}
	else
	  info = add_association(&temp);
	if (info) {
	  display_associations(hwnd, &temp, info);
	  save_associations();
	}
	free(temp.pszCmdLine);
      }
      {
	ASSOC temp;
        CHAR dummy[34];
        CHAR key[CCHMAXPATH];

	temp.pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
	if (!temp.pszCmdLine)
	  break; //already complained
	WinSendDlgItemMsg(hwnd,
			  ASS_LISTBOX,
			  LM_SELECTITEM, MPFROMSHORT(y), MPFROMSHORT(TRUE));
	memset(temp.sig, 0, sizeof(temp.sig));
	memset(temp.mask, 0, sizeof(temp.mask));
	temp.offset = 0;
	WinQueryDlgItemText(hwnd, ASS_MASK, sizeof(temp.mask), temp.mask);
	WinQueryDlgItemText(hwnd, ASS_SIG, sizeof(temp.sig), temp.sig);
	rstrip(temp.sig);
	if (*temp.sig) {
	  WinQueryDlgItemText(hwnd, ASS_OFFSET, sizeof(dummy), dummy);
	  temp.offset = atol(dummy);
	}
        sprintf(key, "ASSOC.%senv", temp.pszCmdLine);
	PrfWriteProfileString(fmprof, FM3Str, key, NULL);
	if (!kill_association(&temp))
	  Runtime_Error(pszSrcFile, __LINE__, "kill_association");
	else {

	  if (y >= 0) {
	    WinSendDlgItemMsg(hwnd,
			      ASS_LISTBOX,
			      LM_DELETEITEM, MPFROMSHORT(y), MPVOID);
	    WinSendDlgItemMsg(hwnd, ASS_LISTBOX, LM_SELECTITEM,
			      MPFROMSHORT(x - 1), MPFROMSHORT(TRUE));
	  }
	  save_associations();
	}
	free(temp.pszCmdLine);
      }
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

VOID EditAssociations(HWND hwnd)
{
  static CHAR stop = 0;

  if (stop)
    return;
  stop++;
  if (!assloaded)
    load_associations();
  WinDlgBox(HWND_DESKTOP, hwnd, AssocDlgProc, FM3ModHandle, ASS_FRAME, NULL);
  stop = 0;
}

#pragma alloc_text(ASSOC2,free_associations,load_associations,save_associations,display_associations)
#pragma alloc_text(ASSOC2,ExecAssociation,AssocTextProc)
#pragma alloc_text(ASSOC,add_association,kill_association,AssocDlgProc,EditAssociations)
