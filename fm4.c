
/***********************************************************************

  $Id$

  fm/2 lite applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "dll\fm3dll.h"
#include "dll\tools.h"
#include "dll\version.h"
#include "dll\mainwnd2.h"			// StartFM32
#include "dll\init.h"			// InitFM3DLL

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  HWND hwndFrame;

  strcpy(appname, "FM/4");
  {
    INT x;

    for (x = 1; x < argc; x++) {
      if (*argv[x] == '+' && !argv[x][1])
	fLogFile = TRUE;
      if (*argv[x] == '-') {
	if (argv[x][1])
	  strcpy(profile, &argv[x][1]);
      }
    }
  }
  DosError(FERR_DISABLEHARDERR);
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
