
/***********************************************************************

  $Id$

  attributes editor

  Copyright (c) 1993, 1998 M. Kimes
  Copyright (c) 2006 Steven H.Levine

  14 Jul 06 SHL Use Runtime_Error
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

static PSZ pszSrcFile = __FILE__;

MRESULT EXPENTRY AttrListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  LISTINFO *li;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2)
      WinDismissDlg(hwnd, 0);
    else {
      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      li = (LISTINFO *) mp2;
      if (!li->list || !li->list[0]) {
	WinSendMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	break;
      }
    }
    WinSendDlgItemMsg(hwnd, ATR_YEAR, SPBM_SETTEXTLIMIT, MPFROMSHORT(4L),
		      MPVOID);
    WinSendDlgItemMsg(hwnd, ATR_MONTH, SPBM_SETTEXTLIMIT, MPFROMSHORT(2L),
		      MPVOID);
    WinSendDlgItemMsg(hwnd, ATR_DAY, SPBM_SETTEXTLIMIT, MPFROMSHORT(2L),
		      MPVOID);
    WinSendDlgItemMsg(hwnd, ATR_HOUR, SPBM_SETTEXTLIMIT, MPFROMSHORT(2L),
		      MPVOID);
    WinSendDlgItemMsg(hwnd, ATR_MINUTES, SPBM_SETTEXTLIMIT, MPFROMSHORT(2L),
		      MPVOID);
    WinSendDlgItemMsg(hwnd, ATR_SECONDS, SPBM_SETTEXTLIMIT, MPFROMSHORT(2L),
		      MPVOID);
    WinSendDlgItemMsg(hwnd, ATR_YEAR, SPBM_OVERRIDESETLIMITS,
		      MPFROMLONG(2200L), MPFROMLONG(1980L));
    WinSendDlgItemMsg(hwnd, ATR_MONTH, SPBM_OVERRIDESETLIMITS,
		      MPFROMLONG(12L), MPFROMLONG(1L));
    WinSendDlgItemMsg(hwnd, ATR_DAY, SPBM_OVERRIDESETLIMITS, MPFROMLONG(31L),
		      MPFROMLONG(1L));
    WinSendDlgItemMsg(hwnd, ATR_HOUR, SPBM_OVERRIDESETLIMITS, MPFROMLONG(23L),
		      MPFROMLONG(0L));
    WinSendDlgItemMsg(hwnd, ATR_MINUTES, SPBM_OVERRIDESETLIMITS,
		      MPFROMLONG(59L), MPFROMLONG(0L));
    WinSendDlgItemMsg(hwnd, ATR_SECONDS, SPBM_OVERRIDESETLIMITS,
		      MPFROMLONG(59L), MPFROMLONG(0L));
    WinCheckButton(hwnd, ATR_USEDATETIME, FALSE);
    {
      INT x;

      for (x = 0; li->list[x]; x++) {
	WinSendDlgItemMsg(hwnd, ATR_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0), MPFROMP(li->list[x]));
	WinSendDlgItemMsg(hwnd, ATR_LISTBOX, LM_SELECTITEM,
			  MPFROM2SHORT(x, 0), MPFROMSHORT(TRUE));
      }
    }
    PostMsg(hwnd, UM_UNDO, MPFROMLONG(((!li->list[1]) ? 1L : 0L)), MPVOID);
    if (li->list[1])
      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(ATR_NOW, 0), MPVOID);
    break;

  case UM_UNDO:
    {
      FILESTATUS3 fi;
      long ro = 2, hi = 2, sy = 2, ar = 2;
      BOOL allgrey;

      allgrey = ((long)WinQueryButtonCheckstate(hwnd, ATR_READONLY) == 2 &&
		 (long)WinQueryButtonCheckstate(hwnd, ATR_SYSTEM) == 2 &&
		 (long)WinQueryButtonCheckstate(hwnd, ATR_HIDDEN) == 2 &&
		 (long)WinQueryButtonCheckstate(hwnd, ATR_ARCHIVED) == 2);
      li = INSTDATA(hwnd);
      if (li && li->list[0] && (allgrey || mp1) &&
	  !DosQueryPathInfo(li->list[0], FIL_STANDARD, &fi,
			    (ULONG) sizeof(fi))) {
	ro = ((fi.attrFile & FILE_READONLY) != 0);
	hi = ((fi.attrFile & FILE_HIDDEN) != 0);
	sy = ((fi.attrFile & FILE_SYSTEM) != 0);
	ar = ((fi.attrFile & FILE_ARCHIVED) != 0);
	WinSendDlgItemMsg(hwnd, ATR_YEAR, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(fi.fdateLastWrite.year + 1980), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_MONTH, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(fi.fdateLastWrite.month), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_DAY, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(fi.fdateLastWrite.day), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_HOUR, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(fi.ftimeLastWrite.hours), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_MINUTES, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(fi.ftimeLastWrite.minutes), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_SECONDS, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(fi.ftimeLastWrite.twosecs * 2), MPVOID);
      }
      WinCheckButton(hwnd, ATR_READONLY, ro);
      WinCheckButton(hwnd, ATR_HIDDEN, hi);
      WinCheckButton(hwnd, ATR_SYSTEM, sy);
      WinCheckButton(hwnd, ATR_ARCHIVED, ar);
      PostMsg(hwnd, UM_CONTROL, MPFROM2SHORT(ATR_READONLY, BN_CLICKED),
	      MPVOID);
      PostMsg(hwnd, UM_CONTROL, MPFROM2SHORT(ATR_HIDDEN, BN_CLICKED), MPVOID);
      PostMsg(hwnd, UM_CONTROL, MPFROM2SHORT(ATR_SYSTEM, BN_CLICKED), MPVOID);
      PostMsg(hwnd, UM_CONTROL, MPFROM2SHORT(ATR_ARCHIVED, BN_CLICKED),
	      MPVOID);
    }
    return 0;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, ATR_HELP), (HPS) 0, FALSE,
			TRUE);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case ATR_LISTBOX:
      if (SHORT2FROMMP(mp1) == LN_ENTER) {

	SHORT x;
	CHAR szBuffer[CCHMAXPATH];

	x = (SHORT) WinSendDlgItemMsg(hwnd, ATR_LISTBOX, LM_QUERYSELECTION,
				      MPFROMSHORT(LIT_FIRST), MPVOID);
	if (x >= 0) {
	  *szBuffer = 0;
	  WinSendDlgItemMsg(hwnd, ATR_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(x, CCHMAXPATH), MPFROMP(szBuffer));
	  if (*szBuffer)
	    QuickView(hwnd, szBuffer);
	}
      }
      else if (SHORT2FROMMP(mp1) == LN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, NullStr);
      else if (SHORT2FROMMP(mp1) == LN_SETFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP,
			  GetPString(IDS_ATTRLISTBOXHELPTEXT));
      break;
    case ATR_YEAR:
      if (SHORT2FROMMP(mp1) == SPBN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == SPBN_SETFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, GetPString(IDS_ATTRYEARHELPTEXT));
      break;
    case ATR_MONTH:
      if (SHORT2FROMMP(mp1) == SPBN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == SPBN_SETFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, GetPString(IDS_ATTRMONTHHELPTEXT));
      break;
    case ATR_DAY:
      if (SHORT2FROMMP(mp1) == SPBN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == SPBN_SETFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, GetPString(IDS_ATTRDAYHELPTEXT));
      break;
    case ATR_HOUR:
      if (SHORT2FROMMP(mp1) == SPBN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == SPBN_SETFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, GetPString(IDS_ATTRHOURHELPTEXT));
      break;
    case ATR_MINUTES:
      if (SHORT2FROMMP(mp1) == SPBN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == SPBN_SETFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, GetPString(IDS_ATTRMINHELPTEXT));
      break;
    case ATR_SECONDS:
      if (SHORT2FROMMP(mp1) == SPBN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == SPBN_SETFOCUS)
	WinSetDlgItemText(hwnd, ATR_HELP, GetPString(IDS_ATTRSECHELPTEXT));
      break;
    case ATR_READONLY:
    case ATR_SYSTEM:
    case ATR_HIDDEN:
    case ATR_ARCHIVED:
      WinSendMsg(hwnd, UM_CONTROL, mp1, mp2);
      break;
    }
    return 0;

  case UM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case ATR_READONLY:
    case ATR_SYSTEM:
    case ATR_HIDDEN:
    case ATR_ARCHIVED:
      {

	LONG check = WinQueryButtonCheckstate(hwnd, SHORT1FROMMP(mp1));
	CHAR s[80];

	sprintf(s, "%s %s",
		(SHORT1FROMMP(mp1) == ATR_READONLY) ?
		GetPString(IDS_ATTRREADONLYBTEXT) :
		(SHORT1FROMMP(mp1) == ATR_SYSTEM) ?
		GetPString(IDS_ATTRSYSTEMBTEXT) :
		(SHORT1FROMMP(mp1) == ATR_HIDDEN) ?
		GetPString(IDS_ATTRHIDDENBTEXT) :
		GetPString(IDS_ATTRARCHIVEDBTEXT),
		(check == 1) ?
		GetPString(IDS_ONTEXT) :
		(check == 0) ?
		GetPString(IDS_OFFTEXT) : GetPString(IDS_UNCHANGEDTEXT));
	WinSetDlgItemText(hwnd, SHORT1FROMMP(mp1), s);
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case ATR_LEAVEALL:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case ATR_NOW:
      {
	time_t t;
	struct tm *tm;

	t = time(NULL);
	tm = localtime(&t);
	WinSendDlgItemMsg(hwnd, ATR_YEAR, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(tm->tm_year + 1900), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_MONTH, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(tm->tm_mon + 1), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_DAY, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(tm->tm_mday), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_HOUR, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(tm->tm_hour), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_MINUTES, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(tm->tm_min), MPVOID);
	WinSendDlgItemMsg(hwnd, ATR_SECONDS, SPBM_SETCURRENTVALUE,
			  MPFROMSHORT(tm->tm_sec), MPVOID);
      }
      break;

    case DID_OK:
      if (!WinQueryButtonCheckstate(hwnd, ATR_USEDATETIME) &&
	  WinQueryButtonCheckstate(hwnd, ATR_HIDDEN) == 2 &&
	  WinQueryButtonCheckstate(hwnd, ATR_SYSTEM) == 2 &&
	  WinQueryButtonCheckstate(hwnd, ATR_READONLY) == 2 &&
	  WinQueryButtonCheckstate(hwnd, ATR_ARCHIVED) == 2) {
	saymsg(MB_ENTER,
	       hwnd,
	       GetPString(IDS_UHYOTEXT), GetPString(IDS_ATTRADVANTAGETEXT));
	break;
      }
      {
	ULONG temp = 0L;
	FILESTATUS3 fi;
	SHORT x;
	APIRET rc;
	USHORT state;

	li = INSTDATA(hwnd);
	if (!li) {
	  Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
	  break;
	}
	{
	  CHAR szBuffer[CCHMAXPATH + 1];
	  INT numfiles = 0, numalloc = 0, error;

	  if (li->list)
	    FreeList(li->list);
	  li->list = NULL;
	  x = (SHORT) WinSendDlgItemMsg(hwnd, ATR_LISTBOX, LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (x >= 0) {
	    do {
	      *szBuffer = 0;
	      WinSendDlgItemMsg(hwnd, ATR_LISTBOX, LM_QUERYITEMTEXT,
				MPFROM2SHORT(x, CCHMAXPATH),
				MPFROMP(szBuffer));
	      if (*szBuffer && isalpha(*szBuffer) &&
		  !(driveflags[toupper(*szBuffer) - 'A'] &
		    DRIVE_NOTWRITEABLE)) {
		error = AddToList(szBuffer, &li->list, &numfiles, &numalloc);
		if (error) {
		  Runtime_Error(pszSrcFile, __LINE__, "AddToList");
		  break;
		}
	      }
	      x = (SHORT) WinSendDlgItemMsg(hwnd, ATR_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(x), MPVOID);
	    } while (x >= 0);
	  }
	}
	if (!li->list || !li->list[0]) {
	  Runtime_Error(pszSrcFile, __LINE__, "list build failed");
	  break;
	}
	for (x = 0; li->list[x]; x++) {
	  DosError(FERR_DISABLEHARDERR);
	  rc = DosQueryPathInfo(li->list[x], FIL_STANDARD, &fi,
				(ULONG) sizeof(fi));
	  if (rc)
	    continue;
	  if (WinQueryButtonCheckstate(hwnd, ATR_USEDATETIME)) {
	    WinSendDlgItemMsg(hwnd, ATR_YEAR, SPBM_QUERYVALUE, MPFROMP(&temp),
			      MPFROM2SHORT(0, SPBQ_DONOTUPDATE));
	    fi.fdateLastWrite.year = temp - 1980L;
	    WinSendDlgItemMsg(hwnd, ATR_MONTH, SPBM_QUERYVALUE,
			      MPFROMP(&temp), MPFROM2SHORT(0,
							   SPBQ_DONOTUPDATE));
	    fi.fdateLastWrite.month = temp;
	    WinSendDlgItemMsg(hwnd, ATR_DAY, SPBM_QUERYVALUE, MPFROMP(&temp),
			      MPFROM2SHORT(0, SPBQ_DONOTUPDATE));
	    fi.fdateLastWrite.day = temp;
	    WinSendDlgItemMsg(hwnd, ATR_HOUR, SPBM_QUERYVALUE, MPFROMP(&temp),
			      MPFROM2SHORT(0, SPBQ_DONOTUPDATE));
	    fi.ftimeLastWrite.hours = temp;
	    WinSendDlgItemMsg(hwnd, ATR_MINUTES, SPBM_QUERYVALUE,
			      MPFROMP(&temp), MPFROM2SHORT(0,
							   SPBQ_DONOTUPDATE));
	    fi.ftimeLastWrite.minutes = temp;
	    WinSendDlgItemMsg(hwnd, ATR_SECONDS, SPBM_QUERYVALUE,
			      MPFROMP(&temp), MPFROM2SHORT(0,
							   SPBQ_DONOTUPDATE));
	    fi.ftimeLastWrite.twosecs = (temp / 2L);
	  }
	  fi.attrFile &= (~FILE_DIRECTORY);
	  state = (USHORT) WinSendDlgItemMsg(hwnd, ATR_READONLY,
					     BM_QUERYCHECK, MPVOID, MPVOID);
	  if (!state)
	    fi.attrFile &= (~FILE_READONLY);
	  else if (state == 1)
	    fi.attrFile |= FILE_READONLY;
	  state = (USHORT) WinSendDlgItemMsg(hwnd, ATR_HIDDEN, BM_QUERYCHECK,
					     MPVOID, MPVOID);
	  if (!state)
	    fi.attrFile &= (~FILE_HIDDEN);
	  else if (state == 1)
	    fi.attrFile |= FILE_HIDDEN;
	  state = (USHORT) WinSendDlgItemMsg(hwnd, ATR_SYSTEM, BM_QUERYCHECK,
					     MPVOID, MPVOID);
	  if (!state)
	    fi.attrFile &= (~FILE_SYSTEM);
	  else if (state == 1)
	    fi.attrFile |= FILE_SYSTEM;
	  state = (USHORT) WinSendDlgItemMsg(hwnd, ATR_ARCHIVED,
					     BM_QUERYCHECK, MPVOID, MPVOID);
	  if (!state)
	    fi.attrFile &= (~FILE_ARCHIVED);
	  else if (state == 1)
	    fi.attrFile |= FILE_ARCHIVED;
	  DosSetPathInfo(li->list[x], FIL_STANDARD, (PVOID) & fi,
			 (ULONG) sizeof(FILESTATUS3), 0L);
	}
	WinDismissDlg(hwnd, 1);
      }
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_ATTRIBSLIST, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(ATTRIBS,AttrListDlgProc)
