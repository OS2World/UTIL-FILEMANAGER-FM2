
/***********************************************************************

  $Id$

  Drive tree container management

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005-2015 Steven H. Levine

  24 May 05 SHL Rework Win_Error usage
  25 May 05 SHL Rework for ProcessDirectory
  28 May 05 SHL Clean while reading code
  24 Oct 05 SHL Delete obsolete code
  22 Jul 06 SHL Check more run time errors
  19 Oct 06 SHL Stubby - correct . and .. detect
  22 Mar 07 GKY Use QWL_USER
  01 Aug 07 SHL Sync with CNRITEM mods
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Feb 08 GKY Use xfree where appropriate
  24 Nov 08 GKY remove redundant code and minor speed up of Stubby
  25 Dec 08 GKY Add ProcessDirectoryThread to allow optional recursive drive scan at startup.
  25 Dec 08 GKY Add DRIVE_RSCANNED flag to monitor for the first recursive drive scan per session
		to prevent duplicate directory names in tree following a copy before initial scan.
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  13 Dec 09 GKY Fixed separate paramenters. Please note that appname should be used in
		profile calls for user settings that work and are setable in more than one
		miniapp; FM3Str should be used for setting only relavent to FM/2 or that
		aren't user settable; realappname should be used for setting applicable to
		one or more miniapp but not to FM/2
  17 Jan 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  04 Aug 12 GKY Fix trap on close during drive scan
  02 Aug 15 GKY Fix trap in Stubby
  03 Aug 15 SHL Document Stubby a bit better
  07 Aug 15 SHL Rework to use AddFleshWorkRequest rather than direct calls to Stubby/Flesh/Unflesh

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <malloc.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "draglist.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3str.h"
#include "filldir.h"			// FileAttrToString...
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "flesh.h"
#include "valid.h"			// IsValidDir
#include "misc.h"			// LoadLibPath GetTidForThread
#include "findrec.h"			// FindCnrRecord
#include "notify.h"			// Notify
#include "wrappers.h"			// xfree
#include "excputil.h"			// xbeginthread
#include "listutil.h"			// List...
#include "common.h"			// IncrThreadUsage DecrThreadUsage

#if 0
#define  __PMPRINTF__
#include "PMPRINTF.H"
#endif

// Data definitions
#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;


static INT tidFleshWorkListThread = -1;	// 2015-08-08 SHL

static CHAR chFleshFocusDrive;	// 2015-08-16 SHL A..Z or 0


BOOL fNoFleshDbgMsg;	// 2015-08-09 SHL FIXME to be gone

#pragma data_seg(GLOBAL1)
ULONG NoBrokenNotify;
BOOL fFilesInTree;

BOOL Flesh(HWND hwndCnr, PCNRITEM pciParent);

BOOL Stubby(HWND hwndCnr, PCNRITEM pciParent);
BOOL FleshEnv(HWND hwndCnr, PCNRITEM pciParent);
BOOL UnFlesh(HWND hwndCnr, PCNRITEM pciParent);

/**
 * Insert CNRITEMs for members of PATH-like environment variable
 * @param hwndCnr is the container to be populated
 * @param pciParent is CNRITEM defining PATH-like environment variable
 * @return TRUE if OK, FALSE is error detected
 */

BOOL FleshEnv(HWND hwndCnr, PCNRITEM pciParent)
{
  PCNRITEM pciL;
  DIRCNRDATA *dcd;
  CHAR path[CCHMAXPATH + 12],
    fullpath[CCHMAXPATH + 12], *env, *p, *pp, *var = NULL;

  if (!pciParent || (INT) pciParent == -1 || !hwndCnr)
    return FALSE;

  dcd = (DIRCNRDATA *) WinQueryWindowPtr(hwndCnr, QWL_USER);
  if (!dcd)
    return FALSE;

  strcpy(path, pciParent->pszFileName + 1);
  if (stricmp(path, GetPString(IDS_ENVVARSTEXT) + 1))
    UnFlesh(hwndCnr, pciParent);
  if (*path) {
    path[strlen(path) - 1] = 0;
    if (!stricmp(path, PCSZ_LIBPATH)) {
      var = xmalloc(65536, pszSrcFile, __LINE__);
      if (var)
	LoadLibPath(var, 65536);
      env = var;
    }
    else
      env = getenv(path);
    if (env && *env) {
      p = env;
      while (*p) {
	pp = path;
	while (*p == ';')
	  p++;
	while (*p && *p != ';') {
	  *pp = *p;
	  p++;
	  pp++;
	}
	*pp = 0;
	if (*path &&
	    strcmp(path, ".") &&
	    strcmp(path, ".\\") &&
	    strcmp(path, "..") &&
	    strcmp(path, "..\\") &&
	    strncmp(path, ".\\", 2) && strncmp(path, "..\\", 3)) {
	  if (!DosQueryPathInfo(path,
				FIL_QUERYFULLNAME,
				fullpath,
				sizeof(fullpath)) && IsValidDir(fullpath)) {
	    pciL = FindCnrRecord(hwndCnr,
				 fullpath, pciParent, FALSE, FALSE, FALSE);
	    if (pciL) {
	      while (pciL && pciL != (PCNRITEM) - 1 && pciL != pciParent)
		pciL = WinSendMsg(hwndCnr,
				  CM_QUERYRECORD,
				  MPFROMP(pciL),
				  MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER));
	    }
	    if (!pciL) {

	      RECORDINSERT ri;

	      pciL = WinSendMsg(hwndCnr,
				CM_ALLOCRECORD,
				MPFROMLONG(EXTRA_RECORD_BYTES),
				MPFROMLONG(1));
	      if (pciL) {
		pciL->pszFileName = xstrdup(fullpath, pszSrcFile, __LINE__);
		pciL->rc.pszIcon = pciL->pszFileName;
		if (!fNoIconsDirs &&
		    (!isalpha(*fullpath) ||
		     !(driveflags[toupper(*fullpath) - 'A'] &
		       DRIVE_NOLOADICONS)))
		  pciL->rc.hptrIcon = WinLoadFileIcon(fullpath, FALSE);
		if (!pciL->rc.hptrIcon)
		  pciL->rc.hptrIcon = hptrDir;
		pciL->attrFile = FILE_DIRECTORY;
		pciL->pszDispAttr = FileAttrToString(pciL->attrFile);
		memset(&ri, 0, sizeof(ri));
		ri.cb = sizeof(ri);
		ri.pRecordOrder = (PRECORDCORE) CMA_END;
		ri.pRecordParent = (PRECORDCORE) pciParent;
		ri.zOrder = (ULONG) CMA_TOP;
		ri.cRecordsInsert = 1;
		ri.fInvalidateRecord = FALSE;
		if (!WinSendMsg(hwndCnr,
				CM_INSERTRECORD, MPFROMP(pciL), MPFROMP(&ri)))
		  FreeCnrItem(hwndCnr, pciL);
	      }
	    }
	  }
	}
      }
    }
    xfree(var, pszSrcFile, __LINE__);
    pciL = (PCNRITEM) WinSendMsg(hwndCnr,
				 CM_QUERYRECORD,
				 MPFROMP(pciParent),
				 MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    while (pciL && (INT) pciL != -1) {
      pciL->flags |= (RECFLAGS_NODRAG | RECFLAGS_UNDERENV);
      WinSendMsg(hwndCnr,
		 CM_INVALIDATERECORD, MPFROMP(&pciL), MPFROM2SHORT(1, 0));
      pciL = WinSendMsg(hwndCnr,
			CM_QUERYRECORD,
			MPFROMP(pciL), MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    }
  }
  return TRUE;
}

/**
 * Insert CNRITEMs for all children of pciParent
 * @param hwnCnr is container to receive CNRITEMs
 * @param pciParent is CNRITEM to have children inserted
 * @return TRUE if OK, FALSE is error detected
 */

BOOL Flesh(HWND hwndCnr, PCNRITEM pciParent)
{
  PCNRITEM pciL;
  DIRCNRDATA *dcd;
  BOOL includefiles;

  if (!pciParent || (INT)pciParent == -1 || !hwndCnr)
    return FALSE;

  // 2015-08-13 SHL FIXME to optimize
  if (fAmClosing) {
    // Can jiggle - see MainWndProc WM_CLOSE
    DosSleep(100);
    if (fAmClosing)
      return FALSE;			// time to die
  }

  // 2015-08-03 SHL FIXME debug
  if (!fNoFleshDbgMsg) {
    DbgMsg(pszSrcFile, __LINE__, "Flesh %s pciParent %p pszFileName %p",
	   pciParent && (INT)pciParent != -1 && pciParent->pszFileName ?
	     pciParent->pszFileName : "(null)",
	   pciParent,
	   pciParent && (INT)pciParent != -1 ? pciParent->pszFileName : (PVOID)-1); // 2015-08-03 SHL FIXME debug
  }

  pciL = (PCNRITEM)WinSendMsg(hwndCnr,
			      CM_QUERYRECORD,
			      MPFROMP(pciParent),
			      MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));

  // 2015-08-06 SHL allow pciL -1
  if (!pciL || (INT)pciL == -1 || !*pciL->pszFileName) {

    // No children or filename null
    // 2015-08-06 SHL FIXME to know how this can happen
    if (pciL && (INT)pciL != -1) {
      // 2015-08-06 SHL FIXME to loop and timeout if Stubby threads working
      if (!fNoFleshDbgMsg)
	DbgMsg(pszSrcFile, __LINE__, "Flesh RemoveCnrItems() pciL %p", pciL); // 2015-08-04 SHL FIXME debug
      RemoveCnrItems(hwndCnr, pciL, 1, CMA_FREE);
    }

    dcd = INSTDATA(hwndCnr);
    if (dcd && dcd->size != sizeof(DIRCNRDATA))
      dcd = NULL;

    // DbgMsg(pszSrcFile, __LINE__, "Flesh %s dcd %p called by %s#%u", pciParent->pszFileName, dcd, pszSrcFile_, uSrcLineNo_); // 2015-08-03 SHL FIXME debug

    // 2015-08-06 SHL FIXME to not trap here
    includefiles =
      driveflags[toupper(*pciParent->pszFileName) - 'A'] & DRIVE_INCLUDEFILES ?
	TRUE : fFilesInTree;

    ProcessDirectory(hwndCnr,
		     pciParent,
		     pciParent->pszFileName,
		     includefiles,	// filestoo
		     TRUE,		// recurse
		     TRUE,		// partial
		     NULL,		// stop flag
		     dcd,
		     NULL,		// total files
		     NULL);		// total bytes
    if (pciParent && pciParent->pszFileName && !(driveflags[toupper(*pciParent->pszFileName) - 'A'] & DRIVE_RSCANNED))
      driveflags[toupper(*pciParent->pszFileName) - 'A'] |= DRIVE_RSCANNED;
    return TRUE;
  }

  return FALSE;
}

// 2015-08-06 SHL FIXME to be VOID

BOOL UnFlesh(HWND hwndCnr, PCNRITEM pciParent)
{
  BOOL removed = FALSE;
  PCNRITEM pciL;

  if (!pciParent || !hwndCnr)
    return FALSE;

  if (!fNoFleshDbgMsg)
    DbgMsg(pszSrcFile, __LINE__, "UnFlesh pciParent %p pszFileName %s", pciParent, pciParent->pszFileName ? pciParent->pszFileName : "(null)"); // 2015-08-03 SHL FIXME debug

  for (;;) {
    pciL = (PCNRITEM) WinSendMsg(hwndCnr,
				 CM_QUERYRECORD,
				 MPFROMP(pciParent),
				 MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    if (!pciL || (INT)pciL == -1)
      break;			// Done

    if (!fNoFleshDbgMsg)
      DbgMsg(pszSrcFile, __LINE__, "UnFlesh RemoveCnrItems() pciL %p %s", pciL, pciL->pszFileName ? pciL->pszFileName : "(null)"); // 2015-08-03 SHL FIXME debug
    RemoveCnrItems(hwndCnr, pciL, 1, CMA_FREE);
    removed = TRUE;
  } // for

  if (removed) {
    WinSendMsg(hwndCnr,
	       CM_INVALIDATERECORD,
	       MPFROMP(&pciParent),
	       MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
  }
  return removed;
}

#define DDEPTH 64

/**
 * Insert CNRITEM for 1st subdirectory [or file] of pciParent
 * @param hwdCnr is container to be filled
 * @param pciParent is CNRITEM to receive child record
 * @return TRUE if record inserted, else FALSE
 * Ensures that expand/collapse button displays if directory has children
 * Secondary purpose is to detect broken LANs and inaccesible mapped drives
 */

BOOL Stubby(HWND hwndCnr, PCNRITEM pciParent)
{
  /**
   * this code is full of workarounds for screwed up LANs.
   * let's hope all the current LAN programmers fall into
   * a black hole and make way for people who can get it right...
   */

  BOOL ok = FALSE;
  FILEFINDBUF3 ffb[DDEPTH];
  PFILEFINDBUF3 pffb;
  HDIR hDir = HDIR_CREATE;
  ULONG nm, ulM = 1, total = 0, fl;
  CHAR str[CCHMAXPATH];
  register INT len;
  APIRET rc, prc;
  BOOL isadir = FALSE, isremote, includefiles;
  ULONG ddepth = DDEPTH;
  ULONG drvNum;
  ULONG flags;
  static BOOL brokenlan = FALSE, isbroken = FALSE;

  if (!pciParent || (INT) pciParent == -1 || !*pciParent->pszFileName
      || pciParent->pszFileName == NullStr || !hwndCnr)
    return FALSE;

  if (!fNoFleshDbgMsg)
    DbgMsg(pszSrcFile, __LINE__, "Stubby pciParent %p pszFileName %s", pciParent, pciParent->pszFileName); // 2015-08-03 SHL FIXME debug

  // Build wildcard
  len = strlen(pciParent->pszFileName);
  memcpy(str, pciParent->pszFileName, len + 1);
  if (str[len - 1] != '\\')
    str[len++] = '\\';
  str[len++] = '*';
  str[len] = 0;

  if (!isalpha(*str) || str[1] != ':' || str[2] != '\\')
    MakeFullName(str);

  drvNum = toupper(*pciParent->pszFileName) - 'A';
  flags = driveflags[drvNum];
  if (!isalpha(*str) ||
      str[1] != ':' ||
      str[2] != '\\' || ((flags & DRIVE_IGNORE)))
    return FALSE;			// Not a directory or ignore requested

  includefiles = flags & DRIVE_INCLUDEFILES ? TRUE : fFilesInTree;

  isremote = flags & DRIVE_REMOTE ? TRUE : FALSE;

  if (isremote) {
    if (fRemoteBug) {
      if (brokenlan) {
	ddepth = (ULONG) - 1;
	ddepth--;
      }
      ulM = 1;
    }
  }
  else if (isbroken)
    ddepth = 14;

  if (!fRemoteBug)
    ulM = ddepth <= DDEPTH ? ddepth : 1;

  nm = ulM;

  DosError(FERR_DISABLEHARDERR);

  fl = includefiles ? FILE_DIRECTORY : MUST_HAVE_DIRECTORY;

  rc = DosFindFirst(str,
		    &hDir,
		    FILE_NORMAL | fl |
		    FILE_READONLY | FILE_ARCHIVED |
		    FILE_SYSTEM | FILE_HIDDEN,
		    &ffb, ulM * sizeof(FILEFINDBUF3), &nm, FIL_STANDARD);
  if (ulM == 1 && !rc) {
    // Loop looking for 1st directory (or file)
    do {
      pffb = &ffb[0];
      if (!includefiles && !(pffb->attrFile & FILE_DIRECTORY) && !brokenlan) {
	// Find returned file when only directories requested
	brokenlan = TRUE;
	ddepth = (ULONG) - 1;
	ddepth--;
	if (!NoBrokenNotify) {
	  prc = saymsg(MB_YESNO | MB_ICONEXCLAMATION,
		       HWND_DESKTOP,
		       GetPString(IDS_LANERRORTITLETEXT),
		       GetPString(IDS_LANERRORTEXT));
	  if (prc == MBID_NO) {
	    saymsg(MB_ENTER,
		   HWND_DESKTOP,
		   GetPString(IDS_LANERROR2TITLETEXT),
		   GetPString(IDS_LANERROR2TEXT));
	    NoBrokenNotify = 255;
	    PrfWriteProfileData(fmprof,	FM3Str, "NoBrokenNotify",
				&NoBrokenNotify, sizeof(ULONG));
	  }
	}
	else {
	  NoBrokenNotify--;
	  PrfWriteProfileData(fmprof, FM3Str, "NoBrokenNotify",
			      &NoBrokenNotify, sizeof(ULONG));
	}
      }

      if (*pffb->achName &&
	  (includefiles || (pffb->attrFile & FILE_DIRECTORY)) &&
	  (pffb->achName[0] != '.' ||
	   (pffb->achName[1] &&
	    (pffb->achName[1] != '.' || pffb->achName[2])))) {
	// Got directory other than . or .. (or a file)
	DosFindClose(hDir);
	isadir = TRUE;
	goto Interruptus;
      }
      nm = 1;
      DosError(FERR_DISABLEHARDERR);
    } while (++total < ddepth && !(rc = (DosFindNext(hDir,
						     &ffb,
						     sizeof(FILEFINDBUF3),
						     &nm))));
    DosFindClose(hDir);

    if (toupper(*pciParent->pszFileName) > 'B' &&
	(*(pciParent->pszFileName + 1)) == ':' &&
	(*(pciParent->pszFileName + 2)) == '\\' && !(*(pciParent->pszFileName + 3))) {

      // Searching root of hard or remote drive and find reported error
      CHAR s[132];
      sprintf(s,
	      GetPString(IDS_NOSUBDIRSTEXT),
	      total, toupper(*pciParent->pszFileName));
      if (rc && rc != ERROR_NO_MORE_FILES)
	sprintf(&s[strlen(s)], GetPString(IDS_SEARCHERRORTEXT), rc, str);
      else if (ddepth < 16)
	brokenlan = TRUE;
      Notify(s);
    }
    goto None;				// Done
  }

  if (!rc) {
    DosFindClose(hDir);
    if (nm) {
      PBYTE fb = (PBYTE)&ffb[0];
      for (len = 0; len < nm; len++) {
	pffb = (PFILEFINDBUF3) fb;
	if (!includefiles && !(pffb->attrFile & FILE_DIRECTORY)) {
	  // Got file(s), but did not ask for files
	  if (!isbroken) {
	    isbroken = TRUE;
	    if (!NoBrokenNotify) {
	      prc = saymsg(MB_YESNO | MB_ICONEXCLAMATION,
			   HWND_DESKTOP,
			   GetPString(IDS_FSDERRORTITLETEXT),
			   GetPString(IDS_FSDERRORTEXT),
			   isremote ? GetPString(IDS_REMOTETEXT) :
				      GetPString(IDS_LOCALTEXT),
			   *str);
	      if (prc == MBID_NO) {
		saymsg(MB_ENTER,
		       HWND_DESKTOP,
		       GetPString(IDS_FSDERROR2TITLETEXT),
		       GetPString(IDS_FSDERROR2TEXT));
		NoBrokenNotify = 255;
		PrfWriteProfileData(fmprof, FM3Str, "NoBrokenNotify",
				    &NoBrokenNotify, sizeof(ULONG));
	      }
	    }
	    else {
	      NoBrokenNotify--;
	      PrfWriteProfileData(fmprof, FM3Str, "NoBrokenNotify",
				  &NoBrokenNotify, sizeof(ULONG));
	    }
	  } // if !broken
	} // if !directory

	if (*pffb->achName &&
	    (includefiles || (pffb->attrFile & FILE_DIRECTORY)) &&
	    ((pffb->achName[0] && pffb->achName[0] != '.') ||
	     (pffb->achName[1] &&
	      (pffb->achName[1] != '.' || pffb->achName[2]))))
	{
	  // Got directory other than . or .. (or a file)
	  isadir = TRUE;
	  break;
	}
	fb += pffb->oNextEntryOffset;
      } // for

    Interruptus:

      if (isadir) {

	// Insert CNRITEM for selected directory (or file)
	PCNRITEM pci;

	if (WinIsWindow((HAB)0, hwndCnr)) {
	  pci = WinSendMsg(hwndCnr,
			   CM_ALLOCRECORD,
			   MPFROMLONG(EXTRA_RECORD_BYTES), MPFROMLONG(1));
	  if (!pci) {
	    Win_Error(hwndCnr, HWND_DESKTOP, __FILE__, __LINE__,
		      GetPString(IDS_RECORDALLOCFAILEDTEXT));
	  }
	  else {
	    RECORDINSERT ri;
	    pci->pszFileName = NullStr;
	    pci->pszDisplayName = pci->pszFileName;
	    pci->rc.pszIcon = pci->pszDisplayName;
	    memset(&ri, 0, sizeof(RECORDINSERT));
	    ri.cb = sizeof(RECORDINSERT);
	    ri.pRecordOrder = (PRECORDCORE) CMA_END;
	    ri.pRecordParent = (PRECORDCORE) pciParent;
	    ri.zOrder = (ULONG) CMA_TOP;
	    ri.cRecordsInsert = 1;
	    ri.fInvalidateRecord = TRUE;
	    // DbgMsg(pszSrcFile, __LINE__, "Stubby CM_INSERTRECORD %p \"%s\" %.255s", pci, pci->pszFileName, pffb->achName); // 2015-08-03 SHL FIXME debug
	    if (!WinSendMsg(hwndCnr,
			    CM_INSERTRECORD, MPFROMP(pci), MPFROMP(&ri))) {
	      // Assume busy and try again
	      DosSleep(50); //05 Aug 07 GKY 100
	      WinSetFocus(HWND_DESKTOP, hwndCnr);
	      if (WinIsWindow((HAB)0, hwndCnr)) {
		if (!fNoFleshDbgMsg)
		  DbgMsg(pszSrcFile, __LINE__, "Stubby CM_INSERTRECORD pci %p pszFileName %s", pci, pci->pszFileName); // 2015-08-03 SHL FIXME debug
		if (!WinSendMsg(hwndCnr,
				CM_INSERTRECORD, MPFROMP(pci), MPFROMP(&ri))) {
		  Win_Error(hwndCnr, HWND_DESKTOP, __FILE__, __LINE__,
			    GetPString(IDS_RECORDINSERTFAILEDTEXT));
		  FreeCnrItem(hwndCnr, pci);
		}
		else
		  ok = TRUE;
	      }
	    }
	    else
	      ok = TRUE;
	  }
	}
      } // if isadir
      else if (toupper(*str) > 'B' && str[1] == ':' && str[2] == '\\' &&
	       !str[3]) {

	// Is root and no subdirectories
	CHAR s[162];
	sprintf(s,
		GetPString(IDS_NOSUBDIRS2TEXT),
		nm,
		toupper(*pciParent->pszFileName),
		isremote ? GetPString(IDS_NOSUBDIRS3TEXT) : NullStr);
	Notify(s);
      }
    }
  }
  else if (toupper(*str) > 'B' && rc != ERROR_NO_MORE_FILES) {
    // Find for remote or hard drive failed with error
    CHAR s[CCHMAXPATH + 80];
    sprintf(s, GetPString(IDS_SEARCHERRORTEXT), rc, str);
    Notify(s);
  }

None:

  DosError(FERR_DISABLEHARDERR);
  return ok;
} // Stubby

// Stubby/Flesh/Unflesh work list item

typedef struct {
  LIST2 list;
  HWND hwndCnr;
  PCNRITEM pci;
  FLESHWORKACTION action;
} FLESHWORKITEM;
typedef FLESHWORKITEM *PFLESHWORKITEM;

// Stubby/Flesh/Unflesh work list
LIST2 FleshWorkList;

HMTX hmtxFleshWork;
HEV  hevFleshWorkListChanged;

/**
 * Check pci matches
 */

BOOL WorkListItemMatches(PLIST2 item, PVOID data)
{
  return ((PFLESHWORKITEM)data)->pci == ((PFLESHWORKITEM)item)->pci;
}

/**
 * Delete stale items from flesh queue
 */

VOID DeleteStaleFleshWorkListItems(PCNRITEM pci)
{
  FLESHWORKITEM match;
  PLIST2 item;

  match.pci = pci;

  for (;;) {
    item = List2Search(&FleshWorkList, WorkListItemMatches, &match);
    if (!item)
      break;
    DbgMsg(pszSrcFile, __LINE__, "DeleteStaleFleshWorkListItems deleting %p %s", pci, pci->pszFileName ? pci->pszFileName : "(null)"); // 2015-08-03 SHL FIXME debug
    List2Delete(&FleshWorkList, item);
    xfree(item, pszSrcFile, __LINE__);
  }
}

/**
 * Add item to flesh work list
 * eUnFlesh requests get special handling
 * eFlesh etc. items for the same CNRITEM are considered stale and are
 * deleted because eUnFlesh will free the CNRITEM associated with the item
 * before the work list item is processed
 */

#if 0 // 2015-08-03 SHL FIXME debug
BOOL AddFleshWorkRequest(HWND hwndCnr, PCNRITEM pci, FLESHWORKACTION action)
#else
BOOL AddFleshWorkRequestDbg(HWND hwndCnr, PCNRITEM pci, FLESHWORKACTION action, PCSZ pszSrcFile_, UINT uSrcLineNo)
#endif
{
  APIRET rc;

  PFLESHWORKITEM item = xmallocz(sizeof(FLESHWORKITEM), pszSrcFile, __LINE__);
  item->hwndCnr = hwndCnr;
  item->pci = pci;
  item->action= action;

  if (fAmClosing) {
    // Can jiggle - see MainWndProc WM_CLOSE
    DosSleep(100);
    if (fAmClosing)
      return FALSE;
  }

  // 2015-08-03 SHL FIXME debug
  {
#if 0 // 2015-08-13 SHL FIXME to be gone
    static PSZ itemNames[] = {
	    "eStubby", "eFlesh", "eFleshEnv", "eUnFlesh"
    };

#   ifdef AddFleshWorkRequest
    if (!pci || (INT)pci == -1) {
      Runtime_Error(pszSrcFile, __LINE__, "AddFleshWorkRequest called with action %s pci %p by %s:%u",
	     itemNames[item->action],
	     pci,
	     pszSrcFile_, uSrcLineNo); // 2015-08-03 SHL FIXME debug
    }
    else if (!pci->pszFileName) {
      Runtime_Error(pszSrcFile, __LINE__, "AddFleshWorkRequest call with action %s pci %p pszFileName (null) by %s:%u",
	     itemNames[item->action],
	     pci,
	     pszSrcFile_, uSrcLineNo); // 2015-08-03 SHL FIXME debug
    }
    else if (!fNoFleshDbgMsg) {
      DbgMsg(pszSrcFile, __LINE__, "AddFleshWorkRequest called with action %s pci %p pszFileName %s by %s:%u",
	     itemNames[item->action],
	     pci,
	     pci->pszFileName,
	     pszSrcFile_, uSrcLineNo); // 2015-08-03 SHL FIXME debug
    }
#else
    if (!pci || (INT)pci == -1) {
      Runtime_Error(pszSrcFile, __LINE__, "AddFleshWorkRequest call with action %s pci %p",
	     itemNames[item->action],
	     pci); // 2015-08-03 SHL FIXME debug
    }
    else if (!pci->pszFileName) {
      Runtime_Error(pszSrcFile, __LINE__, "AddFleshWorkRequest called with action %s pci %p pszFileName (null)",
	     itemNames[item->action],
	     pci); // 2015-08-03 SHL FIXME debug
    }
    else if (!fNoFleshDbgMsg) {
      DbgMsg(pszSrcFile, __LINE__, "AddFleshWorkRequest action %s pci %p pszFileName %s",
	     itemNames[item->action],
	     pci,
	     pci->pszFileName); // 2015-08-03 SHL FIXME debug
    }
#endif
#endif
  }

  rc = DosRequestMutexSem(hmtxFleshWork, SEM_INDEFINITE_WAIT);
  if (rc)
    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosRequestMutexSem");

  // 2015-08-13 SHL FIXME to be done
  // Delete stale requests
  if (item->action == eUnFlesh) {
    DeleteStaleFleshWorkListItems(pci);
  }

  List2Append(&FleshWorkList, (PLIST2)item);

  rc = DosReleaseMutexSem(hmtxFleshWork);
  if (rc)
    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosReleaseMutexSem");

  rc = DosPostEventSem(hevFleshWorkListChanged);
  if (rc && rc != ERROR_ALREADY_POSTED)
    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosPostEventSem");

  return TRUE;
}

/**
 * Return TRUE if work list empty
 * Advisory only
 */

BOOL IsFleshWorkListEmpty()
{
  return FleshWorkList.next == NULL;
}

/**
 * Wait until work list empty
 * Advisory only
 */

#if 0 // 2015-08-03 SHL FIXME debug
VOID WaitFleshWorkListEmpty()
#else
VOID WaitFleshWorkListEmptyDbg(PCSZ pszSrcFile_, UINT uSrcLineNo_)
#endif
{
  INT tid = GetTidForThread();

  if (tid == 1 || tid == tidFleshWorkListThread) {
#   ifdef WaitFleshWorkListEmpty
    Runtime_Error(pszSrcFile, __LINE__, "WaitFleshWorkListEmpty called with worklist %sempty by tid %u at %s:%u", IsFleshWorkListEmpty() ? "" : "not ", tid, pszSrcFile_, uSrcLineNo_);
#   else
    Runtime_Error(pszSrcFile, __LINE__, "WaitFleshWorkListEmpty called by tid %u", tid);
#   endif
    return;		// Avoid hang
  }
  else if (IsFleshWorkListEmpty()) {
#   ifdef WaitFleshWorkListEmpty
    DbgMsg(pszSrcFile, __LINE__, "WaitFleshWorkListEmpty called with worklist empty by tid %u at %s:%u", tid, pszSrcFile_, uSrcLineNo_);
#   else
    DbgMsg(pszSrcFile, __LINE__, "WaitFleshWorkListEmpty called with work list empty by tid %u", tid);
#   endif
  }

  // Can not wait if call from thread 1 or FleshWorkListThread
  while (!IsFleshWorkListEmpty()) {
#   ifdef WaitFleshWorkListEmpty
    if (!fNoFleshDbgMsg)
      // DbgMsg(pszSrcFile, __LINE__, "WaitFleshWorkListEmpty called with work list not empty by %s:%u", pszSrcFile_, uSrcLineNo_); // 2015-08-07 SHL FIXME debug
#   else
    if (!fNoFleshDbgMsg)
      // DbgMsg(pszSrcFile, __LINE__, "WaitFleshWorkListEmpty called with work list not empty"); // 2015-08-07 SHL FIXME debug
#   endif
    // 2015-08-13 SHL FIXME to optimize
    if (fAmClosing) {
      // Can jiggle - see MainWndProc WM_CLOSE
      DosSleep(100);
      if (fAmClosing)
	return;
    }
    DosSleep(250);
  }
}

/**
 * Set focus drive to optimize work list processing
 * @param chDriveLetter is upper case drive letter (A-Z)
 */

VOID SetFleshFocusDrive(CHAR chDriveLetter) {
  chFleshFocusDrive = chDriveLetter;
  DbgMsg(pszSrcFile, __LINE__, "SetFleshFocusDrive focus drive set to %c", chFleshFocusDrive); // 2015-08-03 SHL FIXME debug

}

/**
 * Check pci pathname matches focus drive
 * @param data is uppercase drive letter mapped to PVOID
 * @return TRUE if drive letter matches
 */

BOOL MatchesFocusDrive(PLIST2 item, PVOID data)
{
  if (!((PFLESHWORKITEM)item)->pci->pszFileName) {
    Runtime_Error(pszSrcFile, __LINE__, "MatchesFocusDrive called with pci %p pszFileName (null) by %s:%u", ((PFLESHWORKITEM)item)->pci);
    return FALSE;
  }
  return ((PFLESHWORKITEM)item)->pci->pszFileName[0] == (CHAR)data;
}

/**
 * Run Flesh, UnFlesh, FleshEnv, Stubby for directory for items in work list
 */

VOID FleshWorkThread(PVOID arg)
{
  HAB thab;
  HMQ hmq = (HMQ)0;
  APIRET rc;

  // 2015-08-07 SHL FIXME to be gone
  static INT ProcessDirCount = 0;

  DosError(FERR_DISABLEHARDERR);

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif

  thab = WinInitialize(0);
  if (thab) {
    hmq = WinCreateMsgQueue(thab, 0);
    if (hmq) {
      IncrThreadUsage();
      priority_normal();

      // 2015-08-07 SHL FIXME to know when to terminate
      // process list entries forever
      for (;;) {

	PFLESHWORKITEM item;

	// 2015-08-13 SHL FIXME to optimize
	if (fAmClosing) {
	  // Can jiggle - see MainWndProc WM_CLOSE
	  DosSleep(100);
	  if (fAmClosing)
	    break;			// time to die
	}

	// 2015-08-07 SHL FIXME to use SMPSafe...
	rc = DosRequestMutexSem(hmtxFleshWork, SEM_INDEFINITE_WAIT);
	if (rc)
	  Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosRequestMutexSem");

	// 2015-08-14 SHL
	// Get next work list item and remove from list
	// If focus drive known, process items for focus drive first
	if (chFleshFocusDrive) {
	  item = (PFLESHWORKITEM)List2Search(&FleshWorkList, MatchesFocusDrive, (PVOID)chFleshFocusDrive);
	  if (!item)
	    chFleshFocusDrive = 0;		// Revert to normal
	  else
	    List2Delete(&FleshWorkList, (PLIST2)item);
	}
	else
	  item = NULL;

	if (!item)
	  item = (PFLESHWORKITEM)List2DeleteFirst(&FleshWorkList);

	rc = DosReleaseMutexSem(hmtxFleshWork);
	if (rc)
	  Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosReleaseMutexSem");

	// 2015-08-07 SHL FIXME to have event sem
	if (!item) {
	  ULONG ul;
	  if (!fNoFleshDbgMsg)
	    DbgMsg(pszSrcFile, __LINE__, "FleshWorkThread work list empty - waiting"); // 2015-08-03 SHL FIXME debug
	  rc = DosWaitEventSem(hevFleshWorkListChanged, SEM_INDEFINITE_WAIT);
	  if (rc)
	    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosWaitEventSem");
	  rc = DosResetEventSem(hevFleshWorkListChanged, &ul);
	  if (rc)
	    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosResetEventSem");
	  if (!fNoFleshDbgMsg)
	    DbgMsg(pszSrcFile, __LINE__, "FleshWorkThread work hev posted"); // 2015-08-03 SHL FIXME debug
	  continue;
	}

	if (WinIsWindow((HAB)0, item->hwndCnr)) {

	  ULONG flags;

#if 0 // 2015-08-07 SHL	FIXME debug
	  // 2015-08-03 SHL FIXME debug
	  {
	    static PSZ itemNames[] = {
		    "eStubby", "eFlesh", "eFleshEnv", "eUnFlesh"
	    };

	    PCNRITEM pci = item->pci;
	    if (!fNoFleshDbgMsg) {
	      DbgMsg(pszSrcFile, __LINE__, "FleshWorkThread action %s pci %p pszFileName %s",
		     itemNames[item->action],
		     pci,
		     pci && (INT)pci != -1 ?
		       (pci->pszFileName ? pci->pszFileName : "(nullname)") :
		       "(nullpci)"); // 2015-08-03 SHL FIXME debug
	    }
	  }
#endif

	  switch (item->action) {
	  case eUnFlesh:
	    UnFlesh(item->hwndCnr, item->pci);
	    break;
	  case eFleshEnv:
	    FleshEnv(item->hwndCnr, item->pci);
	    break;
	  case eFillDir:
	  case eStubby:
	    // DbgMsg(pszSrcFile, __LINE__, "FleshWorkThread pci %p pszFileName %s", stubbyArgs->pci, stubbyArgs->pci->pszFileName); // 2015-08-03 SHL FIXME debug
	    flags = driveflags[toupper(*item->pci->pszFileName) - 'A'];

#if 0 // 2015-08-07 SHL FIXME to be gone
	    if (flags & DRIVE_LOCALHD) {
	      // Serialize to prevent head thrashing
	      rc = DosRequestMutexSem(hmtxScanningLocalHD, SEM_INDEFINITE_WAIT);
	      if (rc)
		Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosRequestMutexSem");
	    }
#endif

#if 1
	    if (item->action == eFillDir) {
	      // eFillDir maps to eFlesh or eStubby depending on fRScan.. settings
	      if (((fRScanLocal && flags & DRIVE_LOCALHD) ||
		   (fRScanRemote && flags & DRIVE_REMOTE) ||
		   (fRScanVirtual && flags & DRIVE_VIRTUAL))  &&
		  (!(flags & ((fRScanNoWrite ? 0 : DRIVE_NOTWRITEABLE) |
			      (fRScanSlow ? 0 : DRIVE_SLOW)))))
	      {
		item->action = eFlesh;
	      }
	      else
		item->action = eStubby;
	    }
#else
	    wantFlesh = ((fRScanLocal && flags & DRIVE_LOCALHD ) ||
			 (fRScanRemote && flags & DRIVE_REMOTE) ||
			 (fRScanVirtual && flags & DRIVE_VIRTUAL))  &&
			 // 2015-08-11 SHL typo - should not be local or
			(!(flags & ((fRScanNoWrite ? 0 : DRIVE_NOTWRITEABLE) ||
				    (fRScanSlow ? 0 : DRIVE_SLOW))));
#endif
	    if (item->action == eStubby) {
	      Stubby(item->hwndCnr, item->pci);
	      break;
	    }
	    // Drop through to eFlesh


#if 0 // 2015-08-07 SHL FIXME to be gone
	    if (flags & DRIVE_LOCALHD) {
	      rc = DosReleaseMutexSem(hmtxScanningLocalHD);
	      if (rc) {
		Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
			  "DosReleaseMutexSem(hmtxScanningLocalHD)");
	      }
	    }
#endif // 2015-08-07 SHL FIXME to be gone

	    // Drop through
	  case eFlesh:
	    if (Flesh(item->hwndCnr, item->pci)) {
	      // 2015-08-06 SHL FIXME to report?
	    }
	    break;
	  default:
	    Runtime_Error(pszSrcFile, __LINE__, "item %u unexpected", item->action);
	  } // switch


	} // if window

	xfree(item, pszSrcFile, __LINE__);

      } // for

      WinDestroyMsgQueue(hmq);
    }
    DecrThreadUsage();
    WinTerminate(thab);
  }

  ProcessDirCount++;
  // DbgMsg(pszSrcFile, __LINE__, "ProcessDirCount %i FixedVolume %i", ProcessDirCount, FixedVolume);
  if (ProcessDirCount >= FixedVolume) {

#if 0 // 2015-08-04 SHL FIXME to be gone
    rc = DosReleaseMutexSem(hmtxScanning);
    if (rc)
      Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosReleaseMutexSem");
#endif // 2015-08-04 SHL FIXME to be gone

#if 0 // 2015-08-04 SHL FIXME to be gone
    rc = DosPostEventSem(hevTreeCnrScanComplete);
    if (rc && rc != ERROR_ALREADY_POSTED)
      Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__, "DosPostEventSem");
#endif // 2015-08-04 SHL FIXME to be gone

    ProcessDirCount = 0;
    FixedVolume = 0;
  }

# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif

}

/**
 * Allocate resources and start FleshWorkThread
 * @return TRUE if OK
 */

BOOL StartFleshWorkThread()
{
  APIRET rc = DosCreateMutexSem(NULL, &hmtxFleshWork, 0L /* Not shared */, FALSE /* Not owned */);
  if (rc) {
    Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSCREATEMUTEXSEM);
    return FALSE;
  }

  rc = DosCreateEventSem(NULL, &hevFleshWorkListChanged, 0 /* Not shared */, FALSE /* Reset */);
  if (rc) {
    Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSCREATEEVENTSEM);
    return FALSE;
  }

  /* DbgMsg is time consuming
     define FM2_NO_FLESH_DBGMSG to suppress
     2015-08-09 SHL FIXME to be gone
   */

  fNoFleshDbgMsg = getenv("FM2_NO_FLESH_DBGMSG") != NULL;

  tidFleshWorkListThread = xbeginthread(FleshWorkThread,
			   65536,
			   NULL,
			   pszSrcFile, __LINE__);
  return tidFleshWorkListThread != -1;

}

#pragma alloc_text(FLESH,Flesh,FleshEnv,UnFlesh,Stubby,FleshWorkThread,StartFleshWorkThread,AddFleshWorkRequest)
