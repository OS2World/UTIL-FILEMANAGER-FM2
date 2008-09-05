
/***********************************************************************

  $Id$

  See all files applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN

#include "dll\fm3dll.h"
#include "dll\seeall.h"                 // StartSeeAll
#include "dll\fm3str.h"

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  HWND hwndFrame;
  static CHAR fullname[CCHMAXPATH];
  INT x;

  *fullname = 0;
  strcpy(appname, "SEEALL");
  DosError(FERR_DISABLEHARDERR);
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
              *fullname = 0;
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
