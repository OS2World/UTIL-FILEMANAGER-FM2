
/***********************************************************************

  $Id$

  Directory containers

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006 Steven H. Levine

  23 Aug 06 SHL Comments
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <ctype.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA1)

MRESULT EXPENTRY CustomFileDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
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
	  ulSearchCount = 1L;
	  if (!IsRoot(info->path))
	    rc = xDosFindFirst(info->path, &hDir, FILE_DIRECTORY |
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
	      WinSendDlgItemMsg(hwnd, FDLG_USERDIRS, LM_INSERTITEM,
				MPFROM2SHORT(LIT_SORTASCENDING, 0),
				MPFROMP(info->path));
	    else {
	      temp = info->next;
	      remove_udir(info->path);
	      info = temp;
	      continue;
	    }
	  }
	  else if (!(ulDriveMap & (1L << (toupper(*info->path) - 'A')))) {
	    temp = info->next;
	    remove_udir(info->path);
	    info = temp;
	    continue;
	  }
	}
	info = info->next;
      }
    }
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, FDLG_HELP),
			(HPS) 0, FALSE, TRUE);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case 260:				/* drives dropdown list */
      switch (SHORT2FROMMP(mp1)) {
      case CBN_SHOWLIST:
	WinSetDlgItemText(hwnd,
			  FDLG_HELP, GetPString(IDS_CLICKDRIVEHELPTEXT));
	break;
      }
      break;

    case 258:				/* name entry field */
      switch (SHORT2FROMMP(mp1)) {
      case EN_SETFOCUS:
	WinSetDlgItemText(hwnd,
			  FDLG_HELP, GetPString(IDS_ENTERFILEORMASKHELPTEXT));
	break;
      case EN_KILLFOCUS:
	WinSetDlgItemText(hwnd, FDLG_HELP, GetPString(IDS_NAMEDEFHELPTEXT));
	break;
      }
      break;

    case 264:				/* dirs listbox */
      switch (SHORT2FROMMP(mp1)) {
      case LN_SETFOCUS:
	WinSetDlgItemText(hwnd,
			  FDLG_HELP, GetPString(IDS_DBLCLKDIRSWITCHHELPTEXT));
	break;
      case LN_KILLFOCUS:
	WinSetDlgItemText(hwnd, FDLG_HELP, GetPString(IDS_NAMEDEFHELPTEXT));
	break;
      }
      break;

    case 266:				/* files listbox */
      switch (SHORT2FROMMP(mp1)) {
      case LN_SETFOCUS:
	WinSetDlgItemText(hwnd, FDLG_HELP,
			  GetPString(IDS_DBLCLKFILEUSEHELPTEXT));
	break;
      case LN_KILLFOCUS:
	WinSetDlgItemText(hwnd, FDLG_HELP, GetPString(IDS_NAMEDEFHELPTEXT));
	break;
      }
      break;

    case FDLG_USERDIRS:
      switch (SHORT2FROMMP(mp1)) {
      case CBN_SHOWLIST:
	WinSetDlgItemText(hwnd,
			  FDLG_HELP, GetPString(IDS_DBLCLKDIRSWITCHHELPTEXT));
	break;
      case CBN_ENTER:
	{
	  SHORT sSelect;
	  CHAR szBuffer[CCHMAXPATH], szTemp[CCHMAXPATH], *p;

	  sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
					       FDLG_USERDIRS,
					       LM_QUERYSELECTION,
					       MPVOID, MPVOID);
	  *szBuffer = 0;
	  if (sSelect >= 0) {
	    WinSendDlgItemMsg(hwnd,
			      FDLG_USERDIRS,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect,
					   CCHMAXPATH), MPFROMP(szBuffer));
	    if (*szBuffer) {
	      if (szBuffer[strlen(szBuffer) - 1] != '\\')
		strcat(szBuffer, "\\");
	      *szTemp = 0;
	      WinQueryDlgItemText(hwnd, 258, CCHMAXPATH, szTemp);
	      p = strrchr(szTemp, '\\');
	      if (!p)
		p = szTemp;
	      else
		p++;
	      if (*p)
		strcat(szBuffer, p);
	      if (!strchr(szBuffer, '?') && !strchr(szBuffer, '*'))
		strcat(szBuffer, "*");
	      WinSetDlgItemText(hwnd, 258, szBuffer);
	      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	    }
	  }
	}
	break;
      default:
	break;
      }
      break;
    }
    break;

  case WM_HELP:
    if (hwndHelp) {
      WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		 MPFROM2SHORT(HELP_FILEDLG, 0), MPFROMSHORT(HM_RESOURCEID));
      return 0;
    }
    break;
  }
  return WinDefFileDlgProc(hwnd, msg, mp1, mp2);
}

BOOL insert_filename(HWND hwnd, CHAR * filename, INT loadit, BOOL newok)
{
  FILEDLG fdlg;
  FILESTATUS3 fsa;
  CHAR drive[3], *pdrive = drive, *p;
  APIRET rc;
  static CHAR lastfilename[CCHMAXPATH] = "";

  if (!filename)
    return FALSE;
  memset(&fdlg, 0, sizeof(FILEDLG));
  fdlg.cbSize = (ULONG) sizeof(FILEDLG);
  fdlg.fl = FDS_CENTER | FDS_OPEN_DIALOG;
  if (!loadit) {
    fdlg.pszTitle = GetPString(IDS_ENTERFILEINSERTTEXT);
    fdlg.pszOKButton = GetPString(IDS_INSERTTEXT);
  }
  else if (loadit == TRUE) {
    fdlg.pszTitle = GetPString(IDS_ENTERFILELOADTEXT);
    fdlg.pszOKButton = GetPString(IDS_LOADTEXT);
  }
  else {
    fdlg.pszTitle = GetPString(IDS_ENTERFILETEXT);
    fdlg.pszOKButton = GetPString(IDS_OKAYTEXT);
  }
  if (IsFullName(filename)) {
    *drive = *filename;
    drive[1] = ':';
    drive[2] = 0;
    fdlg.pszIDrive = pdrive;
  }
  else if (*lastfilename) {
    *drive = *lastfilename;
    drive[1] = ':';
    drive[2] = 0;
    fdlg.pszIDrive = pdrive;
  }

  if (!*filename) {
    if (*lastfilename) {
      strcpy(fdlg.szFullFile, lastfilename);
      p = strrchr(fdlg.szFullFile, '\\');
      if (p) {
	p++;
	*p = 0;
      }
    }
    if (!loadit || loadit == TRUE)
      strcat(fdlg.szFullFile, "*.TXT");
    else
      strcat(fdlg.szFullFile, "*");
  }
  else
    strcpy(fdlg.szFullFile, filename);

  if (fCustomFileDlg) {
    fdlg.fl |= FDS_HELPBUTTON | FDS_CUSTOM;
    fdlg.pfnDlgProc = (PFNWP) CustomFileDlg;
    fdlg.hMod = FM3ModHandle;
    fdlg.usDlgId = FDLG_FRAME;
  }

  if (WinFileDlg(HWND_DESKTOP, hwnd, &fdlg)) {
    if (fdlg.lReturn != DID_CANCEL && !fdlg.lSRC)
      strcpy(filename, fdlg.szFullFile);
    else
      return FALSE;
  }
  else
    return FALSE;
  MakeFullName(filename);
  if (!DosQueryPathInfo(filename, FIL_STANDARD, &fsa, sizeof(fsa))) {
    if (fsa.attrFile & FILE_DIRECTORY) {
      /* device or directory */
      saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
	     hwnd, filename, GetPString(IDS_EXISTSBUTNOTFILETEXT), filename);
      return FALSE;
    }
    else if (fsa.cbFile == 0L) {
      saymsg(MB_CANCEL,
	     hwnd, filename, GetPString(IDS_ISZEROLENGTHTEXT), filename);
      return FALSE;
    }
  }
  else if (!newok) {
    saymsg(MB_CANCEL,
	   hwnd, filename, GetPString(IDS_DOESNTEXISTTEXT), filename);
    return FALSE;
  }
  else {
    rc = saymsg(MB_YESNOCANCEL,
		hwnd, filename, GetPString(IDS_CREATENEWTEXT));
    if (rc != MBID_YES)
      return FALSE;
  }
  p = filename;
  while (*p) {
    if (*p == '/')
      *p = '\\';
    p++;
  }
  if (*filename)
    strcpy(lastfilename, filename);
  return TRUE;
}

BOOL export_filename(HWND hwnd, CHAR * filename, INT overwrite)
{
  FILEDLG fdlg;
  FILESTATUS3 fsa;
  CHAR drive[3], *pdrive = drive, *p;
  static CHAR lastfilename[CCHMAXPATH] = "";

  if (!filename)
    return FALSE;
  memset(&fdlg, 0, sizeof(FILEDLG));
  fdlg.cbSize = sizeof(FILEDLG);
  fdlg.fl = FDS_CENTER | FDS_OPEN_DIALOG;
  fdlg.pszTitle = GetPString(IDS_EXPORTNAMETITLETEXT);
  fdlg.pszOKButton = GetPString(IDS_OKAYTEXT);
  if (IsFullName(filename)) {
    *drive = *filename;
    drive[1] = ':';
    drive[2] = 0;
    fdlg.pszIDrive = pdrive;
  }
  else if (*lastfilename) {
    *drive = *lastfilename;
    drive[1] = ':';
    drive[2] = 0;
    fdlg.pszIDrive = pdrive;
  }
  if (!*filename) {
    if (*lastfilename) {
      strcpy(fdlg.szFullFile, lastfilename);
      p = strrchr(fdlg.szFullFile, '\\');
      if (p) {
	p++;
	*p = 0;
      }
    }
    strcat(fdlg.szFullFile, "*.TXT");
  }
  else
    strcpy(fdlg.szFullFile, filename);

  if (fCustomFileDlg) {
    fdlg.fl |= FDS_HELPBUTTON | FDS_CUSTOM;
    fdlg.pfnDlgProc = (PFNWP) CustomFileDlg;
    fdlg.hMod = FM3ModHandle;
    fdlg.usDlgId = FDLG_FRAME;
  }

  if (WinFileDlg(HWND_DESKTOP, hwnd, &fdlg)) {
    if (fdlg.lReturn != DID_CANCEL && !fdlg.lSRC)
      strcpy(filename, fdlg.szFullFile);
    else
      return FALSE;
  }
  else
    return FALSE;
  MakeFullName(filename);
  if (!DosQueryPathInfo(filename, FIL_STANDARD, &fsa, sizeof(fsa))) {
    if (fsa.attrFile & FILE_DIRECTORY) {	/* device or directory */
      saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
	     hwnd, filename, GetPString(IDS_EXISTSBUTNOTFILETEXT), filename);
      return FALSE;
    }
    else if (overwrite && fsa.cbFile != 0L) {
      if (saymsg(MB_YESNO,
		 hwnd,
		 filename,
		 GetPString(IDS_EXISTSERASETEXT), filename) == MBID_YES)
	unlinkf("%s", filename);
    }
  }
  p = filename;
  while (*p) {
    if (*p == '/')
      *p = '\\';
    p++;
  }
  if (*filename)
    strcpy(lastfilename, filename);
  return TRUE;
}

#pragma alloc_text(GETNAMES,insert_filename,export_filename,CustomFileDlg)
