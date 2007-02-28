#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dll\fm3dll.h"
#include "dll\fm3dlg.h"

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
