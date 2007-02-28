#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <direct.h>
#include "dll\fm3dll.h"
#include "dll\tools.h"
#include "dll\version.h"

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
	}
      }
      DosSleep(250L);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
