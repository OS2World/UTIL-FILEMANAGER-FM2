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


int main (int argc,char *argv[]) {

  HAB         hab;
  HMQ         hmq;

  hab = WinInitialize(0);
  if(hab) {
    hmq = WinCreateMsgQueue(hab,256);
    if(hmq) {
      if(InitFM3DLL(hab,argc,argv)) {
        WinDlgBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  ViewInfProc,
                  FM3ModHandle,
                  VINF_FRAME,((argc > 1) ? MPFROMP("") : MPVOID));
      }
      DosSleep(250L);
Abort:
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}

