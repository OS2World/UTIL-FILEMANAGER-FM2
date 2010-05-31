
/***********************************************************************

  $Id$

  EA viewer applet

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2002, 2008 Steven H.Levine

  16 Oct 02 SHL Reformat
  08 Feb 03 SHL Free list with free() since we don't
		allocate list contents
  08 Apr 07 SHL Minor reformat
  10 May 08 SHL Correct compare typo
  14 Dec 08 SHL Add exception handler support

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSERRORS			// NO_ERROR

#include "dll\fm3dll.h"
#include "dll\mainwnd.h"		// FM3ModHandle
#include "dll\fm3dlg.h"
#include "dll\makelist.h"		// AddToList
#include "dll\eas.h"			// DisplayEAsProc
#include "dll\init.h"			// InitFM3DLL
#include "dll\valid.h"			// IsFile
#include "dll\wrappers.h"		// xfree
#include "dll\getnames.h"		// insert_filename
#include "dll\errutil.h"		// Error reporting
#include "dll\excputil.h"		// Exception handlers

static PSZ pszSrcFile = __FILE__;

int main (int argc,char *argv[])
{
  HAB hab;
  HMQ hmq;
  CHAR fullname[CCHMAXPATH];
  CHAR **list = NULL;
  UINT x;
  UINT numfiles = 0;
  UINT numalloc = 0;
  APIRET regRet;
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };

  DosError(FERR_DISABLEHARDERR);

  regRec.ExceptionHandler = HandleException;
  regRet = DosSetExceptionHandler(&regRec);
  if (regRet != NO_ERROR) {
    DbgMsg(pszSrcFile, __LINE__,
	   "DosSetExceptionHandler failed with error %u", regRet);
  }

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

