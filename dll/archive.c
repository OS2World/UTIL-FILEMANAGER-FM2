
/***********************************************************************

  $Id$

  Archive create/update dialog procedure

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2006 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  28 Jun 06 SHL Drop obsoletes
  17 Jul 06 SHL Use Runtime_Error
  22 Mar 07 GKY Use QWL_USER

***********************************************************************/

#define INCL_WIN
#define INCL_DOS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <share.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(FMARCHIVE,ArchiveDlgProc)

MRESULT EXPENTRY ArchiveDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *arcdata = NULL;

  switch (msg) {
  case WM_INITDLG:
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    arcdata = (DIRCNRDATA *) mp2;
    WinSendDlgItemMsg(hwnd, ARCH_ARCNAME, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    if (!arcdata->namecanchange) {
      WinSendDlgItemMsg(hwnd, ARCH_ARCNAME, EM_SETREADONLY,
			MPFROM2SHORT(TRUE, 0), MPVOID);
      WinEnableWindow(WinWindowFromID(hwnd, ARCH_FIND), FALSE);
      WinShowWindow(WinWindowFromID(hwnd, ARCH_FIND), FALSE);
    }
    WinSendDlgItemMsg(hwnd, ARCH_COMMAND, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(256, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, ARCH_MASKS, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(256, 0), MPVOID);
    WinSetDlgItemText(hwnd, ARCH_ARCNAME, arcdata->arcname);
    if (arcdata->fmoving && arcdata->info->move) {
      WinSetDlgItemText(hwnd, ARCH_COMMAND, arcdata->info->move);
      WinSendDlgItemMsg(hwnd, ARCH_MOVE, BM_SETCHECK,
			MPFROM2SHORT(TRUE, 0), MPVOID);
    }
    else
      WinSetDlgItemText(hwnd, ARCH_COMMAND, arcdata->info->create);
    if (!arcdata->info->createrecurse)
      WinEnableWindow(WinWindowFromID(hwnd, ARCH_RECURSE), FALSE);
    if (!arcdata->info->move && !arcdata->info->movewdirs)
      WinEnableWindow(WinWindowFromID(hwnd, ARCH_MOVE), FALSE);
    if (!arcdata->info->createwdirs && !arcdata->info->movewdirs)
      WinEnableWindow(WinWindowFromID(hwnd, ARCH_INCLPATH), FALSE);
    *arcdata->command = 0;
    PosOverOkay(hwnd);
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, ARCH_HELP), (HPS) 0, FALSE,
			TRUE);
    return 0;

  case WM_CONTROL:
    arcdata = (DIRCNRDATA *) WinQueryWindowPtr(hwnd, QWL_USER);
    switch (SHORT1FROMMP(mp1)) {
    case ARCH_ARCNAME:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ARCH_HELP,
			  GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, ARCH_HELP,
			  GetPString(IDS_ARCARCNAMEHELPTEXT));
      break;

    case ARCH_COMMAND:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ARCH_HELP,
			  GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, ARCH_HELP, GetPString(IDS_ARCCMDHELPTEXT));
      break;

    case ARCH_MASKS:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ARCH_HELP,
			  GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, ARCH_HELP, GetPString(IDS_ARCMASKHELPTEXT));

      break;

    case ARCH_INCLPATH:
    case ARCH_RECURSE:
    case ARCH_MOVE:
      {
	BOOL fRecurse = FALSE, fMove = FALSE, fInclDirs = FALSE;
	CHAR *cmd;

	if ((BOOL) WinSendDlgItemMsg(hwnd, ARCH_RECURSE, BM_QUERYCHECK,
				     MPVOID, MPVOID)) {
	  fRecurse = TRUE;
	  if (SHORT1FROMMP(mp1) == ARCH_RECURSE) {
	    if (arcdata->info->createrecurse && *arcdata->info->createrecurse) {
	      fMove = FALSE;
	      fInclDirs = FALSE;
	      WinSendDlgItemMsg(hwnd, ARCH_INCLPATH, BM_SETCHECK,
				MPVOID, MPVOID);
	      WinSendDlgItemMsg(hwnd, ARCH_MOVE, BM_SETCHECK, MPVOID, MPVOID);
	    }
	    else {
	      fRecurse = FALSE;
	      WinSendDlgItemMsg(hwnd, ARCH_RECURSE, BM_SETCHECK,
				MPVOID, MPVOID);
	    }
	  }
	}
	else
	  fRecurse = FALSE;
	if ((BOOL) WinSendDlgItemMsg(hwnd, ARCH_MOVE, BM_QUERYCHECK,
				     MPVOID, MPVOID)) {
	  fMove = TRUE;
	  fRecurse = FALSE;
	  WinSendDlgItemMsg(hwnd, ARCH_RECURSE, BM_SETCHECK, MPVOID, MPVOID);
	}
	else
	  fMove = FALSE;
	if ((BOOL) WinSendDlgItemMsg(hwnd, ARCH_INCLPATH, BM_QUERYCHECK,
				     MPVOID, MPVOID)) {
	  fInclDirs = TRUE;
	  fRecurse = FALSE;
	  WinSendDlgItemMsg(hwnd, ARCH_RECURSE, BM_SETCHECK, MPVOID, MPVOID);
	}
	else
	  fInclDirs = FALSE;
	cmd = NULL;
	if (fMove) {
	  if (fInclDirs || fRecurse)
	    cmd = arcdata->info->movewdirs;
	  else
	    cmd = arcdata->info->move;
	  if (!cmd || !*cmd)
	    cmd = arcdata->info->move;
	}
	if (!cmd || !*cmd) {
	  if (fInclDirs) {
	    cmd = arcdata->info->createwdirs;
	    if (!cmd || !*cmd)
	      cmd = arcdata->info->createrecurse;
	  }
	  else if (fRecurse) {
	    cmd = arcdata->info->createrecurse;
	    if (!cmd || !*cmd)
	      cmd = arcdata->info->createwdirs;
	  }
	  if (!cmd || !*cmd)
	    cmd = arcdata->info->create;
	}
	if (cmd)
	  WinSetDlgItemText(hwnd, ARCH_COMMAND, cmd);
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    arcdata = (DIRCNRDATA *) WinQueryWindowPtr(hwnd, QWL_USER);
    if (!arcdata) {
      Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      return 0;
    }
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    case DID_OK:
      {
	CHAR s[CCHMAXPATH + 1];

	*s = 0;
	WinQueryDlgItemText(hwnd, ARCH_ARCNAME, CCHMAXPATH, s);
	bstrip(s);
	if (*s) {
	  if (DosQueryPathInfo(s,
			       FIL_QUERYFULLNAME,
			       arcdata->arcname, CCHMAXPATH))
	    strcpy(arcdata->arcname, s);
	  *s = 0;
	  WinQueryDlgItemText(hwnd, ARCH_COMMAND, 256, s);
	  if (*s) {
	    strcpy(arcdata->command, s);
	    *s = 0;
	    WinQueryDlgItemText(hwnd, ARCH_MASKS, 256, s);
	    *arcdata->mask.szMask = 0;
	    strcpy(arcdata->mask.szMask, s);
	    WinDismissDlg(hwnd, 1);
	    break;
	  }
	}
      }
      DosBeep(50, 100);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_ARCHIVE, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case ARCH_FIND:
      if (arcdata->namecanchange) {

	CHAR arcname[CCHMAXPATH], s[CCHMAXPATH], *p;

	*s = 0;
	WinQueryDlgItemText(hwnd, ARCH_ARCNAME, CCHMAXPATH, s);
	bstrip(s);
	if (DosQueryPathInfo(s, FIL_QUERYFULLNAME, arcname, CCHMAXPATH))
	  strcpy(arcname, s);
	p = strrchr(arcname, '\\');
	if (p) {
	  p++;
	  p = strrchr(arcname, '.');
	}
	if (!p && arcdata->info->ext && *arcdata->info->ext) {
	  strcat(arcname, "*.");
	  strcat(arcname, arcdata->info->ext);
	}
	if (export_filename(hwnd, arcname, FALSE))
	  WinSetDlgItemText(hwnd, ARCH_ARCNAME, arcname);
      }
      break;

    case ARCH_SEE:
      {
	CHAR s[1001], *p;

	*s = 0;
	WinQueryDlgItemText(hwnd, ARCH_COMMAND, 256, s);
	bstrip(s);
	if (*s) {
	  p = strchr(s, ' ');
	  if (p)
	    *p = 0;
	  ExecOnList(hwnd, s, WINDOWED | SEPARATEKEEP | MAXIMIZED | PROMPT,
		     NULL, NULL, GetPString(IDS_ARCEXECHELPTEXT));
	}
	else
	  DosBeep(50, 100);
      }
      break;
    }
    return 0;

  case WM_CLOSE:
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
