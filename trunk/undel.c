
/***********************************************************************

  $Id$

  File undelete applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2007, 2008 Steven H. Levine

  23 Sep 07 SHL Sync with standards
  23 Sep 07 SHL Get rid of statics
  14 Dec 08 SHL Add exception handler support

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// FM3ModHandle
#include "dll\fm3dlg.h"
#include "dll\undel.h"			// UndeleteDlgProc
#include "dll\init.h"			// InitFM3DLL
#include "dll\valid.h"			// MakeValidDir
#include "dll\dirs.h"			// save_dir
#include "dll\errutil.h"		// Error reporting
#include "dll\excputil.h"		// Exception handlers

static PSZ pszSrcFile = __FILE__;

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  FILESTATUS3 fs;
  CHAR fullname[CCHMAXPATH];
  PSZ thisarg = NULL;
  UINT x;
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
    if (!strchr("/;,`\'", *argv[x]) && !thisarg) {
      thisarg = argv[x];
      break;
    }
  }
  if (!thisarg) {
    thisarg = fullname;
    save_dir(fullname);
  }
  DosError(FERR_DISABLEHARDERR);
  if (thisarg && !DosQueryPathInfo(thisarg, FIL_STANDARD, &fs, sizeof(fs))) {
    if (DosQueryPathInfo(thisarg,
			 FIL_QUERYFULLNAME, fullname, sizeof(fullname)))
      strcpy(fullname, thisarg);
    hab = WinInitialize(0);
    if (hab) {
      hmq = WinCreateMsgQueue(hab, 256);
      if (hmq) {
	if (InitFM3DLL(hab, argc, argv)) {
	  MakeValidDir(fullname);
	  WinDlgBox(HWND_DESKTOP,
		    HWND_DESKTOP,
		    UndeleteDlgProc,
		    FM3ModHandle,
		    UNDEL_FRAME,
		    fullname);
	}
	DosSleep(250);
	WinDestroyMsgQueue(hmq);
      }
      WinTerminate(hab);
    }
  }
  else
    DosBeep(250, 100);
  return 0;
}
