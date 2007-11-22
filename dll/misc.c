
/***********************************************************************

  $Id$

  Misc GUI support functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2007 Steven H. Levine

  11 Jun 03 SHL Add JFS and FAT32 support
  01 Aug 04 SHL Rework lstrip/rstrip usage
  01 Aug 04 SHL LoadLibPath: avoid buffer overflow
  07 Jun 05 SHL Drop obsoletes
  24 Jul 05 SHL Beautify
  24 Jul 05 SHL Correct longname display option
  17 Jul 06 SHL Use Runtime_Error
  26 Jul 06 SHL Use chop_at_crnl
  27 Jul 06 SHL Comments, apply indent
  29 Jul 06 SHL Use xfgets_bstripcr
  16 Aug 06 SHL Comments
  31 Aug 06 SHL disable_menuitem: rework args to match name - sheesh
  10 Oct 06 GKY Add NDFS32 support
  18 Feb 07 GKY More drive type and drive icon support
  10 Jun 07 GKY Add IsFm2Window as part of work around PM drag limit
  05 Jul 07 GKY Fix menu removals for WORKPLACE_PROCESS=YES
  23 Jul 07 SHL Sync with CNRITEM updates (ticket#24)
  31 Jul 07 SHL Clean up and report errors (ticket#24)
  03 Aug 07 GKY Direct editting fixed (ticket#24)
  06 Aug 07 SHL Use BldQuotedFileName
  06 Aug 07 GKY Increase Subject EA to 1024
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  14 Aug 07 SHL Delete obsoletes
  14 Aug 07 SHL Move #pragma alloc_text to end for OpenWatcom compat
  01 Sep 07 GKY Use xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundry
  05 Nov 07 GKY Use commafmtULL to display file sizes for large file support
  22 Nov 07 GKY Use CopyPresParams to fix presparam inconsistencies in menus

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG
#include <os2.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <malloc.h>			// headmin

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

#ifndef BEGIN_LIBPATH
#define BEGIN_LIBPATH            1
#endif

#ifndef END_LIBPATH
#define END_LIBPATH              2
#endif

#ifndef ORD_DOS32QUERYEXTLIBPATH
#define ORD_DOS32QUERYEXTLIBPATH 874
#endif

BOOL IsFm2Window(HWND hwnd, BOOL chkTid)
{
    PIB *ppib;
    TIB *ptib;
    BOOL yes;
    APIRET rc = DosGetInfoBlocks(&ptib, &ppib);

    if (rc) {
      Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		"DosGetInfoBlocks");
      yes = FALSE;
    }
    else {
      PID pid;
      TID tid;

      // Check window owned by FM2 process
      // Check say same thread too, if requested
      // OK for window to be dead - just return FALSE
      yes = WinQueryWindowProcess(hwnd, &pid, &tid) &&
	    pid == ppib->pib_ulpid &&
	    (!chkTid || tid == ptib->tib_ptib2->tib2_ultid);
    }
    return yes;
}

VOID SetShiftState(VOID)
{
  shiftstate = 0;
  if (WinGetKeyState(HWND_DESKTOP, VK_CTRL) & 0x8000)
    shiftstate |= KC_CTRL;
  if (WinGetKeyState(HWND_DESKTOP, VK_SHIFT) & 0x8000)
    shiftstate |= KC_SHIFT;
  if (WinGetKeyState(HWND_DESKTOP, VK_ALT) & 0x8000)
    shiftstate |= KC_ALT;
}

void EmphasizeButton(HWND hwnd, BOOL on)
{
  HPS hps = DrgGetPS(hwnd);

  // fixme to complain?
  if (hps) {
    POINTL ptl;
    SWP swp;

    WinQueryWindowPos(hwnd, &swp);
    ptl.x = 1;
    ptl.y = 1;
    GpiMove(hps, &ptl);
    GpiSetColor(hps, on ? CLR_BLACK : CLR_PALEGRAY);
    ptl.x = swp.cx - 2;
    ptl.y = swp.cy - 2;
    GpiBox(hps, DRO_OUTLINE, &ptl, 0, 0);
    DrgReleasePS(hps);
    if (remove) //fixme always true
      WinInvalidateRect(hwnd, NULL, FALSE);
  }
}

void DrawTargetEmphasis(HWND hwnd, BOOL on)
{
  HPS hps = DrgGetPS(WinQueryWindow(hwnd, QW_PARENT));

  if (hps) {
    BoxWindow(hwnd, hps, on ? CLR_BLACK : CLR_PALEGRAY);
    DrgReleasePS(hps);
  }
}

void BoxWindow(HWND hwnd, HPS hps, LONG color)
{
  POINTL ptl;
  SWP swp;
  BOOL releaseme = FALSE;

  if (!hps) {
    hps = WinGetPS(WinQueryWindow(hwnd, QW_PARENT));
    releaseme = TRUE;
  }
  if (hps && WinQueryWindowPos(hwnd, &swp)) {
    ptl.x = swp.x - 2;
    ptl.y = swp.y - 2;
    GpiMove(hps, &ptl);
    GpiSetColor(hps, color);
    ptl.x = swp.x + swp.cx + 1;
    ptl.y = swp.y + swp.cy + 1;
    GpiBox(hps, DRO_OUTLINE, &ptl, 0, 0);
  }
  if (releaseme && hps)
    WinReleasePS(hps);
}

void PaintSTextWindow(HWND hwnd, HPS hps)
{
  /*
   * paint a text window such that the rightmost part of the text is
   * always visible even if the text length exceeds the length of the
   * window -- otherwise, paint the window so that it is left-justified
   * and vertically centered.
   */

  char *s = NULL;
  long len;
  POINTL aptl[TXTBOX_COUNT], ptl;
  RECTL rcl;
  char *p;
  BOOL releaseme = FALSE;

  if (!hps) {
    releaseme = TRUE;
    hps = WinGetPS(hwnd);
  }
  if (hps) {
    WinQueryWindowRect(hwnd, &rcl);
    WinFillRect(hps, &rcl, CLR_PALEGRAY);
    len = WinQueryWindowTextLength(hwnd);
    if (len)
      s = xmalloc(len + 1, pszSrcFile, __LINE__);
    if (s) {
      *s = 0;
      WinQueryWindowText(hwnd, CCHMAXPATH, s);
      if (*s) {
	rcl.xRight -= 3;
	p = s;
	GpiQueryTextBox(hps, 3, "...", TXTBOX_COUNT, aptl);
	len = aptl[TXTBOX_TOPRIGHT].x;
	do {
	  GpiQueryTextBox(hps, strlen(p), p, TXTBOX_COUNT, aptl);
	  if (aptl[TXTBOX_TOPRIGHT].x > (rcl.xRight - (p != s ? len : 0)))
	    p++;
	  else
	    break;
	}
	while (*p);
	if (*p) {
	  GpiSetMix(hps, FM_OVERPAINT);
	  GpiSetColor(hps, CLR_BLACK);
	  ptl.x = 3;
	  ptl.y = ((rcl.yTop / 2) -
		   ((aptl[TXTBOX_TOPRIGHT].y +
		     aptl[TXTBOX_BOTTOMLEFT].y) / 2));
	  GpiMove(hps, &ptl);
	  if (p != s)
	    GpiCharString(hps, 3, "...");
	  GpiCharString(hps, strlen(p), p);
	}
      }
      free(s);
    }
    if (releaseme)
      WinReleasePS(hps);
  }
}

VOID PaintRecessedWindow(HWND hwnd, HPS hps, BOOL outtie, BOOL dbl)
{
  /*
   * paint a recessed box around the window
   * two pixels width required around window for painting...
   */
  BOOL releaseme = FALSE;

  if (!hps) {
    hps = WinGetPS(WinQueryWindow(hwnd, QW_PARENT));
    releaseme = TRUE;
  }
  if (hps) {

    POINTL ptl;
    SWP swp;

    WinQueryWindowPos(hwnd, &swp);
    ptl.x = swp.x - 1;
    ptl.y = swp.y - 1;
    GpiMove(hps, &ptl);
    if (!outtie)
      GpiSetColor(hps, CLR_WHITE);
    else
      GpiSetColor(hps, CLR_DARKGRAY);
    ptl.x = swp.x + swp.cx;
    GpiLine(hps, &ptl);
    ptl.y = swp.y + swp.cy;
    GpiLine(hps, &ptl);
    if (dbl) {
      ptl.x = swp.x - 2;
      ptl.y = swp.y - 2;
      GpiMove(hps, &ptl);
      ptl.x = swp.x + swp.cx + 1;
      GpiLine(hps, &ptl);
      ptl.y = swp.y + swp.cy + 1;
      GpiLine(hps, &ptl);
    }
    if (!outtie)
      GpiSetColor(hps, CLR_DARKGRAY);
    else
      GpiSetColor(hps, CLR_WHITE);
    if (dbl) {
      ptl.x = swp.x - 2;
      GpiLine(hps, &ptl);
      ptl.y = swp.y - 2;
      GpiLine(hps, &ptl);
      ptl.x = swp.x + swp.cx;
      ptl.y = swp.y + swp.cy;
      GpiMove(hps, &ptl);
    }
    ptl.x = swp.x - 1;
    GpiLine(hps, &ptl);
    ptl.y = swp.y - 1;
    GpiLine(hps, &ptl);
    GpiSetColor(hps, CLR_PALEGRAY);
    ptl.x = swp.x - (2 + (dbl != FALSE));
    ptl.y = swp.y - (2 + (dbl != FALSE));
    GpiMove(hps, &ptl);
    ptl.x = swp.x + swp.cx + (1 + (dbl != FALSE));
    GpiLine(hps, &ptl);
    ptl.y = swp.y + swp.cy + (1 + (dbl != FALSE));
    GpiLine(hps, &ptl);
    ptl.x = swp.x - (2 + (dbl != FALSE));
    GpiLine(hps, &ptl);
    ptl.y = swp.y - (2 + (dbl != FALSE));
    GpiLine(hps, &ptl);
    if (releaseme)
      WinReleasePS(hps);
  }
}

BOOL AdjustCnrColVis(HWND hwndCnr, CHAR * title, BOOL visible, BOOL toggle)
{
  PFIELDINFO pfi = (PFIELDINFO) WinSendMsg(hwndCnr,
					   CM_QUERYDETAILFIELDINFO,
					   MPVOID, MPFROMSHORT(CMA_FIRST));

  while (pfi) {
    if (!strcmp(pfi->pTitleData, title)) {
      if (toggle) {
	if (pfi->flData & CFA_INVISIBLE)
	  pfi->flData &= (~CFA_INVISIBLE);
	else
	  pfi->flData |= CFA_INVISIBLE;
	return !(pfi->flData & CFA_INVISIBLE);
      }
      else {
	if (visible)
	  pfi->flData &= (~CFA_INVISIBLE);
	else
	  pfi->flData |= CFA_INVISIBLE;
      }
      return TRUE;
    }
    pfi = pfi->pNextFieldInfo;
  }
  return FALSE;
}

BOOL AdjustCnrColRO(HWND hwndCnr, CHAR * title, BOOL readonly, BOOL toggle)
{
  PFIELDINFO pfi = (PFIELDINFO) WinSendMsg(hwndCnr,
					   CM_QUERYDETAILFIELDINFO,
					   MPVOID, MPFROMSHORT(CMA_FIRST));

  while (pfi) {
    if (!strcmp(pfi->pTitleData, title)) {
      if (toggle) {
	if (pfi->flData & CFA_FIREADONLY)
	  pfi->flData &= (~CFA_FIREADONLY);
	else
	  pfi->flData |= CFA_FIREADONLY;
	return (pfi->flData & CFA_FIREADONLY);
      }
      else {
	if (!readonly)
	  pfi->flData &= (~CFA_FIREADONLY);
	else
	  pfi->flData |= CFA_FIREADONLY;
      }
      return TRUE;
    }
    pfi = pfi->pNextFieldInfo;
  }
  return FALSE;
}

VOID AdjustCnrColsForFSType(HWND hwndCnr, CHAR * directory, DIRCNRDATA * dcd)
{
  CHAR FileSystem[CCHMAXPATH];
  INT x;
  BOOL hasCreateDT;
  BOOL hasAccessDT;
  BOOL hasLongNames;
  BOOL *pBool;

  if (!directory || !*directory)
    return;
  x = CheckDrive(toupper(*directory), FileSystem, NULL);
  if (x != -1) {
    if (!stricmp(FileSystem, HPFS) ||
	!stricmp(FileSystem, JFS) ||
	!stricmp(FileSystem, FAT32) ||
	!stricmp(FileSystem, RAMFS) ||
	!stricmp(FileSystem, NDFS32) ||
	!stricmp(FileSystem, NTFS) ||
	!stricmp(FileSystem, HPFS386)) {
      hasCreateDT = TRUE;
      hasAccessDT = TRUE;
      hasLongNames = TRUE;
    }
    else if (!strcmp(FileSystem, CDFS) || !strcmp(FileSystem, ISOFS)) {
      hasCreateDT = TRUE;
      hasAccessDT = FALSE;
      hasLongNames = FALSE;
    }
    else {
      // Assume FAT
      hasCreateDT = FALSE;
      hasAccessDT = FALSE;
      hasLongNames = FALSE;
    }
  }
  else {
    // Assume FAT
    hasCreateDT = FALSE;
    hasAccessDT = FALSE;
    hasLongNames = FALSE;
  }
  pBool = dcd ? &dcd->detailsladate : &detailsladate;
  AdjustCnrColVis(hwndCnr,
		  GetPString(IDS_LADATE),
		  *pBool ? hasAccessDT : FALSE,
		  FALSE);
  pBool = dcd ? &dcd->detailslatime : &detailslatime;
  AdjustCnrColVis(hwndCnr,
		  GetPString(IDS_LATIME),
		  *pBool ? hasAccessDT : FALSE,
		  FALSE);
  pBool = dcd ? &dcd->detailscrdate : &detailscrdate;
  AdjustCnrColVis(hwndCnr,
		  GetPString(IDS_CRDATE),
		  *pBool ? hasCreateDT : FALSE,
		  FALSE);
  pBool = dcd ? &dcd->detailscrtime : &detailscrtime;
  AdjustCnrColVis(hwndCnr,
		  GetPString(IDS_CRTIME),
		  *pBool ? hasCreateDT : FALSE,
		  FALSE);
  pBool = dcd ? &dcd->detailslongname : &detailslongname;
  AdjustCnrColVis(hwndCnr,
		  GetPString(IDS_LNAME),
		  *pBool ? hasLongNames : FALSE,
		  FALSE);
  WinSendMsg(hwndCnr, CM_INVALIDATEDETAILFIELDINFO, MPVOID, MPVOID);
}

VOID AdjustCnrColsForPref(HWND hwndCnr, CHAR * directory, DIRCNRDATA * dcd,
			  BOOL compare)
{
  BOOL *bool;

  bool = dcd ? &dcd->detailssubject : &detailssubject;
  AdjustCnrColVis(hwndCnr,
		  compare ? GetPString(IDS_STATUS) : GetPString(IDS_SUBJ),
		  *bool,
		  FALSE);

  bool = dcd ? &dcd->detailsattr : &detailsattr;
  AdjustCnrColVis(hwndCnr, GetPString(IDS_ATTR), *bool, FALSE);
  bool = dcd ? &dcd->detailsicon : &detailsicon;
  AdjustCnrColVis(hwndCnr, GetPString(IDS_ICON), *bool, FALSE);
  bool = dcd ? &dcd->detailslwdate : &detailslwdate;
  AdjustCnrColVis(hwndCnr, GetPString(IDS_LWDATE), *bool, FALSE);
  bool = dcd ? &dcd->detailslwtime : &detailslwtime;
  AdjustCnrColVis(hwndCnr, GetPString(IDS_LWTIME), *bool, FALSE);
  bool = dcd ? &dcd->detailsea : &detailsea;
  AdjustCnrColVis(hwndCnr, GetPString(IDS_EA), *bool, FALSE);
  bool = dcd ? &dcd->detailssize : &detailssize;
  AdjustCnrColVis(hwndCnr, GetPString(IDS_SIZE), *bool, FALSE);

  if (!directory) {
    bool = dcd ? &dcd->detailsladate : &detailsladate;
    AdjustCnrColVis(hwndCnr, GetPString(IDS_LADATE), *bool, FALSE);
    bool = dcd ? &dcd->detailslatime : &detailslatime;
    AdjustCnrColVis(hwndCnr, GetPString(IDS_LATIME), *bool, FALSE);
    bool = dcd ? &dcd->detailscrdate : &detailscrdate;
    AdjustCnrColVis(hwndCnr, GetPString(IDS_CRDATE), *bool, FALSE);
    bool = dcd ? &dcd->detailscrtime : &detailscrtime;
    AdjustCnrColVis(hwndCnr, GetPString(IDS_CRTIME), *bool, FALSE);
    bool = dcd ? &dcd->detailslongname : &detailslongname;
    AdjustCnrColVis(hwndCnr, GetPString(IDS_LNAME), *bool, FALSE);
    WinSendMsg(hwndCnr, CM_INVALIDATEDETAILFIELDINFO, MPVOID, MPVOID);
  }
  else
    AdjustCnrColsForFSType(hwndCnr, directory, dcd);
}

BOOL SetCnrCols(HWND hwndCnr, BOOL isCompCnr)
{
  BOOL fSuccess = TRUE;
  PFIELDINFO pfi, pfiLastLeftCol, pfiIconCol;

  // Allocate storage for container column data

  pfi = WinSendMsg(hwndCnr, CM_ALLOCDETAILFIELDINFO,
		   MPFROMLONG(CONTAINER_COLUMNS), NULL);

  if (!pfi) {
    Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__, "CM_ALLOCDETAILFIELDINFO");
    fSuccess = FALSE;
  }
  else {

    PFIELDINFO pfiFirst;
    FIELDINFOINSERT fii;

    // Store original value of pfi so we won't lose it when it changes.
    // This will be needed on the CM_INSERTDETAILFIELDINFO message.

    pfiFirst = pfi;

    // Fill in column information for the icon column

    pfi->flData = CFA_BITMAPORICON | CFA_CENTER | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER | CFA_FITITLEREADONLY;
    pfi->pTitleData = GetPString(IDS_ICON);
    pfi->offStruct = FIELDOFFSET(MINIRECORDCORE, hptrIcon);

    pfiIconCol = pfi;

    // Fill in column information for the file name. Note that we are
    // using the pszDisplayName variable rather than pszFileName. We do this
    // because the container does not always display the full path file name.

    pfi = pfi->pNextFieldInfo;

    pfi->flData = CFA_STRING | CFA_LEFT | CFA_SEPARATOR;
    pfi->flTitle = CFA_LEFT;
    pfi->pTitleData = GetPString(IDS_FILENAME);
    pfi->offStruct = FIELDOFFSET(CNRITEM, pszDisplayName);

    // Fill in column information for the longname.

    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_STRING | CFA_LEFT;
    pfi->flTitle = CFA_LEFT | CFA_FITITLEREADONLY;
    pfi->pTitleData = GetPString(IDS_LNAME);
    pfi->offStruct = FIELDOFFSET(CNRITEM, pszLongName);

    // Fill in column info for subjects

    if (fSubjectInLeftPane) {
      pfi = pfi->pNextFieldInfo;
      pfi->flData = CFA_STRING | CFA_LEFT | CFA_SEPARATOR;
      if (isCompCnr)
	pfi->flData |= CFA_FIREADONLY;
      pfi->flTitle = CFA_LEFT | CFA_FITITLEREADONLY;
      pfi->pTitleData = isCompCnr ? GetPString(IDS_STATUS) :
				  GetPString(IDS_SUBJ);
      pfi->offStruct = FIELDOFFSET(CNRITEM, pszSubject);
      pfi->cxWidth = SubjectDisplayWidth;

      // Store the current pfi value as that will be used to indicate the
      // last column in the lefthand container window (we have a splitbar)

      pfiLastLeftCol = pfi;
    }
    else {
      // Store the current pfi value as that will be used to indicate the
      // last column in the lefthand container window (we have a splitbar)

      pfiLastLeftCol = pfi;
      pfi = pfi->pNextFieldInfo;
      pfi->flData = CFA_STRING | CFA_LEFT | CFA_SEPARATOR;
      if (isCompCnr)
	pfi->flData |= CFA_FIREADONLY;
      pfi->flTitle = CFA_LEFT | CFA_FITITLEREADONLY;
      pfi->pTitleData = isCompCnr ? GetPString(IDS_STATUS) :
				  GetPString(IDS_SUBJ);
      pfi->offStruct = FIELDOFFSET(CNRITEM, pszSubject);
      pfi->cxWidth = SubjectDisplayWidth;
    }

    // Fill in column information for the file size


    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_STRING | CFA_RIGHT | CFA_SEPARATOR | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER;
    pfi->pTitleData = GetPString(IDS_SIZE);
    pfi->offStruct = FIELDOFFSET(CNRITEM, pszFmtFileSize);


    // Fill in the column information for the file's ea size

    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_ULONG | CFA_RIGHT | CFA_SEPARATOR | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER;
    pfi->pTitleData = GetPString(IDS_EA);
    pfi->offStruct = FIELDOFFSET(CNRITEM, easize);

    // Fill in the column information for the file attribute

    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_STRING | CFA_CENTER | CFA_SEPARATOR | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER | CFA_FITITLEREADONLY;
    pfi->pTitleData = GetPString(IDS_ATTR);
    pfi->offStruct = FIELDOFFSET(CNRITEM, pszDispAttr);

    // Fill in column information for last write file date

    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_DATE | CFA_RIGHT | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER;
    pfi->pTitleData = GetPString(IDS_LWDATE);
    pfi->offStruct = FIELDOFFSET(CNRITEM, date);

    // Fill in column information for the last write file time

    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_TIME | CFA_RIGHT | CFA_SEPARATOR | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER;
    pfi->pTitleData = GetPString(IDS_LWTIME);
    pfi->offStruct = FIELDOFFSET(CNRITEM, time);

    // Fill in column information for last access file date

    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_DATE | CFA_RIGHT | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER;
    pfi->pTitleData = GetPString(IDS_LADATE);
    pfi->offStruct = FIELDOFFSET(CNRITEM, ladate);

    // Fill in column information for the last access file time

    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_TIME | CFA_RIGHT | CFA_SEPARATOR | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER;
    pfi->pTitleData = GetPString(IDS_LATIME);
    pfi->offStruct = FIELDOFFSET(CNRITEM, latime);

    // Fill in column information for create file date

    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_DATE | CFA_RIGHT | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER;
    pfi->pTitleData = GetPString(IDS_CRDATE);
    pfi->offStruct = FIELDOFFSET(CNRITEM, crdate);

    // Fill in column information for the create file time

    pfi = pfi->pNextFieldInfo;
    pfi->flData = CFA_TIME | CFA_RIGHT | CFA_FIREADONLY;
    pfi->flTitle = CFA_CENTER;
    pfi->pTitleData = GetPString(IDS_CRTIME);
    pfi->offStruct = FIELDOFFSET(CNRITEM, crtime);

    // Use the CM_INSERTDETAILFIELDINFO message to tell the container
    // all the column information it needs to function properly. Place
    // this column info first in the column list and update the display
    // after they are inserted (fInvalidateFieldInfo = TRUE)

    (void)memset(&fii, 0, sizeof(FIELDINFOINSERT));

    fii.cb = sizeof(FIELDINFOINSERT);
    fii.pFieldInfoOrder = (PFIELDINFO) CMA_FIRST;
    fii.cFieldInfoInsert = (SHORT) CONTAINER_COLUMNS;
    fii.fInvalidateFieldInfo = TRUE;

    if (!WinSendMsg(hwndCnr, CM_INSERTDETAILFIELDINFO, MPFROMP(pfiFirst),
		    MPFROMP(&fii))) {
      Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__, "CM_INSERTDETAILFIELDINFO");
      fSuccess = FALSE;
    }
  }

  if (fSuccess) {

    CNRINFO cnri;
    ULONG size;

    // Tell the container about the splitbar and where it goes

    cnri.cb = sizeof(CNRINFO);
    cnri.pFieldInfoLast = pfiLastLeftCol;
    cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 32;
    cnri.pFieldInfoObject = pfiIconCol;
    size = sizeof(LONG);
    PrfQueryProfileData(fmprof,
			appname, "CnrSplitBar", &cnri.xVertSplitbar, &size);
    if (cnri.xVertSplitbar <= 0)
      cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 32;
    if (!WinSendMsg(hwndCnr, CM_SETCNRINFO, MPFROMP(&cnri),
		    MPFROMLONG(CMA_PFIELDINFOLAST | CMA_PFIELDINFOOBJECT |
			       CMA_XVERTSPLITBAR))) {
      Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__, "CM_SETCNRINFO");
      fSuccess = FALSE;
    }
  }

  return fSuccess;
}

MRESULT CnrDirectEdit(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (SHORT2FROMMP(mp1)) {
  case CN_BEGINEDIT:
    if (mp2) {
      PFIELDINFO pfi = ((PCNREDITDATA) mp2)->pFieldInfo;
      PCNRITEM pci = (PCNRITEM) ((PCNREDITDATA) mp2)->pRecord;

      if (pci &&
	  (INT) pci != -1 &&
	  !IsRoot(pci->pszFileName) &&
	  !(pci->flags & RECFLAGS_ENV) && !(pci->flags & RECFLAGS_UNDERENV)) {
	if (!pfi || pfi->offStruct == FIELDOFFSET(CNRITEM, pszDisplayName)) {
	  PostMsg(hwnd, UM_FIXEDITNAME, MPFROMP(pci->pszFileName), MPVOID);
	}
	else if (pfi->offStruct == FIELDOFFSET(CNRITEM, pszSubject))
	  PostMsg(hwnd, UM_FIXCNRMLE, MPFROMLONG(40), MPVOID);
	else
	  PostMsg(hwnd, UM_FIXCNRMLE, MPFROMLONG(CCHMAXPATH), MPVOID);
      }
      else
	PostMsg(hwnd, CM_CLOSEEDIT, MPVOID, MPVOID);
    }
    break;

  case CN_REALLOCPSZ:
    if (mp2) {
      PFIELDINFO pfi = ((PCNREDITDATA) mp2)->pFieldInfo;
      PCNRITEM pci = (PCNRITEM) ((PCNREDITDATA) mp2)->pRecord;
      CHAR szData[CCHMAXPATH], testname[CCHMAXPATH];
      HWND hwndMLE = WinWindowFromID(hwnd, CID_MLE);

      if (pci && (INT) pci != -1 && !IsRoot(pci->pszFileName)) {
	if (pfi && pfi->offStruct == FIELDOFFSET(CNRITEM, pszSubject)) {

	  APIRET rc;
	  EAOP2 eaop;
	  PFEA2LIST pfealist = NULL;
	  CHAR szSubject[1048];
	  ULONG ealen;
	  USHORT len;
	  CHAR *eaval;
	  LONG retlen;

	  retlen = WinQueryWindowText(hwndMLE, sizeof(szSubject), szSubject);
	  szSubject[retlen + 1] = 0;
	  //chop_at_crnl(szSubject);
	  bstrip(szSubject);
	  pci->pszSubject = xrealloc(pci->pszSubject, retlen + 1, pszSrcFile, __LINE__);
	  WinSetWindowText(hwndMLE, szSubject);
	  len = strlen(szSubject);
	  if (len)
	    ealen = sizeof(FEA2LIST) + 9 + len + 4;
	  else
	    ealen = sizeof(FEALIST) + 9;
	  rc = DosAllocMem((PPVOID) & pfealist, ealen + 64,
			   OBJ_TILE | PAG_COMMIT | PAG_READ | PAG_WRITE);
	  if (rc)
	    Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile,
		      __LINE__, GetPString(IDS_OUTOFMEMORY));
	  else {
	    memset(pfealist, 0, ealen + 1);
	    pfealist->cbList = ealen;
	    pfealist->list[0].oNextEntryOffset = 0;
	    pfealist->list[0].fEA = 0;
	    pfealist->list[0].cbName = 8;
	    strcpy(pfealist->list[0].szName, SUBJECT);
	    if (len) {
	      eaval = pfealist->list[0].szName + 9;
	      *(USHORT *) eaval = (USHORT) EAT_ASCII;
	      eaval += sizeof(USHORT);
	      *(USHORT *) eaval = (USHORT) len;
	      eaval += sizeof(USHORT);
	      memcpy(eaval, szSubject, len);
	      pfealist->list[0].cbValue = len + (sizeof(USHORT) * 2);
	    }
	    else
	      pfealist->list[0].cbValue = 0;
	    eaop.fpGEA2List = (PGEA2LIST) 0;
	    eaop.fpFEA2List = pfealist;
	    eaop.oError = 0L;
	    rc = xDosSetPathInfo(pci->pszFileName, FIL_QUERYEASIZE,
				 &eaop, sizeof(eaop), DSPI_WRTTHRU);
	    DosFreeMem(pfealist);
	    if (rc)
	      return FALSE;
	  }
	  return (MRESULT) TRUE;
	}
	else if (pfi && pfi->offStruct == FIELDOFFSET(CNRITEM, pszLongName)) {

	  CHAR longname[CCHMAXPATHCOMP];
	LONG retlen;

	  *longname = 0;
	  retlen = WinQueryWindowText(hwndMLE, sizeof(longname), longname);
	  longname[retlen + 1] = 0;
	  //chop_at_crnl(longname);
	  pci->pszLongName = xrealloc(pci->pszLongName, retlen + 1, pszSrcFile, __LINE__);
	  WinSetWindowText(hwndMLE, longname);
	  pci->pszFileName = xrealloc(pci->pszFileName, retlen + 1, pszSrcFile, __LINE__);
	  return (MRESULT) WriteLongName(pci->pszFileName, longname);
	}
	else {
	  WinQueryWindowText(hwndMLE, sizeof(szData), szData);
	  if (strchr(szData, '?') ||
	      strchr(szData, '*') || IsRoot(pci->pszFileName))
	    return (MRESULT) FALSE;
	  /* If the text changed, rename the file system object. */
	  chop_at_crnl(szData);
	  bstrip(szData);
	  if (!IsFullName(szData))
	    Runtime_Error(pszSrcFile, __LINE__, "bad name");
	  else {
	    if (DosQueryPathInfo(szData,
				 FIL_QUERYFULLNAME,
				 testname, sizeof(testname)))
		return FALSE;
	    if (DosQueryPathInfo(pci->pszFileName,
				 FIL_QUERYFULLNAME, szData, sizeof(szData))){
	      pci->pszFileName = xrealloc(pci->pszFileName, sizeof(szData), pszSrcFile, __LINE__);
	      strcpy(szData, pci->pszFileName);
	    }
	    WinSetWindowText(hwndMLE, szData);
	    if (strcmp(szData, testname)) {
	      if (stricmp(szData, testname) && IsFile(testname) != -1) {
		DosBeep(50, 100);	/* exists; disallow */
		return (MRESULT) FALSE;
	      }
	      if (docopyf(MOVE, szData, "%s", testname))
		Runtime_Error(pszSrcFile, __LINE__, "docopyf");
	      else {
		CHAR *filename;

		filename = xstrdup(testname, pszSrcFile, __LINE__);
		if (filename) {
		  if (!PostMsg(hwnd,
			       UM_FIXEDITNAME, MPVOID, MPFROMP(filename)))
		    free(filename);
		}
		if (stricmp(testname, pci->pszFileName)) {
		  PostMsg(hwnd, UM_FIXEDITNAME, MPFROMLONG(-1), MPFROMP(pci));
		  filename = xstrdup(pci->pszFileName, pszSrcFile, __LINE__);
		  if (filename) {
		    if (!PostMsg(hwnd,
				 UM_FIXEDITNAME, MPVOID, MPFROMP(filename)))
		      free(filename);
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    return FALSE;

  case CN_ENDEDIT:
    if (mp2) {
      PFIELDINFO pfi = ((PCNREDITDATA) mp2)->pFieldInfo;
      PCNRITEM pci = (PCNRITEM) ((PCNREDITDATA) mp2)->pRecord;

      if (pci && (INT) pci != -1 && !IsRoot(pci->pszFileName)) {
	WinSendMsg(hwnd,
		   CM_INVALIDATERECORD,
		   MPFROMP(&pci),
		   MPFROM2SHORT(1, CMA_ERASE | CMA_TEXTCHANGED));
	if (pfi && pfi->offStruct == FIELDOFFSET(CNRITEM, pszDisplayName))
	  PostMsg(hwnd, UM_SORTRECORD, MPVOID, MPVOID);
      }
      else {
	USHORT cmd = 0;

	if (!pfi || pfi->offStruct == FIELDOFFSET(CNRITEM, pszDisplayName))
	  cmd = IDM_SORTSMARTNAME;
	else if (pfi->offStruct == FIELDOFFSET(CNRITEM, cbFile))
	  cmd = IDM_SORTSIZE;
	else if (pfi->offStruct == FIELDOFFSET(CNRITEM, easize))
	  cmd = IDM_SORTEASIZE;
	else if (pfi->offStruct == FIELDOFFSET(CNRITEM, date))
	  cmd = IDM_SORTLWDATE;
	else if (pfi->offStruct == FIELDOFFSET(CNRITEM, time))
	  cmd = IDM_SORTLWDATE;
	else if (pfi->offStruct == FIELDOFFSET(CNRITEM, ladate))
	  cmd = IDM_SORTLADATE;
	else if (pfi->offStruct == FIELDOFFSET(CNRITEM, latime))
	  cmd = IDM_SORTLADATE;
	else if (pfi->offStruct == FIELDOFFSET(CNRITEM, crdate))
	  cmd = IDM_SORTCRDATE;
	else if (pfi->offStruct == FIELDOFFSET(CNRITEM, crtime))
	  cmd = IDM_SORTCRDATE;
	if (cmd)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(cmd, 0), MPVOID);
      }
    }
    break;
  }
  return (MRESULT) - 1;
}

BOOL SetMenuCheck(HWND hwndMenu, USHORT id, BOOL * bool, BOOL toggle,
		  CHAR * savename)
{
  if (toggle) {
    *bool = *bool ? FALSE : TRUE;
    if (savename && *savename)
      PrfWriteProfileData(fmprof, appname, savename, bool, sizeof(BOOL));
  }
  WinSendMsg(hwndMenu, MM_SETITEMATTR,
	     MPFROM2SHORT(id, 1),
	     MPFROM2SHORT(MIA_CHECKED, (*bool ? MIA_CHECKED : 0)));
  return *bool;
}

//== disable_menuitem() disable or enable_menuitem ==

VOID disable_menuitem(HWND hwndMenu, USHORT id, BOOL disable)
{
  WinSendMsg(hwndMenu, MM_SETITEMATTR,
	     MPFROM2SHORT(id, TRUE),
	     MPFROM2SHORT(MIA_DISABLED, (disable ? MIA_DISABLED : 0)));
}

//== ViewHelp() invoke view.exe, return TRUE if OK ==

BOOL ViewHelp(CHAR * filename)
{
  CHAR s[CCHMAXPATH + 81];
  FILE *fp;
  INT ret = -1;

  fp = _fsopen(filename, "rb", SH_DENYNO);
  if (fp) {
    *s = 0;
    fread(s, 1, 3, fp);
    if (*s != 'H' || s[1] != 'S' || s[2] != 'P') {
      fclose(fp);
      return FALSE;
    }
    fclose(fp);
    ret = runemf2(SEPARATE | WINDOWED, HWND_DESKTOP, NULL, NULL,
		  "VIEW.EXE \"%s\"", filename);
  }

  return (ret != -1);
}

//== ExecFile() run file, return 1 if OK 0 if skipped -1 if can't run ==

INT ExecFile(HWND hwnd, CHAR * filename)
{
  EXECARGS ex;
  CHAR cl[1001], path[CCHMAXPATH], *p;
  APIRET ret;
  static INT lastflags = 0;

  strcpy(path, filename);
  p = strrchr(path, '\\');
  if (!p)
    p = strrchr(path, ':');
  if (p) {
    if (*p == ':') {
      p++;
      *p = '\\';
      p++;
    }
    *p = 0;
  }
  else
    *path = 0;
  *cl = 0;
  BldQuotedFileName(cl, filename);
  // *cl = 0;
  // if (needs_quoting(filename))
  //   strcat(cl, "\"");
  // strcat(cl, filename);
  // if (needs_quoting(filename))
  //   strcat(cl, "\"");
  memset(&ex, 0, sizeof(ex));
  ex.flags = lastflags;
  ex.commandline = cl;
  *ex.path = 0;
  *ex.environment = 0;
  ret = WinDlgBox(HWND_DESKTOP, hwnd, CmdLineDlgProc, FM3ModHandle,
		  EXEC_FRAME, &ex);
  if (ret == 1) {
    lastflags = ex.flags;
    return runemf2(ex.flags, hwnd, path,
		   *ex.environment ? ex.environment : NULL,
		   "%s", cl) != -1;
  }
  else if (ret != 0)
    return -1;
  return 0;
}

VOID SetDetailsSwitches(HWND hwnd, DIRCNRDATA * dcd)
{
  WinCheckMenuItem(hwnd, IDM_SHOWLNAMES,
		   dcd ? dcd->detailslongname : detailslongname);
  WinCheckMenuItem(hwnd, IDM_SHOWSUBJECT,
		   dcd ? dcd->detailssubject : detailssubject);
  WinCheckMenuItem(hwnd, IDM_SHOWEAS, dcd ? dcd->detailsea : detailsea);
  WinCheckMenuItem(hwnd, IDM_SHOWSIZE,
		   dcd ? dcd->detailssize : detailssize);
  WinCheckMenuItem(hwnd, IDM_SHOWICON,
		   dcd ? dcd->detailsicon : detailsicon);
  WinCheckMenuItem(hwnd, IDM_SHOWLWDATE,
		   dcd ? dcd->detailslwdate : detailslwdate);
  WinCheckMenuItem(hwnd, IDM_SHOWLWTIME,
		   dcd ? dcd->detailslwtime : detailslwtime);
  WinCheckMenuItem(hwnd, IDM_SHOWLADATE,
		   dcd ? dcd->detailsladate : detailsladate);
  WinCheckMenuItem(hwnd, IDM_SHOWLATIME,
		   dcd ? dcd->detailslatime : detailslatime);
  WinCheckMenuItem(hwnd, IDM_SHOWCRDATE,
		   dcd ? dcd->detailscrdate : detailscrdate);
  WinCheckMenuItem(hwnd, IDM_SHOWCRTIME,
		   dcd ? dcd->detailscrtime : detailscrtime);
  WinCheckMenuItem(hwnd, IDM_SHOWATTR,
		   dcd ? dcd->detailsattr : detailsattr);
}

VOID AdjustDetailsSwitches(HWND hwnd, HWND hwndMenu, USHORT cmd,
			   CHAR * directory, CHAR * keyroot,
			   DIRCNRDATA * dcd, BOOL compare)
{
  CHAR s[CCHMAXPATH], *eos = s;
  BOOL *bool = NULL;

  *s = 0;
  if (keyroot) {
    strcpy(s, keyroot);
    strcat(s, ".");
    eos = &s[strlen(s)];
  }
  switch (cmd) {
  case IDM_SHOWLNAMES:
    bool = dcd ? &dcd->detailslongname : &detailslongname;
    strcpy(eos, "DetailsLongname");
    break;
  case IDM_SHOWSUBJECT:
    bool = dcd ? &dcd->detailssubject : &detailssubject;
    strcpy(eos, "DetailsSubject");
    break;
  case IDM_SHOWEAS:
    bool = dcd ? &dcd->detailsea : &detailsea;
    strcpy(eos, "DetailsEA");
    break;
  case IDM_SHOWSIZE:
    bool = dcd ? &dcd->detailssize : &detailssize;
    strcpy(eos, "DetailsSize");
    break;
  case IDM_SHOWICON:
    bool = dcd ? &dcd->detailsicon : &detailsicon;
    strcpy(eos, "DetailsIcon");
    break;
  case IDM_SHOWLWDATE:
    bool = dcd ? &dcd->detailslwdate : &detailslwdate;
    strcpy(eos, "DetailsLWDate");
    break;
  case IDM_SHOWLWTIME:
    bool = dcd ? &dcd->detailslwtime : &detailslwtime;
    strcpy(eos, "DetailsLWTime");
    break;
  case IDM_SHOWLADATE:
    bool = dcd ? &dcd->detailsladate : &detailsladate;
    strcpy(eos, "DetailsLADate");
    break;
  case IDM_SHOWLATIME:
    bool = dcd ? &dcd->detailslatime : &detailslatime;
    strcpy(eos, "DetailsLATime");
    break;
  case IDM_SHOWCRDATE:
    bool = dcd ? &dcd->detailscrdate : &detailscrdate;
    strcpy(eos, "DetailsCRDate");
    break;
  case IDM_SHOWCRTIME:
    bool = dcd ? &dcd->detailscrtime : &detailscrtime;
    strcpy(eos, "DetailsCRTime");
    break;
  case IDM_SHOWATTR:
    bool = dcd ? &dcd->detailsattr : &detailsattr;
    strcpy(eos, "DetailsAttr");
    break;
  default:
    if (hwndMenu)
      SetDetailsSwitches(hwndMenu, dcd);
    return;
  }
  if (bool)
    *bool = *bool ? FALSE : TRUE;
  if (*s && bool)
    PrfWriteProfileData(fmprof, appname, s, bool, sizeof(BOOL));
  if (hwnd)
    AdjustCnrColsForPref(hwnd, directory, dcd, compare);
  if (hwndMenu)
    SetDetailsSwitches(hwndMenu, dcd);
}

VOID SetConditionalCascade(HWND hwndMenu, USHORT id, USHORT def)
{
  MENUITEM mi;

  mi.iPosition = MIT_END;
  mi.hItem = 0L;
  mi.hwndSubMenu = (HWND) 0;
  mi.afAttribute = 0;
  mi.afStyle = MIS_TEXT;
  if (WinSendMsg
      (hwndMenu, MM_QUERYITEM, MPFROM2SHORT(id, TRUE), MPFROMP(&mi))) {
    WinSetWindowBits(mi.hwndSubMenu, QWL_STYLE, MS_CONDITIONALCASCADE,
		     MS_CONDITIONALCASCADE);
    WinSendMsg(mi.hwndSubMenu, MM_SETDEFAULTITEMID, MPFROMSHORT(def), MPVOID);
    WinCheckMenuItem(mi.hwndSubMenu, def, TRUE);
  }
}

VOID SetSortChecks(HWND hwndMenu, INT sortflags)
{
  WinCheckMenuItem(hwndMenu, IDM_SORTNONE, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTFIRST, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTLAST, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTSIZE, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTEASIZE, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTLWDATE, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTLADATE, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTCRDATE, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTFILENAME, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTNAME, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTSUBJECT, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTDIRSFIRST, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTDIRSLAST, FALSE);
  WinCheckMenuItem(hwndMenu, IDM_SORTREVERSE, FALSE);
  if (sortflags & SORT_FIRSTEXTENSION)
    WinCheckMenuItem(hwndMenu, IDM_SORTFIRST, TRUE);
  else if (sortflags & SORT_LASTEXTENSION)
    WinCheckMenuItem(hwndMenu, IDM_SORTLAST, TRUE);
  else if (sortflags & SORT_SIZE)
    WinCheckMenuItem(hwndMenu, IDM_SORTSIZE, TRUE);
  else if (sortflags & SORT_EASIZE)
    WinCheckMenuItem(hwndMenu, IDM_SORTEASIZE, TRUE);
  else if (sortflags & SORT_LWDATE)
    WinCheckMenuItem(hwndMenu, IDM_SORTLWDATE, TRUE);
  else if (sortflags & SORT_LADATE)
    WinCheckMenuItem(hwndMenu, IDM_SORTLADATE, TRUE);
  else if (sortflags & SORT_CRDATE)
    WinCheckMenuItem(hwndMenu, IDM_SORTCRDATE, TRUE);
  else if (sortflags & SORT_FILENAME)
    WinCheckMenuItem(hwndMenu, IDM_SORTFILENAME, TRUE);
  else if (sortflags & SORT_NOSORT)
    WinCheckMenuItem(hwndMenu, IDM_SORTNONE, TRUE);
  else if (sortflags & SORT_SUBJECT)
    WinCheckMenuItem(hwndMenu, IDM_SORTSUBJECT, TRUE);
  else
    WinCheckMenuItem(hwndMenu, IDM_SORTNAME, TRUE);
  if (sortflags & SORT_DIRSFIRST)
    WinCheckMenuItem(hwndMenu, IDM_SORTDIRSFIRST, TRUE);
  else if (sortflags & SORT_DIRSLAST)
    WinCheckMenuItem(hwndMenu, IDM_SORTDIRSLAST, TRUE);
  if (sortflags & SORT_REVERSE)
    WinCheckMenuItem(hwndMenu, IDM_SORTREVERSE, TRUE);
}

VOID FcloseFile(FILE * fp)
{
  /* for use by apps that don't use the DLLs runtime library */
  fclose(fp);
}

VOID SetupCommandMenu(HWND hwndMenu, HWND hwndCnr)
{
  MENUITEM mi, mit;
  INT x;
  SHORT numitems;
  LINKCMDS *info;

  if (!cmdloaded)
    load_commands();
  mi.iPosition = MIT_END;
  mi.hwndSubMenu = (HWND) 0;
  mi.hItem = 0L;
  mi.afAttribute = 0;
  mi.afStyle = MIS_TEXT;
  memset(&mit, 0, sizeof(MENUITEM));
  if (WinQueryWindowUShort(hwndMenu, QWS_ID) == IDM_COMMANDSMENU)
    mit.hwndSubMenu = hwndMenu;
  else
    WinSendMsg(hwndMenu, MM_QUERYITEM,
	       MPFROM2SHORT(IDM_COMMANDSMENU, TRUE), MPFROMP(&mit));
  if (mit.hwndSubMenu) {
    numitems = (SHORT) WinSendMsg(mit.hwndSubMenu, MM_QUERYITEMCOUNT,
				  MPVOID, MPVOID);
    WinSendMsg(mit.hwndSubMenu, MM_DELETEITEM, MPFROMSHORT(-1), MPVOID);
    for (x = 0; x < numitems; x++)
      WinSendMsg(mit.hwndSubMenu, MM_DELETEITEM,
		 MPFROMSHORT((SHORT) (x + IDM_COMMANDSTART)), MPVOID);
    if (hwndCnr && cmdhead) {
      x = 0;
      info = cmdhead;
      while (info) {

	CHAR s[CCHMAXPATH + 24];

	sprintf(s,
		"%s%s%s",
		info->title,
		x < 20 ? "\tCtrl + " : NullStr,
		x < 20 && x > 9 ? "Shift + " : NullStr);
	if (x < 20)
	  sprintf(&s[strlen(s)], "%d",
		  ((x % 10) + 1) == 10 ? 0 : (x % 10) + 1);
	mi.id = IDM_COMMANDSTART + x;
	mi.afAttribute = (info->flags & ONCE ? MIA_CHECKED : 0) |
			 (info->flags & PROMPT ? MIA_FRAMED : 0);
	mi.afStyle = MIS_TEXT;
	if (!(x % 24) && x && info->next)
	  mi.afStyle |= MIS_BREAK;
	WinSendMsg(mit.hwndSubMenu, MM_INSERTITEM, MPFROMP(&mi), MPFROMP(s));
	x++;
	info = info->next;
      }
    }
  }
}

VOID LoadDetailsSwitches(CHAR * keyroot, DIRCNRDATA * dcd)
{
  ULONG size;
  CHAR s[CCHMAXPATH], *eos = s;
  BOOL *bool;

  *s = 0;
  if (keyroot) {
    strcpy(s, keyroot);
    strcat(s, ".");
    eos = &s[strlen(s)];
  }
  strcpy(eos, "DetailsLongname");
  if (dcd)
    bool = &dcd->detailslongname;
  else
    bool = &detailslongname;
  *bool = detailslongname;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsSubject");
  if (dcd)
    bool = &dcd->detailssubject;
  else
    bool = &detailssubject;
  *bool = detailssubject;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsEA");
  if (dcd)
    bool = &dcd->detailsea;
  else
    bool = &detailsea;
  *bool = detailsea;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsSize");
  if (dcd)
    bool = &dcd->detailssize;
  else
    bool = &detailssize;
  *bool = detailssize;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsIcon");
  if (dcd)
    bool = &dcd->detailsicon;
  else
    bool = &detailsicon;
  *bool = detailsicon;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsAttr");
  if (dcd)
    bool = &dcd->detailsattr;
  else
    bool = &detailsattr;
  *bool = detailsattr;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsCRDate");
  if (dcd)
    bool = &dcd->detailscrdate;
  else
    bool = &detailscrdate;
  *bool = detailscrdate;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsCRTime");
  if (dcd)
    bool = &dcd->detailscrtime;
  else
    bool = &detailscrtime;
  *bool = detailscrtime;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsLWDate");
  if (dcd)
    bool = &dcd->detailslwdate;
  else
    bool = &detailslwdate;
  *bool = detailslwdate;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsLWTime");
  if (dcd)
    bool = &dcd->detailslwtime;
  else
    bool = &detailslwtime;
  *bool = detailslwtime;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsLADate");
  if (dcd)
    bool = &dcd->detailsladate;
  else
    bool = &detailsladate;
  *bool = detailsladate;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
  strcpy(eos, "DetailsLATime");
  if (dcd)
    bool = &dcd->detailslatime;
  else
    bool = &detailslatime;
  *bool = detailslatime;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, s, (PVOID) bool, &size);
}

HWND FindDirCnr(HWND hwndParent)
{
  HWND found, hwndDir = (HWND) 0;
  HENUM henum;

  henum = WinBeginEnumWindows(hwndParent);
  while ((found = WinGetNextWindow(henum)) != NULLHANDLE) {
    hwndDir = WinWindowFromID(found, FID_CLIENT);
    if (hwndDir) {
      hwndDir = WinWindowFromID(hwndDir, DIR_CNR);
      if (hwndDir)
	break;
      hwndDir = (HWND) 0;
    }
  }
  WinEndEnumWindows(henum);

  return hwndDir;
}

VOID HeapThread(VOID * dummy)
{
  ULONG postcount;
  APIRET rc;

  rc = DosCreateEventSem(NULL, &CompactSem, 0L, FALSE);
  if (rc)
    Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      "DosCreateEventSem");
  else {
    priority_normal();
    for (;;) {
      if (DosWaitEventSem(CompactSem, SEM_INDEFINITE_WAIT))
	break;
      _heapmin();
      DosResetEventSem(CompactSem, &postcount);
    }
  }
}

VOID FixSwitchList(HWND hwnd, CHAR * text)
{
  HSWITCH hswitch;
  SWCNTRL swctl;

  hswitch = WinQuerySwitchHandle(hwnd, 0);
  if (hswitch) {
    if (!WinQuerySwitchEntry(hswitch, &swctl)) {
      strcpy(swctl.szSwtitle, "FM/2");
      WinChangeSwitchEntry(hswitch, &swctl);
    }
  }
}

VOID QuickPopup(HWND hwnd, DIRCNRDATA * dcd, HWND hwndMenu, USHORT id)
{
  dcd->hwndLastMenu = hwndMenu;
  if (dcd->hwndLastMenu && !dcd->cnremphasized) {
    WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
	       MPFROM2SHORT(TRUE, CRA_SOURCE));
    dcd->cnremphasized = TRUE;
  }
  if (dcd->flWindowAttr & CV_MINI)
    WinCheckMenuItem(dcd->hwndLastMenu, IDM_MINIICONS, TRUE);
  if (!WinPopupMenu(hwnd, hwnd, dcd->hwndLastMenu,
		    8, 8, 0,
		    PU_HCONSTRAIN | PU_VCONSTRAIN |
		    PU_KEYBOARD | PU_MOUSEBUTTON1)) {
    if (dcd->cnremphasized) {
      WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
		 MPFROM2SHORT(FALSE, CRA_SOURCE));
      dcd->cnremphasized = FALSE;
    }
  }
  else
    WinSendMsg(dcd->hwndLastMenu, MM_SELECTITEM,
	       MPFROM2SHORT(id, TRUE), MPFROM2SHORT(0, FALSE));
}

PMINIRECORDCORE CurrentRecord(HWND hwndCnr)
{
  SHORT attrib = fSelectedAlways ? CRA_SELECTED : CRA_CURSORED;
  PMINIRECORDCORE pmi;

  for (;;) {
    pmi = (PMINIRECORDCORE) WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
				       MPFROMLONG(CMA_FIRST),
				       MPFROMSHORT(attrib));
    if ((!pmi || (INT) pmi == -1) && attrib == CRA_SELECTED)	/* punt */
      attrib = CRA_CURSORED;
    else
      break;
  }
  return ((INT)pmi == -1) ? NULL : pmi;
}

BOOL PostMsg(HWND h, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  BOOL rc = WinPostMsg(h, msg, mp1, mp2);

  if (!rc) {

      // If window owned by some other process or some other thread?
      if (!IsFm2Window(h, 1)) {
	  QMSG qmsg;
	  for (;;) {
	    DosSleep(1);
	    rc = WinPostMsg(h, msg, mp1, mp2);
	    if (rc)
	      break;			// OK
	    if (!WinIsWindow((HAB) 0, h))
	      break;			// Window gone
	    if (WinPeekMsg((HAB) 0, &qmsg, (HWND) 0, 0, 0, PM_NOREMOVE))
	      break;			// Queue has message(s)
	  }				// for
	}
    }
  return rc;
}

VOID OpenEdit(HWND hwnd)
{
  CNREDITDATA ced;
  PCNRITEM pci;
  PFIELDINFO pfi;

  pci = (PCNRITEM) WinSendMsg(hwnd,
			      CM_QUERYRECORDEMPHASIS,
			      MPFROMLONG(CMA_FIRST),
			      MPFROMSHORT(CRA_CURSORED));
  if (pci && (INT) pci != -1) {
    memset(&ced, 0, sizeof(ced));
    ced.cb = sizeof(ced);
    ced.hwndCnr = hwnd;
    ced.id = WinQueryWindowUShort(hwnd, QWS_ID);
    ced.pRecord = (PRECORDCORE) pci;
    pfi = (PFIELDINFO) WinSendMsg(hwnd,
				  CM_QUERYDETAILFIELDINFO,
				  MPVOID, MPFROMSHORT(CMA_FIRST));
    if (!pfi)
      WinSendMsg(hwnd, CM_OPENEDIT, MPFROMP(&ced), MPVOID);
    else {
      while (pfi && (INT) pfi != -1 &&
	     pfi->offStruct != FIELDOFFSET(CNRITEM, pszFileName))
	pfi = (PFIELDINFO) WinSendMsg(hwnd,
				      CM_QUERYDETAILFIELDINFO,
				      MPFROMP(pfi), MPFROMSHORT(CMA_NEXT));
      if (pfi && (INT) pfi != -1) {
	ced.pFieldInfo = pfi;
	{
	  CNRINFO cnri;

	  memset(&cnri, 0, sizeof(CNRINFO));
	  cnri.cb = sizeof(CNRINFO);
	  WinSendMsg(hwnd,
		     CM_QUERYCNRINFO,
		     MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
	  if (cnri.flWindowAttr & CV_DETAIL)
	    ced.id = CID_LEFTDVWND;
	}
	WinSendMsg(hwnd, CM_OPENEDIT, MPFROMP(&ced), MPVOID);
      }
    }
  }
}

#ifdef NEVER
VOID QuickView(HWND hwnd, CHAR * filename)
{
  if (filename && IsFile(filename) == 1) {
    if (TestBinary(filename) && *binview) {

      CHAR *list[2];

      list[0] = filename;
      list[1] = NULL;
      ExecOnList(hwnd, binview, WINDOWED | SEPARATE, NULL, list, NULL);
      return;
    }
    else if (*viewer) {

      CHAR *list[2];

      list[0] = filename;
      list[1] = NULL;
      ExecOnList(hwnd, viewer,
		 WINDOWED | SEPARATE | (fViewChild ? CHILD : 0),
		 NULL, list, NULL);
      return;
    }
    StartMLEEditor(HWND_DESKTOP, 5, filename, (HWND) 0);
  }
}

VOID QuickEdit(HWND hwnd, CHAR * filename)
{
  if (filename && IsFile(filename) == 1) {
    if (TestBinary(filename) && *bined) {

      CHAR *list[2];

      list[0] = filename;
      list[1] = NULL;
      ExecOnList(hwnd, bined, WINDOWED | SEPARATE, NULL, list, NULL);
      return;
    }
    else if (*editor) {

      CHAR *list[2];

      list[0] = filename;
      list[1] = NULL;
      ExecOnList(hwnd, editor, WINDOWED | SEPARATE, NULL, list, NULL);
      return;
    }
    StartMLEEditor(HWND_DESKTOP, 4, filename, (HWND) 0);
  }
}
#endif

VOID PortholeInit(HWND hwndNew, MPARAM mp1, MPARAM mp2)
{
  static HWND DefMenu = (HWND) 0;
  HWND hwndMenu = (HWND) mp2;

  {
    ULONG style;

    style = WinQueryWindowULong(hwndMenu, QWL_STYLE);
    if (!(style & MS_ACTIONBAR))
      return;
  }

  switch (SHORT1FROMMP(mp1)) {
  case 0:
    {
      HWND hwndNow;
      MENUITEM mi;
      ULONG ulStyle;

      memset(&mi, 0, sizeof(mi));
      mi.iPosition = MIT_END;
      mi.afStyle = MIS_TEXT;
      WinSendMsg(hwndMenu, MM_QUERYITEM,
		 MPFROM2SHORT(IDM_FILESMENU, TRUE), MPFROMP(&mi));
      if (!DefMenu)
	DefMenu = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, DEFMENU);
      hwndNow = mi.hwndSubMenu;
      mi.hwndSubMenu = hwndNew;
      if (!mi.hwndSubMenu)
	mi.hwndSubMenu = DefMenu;
      WinSetParent(hwndNow, WinQueryObjectWindow(HWND_DESKTOP), FALSE);
      WinSetOwner(hwndNow, WinQueryObjectWindow(HWND_DESKTOP));
      WinSetOwner(mi.hwndSubMenu, hwndMenu);
      WinSetParent(mi.hwndSubMenu, hwndMenu, FALSE);
      WinSetWindowUShort(mi.hwndSubMenu, QWS_ID, IDM_FILESMENU);
      CopyPresParams(mi.hwndSubMenu, hwndNow);
      mi.afStyle = MIS_SUBMENU;
      ulStyle = WinQueryWindowULong(mi.hwndSubMenu, QWL_STYLE);
      ulStyle &= -WS_SAVEBITS;
      ulStyle |= MS_POPUP | WS_CLIPSIBLINGS | WS_SAVEBITS;
      WinSetWindowULong(mi.hwndSubMenu, QWL_STYLE, ulStyle);
      WinSendMsg(hwndMenu, MM_SETITEM, MPFROM2SHORT(0, TRUE), MPFROMP(&mi));
    }
    break;

  case 1:
    {
      HWND hwndNow;
      MENUITEM mi;
      ULONG ulStyle;

      memset(&mi, 0, sizeof(mi));
      mi.iPosition = MIT_END;
      mi.afStyle = MIS_TEXT;
      WinSendMsg(hwndMenu, MM_QUERYITEM,
		 MPFROM2SHORT(IDM_VIEWSMENU, TRUE), MPFROMP(&mi));
      if (!DefMenu)
	DefMenu = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, DEFMENU);
      hwndNow = mi.hwndSubMenu;
      mi.hwndSubMenu = hwndNew;
      if (!mi.hwndSubMenu)
	mi.hwndSubMenu = DefMenu;
      WinSetParent(hwndNow, WinQueryObjectWindow(HWND_DESKTOP), FALSE);
      WinSetOwner(hwndNow, WinQueryObjectWindow(HWND_DESKTOP));
      WinSetOwner(mi.hwndSubMenu, hwndMenu);
      WinSetParent(mi.hwndSubMenu, hwndMenu, FALSE);
      WinSetWindowUShort(mi.hwndSubMenu, QWS_ID, IDM_VIEWSMENU);
      CopyPresParams(mi.hwndSubMenu, hwndNow);
      mi.afStyle = MIS_SUBMENU;
      ulStyle = WinQueryWindowULong(mi.hwndSubMenu, QWL_STYLE);
      ulStyle &= -WS_SAVEBITS;
      ulStyle |= MS_POPUP | WS_CLIPSIBLINGS | WS_SAVEBITS;
      WinSetWindowULong(mi.hwndSubMenu, QWL_STYLE, ulStyle);
      WinSendMsg(hwndMenu, MM_SETITEM, MPFROM2SHORT(0, TRUE), MPFROMP(&mi));
    }
    break;
  }
}

HWND CheckMenu(HWND hwnd, HWND * hwndMenu, USHORT id)
{
  /* load and adjust menus as required */
  if (!*hwndMenu || !WinIsWindow((HAB) 0, *hwndMenu)) {
    *hwndMenu = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, id);
    if (hwndMenu == &DirMenu) {
      WinSetWindowUShort(DirMenu, QWS_ID, IDM_FILESMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(DirMenu, IDM_COMMANDSMENU, IDM_DOITYOURSELF);
      SetConditionalCascade(DirMenu, IDM_COPYMENU, IDM_COPY);
      SetConditionalCascade(DirMenu, IDM_MOVEMENU, IDM_MOVE);
      SetConditionalCascade(DirMenu, IDM_SAVESUBMENU, IDM_SAVETOCLIP);
      SetConditionalCascade(DirMenu, IDM_VIEWSUBMENU, IDM_INFO);
      SetConditionalCascade(DirMenu, IDM_EDITSUBMENU, IDM_ATTRS);
      SetConditionalCascade(DirMenu, IDM_DELETESUBMENU,
			    fDefaultDeletePerm ? IDM_PERMDELETE : IDM_DELETE);
      SetConditionalCascade(DirMenu, IDM_MISCSUBMENU, IDM_SIZES);
      SetConditionalCascade(DirMenu, IDM_OPENSUBMENU, IDM_OPENWINDOW);
      if (fWorkPlace) {
	WinSendMsg(DirMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OPENSUBMENU, TRUE), MPVOID);
	WinSendMsg(DirMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OBJECTSUBMENU, TRUE), MPVOID);
      }
    }
    else if (hwndMenu == &TreeMenu) {
      WinSetWindowUShort(TreeMenu, QWS_ID, IDM_FILESMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(TreeMenu, IDM_COMMANDSMENU, IDM_DOITYOURSELF);
      SetConditionalCascade(TreeMenu, IDM_SAVESUBMENU, IDM_SAVETOCLIP);
      SetConditionalCascade(TreeMenu, IDM_EDITSUBMENU, IDM_ATTRS);
      SetConditionalCascade(TreeMenu, IDM_EXPANDSUBMENU, IDM_EXPAND);
      SetConditionalCascade(TreeMenu, IDM_MISCSUBMENU, IDM_SIZES);
      SetConditionalCascade(TreeMenu, IDM_OPENSUBMENU, IDM_OPENWINDOW);
      if (fWorkPlace) {
	WinSendMsg(TreeMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OPENSUBMENU, TRUE), MPVOID);
	WinSendMsg(TreeMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OBJECTSUBMENU, TRUE), MPVOID);
      }
    }
    else if (hwndMenu == &ArcMenu) {
      WinSetWindowUShort(ArcMenu, QWS_ID, IDM_FILESMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(ArcMenu, IDM_EXTRACTSUBMENU, IDM_EXTRACT);
      SetConditionalCascade(ArcMenu, IDM_EDITSUBMENU, IDM_EDIT);
      SetConditionalCascade(ArcMenu, IDM_VIEWSUBMENU, IDM_VIEW);
      if (fWorkPlace)
	WinSendMsg(ArcMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_FOLDERAFTEREXTRACT, TRUE), MPVOID);
    }
    else if (hwndMenu == &FileMenu) {
      WinSetWindowUShort(FileMenu, QWS_ID, IDM_FILESMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(FileMenu, IDM_COMMANDSMENU, IDM_DOITYOURSELF);
      SetConditionalCascade(FileMenu, IDM_COPYMENU, IDM_COPY);
      SetConditionalCascade(FileMenu, IDM_MOVEMENU, IDM_MOVE);
      SetConditionalCascade(FileMenu, IDM_SAVESUBMENU, IDM_SAVETOCLIP);
      SetConditionalCascade(FileMenu, IDM_VIEWSUBMENU, IDM_VIEW);
      SetConditionalCascade(FileMenu, IDM_EDITSUBMENU, IDM_EDIT);
      SetConditionalCascade(FileMenu, IDM_COLLECTMENU, IDM_COLLECT);
      SetConditionalCascade(FileMenu, IDM_DELETESUBMENU,
			    fDefaultDeletePerm ? IDM_PERMDELETE : IDM_DELETE);
      SetConditionalCascade(FileMenu, IDM_OPENSUBMENU, IDM_OPENDEFAULT);
      SetConditionalCascade(FileMenu, IDM_OBJECTSUBMENU, IDM_SHADOW);
      if (fWorkPlace) {
	WinSendMsg(FileMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OPENSUBMENU, TRUE), MPVOID);
	WinSendMsg(FileMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OBJECTSUBMENU, TRUE), MPVOID);
      }
    }
    else if (hwndMenu == &DirCnrMenu) {
      WinSetWindowUShort(DirCnrMenu, QWS_ID, IDM_VIEWSMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(DirCnrMenu, IDM_MISCSUBMENU, IDM_SIZES);
      SetConditionalCascade(DirCnrMenu, IDM_OPENSUBMENU, IDM_OPENSETTINGSME);
      if (fWorkPlace)
	WinSendMsg(DirCnrMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OPENSUBMENU, TRUE), MPVOID);
    }
    else if (hwndMenu == &TreeCnrMenu) {
      WinSetWindowUShort(TreeCnrMenu, QWS_ID, IDM_VIEWSMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(TreeCnrMenu, IDM_PARTITIONSMENU, IDM_PARTITION);
    }
    else if (hwndMenu == &ArcCnrMenu) {
      WinSetWindowUShort(ArcCnrMenu, QWS_ID, IDM_VIEWSMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(ArcCnrMenu, IDM_EXTRACTSUBMENU, IDM_ARCEXTRACT);
      if (fWorkPlace)
	WinSendMsg(ArcCnrMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_FOLDERAFTEREXTRACT, TRUE), MPVOID);
    }
    else if (hwndMenu == &CollectorCnrMenu) {
      WinSetWindowUShort(CollectorCnrMenu, QWS_ID, IDM_VIEWSMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(CollectorCnrMenu, IDM_COLLECTMENU,
			    IDM_COLLECTFROMCLIP);
    }
    else if (hwndMenu == &CollectorFileMenu) {
      WinSetWindowUShort(CollectorFileMenu, QWS_ID, IDM_FILESMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(CollectorFileMenu, IDM_COMMANDSMENU,
			    IDM_DOITYOURSELF);
      SetConditionalCascade(CollectorFileMenu, IDM_COPYMENU, IDM_COPY);
      SetConditionalCascade(CollectorFileMenu, IDM_MOVEMENU, IDM_MOVE);
      SetConditionalCascade(CollectorFileMenu, IDM_SAVESUBMENU,
			    IDM_SAVETOCLIP);
      SetConditionalCascade(CollectorFileMenu, IDM_VIEWSUBMENU, IDM_VIEW);
      SetConditionalCascade(CollectorFileMenu, IDM_EDITSUBMENU, IDM_EDIT);
      SetConditionalCascade(CollectorFileMenu, IDM_DELETESUBMENU,
			    fDefaultDeletePerm ? IDM_PERMDELETE : IDM_DELETE);
      SetConditionalCascade(CollectorFileMenu, IDM_OPENSUBMENU,
			    IDM_OPENDEFAULT);
      SetConditionalCascade(CollectorFileMenu, IDM_OBJECTSUBMENU, IDM_SHADOW);
      if (fWorkPlace) {
	WinSendMsg(CollectorFileMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OPENSUBMENU, TRUE), MPVOID);
	WinSendMsg(CollectorFileMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OBJECTSUBMENU, TRUE), MPVOID);
      }
    }
    else if (hwndMenu == &CollectorDirMenu) {
      WinSetWindowUShort(CollectorDirMenu, QWS_ID, IDM_FILESMENU);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(CollectorDirMenu, IDM_COMMANDSMENU,
			    IDM_DOITYOURSELF);
      SetConditionalCascade(CollectorDirMenu, IDM_COPYMENU, IDM_COPY);
      SetConditionalCascade(CollectorDirMenu, IDM_MOVEMENU, IDM_MOVE);
      SetConditionalCascade(CollectorDirMenu, IDM_SAVESUBMENU,
			    IDM_SAVETOCLIP);
      SetConditionalCascade(CollectorDirMenu, IDM_VIEWSUBMENU, IDM_INFO);
      SetConditionalCascade(CollectorDirMenu, IDM_EDITSUBMENU, IDM_ATTRS);
      SetConditionalCascade(CollectorDirMenu, IDM_DELETESUBMENU,
			    fDefaultDeletePerm ? IDM_PERMDELETE : IDM_DELETE);
      SetConditionalCascade(CollectorDirMenu, IDM_MISCSUBMENU, IDM_SIZES);
      SetConditionalCascade(CollectorDirMenu, IDM_OPENSUBMENU,
			    IDM_OPENWINDOW);
      if (fWorkPlace) {
	WinSendMsg(CollectorDirMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OPENSUBMENU, TRUE), MPVOID);
	WinSendMsg(CollectorDirMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OBJECTSUBMENU, TRUE), MPVOID);
      }
    }
    else if (hwndMenu == &MainPopupMenu) {
      WinSetWindowUShort(MainPopupMenu, QWS_ID, IDM_MAINPOPUP);
      CopyPresParams(*hwndMenu, hwnd);
      SetConditionalCascade(MainPopupMenu, IDM_TOOLSUBMENU, IDM_TOOLBAR);
      SetConditionalCascade(MainPopupMenu, IDM_AUTOVIEWSUBMENU, IDM_AUTOVIEW);
    }
  }
  return *hwndMenu;
}

SHORT AddToListboxBottom(HWND hwnd, CHAR * str)
{
  SHORT ln;

  ln = (SHORT) WinSendMsg(hwnd, LM_INSERTITEM, MPFROM2SHORT(LIT_END, 0),
			  MPFROMP(str));
  if (ln)
    WinSendMsg(hwnd, LM_SELECTITEM, MPFROM2SHORT(ln, 0), MPVOID);
  return ln;
}

VOID SetSysMenu(HWND hwndSysMenu)
{
  CHAR s[128], *p;

  if (WinSendMsg(hwndSysMenu, MM_QUERYITEMTEXT,
		 MPFROM2SHORT(SC_RESTORE, 128), MPFROMP(s))) {
    p = strchr(s, '\t');
    if (p) {
      p++;
      strcpy(p, "Ctrl+Alt+F5");
      WinSetMenuItemText(hwndSysMenu, SC_RESTORE, s);
    }
  }
  if (WinSendMsg(hwndSysMenu, MM_QUERYITEMTEXT,
		 MPFROM2SHORT(SC_CLOSE, 128), MPFROMP(s))) {
    p = strchr(s, '\t');
    if (p) {
      p++;
      strcpy(p, "Ctrl+Alt+F4");
      WinSetMenuItemText(hwndSysMenu, SC_CLOSE, s);
    }
  }
  if (WinSendMsg(hwndSysMenu, MM_QUERYITEMTEXT,
		 MPFROM2SHORT(SC_MOVE, 128), MPFROMP(s))) {
    p = strchr(s, '\t');
    if (p) {
      p++;
      strcpy(p, "Ctrl+Alt+F7");
      WinSetMenuItemText(hwndSysMenu, SC_MOVE, s);
    }
  }
  if (WinSendMsg(hwndSysMenu, MM_QUERYITEMTEXT,
		 MPFROM2SHORT(SC_SIZE, 128), MPFROMP(s))) {
    p = strchr(s, '\t');
    if (p) {
      p++;
      strcpy(p, "Ctrl+Alt+F8");
      WinSetMenuItemText(hwndSysMenu, SC_SIZE, s);
    }
  }
  if (WinSendMsg(hwndSysMenu, MM_QUERYITEMTEXT,
		 MPFROM2SHORT(SC_MINIMIZE, 128), MPFROMP(s))) {
    p = strchr(s, '\t');
    if (p) {
      p++;
      strcpy(p, "Ctrl+Alt+F9");
      WinSetMenuItemText(hwndSysMenu, SC_MINIMIZE, s);
    }
  }
  if (WinSendMsg(hwndSysMenu,
		 MM_QUERYITEMTEXT,
		 MPFROM2SHORT(SC_MAXIMIZE, 128), MPFROMP(s))) {
    p = strchr(s, '\t');
    if (p) {
      p++;
      strcpy(p, "Ctrl+Alt+F10");
      WinSetMenuItemText(hwndSysMenu, SC_MAXIMIZE, s);
    }
  }
  if (WinSendMsg(hwndSysMenu,
		 MM_QUERYITEMTEXT, MPFROM2SHORT(SC_HIDE, 128), MPFROMP(s))) {
    p = strchr(s, '\t');
    if (p) {
      p++;
      strcpy(p, "Ctrl+Alt+F11");
      WinSetMenuItemText(hwndSysMenu, SC_HIDE, s);
    }
  }
}

VOID LoadLibPath(CHAR * str, LONG len)
{
  ULONG ver[2];
  CHAR configsys[] = "C:\\CONFIG.SYS";
  static CHAR var[8192], beg[16384], end[16384];
  BOOL warp;
  FILE *fp;
  PFN DQELIBPATH = NULL;
  HMODULE hmod;

  if (str && len) {
    *str = 0;
    if (DosQuerySysInfo(QSV_BOOT_DRIVE,
			QSV_BOOT_DRIVE, (PVOID) ver, (ULONG) sizeof(ULONG)))
      ver[0] = 3L;
    *configsys = (CHAR) ver[0] + '@';
    if (!DosQuerySysInfo(QSV_VERSION_MAJOR,
			 QSV_VERSION_MINOR,
			 (PVOID) ver, (ULONG) sizeof(ver)) && ver[1] >= 30)
      warp = TRUE;
    *var = *beg = *end = 0;
    if (warp) {
      if (!DosLoadModule(var, sizeof(var), "DOSCALL1.DLL", &hmod)) {
	if (!DosQueryProcAddr(hmod,
			      ORD_DOS32QUERYEXTLIBPATH,
			      NULL, (PFN *) & DQELIBPATH)) {
	  DQELIBPATH(beg, BEGIN_LIBPATH);
	  DQELIBPATH(end, END_LIBPATH);
	}
	DosFreeModule(hmod);
      }
      *var = 0;
    }
    fp = xfopen(configsys, "r", pszSrcFile, __LINE__);
    if (fp) {
      while (!feof(fp)) {
	if (!xfgets_bstripcr(var, sizeof(var), fp, pszSrcFile, __LINE__))
	  break;
	if (!strnicmp(var, "LIBPATH=", 8)) {
	  memmove(var, var + 8, strlen(var + 8) + 1);
	  lstrip(var);
	  break;
	}
      }
      fclose(fp);
    }
    strncpy(str, beg, len);
    strncat(str, var, len - strlen(str));
    strncat(str, end, len - strlen(str));
    str[len - 1] = 0;
  }
}

void SetViewMenu(HWND hwndMenu, ULONG flWindowAttr)
{
  WinCheckMenuItem(hwndMenu, IDM_MINIICONS, ((flWindowAttr & CV_MINI)));
  WinCheckMenuItem(hwndMenu, IDM_TEXT, ((flWindowAttr & CV_TEXT)));
  WinCheckMenuItem(hwndMenu, IDM_ICON, ((flWindowAttr & CV_ICON) &&
					!(flWindowAttr & CV_TREE)));
  WinCheckMenuItem(hwndMenu, IDM_TREEVIEW, ((flWindowAttr & CV_TREE)));
  WinCheckMenuItem(hwndMenu, IDM_DETAILS, ((flWindowAttr & CV_DETAIL)));
  WinCheckMenuItem(hwndMenu, IDM_NAME, ((flWindowAttr & CV_NAME)));
}

void SaySort(HWND hwnd, INT sortflags, BOOL archive)
{
  char *s = NULL;

  s = xmalloc(CCHMAXPATH, pszSrcFile, __LINE__);
  if (s) {
    sprintf(s, "S:%s%s",
	    sortflags & SORT_REVERSE ? "^" : NullStr,
	    (sortflags & SORT_FIRSTEXTENSION) ?
	      GetPString(IDS_FIRSTX) : (sortflags & SORT_LASTEXTENSION) ?
		GetPString(IDS_LASTX) : (sortflags & SORT_SIZE) ?
		  "Size" : (sortflags & SORT_EASIZE) ?
		    (archive == 0) ?
		      GetPString(IDS_EASIZE) : GetPString(IDS_CSIZE) :
		    (sortflags & SORT_LWDATE) ?
		      (archive == 0) ?
			GetPString(IDS_LWDATE) : GetPString(IDS_DATE) :
			(sortflags & SORT_LADATE) ?
			  GetPString(IDS_LADATE) : (sortflags & SORT_CRDATE) ?
			    GetPString(IDS_CRDATE) :
			    (sortflags & SORT_PATHNAME) ?
			      GetPString(IDS_PATH) : (sortflags & SORT_NOSORT) ?
				GetPString(IDS_NONE) : (sortflags & SORT_SUBJECT) ?
				  GetPString(IDS_SUBJ) : GetPString(IDS_NAME));
    WinSetWindowText(hwnd, s);
    free(s);
  }
}

void SayView(HWND hwnd, ULONG flWindowAttr)
{
  char *s = NULL;

  s = xmalloc(CCHMAXPATH, pszSrcFile, __LINE__);
  if (s) {
    sprintf(s, "V:%s%s",
	    (flWindowAttr & CV_TREE) ? GetPString(IDS_TREE) :
	    (flWindowAttr & CV_NAME) ? GetPString(IDS_NAME) :
	    (flWindowAttr & CV_DETAIL) ? GetPString(IDS_DETAIL) :
	    (flWindowAttr & CV_TEXT) ? GetPString(IDS_TEXT) :
	    GetPString(IDS_ICON),
	    ((flWindowAttr & CV_MINI) &&
	     !(flWindowAttr & CV_TEXT)) ? GetPString(IDS_MINI) : NullStr);
    WinSetWindowText(hwnd, s);
    free(s);
  }
}

void SayFilter(HWND hwnd, MASK * mask, BOOL archive)
{
  char *s = NULL;

  s = xmalloc(CCHMAXPATH * 2, pszSrcFile, __LINE__);
  if (s) {
    sprintf(s, "F:%s%s",
	    mask->szMask,
	    (!archive && (mask->attrFile != ALLATTRS ||
			  mask->antiattr != 0)) ? " " : NullStr,
	    (!archive && (mask->attrFile != ALLATTRS ||
			  mask->antiattr !=
			  0)) ? GetPString(IDS_ATTRTEXT) : NullStr);
    if (!s[2])
      sprintf(s, "F:%s", GetPString(IDS_ALLTEXT));
    WinSetWindowText(hwnd, s);
    free(s);
  }
}

char *GetCmdSpec(BOOL dos)
{
  char *cmspec;

  if (!dos) {
    cmspec = getenv("OS2_SHELL");
    if (!cmspec)
      cmspec = getenv("COMSPEC");
    if (!cmspec)
      cmspec = "CMD.EXE";
  }
  else {
    cmspec = getenv("DOS_SHELL");
    if (!cmspec)
      cmspec = "COMMAND.COM";
  }
  return cmspec;
}

void Broadcast(HAB hab, HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  if (hwndMain)
    WinBroadcastMsg(hwndMain, msg, mp1, mp2, BMSG_SEND | BMSG_FRAMEONLY);
  if (hwnd &&
      hwnd != HWND_DESKTOP &&
      hwnd != hwndMain &&
      hwnd != WinQueryDesktopWindow(hab, NULLHANDLE) &&
      WinIsWindow(hab, hwnd) && (!hwndMain || !WinIsChild(hwnd, hwndMain)))
    WinSendMsg(hwnd, msg, mp1, mp2);
}

void SetupWinList(HWND hwndMenu, HWND hwndTop, HWND hwndFrame)
{
  /*
   * add switchlist entries to end of pulldown menu
   */

  SHORT sItemCount, x = 0, y = 0;
  MENUITEM mi;

  sItemCount = (SHORT) WinSendMsg(hwndMenu,
				  MM_QUERYITEMCOUNT, MPVOID, MPVOID);

  /* clean out old additions */
  while ((SHORT) WinSendMsg(hwndMenu,
			    MM_DELETEITEM,
			    MPFROM2SHORT(IDM_SWITCHSTART + x++,
					 TRUE), MPVOID) < sItemCount)
    sItemCount--;
  x = 0;
  while ((SHORT) WinSendMsg(hwndMenu,
			    MM_DELETEITEM,
			    MPFROM2SHORT(IDM_WINDOWSTART + x++,
					 TRUE), MPVOID) < sItemCount)
    sItemCount--;

  x = 0;
  if (hwndTop) {

    char wtext[CCHMAXPATH + 8];
    HENUM henum;
    HWND hwndChild;

    /* add children of the main FM/2 client */
    henum = WinBeginEnumWindows(hwndTop);
    memset(&mi, 0, sizeof(mi));
    while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
      if (WinQueryWindowUShort(hwndChild, QWS_ID) && hwndChild != hwndFrame) {
	*wtext = 0;
	WinQueryWindowText(hwndChild, CCHMAXPATH + 8, wtext);
	if (*wtext) {
	  wtext[CCHMAXPATH + 7] = 0;
	  mi.afStyle = MIS_TEXT;
	  if (!((x + sItemCount) % 28))
	    mi.afStyle |= MIS_BREAK;
	  mi.id = IDM_WINDOWSTART + x;
	  mi.iPosition = MIT_END;
	  if ((SHORT) WinSendMsg(hwndMenu,
				 MM_INSERTITEM,
				 MPFROMP(&mi), MPFROMP(wtext)) >= 0)
	    x++;
	}
      }
    }
    WinEndEnumWindows(henum);
  }

  /* add external FM/2 windows */
  {
    PSWBLOCK pswb;
    ULONG ulSize, ulcEntries;
    HWND hwndTopFrame;
    register INT i;

    hwndTopFrame = hwndTop ? WinQueryWindow(hwndTop, QW_PARENT) : (HWND)0;
    /* Get the switch list information */
    x = 0;
    ulcEntries = WinQuerySwitchList(0, NULL, 0);
    ulSize = sizeof(SWBLOCK) + sizeof(HSWITCH) + (ulcEntries + 4L) *
      (LONG) sizeof(SWENTRY);
    /* Allocate memory for list */
    pswb = xmalloc(ulSize, pszSrcFile, __LINE__);
    if (pswb) {
      /* Put the info in the list */
      ulcEntries = WinQuerySwitchList(0, pswb, ulSize - sizeof(SWENTRY));
      /* do the dirty deed */
      memset(&mi, 0, sizeof(mi));
      for (i = 0; i < pswb->cswentry; i++) {
	if (pswb->aswentry[i].swctl.uchVisibility == SWL_VISIBLE &&
	    pswb->aswentry[i].swctl.fbJump == SWL_JUMPABLE &&
	    (pswb->aswentry[i].swctl.idProcess != mypid ||
	     !hwndFrame ||
	     pswb->aswentry[i].swctl.hwnd != hwndFrame) &&
	    (pswb->aswentry[i].swctl.idProcess != mypid ||
	     !hwndTopFrame ||
	     pswb->aswentry[i].swctl.hwnd != hwndTopFrame ||
	     !WinIsChild(hwndFrame, hwndTop))) {
	  if (!strnicmp(pswb->aswentry[i].swctl.szSwtitle, "AV/2", 4)
	      || !stricmp(pswb->aswentry[i].swctl.szSwtitle, "File Manager/2")
	      || !stricmp(pswb->aswentry[i].swctl.szSwtitle, "Collector")
	      || !strnicmp(pswb->aswentry[i].swctl.szSwtitle, "VTree", 5)
	      || !strnicmp(pswb->aswentry[i].swctl.szSwtitle, "VDir", 4)
	      || !strnicmp(pswb->aswentry[i].swctl.szSwtitle, FM2Str, 4)) {
	    mi.afStyle = MIS_TEXT;
	    if (x && !(x % 28))
	      mi.afStyle |= MIS_BREAK;
	    mi.id = IDM_SWITCHSTART + y;
	    mi.iPosition = MIT_END;
	    switches[y] = pswb->aswentry[i].hswitch;
	    if ((SHORT) WinSendMsg(hwndMenu,
				   MM_INSERTITEM,
				   MPFROMP(&mi),
				   MPFROMP(pswb->aswentry[i].
					   swctl.szSwtitle)) >= 0) {
	      y++;
	      x++;
	    }
	  }
	}
      }
      numswitches = y;
      free(pswb);
      DosPostEventSem(CompactSem);
    }
  }
}

BOOL SwitchCommand(HWND hwndMenu, USHORT cmd)
{
  BOOL ret = FALSE;

  if (hwndMain && hwndMenu && cmd >= IDM_WINDOWSTART && cmd < IDM_SWITCHSTART) {
    /*
     * select a child window (of client)
     */

    MENUITEM mi;
    HWND hwndSubMenu = (HWND) 0, hwndChild;
    CHAR s[CCHMAXPATH + 8];

    if (WinQueryWindowUShort(hwndMenu, QWS_ID) != IDM_WINDOWSMENU) {
      memset(&mi, 0, sizeof(mi));
      mi.iPosition = MIT_END;
      mi.afStyle = MIS_TEXT;
      if (WinSendMsg(hwndMenu,
		     MM_QUERYITEM,
		     MPFROM2SHORT(IDM_WINDOWSMENU, TRUE), MPFROMP(&mi)))
	hwndSubMenu = mi.hwndSubMenu;
    }
    else
      hwndSubMenu = hwndMenu;
    if (hwndSubMenu) {
      *s = 0;
      if (WinSendMsg(hwndSubMenu,
		     MM_QUERYITEMTEXT,
		     MPFROM2SHORT(cmd, CCHMAXPATH + 8), MPFROMP(s)) && *s) {

	HENUM henum;
	CHAR checkText[CCHMAXPATH + 8];
	SWP swp;

	s[CCHMAXPATH + 7] = 0;
	henum = WinBeginEnumWindows(hwndMain);
	while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
	  if (WinQueryWindowUShort(hwndChild, QWS_ID)) {
	    *checkText = 0;
	    WinQueryWindowText(hwndChild, CCHMAXPATH + 8, checkText);
	    checkText[CCHMAXPATH + 7] = 0;
	    if (!stricmp(checkText, s)) {
	      if (WinQueryWindowPos(hwndChild, &swp)) {
		if (swp.fl & (SWP_MINIMIZE | SWP_HIDE))
		  WinSetWindowPos(hwndChild,
				  HWND_TOP,
				  0, 0, 0, 0, SWP_RESTORE | SWP_ZORDER);
	      }
	      WinSetActiveWindow(HWND_DESKTOP, hwndChild);
	      ret = TRUE;
	      break;
	    }
	  }
	}
	WinEndEnumWindows(henum);
      }
    }
  }
  else if (cmd >= IDM_SWITCHSTART && cmd < IDM_SWITCHSTART + 499) {
    if (cmd - IDM_SWITCHSTART < numswitches) {
      WinSwitchToProgram(switches[cmd - IDM_SWITCHSTART]);
      ret = TRUE;
    }
  }

  return ret;
}

#pragma alloc_text(MAINWND5,SetSysMenu)
#pragma alloc_text(MISC1,BoxWindow,PaintRecessedWindow,PostMsg,PaintSTextWindow,IsFm2Window)
#pragma alloc_text(MISC1,FixSwitchList,FindDirCnr,CurrentRecord,SetShiftState,AddToListboxBottom)
#pragma alloc_text(CNR_MISC1,AdjustCnrColVis,AdjustCnrColsForFSType)
#pragma alloc_text(CNR_MISC1,AdjustCnrColsForPref,SetCnrCols)
#pragma alloc_text(CNR_MISC2,CnrDirectEdit,OpenEdit)
#pragma alloc_text(MISC2,SetMenuCheck,disable_menuitem,SetSortChecks)
#pragma alloc_text(MISC2,SetDetailsSwitches,SetViewMenu)
#pragma alloc_text(MISC3,SetupCommandMenu,AdjustDetailsSwitches)
#pragma alloc_text(MISC3,ViewHelp,GetCmdSpec)
#pragma alloc_text(MISC3,ExecFile,SetConditionalCascade,LoadDetailsSwitches)
#pragma alloc_text(MISC4,PortholeInit,CheckMenu,Broadcast,SetupWinList,SwitchCommand)
#pragma alloc_text(MISC6,DrawTargetEmphasis,EmphasizeButton)
#pragma alloc_text(MISC_LIBPATH,LoadLibPath)
#pragma alloc_text(MISC_SAY,SayView,SaySort,SayFilter)

