
/***********************************************************************

  $Id$

  Collector applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2007, 2008 Steven H.Levine

  05 Jan 08 SHL Sync
  14 Dec 08 SHL Add exception handler support

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\fm3dll2.h"		// IDM_GREP
#include "dll\mainwnd.h"		// Data declaration(s)
#include "dll\notebook.h"		// Data declaration(s)
#include "dll\fm3str.h"
#include "dll\init.h"			// InitFM3DLL
#include "dll\collect.h"		// StartCollector
#include "dll\errutil.h"		// Error reporting
#include "dll\excputil.h"		// Exception handlers

static PSZ pszSrcFile = __FILE__;

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  HWND hwndFrame;
  UINT x;
  BOOL seekandscan = FALSE;
  APIRET regRet;
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };

  strcpy(appname, "VCOLLECT");
  DosError(FERR_DISABLEHARDERR);

  regRec.ExceptionHandler = HandleException;
  regRet = DosSetExceptionHandler(&regRec);
  if (regRet != NO_ERROR) {
    DbgMsg(pszSrcFile, __LINE__,
	   "DosSetExceptionHandler failed with error %u", regRet);
  }

  for (x = 1; x < argc; x++) {
    if (*argv[x] == '*' && argv[x][1] == '*') {
      seekandscan = TRUE;
      break;
    }
  }

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 1024);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
	hwndFrame = StartCollector(HWND_DESKTOP, 0);
	if (hwndFrame) {
	  if (hwndHelp)
	    WinAssociateHelpInstance(hwndHelp, hwndFrame);
	  if (seekandscan)
	    WinPostMsg(WinWindowFromID(hwndFrame, FID_CLIENT), WM_COMMAND,
		       MPFROM2SHORT(IDM_GREP, 0), MPVOID);
	  while (WinGetMsg(hab, &qmsg, (HWND) 0, 0, 0)) {
	    if (hwndBubble &&
		((qmsg.msg > (WM_BUTTON1DOWN - 1) &&
		  qmsg.msg < (WM_BUTTON3DBLCLK + 1)) ||
		 (qmsg.msg > (WM_CHORD - 1) &&
		  qmsg.msg < (WM_BUTTON3CLICK + 1))) &&
		WinIsWindowVisible(hwndBubble))
	      WinShowWindow(hwndBubble, FALSE);
	    WinDispatchMsg(hab, &qmsg);
	  }
	  DosSleep(125L);
	}
      }
      DosSleep(125L);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
