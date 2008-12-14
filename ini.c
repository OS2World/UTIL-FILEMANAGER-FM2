
/***********************************************************************

  $Id$

  Ini view/edit applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync
  14 Dec 08 SHL Add exception handler support

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\notebook.h"		// hwndHelp
#include "dll\init.h"			// InitFM3DLL
#include "dll\inis.h"                   // StartIniEditor
#include "dll\errutil.h"		// Error reporting
#include "dll\excputil.h"		// Exception handlers

static PSZ pszSrcFile = __FILE__;

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  HWND hwndFrame;
  APIRET regRet;
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };

  DosError(FERR_DISABLEHARDERR);

  regRec.ExceptionHandler = HandleException;
  regRet = DosSetExceptionHandler(&regRec);
  if (regRet != NO_ERROR) {
    DbgMsg(pszSrcFile, __LINE__,
	   "DosSetExceptionHandler failed with error %u", regRet);
  }

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 512);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv) &&
          ((hwndFrame =
            StartIniEditor(HWND_DESKTOP, argv[1], 0)) != (HWND) 0)) {
        if (hwndHelp)
          WinAssociateHelpInstance(hwndHelp, hwndFrame);
        while (WinGetMsg(hab, &qmsg, (HWND) 0, 0, 0))
          WinDispatchMsg(hab, &qmsg);
      }
      DosSleep(125L);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
