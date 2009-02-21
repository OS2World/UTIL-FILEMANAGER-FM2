
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2008 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  01 Aug 04 SHL Rework fixup usage
  24 May 05 SHL Rework Win_Error usage
  05 Jun 05 SHL Use QWL_USER
  17 Jul 06 SHL Use Runtime_Error
  03 Nov 06 SHL Renames
  03 Nov 06 SHL Count thread usage
  22 Mar 07 GKY Use QWL_USER
  30 Mar 07 GKY Remove GetPString for window class names
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits
  06 Apr 07 GKY Add some error checking in drag/drop
  19 Apr 07 SHL Use FreeDragInfoData.  Add more drag/drop error checks.
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  16 Nov 07 SHL Ensure fixup buffer sufficiently large
  09 Jan 08 SHL Avoid closing INI more times than opened
  09 Jan 08 SHL Add some missing error reporting
  09 Jan 08 SHL Standardize PrfOpenProfile return checks
  09 Jan 08 SHL Use CloseProfile to avoid spurious system INI closes
  29 Feb 08 GKY Use xfree where appropriate
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory or pTmpDir and use BldFullPathName
  10 Dec 08 SHL Integrate exception handler support
  11 Jan 09 GKY Replace font names in the string file with global set at compile in init.c
  07 Feb 09 GKY Move repeated strings to PCSZs.
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
// #include <process.h>                    // _beginthread

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG                   // dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "mainwnd2.h"			// Data declaration(s)
#include "collect.h"			// Data declaration(s)
#include "grep.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "errutil.h"                    // Dos_Error...
#include "strutil.h"                    // GetPString
#include "pathutil.h"                   // BldFullPathName
#include "mainwnd.h"                    // FillClient
#include "droplist.h"                   // FullDrgName
#include "inis.h"
#include "literal.h"			// fixup
#include "common.h"			// DecrThreadUsage, IncrThreadUsage
#include "draglist.h"			// FreeDragInfoData
#include "input.h"			// InputDlgProc
#include "valid.h"			// MakeFullName
#include "notify.h"			// Notify
#include "presparm.h"			// PresParamChanged
#include "getnames.h"			// export_filename
#include "copyf.h"			// unlinkf
#include "eas.h"                        // HexDump
#include "strips.h"			// bstrip
#include "misc.h"                       // BoxWindow
#include "dirs.h"			// save_dir2
#include "wrappers.h"			// xfopen
#include "fortify.h"
#include "excputil.h"			// xbeginthread

// Data definitions
#pragma data_seg(GLOBAL1)
HELPINIT hini;

#pragma data_seg(GLOBAL2)
CHAR *DRF_FM2INI;
CHAR *DRM_FM2INIRECORD;
RGB2 RGBBLACK;

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

typedef struct
{
  USHORT size;
  CHAR ininame[CCHMAXPATH];     // Must be null string if user or system ini
  HINI hini;
  BOOL working;
  BOOL edit;
  BOOL confirm;
  HWND hwndMenu;
  CHAR applname[CCHMAXPATH];
  CHAR keyname[CCHMAXPATH];
  CHAR *data;
  ULONG datalen;
  SHORT appindex;
  SHORT keyindex;
  BOOL dontclose;
  USHORT currid;
  BOOL isbinary;
  HWND hwndIni;
  HWND hwndApp;
  HWND hwndKey;
  HWND hwndData;
  HWND hwndPopup;
  HWND hwndCurrent;
}
INIDATA;

typedef struct
{
  USHORT size;
  USHORT action;
  CHAR filename1[CCHMAXPATH];
  CHAR filename2[CCHMAXPATH];
  CHAR app[CCHMAXPATH];
  CHAR key[CCHMAXPATH];
  CHAR app2[CCHMAXPATH];
  CHAR key2[CCHMAXPATH];
  HWND hwndDlg;
  HWND hwndSource;
  BOOL confirm;
}
INIREC;

/**
 * Close profile unless it is user or system INI
 * @param hini is profile handle
 * @param fSkipIfSystem bypasses close for system INIs
 */

static VOID CloseProfile(HINI hini, BOOL fSkipIfSystem)
{
  BOOL ok = fSkipIfSystem &&
	    (hini == HINI_USERPROFILE || hini == HINI_SYSTEMPROFILE);

  if (!ok) {
    ok = PrfCloseProfile(hini);
    if (!ok) {
      Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		"PrfCloseProfile failed for handle 0x%x", hini);
    }
  }
}

VOID CopyIniThread(VOID * args)
{
  INIREC *inirec = (INIREC *) args;
  HAB hab2;
  HMQ hmq2;
  HINI hiniFrom = NULLHANDLE;
  HINI hiniTo = NULLHANDLE;
  PRFPROFILE cprfp;
  CHAR userini[CCHMAXPATH], sysini[CCHMAXPATH];

  if (inirec) {
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
    hab2 = WinInitialize(0);
    if (hab2) {
      hmq2 = WinCreateMsgQueue(hab2, 0);
      if (hmq2) {
	WinCancelShutdown(hmq2, TRUE);
	IncrThreadUsage();
	*userini = *sysini = 0;
	memset(&cprfp, 0, sizeof(PRFPROFILE));
	cprfp.cchUserName = CCHMAXPATH;
	cprfp.cchSysName = CCHMAXPATH;
	cprfp.pszUserName = (PSZ) userini;
	cprfp.pszSysName = (PSZ) sysini;
	if (PrfQueryProfile(hab2, &cprfp)) {
	  if (!stricmp(cprfp.pszUserName, inirec->filename1))
	    hiniFrom = HINI_USERPROFILE;
	  else if (!stricmp(cprfp.pszSysName, inirec->filename1))
	    hiniFrom = HINI_SYSTEMPROFILE;
	  if (!stricmp(cprfp.pszUserName, inirec->filename2))
	    hiniTo = HINI_USERPROFILE;
	  else if (!stricmp(cprfp.pszSysName, inirec->filename2))
	    hiniTo = HINI_SYSTEMPROFILE;
	}
	if (hiniFrom == NULLHANDLE) {
	  hiniFrom = PrfOpenProfile(hab2, inirec->filename1);
	  if (hiniFrom == NULLHANDLE) {
	    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		      "PrfOpenProfile failed for %s", inirec->filename1);
	  }
	}
	if (hiniTo == NULLHANDLE) {
	  if (!stricmp(inirec->filename1, inirec->filename2))
	    hiniTo = hiniFrom;
	  else {
	    hiniTo = PrfOpenProfile(hab2, inirec->filename2);
	    if (hiniTo == NULLHANDLE) {
	      Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
			"PrfOpenProfile failed for %s", inirec->filename2);
	    }
	  }
	}
	if (hiniFrom && hiniTo && (*inirec->app2 || hiniFrom != hiniTo)) {

	  PVOID pDataK, pData;
	  PSZ pCurrentK;
	  ULONG ulSize = 0L;

	  if (inirec->action == IDM_MOVE || inirec->action == IDM_COPY) {
	    if (!*inirec->key) {
	      if (inirec->confirm &&
		  PrfQueryProfileSize(hiniFrom,
				      (PSZ)((*inirec->app2) ?
					inirec->app2 : inirec->	app),
				      NULL,
				      &ulSize)
		  && ulSize) {
		if (saymsg
		    (MB_YESNOCANCEL, HWND_DESKTOP,
		     GetPString(IDS_CONFIRMTEXT),
		     GetPString(IDS_INIOVERAPPNAMETEXT),
		     ((*inirec->app2) ? inirec->app2 : inirec->app),
		     inirec->filename1) != MBID_YES)
		  goto Abort;
	      }
	      if (PrfQueryProfileSize(hiniTo,
				      (PSZ) inirec->app,
				      NULL, (PULONG) & ulSize) && ulSize) {
		pDataK = xmalloc(ulSize, pszSrcFile, __LINE__); /* allocate space for keynames */
		if (pDataK) {
		  /* get keynames */
		  if (PrfQueryProfileString(hiniTo,
					    (PSZ) inirec->app,
					    NULL, "\0", pDataK, ulSize)) {
		    pCurrentK = pDataK;
		    /* step through keynames */
		    while (*pCurrentK) {
		      if (PrfQueryProfileSize(hiniTo, inirec->app,
					      pCurrentK,
					      (PULONG)&ulSize) && ulSize) {
			pData = xmalloc(ulSize, pszSrcFile, __LINE__);
			if (pData) {
			  /* get data */
			  if (PrfQueryProfileData(hiniTo, inirec->app,
						  pCurrentK,
						  pData, (PULONG) & ulSize))
			    /* write data to new ini file */
			    PrfWriteProfileData(hiniFrom, ((*inirec->app2) ?
							inirec->
							app2 : inirec->app),
						pCurrentK, pData, ulSize);
			  free(pData);  /* free data */
			}
		      }
		      while (*pCurrentK)        /* next keyname */
			pCurrentK++;
		      pCurrentK++;
		    }
		  }
		  free(pDataK);         /* free keynames */
		}
	      }
	    }
	    else {
	      if (inirec->confirm &&
		  PrfQueryProfileSize(hiniFrom, (PSZ) ((*inirec->app2) ?
						    inirec->app2 : inirec->
						    app),
				      ((*inirec->key2) ? inirec->
				       key2 : inirec->key), (PULONG) & ulSize)
		  && ulSize) {
		if (saymsg
		    (MB_YESNOCANCEL, HWND_DESKTOP,
		     GetPString(IDS_CONFIRMTEXT),
		     GetPString(IDS_INIOVERAPPKEYNAMETEXT),
		     ((*inirec->app2) ? inirec->app2 : inirec->app),
		     ((*inirec->key2) ? inirec->key2 : inirec->key),
		     inirec->filename1) != MBID_YES)
		  goto Abort;
	      }
	      if (PrfQueryProfileSize(hiniTo, inirec->app,
				      inirec->key,
				      (PULONG) & ulSize) && ulSize) {
		pData = xmalloc(ulSize, pszSrcFile, __LINE__);
		if (pData) {
		  /* get data */
		  if (PrfQueryProfileData(hiniTo, inirec->app,
					  inirec->key,
					  pData, (PULONG) & ulSize))
		    /* write data to new ini file */
		    PrfWriteProfileData(hiniFrom, ((*inirec->app2) ?
						inirec->app2 : inirec->app),
					((*inirec->key2) ?
					 inirec->key2 : inirec->key),
					pData, ulSize);
		  free(pData);          /* free data */
		}
	      }
	    }
	    if (inirec->action == IDM_MOVE) {
	      if (inirec->confirm &&
		  saymsg(MB_YESNOCANCEL,
			 HWND_DESKTOP,
			 GetPString(IDS_CONFIRMTEXT),
			 GetPString(IDS_INIDELETEMOVEDTEXT),
			 inirec->filename2) != MBID_YES)
		goto Abort;
	      PrfWriteProfileData(hiniTo,
				  inirec->app,
				  ((*inirec->key) ? inirec->key : NULL),
				  NULL, 0L);
	    }
	  }
	}
      Abort:
	if (hiniFrom)
	  CloseProfile(hiniFrom, TRUE);
	if (hiniTo && hiniTo != hiniFrom)
	  CloseProfile(hiniTo, FALSE);
	WinDestroyMsgQueue(hmq2);
      }
      DecrThreadUsage();
      WinTerminate(hab2);
    }
    PostMsg(inirec->hwndDlg, WM_COMMAND, MPFROM2SHORT(INI_REFRESH, 0),
	    MPVOID);
    if (inirec->action == IDM_MOVE && inirec->hwndSource &&
	inirec->hwndSource != inirec->hwndDlg)
      PostMsg(inirec->hwndSource, WM_COMMAND, MPFROM2SHORT(INI_REFRESH, 0),
	      MPVOID);
    free(inirec);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
  }
}

static VOID CompareIniThread(VOID * args)
{
  INIREC *inirec = (INIREC *) args;
  HAB hab2;
  HMQ hmq2;

  if (inirec) {
    hab2 = WinInitialize(0);
    if (hab2) {
      hmq2 = WinCreateMsgQueue(hab2, 0);
      if (hmq2) {
	WinCancelShutdown(hmq2, TRUE);
	IncrThreadUsage();

	// fixme to do something?

	WinDestroyMsgQueue(hmq2);
      }
      DecrThreadUsage();
      WinTerminate(hab2);
    }
    free(inirec);
  }
}

static VOID BackupIniThread(VOID * args)
{
  PPRFPROFILE prfp = (PPRFPROFILE)args;
  HAB hab2;
  HMQ hmq2;
  HINI orig = NULLHANDLE;
  HINI new;
  PVOID pDataA, pDataK, pData;
  PSZ pCurrentA, pCurrentK;
  ULONG ulSize;
  PRFPROFILE cprfp;
  CHAR userini[CCHMAXPATH], sysini[CCHMAXPATH];

  if (prfp) {
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
    hab2 = WinInitialize(0);
    if (hab2) {
      hmq2 = WinCreateMsgQueue(hab2, 0);
      if (hmq2) {
	WinCancelShutdown(hmq2, TRUE);
	IncrThreadUsage();
	*userini = *sysini = 0;
	memset(&cprfp, 0, sizeof(PRFPROFILE));
	cprfp.cchUserName = CCHMAXPATH;
	cprfp.cchSysName = CCHMAXPATH;
	cprfp.pszUserName = (PSZ) userini;
	cprfp.pszSysName = (PSZ) sysini;
	if (PrfQueryProfile(hab2, &cprfp)) {
	  if (!stricmp(cprfp.pszUserName, prfp->pszUserName))
	    orig = HINI_USERPROFILE;
	  else if (!stricmp(cprfp.pszSysName, prfp->pszUserName))
	    orig = HINI_SYSTEMPROFILE;
	}
	if (orig == NULLHANDLE)
	  orig = PrfOpenProfile(hab2, prfp->pszUserName);
	if (orig == NULLHANDLE) {
	  Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		    "PrfOpenProfile failed for %s", prfp->pszUserName);
	}
	else {
	  new = PrfOpenProfile(hab2, prfp->pszSysName);
	  if (new == NULLHANDLE) {
	    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		      "PrfOpenProfile failed for %s", prfp->pszSysName);
	  }
	  else {
	    ulSize = 0;
	    if (PrfQueryProfileSize(orig, NULL, NULL, (PULONG) & ulSize)
		&& ulSize) {
	      pDataA = xmalloc(ulSize, pszSrcFile, __LINE__);   /* allocate space for applnames */
	      if (pDataA) {
		/* get applnames */
		if (PrfQueryProfileString
		    (orig, NULL, NULL, "\0", pDataA, ulSize)) {
		  pCurrentA = pDataA;
		  /* step through applnames */
		  while (*pCurrentA) {
		    /* now keynames for this applname */
		    if (PrfQueryProfileSize(orig, (PSZ) pCurrentA, NULL,
					    (PULONG) & ulSize) && ulSize) {
		      pDataK = xmalloc(ulSize, pszSrcFile, __LINE__);   /* allocate space for keynames */
		      if (pDataK) {
			/* get keynames */
			if (PrfQueryProfileString(orig, (PSZ) pCurrentA, NULL,
						  "\0", pDataK, ulSize)) {
			  pCurrentK = pDataK;
			  /* step through keynames */
			  while (*pCurrentK) {
			    if (PrfQueryProfileSize(orig, pCurrentA,
						    pCurrentK,
						    (PULONG) & ulSize) &&
				ulSize) {
			      pData = xmalloc(ulSize, pszSrcFile, __LINE__);
			      if (pData) {
				/* get data */
				if (PrfQueryProfileData(orig, pCurrentA,
							pCurrentK,
							pData,
							(PULONG) & ulSize))
				  /* write data to new ini file */
				  PrfWriteProfileData(new, pCurrentA,
						      pCurrentK, pData,
						      ulSize);
				free(pData);    /* free data */
			      }
			    }
			    while (*pCurrentK)  /* next keyname */
			      pCurrentK++;
			    pCurrentK++;
			  }
			}
			free(pDataK);   /* free keynames */
		      }
		    }
		    while (*pCurrentA)  /* next applname */
		      pCurrentA++;
		    pCurrentA++;
		  }
		}
		free(pDataA);           /* free applnames */
	      }
	    }
	    CloseProfile(new, FALSE);
	  }
	  CloseProfile(orig, TRUE);
	}
	WinDestroyMsgQueue(hmq2);
      }
      DecrThreadUsage();
      WinTerminate(hab2);
    }
    xfree(prfp->pszUserName, pszSrcFile, __LINE__);
    xfree(prfp->pszSysName, pszSrcFile, __LINE__);
    free(prfp);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
  }
}

static VOID EnumAppNames(HWND hwndList, HINI hini)
{
  PVOID pData;
  PBYTE pCurrent;
  ULONG ulSize = 0;

  WinSendMsg(hwndList, LM_DELETEALL, NULL, NULL);
  if (!PrfQueryProfileSize(hini, NULL, NULL, (PULONG) & ulSize))
    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_PRFQUERYPROFILESIZE);
  else if (!ulSize)
    Runtime_Error(pszSrcFile, __LINE__, NULL);
  else {
    pData = xmalloc(ulSize, pszSrcFile, __LINE__);
    if (pData) {
      if (PrfQueryProfileString(hini, NULL, NULL, "\0", pData, ulSize)) {
	pCurrent = pData;
	WinEnableWindowUpdate(hwndList, FALSE);
	while (*pCurrent) {
	  WinSendMsg(hwndList, LM_INSERTITEM, MPFROMSHORT(LIT_SORTASCENDING),
		     MPFROMP(pCurrent));
	  while (*pCurrent)
	    pCurrent++;
	  pCurrent++;
	}
	WinSendMsg(hwndList, LM_SELECTITEM, MPFROMSHORT(0),
		   MPFROMSHORT(TRUE));
	WinEnableWindowUpdate(hwndList, TRUE);
      }
      free(pData);
    }
  }
}

static CHAR *GetKeyData(HWND hwndList, HINI hini, PSZ pAppName,
			PSZ pKeyName, PULONG datalen)
{
  ULONG ulKeySize = 0L;
  PVOID pKeyData = NULL;

  *datalen = 0L;
  if (!PrfQueryProfileSize(hini, pAppName, pKeyName, (PULONG) & ulKeySize))
    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_PRFQUERYPROFILESIZE);
  else {
    pKeyData = xmalloc(ulKeySize + 1L, pszSrcFile, __LINE__);
    if (pKeyData) {
      if (!PrfQueryProfileData
	  (hini, pAppName, pKeyName, pKeyData, (PULONG) & ulKeySize))
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		  PCSZ_PRFQUERYPROFILEDATA);
      else {
	HexDump(hwndList, pKeyData, ulKeySize);
	{
	  CHAR s[81];

	  sprintf(s,
		  GetPString(IDS_INIBYTESTEXT),
		  ulKeySize, &"s"[ulKeySize == 1]);
	  WinSetDlgItemText(WinQueryWindow(hwndList, QW_PARENT),
			    INI_NUMDATA, s);
	}
	*datalen = ulKeySize;
	return (CHAR *)pKeyData;
      }
    }
  }
  return NULL;
}

static BOOL EnumKeyNames(HWND hwndList, HINI hini, PSZ pAppName)
{
  PVOID pData;
  PBYTE pCurrent;
  ULONG ulSize = 0;

  WinSendMsg(hwndList, LM_DELETEALL, NULL, NULL);
  if (!PrfQueryProfileSize(hini, pAppName, NULL, (PULONG) & ulSize))
    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_PRFQUERYPROFILESIZE);
  else {
    if (!ulSize)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    pData = xmalloc(ulSize + 1L, pszSrcFile, __LINE__);
    if (pData) {
      if (!PrfQueryProfileString(hini, pAppName, NULL, "\0", pData, ulSize)) {
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		  PCSZ_PRFQUERYPROFILESTRING);
	free(pData);
      }
      else {
	pCurrent = pData;
	WinEnableWindowUpdate(hwndList, FALSE);
	while (*pCurrent) {
	  WinSendMsg(hwndList, LM_INSERTITEM, MPFROMSHORT(LIT_SORTASCENDING),
		     MPFROMP(pCurrent));
	  while (*pCurrent)
	    pCurrent++;
	  pCurrent++;
	}
	WinSendMsg(hwndList, LM_SELECTITEM, MPFROMSHORT(0),
		   MPFROMSHORT(TRUE));
	WinEnableWindowUpdate(hwndList, TRUE);
	free(pData);
	return TRUE;
      }
    }
  }
  return FALSE;
}

#define hwndMLE WinWindowFromID(hwnd,IAF_MLE)

MRESULT EXPENTRY FilterIniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  INIDATA *inidata;
  static CHAR lasttext[8192] = "";

  switch (msg) {
  case WM_INITDLG:
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    inidata = (INIDATA *) mp2;
    WinSendDlgItemMsg(hwnd, IAF_SAVENAME, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    MLEsetformat(hwndMLE, MLFIE_NOTRANS);
    MLEsetlimit(hwndMLE, 8192);
    WinSetWindowText(hwndMLE, lasttext);
    {
      FILEFINDBUF3 ffb;
      ULONG nm;
      HDIR hdir;

      hdir = HDIR_CREATE;
      nm = 1;
      DosError(FERR_DISABLEHARDERR);
      if (!DosFindFirst("*.IST",
			&hdir,
			FILE_NORMAL | FILE_ARCHIVED,
			&ffb, sizeof(ffb), &nm, FIL_STANDARD)) {
	do {
	  priority_bumped();
	  WinSendDlgItemMsg(hwnd, IAF_LISTBOX, LM_INSERTITEM,
			    MPFROMSHORT(LIT_SORTASCENDING),
			    MPFROMP(ffb.achName));
	  nm = 1;
	} while (!DosFindNext(hdir, &ffb, sizeof(ffb), &nm));
	DosFindClose(hdir);
	priority_bumped();
      }
    }
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, IAF_HELP), (HPS) 0, FALSE,
			TRUE);
    PaintRecessedWindow(WinWindowFromID(hwnd, IAF_SAVENAME), (HPS) 0, FALSE,
			FALSE);
    PaintRecessedWindow(WinWindowFromID(hwnd, IAF_LISTBOX), (HPS) 0, FALSE,
			FALSE);
    PaintRecessedWindow(WinWindowFromID(hwnd, IAF_MLE), (HPS) 0, FALSE,
			FALSE);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case IAF_SAVENAME:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, IAF_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd,
			  IAF_HELP, GetPString(IDS_MASKLISTNAMEHELPTEXT));
      break;

    case IAF_LISTBOX:
      if (SHORT2FROMMP(mp1) == LN_KILLFOCUS)
	WinSetDlgItemText(hwnd, IAF_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == LN_SETFOCUS)
	WinSetDlgItemText(hwnd,
			  IAF_HELP, GetPString(IDS_MASKLISTFILESHELPTEXT));
      else if (SHORT2FROMMP(mp1) == LN_ENTER)
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IAF_LOAD, 0), MPVOID);
      break;

    case IAF_MLE:
      if (SHORT2FROMMP(mp1) == MLN_KILLFOCUS)
	WinSetDlgItemText(hwnd, IAF_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == MLN_SETFOCUS)
	WinSetDlgItemText(hwnd,
			  IAF_HELP, GetPString(IDS_MASKLISTMASKSHELPTEXT));
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	CHAR s[8193], app[1024];
	register CHAR *p;
	SHORT sSelect, numitems;
	BOOL match;

	*s = 0;
	WinQueryWindowText(hwndMLE, 8192, s);
	if (!*s) {
          if (!fAlertBeepOff)
	    DosBeep(250, 100);
	  break;
	}
	else {
	  strcpy(lasttext, s);
	  p = s;
	  while (*p) {
	    if (*p == '\r') {
	      memmove(p, p + 1, strlen(p));
	      continue;
	    }
	    if (*p == '\n')
	      *p = 0;
	    p++;
	  }
	  p++;
	  *p = 0;
	}
	numitems = (SHORT) WinSendMsg(inidata->hwndApp,
				      LM_QUERYITEMCOUNT, MPVOID, MPVOID);
	if (!numitems) {
          if (!fAlertBeepOff)
	    DosBeep(250, 100);
	  break;
	}
	else {
	  WinSetPointer(HWND_DESKTOP, hptrBusy);
	  WinSetDlgItemText(hwnd, IAF_HELP, GetPString(IDS_FILTERINGTEXT));
	  sSelect = 0;
	  while (numitems) {
	    *app = 0;
	    WinSendMsg(inidata->hwndApp,
		       LM_QUERYITEMTEXT,
		       MPFROM2SHORT(sSelect, 1024), MPFROMP(app));
	    match = FALSE;
	    if (*app) {
	      p = s;
	      while (*p) {
		if (*p != '/') {
		  if (wildcard(app, p, TRUE))
		    match = TRUE;
		}
		else if (wildcard(app, p + 1, TRUE)) {
		  match = FALSE;
		  break;
		}
		while (*p)
		  p++;
		p++;
	      }
	    }
	    if (!match)
	      WinSendMsg(inidata->hwndApp, LM_DELETEITEM,
			 MPFROMSHORT(sSelect), MPVOID);
	    else
	      sSelect++;
	    numitems--;
	  }
	  WinSetPointer(HWND_DESKTOP, hptrArrow);
	}
      }
      WinDismissDlg(hwnd, 1);
      break;

    case IAF_LOAD:
      {
	CHAR s[8193];
	FILE *fp;
	INT len;
	SHORT sSelect;

	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, IAF_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROM2SHORT(LIT_FIRST, 0),
					    MPVOID);
	if (sSelect < 0)
	  Runtime_Error(pszSrcFile, __LINE__, NULL);
	else {
	  *s = 0;
	  WinSendDlgItemMsg(hwnd, IAF_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, CCHMAXPATH), MPFROMP(s));
	  bstrip(s);
	  if (*s) {
	    fp = _fsopen(s, "r", SH_DENYWR);
	    if (fp) {
	      len = fread(s, 1, 8192, fp);
	      s[len] = 0;
	      WinSetWindowText(hwndMLE, s);
	      fclose(fp);
	    }
	  }
	}
      }
      break;

    case IAF_SAVE:
      {
	CHAR s[8193], filename[CCHMAXPATH], *p;
	FILE *fp;

	*filename = 0;
	WinQueryDlgItemText(hwnd, IAF_SAVENAME, CCHMAXPATH, filename);
	bstrip(filename);
	if (*filename) {
	  p = strchr(filename, '.');
	  if (p) {
	    strcpy(p, ".IST");
	    WinSetDlgItemText(hwnd, IAF_SAVENAME, filename);
	  }
	  *s = 0;
	  WinQueryWindowText(hwndMLE, 8192, s);
	  if (!*s)
	    Runtime_Error(pszSrcFile, __LINE__, NULL);
	  else {
	    fp = xfopen(filename, "w", pszSrcFile, __LINE__);
	    if (fp) {
	      fwrite(s, 1, strlen(s), fp);
	      fclose(fp);
	      WinSendDlgItemMsg(hwnd,
				IAF_LISTBOX,
				LM_INSERTITEM,
				MPFROMSHORT(LIT_SORTASCENDING),
				MPFROMP(filename));
	    }
	  }
	}
      }
      break;

    case IAF_DELETE:
      {
	CHAR s[CCHMAXPATH];
	SHORT sSelect;

	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, IAF_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROM2SHORT(LIT_FIRST, 0),
					    MPVOID);
	if (sSelect >= 0) {
	  *s = 0;
	  WinSendDlgItemMsg(hwnd, IAF_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, CCHMAXPATH), MPFROMP(s));
	  bstrip(s);
	  if (*s) {
	    unlinkf(s);
	    WinSendDlgItemMsg(hwnd, IAF_LISTBOX, LM_DELETEITEM,
			      MPFROMSHORT(sSelect), MPVOID);
	  }
	}
      }
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_FILTERINI, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY IntraIniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  INIREC *inirec;

  switch (msg) {
  case WM_INITDLG:
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    inirec = (INIREC *) mp2;
    WinSendDlgItemMsg(hwnd,
		      INII_NEWAPP,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd,
		      INII_NEWKEY,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    {
      CHAR s[CCHMAXPATH + 34];

      sprintf(s, GetPString(IDS_INIWASTEXT), inirec->app);
      WinSetDlgItemText(hwnd, INII_OLDAPP, s);
    }
    if (!*inirec->key) {
      WinEnableWindow(WinWindowFromID(hwnd, INII_NEWKEY), FALSE);
      WinEnableWindow(WinWindowFromID(hwnd, INII_NEWKEYHDR), FALSE);
      switch (inirec->action) {
      case IDM_MOVE:
	WinSetWindowText(hwnd, GetPString(IDS_INIRENAMEAPPTITLETEXT));
	break;
      case IDM_COPY:
	WinSetWindowText(hwnd, GetPString(IDS_INICOPYAPPTITLETEXT));
	break;
      }
    }
    else {
      {
	CHAR s[CCHMAXPATH + 34];

	sprintf(s, GetPString(IDS_INIWASTEXT), inirec->key);
	WinSetDlgItemText(hwnd, INII_OLDKEY, s);
      }
      WinSetDlgItemText(hwnd, INII_NEWAPP, inirec->app);
      WinSendDlgItemMsg(hwnd,
			INII_NEWAPP,
			EM_SETSEL, MPFROM2SHORT(0, CCHMAXPATH), MPVOID);
      switch (inirec->action) {
      case IDM_MOVE:
	WinSetWindowText(hwnd, GetPString(IDS_INIRENAMEKEYTITLETEXT));
	break;
      case IDM_COPY:
	WinSetWindowText(hwnd, GetPString(IDS_INICOPYKEYTITLETEXT));
	break;
      }
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      inirec = (INIREC *) WinQueryWindowPtr(hwnd, QWL_USER);
      if (inirec) {
	WinQueryDlgItemText(hwnd, INII_NEWAPP, CCHMAXPATH, inirec->app2);
	bstrip(inirec->app2);
	if (!*inirec->app2) {
          if (!fAlertBeepOff)
	    DosBeep(50, 100);
	  break;
	}
	if (*inirec->key) {
	  WinQueryDlgItemText(hwnd, INII_NEWKEY, CCHMAXPATH, inirec->key2);
	  bstrip(inirec->key2);
	  if (!*inirec->key2) {
            if (!fAlertBeepOff)
	      DosBeep(50, 100);
	    break;
	  }
	}
	WinDismissDlg(hwnd, 1);
      }
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_INTRAINI, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ChangeIniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    WinSendDlgItemMsg(hwnd, INIR_USERPROFILE, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, INIR_SYSTEMPROFILE, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case INIR_FIND:
      {
	CHAR filename[CCHMAXPATH], *p, *ininame;
	USHORT id = 0;
	HWND hwndFocus;

	hwndFocus = WinQueryFocus(HWND_DESKTOP);
	if (hwndFocus)
	  id = WinQueryWindowUShort(hwndFocus, QWS_ID);
	if (id != INIR_SYSTEMPROFILE)
	  id = INIR_USERPROFILE;
	ininame = INSTDATA(hwnd);
	if (ininame && *ininame) {
	  strcpy(filename, ininame);
	  p = filename;
	  while (*p) {
	    if (*p == '/')
	      *p = '\\';
	    p++;
	  }
	  p = strrchr(filename, '\\');
	  if (p) {
	    p++;
	    *p = 0;
	  }
	  else
	    *filename = 0;
	}
	else
	  *filename = 0;
	strcat(filename, PCSZ_STARDOTINI);
	if (insert_filename(hwnd, filename, TRUE, FALSE) && *filename)
	  WinSetDlgItemText(hwnd, id, filename);
      }
      break;

    case DID_OK:
      {
	HINI testini;
	PRFPROFILE prfp;
	CHAR sysini[CCHMAXPATH];
	CHAR userini[CCHMAXPATH];

	WinQueryDlgItemText(hwnd, INIR_USERPROFILE, CCHMAXPATH, userini);
	WinQueryDlgItemText(hwnd, INIR_SYSTEMPROFILE, CCHMAXPATH, sysini);
	testini = PrfOpenProfile(WinQueryAnchorBlock(hwnd), userini);
	if (testini == NULLHANDLE) {
	  saymsg(MB_CANCEL,
		 hwnd,
		 GetPString(IDS_ERRORTEXT),
		 GetPString(IDS_INICANTOPENUSERTEXT), userini);
	  break;
	}
	CloseProfile(testini, FALSE);
	testini = PrfOpenProfile(WinQueryAnchorBlock(hwnd), sysini);
	if (testini == NULLHANDLE) {
	  saymsg(MB_CANCEL,
		 hwnd,
		 GetPString(IDS_ERRORTEXT),
		 GetPString(IDS_INICANTOPENSYSTEXT), sysini);
	  break;
	}
	CloseProfile(testini, FALSE);
	memset(&prfp, 0, sizeof(PRFPROFILE));
	prfp.cchUserName = strlen(sysini);
	prfp.cchSysName = strlen(userini);
	prfp.pszUserName = (PSZ) userini;
	prfp.pszSysName = (PSZ) sysini;
	if (!PrfReset(WinQueryAnchorBlock(hwnd), &prfp)) {
	  Win_Error(hwnd, hwnd, __FILE__, __LINE__,
		    GetPString(IDS_INIPRFRESETFAILEDTEXT));
	}
	else
	  WinDismissDlg(hwnd, 1);
      }
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_CHANGEINI, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY SwapIniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    WinSendDlgItemMsg(hwnd,
		      INIR_USERPROFILE,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd,
		      INIR_SYSTEMPROFILE,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSetWindowText(hwnd, GetPString(IDS_INISWAPOS2INISTITLETEXT));
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case INIR_FIND:
      {
	CHAR filename[CCHMAXPATH], *p, *ininame;
	USHORT id = 0;
	HWND hwndFocus;

	hwndFocus = WinQueryFocus(HWND_DESKTOP);
	if (hwndFocus)
	  id = WinQueryWindowUShort(hwndFocus, QWS_ID);
	if (id != INIR_SYSTEMPROFILE)
	  id = INIR_USERPROFILE;
	ininame = INSTDATA(hwnd);
	if (ininame && *ininame) {
	  strcpy(filename, ininame);
	  p = filename;
	  while (*p) {
	    if (*p == '/')
	      *p = '\\';
	    p++;
	  }
	  p = strrchr(filename, '\\');
	  if (p) {
	    p++;
	    *p = 0;
	  }
	  else
	    *filename = 0;
	}
	else
	  *filename = 0;
	strcat(filename, PCSZ_STARDOTINI);
	if (insert_filename(hwnd, filename, TRUE, FALSE) && *filename)
	  WinSetDlgItemText(hwnd, id, filename);
      }
      break;

    case DID_OK:
      {
	HINI testini;
	PRFPROFILE prfp;
	CHAR sysini[CCHMAXPATH];
	CHAR userini[CCHMAXPATH];
	CHAR oldsysini[CCHMAXPATH];
	CHAR olduserini[CCHMAXPATH];
	CHAR tempsysini[CCHMAXPATH];
	CHAR tempuserini[CCHMAXPATH];
	CHAR tempsysini2[CCHMAXPATH];
	CHAR tempuserini2[CCHMAXPATH];
	CHAR *p;
	APIRET rc;

	*tempuserini = 0;
	*tempsysini = 0;
	*tempuserini2 = 0;
	*tempsysini2 = 0;
	memset(&prfp, 0, sizeof(PRFPROFILE));
	prfp.cchUserName = CCHMAXPATH;
	prfp.cchSysName = CCHMAXPATH;
	prfp.pszUserName = (PSZ) olduserini;
	prfp.pszSysName = (PSZ) oldsysini;
	if (!PrfQueryProfile(WinQueryAnchorBlock(hwnd), &prfp)) {
          Win_Error(hwnd, hwnd, __FILE__, __LINE__,
                    PCSZ_INIQUERYPRFTEXT);
	  break;
	}
	WinQueryDlgItemText(hwnd, INIR_USERPROFILE, CCHMAXPATH, userini);
	WinQueryDlgItemText(hwnd, INIR_SYSTEMPROFILE, CCHMAXPATH, sysini);
	MakeFullName(userini);
	MakeFullName(sysini);
	testini = PrfOpenProfile(WinQueryAnchorBlock(hwnd), userini);
	if (testini == NULLHANDLE) {
	  saymsg(MB_CANCEL,
		 hwnd,
		 GetPString(IDS_ERRORTEXT),
		 GetPString(IDS_INICANTOPENUSERTEXT), userini);
	  break;
	}
	CloseProfile(testini, FALSE);
	testini = PrfOpenProfile(WinQueryAnchorBlock(hwnd), sysini);
	if (testini == NULLHANDLE) {
	  saymsg(MB_CANCEL,
		 hwnd,
		 GetPString(IDS_ERRORTEXT),
		 GetPString(IDS_INICANTOPENSYSTEXT), sysini);
	  break;
	}
	CloseProfile(testini, FALSE);
	/* make copies of new inis */
	*tempuserini = 0;
	*tempsysini = 0;
	BldFullPathName(tempuserini, pTmpDir ? pTmpDir : pFM2SaveDirectory, "TEMPUSER.INI");
	rc = DosCopy(userini, tempuserini, DCPY_EXISTING);
	if (rc) {
	  Dos_Error(MB_CANCEL,
		    rc,
		    hwnd,
		    __FILE__,
		    __LINE__,
		    GetPString(IDS_COMPCOPYFAILEDTEXT), userini, tempuserini);
	  break;
	}
	BldFullPathName(tempsysini, pTmpDir ? pTmpDir : pFM2SaveDirectory, "TEMPSYS.INI");
	rc = DosCopy(sysini, tempsysini, DCPY_EXISTING);
	if (rc) {
	  Dos_Error(MB_CANCEL,
		    rc,
		    hwnd,
		    __FILE__,
		    __LINE__,
		    GetPString(IDS_COMPCOPYFAILEDTEXT), sysini, tempsysini);
	  break;
	}
	memset(&prfp, 0, sizeof(PRFPROFILE));
	prfp.cchUserName = strlen(tempuserini);
	prfp.cchSysName = strlen(tempsysini);
	prfp.pszUserName = (PSZ) tempuserini;
	prfp.pszSysName = (PSZ) tempsysini;
	if (!PrfReset(WinQueryAnchorBlock(hwnd), &prfp)) {
	  Win_Error(hwnd, hwnd, __FILE__, __LINE__,
		    GetPString(IDS_INIPRFRESETFAILEDTEXT));
	}
	else {
	  /* backup old inis */
	  strcpy(tempuserini2, olduserini);
	  p = strrchr(tempuserini2, '\\');
	  if (!p)
	    *tempuserini2 = 0;
	  else {
	    p++;
	    *p = 0;
	  }
	  strcat(tempuserini2, "OS2INI.BAK");
	  rc = DosCopy(olduserini, tempuserini2, DCPY_EXISTING);
	  if (rc) {
	    Dos_Error(MB_CANCEL,
		      rc,
		      hwnd,
		      __FILE__,
		      __LINE__,
		      GetPString(IDS_COMPCOPYFAILEDTEXT),
		      olduserini, tempuserini2);
	    WinDismissDlg(hwnd, 1);
	    break;
	  }
	  strcpy(tempsysini2, oldsysini);
	  p = strrchr(tempsysini2, '\\');
	  if (!p)
	    *tempsysini2 = 0;
	  else {
	    p++;
	    *p = 0;
	  }
	  strcat(tempsysini2, "OS2SYINI.BAK");
	  rc = DosCopy(oldsysini, tempsysini2, DCPY_EXISTING);
	  if (rc) {
	    Dos_Error(MB_CANCEL,
		      rc,
		      hwnd,
		      __FILE__,
		      __LINE__,
		      GetPString(IDS_COMPCOPYFAILEDTEXT),
		      oldsysini, tempsysini2);
	    WinDismissDlg(hwnd, 1);
	    break;
	  }
	  /* copy new inis to old ini names */
	  rc = DosCopy(userini, olduserini, DCPY_EXISTING);
	  if (rc) {
	    Dos_Error(MB_CANCEL,
		      rc,
		      hwnd,
		      __FILE__,
		      __LINE__,
		      GetPString(IDS_COMPCOPYFAILEDTEXT),
		      userini, olduserini);
	    WinDismissDlg(hwnd, 1);
	    break;
	  }
	  rc = DosCopy(sysini, oldsysini, DCPY_EXISTING);
	  if (rc) {
	    Dos_Error(MB_CANCEL,
		      rc,
		      hwnd,
		      __FILE__,
		      __LINE__,
		      GetPString(IDS_COMPCOPYFAILEDTEXT),
		      userini, olduserini);
	    WinDismissDlg(hwnd, 1);
	    break;
	  }
	  /* replace temp inis with new permanent inis */
	  memset(&prfp, 0, sizeof(PRFPROFILE));
	  prfp.cchUserName = strlen(oldsysini);
	  prfp.cchSysName = strlen(olduserini);
	  prfp.pszUserName = (PSZ) olduserini;
	  prfp.pszSysName = (PSZ) oldsysini;
	  if (!PrfReset(WinQueryAnchorBlock(hwnd), &prfp)) {
	    Win_Error(hwnd, hwnd, __FILE__, __LINE__,
		      GetPString(IDS_INIPRFRESETFAILEDTEXT));
	  }
	  else {
	    Notify(GetPString(IDS_SUCCESSTEXT));
	    unlinkf(tempuserini);
	    unlinkf(tempsysini);
	  }
	  WinDismissDlg(hwnd, 1);
	}
      }
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_SWAPINI, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY AddIniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  INIDATA *inidata = INSTDATA(hwnd);
  size_t l;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    inidata = (INIDATA *) mp2;
    if (inidata->edit)
      WinSetWindowText(hwnd, GetPString(IDS_INIEDITINITITLETEXT));
    WinSetWindowPtr(hwnd, QWL_USER, (PVOID) mp2);
    WinSendDlgItemMsg(hwnd,
		      IAD_APPNAME,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd,
		      IAD_KEYNAME,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd,
		      IAD_DATA,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(32767, 0), MPVOID);
    WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
    WinSetDlgItemText(hwnd, IAD_APPNAME, inidata->applname);
    if (inidata->edit) {
      WinSetDlgItemText(hwnd, IAD_KEYNAME, inidata->keyname);
      if (inidata->data && inidata->datalen) {

	CHAR *p;

	inidata->isbinary = FALSE;
	p = inidata->data;
	while ((p - inidata->data) < inidata->datalen - 1) {
	  if (*p < ' ' || !isprint(*p)) {
	    inidata->isbinary = TRUE;
	    break;
	  }
	  p++;
	}
	if (inidata->isbinary) {
	  inidata->isbinary = TRUE;
	  WinCheckButton(hwnd, IAD_ISBINARY, TRUE);
	  if (saymsg(MB_ENTERCANCEL | MB_ICONEXCLAMATION,
		     hwnd,
		     GetPString(IDS_WARNINGTEXT),
		     GetPString(IDS_INIBINARYDATASKIPTEXT)) == MBID_CANCEL)
	    WinDismissDlg(hwnd, 0);
	  l = inidata->datalen * 4 + 1;
	  p = xmallocz(l, pszSrcFile, __LINE__);
	  if (p) {
	    fixup(inidata->data, p, l, inidata->datalen);
	    WinSetDlgItemText(hwnd, IAD_DATA, p);
	    free(p);
	  }
	}
	else
	  WinSetDlgItemText(hwnd, IAD_DATA, inidata->data);
      }
      PostMsg(hwnd, UM_SETDIR, MPFROMLONG(1L), MPVOID);
    }
    else
      PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    {
      CHAR s[32];

      *s = 0;
      WinQueryDlgItemText(hwnd, IAD_DATA, 32, s);
      if (*s)
	WinEnableWindow(WinWindowFromID(hwnd, DID_OK), TRUE);
    }
    WinSendDlgItemMsg(hwnd, IAD_APPNAME, EM_QUERYCHANGED, MPVOID, MPVOID);
    WinSendDlgItemMsg(hwnd, IAD_KEYNAME, EM_QUERYCHANGED, MPVOID, MPVOID);
    WinSendDlgItemMsg(hwnd, IAD_DATA, EM_QUERYCHANGED, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    if (mp1)
      WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IAD_DATA));
    else
      WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IAD_KEYNAME));
    return 0;

  case UM_SETUP:
    saymsg(MB_ENTER | MB_ICONASTERISK,
	   hwnd,
	   GetPString(IDS_WARNINGTEXT), GetPString(IDS_INIAPPKEYEXISTSTEXT));
    return 0;

  case WM_CONTROL:
    switch (SHORT2FROMMP(mp1)) {
    case EN_KILLFOCUS:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	CHAR s[CCHMAXPATH], applname[CCHMAXPATH];
	BOOL appchanged = FALSE, keychanged = FALSE;

	*applname = 0;
	WinQueryDlgItemText(hwnd, IAD_APPNAME, CCHMAXPATH, applname);
	bstrip(applname);
	if (*applname) {
	  *s = 0;
	  WinQueryDlgItemText(hwnd, IAD_KEYNAME, CCHMAXPATH, s);
	  bstrip(s);
	  if (*s) {
	    appchanged = (BOOL) WinSendDlgItemMsg(hwnd, IAD_APPNAME,
						  EM_QUERYCHANGED,
						  MPVOID, MPVOID);
	    keychanged = (BOOL) WinSendDlgItemMsg(hwnd, IAD_KEYNAME,
						  EM_QUERYCHANGED,
						  MPVOID, MPVOID);
	    if (appchanged || keychanged) {

	      ULONG size = 0L;

	      if (PrfQueryProfileSize(inidata->hini, applname, s, &size))
		PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	    }
	    *s = 0;
	    WinQueryDlgItemText(hwnd, IAD_DATA, CCHMAXPATH, s);
	    bstrip(s);
	    WinEnableWindow(WinWindowFromID(hwnd, DID_OK), (*s != 0));
	  }
	  else
	    WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
	}
	else
	  WinEnableWindow(WinWindowFromID(hwnd, DID_OK), FALSE);
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      {
	CHAR applname[CCHMAXPATH], keyname[CCHMAXPATH], data[CCHMAXPATH];
	INT len;

	inidata = INSTDATA(hwnd);
	if (!inidata) {
	  Runtime_Error(pszSrcFile, __LINE__, NULL);
	  break;
	}
	inidata->isbinary = WinQueryButtonCheckstate(hwnd, IAD_ISBINARY);
	*applname = 0;
	WinQueryDlgItemText(hwnd, IAD_APPNAME, CCHMAXPATH, applname);
	bstrip(applname);
	if (*applname) {
	  *keyname = 0;
	  WinQueryDlgItemText(hwnd, IAD_KEYNAME, CCHMAXPATH, keyname);
	  bstrip(keyname);
	  if (*keyname) {
	    *data = 0;
	    WinQueryDlgItemText(hwnd, IAD_DATA, CCHMAXPATH, data);
	    if (*data) {
	      if (inidata->isbinary)
		len = literal(data);
	      else
		len = strlen(data) + 1;
	      PrfWriteProfileData(inidata->hini, applname, keyname, data,
				  (ULONG) len);
	      WinSendMsg(inidata->hwndIni, UM_RESCAN,
			 MPFROMP(applname), MPFROMP(keyname));
	      WinDismissDlg(hwnd, 1);
	    }
	  }
	}
      }
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_INIADD, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static ULONG flFrameFlags = FCF_SYSMENU | FCF_SIZEBORDER | FCF_ICON |
  FCF_TITLEBAR | FCF_MINMAX | FCF_MENU | FCF_ACCELTABLE | FCF_NOBYTEALIGN;

HWND StartIniEditor(HWND hwnd, CHAR * fname, INT flags)
{
  /*
   * create an ini editor window
   * bitmapped flags:
   *  2 = don't position window for non-desktop client
   *  4 = don't kill proc when closed
   */

  HWND hwndFrame, hwndClient;
  HINI hINI;
  HAB useHab = (HAB) 0;
  CHAR *filename = NULL;

  if (fExternalINIs || strcmp(realappname, FM3Str))
    hwnd = HWND_DESKTOP;
  if (hwnd != HWND_DESKTOP)
    useHab = WinQueryAnchorBlock(hwnd);
  if (fname && *fname) {
    filename = xstrdup(fname, pszSrcFile, __LINE__);
    if (!filename)
      return (HWND) 0;
    hINI = PrfOpenProfile(useHab, filename);
    if (hINI == NULLHANDLE) {
      free(filename);
      return (HWND) 0;
    }
    else
      CloseProfile(hINI, FALSE);
  }

  if (ParentIsDesktop(hwnd, hwnd))
    flFrameFlags |= FCF_TASKLIST;
  hwndFrame = WinCreateStdWindow(hwnd,
				 0,
				 &flFrameFlags,
				 WC_INIEDITOR,
				 NullStr,
				 fwsAnimate,
				 FM3ModHandle, INI_FRAME, &hwndClient);
  if (hwndFrame) {
    WinSetWindowUShort(hwndFrame, QWS_ID, INI_FRAME);
    if (!ParentIsDesktop(hwndFrame, hwnd) && !(flags & 2)) {

      SWP swp;

      FillClient(hwnd, &swp, NULL, FALSE);
      WinSetWindowPos(hwndFrame, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
		      SWP_SIZE | SWP_MOVE);
    }
    else if (ParentIsDesktop(hwndFrame, hwnd)) {
      if (!WinRestoreWindowPos(FM2Str, "INIWindowPos", hwndFrame)) {

	ULONG fl = SWP_MOVE | SWP_SIZE;
	SWP swp;

	WinQueryTaskSizePos(WinQueryAnchorBlock(hwndFrame), 0L, &swp);
	if (swp.fl & (SWP_MINIMIZE | SWP_HIDE))
	  fl = swp.fl;
	else if (swp.fl & SWP_ACTIVATE)
	  fl |= SWP_ACTIVATE;
	WinSetWindowPos(hwndFrame, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
			fl);
      }
    }
    PostMsg(hwndClient, UM_SELECT, MPVOID, MPVOID);
    PostMsg(hwndClient, UM_ACTION, MPVOID, MPVOID);
    if (!PostMsg(hwndClient, UM_LOADFILE, MPFROMP(((filename && *filename) ?
						   filename : NULL)), MPVOID))
      WinSendMsg(hwndClient, UM_LOADFILE, MPFROMP(((filename && *filename) ?
						   filename : NULL)), MPVOID);
    if (flags & 4)
      PostMsg(hwndClient, UM_INITIALSIZE, MPVOID, MPVOID);
  }
  else
    free(filename);
  return hwndFrame;
}

MRESULT EXPENTRY IniLBSubProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);
  static HWND hwndPopup = (HWND) 0;

  switch (msg) {
  case WM_SETFOCUS:
    if (mp2)
      PostMsg(WinQueryWindow(hwnd, QW_PARENT), UM_FOCUSME, mp1, mp2);
    break;

  case WM_MENUEND:
    if (hwndPopup == (HWND) mp2) {
      WinDestroyWindow(hwndPopup);
      hwndPopup = (HWND) 0;
    }
    break;

  case WM_CONTEXTMENU:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);

      switch (id) {
      case INI_APPLIST:
	id = INI_APPMENU;
	break;
      case INI_KEYLIST:
	id = INI_KEYMENU;
	break;
      default:
	id = 0;
	break;
      }
      if (id) {
	if (hwndPopup)
	  WinDestroyWindow(hwndPopup);
	hwndPopup = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, id);
	if (hwndPopup)
	  WinPopupMenu(hwnd, WinQueryWindow(hwnd, QW_PARENT),
		       hwndPopup, 8, 8, 0,
		       PU_HCONSTRAIN | PU_VCONSTRAIN |
		       PU_KEYBOARD | PU_MOUSEBUTTON1);
      }
    }
    break;

  case WM_BEGINDRAG:
    {
      PDRAGINFO pDInfo;
      DRAGITEM DItem;
      DRAGIMAGE DIcon;
      INIDATA *inidata;
      HPOINTER hptrINI;
      USHORT id;
      HWND hDrop = 0;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      inidata = WinQueryWindowPtr(WinQueryWindow(hwnd, QW_PARENT), QWL_USER);
      if (!inidata || !*inidata->ininame || !*inidata->applname ||
	  !inidata->keyname) {
        if (!fAlertBeepOff)
	  DosBeep(50, 100);
	break;
      }
      hptrINI = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, INI_FRAME);
      memset(&DItem, 0, sizeof(DItem));
      memset(&DIcon, 0, sizeof(DIcon));
      DIcon.cb = sizeof(DIcon);
      DIcon.cptl = 0;
      DIcon.hImage = hptrINI;
      DIcon.fl = DRG_ICON;
      DIcon.sizlStretch.cx = 32;
      DIcon.sizlStretch.cy = 32;
      DIcon.cxOffset = -16;
      DIcon.cyOffset = 0;
      DItem.hwndItem = hwnd;
      DItem.ulItemID = 0L;
      DItem.hstrType = DrgAddStrHandle(DRT_UNKNOWN);
      DItem.hstrRMF = DrgAddStrHandle("<DRM_FM2INIRECORD,DRF_FM2INI>");
      DItem.hstrContainerName = DrgAddStrHandle(inidata->ininame);
      DItem.hstrSourceName = DrgAddStrHandle(inidata->applname);
      if (id == INI_APPLIST)
	DItem.hstrTargetName = DrgAddStrHandle(NullStr);
      else
	DItem.hstrTargetName = DrgAddStrHandle(inidata->keyname);
      DItem.fsControl = 0;
      DItem.fsSupportedOps = DO_COPYABLE | DO_LINKABLE | DO_MOVEABLE;
      pDInfo = DrgAllocDraginfo(1L);
      DrgSetDragitem(pDInfo, &DItem, sizeof(DRAGITEM), 0L);
      hDrop = DrgDrag(hwnd, pDInfo, &DIcon, 1L, VK_ENDDRAG, (PVOID) NULL);
      if (hDrop == NULLHANDLE)
	FreeDragInfoData(hwnd, pDInfo);
      WinDestroyPointer(hptrINI);
    }
    break;

  case DM_DRAGOVER:
    {
      PDRAGINFO pDInfo = (PDRAGINFO) mp1;
      PDRAGITEM pDItem;

      if (!DrgAccessDraginfo(pDInfo)) {
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		  "DrgAccessDraginfo");
      }
      else {
	pDItem = DrgQueryDragitemPtr(pDInfo,0);
	/* Check valid rendering mechanisms and data */
	if (DrgVerifyRMF(pDItem, DRM_OS2FILE, NULL)) {
	  DrgFreeDraginfo(pDInfo);
	  return (MRFROM2SHORT(DOR_DROP, DO_LINK));     /* OK to drop */
	}
	else if (DrgVerifyRMF(pDItem, DRM_FM2INIRECORD, DRF_FM2INI)) {
	  if (WinQueryWindow(pDInfo->hwndSource, QW_PARENT) !=
	      WinQueryWindow(hwnd, QW_PARENT))
	  {
	    DrgFreeDraginfo(pDInfo);
	    return (MRFROM2SHORT(DOR_DROP, ((fCopyDefault) ? DO_COPY : DO_MOVE)));
	  }
	}
	DrgFreeDraginfo(pDInfo);
      }
    }
    return MRFROM2SHORT(DOR_NEVERDROP, 0);

  case DM_DRAGLEAVE:

    break;

  case DM_DROP:
    {
      PDRAGINFO pDInfo = (PDRAGINFO) mp1;
      PDRAGITEM pDItem;         /* Pointer to DRAGITEM */
      ULONG numitems, curitem, len;
      USHORT action;
      CHAR szFrom[CCHMAXPATH + 2], szDir[CCHMAXPATH + 1],
	szTemp[CCHMAXPATH + 2];
      FILESTATUS3 fsa;
      INIREC inirec;

      if (!DrgAccessDraginfo(pDInfo)) {
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		  "DrgAccessDraginfo");
	return 0;
      }
      numitems = DrgQueryDragitemCount(pDInfo);
      for (curitem = 0; curitem < numitems; curitem++) {
	pDItem = DrgQueryDragitemPtr(pDInfo, curitem);
	if (DrgVerifyRMF(pDItem, DRM_OS2FILE, NULL)) {
	  if (pDItem->fsControl & DC_PREPARE)
	    DrgSendTransferMsg(pDItem->hwndItem,
			       DM_ENDCONVERSATION,
			       MPFROMLONG(pDItem->ulItemID),
			       MPFROMLONG(DMFL_TARGETFAIL));
	  else {
	    if (FullDrgName(pDItem, szFrom, sizeof(szFrom)) &&
		!DosQueryPathInfo(szFrom, FIL_STANDARD, &fsa, sizeof(fsa)))
	      WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_COMMAND,
			 MPFROM2SHORT(IDM_COMPARE, 0), MPFROMP(szFrom));
	    DrgSendTransferMsg(pDItem->hwndItem,
			       DM_ENDCONVERSATION,
			       MPFROMLONG(pDItem->ulItemID),
			       MPFROMLONG(DMFL_TARGETFAIL));
	  }
	}
	else if (DrgVerifyRMF(pDItem, DRM_FM2INIRECORD, DRF_FM2INI)) {
	  *szDir = *szFrom = *szTemp = 0;
	  len = DrgQueryStrName(pDItem->hstrContainerName, CCHMAXPATH, szDir);
	  szDir[len] = 0;
	  len = DrgQueryStrName(pDItem->hstrSourceName, CCHMAXPATH, szFrom);
	  szFrom[len] = 0;
	  len = DrgQueryStrName(pDItem->hstrTargetName, CCHMAXPATH, szTemp);
	  szTemp[len] = 0;
	  switch (pDInfo->usOperation) {
	  case DO_MOVE:
	    action = IDM_MOVE;
	    break;
	  case DO_COPY:
	    action = IDM_COPY;
	    break;
	  default:
	    action = IDM_INFO;
	    break;
	  }
	  memset(&inirec, 0, sizeof(inirec));
	  inirec.size = sizeof(inirec);
	  strcpy(inirec.filename2, szDir);
	  strcpy(inirec.app, szFrom);
	  strcpy(inirec.key, szTemp);
	  inirec.action = action;
	  inirec.hwndSource = WinQueryWindow(pDInfo->hwndSource, QW_PARENT);
	  WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_COMMAND,
		     MPFROM2SHORT(action, 0), MPFROMP(&inirec));
	  DrgSendTransferMsg(pDItem->hwndItem,
			     DM_ENDCONVERSATION,
			     MPFROMLONG(pDItem->ulItemID),
			     MPFROMLONG(DMFL_TARGETFAIL));
	}
      } // for
      FreeDragInfoData(hwnd, pDInfo);
    }
    return 0;

  case WM_DESTROY:
    if (hwndPopup)
      WinDestroyWindow(hwndPopup);
    hwndPopup = (HWND) 0;
    break;
  } // switch
  if (oldproc)
    return oldproc(hwnd, msg, mp1, mp2);
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY IniLBSubProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg) {
  case WM_SETFOCUS:
    if (mp2)
      PostMsg(WinQueryWindow(hwnd, QW_PARENT), UM_FOCUSME, mp1, mp2);
    break;
  }
  if (oldproc)
    return oldproc(hwnd, msg, mp1, mp2);
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY IniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  INIDATA *inidata;
  SHORT sSel;
  PFNWP oldproc;

  switch (msg) {
  case WM_CREATE:
    inidata = xmallocz(sizeof(INIDATA), pszSrcFile, __LINE__);
    if (!inidata) {
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      break;
    }
    inidata->size = sizeof(INIDATA);
    inidata->confirm = TRUE;
    inidata->currid = INI_APPLIST;
    inidata->hwndMenu = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					FID_MENU);
    inidata->hwndIni = hwnd;
    WinSetWindowPtr(hwnd, QWL_USER, (PVOID) inidata);
    WinCheckMenuItem(inidata->hwndMenu, INI_CONFIRM, inidata->confirm);

    if (!WinCreateWindow
	(hwnd, WC_LISTBOX, (PSZ) NULL,
	 WS_VISIBLE | LS_HORZSCROLL | LS_NOADJUSTPOS, 0, 0, 0, 0, hwnd,
	 HWND_TOP, INI_APPLIST, NULL, NULL)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                PCSZ_WINCREATEWINDOW);
    }
    if (!WinCreateWindow
	(hwnd, WC_LISTBOX, (PSZ) NULL,
	 WS_VISIBLE | LS_HORZSCROLL | LS_NOADJUSTPOS, 0, 0, 0, 0, hwnd,
	 HWND_TOP, INI_KEYLIST, NULL, NULL)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                PCSZ_WINCREATEWINDOW);
    }
    if (!WinCreateWindow
	(hwnd, WC_LISTBOX, (PSZ) NULL,
	 WS_VISIBLE | LS_HORZSCROLL | LS_NOADJUSTPOS, 0, 0, 0, 0, hwnd,
	 HWND_TOP, INI_DATALIST, NULL, NULL)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                PCSZ_WINCREATEWINDOW);
    }
    if (!WinCreateWindow(hwnd, WC_STATIC, (PSZ) NULL, WS_VISIBLE | SS_TEXT |
			 DT_CENTER | DT_VCENTER, 0, 0, 0, 0, hwnd, HWND_TOP,
			 INI_NUMAPPS, NULL, NULL)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                PCSZ_WINCREATEWINDOW);
    }
    if (!WinCreateWindow(hwnd, WC_STATIC, (PSZ) NULL, WS_VISIBLE | SS_TEXT |
			 DT_CENTER | DT_VCENTER, 0, 0, 0, 0, hwnd, HWND_TOP,
			 INI_NUMKEYS, NULL, NULL)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                PCSZ_WINCREATEWINDOW);
    }
    if (!WinCreateWindow(hwnd, WC_STATIC, (PSZ) NULL, WS_VISIBLE | SS_TEXT |
			 DT_CENTER | DT_VCENTER, 0, 0, 0, 0, hwnd, HWND_TOP,
			 INI_NUMDATA, NULL, NULL)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                PCSZ_WINCREATEWINDOW);
    }
    if (!WinCreateWindow(hwnd,
			 WC_STATIC,
			 GetPString(IDS_APPLICATIONSTITLETEXT),
			 WS_VISIBLE | SS_TEXT |
			 DT_CENTER | DT_VCENTER,
			 0,
			 0, 0, 0, hwnd, HWND_TOP, INI_APPHDR, NULL, NULL)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                PCSZ_WINCREATEWINDOW);
    }
    if (!WinCreateWindow(hwnd,
			 WC_STATIC,
			 GetPString(IDS_KEYWORDSTITLETEXT),
			 WS_VISIBLE | SS_TEXT |
			 DT_CENTER | DT_VCENTER,
			 0,
			 0, 0, 0, hwnd, HWND_TOP, INI_KEYHDR, NULL, NULL)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                PCSZ_WINCREATEWINDOW);
    }
    if (!WinCreateWindow(hwnd,
			 WC_STATIC,
			 GetPString(IDS_DATABYTESTITLETEXT),
			 WS_VISIBLE | SS_TEXT |
			 DT_CENTER | DT_VCENTER,
			 0,
			 0, 0, 0, hwnd, HWND_TOP, INI_DATAHDR, NULL, NULL)) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                PCSZ_WINCREATEWINDOW);
    }

    inidata->hwndApp = WinWindowFromID(hwnd, INI_APPLIST);
    inidata->hwndKey = WinWindowFromID(hwnd, INI_KEYLIST);
    inidata->hwndData = WinWindowFromID(hwnd, INI_DATALIST);

    oldproc = WinSubclassWindow(WinWindowFromID(hwnd, INI_APPLIST),
				IniLBSubProc);
    WinSetWindowPtr(WinWindowFromID(hwnd, INI_APPLIST),
		    QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(WinWindowFromID(hwnd, INI_KEYLIST),
				IniLBSubProc);
    WinSetWindowPtr(WinWindowFromID(hwnd, INI_KEYLIST),
		    QWL_USER, (PVOID) oldproc);
    oldproc = WinSubclassWindow(WinWindowFromID(hwnd, INI_DATALIST),
				IniLBSubProc2);
    WinSetWindowPtr(WinWindowFromID(hwnd, INI_DATALIST),
		    QWL_USER, (PVOID) oldproc);
    break;

  case UM_FOCUSME:
  case WM_SETFOCUS:
    if (mp2) {
      if (hwndMain && fAutoView)
	PostMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else if (hwndStatus) {
	if (*inidata->ininame) {
	  WinSetWindowText(hwndStatus, GetPString(IDS_INTERNALINIVIEWERTEXT));
	  if (hwndStatus2)
	    WinSetWindowText(hwndStatus2, inidata->ininame);
	}
	else
	  PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      }
    }
    if (msg == WM_SETFOCUS)
      break;
    return 0;

  case UM_SELECT:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, INI_APPLIST));
    break;

  case UM_ACTION:
    SetPresParams(WinWindowFromID(hwnd, INI_APPHDR),
		  &RGBGREY,
		  &RGBBLACK, &RGBBLACK, FNT_8HELVETICABOLD);
    SetPresParams(WinWindowFromID(hwnd, INI_KEYHDR),
		  &RGBGREY,
		  &RGBBLACK, &RGBBLACK, FNT_8HELVETICABOLD);
    SetPresParams(WinWindowFromID(hwnd, INI_DATAHDR),
		  &RGBGREY,
		  &RGBBLACK, &RGBBLACK, FNT_8HELVETICABOLD);
    SetPresParams(WinWindowFromID(hwnd, INI_NUMAPPS),
		  &RGBGREY,
		  &RGBBLACK, &RGBBLACK, FNT_8HELVETICABOLD);
    SetPresParams(WinWindowFromID(hwnd, INI_NUMKEYS),
		  &RGBGREY,
		  &RGBBLACK, &RGBBLACK, FNT_8HELVETICABOLD);
    SetPresParams(WinWindowFromID(hwnd, INI_NUMDATA),
		  &RGBGREY,
		  &RGBBLACK, &RGBBLACK, FNT_8HELVETICABOLD);
    if (!WinSetPresParam(WinWindowFromID(hwnd, INI_DATALIST),
			 PP_FONTNAMESIZE,
			 strlen(FNT_10SYSTEMMONOTEXT) + 1,
			 FNT_10SYSTEMMONOTEXT))
      WinSetPresParam(WinWindowFromID(hwnd, INI_DATALIST),
		      PP_FONTNAMESIZE,
		      strlen(FNT_10SYSTEMMONOTEXT) + 1,
		      FNT_10SYSTEMMONOTEXT);
    RestorePresParams(hwnd, "IniUtil");
    return 0;

  case WM_PRESPARAMCHANGED:
    PresParamChanged(hwnd, "IniUtil", mp1, mp2);
    break;

  case UM_RESCAN:
    inidata = INSTDATA(hwnd);
    if (!inidata)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else if (mp1) {
      SHORT sSelect;
      BOOL inprofile;
      ULONG size;
      SHORT numitems = (SHORT)WinSendDlgItemMsg(hwnd,
						INI_APPLIST,
						LM_QUERYITEMCOUNT,
						MPVOID, MPVOID);
      if (!numitems) {
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(INI_REFRESH, 0), MPVOID);
	return 0;
      }
      size = 0;
      if (PrfQueryProfileSize(inidata->hini,
			      (CHAR *)mp1, NULL, (PULONG) & size) && size)
	inprofile = TRUE;
      else
	inprofile = FALSE;
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					  INI_APPLIST,
					  LM_SEARCHSTRING,
					  MPFROM2SHORT(LSS_CASESENSITIVE,
						       LIT_FIRST),
					  MPFROMP((CHAR *)mp1));
      if (sSelect >= 0) {
	if (!inprofile)
	  WinSendDlgItemMsg(hwnd,
			    INI_APPLIST,
			    LM_DELETEITEM, MPFROMSHORT(sSelect), MPVOID);
	else if (!strcmp(inidata->applname, (CHAR *)mp1))
	  PostMsg(hwnd,
		  WM_CONTROL,
		  MPFROM2SHORT(INI_APPLIST, LN_SELECT),
		  MPFROMLONG(WinWindowFromID(hwnd, INI_APPLIST)));
      }
      else if (inprofile)
	WinSendDlgItemMsg(hwnd,
			  INI_APPLIST,
			  LM_INSERTITEM,
			  MPFROM2SHORT(LIT_SORTASCENDING, 0),
			  MPFROMP((CHAR *)mp1));
      if (mp2 && inidata->applname) {
	if (!EnumKeyNames(WinWindowFromID(hwnd, INI_KEYLIST),
			  inidata->hini, inidata->applname))
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(INI_REFRESH, 0), MPVOID);
      }
    }
    return 0;

  case UM_INITIALSIZE:                  /* kludge */
    inidata = INSTDATA(hwnd);
    if (!inidata)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else
      inidata->dontclose = TRUE;
    return 0;

  case WM_SIZE:
    {
      SHORT cx = SHORT1FROMMP(mp2);
      SHORT cy = SHORT2FROMMP(mp2);

      WinSetWindowPos(WinWindowFromID(hwnd, INI_APPHDR), HWND_TOP,
		      2, cy - 22, (cx / 4) - 4, 20, SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, INI_NUMAPPS), HWND_TOP,
		      (cx / 4) + 2, cy - 22, (cx / 4) - 4, 20,
		      SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, INI_APPLIST), HWND_TOP,
		      2, (cy / 2) + 2, (cx / 2) - 4, (cy / 2) - 28,
		      SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, INI_KEYHDR), HWND_TOP,
		      (cx / 2) + 2, cy - 22, (cx / 4) - 4, 20,
		      SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, INI_NUMKEYS), HWND_TOP,
		      (cx - (cx / 4)) + 2, cy - 22, (cx / 4) - 4, 20,
		      SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, INI_KEYLIST), HWND_TOP,
		      (cx / 2) + 2, (cy / 2) + 2, (cx / 2) - 4, (cy / 2) - 28,
		      SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, INI_DATAHDR), HWND_TOP,
		      2, (cy / 2) - 22, (cx / 2) - 4, 20,
		      SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, INI_NUMDATA), HWND_TOP,
		      (cx / 2) + 2, (cy / 2) - 22, (cx / 2) - 4, 20,
		      SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, INI_DATALIST), HWND_TOP,
		      2, 2, cx - 4, (cy / 2) - 28, SWP_MOVE | SWP_SIZE);
    }
    break;

  case UM_LOADFILE:
    /* load initial file */
    inidata = INSTDATA(hwnd);
    if (!inidata)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else {
      if (mp1) {
	strcpy(inidata->ininame, (CHAR *)mp1);
	inidata->hini = PrfOpenProfile(WinQueryAnchorBlock(hwnd),
				       inidata->ininame);
	free(mp1);
      }
      else
	inidata->hini = HINI_USERPROFILE;
      if (inidata->hini == NULLHANDLE) {
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		  "PrfOpenProfile failed for %s", inidata->ininame);
      }
      else {
	WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT),
			HWND_TOP,
			0, 0, 0, 0, SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER);
	EnumAppNames(WinWindowFromID(hwnd, INI_APPLIST), inidata->hini);
	return 0;
      }
    }
    PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    return 0;

  case WM_ERASEBACKGROUND:
    WinFillRect((HPS) mp1, (PRECTL) mp2, 0x00d0d0d0);
    return 0;

  case WM_PAINT:
    {
      HPS hps;
      RECTL rcl;

      hps = WinBeginPaint(hwnd, (HPS) 0, NULL);
      if (hps) {
	WinQueryWindowRect(hwnd, &rcl);
	WinFillRect(hps, &rcl, CLR_PALEGRAY);
	WinEndPaint(hps);
	/*
	 * tell status window to paint its box
	 */
	PaintRecessedWindow(WinWindowFromID(hwnd, INI_APPHDR),
			    (HPS) 0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, INI_KEYHDR),
			    (HPS) 0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, INI_DATAHDR),
			    (HPS) 0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, INI_NUMAPPS),
			    (HPS) 0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, INI_NUMKEYS),
			    (HPS) 0, FALSE, FALSE);
	PaintRecessedWindow(WinWindowFromID(hwnd, INI_NUMDATA),
			    (HPS) 0, FALSE, FALSE);
	inidata = INSTDATA(hwnd);
	if (!inidata)
	  Runtime_Error(pszSrcFile, __LINE__, NULL);
	else if (inidata->hwndCurrent)
	  BoxWindow(inidata->hwndCurrent, (HPS) 0, CLR_RED);
      }
    }
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case INI_KEYLIST:
    case INI_APPLIST:
    case INI_DATALIST:
      switch (SHORT2FROMMP(mp1)) {
      case LN_SETFOCUS:
	inidata = INSTDATA(hwnd);
	if (!inidata)
	  Runtime_Error(pszSrcFile, __LINE__, NULL);
	else {
	  if (inidata->hwndCurrent)
	    BoxWindow(inidata->hwndCurrent, (HPS) 0, CLR_PALEGRAY);
	  inidata->hwndCurrent = WinWindowFromID(hwnd, SHORT1FROMMP(mp1));
	  BoxWindow(WinWindowFromID(hwnd,
				    SHORT1FROMMP(mp1)), (HPS) 0, CLR_RED);
	}
	break;

      case LN_ENTER:
      case LN_SELECT:
	{
	  CHAR applname[CCHMAXPATH], keyname[CCHMAXPATH];
	  SHORT sSelect;

	  if (SHORT1FROMMP(mp1) == INI_DATALIST)
	    break;

	  inidata = INSTDATA(hwnd);
	  if (!inidata || !inidata->hini) {
	    Runtime_Error(pszSrcFile, __LINE__, NULL);
	    break;
	  }
	  WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	  inidata->keyindex = inidata->appindex = 0;
	  *applname = *keyname = *inidata->applname = *inidata->keyname = 0;
	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      INI_APPLIST,
					      LM_QUERYSELECTION,
					      MPFROM2SHORT(LIT_FIRST, 0),
					      MPVOID);
	  if (sSelect < 0)
	    break;
	  inidata->appindex = sSelect;
	  WinSendDlgItemMsg(hwnd,
			    INI_APPLIST,
			    LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, CCHMAXPATH - 1),
			    MPFROMP(applname));
	  if (SHORT1FROMMP(mp1) == INI_APPLIST) {
	    if (!EnumKeyNames(WinWindowFromID(hwnd, INI_KEYLIST),
			      inidata->hini, applname))
	      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(INI_REFRESH, 0), MPVOID);
	  }
	  else {
	    sSelect = (SHORT) WinSendDlgItemMsg(hwnd, INI_KEYLIST,
						LM_QUERYSELECTION,
						MPFROM2SHORT(LIT_FIRST, 0),
						MPVOID);
	    if (sSelect < 0)
	      break;
	    inidata->keyindex = sSelect;
	    WinSendDlgItemMsg(hwnd,
			      INI_KEYLIST,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect, CCHMAXPATH - 1),
			      MPFROMP(keyname));
	    if (!*keyname || !*applname)
	      break;
	    strcpy(inidata->keyname, keyname);
	    strcpy(inidata->applname, applname);
	    xfree(inidata->data, pszSrcFile, __LINE__);
	    inidata->data = GetKeyData(WinWindowFromID(hwnd,
						       INI_DATALIST),
				       inidata->hini,
				       applname, keyname, &inidata->datalen);
	    if (!inidata->data)
	      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(INI_REFRESH, 0), MPVOID);
	  }
	}
	break;
      }
      break;
    }
    return 0;

  case UM_SETDIR:
    {
      SHORT num;
      CHAR s[81];

      num = (SHORT) WinSendDlgItemMsg(hwnd,
				      INI_APPLIST,
				      LM_QUERYITEMCOUNT, MPVOID, MPVOID);
      sprintf(s, "%u", num);
      WinSetDlgItemText(hwnd, INI_NUMAPPS, s);
      num = (SHORT) WinSendDlgItemMsg(hwnd,
				      INI_KEYLIST,
				      LM_QUERYITEMCOUNT, MPVOID, MPVOID);
      sprintf(s, "%u", num);
      WinSetDlgItemText(hwnd, INI_NUMKEYS, s);
      WinSetDlgItemText(hwnd, INI_NUMDATA, "0");
      WinSendDlgItemMsg(hwnd, INI_DATALIST, LM_DELETEALL, MPVOID, MPVOID);
      // inidata = WinQueryWindowPtr(hwnd, QWL_USER);   // 09 Jan 08 SHL
    }
    return 0;

  case UM_SETUP:
    inidata = INSTDATA(hwnd);
    if (!inidata)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else {
      PRFPROFILE prfp;
      CHAR sysini[CCHMAXPATH + 81];
      CHAR userini[CCHMAXPATH];

      memset(&prfp, 0, sizeof(PRFPROFILE));
      prfp.cchUserName = CCHMAXPATH;
      prfp.cchSysName = CCHMAXPATH;
      prfp.pszUserName = (PSZ) userini;
      prfp.pszSysName = (PSZ) sysini;
      if (PrfQueryProfile(WinQueryAnchorBlock(hwnd), &prfp)) {
	if (inidata->hini == HINI_SYSTEMPROFILE)
	  strcpy(inidata->ininame, sysini);
	else if (inidata->hini == HINI_USERPROFILE)
	  strcpy(inidata->ininame, userini);
	sprintf(sysini, GetPString(IDS_INITITLETEXT), inidata->ininame);
	WinSetWindowText(WinQueryWindow(hwnd, QW_PARENT), sysini);
	if (WinQueryWindow(hwnd, QW_PARENT) ==
	    WinQueryActiveWindow(WinQueryWindow
				 (WinQueryWindow(hwnd, QW_PARENT),
				  QW_PARENT))) {
	  if (hwndStatus)
	    WinSetWindowText(hwndStatus,
			     GetPString(IDS_INTERNALINIVIEWERTEXT));
	  if (hwndStatus2)
	    WinSetWindowText(hwndStatus2, inidata->ininame);
	}
      }
      else {
        CHAR s[100];

        sprintf(s, PCSZ_INIQUERYPRFTEXT, GetPString(IDS_FAILEDTEXT));
        WinSetWindowText(WinQueryWindow(hwnd, QW_PARENT), s);
      }
      return 0;
    }

    PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    return 0;

  case WM_CHAR:
    if (!(SHORT1FROMMP(mp1) & KC_KEYUP)) {
      if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY) {
	switch (SHORT2FROMMP(mp2)) {
	case VK_DELETE:
	  inidata = INSTDATA(hwnd);
	  if (!inidata)
	    Runtime_Error(pszSrcFile, __LINE__, NULL);
	  else {
	    SHORT cmd = inidata->hwndCurrent &&
			WinQueryWindowUShort(inidata->hwndCurrent, QWS_ID) ==
			 INI_APPLIST ?
			INI_DELETEAPP :INI_DELETEKEY;
	    PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(cmd, 0), MPVOID);
	  }
	  break;
	}
      }
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_FINDNEXT:
    case IDM_FINDFIRST:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else if (inidata->hwndCurrent) {
	STRINGINPARMS sip;
	static CHAR tofind[258] = "";
	SHORT x, z;

	if (SHORT1FROMMP(mp1) == IDM_FINDFIRST || !*tofind) {
	  z = LIT_FIRST;
	  memset(&sip, 0, sizeof(sip));
	  sip.help = GetPString(IDS_INISEARCHHELPTEXT);
	  sip.prompt = GetPString(IDS_INISEARCHPROMPTTEXT);
	  sip.inputlen = 257;
	  sip.ret = tofind;
	  sip.title = GetPString(IDS_INISEARCHTITLETEXT);
	  if (WinDlgBox(HWND_DESKTOP,
			hwnd,
			InputDlgProc, FM3ModHandle, STR_FRAME, &sip)) {
	    rstrip(tofind);
	    if (!*tofind) {
              if (!fAlertBeepOff)
	        DosBeep(50, 100);
	      break;
	    }
	  }
	  else
	    break;
	}
	else {
	  z = (SHORT) WinSendMsg(inidata->hwndCurrent,
				 LM_QUERYSELECTION,
				 MPFROM2SHORT(LIT_CURSOR, 0), MPVOID);
	  if (z < 0)
	    z = LIT_FIRST;
	}
	x = (SHORT) WinSendMsg(inidata->hwndCurrent,
			       LM_SEARCHSTRING,
			       MPFROM2SHORT(LSS_SUBSTRING, z),
			       MPFROMP(tofind));
	if (x >= 0 && x > z) {
	  WinSendMsg(inidata->hwndCurrent,
		     LM_SETTOPINDEX, MPFROM2SHORT(x, 0), MPVOID);
	  WinSendMsg(inidata->hwndCurrent,
		     LM_SELECTITEM,
		     MPFROM2SHORT(x, 0), MPFROM2SHORT(TRUE, 0));
	}
	else if (!fAlertBeepOff)
	  DosBeep(250, 100);
      }
      break;

    case IDM_FILTER:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	SHORT numitems = (SHORT)WinSendDlgItemMsg(hwnd,
						  INI_APPLIST,
						  LM_QUERYITEMCOUNT,
						  MPVOID, MPVOID);
	if (numitems)
	  WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    FilterIniProc, FM3ModHandle, IAF_FRAME, (PVOID) inidata);
	else if (!fAlertBeepOff)
	  DosBeep(50, 100);
      }
      break;

    case IDM_COPY:
    case IDM_INFO:
    case IDM_MOVE:
      inidata = INSTDATA(hwnd);
      if (!inidata || !*inidata->ininame)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else if (mp2) {
	INIREC *inirec = xmallocz(sizeof(INIREC), pszSrcFile, __LINE__);
	if (inirec) {
	  *inirec = *(INIREC *) mp2;
	  inirec->hwndDlg = hwnd;
	  inirec->confirm = inidata->confirm;
	  strcpy(inirec->filename1, inidata->ininame);
	  if (xbeginthread(CopyIniThread,
			   122880,
			   inirec,
			   pszSrcFile,
			   __LINE__) == -1)
	  {
	    free(inirec);
	  }
	}
      }
      break;

    case IDM_COMPARE:
      inidata = INSTDATA(hwnd);
      if (!inidata || !*inidata->ininame)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else if (mp2) {
	INIREC *inirec = xmalloc(sizeof(INIREC), pszSrcFile, __LINE__);
	if (inirec) {
	  strcpy(inirec->filename2, (CHAR *)(mp2));
	  strcpy(inirec->filename1, inidata->ininame);
	  inirec->hwndDlg = hwnd;
	  if (xbeginthread(CompareIniThread,
			   122880,
			   inirec,
			   pszSrcFile,
			   __LINE__) == -1)
	  {
	    free(inirec);
	  }
	}
      }
      break;

    case INI_COPYAPP:
    case INI_COPYKEY:
    case INI_RENAMEAPP:
    case INI_RENAMEKEY:
      inidata = INSTDATA(hwnd);
      if (!inidata) {
	Runtime_Error(pszSrcFile, __LINE__, NULL);
	break;
      }
      if (!*inidata->ininame ||
	  !*inidata->applname ||
	  (!*inidata->keyname &&
	   (SHORT1FROMMP(mp1) == INI_RENAMEKEY ||
	    SHORT1FROMMP(mp1) == INI_COPYKEY)))
	break;
      {
	INIREC *inirec = xmallocz(sizeof(INIREC), pszSrcFile, __LINE__);
	if (inirec) {
	  inirec->size = sizeof(INIREC);
	  inirec->hwndDlg = hwnd;
	  inirec->confirm = inidata->confirm;
	  inirec->action = (SHORT1FROMMP(mp1) == INI_COPYAPP ||
			    SHORT1FROMMP(mp1) == INI_COPYKEY) ?
	    IDM_COPY : IDM_MOVE;
	  strcpy(inirec->filename1, inidata->ininame);
	  strcpy(inirec->filename2, inidata->ininame);
	  if (SHORT1FROMMP(mp1) == INI_COPYKEY ||
	      SHORT1FROMMP(mp1) == INI_RENAMEKEY)
	    strcpy(inirec->key, inidata->keyname);
	  strcpy(inirec->app, inidata->applname);
	  if (!WinDlgBox(HWND_DESKTOP,
			 hwnd,
			 IntraIniProc,
			 FM3ModHandle, INII_FRAME, (PVOID) inirec)) {
	    free(inirec);
	    break;
	  }
	  if (xbeginthread(CopyIniThread,
			   122880,
			   inirec,
			   pszSrcFile,
			   __LINE__) == -1)
	  {
	    free(inirec);
	  }
	}
      }
      break;

    case INI_BACKUPINI:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else if (*inidata->ininame) {
	// 09 Jan 08 SHL fixme to complain
	CHAR filename[CCHMAXPATH], *p;

	strcpy(filename, inidata->ininame);
	p = filename;
	while (*p) {
	  if (*p == '/')
	    *p = '\\';
	  p++;
	}
	p = strrchr(filename, '\\');
	if (p) {
	  p++;
	  *p = 0;
	}
	else
	  *filename = 0;
	strcat(filename, PCSZ_STARDOTINI);
	if (export_filename(hwnd, filename, TRUE)) {

	  PPRFPROFILE prfp;

	  prfp = xmallocz(sizeof(PRFPROFILE), pszSrcFile, __LINE__);
	  if (prfp) {
	    prfp->pszUserName =
	      xstrdup(inidata->ininame, pszSrcFile, __LINE__);
	    if (!prfp->pszUserName)
	      free(prfp);
	    else {
	      prfp->cchUserName = strlen(prfp->pszUserName);
	      prfp->pszSysName = xstrdup(filename, pszSrcFile, __LINE__);
	      if (!prfp->pszSysName) {
		free(prfp->pszUserName);
		free(prfp);
	      }
	      else {
		prfp->cchSysName = strlen(prfp->pszSysName);
		if (xbeginthread(BackupIniThread,
				 122880,
				 prfp,
				 pszSrcFile,
				 __LINE__) == -1)
		{
		  free(prfp->pszSysName);
		  free(prfp->pszUserName);
		  free(prfp);
		}
		else
		  DosSleep(100); //05 Aug 07 GKY 250
	      }
	    }
	  }
	}
      }
      break;

    case INI_CHANGEINI:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	if (WinDlgBox(HWND_DESKTOP,
		      hwnd,
		      ChangeIniProc,
		      FM3ModHandle, INIR_FRAME, (PVOID) inidata->ininame)) {
	  WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	  WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	  EnumAppNames(WinWindowFromID(hwnd, INI_APPLIST), inidata->hini);
	}
      }
      break;

    case INI_SWAPINI:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	if (WinDlgBox(HWND_DESKTOP,
		      hwnd,
		      SwapIniProc,
		      FM3ModHandle, INIR_FRAME, (PVOID) inidata->ininame)) {
	  WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	  WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	  EnumAppNames(WinWindowFromID(hwnd, INI_APPLIST), inidata->hini);
	}
      }
      break;

    case IDM_NEXTWINDOW:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	inidata->currid++;
	if (inidata->currid > INI_DATALIST)
	  inidata->currid = INI_APPLIST;
      }
      WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, inidata->currid));
      break;

    case IDM_PREVWINDOW:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	inidata->currid--;
	if (inidata->currid < INI_APPLIST)
	  inidata->currid = INI_DATALIST;
      }
      WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, inidata->currid));
      break;

    case INI_CONFIRM:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	inidata->confirm = (inidata->confirm) ? FALSE : TRUE;
	WinCheckMenuItem(inidata->hwndMenu, INI_CONFIRM, inidata->confirm);

      }
      break;

    case INI_EDITENTRY:
    case INI_ADDENTRY:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	inidata->edit = (SHORT1FROMMP(mp1) == INI_EDITENTRY);
	WinDlgBox(HWND_DESKTOP,
		  hwnd, AddIniProc, FM3ModHandle, IAD_FRAME, (PVOID) inidata);
      }
      break;

    case INI_OTHERPROFILE:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	CHAR filename[CCHMAXPATH + 81], *p;
	FILESTATUS3 fsa;
	HINI hINI;

	strcpy(filename, inidata->ininame);
	p = filename;
	while (*p) {
	  if (*p == '/')
	    *p = '\\';
	  p++;
	}
	p = strrchr(filename, '\\');
	if (p) {
	  p++;
	  *p = 0;
	}
	else
	  *filename = 0;
	strcat(filename, PCSZ_STARDOTINI);
	if (insert_filename(hwnd,
			    filename,
			    TRUE,
			    TRUE) &&
	    *filename &&
	    !DosQueryPathInfo(filename, FIL_STANDARD, &fsa, sizeof(fsa))) {
	  hINI = PrfOpenProfile(WinQueryAnchorBlock(hwnd), filename);
	  if (hINI == NULLHANDLE) {
	    Win_Error(hwnd, hwnd, __FILE__, __LINE__,
		      GetPString(IDS_INICANTOPENINITEXT), filename);
	  }
	  else {
	    if (*inidata->ininame) {
	      if (inidata->hini)
		CloseProfile(inidata->hini, FALSE);
	      *inidata->ininame = 0;
	    }
	    inidata->hini = hINI;
	    strcpy(inidata->ininame, filename);
	    WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	    WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	    EnumAppNames(WinWindowFromID(hwnd, INI_APPLIST), inidata->hini);
	  }
	}
      }
      break;

    case INI_USERPROFILE:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	if (inidata->hini && *inidata->ininame)
	  CloseProfile(inidata->hini, FALSE);
	*inidata->ininame = 0;
	inidata->hini = HINI_USERPROFILE;
	WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	EnumAppNames(WinWindowFromID(hwnd, INI_APPLIST), inidata->hini);
      }
      break;

    case INI_SYSTEMPROFILE:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	if (inidata->hini && *inidata->ininame)
	  CloseProfile(inidata->hini, FALSE);
	*inidata->ininame = 0;
	inidata->hini = HINI_SYSTEMPROFILE;
	WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	EnumAppNames(WinWindowFromID(hwnd, INI_APPLIST), inidata->hini);
      }
      break;

    case INI_REFRESH:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	EnumAppNames(WinWindowFromID(hwnd, INI_APPLIST), inidata->hini);
      }
      break;

    case INI_DELETEKEY:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else if (*inidata->applname && *inidata->keyname) {

	SHORT keyindex = inidata->keyindex;

	if (inidata->confirm && saymsg(MB_YESNO,
				       hwnd,
				       GetPString(IDS_CONFIRMTEXT),
				       GetPString(IDS_INIDELETEKEYTEXT),
				       inidata->keyname) == MBID_NO)
	  break;
	PrfWriteProfileData(inidata->hini,
			    inidata->applname, inidata->keyname, NULL, 0L);
	*inidata->keyname = 0;
	WinSendDlgItemMsg(hwnd,
			  INI_KEYLIST,
			  LM_DELETEITEM, MPFROM2SHORT(keyindex, 0), MPVOID);
	WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	sSel = (SHORT) WinSendDlgItemMsg(hwnd,
					 INI_KEYLIST,
					 LM_QUERYITEMCOUNT, MPVOID, MPVOID);
	if (!sSel)
	  WinSendMsg(hwnd, INI_DELETEAPP, MPVOID, MPVOID);
	else {
	  sSel = min(keyindex, sSel - 1);
	  WinSendDlgItemMsg(hwnd,
			    INI_KEYLIST,
			    LM_SELECTITEM,
			    MPFROMSHORT(sSel), MPFROMSHORT(TRUE));
	}
      }
      break;

    case INI_DELETEAPP:
      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      // 09 Jan 08 SHL fixme to complain?
      else if (*inidata->applname) {

	SHORT appindex = inidata->appindex;

	if (inidata->confirm && saymsg(MB_YESNO,
				       hwnd,
				       GetPString(IDS_CONFIRMTEXT),
				       GetPString(IDS_INIDELETEAPPTEXT),
				       inidata->applname) == MBID_NO)
	  break;
	PrfWriteProfileData(inidata->hini, inidata->applname, NULL, NULL, 0L);
	*inidata->applname = *inidata->keyname = 0;
	WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	WinSendDlgItemMsg(hwnd, INI_KEYLIST, LM_DELETEALL, MPVOID, MPVOID);
	WinSendDlgItemMsg(hwnd,
			  INI_APPLIST,
			  LM_DELETEITEM, MPFROM2SHORT(appindex, 0), MPVOID);
	WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	sSel = (SHORT) WinSendDlgItemMsg(hwnd,
					 INI_APPLIST,
					 LM_QUERYITEMCOUNT, MPVOID, MPVOID);
	if (sSel) {
	  sSel = min(appindex, sSel - 1);
	  WinSendDlgItemMsg(hwnd,
			    INI_APPLIST,
			    LM_SELECTITEM,
			    MPFROMSHORT(sSel), MPFROMSHORT(TRUE));
	}
      }
      break;

    case DID_OK:
    case DID_CANCEL:
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_INI, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case IDM_ABOUT:
      saymsg(MB_ENTER | MB_ICONASTERISK,
	     hwnd,
	     GetPString(IDS_VIEWABOUTTITLETEXT),
	     GetPString(IDS_INIABOUTTEXT));
      break;
    }
    return 0;

  case WM_CLOSE:
    if (ParentIsDesktop(WinQueryWindow(hwnd, QW_PARENT), (HWND) 0)) {

      SWP swp;

      WinQueryWindowPos(WinQueryWindow(hwnd, QW_PARENT), &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE)))
	WinStoreWindowPos(FM2Str,
			  "INIWindowPos", WinQueryWindow(hwnd, QW_PARENT));
      // inidata = WinQueryWindowPtr(hwnd, QWL_USER);   // 09 Jan 08 SHL
    }
    WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
    return 0;

  case WM_DESTROY:
    {
      BOOL dontclose = FALSE;

      inidata = INSTDATA(hwnd);
      if (!inidata)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	dontclose = inidata->dontclose;
	if (inidata->hini != NULLHANDLE && *inidata->ininame)
	  CloseProfile(inidata->hini, FALSE);
	free(inidata->data);
	if (inidata->hwndPopup)
	  WinDestroyWindow(inidata->hwndPopup);
	free(inidata);
      }
      if (!dontclose &&
	  ParentIsDesktop(hwnd, WinQueryWindow(WinQueryWindow(hwnd,
							      QW_PARENT),
					       QW_PARENT))) {
	if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
	  DosExit(EXIT_PROCESS, 1);
      }
    }
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(INIS,EnumAppNames,GetKeyData,EnumKeyNames,AddIniProc,IniProc,BackupIniThread)
#pragma alloc_text(INIS,ChangeIniProc,SwapIniProc,IniLBSubProc,IniLBSubProc2,CopyIniThread)
#pragma alloc_text(INIS,IntraIniProc,FilterIniProc)
#pragma alloc_text(STARTUP,StartIniEditor)
