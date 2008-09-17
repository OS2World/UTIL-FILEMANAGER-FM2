
/***********************************************************************

  $Id$

  Archive viewer applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2007, 2008 Steven H. Levine

  23 Sep 07 SHL Sync with standards
  23 Sep 07 SHL Get rid of statics
  17 Sep 08 JBS Convert to use of wrapped DosFind... (i.e. xDosFind...)

***********************************************************************/

#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// Data declaration(s)
#include "dll\init.h"			// Data declaration(s)
#include "dll\notebook.h"		// Data declaration(s)
#include "dll\arccnrs.h"
#include "dll\fm3str.h"
#include "dll\version.h"
#include "dll\arccnrs.h"
#include "dll\assoc.h"			// ExecAssociation
#include "dll\defview.h"		// ShowMultimedia
#include "dll\inis.h"			// StartIniEditor
#include "dll\dirs.h"			// switch_to
#include "dll\viewer.h"			// StartMLEEditor
#include "dll\getnames.h"			// insert_filename
#include "dll\copyf.h"			// unlinkf
#include "dll\init.h"			// InitFM3DLL
#include "dll\valid.h"			// IsFile
#include "dll\wrappers.h"		// xDosFind...

HMTX av2Sem;

VOID APIENTRY deinit(ULONG why)
{
  /* cleanup before exiting */

  DosCloseMutexSem(av2Sem);
  if (DosOpenMutexSem("\\SEM32\\AV2", &av2Sem)) {
    CHAR s[CCHMAXPATH];			// 23 Sep 07 SHL
    CHAR *enddir;
    HDIR search_handle;
    ULONG num_matches;
    FILEFINDBUF3 ffb3;

    save_dir(s);
    if (s[strlen(s) - 1] != '\\')
      strcat(s, "\\");
    enddir = &s[strlen(s)];
    if (*ArcTempRoot) {
      strcat(s, ArcTempRoot);
      strcat(s, "*");
      search_handle = HDIR_CREATE;
      num_matches = 1;
      if (!xDosFindFirst(s,
			&search_handle,
			FILE_NORMAL | FILE_DIRECTORY | FILE_SYSTEM |
			FILE_READONLY | FILE_HIDDEN | FILE_ARCHIVED,
			&ffb3,
			sizeof(ffb3),
			&num_matches,
			FIL_STANDARD)) {
	do {
	  strcpy(enddir, ffb3.achName);
	  if (ffb3.attrFile & FILE_DIRECTORY) {
	    wipeallf("%s\\*", s);
	    DosDeleteDir(s);
	  }
	  else
	    unlinkf("%s", s);
	} while (!xDosFindNext(search_handle,
			      &ffb3, sizeof(FILEFINDBUF3), &num_matches, FIL_STANDARD));
	DosFindClose(search_handle);
      }
    }
  }
  else
    DosCloseMutexSem(av2Sem);

  DosExitList(EXLST_REMOVE, deinit);
}

int main(int argc, char *argv[])
{
  HAB hab;
  HMQ hmq;
  QMSG qmsg;
  HWND hwndFrame = (HWND) 0;
  static CHAR fullname[CCHMAXPATH];	// 23 Sep 07 SHL fixme to not be static
  CHAR *thisarg = NULL;
  INT x;

  *fullname = 0;
  strcpy(appname, "AV/2");
  fAmAV2 = TRUE;
  DosError(FERR_DISABLEHARDERR);
  for (x = 1; x < argc; x++) {
    if (!strchr("/;,`\'", *argv[x]) &&
	!thisarg &&
	(IsFile(argv[x]) == 1 ||
	 (strchr(argv[x], '?') || strchr(argv[x], '*') ||
	  !strchr(argv[x], '.')))) {
      thisarg = argv[x];
      break;
    }
  }
  DosExitList(EXLST_ADD, deinit);
  if (DosOpenMutexSem("\\SEM32\\AV2", &av2Sem))
    DosCreateMutexSem("\\SEM32\\AV2", &av2Sem, DC_SEM_SHARED, FALSE);
  if (thisarg) {
    if (DosQueryPathInfo(thisarg,
			 FIL_QUERYFULLNAME, fullname, sizeof(fullname)))
      strcpy(fullname, thisarg);
    if (*fullname && (strchr(fullname, '?') ||
		      strchr(fullname, '*') || !strchr(fullname, '.'))) {

      FILEFINDBUF3 ffb3;
      ULONG nm;
      HDIR hdir;
      CHAR *enddir;

      if (!strchr(fullname, '.'))
	strcat(fullname, ".*");
      enddir = strrchr(fullname, '\\');
      if (enddir) {
	enddir++;
	hdir = HDIR_CREATE;
	nm = 1;
	if (!DosFindFirst(fullname,
			  &hdir,
			  FILE_NORMAL | FILE_SYSTEM |
			  FILE_READONLY | FILE_HIDDEN | FILE_ARCHIVED,
			  &ffb3, sizeof(FILEFINDBUF3), &nm, FIL_STANDARD)) {
	  strcpy(enddir, ffb3.achName);
	  DosFindClose(hdir);
	}
      }
    }
  }
  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab, 1024);
    if (hmq) {
      {
	CHAR path[CCHMAXPATH];
	CHAR *env;
	FILESTATUS3 fs;

	env = getenv("FM3INI");
	if (env && *env) {
	  DosError(FERR_DISABLEHARDERR);
	  if (!DosQueryPathInfo(env, FIL_QUERYFULLNAME, path, sizeof(path))) {
	    DosError(FERR_DISABLEHARDERR);
	    if (!DosQueryPathInfo(path, FIL_STANDARD, &fs, sizeof(fs))) {
	      if (!(fs.attrFile & FILE_DIRECTORY)) {
		env = strrchr(path, '\\');
		if (env)
		  *env = 0;
	      }
	      DosError(FERR_DISABLEHARDERR);
	      if (!DosQueryPathInfo(path, FIL_STANDARD, &fs, sizeof(fs))) {
		if (fs.attrFile & FILE_DIRECTORY)
		  switch_to(path);
	      }
	    }
	  }
	}
      }
      if (InitFM3DLL(hab, argc, argv)) {
	if (CheckVersion(VERMAJOR, VERMINOR)) {
	  fAmAV2 = TRUE;
	  if (!*fullname) {
	    strcpy(fullname, "*");
	    if (!insert_filename(HWND_DESKTOP,
				 fullname,
				 TRUE,
				 FALSE) || !*fullname || *fullname == '*')
	      goto Abort;
	  }
	  if (*fullname) {
	    if (ExecAssociation(HWND_DESKTOP, fullname) == -1) {
	      hwndFrame = StartArcCnr(HWND_DESKTOP,
				      (HWND) 0, fullname, 0, NULL);
	      if (!hwndFrame) {

		CHAR *p = strrchr(fullname, '.');

		if (p) {
		  if (!stricmp(p, ".INI"))
		    hwndFrame = StartIniEditor(HWND_DESKTOP, fullname, 0);
		}
		if (!ShowMultimedia(fullname))
		  hwndFrame = StartMLEEditor(HWND_DESKTOP,
					     1,
					     ((*fullname) ?
					      fullname : NULL), (HWND) 0);
	      }
	      if (hwndFrame && WinIsWindow(hab, hwndFrame)) {
		if (hwndHelp)
		  WinAssociateHelpInstance(hwndHelp, hwndFrame);
		for (;;) {
		  if (!WinGetMsg(hab, &qmsg, (HWND) 0, 0, 0)) {
		    if (!WinIsWindow(hab, hwndFrame))
		      break;
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
		DosSleep(125);
	      }
	    }
	  }
	}
      }
    Abort:
      DosSleep(125);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  return 0;
}
