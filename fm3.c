
/***********************************************************************

  $Id$

  fm/2 starter

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2015 Steven H.Levine

  05 Jan 08 SHL Sync
  18 Jul 08 SHL Add Fortify support
  11 Dec 08 SHL Add exception handler support
  08 Aug 15 SHL Comments

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// hwndBubble
#include "dll\version.h"		// VER...
#include "dll\init.h"			// StartFM3
#include "dll\notebook.h"		// appname
#include "dll\fortify.h"
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

  strcpy(appname, "FM/3");
  DosError(FERR_DISABLEHARDERR);

  regRec.ExceptionHandler = HandleException;
  regRet = DosSetExceptionHandler(&regRec);
  if (regRet != NO_ERROR) {
#if 0 // 10 Dec 08 SHL fixme to report later maybe?
    Dos_Error(MB_ENTER, regRet, HWND_DESKTOP, pszSrcFile, __LINE__,
	      "DosSetExceptionHandler");
#endif
    DbgMsg(pszSrcFile, __LINE__,
	   "DosSetExceptionHandler failed with error %u", regRet);
  }

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 2048);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
	if (CheckVersion(VERMAJOR, VERMINOR)) {
#	  ifdef FORTIFY
	  Fortify_EnterScope();
#	  endif
	  hwndFrame = StartFM3(hab, argc, argv);
	  if (hwndFrame != (HWND)0) {
	    for (;;) {
	      if (!WinGetMsg(hab, &qmsg, (HWND) 0, 0, 0)) {
		// Get WM_QUIT
		if (!qmsg.hwnd)
		  break;		// Time to die if no window
		// Map quit to close
		// Must use System Menu or Ctrl-Alt-F4 accel to shutdown
		qmsg.msg = WM_CLOSE;
	      }
	      if (hwndBubble &&
		  ((qmsg.msg > WM_BUTTON1DOWN - 1 &&
		    qmsg.msg < WM_BUTTON3DBLCLK + 1) ||
		   (qmsg.msg > WM_CHORD - 1 &&
		    qmsg.msg < WM_BUTTON3CLICK + 1)) &&
		  WinIsWindowVisible(hwndBubble))
	      {
		WinShowWindow(hwndBubble, FALSE);	// Hide
	      }
	      WinDispatchMsg(hab, &qmsg);
	    } // for
	    // Time to die
	    if (WinIsWindow(hab, WinWindowFromID(hwndFrame, FID_CLIENT)))
	      WinSendMsg(WinWindowFromID(hwndFrame, FID_CLIENT), WM_CLOSE,
			 MPVOID, MPVOID);
	  }
#	  ifdef FORTIFY
	  for (;;) {
	    UCHAR scope = Fortify_LeaveScope();
	    if ((CHAR)scope == 0)
	      break;
	    Runtime_Error(__FILE__, __LINE__, "Attempting to exit thread with scope non-zero (%u)", scope);
	    if ((CHAR)scope < 0)
	      break;
	  }
	  Fortify_DumpAllMemory();
#	  endif
	}
      }
      DosSleep(250L);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }

  if (regRet == NO_ERROR) {
    regRet = DosUnsetExceptionHandler(&regRec);
    if (regRet != NO_ERROR) {
      DbgMsg(pszSrcFile, __LINE__,
	     "DosUnsetExceptionHandler failed with error %u", regRet);
    }
  }
  return 0;
}
