
/***********************************************************************

  $Id$

  Kill a process

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2010 Steven H. Levine

  24 May 05 SHL Rework Win_Error usage
  14 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets
  03 Nov 06 SHL Renames
  03 Nov 06 SHL Count thread usage
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  02 Sep 07 GKY Replaced DosQProcStatus with DosQuerySysState to fix trap in thunk code
  02 Sep 07 SHL Expand FillKillListThread2 stack to avoid exception in __TNK
  16 JUL 08 GKY Use TMP directory for temp files
  10 Dec 08 SHL Integrate exception handler support
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)
  12 Jul 09 GKY Add xDosQueryAppType and xDosAlloc... to allow FM/2 to load in high memory
  13 Dec 09 GKY Fixed separate paramenters. Please note that appname should be used in
                profile calls for user settings that work and are setable in more than one
                miniapp; FM3Str should be used for setting only relavent to FM/2 or that
                aren't user settable; realappname should be used for setting applicable to
                one or more miniapp but not to FM/2
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10).
                Mostly cast CHAR CONSTANT * as CHAR *.
  20 Nov 10 GKY Check that pTmpDir IsValid and recreate if not found; Fixes hangs caused
                by temp file creation failures.
  26 Aug 11 GKY Add a low mem version of xDosAlloc* wrappers; move error checking into all the
                xDosAlloc* wrappers.
  22 Aug 14 JBS Ticket #519: Corrected mis-coded but probably harmless calls to strtol
                and removed unneeded second parameter variables.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// #include <process.h>
#include <limits.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "init.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "procstat.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "pathutil.h"			// BldFullPathName
#include "killproc.h"
#include "systemf.h"			// ShowSession
#include "common.h"			// DecrThreadUsage, IncrThreadUsage
#include "notify.h"			// Notify
#include "copyf.h"			// unlinkf
#include "wrappers.h"			// xfgets
#include "stristr.h"			// stristr
#include "misc.h"			// PostMsg
#include "fortify.h"
#include "excputil.h"			// xbeginthread
#include "valid.h"                      // IsValidDir

// Data definitions
#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

#pragma data_seg(GLOBAL1)
BOOL fUseQProcStat;
BOOL fUseQSysState;
PID mypid;

CHAR *GetDosPgmName(PID pid, CHAR * string)
{
  HSWITCH hs;
  SWCNTRL swctl;
  PCH pch;

  *string = 0;
  hs = WinQuerySwitchHandle(0, pid);
  if (hs) {
    WinQuerySwitchEntry(hs, &swctl);
    pch = swctl.szSwtitle;
    while (*pch) {
      if (*pch < 0x10)
	if (pch != swctl.szSwtitle && *(pch - 1) == 0x20)
	  memmove(pch, pch + 1, strlen(pch));
	else {
	  *pch = 0x20;
	  pch++;
	}
      else
	pch++;
    }
    strcpy(string, swctl.szSwtitle);
  }
  if (!*string)
    strcpy(string, GetPString(IDS_UNKNOWNDOSPROCTEXT));
  return string;
}

static VOID FillKillListThread2(VOID * arg)
{
  HWND hwnd = *(HWND *) arg;
  CHAR s[1036];
  HAB thab;
  HMQ thmq;
  INT rc;
  PROCESSINFO *ppi;
  BUFFHEADER *pbh;
  MODINFO *pmi;

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif
  thab = WinInitialize(0);
  thmq = WinCreateMsgQueue(thab, 0);
  WinCancelShutdown(thmq, TRUE);
  IncrThreadUsage();

  WinSendDlgItemMsg(hwnd, KILL_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
  if (!xDosAllocMem((PVOID) & pbh, USHRT_MAX + 4096, pszSrcFile, __LINE__)) {
    rc = DosQProcStatus((ULONG *)pbh, USHRT_MAX);
    if (!rc) {
      ppi = pbh->ppi;
      while (ppi->ulEndIndicator != PROCESS_END_INDICATOR) {
	if (ppi->pid != mypid) {
	  pmi = pbh->pmi;
	  while (pmi && ppi->hModRef != pmi->hMod)
	    pmi = pmi->pNext;
	  if (pmi) {
	    sprintf(s, "%04x ", ppi->pid);
	    if (!stricmp(pmi->szModName, "SYSINIT"))
	      GetDosPgmName(ppi->pid, s + strlen(s));
	    else {
	      if (*pmi->szModName)
		strcat(s, pmi->szModName);
	      else
		strcat(s, GetPString(IDS_UNKNOWNPROCTEXT));
	    }
	    if (WinIsWindow(thab, hwnd)) {
	      WinSendDlgItemMsg(hwnd, KILL_LISTBOX, LM_INSERTITEM,
				MPFROM2SHORT(LIT_SORTASCENDING, 0),
				MPFROMP(s));
	    }
	    else
	      break;
	  }
	}
	ppi = (PPROCESSINFO) (ppi->ptiFirst + ppi->usThreadCount);
      }					// while
    }
    DosFreeMem(pbh);
  }

  if (WinIsWindow(thab, hwnd))
    PostMsg(hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
  WinDestroyMsgQueue(thmq);
  DecrThreadUsage();
  WinTerminate(thab);
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

static VOID FillKillListThread3(VOID * arg)
{
  HWND hwnd = *(HWND *) arg;
  CHAR s[1036];
  HAB thab;
  HMQ thmq;
  INT rc;
  QSPREC *ppi;
  QSPTRREC *pbh;
  QSLREC *pmi;

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif
  thab = WinInitialize(0);
  thmq = WinCreateMsgQueue(thab, 0);
  WinCancelShutdown(thmq, TRUE);
  IncrThreadUsage();

  WinSendDlgItemMsg(hwnd, KILL_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
  if (!xDosAllocMem((PVOID) & pbh, USHRT_MAX + 4096, pszSrcFile, __LINE__)) {
    rc = DosQuerySysState(QS_PROCESS | QS_MTE, 0, 0, 0, pbh, USHRT_MAX);
    if (!rc) {
      ppi = pbh->pProcRec;
      while (ppi->RecType == 1) {
	if (ppi->pid != mypid) {
	  pmi = pbh->pLibRec;
	  while (pmi && ppi->hMte != pmi->hmte)
	    pmi = pmi->pNextRec;
	  if (pmi) {
	    sprintf(s, "%04x ", ppi->pid);
	    if (!stricmp((CHAR *) pmi->pName, "SYSINIT"))
	      GetDosPgmName(ppi->pid, s + strlen(s));
	    else {
	      if (*pmi->pName)
		strcat(s, (CHAR *) pmi->pName);
	      else
		strcat(s, GetPString(IDS_UNKNOWNPROCTEXT));
	    }
	    if (WinIsWindow(thab, hwnd)) {
	      WinSendDlgItemMsg(hwnd, KILL_LISTBOX, LM_INSERTITEM,
				MPFROM2SHORT(LIT_SORTASCENDING, 0),
				MPFROMP(s));
	    }
	    else
	      break;
	  }
	}
	ppi = (QSPREC *) (ppi->pThrdRec + ppi->cTCB);	// 22 Jun 08 SHL fixme to know why this looks odd
      }					// while
    }
    DosFreeMem(pbh);
  }

  if (WinIsWindow(thab, hwnd))
    PostMsg(hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
  WinDestroyMsgQueue(thmq);
  DecrThreadUsage();
  WinTerminate(thab);
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

static VOID FillKillListThread(VOID * arg)
{
  HWND hwnd = *(HWND *) arg;
  CHAR s[1036], progname[1027], *p;
  HAB thab;
  HMQ thmq;
  FILE *fp;
  BOOL foundstart = FALSE;
  INT rc;
  CHAR *startstring = "Process and Thread Information";
  CHAR *endstring = "System Semaphore Information";
  PID pid;
  HFILE oldstdout, newstdout;
  CHAR *mode;

  DosError(FERR_DISABLEHARDERR);

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif
  thab = WinInitialize(0);
  thmq = WinCreateMsgQueue(thab, 0);
  WinCancelShutdown(thmq, TRUE);
  IncrThreadUsage();

  WinSendDlgItemMsg(hwnd, KILL_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
  if (pTmpDir && !IsValidDir(pTmpDir))
    DosCreateDir(pTmpDir, 0);
  BldFullPathName(s, pTmpDir, "$PSTAT#$.#$#");
  unlinkf(s);
  mode = "w";
  fp = xfopen(s, mode, pszSrcFile, __LINE__, TRUE);
  if (!fp) {
    Win_Error(NULLHANDLE, HWND_DESKTOP, __FILE__, __LINE__,
	      GetPString(IDS_REDIRECTERRORTEXT));
    goto Abort;
  }
  else {
    newstdout = -1;
    rc = DosDupHandle(fileno(stdout), &newstdout);
    if (rc)
      Dos_Error(MB_CANCEL, rc, hwnd, __FILE__, __LINE__, PCSZ_DOSDUPHANDLE);
    oldstdout = fileno(stdout);
    DosDupHandle(fileno(fp), &oldstdout);
    rc = runemf2(SEPARATE | INVISIBLE | FULLSCREEN | BACKGROUND | WAIT,
		 hwnd, pszSrcFile, __LINE__, NULL, NULL,
		 "%s", "PSTAT.EXE /C");
    oldstdout = fileno(stdout);
    DosDupHandle(newstdout, &oldstdout);
    DosClose(newstdout);
    fclose(fp);
    // fixme to be gone?
    if (rc == -1) {
      saymsg(MB_CANCEL,
	     hwnd,
	     GetPString(IDS_ARGHTEXT), GetPString(IDS_CANTRUNPSTATTEXT));
      goto Abort;
    }
  }
  mode = "r";
  fp = xfopen(s, mode, pszSrcFile, __LINE__, TRUE);
  if (fp) {
    while (!feof(fp)) {
      strset(s, 0);
      if (!xfgets(s, 1025, fp, pszSrcFile, __LINE__))
	break;
      if (!foundstart) {
	if (*s == ' ' && strstr(s, startstring))
	  foundstart = TRUE;
      }
      else {
	if (*s == ' ' && strstr(s, endstring))
	  break;
	if (*s == ' ' && s[5] == ' ' && isxdigit(s[1]) &&
	    isxdigit(s[2]) && isxdigit(s[3]) && isxdigit(s[4])) {
	  pid = strtol(&s[1], NULL, 16);
	  if (pid && pid != mypid) {
	    strcpy(progname, &s[30]);
	    p = strchr(progname, ' ');
	    if (p)
	      *p = 0;
	    if (!stristr(progname, "\\PSTAT.EXE")) {
	      sprintf(s, "%04x %s", pid, progname);
	      WinSendDlgItemMsg(hwnd,
				KILL_LISTBOX,
				LM_INSERTITEM,
				MPFROM2SHORT(LIT_SORTASCENDING, 0),
				MPFROMP(s));
	    }
	  }
	}
      }
    }
    fclose(fp);
  }
Abort:
  BldFullPathName(s, pTmpDir, "$PSTAT#$.#$#");
  DosForceDelete(s);
  PostMsg(hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
  WinDestroyMsgQueue(thmq);
  DecrThreadUsage();
  WinTerminate(thab);
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

MRESULT EXPENTRY KillDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SHORT sSelect;
  PID pid;
  static BOOL listdone;
  static HPOINTER hptrIcon = (HPOINTER) 0;

  switch (msg) {
  case WM_INITDLG:
    hptrIcon = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, KILL_FRAME);
    WinDefDlgProc(hwnd, WM_SETICON, MPFROMLONG(hptrIcon), MPVOID);
    WinCheckButton(hwnd, KILL_CHECKBOX, fUseQProcStat);
    WinCheckButton(hwnd, KILL2_CHECKBOX, fUseQSysState);
    if (WinQueryButtonCheckstate(hwnd, KILL2_CHECKBOX)) {
      WinCheckButton(hwnd, KILL_CHECKBOX, FALSE);
      WinEnableWindow(WinWindowFromID(hwnd, KILL_CHECKBOX), FALSE);
    }
    if (WinQueryButtonCheckstate(hwnd, KILL_CHECKBOX)) {
      WinCheckButton(hwnd, KILL2_CHECKBOX, FALSE);
      WinEnableWindow(WinWindowFromID(hwnd, KILL2_CHECKBOX), FALSE);
    }
    PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(KILL_RESCAN, 0), MPVOID);
    break;

  case UM_CONTAINER_FILLED:
    listdone = TRUE;
    if ((SHORT) WinSendDlgItemMsg(hwnd,
				  KILL_LISTBOX,
				  LM_QUERYITEMCOUNT, MPVOID, MPVOID) == 0) {
      if (!fUseQProcStat)
	saymsg(MB_CANCEL,
	       hwnd,
	       GetPString(IDS_ICHOKEDTEXT), GetPString(IDS_ISPSTATTHERETEXT));
      else
	saymsg(MB_CANCEL,
	       hwnd,
	       GetPString(IDS_ICHOKEDTEXT),
	       GetPString(IDS_DOSQPROCSTATFAILEDTEXT));
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case KILL_CHECKBOX:
      fUseQProcStat = WinQueryButtonCheckstate(hwnd, KILL_CHECKBOX);
      PrfWriteProfileData(fmprof, FM3Str, "UseQProcStat",
                          &fUseQProcStat, sizeof(BOOL));
      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(KILL_RESCAN, 0), MPVOID);
      if (WinQueryButtonCheckstate(hwnd, KILL_CHECKBOX)) {
	WinCheckButton(hwnd, KILL2_CHECKBOX, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, KILL2_CHECKBOX), FALSE);
      }
      else
	WinEnableWindow(WinWindowFromID(hwnd, KILL2_CHECKBOX), TRUE);
      break;
    case KILL2_CHECKBOX:
      fUseQSysState = WinQueryButtonCheckstate(hwnd, KILL2_CHECKBOX);
      PrfWriteProfileData(fmprof, FM3Str, "UseQSysState",
                          &fUseQSysState, sizeof(BOOL));
      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(KILL_RESCAN, 0), MPVOID);
      if (WinQueryButtonCheckstate(hwnd, KILL2_CHECKBOX)) {
	WinCheckButton(hwnd, KILL_CHECKBOX, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, KILL_CHECKBOX), FALSE);
      }
      else
	WinEnableWindow(WinWindowFromID(hwnd, KILL_CHECKBOX), TRUE);
      break;

    case KILL_LISTBOX:
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
      SWP swpC, swp, swpH;

      WinQueryWindowPos(hwnd, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
	WinQueryWindowPos(WinWindowFromID(hwnd, KILL_LISTBOX), &swpC);
	WinQueryWindowPos(WinWindowFromID(hwnd, KILL_HDR), &swpH);
	WinSetWindowPos(WinWindowFromID(hwnd, KILL_LISTBOX), HWND_TOP,
			SysVal(SV_CXSIZEBORDER),
			swpC.y,
			swp.cx - (SysVal(SV_CXSIZEBORDER) * 2),
			((swp.cy - swpC.y) - (SysVal(SV_CYTITLEBAR) +
					      SysVal(SV_CYSIZEBORDER)) -
			 (swpH.cy + 8)), SWP_MOVE | SWP_SIZE);
	WinSetWindowPos(WinWindowFromID(hwnd, KILL_HDR), HWND_TOP,
			SysVal(SV_CXSIZEBORDER) + 4,
			swpC.y + ((swp.cy - swpC.y) -
				  (SysVal(SV_CYTITLEBAR) +
				   SysVal(SV_CYSIZEBORDER)) -
				  (swpH.cy + 4)), swpH.cx, swpH.cy, SWP_MOVE);
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case KILL_RESCAN:
      listdone = FALSE;
      if (fUseQProcStat) {
	if (xbeginthread(FillKillListThread2,
			 65536 + 8192,
			 &hwnd,
			 pszSrcFile,
			 __LINE__) == -1)
	{
	  WinDismissDlg(hwnd, 0);
	}
	else
	  DosSleep(100);		// 05 Aug 07 GKY 250
      }
      else if (fUseQSysState)
	if (xbeginthread(FillKillListThread3,
			 65536,
			 &hwnd,
			 pszSrcFile,
			 __LINE__) == -1)
	{
	  WinDismissDlg(hwnd, 0);
	}
	else
	  DosSleep(100);//05 Aug 07 GKY 250
      else {
	if (xbeginthread(FillKillListThread,
			 65536,
			 &hwnd,
			 pszSrcFile,
			 __LINE__) == -1)
	{
	  WinDismissDlg(hwnd, 0);
	}
	else
	  DosSleep(100);		// 05 Aug 07 GKY 250
      }
      break;

    case KILL_SHOW:
    case DID_OK:
      sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
					   KILL_LISTBOX,
					   LM_QUERYSELECTION,
					   MPFROMSHORT(LIT_FIRST), MPVOID);
      if (sSelect >= 0) {

	CHAR s[31];
	APIRET error;

	*s = 0;
	WinSendDlgItemMsg(hwnd,
			  KILL_LISTBOX,
			  LM_QUERYITEMTEXT,
			  MPFROM2SHORT(sSelect, 30), MPFROMP(s));
	if (*s) {
	  pid = strtol(s, NULL, 16);
	  if (pid) {
	    if (SHORT1FROMMP(mp1) == DID_OK) {
	      error = DosKillProcess(DKP_PROCESS, pid);
	      if (error && error != ERROR_INVALID_PROCID) {
		Dos_Error(MB_CANCEL,
			  error,
			  hwnd,
			  __FILE__,
			  __LINE__, GetPString(IDS_DOSKILLFAILEDTEXT));
	      }
	      else
		WinSendDlgItemMsg(hwnd,
				  KILL_LISTBOX,
				  LM_DELETEITEM,
				  MPFROM2SHORT(sSelect, 0), MPVOID);
	    }
	    else if (!ShowSession(hwnd, pid))
	      Notify(GetPString(IDS_SORRYCANTSHOWTEXT));
	  }
	}
      }
      break;

    case DID_CANCEL:
      if (!listdone)
	Runtime_Error(pszSrcFile, __LINE__, "busy");
      else
	WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      saymsg(MB_ENTER | MB_ICONASTERISK,
	     hwnd,
	     GetPString(IDS_KILLPROCHELPTITLETEXT),
	     GetPString(IDS_KILLPROCHELPTEXT));
      break;
    }
    return 0;

  case WM_CLOSE:
    if (!listdone) {
      Runtime_Error(pszSrcFile, __LINE__, "busy");
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

#pragma alloc_text(KILLPROC,FillKillListThread,FillKillListThread2,GetDosPgmName,KillDlgProc)
#pragma alloc_text(KILLPROC,FillKillListThread3)
