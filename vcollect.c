
/***********************************************************************

  $Id$

  Collector applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2007, 2008 Steven H.Levine

  05 Jan 08 SHL Sync

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN

#include "dll\fm3dll.h"
#include "dll\fm3dll2.h"		// #define's for UM_*, control id's, etc.
#include "dll\mainwnd.h"		// Data declaration(s)
#include "dll\notebook.h"		// Data declaration(s)
#include "dll\fm3str.h"
#include "dll\init.h"			// InitFM3DLL
#include "dll\collect.h"		// StartCollector

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  HWND hwndFrame;
  int x;
  BOOL seekandscan = FALSE;

  strcpy(appname, "VCOLLECT");
  DosError(FERR_DISABLEHARDERR);
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
