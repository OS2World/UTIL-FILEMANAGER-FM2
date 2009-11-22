
/***********************************************************************

  $Id$

  Instant command

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2008 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  14 Jul 06 SHL Use Runtime_Error
  22 Mar 07 GKY Use QWL_USER
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  12 Jul 09 GKY Add xDosQueryAppType and xDosAlloc... to allow FM/2 to load in high memory

***********************************************************************/

#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "instant.h"
#include "misc.h"			// GetCmdSpec
#include "systemf.h"			// runemf2
#include "strips.h"			// bstrip
#include "wrappers.h"                   // xDosAllocMem
#include "init.h"                       // Strings

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

#define hwndMLE            WinWindowFromID(hwnd,BAT_MLE)

static INT batches = 0;

MRESULT EXPENTRY InstantDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CHAR *path;
  APIRET rc;
  static CHAR *bat = NULL;
  static HWND myhwnd = (HWND) 0;

  switch (msg) {
  case WM_INITDLG:
    if (myhwnd) {
      Runtime_Error(pszSrcFile, __LINE__, "busy");
      WinSendMsg(myhwnd, WM_SYSCOMMAND, MPFROM2SHORT(SC_RESTORE, 0), MPVOID);
      WinSetActiveWindow(HWND_DESKTOP, myhwnd);
      WinDismissDlg(hwnd, 0);
      break;
    }
    if (!mp2) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      WinDismissDlg(hwnd, 0);
      break;
    }
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    path = (CHAR *) mp2;
    {
      CHAR s[CCHMAXPATH + 81];

      sprintf(s, GetPString(IDS_INSTANTTITLETEXT), path);
      WinSetWindowText(hwnd, s);
    }
    WinSendMsg(hwndMLE,
	       MLM_SETTEXTLIMIT, MPFROMLONG((LONG) (10240L)), MPVOID);
    WinSendMsg(hwndMLE, MLM_FORMAT, MPFROM2SHORT(MLFIE_NOTRANS, 0), MPVOID);
    if (bat) {

      ULONG tlen = strlen(bat);
      IPT iptOffset = 0L;

      WinSendMsg(hwndMLE, MLM_SETIMPORTEXPORT,
		 MPFROMP(bat), MPFROMLONG(12287L));
      WinSendMsg(hwndMLE, MLM_IMPORT, MPFROMP(&iptOffset), MPFROMP(tlen));
      DosFreeMem(bat);
      bat = NULL;
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      path = (CHAR *) WinQueryWindowPtr(hwnd, QWL_USER);
      {
	CHAR s[CCHMAXPATH + 1];
	FILE *fp;
	IPT iptOffset = 0L;
	LONG len, tlen, mem;
	CHAR *rexx = "";

	mem = MLEgetlen(hwndMLE);
	if (mem) {
	  rc = xDosAllocMem((PVOID) & bat, mem,
			    PAG_COMMIT | PAG_READ | PAG_WRITE, pszSrcFile, __LINE__);
	  if (rc || !bat) {
	    Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		      GetPString(IDS_OUTOFMEMORY));
	    WinDismissDlg(hwnd, 0);
	    break;
	  }
	  tlen =
	    (LONG) WinSendMsg(hwndMLE, MLM_QUERYTEXTLENGTH, MPVOID, MPVOID);
	  if (!tlen)
	    Runtime_Error(pszSrcFile, __LINE__, NULL);
	  else {
	    WinSendMsg(hwndMLE, MLM_SETIMPORTEXPORT,
		       MPFROMP(bat), MPFROMLONG(mem));
	    len = (LONG) WinSendMsg(hwndMLE, MLM_EXPORT,
				    MPFROMP(&iptOffset), MPFROMP(&tlen));
	    bat[len] = 0;
	    lstrip(bat);
	    while (strlen(bat) && bat[strlen(bat) - 1] == '\n' ||
		   bat[strlen(bat) - 1] == ' ') {
	      // fixme to understand
	      stripcr(bat);
	      rstrip(bat);
	      stripcr(bat);
	      rstrip(bat);
	    }
	    if (!*bat)
	      Runtime_Error(pszSrcFile, __LINE__, NULL);
	    else {
	      sprintf(s, "%s%sFMTMP%d.CMD", path,
		      (path[strlen(path) - 1] == '\\') ? NullStr : PCSZ_BACKSLASH,
		      batches++);
	      fp = fopen(s, "w");
	      if (!fp)
		Runtime_Error(pszSrcFile, __LINE__, "fopen");
	      else {
		if (!strncmp(bat, "/*", 2)) {
		  rexx = "'";
		  fprintf(fp, "%s\n", GetPString(IDS_REXXCOMMENT));
		}
		fprintf(fp, "%s%c:%s\n", rexx, toupper(*path), rexx);
		fprintf(fp, "%sCD \"%s%s\"%s\n", rexx, path,
			(strlen(path) < 3) ? PCSZ_BACKSLASH : NullStr, rexx);
		fprintf(fp, "%s", bat);
		fprintf(fp, "\n%sDEL \"%s\"%s\n", rexx, s, rexx);
		fclose(fp);
		runemf2(WINDOWED | SEPARATE,
			hwnd, pszSrcFile, __LINE__,
			path, NULL, "%s /C \"%s\"", GetCmdSpec(FALSE), s);
	      }
	    }
	  }
	}
      }
      WinDismissDlg(hwnd, 0);
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      path = WinQueryWindowPtr(hwnd, QWL_USER);
      rc = saymsg(MB_YESNOCANCEL,
		  hwnd,
		  GetPString(IDS_INSTANTHELPTITLETEXT),
		  GetPString(IDS_INSTANTHELPTEXT),
		  path, (strlen(path) < 3) ? PCSZ_BACKSLASH : NullStr, path,
		  (path[strlen(path) - 1] == '\\') ? NullStr : PCSZ_BACKSLASH, batches);
      if (rc == MBID_YES)
	runemf2(WINDOWED | INVISIBLE | BACKGROUND,
                hwnd, pszSrcFile, __LINE__, NULL, NULL,
                "%s /C HELP BATCH", GetCmdSpec(FALSE));
      else if (rc == MBID_CANCEL)
	WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;

  case WM_CLOSE:
  case WM_DESTROY:
    myhwnd = (HWND) 0;
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(INSTANT,InstantDlgProc)
