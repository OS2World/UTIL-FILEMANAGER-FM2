
/***********************************************************************

  $Id$

  databar applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2008 Steven H. Levine

  Oct 15 21:42:56 2002 967 ______ databar.c

  15 Oct 02 SHL Baseline
  07 Dec 05 SHL Avoid warnings

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "dll\fm3dll.h"
#include "dll\datamin.h"
#include "dll\fm3dlg.h"
#include "dll\init.h"			// InitFM3DLL

#ifdef NEVER				// 05 Jan 08 SHL fixme to be gone?

VOID APIENTRY deinit(ULONG why)
{
  if (fmprof)
    PrfCloseProfile(fmprof);
  fmprof = (HINI) 0;

  flushall();

  DosExitList(EXLST_REMOVE, deinit);
}

#endif

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;

  strcpy(appname, "DATABAR");

# ifdef NEVER
  DosExitList(EXLST_ADD, deinit);
# endif

  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 384);
    if (hmq) {
      if (InitFM3DLL(hab, argc, argv)) {
	if (CreateDataBar(HWND_DESKTOP, 0)) {
	  while (WinGetMsg(hab, &qmsg, (HWND) 0, 0, 0))
	    WinDispatchMsg(hab, &qmsg);
	}
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
