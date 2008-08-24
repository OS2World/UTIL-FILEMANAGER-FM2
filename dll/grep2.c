
/***********************************************************************

  $Id$

  Grep dialog for collector

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2006 Steven H. Levine

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

  fixme for more excess locals to be gone

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <process.h>			// _beginthread

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h
#define INCL_WINSTDCNR			// makelist.h

#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "grep.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "pathutil.h"                   // BldFullPathName
#include "fm3dll.h"
#include "fortify.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

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
	CHAR *p;
	CHAR *pp;

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

MRESULT EXPENTRY GrepDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  HWND hwndCollect;
  HWND hwndMLE = WinWindowFromID(hwnd, GREP_SEARCH);
  FILE *fp;
  ULONG ul;
  LONG lLen;
  SHORT sSelect;
  CHAR *p;
  CHAR s[8192 + 14];
  CHAR simple[8192];
  CHAR path[CCHMAXPATH];

  static CHAR lastmask[8192] = "*";
  static CHAR lasttext[4096] = "";
  static BOOL recurse = TRUE;
  static BOOL sensitive = FALSE;
  static BOOL absolute = FALSE;
  static BOOL sayfiles = TRUE;
  static BOOL searchEAs = TRUE;
  static BOOL searchFiles = TRUE;
  static BOOL changed = FALSE;
  static BOOL findifany = TRUE;
  static BOOL gRemember = FALSE;
  ULONG size = sizeof(BOOL);
  static UINT newer = 0;
  static UINT older = 0;
  static ULONG greater = 0;
  static ULONG lesser = 0;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    WinSetWindowULong(hwnd, QWL_USER, *(HWND *) mp2);
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
    WinSetDlgItemText(hwnd, GREP_MASK, lastmask);
    WinSendDlgItemMsg(hwnd,
		      GREP_MASK, EM_SETSEL, MPFROM2SHORT(0, 8192), MPVOID);
    PrfQueryProfileData(fmprof, FM3Str, "RememberFlagsGrep",
			(PVOID) & gRemember, &size);
    WinCheckButton(hwnd, GREP_REMEMBERFLAGS, gRemember);
    if (gRemember) {
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Recurse",
			  (PVOID) & recurse, &size);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Absolute",
			  (PVOID) & absolute, &size);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Case",
			  (PVOID) & sensitive, &size);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Sayfiles",
			  (PVOID) & sayfiles, &size);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_Searchfiles",
			  (PVOID) & searchFiles, &size);
      PrfQueryProfileData(fmprof, FM3Str, "Grep_SearchfEAs",
			  (PVOID) & searchEAs, &size);
    }
    if (!gRemember) {
      recurse = TRUE;
      sensitive = FALSE;
      absolute = FALSE;
      sayfiles = TRUE;
      searchEAs = TRUE;
      searchFiles = TRUE;
    }
    WinSetWindowText(hwndMLE, lasttext);
    if (*lasttext) {
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

    sprintf(s, "%lu", greater);
    WinSetDlgItemText(hwnd, GREP_GREATER, s);
    sprintf(s, "%lu", lesser);
    WinSetDlgItemText(hwnd, GREP_LESSER, s);
    sprintf(s, "%u", newer);
    WinSetDlgItemText(hwnd, GREP_NEWER, s);
    sprintf(s, "%u", older);
    WinSetDlgItemText(hwnd, GREP_OLDER, s);

    WinEnableWindow(WinWindowFromID(hwnd, GREP_IGNOREEXTDUPES), FALSE);
    WinEnableWindow(WinWindowFromID(hwnd, GREP_CRCDUPES), FALSE);
    WinEnableWindow(WinWindowFromID(hwnd, GREP_NOSIZEDUPES), FALSE);

    BldFullPathName(s, pFM2SaveDirectory, "GREPMASK.DAT");
    fp = _fsopen(s, "r", SH_DENYWR);
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
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, GREP_HELP),
			(HPS) 0, FALSE, TRUE);
    return 0;

  case UM_FOCUSME:
    /* set focus to window hwnd in mp1 */
    if (mp1)
      WinSetFocus(HWND_DESKTOP, (HWND) mp1);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case GREP_REMEMBERFLAGS:
      {
	BOOL gRemember = WinQueryButtonCheckstate(hwnd, GREP_REMEMBERFLAGS);

	PrfWriteProfileData(fmprof, FM3Str, "RememberFlagsGrep",
			    (PVOID) & gRemember, sizeof(BOOL));
      }
      break;

    case GREP_DRIVELIST:
      switch (SHORT2FROMMP(mp1)) {
      case LN_KILLFOCUS:
	WinSetDlgItemText(hwnd,
			  GREP_HELP, GetPString(IDS_ARCDEFAULTHELPTEXT));
	break;
      case LN_SETFOCUS:
	WinSetDlgItemText(hwnd,
			  GREP_HELP, GetPString(IDS_2CLICKADDDRVMASKTEXT));
	break;
      case LN_ENTER:
	WinQueryDlgItemText(hwnd, GREP_MASK, 8192, s);
	bstrip(s);
	p = strrchr(s, '\\');
	if (p)
	  strcpy(simple, p);
	else if (*s) {
	  strcpy(simple, "\\");
	  strcat(simple, s);
	  *s = 0;
	}
	else
	  strcpy(simple, "\\*");
	if (simple[strlen(simple) - 1] == ';')
	  simple[strlen(simple) - 1] = 0;
	lLen = strlen(simple) + 1;
	if (strlen(s) > 8192 - lLen) {
	  Runtime_Error(pszSrcFile, __LINE__, "too big");
	  WinSetDlgItemText(hwnd, GREP_MASK, s);
	  break;
	}

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
	break;				// LN_ENTER
      }					// switch
      break;

    case GREP_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_KILLFOCUS:
	WinSetDlgItemText(hwnd,
			  GREP_HELP, GetPString(IDS_ARCDEFAULTHELPTEXT));
	break;
      case LN_SETFOCUS:
	WinSetDlgItemText(hwnd, GREP_HELP, GetPString(IDS_ADDSELDELMASKTEXT));
	break;
      case LN_ENTER:
      case LN_SELECT:
	if ((SHORT2FROMMP(mp1) == LN_ENTER &&
	     !WinQueryButtonCheckstate(hwnd, GREP_APPEND)) ||
	    (SHORT2FROMMP(mp1) == LN_SELECT &&
	     WinQueryButtonCheckstate(hwnd, GREP_APPEND)))
	  break;
	{
	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      GREP_LISTBOX,
					      LM_QUERYSELECTION,
					      MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (sSelect >= 0) {
	    *s = 0;
	    if (WinQueryButtonCheckstate(hwnd, GREP_APPEND)) {
	      WinQueryDlgItemText(hwnd, GREP_MASK, 8192, s);
	      bstrip(s);
	      if (*s && strlen(s) < 8190 && s[strlen(s) - 1] != ';')
		strcat(s, ";");
	    }
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
      break;

    case GREP_MASK:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, GetPString(IDS_MASKSFINDTEXT));
      break;
    case GREP_SEARCH:
      if (SHORT2FROMMP(mp1) == MLN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == MLN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, GetPString(IDS_TEXTFINDTEXT));
      break;
    case GREP_GREATER:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, GetPString(IDS_MINSIZEFINDTEXT));
      break;
    case GREP_LESSER:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, GetPString(IDS_MAXSIZEFINDTEXT));
      break;
    case GREP_NEWER:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, GetPString(IDS_MAXAGEFINDTEXT));
      break;
    case GREP_OLDER:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  GREP_HELP, GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, GREP_HELP, GetPString(IDS_MINAGEFINDTEXT));
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
	if (!stricmp(path, "LIBPATH"))
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
		strcat(s, "\\");
	      }
	      rstrip(s);
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
	    strcat(s, "\\");
	  }
	  rstrip(s);
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
	  changed = TRUE;
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
	changed = TRUE;
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
	  if (ulDriveMap & (1L << x)) {
	    incl = FALSE;
	    switch (SHORT1FROMMP(mp1)) {
	    case GREP_ALLHDS:
	      if (!(driveflags[x] & (DRIVE_REMOVABLE | DRIVE_IGNORE)))
		incl = TRUE;
	      break;
	    case GREP_LOCALHDS:
	      if (!(driveflags[x] &
		    (DRIVE_REMOVABLE | DRIVE_IGNORE | DRIVE_REMOTE |
		     DRIVE_VIRTUAL)))
		incl = TRUE;
	      break;
	    case GREP_REMOTEHDS:
	      if (!(driveflags[x] &
		    (DRIVE_REMOVABLE | DRIVE_IGNORE)) &&
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
	Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      else {
	// 07 Feb 08 SHL - fixme to malloc and free in thread
	static GREP g;		// Passed to thread

	p = xmalloc(8192 + 512, pszSrcFile, __LINE__);
	if (!p)
	  break;
	memset(&g, 0, sizeof(GREP));
	g.size = sizeof(GREP);
	recurse = WinQueryButtonCheckstate(hwnd, GREP_RECURSE) != 0;
	absolute = WinQueryButtonCheckstate(hwnd, GREP_ABSOLUTE) != 0;
	sensitive = WinQueryButtonCheckstate(hwnd, GREP_CASE) != 0;
	sayfiles = WinQueryButtonCheckstate(hwnd, GREP_SAYFILES) != 0;
	searchEAs = WinQueryButtonCheckstate(hwnd, GREP_SEARCHEAS) != 0;
	searchFiles = WinQueryButtonCheckstate(hwnd, GREP_SEARCHFILES) != 0;
	findifany = WinQueryButtonCheckstate(hwnd, GREP_FINDIFANY) != 0;
	gRemember = WinQueryButtonCheckstate(hwnd, GREP_REMEMBERFLAGS);
	if (gRemember) {
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
	  DosBeep(50, 100);
	  WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, GREP_MASK));
	  free(p);
#         ifdef FORTIFY
          Fortify_LeaveScope();
#          endif
	  break;
	}
	strcpy(g.tosearch, p);
	strcpy(lastmask, p);
	// Parse search strings
	*p = 0;
	WinQueryWindowText(hwndMLE, 4096, p);
	strcpy(lasttext, p);
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
	greater = atol(p);
	*p = 0;
	WinQueryDlgItemText(hwnd, GREP_LESSER, 34, p);
	lesser = atol(p);
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
	g.greaterthan = greater;
	g.lessthan = lesser;
	g.absFlag = absolute;
	g.caseFlag = sensitive;
	g.dirFlag = recurse;
	g.sayfiles = sayfiles;
	g.searchEAs = searchEAs;
	g.searchFiles = searchFiles;
	g.findifany = findifany;
	g.hwndFiles = hwndCollect;
	g.hwnd = WinQueryWindow(hwndCollect, QW_PARENT);
	g.hwndCurFile = WinWindowFromID(g.hwnd, DIR_SELECTED);
	g.attrFile = ((DIRCNRDATA *)INSTDATA(hwndCollect))->mask.attrFile;
	g.antiattr = ((DIRCNRDATA *)INSTDATA(hwndCollect))->mask.antiattr;
	g.stopflag = &((DIRCNRDATA *)INSTDATA(hwndCollect))->stopflag;
	if (_beginthread(GrepThread, NULL, 524280, (PVOID) & g) == -1) {
	  Runtime_Error(pszSrcFile, __LINE__,
			GetPString(IDS_COULDNTSTARTTHREADTEXT));
	  free(p);
#         ifdef FORTIFY
          Fortify_LeaveScope();
#          endif
	  WinDismissDlg(hwnd, 0);
	  break;
	}
	DosSleep(100); //05 Aug 07 GKY 128
	free(p);
#       ifdef FORTIFY
        Fortify_LeaveScope();
#        endif
      }
      if (changed) {
	// Grep mask list changed
	SHORT x;

	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    GREP_LISTBOX,
					    LM_QUERYITEMCOUNT,
					    MPVOID, MPVOID);
        if (sSelect > 0) {
          BldFullPathName(s, pFM2SaveDirectory, "GREPMASK.DAT");
          if (CheckDriveSpaceAvail(s, ullDATFileSpaceNeeded, 1) == 2)
            break; //already gave error msg
	  fp = xfopen(s, "w", pszSrcFile, __LINE__);
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
