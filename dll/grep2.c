
/***********************************************************************

  $Id$

  grep dialog for collector

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2014 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  06 Jun 05 SHL Indent -i2
  06 Jun 05 SHL Rework for VAC3.65 compat, lose excess statics
  17 Jul 06 SHL Use Runtime_Error
  28 Jul 06 SHL Avoid 0 length malloc, optimize option checks
  29 Jul 06 SHL Use xfgets
  22 Oct 06 GKY Switch say files on as default so you can tell that seek and scan files is doing something
  07 Jan 07 GKY Add remember search flags to seek and scan
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory and use BldFullPathName
  24 Aug 08 GKY Warn full drive on save of .DAT file; prevent loss of existing file
  10 Dec 08 SHL Integrate exception handler support
  01 Jan 09 GKY Add Seek and Scan to drives & directory context menus pass drive/dir as search root
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  07 Oct 09 SHL Remember last search mask across runs
  17 Jan 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  08 Feb 14 SHL Support svn git hg CVS etc. metadata directory ignore

  FIXME for more excess locals to be gone

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <time.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h
#define INCL_WINSTDCNR			// makelist.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "arccnrs.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "grep.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "pathutil.h"			// BldFullPathName
#include "walkem.h"			// FillPathListBox
#include "grep2.h"
#include "wrappers.h"			// xfgets
#include "misc.h"			// LoadLibPath
#include "strips.h"			// bstrip
#include "dirs.h"			// save_dir2
#include "fortify.h"
#include "excputil.h"			// xbeginthread
#include "valid.h"			// IsFile

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static PCSZ PSCZ_GREP_LASTMASK_SELECT = "Grep_LastMaskSelect";

MRESULT EXPENTRY EnvDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SHORT sSelect;
  CHAR *p;
  CHAR s[CCHMAXPATH];
  CHAR szPath[CCHMAXPATH];

  static CHAR lastenv[CCHMAXPATH] = "DPATH";

  switch (msg) {
  case WM_INITDLG:
    if (mp2) {
      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      *(CHAR *)mp2 = 0;
      {
	PCSZ p;
	PSZ pp;

	p = GetPString(IDS_ENVVARNAMES);
	while (*p == ' ')
	  p++;
	while (*p) {
	  *szPath = 0;
	  pp = szPath;
	  while (*p && *p != ' ')
	    *pp++ = *p++;
	  *pp = 0;
	  while (*p == ' ')
	    p++;
	  if (*szPath)
	    WinSendDlgItemMsg(hwnd,
			      ENV_LISTBOX,
			      LM_INSERTITEM,
			      MPFROM2SHORT(LIT_END, 0), MPFROMP(szPath));
	}
      }
      WinSendDlgItemMsg(hwnd,
			ENV_NAME,
			EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
      WinSetDlgItemText(hwnd, ENV_NAME, lastenv);
      WinSendDlgItemMsg(hwnd,
			ENV_NAME,
			EM_SETSEL, MPFROM2SHORT(0, CCHMAXPATH), MPVOID);
    }
    else
      WinDismissDlg(hwnd, 0);
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case ENV_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_SELECT:
	{
	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      ENV_LISTBOX,
					      LM_QUERYSELECTION,
					      MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (sSelect >= 0) {
	    *s = 0;
	    WinSendDlgItemMsg(hwnd,
			      ENV_LISTBOX,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect, CCHMAXPATH), MPFROMP(s));
	    bstrip(s);
	    if (*s)
	      WinSetDlgItemText(hwnd, ENV_NAME, s);
	  }
	}
	break;
      case LN_ENTER:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    case DID_OK:
      p = WinQueryWindowPtr(hwnd, QWL_USER);
      if (p) {
	WinQueryDlgItemText(hwnd, ENV_NAME, CCHMAXPATH, p);
	bstrip(p);
	if (!*p) {
	  if (!fAlertBeepOff)
	    DosBeep(50, 100);
	  WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, ENV_NAME));
	}
	else {
	  strcpy(lastenv, p);
	  WinDismissDlg(hwnd, 1);
	}
      }
      break;
    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_ENV, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/**
 * Add mask to mask list
 * If mask contains both path an wildcard components if is just appended
 * If mask contains only wildcard component, path is extracted from last mask in masks
 * If mask contains only path components, wildcard is extracted from last mask in masks
 * @param masks is semicolon separated mask string to be updated
 * @param newMask is mask to append
 * @note code limits mask length to 8192 to match GrepDlgProc
 * @returns true if OK
 */

#if 0 // 2010-09-27 SHL	FIXME to use

static BOOL updateMaskString(CHAR masks[8192], PCSZ newMask)
{
  BOOL ok = TRUE;
  BOOL isPath;
  BOOL isWild;
  UINT len;
  CHAR mask[CCHMAXPATH + 2];		// 2 for semicolons
  CHAR wildcard[CCHMAXPATH];
  PSZ p;

  // 2010-09-27 SHL FIXME to be globally available
  #define CH_QMARK '?'
  #define CH_STAR '*'
  #define CH_BACKSLASH '\\'
  #define CH_SEMICOLON ';'
  static PCSZ PCSZ_SEMICOLON = ";";	// 2010-09-27 SHL FIXME to be globally available from init.c

  // Extract last mask
  len = strlen(masks);
  if (!len)
    *mask = 0;
  else {
    if (len < CCHMAXPATH + 2)
      strcpy(mask, masks);
    else {
      strcpy(mask, masks + len - (CCHMAXPATH + 1));
      len = strlen(mask);
    }
  }
  if (len) {
    p = mask + len - 1;
    if (*p == CH_SEMICOLON)
      *p = 0;				// Chop trailing
    p = strrchr(mask, CH_SEMICOLON);	// Find last
    if (p)
      memmove(mask, p + 1, strlen(p + 1) + 1);	// Move to front
  }

  isPath = strchr(newMask, CH_BACKSLASH) != NULL;	// 2010-09-27 SHL FIXME if test too weak
  // 2010-09-27 SHL FIXME to have function
  isWild = strchr(newMask, CH_STAR) || strchr(newMask, CH_QMARK);

  // Assume ready to do if both path and wildcard otherwise edit

  if (isPath && !isWild) {
    // Extract wildcard from old mask
    p = strrchr(mask, CH_BACKSLASH);
    if (!p)
      strcpy(wildcard, mask);
    else
      strcpy(wildcard, p + 1);
    // Append wildcard to path
    strcpy(mask, newMask);
    bstrip(mask);
    len = strlen(mask);
    if (len && mask[len - 1] != CH_BACKSLASH)
      strcat(mask, PCSZ_BACKSLASH);
    strcat(mask, wildcard);
  }
  else if (!isPath) {
    // Use path from old mask
    len = strlen(mask);
    p = strrchr(mask, CH_BACKSLASH);
    if (p)
      *(p + 1) = 0;
    else
      *mask = 0;			// Assume just wilcard
    // Append new wildcard or filename
    strcat(mask, newMask);
    bstrip(mask);
  }

  if (strlen(masks) + strlen(mask) + 3 > sizeof(masks))
    Runtime_Error(pszSrcFile, __LINE__, "too big");

  // Append separator
  if (masks[strlen(masks) - 1] != CH_SEMICOLON)
    strcat(masks, PCSZ_SEMICOLON);
  // Append mask to list
  strcat(masks, mask);

  return ok;
}

#endif // 2010-09-27 SHL	FIXME to use

MRESULT EXPENTRY GrepDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  HWND hwndCollect;
  HWND hwndMLE = WinWindowFromID(hwnd, GREP_SEARCH);
  FILE *fp;
  ULONG ul;
  LONG lLen;
  SHORT sSelect;
  CHAR *p;
  GREPINFO *GrepInfo;
  ULONG size;
  CHAR simple[8192];
  CHAR path[CCHMAXPATH];
  CHAR s[8192 + 14];
  CHAR *moder = "r";

  static BOOL fInitDone;		// First time init done
  // 07 Oct 09 SHL FIXME to not be static
  // 07 Oct 09 SHL FIXME to save to profile?
  // GREP_FRAME dialog buffers
  static CHAR fileMasks[8192];		// ; separated
  static CHAR searchText[4096];		// Structured
  static BOOL recurse = TRUE;
  static BOOL sensitive;
  static BOOL absolute;
  static BOOL sayfiles;
  static BOOL searchEAs = TRUE;
  static BOOL searchFiles = TRUE;
  static BOOL findifany = TRUE;
  static BOOL rememberSettings;
  static UINT newer = 0;
  static UINT older = 0;
  static ULONG greaterthan = 0;
  static ULONG lessthan = 0;
  static BOOL ignoreSVN;

  static BOOL maskListChanged;
  static SHORT sLastMaskSelect = LIT_NONE;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    GrepInfo = mp2;
    if (GrepInfo->szGrepPath && IsFile(GrepInfo->szGrepPath) == 0) {
      BldFullPathName(fileMasks, GrepInfo->szGrepPath, "*");	// Directory passed
      sLastMaskSelect = LIT_NONE;
      fInitDone = TRUE;
    }
    else {
      size = sizeof(sLastMaskSelect);
      PrfQueryProfileData(fmprof, appname, (CHAR *) PSCZ_GREP_LASTMASK_SELECT, &sLastMaskSelect, &size);
      if (sLastMaskSelect >= 0)
      fInitDone = TRUE;
    }
    if (!fInitDone) {
      fileMasks[0] = '*';
      fileMasks[1] = 0;
    }

    WinSetWindowULong(hwnd, QWL_USER, *(HWND *) GrepInfo->hwnd);
    WinSendDlgItemMsg(hwnd,
		      GREP_MASK,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(8192, 0), MPVOID);
    MLEsetlimit(hwndMLE, 4096);
    MLEsetformat(hwndMLE, MLFIE_NOTRANS);
    WinSendDlgItemMsg(hwnd,
		      GREP_NEWER,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(34, 0), MPVOID);
    WinSendDlgItemMsg(hwnd,
		      GREP_OLDER,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(34, 0), MPVOID);
    WinSendDlgItemMsg(hwnd,
		      GREP_GREATER,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(34, 0), MPVOID);
    WinSendDlgItemMsg(hwnd,
		      GREP_LESSER,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(34, 0), MPVOID);
    WinSetDlgItemText(hwnd, GREP_MASK, fileMasks);
    WinSendDlgItemMsg(hwnd,
		      GREP_MASK, EM_SETSEL, MPFROM2SHORT(0, 8192), MPVOID);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "RememberFlagsGrep",
			(PVOID) & rememberSettings, &size);
    WinCheckButton(hwnd, GREP_REMEMBERFLAGS, rememberSettings);
    if (rememberSettings) {
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Recurse",
			  (PVOID) & recurse, &size);
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Absolute",
			  (PVOID) & absolute, &size);
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Case",
			  (PVOID) & sensitive, &size);
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Sayfiles",
			  (PVOID) & sayfiles, &size);
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Searchfiles",
			  (PVOID) & searchFiles, &size);
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_SearchfEAs",
			  (PVOID) & searchEAs, &size);
    }
    if (!rememberSettings) {
      recurse = TRUE;
      sensitive = FALSE;
      absolute = FALSE;
      sayfiles = TRUE;
      ignoreSVN = TRUE;
      searchEAs = TRUE;
      searchFiles = TRUE;
    }
    WinSetWindowText(hwndMLE, searchText);
    if (*searchText) {
      MLEsetcurpos(hwndMLE, 0);
      MLEsetcurposa(hwndMLE, 4096);
      if (!searchEAs)
	searchFiles = TRUE;
    }
    WinCheckButton(hwnd, GREP_RECURSE, recurse);
    WinCheckButton(hwnd, GREP_ABSOLUTE, absolute);
    WinCheckButton(hwnd, GREP_CASE, sensitive);
    WinCheckButton(hwnd, GREP_SAYFILES, sayfiles);
    WinCheckButton(hwnd, GREP_SEARCHEAS, searchEAs);
    WinCheckButton(hwnd, GREP_SEARCHFILES, searchFiles);
    WinCheckButton(hwnd, GREP_FINDIFANY, findifany);
    WinCheckButton(hwnd, GREP_IGNORESVN, ignoreSVN);

    sprintf(s, "%lu", greaterthan);
    WinSetDlgItemText(hwnd, GREP_GREATER, s);
    sprintf(s, "%lu", lessthan);
    WinSetDlgItemText(hwnd, GREP_LESSER, s);
    sprintf(s, "%u", newer);
    WinSetDlgItemText(hwnd, GREP_NEWER, s);
    sprintf(s, "%u", older);
    WinSetDlgItemText(hwnd, GREP_OLDER, s);

    WinEnableWindow(WinWindowFromID(hwnd, GREP_IGNOREEXTDUPES), FALSE);
    WinEnableWindow(WinWindowFromID(hwnd, GREP_CRCDUPES), FALSE);
    WinEnableWindow(WinWindowFromID(hwnd, GREP_NOSIZEDUPES), FALSE);

    // Fill mask listbox
    BldFullPathName(s, pFM2SaveDirectory, PCSZ_GREPMASKDAT);
    fp = xfsopen(s, moder, SH_DENYWR, pszSrcFile, __LINE__, TRUE);
    if (fp) {
      while (!feof(fp)) {
	if (!xfgets_bstripcr(s, 8192 + 4, fp, pszSrcFile, __LINE__))
	  break;
	if (*s && *s != ';') {
	  WinSendDlgItemMsg(hwnd,
			    GREP_LISTBOX,
			    LM_INSERTITEM,
			    MPFROM2SHORT(LIT_SORTASCENDING, 0), MPFROMP(s));
	}
      }
      fclose(fp);
    }
    FillPathListBox(hwnd,
		    WinWindowFromID(hwnd, GREP_DRIVELIST),
		    (HWND) 0, NULL, FALSE);
    // 25 Sep 09 SHL FIXME select drive matching current container?
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, GREP_HELP),
			(HPS) 0, FALSE, TRUE);
    return 0;

  case UM_FOCUSME:
    // set focus to window hwnd in mp1
    if (mp1)
      WinSetFocus(HWND_DESKTOP, (HWND) mp1);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case GREP_REMEMBERFLAGS:
      {
	BOOL rememberSettings = WinQueryButtonCheckstate(hwnd, GREP_REMEMBERFLAGS);

	PrfWriteProfileData(fmprof, FM3Str, "RememberFlagsGrep",
			    (PVOID) & rememberSettings, sizeof(BOOL));
      }
      break;

    case GREP_DRIVELIST:
      switch (SHORT2FROMMP(mp1)) {
      case LN_KILLFOCUS:
	WinSetDlgItemText(hwnd,
			  GREP_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
	break;
      case LN_SETFOCUS:
	WinSetDlgItemText(hwnd,
			  GREP_HELP, (CHAR *) GetPString(IDS_2CLICKADDDRVMASKTEXT));
	break;
      case LN_ENTER:
	WinQueryDlgItemText(hwnd, GREP_MASK, 8192, s);
	bstrip(s);
	// Find last wildcard
	p = strrchr(s, '\\');
	if (p)
	  strcpy(simple, p);
	else if (*s) {
	  strcpy(simple, PCSZ_BACKSLASH);
	  strcat(simple, s);
	  *s = 0;
	}
	else
	  strcpy(simple, "\\*");
	if (simple[strlen(simple) - 1] == ';')
	  simple[strlen(simple) - 1] = 0;	// Chop trailing semi
	lLen = strlen(simple) + 1;
	if (strlen(s) > 8192 - lLen) {
	  Runtime_Error(pszSrcFile, __LINE__, "too big");
	  WinSetDlgItemText(hwnd, GREP_MASK, s);
	  break;
	}

	// 19 Aug 10 SHL FIXME to honor append
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    GREP_DRIVELIST,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(LIT_FIRST), MPVOID);
	if (sSelect >= 0) {
	  if (*s && s[strlen(s) - 1] != ';')
	    strcat(s, ";");
	  WinSendDlgItemMsg(hwnd,
			    GREP_DRIVELIST,
			    LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect,
					 (8192 - strlen(s)) - lLen),
			    MPFROMP(&s[strlen(s)]));
	  rstrip(s);
	  if (*s) {
	    strcat(s, simple);
	    WinSetDlgItemText(hwnd, GREP_MASK, s);
	    WinSendDlgItemMsg(hwnd,
			      GREP_MASK,
			      EM_SETSEL,
			      MPFROM2SHORT(strlen(s) - (lLen + 1),
					   strlen(s)), MPVOID);
	    PostMsg(hwnd,
		    UM_FOCUSME,
		    MPFROMLONG(WinWindowFromID(hwnd, GREP_MASK)), MPVOID);
	  }
	}
	break; // LN_ENTER
      } // switch
      break; //GREP_DRIVELIST

    case GREP_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_KILLFOCUS:
	WinSetDlgItemText(hwnd,
			  GREP_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
	break;
      case LN_SETFOCUS:
	WinSetDlgItemText(hwnd, GREP_HELP, (CHAR *) GetPString(IDS_ADDSELDELMASKTEXT));
	break;
      case LN_ENTER:
      case LN_SELECT:
	// Enter checkbox controls whether or not notice is ignored
	// If not checked, enter is ignored because select has already been processed
	// If check, select is ignored and enter is processed
	if ((SHORT2FROMMP(mp1) == LN_ENTER &&
	     !WinQueryButtonCheckstate(hwnd, GREP_APPEND)) ||
	    (SHORT2FROMMP(mp1) == LN_SELECT &&
	     WinQueryButtonCheckstate(hwnd, GREP_APPEND)))
	  break;			// Ignore
	{
	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      GREP_LISTBOX,
					      LM_QUERYSELECTION,
					      MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (sSelect >= 0) {
	    sLastMaskSelect = sSelect;
	    *s = 0;
	    if (WinQueryButtonCheckstate(hwnd, GREP_APPEND)) {
	      // Append to existing
	      WinQueryDlgItemText(hwnd, GREP_MASK, 8192, s);
	      bstrip(s);
	      if (*s && strlen(s) < 8190 && s[strlen(s) - 1] != ';')
		strcat(s, ";");
	    }
	    // Append listbox item to mask string
	    WinSendDlgItemMsg(hwnd,
			      GREP_LISTBOX,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect, 8192 - strlen(s)),
			      MPFROMP(s + strlen(s)));
	    bstrip(s);
	    if (*s)
	      WinSetDlgItemText(hwnd, GREP_MASK, s);
	  }
	}
	break;
      }
      break; // GREP_LISTBOX

    case GREP_MASK:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, (CHAR *) GetPString(IDS_MASKSFINDTEXT));
      break;
    case GREP_SEARCH:
      if (SHORT2FROMMP(mp1) == MLN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == MLN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, (CHAR *) GetPString(IDS_TEXTFINDTEXT));
      break;
    case GREP_GREATER:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, (CHAR *) GetPString(IDS_MINSIZEFINDTEXT));
      break;
    case GREP_LESSER:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, (CHAR *) GetPString(IDS_MAXSIZEFINDTEXT));
      break;
    case GREP_NEWER:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, (CHAR *) GetPString(IDS_MAXAGEFINDTEXT));
      break;
    case GREP_OLDER:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, (CHAR *) GetPString(IDS_MINAGEFINDTEXT));
      break;
    case GREP_FINDDUPES:
      {
	BOOL finddupes = WinQueryButtonCheckstate(hwnd, GREP_FINDDUPES);

	WinEnableWindow(WinWindowFromID(hwnd, GREP_SEARCH), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_ABSOLUTE), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_CASE), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_CRCDUPES), finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_NOSIZEDUPES), finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_IGNOREEXTDUPES),
			finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_SEARCHFILES), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_SEARCHEAS), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_GREATER), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_LESSER), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_NEWER), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_OLDER), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_FINDIFANY), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_GK), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_LK), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_NM), !finddupes);
	WinEnableWindow(WinWindowFromID(hwnd, GREP_OM), !finddupes);
	if (finddupes)
	  WinCheckButton(hwnd, GREP_RECURSE, TRUE);
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case GREP_ENV:
      {
	CHAR *t;
	CHAR env[8192];

	*path = 0;
	if (!WinDlgBox(HWND_DESKTOP,
		       hwnd, EnvDlgProc, FM3ModHandle, ENV_FRAME, path)) {
	  break;
	}
	bstrip(path);
	if (!*path)
	  break;
	if (!stricmp(path, PCSZ_LIBPATH))
	  LoadLibPath(env, 8192);
	else {
	  p = getenv(path);
	  if (!p)
	    break;
	  strcpy(env, p);
	}
	bstrip(env);
	if (!*env)
	  break;
	WinQueryDlgItemText(hwnd, GREP_MASK, 8192, s);
	bstrip(s);
	if (strlen(s) > 8192 - 5) {
	  Runtime_Error(pszSrcFile, __LINE__, "too big");
	  break;
	}
	p = strrchr(s, '\\');
	if (p)
	  strcpy(simple, p + 1);
	else if (*s)
	  strcpy(simple, s);
	else
	  strcpy(simple, "*");
	if (!p)
	  *s = 0;
	if (simple[strlen(simple) - 1] == ';')
	  simple[strlen(simple) - 1] = 0;
	lLen = strlen(simple) + 1;
	p = env;
	while (p && *p) {
	  strncpy(path, p, CCHMAXPATH - 1);
	  path[CCHMAXPATH - 1] = 0;
	  t = strchr(path, ';');
	  if (t)
	    *t = 0;
	  bstrip(path);
	  if (isalpha(*path) && path[1] == ':' && path[2] == '\\') {
	    if (strlen(s) > (8192 - lLen) - (strlen(path) + 1)) {
	      WinSetDlgItemText(hwnd, GREP_MASK, s);
	      break;
	    }
	    if (!*s || (*s && s[strlen(s) - 1] != ';')) {
	      if (*s)
		strcat(s, ";");
	      strcat(s, path);
	      lLen += strlen(path);
	      if (s[strlen(s) - 1] != '\\') {
		lLen++;
		strcat(s, PCSZ_BACKSLASH);
	      }
	      rstrip(s);
	      if (*s) {
		strcat(s, simple);
		WinSetDlgItemText(hwnd, GREP_MASK, s);
		// 19 Aug 10 SHL FIXME to honor append
		WinSendDlgItemMsg(hwnd,
				  GREP_MASK,
				  EM_SETSEL,
				  MPFROM2SHORT(strlen(s) - (lLen - 1),
					       strlen(s)), MPVOID);
	      }
	    }
	  }
	  p = strchr(p, ';');
	  if (p)
	    p++;
	}
      }
      break;

    case GREP_WALK:
      WinQueryDlgItemText(hwnd, GREP_MASK, 8192, s);
      bstrip(s);
      if (strlen(s) > 8192 - 5) {
	Runtime_Error(pszSrcFile, __LINE__, "too big");
	break;
      }
      *path = 0;
      if (WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    WalkAllDlgProc,
		    FM3ModHandle, WALK_FRAME, MPFROMP(path)) && *path) {
	p = strrchr(s, '\\');
	if (p)
	  strcpy(simple, p + 1);
	else if (*s)
	  strcpy(simple, s);
	else
	  strcpy(simple, "*");
	if (!p)
	  *s = 0;
	if (simple[strlen(simple) - 1] == ';')
	  simple[strlen(simple) - 1] = 0;
	lLen = strlen(simple) + 1;
	if (strlen(s) > (8192 - lLen) - (strlen(path) + 1)) {
	  Runtime_Error(pszSrcFile, __LINE__, "too big");
	  WinSetDlgItemText(hwnd, GREP_MASK, s);
	  break;
	}
	if (!*s || (*s && s[strlen(s) - 1] != ';')) {
	  if (*s)
	    strcat(s, ";");
	  strcat(s, path);
	  lLen += strlen(path);
	  if (s[strlen(s) - 1] != '\\') {
	    lLen++;
	    strcat(s, PCSZ_BACKSLASH);
	  }
	  rstrip(s);
	  // 25 Sep 09 SHL FIXME to honor append
	  if (*s) {
	    strcat(s, simple);
	    WinSetDlgItemText(hwnd, GREP_MASK, s);
	    WinSendDlgItemMsg(hwnd,
			      GREP_MASK,
			      EM_SETSEL,
			      MPFROM2SHORT(strlen(s) - (lLen - 1),
					   strlen(s)), MPVOID);
	  }
	}
      }
      break;

    case GREP_ADD:
      *s = 0;
      WinQueryDlgItemText(hwnd, GREP_MASK, 8192, s);
      bstrip(s);
      if (*s) {
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    GREP_LISTBOX,
					    LM_SEARCHSTRING,
					    MPFROM2SHORT(0, LIT_FIRST),
					    MPFROMP(s));
	if (sSelect < 0) {
	  WinSendDlgItemMsg(hwnd,
			    GREP_LISTBOX,
			    LM_INSERTITEM,
			    MPFROM2SHORT(LIT_SORTASCENDING, 0), MPFROMP(s));
	  maskListChanged = TRUE;
	}
      }
      break;

    case GREP_DELETE:
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					  GREP_LISTBOX,
					  LM_QUERYSELECTION,
					  MPFROMSHORT(LIT_FIRST), MPVOID);
      if (sSelect >= 0) {
	WinSendDlgItemMsg(hwnd,
			  GREP_LISTBOX,
			  LM_DELETEITEM, MPFROM2SHORT(sSelect, 0), MPVOID);
	if (sSelect >= sLastMaskSelect)
	  sLastMaskSelect--;
	maskListChanged = TRUE;
      }
      break;

    case GREP_OM:
      *s = 0;
      WinQueryDlgItemText(hwnd, GREP_OLDER, 34, s);
      ul = atoi(s) * 30L;
      sprintf(s, "%lu", ul);
      WinSetDlgItemText(hwnd, GREP_OLDER, s);
      break;

    case GREP_NM:
      *s = 0;
      WinQueryDlgItemText(hwnd, GREP_NEWER, 34, s);
      ul = atoi(s) * 30L;
      sprintf(s, "%lu", ul);
      WinSetDlgItemText(hwnd, GREP_NEWER, s);
      break;

    case GREP_GK:
      *s = 0;
      WinQueryDlgItemText(hwnd, GREP_GREATER, 34, s);
      ul = atol(s) * 1024L;
      sprintf(s, "%lu", ul);
      WinSetDlgItemText(hwnd, GREP_GREATER, s);
      break;

    case GREP_LK:
      *s = 0;
      WinQueryDlgItemText(hwnd, GREP_LESSER, 34, s);
      ul = atol(s) * 1024L;
      sprintf(s, "%lu", ul);
      WinSetDlgItemText(hwnd, GREP_LESSER, s);
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_GREP, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case GREP_LOCALHDS:
    case GREP_REMOTEHDS:
    case GREP_ALLHDS:
      {
	CHAR szDrive[] = " :\\";
	ULONG ulDriveNum;
	ULONG ulDriveMap;
	INT x;
	BOOL incl;
	CHAR new[8192];

	// Find first mask
	*s = 0;
	WinQueryDlgItemText(hwnd, GREP_MASK, 8192, s);
	s[8192 - 1] = 0;
	p = strchr(s, ';');
	if (p)
	  *p = 0;
	p = strrchr(s, '\\');
	if (!p)
	  p = strrchr(s, '/');
	if (!p)
	  p = strrchr(s, ':');
	if (p)
	  strcpy(s, p + 1);
	if (!*s)
	  strcpy(s, "*");

	DosError(FERR_DISABLEHARDERR);
	DosQCurDisk(&ulDriveNum, &ulDriveMap);
	*new = 0;
	for (x = 2; x < 26; x++) {
	  incl = FALSE;
	  if (ulDriveMap & (1L << x)) {
	    switch (SHORT1FROMMP(mp1)) {
	    case GREP_ALLHDS:
	      if (!(driveflags[x] & (DRIVE_REMOVABLE | DRIVE_IGNORE | DRIVE_RAMDISK)))
		incl = TRUE;
	      break;
	    case GREP_LOCALHDS:
	      // 06 Jun 10 SHL FIXME to work if drive not prescanned
	      if (!(driveflags[x] &
		    (DRIVE_REMOVABLE | DRIVE_IGNORE | DRIVE_REMOTE |
		     DRIVE_VIRTUAL | DRIVE_RAMDISK)))
		incl = TRUE;
	      break;
	    case GREP_REMOTEHDS:
	      if (!(driveflags[x] & (DRIVE_REMOVABLE | DRIVE_IGNORE)) &&
		  (driveflags[x] & DRIVE_REMOTE))
		incl = TRUE;
	      break;
	    }
	  }
	  if (incl) {
	    if (strlen(new) + strlen(s) + 5 < 8192 - 1) {
	      if (*new)
		strcat(new, ";");
	      *szDrive = x + 'A';
	      strcat(new, szDrive);
	      strcat(new, s);
	    }
	  }
	}
	if (*new)
	  WinSetDlgItemText(hwnd, GREP_MASK, new);
      }
      break;

    case DID_OK:
      hwndCollect = WinQueryWindowULong(hwnd, QWL_USER);
      if (!hwndCollect)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	// 07 Feb 08 SHL - FIXME to malloc and free in thread
	static GREP g;			// Passed to thread
	p = xmalloc(8192 + 512, pszSrcFile, __LINE__);
	if (!p)
	  break;			// Already complained
	memset(&g, 0, sizeof(GREP));
	g.size = sizeof(GREP);
	recurse = WinQueryButtonCheckstate(hwnd, GREP_RECURSE) != 0;
	absolute = WinQueryButtonCheckstate(hwnd, GREP_ABSOLUTE) != 0;
	sensitive = WinQueryButtonCheckstate(hwnd, GREP_CASE) != 0;
	sayfiles = WinQueryButtonCheckstate(hwnd, GREP_SAYFILES) != 0;
	searchEAs = WinQueryButtonCheckstate(hwnd, GREP_SEARCHEAS) != 0;
	searchFiles = WinQueryButtonCheckstate(hwnd, GREP_SEARCHFILES) != 0;
	findifany = WinQueryButtonCheckstate(hwnd, GREP_FINDIFANY) != 0;
	ignoreSVN = WinQueryButtonCheckstate(hwnd, GREP_IGNORESVN) != 0;
	rememberSettings = WinQueryButtonCheckstate(hwnd, GREP_REMEMBERFLAGS);
	if (rememberSettings) {
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Recurse",
			      (PVOID) & recurse, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Absolute",
			      (PVOID) & absolute, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Case",
			      (PVOID) & sensitive, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Sayfiles",
			      (PVOID) & sayfiles, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Searchfiles",
			      (PVOID) & searchFiles, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_SearchfEAs",
			      (PVOID) & searchEAs, sizeof(BOOL));
	}
	PrfWriteProfileData(fmprof, appname,
			    (CHAR *) PSCZ_GREP_LASTMASK_SELECT, &sLastMaskSelect, sizeof(sLastMaskSelect));
	g.finddupes = WinQueryButtonCheckstate(hwnd, GREP_FINDDUPES) != 0;
	if (g.finddupes) {
	  g.CRCdupes = WinQueryButtonCheckstate(hwnd, GREP_CRCDUPES) != 0;
	  g.nosizedupes =
	    WinQueryButtonCheckstate(hwnd, GREP_NOSIZEDUPES) != 0;
	  g.ignoreextdupes =
	    WinQueryButtonCheckstate(hwnd, GREP_IGNOREEXTDUPES) != 0;
	}
	// Parse file masks
	*p = 0;
	WinQueryDlgItemText(hwnd, GREP_MASK, 8192, p);
	bstrip(p);
	if (!*p) {
	  if (!fAlertBeepOff)
	    DosBeep(50, 100);
	  WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, GREP_MASK));
	  free(p);
#	  ifdef FORTIFY
	  Fortify_LeaveScope();
#	   endif
	  break;
	}
	strcpy(g.fileMasks, p);
	strcpy(fileMasks, p);
	// Parse search strings
	*p = 0;
	WinQueryWindowText(hwndMLE, 4096, p);
	strcpy(searchText, p);
	{
	  CHAR *pszFrom;
	  CHAR *pszTo;
	  ULONG matched = 0;

	  pszTo = g.searchPattern;
	  pszFrom = p;
	  while (*pszFrom) {
	    if (*pszFrom == '\r') {
	      pszFrom++;
	      continue;
	    }
	    if (*pszFrom == '\n') {
	      if (*(pszFrom + 1))
		matched++;
	      *pszTo = 0;
	    }
	    else
	      *pszTo = *pszFrom;
	    pszTo++;
	    pszFrom++;
	  }
	  if (*g.searchPattern)
	    matched++;
	  *pszTo++ = 0;
	  *pszTo = 0;
	  g.numlines = matched;
	  if (matched) {
	    g.matched = xmalloc(g.numlines, pszSrcFile, __LINE__);
	    if (!g.matched)
	      g.numlines = 0;
	  }
	}
	*p = 0;
	WinQueryDlgItemText(hwnd, GREP_GREATER, 34, p);
	greaterthan = atol(p);
	*p = 0;
	WinQueryDlgItemText(hwnd, GREP_LESSER, 34, p);
	lessthan = atol(p);
	*p = 0;
	WinQueryDlgItemText(hwnd, GREP_NEWER, 34, p);
	newer = atoi(p);
	*p = 0;
	WinQueryDlgItemText(hwnd, GREP_OLDER, 34, p);
	older = atoi(p);
	if (older || newer) {
	  FDATE fdate;
	  FTIME ftime;
	  struct tm tm;
	  time_t t;

	  t = time(NULL);
	  tm = *localtime(&t);
	  fdate.day = tm.tm_mday;
	  fdate.month = tm.tm_mon + 1;
	  fdate.year = tm.tm_year - 80;
	  ftime.hours = tm.tm_hour;
	  ftime.minutes = tm.tm_min;
	  ftime.twosecs = tm.tm_sec / 2;
	  if (older) {
	    g.olderthan = SecsSince1980(&fdate, &ftime);
	    g.olderthan -= (older * (24L * 60L * 60L));
	  }
	  if (newer) {
	    g.newerthan = SecsSince1980(&fdate, &ftime);
	    g.newerthan -= (newer * (24L * 60L * 60L));
	  }
	}
	if (!newer)
	  g.newerthan = 0;
	if (!older)
	  g.olderthan = 0;
	g.greaterthan = greaterthan;
	g.lessthan = lessthan;
	g.absFlag = absolute;
	g.caseFlag = sensitive;
	g.dirFlag = recurse;
	g.sayfiles = sayfiles;
	g.searchEAs = searchEAs;
	g.searchFiles = searchFiles;
	g.findifany = findifany;
	g.ignoreSVN = ignoreSVN;

	g.hwndFiles = hwndCollect;
	g.hwnd = WinQueryWindow(hwndCollect, QW_PARENT);
	g.hwndCurFile = WinWindowFromID(g.hwnd, DIR_SELECTED);

	// Get settings from collector filter
	g.attrFile = ((DIRCNRDATA *)INSTDATA(hwndCollect))->mask.attrFile;
	g.antiattr = ((DIRCNRDATA *)INSTDATA(hwndCollect))->mask.antiattr;
	g.stopflag = &((DIRCNRDATA *)INSTDATA(hwndCollect))->stopflag;

	if (rememberSettings) {
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Recurse",
			      (PVOID) & recurse, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Absolute",
			      (PVOID) & absolute, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Case",
			      (PVOID) & sensitive, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Sayfiles",
			      (PVOID) & sayfiles, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_Searchfiles",
			      (PVOID) & searchFiles, sizeof(BOOL));
	  PrfWriteProfileData(fmprof, FM3Str, "Grep_SearchfEAs",
			      (PVOID) & searchEAs, sizeof(BOOL));
	}

	if (xbeginthread(GrepThread,
			 524280,
			 &g,
			 pszSrcFile,
			 __LINE__) == -1)
	{
	  free(p);
#	  ifdef FORTIFY
	  Fortify_LeaveScope();
#	   endif
	  WinDismissDlg(hwnd, 0);
	  break;
	}
	DosSleep(100);	  
	free(p);
#	ifdef FORTIFY
	Fortify_LeaveScope();
#	 endif
      }
      if (maskListChanged) {
	// Save modified mask list
	SHORT x;

	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    GREP_LISTBOX,
					    LM_QUERYITEMCOUNT,
					    MPVOID, MPVOID);
	// 07 Oct 09 SHL Rewrite if list empty
	if (sSelect >= 0) {
	  CHAR *modew = "w";

	  BldFullPathName(s, pFM2SaveDirectory, PCSZ_GREPMASKDAT);
	  if (CheckDriveSpaceAvail(s, ullDATFileSpaceNeeded, 1) == 2)
	    break; //already gave error msg
	  fp = xfopen(s, modew, pszSrcFile, __LINE__, FALSE);
	  if (fp) {
	    fputs(GetPString(IDS_GREPFILETEXT), fp);
	    for (x = 0; x < sSelect; x++) {
	      *s = 0;
	      WinSendDlgItemMsg(hwnd,
				GREP_LISTBOX,
				LM_QUERYITEMTEXT,
				MPFROM2SHORT(x, 8192), MPFROMP(s));
	      bstrip(s);
	      if (*s)
		fprintf(fp, "%s\n", s);
	    }
	    fclose(fp);
	  }
	}
      }
      WinDismissDlg(hwnd, 1);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(GREP,GrepDlgProc,EnvDlgProc)
