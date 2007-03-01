
/***********************************************************************

  $Id$

  Info window

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2006 Steven H. Levine

  16 Oct 02 SHL Handle large partitions
  12 Feb 03 SHL FileInfoProc: standardize EA math
  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  05 Jun 05 SHL Use QWL_USER
  14 Jul 06 SHL Use Runtime_Error

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(FMINFO,FileInfoProc,IconProc)
#pragma alloc_text(FMINFO2,SetDrvProc,DrvInfoProc)

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
	for (x = IDS_FLREMOVABLETEXT; x < IDS_FLRAMDISKTEXT + 1; x++) {
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

      pszFileName = (CHAR *) mp2;
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
	      pfsn = pfsq->szName + pfsq->cbName + 1;
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
	      sprintf(s,
		      "%s, %s, %s %s%s",
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
                 WinSetDlgItemText(hwnd, INFO_AVAILABLE, GetPString(IDS_STATSMEANINGLESSTEXT));
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

typedef struct
{
  USHORT size;
  USHORT dummy;
  PFNWP oldproc;
  HWND lasthwndMenu;
  CHAR szFileName[CCHMAXPATH];
  CHAR **list;
  BOOL madechanges;
}
ICONSTUF;

/*
 * subclass routine to allow changing a program's icon
 */

MRESULT EXPENTRY IconProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ICONSTUF *is;
  static BOOL emphasized = FALSE;

  is = (ICONSTUF *) WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg) {
  case DM_DRAGOVER:
    if (!emphasized) {
      emphasized = TRUE;
      DrawTargetEmphasis(hwnd, emphasized);
    }
    if (AcceptOneDrop(mp1, mp2))
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
      if (GetOneDrop(mp1, mp2, szFrom, sizeof(szFrom))) {
	memset(&ici, 0, sizeof(ICONINFO));
	ici.cb = sizeof(ICONINFO);
	ici.fFormat = ICON_FILE;
	ici.pszFileName = szFrom;
	if (!WinSetFileIcon((PSZ) is->szFileName, (PICONINFO) & ici)) {
	  ici.fFormat = ICON_CLEAR;
	  WinSetFileIcon((PSZ) is->szFileName, (PICONINFO) & ici);
	}
	hptr = WinLoadFileIcon(is->szFileName, FALSE);
	if (!hptr)
	  hptr = (!IsFile(is->szFileName)) ? hptrDir : hptrFile;
	if (is && is->oldproc) {
	  WinShowWindow(hwnd, FALSE);
	  is->oldproc(hwnd, SM_SETHANDLE, MPFROMLONG(hptr), MPVOID);
	  WinShowWindow(hwnd, TRUE);
	  WinInvalidateRect(WinQueryWindow(hwnd, QW_PARENT), NULL, TRUE);
	}
      }
    }
    return 0;

  case WM_PAINT:
    if (is) {

      MRESULT mr;

      mr = is->oldproc(hwnd, msg, mp1, mp2);
      PaintRecessedWindow(hwnd, (HPS) 0, FALSE, FALSE);
      return mr;
    }
    break;

  case WM_MENUEND:
    if (is) {
      if (is->lasthwndMenu == (HWND) mp2)
	WinDestroyWindow(is->lasthwndMenu);
      is->lasthwndMenu = (HWND) 0;
    }
    break;

  case WM_CONTEXTMENU:
    if (is) {

      CHAR *p;

      if (is->lasthwndMenu)
	WinDestroyWindow(is->lasthwndMenu);
      is->lasthwndMenu = WinLoadMenu(hwnd, FM3ModHandle, FLE_FRAME);
      if (is->lasthwndMenu) {
	p = strrchr(is->szFileName, '.');
	if (!p || (stricmp(p, ".ICO") && stricmp(p, ".PTR")))
	  WinSendMsg(is->lasthwndMenu,
		     MM_DELETEITEM,
		     MPFROM2SHORT(IDM_SELECTALL, TRUE), MPVOID);
	PopupMenu(hwnd, hwnd, is->lasthwndMenu);
      }
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
			 is->szFileName : NULL));
      break;
    }
    return 0;

  case WM_DESTROY:
    emphasized = FALSE;
    if (is && is->lasthwndMenu) {
      WinDestroyWindow(is->lasthwndMenu);
      is->lasthwndMenu = (HWND) 0;
    }
    break;
  }

  if (is && is->oldproc)
    return is->oldproc(hwnd, msg, mp1, mp2);
  else
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY FileInfoProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ICONSTUF *is;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2) {
      WinDismissDlg(hwnd, 1);
      break;
    }
    is = xmallocz(sizeof(ICONSTUF), pszSrcFile, __LINE__);
    if (!is) {
      WinDismissDlg(hwnd, 1);
      break;
    }
    is->list = (CHAR **) mp2;
    is->size = sizeof(ICONSTUF);
    WinSetWindowPtr(hwnd, QWL_USER, is);
    {
      USHORT ids[] = { FLE_SIZES, FLE_SLACK, FLE_LASTWRITE, FLE_CREATE,
	FLE_LASTACCESS, 0
      };
      INT x;
      CHAR s[CCHMAXPATH];

      for (x = 0; is->list[x]; x++) {
	if (DosQueryPathInfo(is->list[x], FIL_QUERYFULLNAME, s, sizeof(s)))
	  strcpy(s, is->list[x]);
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
    WinSetWindowPtr(WinWindowFromID(hwnd, FLE_ICON), QWL_USER, (PVOID) is);
    is->oldproc = WinSubclassWindow(WinWindowFromID(hwnd, FLE_ICON),
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
	is = WinQueryWindowPtr(hwnd, QWL_USER);
	if (is && *is->szFileName) {

	  LISTINFO li;
	  INT numfiles = 0, numalloc = 0;

	  memset(&li, 0, sizeof(LISTINFO));
	  if (!AddToList(is->szFileName, &li.list, &numfiles, &numalloc)) {
	    if (WinDlgBox(HWND_DESKTOP,
			  hwnd,
			  AttrListDlgProc,
			  FM3ModHandle,
			  ATR_FRAME, MPFROMP(&li)) && li.list && li.list[0]) {
	      is->madechanges = TRUE;
	      WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	    }
	    FreeList(li.list);
	  }
	}
	break;
      }
      break;
    case FLE_NAME:
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
      case LN_SELECT:
	is = WinQueryWindowPtr(hwnd, QWL_USER);
	if (!is) {
	  Runtime_Error(pszSrcFile, __LINE__, "no data");
	  WinDismissDlg(hwnd, 1);
	}
	else {

	  SHORT sSelect;

	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      FLE_NAME,
					      LM_QUERYSELECTION,
					      MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (sSelect >= 0) {
	    *is->szFileName = 0;
	    WinSendDlgItemMsg(hwnd,
			      FLE_NAME,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect, CCHMAXPATH),
			      MPFROMP(is->szFileName));
	    if (*is->szFileName) {
	      if (SHORT2FROMMP(mp1) == LN_SELECT)
		WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	      else
		DefaultView(hwnd,
			    (HWND) 0, (HWND) 0, NULL, 32, is->szFileName);
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
    is = WinQueryWindowPtr(hwnd, QWL_USER);
    if (is && *is->szFileName) {
      CHAR s[97];
      FILEFINDBUF4 fs;
      HDIR hdir = HDIR_CREATE;
      ULONG apptype = 1L;
      FILE *fp;
      HPOINTER hptr;
      ARC_TYPE *info;

      DosError(FERR_DISABLEHARDERR);
      if (DosFindFirst(is->szFileName,
		       &hdir,
		       FILE_NORMAL | FILE_ARCHIVED |
		       FILE_DIRECTORY | FILE_READONLY | FILE_HIDDEN |
		       FILE_SYSTEM,
		       &fs, sizeof(fs), &apptype, FIL_QUERYEASIZE)) {
	// Not found
	SHORT sSelect, numitems;

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
	sprintf(s,
		"%04u/%02u/%02u  %02u:%02u:%02u",
		1980 + fs.fdateLastWrite.year,
		fs.fdateLastWrite.month,
		fs.fdateLastWrite.day,
		fs.ftimeLastWrite.hours,
		fs.ftimeLastWrite.minutes, fs.ftimeLastWrite.twosecs * 2);
	WinSetDlgItemText(hwnd, FLE_LASTWRITE, s);
	if (fs.fdateCreation.year &&
	    fs.fdateCreation.month && fs.fdateCreation.day) {
	  sprintf(s,
		  "%04u/%02u/%02u  %02u:%02u:%02u",
		  1980 + fs.fdateCreation.year,
		  fs.fdateCreation.month,
		  fs.fdateCreation.day,
		  fs.ftimeCreation.hours,
		  fs.ftimeCreation.minutes, fs.ftimeCreation.twosecs * 2);
	  WinSetDlgItemText(hwnd, FLE_CREATE, s);
	}
	if (fs.fdateLastAccess.year &&
	    fs.fdateLastAccess.month && fs.fdateLastAccess.day) {
	  sprintf(s,
		  "%04u/%02u/%02u  %02u:%02u:%02u",
		  1980 + fs.fdateLastAccess.year,
		  fs.fdateLastAccess.month,
		  fs.fdateLastAccess.day,
		  fs.ftimeLastAccess.hours,
		  fs.ftimeLastAccess.minutes, fs.ftimeLastAccess.twosecs * 2);
	  WinSetDlgItemText(hwnd, FLE_LASTACCESS, s);
	}
	sprintf(s,
		GetPString(IDS_SIZEINCLEASTEXT),
		fs.cbFile,
		CBLIST_TO_EASIZE(fs.cbList),
		fs.cbFile + CBLIST_TO_EASIZE(fs.cbList),
		(fs.cbFile + CBLIST_TO_EASIZE(fs.cbList)) / 1024);
	WinSetDlgItemText(hwnd, FLE_SIZES, s);
	sprintf(s, "%lub", fs.cbFileAlloc - fs.cbFile);
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
	if (!DosQueryAppType(is->szFileName, &apptype)) {
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
	hptr = WinLoadFileIcon(is->szFileName, FALSE);
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
	  fp = _fsopen(is->szFileName, "rb", SH_DENYNO);
	  if (fp) {
	    char buff[512];
	    ULONG len;
	    APIRET rc;

	    len = 512;
	    rc = DosRead(fileno(fp), buff, len, &len);
	    fclose(fp);
	    WinCheckButton(hwnd,
			   FLE_BINARY,
			   ((len && rc) ? IsBinary(buff, len) : 2));
	    WinCheckButton(hwnd, FLE_READABLE, TRUE);
	    info = find_type(is->szFileName, NULL);
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
	  fp = _fsopen(is->szFileName, "ab", SH_DENYNO);
	  if (fp) {
	    WinCheckButton(hwnd, FLE_WRITEABLE, TRUE);
	    fclose(fp);
	  }
	  fp = _fsopen(is->szFileName, "rb", SH_DENYRW);
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
      is = WinQueryWindowPtr(hwnd, QWL_USER);
      WinDismissDlg(hwnd, (is && is->madechanges) ? 2 : 1);
      break;
    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_INFO, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    case FLE_SETTINGS:
      is = WinQueryWindowPtr(hwnd, QWL_USER);
      if (is && *is->szFileName)
	OpenObject(is->szFileName, Settings, hwnd);
      break;
    case FLE_EAS:
      is = WinQueryWindowPtr(hwnd, QWL_USER);
      if (is && *is->szFileName) {

	CHAR *list[2];

	list[0] = is->szFileName;
	list[1] = NULL;
	WinDlgBox(HWND_DESKTOP,
		  hwnd, DisplayEAsProc, FM3ModHandle, EA_FRAME, (PVOID) list);
      }
      break;
    case DID_CANCEL:
      is = WinQueryWindowPtr(hwnd, QWL_USER);
      WinDismissDlg(hwnd, (is && is->madechanges) ? 2 : 0);
      break;
    }
    return 0;

  case WM_DESTROY:
    is = WinQueryWindowPtr(hwnd, QWL_USER);
    if (is)
      free(is);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY SetDrvProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    if (!mp2 || !isalpha(*(CHAR *) mp2))
      WinDismissDlg(hwnd, 0);
    else {

      CHAR s[80];

      WinSetWindowULong(hwnd, QWL_USER, (toupper(*(CHAR *) mp2) - 'A'));
      sprintf(s, GetPString(IDS_DRIVEFLAGSTITLETEXT), toupper(*(CHAR *) mp2));
      WinSetWindowText(hwnd, s);
/*
        WinEnableWindow(WinWindowFromID(hwnd,DVS_REMOVABLE),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_NOTWRITEABLE),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_IGNORE),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_CDROM),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_NOLONGNAMES),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_REMOTE),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_VIRTUAL),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_RAMDISK),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_BOOT),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_INVALID),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_ZIPSTREAM),FALSE);
        WinEnableWindow(WinWindowFromID(hwnd,DVS_NOSTATS),FALSE);
*/
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
	{
	  ULONG flags;
	  CHAR s[80];

	  sprintf(s, "%c.DriveFlags", (CHAR) (drive + 'A'));
	  flags = driveflags[drive];
	  flags &= (~(DRIVE_REMOVABLE | DRIVE_NOTWRITEABLE |
		      DRIVE_IGNORE | DRIVE_CDROM |
		      DRIVE_NOLONGNAMES | DRIVE_REMOTE |
                      DRIVE_BOOT | DRIVE_INVALID | DRIVE_ZIPSTREAM |
                      DRIVE_VIRTUAL  | DRIVE_RAMDISK));
	  PrfWriteProfileData(fmprof, appname, s, &flags, sizeof(ULONG));
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
