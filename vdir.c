#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dll\fm3dll.h"


int main (int argc,char *argv[]) {

  HAB   hab;
  HMQ   hmq;
  QMSG  qmsg;
  HWND  hwndFrame;
  CHAR  fullname[CCHMAXPATH],*thisarg = NULL;
  INT   x;

  strcpy(appname,"VDIR");
  DosError(FERR_DISABLEHARDERR);
  for(x = 1;x < argc;x++) {
    if(!strchr("/;,`\'",*argv[x]) &&
       (IsRoot(argv[x]) ||
        !IsFile(argv[x]))) {
      thisarg = argv[x];
      break;
    }
  }

  if(thisarg) {
    if(DosQueryPathInfo(thisarg,
                        FIL_QUERYFULLNAME,
                        fullname,
                        sizeof(fullname)))
      strcpy(fullname,thisarg);
  }
  else
    save_dir(fullname);
  hab = WinInitialize(0);
  if(hab) {
    hmq = WinCreateMsgQueue(hab,
                            1024);
    if(hmq) {
      if(InitFM3DLL(hab,argc,argv)) {
        hwndFrame = StartDirCnr(HWND_DESKTOP,
                                fullname,
                                (HWND)0,
                                0);
        if(hwndFrame) {
          if(hwndHelp)
            WinAssociateHelpInstance(hwndHelp,
                                     hwndFrame);
          for(;;) {
            if(!WinGetMsg(hab,
                          &qmsg,
                          (HWND)0,
                          0,
                          0)) {
              if(qmsg.hwnd)
                qmsg.msg = WM_CLOSE;
              else
                break;
            }
            if(hwndBubble &&
               ((qmsg.msg > (WM_BUTTON1DOWN - 1) &&
                 qmsg.msg < (WM_BUTTON3DBLCLK + 1)) ||
                (qmsg.msg > (WM_CHORD - 1) &&
                 qmsg.msg < (WM_BUTTON3CLICK + 1))) &&
               WinIsWindowVisible(hwndBubble))
              WinShowWindow(hwndBubble,
                            FALSE);
            WinDispatchMsg(hab,
                           &qmsg);
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

