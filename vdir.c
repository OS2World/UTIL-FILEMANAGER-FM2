
/***********************************************************************

  $Id$

  INF directory viewer applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2007, 2008 Steven H.Levine

  05 Jan 08 SHL Sync

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// Data declaration(s)
#include "dll\notebook.h"		// Data declaration(s)
#include "dll\init.h"			// InitFM3DLL
#include "dll\valid.h"			// IsFile
#include "dll\dirs.h"			// save_dir

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  HWND hwndFrame;
  CHAR fullname[CCHMAXPATH], *thisarg = NULL;
  INT x;

  strcpy(appname, "VDIR");
  DosError(FERR_DISABLEHARDERR);
  for (x = 1; x < argc; x++) {
    if (!strchr("/;,`\'", *argv[x]) && (IsRoot(argv[x]) || !IsFile(argv[x]))) {
      thisarg = argv[x];
      break;
    }
  }

  if (thisarg) {
    if (DosQueryPathInfo(thisarg,
			 FIL_QUERYFULLNAME, fullname, sizeof(fullname)))
      strcpy(fullname, thisarg);
  }
  else
    save_dir(fullname);
  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 1024);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
	hwndFrame = StartDirCnr(HWND_DESKTOP, fullname, (HWND) 0, 0);
	if (hwndFrame) {
	  if (hwndHelp)
	    WinAssociateHelpInstance(hwndHelp, hwndFrame);
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
      DosSleep(125L);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
