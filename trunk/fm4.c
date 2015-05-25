
/***********************************************************************

  $Id$

  fm/2 lite applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync
  14 Dec 08 SHL Add exception handler support

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// hwndBubble
#include "dll\notebook.h"		// appname
#include "dll\tools.h"
#include "dll\version.h"
#include "dll\mainwnd2.h"		// StartFM32
#include "dll\init.h"			// InitFM3DLL
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
  UINT x;
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };

  strcpy(appname, "FM/4");
  DosError(FERR_DISABLEHARDERR);

  regRec.ExceptionHandler = HandleException;
  regRet = DosSetExceptionHandler(&regRec);
  if (regRet != NO_ERROR) {
    DbgMsg(pszSrcFile, __LINE__,
	   "DosSetExceptionHandler failed with error %u", regRet);
  }

  for (x = 1; x < argc; x++) {
    if (*argv[x] == '+' && !argv[x][1])
      fLogFile = TRUE;
    if (*argv[x] == '-') {
      if (argv[x][1])
	strcpy(profile, &argv[x][1]);
    }
  }

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 2048);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
	if (CheckVersion(VERMAJOR, VERMINOR)) {
	  hwndFrame = StartFM32(hab, argc, argv);
	  if (hwndFrame != (HWND) 0) {
	    for (;;) {
	      if (!WinGetMsg(hab, &qmsg, (HWND) 0, 0, 0)) {
		if (qmsg.hwnd)
		  qmsg.msg = WM_CLOSE;
		else
		  break;
	      }
	      if (hwndBubble &&
		  ((qmsg.msg > (WM_BUTTON1DOWN - 1) &&
		    qmsg.msg < (WM_BUTTON3DBLCLK + 1)) ||
		   (qmsg.msg > (WM_CHORD - 1) &&
		    qmsg.msg < (WM_BUTTON3CLICK + 1))) &&
		  WinIsWindowVisible(hwndBubble))
		WinShowWindow(hwndBubble, FALSE);
	      WinDispatchMsg(hab, &qmsg);
	    }
	    if (WinIsWindow(hab, WinWindowFromID(hwndFrame, FID_CLIENT)))
	      WinSendMsg(WinWindowFromID(hwndFrame,
					 FID_CLIENT),
			 WM_CLOSE, MPVOID, MPVOID);
	  }
	}
      }
      DosSleep(250L);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
