
/***********************************************************************

  $Id$

  EA viewer applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2002, 2007 Steven H.Levine

  16 Oct 02 SHL Reformat
  08 Feb 03 SHL Free list with free() since we don't
		allocate list contents
  08 Apr 07 SHL Minor reformat
  10 May 08 SHL Correct compare typo

***********************************************************************/

#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN

#include "dll\fm3dll.h"
#include "dll\fm3dlg.h"
#include "dll\makelist.h"
#include "dll\eas.h"			// DisplayEAsProc
#include "dll\init.h"			// InitFM3DLL
#include "dll\valid.h"			// IsFile
#include "dll\wrappers.h"			// xfree
#include "dll\getnames.h"			// insert_filename

static PSZ pszSrcFile = __FILE__;

int main (int argc,char *argv[])
{
  HAB hab;
  HMQ hmq;
  CHAR fullname[CCHMAXPATH];
  CHAR **list = NULL;
  UINT x,numfiles = 0,numalloc = 0;

  DosError(FERR_DISABLEHARDERR);
  for(x = 1; x < argc; x++) {
    if (!strchr("/;,`\'",*argv[x]) && IsFile(argv[x]) != -1) {
      if (DosQueryPathInfo(argv[x],
			   FIL_QUERYFULLNAME,fullname,
			   sizeof(fullname)))
	strcpy(fullname, argv[x]);
      AddToList(fullname,&list,&numfiles,&numalloc);
    }
  }
  hab = WinInitialize(0);
  if (hab) {
    hmq = WinCreateMsgQueue(hab,384);
    if (hmq) {
      if (InitFM3DLL(hab,argc,argv)) {
	if (!list) {
	  strcpy(fullname, "*");
	  list = xmalloc(sizeof(CHAR *) * 2, pszSrcFile, __LINE__);
	  if (list &&
	      insert_filename(HWND_DESKTOP,fullname,TRUE,FALSE) &&
	      *fullname && *fullname != '*') {
	   list[0] = fullname;
	   list[1] = NULL;
	  }
	}
	if (list) {
	  WinDlgBox(HWND_DESKTOP,
		    HWND_DESKTOP,
		    DisplayEAsProc,
		    FM3ModHandle,
		    EA_FRAME,
		    (PVOID)list);
	}
      }
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  xfree(list, pszSrcFile, __LINE__);
  return 0;
}

