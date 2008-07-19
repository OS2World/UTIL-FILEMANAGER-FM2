
/***********************************************************************

  $Id$

  fm/2 starter

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync
  18 Jul 08 SHL Add Fortify support

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "dll\tools.h"
#include "dll\version.h"
#include "dll\errutil.h"
#include "dll\fortify.h"
#include "dll\fm3dll.h"

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  HWND hwndFrame;

  strcpy(appname, "FM/3");
  DosError(FERR_DISABLEHARDERR);
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
#	  endif
	}
      }
      DosSleep(250L);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
