
/***********************************************************************

  $Id$

  File undelete applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2007 Steven H. Levine

  23 Sep 07 SHL Sync with standards
  23 Sep 07 SHL Get rid of statics

***********************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include "dll\fm3dll.h"
#include "dll\fm3dlg.h"

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  FILESTATUS3 fs;
  CHAR fullname[CCHMAXPATH];
  CHAR *thisarg = NULL;
  INT x;

  DosError(FERR_DISABLEHARDERR);
  for (x = 1; x < argc; x++) {
    if (!strchr("/;,`\'", *argv[x]) && !thisarg) {
      thisarg = argv[x];
      break;
    }
  }
  if (!thisarg) {
    thisarg = fullname;
    save_dir(fullname);
  }
  DosError(FERR_DISABLEHARDERR);
  if (thisarg && !DosQueryPathInfo(thisarg, FIL_STANDARD, &fs, sizeof(fs))) {
    if (DosQueryPathInfo(thisarg,
			 FIL_QUERYFULLNAME, fullname, sizeof(fullname)))
      strcpy(fullname, thisarg);
    hab = WinInitialize(0);
    if (hab) {
      hmq = WinCreateMsgQueue(hab, 256);
      if (hmq) {
	if (InitFM3DLL(hab, argc, argv)) {
	  MakeValidDir(fullname);
	  WinDlgBox(HWND_DESKTOP,
		    HWND_DESKTOP,
		    UndeleteDlgProc,
		    FM3ModHandle,
		    UNDEL_FRAME,
		    fullname);
	}
	DosSleep(250);
	WinDestroyMsgQueue(hmq);
      }
      WinTerminate(hab);
    }
  }
  else
    DosBeep(250, 100);
  return 0;
}
