
/***********************************************************************

  $Id$

  Collector

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2005 Steven H. Levine

  15 Oct 02 MK Baseline
  10 Jan 04 SHL Avoid -1L byte counts
  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  24 May 05 SHL Rework Win_Error usage
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  25 May 05 SHL Rework for FillInRecordFromFFB
  05 Jun 05 SHL Use QWL_USER
  06 Jun 05 SHL Indent -i2
  06 Jun 05 SHL Make savedSortFlags static to avoid referencing garbage
  24 Oct 05 SHL Sanitize handle references
  24 Oct 05 SHL CollectorCnrWndProc: avoid excess writes to Status2 window
  10 Nov 05 SHL CollectorCnrWndProc: correct missing button window updates

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSERRORS
#define INCL_LONGLONG
#include <os2.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <share.h>
#include <limits.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "grep.h"

#pragma data_seg(DATA1)
#pragma alloc_text(COLLECTOR,CollectorCnrWndProc,CollectorObjWndProc)
#pragma alloc_text(COLLECTOR,CollectorClientWndProc,CollectorTextProc)
#pragma alloc_text(COLLECTOR,CollectorFrameWndProc)
#pragma alloc_text(STARTUP,StartCollector)

MRESULT EXPENTRY CollectorFrameWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				       MPARAM mp2)
{
  return CommonFrameWndProc(COLLECTOR_CNR, hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CollectorTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *dcd;

  static BOOL emphasized = FALSE;
  static HWND hwndButtonPopup = (HWND) 0;
  static ULONG timestamp = ULONG_MAX;
  static USHORT lastid = 0;

  switch (msg)
  {
  case WM_CREATE:
    return CommonTextProc(hwnd, msg, mp1, mp2);

  case UM_CONTEXTMENU:
  case WM_CONTEXTMENU:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id)
      {
      case DIR_SELECTED:
      case DIR_VIEW:
      case DIR_SORT:
	{
	  POINTL ptl = {0, 0};
	  SWP swp;

	  if (hwndButtonPopup)
	    WinDestroyWindow(hwndButtonPopup);
	  if (id == lastid)
	  {
	    ULONG check;

	    DosQuerySysInfo(QSV_MS_COUNT,
			    QSV_MS_COUNT,
			    &check,
			    sizeof(check));
	    if (check < timestamp + 500)
	    {
	      lastid = 0;
	      goto MenuAbort;
	    }
	  }
	  hwndButtonPopup = WinLoadMenu(HWND_DESKTOP,
					FM3ModHandle,
					id);
	  if (hwndButtonPopup)
	  {
	    WinSetWindowUShort(hwndButtonPopup,
			       QWS_ID,
			       id);
	    dcd = WinQueryWindowPtr(WinWindowFromID(WinQueryWindow(hwnd,
								 QW_PARENT),
						    COLLECTOR_CNR),
				    QWL_USER);
	    if (id == DIR_VIEW)
	    {
	      if (dcd)
	      {
		SetViewMenu(hwndButtonPopup,
			    dcd -> flWindowAttr);
		SetDetailsSwitches(hwndButtonPopup,
				   dcd);
	      }

	      /* don't have tree view in collector */
	      WinSendMsg(hwndButtonPopup,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_TREEVIEW,
				      FALSE),
			 MPVOID);

	    }
	    else if (id == DIR_SORT)
	    {
	      if (dcd)
		SetSortChecks(hwndButtonPopup,
			      dcd -> sortFlags);
	    }
	    ptl.x = 0;
	    if (WinPopupMenu(HWND_OBJECT,
			     HWND_OBJECT,
			     hwndButtonPopup,
			     -32767,
			     -32767,
			     0,
			     0))
	    {
	      WinQueryWindowPos(hwndButtonPopup,
				&swp);
	      ptl.y = -(swp.cy + 2);
	    }
	    else
	    {
	      WinQueryWindowPos(hwnd,
				&swp);
	      ptl.y = swp.cy + 2;
	    }
	    if (WinPopupMenu(hwnd,
			     hwnd,
			     hwndButtonPopup,
			     ptl.x,
			     ptl.y,
			     0,
			     PU_HCONSTRAIN | PU_VCONSTRAIN |
			     PU_KEYBOARD | PU_MOUSEBUTTON1))
	    {
	      CenterOverWindow(hwndButtonPopup);
	      PaintRecessedWindow(hwnd,
				  NULLHANDLE,
				  FALSE,
				  FALSE);
	    }
	  }
	}
	break;
      default:
	PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				COLLECTOR_CNR),
		WM_CONTROL,
		MPFROM2SHORT(COLLECTOR_CNR,
			     CN_CONTEXTMENU),
		MPVOID);
	break;
      }
    }
  MenuAbort:
    if (msg == UM_CONTEXTMENU)
      return 0;
    break;

  case WM_MENUEND:
    if (hwndButtonPopup == (HWND) mp2)
    {
      lastid = WinQueryWindowUShort((HWND) mp2, QWS_ID);
      WinDestroyWindow(hwndButtonPopup);
      hwndButtonPopup = (HWND) 0;
      DosQuerySysInfo(QSV_MS_COUNT,
		      QSV_MS_COUNT,
		      &timestamp,
		      sizeof(timestamp));
      switch (lastid)
      {
      case DIR_SELECTED:
      case DIR_VIEW:
      case DIR_SORT:
	PaintRecessedWindow(hwnd,
			    NULLHANDLE,
			    TRUE,
			    FALSE);
	break;
      }
    }
    break;

  case WM_COMMAND:
    {
      DIRCNRDATA *dcd;
      MRESULT mr;

      mr = WinSendMsg(WinWindowFromID(WinQueryWindow(hwnd,
						     QW_PARENT),
				      COLLECTOR_CNR),
		      msg,
		      mp1,
		      mp2);
      if (hwndButtonPopup &&
	  SHORT1FROMMP(mp1) > IDM_DETAILSTITLES &&
	  SHORT1FROMMP(mp1) < IDM_DETAILSSETUP)
      {
	dcd = WinQueryWindowPtr(WinWindowFromID(WinQueryWindow(hwnd,
							       QW_PARENT),
						COLLECTOR_CNR),
				QWL_USER);
	if (dcd)
	  SetDetailsSwitches(hwndButtonPopup,
			     dcd);
      }
      return mr;
    }

  case WM_MOUSEMOVE:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);
      char *s = NULL;

      if (fOtherHelp)
      {
	if ((!hwndBubble ||
	     WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd) &&
	    !WinQueryCapture(HWND_DESKTOP))
	{
	  switch (id)
	  {
	  case DIR_SELECTED:
	    s = GetPString(IDS_COLSELECTEDHELP);
	    break;
	  case DIR_TOTALS:
	    s = GetPString(IDS_COLTOTALSHELP);
	    break;
	  case DIR_VIEW:
	    s = GetPString(IDS_DIRCNRVIEWHELP);
	    break;
	  case DIR_SORT:
	    s = GetPString(IDS_DIRCNRSORTHELP);
	    break;
	  case DIR_FILTER:
	    s = GetPString(IDS_DIRCNRFILTERHELP);
	    break;
	  default:
	    break;
	  }
	  if (s)
	    MakeBubble(hwnd, TRUE, s);
	  else if (hwndBubble)
	    WinDestroyWindow(hwndBubble);
	}
      }
      switch (id)
      {
      case DIR_FILTER:
      case DIR_SORT:
      case DIR_VIEW:
      case DIR_SELECTED:
	return CommonTextButton(hwnd, msg, mp1, mp2);
      }
    }
    break;

  case WM_BUTTON3UP:
  case WM_BUTTON1UP:
  case WM_BUTTON3DOWN:
  case WM_BUTTON1DOWN:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id)
      {
      case DIR_FILTER:
      case DIR_SORT:
      case DIR_VIEW:
      case DIR_SELECTED:
	return CommonTextButton(hwnd, msg, mp1, mp2);
      }
    }
    break;

  case UM_CLICKED:
  case UM_CLICKED3:
    {
      USHORT id, cmd = 0;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id)
      {
      case DIR_VIEW:
      case DIR_SORT:
      case DIR_SELECTED:
	PostMsg(hwnd,
		UM_CONTEXTMENU,
		MPVOID,
		MPVOID);
	break;
      case DIR_FILTER:
	cmd = IDM_FILTER;
	break;
      default:
	break;
      }
      if (cmd)
	PostMsg(WinWindowFromID(WinQueryWindow(hwnd,
					       QW_PARENT),
				COLLECTOR_CNR),
		WM_COMMAND,
		MPFROM2SHORT(cmd, 0),
		MPVOID);
    }
    return 0;

  case DM_DROP:
  case DM_DRAGOVER:
  case DM_DRAGLEAVE:
  case DM_DROPHELP:
    if (msg == DM_DRAGOVER)
    {
      if (!emphasized)
      {
	emphasized = TRUE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
    }
    else
    {
      if (emphasized)
      {
	emphasized = FALSE;
	DrawTargetEmphasis(hwnd, emphasized);
      }
    }
    {
      CNRDRAGINFO cnd;
      USHORT dcmd;

      switch (msg)
      {
      case DM_DROP:
	dcmd = CN_DROP;
	break;
      case DM_DRAGOVER:
	dcmd = CN_DRAGOVER;
	break;
      case DM_DRAGLEAVE:
	dcmd = CN_DRAGLEAVE;
	break;
      case DM_DROPHELP:
	dcmd = CN_DROPHELP;
	break;
      }
      memset(&cnd, 0, sizeof(cnd));
      cnd.pDragInfo = (PDRAGINFO) mp1;
      cnd.pRecord = NULL;
      return WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), WM_CONTROL,
			MPFROM2SHORT(COLLECTOR_CNR, dcmd), MPFROMP(&cnd));
    }
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CollectorClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
					MPARAM mp2)
{
  switch (msg)
  {
  case UM_CONTAINERHWND:
    return MRFROMLONG(WinWindowFromID(hwnd, COLLECTOR_CNR));

  case UM_VIEWSMENU:
    return MRFROMLONG(CheckMenu(&CollectorCnrMenu, COLLECTORCNR_POPUP));

  case MM_PORTHOLEINIT:
  case WM_INITMENU:
  case UM_INITMENU:
  case UM_CONTAINER_FILLED:
  case UM_FILESMENU:
  case UM_UPDATERECORD:
  case UM_UPDATERECORDLIST:
    return WinSendMsg(WinWindowFromID(hwnd, COLLECTOR_CNR), msg, mp1, mp2);

  case WM_PSETFOCUS:
  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, COLLECTOR_CNR));
    break;

  case WM_PAINT:
    {
      HPS hps;
      RECTL rcl;

      hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
      if (hps)
      {
	WinQueryWindowRect(hwnd, &rcl);
	WinFillRect(hps, &rcl, CLR_PALEGRAY);
	CommonTextPaint(hwnd, hps);
	WinEndPaint(hps);
      }
    }
    break;

  case UM_SIZE:
  case WM_SIZE:
    if (msg == UM_SIZE)
    {
      SWP swp;

      WinQueryWindowPos(hwnd, &swp);
      mp1 = MPFROM2SHORT(swp.cx, swp.cy);
      mp2 = MPFROM2SHORT(swp.cx, swp.cy);
    }
    {
      USHORT cx, cy, bx;

      cx = SHORT1FROMMP(mp2);
      cy = SHORT2FROMMP(mp2);
      WinSetWindowPos(WinWindowFromID(hwnd, COLLECTOR_CNR), HWND_TOP,
		      0,
		      0,
		      cx,
		      cy - 24,
		      SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_TOTALS), HWND_TOP,
		      2,
		      cy - 22,
		      (cx / 3) - 2,
		      20,
		      SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_SELECTED), HWND_TOP,
		      2 + (cx / 3) + 2,
		      cy - 22,
		      (cx / 3) - 2,
		      20,
		      SWP_SHOW | SWP_MOVE | SWP_SIZE);
      bx = (cx - (2 + (((cx / 3) + 2) * 2))) / 3;
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_VIEW), HWND_TOP,
		      2 + (((cx / 3) + 2) * 2),
		      cy - 22,
		      bx - 4,
		      20,
		      SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_SORT), HWND_TOP,
		      2 + (((cx / 3) + 2) * 2) + bx,
		      cy - 22,
		      bx - 4,
		      20,
		      SWP_SHOW | SWP_MOVE | SWP_SIZE);
      WinSetWindowPos(WinWindowFromID(hwnd, DIR_FILTER), HWND_TOP,
		      2 + (((cx / 3) + 2) * 2) + (bx * 2),
		      cy - 22,
		      bx - 4,
		      20,
		      SWP_SHOW | SWP_MOVE | SWP_SIZE);
    }
    CommonTextPaint(hwnd, NULLHANDLE);
    if (msg == UM_SIZE)
    {
      WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), HWND_TOP, 0, 0, 0, 0,
		      SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE);
      return 0;
    }
    break;

  case UM_COMMAND:
  case WM_COMMAND:
  case WM_CONTROL:
  case WM_CLOSE:
    return WinSendMsg(WinWindowFromID(hwnd, COLLECTOR_CNR), msg, mp1, mp2);
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CollectorObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				     MPARAM mp2)
{
  ULONG size;
  DIRCNRDATA *dcd;

  switch (msg)
  {
  case WM_CREATE:
    break;

  case DM_PRINTOBJECT:
    return MRFROMLONG(DRR_TARGET);

  case DM_DISCARDOBJECT:
    dcd = INSTDATA(hwnd);
    if (fFM2Deletes && dcd)
    {
      LISTINFO *li;
      CNRDRAGINFO cni;

      cni.pRecord = NULL;
      cni.pDragInfo = (PDRAGINFO) mp1;
      li = DoFileDrop(dcd -> hwndCnr, NULL, FALSE, MPVOID, MPFROMP(&cni));
      if (li)
      {
	li -> type = (fDefaultDeletePerm) ? IDM_PERMDELETE : IDM_DELETE;
	if (!PostMsg(hwnd, UM_MASSACTION, MPFROMP(li), MPVOID))
	  FreeListInfo(li);
	else
	  return MRFROMLONG(DRR_SOURCE);
      }
    }
    return MRFROMLONG(DRR_TARGET);

  case UM_UPDATERECORDLIST:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd && mp1)
    {
      INT numentries = 0;
      CHAR **list = (CHAR **) mp1;

      while (list[numentries])
	numentries++;
      if (numentries)
	UpdateCnrList(dcd -> hwndCnr, list, numentries, FALSE, dcd);
    }
    return 0;

  case UM_SETUP:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd)
    {
      /* set unique id */
      WinSetWindowUShort(hwnd,
			 QWS_ID,
			 COLLECTOROBJ_FRAME + (COLLECTOR_FRAME - dcd -> id));
      dcd -> hwndObject = hwnd;
      if (ParentIsDesktop(hwnd, dcd -> hwndParent))
	DosSleep(250L);
    }
    else
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    return 0;

  case UM_COMMAND:
    if (mp1)
    {
      LISTINFO *li = (LISTINFO *) mp1;

      switch (li -> type)
      {
      case IDM_DOITYOURSELF:
      case IDM_APPENDTOCLIP:
      case IDM_SAVETOCLIP:
      case IDM_ARCHIVE:
      case IDM_ARCHIVEM:
      case IDM_VIEW:
      case IDM_VIEWTEXT:
      case IDM_VIEWBINARY:
      case IDM_VIEWARCHIVE:
      case IDM_EDIT:
      case IDM_EDITTEXT:
      case IDM_EDITBINARY:
      case IDM_OBJECT:
      case IDM_SHADOW:
      case IDM_SHADOW2:
      case IDM_PRINT:
      case IDM_ATTRS:
      case IDM_DELETE:
      case IDM_PERMDELETE:
      case IDM_FAKEEXTRACT:
      case IDM_FAKEEXTRACTM:
      case IDM_MCIPLAY:
      case IDM_UPDATE:
	if (PostMsg(hwnd, UM_MASSACTION, mp1, mp2))
	  return (MRESULT) TRUE;
	break;
      default:
	if (PostMsg(hwnd, UM_ACTION, mp1, mp2))
	  return (MRESULT) TRUE;
      }
    }
    return 0;

  case UM_COLLECT:
    DosError(FERR_DISABLEHARDERR);
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd)
    {
      LISTINFO *li = (LISTINFO *) mp1;
      INT x;
      FILEFINDBUF4 fb4;
      HDIR hdir;
      ULONG nm;
      PCNRITEM pci, pciFirst, pciT, pciP = NULL;
      RECORDINSERT ri;
      ULONG ulMaxFiles;
      ULONGLONG ullTotalBytes;
      CHAR fullname[CCHMAXPATH];

      WinSetWindowText(WinWindowFromID(dcd -> hwndClient,
				       DIR_SELECTED),
		       GetPString(IDS_COLLECTINGTEXT));
      for (x = 0; li -> list[x]; x++)
      {
	;
      }
      ulMaxFiles = x;
      if (ulMaxFiles)
      {
	pci = WinSendMsg(dcd -> hwndCnr, CM_ALLOCRECORD,
			 MPFROMLONG(EXTRA_RECORD_BYTES),
			 MPFROMLONG(ulMaxFiles));
	if (pci)
	{
	  pciFirst = pci;
	  for (x = 0; li -> list[x]; x++)
	  {
	    nm = 1L;
	    hdir = HDIR_CREATE;
	    DosError(FERR_DISABLEHARDERR);
	    if (*li -> list[x] &&
		!DosQueryPathInfo(li -> list[x], FIL_QUERYFULLNAME,
				  fullname, sizeof(fullname)) &&
		!IsRoot(fullname) &&
		!FindCnrRecord(dcd -> hwndCnr,
			       fullname,
			       NULL,
			       FALSE,
			       FALSE,
			       TRUE) &&
		!DosFindFirst(fullname,
			      &hdir,
			      FILE_NORMAL | FILE_DIRECTORY |
			      FILE_ARCHIVED | FILE_SYSTEM |
			      FILE_HIDDEN | FILE_READONLY,
			      &fb4,
			      sizeof(fb4),
			      &nm,
			      FIL_QUERYEASIZE))
	    {
	      DosFindClose(hdir);
	      priority_normal();
	      *fb4.achName = 0;
	      ullTotalBytes = FillInRecordFromFFB(dcd -> hwndCnr,
						  pci,
						  fullname,
						  &fb4,
						  FALSE,
						  dcd);
	      dcd -> ullTotalBytes += ullTotalBytes;
	      pciP = pci;
	      pci = (PCNRITEM) pci -> rc.preccNextRecord;
	    }
	    else
	    {
	      pciT = pci;
	      pci = (PCNRITEM) pci -> rc.preccNextRecord;
	      if (pciP)
		pciP -> rc.preccNextRecord = (PMINIRECORDCORE) pci;
	      else
		pciFirst = pci;
	      WinSendMsg(hwnd, CM_FREERECORD, MPFROMP(&pciT),
			 MPFROM2SHORT(1, 0));
	      ulMaxFiles--;
	    }
	    DosSleep(1L);
	  }
	  if (ulMaxFiles)
	  {
	    memset(&ri, 0, sizeof(RECORDINSERT));
	    ri.cb = sizeof(RECORDINSERT);
	    ri.pRecordOrder = (PRECORDCORE) CMA_END;
	    ri.pRecordParent = (PRECORDCORE) 0;
	    ri.zOrder = (ULONG) CMA_TOP;
	    ri.cRecordsInsert = ulMaxFiles;
	    ri.fInvalidateRecord = TRUE;
	    WinSendMsg(dcd -> hwndCnr,
		       CM_INSERTRECORD,
		       MPFROMP(pciFirst),
		       MPFROMP(&ri));
	    PostMsg(dcd -> hwndCnr,
		    UM_RESCAN,
		    MPVOID,
		    MPVOID);
	  }
	}
      }
    }
    if (dcd -> flWindowAttr & CV_DETAIL)
      WinSendDlgItemMsg(hwnd,
			COLLECTOR_CNR,
			CM_INVALIDATERECORD,
			MPVOID,
			MPFROM2SHORT(0,
				     CMA_ERASE | CMA_REPOSITION));
    return 0;

  case UM_COLLECTFROMFILE:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd && mp1)
    {
      FILESTATUS4 fs4;
      PCNRITEM pci;
      RECORDINSERT ri;
      CHAR fullname[1024], *p;
      FILE *fp;
      ULONG errs = 0L;

      fp = _fsopen((CHAR *) mp1, "r", SH_DENYNO);
      if (fp)
      {
	while (!feof(fp))
	{
	  if (!fgets(fullname, 1024, fp))
	    break;
	  fullname[1023] = 0;
	  bstripcr(fullname);
	  if (*fullname == '\"')
	  {
	    memmove(fullname, fullname + 1, strlen(fullname) + 1);
	    lstrip(fullname);
	    p = strchr(fullname, '\"');
	    if (p)
	      *p = 0;
	    rstrip(fullname);
	  }
	  else
	  {
	    p = strchr(fullname, ' ');
	    if (p)
	      *p = 0;
	  }
	  /* fullname now contains name of file to collect */
	  DosError(FERR_DISABLEHARDERR);
	  if (IsFullName(fullname) &&
	      !IsRoot(fullname) &&
	      !DosQueryPathInfo(fullname,
				FIL_QUERYEASIZE,
				&fs4,
				sizeof(fs4)) &&
	      !FindCnrRecord(dcd -> hwndCnr,
			     fullname,
			     NULL,
			     FALSE,
			     FALSE,
			     TRUE))
	  {
	    /* collect it */
	    pci = WinSendMsg(dcd -> hwndCnr,
			     CM_ALLOCRECORD,
			     MPFROMLONG(EXTRA_RECORD_BYTES),
			     MPFROMLONG(1L));
	    if (pci)
	    {
	      dcd -> ullTotalBytes += FillInRecordFromFSA(dcd -> hwndCnr, pci,
							  fullname,
							  &fs4, FALSE, dcd);
	      memset(&ri, 0, sizeof(RECORDINSERT));
	      ri.cb = sizeof(RECORDINSERT);
	      ri.pRecordOrder = (PRECORDCORE) CMA_END;
	      ri.pRecordParent = (PRECORDCORE) 0;
	      ri.zOrder = (ULONG) CMA_TOP;
	      ri.cRecordsInsert = 1L;
	      ri.fInvalidateRecord = TRUE;
	      WinSendMsg(dcd -> hwndCnr, CM_INSERTRECORD,
			 MPFROMP(pci), MPFROMP(&ri));
	    }
	  }
	  else
	  {
	    errs++;
	    if (errs > 50L)
	    {				/* prevent runaway on bad file */

	      APIRET ret;

	      ret = saymsg(MB_YESNO, dcd -> hwndCnr,
			   GetPString(IDS_COLLECTNOLISTHDRTEXT),
			   GetPString(IDS_COLLECTNOLISTTEXT),
			   (CHAR *) mp1);
	      if (ret == MBID_NO)
		break;
	      errs = 0L;
	    }
	  }
	}
	fclose(fp);
      }
    }
    if (mp1)
      free(mp1);
    return 0;

  case UM_SELECT:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd)
    {
      switch (SHORT1FROMMP(mp1))
      {
      case IDM_SELECTLIST:
	{
	  CHAR filename[CCHMAXPATH], *p, *pp;

	  strcpy(filename, "*.LST");
	  size = CCHMAXPATH;
	  PrfQueryProfileData(fmprof,
			      appname,
			      "SaveToListName",
			      filename,
			      &size);
	  pp = strrchr(filename, '\\');
	  if (!pp)
	    pp = filename;
	  p = strrchr(pp, '.');
	  if (p && *(p + 1) && p > pp + 1)
	  {
	    if (pp > filename)
	      pp++;
	    *pp = '*';
	    pp++;
	    if (p > pp)
	      memmove(pp, p, strlen(p) + 1);
	  }
	  if (insert_filename(hwnd, filename, FALSE, FALSE))
	    SelectList(dcd -> hwndCnr, TRUE, FALSE, FALSE, NULL, filename, NULL);
	}
	break;
      case IDM_SELECTALL:
	SelectAll(dcd -> hwndCnr, TRUE, TRUE, NULL, NULL, FALSE);
	break;
      case IDM_DESELECTALL:
	DeselectAll(dcd -> hwndCnr, TRUE, TRUE, NULL, NULL, FALSE);
	break;
      case IDM_SELECTALLFILES:
	SelectAll(dcd -> hwndCnr, TRUE, FALSE, NULL, NULL, FALSE);
	break;
      case IDM_DESELECTALLFILES:
	DeselectAll(dcd -> hwndCnr, TRUE, FALSE, NULL, NULL, FALSE);
	break;
      case IDM_SELECTALLDIRS:
	SelectAll(dcd -> hwndCnr, FALSE, TRUE, NULL, NULL, FALSE);
	break;
      case IDM_DESELECTALLDIRS:
	DeselectAll(dcd -> hwndCnr, FALSE, TRUE, NULL, NULL, FALSE);
	break;
      case IDM_DESELECTMASK:
      case IDM_SELECTMASK:
	{
	  MASK mask;
	  PCNRITEM pci = (PCNRITEM) mp2;

	  memset(&mask, 0, sizeof(MASK));
	  mask.fNoAttribs = TRUE;
	  mask.fNoDirs = TRUE;
	  mask.fText = TRUE;
	  strcpy(mask.prompt,
		 GetPString((SHORT1FROMMP(mp1) == IDM_SELECTMASK) ?
			    IDS_SELECTFILTERTEXT :
			    IDS_DESELECTFILTERTEXT));
	  if (pci && (INT) pci != -1)
	    strcpy(mask.szMask, pci -> szFileName);
	  if (WinDlgBox(HWND_DESKTOP, dcd -> hwndCnr, PickMaskDlgProc,
			FM3ModHandle, MSK_FRAME, MPFROMP(&mask)))
	  {
	    if (SHORT1FROMMP(mp1) == IDM_SELECTMASK)
	      SelectAll(dcd -> hwndCnr, TRUE, TRUE, mask.szMask, mask.szText,
			FALSE);
	    else
	      DeselectAll(dcd -> hwndCnr, TRUE, TRUE, mask.szMask, mask.szText,
			  FALSE);
	  }
	}

      case IDM_DESELECTCLIP:
      case IDM_SELECTCLIP:
	{
	  CHAR **list;

	  list = ListFromClipboard(hwnd);
	  if (list)
	  {
	    SelectList(dcd -> hwndCnr, TRUE, FALSE,
		       (SHORT1FROMMP(mp1) == IDM_DESELECTCLIP),
		       NULL, NULL, list);
	    FreeList(list);
	  }
	}
	break;

      case IDM_INVERT:
	InvertAll(dcd -> hwndCnr);
	break;
      }
    }
    return 0;

  case UM_MASSACTION:
    if (mp1)
    {
      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (dcd)
      {
	WORKER *wk;

	wk = malloc(sizeof(WORKER));
	if (wk)
	{
	  memset(wk, 0, sizeof(WORKER));
	  wk -> size = sizeof(WORKER);
	  wk -> hwndCnr = dcd -> hwndCnr;
	  wk -> hwndParent = dcd -> hwndParent;
	  wk -> hwndFrame = dcd -> hwndFrame;
	  wk -> hwndClient = dcd -> hwndClient;
	  wk -> li = (LISTINFO *) mp1;
	  strcpy(wk -> directory, dcd -> directory);
	  if (_beginthread(MassAction, NULL, 122880, (PVOID) wk) == -1)
	  {
	    free(wk);
	    FreeListInfo((LISTINFO *) mp1);
	  }
	}
	else
	  FreeListInfo((LISTINFO *) mp1);
      }
    }
    return 0;

  case UM_ACTION:
    if (mp1)
    {
      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (dcd)
      {
	WORKER *wk;

	wk = malloc(sizeof(WORKER));
	if (wk)
	{
	  memset(wk, 0, sizeof(WORKER));
	  wk -> size = sizeof(WORKER);
	  wk -> hwndCnr = dcd -> hwndCnr;
	  wk -> hwndParent = dcd -> hwndParent;
	  wk -> hwndFrame = dcd -> hwndFrame;
	  wk -> hwndClient = dcd -> hwndClient;
	  wk -> li = (LISTINFO *) mp1;
	  strcpy(wk -> directory, dcd -> directory);
	  if (_beginthread(Action, NULL, 122880, (PVOID) wk) == -1)
	  {
	    free(wk);
	    FreeListInfo((LISTINFO *) mp1);
	  }
	}
	else
	  FreeListInfo((LISTINFO *) mp1);
      }
    }
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    break;

  case WM_DESTROY:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd)
    {
      INT x = 0;

      dcd -> stopflag = 1;
      // Allow rescan logic to quiesce
      while (x++ < 10 && dcd -> amextracted)
	DosSleep(250L);
      WinSendMsg(dcd -> hwndCnr, UM_CLOSE, MPVOID, MPVOID);
      FreeList(dcd -> lastselection);
      free(dcd);
    }
    DosPostEventSem(CompactSem);
    if (!PostMsg(HWND_DESKTOP, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg(HWND_DESKTOP, WM_QUIT, MPVOID, MPVOID);
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CollectorCnrWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *dcd = WinQueryWindowPtr(hwnd, QWL_USER);
  ULONG size;

  static INT savedSortFlags;

  switch (msg)
  {
  case DM_PRINTOBJECT:
    return MRFROMLONG(DRR_TARGET);

  case DM_DISCARDOBJECT:
    if (dcd)
      return WinSendMsg(dcd -> hwndObject, msg, mp1, mp2);
    else
      return MRFROMLONG(DRR_TARGET);

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (SHORT1FROMMP(mp1) & KC_KEYUP)
      return (MRESULT) TRUE;
    if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY)
    {
      switch (SHORT2FROMMP(mp2))
      {
      case VK_DELETE:
	if ((shiftstate & KC_CTRL) == KC_CTRL)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_PERMDELETE, 0), MPVOID);
	else if ((shiftstate & KC_SHIFT) == KC_SHIFT)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_SAVETOCLIP, 0), MPVOID);
	else
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_DELETE, 0), MPVOID);
	break;
      }
    }
    if (shiftstate || fNoSearch)
      break;
    if (SHORT1FROMMP(mp1) & KC_CHAR)
    {
      ULONG thistime, len;
      SEARCHSTRING srch;
      PCNRITEM pci;

      if (!dcd)
	break;
      switch (SHORT1FROMMP(mp2))
      {
      case '\x1b':
      case '\r':
      case '\n':
	dcd -> lasttime = 0;
	*dcd -> szCommonName = 0;
	break;
      default:
	thistime = WinQueryMsgTime(WinQueryAnchorBlock(hwnd));
	if (thistime > dcd -> lasttime + 1250)
	  *dcd -> szCommonName = 0;
	dcd -> lasttime = thistime;
	if (SHORT1FROMMP(mp2) == ' ' && !dcd -> szCommonName)
	  break;
      KbdRetry:
	len = strlen(dcd -> szCommonName);
	if (len >= CCHMAXPATH - 1)
	{
	  *dcd -> szCommonName = 0;
	  len = 0;
	}
	dcd -> szCommonName[len] = toupper(SHORT1FROMMP(mp2));
	dcd -> szCommonName[len + 1] = 0;
	memset(&srch, 0, sizeof(SEARCHSTRING));
	srch.cb = sizeof(SEARCHSTRING);
	srch.pszSearch = dcd -> szCommonName;
	srch.fsPrefix = TRUE;
	srch.fsCaseSensitive = FALSE;
	srch.usView = CV_ICON;
	pci = WinSendMsg(hwnd, CM_SEARCHSTRING, MPFROMP(&srch),
			 MPFROMLONG(CMA_FIRST));
	if (pci && (INT) pci != -1)
	{
	  USHORT attrib = CRA_CURSORED;

	  /* make found item current item */
	  if (!stricmp(pci -> pszFileName, dcd -> szCommonName))
	    attrib |= CRA_SELECTED;
	  WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		     MPFROM2SHORT(TRUE, attrib));
	  /* make sure that record shows in viewport */
	  ShowCnrRecord(hwnd, (PMINIRECORDCORE) pci);
	  return (MRESULT) TRUE;
	}
	else
	{
	  if (SHORT1FROMMP(mp2) == ' ')
	  {
	    dcd -> szCommonName[len] = 0;
	    break;
	  }
	  *dcd -> szCommonName = 0;
	  dcd -> lasttime = 0;
	  if (len)			// retry as first letter if no match

	    goto KbdRetry;
	}
	break;
      }
    }
    break;

  case WM_MOUSEMOVE:
  case WM_BUTTON1UP:
  case WM_BUTTON2UP:
  case WM_BUTTON3UP:
  case WM_CHORD:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_BUTTON1MOTIONEND:
    {
      CNRINFO cnri;

      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      if (WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnri),
		     MPFROMLONG(sizeof(CNRINFO))))
      {
	if (cnri.flWindowAttr & CV_DETAIL)
	  PrfWriteProfileData(fmprof, appname, "CollectorCnrSplitBar",
			      (PVOID) & cnri.xVertSplitbar, sizeof(LONG));
      }
    }
    break;

  case WM_PRESPARAMCHANGED:
    PresParamChanged(hwnd, "Collector", mp1, mp2);
    break;

  case UM_COMPARE:
    if (dcd && mp1 && mp2)
    {
      COMPARE *cmp;
      CHAR *leftdir = (CHAR *) mp1, *rightdir = (CHAR *) mp2;

      if (!IsFile(leftdir) && !IsFile(rightdir))
      {
	cmp = malloc(sizeof(COMPARE));
	if (cmp)
	{
	  memset(cmp, 0, sizeof(COMPARE));
	  cmp -> size = sizeof(COMPARE);
	  strcpy(cmp -> leftdir, leftdir);
	  strcpy(cmp -> rightdir, rightdir);
	  cmp -> hwndParent = dcd -> hwndParent;
	  cmp -> dcd.hwndParent = dcd -> hwndParent;
	  WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, CompareDlgProc,
		    FM3ModHandle, COMP_FRAME, MPFROMP(cmp));
	}
      }
    }
    return 0;

  case UM_UPDATERECORDLIST:
    if (dcd && mp1)
      WinSendMsg(dcd -> hwndObject, msg, mp1, mp2);
    return 0;

  case UM_UPDATERECORD:
    if (dcd && mp1)
    {
      CHAR *filename;

      filename = mp1;
      if (filename)
	UpdateCnrRecord(hwnd, filename, TRUE, dcd);
    }
    return 0;

  case WM_SETFOCUS:
    /*
     * put name of our window on status line
     */
    if (dcd && hwndStatus && mp2)
    {
      PCNRITEM pci = NULL;

      if (fAutoView && hwndMain)
      {
	pci = WinSendMsg(hwnd,
			 CM_QUERYRECORDEMPHASIS,
			 MPFROMLONG(CMA_FIRST),
			 MPFROMSHORT(CRA_CURSORED));
	if (pci && (INT) pci != -1 &&
	    (!(driveflags[toupper(*pci -> szFileName) - 'A'] & DRIVE_SLOW)))
	  WinSendMsg(hwndMain,
		     UM_LOADFILE,
		     MPFROMP(pci -> szFileName),
		     MPVOID);
	else
	  WinSendMsg(hwndMain,
		     UM_LOADFILE,
		     MPVOID,
		     MPVOID);
      }
      if (dcd -> amextracted)
	WinSetWindowText(hwndStatus2,
			 GetPString(IDS_INSEEKSCANTEXT));	// Say working
      WinSendMsg(hwnd,
		 UM_RESCAN,
		 MPVOID,
		 MPVOID);
    }
    break;

  case UM_RESCAN:
    if (dcd)
    {
      CNRINFO cnri;
      CHAR s[CCHMAXPATH + 69], tb[81], tf[81], *p;
      PCNRITEM pci = NULL;

      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnri),
		 MPFROMLONG(sizeof(CNRINFO)));
      dcd -> totalfiles = cnri.cRecords;
      commafmt(tf, sizeof(tf), dcd -> totalfiles);
      CommaFmtULL(tb, sizeof(tb), dcd -> ullTotalBytes, ' ');
      sprintf(s, "%s / %s", tf, tb);
      WinSetDlgItemText(dcd -> hwndClient, DIR_TOTALS, s);

      commafmt(tf, sizeof(tf), dcd -> selectedfiles);
      CommaFmtULL(tb, sizeof(tb), dcd -> selectedbytes, ' ');
      sprintf(s, "%s / %s", tf, tb);
      WinSetDlgItemText(dcd -> hwndClient, DIR_SELECTED, s);

      if (hwndStatus &&
	  dcd -> hwndFrame == WinQueryActiveWindow(dcd -> hwndParent))
      {
	if (hwndMain)
	{
	  pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST),
			   MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1)
	    PostMsg(hwndMain, UM_LOADFILE, MPFROMP(pci -> szFileName), MPVOID);
	  else
	    PostMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	}
	if (!fMoreButtons)
	  sprintf(s, " %s%s%s%s", GetPString(IDS_COLLECTORTEXT),
		  (*dcd -> mask.szMask || dcd -> mask.antiattr ||
		   dcd -> mask.attrFile != ALLATTRS) ? "  (" : NullStr,
		  (*dcd -> mask.szMask) ? dcd -> mask.szMask :
		  (dcd -> mask.antiattr ||
		   dcd -> mask.attrFile != ALLATTRS) ?
		  GetPString(IDS_ATTRTEXT) : NullStr,
		  (*dcd -> mask.szMask || dcd -> mask.antiattr ||
		   dcd -> mask.attrFile != ALLATTRS) ? ")" : NullStr);
	else
	  strcpy(s, GetPString(IDS_COLLECTORTEXT));
	WinSetWindowText(hwndStatus, s);
	if (!pci)
	  pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST),
			   MPFROMSHORT(CRA_CURSORED));
	if (pci && (INT) pci != -1)
	{
	  BOOL fStatus2Used = FALSE;
	  if (fSplitStatus && hwndStatus2)
	  {
	    if (pci -> attrFile & FILE_DIRECTORY)
	      p = pci -> pszFileName;
	    else
	    {
	      p = strrchr(pci -> szFileName, '\\');
	      if (p)
	      {
		if (*(p + 1))
		  p++;
		else
		  p = pci -> pszFileName;
	      }
	      else
		p = pci -> pszFileName;
	    }
	    CommaFmtULL(tb, sizeof(tb), pci -> cbFile + pci -> easize, ' ');
	    if (!fMoreButtons)
	    {
	      sprintf(s, " %s  %04u/%02u/%02u %02u:%02u:%02u  [%s]  %s",
		      tb, pci -> date.year, pci -> date.month,
		    pci -> date.day, pci -> time.hours, pci -> time.minutes,
		      pci -> time.seconds, pci -> pszDispAttr, p);
	    }
	    else
	    {
	      if (pci -> cbFile + pci -> easize > 1024)
		CommaFmtULL(tf, sizeof(tf), pci -> cbFile + pci -> easize, 'K');
	      else
		*tf = 0;
	      sprintf(s, GetPString(IDS_STATUSSIZETEXT),
		      tb,
		      *tf ? " (" : NullStr,
		      tf,
		      *tf ? ")" : NullStr);
	    }
	    WinSetWindowText(hwndStatus2, s);
	    fStatus2Used = TRUE;
	  }
	  if (fMoreButtons)
	  {
	    WinSetWindowText(hwndName, pci -> pszFileName);
	    sprintf(s, "%04u/%02u/%02u %02u:%02u:%02u",
		    pci -> date.year, pci -> date.month,
		    pci -> date.day, pci -> time.hours, pci -> time.minutes,
		    pci -> time.seconds);
	    WinSetWindowText(hwndDate, s);
	    WinSetWindowText(hwndAttr, pci -> pszDispAttr);
	  }
	  if (dcd -> amextracted && hwndStatus2 && !fStatus2Used)
	    WinSetWindowText(hwndStatus2, GetPString(IDS_INSEEKSCANTEXT));	// Say working
	}
	else
	{
	  if (hwndStatus2)
	    WinSetWindowText(hwndStatus2, NullStr);
	  if (fMoreButtons)
	  {
	    WinSetWindowText(hwndName, NullStr);
	    WinSetWindowText(hwndDate, NullStr);
	    WinSetWindowText(hwndAttr, NullStr);
	  }
	}
      }
    }
    return 0;

  case UM_CONTAINER_FILLED:
    DosBeep(1000, 50);
    WinSendMsg(hwnd,
	       CM_INVALIDATERECORD,
	       MPVOID,
	       MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
    WinSendMsg(CollectorCnrMenu,
	       MM_SETITEMATTR,
	       MPFROM2SHORT(DID_CANCEL, TRUE),
	       MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
    WinSendMsg(CollectorCnrMenu,
	       MM_SETITEMATTR,
	       MPFROM2SHORT(IDM_GREP, TRUE),
	       MPFROM2SHORT(MIA_DISABLED, 0));
    PostMsg(hwnd,
	    UM_RESCAN,
	    MPVOID,
	    MPVOID);
    if (dcd)
    {
      dcd -> stopflag = 0;
      dcd -> amextracted = FALSE;		// Say not scanning
      if (dcd -> namecanchange)
      {
	if (!PostMsg(hwnd,
		     WM_CLOSE,
		     MPVOID,
		     MPVOID))
	  WinSendMsg(hwnd,
		     WM_CLOSE,
		     MPVOID,
		     MPVOID);
      }
      else
	WinSetWindowPos(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				       QW_PARENT),
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_SHOW | SWP_RESTORE | SWP_ZORDER);
    }
    return 0;

  case UM_SETUP:
    if (dcd)
    {
      if (!dcd -> hwndObject)
      {
	/*
	 * first time through -- set things up
	 */

	CNRINFO cnri;

	RestorePresParams(hwnd, "Collector");
	LoadDetailsSwitches("Collector", dcd);
	WinSendMsg(CollectorCnrMenu, MM_SETITEMATTR,
		   MPFROM2SHORT(DID_CANCEL, TRUE),
		   MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
	WinSendMsg(CollectorCnrMenu, MM_SETITEMATTR, MPFROM2SHORT(IDM_GREP, TRUE),
		   MPFROM2SHORT(MIA_DISABLED, 0));
	dcd -> amextracted = FALSE;	// Use to indicate scan busy
	dcd -> stopflag = 0;
	memset(&cnri, 0, sizeof(CNRINFO));
	cnri.cb = sizeof(CNRINFO);
	WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnri),
		   MPFROMLONG(sizeof(CNRINFO)));
	cnri.cyLineSpacing = 0;
	cnri.cxTreeIndent = 12L;

	cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				CV_DETAIL));
	cnri.flWindowAttr |= (CV_NAME | CA_DETAILSVIEWTITLES |
			      CV_MINI | CV_FLOW);
	cnri.pSortRecord = (PVOID) SortCollectorCnr;

	{
	  size = sizeof(ULONG);
	  PrfQueryProfileData(fmprof, appname, "CollectorflWindowAttr",
			      (PVOID) & cnri.flWindowAttr, &size);
	  size = sizeof(MASK);
	  if (PrfQueryProfileSize(fmprof, appname, "CollectorFilter", &size) &&
	      size)
	  {
	    PrfQueryProfileData(fmprof, appname, "CollectorFilter", &dcd -> mask,
				&size);
	    SetMask(dcd -> mask.szMask, &dcd -> mask);
	  }
	  else
	  {
	    dcd -> mask.attrFile = (FILE_NORMAL | FILE_READONLY |
				    FILE_DIRECTORY | FILE_HIDDEN |
				    FILE_SYSTEM | FILE_ARCHIVED);
	    dcd -> mask.antiattr = 0;
	  }
	  *(dcd -> mask.prompt) = 0;
	}
	cnri.flWindowAttr |= CV_FLOW;
	cnri.flWindowAttr &= (~(CA_MIXEDTARGETEMPH | CA_ORDEREDTARGETEMPH));
	dcd -> flWindowAttr = cnri.flWindowAttr;
	WinSendMsg(hwnd, CM_SETCNRINFO, MPFROMP(&cnri),
		   MPFROMLONG(CMA_FLWINDOWATTR | CMA_LINESPACING |
			      CMA_CXTREEINDENT | CMA_PSORTRECORD));
	SetCnrCols(hwnd, FALSE);
	AdjustCnrColsForPref(hwnd, NULL, dcd, FALSE);

	/* fix splitbar for collector container */
	cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 32;
	size = sizeof(LONG);
	PrfQueryProfileData(fmprof, appname, "CollectorCnrSplitBar",
			    &cnri.xVertSplitbar, &size);
	if (cnri.xVertSplitbar <= 0)
	  cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 32;
	WinSendMsg(hwnd, CM_SETCNRINFO, MPFROMP(&cnri),
		   MPFROMLONG(CMA_XVERTSPLITBAR));

	if (_beginthread(MakeObjWin, NULL, 245760, (PVOID) dcd) == -1)
	{
	  PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	  return 0;
	}
	else
	  DosSleep(64L);
      }
      SayFilter(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				DIR_FILTER), &dcd -> mask, FALSE);
      SaySort(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
			      DIR_SORT), CollectorsortFlags, FALSE);
      SayView(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
			      DIR_VIEW), dcd -> flWindowAttr);
    }
    else
    {
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      return 0;
    }
    return 0;

  case WM_MENUEND:
    if (dcd)
    {
      HWND hwndMenu = (HWND) mp2;

      if (hwndMenu == CollectorCnrMenu || hwndMenu == CollectorFileMenu ||
	  hwndMenu == CollectorDirMenu)
      {
	MarkAll(hwnd, TRUE, FALSE, TRUE);
	if (dcd -> cnremphasized)
	{
	  WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
		     MPFROM2SHORT(FALSE, CRA_SOURCE));
	  dcd -> cnremphasized = FALSE;
	}
      }
    }
    break;

  case UM_OPENWINDOWFORME:
    if (dcd)
    {
      if (mp1 &&
	  !IsFile((CHAR *) mp1))
	OpenDirCnr(HWND_DESKTOP,
		   hwndMain,
		   dcd -> hwndFrame,
		   FALSE,
		   (char *) mp1);
      else if (mp1 &&
	       IsFile(mp1) == 1)
	StartArcCnr(HWND_DESKTOP,
		    dcd -> hwndFrame,
		    (CHAR *) mp1,
		    4,
		    (ARC_TYPE *) mp2);
    }
    return 0;

  case MM_PORTHOLEINIT:
    if (dcd)
    {
      switch (SHORT1FROMMP(mp1))
      {
      case 0:
      case 1:
	{
	  ULONG wmsg;

	  wmsg = (SHORT1FROMMP(mp1) == 0) ? UM_FILESMENU : UM_VIEWSMENU;
	  PortholeInit((HWND) WinSendMsg(dcd -> hwndClient, wmsg, MPVOID,
					 MPVOID), mp1, mp2);
	}
	break;
      }
    }
    break;

  case UM_INITMENU:
  case WM_INITMENU:
    if (dcd)
    {
      switch (SHORT1FROMMP(mp1))
      {
      case IDM_VIEWSMENU:
	SetViewMenu((HWND) mp2, dcd -> flWindowAttr);
	WinEnableMenuItem((HWND) mp2, IDM_RESELECT,
			  (dcd -> lastselection != NULL));
	break;

      case IDM_DETAILSSETUP:
	SetDetailsSwitches((HWND) mp2, dcd);
	break;

      case IDM_COMMANDSMENU:
	SetupCommandMenu((HWND) mp2, hwnd);
	break;

      case IDM_SORTSUBMENU:
	SetSortChecks((HWND) mp2, CollectorsortFlags);
	break;
      }
      dcd -> hwndLastMenu = (HWND) mp2;
    }
    if (msg == WM_INITMENU)
      break;
    return 0;

  case UM_COLLECTFROMFILE:
    if (mp1)
    {
      if (dcd)
      {
	if (!PostMsg(dcd -> hwndObject, UM_COLLECTFROMFILE, mp1, mp2))
	{
	  free(mp1);
	  DosBeep(50, 100);
	}
      }
      else
	free(mp1);
    }
    else
      free(mp1);
    return 0;

  case UM_COMMAND:
    if (mp1)
    {
      if (dcd)
      {
	if (!PostMsg(dcd -> hwndObject, UM_COMMAND, mp1, mp2))
	{
	  FreeListInfo((LISTINFO *) mp1);
	  DosBeep(50, 100);
	}
	else
	  return (MRESULT) TRUE;
      }
      else
	FreeListInfo((LISTINFO *) mp1);
    }
    return 0;

  case UM_NOTIFY:
    if (mp2)
      AddNote((CHAR *) mp2);
    return 0;

  case WM_COMMAND:
    DosError(FERR_DISABLEHARDERR);
    if (dcd)
    {
      switch (SHORT1FROMMP(mp1))
      {
      case IDM_SETTARGET:
	SetTargetDir(hwnd, FALSE);
	break;

      case IDM_CONTEXTMENU:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  PostMsg(hwnd, WM_CONTROL, MPFROM2SHORT(COLLECTOR_CNR,
					     CN_CONTEXTMENU), MPFROMP(pci));
	}
	break;

      case IDM_SHOWALLFILES:
	{
	  PCNRITEM pci;

	  pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST),
			   MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1)
	  {
	    static CHAR dirname[CCHMAXPATH];

	    strcpy(dirname, pci -> szFileName);
	    MakeValidDir(dirname);
	    StartSeeAll(HWND_DESKTOP, FALSE, dirname);
	  }
	}
	break;

      case IDM_BEGINEDIT:
	OpenEdit(hwnd);
	break;

      case IDM_ENDEDIT:
	WinSendMsg(hwnd, CM_CLOSEEDIT, MPVOID, MPVOID);
	break;

      case IDM_SHOWSELECT:
	QuickPopup(hwnd, dcd, CheckMenu(&CollectorCnrMenu, COLLECTORCNR_POPUP),
		   IDM_SELECTSUBMENU);
	break;

      case IDM_SHOWSORT:
	QuickPopup(hwnd, dcd, CheckMenu(&CollectorCnrMenu, COLLECTORCNR_POPUP),
		   IDM_SORTSUBMENU);
	break;

      case IDM_VIEWORARC:
	{
	  SWP swp;
	  PCNRITEM pci;

	  pci = (PCNRITEM) WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
				      MPFROMLONG(CMA_FIRST),
				      MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1)
	  {
	    WinQueryWindowPos(dcd -> hwndFrame, &swp);
	    DefaultViewKeys(hwnd, dcd -> hwndFrame, dcd -> hwndParent, &swp,
			    pci -> szFileName);
	  }
	}
	break;

      case IDM_SEEALL:
	StartSeeAll(HWND_DESKTOP,
		    FALSE,
		    NULL);
	break;

      case IDM_COLLECTSELECT:
	{
	  CHAR filename[CCHMAXPATH], *p, *pp;

	  strcpy(filename, "*.LST");
	  size = CCHMAXPATH;
	  PrfQueryProfileData(fmprof, appname, "SaveToListName",
			      filename, &size);
	  pp = strrchr(filename, '\\');
	  if (!pp)
	    pp = filename;
	  p = strrchr(pp, '.');
	  if (p && *(p + 1) && p > pp + 1)
	  {
	    if (pp > filename)
	      pp++;
	    *pp = '*';
	    pp++;
	    if (p > pp)
	      memmove(pp, p, strlen(p) + 1);
	  }
	  if (insert_filename(hwnd, filename, FALSE, FALSE))
	  {
	    p = strdup(filename);
	    if (p)
	    {
	      if (!PostMsg(hwnd, UM_COLLECTFROMFILE, MPFROMP(p), MPVOID))
		free(p);
	    }
	  }
	}
	break;

      case IDM_NOTEBOOK:
	if (!ParentIsDesktop(dcd -> hwndParent, dcd -> hwndParent))
	  PostMsg(dcd -> hwndParent, msg, mp1, mp2);
	else
	  WinDlgBox(HWND_DESKTOP, hwnd, CfgDlgProc, FM3ModHandle,
		    CFG_FRAME, (PVOID) "Collector");
	break;

      case IDM_RESELECT:
	SelectList(hwnd, FALSE, FALSE, FALSE, NULL, NULL, dcd -> lastselection);
	break;

      case IDM_HELP:
	if (hwndHelp)
	  WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_COLLECT, 0),
		     MPFROMSHORT(HM_RESOURCEID));
	break;

      case IDM_SORTNONE:
      case IDM_SORTSMARTNAME:
      case IDM_SORTNAME:
      case IDM_SORTFILENAME:
      case IDM_SORTSIZE:
      case IDM_SORTEASIZE:
      case IDM_SORTFIRST:
      case IDM_SORTLAST:
      case IDM_SORTLWDATE:
      case IDM_SORTLADATE:
      case IDM_SORTCRDATE:
      case IDM_SORTSUBJECT:
	savedSortFlags = CollectorsortFlags;
	CollectorsortFlags &= (SORT_REVERSE | SORT_DIRSFIRST | SORT_DIRSLAST);
      case IDM_SORTDIRSFIRST:
      case IDM_SORTDIRSLAST:
      case IDM_SORTREVERSE:
	switch (SHORT1FROMMP(mp1))
	{
	case IDM_SORTSUBJECT:
	  CollectorsortFlags |= SORT_SUBJECT;
	  break;
	case IDM_SORTNONE:
	  CollectorsortFlags |= SORT_NOSORT;
	  break;
	case IDM_SORTSMARTNAME:
	  if (~savedSortFlags & SORT_FILENAME)
	    CollectorsortFlags |= SORT_FILENAME;
	  break;
	case IDM_SORTFILENAME:
	  CollectorsortFlags |= SORT_FILENAME;
	  break;
	case IDM_SORTSIZE:
	  CollectorsortFlags |= SORT_SIZE;
	  break;
	case IDM_SORTEASIZE:
	  CollectorsortFlags |= SORT_EASIZE;
	  break;
	case IDM_SORTFIRST:
	  CollectorsortFlags |= SORT_FIRSTEXTENSION;
	  break;
	case IDM_SORTLAST:
	  CollectorsortFlags |= SORT_LASTEXTENSION;
	  break;
	case IDM_SORTLWDATE:
	  CollectorsortFlags |= SORT_LWDATE;
	  break;
	case IDM_SORTLADATE:
	  CollectorsortFlags |= SORT_LADATE;
	  break;
	case IDM_SORTCRDATE:
	  CollectorsortFlags |= SORT_CRDATE;
	  break;
	case IDM_SORTDIRSFIRST:
	  if (CollectorsortFlags & SORT_DIRSFIRST)
	    CollectorsortFlags &= (~SORT_DIRSFIRST);
	  else
	  {
	    CollectorsortFlags |= SORT_DIRSFIRST;
	    CollectorsortFlags &= (~SORT_DIRSLAST);
	  }
	  break;
	case IDM_SORTDIRSLAST:
	  if (CollectorsortFlags & SORT_DIRSLAST)
	    CollectorsortFlags &= (~SORT_DIRSLAST);
	  else
	  {
	    CollectorsortFlags |= SORT_DIRSLAST;
	    CollectorsortFlags &= (~SORT_DIRSFIRST);
	  }
	  break;
	case IDM_SORTREVERSE:
	  if (CollectorsortFlags & SORT_REVERSE)
	    CollectorsortFlags &= (~SORT_REVERSE);
	  else
	    CollectorsortFlags |= SORT_REVERSE;
	  break;
	}
	PrfWriteProfileData(fmprof, appname, "CollectorSort",
			    &CollectorsortFlags, sizeof(INT));
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(SortCollectorCnr), MPVOID);
	SaySort(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				DIR_SORT), CollectorsortFlags, FALSE);
	break;

      case IDM_COLLECTFROMCLIP:
	{
	  LISTINFO *li;

	  li = malloc(sizeof(LISTINFO));
	  if (li)
	  {
	    memset(li, 0, sizeof(LISTINFO));
	    li -> list = ListFromClipboard(hwnd);
	    if (!li -> list || !li -> list[0])
	      FreeListInfo(li);
	    else
	    {
	      li -> type = IDM_COLLECT;
	      if (!PostMsg(dcd -> hwndObject, UM_COLLECT, MPFROMP(li),
			   MPVOID))
		FreeListInfo(li);
	    }
	  }
	}
	break;

      case IDM_REMOVE:
	if (fAutoView &&
	    hwndMain)
	  PostMsg(hwndMain,
		  UM_LOADFILE,
		  MPVOID,
		  MPVOID);
	dcd -> suspendview = 1;
	RemoveAll(hwnd,
		  &dcd -> ullTotalBytes,
		  &dcd -> totalfiles);
	dcd -> suspendview = 0;
	PostMsg(hwnd,
		UM_RESCAN,
		MPVOID,
		MPVOID);
	break;

      case IDM_CLEARCNR:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) WinSendMsg(hwnd,
				      CM_QUERYRECORD,
				      MPVOID,
				      MPFROM2SHORT(CMA_FIRST,
						   CMA_ITEMORDER));
	  if (pci && (INT) pci != -1)
	  {
	    WinSendMsg(hwnd,
		       CM_REMOVERECORD,
		       MPVOID,
		       MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE));
	    dcd -> ullTotalBytes = dcd -> selectedbytes = dcd -> selectedfiles =
	      dcd -> totalfiles = 0L;
	    PostMsg(hwnd,
		    UM_RESCAN,
		    MPVOID,
		    MPVOID);
	  }
	}
	break;

      case DID_CANCEL:
	if (dcd -> amextracted)
	  dcd -> stopflag = 1;		// Request cancel
	break;

      case IDM_COLLECTOR:
	if (mp2)
	{
	  LISTINFO *li;

	  li = malloc(sizeof(LISTINFO));
	  if (li)
	  {
	    memset(li, 0, sizeof(LISTINFO));
	    li -> list = mp2;
	    if (!li -> list || !li -> list[0])
	      FreeListInfo(li);
	    else
	    {
	      li -> type = IDM_COLLECT;
	      if (!PostMsg(dcd -> hwndObject, UM_COLLECT, MPFROMP(li),
			   MPVOID))
		FreeListInfo(li);
	    }
	  }
	  else
	    FreeList(mp2);
	}
	break;

      case IDM_UNDELETE:
	{
	  PCNRITEM pci;
	  CHAR path[CCHMAXPATH];

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci)
	  {
	    strcpy(path, pci -> szFileName);
	    MakeValidDir(path);
	    WinDlgBox(HWND_DESKTOP, hwnd, UndeleteDlgProc, FM3ModHandle,
		      UNDEL_FRAME, MPFROMP(path));
	  }
	}
	break;

      case IDM_GREP:
	if (!dcd -> amextracted)
	{
	  if (WinDlgBox(HWND_DESKTOP, hwnd, GrepDlgProc,
			FM3ModHandle, GREP_FRAME, (PVOID) & hwnd))
	  {
	    dcd -> amextracted = TRUE;	// Collector scan in progress
	    WinSendMsg(CollectorCnrMenu, MM_SETITEMATTR,
		       MPFROM2SHORT(DID_CANCEL, TRUE),
		       MPFROM2SHORT(MIA_DISABLED, 0));
	    WinSendMsg(CollectorCnrMenu, MM_SETITEMATTR,
		       MPFROM2SHORT(IDM_GREP, TRUE),
		       MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
	    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	  }
	}
	else
	  DosBeep(50, 100);		// Complain about busy
	break;

      case IDM_RESORT:
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(SortCollectorCnr), MPVOID);
	break;

      case IDM_FILTER:
	{
	  BOOL empty = FALSE;
	  PCNRITEM pci;
	  CHAR *p;

	  if (!*dcd -> mask.szMask)
	  {
	    empty = TRUE;
	    pci = (PCNRITEM) CurrentRecord(hwnd);
	    if (pci && !(pci -> attrFile & FILE_DIRECTORY))
	    {
	      p = strrchr(pci -> szFileName, '\\');
	      if (p)
	      {
		p++;
		strcpy(dcd -> mask.szMask, p);
	      }
	    }
	  }
	  *(dcd -> mask.prompt) = 0;

	  if (WinDlgBox(HWND_DESKTOP, hwnd, PickMaskDlgProc,
			FM3ModHandle, MSK_FRAME, MPFROMP(&dcd -> mask)))
	  {
	    size = sizeof(MASK);
	    PrfWriteProfileData(fmprof, appname, "CollectorFilter",
	                        &dcd -> mask, size);
	    dcd -> suspendview = 1;
	    WinSendMsg(hwnd, CM_FILTER, MPFROMP(Filter), MPFROMP(&dcd -> mask));
	    dcd -> suspendview = 0;
	    if (fAutoView && hwndMain)
	    {
	      pci = WinSendMsg(hwnd, CM_QUERYRECORDEMPHASIS,
			       MPFROMLONG(CMA_FIRST),
			       MPFROMSHORT(CRA_CURSORED));
	      if (pci && (INT) pci != -1 &&
		  (!(driveflags[toupper(*pci -> szFileName) - 'A'] & DRIVE_SLOW)))
		WinSendMsg(hwndMain, UM_LOADFILE, MPFROMP(pci -> szFileName),
			   MPVOID);
	      else
		WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	    }
	    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	  }
	  else if (empty)
	    *dcd -> mask.szMask = 0;
	  SayFilter(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				    DIR_FILTER), &dcd -> mask, FALSE);
	}
	break;

      case IDM_HIDEALL:
	if (fAutoView && hwndMain)
	  PostMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	dcd -> suspendview = 1;
	HideAll(hwnd);
	dcd -> suspendview = 0;
	PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	break;

      case IDM_SELECTLIST:
      case IDM_SELECTALL:
      case IDM_DESELECTALL:
      case IDM_SELECTALLFILES:
      case IDM_DESELECTALLFILES:
      case IDM_SELECTALLDIRS:
      case IDM_DESELECTALLDIRS:
      case IDM_SELECTMASK:
      case IDM_DESELECTMASK:
      case IDM_INVERT:
      case IDM_SELECTCLIP:
      case IDM_DESELECTCLIP:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if ((INT) pci == -1)
	    pci = NULL;
	  if (SHORT1FROMMP(mp1) == IDM_HIDEALL)
	  {
	    if (pci)
	    {
	      if (!(pci -> rc.flRecordAttr & CRA_SELECTED))
		pci -> rc.flRecordAttr |= CRA_FILTERED;
	      WinSendMsg(hwnd, CM_INVALIDATERECORD, MPFROMP(&pci),
			 MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
	      break;
	    }
	  }
	  PostMsg(dcd -> hwndObject, UM_SELECT, mp1, MPFROMP(pci));
	}
	break;

      case IDM_RESCAN:
	PostMsg(dcd -> hwndObject, UM_RESCAN, MPVOID, MPVOID);
	break;

      case IDM_SHOWLNAMES:
      case IDM_SHOWSUBJECT:
      case IDM_SHOWEAS:
      case IDM_SHOWSIZE:
      case IDM_SHOWICON:
      case IDM_SHOWLWDATE:
      case IDM_SHOWLWTIME:
      case IDM_SHOWLADATE:
      case IDM_SHOWLATIME:
      case IDM_SHOWCRDATE:
      case IDM_SHOWCRTIME:
      case IDM_SHOWATTR:
	AdjustDetailsSwitches(hwnd, dcd -> hwndLastMenu,
			      SHORT1FROMMP(mp1), NULL,
			      "Collector", dcd, FALSE);
	break;

      case IDM_ICON:
      case IDM_TEXT:
      case IDM_DETAILS:
      case IDM_NAME:
      case IDM_MINIICONS:
      case IDM_DETAILSTITLES:
	{
	  CNRINFO cnri;

	  memset(&cnri, 0, sizeof(CNRINFO));
	  cnri.cb = sizeof(CNRINFO);
	  WinSendMsg(hwnd, CM_QUERYCNRINFO, MPFROMP(&cnri),
		     MPFROMLONG(sizeof(CNRINFO)));
	  switch (SHORT1FROMMP(mp1))
	  {
	  case IDM_ICON:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME));
	    cnri.flWindowAttr |= CV_ICON;
	    break;
	  case IDM_NAME:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME));
	    cnri.flWindowAttr |= CV_NAME;
	    break;
	  case IDM_TEXT:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME));
	    cnri.flWindowAttr |= CV_TEXT;
	    break;
	  case IDM_DETAILS:
	    cnri.flWindowAttr &= (~(CV_ICON | CV_TREE | CV_TEXT |
				    CV_DETAIL | CV_NAME));
	    cnri.flWindowAttr |= CV_DETAIL;
	    break;
	  case IDM_MINIICONS:
	    if (cnri.flWindowAttr & CV_MINI)
	      cnri.flWindowAttr &= (~CV_MINI);
	    else
	      cnri.flWindowAttr |= CV_MINI;
	    break;
	  case IDM_DETAILSTITLES:
	    if (cnri.flWindowAttr & CA_DETAILSVIEWTITLES)
	      cnri.flWindowAttr &= (~CA_DETAILSVIEWTITLES);
	    else
	      cnri.flWindowAttr |= CA_DETAILSVIEWTITLES;
	    break;
	  }
	  cnri.flWindowAttr &= (~(CA_ORDEREDTARGETEMPH |
				  CA_MIXEDTARGETEMPH));
	  cnri.flWindowAttr |= CV_FLOW;
	  dcd -> flWindowAttr = cnri.flWindowAttr;
	  PrfWriteProfileData(fmprof, appname, "CollectorflWindowAttr",
			      &cnri.flWindowAttr, sizeof(ULONG));
	  WinSendMsg(hwnd, CM_SETCNRINFO, MPFROMP(&cnri),
		     MPFROMLONG(CMA_FLWINDOWATTR));
	  WinSendMsg(hwnd, CM_INVALIDATERECORD, MPVOID,
		     MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
	  SayView(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				  DIR_VIEW), dcd -> flWindowAttr);
	}
	break;

      case IDM_SAVETOLIST:
	WinDlgBox(HWND_DESKTOP, hwnd, SaveListDlgProc, FM3ModHandle,
		  SAV_FRAME, MPFROMP(&hwnd));
	break;

      case IDM_SIZES:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1)
	    WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, DirSizeProc, FM3ModHandle,
		      DSZ_FRAME, pci -> szFileName);
	}
	break;

      case IDM_MKDIR:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  PMMkDir(dcd -> hwndParent, (pci && (INT) pci != -1) ?
		  pci -> szFileName : NULL, FALSE);
	}
	break;

      case IDM_DOITYOURSELF:
      case IDM_UPDATE:
      case IDM_COLLECTFROMFILE:
      case IDM_OPENWINDOW:
      case IDM_OPENSETTINGS:
      case IDM_OPENDEFAULT:
      case IDM_OPENICON:
      case IDM_OPENDETAILS:
      case IDM_OPENTREE:
      case IDM_OBJECT:
      case IDM_SHADOW:
      case IDM_SHADOW2:
      case IDM_DELETE:
      case IDM_PERMDELETE:
      case IDM_PRINT:
      case IDM_ATTRS:
      case IDM_INFO:
      case IDM_COPY:
      case IDM_MOVE:
      case IDM_WPSCOPY:
      case IDM_WPSMOVE:
      case IDM_COPYPRESERVE:
      case IDM_MOVEPRESERVE:
      case IDM_WILDCOPY:
      case IDM_WILDMOVE:
      case IDM_RENAME:
      case IDM_COMPARE:
      case IDM_EAS:
      case IDM_SUBJECT:
      case IDM_VIEW:
      case IDM_VIEWTEXT:
      case IDM_VIEWBINARY:
      case IDM_VIEWARCHIVE:
      case IDM_EDIT:
      case IDM_EDITTEXT:
      case IDM_EDITBINARY:
      case IDM_SAVETOCLIP:
      case IDM_APPENDTOCLIP:
      case IDM_ARCHIVE:
      case IDM_ARCHIVEM:
      case IDM_EXTRACT:
      case IDM_MCIPLAY:
      case IDM_UUDECODE:
      case IDM_MERGE:
	{
	  LISTINFO *li;
	  ULONG action = UM_ACTION;

	  li = malloc(sizeof(LISTINFO));
	  if (li)
	  {
	    memset(li, 0, sizeof(LISTINFO));
	    li -> type = SHORT1FROMMP(mp1);
	    li -> hwnd = hwnd;
	    li -> list = BuildList(hwnd);
	    if (li -> list)
	    {
	      switch (SHORT1FROMMP(mp1))
	      {
	      case IDM_DOITYOURSELF:
	      case IDM_APPENDTOCLIP:
	      case IDM_SAVETOCLIP:
	      case IDM_ARCHIVE:
	      case IDM_ARCHIVEM:
	      case IDM_DELETE:
	      case IDM_PERMDELETE:
	      case IDM_ATTRS:
	      case IDM_PRINT:
	      case IDM_SHADOW:
	      case IDM_SHADOW2:
	      case IDM_OBJECT:
	      case IDM_VIEW:
	      case IDM_VIEWTEXT:
	      case IDM_VIEWBINARY:
	      case IDM_EDIT:
	      case IDM_EDITTEXT:
	      case IDM_EDITBINARY:
	      case IDM_MCIPLAY:
	      case IDM_UPDATE:
	      case IDM_INFO:
	      case IDM_EAS:
		action = UM_MASSACTION;
		break;
	      }
	      if (li -> type == IDM_SHADOW || li -> type == IDM_OBJECT ||
		  li -> type == IDM_SHADOW2)
		*li -> targetpath = 0;
	      if (!PostMsg(dcd -> hwndObject, action, MPFROMP(li),
			   MPVOID))
	      {
		FreeListInfo(li);
		DosBeep(50, 100);
	      }
	      else if (fUnHilite)
		UnHilite(hwnd, TRUE, &dcd -> lastselection);
	    }
	    else
	      free(li);
	  }
	}
	break;

      default:
	if (!cmdloaded)
	  load_commands();
	if (SHORT1FROMMP(mp1) >= IDM_COMMANDSTART &&
	    SHORT1FROMMP(mp1) < IDM_QUICKTOOLSTART)
	{
	  INT x;

	  x = SHORT1FROMMP(mp1) - IDM_COMMANDSTART;
	  if (x >= 0)
	  {
	    x++;
	    RunCommand(hwnd, x);
	    if (fUnHilite)
	      UnHilite(hwnd, TRUE, &dcd -> lastselection);
	  }
	}
	break;
      }
    }
    return 0;

  case UM_FIXCNRMLE:
  case UM_FIXEDITNAME:
    return CommonCnrProc(hwnd, msg, mp1, mp2);

  case UM_FILESMENU:
    {
      PCNRITEM pci;
      HWND menuHwnd = (HWND) 0;

      pci = (PCNRITEM) CurrentRecord(hwnd);
      if (pci && (INT) pci != -1)
      {
	if (pci -> attrFile & FILE_DIRECTORY)
	  menuHwnd = CheckMenu(&CollectorDirMenu, COLLECTORDIR_POPUP);
	else
	  menuHwnd = CheckMenu(&CollectorFileMenu, COLLECTORFILE_POPUP);
      }
      return MRFROMLONG(menuHwnd);
    }

  case WM_CONTROL:
    DosError(FERR_DISABLEHARDERR);
    if (dcd)
    {
      switch (SHORT2FROMMP(mp1))
      {
      case CN_CONTEXTMENU:
	{
	  PCNRITEM pci = (PCNRITEM) mp2;

	  if (pci)
	  {
	    WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		       MPFROM2SHORT(TRUE, CRA_CURSORED));
	    MarkAll(hwnd, FALSE, FALSE, TRUE);
	    if (pci -> attrFile & FILE_DIRECTORY)
	      dcd -> hwndLastMenu = CheckMenu(&CollectorDirMenu,
					      COLLECTORDIR_POPUP);
	    else
	      dcd -> hwndLastMenu = CheckMenu(&CollectorFileMenu,
					      COLLECTORFILE_POPUP);
	  }
	  else
	  {
	    dcd -> hwndLastMenu = CheckMenu(&CollectorCnrMenu,
					    COLLECTORCNR_POPUP);
	    if (dcd -> hwndLastMenu && !dcd -> cnremphasized)
	    {
	      WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
			 MPFROM2SHORT(TRUE, CRA_SOURCE));
	      dcd -> cnremphasized = TRUE;
	    }
	  }
	  if (dcd -> hwndLastMenu)
	  {
	    if (dcd -> hwndLastMenu == CollectorCnrMenu)
	    {
	      SetViewMenu(dcd -> hwndLastMenu, dcd -> flWindowAttr);
	      SetDetailsSwitches(dcd -> hwndLastMenu, dcd);
	      if (dcd -> flWindowAttr & CV_MINI)
		WinCheckMenuItem(dcd -> hwndLastMenu, IDM_MINIICONS, TRUE);
	    }
	    if (!PopupMenu(hwnd, hwnd, dcd -> hwndLastMenu))
	    {
	      if (dcd -> cnremphasized)
	      {
		WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
			   MPFROM2SHORT(FALSE, CRA_SOURCE));
		dcd -> cnremphasized = TRUE;
	      }
	      MarkAll(hwnd, TRUE, FALSE, TRUE);
	    }
	  }
	}
	break;

      case CN_DROPHELP:
	if (mp2)
	{
	  PDRAGINFO pDInfo;
	  PCNRITEM pci;
	  ULONG numitems;
	  USHORT usOperation;

	  pci = (PCNRITEM) ((PCNRDRAGINFO) mp2) -> pRecord;
	  pDInfo = ((PCNRDRAGINFO) mp2) -> pDragInfo;
	  if (!DrgAccessDraginfo(pDInfo))
	  {
	    Win_Error(hwnd, hwnd, __FILE__, __LINE__,
		      "%s",
		      GetPString(IDS_DROPERRORTEXT));
	    break;
	  }
	  numitems = DrgQueryDragitemCount(pDInfo);
	  usOperation = pDInfo -> usOperation;
	  DrgDeleteDraginfoStrHandles(pDInfo);
	  DrgFreeDraginfo(pDInfo);
	  saymsg(MB_ENTER | MB_ICONASTERISK,
		 hwnd,
		 GetPString(IDS_DROPHELPHDRTEXT),
		 GetPString(IDS_DROPHELPTEXT),
		 numitems,
		 &"s"[numitems == 1L],
		 (pci) ? NullStr : GetPString(IDS_NOTEXT),
		 (pci) ? NullStr : " ",
		 (pci) ? pci -> szFileName : NullStr,
		 (pci) ? " " : NullStr,
		 GetPString((usOperation == DO_COPY) ?
			    IDS_COPYTEXT :
			    (usOperation == DO_LINK) ?
			    IDS_LINKTEXT : IDS_MOVETEXT));
	}
	return 0;

      case CN_DRAGLEAVE:
	if (mp2)
	{
	  PDRAGINFO pDInfo;

	  pDInfo = ((PCNRDRAGINFO) mp2) -> pDragInfo;
	  DrgAccessDraginfo(pDInfo);	/* Access DRAGINFO       */
	  DrgFreeDraginfo(pDInfo);	/* Free DRAGINFO         */
	}
	return 0;

      case CN_DRAGAFTER:
      case CN_DRAGOVER:
	if (mp2)
	{
	  PDRAGITEM pDItem;	/* Pointer to DRAGITEM   */
	  PDRAGINFO pDInfo;	/* Pointer to DRAGINFO   */
	  PCNRITEM pci;
	  USHORT uso;

	  pci = (PCNRITEM) ((PCNRDRAGINFO) mp2) -> pRecord;
//              if(SHORT1FROMMP(mp1) == CN_DRAGAFTER)
	  //                pci = NULL;
	  pDInfo = ((PCNRDRAGINFO) mp2) -> pDragInfo;
	  DrgAccessDraginfo(pDInfo);	/* Access DRAGINFO       */
	  if (pci)
	  {
	    if (pci -> rc.flRecordAttr & CRA_SOURCE)
	    {
	      DrgFreeDraginfo(pDInfo);
	      return (MRFROM2SHORT(DOR_NODROP, 0));
	    }
	    uso = pDInfo -> usOperation;
	    if (uso == DO_DEFAULT)
	      uso = (fCopyDefault) ? DO_COPY : DO_MOVE;
	    if (!(pci -> attrFile & FILE_DIRECTORY))
	    {
	      if (uso != DO_LINK && uso != DO_MOVE &&
		  uso != DO_COPY)
	      {
		DrgFreeDraginfo(pDInfo);
		return MRFROM2SHORT(DOR_NODROP, 0);
	      }
	      if (uso != DO_LINK &&
		  !(driveflags[toupper(*pci -> szFileName) - 'A'] &
		    DRIVE_NOTWRITEABLE))
	      {
		ARC_TYPE *info = NULL;

		if (!fQuickArcFind &&
		    !(driveflags[toupper(*pci -> szFileName) - 'A'] &
		      DRIVE_SLOW))
		  info = find_type(pci -> szFileName, NULL);
		else
		  info = quick_find_type(pci -> szFileName, NULL);
		if (!info || ((uso == DO_MOVE && !info -> move) ||
			      (uso == DO_COPY && !info -> create)))
		{
		  DrgFreeDraginfo(pDInfo);
		  return MRFROM2SHORT(DOR_NODROP, 0);
		}
	      }
	    }
	  }
	  pDItem = DrgQueryDragitemPtr(pDInfo,	/* Access DRAGITEM       */
				       0);	/* Index to DRAGITEM     */
	  if (DrgVerifyRMF(pDItem,	/* Check valid rendering */
			   DRM_OS2FILE,	/* mechanisms and data   */
			   NULL))
	  {
	    DrgFreeDraginfo(pDInfo);	/* Free DRAGINFO         */
	    if (pci)
	    {
	      if (driveflags[toupper(*pci -> szFileName) - 'A'] &
		  DRIVE_NOTWRITEABLE)
		return MRFROM2SHORT(DOR_DROP, DO_LINK);
	      if (toupper(*pci -> szFileName) < 'C')
		return MRFROM2SHORT(DOR_DROP, DO_COPY);
	      return MRFROM2SHORT(DOR_DROP,	/* Return okay to drop   */
				  ((fCopyDefault) ?
				   DO_COPY :
				   DO_MOVE));
	    }
	    else
	      return MRFROM2SHORT(DOR_DROP,	/* Return okay to drop   */
				  DO_COPY);
	  }
	  DrgFreeDraginfo(pDInfo);	/* Free DRAGINFO         */
	}
	return (MRFROM2SHORT(DOR_NODROP, 0));	/* Drop not valid        */

      case CN_INITDRAG:
	if (mp2)
	{
	  BOOL wasemphasized = FALSE;
	  PCNRDRAGINIT pcd = (PCNRDRAGINIT) mp2;
	  PCNRITEM pci;

	  if (pcd)
	  {
	    pci = (PCNRITEM) pcd -> pRecord;
	    if (pci)
	    {
	      if (pci -> rc.flRecordAttr & CRA_SELECTED)
		wasemphasized = TRUE;
	      if (IsRoot(pci -> szFileName))
		break;
	      if (hwndStatus2)
		WinSetWindowText(hwndStatus2,
				 GetPString(IDS_DRAGFILEOBJTEXT));
	      if (DoFileDrag(hwnd,
			     dcd -> hwndObject,
			     mp2,
			     NULL,
			     NULL,
			     TRUE))
	      {
		if (fUnHilite && wasemphasized)
		  UnHilite(hwnd, TRUE, &dcd -> lastselection);
	      }
	      if (hwndStatus2)
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	  }
	}
	return 0;

      case CN_DROP:
	if (mp2)
	{
	  LISTINFO *li;
	  ULONG action = UM_ACTION;

	  li = DoFileDrop(hwnd, NULL, TRUE, mp1, mp2);
	  if (li)
	  {
	    if (!*li -> targetpath)
	    {
	      li -> type = IDM_COLLECT;
	      action = UM_COLLECT;
	    }
	    else
	    {
	      if (li -> list && li -> list[0] && IsRoot(li -> list[0]))
		li -> type = DO_LINK;
	      else if (fDragndropDlg && (!*li -> arcname || !li -> info))
	      {
		CHECKLIST cl;

		memset(&cl, 0, sizeof(cl));
		cl.size = sizeof(cl);
		cl.flags = li -> type;
		cl.list = li -> list;
		cl.cmd = li -> type;
		cl.prompt = li -> targetpath;
		li -> type = WinDlgBox(HWND_DESKTOP, dcd -> hwndParent,
				       DropListProc, FM3ModHandle,
				       DND_FRAME, MPFROMP(&cl));
		if (!li -> type)
		{
		  FreeListInfo(li);
		  return 0;
		}
		li -> list = cl.list;
		if (!li -> list || !li -> list[0])
		{
		  FreeListInfo(li);
		  return 0;
		}
	      }
	      switch (li -> type)
	      {
	      case DND_LAUNCH:
		strcat(li -> targetpath, " %a");
		ExecOnList(dcd -> hwndParent, li -> targetpath,
			   PROMPT | WINDOWED, NULL, li -> list, NULL);
		FreeList(li -> list);
		li -> list = NULL;
		break;
	      case DO_LINK:
		if (fLinkSetsIcon)
		{
		  li -> type = IDM_SETICON;
		  action = UM_MASSACTION;
		}
		else
		  li -> type = IDM_COMPARE;
		break;
	      case DND_EXTRACT:
		if (*li -> targetpath && !IsFile(li -> targetpath))
		  li -> type = IDM_EXTRACT;
		break;
	      case DND_MOVE:
		li -> type = IDM_MOVE;
		if (*li -> targetpath && IsFile(li -> targetpath) == 1)
		{
		  action = UM_MASSACTION;
		  li -> type = IDM_ARCHIVEM;
		}
		break;
	      case DND_WILDMOVE:
		li -> type = IDM_WILDMOVE;
		if (*li -> targetpath && IsFile(li -> targetpath) == 1)
		{
		  action = UM_MASSACTION;
		  li -> type = IDM_ARCHIVEM;
		}
		break;
	      case DND_OBJECT:
		li -> type = IDM_OBJECT;
		action = UM_MASSACTION;
		break;
	      case DND_SHADOW:
		li -> type = IDM_SHADOW;
		action = UM_MASSACTION;
		break;
	      case DND_COMPARE:
		li -> type = IDM_COMPARE;
		break;
	      case DND_SETICON:
		action = UM_MASSACTION;
		li -> type = IDM_SETICON;
		break;
	      case DND_WILDCOPY:
		li -> type = IDM_WILDCOPY;
		if (*li -> targetpath && IsFile(li -> targetpath) == 1)
		{
		  action = UM_MASSACTION;
		  li -> type = IDM_ARCHIVE;
		}
		break;
	      case DND_COPY:
		li -> type = IDM_COPY;
		if (*li -> targetpath && IsFile(li -> targetpath) == 1)
		{
		  action = UM_MASSACTION;
		  li -> type = IDM_ARCHIVE;
		}
		break;
	      default:
		if (*li -> arcname && li -> info)
		{
		  action = UM_MASSACTION;
		  li -> type = (li -> type == DO_MOVE) ? IDM_FAKEEXTRACTM : IDM_FAKEEXTRACT;
		}
		else if (*li -> targetpath && IsFile(li -> targetpath) == 1)
		{
		  action = UM_MASSACTION;
		  li -> type = (li -> type == DO_MOVE) ? IDM_ARCHIVEM : IDM_ARCHIVE;
		}
		else
		  li -> type = (li -> type == DO_MOVE) ? IDM_MOVE : IDM_COPY;
		break;
	      }
	    }
	    if (!li -> list || !li -> list[0])
	      FreeListInfo(li);
	    else if (!PostMsg(dcd -> hwndObject, action, MPFROMP(li), MPVOID))
	      FreeListInfo(li);
	    else
	    {
	      USHORT usop = 0;

	      switch (li -> type)
	      {
	      case IDM_COPY:
	      case IDM_WILDCOPY:
		usop = DO_COPY;
		break;
	      case IDM_MOVE:
	      case IDM_WILDMOVE:
	      case IDM_ARCHIVEM:
		usop = DO_MOVE;
		break;
	      }
	      if (usop)
		return MRFROM2SHORT(DOR_DROP, usop);
	    }
	  }
	}
	return 0;

      case CN_BEGINEDIT:
      case CN_REALLOCPSZ:
      case CN_ENDEDIT:
	{
	  MRESULT mre;

	  mre = CnrDirectEdit(hwnd, msg, mp1, mp2);
	  if (mre != (MRESULT) - 1)
	    return mre;
	}
	break;

      case CN_EMPHASIS:
	if (mp2)
	{
	  PNOTIFYRECORDEMPHASIS pre = mp2;
	  PCNRITEM pci;
	  CHAR s[CCHMAXPATH + 91], tb[81], tf[81], *p;

	  pci = (PCNRITEM) ((pre) ? pre -> pRecord : NULL);
	  if (!pci)
	  {
	    if (hwndStatus2)
	      WinSetWindowText(hwndStatus2, NullStr);
	    if (fMoreButtons)
	    {
	      WinSetWindowText(hwndName, NullStr);
	      WinSetWindowText(hwndDate, NullStr);
	      WinSetWindowText(hwndAttr, NullStr);
	    }
	    if (hwndMain)
	      WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
	    break;
	  }
	  if (pre -> fEmphasisMask & CRA_SELECTED)
	  {
	    if (pci -> rc.flRecordAttr & CRA_SELECTED)
	    {
	      dcd -> selectedbytes += (pci -> cbFile + pci -> easize);
	      dcd -> selectedfiles++;
	    }
	    else if (dcd -> selectedfiles)
	    {
	      dcd -> selectedbytes -= (pci -> cbFile + pci -> easize);
	      dcd -> selectedfiles--;
	    }
	    if (!dcd -> suspendview)
	    {
	      commafmt(tf, sizeof(tf), dcd -> selectedfiles);
	      CommaFmtULL(tb, sizeof(tb), dcd -> selectedbytes, ' ');
	      sprintf(s, "%s / %s", tf, tb);
	      WinSetDlgItemText(dcd -> hwndClient, DIR_SELECTED, s);
	    }
	  }
	  if (!dcd -> suspendview &&
	      WinQueryActiveWindow(dcd -> hwndParent) == dcd -> hwndFrame)
	  {
	    if (pre -> fEmphasisMask & CRA_CURSORED)
	    {
	      if (pci -> rc.flRecordAttr & CRA_CURSORED)
	      {
		if (fSplitStatus && hwndStatus2)
		{
		  if (pci -> attrFile & FILE_DIRECTORY)
		    p = pci -> pszFileName;
		  else
		  {
		    p = strrchr(pci -> szFileName, '\\');
		    if (p)
		    {
		      if (*(p + 1))
			p++;
		      else
			p = pci -> pszFileName;
		    }
		    else
		      p = pci -> pszFileName;
		  }
		  CommaFmtULL(tb, sizeof(tb), pci -> cbFile + pci -> easize, ' ');
		  if (!fMoreButtons)
		    sprintf(s, " %s  %04u/%02u/%02u %02u:%02u:%02u  [%s]  %s",
			    tb, pci -> date.year,
		      pci -> date.month, pci -> date.day, pci -> time.hours,
			    pci -> time.minutes, pci -> time.seconds,
			    pci -> pszDispAttr, p);
		  else
		  {
		    if (pci -> cbFile + pci -> easize > 1024)
		      CommaFmtULL(tf, sizeof(tf), pci -> cbFile + pci -> easize, ' ');
		    else
		      *tf = 0;
		    sprintf(s, GetPString(IDS_STATUSSIZETEXT),
			    tb,
			    *tf ? " (" : NullStr,
			    tf,
			    *tf ? ")" : NullStr);
		  }
		  WinSetWindowText(hwndStatus2, s);
		}
		if (fMoreButtons)
		{
		  WinSetWindowText(hwndName, pci -> pszFileName);
		  sprintf(s, "%04u/%02u/%02u %02u:%02u:%02u",
			  pci -> date.year, pci -> date.month,
		    pci -> date.day, pci -> time.hours, pci -> time.minutes,
			  pci -> time.seconds);
		  WinSetWindowText(hwndDate, s);
		  WinSetWindowText(hwndAttr, pci -> pszDispAttr);
		}
	      }
	    }
	  }
	  if (!dcd -> suspendview && hwndMain &&
	      (pre -> fEmphasisMask & CRA_CURSORED) &&
	      (pci -> rc.flRecordAttr & CRA_CURSORED) &&
	      WinQueryActiveWindow(dcd -> hwndParent) == dcd -> hwndFrame)
	    WinSendMsg(hwndMain, UM_LOADFILE,
	    MPFROMP(((fComments || (pci -> attrFile & FILE_DIRECTORY) == 0) ?
		     pci -> szFileName : NULL)), MPVOID);
	}
	break;

      case CN_ENTER:
	if (mp2)
	{
	  PCNRITEM pci = (PCNRITEM) ((PNOTIFYRECORDENTER) mp2) -> pRecord;
	  FILEFINDBUF3 ffb;
	  HDIR hDir = HDIR_CREATE;
	  ULONG nm = 1L;
	  APIRET status = 0;

	  SetShiftState();
	  if (pci)
	  {
	    if (pci -> rc.flRecordAttr & CRA_INUSE)
	      break;
	    DosError(FERR_DISABLEHARDERR);
	    status = DosFindFirst(pci -> szFileName, &hDir,
				  FILE_NORMAL | FILE_DIRECTORY |
				  FILE_ARCHIVED | FILE_READONLY |
				  FILE_HIDDEN | FILE_SYSTEM,
				  &ffb, sizeof(ffb), &nm,
				  FIL_STANDARD);
	    priority_bumped();
	    if (!status)
	    {
	      DosFindClose(hDir);
	      if (ffb.attrFile & FILE_DIRECTORY)
	      {
		if ((shiftstate & (KC_CTRL | KC_ALT)) ==
		    (KC_CTRL | KC_ALT))
		  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_SHOWALLFILES, 0),
			  MPVOID);
		else if ((shiftstate & (KC_CTRL | KC_SHIFT)) ==
			 (KC_CTRL | KC_SHIFT))
		  OpenObject(pci -> szFileName, Settings, dcd -> hwndFrame);
		else if (shiftstate & KC_CTRL)
		  OpenObject(pci -> szFileName, Default, dcd -> hwndFrame);
		else
		  OpenDirCnr(HWND_DESKTOP,
			     hwndMain,
			     dcd -> hwndFrame,
			     FALSE,
			     pci -> szFileName);
	      }
	      else
	      {
		SWP swp;

		WinSendMsg(hwnd,
			   CM_SETRECORDEMPHASIS,
			   MPFROMP(pci),
			   MPFROM2SHORT(TRUE, CRA_INUSE));
		WinQueryWindowPos(dcd -> hwndFrame, &swp);
		DefaultViewKeys(hwnd,
				dcd -> hwndFrame,
				dcd -> hwndParent,
				&swp,
				pci -> szFileName);
		WinSendMsg(hwnd,
			   CM_SETRECORDEMPHASIS,
			   MPFROMP(pci),
			   MPFROM2SHORT(FALSE, CRA_INUSE |
					((fUnHilite) ? CRA_SELECTED : 0)));
	      }
	    }
	    else
	      WinSendMsg(hwnd,
			 CM_REMOVERECORD,
			 MPFROMP(&pci),
			 MPFROM2SHORT(1,
				    CMA_FREE | CMA_INVALIDATE | CMA_ERASE));
	  }
	}
	break;
      }
    }
    return 0;

  case UM_LOADFILE:
    if (dcd && mp2)
    {
      HWND ret;

      ret = StartMLEEditor(dcd -> hwndParent,
			   (INT) mp1,
			   (CHAR *) mp2,
			   dcd -> hwndFrame);
      if (mp2)
	free((CHAR *) mp2);
      return MRFROMLONG(ret);
    }
    return 0;

  case UM_CLOSE:
    WinDestroyWindow(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
				    QW_PARENT));
    return 0;

  case UM_FOLDUP:
    if (!PostMsg(HWND_DESKTOP, WM_QUIT, MPVOID, MPVOID))
      DosExit(EXIT_PROCESS, 1);
    return 0;

  case WM_CLOSE:
    if (dcd)
    {
      dcd -> namecanchange = TRUE;
      dcd -> stopflag = 1;
      if (dcd -> amextracted)
	return 0;		// Can not close yet
    }
    WinSendMsg(hwnd, WM_SAVEAPPLICATION, MPVOID, MPVOID);
    if (dcd)
    {
      if (!dcd -> dontclose && ParentIsDesktop(hwnd, dcd -> hwndParent))
	PostMsg(hwnd, UM_FOLDUP, MPVOID, MPVOID);
      if (dcd -> hwndObject)
      {
	DosSleep(64L);
	if (!PostMsg(dcd -> hwndObject, WM_CLOSE, MPVOID, MPVOID))
	  WinSendMsg(dcd -> hwndObject, WM_CLOSE, MPVOID, MPVOID);
      }
    }
    else
      WinSendMsg(hwnd, UM_CLOSE, MPVOID, MPVOID);
    return 0;

  case WM_DESTROY:
    if (CollectorDirMenu)
      WinDestroyWindow(CollectorDirMenu);
    if (CollectorFileMenu)
      WinDestroyWindow(CollectorFileMenu);
    if (CollectorCnrMenu)
      WinDestroyWindow(CollectorCnrMenu);
    CollectorCnrMenu = CollectorFileMenu = CollectorDirMenu = (HWND) 0;
    Collector = (HWND) 0;
    EmptyCnr(hwnd);
    break;
  }
  return (dcd && dcd -> oldproc) ? dcd -> oldproc(hwnd, msg, mp1, mp2) :
    PFNWPCnr(hwnd, msg, mp1, mp2);
}

HWND StartCollector(HWND hwndParent, INT flags)
{
  HWND hwndFrame = (HWND) 0;
  HWND hwndClient;
  ULONG FrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
                     FCF_SIZEBORDER | FCF_MINMAX |
                     FCF_ICON | FCF_NOBYTEALIGN |
                     FCF_ACCELTABLE;
  USHORT id;
  DIRCNRDATA *dcd;

  static USHORT idinc = 0;

  if (ParentIsDesktop(hwndParent, hwndParent))
    FrameFlags |= (FCF_TASKLIST | FCF_SHELLPOSITION | FCF_MENU);
  if (Collector)
  {
    WinSetWindowPos(WinQueryWindow(WinQueryWindow(Collector,
						  QW_PARENT),
				   QW_PARENT),
		    HWND_TOP,
		    0,
		    0,
		    0,
		    0,
		    SWP_SHOW | SWP_RESTORE);
    return WinQueryWindow(WinQueryWindow(Collector,
					 QW_PARENT),
			  QW_PARENT);
  }
  hwndFrame = WinCreateStdWindow(hwndParent,
				 WS_VISIBLE,
				 &FrameFlags,
				 GetPString(IDS_WCCOLLECTOR),
				 NULL,
				 WS_VISIBLE | fwsAnimate,
				 FM3ModHandle,
				 COLLECTOR_FRAME,
				 &hwndClient);
  if (hwndFrame && hwndClient)
  {
    id = COLLECTOR_FRAME + idinc++;
    WinSetWindowUShort(hwndFrame, QWS_ID, id);
    dcd = malloc(sizeof(DIRCNRDATA));
    if (dcd)
    {
      memset(dcd, 0, sizeof(DIRCNRDATA));
      dcd -> size = sizeof(DIRCNRDATA);
      dcd -> id = id;
      dcd -> type = COLLECTOR_FRAME;
      dcd -> hwndParent = (hwndParent) ? hwndParent : HWND_DESKTOP;
      dcd -> hwndFrame = hwndFrame;
      dcd -> hwndClient = hwndClient;
      if (flags & 4)
	dcd -> dontclose = TRUE;
      {
	PFNWP oldproc;

	oldproc = WinSubclassWindow(hwndFrame,
				    (PFNWP) CollectorFrameWndProc);
	WinSetWindowPtr(hwndFrame,
			QWL_USER,
			(PVOID) oldproc);
      }
      dcd -> hwndCnr = WinCreateWindow(hwndClient,
				       WC_CONTAINER,
				       NULL,
				       CCS_AUTOPOSITION | CCS_MINIICONS |
				       CCS_MINIRECORDCORE | ulCnrType |
				       WS_VISIBLE,
				       0,
				       0,
				       0,
				       0,
				       hwndClient,
				       HWND_TOP,
				       (ULONG) COLLECTOR_CNR,
				       NULL,
				       NULL);
      if (dcd -> hwndCnr)
      {
	Collector = dcd -> hwndCnr;
	WinSetWindowPtr(dcd -> hwndCnr, QWL_USER, (PVOID) dcd);
	WinSetWindowText(hwndFrame,
			 GetPString(IDS_COLLECTORTITLETEXT));
	if (FrameFlags & FCF_MENU)
	{
	  if (!fToolbar)
	  {
	    HWND hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);

	    if (hwndMenu)
	    {
	      WinSendMsg(hwndMenu,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_SEEALL, FALSE),
			 MPVOID);
	      WinSendMsg(hwndMenu,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_GREP, FALSE),
			 MPVOID);
	      WinSendMsg(hwndMenu,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_CLEARCNR, FALSE),
			 MPVOID);
	      WinSendMsg(hwndMenu,
			 MM_DELETEITEM,
			 MPFROM2SHORT(IDM_REMOVE, FALSE),
			 MPVOID);
	    }
	  }
	}
	dcd -> oldproc = WinSubclassWindow(dcd -> hwndCnr,
					   (PFNWP) CollectorCnrWndProc);
	{
	  USHORT ids[] =
	  {DIR_TOTALS, DIR_SELECTED, DIR_VIEW, DIR_SORT,
	   DIR_FILTER, 0};

	  CommonCreateTextChildren(dcd -> hwndClient,
				   GetPString(IDS_WCCOLSTATUS),
				   ids);
	}
	if (FrameFlags & FCF_SHELLPOSITION)
	  PostMsg(hwndClient,
		  UM_SIZE,
		  MPVOID,
		  MPVOID);
	if (!PostMsg(dcd -> hwndCnr,
		     UM_SETUP,
		     MPVOID,
		     MPVOID))
	  WinSendMsg(dcd -> hwndCnr,
		     UM_SETUP,
		     MPVOID,
		     MPVOID);
      }
      else
      {
	PostMsg(hwndClient,
		WM_CLOSE,
		MPVOID,
		MPVOID);
	free(dcd);
	hwndFrame = (HWND) 0;
      }
    }
    else
    {
      PostMsg(hwndClient,
	      WM_CLOSE,
	      MPVOID,
	      MPVOID);
      hwndFrame = (HWND) 0;
    }
  }
  return hwndFrame;
}
