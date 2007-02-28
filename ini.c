#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dll\fm3dll.h"

int main(int argc, char *argv[])
{

  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  HWND hwndFrame;

  DosError(FERR_DISABLEHARDERR);
  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 512);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv) &&
	  ((hwndFrame =
	    StartIniEditor(HWND_DESKTOP, argv[1], 0)) != (HWND) 0)) {
	if (hwndHelp)
	  WinAssociateHelpInstance(hwndHelp, hwndFrame);
	while (WinGetMsg(hab, &qmsg, (HWND) 0, 0, 0))
	  WinDispatchMsg(hab, &qmsg);
      }
      DosSleep(125L);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
