
/***********************************************************************

  $Id$

  Drive tree container management

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2015 Steven H. Levine

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
  19 Aug 15 SHL Allow WaitFleshWorkListEmpty to wait for dependent items
  23 Aug 15 GKY Fixed code to notify on drive with no subdirectories in first 64 entries
  20 Sep 15 GKY Add code for Flesh to skip the directory entry added by Stubby (eliminate
                use of NULL/Nullstr pszFileNames by Stubby). Add code in Stubby to insert a
                complete container item. Add a flag to indicate when a directory needed to be
                Fleshed
  26 Sep 15 GKY Changes to speed up ExpandAll
  26 Sep 15 GKY WaitFleshWorkListEmpty now gives error message and returns if semaphore request
                fails more than 5 consecutive times.
  27 Sep 15 GKY DosSleep times in WaitFleshWorkListEmpty set by caller
  10 Oct 15 GKY Don't use Flesh thread for floppy drive scans fix them getting mistakenly identified
                as directories and add nonexistent subdirectories.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
#include "pathutil.h"
#if 0
#define  __PMPRINTF__
#include "PMPRINTF.H"
#endif

// Data definitions
#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;


static INT tidFleshWorkListThread = -1;	// 2015-08-08 SHL

static PCSZ pszFleshFocusPath;	// 2015-08-20 SHL

#pragma data_seg(GLOBAL1)
ULONG NoBrokenNotify;
BOOL fFilesInTree;

BOOL Stubby(HWND hwndCnr, PCNRITEM pciParent);
BOOL FleshEnv(HWND hwndCnr, PCNRITEM pciParent);

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

  if (!pciParent || (INT)pciParent == -1 || !hwndCnr)
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
	      while (pciL && pciL != (PCNRITEM)-1 && pciL != pciParent)
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
		ri.pRecordOrder = (PRECORDCORE)CMA_END;
		ri.pRecordParent = (PRECORDCORE)pciParent;
		ri.zOrder = (ULONG)CMA_TOP;
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
    pciL = (PCNRITEM)WinSendMsg(hwndCnr,
				CM_QUERYRECORD,
				MPFROMP(pciParent),
				MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    while (pciL && (INT)pciL != -1) {
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

  // 2015-08-13 SHL
  if (fAmQuitting)
    return FALSE;

  if (!pciParent->fleshed) {
    pciL = (PCNRITEM)WinSendMsg(hwndCnr,
                                CM_QUERYRECORD,
                                MPFROMP(pciParent),
                                MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    // Added by Stubby to create plus sign run Stubby on it here and skip it in ProcessDirectory
    if (pciL && (INT)pciL != -1) {
      AddFleshWorkRequest(hwndCnr, pciL, eStubby);
      // 2015-08-06 SHL FIXME to ensure this can not happen
      if (!*pciL->pszFileName || !strcmp(pciL->pszFileName, NullStr))
        Runtime_Error(pszSrcFile, __LINE__, "Flesh called with pci %p pszFileName (null)",
                      pciL);
    }
    dcd = INSTDATA(hwndCnr);
    if (dcd && dcd->size != sizeof(DIRCNRDATA))
      dcd = NULL;

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
                     NULL,               // total bytes
                     (pciL && (INT)pciL != -1) ? pciL->pszDisplayName : 0);
    pciParent->fleshed = TRUE;
    return TRUE;
  }

  return FALSE;
}

/**
 * Remove children from container
 * @param pciParent is parent of children to be removed
 */

VOID UnFlesh(HWND hwndCnr, PCNRITEM pciParent)
{
  BOOL removed = FALSE;
  PCNRITEM pciL;

  if (!pciParent || !hwndCnr)
    return;
  for (;;) {
    pciL = (PCNRITEM)WinSendMsg(hwndCnr,
				CM_QUERYRECORD,
				MPFROMP(pciParent),
				MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    if (!pciL || (INT)pciL == -1)
      break;   
    RemoveCnrItems(hwndCnr, pciL, 1, CMA_FREE);
    removed = TRUE;
  } // for

  if (removed) {
    WinSendMsg(hwndCnr,
	       CM_INVALIDATERECORD,
	       MPFROMP(&pciParent),
               MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
    pciParent->fleshed = FALSE;
    DosSleep(1); // Let container items go away
  }
  return;
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
  CHAR wildcard[CCHMAXPATH];
  register INT len;
  APIRET rc, prc;
  BOOL isadir = FALSE;
  BOOL isremote;
  BOOL includefiles;
  ULONG ddepth = DDEPTH;
  ULONG drvNum;
  ULONG flags;
  static BOOL brokenlan = FALSE, isbroken = FALSE;

  if (!pciParent || (INT)pciParent == -1 || !*pciParent->pszFileName
      || pciParent->pszFileName == NullStr || !hwndCnr)
    return FALSE;
  // Build wildcard
  len = strlen(pciParent->pszFileName);
  memcpy(wildcard, pciParent->pszFileName, len + 1);
  if (wildcard[len - 1] != '\\')
    wildcard[len++] = '\\';
  wildcard[len++] = '*';
  wildcard[len] = 0;

  // 2015-08-19 SHL FIXME to know how this can happen
  if (!isalpha(*wildcard) || wildcard[1] != ':' || wildcard[2] != '\\') {
    MakeFullName(wildcard);
  }
  drvNum = toupper(*pciParent->pszFileName) - 'A';
  flags = driveflags[drvNum];
  if (!isalpha(*wildcard) ||
      wildcard[1] != ':' ||
      wildcard[2] != '\\' || ((flags & DRIVE_IGNORE)))
    return FALSE;			// Not a directory or ignore requested

  includefiles = flags & DRIVE_INCLUDEFILES ? TRUE : fFilesInTree;

  isremote = flags & DRIVE_REMOTE ? TRUE : FALSE;

  if (isremote) {
    if (fRemoteBug) {
      if (brokenlan) {
	ddepth = (ULONG)-1;
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
  rc = DosFindFirst(wildcard,
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
	ddepth = (ULONG)-1;
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
	sprintf(&s[strlen(s)], GetPString(IDS_SEARCHERRORTEXT), rc, wildcard);
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
			   *wildcard);
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
            CHAR szBuffer[CCHMAXPATH + 14];
            CHAR *p;
            HPOINTER hptr;

            p = strchr(wildcard, '*');
            *p = 0;;
            BldFullPathName(szBuffer, wildcard, pffb->achName);
	    pci->pszFileName =  xstrdup(szBuffer, pszSrcFile, __LINE__); 
            p = strrchr(pci->pszFileName, '\\');
            p++;
            pci->pszDisplayName = p; 
            pci->rc.pszIcon = pci->pszDisplayName;
            if (fForceUpper)
              strupr(pci->pszFileName);
            else if (fForceLower)
              strlwr(pci->pszFileName);
          
            flags = driveflags[toupper(*pci->pszFileName) - 'A'];
          
            // get an icon to use with it
            if (pffb->attrFile & FILE_DIRECTORY) {
              // is directory
              if (fNoIconsDirs ||
                  (flags & DRIVE_NOLOADICONS) ||
                  !isalpha(*pci->pszFileName)) {
                hptr = (HPOINTER) 0;
              }
              else
                hptr = WinLoadFileIcon(pci->pszFileName, FALSE);
            }
            else {
              // is file
              if (fNoIconsFiles ||
                  (flags & DRIVE_NOLOADICONS) ||
                  !isalpha(*pci->pszFileName)) {
                hptr = (HPOINTER) 0;
              }
              else
                hptr = WinLoadFileIcon(pci->pszFileName, FALSE);
          
              if (!hptr || IsDefaultIcon(hptr))
                hptr = IDFile(pci->pszFileName);
            }
          
            if (!hptr) {
              hptr = pffb->attrFile & FILE_DIRECTORY ?
                     hptrDir : pffb->attrFile & FILE_SYSTEM ?
                     hptrSystem : pffb->attrFile & FILE_HIDDEN ?
                     hptrHidden : pffb->attrFile & FILE_READONLY ?
                     hptrReadonly : hptrFile;
            }
            pci->rc.hptrIcon = hptr;
	    memset(&ri, 0, sizeof(RECORDINSERT));
	    ri.cb = sizeof(RECORDINSERT);
	    ri.pRecordOrder = (PRECORDCORE)CMA_END;
	    ri.pRecordParent = (PRECORDCORE)pciParent;
	    ri.zOrder = (ULONG)CMA_TOP;
	    ri.cRecordsInsert = 1;
	    ri.fInvalidateRecord = TRUE;
            if (!WinSendMsg(hwndCnr,
			    CM_INSERTRECORD, MPFROMP(pci), MPFROMP(&ri))) {
	      // Assume busy and try again
	      DosSleep(50); 
	      WinSetFocus(HWND_DESKTOP, hwndCnr);
              if (WinIsWindow((HAB)0, hwndCnr)) {
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
    }
  } // if !rc
  else if (toupper(*wildcard) > 'B' && wildcard[1] == ':' && wildcard[2] == '\\' &&
	   wildcard[3] == '*' && !wildcard[4]) {
    // Is root and no subdirectories
    CHAR s[162];

    sprintf(s,
	    GetPString(IDS_NOSUBDIRS2TEXT),
	    nm,
	    toupper(*pciParent->pszFileName),
	    isremote ? GetPString(IDS_NOSUBDIRS3TEXT) : NullStr);
    Notify(s);
  }
  else if (toupper(*wildcard) > 'B' && rc != ERROR_NO_MORE_FILES) {
    // Find for remote or hard drive failed with error
    CHAR s[CCHMAXPATH + 80];
    sprintf(s, GetPString(IDS_SEARCHERRORTEXT), rc, wildcard);
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
 * Check work list item pci matches passed pci
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


BOOL AddFleshWorkRequest(HWND hwndCnr, PCNRITEM pci, FLESHWORKACTION action)
{
  PFLESHWORKITEM item = xmallocz(sizeof(FLESHWORKITEM), pszSrcFile, __LINE__);
  item->hwndCnr = hwndCnr;
  item->pci = pci;
  item->action= action;

  if (fAmQuitting)
    return FALSE;


  xDosRequestMutexSem(hmtxFleshWork, SEM_INDEFINITE_WAIT);

  // Delete stale requests
  if (item->action == eUnFlesh)
    DeleteStaleFleshWorkListItems(pci);

  List2Append(&FleshWorkList, (PLIST2)item);

  xDosReleaseMutexSem(hmtxFleshWork);
  xDosPostEventSem(hevFleshWorkListChanged);

  return TRUE;
}

/**
 * Return TRUE if work list empty
 * Advisory only
 */

BOOL IsFleshWorkListEmpty(VOID)
{
  return FleshWorkList.next == NULL;
}

/**
 * Check if pci pathname is parent of child path name
 * @param data is child path name
 * @return TRUE if is work item path is parent of given path
 */

BOOL IsParentOfChildPath(PLIST2 item, PVOID data)
{
  UINT c;
  if (!((PFLESHWORKITEM)item)->pci->pszFileName) {
    Runtime_Error(pszSrcFile, __LINE__, "IsParentOfChildPath called with pci %p pszFileName (null)", ((PFLESHWORKITEM)item)->pci);
    return FALSE;
  }
  c = strlen(((PFLESHWORKITEM)item)->pci->pszFileName);
  return strncmp(((PFLESHWORKITEM)item)->pci->pszFileName, (PCSZ)data, c) == 0;
}

/**
 * Wait until work list empty or until dependent items removed from list
 * Advisory only
 * @parse pszFileName is dependent pathName
 */


VOID WaitFleshWorkListEmpty(PCSZ pszDirName, ULONG ulSleep)
{
  APIRET rc;
  PFLESHWORKITEM item;
  INT tid = GetTidForThread();
  BOOL pathSaved = FALSE;
  BOOL waited;
  PCSZ pszSavedFleshFocusPath;
  INT rcCount = 0;

  if (tid == 1 || tid == tidFleshWorkListThread) { // 11 Oct 15 GKY Fixme did we intend to keep this
    Runtime_Error(pszSrcFile, __LINE__, "WaitFleshWorkListEmpty called by tid %u", tid);
    return;		// Avoid hang
  }
  // Can not wait if call from thread 1 or FleshWorkListThread
  for (waited = FALSE; !IsFleshWorkListEmpty(); waited = TRUE) {
    // 2015-08-13 SHL
    if (fAmQuitting)
      return;

    // Just wait for dependents to be gone if path name given
    if (pszDirName) {
      rc = xDosRequestMutexSem(hmtxFleshWork, SEM_INDEFINITE_WAIT);
      if (rc) {
        rcCount++;
        if (rcCount < 6)
          continue;			// Maybe should return ???
        else {
          Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
                    PCSZ_DOSREQUESTMUTEXSEM);
          return;
        }

      }

      if (!pathSaved) {
	// Give priority to work items for parents of this path
	pathSaved = TRUE;
	pszSavedFleshFocusPath = pszFleshFocusPath;
	pszFleshFocusPath = pszDirName;
      }

      item = (PFLESHWORKITEM)List2Search(&FleshWorkList, IsParentOfChildPath, (PVOID)pszDirName);

      xDosReleaseMutexSem(hmtxFleshWork);
      rcCount = 0;

      if (!item) {
	if (waited)
          DosSleep(ulSleep);		// Let PM do some work
	break;				// Dependents gone from work list
      }
    } // if pszDirName
    DosSleep(ulSleep);
  } // for

  if (pathSaved) {
    xDosRequestMutexSem(hmtxFleshWork, SEM_INDEFINITE_WAIT);
    pszFleshFocusPath = pszSavedFleshFocusPath;
    xDosReleaseMutexSem(hmtxFleshWork);
  }

}

/**
 * Set focus drive to optimize work list processing
 * @param chDriveLetter is upper case drive letter (A-Z)
 */

VOID SetFleshFocusPath(PCSZ pszPath) {
  PCSZ pszOld;
  PCSZ pszNew = strdup(pszPath);
  xDosRequestMutexSem(hmtxFleshWork, SEM_INDEFINITE_WAIT);
  pszOld = pszFleshFocusPath;
  pszFleshFocusPath = pszNew;
  xDosReleaseMutexSem(hmtxFleshWork);
  if (pszOld)
    xfree((PVOID)pszOld, pszSrcFile, __LINE__);
}

/**
 * Run Flesh, UnFlesh, FleshEnv, Stubby for directory for items in work list
 */

VOID FleshWorkThread(PVOID arg)
{
  HAB thab;
  HMQ hmq = (HMQ)0;

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

      // process list entries until time to die
      for (;!fAmQuitting;) {

	PFLESHWORKITEM item;

	// 2015-08-07 SHL FIXME to use SMPSafe...
	xDosRequestMutexSem(hmtxFleshWork, SEM_INDEFINITE_WAIT);

	// 2015-08-14 SHL
	// Get next work list item and remove from list
	// If focus path set, process parents of focus path first
	if (pszFleshFocusPath) {
	  item = (PFLESHWORKITEM)List2Search(&FleshWorkList, IsParentOfChildPath, (PVOID)pszFleshFocusPath);
	  if (!item) {
	    xfree((PSZ)pszFleshFocusPath, pszSrcFile, __LINE__);
	    pszFleshFocusPath = NULL;		// Revert to normal
	  }
	  else
	    List2Delete(&FleshWorkList, (PLIST2)item);
	}
	else
	  item = NULL;

	if (!item)
	  item = (PFLESHWORKITEM)List2DeleteFirst(&FleshWorkList);

	xDosReleaseMutexSem(hmtxFleshWork);

	// Wait for new items to be added to list
	if (!item) {
          ULONG ul;
          xDosWaitEventSem(hevFleshWorkListChanged, SEM_INDEFINITE_WAIT);
          xDosResetEventSem(hevFleshWorkListChanged, &ul);
          continue;
	}

        if (WinIsWindow((HAB)0, item->hwndCnr)) {

	  switch (item->action) {
          case eUnFlesh:
            UnFlesh(item->hwndCnr, item->pci);
	    break;
	  case eFleshEnv:
	    FleshEnv(item->hwndCnr, item->pci);
	    break;
	  case eStubby:
            priority_bumped();
            Stubby(item->hwndCnr, item->pci);
            priority_normal();
	    break;
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
  if (ProcessDirCount >= FixedVolume) {
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

BOOL StartFleshWorkThread(VOID)
{
  APIRET rc = DosCreateMutexSem(NULL, &hmtxFleshWork, 0L /* Not shared */, FALSE /* Not owned */);
  if (rc) {
    Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      PCSZ_DOSCREATEMUTEXSEM);
    return FALSE;
  }

  rc = xDosCreateEventSem(NULL, &hevFleshWorkListChanged, 0 /* Not shared */, FALSE /* Reset */);
  if (rc)
    return FALSE;			// Give up
  tidFleshWorkListThread = xbeginthread(FleshWorkThread,
			   65536,
			   NULL,
			   pszSrcFile, __LINE__);
  return tidFleshWorkListThread != -1;

}

#pragma alloc_text(FLESH,Flesh,FleshEnv,UnFlesh,Stubby,FleshWorkThread,StartFleshWorkThread,AddFleshWorkRequest)
