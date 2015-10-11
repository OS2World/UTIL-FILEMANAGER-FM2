
/***********************************************************************

  $Id$

  Custom commands

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2010 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  06 Jun 05 SHL Drop unused code
  14 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets_bstripcr
  15 Aug 06 SHL Better can't add message
  18 Sep 06 GKY Add replace command and update okay to add if changed
  17 Feb 07 GKY Move error messages etc to string file
  22 Mar 07 GKY Use QWL_USER
  23 Mar 07 GKY Replace doesn't change item position
  23 Mar 07 GKY Okay fails silently when item not changed
  19 Apr 07 SHL Sync with AcceptOneDrop GetOneDrop mods
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  06 Jan 08 GKY Use NormalizeCmdLine to check program strings on entry
  29 Feb 08 GKY Changes to enable user settable command line length
  29 Feb 08 GKY Use xfree where appropriate
  18 Jul 08 SHL Add Fortify support
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory and use BldFullPathName
  24 Aug 08 GKY Warn full drive on save of .DAT file; prevent loss of existing file
  15 Oct 08 GKY Prevent asking to add %a on NormalizeCmdLine abort
  21 Dec 09 GKY Fix the environment so it can be saved, deleted and used consistently.
  21 Dec 09 GKY Allow command menu reorder without changing the "ID" or hot key for a command.
                Added load_inicommand to load the IDs from the ini file.
  21 Dec 09 GKY Added 20 new hot keys for commands.
  21 Dec 09 GKY Added CheckExecutibleFlags to streamline code in command.c assoc.c & cmdline.c
  27 Dec 09 GKY Moved Commands to the INI file this makes commands.dat obsolete
  27 Dec 09 GKY Added QueryCommandSettings to streamline code
  27 Dec 09 GKY Made command hotkeys user selectable.
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10).
                Mostly cast CHAR CONSTANT * as CHAR *.
  01 May 10 GKY Add ENVIRONMENT_SIZE variable to standardize this size everywhere.
  01 May 10 GKY Changes to move environment storage to INI file
  03 Jul 11 GKY Fixed problem with creation of duplicate command IDs.
  24 Sep 11 GKY Fixed trap caused by selecting cancel from duplicate hotkey dialog if adding or replacing a command
  28 Jun 14 GKY Fix errors identified with CPPCheck

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "newview.h"			// Data declarations
#include "fm3dlg.h"
#include "fm3str.h"
#include "tools.h"
#include "arccnrs.h"			// BldQuotedFileName
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "droplist.h"			// AcceptOneDrop, DropHelp, GetOneDrop
#include "misc.h"			// DrawTargetEmphasis
#include "systemf.h"			// ExecOnList
#include "getnames.h"			// insert_filename
#include "wrappers.h"			// xfgets
#include "pathutil.h"                   // NormalizeCmdLine
#include "command.h"
#include "strips.h"			// bstrip
#include "dirs.h"			// save_dir2
#include "fortify.h"

VOID load_inicommands(VOID);

typedef struct
{
  PSZ pszCmdLine;
  CHAR title[100];
  CHAR env[ENVIRONMENT_SIZE];
  ULONG flags;
  ULONG ID;
  ULONG HotKeyID;
}
COMMAND;

BOOL QueryCommandSettings(HWND hwnd, COMMAND *temp);
VOID save_commands(VOID);

// Data defintions
#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;
static LINKCMDS *cmdtail;
static BOOL UsedCommandIDs[300] = {FALSE};
static BOOL UsedHotKeyIDs[40] = {FALSE};
static PSZ pszCommandsList;
static ULONG ulSizeCommandsList = 10000;
static BOOL fLoadCommandsFromINI = FALSE;

#pragma data_seg(GLOBAL2)
LINKCMDS *cmdhead;
BOOL cmdloaded;

MRESULT EXPENTRY CommandTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
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
    DropHelp(mp1, mp2, hwnd, GetPString(IDS_DROPCMDHELPTEXT));
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

MRESULT EXPENTRY ReOrderProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    if (!cmdhead) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    {
      LINKCMDS *info;
      SHORT x;

      info = cmdhead;
      while (info) {
	x = (SHORT) WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX, LM_INSERTITEM,
				      MPFROMSHORT(LIT_END),
				      MPFROMP(info->title));
	if (x < 0) {
	  Runtime_Error(pszSrcFile, __LINE__, "no cmd");
	  WinDismissDlg(hwnd, 0);
	}
	else {
	  WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX, LM_SETITEMHANDLE,
			    MPFROMSHORT(x), MPFROMP(info));
	}
	info = info->next;
      }
    }
    break;

  case WM_CONTROL:
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case DID_OK:
      {
	LINKCMDS *temphead = NULL, *info, *last = NULL, *temptail = NULL;
	SHORT sSelect, numitems;

	sSelect = 0;
	numitems = (SHORT) WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX,
					     LM_QUERYITEMCOUNT,
					     MPVOID, MPVOID);
	while (numitems) {
	  info = (LINKCMDS *) WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX,
						LM_QUERYITEMHANDLE,
						MPFROMSHORT(sSelect++),
						MPVOID);
	  if (info) {
	    if (!temphead) {
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
	numitems = (SHORT) WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX,
					     LM_QUERYITEMCOUNT,
					     MPVOID, MPVOID);
	while (numitems) {
	  info = (LINKCMDS *) WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX,
						LM_QUERYITEMHANDLE,
						MPFROMSHORT(sSelect++),
						MPVOID);
	  if (info) {
	    if (!temphead) {
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
      WinDismissDlg(hwnd, 1);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_REORDERCOMMANDS, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;

    case RE_ADD:
      {
	SHORT sSelect, x;
	LINKCMDS *info;

	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(LIT_FIRST), MPVOID);
	while (sSelect >= 0) {
	  info = (LINKCMDS *) WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX,
						LM_QUERYITEMHANDLE,
						MPFROMSHORT(sSelect), MPVOID);
	  if (info) {
	    x = (SHORT) WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX,
					  LM_INSERTITEM,
					  MPFROM2SHORT(LIT_END, 0),
					  MPFROMP(info->title));
	    if (x >= 0) {
	      WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX,
				LM_SETITEMHANDLE,
				MPFROMSHORT(x), MPFROMP(info));
	      WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX, LM_DELETEITEM,
				MPFROMSHORT(sSelect), MPVOID);
	    }
	  }
	  else
	    WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX, LM_SELECTITEM,
			      MPFROMSHORT(sSelect), MPFROMSHORT(FALSE));
	  sSelect = (USHORT) WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX,
					       LM_QUERYSELECTION,
					       MPFROMSHORT(LIT_FIRST),
					       MPVOID);
	}
      }
      break;

    case RE_REMOVE:
      {
	SHORT sSelect, x;
	LINKCMDS *info;

	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(LIT_FIRST), MPVOID);
	while (sSelect >= 0) {
	  info = (LINKCMDS *) WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX,
						LM_QUERYITEMHANDLE,
						MPFROMSHORT(sSelect), MPVOID);
	  if (info) {
	    x = (SHORT) WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX,
					  LM_INSERTITEM,
					  MPFROM2SHORT(LIT_END, 0),
					  MPFROMP(info->title));
	    if (x >= 0) {
	      WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX,
				LM_SETITEMHANDLE,
				MPFROMSHORT(x), MPFROMP(info));
	      WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX, LM_DELETEITEM,
				MPFROMSHORT(sSelect), MPVOID);
	    }
	  }
	  else
	    WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX, LM_SELECTITEM,
			      MPFROMSHORT(sSelect), MPFROMSHORT(FALSE));
	  sSelect = (USHORT) WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX,
					       LM_QUERYSELECTION,
					       MPFROMSHORT(LIT_FIRST),
					       MPVOID);
	}
      }
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

CHAR *command_title(INT cx)
{
  static CHAR duh[] = "???";
  LINKCMDS *info;
  INT x = 0;

  if (!cmdloaded)
    load_commands();
  info = cmdhead;
  while (info) {
    if (x == cx)
      return info->title;
    info = info->next;
  }
  return duh;
}

VOID free_commands(VOID)
{
  LINKCMDS *info, *next;

  info = cmdhead;
  while (info) {
    next = info->next;
    xfree(info->title, pszSrcFile, __LINE__);
    xfree(info->pszCmdLine, pszSrcFile, __LINE__);
    xfree(info->env, pszSrcFile, __LINE__);
    free(info);
    info = next;
  }
  free(pszCommandsList);
  cmdhead = cmdtail = NULL;
}

VOID load_commands(VOID)
{
  FILE *fp;
  LINKCMDS *info;
  PSZ pszCmdLine;
  CHAR title[100];
  CHAR flags[34];
  CHAR *moder = "r";
  ULONG size;

  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "COMMANDS.LoadCommandsFromINI",
                      &fLoadCommandsFromINI, &size);
  if (!fLoadCommandsFromINI) {
    if (cmdhead)
      free_commands();
    cmdloaded = TRUE;
    pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
    pszCommandsList = xmallocz(ulSizeCommandsList, pszSrcFile, __LINE__);
    if (pszCmdLine) {
      BldFullPathName(pszCmdLine, pFM2SaveDirectory, PCSZ_COMMANDSDAT);
      fp = xfsopen(pszCmdLine, moder, SH_DENYWR, pszSrcFile, __LINE__, TRUE);
      if (fp) {
        while (!feof(fp)) {
          if (!xfgets_bstripcr(title, sizeof(title), fp, pszSrcFile, __LINE__))
            break;
          if (!*title || *title == ';')
            continue;
          if (!xfgets_bstripcr(pszCmdLine, MaxComLineStrg, fp, pszSrcFile, __LINE__))
            break;				// error!
          if (!xfgets_bstripcr(flags, sizeof(flags), fp, pszSrcFile, __LINE__))
            break;
          flags[33] = 0;
          if (!pszCmdLine)
            continue;
          info = xmallocz(sizeof(LINKCMDS), pszSrcFile, __LINE__);
          if (info) {
  #	  ifdef FORTIFY
            Fortify_SetOwner(info, 1);
            Fortify_SetScope(info, 1);
  #	  endif
            info->pszCmdLine = xstrdup(pszCmdLine, pszSrcFile, __LINE__);

            if (pszCommandsList) {
              strcpy(pszCommandsList + strlen(pszCommandsList), title);
              strcpy(pszCommandsList + strlen(pszCommandsList), ";");
            }
            info->title = xstrdup(title, pszSrcFile, __LINE__);
            info->flags = atol(flags);
            if (!info->pszCmdLine || !info->title) {
              xfree(info->pszCmdLine, pszSrcFile, __LINE__);
              xfree(info->title, pszSrcFile, __LINE__);
              free(info);
              break;
            }
  #	  ifdef FORTIFY
            Fortify_SetOwner(info->pszCmdLine, 1);
            Fortify_SetScope(info->pszCmdLine, 1);
            Fortify_SetOwner(info->title, 1);
            Fortify_SetScope(info->title, 1);
  #	  endif
            if (!cmdhead)
              cmdhead = info;
            else {
              cmdtail->next = info;
              info->prev = cmdtail;
            }
            cmdtail = info;
          }
        }
        free(pszCmdLine);
        fclose(fp);
        if (pszCommandsList) {
          ulSizeCommandsList = strlen(pszCommandsList) + 1;
        }
      }
    }
  }
  load_inicommands();
}

/**
 * load_inicommand loads the data from the ini file into an info struct
 * after COMMANDS.DAT has been loaded; It generates new IDs where necessary
 * it saves the environment from the old ini key and deletes it as needed
 **/

VOID load_inicommands(VOID)
{
  LINKCMDS *info;;
  INT x = 0;
  INT y = 0;
  ULONG ID = 0;
  ULONG HotKeyID = 0;
  CHAR env[ENVIRONMENT_SIZE];
  CHAR key[120];
  CHAR szTitle[100];
  ULONG size;
  CHAR *p, *pp;
  PSZ pszCmdLine;
  ULONG flags;

#   ifdef FORTIFY
  Fortify_EnterScope();
#    endif
  pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
  if (pszCmdLine) {
    if (fLoadCommandsFromINI) {
      if (cmdhead)
        free_commands();
      cmdloaded = TRUE;
      size = sizeof(ULONG);
      PrfQueryProfileData(fmprof, FM3Str, "COMMANDS.SizeSortOrder",
                          &ulSizeCommandsList, &size);
      pszCommandsList = xmallocz(ulSizeCommandsList, pszSrcFile, __LINE__);
      if (pszCommandsList) {
        PrfQueryProfileString(fmprof, FM3Str, "COMMANDS.SortOrder",
                              NullStr, pszCommandsList, ulSizeCommandsList);
        p = pszCommandsList;
        while (*p == ';')
          p++;
        while (*p) {
          *szTitle = 0;
          pp = szTitle;
          while (*p && *p != ';')
            *pp++ = *p++;
          *pp = 0;
          while (*p == ';')
            p++;
          if (*szTitle) {
            bstripcr(szTitle);
            sprintf(key, "COMMAND.%sID", szTitle);
            size = sizeof(ULONG);
            PrfQueryProfileData(fmprof, FM3Str, key, &ID, &size);
            UsedCommandIDs[ID - IDM_COMMANDSTART] = TRUE; // No need to use profile just count them GKY 02 JUL 11
            sprintf(key, "COMMAND.%sHotKeyID", szTitle);
            size = sizeof(ULONG);
            PrfQueryProfileData(fmprof, FM3Str, key, &HotKeyID, &size);
            UsedHotKeyIDs[HotKeyID - IDM_COMMANDNUM0] = TRUE;  // No need to use profile just count them GKY 02 JUL 11
            sprintf(key, "COMMAND.%sflags", szTitle);
            size = sizeof(ULONG);
            PrfQueryProfileData(fmprof, FM3Str, key, &flags, &size);
            sprintf(key, "COMMAND.%senv", szTitle);
            PrfQueryProfileString(fmprof, FM3Str, key, NullStr, env, sizeof(env));
            sprintf(key, "COMMAND.%sCmdLine", szTitle);
            PrfQueryProfileString(fmprof, FM3Str, key, NullStr, pszCmdLine, MaxComLineStrg);
          }
          info = xmallocz(sizeof(LINKCMDS), pszSrcFile, __LINE__);
          if (info) {
  #	  ifdef FORTIFY
            Fortify_SetOwner(info, 1);
            Fortify_SetScope(info, 1);
  #	  endif
            info->pszCmdLine = xstrdup(pszCmdLine, pszSrcFile, __LINE__);
            info->title = xstrdup(szTitle, pszSrcFile, __LINE__);
            info->flags = flags;
            if (env != NullStr)
              info->env = xstrdup(env, pszSrcFile, __LINE__);
            info->ID = ID;
            info->HotKeyID = HotKeyID;
            if (!info->pszCmdLine || !info->title) {
              xfree(info->pszCmdLine, pszSrcFile, __LINE__);
              xfree(info->title, pszSrcFile, __LINE__);
              xfree(info->env, pszSrcFile, __LINE__);
              free(info);
              break;
            }
            if (!cmdhead)
              cmdhead = info;
            else {
              cmdtail->next = info;
              info->prev = cmdtail;
            }
            cmdtail = info;
          }
        }
      }
    }
    else {
      info = cmdhead;
      while (info) {
        bstripcr(info->title);
        sprintf(key, "COMMAND.%sID", info->title);
        size = sizeof(ULONG);
        PrfQueryProfileData(fmprof, FM3Str, key, &ID, &size);
        sprintf(key, "COMMAND.%sHotKeyID", info->title);
        size = sizeof(ULONG);
        PrfQueryProfileData(fmprof, FM3Str, key, &HotKeyID, &size);
        sprintf(key, "COMMAND.%senv", info->title);
        PrfQueryProfileString(fmprof, FM3Str, key, NullStr, env, sizeof(env));
        if (ID != 0) {
          if (env != NullStr)
            info->env = xstrdup(env, pszSrcFile, __LINE__);
          info->ID = ID;
          info->HotKeyID = HotKeyID;
        }
        //This updates the old commands.dat file to the new format
        //assigning the IDs based on file order or on next available ID 
        else {
          for (x = 0; x < 300; x++) {
            if (!UsedCommandIDs[x]) {
              ID = info->ID = IDM_COMMANDSTART + x;
              UsedCommandIDs[x] = TRUE;
              for (y = 0; y < 40; y++) {
                if (!UsedHotKeyIDs[y]) {
                  HotKeyID = info->HotKeyID = IDM_COMMANDNUM0 + y;
                  UsedHotKeyIDs[y] = TRUE;
                  break;
                }
              }
              break;
            }
            if (x == 299)
              saymsg(MB_OK | MB_ICONEXCLAMATION , HWND_DESKTOP,
                     GetPString(IDS_COMMANDSLIMITTITLETEXT),
                     GetPString(IDS_COMMANDSLIMITREACHEDTEXT ));
          }
          PrfQueryProfileString(fmprof, FM3Str, info->pszCmdLine, NullStr, env, sizeof(env));
          info->env = xstrdup(env, pszSrcFile, __LINE__);
        }
        ID = 0;
        HotKeyID = 0;
        info = info->next;
      }
      fLoadCommandsFromINI = TRUE;
      save_commands();
    }
    free(pszCmdLine);
  }
}

VOID save_commands(VOID)
{
  LINKCMDS *info;
  CHAR key[120];
  
  if (!cmdloaded || !cmdhead)
    return;
  
  info = cmdhead;
  pszCommandsList[0] = 0;
    while (info) {
      sprintf(key, "COMMAND.%sflags", info->title);
      PrfWriteProfileData(fmprof, FM3Str, key, &info->flags, sizeof(ULONG));
      sprintf(key, "COMMAND.%sCmdLine", info->title);
      PrfWriteProfileString(fmprof, FM3Str, key, info->pszCmdLine);
      bstripcr(info->title);
      sprintf(key, "COMMAND.%sID", info->title);
      PrfWriteProfileData(fmprof, FM3Str, key, &info->ID, sizeof(INT));
      UsedCommandIDs[info->ID - IDM_COMMANDSTART] = TRUE;
      sprintf(key, "COMMAND.%sHotKeyID", info->title);
      PrfWriteProfileData(fmprof, FM3Str, key, &info->HotKeyID, sizeof(INT));
      UsedHotKeyIDs[info->HotKeyID - IDM_COMMANDNUM0] = TRUE;
      if (info->env != NullStr) {
        sprintf(key, "COMMAND.%senv", info->title);
        PrfWriteProfileString(fmprof, FM3Str, key, info->env);
      }
      if ((strlen(pszCommandsList) + strlen(info->title) + 1) > ulSizeCommandsList)
        pszCommandsList = xrealloc(pszCommandsList,
                                   ulSizeCommandsList + strlen(info->title) + 1,
                                   pszSrcFile, __LINE__);
      strcpy(pszCommandsList + strlen(pszCommandsList), info->title);
      strcpy(pszCommandsList + strlen(pszCommandsList), ";");
      info = info->next;
    } // while info
    PrfWriteProfileData(fmprof, FM3Str, "COMMANDS.UsedCommandIDs", &UsedCommandIDs,
                        sizeof(BOOL) * 300); // left for backward compatability GKY 02 Jul 11
    PrfWriteProfileData(fmprof, FM3Str, "COMMANDS.UsedHotKeyIDs", &UsedHotKeyIDs,
                        sizeof(BOOL) * 40);// left for backward compatability GKY 02 Jul 11
    ulSizeCommandsList = strlen(pszCommandsList) + 1;
    PrfWriteProfileData(fmprof, FM3Str, "COMMANDS.SizeSortOrder",
                        &ulSizeCommandsList, sizeof(ULONG));
    PrfWriteProfileString(fmprof, FM3Str, "COMMANDS.SortOrder", pszCommandsList);
    PrfWriteProfileData(fmprof, FM3Str, "COMMANDS.LoadCommandsFromINI",
                        &fLoadCommandsFromINI, sizeof(BOOL));
}

//== add_command() Add command to list ==

LINKCMDS *add_command(COMMAND *addme, BOOL fDontCheckHotKey)
{
  LINKCMDS *info;
  INT x;
  INT y;
  APIRET ret;

  if (!addme || !*addme->pszCmdLine || !*addme->title)
    return NULL;			// No data
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
  info = cmdhead;
  while (info) {
    if (!stricmp(info->title, addme->title))
      return NULL;			// Got a dup
    info = info->next;
  }
  if (!fDontCheckHotKey && UsedHotKeyIDs[addme->HotKeyID - IDM_COMMANDNUM0]) {
    info = cmdhead;
    while (info) {
      if (info->HotKeyID == addme->HotKeyID) { //avoid assigning hot key to multiple commands
        ret = saymsg(MB_YESNOCANCEL, HWND_DESKTOP, NullStr,
                     GetPString(IDS_DUPLICATEHOTKEYTEXT));
        if (ret == MBID_YES) {
          info->HotKeyID = 0;  
          UsedHotKeyIDs[addme->HotKeyID - IDM_COMMANDNUM0] = FALSE;
          break;
        }
        else if (ret == MBID_NO) {
          addme->HotKeyID = 0;
          break;
        }
        else
          return (LINKCMDS *) -1;
      }
      info = info->next;
    }
  }
  info = xmallocz(sizeof(LINKCMDS), pszSrcFile, __LINE__);
  if (!info)
    return NULL;
  info->pszCmdLine = xstrdup(addme->pszCmdLine, pszSrcFile, __LINE__);
  info->title = xstrdup(addme->title, pszSrcFile, __LINE__);
  info->HotKeyID = addme->HotKeyID;
  info->ID = addme->ID;
  if (info->HotKeyID >= IDM_COMMANDNUM0 && info->HotKeyID <= IDM_COMMANDNUM39) {
    UsedHotKeyIDs[info->HotKeyID - IDM_COMMANDNUM0] = TRUE;
  }
  else
    info->HotKeyID = 0;
  if (!info->ID) {
    //profile updated by save_commands
    for (x = 0; x < 300; x++) {
      if (!UsedCommandIDs[x]) {
        info->ID = IDM_COMMANDSTART + x;
        UsedCommandIDs[x] = TRUE;
        if (!info->HotKeyID && !fDontCheckHotKey) {
          for (y = 0; y < 40; y++) {
            if (!UsedHotKeyIDs[y]) {
              info->HotKeyID = IDM_COMMANDNUM0 + y;
              UsedHotKeyIDs[y] = TRUE;
              break;
            }
          }
        }
        break;
      }
      if (x == 299)
        saymsg(MB_OK | MB_ICONEXCLAMATION , HWND_DESKTOP,
               GetPString(IDS_COMMANDSLIMITTITLETEXT),
               GetPString(IDS_COMMANDSLIMITREACHEDTEXT ));
    }
  }
  if (addme->flags)
    info->flags = addme->flags;
  if (addme->env)
    info->env = xstrdup(addme->env, pszSrcFile, __LINE__);
  if (!info->pszCmdLine || !info->title || !info->ID) {
    xfree(info->pszCmdLine, pszSrcFile, __LINE__);
    xfree(info->title, pszSrcFile, __LINE__);
    xfree(info->env, pszSrcFile, __LINE__);
    free(info);
    return NULL;
  }
  if (!cmdhead)				// only item in list
    cmdhead = cmdtail = info;
  else {
    // place at tail
    cmdtail->next = info;
    info->prev = cmdtail;
    cmdtail = info;
  }
  pszCommandsList = xrealloc(pszCommandsList, ulSizeCommandsList + strlen(info->title) + 1,
                             pszSrcFile, __LINE__);
  if (pszCommandsList) {
    strcpy(pszCommandsList + strlen(pszCommandsList), info->title);
    strcpy(pszCommandsList + strlen(pszCommandsList), ";");
    ulSizeCommandsList = ulSizeCommandsList + strlen(info->title) + 1;
  }
  else {
    pszCommandsList = xmallocz(ulSizeCommandsList, pszSrcFile, __LINE__);
    if (pszCommandsList)
      PrfQueryProfileString(fmprof, FM3Str, "COMMANDS.SortOrder",
                            NullStr, pszCommandsList, ulSizeCommandsList);
    return 0;
  }
  return info;
}

BOOL kill_command(CHAR * killme)
{
  LINKCMDS *info;

  if (killme && *killme) {
    info = cmdhead;
    while (info) {
      if (!stricmp(info->title, killme)) {
        UsedCommandIDs[info->ID - IDM_COMMANDSTART] = FALSE;
        if (info->HotKeyID)
          UsedHotKeyIDs[info->HotKeyID - IDM_COMMANDNUM0] = FALSE;
	if (info == cmdhead) {
	  cmdhead = info->next;
	  if (info == cmdtail)
	    cmdtail = info->prev;
	}
	else {
	  if (info->next)
	    (info->next)->prev = info->prev;
	  if (info->prev)
	    (info->prev)->next = info->next;
	  if (info == cmdtail)
	    cmdtail = info->prev;
	}
	xfree(info->pszCmdLine, pszSrcFile, __LINE__);
        xfree(info->title, pszSrcFile, __LINE__);
        xfree(info->env, pszSrcFile, __LINE__);
	free(info);
	return TRUE;
      }
      info = info->next;
    }
  }
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
  return FALSE;
}

BOOL QueryCommandSettings(HWND hwnd, COMMAND *temp)
{
   PSZ pszWorkBuf;
   APIRET ret;
   CHAR env[ENVIRONMENT_SIZE];
   INT x;

   pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
   if (!pszWorkBuf) {
     return FALSE; //already complained
   }
   WinQueryDlgItemText(hwnd, CMD_CL, MaxComLineStrg, temp->pszCmdLine);
   NormalizeCmdLine(pszWorkBuf, temp->pszCmdLine);
   memcpy(temp->pszCmdLine, pszWorkBuf, strlen(pszWorkBuf) + 1);
   free(pszWorkBuf);
   if (fCancelAction) {
     fCancelAction = FALSE;
     return FALSE;
   }
   if (!strchr(temp->pszCmdLine, '%')) {
     ret = saymsg(MB_YESNO,
                  HWND_DESKTOP,
                  NullStr,
                  GetPString(IDS_TOACTONSELECTEDTEXT));
     if (ret == MBID_YES)
       strcat(temp->pszCmdLine, " %a");
   }
   WinQueryDlgItemText(hwnd, CMD_TITLE, sizeof(temp->title), temp->title);
   bstripcr(temp->title);
   temp->flags = CheckExecutibleFlags(hwnd, 3);
   *env = 0;
   WinQueryDlgItemText(hwnd, CMD_ENVIRON, ENVIRONMENT_SIZE - 1, env);
   bstripcr(env);
   if (*env)
     strcpy(temp->env, env);
   x = (SHORT) WinSendDlgItemMsg(hwnd, CMD_HOTKEY,
				 LM_QUERYSELECTION,
				 MPFROMSHORT(LIT_FIRST), MPVOID);
   if (x < 40)
     temp->HotKeyID = x + IDM_COMMANDNUM0;
   else
     temp->HotKeyID = 0;
   return TRUE;
}

MRESULT EXPENTRY CommandDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SHORT x, y;
  LINKCMDS *info;

#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CMD_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
    WinSendDlgItemMsg(hwnd, CMD_CL, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(MaxComLineStrg - 1, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CMD_TITLE, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(99, 0), MPVOID);
    WinSetDlgItemText(hwnd, CMD_CL, NullStr);
    WinSetDlgItemText(hwnd, CMD_TITLE, NullStr);
    WinCheckButton(hwnd, CMD_DEFAULT, TRUE);
    WinCheckButton(hwnd, CMD_PROMPT, FALSE);
    WinCheckButton(hwnd, CMD_ONCE, FALSE);
    info = cmdhead;
    while (info) {
      x = (SHORT) WinSendDlgItemMsg(hwnd, CMD_LISTBOX, LM_INSERTITEM,
				    MPFROM2SHORT(LIT_END, 0),
				    MPFROMP(info->title));
      if (x >= 0)
	WinSendDlgItemMsg(hwnd, CMD_LISTBOX, LM_SETITEMHANDLE,
			  MPFROMSHORT(x), MPFROMP(info));
      info = info->next;
    }
    WinSendDlgItemMsg(hwnd, CMD_LISTBOX, LM_SELECTITEM,
		      MPFROMSHORT(0), MPFROMSHORT(TRUE));
    {
      PFNWP oldproc;

      oldproc = WinSubclassWindow(WinWindowFromID(hwnd, CMD_CL),
				  (PFNWP) CommandTextProc);
      if (oldproc)
	WinSetWindowPtr(WinWindowFromID(hwnd, CMD_CL), QWL_USER, (PVOID) oldproc);
    }
    break;

  case WM_CONTROL:
    if (SHORT1FROMMP(mp1) == CMD_LISTBOX) {
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
      case LN_SELECT:
	x = (SHORT) WinSendDlgItemMsg(hwnd, CMD_LISTBOX,
				      LM_QUERYSELECTION,
				      MPFROMSHORT(LIT_FIRST), MPVOID);
	if (x >= 0) {
	  info = (LINKCMDS *) WinSendDlgItemMsg(hwnd, CMD_LISTBOX,
						LM_QUERYITEMHANDLE,
						MPFROMSHORT(x), MPVOID);
	  if (!info) {
	    Runtime_Error(pszSrcFile, __LINE__, "LM_QUERYITEMHANDLE");
	    break;
	  }
	  WinSetDlgItemText(hwnd, CMD_CL, info->pszCmdLine);
	  if (!(info->flags & 1023))
	    WinCheckButton(hwnd, CMD_DEFAULT, TRUE);
	  else {
	    if (info->flags & FULLSCREEN)
	      WinCheckButton(hwnd, CMD_FULLSCREEN, TRUE);
	    else if (info->flags & MINIMIZED)
	      WinCheckButton(hwnd, CMD_MINIMIZED, TRUE);
	    else if (info->flags & MAXIMIZED)
	      WinCheckButton(hwnd, CMD_MAXIMIZED, TRUE);
	    else if (info->flags & INVISIBLE)
	      WinCheckButton(hwnd, CMD_INVISIBLE, TRUE);
	  }
	  WinCheckButton(hwnd, CMD_PROMPT, ((info->flags & PROMPT) != 0));
	  WinCheckButton(hwnd, CMD_KEEP, ((info->flags & KEEP) != 0));
	  WinCheckButton(hwnd, CMD_ONCE, ((info->flags & ONCE) != 0));
          WinSetDlgItemText(hwnd, CMD_TITLE, info->title);
          if (info->env)
            WinSetDlgItemText(hwnd, CMD_ENVIRON, info->env);
	  else
            WinSetDlgItemText(hwnd, CMD_ENVIRON, NullStr);
          for (x = 0; x < 40; x++) {
            CHAR s[CCHMAXPATH + 24];

            sprintf(s, "%s%s%s",
                    x < IDM_COMMANDNUM20 - IDM_COMMANDNUM0 ? "Ctrl+" : NullStr,
                    x > IDM_COMMANDNUM19 - IDM_COMMANDNUM0 ? "Alt+" : NullStr,
                    (x > IDM_COMMANDNUM9 - IDM_COMMANDNUM0 &&
                    x < IDM_COMMANDNUM20 - IDM_COMMANDNUM0) ||
                    x > IDM_COMMANDNUM29 - IDM_COMMANDNUM0 ? "Shift+" : NullStr);
            sprintf(&s[strlen(s)], "%d", (x % 10) + 1 == 10 ? 0 : (x % 10) + 1);
            sprintf(&s[strlen(s)], " %s", UsedHotKeyIDs[x] ? "(in use)" : NullStr);
            WinSendMsg(WinWindowFromID(hwnd, CMD_HOTKEY), LM_INSERTITEM,
                       MPFROM2SHORT(LIT_END, 0), MPFROMP(s));
            if (x == 39)
              WinSendMsg(WinWindowFromID(hwnd, CMD_HOTKEY), LM_INSERTITEM,
                         MPFROM2SHORT(LIT_END, 0), MPFROMP("none"));
            if (info->HotKeyID == x + IDM_COMMANDNUM0)
              WinSendMsg(WinWindowFromID(hwnd, CMD_HOTKEY), LM_SELECTITEM,
                         MPFROM2SHORT(x, 0), MPFROMLONG(TRUE));
            if (x == 39 && info->HotKeyID == 0)
              WinSendMsg(WinWindowFromID(hwnd, CMD_HOTKEY), LM_SELECTITEM,
                         MPFROM2SHORT(x + 1, 0), MPFROMLONG(TRUE));
          } // for
        }
	break;
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case CMD_FIND:
      {
	CHAR filename[CCHMAXPATH + 9], szfilename[CCHMAXPATH + 9];

	*filename = 0;
        if (insert_filename(hwnd, filename, 2, FALSE) && *filename) {
          BldQuotedFileName(szfilename, filename);
	  WinSetDlgItemText(hwnd, CMD_CL, szfilename);
	}
      }
      break;

    case CMD_REORDER:
      if (!cmdhead || !cmdhead->next) {
	Runtime_Error(pszSrcFile, __LINE__, "no cmd");
	break;
      }
      if (WinDlgBox(HWND_DESKTOP, hwnd, ReOrderProc, FM3ModHandle,
		    RE_FRAME, MPVOID)) {
	WinSendDlgItemMsg(hwnd, CMD_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
	WinSetDlgItemText(hwnd, CMD_CL, NullStr);
	WinSetDlgItemText(hwnd, CMD_TITLE, NullStr);
	info = cmdhead;
	while (info) {
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					CMD_LISTBOX,
					LM_INSERTITEM,
					MPFROM2SHORT(LIT_END, 0),
					MPFROMP(info->title));
	  if (x >= 0)
	    WinSendDlgItemMsg(hwnd,
			      CMD_LISTBOX,
			      LM_SETITEMHANDLE,
			      MPFROMSHORT(x), MPFROMP(info));
	  info = info->next;
	}
	WinSendDlgItemMsg(hwnd,
			  CMD_LISTBOX,
			  LM_SELECTITEM, MPFROMSHORT(0), MPFROMSHORT(TRUE));
	WinCheckButton(hwnd, CMD_DEFAULT, TRUE);
	WinCheckButton(hwnd, CMD_PROMPT, FALSE);
	WinCheckButton(hwnd, CMD_ONCE, FALSE);
	save_commands();
      }
      break;

    case DID_OK:
      {
        COMMAND *temp;
        BOOL fDontCheckHotKey = FALSE;

        temp = xmallocz(sizeof(COMMAND), pszSrcFile, __LINE__);
        if (!temp)
          break;  //already complained
        temp->pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!temp->pszCmdLine) {
          free (temp);
          break; //already complained
        }
        if (QueryCommandSettings(hwnd, temp)){
          if (temp->HotKeyID == 0)
            fDontCheckHotKey = TRUE;
          info = add_command(temp, fDontCheckHotKey);
          if (info == (LINKCMDS *) -1) {
            free(temp->pszCmdLine);
            free(temp);
            break;
          }
        }
        else {
          free(temp->pszCmdLine);
          free(temp);
          break;
        }
        free(temp->pszCmdLine);
        free(temp);
        save_commands();
        WinDismissDlg(hwnd, 0);
        break;
      }

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_COMMAND, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case CMD_ADD:
      {
        COMMAND *temp;
        BOOL fDontCheckHotKey = FALSE;

        temp = xmallocz(sizeof(COMMAND), pszSrcFile, __LINE__);
        if (!temp)
          break;  //already complained
        temp->pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!temp->pszCmdLine) {
          free (temp);
          break; //already complained
        }
        if (!QueryCommandSettings(hwnd, temp)) {
          free(temp->pszCmdLine);
          free(temp);
          break;
        }
        else {
          if (temp->HotKeyID == 0)
            fDontCheckHotKey = TRUE;
          info = add_command(temp, fDontCheckHotKey);
	  if (!info) {
	    saymsg(MB_ENTER, hwnd, GetPString(IDS_ERRORTEXT),
                   GetPString(IDS_CANTADDCOMMANDTEXTDUP), temp->title);
            free(temp->pszCmdLine);
            free(temp);
            break;
          }
          if (info == (LINKCMDS *) -1) {
            free(temp->pszCmdLine);
            free(temp);
            break;
          }
          x = (SHORT) WinSendDlgItemMsg(hwnd,
                                        CMD_LISTBOX,
                                        LM_INSERTITEM,
                                        MPFROM2SHORT(LIT_END, 0),
                                        MPFROMP(temp->title));
          if (x >= 0) {
            WinSendDlgItemMsg(hwnd,
                              CMD_LISTBOX,
                              LM_SETITEMHANDLE,
                              MPFROMSHORT(x), MPFROMP(info));
            WinSendDlgItemMsg(hwnd,
                              CMD_LISTBOX,
                              LM_SELECTITEM,
                              MPFROMSHORT(x), MPFROMSHORT(TRUE));
          }
          save_commands();
        }
        free(temp->pszCmdLine);
        free(temp);
        break;
      }

    case CMD_DELETE:
      {
        CHAR temp[100];
        CHAR key[120];

	WinQueryDlgItemText(hwnd, CMD_TITLE, 100, temp);
        bstripcr(temp);
	if (!kill_command(temp))
	  Runtime_Error(pszSrcFile, __LINE__, "kill_command");
        else {
          sprintf(key, "COMMAND.%sID", temp);
          PrfWriteProfileData(fmprof, FM3Str, key, NULL, 0);
          sprintf(key, "COMMAND.%sHotKeyID", temp);
          PrfWriteProfileData(fmprof, FM3Str, key, NULL, 0);
          sprintf(key, "COMMAND.%senv", temp);
          PrfWriteProfileData(fmprof, FM3Str, key, NULL, 0);
          sprintf(key, "COMMAND.%sflags", temp);
          PrfWriteProfileData(fmprof, FM3Str, key, NULL, 0);
          sprintf(key, "COMMAND.%sCmdLine", temp);
          PrfWriteProfileData(fmprof, FM3Str, key, NULL, 0);
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					CMD_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (x >= 0) {
	    WinSendDlgItemMsg(hwnd,
			      CMD_LISTBOX,
			      LM_DELETEITEM, MPFROMSHORT(x), MPVOID);
	    WinSendDlgItemMsg(hwnd,
			      CMD_LISTBOX,
			      LM_SELECTITEM,
			      MPFROMSHORT(0), MPFROMSHORT(TRUE));
	  }
          save_commands();
	}
      }
      break;

    case CMD_REPLACE:
      { 
        COMMAND *temp;
        CHAR keyID[120];
        CHAR keyHotKeyID[120];
        CHAR keyenv[120];
        INT ID = 0;
        INT HotKeyID = 0;
        ULONG size;
        BOOL fDontCheckHotKey = FALSE;

        // Query the dialog
        temp = xmallocz(sizeof(COMMAND), pszSrcFile, __LINE__);
        if (!temp)
          break;  //already complained
        temp->pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!temp->pszCmdLine) {
          free (temp);
          break; //already complained
        }
        if (!QueryCommandSettings(hwnd, temp)) {
          free(temp->pszCmdLine);
          free(temp);
          break;
        }
        //remember item location in the list
        y = (SHORT) WinSendDlgItemMsg(hwnd,
				      CMD_LISTBOX,
				      LM_QUERYSELECTION,
				      MPFROMSHORT(LIT_CURSOR), MPVOID);
        //Delete
        sprintf(keyID, "COMMAND.%sID", temp->title);
        sprintf(keyHotKeyID, "COMMAND.%sHotKeyID", temp->title);
        sprintf(keyenv, "COMMAND.%senv", temp->title);
        PrfQueryProfileData(fmprof, FM3Str, keyID, &ID, &size);
        PrfQueryProfileData(fmprof, FM3Str, keyHotKeyID, &HotKeyID, &size);
        temp->ID = ID;
        if (temp->HotKeyID == HotKeyID || temp->HotKeyID == 0)
          fDontCheckHotKey = TRUE;
        if (kill_command(temp->title)) {
          PrfWriteProfileData(fmprof, FM3Str, keyID, NULL, 0);
          PrfWriteProfileData(fmprof, FM3Str, keyHotKeyID, NULL, 0);
          PrfWriteProfileString(fmprof, FM3Str, keyenv, NULL);
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					CMD_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (x >= 0) {
	    WinSendDlgItemMsg(hwnd,
			      CMD_LISTBOX,
			      LM_DELETEITEM, MPFROMSHORT(x), MPVOID);
	    WinSendDlgItemMsg(hwnd,
			      CMD_LISTBOX,
			      LM_SELECTITEM,
			      MPFROMSHORT(LIT_NONE), MPFROMSHORT(FALSE));
	  }
        } // then do an add
        info = add_command(temp, fDontCheckHotKey);
        if (!info || info == (LINKCMDS *) -1) {
          if (!info)
            saymsg(MB_ENTER, hwnd, GetPString(IDS_ERRORTEXT),
                   GetPString(IDS_CANTADDCOMMANDTEXT),
                   temp->title);
        }
	else {
          //put item back in original place
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					CMD_LISTBOX,
					LM_INSERTITEM,
					MPFROM2SHORT(y, 0),
					MPFROMP(temp->title));
          if (x >= 0) {
            LINKCMDS *temphead = NULL,*last = NULL, *temptail = NULL;
            SHORT numitems, sSelect = 0;

	    WinSendDlgItemMsg(hwnd,
			      CMD_LISTBOX,
			      LM_SETITEMHANDLE,
			      MPFROMSHORT(x), MPFROMP(info));
	    WinSendDlgItemMsg(hwnd,
			      CMD_LISTBOX,
			      LM_SELECTITEM,
                              MPFROMSHORT(x), MPFROMSHORT(TRUE));
            //then reorder
            numitems = (SHORT) WinSendDlgItemMsg(hwnd, CMD_LISTBOX,
                                                 LM_QUERYITEMCOUNT,
                                                 MPVOID, MPVOID);

            while (numitems) {


              info = (LINKCMDS *) WinSendDlgItemMsg(hwnd, CMD_LISTBOX,
                                                    LM_QUERYITEMHANDLE,
                                                    MPFROMSHORT(sSelect++),
                                                    MPVOID);
              if (info) {
                if (!temphead) {
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
            save_commands();
          }
        }
        xfree(temp->pszCmdLine, pszSrcFile, __LINE__);
        xfree(temp, pszSrcFile, __LINE__);
      }
      break;
    }
    return 0;
  }
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

VOID RunCommand(HWND hwnd, INT cx)
{
  INT x;
  CHAR **list;
  LINKCMDS *info;

  list = BuildList(hwnd);
  x = 0;
  info = cmdhead;
  while (info) {
    if (cx < 4300) {
      if (info->ID == cx)
        break;
    }
    else {
      if (info->HotKeyID == cx)
        break;
    }
    info = info->next;
  }
  if (info) {

    INT flags;

    x--;
    flags = info->flags;
    if (!(flags & FULLSCREEN))
      flags |= WINDOWED;
    if (flags & KEEP)
      flags |= SEPARATEKEEP;
    else
      flags |= SEPARATE;
    flags &= ~(KEEP | DIEAFTER);
    if (!strchr(info->pszCmdLine, '%')) {
      CHAR *fakelist[2];

      *fakelist = "C";
      fakelist[1] = NULL;
      ExecOnList(hwnd,
         	 info->pszCmdLine,
                 flags, NULL, info->env != NullStr ? info->env : NULL,
                 fakelist, GetPString(IDS_EXECCMDTITLETEXT),
                 pszSrcFile, __LINE__);
    }
    else if ((flags & ONCE) && list && list[0]) {

      CHAR *fakelist[2];
      INT cntr;

      flags &= (~ONCE);
      for (cntr = 0; list[cntr]; cntr++) {
	*fakelist = list[cntr];
	fakelist[1] = NULL;
	ExecOnList(hwnd,
		   info->pszCmdLine,
                   flags, NULL, info->env != NullStr ? info->env : NULL,
                   fakelist, GetPString(IDS_EXECCMDTITLETEXT),
                   pszSrcFile, __LINE__);
      }
    }
    else if (list && list[0])
      ExecOnList(hwnd,
		 info->pszCmdLine,
                 flags, NULL, info->env != NullStr ? info->env : NULL,
                 list, GetPString(IDS_EXECCMDTITLETEXT),
                 pszSrcFile, __LINE__);
    else
      return;
  }
  FreeList(list);
  DosPostEventSem(CompactSem);
}

VOID EditCommands(HWND hwnd)
{
  static CHAR stop = 0;

  if (stop)
    return;
  stop++;
  if (!cmdloaded)
    load_commands();
  WinDlgBox(HWND_DESKTOP,
	    hwnd, CommandDlgProc, FM3ModHandle, CMD_FRAME, MPFROMP(&hwnd));
  stop = 0;
}

#pragma alloc_text(COMMAND,command_title,free_commands,add_command,kill_command)
#pragma alloc_text(COMMAND,CommandDlgProc,EditCommands,ReOrderProc,CommandTextProc)
