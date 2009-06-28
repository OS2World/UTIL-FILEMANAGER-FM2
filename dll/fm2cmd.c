
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2008 Steven H.Levine

  Command processing

  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "notebook.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "fm2cmd.h"
#include "mainwnd.h"			// FindDirCnrByName
#include "valid.h"			// MakeFullName
#include "misc.h"			// PostMsg
#include "delims.h"			// skip_delim
#include "pathutil.h"                   // AddBackslashToPath


static VOID fullname(CHAR * directory, CHAR * name);
static VOID parse(CHAR * command, CHAR * key, CHAR * rest);

// Data definitions
#pragma data_seg(GLOBAL1)
BOOL fKeepCmdLine;
BOOL fSaveMiniCmds;

#pragma data_seg(DATA2)

static VOID fullname(CHAR * directory, CHAR * name)
{

  CHAR temp[CCHMAXPATH];

  if (!*name) {
    strcpy(name, directory);
    return;
  }
  if (!strchr(name, ':')) {
    if (*name != '\\' && *name != '/') {
      strcpy(temp, directory);
      AddBackslashToPath(directory);
      //if (directory[strlen(directory) - 1] != '\\')
      //  strcat(temp, "\\");
    }
    else {
      *temp = *directory;
      temp[1] = ':';
      temp[2] = 0;
    }
    strcat(temp, name);
    strcpy(name, temp);
  }
  MakeFullName(name);
}

static VOID parse(CHAR * command, CHAR * key, CHAR * rest)
{

  CHAR *p;

  *key = *rest = 0;
  strcpy(key, command);
  p = strchr(key, ' ');
  if (p) {
    *p = 0;
    p++;
    p = skip_delim(p, " \t");
    strcpy(rest, p);
  }
}

BOOL FM2Command(CHAR * directory, CHAR * command)
{

  BOOL ret = FALSE;
  CHAR key[CCHMAXPATH], rest[CCHMAXPATH];
  HWND hwnd;

  if (command && *command == '/') {
    parse(command, key, rest);
    if (!stricmp(key, GetPString(IDS_OPENCMDTEXT))) {
      fullname(directory, rest);
      WinSendMsg(hwndTree, UM_OPENWINDOWFORME, MPFROMP(rest), MPVOID);
      ret = TRUE;
    }
    else if (!stricmp(key, GetPString(IDS_CLOSECMDTEXT))) {
      fullname(directory, rest);
      hwnd = FindDirCnrByName(rest, FALSE);
      if (hwnd)
	PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      ret = TRUE;
    }
    else if (!stricmp(key, GetPString(IDS_HELPCMDTEXT))) {
      saymsg(MB_ENTER, (hwndMain) ? hwndMain : HWND_DESKTOP,
	     GetPString(IDS_FM2CMDHELPHDRTEXT), GetPString(IDS_FM2CMDHELPTEXT));
      ret = TRUE;
    }
    else if (!stricmp(key, GetPString(IDS_FILTERCMDTEXT))) {
      hwnd = FindDirCnrByName(directory, FALSE);
      if (hwnd) {
	WinSendMsg(hwnd, UM_FILTER, MPFROMP(rest), MPVOID);
	ret = TRUE;
      }
    }
    else if (!stricmp(key, GetPString(IDS_KEEPCMDTEXT)) ||
	     !stricmp(key, GetPString(IDS_NOKEEPCMDTEXT))) {
      if (!stricmp(key, GetPString(IDS_NOKEEPCMDTEXT)))
	fKeepCmdLine = FALSE;
      else
	fKeepCmdLine = TRUE;
      PrfWriteProfileData(fmprof, FM3Str, "KeepCmdLine", &fKeepCmdLine,
			  sizeof(BOOL));
      ret = TRUE;
    }
    else if (!stricmp(key, GetPString(IDS_SAVECMDTEXT)) ||
	     !stricmp(key, GetPString(IDS_NOSAVECMDTEXT))) {
      if (!stricmp(key, GetPString(IDS_NOSAVECMDTEXT)))
	fSaveMiniCmds = FALSE;
      else
	fSaveMiniCmds = TRUE;
      PrfWriteProfileData(fmprof, FM3Str, "SaveMiniCmds", &fSaveMiniCmds,
			  sizeof(BOOL));
      ret = TRUE;
    }
  }
  return ret;
}

#pragma alloc_text(FM2CMD,FM2Command,fullname,parse)
