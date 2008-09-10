
/***********************************************************************

  $Id$

  Flesh

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2008 Steven H. Levine

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

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3str.h"
#include "filldir.h"			// FileAttrToString...
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "flesh.h"
#include "valid.h"			// IsValidDir
#include "misc.h"			// LoadLibPath
#include "findrec.h"			// FindCnrRecord
#include "notify.h"			// Notify
#include "wrappers.h"			// xfree

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

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
    if (!stricmp(path, "LIBPATH")) {
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

BOOL Flesh(HWND hwndCnr, PCNRITEM pciParent)
{
  PCNRITEM pciL;
  DIRCNRDATA *dcd;
  BOOL includefiles = fFilesInTree;

  if (!pciParent || (INT) pciParent == -1 || !hwndCnr)
    return FALSE;
  pciL = (PCNRITEM) WinSendMsg(hwndCnr,
			       CM_QUERYRECORD,
			       MPFROMP(pciParent),
			       MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
  if (!pciL || !*pciL->pszFileName) {
    if (pciL && (INT) pciL != -1)
      RemoveCnrItems(hwndCnr, pciL, 1, CMA_FREE);
    dcd = INSTDATA(hwndCnr);
    if (dcd && dcd->size != sizeof(DIRCNRDATA))
      dcd = NULL;
    if (driveflags[toupper(*pciParent->pszFileName) - 'A'] &
	DRIVE_INCLUDEFILES)
      includefiles = TRUE;
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
  }
  return TRUE;
}

BOOL UnFlesh(HWND hwndCnr, PCNRITEM pciParent)
{
  BOOL ret = FALSE;
  PCNRITEM pciL;

  if (!pciParent || !hwndCnr)
    return FALSE;
  for (;;) {
    pciL = (PCNRITEM) WinSendMsg(hwndCnr,
				 CM_QUERYRECORD,
				 MPFROMP(pciParent),
				 MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    if (pciL && (INT) pciL != -1) {
      ret = TRUE;
      RemoveCnrItems(hwndCnr, pciL, 1, CMA_FREE);
    }
    else
      break;
  }
  if (ret) {
    WinSendMsg(hwndCnr,
	       CM_INVALIDATERECORD,
	       MPFROMP(&pciParent),
	       MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
  }
  return ret;
}

#define DDEPTH 16

BOOL Stubby(HWND hwndCnr, PCNRITEM pciParent)
{
  /*
   * this code is full of workarounds for screwed up LANs.
   * let's hope all the current LAN programmers fall into
   * a black hole and make way for people who can get it right...
   */

  BOOL ret = FALSE;
  FILEFINDBUF3 ffb[DDEPTH];
  PFILEFINDBUF3 pffb;
  HDIR hDir = HDIR_CREATE;
  ULONG nm, ulM = 1, total = 0, fl = MUST_HAVE_DIRECTORY;
  CHAR str[CCHMAXPATH];
  register INT len;
  APIRET rc, prc;
  BOOL isadir = FALSE, isremote = FALSE, includefiles = fFilesInTree;
  ULONG ddepth = 3;
  static BOOL brokenlan = FALSE, isbroken = FALSE;

  if (!pciParent || !*pciParent->pszFileName || !hwndCnr)
    return FALSE;

  len = strlen(pciParent->pszFileName);
  memcpy(str, pciParent->pszFileName, len + 1);
  if (str[len - 1] != '\\')
    str[len++] = '\\';
  str[len++] = '*';
  str[len] = 0;

  if (!isalpha(*str) || str[1] != ':' || str[2] != '\\')
    MakeFullName(str);

  if (!isalpha(*str) ||
      str[1] != ':' ||
      str[2] != '\\' || ((driveflags[toupper(*str) - 'A'] & DRIVE_IGNORE)))
    return FALSE;

  if (isalpha(*str) && driveflags[toupper(*str) - 'A'] & DRIVE_INCLUDEFILES)
    includefiles = TRUE;

  if (!isalpha(*str) ||
      str[1] != ':' ||
      str[2] != '\\' || ((driveflags[toupper(*str) - 'A'] & DRIVE_REMOTE)))
    isremote = TRUE;

  if (isremote) {
    ddepth = 14;
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

  if (!isremote || !fRemoteBug)
    ulM = (ddepth < 16) ? ddepth : 1;

  nm = ulM;

  DosError(FERR_DISABLEHARDERR);
  if (includefiles)
    fl = FILE_DIRECTORY;
  rc = DosFindFirst(str,
		    &hDir,
		    FILE_NORMAL | fl |
		    FILE_READONLY | FILE_ARCHIVED |
		    FILE_SYSTEM | FILE_HIDDEN,
		    &ffb, ulM * sizeof(FILEFINDBUF3), &nm, FIL_STANDARD);
  if (ulM == 1 && !rc) {
    do {
      pffb = &ffb[0];
      if (!includefiles && !(pffb->attrFile & FILE_DIRECTORY) && !brokenlan) {
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
	    PrfWriteProfileData(fmprof,
				FM3Str,
				"NoBrokenNotify",
				&NoBrokenNotify, sizeof(ULONG));
	  }
	}
	else {
	  NoBrokenNotify--;
	  PrfWriteProfileData(fmprof,
			      FM3Str,
			      "NoBrokenNotify",
			      &NoBrokenNotify, sizeof(ULONG));
	}
      }
      if (*pffb->achName &&
	  (includefiles || (pffb->attrFile & FILE_DIRECTORY)) &&
	  // Skip . and ..
	  (pffb->achName[0] != '.' ||
	   (pffb->achName[1] &&
	    (pffb->achName[1] != '.' || pffb->achName[2])))) {
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
    goto None;
  }

  if (!rc) {
    DosFindClose(hDir);
    if (nm) {

      register PBYTE fb = (PBYTE) & ffb[0];

      for (len = 0; len < nm; len++) {
	pffb = (PFILEFINDBUF3) fb;
	if (!includefiles && !(pffb->attrFile & FILE_DIRECTORY)) {
	  if (!isbroken) {
	    isbroken = TRUE;
	    if (!NoBrokenNotify) {
	      prc = saymsg(MB_YESNO | MB_ICONEXCLAMATION,
			   HWND_DESKTOP,
			   GetPString(IDS_FSDERRORTITLETEXT),
			   GetPString(IDS_FSDERRORTEXT),
			   (isremote) ?
			   GetPString(IDS_REMOTETEXT) :
			   GetPString(IDS_LOCALTEXT), *str);
	      if (prc == MBID_NO) {
		saymsg(MB_ENTER,
		       HWND_DESKTOP,
		       GetPString(IDS_FSDERROR2TITLETEXT),
		       GetPString(IDS_FSDERROR2TEXT));
		NoBrokenNotify = 255;
		PrfWriteProfileData(fmprof,
				    FM3Str,
				    "NoBrokenNotify",
				    &NoBrokenNotify, sizeof(ULONG));
	      }
	    }
	    else {
	      NoBrokenNotify--;
	      PrfWriteProfileData(fmprof,
				  FM3Str,
				  "NoBrokenNotify",
				  &NoBrokenNotify, sizeof(ULONG));
	    }
	  }
	}
	if (*pffb->achName &&
	    (includefiles || (pffb->attrFile & FILE_DIRECTORY)) &&
	    // Skip . and ..
	    (pffb->achName[0] != '.' || (pffb->achName[1]
					 && (pffb->achName[1] != '.'
					     || pffb->achName[2])))) {
	  isadir = TRUE;
	  break;
	}
	fb += pffb->oNextEntryOffset;
      }

    Interruptus:

      if (isadir) {

	PCNRITEM pci;

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
	  ri.cRecordsInsert = 1L;
	  ri.fInvalidateRecord = TRUE;
	  if (!WinSendMsg(hwndCnr,
			  CM_INSERTRECORD, MPFROMP(pci), MPFROMP(&ri))) {
	    DosSleep(50); //05 Aug 07 GKY 100
	    WinSetFocus(HWND_DESKTOP, hwndCnr);
	    if (WinIsWindow((HAB)0, hwndCnr)) {
	      if (!WinSendMsg(hwndCnr,
			      CM_INSERTRECORD, MPFROMP(pci), MPFROMP(&ri))) {
		Win_Error(hwndCnr, HWND_DESKTOP, __FILE__, __LINE__,
			  GetPString(IDS_RECORDINSERTFAILEDTEXT));
		FreeCnrItem(hwndCnr, pci);
	      }
	      else
		ret = TRUE;
	    }
	  }
	  else
	    ret = TRUE;
	}
      }
      else if (toupper(*str) > 'B' && str[1] == ':' && str[2] == '\\' &&
	       !str[3]) {

	CHAR s[162];

	sprintf(s,
		GetPString(IDS_NOSUBDIRS2TEXT),
		nm,
		toupper(*pciParent->pszFileName),
		(isremote) ? GetPString(IDS_NOSUBDIRS3TEXT) : NullStr);
	Notify(s);
      }
    }
  }
  else if (toupper(*str) > 'B' && rc != ERROR_NO_MORE_FILES) {

    CHAR s[CCHMAXPATH + 80];

    sprintf(s, GetPString(IDS_SEARCHERRORTEXT), rc, str);
    Notify(s);
  }

None:

  DosError(FERR_DISABLEHARDERR);
  return ret;
}

#pragma alloc_text(FLESH,Flesh,FleshEnv,Unflesh,Stubby)
