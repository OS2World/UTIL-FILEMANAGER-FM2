
/***********************************************************************

  $Id$

  Info windows

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2010 Steven H. Levine

  16 Oct 02 SHL Handle large partitions
  12 Feb 03 SHL FileInfoProc: standardize EA math
  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  05 Jun 05 SHL Use QWL_USER
  14 Jul 06 SHL Use Runtime_Error
  24 Mar 07 SHL Correct FileInfoProc binary file detect
  24 Mar 07 SHL Correct FileInfoProc/IconProc race crash
  19 Apr 07 SHL Sync with AcceptOneDrop GetOneDrop mods
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  25 Aug 07 SHL Drop list from FILESTUF - data not static
  25 Aug 07 SHL IconProc: do not use freed memory - random bad things happen
  27 Sep 07 SHL Correct ULONGLONG size formatting
  30 Dec 07 GKY Use CommaFmtULL
  29 Feb 08 GKY Use xfree where appropriate
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  25 Dec 08 GKY Add DRIVE_RSCANNED flag to monitor for the first recursive drive scan per session
                to prevent duplicate directory names in tree following a copy before initial scan.
  11 Jan 08 GKY Add Write verify off and recures scan to drive info display when appropriate.
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  12 Jul 09 GKY Remove code to update recursive scan setting which isn't user setable
  22 Jul 09 GKY Check if drives support EAs add driveflag for this
  22 Jul 09 GKY Add LocalHD driveflag
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "draglist.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "inis.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "makelist.h"			// AddToList
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "attribs.h"			// AttrListDlgProc
#include "defview.h"			// DefaultView
#include "info.h"
#include "valid.h"			// CheckDrive, IsBinary
#include "seticon.h"			// SetIconDlgProc
#include "droplist.h"			// AcceptOneDrop, DropHelp, GetOneDrop
#include "eas.h"			// DisplayEAsProc
#include "misc.h"			// DrawTargetEmphasis
#include "notify.h"			// Notify
#include "shadow.h"			// OpenObject
#include "chklist.h"			// PopupMenu
#include "presparm.h"			// SetPresParams
#include "strips.h"			// bstrip
#include "i18nutil.h"			// CommaFmtULL
#include "wrappers.h"			// xDosFindFirst
#include "fortify.h"

// Data definitions
#pragma data_seg(DATA1)
static PSZ pszSrcFile = __FILE__;

#pragma data_seg(GLOBAL1)
INT driveflags[26];

CHAR *FlagMsg(CHAR drive, CHAR * buffer)
{
  ULONG x;
  BOOL once = FALSE;
  register CHAR *p;

  if (buffer) {
    *buffer = 0;
    p = buffer;
    if (isalpha(drive)) {
      if (driveflags[toupper(drive) - 'A']) {
	for (x = IDS_FLREMOVABLETEXT; x < IDS_FLRECURSESCANDONETEXT + 1; x++) {
	  if (driveflags[toupper(drive) - 'A'] &
	      (1 << (x - IDS_FLREMOVABLETEXT))) {
	    if (once) {
	      *p = ' ';
	      p++;
	    }
	    else
	      once = TRUE;
	    *p = '[';
	    p++;
	    strcpy(p, GetPString(x));
	    p += strlen(p);
	    *p = ']';
	    p++;
	    *p = 0;
	  }
	}
      }
      else
	strcpy(buffer, "[None]");
    }
  }
  return buffer;
}

MRESULT EXPENTRY DrvInfoProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CHAR *pszFileName;
  CHAR szMB[20];
  CHAR szKB[20];
  CHAR szUnits[20];
  APIRET rc;

  switch (msg) {
  case WM_INITDLG:
    if (mp2) {

      CHAR s[CCHMAXPATH * 2];
      ULONG type;

      pszFileName = (CHAR *)mp2;
      WinSetWindowPtr(hwnd, QWL_USER, (PVOID) pszFileName);
      WinSendDlgItemMsg(hwnd,
			INFO_LABEL,
			EM_SETTEXTLIMIT,
			MPFROM2SHORT(CCHMAXPATHCOMP, 0), MPVOID);
      if (!(driveflags[toupper(*pszFileName) - 'A'] & DRIVE_NOSTATS)){
      WinSendDlgItemMsg(hwnd,
			INFO_FREE,
			SLM_SETSLIDERINFO,
			MPFROM2SHORT(SMA_SLIDERARMDIMENSIONS, 0),
			MPFROM2SHORT(0, 0));
      WinSendDlgItemMsg(hwnd,
			INFO_USED,
			SLM_SETSLIDERINFO,
			MPFROM2SHORT(SMA_SLIDERARMDIMENSIONS, 0),
			MPFROM2SHORT(0, 0));
      }
      if (driveflags[toupper(*pszFileName) - 'A'] & DRIVE_NOTWRITEABLE) {
	WinSendDlgItemMsg(hwnd,
			  INFO_LABEL,
			  EM_SETREADONLY, MPFROM2SHORT(TRUE, 0), MPVOID);
	WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, DID_OK));
      }
      if (IsFullName(pszFileName)) {

	CHAR FileSystem[CCHMAXPATH * 2];

	sprintf(FileSystem,
		GetPString(IDS_DRIVEINFOTITLETEXT), toupper(*pszFileName));
	WinSetWindowText(hwnd, FileSystem);
	if (CheckDrive(toupper(*pszFileName), FileSystem, &type) != -1){

	  FSALLOCATE fsa;

	  if (type & (DRIVE_REMOTE | DRIVE_ZIPSTREAM | DRIVE_VIRTUAL)) {

	    CHAR Path[3], *pfsn, *pfsd;
	    ULONG Size;
	    APIRET rc;
	    PFSQBUFFER2 pfsq;

	    Path[0] = toupper(*pszFileName);
	    Path[1] = ':';
	    Path[2] = 0;
	    Size = sizeof(s);
	    DosError(FERR_DISABLEHARDERR);
	    rc = DosQueryFSAttach(Path,
				  0, FSAIL_QUERYNAME, (PFSQBUFFER2) s, &Size);
	    if (!rc) {
	      pfsq = (PFSQBUFFER2) s;
	      pfsn = (PCHAR)(pfsq->szName) + pfsq->cbName + 1;
	      pfsd = pfsn + pfsq->cbFSDName + 1;
	      if (pfsq->cbFSAData && pfsd && *pfsd) {
		sprintf(s, " (%s)", pfsd);
		WinSetDlgItemText(hwnd, INFO_REALPATH, s);
	      }
	    }
	  }

	  DosError(FERR_DISABLEHARDERR);
	  if (!DosQueryFSInfo(toupper(*pszFileName) - '@',
			      FSIL_ALLOC, &fsa, sizeof(FSALLOCATE))) {

	    struct
	    {
	      ULONG serial;
	      CHAR volumelength;
	      CHAR volumelabel[CCHMAXPATH];
	    }
	    volser;
	    USHORT percentfree, percentused;

	    memset(&volser, 0, sizeof(volser));
	    DosError(FERR_DISABLEHARDERR);
	    if (!DosQueryFSInfo(toupper(*pszFileName) - '@',
				FSIL_VOLSER,
				&volser, (ULONG) sizeof(volser))) {
	      WinSetDlgItemText(hwnd, INFO_FS, FileSystem);
	      WinSetDlgItemText(hwnd, INFO_LABEL, volser.volumelabel);
	      sprintf(s, "%lx", volser.serial);
	      WinSetDlgItemText(hwnd, INFO_SERIAL, s);
	      FlagMsg(*pszFileName, s);
	      WinSetDlgItemText(hwnd, INFO_FLAGS, s);
	      if (!(driveflags[toupper(*pszFileName) - 'A'] & DRIVE_NOSTATS)){
	      CommaFmtULL(szMB, sizeof(szMB),
			  (ULONGLONG) fsa.cUnit *
			  (fsa.cSectorUnit * fsa.cbSector), 'M');
	      CommaFmtULL(szKB, sizeof(szKB),
			  (ULONGLONG) fsa.cUnit *
			  (fsa.cSectorUnit * fsa.cbSector), 'K');
	      CommaFmtULL(szUnits, sizeof(szUnits),
			  (ULONGLONG) fsa.cUnit, ' ');
	      sprintf(s, "%s, %s, %s %s%s", szMB, szKB, szUnits, GetPString(IDS_UNITTEXT), &"s"[fsa.cUnit == 1L]);	// hack cough
	      WinSetDlgItemText(hwnd, INFO_TOTAL, s);

	      CommaFmtULL(szMB, sizeof(szMB),
			  (ULONGLONG) fsa.cUnitAvail *
			  (fsa.cSectorUnit * fsa.cbSector), 'M');
	      CommaFmtULL(szKB, sizeof(szKB),
			  (ULONGLONG) fsa.cUnitAvail *
			  (fsa.cSectorUnit * fsa.cbSector), 'K');
	      CommaFmtULL(szUnits, sizeof(szUnits),
			  (ULONGLONG) fsa.cUnitAvail, ' ');
	      sprintf(s, "%s, %s, %s %s%s",
	              szMB, szKB, szUnits,
	              GetPString(IDS_UNITTEXT), &"s"[fsa.cUnitAvail == 1L]);
	      WinSetDlgItemText(hwnd, INFO_AVAILABLE, s);
	      sprintf(s,
	              GetPString(IDS_SECTORSTEXT),
	              fsa.cbSector,
	              fsa.cSectorUnit, &"s"[fsa.cSectorUnit == 1L]);
	      WinSetDlgItemText(hwnd, INFO_ALLOCUNITS, s);

	      percentfree = (fsa.cUnitAvail && fsa.cUnit) ?
		(fsa.cUnitAvail * 100) / fsa.cUnit : 0;
	      if (!percentfree && fsa.cUnitAvail)
		percentfree = 1;
	      percentused = 100 - percentfree;
	      WinSendDlgItemMsg(hwnd,
				INFO_USED,
				SLM_SETSLIDERINFO,
				MPFROM2SHORT(SMA_SLIDERARMPOSITION,
					     SMA_INCREMENTVALUE),
				MPFROMSHORT(percentused));
	      WinSendDlgItemMsg(hwnd,
				INFO_FREE,
				SLM_SETSLIDERINFO,
				MPFROM2SHORT(SMA_SLIDERARMPOSITION,
					     SMA_INCREMENTVALUE),
				MPFROMSHORT(percentfree));
	      sprintf(s, "%u%%", percentused);
	      WinSetDlgItemText(hwnd, INFO_USEDPERCENT, s);
	      sprintf(s, "%u%%", percentfree);
	      WinSetDlgItemText(hwnd, INFO_FREEPERCENT, s);
	      }
	      else
		 WinSetDlgItemText(hwnd, INFO_AVAILABLE, (CHAR *) GetPString(IDS_STATSMEANINGLESSTEXT));
	    }
	    else {
	      sprintf(FileSystem,
	              GetPString(IDS_CANTQUERYVOLTEXT),
	              toupper(*pszFileName));
	      Notify(FileSystem);
	      WinDismissDlg(hwnd, 0);
	    }
	  }
	  else {
	    sprintf(FileSystem,
	            GetPString(IDS_CANTQUERYALLOCTEXT),
	            toupper(*pszFileName));
	    Notify(FileSystem);
	    WinDismissDlg(hwnd, 0);
	  }
	}
	else {
	  FlagMsg(*pszFileName, s);
	  sprintf(FileSystem,
	          GetPString(IDS_DRIVEINACCESSIBLETEXT),
	          toupper(*pszFileName), s);
	  Notify(FileSystem);
	  WinDismissDlg(hwnd, 0);
	}
      }
      else {
	WinDismissDlg(hwnd, 0);
      }
    }
    else
      WinDismissDlg(hwnd, 0);
    break;

  case WM_CONTROL:
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_DRVINFO, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_OK:
      pszFileName = INSTDATA(hwnd);
      if (!(driveflags[toupper(*pszFileName) - 'A'] & DRIVE_NOTWRITEABLE)) {

	CHAR s[CCHMAXPATHCOMP + 3];

	*s = 0;
	WinQueryDlgItemText(hwnd, INFO_LABEL, CCHMAXPATHCOMP, s);
	bstrip(s);
	if (*s) {
	  struct
	  {
	    ULONG serial;
	    CHAR volumelength;
	    CHAR volumelabel[CCHMAXPATH];
	  }
	  volser;

	  memset(&volser, 0, sizeof(volser));
	  DosError(FERR_DISABLEHARDERR);
	  if (!DosQueryFSInfo(toupper(*pszFileName) - '@',
			      FSIL_VOLSER,
			      &volser,
			      (ULONG) sizeof(volser)) &&
	      stricmp(s, volser.volumelabel)) {
	    memmove(s + 1, s, strlen(s) + 1);
	    *s = strlen(s + 1);
	    DosError(FERR_DISABLEHARDERR);
	    rc = DosSetFSInfo(toupper(*pszFileName) - '@',
			      2L, (PVOID) s, (ULONG) sizeof(s));
	    if (rc) {
	      Dos_Error(MB_CANCEL, rc, hwnd, __FILE__, __LINE__,
			"DosSetFSInfo failed");
	    }
	  }
	}
      }
      WinDismissDlg(hwnd, 1);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

typedef struct {
  USHORT size;
  CHAR szFileName[CCHMAXPATH];
  BOOL madechanges;
} FILESTUF;

typedef struct {
  USHORT size;
  PFNWP oldproc;
  FILESTUF *pfs;
  HWND lasthwndMenu;
} ICONSTUF;

/*
 * subclass routine to allow changing a program's icon
 */

MRESULT EXPENTRY IconProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ICONSTUF *pis = (ICONSTUF *)WinQueryWindowPtr(hwnd, QWL_USER);
  MRESULT mr;
  CHAR *p;

  static BOOL emphasized = FALSE;

  if (!pis) {
    Runtime_Error(pszSrcFile, __LINE__, NULL);
    if (msg != WM_DESTROY)
      return WinDefWindowProc(hwnd, msg, mp1, mp2);
  }

  switch (msg) {
  case DM_DRAGOVER:
    if (!emphasized) {
      emphasized = TRUE;
      DrawTargetEmphasis(hwnd, emphasized);
    }
    if (AcceptOneDrop(hwnd, mp1, mp2))
      return MRFROM2SHORT(DOR_DROP, DO_MOVE);
    return MRFROM2SHORT(DOR_NEVERDROP, 0);

  case DM_DRAGLEAVE:
    emphasized = FALSE;
    DrawTargetEmphasis(hwnd, emphasized);
    break;

  case DM_DROPHELP:
    DropHelp(mp1, mp2, hwnd, GetPString(IDS_DROPCHGICONHELPTEXT));
    return 0;

  case DM_DROP:
    {
      HPOINTER hptr;
      CHAR szFrom[CCHMAXPATH + 2];
      ICONINFO ici;

      emphasized = FALSE;
      DrawTargetEmphasis(hwnd, emphasized);
      if (GetOneDrop(hwnd, mp1, mp2, szFrom, sizeof(szFrom))) {
	memset(&ici, 0, sizeof(ICONINFO));
	ici.cb = sizeof(ICONINFO);
	ici.fFormat = ICON_FILE;
	ici.pszFileName = szFrom;
	if (!WinSetFileIcon((PSZ) pis->pfs->szFileName, (PICONINFO) & ici)) {
	  ici.fFormat = ICON_CLEAR;
	  WinSetFileIcon((PSZ) pis->pfs->szFileName, (PICONINFO) & ici);
	}
	hptr = WinLoadFileIcon(pis->pfs->szFileName, FALSE);
	if (!hptr)
	  hptr = (!IsFile(pis->pfs->szFileName)) ? hptrDir : hptrFile;
	if (pis && pis->oldproc) {
	  WinShowWindow(hwnd, FALSE);
	  pis->oldproc(hwnd, SM_SETHANDLE, MPFROMLONG(hptr), MPVOID);
	  WinShowWindow(hwnd, TRUE);
	  WinInvalidateRect(WinQueryWindow(hwnd, QW_PARENT), NULL, TRUE);
	}
      }
    }
    return 0;

  case WM_PAINT:
    mr = pis->oldproc(hwnd, msg, mp1, mp2);
    PaintRecessedWindow(hwnd, (HPS) 0, FALSE, FALSE);
    return mr;
    break;

  case WM_MENUEND:
    if (pis->lasthwndMenu == (HWND)mp2)
      WinDestroyWindow(pis->lasthwndMenu);
    pis->lasthwndMenu = (HWND) 0;
    break;

  case WM_CONTEXTMENU:
    if (pis->lasthwndMenu)
      WinDestroyWindow(pis->lasthwndMenu);
    pis->lasthwndMenu = WinLoadMenu(hwnd, FM3ModHandle, FLE_FRAME);
    if (pis->lasthwndMenu) {
      p = strrchr(pis->pfs->szFileName, '.');
      if (!p || (stricmp(p, PCSZ_DOTICO) && stricmp(p, PCSZ_DOTPTR)))
	WinSendMsg(pis->lasthwndMenu,
		   MM_DELETEITEM,
		   MPFROM2SHORT(IDM_SELECTALL, TRUE), MPVOID);
      PopupMenu(hwnd, hwnd, pis->lasthwndMenu);
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_SELECTALL:
    case IDM_DESELECTALL:
      WinDlgBox(HWND_DESKTOP,
		hwnd,
		SetIconDlgProc,
		FM3ModHandle,
		SETICON_FRAME,
		(PVOID) ((SHORT1FROMMP(mp1) == IDM_SELECTALL) ?
			 pis->pfs->szFileName : NULL));
      break;
    }
    return 0;

  case WM_DESTROY:
    emphasized = FALSE;
    if (!pis)
      return WinDefWindowProc(hwnd, msg, mp1, mp2);
    else {
      PFNWP oldproc = pis->oldproc;
      if (pis->lasthwndMenu)
	WinDestroyWindow(pis->lasthwndMenu);
      free(pis);
      return oldproc(hwnd, msg, mp1, mp2);
    }
    break;
  }

  return pis->oldproc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY FileInfoProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  FILESTUF *pfs;
  ICONSTUF *pis;
  CHAR **ppsz;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2) {
      WinDismissDlg(hwnd, 1);
      break;
    }
    pfs = xmallocz(sizeof(FILESTUF), pszSrcFile, __LINE__);
    if (!pfs) {
      WinDismissDlg(hwnd, 1);
      break;
    }
    pfs->size = sizeof(FILESTUF);
    WinSetWindowPtr(hwnd, QWL_USER, pfs);
    {
      USHORT ids[] = { FLE_SIZES, FLE_SLACK, FLE_LASTWRITE, FLE_CREATE,
	FLE_LASTACCESS, 0
      };
      INT x;
      CHAR s[CCHMAXPATH];

      ppsz = (CHAR **)mp2;
      for (x = 0; ppsz[x]; x++) {
	if (DosQueryPathInfo(ppsz[x], FIL_QUERYFULLNAME, s, sizeof(s)))
	  strcpy(s, ppsz[x]);
	WinSendDlgItemMsg(hwnd,
			  FLE_NAME,
			  LM_INSERTITEM,
			  MPFROM2SHORT(LIT_SORTASCENDING, 0), MPFROMP(s));
      }
      if (!x) {
	WinDismissDlg(hwnd, 1);
	break;
      }
      WinSendDlgItemMsg(hwnd,
			FLE_NAME,
			LM_SELECTITEM, MPFROM2SHORT(0, 0), MPFROMSHORT(TRUE));
      for (x = 0; ids[x]; x++)
	SetPresParams(WinWindowFromID(hwnd, ids[x]),
		      &RGBGREY, &RGBBLACK, &RGBBLACK, NULL);
    }
    pis = xmallocz(sizeof(ICONSTUF), pszSrcFile, __LINE__);
    if (!pis) {
      WinDismissDlg(hwnd, 1);
      break;
    }
    WinSetWindowPtr(WinWindowFromID(hwnd, FLE_ICON), QWL_USER, (PVOID) pis);
    pis->size = sizeof(ICONSTUF);
    pis->pfs = pfs;
    pis->oldproc = WinSubclassWindow(WinWindowFromID(hwnd, FLE_ICON),
				    IconProc);
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case FLE_READONLY:
    case FLE_ARCHIVED:
    case FLE_SYSTEM:
    case FLE_HIDDEN:
      switch (SHORT2FROMMP(mp1)) {
      case BN_CLICKED:
	pfs = WinQueryWindowPtr(hwnd, QWL_USER);
	if (pfs && *pfs->szFileName) {

	  LISTINFO li;
	  UINT numfiles = 0, numalloc = 0;

	  memset(&li, 0, sizeof(LISTINFO));
	  if (!AddToList(pfs->szFileName, &li.list, &numfiles, &numalloc)) {
	    if (WinDlgBox(HWND_DESKTOP,
			  hwnd,
			  AttrListDlgProc,
			  FM3ModHandle,
			  ATR_FRAME, MPFROMP(&li)) && li.list && li.list[0]) {
	      pfs->madechanges = TRUE;
	      WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	    }
	    FreeList(li.list);
	  }
	}
	break;
      } // switch
      break;
    case FLE_NAME:
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
      case LN_SELECT:
	pfs = WinQueryWindowPtr(hwnd, QWL_USER);
	if (!pfs) {
	  Runtime_Error(pszSrcFile, __LINE__, NULL);
	  WinDismissDlg(hwnd, 1);
	}
	else {

	  SHORT sSelect;

	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      FLE_NAME,
					      LM_QUERYSELECTION,
					      MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (sSelect >= 0) {
	    *pfs->szFileName = 0;
	    WinSendDlgItemMsg(hwnd,
			      FLE_NAME,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect, CCHMAXPATH),
			      MPFROMP(pfs->szFileName));
	    if (*pfs->szFileName) {
	      if (SHORT2FROMMP(mp1) == LN_SELECT)
		WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	      else
		DefaultView(hwnd,
			    (HWND) 0, (HWND) 0, NULL, 32, pfs->szFileName);
	    }
	  }
	}
	break;
      }
      break;
    }
    return 0;

  case UM_SETDIR:
    WinCheckButton(hwnd, FLE_READONLY, FALSE);
    WinCheckButton(hwnd, FLE_ARCHIVED, FALSE);
    WinCheckButton(hwnd, FLE_SYSTEM, FALSE);
    WinCheckButton(hwnd, FLE_HIDDEN, FALSE);
    WinCheckButton(hwnd, FLE_DIRECTORY, FALSE);
    WinCheckButton(hwnd, FLE_READABLE, FALSE);
    WinCheckButton(hwnd, FLE_WRITEABLE, FALSE);
    WinCheckButton(hwnd, FLE_OPEN, FALSE);
    WinCheckButton(hwnd, FLE_BINARY, FALSE);
    WinCheckButton(hwnd, FLE_ISARCHIVE, FALSE);
    WinSetDlgItemText(hwnd, FLE_ARCNAME, NullStr);
    WinCheckButton(hwnd, FLE_OS2FS, FALSE);
    WinCheckButton(hwnd, FLE_OS2WIN, FALSE);
    WinCheckButton(hwnd, FLE_OS2PM, FALSE);
    WinCheckButton(hwnd, FLE_DOS, FALSE);
    WinCheckButton(hwnd, FLE_32BIT, FALSE);
    WinCheckButton(hwnd, FLE_WINREAL, FALSE);
    WinCheckButton(hwnd, FLE_WINPROT, FALSE);
    WinCheckButton(hwnd, FLE_WINENH, FALSE);
    WinCheckButton(hwnd, FLE_DLL, FALSE);
    WinCheckButton(hwnd, FLE_PHYSDRV, FALSE);
    WinCheckButton(hwnd, FLE_VIRTDRV, FALSE);
    WinCheckButton(hwnd, FLE_PROTDLL, FALSE);
    pfs = WinQueryWindowPtr(hwnd, QWL_USER);
    if (pfs && *pfs->szFileName) {
      CHAR s[97];
      CHAR szCmmaFmtFileSize[81], szCmmaFmtEASize[81];
      CHAR szCmmaFmtFileEASize[81], szCmmaFmtFileEASizeK[81], szDate[DATE_BUF_BYTES];
      FILEFINDBUF4L fs;
      HDIR hdir = HDIR_CREATE;
      ULONG apptype = 1;
      FILE *fp;
      HPOINTER hptr;
      ARC_TYPE *info;

      DosError(FERR_DISABLEHARDERR);
      if (xDosFindFirst(pfs->szFileName,
		        &hdir,
		        FILE_NORMAL | FILE_ARCHIVED |
		        FILE_DIRECTORY | FILE_READONLY | FILE_HIDDEN |
		        FILE_SYSTEM,
		        &fs, sizeof(fs), &apptype, FIL_QUERYEASIZEL)) {
	// Not found
	SHORT sSelect, numitems;

        if (!fAlertBeepOff)
	  DosBeep(250, 100);		// Wake up user
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    FLE_NAME,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(LIT_FIRST), MPVOID);
	if (sSelect >= 0) {
	  WinSendDlgItemMsg(hwnd,
			    FLE_NAME,
			    LM_DELETEITEM, MPFROMSHORT(sSelect), MPVOID);
	  numitems = (SHORT) WinSendDlgItemMsg(hwnd,
					       FLE_NAME,
					       LM_QUERYITEMCOUNT,
					       MPVOID, MPVOID);
	  if (numitems)
	    PostMsg(WinWindowFromID(hwnd, FLE_NAME),
		    LM_SELECTITEM,
		    MPFROMSHORT(((sSelect) ? sSelect - 1 : 0)),
		    MPFROMSHORT(TRUE));
	}
      }
      else {
        DosFindClose(hdir);
        FDateFormat(szDate, fs.fdateLastWrite);
	sprintf(s, "%s  %02u%s%02u%s%02u",
		szDate,
		fs.ftimeLastWrite.hours, TimeSeparator,
		fs.ftimeLastWrite.minutes, TimeSeparator, fs.ftimeLastWrite.twosecs * 2);
	WinSetDlgItemText(hwnd, FLE_LASTWRITE, s);
	if (fs.fdateCreation.year &&
            fs.fdateCreation.month && fs.fdateCreation.day) {
          FDateFormat(szDate, fs.fdateCreation);
	  sprintf(s, "%s  %02u%s%02u%s%02u",
		  szDate,
		  fs.ftimeCreation.hours, TimeSeparator,
		  fs.ftimeCreation.minutes, TimeSeparator, fs.ftimeCreation.twosecs * 2);
	  WinSetDlgItemText(hwnd, FLE_CREATE, s);
	}
	if (fs.fdateLastAccess.year &&
            fs.fdateLastAccess.month && fs.fdateLastAccess.day) {
          FDateFormat(szDate, fs.fdateLastAccess);
	  sprintf(s, "%s  %02u%s%02u%s%02u",
		  szDate,
		  fs.ftimeLastAccess.hours, TimeSeparator,
		  fs.ftimeLastAccess.minutes, TimeSeparator, fs.ftimeLastAccess.twosecs * 2);
	  WinSetDlgItemText(hwnd, FLE_LASTACCESS, s);
	}
        CommaFmtULL(szCmmaFmtFileSize,
                    sizeof(szCmmaFmtFileSize), fs.cbFile, ' ');
        CommaFmtULL(szCmmaFmtEASize,
                    sizeof(szCmmaFmtEASize), CBLIST_TO_EASIZE(fs.cbList), ' ');
        CommaFmtULL(szCmmaFmtFileEASize,
                    sizeof(szCmmaFmtFileEASize),
                    fs.cbFile + CBLIST_TO_EASIZE(fs.cbList),
                    ' ');
        CommaFmtULL(szCmmaFmtFileEASizeK,
                    sizeof(szCmmaFmtFileEASizeK),
                    fs.cbFile + CBLIST_TO_EASIZE(fs.cbList),
                    'K');
	sprintf(s,
		GetPString(IDS_SIZEINCLEASTEXT),
		szCmmaFmtFileSize,
		szCmmaFmtEASize,
		szCmmaFmtFileEASize,
		szCmmaFmtFileEASizeK);
	WinSetDlgItemText(hwnd, FLE_SIZES, s);
        CommaFmtULL(szCmmaFmtFileSize,
                    sizeof(szCmmaFmtFileSize), fs.cbFileAlloc - fs.cbFile, ' ');
	sprintf(s, "%s", szCmmaFmtFileSize);
	WinSetDlgItemText(hwnd, FLE_SLACK, s);
	WinCheckButton(hwnd,
		       FLE_READONLY, ((fs.attrFile & FILE_READONLY) != 0));
	WinCheckButton(hwnd,
		       FLE_ARCHIVED, ((fs.attrFile & FILE_ARCHIVED) != 0));
	WinCheckButton(hwnd,
		       FLE_DIRECTORY, ((fs.attrFile & FILE_DIRECTORY) != 0));
	WinCheckButton(hwnd, FLE_HIDDEN, ((fs.attrFile & FILE_HIDDEN) != 0));
	WinCheckButton(hwnd, FLE_SYSTEM, ((fs.attrFile & FILE_SYSTEM) != 0));
	DosError(FERR_DISABLEHARDERR);
	if (!DosQueryAppType(pfs->szFileName, &apptype)) {
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_OS2FS), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_OS2WIN), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_OS2PM), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_BOUND), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_32BIT), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_DOS), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_WINPROT), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_WINREAL), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_WINENH), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_DLL), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_PHYSDRV), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_VIRTDRV), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_PROTDLL), TRUE);
	  WinCheckButton(hwnd, FLE_OS2FS,
			 ((apptype & FAPPTYP_NOTWINDOWCOMPAT) &&
			  !(apptype & FAPPTYP_WINDOWCOMPAT)));
	  WinCheckButton(hwnd, FLE_OS2WIN,
			 ((apptype & FAPPTYP_WINDOWCOMPAT) &&
			  !(apptype & FAPPTYP_NOTWINDOWCOMPAT)));
	  WinCheckButton(hwnd, FLE_OS2PM,
			 ((apptype & FAPPTYP_WINDOWAPI) ==
			  FAPPTYP_WINDOWAPI));
	  WinCheckButton(hwnd, FLE_BOUND, ((apptype & FAPPTYP_BOUND) != 0));
	  WinCheckButton(hwnd, FLE_DLL, ((apptype & FAPPTYP_DLL) != 0));
	  WinCheckButton(hwnd, FLE_DOS, ((apptype & FAPPTYP_DOS) != 0));
	  WinCheckButton(hwnd, FLE_PHYSDRV,
			 ((apptype & FAPPTYP_PHYSDRV) != 0));
	  WinCheckButton(hwnd, FLE_VIRTDRV,
			 ((apptype & FAPPTYP_VIRTDRV) != 0));
	  WinCheckButton(hwnd, FLE_PROTDLL,
			 ((apptype & FAPPTYP_PROTDLL) != 0));
	  WinCheckButton(hwnd, FLE_WINREAL,
			 ((apptype & FAPPTYP_WINDOWSREAL) != 0));
	  WinCheckButton(hwnd, FLE_WINPROT,
			 ((apptype & FAPPTYP_WINDOWSPROT) != 0));
	  WinCheckButton(hwnd, FLE_32BIT, ((apptype & FAPPTYP_32BIT) != 0));
	  WinCheckButton(hwnd, FLE_WINENH, ((apptype & 0x1000) != 0));
	}
	else {
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_OS2FS), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_OS2WIN), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_OS2PM), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_BOUND), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_32BIT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_DOS), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_WINPROT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_WINREAL), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_WINENH), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_DLL), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_PHYSDRV), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_VIRTDRV), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_PROTDLL), FALSE);
	}
	hptr = WinLoadFileIcon(pfs->szFileName, FALSE);
	WinShowWindow(WinWindowFromID(hwnd, FLE_ICON), FALSE);
	if (hptr) {
	  WinSendDlgItemMsg(hwnd,
			    FLE_ICON, SM_SETHANDLE, MPFROMLONG(hptr), MPVOID);
	  WinShowWindow(WinWindowFromID(hwnd, FLE_ICON), TRUE);
	}
	WinShowWindow(WinWindowFromID(hwnd, FLE_EAS), fs.cbList > 4);
	if (!(fs.attrFile & FILE_DIRECTORY)) {
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_READABLE), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_WRITEABLE), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_OPEN), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_ISARCHIVE), TRUE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_BINARY), TRUE);
	  fp = _fsopen(pfs->szFileName, "rb", SH_DENYNO);
	  if (fp) {
	    char buff[4096];		// 06 Oct 07 SHL protect against NTFS defect
	    ULONG len;
	    APIRET rc;

	    len = 512;
	    rc = DosRead(fileno(fp), buff, len, &len);
	    fclose(fp);
	    WinCheckButton(hwnd,
			   FLE_BINARY,
			   ((len && rc) ? IsBinary(buff, len) : 2));
	    WinCheckButton(hwnd, FLE_READABLE, TRUE);
	    info = find_type(pfs->szFileName, NULL);
	    if (info) {
	      WinCheckButton(hwnd, FLE_ISARCHIVE, 1);
	      if (info->id)
		WinSetDlgItemText(hwnd, FLE_ARCNAME, info->id);
	    }
	  }
	  else {
	    WinCheckButton(hwnd, FLE_ISARCHIVE, 2);
	    WinCheckButton(hwnd, FLE_BINARY, 2);
	  }
	  fp = _fsopen(pfs->szFileName, "ab", SH_DENYNO);
	  if (fp) {
	    WinCheckButton(hwnd, FLE_WRITEABLE, TRUE);
	    fclose(fp);
	  }
	  fp = _fsopen(pfs->szFileName, "rb", SH_DENYRW);
	  if (!fp)
	    WinCheckButton(hwnd, FLE_OPEN, TRUE);
	  else
	    fclose(fp);
	}
	else {
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_READABLE), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_WRITEABLE), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_OPEN), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_ISARCHIVE), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, FLE_BINARY), FALSE);
	}
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      pfs = WinQueryWindowPtr(hwnd, QWL_USER);
      WinDismissDlg(hwnd, (pfs && pfs->madechanges) ? 2 : 1);
      break;
    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_INFO, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    case FLE_SETTINGS:
      pfs = WinQueryWindowPtr(hwnd, QWL_USER);
      if (pfs && *pfs->szFileName)
	OpenObject(pfs->szFileName, Settings, hwnd);
      break;
    case FLE_EAS:
      pfs = WinQueryWindowPtr(hwnd, QWL_USER);
      if (pfs && *pfs->szFileName) {

	CHAR *list[2];

	list[0] = pfs->szFileName;
	list[1] = NULL;
	WinDlgBox(HWND_DESKTOP,
		  hwnd, DisplayEAsProc, FM3ModHandle, EA_FRAME, (PVOID) list);
      }
      break;
    case DID_CANCEL:
      pfs = WinQueryWindowPtr(hwnd, QWL_USER);
      WinDismissDlg(hwnd, (pfs && pfs->madechanges) ? 2 : 0);
      break;
    }
    return 0;

  case WM_DESTROY:
    pfs = WinQueryWindowPtr(hwnd, QWL_USER);
    xfree(pfs, pszSrcFile, __LINE__);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY SetDrvProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    if (!mp2 || !isalpha(*(CHAR *)mp2))
      WinDismissDlg(hwnd, 0);
    else {

      CHAR s[80];

      WinSetWindowULong(hwnd, QWL_USER, (toupper(*(CHAR *)mp2) - 'A'));
      sprintf(s, GetPString(IDS_DRIVEFLAGSTITLETEXT), toupper(*(CHAR *)mp2));
      WinSetWindowText(hwnd, s);
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    }
    break;

  case UM_UNDO:
    {
      ULONG drive = WinQueryWindowULong(hwnd, QWL_USER);

      WinCheckButton(hwnd, DVS_REMOVABLE,
		     ((driveflags[drive] & DRIVE_REMOVABLE) != 0));
      WinCheckButton(hwnd, DVS_NOTWRITEABLE,
		     ((driveflags[drive] & DRIVE_NOTWRITEABLE) != 0));
      WinCheckButton(hwnd, DVS_IGNORE,
		     ((driveflags[drive] & DRIVE_IGNORE) != 0));
      WinCheckButton(hwnd, DVS_CDROM,
		     ((driveflags[drive] & DRIVE_CDROM) != 0));
      WinCheckButton(hwnd, DVS_NOLONGNAMES,
		     ((driveflags[drive] & DRIVE_NOLONGNAMES) != 0));
      WinCheckButton(hwnd, DVS_REMOTE,
		     ((driveflags[drive] & DRIVE_REMOTE) != 0));
      WinCheckButton(hwnd,DVS_VIRTUAL,
		     ((driveflags[drive] & DRIVE_VIRTUAL) != 0));
      WinCheckButton(hwnd,DVS_RAMDISK,
		     ((driveflags[drive] & DRIVE_RAMDISK) != 0));
      WinCheckButton(hwnd, DVS_BOOT,
		     ((driveflags[drive] & DRIVE_BOOT) != 0));
      WinCheckButton(hwnd, DVS_INVALID,
		     ((driveflags[drive] & DRIVE_INVALID) != 0));
      WinCheckButton(hwnd, DVS_NOPRESCAN,
		     ((driveflags[drive] & DRIVE_NOPRESCAN) != 0));
      WinCheckButton(hwnd, DVS_ZIPSTREAM,
		     ((driveflags[drive] & DRIVE_ZIPSTREAM) != 0));
      WinCheckButton(hwnd, DVS_NOLOADICONS,
		     ((driveflags[drive] & DRIVE_NOLOADICONS) != 0));
      WinCheckButton(hwnd, DVS_NOLOADSUBJS,
		     ((driveflags[drive] & DRIVE_NOLOADSUBJS) != 0));
      WinCheckButton(hwnd, DVS_NOLOADLONGS,
		     ((driveflags[drive] & DRIVE_NOLOADLONGS) != 0));
      WinCheckButton(hwnd, DVS_SLOW, ((driveflags[drive] & DRIVE_SLOW) != 0));
      WinCheckButton(hwnd, DVS_INCLUDEFILES,
		     ((driveflags[drive] & DRIVE_INCLUDEFILES) != 0));
      WinCheckButton(hwnd,DVS_NOSTATS,
                     ((driveflags[drive] & DRIVE_NOSTATS) != 0));
      WinCheckButton(hwnd,DVS_WRITEVERIFYOFF,
                     ((driveflags[drive] & DRIVE_WRITEVERIFYOFF) != 0));
      WinCheckButton(hwnd,DVS_RSCANNED,
                     ((driveflags[drive] & DRIVE_RSCANNED) != 0));
      WinCheckButton(hwnd,DVS_LOCALHD,
                     ((driveflags[drive] & DRIVE_LOCALHD) != 0));
      WinCheckButton(hwnd,DVS_NOEASUPPORT,
		     ((driveflags[drive] & DRIVE_NOEASUPPORT) != 0));
    }
    return 0;

  case WM_CONTROL:
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      {
	ULONG drive = WinQueryWindowULong(hwnd, QWL_USER);

	if (WinQueryButtonCheckstate(hwnd, DVS_NOPRESCAN))
	  driveflags[drive] |= DRIVE_NOPRESCAN;
	else
	  driveflags[drive] &= (~DRIVE_NOPRESCAN);
	if (WinQueryButtonCheckstate(hwnd, DVS_NOLOADICONS))
	  driveflags[drive] |= DRIVE_NOLOADICONS;
	else
	  driveflags[drive] &= (~DRIVE_NOLOADICONS);
	if (WinQueryButtonCheckstate(hwnd, DVS_NOLOADSUBJS))
	  driveflags[drive] |= DRIVE_NOLOADSUBJS;
	else
	  driveflags[drive] &= (~DRIVE_NOLOADSUBJS);
	if (WinQueryButtonCheckstate(hwnd, DVS_NOLOADLONGS))
	  driveflags[drive] |= DRIVE_NOLOADLONGS;
	else
	  driveflags[drive] &= (~DRIVE_NOLOADLONGS);
	if (WinQueryButtonCheckstate(hwnd, DVS_SLOW))
	  driveflags[drive] |= DRIVE_SLOW;
	else
	  driveflags[drive] &= (~DRIVE_SLOW);
	if (WinQueryButtonCheckstate(hwnd, DVS_INCLUDEFILES))
	  driveflags[drive] |= DRIVE_INCLUDEFILES;
	else
	  driveflags[drive] &= (~DRIVE_INCLUDEFILES);
	if (WinQueryButtonCheckstate(hwnd,DVS_NOSTATS))
	  driveflags[drive] |= DRIVE_NOSTATS;
	else
          driveflags[drive] &= (~DRIVE_NOSTATS);
        if (WinQueryButtonCheckstate(hwnd,DVS_WRITEVERIFYOFF))
          driveflags[drive] |= DRIVE_WRITEVERIFYOFF;
	else
	  driveflags[drive] &= (~DRIVE_WRITEVERIFYOFF);
	{
	  ULONG flags;
          CHAR FlagKey[80];

          sprintf(FlagKey, "%c.DriveFlags", (CHAR) (drive + 'A'));
	  flags = driveflags[drive];
	  flags &= (~(DRIVE_REMOVABLE | DRIVE_NOTWRITEABLE |
		      DRIVE_IGNORE | DRIVE_CDROM |
                      DRIVE_REMOTE | DRIVE_RSCANNED |
		      DRIVE_BOOT | DRIVE_INVALID | DRIVE_ZIPSTREAM |
		      DRIVE_VIRTUAL  | DRIVE_RAMDISK));
          PrfWriteProfileData(fmprof, appname, FlagKey, &flags, sizeof(ULONG));
	}
      }
      WinDismissDlg(hwnd, 1);
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_FLAGS, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(FMINFO,FileInfoProc,IconProc)
#pragma alloc_text(FMINFO2,SetDrvProc,DrvInfoProc)
