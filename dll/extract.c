
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2013 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  05 Jun 05 SHL Use QWL_USER
  17 Jul 06 SHL Use Runtime_Error
  20 Dec 06 GKY Added checkbox to make default extract with directories
  22 Mar 07 GKY Use QWL_USER
  19 Apr 07 SHL Sync with AcceptOneDrop GetOneDrop mods
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory
  29 Nov 08 GKY Add the option of creating a subdirectory from the arcname
                for the extract path.
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  17 Jan 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  23 Oct 10 GKY Changes to populate and utilize a HELPTABLE for context specific help
  11 Aug 13 GKY Fix directory create failure on extract to directory based on archive name
                if the name needed quoting.
  15 Feb 14 GKY Assure the title is blank on the execute dialog call with the "see" button

***********************************************************************/

#include <string.h>
#include <ctype.h>

#define INCL_WIN
#define INCL_DOS
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "init.h"			// Data declaration(s)
#include "arccnrs.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "cmdline.h"			// CmdLineDlgProc
#include "extract.h"
#include "walkem.h"			// WalkExtractDlgProc
#include "droplist.h"			// AcceptOneDrop, DropHelp, GetOneDrop
#include "misc.h"			// DrawTargetEmphasis
#include "chklist.h"			// PosOverOkay
#include "mkdir.h"			// SetDir
#include "valid.h"			// MakeValidDir
#include "systemf.h"			// runemf2
#include "dirs.h"			// save_dir2
#include "strips.h"			// bstrip

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

MRESULT EXPENTRY ExtractTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
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
    DropHelp(mp1, mp2, hwnd, GetPString(IDS_EXTDROPHELPTEXT));
    return 0;

  case DM_DROP:
    {
      char szFrom[CCHMAXPATH + 2];

      if (emphasized) {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
      if (GetOneDrop(hwnd, mp1, mp2, szFrom, sizeof(szFrom)))
	WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_COMMAND,
		   MPFROM2SHORT(IDM_SWITCH, 0), MPFROMP(szFrom));
    }
    return 0;
  }
  return (oldproc) ? oldproc(hwnd, msg, mp1, mp2) :
    WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ExtractDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  EXTRDATA *arcdata = NULL;
  ULONG size;
  BOOL fFileNameExtPath;

  switch (msg) {
  case WM_INITDLG:
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    arcdata = (EXTRDATA *) mp2;
    {
      BOOL fDirectory = FALSE;
      BOOL fRemember = FALSE;
      PFNWP oldproc;

      fFileNameExtPath = FALSE;
      oldproc = WinSubclassWindow(WinWindowFromID(hwnd, EXT_DIRECTORY),
				  (PFNWP) ExtractTextProc);
      if (oldproc)
	WinSetWindowPtr(WinWindowFromID(hwnd, EXT_DIRECTORY),
                        QWL_USER, (PVOID) oldproc);
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, FM3Str, "RememberExt",
                          (PVOID) & fRemember, &size);
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, FM3Str, "DirectoryExt",
                          (PVOID) & fDirectory, &size);
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, FM3Str, "FileNamePathExt",
                          (PVOID) & fFileNameExtPath, &size);
      WinCheckButton(hwnd, EXT_REMEMBER, fRemember);
      WinCheckButton(hwnd, EXT_AWDIRS, fDirectory);
      WinCheckButton(hwnd, EXT_FILENAMEEXT, fFileNameExtPath);
      WinSendDlgItemMsg(hwnd, EXT_DIRECTORY, EM_SETTEXTLIMIT,
			MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, EXT_COMMAND, EM_SETTEXTLIMIT,
			MPFROM2SHORT(256, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, EXT_MASK, EM_SETTEXTLIMIT,
			MPFROM2SHORT(256, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, EXT_FILENAME, EM_SETTEXTLIMIT,
			MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
      if (arcdata->arcname && *arcdata->arcname)
	WinSetDlgItemText(hwnd, EXT_FILENAME, arcdata->arcname);
      else
        WinSetDlgItemText(hwnd, EXT_FILENAME, (CHAR *) GetPString(IDS_EXTVARIOUSTEXT));

      if (fFileNameExtPath && arcdata->arcname) {

        CHAR FileName[CCHMAXPATH];
        PSZ p;

        strcpy(FileName, arcdata->arcname);
        p = strrchr(FileName, '.');
        if (p) {
          *p = 0;
          if (strchr(FileName, '\"'))
            memmove(FileName, FileName + 1, strlen(FileName) + 1);
        }
        else {
          p = FileName + strlen(arcdata->arcname);
          p--;
          *p = 0;
          if (strchr(FileName, '\"'))
            memmove(FileName , FileName + 1, strlen(FileName) + 1);
        }
        strcpy(arcdata->extractdir, FileName);
        WinSetDlgItemText(hwnd, EXT_DIRECTORY, arcdata->extractdir);
      }
      if (fDirectory) {
	WinSendDlgItemMsg(hwnd, EXT_WDIRS, BM_SETCHECK,
			  MPFROM2SHORT(TRUE, 0), MPVOID);
	WinSetDlgItemText(hwnd, EXT_COMMAND, arcdata->info->exwdirs);
      }
      else {
	WinSendDlgItemMsg(hwnd, EXT_NORMAL, BM_SETCHECK,
			  MPFROM2SHORT(TRUE, 0), MPVOID);
	WinSetDlgItemText(hwnd, EXT_COMMAND, arcdata->info->extract);

      }
      if (fRemember) {

        CHAR textdir[CCHMAXPATH];

	PrfQueryProfileString(fmprof, FM3Str, "Ext_ExtractDir", NULL, textdir, sizeof(textdir));
	if (*textdir && !IsFile(textdir))
	  strcpy(arcdata->extractdir, textdir);
	PrfQueryProfileString(fmprof, FM3Str, "Ext_Mask", NULL, textdir, sizeof(textdir));
	WinSetDlgItemText(hwnd, EXT_MASK, textdir);
      }
      if (*extractpath && (!fRemember || !*arcdata->extractdir)) {
	if (arcdata->arcname && *arcdata->arcname &&
	    !strcmp(extractpath, "*")) {

	  CHAR *p;

	  strcpy(arcdata->extractdir, arcdata->arcname);
	  p = strrchr(arcdata->extractdir, '\\');
	  if (p) {
	    if (p < arcdata->extractdir + 3)
	      p++;
	    *p = 0;
	  }
	}
	else
	  strcpy(arcdata->extractdir, extractpath);
      }
      if (!*arcdata->extractdir) {
	if (*lastextractpath)
	  strcpy(arcdata->extractdir, lastextractpath);
	else if (arcdata->arcname && *arcdata->arcname) {

	  CHAR *p;

	  strcpy(arcdata->extractdir, arcdata->arcname);
	  p = strrchr(arcdata->extractdir, '\\');
	  if (p) {
	    if (p < arcdata->extractdir + 3)
	      p++;
	    *p = 0;
	  }
	}
	if (!*arcdata->extractdir)
	  strcpy(arcdata->extractdir, pFM2SaveDirectory);
      }
      WinSetDlgItemText(hwnd, EXT_DIRECTORY, arcdata->extractdir);
      if (!arcdata->info->exwdirs)
	WinEnableWindow(WinWindowFromID(hwnd, EXT_WDIRS), FALSE);
      else if (fRemember) {
        fRemember = FALSE;
        size = sizeof(BOOL);
	PrfQueryProfileData(fmprof, FM3Str, "Ext_WDirs",
			    (PVOID) &fRemember, &size);
	if (fRemember)
	  PostMsg(WinWindowFromID(hwnd, EXT_WDIRS), BM_CLICK, MPVOID, MPVOID);
      }
    }
    *arcdata->command = 0;
    PosOverOkay(hwnd);
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, EXT_HELP), (HPS) 0, FALSE,
			TRUE);
    return 0;

  case WM_CONTROL:
    arcdata = (EXTRDATA *) WinQueryWindowPtr(hwnd, QWL_USER);
    switch (SHORT1FROMMP(mp1)) {
    case EXT_REMEMBER:
      {
	BOOL fRemember = WinQueryButtonCheckstate(hwnd, EXT_REMEMBER);
        size = sizeof(BOOL);
	PrfWriteProfileData(fmprof, FM3Str, "RememberExt",
                            (PVOID) &fRemember, size);
        WinSendDlgItemMsg(hwnd, EXT_FILENAMEEXT, BM_SETCHECK,
			    MPFROM2SHORT(FALSE, 0), MPVOID);
      }
      break;

    case EXT_AWDIRS:
      {
	BOOL fDirectory = WinQueryButtonCheckstate(hwnd, EXT_AWDIRS);
        size = sizeof(BOOL);
	PrfWriteProfileData(fmprof, FM3Str, "DirectoryExt",
			    (PVOID) &fDirectory, size);

	if (fDirectory) {
	  WinSendDlgItemMsg(hwnd, EXT_WDIRS, BM_SETCHECK,
			    MPFROM2SHORT(TRUE, 0), MPVOID);
	  WinSetDlgItemText(hwnd, EXT_COMMAND, arcdata->info->exwdirs);
	}
	else {
	  WinSendDlgItemMsg(hwnd, EXT_NORMAL, BM_SETCHECK,
			    MPFROM2SHORT(TRUE, 0), MPVOID);
	  WinSetDlgItemText(hwnd, EXT_COMMAND, arcdata->info->extract);
	}
      }
      break;

    case EXT_FILENAMEEXT:
      {
        BOOL fFileNameExtPath = WinQueryButtonCheckstate(hwnd, EXT_FILENAMEEXT);
        BOOL fRemember = WinQueryButtonCheckstate(hwnd, EXT_REMEMBER);
        size = sizeof(BOOL);
	PrfWriteProfileData(fmprof, FM3Str, "FileNamePathExt",
                            fRemember ? FALSE : (PVOID) &fFileNameExtPath, size);
        if (fRemember) {
          WinSendDlgItemMsg(hwnd, EXT_FILENAMEEXT, BM_SETCHECK,
			    MPFROM2SHORT(FALSE, 0), MPVOID);
          break;
        }
        if (fFileNameExtPath && arcdata->arcname) {
          CHAR FileName[CCHMAXPATH];
          PSZ p;

          strcpy(FileName, arcdata->arcname);
          p = strrchr(FileName, '.');
          if (p) {
            *p = 0;
            if (strchr(FileName, '\"'))
            memmove(FileName, FileName + 1, strlen(FileName) + 1);
          }
          else {
            p = FileName + strlen(arcdata->arcname);
            p--;
            *p = 0;
            if (strchr(FileName, '\"'))
            memmove(FileName, FileName + 1, strlen(FileName) + 1);
          }
          strcpy(arcdata->extractdir, FileName);
          WinSetDlgItemText(hwnd, EXT_DIRECTORY, arcdata->extractdir);
        }
        else {
          *arcdata->extractdir = 0;
          if (*extractpath) {
            if (arcdata->arcname && *arcdata->arcname &&
                !strcmp(extractpath, "*")) {

              CHAR *p;

              strcpy(arcdata->extractdir, arcdata->arcname);
              p = strrchr(arcdata->extractdir, '\\');
              if (p) {
                if (p < arcdata->extractdir + 3)
                  p++;
                *p = 0;
              }
            }
            else
              strcpy(arcdata->extractdir, extractpath);
          }
          if (!*arcdata->extractdir) {
            if (arcdata->arcname && *arcdata->arcname) {

              CHAR *p;

              strcpy(arcdata->extractdir, arcdata->arcname);
              p = strrchr(arcdata->extractdir, '\\');
              if (p) {
                if (p < arcdata->extractdir + 3)
                  p++;
                *p = 0;
              }
            }
            if (!*arcdata->extractdir)
              strcpy(arcdata->extractdir, pFM2SaveDirectory);
          }
          WinSetDlgItemText(hwnd, EXT_DIRECTORY, arcdata->extractdir);
        }
      }
      break;

    case EXT_FILENAME:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, EXT_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, EXT_HELP, (CHAR *) GetPString(IDS_ARCARCNAMEHELPTEXT));
      break;

    case EXT_DIRECTORY:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, EXT_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, EXT_HELP, (CHAR *) GetPString(IDS_EXTEXTRACTDIRHELPTEXT));
      break;

    case EXT_COMMAND:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, EXT_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, EXT_HELP, (CHAR *) GetPString(IDS_ARCCMDHELPTEXT));
      break;

    case EXT_MASK:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, EXT_HELP, (CHAR *) GetPString(IDS_ARCDEFAULTHELPTEXT));
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, EXT_HELP, (CHAR *) GetPString(IDS_ARCMASKHELPTEXT));
      break;

    case EXT_NORMAL:
      if ((BOOL) WinSendDlgItemMsg(hwnd, EXT_NORMAL, BM_QUERYCHECK,
				   MPVOID, MPVOID))
	WinSetDlgItemText(hwnd, EXT_COMMAND, arcdata->info->extract);
      break;

    case EXT_WDIRS:
      if (arcdata->info->exwdirs) {
	if ((BOOL) WinSendDlgItemMsg(hwnd, EXT_WDIRS, BM_QUERYCHECK,
				     MPVOID, MPVOID))
	  WinSetDlgItemText(hwnd, EXT_COMMAND, arcdata->info->exwdirs);
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    arcdata = (EXTRDATA *) WinQueryWindowPtr(hwnd, QWL_USER);
    switch (SHORT1FROMMP(mp1)) {
    case IDM_SWITCH:
      if (mp2) {

	CHAR tdir[CCHMAXPATH];

	strcpy(tdir, (CHAR *) mp2);
	MakeValidDir(tdir);
	WinSetDlgItemText(hwnd, EXT_DIRECTORY, tdir);
      }
      break;

    case DID_CANCEL:
      arcdata->ret = 0;
      WinDismissDlg(hwnd, 0);
      break;
    case DID_OK:
      {
	CHAR s[CCHMAXPATH + 1];
	BOOL fRemember;

	fRemember = WinQueryButtonCheckstate(hwnd, EXT_REMEMBER);
	*s = 0;
	WinQueryDlgItemText(hwnd, EXT_DIRECTORY, CCHMAXPATH, s);
	bstrip(s);
	if (*s) {
	  if (!SetDir(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
                                     QW_OWNER), hwnd, s, fFileNameExtPath ? 1:0)) {
	    strcpy(arcdata->extractdir, s);
	    WinSetDlgItemText(hwnd, EXT_DIRECTORY, s);
	    if ((!isalpha(*s) || s[1] != ':') && *s != '.')
	      saymsg(MB_ENTER, hwnd,
		     GetPString(IDS_WARNINGTEXT),
		     GetPString(IDS_SPECIFYDRIVETEXT));
	  }
	  else
	    break;
	  strcpy(lastextractpath, s);
	  if (fRemember) {
	    PrfWriteProfileString(fmprof, FM3Str, "Ext_ExtractDir", s);
            fRemember = WinQueryButtonCheckstate(hwnd, EXT_WDIRS);
            size = sizeof(BOOL);
	    PrfWriteProfileData(fmprof, FM3Str, "Ext_WDirs",
				(PVOID) &fRemember, size);
	    fRemember = TRUE;
	  }
	  *s = 0;
	  WinQueryDlgItemText(hwnd, EXT_COMMAND, 256, s);
	  if (*s) {
	    strcpy(arcdata->command, s);
	    *s = 0;
	    WinQueryDlgItemText(hwnd, EXT_MASK, 256, s);
	    *arcdata->masks = 0;
	    strcpy(arcdata->masks, s);
	    if (fRemember)
	      PrfWriteProfileString(fmprof, FM3Str, "Ext_Mask", s);
	    arcdata->ret = 1;
	    WinDismissDlg(hwnd, 1);
	    break;
	  }
	}
      }
      if (!fErrorBeepOff)
        DosBeep(50, 100);			// Complain a refuse to quit
      break;

    case IDM_HELP:
    case WM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_EXTRACT, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case EXT_WALK:
      {
	CHAR temp[CCHMAXPATH + 1];

	strcpy(temp, arcdata->extractdir);
	if (WinDlgBox(HWND_DESKTOP, WinQueryWindow(WinQueryWindow(hwnd,
								  QW_PARENT),
						   QW_OWNER),
		      WalkExtractDlgProc, FM3ModHandle, WALK_FRAME,
		      (PVOID) temp)) {
	  if (*temp && stricmp(temp, arcdata->extractdir)) {
	    strcpy(arcdata->extractdir, temp);
	  }
	}
	WinSetDlgItemText(hwnd, EXT_DIRECTORY, arcdata->extractdir);
      }
      break;

    case EXT_SEE:
      {
	CHAR s[1001], *p;
	EXECARGS ex;

	WinQueryDlgItemText(hwnd, EXT_COMMAND, 256, s);
	lstrip(s);
	if (!*s)
	  Runtime_Error(pszSrcFile, __LINE__, "no command");
	else {
	  p = strchr(s, ' ');
	  if (p)
	    *p = 0;			// Drop options
	  memset(&ex, 0, sizeof(EXECARGS));
	  ex.commandline = s;
	  ex.flags = WINDOWED | SEPARATEKEEP | MAXIMIZED;
	  *ex.path = 0;
          *ex.environment = 0;
          *ex.title = 0;
	  if (WinDlgBox(HWND_DESKTOP,
			hwnd,
			CmdLineDlgProc,
			FM3ModHandle, EXEC_FRAME, MPFROMP(&ex)) && *s) {
	    runemf2(ex.flags,
		    hwnd, pszSrcFile, __LINE__,
		    NULL, (*ex.environment) ? ex.environment : NULL, "%s", s);
	  }
	}
      }
      break;
    }
    return 0;

  case WM_CLOSE:
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(FMEXTRACT,ExtractTextProc,ExtractDlgProc)
