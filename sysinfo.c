
/***********************************************************************

  $Id$

  System information applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H.Levine

  05 Jan 08 SHL Sync

***********************************************************************/

#define INCL_DOS
#define INCL_WIN

#include "dll\fm3dll.h"
#include "dll\fm3dlg.h"
#include "dll\init.h"			// InitFM3DLL
#include "dll\sysinfo.h"                        // SysInfoDlgProc

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 256);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
        WinDlgBox(HWND_DESKTOP, HWND_DESKTOP,
                  SysInfoDlgProc, FM3ModHandle, SYS_FRAME, NULL);
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
