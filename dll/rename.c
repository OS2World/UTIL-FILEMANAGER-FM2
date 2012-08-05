
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2010 Steven H.Levine

  Revisions
  01 Aug 04 SHL - Rework lstrip/rstrip usage
  22 Mar 07 GKY Use QWL_USER
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  27 Sep 07 SHL Correct ULONGLONG size formatting
  30 Dec 07 GKY Use TestFDates for comparing dates
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  23 Oct 10 GKY Add ForwardslashToBackslash function to streamline code
  04 Aug 12 GKY Changes to allow copy and move over readonly files with a warning dialog; also added a warning dialog
                for delete of readonly files

***********************************************************************/

#include <string.h>
#include <ctype.h>

#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "worker.h"			// typedef MOVEIT
#include "rename.h"
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "copyf.h"			// AdjustWildcardName
#include "valid.h"			// TestFDates
#include "mkdir.h"			// SetDir
#include "i18nutil.h"			// CommaFmtULL
#include "strips.h"			// bstrip
#include "info.h"                       // driveflags
#include "wrappers.h"                   // xfopen
#include "pathutil.h"                   // ForwardslashToBackslash

static PSZ pszSrcFile = __FILE__;

MRESULT EXPENTRY RenameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  MOVEIT *mv;

  switch (msg) {
  case WM_INITDLG:
    mv = (MOVEIT *) mp2;
    WinSetWindowPtr(hwnd, QWL_USER, (PVOID) mv);
    if (!mv || !mv->source) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    WinSendDlgItemMsg(hwnd,
		      REN_SOURCE,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd,
		      REN_TARGET,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    if (!*mv->target)
      strcpy(mv->target, mv->source);
    WinSendMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    if (mv->rename || !stricmp(mv->target, mv->source)) {

      CHAR *p = strrchr(mv->target, '\\');
      if (p) {

	USHORT sello, selhi;

	sello = p - mv->target + 1;
	selhi = strlen(mv->target);

	WinSendDlgItemMsg(hwnd,
			  REN_TARGET,
			  EM_SETSEL, MPFROM2SHORT(sello, selhi), MPVOID);
      }
      WinShowWindow(WinWindowFromID(hwnd, REN_OVEROLD), FALSE);
      WinShowWindow(WinWindowFromID(hwnd, REN_OVERNEW), FALSE);
      WinShowWindow(WinWindowFromID(hwnd, REN_NOROWARN), FALSE);
    }
    break;

  case UM_RESCAN:
    {
      mv = WinQueryWindowPtr(hwnd, QWL_USER);
      if (mv) {

	FILESTATUS3L fs1, fs2;
	CHAR s[CCHMAXPATH * 2], *p, chkname[CCHMAXPATH], szCmmaFmtFileSize[81], szDate[DATE_BUF_BYTES];
	INT sourceexists = 0, targetexists = 0,
	    sourcenewer = 0, sourcesmaller = 0;
        ForwardslashToBackslash(mv->target);
	if (!MakeFullName(mv->target))
	  WinSetDlgItemText(hwnd, REN_TARGET, mv->target);
	if (!MakeFullName(mv->source))
	  WinSetDlgItemText(hwnd, REN_SOURCE, mv->source);
	if (!DosQueryPathInfo(mv->source, FIL_STANDARDL, &fs1, sizeof(fs1))) {
          CommaFmtULL(szCmmaFmtFileSize,
                      sizeof(szCmmaFmtFileSize), fs1.cbFile, ' ');
          FDateFormat(szDate, fs1.fdateLastWrite);
	  sprintf(s, " %s%s %ss %s %02u%s%02u%s%02u",
		  fs1.attrFile & FILE_DIRECTORY ?
		    GetPString(IDS_DIRBRKTTEXT) : NullStr,
		  szCmmaFmtFileSize,
		  GetPString(IDS_BYTETEXT),
		  szDate,
		  fs1.ftimeLastWrite.hours, TimeSeparator,
		  fs1.ftimeLastWrite.minutes, TimeSeparator, fs1.ftimeLastWrite.twosecs * 2);
	  WinSetDlgItemText(hwnd, REN_SOURCEINFO, s);
	  sourceexists = 1;
	  if (fs1.attrFile & FILE_DIRECTORY)
	    sourceexists = 3;
	}
	else
	  WinSetDlgItemText(hwnd,
			    REN_SOURCEINFO, (CHAR *) GetPString(IDS_DOESNTEXIST2TEXT));
	strcpy(chkname, mv->target);
	p = strrchr(s, '\\');
	if (p && (strchr(p, '*') || strchr(p, '?'))) {
	  if (!AdjustWildcardName(mv->target, chkname))
	    strcpy(chkname, mv->target);
	}
	if (!DosQueryPathInfo(chkname, FIL_STANDARDL, &fs2, sizeof(fs2))) {
          CommaFmtULL(szCmmaFmtFileSize,
                      sizeof(szCmmaFmtFileSize), fs2.cbFile, ' ');
          FDateFormat(szDate, fs2.fdateLastWrite);
	  sprintf(s, " %s%s %ss %s %02u%s%02u%s%02u",
		  fs2.attrFile & FILE_DIRECTORY ?
		    GetPString(IDS_DIRBRKTTEXT) : NullStr,
		  szCmmaFmtFileSize,
		  GetPString(IDS_BYTETEXT),
		  szDate,
		  fs2.ftimeLastWrite.hours, TimeSeparator,
		  fs2.ftimeLastWrite.minutes, TimeSeparator, fs2.ftimeLastWrite.twosecs * 2);
	  WinSetDlgItemText(hwnd, REN_TARGETINFO, s);
	  targetexists = 1;
	  if (fs2.attrFile & (FILE_DIRECTORY))
	    targetexists = 3;
	  WinEnableWindow(WinWindowFromID(hwnd, REN_RENEXIST), TRUE);
	}
	else {
	  WinSetDlgItemText(hwnd,
			    REN_TARGETINFO, (CHAR *) GetPString(IDS_DOESNTEXIST2TEXT));
	  WinEnableWindow(WinWindowFromID(hwnd, REN_RENEXIST), FALSE);
	}
	*s = 0;
	if (sourceexists)
	  sprintf(s,
		  GetPString(IDS_SOURCEISATEXT),
		  sourceexists & 2 ? GetPString(IDS_DIRECTORYTEXT) :
				     GetPString(IDS_FILETEXT));
	{
          FILE *fp = NULL;
          CHAR *modeab = "ab";
	  if (~sourceexists & 2)
	    fp = xfopen(mv->source, modeab, pszSrcFile, __LINE__, TRUE);
	  if ((!fp && ~sourceexists & 2) || !sourceexists)
	    strcpy(s, GetPString(IDS_CANTACCESSSOURCETEXT));
	  if (fp)
	    fclose(fp);
	}
	if (targetexists && stricmp(mv->source, mv->target))
	  sprintf(&s[strlen(s)],
		  GetPString(IDS_TARGETEXISTSISATEXT),
		  targetexists & 2 ? GetPString(IDS_DIRECTORYTEXT) :
				     GetPString(IDS_FILETEXT));
	if (targetexists && stricmp(mv->source, mv->target))
	  strcpy(&s[strlen(s)], GetPString(IDS_CLICKOVERWRITETEXT));
	else if (targetexists && !stricmp(mv->source, mv->target))
	  strcpy(&s[strlen(s)], GetPString(IDS_ENTERNEWTARGETTEXT));
	WinEnableWindow(WinWindowFromID(hwnd, REN_OVERWRITE),
			stricmp(mv->target, mv->source) &&
			  (!mv->rename || strcmp(mv->target, mv->source)));

	if (targetexists == 1 && sourceexists == 1) {
          sourcenewer = TestFDates(NULL, NULL,
                                   &fs1.fdateLastWrite, &fs1.ftimeLastWrite,
                                   &fs2.fdateLastWrite, &fs2.ftimeLastWrite);
	  sourcesmaller = (fs1.cbFile < fs2.cbFile) ? -1 :
			  (fs1.cbFile > fs2.cbFile) ? 1 : 0;
	  sprintf(&s[strlen(s)], GetPString(IDS_SOURCEISTEXT),
		  (sourcenewer == -1) ? GetPString(IDS_NEWERTEXT) :
		    (sourcenewer == 1) ? GetPString(IDS_OLDERTEXT) :
		      GetPString(IDS_SAMEDATETEXT),
		  (sourcesmaller == -1) ? GetPString(IDS_SMALLERTEXT) :
		    (sourcesmaller == 1) ? GetPString(IDS_LARGERTEXT) :
		      GetPString(IDS_SAMESIZETEXT));
	}
	WinSetDlgItemText(hwnd, REN_INFORMATION, s);
	if (targetexists && stricmp(mv->source, mv->target)) {
	  if (WinQueryButtonCheckstate(hwnd, REN_DONTASK))
	    return 0;
	  return MRFROM2SHORT(1, 0);
	}
	else if (targetexists && !stricmp(mv->source, mv->target)) {
	  if (mv->rename && strcmp(mv->source, mv->target))
	    return 0;
	  WinEnableWindow(WinWindowFromID(hwnd, REN_RENEXIST), FALSE);
	  return MRFROM2SHORT(2, 0);
	}
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_RENAME, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case REN_SKIP:
      mv = WinQueryWindowPtr(hwnd, QWL_USER);
      if (mv) {
	mv->skip = TRUE;
	*mv->target = 0;
	WinDismissDlg(hwnd, 2);
      }
      else
	WinDismissDlg(hwnd, 0);
      break;

    case REN_RENEXIST:
      mv = WinQueryWindowPtr(hwnd, QWL_USER);
      if (mv) {

	CHAR newexist[CCHMAXPATH], fullname[CCHMAXPATH];
	INT was;
        APIRET rc;
        BOOL fResetVerify = FALSE;

	*newexist = 0;
	WinQueryDlgItemText(hwnd, REN_TARGET, CCHMAXPATH, newexist);
	if (*newexist) {
	  if (DosQueryPathInfo(newexist,
			       FIL_QUERYFULLNAME, fullname, sizeof(fullname)))
	    strcpy(fullname, newexist);
	  was = IsFile(fullname);
          if (was == -1) {
            if (fVerify && (driveflags[toupper(*mv->target) - 'A'] & DRIVE_WRITEVERIFYOFF ||
                              driveflags[toupper(*fullname) - 'A'] & DRIVE_WRITEVERIFYOFF)) {
              DosSetVerify(FALSE);
              fResetVerify = TRUE;
            }
            rc = docopyf(MOVE, mv->target, fullname);
            if (fResetVerify) {
              DosSetVerify(fVerify);
              fResetVerify = FALSE;
            }
	    if (rc) {
	      if ((LONG) rc > 0)
		Dos_Error(MB_CANCEL,
			  rc,
			  hwnd,
			  __FILE__,
			  __LINE__,
			  GetPString(IDS_COMPMOVEFAILEDTEXT),
			  mv->target, fullname);
	      else
		saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
		       hwnd,
		       GetPString(IDS_SORRYTEXT),
		       GetPString(IDS_COMPMOVEFAILEDTEXT),
		       mv->target, fullname);
	    }
	    else
	      saymsg(MB_ENTER,
		     hwnd,
		     GetPString(IDS_SUCCESSTEXT),
		     GetPString(IDS_WASMOVEDTOTEXT), mv->target, fullname);
	  }
	  else
	    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
		   hwnd,
		   GetPString(IDS_SORRYTEXT),
		   GetPString(IDS_EXISTSASATEXT),
		   fullname,
		   (was) ?
		   GetPString(IDS_FILETEXT) : GetPString(IDS_DIRECTORYTEXT));
	}
	WinSetDlgItemText(hwnd, REN_TARGET, mv->target);
      }
      break;

    case REN_OVERWRITE:
    case DID_OK:
      mv = WinQueryWindowPtr(hwnd, QWL_USER);
      if (mv) {

	MRESULT mr;

	if (WinQueryButtonCheckstate(hwnd, REN_DONTASK))
	  mv->dontask = TRUE;
	if (WinQueryButtonCheckstate(hwnd, REN_OVEROLD))
	  mv->overold = TRUE;
	if (WinQueryButtonCheckstate(hwnd, REN_OVERNEW))
          mv->overnew = TRUE;
        if (WinQueryButtonCheckstate(hwnd,REN_NOROWARN))
          mv->noreadonlywarn = TRUE;
	*mv->target = 0;
	WinQueryDlgItemText(hwnd, REN_TARGET, CCHMAXPATH, mv->target);
	bstrip(mv->target);
	mr = WinSendMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	if (!mr ||
	    (SHORT1FROMMR(mr) != 2 && SHORT1FROMMP(mp1) == REN_OVERWRITE)) {

	  CHAR path[CCHMAXPATH], *p;

	  mv->overwrite = (SHORT1FROMMP(mp1) == REN_OVERWRITE);
	  strcpy(path, mv->target);
	  p = strrchr(path, '\\');
	  if (p) {
	    p++;
	    *p = 0;
	    if (SetDir(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				      QW_OWNER), hwnd, path, 0)) {
              if (!fAlertBeepOff)
	        DosBeep(250, 100);
	      WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, REN_TARGET));
	      break;
	    }
	  }
	  WinDismissDlg(hwnd, 1);
	}
	else {
          if (!fAlertBeepOff)
	    DosBeep(250, 100);
	  WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, REN_TARGET));
	}
      }
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(FMRENAME,RenameProc)
