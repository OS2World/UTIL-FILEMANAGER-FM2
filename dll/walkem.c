
/***********************************************************************

  $Id$

  Misc persistent lists support

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2008 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  05 Jun 05 SHL Use QWL_USER
  13 Aug 05 SHL Run through indent
  13 Aug 05 SHL remove_udir - avoid corrupting last dirs list
  17 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets
  20 Oct 06 SHL Correct . .. check
  06 Nov 06 SHL Oops - need to allow .. here
  14 Nov 06 SHL Correct FillPathListBox regression
  22 Mar 07 GKY Use QWL_USER
  20 Apr 07 SHL Avoid spurious add_udir error reports
  16 Aug 07 SHL Update add_setups for ticket# 109
  19 Aug 07 SHL Correct load_setups error reporting
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  25 Aug 07 SHL Correct #pragma alloc_text typos
  11 Nov 07 GKY Cancel now directly closes dialog even if directory path text has changed
  20 Jan 08 GKY Walk & walk2 dialogs now save and restore size and position
  19 Feb 08 JBS Add "State at last FM/2 close" to the states combo box
  29 Feb 08 GKY Use xfree where appropriate
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  19 Jun 08 JBS Ticket 227: Allow temporary saving/deleting of the shutdown state of directory containers
  22 Jun 08 GKY Add free_?dir for fortify testing
  18 Jul 08 SHL More Fortify support
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory and use BldFullPathName
  24 Aug 08 GKY Warn full drive on save of .DAT file; prevent loss of existing file

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_SHLERRORS			// PMERR_NOT_IN_IDX
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "notebook.h"			// targetdirectory
#include "pathutil.h"                   // BldFullPathName
#include "walkem.h"
#include "valid.h"			// MakeFullName
#include "chklist.h"			// PosOverOkay
#include "mkdir.h"			// SetDir
#include "wrappers.h"			// xfgets
#include "strips.h"			// bstrip
#include "misc.h"			// CheckDriveSpaceAvail
#include "dirs.h"			// save_dir2
#include "fortify.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static MRESULT EXPENTRY WalkTwoDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static VOID load_setups(VOID);
static BOOL remove_ldir(PSZ path);

typedef struct
{
  USHORT size;
  USHORT changed;
  BOOL nounwriteable;
  CHAR szCurrentPath[CCHMAXPATH];
  CHAR *szReturnPath;
}
WALKER;

static CHAR WalkFont[CCHMAXPATH] = "";
static ULONG WalkFontSize = sizeof(WalkFont);

/**
 * States names management
 */

static BOOL fSetupsLoaded;
static LINKDIRS *pFirstSetup;
static const PSZ pszLastSetups = "LastSetups";
// 18 Aug 07 SHL fixme to stop supporting old style 1 year from now?
static const ULONG ulOldSetupsBytes = 100 * 13;	// Prior to 3.0.7

/**
 * Fill States drop down list with known state names
 */

VOID fill_setups_list(VOID)
{
  WinSendMsg(hwndStatelist, LM_DELETEALL, MPVOID, MPVOID);
  if (fUserComboBox) {
    LINKDIRS *pld;
    load_setups();
    for (pld = pFirstSetup; pld; pld = pld->next) {
      // DbgMsg(pszSrcFile, __LINE__, "Inserted %s", pld->path);
      WinSendMsg(hwndStatelist,
		 LM_INSERTITEM,
		 MPFROM2SHORT(LIT_SORTASCENDING, 0),
		 MPFROMP(pld->path));
    }
    WinSetWindowText(hwndStatelist, GetPString(IDS_STATETEXT));
  }
}

/**
 * Lookup setup and do requested action
 * @param - action, Support old/new style storage method
 * @return 1 if found and action OK, 0 if not found and action OK, -1 if error during action
 */

#define LS_FIND		0
#define LS_ADD		1
#define LS_DELETE	2

static INT lookup_setup(PSZ name, UINT action)
{
  LINKDIRS *pld;
  LINKDIRS *pldLast = NULL;

  if (!name || !*name) {
    Runtime_Error(pszSrcFile, __LINE__, "no data");
    return -1;
  }

  load_setups();

  for (pld = pFirstSetup; pld; pld = pld->next) {
    if (!stricmp(pld->path, name)) {
      if (action == LS_DELETE) {
	if (pldLast)
	  pldLast->next = pld->next;
	else
	  pFirstSetup = pld->next;
	xfree(pld->path, pszSrcFile, __LINE__);
	free(pld);
      }
      return 1;				// Found or added
    }
    pldLast = pld;			// In case deleting
  } // for

  // Not found
  if (action == LS_ADD) {
    pld = xmalloc(sizeof(LINKDIRS), pszSrcFile, __LINE__);
    if (!pld)
      return -1;
    pld->path = xstrdup(name, pszSrcFile, __LINE__);
    if (!pld->path) {
      free(pld);
      return -1;
    }
    // Insert at front of list - drop down will sort
    pld->next = pFirstSetup;
    pFirstSetup = pld;
    return 0;
  }

  return FALSE;				// Not found
}

/**
 * Load state names from ini
 * Support old/new style storage method
 */

VOID load_setups(VOID)
{
  ULONG ulDataBytes;
  ULONG l;
  PSZ pszBuf;
  PSZ psz;
  LINKDIRS *pld;

  if (fSetupsLoaded)
    return;

  if (!PrfQueryProfileSize(fmprof, FM3Str, pszLastSetups, &ulDataBytes)) {
    // fixme to use generic hab
    ERRORID eid = WinGetLastError((HAB)0);
    if ((eid & 0xffff) != PMERR_NOT_IN_IDX) {
      // Get error info back
      PrfQueryProfileSize(fmprof, FM3Str, pszLastSetups, &ulDataBytes);
      Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__, "PrfQueryProfileSize");
    }
    else
      fSetupsLoaded = TRUE;		// Nothing saved
    return;
  }

  if (ulDataBytes == 0) {
    Runtime_Error(pszSrcFile, __LINE__, "PrfQueryProfileSize reported 0 bytes");
    return;
  }

  pszBuf = xmalloc(ulDataBytes + 1, pszSrcFile, __LINE__);	// One extra for end marker
  if (!pszBuf)
    return;
  l = ulDataBytes;
  if (!PrfQueryProfileData(fmprof, FM3Str, pszLastSetups, pszBuf, &l)) {
    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__, "PrfQueryProfileData");
    free(pszBuf);
    return;
  }

  if (ulDataBytes != l) {
    Runtime_Error(pszSrcFile, __LINE__, "PrfQueryProfileData reported %u expected %u", l, ulDataBytes);
    free(pszBuf);
    return;
  }

  *(pszBuf + ulDataBytes) = 0;			// Insert end marker

  psz = pszBuf;
  if (!*psz && ulDataBytes == ulOldSetupsBytes)
    psz += 13;			// Rarely used 1st fixed width entry prior to 3.0.7

  while (*psz) {
    pld = xmalloc(sizeof(LINKDIRS), pszSrcFile, __LINE__);
    if (!pld) {
      free(pszBuf);
      return;
    }

#   ifdef FORTIFY
    Fortify_SetOwner(pld, 1);
    Fortify_SetScope(pld, 1);
#   endif

    pld->path = xstrdup(psz, pszSrcFile, __LINE__);
    if (!pld->path) {
      free(pszBuf);
      free(pld);
      return;
    }

#   ifdef FORTIFY
    Fortify_SetOwner(pld->path, 1);
    Fortify_SetScope(pld->path, 1);
#   endif

    // Insert at front of list - drop down will sort
    pld->next = pFirstSetup;
    pFirstSetup = pld;
    // DbgMsg(pszSrcFile, __LINE__, "Inserted %s", pld->path);

    if (ulDataBytes == ulOldSetupsBytes)
      psz += 13;			// Buffers fixed width prior to 3.0.7
    else
      psz += strlen(psz) + 1;
  } // while

  free(pszBuf);

  fSetupsLoaded = TRUE;
}

VOID save_setups(VOID)
{
  ULONG ulBufBytes;
  ULONG ulFillBytes;
  ULONG l;
  PSZ pszBuf;
  PSZ psz;
  LINKDIRS *pld;

  if (!fSetupsLoaded)
    return;

  ulBufBytes = 0;
  for (pld = pFirstSetup; pld; pld = pld->next) {
    ulBufBytes += strlen(pld->path) + 1;
  } // for

  if (!ulBufBytes)
    pszBuf = NULL;
  else {
    // Ensure different than size prior to 3.0.7
    ulFillBytes = ulBufBytes == ulOldSetupsBytes ? 1 : 0;
    pszBuf = xmalloc(ulBufBytes + ulFillBytes, pszSrcFile, __LINE__);
    if (!pszBuf)
      return;

    psz = pszBuf;
    for (pld = pFirstSetup; pld; pld = pld->next) {
      l = strlen(pld->path) + 1;
      memcpy(psz, pld->path, l);
      psz += l;
    } // for
    if (ulFillBytes)
      *psz = 0;
  }

  if (!PrfWriteProfileData(fmprof,
			   FM3Str,
			    pszLastSetups, pszBuf, ulBufBytes)) {
    // Win_Error(pszSrcFile, __LINE__, HWND_DESKTOP, HWND_DESKTOP, "PrfWriteProfileData");
    ERRORID eid = WinGetLastError((HAB)0);
    if ((eid & 0xffff) != PMERR_NOT_IN_IDX)
      Runtime_Error(pszSrcFile, __LINE__, "PrfWriteProfileData returned %u", eid);
  }

  // Delete obsolete INI entry
  PrfWriteProfileData(fmprof, FM3Str, "LastSetup", NULL, 0);
}

/**
 * Add named state to setups list
 * @return same as lookup_setup
 */

INT add_setup(PSZ name)
{
  return lookup_setup(name, LS_ADD);
}

/**
 * Delete named state from setups list
 * @return same as lookup_setup
 */

INT remove_setup(PSZ name)
{
  return lookup_setup(name, LS_DELETE);
}

#ifdef FORTIFY

VOID free_setups(VOID)
{
  LINKDIRS *pld = pFirstSetup;
  while (pld) {
    LINKDIRS *next = pld->next;
    free(pld->path);
    free(pld);
    pld = next;
  }
  pFirstSetup = NULL;
}

#endif // FORTIFY

VOID load_udirs(VOID)
{
  /* load linked list of user directories from USERDIRS.DAT file */

  FILE *fp;
  LINKDIRS *info;
  LINKDIRS *last = NULL;
  CHAR s[CCHMAXPATH + 24];

  if (udirhead)
    free_udirs();
  loadedudirs = TRUE;
  fUdirsChanged = FALSE;
  BldFullPathName(s, pFM2SaveDirectory, "USERDIRS.DAT");
  fp = _fsopen(s, "r", SH_DENYWR);
  if (fp) {
    while (!feof(fp)) {
      if (!xfgets(s, CCHMAXPATH + 24, fp, pszSrcFile, __LINE__))
	break;
      s[CCHMAXPATH] = 0;
      bstripcr(s);
      if (*s && *s != ';') {
	info = xmalloc(sizeof(LINKDIRS), pszSrcFile, __LINE__);
#	ifdef FORTIFY
	Fortify_SetOwner(info, 1);
	Fortify_SetScope(info, 1);
#	endif
	if (info) {
	  info->path = xstrdup(s, pszSrcFile, __LINE__);
	  if (!info->path)
	    free(info);
	  else {
#	    ifdef FORTIFY
	    Fortify_SetOwner(info->path, 1);
	    Fortify_SetScope(info->path, 1);
#	    endif
	    info->next = NULL;
	    if (!udirhead)
	      udirhead = info;
	    else
	      last->next = info;
	    last = info;
	  }
	}
      }
    }
    fclose(fp);
  }
}

VOID save_udirs(VOID)
{
  FILE *fp;
  LINKDIRS *info;
  CHAR s[CCHMAXPATH + 14];

  if (loadedudirs) {
    fUdirsChanged = FALSE;
    if (udirhead) {
      BldFullPathName(s, pFM2SaveDirectory, "USERDIRS.DAT");
      if (CheckDriveSpaceAvail(s, ullDATFileSpaceNeeded, 1) == 2)
        return; //already gave error msg
      fp = xfopen(s, "w", pszSrcFile, __LINE__);
      if (fp) {
	fputs(GetPString(IDS_USERDEFDIRSTEXT), fp);
	info = udirhead;
	while (info) {
	  fprintf(fp, "%0.*s\n", CCHMAXPATH, info->path);
	  info = info->next;
	}
	fclose(fp);
      }
    }
  }
}

/**
 * Add path to user directory list or last used directory list.
 * Callers need to check fUdirsChanged to know if user dirs change occured.
 * Callers need to check return code to know if last dirs change occured.
 * @param userdirs TRUE to process user directory list. Otherwise last used list.
 * @return TRUE if added, FALSE if already in list or error.
 */

BOOL add_udir(BOOL userdirs, CHAR *inpath)
{
  CHAR path[CCHMAXPATH];
  LINKDIRS *info;
  LINKDIRS *last = NULL;
  LINKDIRS *temp;

  if (inpath && *inpath) {
    if (DosQueryPathInfo(inpath, FIL_QUERYFULLNAME, path, sizeof(path)))
      strcpy(path, inpath);
    if (!userdirs && IsRoot(path))
      return FALSE;
    if (IsFullName(path)) {
      if (!loadedudirs)
	load_udirs();
      // Search user dir list first unless doing last dirs
      info = userdirs ? udirhead : ldirhead;
      while (info) {
	if (!stricmp(info->path, path))
	  return FALSE;			// Already in list
	last = info;			// Remember append to location
	info = info->next;
      }
      // Search last dir list unless doing just last dirs
      if (!userdirs) {
	info = udirhead;
	while (info) {
	  if (!stricmp(info->path, path))
	    return FALSE;
	  info = info->next;
	}
      }
      else {
	/* if adding manual directory, remove from auto list if present */
	info = ldirhead;
	temp = NULL;
	while (info) {
	  if (!stricmp(info->path, path)) {
	    if (temp)
	      temp->next = info->next;
	    else
	      ldirhead = info->next;
	    xfree(info->path, pszSrcFile, __LINE__);
	    free(info);
	    break;
	  }
	  temp = info;
	  info = info->next;
	}
      }
      // Append entry to end of user dirs list
      info = xmalloc(sizeof(LINKDIRS), pszSrcFile, __LINE__);
      if (info) {
#	ifdef FORTIFY
	Fortify_SetScope(info, 1);
#	endif
	info->path = xstrdup(path, pszSrcFile, __LINE__);
	if (!info->path)
	  free(info);
	else {
#	  ifdef FORTIFY
	  Fortify_SetScope(info->path, 1);
#	  endif
	  info->next = NULL;
	  if (userdirs) {
	    fUdirsChanged = TRUE;
	    if (!udirhead)
	      udirhead = info;
	    else
	      last->next = info;
	  }
	  else {
	    if (!ldirhead)
	      ldirhead = info;
	    else
	      last->next = info;
	  }
	  return TRUE;
	}
      }
    }
  }
  return FALSE;
}

//=== remove_udir - remove path from user dir list or last directory list ===

BOOL remove_udir(CHAR * path)
{
  LINKDIRS *info;
  LINKDIRS *last = NULL;

  if (path && *path) {
    if (!loadedudirs)
      load_udirs();
    info = udirhead;
    while (info) {
      if (!stricmp(info->path, path)) {
	if (last)
	  last->next = info->next;
	else
	  udirhead = info->next;
	xfree(info->path, pszSrcFile, __LINE__);
	free(info);
	fUdirsChanged = TRUE;
	return TRUE;
      }
      last = info;
      info = info->next;
    }

    info = ldirhead;
    last = NULL;
    while (info) {
      if (!stricmp(info->path, path)) {
	if (last)
	  last->next = info->next;
	else
	  ldirhead = info->next;
	xfree(info->path, pszSrcFile, __LINE__);
	free(info);
	return TRUE;
      }
      last = info;
      info = info->next;
    }
  }
  return FALSE;
}

BOOL remove_ldir(CHAR * path)
{
  LINKDIRS *info;
  LINKDIRS *last = NULL;

  if (path && *path) {
    info = ldirhead;
    while (info) {
      if (!stricmp(info->path, path)) {
	if (last)
	  last->next = info->next;
	else
	  ldirhead = info->next;
	xfree(info->path, pszSrcFile, __LINE__);
	free(info);
	return TRUE;
      }
      last = info;
      info = info->next;
    }
  }
  return FALSE;
}

# ifdef FORTIFY

VOID free_ldir(VOID)
{
  LINKDIRS *info, *next;

  info = ldirhead;
  while (info) {
    next = info->next;
    xfree(info->path, pszSrcFile, __LINE__);
    free(info);
    info = next;
  }
  ldirhead = NULL;
}

# endif

VOID free_udirs(VOID)
{
  LINKDIRS *info, *next;

  info = udirhead;
  while (info) {
    next = info->next;
    xfree(info->path, pszSrcFile, __LINE__);
    free(info);
    info = next;
  }
  udirhead = NULL;
}

VOID FillPathListBox(HWND hwnd, HWND hwnddrive, HWND hwnddir, CHAR * pszPath,
		     BOOL nounwriteable)
{
  /*
   * this function fills one or two list boxes with drive and directory
   * information showing all available drives and all directories off of
   * the directory represented by path.  This works independently of the
   * current directory.
   */

  CHAR szDrive[] = " :", szTemp[1032];
  FILEFINDBUF3 findbuf;
  HDIR hDir = HDIR_CREATE;
  SHORT sDrive;
  ULONG ulDriveNum, ulSearchCount = 1, ulDriveMap;

  DosError(FERR_DISABLEHARDERR);
  DosQCurDisk(&ulDriveNum, &ulDriveMap);
  if (hwnddrive)
    WinSendMsg(hwnddrive, LM_DELETEALL, MPVOID, MPVOID);
  if (hwnddrive != hwnddir && hwnddir)
    WinSendMsg(hwnddir, LM_DELETEALL, MPVOID, MPVOID);

  if (hwnddrive) {
    // Fill drive listbox
    for (sDrive = 0; sDrive < 26; sDrive++) {
      if (ulDriveMap & (1 << sDrive)) {
	*szDrive = (CHAR) (sDrive + 'A');
	if ((!nounwriteable || !(driveflags[sDrive] & DRIVE_NOTWRITEABLE)) &&
	    !(driveflags[sDrive] & (DRIVE_IGNORE | DRIVE_INVALID)))
	  WinSendMsg(hwnddrive, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0),
		     MPFROMP(szDrive));
      }
    }
    if (hwnddrive != hwnddir && pszPath && isalpha(*pszPath)
	&& pszPath[1] == ':') {
      *szDrive = toupper(*pszPath);
      WinSetWindowText(hwnddrive, szDrive);
    }
  }

  if (hwnddir) {
    // Fill directory listbox
    sprintf(szTemp,
	    "%s%s*",
	    pszPath, (pszPath[strlen(pszPath) - 1] == '\\') ? "" : "\\");
    DosError(FERR_DISABLEHARDERR);
    if (!DosFindFirst(szTemp,
		      &hDir,
		      FILE_DIRECTORY | MUST_HAVE_DIRECTORY |
		      FILE_READONLY | FILE_ARCHIVED | FILE_SYSTEM |
		      FILE_HIDDEN,
		      &findbuf,
		      sizeof(FILEFINDBUF3), &ulSearchCount, FIL_STANDARD)) {
      do {
	if (findbuf.attrFile & FILE_DIRECTORY) {
	  // Skip .. unless full path supplied
	  if (strcmp(findbuf.achName, "..") ||
	      strlen(pszPath) > 3 || pszPath[1] != ':') {
	    // Skip . allow ..
	    if (findbuf.achName[0] != '.' || findbuf.achName[1]) {
	      WinSendMsg(hwnddir,
			 LM_INSERTITEM,
			 MPFROM2SHORT(LIT_SORTASCENDING, 0),
			 MPFROMP(findbuf.achName));
	    }
	  }
	}
	ulSearchCount = 1;
      } while (!DosFindNext(hDir,
			    &findbuf, sizeof(FILEFINDBUF3), &ulSearchCount));
      DosFindClose(hDir);
    }
    DosError(FERR_DISABLEHARDERR);
  }
}

MRESULT EXPENTRY TextSubProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg) {
  case WM_CHAR:
    if (SHORT1FROMMP(mp1) & KC_KEYUP) {
      if ((SHORT1FROMMP(mp1) & KC_VIRTUALKEY) &&
	  (SHORT1FROMMP(mp2) & 255) == '\r')
	PostMsg(WinQueryWindow(hwnd, QW_PARENT), WM_COMMAND,
		MPFROM2SHORT(DID_OK, 0), MPVOID);
    }
    break;
  }
  return oldproc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY WalkDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  WALKER *wa;
  CHAR szBuff[CCHMAXPATH + 1], szBuffer[CCHMAXPATH + 1], *p;
  SHORT sSelect;
  static BOOL okay;		/* avoid combobox selecting as filled */
  static CHAR lastdir[CCHMAXPATH + 1];

  switch (msg) {
  case UM_SETUP2:
  case WM_INITDLG:
    okay = FALSE;
    *lastdir = 0;
    if (!mp2) {
      Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      WinDismissDlg(hwnd, 0);
      break;
    }
    wa = xmallocz(sizeof(WALKER), pszSrcFile, __LINE__);
    if (!wa) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    wa->size = (USHORT) sizeof(WALKER);
    WinSetWindowPtr(hwnd, QWL_USER, (PVOID) wa);
    wa->szReturnPath = (CHAR *)mp2;
    {
      PFNWP oldproc;

      oldproc = WinSubclassWindow(WinWindowFromID(hwnd, WALK_PATH),
				  (PFNWP) TextSubProc);
      if (oldproc)
	WinSetWindowPtr(WinWindowFromID(hwnd, WALK_PATH),
			QWL_USER, (PVOID) oldproc);
      WinSendDlgItemMsg(WinWindowFromID(hwnd, WALK_RECENT),
			CBID_EDIT,
			EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
      WinSendDlgItemMsg(WinWindowFromID(hwnd, WALK_RECENT),
			CBID_EDIT,
			EM_SETREADONLY, MPFROM2SHORT(TRUE, 0), MPVOID);
    }
    {
      SWP swp;
      ULONG size = sizeof(SWP);

      PrfQueryProfileData(fmprof, FM3Str, "WalkDir.Position", (PVOID) &swp, &size);
      WinSetWindowPos(hwnd,
		      HWND_TOP,
		      swp.x,
		      swp.y,
		      swp.cx,
		      swp.cy,
		      swp.fl);
    }
    PosOverOkay(hwnd);
    if (msg == UM_SETUP2)
      wa->nounwriteable = FALSE;
    else
      wa->nounwriteable = TRUE;
    if (!*wa->szReturnPath)
      strcpy(wa->szCurrentPath, pFM2SaveDirectory);
    else {
      strcpy(wa->szCurrentPath, wa->szReturnPath);
      MakeFullName(wa->szCurrentPath);
    }
    if (wa->nounwriteable &&
	(driveflags[toupper(*wa->szCurrentPath) - 'A'] &
	 DRIVE_NOTWRITEABLE)) {

      ULONG bd;

      strcpy(wa->szCurrentPath, "C:\\");
      if (DosQuerySysInfo(QSV_BOOT_DRIVE,
			  QSV_BOOT_DRIVE,
			  (PVOID) & bd, (ULONG) sizeof(ULONG)))
	bd = 3;
      *wa->szCurrentPath = (CHAR) bd + '@';
    }
    WinSendDlgItemMsg(hwnd,
		      WALK_PATH,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath);
    if (!loadedudirs)
      load_udirs();
    {					/* fill user list box */
      ULONG ulDriveNum, ulDriveMap;
      ULONG ulSearchCount;
      FILEFINDBUF3 findbuf;
      HDIR hDir;
      APIRET rc;
      LINKDIRS *info, *temp;

      DosError(FERR_DISABLEHARDERR);
      DosQCurDisk(&ulDriveNum, &ulDriveMap);
      info = udirhead;
      while (info) {
	if (IsFullName(info->path) &&
	    !(driveflags[toupper(*info->path) - 'A'] &
	      (DRIVE_IGNORE | DRIVE_INVALID))) {
	  DosError(FERR_DISABLEHARDERR);
	  hDir = HDIR_CREATE;
	  ulSearchCount = 1;
	  if (!IsRoot(info->path))
	    rc = DosFindFirst(info->path, &hDir, FILE_DIRECTORY |
			      MUST_HAVE_DIRECTORY | FILE_READONLY |
			      FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
			      &findbuf, sizeof(FILEFINDBUF3),
			      &ulSearchCount, FIL_STANDARD);
	  else {
	    rc = 0;
	    findbuf.attrFile = FILE_DIRECTORY;
	  }
	  if (!rc) {
	    if (!IsRoot(info->path))
	      DosFindClose(hDir);
	    if (findbuf.attrFile & FILE_DIRECTORY)
	      WinSendDlgItemMsg(hwnd, WALK_USERLIST, LM_INSERTITEM,
				MPFROM2SHORT(LIT_SORTASCENDING, 0),
				MPFROMP(info->path));
	    else {
	      temp = info->next;
	      remove_udir(info->path);
	      info = temp;
	      continue;
	    }
	  }
	  else if (!(ulDriveMap & (1 << (toupper(*info->path) - 'A')))) {
	    temp = info->next;
	    remove_udir(info->path);
	    info = temp;
	    continue;
	  }
	}
	info = info->next;
      }
      info = ldirhead;
      while (info) {
	if (IsFullName(info->path) &&
	    !(driveflags[toupper(*info->path) - 'A'] &
	      (DRIVE_IGNORE | DRIVE_INVALID))) {
	  DosError(FERR_DISABLEHARDERR);
	  hDir = HDIR_CREATE;
	  ulSearchCount = 1;
	  if (!IsRoot(info->path))
	    rc = DosFindFirst(info->path, &hDir, FILE_DIRECTORY |
			      MUST_HAVE_DIRECTORY | FILE_READONLY |
			      FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
			      &findbuf, sizeof(FILEFINDBUF3),
			      &ulSearchCount, FIL_STANDARD);
	  else {
	    rc = 0;
	    findbuf.attrFile = FILE_DIRECTORY;
	  }
	  if (!rc) {
	    if (!IsRoot(info->path))
	      DosFindClose(hDir);
	    if (findbuf.attrFile & FILE_DIRECTORY)
	      WinSendDlgItemMsg(hwnd, WALK_RECENT, LM_INSERTITEM,
				MPFROM2SHORT(LIT_SORTASCENDING, 0),
				MPFROMP(info->path));
	    else {
	      temp = info->next;
	      remove_ldir(info->path);
	      info = temp;
	      continue;
	    }
	    WinSetDlgItemText(hwnd, WALK_RECENT,
			      GetPString(IDS_WALKRECENTDIRSTEXT));
	  }
	  else if (!(ulDriveMap & (1 << (toupper(*info->path) - 'A')))) {
	    temp = info->next;
	    remove_ldir(info->path);
	    info = temp;
	    continue;
	  }
	}
	info = info->next;
      }
    }
    FillPathListBox(hwnd,
		    WinWindowFromID(hwnd, WALK_DRIVELIST),
		    WinWindowFromID(hwnd, WALK_DIRLIST),
		    wa->szCurrentPath, wa->nounwriteable);
    if (!PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID))
      okay = TRUE;
    {
      MRESULT ret;

      ret = WinDefDlgProc(hwnd, WM_INITDLG, mp1, mp2);
      WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      WinInvalidateRect(WinWindowFromID(hwnd, WALK_PATH), NULL, TRUE);
      return ret;
    }

  case UM_SETUP4:
    okay = TRUE;
    return 0;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, WALK_HELP), (HPS) 0, FALSE,
			TRUE);
    return 0;

  case WM_PRESPARAMCHANGED:
    {
      ULONG AttrFound, AttrValue[64], cbRetLen;

      cbRetLen = WinQueryPresParam(hwnd, (ULONG) mp1, 0, &AttrFound,
				   (ULONG) sizeof(AttrValue), &AttrValue, 0);
      if (cbRetLen) {
	switch (AttrFound) {
	case PP_FONTNAMESIZE:
	  PrfWriteProfileData(fmprof,
			      appname,
			      "WalkFont", (PVOID) AttrValue, cbRetLen);
	  *WalkFont = 0;
	  WalkFontSize = sizeof(WalkFont);
	  WinInvalidateRect(WinWindowFromID(hwnd, WALK_PATH), NULL, TRUE);
	  break;
	}
      }
    }
    break;

  case UM_SETUP3:
    save_udirs();
    if (hwndMain)
      PostMsg(hwndMain, UM_FILLUSERLIST, MPVOID, MPVOID);
    return 0;

  case UM_SETUP:
    {
      INT x;
      USHORT id[] = { WALK_PATH, WALK_DIRLIST, WALK_USERLIST,
	WALK_RECENT, 0
      };

      if (*WalkFont ||
	  (PrfQueryProfileData(fmprof,
			       appname,
			       "WalkFont",
			       (PVOID) WalkFont,
			       &WalkFontSize) && WalkFontSize)) {
	for (x = 0; id[x]; x++)
	  WinSetPresParam(WinWindowFromID(hwnd, id[x]),
			  PP_FONTNAMESIZE, WalkFontSize, (PVOID) WalkFont);
      }
    }
    return 0;

  case UM_CONTROL:
  case WM_CONTROL:
    wa = WinQueryWindowPtr(hwnd, QWL_USER);
    if (SHORT1FROMMP(mp1) == WALK_DRIVELIST ||
	SHORT1FROMMP(mp1) == WALK_DIRLIST ||
	SHORT1FROMMP(mp1) == WALK_USERLIST ||
	SHORT1FROMMP(mp1) == WALK_RECENT) {
      sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
					   SHORT1FROMMP(mp1),
					   LM_QUERYSELECTION, MPVOID, MPVOID);
      *szBuffer = 0;
      if (sSelect >= 0)
	WinSendDlgItemMsg(hwnd, SHORT1FROMMP(mp1), LM_QUERYITEMTEXT,
			  MPFROM2SHORT(sSelect, CCHMAXPATH),
			  MPFROMP(szBuffer));
    }
    switch (SHORT1FROMMP(mp1)) {
    case WALK_PATH:
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, WALK_HELP, GetPString(IDS_WALKCURRDIRTEXT));
      else if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, WALK_HELP,
			  GetPString(IDS_WALKDEFAULTHELPTEXT));
      break;

    case WALK_RECENT:
      if (okay && SHORT2FROMMP(mp1) == CBN_LBSELECT) {

	ULONG ulSearchCount;
	FILEFINDBUF3 findbuf;
	HDIR hDir;
	APIRET rc;

	// *szBuffer = 0;
	// WinQueryDlgItemText(hwnd,WALK_RECENT,CCHMAXPATH,szBuffer);
	if (!*szBuffer)
	  break;
	DosError(FERR_DISABLEHARDERR);
	hDir = HDIR_CREATE;
	ulSearchCount = 1;
	if (!IsRoot(szBuffer)) {
	  rc = DosFindFirst(szBuffer, &hDir, FILE_DIRECTORY |
			    MUST_HAVE_DIRECTORY | FILE_READONLY |
			    FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
			    &findbuf, sizeof(FILEFINDBUF3),
			    &ulSearchCount, FIL_STANDARD);
	  if (!rc)
	    DosFindClose(hDir);
	}
	else {
	  findbuf.attrFile = FILE_DIRECTORY;
	  rc = 0;
	}
	if (rc)
	  Dos_Error(MB_CANCEL, rc, hwnd, pszSrcFile, __LINE__,
		    "xDosFindFirst");
	else if (~findbuf.attrFile & FILE_DIRECTORY)
	  Runtime_Error(pszSrcFile, __LINE__, "not a directory");
	else {
	  strcpy(wa->szCurrentPath, szBuffer);
	  WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath);
	  WinSetDlgItemText(hwnd, WALK_RECENT, wa->szCurrentPath);
	  FillPathListBox(hwnd,
			  WinWindowFromID(hwnd, WALK_DRIVELIST),
			  WinWindowFromID(hwnd, WALK_DIRLIST),
			  wa->szCurrentPath, FALSE);
	}
      }
      else if (SHORT2FROMMP(mp1) == CBN_ENTER)
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
      else if (SHORT2FROMMP(mp1) == CBN_SHOWLIST)
	WinSetDlgItemText(hwnd, WALK_HELP,
			  GetPString(IDS_WALKRECENTDIRSHELPTEXT));
      break;

    case WALK_USERLIST:
      if (okay && *szBuffer && SHORT2FROMMP(mp1) == LN_SELECT) {

	ULONG ulSearchCount;
	FILEFINDBUF3 findbuf;
	HDIR hDir;
	APIRET rc;

	DosError(FERR_DISABLEHARDERR);
	hDir = HDIR_CREATE;
	ulSearchCount = 1;
	if (!IsRoot(szBuffer)) {
	  rc = DosFindFirst(szBuffer,
			    &hDir,
			    FILE_DIRECTORY |
			    MUST_HAVE_DIRECTORY | FILE_READONLY |
			    FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
			    &findbuf,
			    sizeof(FILEFINDBUF3),
			    &ulSearchCount, FIL_STANDARD);
	  if (!rc)
	    DosFindClose(hDir);
	}
	else {
	  findbuf.attrFile = FILE_DIRECTORY;
	  rc = 0;
	}
	if (rc)
	  Dos_Error(MB_CANCEL, rc, hwnd, pszSrcFile, __LINE__,
		    "xDosFindFirst");
	else if (~findbuf.attrFile & FILE_DIRECTORY)
	  Runtime_Error(pszSrcFile, __LINE__, "not a directory");
	else {
	  strcpy(wa->szCurrentPath, szBuffer);
	  WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath);
	  FillPathListBox(hwnd,
			  WinWindowFromID(hwnd, WALK_DRIVELIST),
			  WinWindowFromID(hwnd, WALK_DIRLIST),
			  wa->szCurrentPath, FALSE);
	}
      }
      else if (SHORT2FROMMP(mp1) == LN_ENTER)
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
      else if (SHORT2FROMMP(mp1) == LN_SETFOCUS)
	WinSetDlgItemText(hwnd,
			  WALK_HELP, GetPString(IDS_WALKUSERDIRSHELPTEXT));
      else if (SHORT2FROMMP(mp1) == LN_KILLFOCUS)
	WinSetDlgItemText(hwnd,
			  WALK_HELP, GetPString(IDS_WALKDEFAULTHELPTEXT));
      break;

    case WALK_DRIVELIST:
      if (okay && *szBuffer && SHORT2FROMMP(mp1) == LN_ENTER) {

	ULONG ulDirLen = CCHMAXPATH;
	APIRET rc;

	rc = DosQCurDir(toupper(*szBuffer) - '@', &szBuff[3], &ulDirLen);
	if (!rc) {
	  strcpy(wa->szCurrentPath, "C:\\");
	  *wa->szCurrentPath = toupper(*szBuffer);
	  WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath);
	  FillPathListBox(hwnd,
			  WinWindowFromID(hwnd, WALK_DRIVELIST),
			  WinWindowFromID(hwnd, WALK_DIRLIST),
			  wa->szCurrentPath, FALSE);
	}
      }
      else if (SHORT2FROMMP(mp1) == LN_SETFOCUS)
	WinSetDlgItemText(hwnd, WALK_HELP,
			  GetPString(IDS_WALKDRIVELISTHELPTEXT));
      else if (SHORT2FROMMP(mp1) == LN_KILLFOCUS)
	WinSetDlgItemText(hwnd, WALK_HELP,
			  GetPString(IDS_WALKDEFAULTHELPTEXT));
      break;

    case WALK_DIRLIST:
      if (okay && SHORT2FROMMP(mp1) == LN_ENTER) {

	ULONG ulSearchCount;
	FILEFINDBUF3 findbuf;
	HDIR hDir;
	APIRET rc;

	bstrip(szBuffer);
	if (*szBuffer) {
	  strcpy(szBuff, wa->szCurrentPath);
	  if (szBuff[strlen(szBuff) - 1] != '\\')
	    strcat(szBuff, "\\");
	  strcat(szBuff, szBuffer);
	  MakeFullName(szBuff);
	  DosError(FERR_DISABLEHARDERR);
	  hDir = HDIR_CREATE;
	  ulSearchCount = 1;
	  if (!IsRoot(szBuff)) {
	    rc = DosFindFirst(szBuff,
			      &hDir,
			      FILE_DIRECTORY |
			      MUST_HAVE_DIRECTORY | FILE_READONLY |
			      FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
			      &findbuf,
			      sizeof(FILEFINDBUF3),
			      &ulSearchCount, FIL_STANDARD);
	    if (!rc)
	      DosFindClose(hDir);
	  }
	  else {
	    findbuf.attrFile = FILE_DIRECTORY;
	    rc = 0;
	  }
	  if (!rc && (findbuf.attrFile & FILE_DIRECTORY)) {
	    strcpy(wa->szCurrentPath, szBuff);
	    WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath);
	    FillPathListBox(hwnd,
			    WinWindowFromID(hwnd, WALK_DRIVELIST),
			    WinWindowFromID(hwnd, WALK_DIRLIST),
			    wa->szCurrentPath, FALSE);
	  }
	}
      }
      else if (SHORT2FROMMP(mp1) == LN_SETFOCUS)
	WinSetDlgItemText(hwnd, WALK_HELP,
			  GetPString(IDS_WALKDIRLISTHELPTEXT));
      else if (SHORT2FROMMP(mp1) == LN_KILLFOCUS)
	WinSetDlgItemText(hwnd, WALK_HELP,
			  GetPString(IDS_WALKDEFAULTHELPTEXT));
      break;
    }
    return 0;

  case WM_COMMAND:
    wa = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!wa)
      WinDismissDlg(hwnd, 0);
    *szBuff = 0;
    WinQueryDlgItemText(hwnd, WALK_PATH, CCHMAXPATH, szBuff);
    bstrip(szBuff);
    while ((p = strchr(szBuff, '/')) != NULL)
      *p = '\\';
    while (strlen(szBuff) > 3 && szBuff[strlen(szBuff) - 1] == '\\')
      szBuff[strlen(szBuff) - 1] = 0;
    MakeFullName(szBuff);
    if (*szBuff && stricmp(szBuff, wa->szCurrentPath) && SHORT1FROMMP(mp1) != DID_CANCEL) {
      if (!SetDir(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				 QW_OWNER), hwnd, szBuff, 0))
	strcpy(wa->szCurrentPath, szBuff);
      else
	return 0;
    }
    WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath);
    switch (SHORT1FROMMP(mp1)) {
    case WALK_ADD:
      *szBuff = 0;
      WinQueryDlgItemText(hwnd, WALK_PATH, CCHMAXPATH, szBuff);
      bstrip(szBuff);
      while ((p = strchr(szBuff, '/')) != NULL)
	*p = '\\';
      if (*szBuff && !IsFile(szBuff)) {
	MakeFullName(szBuff);
	add_udir(TRUE, szBuff);
	if (fUdirsChanged) {
	  WinSendDlgItemMsg(hwnd,
			    WALK_USERLIST,
			    LM_INSERTITEM,
			    MPFROM2SHORT(LIT_SORTASCENDING, 0),
			    MPFROMP(szBuff));
	  wa->changed = 1;
	}
      }
      break;

    case WALK_DELETE:
      *szBuff = 0;
      WinQueryDlgItemText(hwnd, WALK_PATH, CCHMAXPATH, szBuff);
      bstrip(szBuff);
      while ((p = strchr(szBuff, '/')) != NULL)
	*p = '\\';
      if (*szBuff && !IsFile(szBuff)) {
	MakeFullName(szBuff);
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    WALK_USERLIST,
					    LM_SEARCHSTRING,
					    MPFROM2SHORT(0, LIT_FIRST),
					    MPFROMP(szBuff));
	if (sSelect >= 0) {
	  WinSendDlgItemMsg(hwnd,
			    WALK_USERLIST,
			    LM_DELETEITEM, MPFROM2SHORT(sSelect, 0), MPVOID);
	  remove_udir(szBuff);
	  wa->changed = 1;
	}
      }
      break;

    case DID_OK:
      if (*wa->szCurrentPath) {
	strcpy(wa->szReturnPath, wa->szCurrentPath);
	MakeValidDir(wa->szReturnPath);
	if (fAutoAddAllDirs)
	  add_udir(FALSE, wa->szReturnPath);
	if (fChangeTarget) {
	  strcpy(targetdir, wa->szReturnPath);
	  PrfWriteProfileString(fmprof, appname, "Targetdir", targetdir);
	}
      }
      {
	SWP swp;
	ULONG size = sizeof(SWP);

	WinQueryWindowPos(hwnd, &swp);
	PrfWriteProfileData(fmprof, FM3Str, "WalkDir.Position", (PVOID) &swp,
			    size);
      }
      if (wa->changed)
	WinSendMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
      WinDismissDlg(hwnd, 1);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_WALKEM, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_CANCEL:
      {
	SWP swp;
	ULONG size = sizeof(SWP);

	WinQueryWindowPos(hwnd, &swp);
	PrfWriteProfileData(fmprof, FM3Str, "WalkDir.Position", (PVOID) &swp,
			    size);
      }
      if (wa->changed)
	WinSendMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
      free(wa);
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;

  case WM_CLOSE:
	break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY WalkAllDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    return WalkDlgProc(hwnd, UM_SETUP2, mp1, mp2);
  }
  return WalkDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY WalkCopyDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSetWindowText(hwnd, GetPString(IDS_WALKCOPYDLGTEXT));
    return WalkDlgProc(hwnd, UM_SETUP2, mp1, mp2);
  }
  return WalkDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY WalkMoveDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSetWindowText(hwnd, GetPString(IDS_WALKMOVEDLGTEXT));
    return WalkDlgProc(hwnd, UM_SETUP2, mp1, mp2);
  }
  return WalkDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY WalkExtractDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				    MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSetWindowText(hwnd, GetPString(IDS_WALKEXTRACTDLGTEXT));
    return WalkDlgProc(hwnd, UM_SETUP2, mp1, mp2);
  }
  return WalkDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY WalkTargetDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    {
      char s[CCHMAXPATH + 32];

      sprintf(s,
	      GetPString(IDS_WALKTARGETDLGTEXT),
	      (*targetdir) ?
	      NullStr :
	      " (",
	      (*targetdir) ?
	      NullStr : GetPString(IDS_NONE), (*targetdir) ? NullStr : ")");
      WinSetWindowText(hwnd, s);
    }
    return WalkDlgProc(hwnd, UM_SETUP2, mp1, mp2);
  }
  return WalkDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY WalkTwoDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  WALK2 *wa;
  CHAR szBuff[CCHMAXPATH + 1], szBuffer[CCHMAXPATH + 1], *p;
  SHORT sSelect;
  static BOOL okay;		/* avoid combobox selecting as filled */

  switch (msg) {
  case UM_SETUP2:
  case WM_INITDLG:
    okay = FALSE;
    if (!mp2) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    wa = mp2;
    {
      PFNWP oldproc;

      oldproc = WinSubclassWindow(WinWindowFromID(hwnd, WALK_PATH),
				  (PFNWP) TextSubProc);
      if (oldproc)
	WinSetWindowPtr(WinWindowFromID(hwnd, WALK_PATH),
			QWL_USER, (PVOID) oldproc);
      oldproc = WinSubclassWindow(WinWindowFromID(hwnd, WALK2_PATH),
				  (PFNWP) TextSubProc);
      if (oldproc)
	WinSetWindowPtr(WinWindowFromID(hwnd, WALK2_PATH),
			QWL_USER, (PVOID) oldproc);
    }
    {
      SWP swp;
      ULONG size = sizeof(SWP);

      PrfQueryProfileData(fmprof, FM3Str, "WalkDir2.Position", (PVOID) &swp, &size);
      WinSetWindowPos(hwnd,
		      HWND_TOP,
		      swp.x,
		      swp.y,
		      swp.cx,
		      swp.cy,
		      swp.fl);
    }
    if (!*wa->szCurrentPath1)
      strcpy(wa->szCurrentPath1, pFM2SaveDirectory);
    MakeFullName(wa->szCurrentPath1);
    if (!*wa->szCurrentPath2)
      strcpy(wa->szCurrentPath2, pFM2SaveDirectory);
    MakeFullName(wa->szCurrentPath2);
    WinSendDlgItemMsg(hwnd,
		      WALK_PATH,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath1);
    WinSendDlgItemMsg(hwnd, WALK2_PATH, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSetDlgItemText(hwnd, WALK2_PATH, wa->szCurrentPath2);
    FillPathListBox(hwnd,
		    WinWindowFromID(hwnd, WALK_DRIVELIST),
		    WinWindowFromID(hwnd, WALK_DIRLIST),
		    wa->szCurrentPath1, FALSE);
    FillPathListBox(hwnd,
		    WinWindowFromID(hwnd, WALK2_DRIVELIST),
		    WinWindowFromID(hwnd, WALK2_DIRLIST),
		    wa->szCurrentPath2, FALSE);
    if (!PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID))
      okay = TRUE;
    {
      MRESULT ret;

      ret = WinDefDlgProc(hwnd, WM_INITDLG, mp1, mp2);
      WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      WinInvalidateRect(WinWindowFromID(hwnd, WALK_PATH), NULL, TRUE);
      WinInvalidateRect(WinWindowFromID(hwnd, WALK2_PATH), NULL, TRUE);
      return ret;
    }

  case UM_SETUP4:
    okay = TRUE;
    return 0;

  case WM_PRESPARAMCHANGED:
    {
      ULONG AttrFound, AttrValue[64], cbRetLen;

      cbRetLen = WinQueryPresParam(hwnd, (ULONG) mp1, 0, &AttrFound,
				   (ULONG) sizeof(AttrValue), &AttrValue, 0);
      if (cbRetLen) {
	switch (AttrFound) {
	case PP_FONTNAMESIZE:
	  PrfWriteProfileData(fmprof,
			      appname,
			      "WalkFont", (PVOID) AttrValue, cbRetLen);
	  *WalkFont = 0;
	  WalkFontSize = sizeof(WalkFont);
	  WinInvalidateRect(WinWindowFromID(hwnd, WALK_PATH), NULL, TRUE);
	  break;
	}
      }
    }
    break;

  case UM_SETUP:
    {
      INT x;
      USHORT id[] = { WALK_PATH, WALK_DIRLIST,
	WALK2_PATH, WALK2_DIRLIST, 0
      };

      if (*WalkFont ||
	  (PrfQueryProfileData(fmprof,
			       appname,
			       "WalkFont",
			       (PVOID) WalkFont,
			       &WalkFontSize) && WalkFontSize)) {
	for (x = 0; id[x]; x++)
	  WinSetPresParam(WinWindowFromID(hwnd, id[x]),
			  PP_FONTNAMESIZE, WalkFontSize, (PVOID) WalkFont);
      }
    }
    return 0;

  case UM_CONTROL:
  case WM_CONTROL:
    wa = WinQueryWindowPtr(hwnd, QWL_USER);
    if (SHORT1FROMMP(mp1) == WALK_DRIVELIST ||
	SHORT1FROMMP(mp1) == WALK_DIRLIST ||
	SHORT1FROMMP(mp1) == WALK2_DRIVELIST ||
	SHORT1FROMMP(mp1) == WALK2_DIRLIST) {
      sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
					   SHORT1FROMMP(mp1),
					   LM_QUERYSELECTION, MPVOID, MPVOID);
      *szBuffer = 0;
      if (sSelect >= 0)
	WinSendDlgItemMsg(hwnd, SHORT1FROMMP(mp1), LM_QUERYITEMTEXT,
			  MPFROM2SHORT(sSelect, CCHMAXPATH),
			  MPFROMP(szBuffer));
    }
    switch (SHORT1FROMMP(mp1)) {
    case WALK_DRIVELIST:
      if (okay && *szBuffer && SHORT2FROMMP(mp1) == LN_ENTER) {

	ULONG ulDirLen = CCHMAXPATH;
	APIRET rc;

	rc = DosQCurDir(toupper(*szBuffer) - '@', &szBuff[3], &ulDirLen);
	if (!rc) {
	  strcpy(wa->szCurrentPath1, "C:\\");
	  *wa->szCurrentPath1 = toupper(*szBuffer);
	  WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath1);
	  FillPathListBox(hwnd,
			  WinWindowFromID(hwnd, WALK_DRIVELIST),
			  WinWindowFromID(hwnd, WALK_DIRLIST),
			  wa->szCurrentPath1, FALSE);
	}
      }
      break;

    case WALK_DIRLIST:
      if (okay && SHORT2FROMMP(mp1) == LN_ENTER) {

	ULONG ulSearchCount;
	FILEFINDBUF3 findbuf;
	HDIR hDir;
	APIRET rc;

	bstrip(szBuffer);
	if (*szBuffer) {
	  strcpy(szBuff, wa->szCurrentPath1);
	  if (szBuff[strlen(szBuff) - 1] != '\\')
	    strcat(szBuff, "\\");
	  strcat(szBuff, szBuffer);
	  MakeFullName(szBuff);
	  DosError(FERR_DISABLEHARDERR);
	  hDir = HDIR_CREATE;
	  ulSearchCount = 1;
	  if (!IsRoot(szBuff)) {
	    rc = DosFindFirst(szBuff,
			      &hDir,
			      FILE_DIRECTORY |
			      MUST_HAVE_DIRECTORY | FILE_READONLY |
			      FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
			      &findbuf,
			      sizeof(FILEFINDBUF3),
			      &ulSearchCount, FIL_STANDARD);
	    if (!rc)
	      DosFindClose(hDir);
	  }
	  else {
	    findbuf.attrFile = FILE_DIRECTORY;
	    rc = 0;
	  }
	  if (!rc && (findbuf.attrFile & FILE_DIRECTORY)) {
	    strcpy(wa->szCurrentPath1, szBuff);
	    WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath1);
	    FillPathListBox(hwnd,
			    WinWindowFromID(hwnd, WALK_DRIVELIST),
			    WinWindowFromID(hwnd, WALK_DIRLIST),
			    wa->szCurrentPath1, FALSE);
	  }
	}
      }
      break;

    case WALK2_DRIVELIST:
      if (okay && *szBuffer && SHORT2FROMMP(mp1) == LN_ENTER) {

	ULONG ulDirLen = CCHMAXPATH;
	APIRET rc;

	rc = DosQCurDir(toupper(*szBuffer) - '@', &szBuff[3], &ulDirLen);
	if (!rc) {
	  strcpy(wa->szCurrentPath2, "C:\\");
	  *wa->szCurrentPath2 = toupper(*szBuffer);
	  WinSetDlgItemText(hwnd, WALK2_PATH, wa->szCurrentPath2);
	  FillPathListBox(hwnd,
			  WinWindowFromID(hwnd, WALK2_DRIVELIST),
			  WinWindowFromID(hwnd, WALK2_DIRLIST),
			  wa->szCurrentPath2, FALSE);
	}
      }
      break;

    case WALK2_DIRLIST:
      if (okay && SHORT2FROMMP(mp1) == LN_ENTER) {

	ULONG ulSearchCount;
	FILEFINDBUF3 findbuf;
	HDIR hDir;
	APIRET rc;

	bstrip(szBuffer);
	if (*szBuffer) {
	  strcpy(szBuff, wa->szCurrentPath2);
	  if (szBuff[strlen(szBuff) - 1] != '\\')
	    strcat(szBuff, "\\");
	  strcat(szBuff, szBuffer);
	  MakeFullName(szBuff);
	  DosError(FERR_DISABLEHARDERR);
	  hDir = HDIR_CREATE;
	  ulSearchCount = 1;
	  if (!IsRoot(szBuff)) {
	    rc = DosFindFirst(szBuff,
			      &hDir,
			      FILE_DIRECTORY |
			      MUST_HAVE_DIRECTORY | FILE_READONLY |
			      FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
			      &findbuf,
			      sizeof(FILEFINDBUF3),
			      &ulSearchCount, FIL_STANDARD);
	    if (!rc)
	      DosFindClose(hDir);
	  }
	  else {
	    findbuf.attrFile = FILE_DIRECTORY;
	    rc = 0;
	  }
	  if (!rc && (findbuf.attrFile & FILE_DIRECTORY)) {
	    strcpy(wa->szCurrentPath2, szBuff);
	    WinSetDlgItemText(hwnd, WALK2_PATH, wa->szCurrentPath2);
	    FillPathListBox(hwnd,
			    WinWindowFromID(hwnd, WALK2_DRIVELIST),
			    WinWindowFromID(hwnd, WALK2_DIRLIST),
			    wa->szCurrentPath2, FALSE);
	  }
	}
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    wa = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!wa)
      WinDismissDlg(hwnd, 0);
    *szBuff = 0;
    WinQueryDlgItemText(hwnd, WALK_PATH, CCHMAXPATH, szBuff);
    bstrip(szBuff);
    while ((p = strchr(szBuff, '/')) != NULL)
      *p = '\\';
    while (strlen(szBuff) > 3 && szBuff[strlen(szBuff) - 1] == '\\')
      szBuff[strlen(szBuff) - 1] = 0;
    MakeFullName(szBuff);
    if (*szBuff && stricmp(szBuff, wa->szCurrentPath1) && SHORT1FROMMP(mp1) != DID_CANCEL) {
      if (!SetDir(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				 QW_OWNER), hwnd, szBuff, 0))
	strcpy(wa->szCurrentPath1, szBuff);
      else
	return 0;
    }
    WinSetDlgItemText(hwnd, WALK_PATH, wa->szCurrentPath1);
    *szBuff = 0;
    WinQueryDlgItemText(hwnd, WALK2_PATH, CCHMAXPATH, szBuff);
    bstrip(szBuff);
    while ((p = strchr(szBuff, '/')) != NULL)
      *p = '\\';
    while (strlen(szBuff) > 3 && szBuff[strlen(szBuff) - 1] == '\\')
      szBuff[strlen(szBuff) - 1] = 0;
    MakeFullName(szBuff);
    if (*szBuff && stricmp(szBuff, wa->szCurrentPath2) && SHORT1FROMMP(mp1) != DID_CANCEL) {
      if (!SetDir(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				 QW_OWNER), hwnd, szBuff, 0))
	strcpy(wa->szCurrentPath2, szBuff);
      else
	return 0;
    }
    WinSetDlgItemText(hwnd, WALK2_PATH, wa->szCurrentPath2);
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      {
      SWP swp;
      ULONG size = sizeof(SWP);

      WinQueryWindowPos(hwnd, &swp);
      PrfWriteProfileData(fmprof, FM3Str, "WalkDir2.Position", (PVOID) &swp,
			  size);
      }
      WinDismissDlg(hwnd, 1);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_WALKEM2, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_CANCEL:
      {
      SWP swp;
      ULONG size = sizeof(SWP);

      WinQueryWindowPos(hwnd, &swp);
      PrfWriteProfileData(fmprof, FM3Str, "WalkDir2.Position", (PVOID) &swp,
			  size);
      }
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;

  case WM_CLOSE:
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY WalkTwoCmpDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSetWindowText(hwnd, GetPString(IDS_WALKCOMPAREDLGTEXT));
    return WalkTwoDlgProc(hwnd, UM_SETUP2, mp1, mp2);
  }
  return WalkTwoDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY WalkTwoSetDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSetWindowText(hwnd, GetPString(IDS_WALKSETDIRSDLGTEXT));
    return WalkTwoDlgProc(hwnd, UM_SETUP2, mp1, mp2);
  }
  return WalkTwoDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(WALKER,FillPathListBox,WalkDlgProc,TextSubProc)
#pragma alloc_text(WALKER,WalkAllDlgProc,WalkCopyDlgProc)
#pragma alloc_text(WALKER,WalkMoveDlgProc,WalkExtractDlgProc,WalkTargetDlgProc)
#pragma alloc_text(WALK2,WalkTwoDlgProc,WalkTwoCmpDlgProc,WalkTwoSetDlgProc)
#pragma alloc_text(UDIRS,add_udir,remove_udir,remove_ldir,load_udirs)
#pragma alloc_text(UDIRS,save_udirs,load_setups,save_setups,add_setups)
#pragma alloc_text(UDIRS,remove_setup)
