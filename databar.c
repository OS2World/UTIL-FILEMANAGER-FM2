
/***********************************************************************

  $Id$

  databar applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2008 Steven H. Levine

  15 Oct 02 SHL Baseline
  07 Dec 05 SHL Avoid warnings
  14 Dec 08 SHL Add exception handler support
  14 Dec 08 SHL Drop NEVER used code

***********************************************************************/

#include <string.h>

#define INCL_WIN
#define INCL_LONGLONG
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\notebook.h"		// appname
#include "dll\datamin.h"
#include "dll\fm3dlg.h"
#include "dll\init.h"			// InitFM3DLL
#include "dll\errutil.h"		// Error reporting
#include "dll\excputil.h"		// Exception handlers

static PSZ pszSrcFile = __FILE__;

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  APIRET regRet;
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };

  strcpy(appname, "DATABAR");

  regRec.ExceptionHandler = HandleException;
  regRet = DosSetExceptionHandler(&regRec);
  if (regRet != NO_ERROR) {
    DbgMsg(pszSrcFile, __LINE__,
	   "DosSetExceptionHandler failed with error %u", regRet);
  }

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 384);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
	if (CreateDataBar(HWND_DESKTOP, 0)) {
	  while (WinGetMsg(hab, &qmsg, (HWND) 0, 0, 0))
	    WinDispatchMsg(hab, &qmsg);
	}
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
