
/***********************************************************************

  $Id$

  Make directory dialog

  Copyright (c) 1993-97 M. Kimes
  Copyright (c) 2004, 2008 Steven H.Levine

  01 Aug 04 SHL Baseline
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.
  23 Oct 10 GKY Add ForwardslashToBackslash function to streamline code
  30 Dec 12 GKY Enhance traget directory drop to give the option of changing the directory or carrying out an
                operation to the current target; Added an error message for target=None;
                Added parameter to SetTargetDir

***********************************************************************/

#include <string.h>
#include <ctype.h>

#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "mkdir.h"
#include "common.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "notebook.h"                   // targetdirectoy
#include "walkem.h"			// WalkTargetDlgProc
#include "misc.h"			// Broadcast
#include "valid.h"			// IsFullName
#include "dirs.h"			// save_dir2
#include "input.h"			// InputDlgProc
#include "pathutil.h"                   // AddBackslashToPath

// Data definitions
#pragma data_seg(GLOBAL2)
CHAR targetdir[CCHMAXPATH];

//static PSZ pszSrcFile = __FILE__;

APIRET MassMkdir(HWND hwndClient, CHAR * dir)
{
  APIRET last, was = 0;
  CHAR *p;
  CHAR s[CCHMAXPATH];

  if (DosQueryPathInfo(dir, FIL_QUERYFULLNAME, s, sizeof(s)))
    strcpy(s, dir);
  ForwardslashToBackslash(s);
  p = s;
  do {
    p = strchr(p, '\\');
    if (p && p > s && *(p - 1) == ':')
      p = strchr(p + 1, '\\');
    if (p && *p) {
      *p = 0;
      was = 1;
    }
    else
      was = 0;
    last = DosCreateDir(s, NULL);
    if (!last) {
      Broadcast((HAB) 0, hwndClient, UM_UPDATERECORD, MPFROMP(s), MPVOID);
    }
    else if (last == ERROR_ACCESS_DENIED) {
      if (!IsFile(s))
	last = 0;
    }
    if (was) {
      *p = '\\';
      p++;
    }
  }
  while (p && *p);
  return last;
}

APIRET SetDir(HWND hwndClient, HWND hwnd, CHAR * dir, INT flags)
{

  /**
   * bitmapped flags:
   * 1 = don't ask to create if non-existent
   */

  CHAR s[CCHMAXPATH], *p;
  APIRET ret = 0, error;
  INT isfile;
  BOOL fResetVerify = FALSE;

  if (DosQueryPathInfo(dir, FIL_QUERYFULLNAME, s, sizeof(s)))
    strcpy(s, dir);
  while ((p = strchr(s, '/')) != NULL)
    *p = '\\';
  while (strlen(s) > 3 && s[strlen(s) - 1] == '\\')
    s[strlen(s) - 1] = 0;
  if (IsFullName(s)) {
    if (driveflags[toupper(*s) - 'A'] & (DRIVE_IGNORE | DRIVE_INVALID)) {
      if (!(flags & 1))
	saymsg(MB_CANCEL,
	       hwnd,
	       GetPString(IDS_NOPETEXT),
	       GetPString(IDS_DRIVEISTEXT),
	       toupper(*s),
	       ((driveflags[toupper(*s) - 'A'] & DRIVE_IGNORE) != 0) ?
	       GetPString(IDS_BEINGIGNOREDTEXT) :
	       GetPString(IDS_INVALIDTEXT));
      return -5;
    }
  }
  isfile = IsFile(s);
  if (isfile == -1) {
    if (IsFullName(s)) {
      if (driveflags[toupper(*s) - 'A'] & DRIVE_NOTWRITEABLE) {
	if (!(flags & 1))
	  saymsg(MB_CANCEL,
		 hwnd,
		 GetPString(IDS_NOPETEXT),
		 GetPString(IDS_DRIVEISTEXT),
		 toupper(*s), GetPString(IDS_NOWRITETEXT));
	return -4;
      }
    }
    if (!(flags & 1)) {
      if (saymsg(MB_YESNO,
		 hwnd,
		 GetPString(IDS_CONFIRMTEXT),
		 GetPString(IDS_NODIRCREATEDIRTEXT), s) != MBID_YES)
	return -3;
    }
    if (fVerify && driveflags[toupper(*s) - 'A'] & DRIVE_WRITEVERIFYOFF) {
      DosSetVerify(FALSE);
      fResetVerify = TRUE;
    }
    error = MassMkdir(hwnd, s);
    if (fResetVerify) {
      DosSetVerify(fVerify);
      fResetVerify = FALSE;
    }
    if (error) {
      Dos_Error(MB_CANCEL,
		error,
		hwnd,
		__FILE__, __LINE__, GetPString(IDS_CREATEDIRFAILEDTEXT), s);
      ret = -1;
    }
  }
  else if (isfile) {
    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
	   hwnd,
	   GetPString(IDS_ERRORTEXT), GetPString(IDS_EXISTSNOTDIRTEXT), s);
    ret = -2;
  }
  return ret;
}

BOOL PMMkDir(HWND hwnd, CHAR * filename, BOOL copy)
{

  STRINGINPARMS sip;
  CHAR szBuff[CCHMAXPATH];
  APIRET error;
  BOOL fResetVerify = FALSE;

Over:
  sip.help = GetPString(IDS_MKDIRHELPTEXT);
  sip.ret = szBuff;
  if (filename)
    strcpy(szBuff, filename);
  else
    strcpy(szBuff, pFM2SaveDirectory);
  MakeValidDir(szBuff);
  AddBackslashToPath(szBuff);
  sip.prompt = GetPString(IDS_MKDIRPROMPTTEXT);
  sip.inputlen = CCHMAXPATH - 1;
  sip.title = GetPString(IDS_MKDIRTITLETEXT);
  if (WinDlgBox(HWND_DESKTOP,
		hwnd,
		InputDlgProc, FM3ModHandle, STR_FRAME, &sip) && *szBuff) {
    if ((strchr(szBuff, '?') ||
	 strchr(szBuff, '*')) || IsFile(szBuff) == 1 || IsRoot(szBuff)) {
      saymsg(MB_ENTER | MB_ICONEXCLAMATION,
	     hwnd,
	     GetPString(IDS_ERRORTEXT),
	     GetPString(IDS_DIRNAMEERRORTEXT), szBuff);
      goto Over;
    }
    if (fVerify && driveflags[toupper(*szBuff) - 'A'] & DRIVE_WRITEVERIFYOFF) {
      DosSetVerify(FALSE);
      fResetVerify = TRUE;
    }
    error = MassMkdir(hwnd, szBuff);
    if (fResetVerify) {
      DosSetVerify(fVerify);
      fResetVerify = FALSE;
    }
    if (error)
      Dos_Error(MB_ENTER,
		error,
		hwnd,
		__FILE__,
		__LINE__, GetPString(IDS_CREATEDIRFAILEDTEXT), szBuff);
    else {
      if (copy && filename)
	strcpy(filename, szBuff);
      return TRUE;
    }
  }
  return FALSE;
}

/*
 * SetTargetDir sets or unsets the target directory
 * either from the walk dialog or a directory dropped
 * on the drivesback bar which is passed as "newtarget".
 * justshow allows you to update the target without opening
 * the walk dialog.
 */
void SetTargetDir(HWND hwnd, BOOL justshow, PSZ newtarget)
{

  char temp[CCHMAXPATH + 12];

  if (!justshow) {
    strcpy(temp, targetdir);
    if (WinDlgBox(HWND_DESKTOP,
		  hwnd,
		  WalkTargetDlgProc,
		  FM3ModHandle, WALK_FRAME, MPFROMP(temp)) && *temp) {
      strcpy(targetdir, temp);
      if (!fChangeTarget)
	PrfWriteProfileString(fmprof, appname, "Targetdir", targetdir);
    }
    else {
      if (*targetdir &&
	  saymsg(MB_YESNOCANCEL,
		 hwnd,
		 GetPString(IDS_CLEARTARGETTITLETEXT),
		 GetPString(IDS_CLEARTARGETTEXT)) == MBID_YES) {
	*targetdir = 0;
	PrfWriteProfileString(fmprof, appname, "Targetdir", NULL);
      }
    }
  }
  if (newtarget && justshow)
    strcpy(targetdir, newtarget);
  if (hwndBack) {
    if (fShowTarget)
      sprintf(temp,
	      "%s%s%s%s",
	      GetPString(IDS_TARGETDIRTITLETEXT),
	      (*targetdir) ? NullStr : "<",
	      (*targetdir) ?
	      targetdir : GetPString(IDS_NONE), (*targetdir) ? NullStr : ">");
    else
      *temp = 0;
    WinSetWindowText(hwndBack, temp);
  }
}

#pragma alloc_text(MKDIR,MassMkdir,SetDir,PMMkDir,SetTargetDir)
