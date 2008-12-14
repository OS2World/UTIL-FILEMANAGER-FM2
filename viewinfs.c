
/***********************************************************************

  $Id$

  INF viewer applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2007, 2008 Steven H.Levine

  03 Aug 07 SHL Minor cleanup
  14 Dec 08 SHL Add exception handler support

***********************************************************************/

#define INCL_DOS			// DosSleep
#define INCL_WIN
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\notebook.h"		// appname
#include "dll\mainwnd.h"		// FM3ModHandle
#include "dll\fm3dlg.h"
#include "dll\init.h"			// InitFM3DLL
#include "dll\viewinf.h"		// ViewInfProc
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
	WinDlgBox(HWND_DESKTOP,
		  HWND_DESKTOP,
		  ViewInfProc,
		  FM3ModHandle,
		  VINF_FRAME, ((argc > 1) ? MPFROMP("") : MPVOID));
      }
      DosSleep(250);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
