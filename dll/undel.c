
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2006 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  24 May 05 SHL Rework Win_Error usage
  17 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets_bstripcr
  03 Nov 06 SHL Count thread usage

***********************************************************************/

#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(UNDELETE,FillUndelListThread,UndeleteDlgProc)

struct tempstruct
{
  HWND hwnd;
  CHAR path[CCHMAXPATH];
  BOOL inclsubdirs;
};

static VOID FillUndelListThread(VOID * arg)
{
  HWND hwnd;
  CHAR s[CCHMAXPATH * 2];
  CHAR *path;
  HAB thab;
  HMQ thmq;
  FILE *fp;
  HFILE oldstdout, newstdout;
  struct tempstruct *undelinfo;
  BOOL killme = FALSE;
  FILESTATUS3 fsa;

  undelinfo = (struct tempstruct *)arg;
  hwnd = undelinfo->hwnd;
  path = undelinfo->path;
  DosError(FERR_DISABLEHARDERR);

  thab = WinInitialize(0);
  thmq = WinCreateMsgQueue(thab, 0);
  if (thab && thmq) {
    WinCancelShutdown(thmq, TRUE);
    IncrThreadUsage();
    WinSendDlgItemMsg(hwnd, UNDEL_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
    unlinkf("%s", "$UDELETE.#$#");
    fp = xfopen("$UDELETE.#$#", "w", pszSrcFile, __LINE__);
    if (!fp) {
      Win_Error(NULLHANDLE, hwnd, pszSrcFile, __LINE__,
		GetPString(IDS_REDIRECTERRORTEXT));
      killme = TRUE;
      goto Abort;
    }
    else {
      newstdout = -1;
      if (DosDupHandle(fileno(stdout), &newstdout)) {
	saymsg(MB_CANCEL,
	       hwnd,
	       GetPString(IDS_MAYDAYTEXT), GetPString(IDS_REDIRECTERRORTEXT));
	fclose(fp);
	killme = TRUE;
	goto Abort;
      }
      oldstdout = fileno(stdout);
      DosDupHandle(fileno(fp), &oldstdout);
      runemf2(SEPARATE | INVISIBLE | WINDOWED | BACKGROUND | WAIT,
	      hwnd,
	      NULL,
	      NULL,
	      "UNDELETE.COM %s /L%s",
	      path, (undelinfo->inclsubdirs) ? " /S" : NullStr);
      oldstdout = fileno(stdout);
      DosDupHandle(newstdout, &oldstdout);
      DosClose(newstdout);
      fclose(fp);
    }
    fp = xfopen("$UDELETE.#$#", "r", pszSrcFile, __LINE__);
    if (fp) {
      xfgets(s, sizeof(s), fp, pszSrcFile, __LINE__);	// Skip 1st line
      while (!feof(fp)) {
	strset(s, 0);
	if (!xfgets_bstripcr(s, CCHMAXPATH + 2, fp, pszSrcFile, __LINE__))
	  break;
	if (*s) {
	  if (!strnicmp(s, "SYS3194: ", 9)) {

	    APIRET temp;

	    strcat(s, " ");
	    xfgets(&s[strlen(s)], CCHMAXPATH + 128 - strlen(s), fp,
		   pszSrcFile, __LINE__);
	    fclose(fp);
	    s[CCHMAXPATH + 128] = 0;
	    stripcr(s);
	    rstrip(s);
	    strcat(s, GetPString(IDS_ASKABOUTUNDELETEHELPTEXT));
	    temp = saymsg(MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			  hwnd, GetPString(IDS_ERRORTEXT), "%s", s);
	    if (temp == MBID_YES)
	      runemf2(BACKGROUND | INVISIBLE | SEPARATE | WINDOWED,
		      hwnd,
		      NULL, NULL, "%s /C HELP UNDELETE", GetCmdSpec(FALSE));
	    if (temp == MBID_CANCEL)
	      killme = TRUE;
	    goto Abort;
	  }
	  else if (s[1] != ':')
	    continue;
	  else if ((SHORT)
		   WinSendDlgItemMsg(hwnd, UNDEL_LISTBOX, LM_SEARCHSTRING,
				     MPFROM2SHORT(0, LIT_FIRST),
				     MPFROMP(s)) < 0
		   && DosQueryPathInfo(s, FIL_STANDARD, &fsa,
				       (ULONG) sizeof(fsa)))
	    WinSendDlgItemMsg(hwnd, UNDEL_LISTBOX, LM_INSERTITEM,
			      MPFROM2SHORT(LIT_SORTASCENDING, 0), MPFROMP(s));
	}
      }
      fclose(fp);
    }
  Abort:
    ;
  }
  DosForceDelete("$UDELETE.#$#");
  if (undelinfo)
    free(undelinfo);
  if (thmq) {
    PostMsg(hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
    if (killme)
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    WinDestroyMsgQueue(thmq);
  }
  if (thab) {
    DecrThreadUsage();
    WinTerminate(thab);
  }
}

MRESULT EXPENTRY UndeleteDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SHORT sSelect;
  static BOOL listdone, changed = FALSE, refresh = FALSE;
  static HPOINTER hptrIcon = (HPOINTER) 0;

  switch (msg) {
  case WM_INITDLG:
    listdone = TRUE;
    if (!mp2 || !*(CHAR *) mp2) {
      Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      WinDismissDlg(hwnd, 0);
      break;
    }
    hptrIcon = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, UNDEL_FRAME);
    WinDefDlgProc(hwnd, WM_SETICON, MPFROMLONG(hptrIcon), MPVOID);
    WinSendDlgItemMsg(hwnd, UNDEL_ENTRY, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    {
      CHAR s[CCHMAXPATH];

      strcpy(s, (CHAR *) mp2);
      if (s[strlen(s) - 1] != '\\')
	strcat(s, "\\");
      strcat(s, "*");
      WinSetDlgItemText(hwnd, UNDEL_ENTRY, s);
      WinCheckButton(hwnd, UNDEL_SUBDIRS, TRUE);
      FillPathListBox(hwnd, WinWindowFromID(hwnd, UNDEL_DRIVELIST), (HWND) 0,
		      s, TRUE);
    }
    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    break;

  case UM_SETUP:
    if (listdone) {

      struct tempstruct *undelinfo;
      CHAR s[CCHMAXPATH];

      listdone = FALSE;
      undelinfo = xmallocz(sizeof(struct tempstruct), pszSrcFile, __LINE__);
      if (!undelinfo) {
	listdone = TRUE;
	WinDismissDlg(hwnd, 0);
      }
      else {
	undelinfo->hwnd = hwnd;
	WinQueryDlgItemText(hwnd,
			    UNDEL_ENTRY,
			    sizeof(undelinfo->path), undelinfo->path);
	bstrip(undelinfo->path);
	MakeFullName(undelinfo->path);
	undelinfo->inclsubdirs = WinQueryButtonCheckstate(hwnd,
							  UNDEL_SUBDIRS);
	sprintf(s,
		GetPString(IDS_UNDELETETITLETEXT), toupper(*undelinfo->path));
	WinSetWindowText(hwnd, s);
	if (_beginthread(FillUndelListThread, NULL, 65536, (PVOID) undelinfo)
	    == -1) {
	  Runtime_Error(pszSrcFile, __LINE__,
			GetPString(IDS_COULDNTSTARTTHREADTEXT));
	  free(undelinfo);
	  listdone = TRUE;
	  WinDismissDlg(hwnd, 0);
	}
	else
	  DosSleep(500L);
      }
      refresh = FALSE;
    }
    else
      refresh = TRUE;
    changed = FALSE;
    return 0;

  case UM_CONTAINER_FILLED:
    listdone = TRUE;
    {
      CHAR s[33];

      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					  UNDEL_LISTBOX,
					  LM_QUERYITEMCOUNT, MPVOID, MPVOID);
      sprintf(s, "%d", sSelect);
      WinSetDlgItemText(hwnd, UNDEL_COUNT, s);
      if (refresh)
	PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      refresh = FALSE;
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case UNDEL_SUBDIRS:
      PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      break;

    case UNDEL_ENTRY:
      switch (SHORT2FROMMP(mp1)) {
      case EN_CHANGE:
	changed = TRUE;
	break;
      case EN_KILLFOCUS:
	if (changed)
	  PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	break;
      }
      break;

    case UNDEL_DRIVELIST:
      if (!listdone) {
	Runtime_Error(pszSrcFile, __LINE__, "not listdone");
	break;
      }
      switch (SHORT2FROMMP(mp1)) {
      case CBN_ENTER:
	{
	  CHAR s[CCHMAXPATH], drive, *p;

	  strset(s, 0);
	  WinQueryDlgItemText(hwnd, UNDEL_DRIVELIST, 3, s);
	  if (!isalpha(*s)) {
	    Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
	  }
	  else {
	    drive = toupper(*s);
	    WinQueryDlgItemText(hwnd, UNDEL_ENTRY, sizeof(s), s);
	    *s = drive;
	    s[1] = ':';
	    s[2] = '\\';
	    p = strrchr(s + 2, '\\');
	    if (p) {
	      p++;
	      if (*p)
		memmove(s + 3, p, strlen(p) + 1);
	      else {
		s[3] = '*';
		s[4] = 0;
	      }
	    }
	    else {
	      s[3] = '*';
	      s[4] = 0;
	    }
	    s[CCHMAXPATH - 1] = 0;
	    WinSetDlgItemText(hwnd, UNDEL_ENTRY, s);
	    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	  }
	}
	break;
      }
      break;

    case UNDEL_LISTBOX:
      switch (SHORT2FROMMP(mp2)) {
      case LN_ENTER:
	WinSendDlgItemMsg(hwnd, DID_OK, BM_CLICK, MPFROMSHORT(TRUE), MPVOID);
	break;
      }
      break;

    default:
      break;
    }
    return 0;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_STRETCH, MPVOID, MPVOID);
    break;

  case UM_STRETCH:
    {
      SWP swpC, swp, swpM, swpD, swpL, swpE;

      WinQueryWindowPos(hwnd, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
	WinQueryWindowPos(WinWindowFromID(hwnd, UNDEL_LISTBOX), &swpC);
	WinQueryWindowPos(WinWindowFromID(hwnd, UNDEL_MASKHDR), &swpM);
	WinQueryWindowPos(WinWindowFromID(hwnd, UNDEL_DRVHDR), &swpD);
	WinQueryWindowPos(WinWindowFromID(hwnd, UNDEL_DRIVELIST), &swpL);
	WinQueryWindowPos(WinWindowFromID(hwnd, UNDEL_ENTRY), &swpE);
	WinSetWindowPos(WinWindowFromID(hwnd, UNDEL_LISTBOX), HWND_TOP,
			SysVal(SV_CXSIZEBORDER),
			swpC.y,
			swp.cx - (SysVal(SV_CXSIZEBORDER) * 2),
			(swp.cy - swpC.y) - (SysVal(SV_CYTITLEBAR) +
					     SysVal(SV_CYSIZEBORDER) +
					     swpM.cy + 16),
			SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, UNDEL_MASKHDR), HWND_TOP,
			swpM.x,
			(swp.cy - swpM.cy) - (SysVal(SV_CYTITLEBAR) +
					      SysVal(SV_CYSIZEBORDER) + 8),
			swpM.cx, swpM.cy, SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, UNDEL_DRVHDR), HWND_TOP,
			swpD.x,
			(swp.cy - swpM.cy) - (SysVal(SV_CYTITLEBAR) +
					      SysVal(SV_CYSIZEBORDER) + 8),
			swpD.cx, swpM.cy, SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, UNDEL_DRIVELIST), HWND_TOP,
			swpL.x,
			SysVal(SV_CYSIZEBORDER),
			swpL.cx,
			swp.cy - (SysVal(SV_CYTITLEBAR) +
				  (SysVal(SV_CYSIZEBORDER) * 2) + 6),
			SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, UNDEL_ENTRY), HWND_TOP,
			swpM.x + swpM.cx + 4,
			(swp.cy - swpM.cy) - (SysVal(SV_CYTITLEBAR) +
					      SysVal(SV_CYSIZEBORDER) + 8),
			swp.cx - ((swpM.x + swpM.cx + 4) +
				  (SysVal(SV_CXSIZEBORDER) + 8)),
			swpM.cy + 2, SWP_MOVE | SWP_SIZE);
	WinInvalidateRect(WinWindowFromID(hwnd, UNDEL_ENTRY), NULL, FALSE);
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case UNDEL_DEL:
    case DID_OK:
      if (changed || !listdone) {
	Runtime_Error(pszSrcFile, __LINE__, "not done");
      }
      else {
	sSelect = (USHORT) WinSendDlgItemMsg(hwnd, UNDEL_LISTBOX,
					     LM_QUERYSELECTION,
					     MPFROMSHORT(LIT_FIRST), MPVOID);
	if (sSelect >= 0) {

	  FILE *fp;
	  CHAR s[CCHMAXPATH + 1];

	  DosForceDelete("\\FMUNDEL.CMD");
	  fp = xfopen("\\FMUNDEL.CMD", "w", pszSrcFile, __LINE__);
	  if (fp) {
	    while (sSelect >= 0) {
	      *s = 0;
	      WinSendDlgItemMsg(hwnd, UNDEL_LISTBOX, LM_QUERYITEMTEXT,
				MPFROM2SHORT(sSelect, CCHMAXPATH),
				MPFROMP(s));
	      if (SHORT1FROMMP(mp1) == UNDEL_DEL)
		WinSendDlgItemMsg(hwnd, UNDEL_LISTBOX, LM_DELETEITEM,
				  MPFROM2SHORT(sSelect, 0), MPVOID);
	      if (*s) {
		if (SHORT1FROMMP(mp1) == DID_OK)
		  fprintf(fp,
			  "IF NOT EXIST \"%s\" UNDELETE.COM \"%s\" /A\n",
			  s, s);
		else
		  fprintf(fp, "UNDELETE.COM \"%s\" /F /A\n", s);
	      }
	      sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
						   UNDEL_LISTBOX,
						   LM_QUERYSELECTION,
						   (SHORT1FROMMP(mp1) ==
						    DID_OK) ?
						   MPFROMSHORT(sSelect) :
						   MPFROMSHORT(LIT_FIRST),
						   MPVOID);
	    }
	    fprintf(fp, "DEL \\FMUNDEL.CMD /F\n");
	    fclose(fp);
	    runemf2(WINDOWED | BACKGROUND | SEPARATE | INVISIBLE,
		    hwnd,
		    NULL, NULL, "%s /C \\FMUNDEL.CMD", GetCmdSpec(FALSE));
	  }
	}
	if (SHORT1FROMMP(mp1) == DID_OK)
	  WinDismissDlg(hwnd, 0);
	{
	  CHAR s[33];

	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      UNDEL_LISTBOX,
					      LM_QUERYITEMCOUNT,
					      MPVOID, MPVOID);
	  sprintf(s, "%d", sSelect);
	  WinSetDlgItemText(hwnd, UNDEL_COUNT, s);
	}
      }
      break;

    case DID_CANCEL:
      if (!listdone)
	Runtime_Error(pszSrcFile, __LINE__, "is busy");
      else
	WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      saymsg(MB_ENTER | MB_ICONASTERISK,
	     hwnd,
	     GetPString(IDS_UNDELETEHELPTITLETEXT),
	     GetPString(IDS_UNDELETEHELPTEXT));
      break;
    }
    return 0;

  case WM_CLOSE:
    if (!listdone) {
      Runtime_Error(pszSrcFile, __LINE__, "not listdone");
      return 0;
    }
    break;

  case WM_DESTROY:
    if (hptrIcon)
      WinDestroyPointer(hptrIcon);
    hptrIcon = (HPOINTER) 0;
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
