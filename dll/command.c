
/***********************************************************************

  $Id$

  Custom commands

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2008 Steven H. Levine

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
  CHAR env[1002];
  ULONG flags;
  ULONG ID;
  ULONG HotKeyID;
}
COMMAND;

// Data defintions
#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;
static LINKCMDS *cmdtail;
static BOOL UsedCommandIDs[300];
static BOOL UsedHotKeyIDs[40];

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
  cmdhead = cmdtail = NULL;
}

VOID load_commands(VOID)
{
  FILE *fp;
  LINKCMDS *info;
  PSZ pszCmdLine;
  CHAR title[100];
  CHAR flags[34];
  ULONG size;

  if (cmdhead)
    free_commands();
  PrfQueryProfileData(fmprof, FM3Str, "UsedCommandIDs", &UsedCommandIDs,
                      &size);
  PrfQueryProfileData(fmprof, FM3Str, "UsedHotKeyIDs", &UsedHotKeyIDs,
                        &size); 
  cmdloaded = TRUE;
  pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
  if (pszCmdLine) {
    BldFullPathName(pszCmdLine, pFM2SaveDirectory, PCSZ_COMMANDSDAT);
    fp = _fsopen(pszCmdLine, "r", SH_DENYWR);
    if (fp) {
      while (!feof(fp)) {
        if (!xfgets_bstripcr(title, sizeof(title), fp, pszSrcFile, __LINE__))
          break;
        //title[strlen(title)] = 0;  // Match size to entry file max? Not needed as bstripcr does this? GKY 21 Dec 09
        if (!*title || *title == ';')
          continue;
        if (!xfgets_bstripcr(pszCmdLine, MaxComLineStrg, fp, pszSrcFile, __LINE__))
          break;				/* error! */
        if (!xfgets_bstripcr(flags, sizeof(flags), fp, pszSrcFile, __LINE__))
          break;
        //pszCmdLine[strlen(pszCmdLine)] = 0;	   // fixme to know why chopped this way? Not needed as bstripcr does this? GKY 21 Dec 09
        flags[34] = 0;
        if (!pszCmdLine)
          continue;
        info = xmallocz(sizeof(LINKCMDS), pszSrcFile, __LINE__);
        if (info) {
#	  ifdef FORTIFY
	  Fortify_SetOwner(info, 1);
	  Fortify_SetScope(info, 1);
#	  endif
          info->pszCmdLine = xstrdup(pszCmdLine, pszSrcFile, __LINE__);
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
      load_inicommands();
    }
  }
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
  CHAR env[1002];
  CHAR keyID[120];
  CHAR keyHotKeyID[120];
  CHAR keyenv[120];
  ULONG size;

  if (!cmdloaded || !cmdhead)
    return;
  info = cmdhead;
  while (info) {
    bstripcr(info->title);
    sprintf(keyID, "COMMAND.%sID", info->title);
    sprintf(keyHotKeyID, "COMMAND.%sHotKeyID", info->title);
    sprintf(keyenv, "COMMAND.%senv", info->title);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, keyID, &ID, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, keyHotKeyID, &HotKeyID, &size);
    PrfQueryProfileString(fmprof, FM3Str, keyenv, NullStr, env, sizeof(env));
    if (ID) {
      if (env != NullStr)
        info->env = xstrdup(env, pszSrcFile, __LINE__);
      info->ID = ID;
      info->HotKeyID = HotKeyID;
    }
    //This updates the old commands.dat file to the new format
    //assigning the IDs based on file order or on next available ID if
    //COMMAND.DAT is hand edited.
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
      info->env = env;
      if (env != NullStr)
        PrfWriteProfileString(fmprof, FM3Str, info->pszCmdLine, NULL);
      if (info->env)
        strcpy(env, info->env);
      ID = info->ID;
      HotKeyID = info->HotKeyID;
      PrfWriteProfileData(fmprof, FM3Str, keyID, &ID, sizeof(INT));
      PrfWriteProfileData(fmprof, FM3Str, keyHotKeyID, &HotKeyID, sizeof(INT));
      if (env != NullStr)
        PrfWriteProfileString(fmprof, FM3Str, keyenv, env);
      PrfWriteProfileData(fmprof, FM3Str, "UsedCommandIDs", &UsedCommandIDs,
                      sizeof(BOOL) * 300);
      PrfWriteProfileData(fmprof, FM3Str, "UsedHotKeyIDs", &UsedHotKeyIDs,
                        sizeof(BOOL) * 40);
    }
    ID = 0;
    HotKeyID = 0;
    info = info->next;
  }
}

VOID save_commands(VOID)
{
  LINKCMDS *info;
  FILE *fp;
  CHAR s[CCHMAXPATH + 14];
  INT x = 0;
  INT ID = 0;
  INT HotKeyID = 0;
  CHAR env[1002];
  CHAR keyID[120];
  CHAR keyHotKeyID[120];
  CHAR keyenv[120];

  if (!cmdloaded || !cmdhead)
    return;
  info = cmdhead;
  BldFullPathName(s, pFM2SaveDirectory, PCSZ_COMMANDSDAT);
  if (CheckDriveSpaceAvail(s, ullDATFileSpaceNeeded, 1) == 2)
    return; //already gave error msg
  fp = xfopen(s, "w", pszSrcFile, __LINE__);
  if (fp) {
    while (info) {
      fprintf(fp, ";\n%0.99s\n%0.*s\n%lu\n",
              info->title, MaxComLineStrg, info->pszCmdLine, info->flags);
      
      if (info->env)
        strcpy(env, info->env);
      ID = info->ID;
      HotKeyID = info->HotKeyID;
      bstripcr(info->title);
      sprintf(keyID, "COMMAND.%sID", info->title);
      sprintf(keyHotKeyID, "COMMAND.%sHotKeyID", info->title);
      sprintf(keyenv, "COMMAND.%senv", info->title);
      PrfWriteProfileData(fmprof, FM3Str, keyID, &ID, sizeof(INT));
      PrfWriteProfileData(fmprof, FM3Str, keyHotKeyID, &HotKeyID, sizeof(INT));
      if (env != NullStr)
        PrfWriteProfileString(fmprof, FM3Str, keyenv, env);
      x++;
      info = info->next;
    } // while info
    PrfWriteProfileData(fmprof, FM3Str, "UsedCommandIDs", &UsedCommandIDs,
                        sizeof(BOOL) * 300);
    PrfWriteProfileData(fmprof, FM3Str, "UsedHotKeyIDs", &UsedHotKeyIDs,
                        sizeof(BOOL) * 40);
    fclose(fp);
  } // if (fp)
}

//== add_command() Add command to list ==

LINKCMDS *add_command(COMMAND *addme)
{
  LINKCMDS *info;
  INT x;
  INT y;

  if (!addme || !*addme->pszCmdLine || !*addme->title)
    return NULL;			// No data
  info = cmdhead;
  while (info) {
    if (!stricmp(info->title, addme->title))
      return NULL;			// Got a dup
    if (addme->HotKeyID && info->HotKeyID && info->HotKeyID == addme->HotKeyID)
      info->HotKeyID = 0;  //avoid assigning hot key to multiple commands
    info = info->next;
  }
  info = xmallocz(sizeof(LINKCMDS), pszSrcFile, __LINE__);
  if (!info)
    return NULL;
  info->pszCmdLine = xstrdup(addme->pszCmdLine, pszSrcFile, __LINE__);
  info->title = xstrdup(addme->title, pszSrcFile, __LINE__);
  info->HotKeyID = addme->HotKeyID;
  if (info->HotKeyID >= IDM_COMMANDNUM0 && info->HotKeyID <= IDM_COMMANDNUM19)
    UsedHotKeyIDs[info->HotKeyID - IDM_COMMANDNUM0] = TRUE;
  else
    info->HotKeyID = 0;
  if (!info->ID) {
    //profile updated by save_commands
    for (x = 0; x < 300; x++) {
      if (!UsedCommandIDs[x]) {
        info->ID = IDM_COMMANDSTART + x;
        UsedCommandIDs[x] = TRUE;
        for (y = 0; y < 40; y++) {
          if (!UsedHotKeyIDs[y]) {
            info->HotKeyID = IDM_COMMANDNUM0 + y;
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
  }
  if (addme->flags)
    info->flags = addme->flags;
  if (addme->env)
    info->env = xstrdup(addme->env, pszSrcFile, __LINE__);
  if (!info->pszCmdLine || !info->title || !info->ID) {
    xfree(info->pszCmdLine, pszSrcFile, __LINE__);
    xfree(info->title, pszSrcFile, __LINE__);
    xfree(info->title, pszSrcFile, __LINE__);
    free(info);
    return NULL;
  }
  if (!cmdhead)				/* only item in list */
    cmdhead = cmdtail = info;
  else {
    /* place at tail */
    cmdtail->next = info;
    info->prev = cmdtail;
    cmdtail = info;
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
  return FALSE;
}

MRESULT EXPENTRY CommandDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SHORT x, y;
  LINKCMDS *info;

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
	x = (SHORT) WinSendDlgItemMsg(hwnd,
				      CMD_LISTBOX,
				      LM_QUERYSELECTION, MPVOID, MPVOID);
	if (x == LIT_NONE)
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					CMD_LISTBOX,
					LM_SELECTITEM,
					MPFROMSHORT(0), MPFROMSHORT(TRUE));
      }
      {
        COMMAND temp;
        PSZ pszWorkBuf;
        APIRET ret;
        CHAR env[1002];

	memset(&temp, 0, sizeof(COMMAND));
        temp.pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!temp.pszCmdLine)
          break; //already complained
        pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!pszWorkBuf) {
          free(temp.pszCmdLine);
          break; //already complained
        }
	WinQueryDlgItemText(hwnd, CMD_CL, MaxComLineStrg, temp.pszCmdLine);
        NormalizeCmdLine(pszWorkBuf, temp.pszCmdLine);
        memcpy(temp.pszCmdLine, pszWorkBuf, strlen(pszWorkBuf) + 1);
        free(pszWorkBuf);
        if (!strchr(temp.pszCmdLine, '%') && !fCancelAction){
          ret = saymsg(MB_YESNO,
                       HWND_DESKTOP,
                       NullStr,
                       GetPString(IDS_TOACTONSELECTEDTEXT));
          if (ret == MBID_YES)
            strcat(temp.pszCmdLine, " %a");
        }
        WinQueryDlgItemText(hwnd, CMD_TITLE, sizeof(temp.title), temp.title);
        bstripcr(temp.title);
        temp.flags = CheckExecutibleFlags(hwnd, 3);
	/*if (WinQueryButtonCheckstate(hwnd, CMD_DEFAULT))
	  temp.flags = 0;
	else if (WinQueryButtonCheckstate(hwnd, CMD_FULLSCREEN))
	  temp.flags = FULLSCREEN;
	else if (WinQueryButtonCheckstate(hwnd, CMD_MINIMIZED))
	  temp.flags = MINIMIZED;
	else if (WinQueryButtonCheckstate(hwnd, CMD_MAXIMIZED))
	  temp.flags = MAXIMIZED;
	else if (WinQueryButtonCheckstate(hwnd, CMD_INVISIBLE))
	  temp.flags = INVISIBLE;
	if (WinQueryButtonCheckstate(hwnd, CMD_KEEP))
	  temp.flags |= KEEP;
	if (WinQueryButtonCheckstate(hwnd, CMD_PROMPT))
	  temp.flags |= PROMPT;
	if (WinQueryButtonCheckstate(hwnd, CMD_ONCE))
          temp.flags |= ONCE;*/
        *env = 0;
	WinQueryDlgItemText(hwnd, CMD_ENVIRON, 1000, env);
	bstripcr(env);
        if (*env) {
          strcpy(temp.env, env);
	  } 
        if (fCancelAction){
          fCancelAction = FALSE;
          free(temp.pszCmdLine);
          break;
        }
        else
          info = add_command(&temp);
        if (!info)
          WinDismissDlg(hwnd, 0);
        else {
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					CMD_LISTBOX,
					LM_INSERTITEM,
					MPFROM2SHORT(LIT_END, 0),
					MPFROMP(temp.title));
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
        }
        save_commands();
        load_commands();
        free(temp.pszCmdLine);
      }
      x = (SHORT) WinSendDlgItemMsg(hwnd,
				    CMD_LISTBOX,
				    LM_QUERYSELECTION,
                                    MPFROMSHORT(LIT_FIRST), MPVOID);
      save_commands();
      WinDismissDlg(hwnd, 0);
      break;

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
        COMMAND temp;
        PSZ pszWorkBuf;
        APIRET ret;
        CHAR env[1002];

        memset(&temp, 0, sizeof(COMMAND));
        temp.pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!temp.pszCmdLine)
          break; //already complained
        pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!pszWorkBuf) {
          free(temp.pszCmdLine);
          break; //already complained
        }
	WinQueryDlgItemText(hwnd, CMD_CL, MaxComLineStrg, temp.pszCmdLine);
        NormalizeCmdLine(pszWorkBuf, temp.pszCmdLine);
        memcpy(temp.pszCmdLine, pszWorkBuf, strlen(pszWorkBuf) + 1);
        free(pszWorkBuf);
        if (!strchr(temp.pszCmdLine, '%') && !fCancelAction){
          ret = saymsg(MB_YESNO,
                       HWND_DESKTOP,
                       NullStr,
                       GetPString(IDS_TOACTONSELECTEDTEXT));
          if (ret == MBID_YES)
            strcat(temp.pszCmdLine, " %a");
        }
        WinQueryDlgItemText(hwnd, CMD_TITLE, sizeof(temp.title), temp.title);
        bstripcr(temp.title);
        temp.flags = CheckExecutibleFlags(hwnd, 3);
	/*if (WinQueryButtonCheckstate(hwnd, CMD_DEFAULT))
	  temp.flags = 0;
	else if (WinQueryButtonCheckstate(hwnd, CMD_FULLSCREEN))
	  temp.flags = FULLSCREEN;
	else if (WinQueryButtonCheckstate(hwnd, CMD_MINIMIZED))
	  temp.flags = MINIMIZED;
	else if (WinQueryButtonCheckstate(hwnd, CMD_MAXIMIZED))
	  temp.flags = MAXIMIZED;
	else if (WinQueryButtonCheckstate(hwnd, CMD_INVISIBLE))
	  temp.flags = INVISIBLE;
	if (WinQueryButtonCheckstate(hwnd, CMD_KEEP))
	  temp.flags |= KEEP;
	if (WinQueryButtonCheckstate(hwnd, CMD_PROMPT))
	  temp.flags |= PROMPT;
	if (WinQueryButtonCheckstate(hwnd, CMD_ONCE))
          temp.flags |= ONCE;*/
        *env = 0;
	WinQueryDlgItemText(hwnd, CMD_ENVIRON, 1000, env);
	bstripcr(env);
        if (*env) {
          strcpy(temp.env, env);
	  } 
        if (fCancelAction){
          fCancelAction = FALSE;
          free(temp.pszCmdLine);
          break;
        }
        else
	  info = add_command(&temp);
	if (!info) {
	  saymsg(MB_ENTER, hwnd, GetPString(IDS_ERRORTEXT),
                 GetPString(IDS_CANTADDCOMMANDTEXTDUP), temp.title);
	}
	else {
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					CMD_LISTBOX,
					LM_INSERTITEM,
					MPFROM2SHORT(LIT_END, 0),
					MPFROMP(temp.title));
	  if (x >= 0) {
	    WinSendDlgItemMsg(hwnd,
			      CMD_LISTBOX,
			      LM_SETITEMHANDLE,
			      MPFROMSHORT(x), MPFROMP(info));
	    WinSendDlgItemMsg(hwnd,
			      CMD_LISTBOX,
			      LM_SELECTITEM,
			      MPFROMSHORT(x), MPFROMSHORT(TRUE));
	    save_commands();
	  }
        }
        free(temp.pszCmdLine);
      }
      break;

    case CMD_DELETE:
      {
        CHAR temp[100];
        CHAR keyID[120];
        CHAR keyHotKeyID[120];
        CHAR keyenv[120];

	WinQueryDlgItemText(hwnd, CMD_TITLE, 100, temp);
        bstripcr(temp);
        sprintf(keyID, "COMMAND.%sID", temp);
        sprintf(keyHotKeyID, "COMMAND.%sHotKeyID", temp);
        sprintf(keyenv, "COMMAND.%senv", temp);
        PrfWriteProfileData(fmprof, FM3Str, keyID, NULL, NULL);
        PrfWriteProfileData(fmprof, FM3Str, keyHotKeyID, NULL, NULL);
        PrfWriteProfileString(fmprof, FM3Str, keyenv, NULL);
	if (!kill_command(temp))
	  Runtime_Error(pszSrcFile, __LINE__, "kill_command");
	else {
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
          save_commands();
	}
      }
      break;

    case CMD_REPLACE:
      { //Delete first
        PSZ pszWorkBuf;
        COMMAND temp;
        APIRET ret;
        CHAR keyID[120];
        CHAR keyHotKeyID[120];
        CHAR keyenv[120];
        CHAR env[1002];
        INT ID = 0;
        INT HotKeyID = 0;
        ULONG size;

        pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!pszWorkBuf) {
          break; //already complained
        }
        memset(&temp, 0, sizeof(COMMAND));
        temp.pszCmdLine = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!temp.pszCmdLine) {
          free(pszWorkBuf);
          break; //already complained
        }
	WinQueryDlgItemText(hwnd, CMD_CL, MaxComLineStrg, temp.pszCmdLine);
        NormalizeCmdLine(pszWorkBuf, temp.pszCmdLine);
        memcpy(temp.pszCmdLine, pszWorkBuf, strlen(pszWorkBuf) + 1);
        free(pszWorkBuf);
        if (fCancelAction){
          fCancelAction = FALSE;
          free(temp.pszCmdLine);
          break;
        }
        if (!strchr(temp.pszCmdLine, '%') && !fCancelAction){
          ret = saymsg(MB_YESNO,
                       HWND_DESKTOP,
                       NullStr,
                       GetPString(IDS_TOACTONSELECTEDTEXT));
          if (ret == MBID_YES)
            strcat(temp.pszCmdLine, " %a");
        }
        //remember item location in the list
        y = (SHORT) WinSendDlgItemMsg(hwnd,
				      CMD_LISTBOX,
				      LM_QUERYSELECTION,
				      MPFROMSHORT(LIT_CURSOR), MPVOID);
	WinQueryDlgItemText(hwnd, CMD_TITLE, sizeof(temp.title), temp.title);
        bstripcr(temp.title);
        sprintf(keyID, "COMMAND.%sID", temp.title);
        sprintf(keyHotKeyID, "COMMAND.%sHotKeyID", temp.title);
        sprintf(keyenv, "COMMAND.%senv", temp.title);
        PrfQueryProfileData(fmprof, FM3Str, keyID, &ID, &size);
        PrfQueryProfileData(fmprof, FM3Str, keyHotKeyID, &HotKeyID, &size);
        PrfQueryProfileString(fmprof, FM3Str, keyenv, NullStr, env, sizeof(env));
        temp.ID = ID;
        temp.HotKeyID = HotKeyID;
        if (env != NullStr)
          strcpy(temp.env, env);
        PrfWriteProfileData(fmprof, FM3Str, keyID, NULL, NULL);
        PrfWriteProfileData(fmprof, FM3Str, keyHotKeyID, NULL, NULL);
        PrfWriteProfileString(fmprof, FM3Str, keyenv, NULL);
	if (kill_command(temp.title)){
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
        temp.flags = CheckExecutibleFlags(hwnd, 3);
        /*if (WinQueryButtonCheckstate(hwnd, CMD_DEFAULT))
          temp.flags = 0;
        else if (WinQueryButtonCheckstate(hwnd, CMD_FULLSCREEN))
          temp.flags = FULLSCREEN;
        else if (WinQueryButtonCheckstate(hwnd, CMD_MINIMIZED))
          temp.flags = MINIMIZED;
        else if (WinQueryButtonCheckstate(hwnd, CMD_MAXIMIZED))
          temp.flags = MAXIMIZED;
        else if (WinQueryButtonCheckstate(hwnd, CMD_INVISIBLE))
          temp.flags = INVISIBLE;
        if (WinQueryButtonCheckstate(hwnd, CMD_KEEP))
          temp.flags |= KEEP;
        if (WinQueryButtonCheckstate(hwnd, CMD_PROMPT))
          temp.flags |= PROMPT;
        if (WinQueryButtonCheckstate(hwnd, CMD_ONCE))
          temp.flags |= ONCE;*/
        *env = 0;
	WinQueryDlgItemText(hwnd, CMD_ENVIRON, 1000, env);
	bstripcr(env);
        if (*env)
          strcpy(temp.env, env);
        if (!*env && temp.env != NullStr)
          strcpy(temp.env, NullStr);
        info = add_command(&temp);
        if (!info) {
	  saymsg(MB_ENTER, hwnd, GetPString(IDS_ERRORTEXT),
	         GetPString(IDS_CANTADDCOMMANDTEXT),
                 temp.title);
          }

	else {
          //put item back in original place
	  x = (SHORT) WinSendDlgItemMsg(hwnd,
					CMD_LISTBOX,
					LM_INSERTITEM,
					MPFROM2SHORT(y, 0),
					MPFROMP(temp.title));
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
        free(temp.pszCmdLine);
      }
      break;
    }
    return 0;
  }
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
                 flags, info->env != NullStr ? info->env : NULL,
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
                   flags, info->env != NullStr ? info->env : NULL,
                   fakelist, GetPString(IDS_EXECCMDTITLETEXT),
                   pszSrcFile, __LINE__);
      }
    }
    else if (list && list[0])
      ExecOnList(hwnd,
		 info->pszCmdLine,
                 flags, info->env != NullStr ? info->env : NULL,
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
