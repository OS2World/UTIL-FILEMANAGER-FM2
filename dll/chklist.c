
/***********************************************************************

  $Id$

  Utility windows and mouse positioning

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2008 Steven H.Levine

  10 Jan 05 SHL Allow DND_TARGET to hold CCHMAXPATH
  14 Jul 06 SHL Use Runtime_Error
  22 Mar 07 GKY Use QWL_USER
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG

#include "fm3dlg.h"
#include "fm3str.h"
#include "makelist.h"			// AddToList
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "getnames.h"			// CustomFileDlg
#include "chklist.h"
#include "info.h"			// FileInfoProc
#include "defview.h"			// QuickView
#include "valid.h"			// IsExecutable
#include "fm3dll.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

VOID CenterOverWindow(HWND hwnd)
{
  SWP swp;
  POINTL ptl;

  if (!fDontMoveMouse) {
    if (WinQueryWindowPos(hwnd, &swp)) {
      ptl.x = swp.x + (swp.cx / 2);
      ptl.y = swp.y + (swp.cy / 2);
      WinMapWindowPoints(WinQueryWindow(hwnd, QW_PARENT), HWND_DESKTOP, &ptl,
			 1L);
      WinSetPointerPos(HWND_DESKTOP, ptl.x, ptl.y);
    }
  }
}

BOOL PopupMenu(HWND hwndParent, HWND hwndOwner, HWND hwndMenu)
{
  POINTL ptl;
  BOOL rc;

  if (!WinQueryPointerPos(HWND_DESKTOP, &ptl))
    ptl.x = ptl.y = 32;
  WinMapWindowPoints(HWND_DESKTOP, hwndParent, &ptl, 1L);
  rc = WinPopupMenu(hwndParent, hwndOwner, hwndMenu,
		    ptl.x, ptl.y, 0,
		    PU_HCONSTRAIN | PU_VCONSTRAIN |
		    PU_KEYBOARD | PU_MOUSEBUTTON1);
  if (rc)
    CenterOverWindow(hwndMenu);
  return rc;
}

VOID PosOverOkay(HWND hwnd)
{
  SWP swp;
  POINTL ptl;

  if (!fDontMoveMouse) {
    if (WinQueryWindowPos(WinWindowFromID(hwnd, DID_OK), &swp)) {
      ptl.x = swp.x + (swp.cx / 2);
      ptl.y = swp.y + (swp.cy / 2);
      WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl, 1L);
      WinSetPointerPos(HWND_DESKTOP, ptl.x, ptl.y);
    }
  }
}

MRESULT EXPENTRY CheckListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CHECKLIST *cl;

  switch (msg) {
  case WM_INITDLG:
    if (mp2) {

      SHORT x;

      WinSetWindowPtr(hwnd, QWL_USER, (PVOID) mp2);
      cl = (CHECKLIST *) mp2;
      if (!cl->list || !cl->list[0]) {
	WinDismissDlg(hwnd, 0);
	break;
      }
      WinSetDlgItemText(hwnd, CHECK_PROMPT, cl->prompt);
      for (x = 0; cl->list[x]; x++) {
	WinSendDlgItemMsg(hwnd, CHECK_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0), MPFROMP(cl->list[x]));
	WinSendDlgItemMsg(hwnd, CHECK_LISTBOX, LM_SELECTITEM,
			  MPFROM2SHORT(x, 0), MPFROMSHORT(TRUE));
      }
      PosOverOkay(hwnd);
      WinEnableWindow(WinWindowFromID(hwnd, CHECK_INFO),
		      ((cl->flags & CHECK_FILES) != 0));
      WinShowWindow(WinWindowFromID(hwnd, CHECK_INFO),
		    ((cl->flags & CHECK_FILES) != 0));
      {
	HBITMAP hbm, hbmd, hbmdd;
	HPS hps;

	hps = WinGetPS(WinWindowFromID(hwnd, CHECK_BITMAP));
	hbm = GpiLoadBitmap(hps, 0, cl->cmd, 28, 28);
	if (hbm) {
	  hbmd =
	    (HBITMAP) WinSendDlgItemMsg(hwnd, CHECK_BITMAP, SM_QUERYHANDLE,
					MPVOID, MPVOID);
	  hbmdd =
	    (HBITMAP) WinSendDlgItemMsg(hwnd, CHECK_BITMAP, SM_SETHANDLE,
					MPFROMLONG(hbm), MPVOID);
	  if (hbmdd && hbmd && hbmd != hbmdd)
	    GpiDeleteBitmap(hbmd);
	}
	if (hps)
	  WinReleasePS(hps);
	WinSetWindowPos(WinWindowFromID(hwnd, CHECK_BITMAP), HWND_TOP,
			0, 0, 28, 28, SWP_SIZE);
      }
    }
    else
      WinDismissDlg(hwnd, 0);
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, CHECK_HELP), (HPS) 0, FALSE,
			TRUE);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CHECK_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
	{
	  SHORT x;
	  CHAR szBuffer[CCHMAXPATH];

	  x = (SHORT) WinSendDlgItemMsg(hwnd, CHECK_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (x >= 0) {
	    *szBuffer = 0;
	    WinSendDlgItemMsg(hwnd, CHECK_LISTBOX, LM_QUERYITEMTEXT,
			      MPFROM2SHORT(x, CCHMAXPATH), MPFROMP(szBuffer));
	    if (*szBuffer)
	      QuickView(hwnd, szBuffer);
	  }
	}
	break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case CHECK_INFO:
      cl = INSTDATA(hwnd);
      if (cl)
	WinDlgBox(HWND_DESKTOP, hwnd, FileInfoProc, FM3ModHandle,
		  FLE_FRAME, (PVOID) cl->list);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_CHECKLIST, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_OK:
      cl = INSTDATA(hwnd);
      if (cl) {

	CHAR szBuffer[CCHMAXPATH + 1];
	UINT numfiles = 0, numalloc = 0;
	INT error;
	SHORT x;

	FreeList(cl->list);
	cl->list = NULL;
	x = (SHORT) WinSendDlgItemMsg(hwnd, CHECK_LISTBOX, LM_QUERYSELECTION,
				      MPFROMSHORT(LIT_FIRST), MPVOID);
	if (x >= 0) {
	  do {
	    *szBuffer = 0;
	    WinSendDlgItemMsg(hwnd, CHECK_LISTBOX, LM_QUERYITEMTEXT,
			      MPFROM2SHORT(x, CCHMAXPATH), MPFROMP(szBuffer));
	    if (*szBuffer) {
	      error = AddToList(szBuffer, &cl->list, &numfiles, &numalloc);
	      if (error) {
		Runtime_Error(pszSrcFile, __LINE__, "AddToList");
		break;
	      }
	    }
	    x = (SHORT) WinSendDlgItemMsg(hwnd, CHECK_LISTBOX,
					  LM_QUERYSELECTION,
					  MPFROMSHORT(x), MPVOID);
	  } while (x >= 0);
	}
	WinDismissDlg(hwnd, 1);
      }
      else
	WinDismissDlg(hwnd, 0);
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY DropListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  CHECKLIST *cl;
  static BOOL Shadow = FALSE, Wild = FALSE;

  switch (msg) {
  case WM_INITDLG:
    if (mp2) {

      SHORT x;

      WinSetWindowPtr(hwnd, QWL_USER, (PVOID) mp2);
      cl = (CHECKLIST *) mp2;
      if (!cl->list || !cl->list[0]) {
	WinDismissDlg(hwnd, 0);
	break;
      }
      if (IsRoot(cl->list[0])) {
	WinDismissDlg(hwnd, DND_COMPARE);
	break;
      }
      WinSendDlgItemMsg(hwnd, DND_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
      for (x = 0; cl->list[x]; x++) {
	WinSendDlgItemMsg(hwnd, DND_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0), MPFROMP(cl->list[x]));
	WinSendDlgItemMsg(hwnd, DND_LISTBOX, LM_SELECTITEM,
			  MPFROM2SHORT(x, 0), MPFROMSHORT(TRUE));
      }
      WinSendDlgItemMsg(hwnd, DND_TARGET, EM_SETTEXTLIMIT,
			(MPARAM) CCHMAXPATH, (MPARAM) 0);
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    }
    else
      WinDismissDlg(hwnd, 0);
    break;

  case UM_UNDO:
    cl = INSTDATA(hwnd);
    if (cl) {

      CHAR *p;

      WinSetDlgItemText(hwnd, DND_TARGET, cl->prompt);
      WinEnableWindow(WinWindowFromID(hwnd, DND_LAUNCH),
		      (IsExecutable(cl->prompt)));
      WinEnableWindow(WinWindowFromID(hwnd, DND_COPY), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, DND_MOVE), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, DND_RENAME), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, DND_OBJECT), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, DND_SHADOW), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, DND_EXTRACT), TRUE);
      WinEnableWindow(WinWindowFromID(hwnd, DND_SETICON), FALSE);
      p = strrchr(cl->list[0], '.');
      if (p) {
	p++;
	if (!stricmp(p, "ICO") || !stricmp(p, "PTR"))
	  WinEnableWindow(WinWindowFromID(hwnd, DND_SETICON), TRUE);
      }
      if (IsFile(cl->prompt)) {
	WinEnableWindow(WinWindowFromID(hwnd, DND_OBJECT), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, DND_SHADOW), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, DND_EXTRACT), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, DND_RENAME), FALSE);
      }
      else {

	ARC_TYPE *info;

	info = find_type(cl->list[0], NULL);
	if (!info)
	  WinEnableWindow(WinWindowFromID(hwnd, DND_EXTRACT), FALSE);
      }
      if (Shadow)
	WinCheckButton(hwnd, DND_SHADOW, TRUE);
      if (Wild)
	WinCheckButton(hwnd, DND_RENAME, TRUE);
      PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      if (cl->prompt && isalpha(*cl->prompt) &&
	  (driveflags[toupper(*cl->prompt) - 'A'] & DRIVE_NOTWRITEABLE)) {
	WinEnableWindow(WinWindowFromID(hwnd, DND_COPY), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, DND_MOVE), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, DND_RENAME), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, DND_OBJECT), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, DND_SHADOW), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, DND_EXTRACT), FALSE);
      }
      if (cl->prompt && IsFile(cl->prompt) == 1) {

	ARC_TYPE *info;

	info = find_type(cl->prompt, NULL);
	if (!info) {
	  WinEnableWindow(WinWindowFromID(hwnd, DND_COPY), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DND_MOVE), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DND_RENAME), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DND_OBJECT), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DND_SHADOW), FALSE);
	  WinEnableWindow(WinWindowFromID(hwnd, DND_EXTRACT), FALSE);
	}
      }
    }
    break;

  case UM_SETUP:
    cl = WinQueryWindowPtr(hwnd, QWL_USER);
    if (cl) {
      if (cl->flags == DO_MOVE)
	WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, DND_MOVE));
      else if (cl->flags == DO_LINK)
	WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, DND_COMPARE));
      else
	WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, DND_COPY));
    }
    return 0;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, DND_HELP), (HPS) 0, FALSE,
			TRUE);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case DND_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_SETFOCUS:
	WinSetDlgItemText(hwnd, DND_HELP, GetPString(IDS_DNDLISTBOXHELPTEXT));
	break;
      case LN_KILLFOCUS:
	WinSetDlgItemText(hwnd, DND_HELP, GetPString(IDS_DNDHELPTEXT));
	break;
      case LN_ENTER:
	{
	  SHORT x;
	  CHAR szBuffer[CCHMAXPATH];

	  x = (SHORT) WinSendDlgItemMsg(hwnd, DND_LISTBOX, LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (x >= 0) {
	    *szBuffer = 0;
	    WinSendDlgItemMsg(hwnd, DND_LISTBOX, LM_QUERYITEMTEXT,
			      MPFROM2SHORT(x, CCHMAXPATH), MPFROMP(szBuffer));
	    if (*szBuffer)
	      QuickView(hwnd, szBuffer);
	  }
	}
	break;
      }
      break;
    case DND_TARGET:
      switch (SHORT2FROMMP(mp1)) {
      case EN_SETFOCUS:
	WinSetDlgItemText(hwnd, DND_HELP, GetPString(IDS_DNDTARGETHELPTEXT));
	break;
      case EN_KILLFOCUS:
	WinSetDlgItemText(hwnd, DND_HELP, GetPString(IDS_DNDHELPTEXT));
	break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_DNDDLG, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DND_INFO:
      cl = INSTDATA(hwnd);
      if (cl)
	WinDlgBox(HWND_DESKTOP, hwnd, FileInfoProc, FM3ModHandle,
		  FLE_FRAME, (PVOID) cl->list);
      break;

    case DND_CHANGETARGET:
      cl = INSTDATA(hwnd);
      if (cl) {
	if (cl->prompt && *cl->prompt) {
	  if (!IsFile(cl->prompt)) {

	    CHAR newpath[CCHMAXPATH];

	    strcpy(newpath, cl->prompt);
	    if (WinDlgBox(HWND_DESKTOP, hwnd, WalkAllDlgProc,
			  FM3ModHandle, WALK_FRAME,
			  MPFROMP(newpath)) && *newpath) {
	      strcpy(cl->prompt, newpath);
	      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
	    }
	  }
	  else {

	    FILEDLG fdlg;
	    FILESTATUS3 fs3;
	    CHAR drive[3], *pdrive = drive, filename[CCHMAXPATH], *p;

	    memset(&fdlg, 0, sizeof(FILEDLG));
	    fdlg.cbSize = (ULONG) sizeof(FILEDLG);
	    fdlg.fl = FDS_HELPBUTTON | FDS_CENTER |
	      FDS_OPEN_DIALOG | FDS_CUSTOM;
	    fdlg.pszTitle = GetPString(IDS_NEWTARGETTEXT);
	    fdlg.pszOKButton = GetPString(IDS_CHANGETEXT);
	    *drive = *cl->prompt;
	    drive[1] = ':';
	    drive[2] = 0;
	    fdlg.pszIDrive = pdrive;
	    strcpy(fdlg.szFullFile, cl->prompt);
	    p = strrchr(fdlg.szFullFile, '\\');
	    if (p)
	      *p = 0;
	    strcat(fdlg.szFullFile, "\\*");

	    fdlg.pfnDlgProc = (PFNWP) CustomFileDlg;
	    fdlg.hMod = FM3ModHandle;
	    fdlg.usDlgId = FDLG_FRAME;

	    if (WinFileDlg(HWND_DESKTOP, hwnd, &fdlg)) {
	      if (fdlg.lReturn != DID_CANCEL && !fdlg.lSRC) {
		if (!DosQueryPathInfo(fdlg.szFullFile,
				      FIL_QUERYFULLNAME,
				      filename, sizeof(filename))) {
		  if (!DosQueryPathInfo(filename,
					FIL_STANDARD, &fs3, sizeof(fs3))) {
		    p = filename;
		    while (*p) {
		      if (*p == '/')
			*p = '\\';
		      p++;
		    }
		    strcpy(cl->prompt, filename);
		    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
		  }
// else saymsg(MB_ENTER,hwnd,DEBUG_STRING,"DosQueryPathInfo FIL_STANDARDL failed");
		}
// else saymsg(MB_ENTER,hwnd,DEBUG_STRING,"DosQueryPathInfo FIL_QUERYFULLNAME failed");
	      }
// else saymsg(MB_ENTER,hwnd,DEBUG_STRING,"lReturn = %lu lSRC = %lu",fdlg.lReturn,fdlg.lSRC);
	    }
// else saymsg(MB_ENTER,hwnd,DEBUG_STRING,"WinFileDlg failed");
	  }
	}
      }
      break;

    case DND_TARGETINFO:
      cl = INSTDATA(hwnd);
      if (cl) {

	CHAR *list[2];

	list[0] = cl->prompt;
	list[1] = NULL;
	WinDlgBox(HWND_DESKTOP, hwnd, FileInfoProc, FM3ModHandle,
		  FLE_FRAME, (PVOID) & list);
      }
      break;

    case DND_EXTRACT:
    case DND_LAUNCH:
    case DND_COPY:
    case DND_MOVE:
    case DND_OBJECT:
    case DND_COMPARE:
    case DND_SETICON:
    case DND_APPEND:
      cl = INSTDATA(hwnd);
      if (cl) {

	SHORT x;
	CHAR szBuffer[CCHMAXPATH + 1];
	UINT numfiles = 0, numalloc = 0;
	INT error;
	USHORT cmd;

	FreeList(cl->list);
	cl->list = NULL;
	x = (SHORT) WinSendDlgItemMsg(hwnd, DND_LISTBOX, LM_QUERYSELECTION,
				      MPFROMSHORT(LIT_FIRST), MPVOID);
	if (x >= 0) {
	  do {
	    *szBuffer = 0;
	    WinSendDlgItemMsg(hwnd, DND_LISTBOX, LM_QUERYITEMTEXT,
			      MPFROM2SHORT(x, CCHMAXPATH), MPFROMP(szBuffer));
	    if (*szBuffer) {
	      error = AddToList(szBuffer, &cl->list, &numfiles, &numalloc);
	      if (error) {
		Runtime_Error(pszSrcFile, __LINE__, "AddToList");
		break;
	      }
	    }
	    x = (SHORT) WinSendDlgItemMsg(hwnd, DND_LISTBOX,
					  LM_QUERYSELECTION,
					  MPFROMSHORT(x), MPVOID);
	  } while (x >= 0);
	}
	cmd = SHORT1FROMMP(mp1);
	switch (cmd) {
	case DND_COPY:
	  if (WinQueryButtonCheckstate(hwnd, DND_RENAME)) {
	    Wild = TRUE;
	    cmd = DND_WILDCOPY;
	  }
	  else
	    Wild = FALSE;
	  break;
	case DND_MOVE:
	  if (WinQueryButtonCheckstate(hwnd, DND_RENAME)) {
	    Wild = TRUE;
	    cmd = DND_WILDMOVE;
	  }
	  else
	    Wild = FALSE;
	  break;
	case DND_OBJECT:
	  if (WinQueryButtonCheckstate(hwnd, DND_SHADOW)) {
	    Shadow = TRUE;
	    cmd = DND_SHADOW;
	  }
	  else
	    Shadow = FALSE;
	  break;
	}
	WinDismissDlg(hwnd, cmd);
      }
      else
	WinDismissDlg(hwnd, 0);
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(CHECKLIST,CheckListProc)
#pragma alloc_text(DNDLIST,DropListProc)
#pragma alloc_text(MISC7,PosOverOkay,CenterOverWindow,PopupMenu)
