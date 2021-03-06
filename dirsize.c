
/***********************************************************************

  $Id$

  Directory sizes applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2008 Steven H.Levine

  11 Jun 02 SHL Baseline
  06 Jan 04 SHL Total drives >4GB better
  08 Jul 08 SHL Avoid WARNALL warning
  14 Dec 08 SHL Add exception handler support
  14 Dec 08 SHL Drop NEVER used code

***********************************************************************/

#include <string.h>
#include <ctype.h>			// toupper

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\fm3dll2.h"		// #define's for UM_*, control id's, etc.
#include "dll\info.h"			// driveflags
#include "dll\mainwnd.h"		// FM3ModHandle
#include "dll\fm3dlg.h"
#include "dll\valid.h"			// CheckDrive
#include "dll\init.h"			// InitFM3DLL
#include "dll\dirsize.h"		// DirSizeProc
#include "dll\errutil.h"		// Error reporting
#include "dll\excputil.h"		// Exception handlers

static PSZ pszSrcFile = __FILE__;

MRESULT EXPENTRY DirMainProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static CHAR curdir[4];

  switch (msg) {
  case WM_INITDLG:
    *curdir = 0;
    WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case UM_UNDO:
    {
      ULONG x;
      ULONG ulDriveMap;
      ULONG ulDriveNum;
      CHAR dirname[] = " :\\";
      BOOL first = TRUE;

      WinSendDlgItemMsg(hwnd, DIRSIZE_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);

      DosError(FERR_DISABLEHARDERR);
      DosQCurDisk(&ulDriveNum, &ulDriveMap);

      for (x = 2; x < 26; x++) {
	if ((ulDriveMap & (1L << x)) && !(driveflags[x] & DRIVE_IGNORE)) {
	  *dirname = (CHAR) x + 'A';
	  WinSendDlgItemMsg(hwnd, DIRSIZE_LISTBOX, LM_INSERTITEM,
			    MPFROM2SHORT(LIT_END, 0), MPFROMP(dirname));
	  if (first) {
	    WinSendDlgItemMsg(hwnd, DIRSIZE_LISTBOX, LM_SELECTITEM,
			      MPFROMSHORT(0), MPFROMSHORT(TRUE));
	    first = FALSE;
	  }
	}
      }
    }
    return 0;

  case UM_RESCAN:
    {
      CHAR FileSystem[CCHMAXPATH];
      CHAR s[CCHMAXPATH * 2];
      FSALLOCATE fsa;
      ULONG type;
      USHORT percentused;
      USHORT percentfree;
      struct
      {
	ULONG serial;
	CHAR volumelength;
	CHAR volumelabel[CCHMAXPATH];
      }
      volser;
      INT removable;

      WinSetDlgItemText(hwnd, DIRSIZE_LABEL, "");
      WinSetDlgItemText(hwnd, DIRSIZE_UNITSFREE, "");
      WinSetDlgItemText(hwnd, DIRSIZE_UNITSUSED, "");
      WinSetDlgItemText(hwnd, DIRSIZE_UNITSIZE, "");
      WinSetDlgItemText(hwnd, DIRSIZE_PERCENT, "");
      WinSetDlgItemText(hwnd, DIRSIZE_BYTESUSED, "");
      WinSetDlgItemText(hwnd, DIRSIZE_BYTESFREE, "");
      WinSetDlgItemText(hwnd, DIRSIZE_IFS, "");
      WinSetDlgItemText(hwnd, DIRSIZE_LOCAL, "");
      WinSendDlgItemMsg(hwnd, DIRSIZE_SLIDER, SLM_SETSLIDERINFO,
			MPFROM2SHORT(SMA_SLIDERARMPOSITION,
				     SMA_INCREMENTVALUE), MPFROMSHORT(0));
      removable = CheckDrive(toupper(*curdir), FileSystem, &type);
      if (removable != -1) {
	if (type & DRIVE_ZIPSTREAM)
	  WinSetDlgItemText(hwnd, DIRSIZE_LOCAL, "Zipstream drive");
	else if (type & DRIVE_REMOTE)
	  WinSetDlgItemText(hwnd, DIRSIZE_LOCAL, "Remote drive");
	else if (type & DRIVE_VIRTUAL)
	    WinSetDlgItemText(hwnd, DIRSIZE_LOCAL, "Virtual drive");
	else if (type & DRIVE_RAMDISK)
	  WinSetDlgItemText(hwnd, DIRSIZE_LOCAL, "Ramdisk");
	else {
	  sprintf(s, "Local drive%s", (removable) ? " (removable)" : "");
	  WinSetDlgItemText(hwnd, DIRSIZE_LOCAL, s);
	}
	sprintf(s, "IFS:  %s", FileSystem);
	WinSetDlgItemText(hwnd, DIRSIZE_IFS, s);
	memset(&volser, 0, sizeof(volser));
	DosError(FERR_DISABLEHARDERR);
	if (!DosQueryFSInfo(toupper(*curdir) - '@', FSIL_VOLSER,
			    &volser, (ULONG) sizeof(volser))) {
	  sprintf(s, "Label:  %s", volser.volumelabel);
	  WinSetDlgItemText(hwnd, DIRSIZE_LABEL, s);
	}
	if (!DosQueryFSInfo(toupper(*curdir) - '@',
			    FSIL_ALLOC, &fsa, sizeof(FSALLOCATE))) {
	  percentfree = fsa.cUnit ? (USHORT)((fsa.cUnitAvail * 100) / fsa.cUnit) : 0;	// 27 May 08 SHL
	  if (!percentfree && fsa.cUnitAvail)
	    percentfree = 1;
	  percentused = 100 - percentfree;
	  sprintf(s, "Units free:  %lu", fsa.cUnitAvail);
	  WinSetDlgItemText(hwnd, DIRSIZE_UNITSFREE, s);
	  sprintf(s, "Unit size:  %lu x %u = %lu",
		  fsa.cSectorUnit,
		  fsa.cbSector, fsa.cSectorUnit * fsa.cbSector);
	  WinSetDlgItemText(hwnd, DIRSIZE_UNITSIZE, s);
	  sprintf(s, "Units used:  %lu", fsa.cUnit - fsa.cUnitAvail);
	  WinSetDlgItemText(hwnd, DIRSIZE_UNITSUSED, s);
	  sprintf(s, "Bytes free:  %.0f",
		  (float)fsa.cUnitAvail * (fsa.cSectorUnit * fsa.cbSector));
	  WinSetDlgItemText(hwnd, DIRSIZE_BYTESFREE, s);
	  sprintf(s, "Bytes used:  %.0f",
		  (float)(fsa.cUnit - fsa.cUnitAvail) *
		  (fsa.cSectorUnit * fsa.cbSector));
	  WinSetDlgItemText(hwnd, DIRSIZE_BYTESUSED, s);
	  sprintf(s, "Percent used:  %u%%", percentused);
	  WinSetDlgItemText(hwnd, DIRSIZE_PERCENT, s);
	  WinSendDlgItemMsg(hwnd, DIRSIZE_SLIDER, SLM_SETSLIDERINFO,
			    MPFROM2SHORT(SMA_SLIDERARMPOSITION,
					 SMA_INCREMENTVALUE),
			    MPFROMSHORT(percentused));
	  WinShowWindow(WinWindowFromID(hwnd, DIRSIZE_SLIDER), TRUE);
	}
      }
      else {
	WinSetDlgItemText(hwnd, DIRSIZE_LOCAL, "Drive not ready.");
	WinShowWindow(WinWindowFromID(hwnd, DIRSIZE_SLIDER), FALSE);
      }
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case DIRSIZE_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
	WinPostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      case LN_SELECT:
	{
	  SHORT x;

	  x = (SHORT) WinSendDlgItemMsg(hwnd, DIRSIZE_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (x >= 0) {
	    WinSendDlgItemMsg(hwnd, DIRSIZE_LISTBOX,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(x, sizeof(curdir)),
			      MPFROMP(curdir));
	    WinPostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	  }
	}
	break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case DID_OK:
      if (*curdir) {
	WinSetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_HIDE);
	WinDlgBox(HWND_DESKTOP, hwnd,
		  DirSizeProc, FM3ModHandle, DSZ_FRAME, curdir);
	WinSetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOW);
      }
      else
	DosBeep(50, 100);
      break;
    }
    return 0;
  } // switch
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  CHAR fullname[CCHMAXPATH] = { 0 };	// 14 Dec 08 SHL was static
  UINT x;
  ULONG rcl;
  APIRET regRet;
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };

  DosError(FERR_DISABLEHARDERR);

  regRec.ExceptionHandler = HandleException;
  regRet = DosSetExceptionHandler(&regRec);
  if (regRet != NO_ERROR) {
    DbgMsg(pszSrcFile, __LINE__,
	   "DosSetExceptionHandler failed with error %u", regRet);
  }

  for (x = 1; x < argc; x++) {
    if (!strchr("/;,`\'", *argv[x]) &&
	!*fullname && (IsRoot(argv[x]) || IsFile(argv[x]) == 0)) {
      if (IsRoot(argv[x]))
	strcpy(fullname, argv[x]);
      else if (DosQueryPathInfo(argv[x],
				FIL_QUERYFULLNAME,
				fullname, sizeof(fullname)))
	*fullname = 0;			// Forget name
    }
  }

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 384);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
	if (!*fullname)
	  rcl = WinDlgBox(HWND_DESKTOP,
			  HWND_DESKTOP, DirMainProc, 0, DIRSIZE_FRAME, NULL);
	else
	  rcl = WinDlgBox(HWND_DESKTOP,
			  HWND_DESKTOP,
			  DirSizeProc, FM3ModHandle, DSZ_FRAME, fullname);
	if (rcl == DID_ERROR)
	  rcl = WinGetLastError(hab);
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;

} // main
