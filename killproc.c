
/***********************************************************************

  $Id$

  Process killer applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync

***********************************************************************/

#define INCL_DOS
#define INCL_WIN

#include "dll\fm3dll.h"
#include "dll\fm3dlg.h"
#include "dll\init.h"			// InitFM3DLL
#include "dll\killproc.h"                       // KillDlgProc

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;

  DosError(FERR_DISABLEHARDERR);
  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 256);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
        WinDlgBox(HWND_DESKTOP,
                  HWND_DESKTOP, KillDlgProc, FM3ModHandle, KILL_FRAME, NULL);
      }
      DosSleep(250L);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
