
/***********************************************************************

  $Id$

  Tree viewer applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync

***********************************************************************/

#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// Data declaration(s)
#include "dll\notebook.h"		// Data declaration(s)
#include "dll\init.h"			// InitFM3DLL
#include "dll\treecnr.h"                // StartTreeCnr

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  INT x;
  BOOL startminimized = FALSE;

  strcpy(appname, "VTREE");
  DosError(FERR_DISABLEHARDERR);
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
