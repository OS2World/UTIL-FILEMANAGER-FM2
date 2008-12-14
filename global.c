
/***********************************************************************

  $Id$

  See all files applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync
  14 Dec 08 SHL Add exception handler support

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// hwndBubble
#include "dll\notebook.h"		// appname
#include "dll\fm3str.h"
#include "dll\seeall.h"			// StartSeeAll
#include "dll\valid.h"			// IsFile
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
  CHAR fullname[CCHMAXPATH] = { 0 };	// 14 Dec 08 SHL was static
  UINT x;
  APIRET regRet;
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };

  strcpy(appname, "SEEALL");
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
	  if (!strchr("/;,`\'", *argv[x]) && !*fullname &&
	      (IsRoot(argv[x]) || IsFile(argv[x]) == 0)) {
	    if (IsRoot(argv[x]))
	      strcpy(fullname, argv[x]);
	    else if (DosQueryPathInfo(argv[x],
				      FIL_QUERYFULLNAME,
				      fullname, sizeof(fullname)))
	      *fullname = 0;		// Forget name
	  }
	}
	hwndFrame = StartSeeAll(HWND_DESKTOP, TRUE, fullname);
	if (hwndFrame) {
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
