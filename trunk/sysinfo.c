
/***********************************************************************

  $Id$

  System information applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync
  14 Dec 08 SHL Add exception handler support

***********************************************************************/

#define INCL_WIN
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// FM3ModHandle
#include "dll\fm3dlg.h"
#include "dll\init.h"			// InitFM3DLL
#include "dll\sysinfo.h"		// SysInfoDlgProc
#include "dll\errutil.h"		// Error reporting
#include "dll\excputil.h"		// Exception handlers

static PSZ pszSrcFile = __FILE__;

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  APIRET regRet;
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };

  regRec.ExceptionHandler = HandleException;
  regRet = DosSetExceptionHandler(&regRec);
  if (regRet != NO_ERROR) {
    DbgMsg(pszSrcFile, __LINE__,
	   "DosSetExceptionHandler failed with error %u", regRet);
  }

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 256);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
	WinDlgBox(HWND_DESKTOP, HWND_DESKTOP,
		  SysInfoDlgProc, FM3ModHandle, SYS_FRAME, NULL);
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
