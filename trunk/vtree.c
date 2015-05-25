
/***********************************************************************

  $Id$

  Tree viewer applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync
  14 Dec 08 SHL Add exception handler support

***********************************************************************/

#include <stdlib.h>			// min
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// hwndTree hwndBubble
#include "dll\notebook.h"		// appname
#include "dll\init.h"			// InitFM3DLL
#include "dll\treecnr.h"		// StartTreeCnr
#include "dll\errutil.h"		// Error reporting
#include "dll\excputil.h"		// Exception handlers

static PSZ pszSrcFile = __FILE__;

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  UINT x;
  BOOL startminimized = FALSE;
  APIRET regRet;
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };

  strcpy(appname, "VTREE");
  DosError(FERR_DISABLEHARDERR);

  regRec.ExceptionHandler = HandleException;
  regRet = DosSetExceptionHandler(&regRec);
  if (regRet != NO_ERROR) {
    DbgMsg(pszSrcFile, __LINE__,
	   "DosSetExceptionHandler failed with error %u", regRet);
  }

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 1024);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
	for (x = 1; x < argc; x++) {
	  if (*argv[x] == '~') {
	    startminimized = TRUE;
	    break;
	  }
	}
	hwndTree = StartTreeCnr(HWND_DESKTOP, 0);
	if (hwndTree) {
	  if (hwndHelp)
	    WinAssociateHelpInstance(hwndHelp, hwndTree);
	  if (!WinRestoreWindowPos("FM/2", "VTreeWindowPos", hwndTree)) {

	    SWP swp;
	    ULONG adjust;

	    adjust = WinQuerySysValue(HWND_DESKTOP, SV_CXICON) * 8L;
	    WinQueryTaskSizePos(hab, 0L, &swp);
	    swp.cx = min(swp.cx, adjust);
	    WinSetWindowPos(hwndTree, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
			    SWP_SHOW | SWP_MOVE | SWP_SIZE | SWP_ZORDER |
			    SWP_ACTIVATE);
	  }
	  if (startminimized)
	    WinSetWindowPos(hwndTree, HWND_TOP, 0, 0, 0, 0, SWP_MINIMIZE);
	  else
	    WinSetWindowPos(hwndTree, HWND_TOP, 0, 0, 0, 0,
			    SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE);
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
	}
      }
      DosSleep(125);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
