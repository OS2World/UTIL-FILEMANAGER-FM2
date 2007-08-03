
/***********************************************************************

  $Id$

  Object containers

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2007 Steven H. Levine

  24 May 05 SHL Rework for CNRITEM.szSubject
  13 Jul 06 SHL Use Runtime_Error
  01 Sep 06 SHL Do not complain for normal cancel
  19 Oct 06 SHL Correct . and .. detect
  03 Nov 06 SHL Renames
  22 Mar 07 GKY Use QWL_USER
  01 Aug 07 SHL Rework to sync with CNRITEM mods

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <process.h>			// _beginthread

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

typedef struct
{
  CHAR *filename;
  HWND hwndCnr;
  CHAR *stopflag;
}
DIRSIZE;

typedef struct
{
  CHAR *dirname;
  CHAR stopflag;
  BOOL dying;
  BOOL working;
}
TEMP;

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static HWND objcnrwnd;

#pragma alloc_text(OBJCNR,ProcessDir,FillCnrsThread,ObjCnrDlgProc)

static VOID ProcessDir(HWND hwndCnr, CHAR * filename, PCNRITEM pciParent,
		       CHAR * stopflag)
{
  CHAR maskstr[CCHMAXPATH], *endpath, *p;
  ULONG nm, ulM;
  HDIR hdir;
  FILEFINDBUF3 *ffb, *fft;
  APIRET rc;
  RECORDINSERT ri;
  PCNRITEM pciP;

  ffb = xmalloc(sizeof(FILEFINDBUF3), pszSrcFile, __LINE__);
  if (!ffb)
    return;
  strcpy(maskstr, filename);
  if (maskstr[strlen(maskstr) - 1] != '\\')
    strcat(maskstr, "\\");
  endpath = &maskstr[strlen(maskstr)];
  strcat(maskstr, "*");
  hdir = HDIR_CREATE;
  nm = 1L;
  rc = DosFindFirst(filename, &hdir,
		    FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
		    FILE_SYSTEM | FILE_HIDDEN | MUST_HAVE_DIRECTORY,
		    ffb, sizeof(FILEFINDBUF3), &nm, FIL_STANDARD);
  if (!rc)
    DosFindClose(hdir);

  if (rc) {				/* work around furshluginer FAT bug... */
    if (IsRoot(filename))
      rc = 0;
  }

  if ((!rc && (ffb->attrFile & FILE_DIRECTORY))) {
    pciP = WinSendMsg(hwndCnr,
		      CM_ALLOCRECORD,
		      MPFROMLONG(EXTRA_RECORD_BYTES),
		      MPFROMLONG(1L));
    if (!pciP) {
      free(ffb);
      return;
    }
    pciP->pszFileName = xstrdup(filename, pszSrcFile, __LINE__);
    pciP->pszDispAttr = NullStr;
    pciP->pszSubject = NullStr;
    pciP->pszLongname = NullStr;
    if (strlen(filename) < 4)
      pciP->pszDisplayName = pciP->pszFileName;
    else {
      p = strrchr(pciP->pszFileName, '\\');
      if (!p)
	pciP->pszFileName = pciP->pszFileName;
      else if (*(p + 1))
	p++;
      pciP->pszDisplayName = p;
    }
    pciP->rc.pszIcon = pciP->pszDisplayName;
    if (fForceUpper)
      strupr(pciP->pszFileName);
    else if (fForceLower)
      strlwr(pciP->pszFileName);
    pciP->rc.flRecordAttr |= CRA_RECORDREADONLY;
  }
  else {
    free(ffb);
    Dos_Error(MB_ENTER,
	      rc,
	      HWND_DESKTOP,
	      pszSrcFile,
	      __LINE__, GetPString(IDS_CANTFINDDIRTEXT), filename);
    return;
  }
  {
    HPOINTER hptr;

    hptr = WinLoadFileIcon(pciP->pszFileName, FALSE);
    if (hptr)
      pciP->rc.hptrIcon = hptr;
  }
  if (!pciP->rc.hptrIcon || pciP->rc.hptrIcon == hptrFile)	/* OS/2 bug bug bug bug */
    pciP->rc.hptrIcon = hptrDir;
  memset(&ri, 0, sizeof(RECORDINSERT));
  ri.cb = sizeof(RECORDINSERT);
  ri.pRecordOrder = (PRECORDCORE) CMA_END;
  ri.pRecordParent = (PRECORDCORE) pciParent;
  ri.zOrder = (USHORT) CMA_TOP;
  ri.cRecordsInsert = 1L;
  ri.fInvalidateRecord = TRUE;
  if (!WinSendMsg(hwndCnr, CM_INSERTRECORD, MPFROMP(pciP), MPFROMP(&ri))) {
    free(ffb);
    return;
  }
  hdir = HDIR_CREATE;
  if (!isalpha(*maskstr) || maskstr[1] != ':' || maskstr[2] != '\\' ||
      ((driveflags[toupper(*maskstr) - 'A'] & DRIVE_REMOTE) && fRemoteBug))
    ulM = 1L;
  else
    ulM = FilesToGet;
  if (ulM > 1L) {
    fft = xrealloc(ffb, sizeof(FILEFINDBUF3) * ulM, pszSrcFile, __LINE__);
    if (!fft)
      ulM = 1L;
    else
      ffb = fft;
  }
  nm = ulM;
  rc = DosFindFirst(maskstr, &hdir,
		    FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
		    FILE_SYSTEM | FILE_HIDDEN | MUST_HAVE_DIRECTORY, ffb,
		    sizeof(FILEFINDBUF3) * ulM, &nm, FIL_STANDARD);
  if (!rc) {

    register PBYTE fb = (PBYTE) ffb;
    FILEFINDBUF3 *pffbFile;
    ULONG x;

    while (!rc) {
      for (x = 0L; x < nm; x++) {
	pffbFile = (FILEFINDBUF3 *) fb;
	if (*stopflag)
	  break;
	if ((pffbFile->attrFile & FILE_DIRECTORY) &&
	    // Skip . and ..
	    (pffbFile->achName[0] != '.' ||
	     (pffbFile->achName[1] &&
	      (pffbFile->achName[1] != '.' || pffbFile->achName[2])))) {
	  strcpy(endpath, pffbFile->achName);
	  ProcessDir(hwndCnr, maskstr, pciP, stopflag);
	}
	if (!pffbFile->oNextEntryOffset)
	  break;
	fb += pffbFile->oNextEntryOffset;
      }
      DosSleep(0L);
      if (*stopflag)
	break;
      nm = ulM;
      rc = DosFindNext(hdir, ffb, sizeof(FILEFINDBUF3) * ulM, &nm);
    }
    DosFindClose(hdir);
  }
  free(ffb);
  WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPFROMP(&pciP),
	     MPFROM2SHORT(1, 0));
}

static VOID FillCnrsThread(VOID * args)
{
  HAB hab;
  HMQ hmq;
  DIRSIZE *dirsize = (DIRSIZE *) args;

  if (!dirsize)
    return;

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 0);
    if (hmq) {
      WinCancelShutdown(hmq, TRUE);
      ProcessDir(dirsize->hwndCnr, dirsize->filename, (PCNRITEM) NULL,
		 dirsize->stopflag);
      DosPostEventSem(CompactSem);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  PostMsg(WinQueryWindow(dirsize->hwndCnr, QW_PARENT), UM_CONTAINER_FILLED,
	  MPVOID, MPVOID);
  free(dirsize);
}

MRESULT EXPENTRY ObjCnrDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  TEMP *data;

  switch (msg) {
  case WM_INITDLG:
    if (objcnrwnd) {
      Runtime_Error(pszSrcFile, __LINE__, "objcnrwnd set");
      WinSetWindowPos(objcnrwnd, HWND_TOP, 0, 0, 0, 0,
		      SWP_RESTORE | SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER);
      WinDismissDlg(hwnd, 0);
      break;
    }
    if (!mp2) {
      Runtime_Error(pszSrcFile, __LINE__, "mp2 NULL");
      WinDismissDlg(hwnd, 0);
      break;
    }
    objcnrwnd = hwnd;
    data = xmallocz(sizeof(TEMP), pszSrcFile, __LINE__);
    if (!data) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    data->dirname = (CHAR *) mp2;
    WinSetWindowPtr(hwnd, QWL_USER, (PVOID) data);
    if (*data->dirname)
      WinSetDlgItemText(hwnd, OBJCNR_DIR, data->dirname);
    {
      DIRSIZE *dirsize;

      dirsize = xmalloc(sizeof(DIRSIZE), pszSrcFile, __LINE__);
      if (!dirsize) {
	WinDismissDlg(hwnd, 0);
	break;
      }
      dirsize->stopflag = (CHAR *) & data->stopflag;
      dirsize->filename = data->dirname;
      dirsize->hwndCnr = WinWindowFromID(hwnd, OBJCNR_CNR);
      if (_beginthread(FillCnrsThread, NULL, 65536 * 8, (PVOID) dirsize) ==
	  -1) {
	Runtime_Error(pszSrcFile, __LINE__,
		      GetPString(IDS_COULDNTSTARTTHREADTEXT));
	free(dirsize);
	WinDismissDlg(hwnd, 0);
	break;
      }
      else
	data->working = TRUE;
    }
    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    break;

  case UM_SETUP:
    // WinEnableWindowUpdate(WinWindowFromID(hwnd,OBJCNR_CNR),FALSE);
    {
      CNRINFO cnri;

      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendDlgItemMsg(hwnd, OBJCNR_CNR, CM_QUERYCNRINFO,
			MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
      cnri.cyLineSpacing = 0;
      cnri.cxTreeIndent = 12L;
      cnri.pszCnrTitle = GetPString(IDS_WORKINGTEXT);
      cnri.flWindowAttr = CV_TREE | CV_FLOW |
	CA_CONTAINERTITLE | CA_TITLESEPARATOR | CA_TREELINE;
      if (WinQueryWindowUShort(hwnd, QWS_ID) == QTREE_FRAME)
	cnri.flWindowAttr |= CV_MINI;
      WinSendDlgItemMsg(hwnd, OBJCNR_CNR, CM_SETCNRINFO, MPFROMP(&cnri),
			MPFROMLONG(CMA_FLWINDOWATTR | CMA_LINESPACING |
				   CMA_CXTREEINDENT));
    }
    return 0;

  case UM_CONTAINER_FILLED:
    WinSetDlgItemText(hwnd, OBJCNR_NOTE, NullStr);
//      WinEnableWindowUpdate(WinWindowFromID(hwnd,OBJCNR_CNR),TRUE);
    WinSendDlgItemMsg(hwnd, OBJCNR_CNR, CM_INVALIDATERECORD, MPVOID,
		      MPFROM2SHORT(0, CMA_ERASE | CMA_INVALIDATE));
    data = INSTDATA(hwnd);
    if (data) {
      data->working = FALSE;
      if (data->dying)
	WinDismissDlg(hwnd, 0);
      {
	PCNRITEM pci;
	USHORT id;

	id = WinQueryWindowUShort(hwnd, QWS_ID);
	pci = (PCNRITEM) WinSendDlgItemMsg(hwnd, OBJCNR_CNR,
					   CM_QUERYRECORD,
					   MPVOID,
					   MPFROM2SHORT(CMA_FIRST,
							CMA_ITEMORDER));
	if (pci && (INT) pci != -1) {
	  ExpandAll(WinWindowFromID(hwnd, OBJCNR_CNR), TRUE, pci);
	  if (id == QTREE_FRAME)
	    pci = (PCNRITEM) WinSendDlgItemMsg(hwnd, OBJCNR_CNR,
					       CM_QUERYRECORD,
					       MPFROMP(pci),
					       MPFROM2SHORT(CMA_FIRSTCHILD,
							    CMA_ITEMORDER));
	}
	if ((!pci || (INT) pci == -1) && id == QTREE_FRAME) {
	  Notify(GetPString(IDS_NODIRSUNDERTEXT));
	  WinDismissDlg(hwnd, 0);
	  break;
	}
      }
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case OBJCNR_CNR:
      if (SHORT2FROMMP(mp1) == CN_ENTER) {

	PCNRITEM pci = (PCNRITEM) ((PNOTIFYRECORDENTER) mp2)->pRecord;

	if (pci && (INT) pci != -1)
	  WinSendDlgItemMsg(hwnd, DID_OK, BM_CLICK, MPVOID, MPVOID);
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_HELP:
      if (hwndHelp) {

	USHORT id;

	id = WinQueryWindowUShort(hwnd, QWS_ID);

	if (id == QTREE_FRAME)
	  WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_QUICKTREE, 0),
		     MPFROMSHORT(HM_RESOURCEID));
	else
	  WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_OBJECTPATH, 0),
		     MPFROMSHORT(HM_RESOURCEID));
      }
      break;

    case OBJCNR_DESKTOP:
    case DID_OK:
      data = INSTDATA(hwnd);
      if (data) {

	PCNRITEM pci;

	if (data->working) {
	  Runtime_Error(pszSrcFile, __LINE__, "working unexpected");
	  break;
	}
	if (SHORT1FROMMP(mp1) == OBJCNR_DESKTOP) {
	  WinDismissDlg(hwnd, 2);
	  break;
	}
	pci = (PCNRITEM) WinSendDlgItemMsg(hwnd, OBJCNR_CNR,
					   CM_QUERYRECORDEMPHASIS,
					   MPFROMLONG(CMA_FIRST),
					   MPFROMSHORT(CRA_CURSORED));
	if (pci && (INT) pci != -1)
	  strcpy(data->dirname, pci->pszFileName);
	WinDismissDlg(hwnd, 1);
      }
      break;

    case DID_CANCEL:
      data = INSTDATA(hwnd);
      if (data) {
	if (data->working) {
	  data->dying = (CHAR)TRUE;
	  data->stopflag = (CHAR)0xff;
	  break;
	}
	WinDismissDlg(hwnd, 0);
      }
      break;
    }
    return 0;

  case WM_DESTROY:
    objcnrwnd = (HWND) 0;
    data = INSTDATA(hwnd);
    if (data)
      free(data);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
