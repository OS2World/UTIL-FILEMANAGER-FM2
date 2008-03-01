
/***********************************************************************

  $Id$

  Launch inf viewer

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2006 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  17 Jul 06 SHL Use Runtime_Error
  03 Nov 06 SHL Renames
  03 Nov 06 SHL Count thread usage
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Feb 08 GKY Use xfree where appropriate

***********************************************************************/

/* offset 107:  title of INF file */

#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <process.h>			// _beginthread

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "fm3dll.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

typedef struct
{
  USHORT size;
  USHORT help;
  HWND hwnd;
}
DUMMY;

static VOID FillListboxThread(VOID * args)
{
  HWND hwnd;
  DUMMY *dummy = (DUMMY *) args;
  HAB hab2;
  HMQ hmq2;
  BOOL repeating = FALSE;

  if (!dummy)
    return;
  hwnd = dummy->hwnd;
  hab2 = WinInitialize(0);
  if (hab2) {
    hmq2 = WinCreateMsgQueue(hab2, 0);
    if (hmq2) {

      CHAR *env, *p, *holdenv;

      WinCancelShutdown(hmq2, TRUE);
      IncrThreadUsage();
      priority_normal();
      if (!dummy->help)
	env = getenv("BOOKSHELF");
      else
	env = getenv("HELP");
      if (!env) {
	saymsg(MB_CANCEL,
	       hwnd,
	       GetPString(IDS_SYSERRORTEXT),
	       GetPString(IDS_ENVPATHERRORTEXT),
	       (dummy->help) ? "HELP" : "BOOKSHELF",
	       (dummy->help) ? ".HLP" : ".INF");
	goto NoEnv;
      }
      else {
	holdenv = xmalloc(strlen(env) + 2, pszSrcFile, __LINE__);
	if (holdenv) {
	  strcpy(holdenv, env);
	Repeat:
	  if (holdenv[strlen(holdenv) - 1] != ';')
	    strcat(holdenv, ";");
	  p = strtok(holdenv, ";");
	  while (p) {
	    bstrip(p);
	    if (*p) {

	      CHAR mask[CCHMAXPATH], *enddir, text[CCHMAXPATH * 2];
	      FILEFINDBUF3 ffb;
	      HDIR hdir;
	      ULONG nm;

	      strcpy(mask, p);
	      if (mask[strlen(mask) - 1] != '\\')
		strcat(mask, "\\");
	      enddir = mask + strlen(mask);
	      if (dummy->help)
		strcat(mask, "*.HLP");
	      else
		strcat(mask, "*.INF");
	      hdir = HDIR_CREATE;
	      nm = 1;
	      DosError(FERR_DISABLEHARDERR);
	      if (!DosFindFirst(mask, &hdir, FILE_NORMAL | FILE_ARCHIVED,
				&ffb, sizeof(ffb), &nm, FIL_STANDARD)) {
		do {
		  priority_normal();
		  strcpy(enddir, ffb.achName);
		  {
		    FILE *fp;
		    CHAR title[CCHMAXPATH];

		    *title = 0;
		    fp = _fsopen(mask, "rb", SH_DENYNO);
		    if (fp) {
		      fread(title, 1, 3, fp);
		      if (*title != 'H' || title[1] != 'S' || title[2] != 'P') {
			fclose(fp);
			goto Continue;
		      }
		      *title = 0;
		      fseek(fp, 107, SEEK_SET);
		      fread(title, 1, CCHMAXPATH - 2, fp);
		      title[CCHMAXPATH - 3] = 0;
		      bstrip(title);
		      fclose(fp);
		    }
		    p = strchr(ffb.achName, '.');
		    if (p)
		      *p = 0;
		    sprintf(text,
			    "%-10.10s  %-42.42s > %s",
			    ffb.achName, title, mask);
		  }
		  if (!WinIsWindow(hab2, hwnd))
		    break;
		  WinSendDlgItemMsg(hwnd,
				    VINF_LISTBOX,
				    LM_INSERTITEM,
				    MPFROM2SHORT(LIT_SORTASCENDING, 0),
				    MPFROMP(text));
		Continue:
		  nm = 1;
		} while (!DosFindNext(hdir, &ffb, sizeof(ffb), &nm));
		DosFindClose(hdir);
		priority_normal();
	      }
	    }
	    if (!WinIsWindow(hab2, hwnd))
	      break;
	    p = strtok(NULL, ";");
	  }
	  xfree(holdenv);
	NoEnv:
	  if (WinIsWindow(hab2, hwnd) && !repeating) {

	    ULONG size;
	    CHAR *key = "INFPaths";

	    if (dummy->help)
	      key = "HLPPaths";
	    repeating = TRUE;
	    if (PrfQueryProfileSize(fmprof, FM3Str, key, &size) && size) {
	      holdenv = xmalloc(size + 2, pszSrcFile, __LINE__);
	      if (holdenv) {
		if (!PrfQueryProfileData(fmprof, FM3Str, key, holdenv, &size)) {
		  Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
			    "PrfQueryProfileData");
		  xfree(holdenv);
		}
		else
		  goto Repeat;
	      }
	    }
	  }
	}
      }
      WinDestroyMsgQueue(hmq2);
    }
    PostMsg(hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
    DecrThreadUsage();
    WinTerminate(hab2);
  }
  xfree(dummy);
}

MRESULT EXPENTRY ViewInfProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static HWND hwndMe = (HWND) 0;
  static BOOL help = FALSE;
  static BOOL threaddone = TRUE;
  static LONG ypos = 0;
  static HPOINTER hptrIcon = (HPOINTER) 0;

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, VINF_ENTRY, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(1000, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, VINF_TOPIC, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH * 2, 0), MPVOID);
    if (hwndMe) {
      WinSetWindowPos(hwndMe, HWND_TOP, 0, 0, 0, 0, SWP_RESTORE | SWP_ZORDER |
		      SWP_ACTIVATE);
      WinDismissDlg(hwnd, 0);
      break;
    }
    help = (mp2 != (MPARAM) 0);
    if (help) {

      SWP swp;

      WinQueryWindowPos(WinWindowFromID(hwnd, VINF_LISTBOX), &swp);
      WinDestroyWindow(WinWindowFromID(hwnd, VINF_LISTBOX));
      if (!WinCreateWindow(hwnd,
			   WC_LISTBOX,
			   (PSZ) NULL,
			   WS_VISIBLE | LS_HORZSCROLL,
			   swp.x,
			   swp.y,
			   swp.cx,
			   swp.cy,
			   hwnd, HWND_TOP, VINF_LISTBOX, NULL, NULL)) {
	Win_Error2(hwnd, hwnd, pszSrcFile, __LINE__, IDS_WINCREATEWINDOW);
      }
      else {
	WinSetPresParam(WinWindowFromID(hwnd, VINF_LISTBOX),
			PP_FONTNAMESIZE,
			strlen(GetPString(IDS_10SYSTEMMONOTEXT)) + 1,
			(PVOID) GetPString(IDS_10SYSTEMMONOTEXT));
      }
      WinSetWindowText(hwnd, GetPString(IDS_VIEWHELPFILESTEXT));
      WinShowWindow(WinWindowFromID(hwnd, VINF_SRCH), FALSE);
      WinShowWindow(WinWindowFromID(hwnd, VINF_FILTER), FALSE);
      WinShowWindow(WinWindowFromID(hwnd, VINF_TOPIC), FALSE);
      WinShowWindow(WinWindowFromID(hwnd, VINF_TOPICHDR), FALSE);
    }
    hptrIcon = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, VINF_FRAME);
    WinDefDlgProc(hwnd, WM_SETICON, MPFROMLONG(hptrIcon), MPVOID);
    hwndMe = hwnd;
    PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    break;

  case UM_RESCAN:
    WinSendDlgItemMsg(hwnd, VINF_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
    {
      DUMMY *d;

      d = xmallocz(sizeof(DUMMY), pszSrcFile, __LINE__);
      if (!d) {
	WinDismissDlg(hwnd, 0);
	return 0;
      }
      else {
	d->size = sizeof(DUMMY);
	d->hwnd = hwnd;
	if (help)
	  d->help = 1;
	if (_beginthread(FillListboxThread, NULL, 65536, (PVOID) d) == -1) {
	  Runtime_Error(pszSrcFile, __LINE__,
			GetPString(IDS_COULDNTSTARTTHREADTEXT));
	  xfree(d);
	  WinDismissDlg(hwnd, 0);
	  return 0;
	}
	WinEnableWindow(WinWindowFromID(hwnd, DID_CANCEL), FALSE);
	threaddone = FALSE;
      }
    }
    return 0;

  case UM_CONTAINER_FILLED:
    if (!(SHORT) WinSendDlgItemMsg(hwnd,
				   VINF_LISTBOX,
				   LM_QUERYITEMCOUNT, MPVOID, MPVOID)) {
      saymsg(MB_CANCEL,
	     hwnd,
	     GetPString(IDS_ERRORTEXT), GetPString(IDS_NOFILESFOUNDTEXT));
      hwndMe = (HWND) 0;
      WinDismissDlg(hwnd, 0);
    }
    WinEnableWindow(WinWindowFromID(hwnd, DID_CANCEL), TRUE);
    threaddone = TRUE;
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case VINF_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      }
      break;
    case VINF_ENTRY:
      switch (SHORT2FROMMP(mp1)) {
      case EN_CHANGE:
	{
	  SHORT sSelect;
	  CHAR szBuffer[CCHMAXPATH];

	  WinQueryDlgItemText(hwnd, VINF_ENTRY, CCHMAXPATH, szBuffer);
	  bstrip(szBuffer);
	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      VINF_LISTBOX,
					      LM_SEARCHSTRING,
					      MPFROM2SHORT(LSS_SUBSTRING,
							   LIT_FIRST),
					      MPFROMP(szBuffer));
	  if (sSelect >= 0)
	    WinSendDlgItemMsg(hwnd,
			      VINF_LISTBOX,
			      LM_SETTOPINDEX,
			      MPFROM2SHORT(sSelect, 0), MPVOID);
	}
	break;
      }
      break;
    }
    return 0;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
    break;

  case UM_STRETCH:
    {
      SWP swpC, swp;

      WinQueryWindowPos(hwnd, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
	WinQueryWindowPos(WinWindowFromID(hwnd, VINF_LISTBOX), &swpC);
	if (!ypos)
	  ypos = swpC.y;
	WinSetWindowPos(WinWindowFromID(hwnd, VINF_LISTBOX), HWND_TOP,
			SysVal(SV_CXSIZEBORDER),
			ypos,
			swp.cx - (SysVal(SV_CXSIZEBORDER) * 2),
			(swp.cy - ypos) - (SysVal(SV_CYTITLEBAR) +
					   SysVal(SV_CYSIZEBORDER)),
			SWP_MOVE | SWP_SIZE);
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case VINF_RESCAN:
      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      break;

    case VINF_SRCH:
      {
	SHORT sSelect, sLast = LIT_FIRST;
	CHAR szBuffer[CCHMAXPATH];

	*szBuffer = 0;
	WinQueryDlgItemText(hwnd, VINF_ENTRY, CCHMAXPATH, szBuffer);
	rstrip(szBuffer);
	if (!*szBuffer) {
	  sLast = (SHORT) WinSendDlgItemMsg(hwnd,
					    VINF_LISTBOX,
					    LM_QUERYITEMCOUNT,
					    MPVOID, MPVOID);
	  for (sSelect = 0; sSelect < sLast; sSelect++)
	    WinSendDlgItemMsg(hwnd,
			      VINF_LISTBOX,
			      LM_SELECTITEM,
			      MPFROM2SHORT(sSelect, 0),
			      MPFROM2SHORT(FALSE, 0));
	  break;
	}
	for (;;) {
	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd, VINF_LISTBOX,
					      LM_SEARCHSTRING,
					      MPFROM2SHORT(LSS_SUBSTRING,
							   sLast),
					      MPFROMP(szBuffer));
	  if (sSelect <= sLast)
	    break;
	  if (sSelect >= 0)
	    WinSendDlgItemMsg(hwnd,
			      VINF_LISTBOX,
			      LM_SELECTITEM,
			      MPFROM2SHORT(sSelect, 0),
			      MPFROM2SHORT(TRUE, 0));
	  else
	    break;
	  sLast = sSelect;
	}
      }
      break;

    case VINF_FILTER:
      {
	SHORT sSelect, sTotal;

	sTotal = (SHORT) WinSendDlgItemMsg(hwnd,
					   VINF_LISTBOX,
					   LM_QUERYITEMCOUNT, MPVOID, MPVOID);
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    VINF_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROM2SHORT(LIT_FIRST, 0),
					    MPVOID);
	if (sSelect == LIT_NONE) {
	  DosBeep(50, 100);
	  break;
	}
#ifdef BUGFIXED
	sSelect = 0;
	while (sSelect < sTotal) {
	  if (!WinSendDlgItemMsg(hwnd,
				 VINF_LISTBOX,
				 LM_SELECTITEM,
				 MPFROM2SHORT(sSelect, 0),
				 MPFROM2SHORT(FALSE, 0))) {
	    WinSendDlgItemMsg(hwnd,
			      VINF_LISTBOX,
			      LM_DELETEITEM,
			      MPFROM2SHORT(sSelect, 0), MPVOID);
	    sTotal--;
	  }
	  else
	    sSelect++;
	}
#else // !BUGIFIXED
	for (sSelect = 0; sSelect < sTotal; sSelect++)
	  WinSendDlgItemMsg(hwnd,
			    VINF_LISTBOX,
			    LM_SETITEMHANDLE,
			    MPFROM2SHORT(sSelect, 0), MPVOID);
	sSelect = LIT_FIRST;
	for (;;) {
	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      VINF_LISTBOX,
					      LM_QUERYSELECTION,
					      MPFROM2SHORT(sSelect, 0),
					      MPVOID);
	  if (sSelect >= 0)
	    WinSendDlgItemMsg(hwnd,
			      VINF_LISTBOX,
			      LM_SETITEMHANDLE,
			      MPFROM2SHORT(sSelect, 0), MPFROMLONG(1));
	  else
	    break;
	}
	for (sSelect = 0; sSelect < sTotal; sSelect++)
	  WinSendDlgItemMsg(hwnd,
			    VINF_LISTBOX,
			    LM_SELECTITEM,
			    MPFROM2SHORT(sSelect, 0), MPFROM2SHORT(FALSE, 0));
	sSelect = 0;
	while (sSelect < sTotal) {
	  if (!WinSendDlgItemMsg(hwnd,
				 VINF_LISTBOX,
				 LM_QUERYITEMHANDLE,
				 MPFROM2SHORT(sSelect, 0), MPVOID)) {
	    WinSendDlgItemMsg(hwnd,
			      VINF_LISTBOX,
			      LM_DELETEITEM,
			      MPFROM2SHORT(sSelect, 0), MPVOID);
	    sTotal--;
	  }
	  else
	    sSelect++;
	}
#endif // BUGFIXED
      }
      break;

    case VINF_DIRS:
      {
	CHAR szBuffer[1001], *key = "INFPaths";

	if (help)
	  key = "HLPPaths";
	*szBuffer = 0;
	WinQueryDlgItemText(hwnd, VINF_ENTRY, 1000, szBuffer);
	bstrip(szBuffer);
	PrfWriteProfileData(fmprof,
			    FM3Str,
			    key,
			    (*szBuffer) ? szBuffer : NULL, strlen(szBuffer));
	PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      }
      break;

    case DID_CANCEL:
      hwndMe = (HWND) 0;
      WinDismissDlg(hwnd, 0);
      break;

    case DID_OK:
      {
	SHORT sSelect;
	CHAR text[CCHMAXPATH * 2], filename[CCHMAXPATH], *p;
	FILE *fp;
	BOOL notfirst = FALSE;

	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    VINF_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROM2SHORT(LIT_FIRST, 0),
					    MPVOID);
	if (sSelect < 0) {
	  DosBeep(50, 100);
	}
	else {
	  if (help) {
	    *text = 0;
	    WinSendDlgItemMsg(hwnd,
			      VINF_LISTBOX,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect, CCHMAXPATH),
			      MPFROMP(text));
	    p = strchr(text, '>');
	    if (!p) {
	      DosBeep(50, 100);
	      break;
	    }
	    p++;
	    bstrip(p);
	    if (!*p)
	      DosBeep(50, 100);
	    else
	      ViewHelp(p);
	    break;
	  }
	  save_dir2(filename);
	  if (filename[strlen(filename) - 1] != '\\')
	    strcat(filename, "\\");
	  strcat(filename, "FM2VINF.CMD");
	  fp = xfopen(filename, "w", pszSrcFile, __LINE__);
	  if (fp) {
	    fprintf(fp, "@ECHO OFF\nSET FM2REF=");
	    while (sSelect >= 0) {
	      *text = 0;
	      WinSendDlgItemMsg(hwnd,
				VINF_LISTBOX,
				LM_QUERYITEMTEXT,
				MPFROM2SHORT(sSelect,
					     CCHMAXPATH), MPFROMP(text));
	      p = strchr(text, '>');
	      if (p) {
		p++;
		bstrip(p);
		if (*p) {
		  if (notfirst)
		    fprintf(fp, "+");
		  notfirst = TRUE;
		  fprintf(fp, "%s", p);
		}
	      }
	      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
						  VINF_LISTBOX,
						  LM_QUERYSELECTION,
						  MPFROM2SHORT(sSelect, 0),
						  MPVOID);
	    }				// while
	    *text = 0;
	    WinQueryDlgItemText(hwnd, VINF_TOPIC, CCHMAXPATH * 2, text);
	    bstrip(text);
	    fprintf(fp,
		    "\nVIEW FM2REF%s%s\nDEL %s\n",
		    (*text) ? " " : NullStr, text, filename);
	    fclose(fp);
	    runemf2(SEPARATE | WINDOWED | MINIMIZED |
		    BACKGROUND | INVISIBLE,
		    hwnd, pszSrcFile, __LINE__,
		    NULL, NULL, "%s /C %s", GetCmdSpec(FALSE), filename);
	  }
	}
      }
      break;

    case IDM_HELP:
      if (hwndHelp) {
	if (help)
	  WinSendMsg(hwndHelp,
		     HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_VIEWHELPS, 0),
		     MPFROMSHORT(HM_RESOURCEID));
	else
	  WinSendMsg(hwndHelp,
		     HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_VIEWINF, 0),
		     MPFROMSHORT(HM_RESOURCEID));
      }
      break;
    }
    return 0;

  case WM_CLOSE:
    if (!threaddone)
      return 0;
    break;

  case WM_DESTROY:
    if (hwndMe == hwnd) {
      hwndMe = (HWND) 0;
      if (hptrIcon)
	WinDestroyPointer(hptrIcon);
      hptrIcon = (HPOINTER) 0;
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(VIEWINFS,FillListboxThread,ViewInfProc)
