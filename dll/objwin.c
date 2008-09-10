
/***********************************************************************

  $Id$

  Object windows

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2008 Steven H.Levine

  26 Jul 06 SHL Check more run time errors
  02 Nov 06 SHL Comments
  30 Mar 07 GKY Remove GetPString for window class names
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  08 Jul 08 SHL Correct Fortify_LeaveScope usage and avoid spurious reports

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "arccnrs.h"			// ArcObjWndProc
#include "errutil.h"			// Win_Error
#include "fortify.h"
#include "misc.h"			// GetTidForThread
#include "collect.h"			// CollectorObjWndProc
#include "objwin.h"
#include "treecnr.h"			// TreeObjWndProc


static PSZ pszSrcFile = __FILE__;

static MRESULT EXPENTRY ObjectWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

MRESULT EXPENTRY ObjectWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *dcd;

  dcd = WinQueryWindowPtr(hwnd, QWL_USER);
  if (dcd) {
    switch (dcd->type) {
    case DIR_FRAME:
      return DirObjWndProc(hwnd, msg, mp1, mp2);
    case TREE_FRAME:
      return TreeObjWndProc(hwnd, msg, mp1, mp2);
    case COLLECTOR_FRAME:
      return CollectorObjWndProc(hwnd, msg, mp1, mp2);
    case ARC_FRAME:
      return ArcObjWndProc(hwnd, msg, mp1, mp2);
    }
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

VOID MakeObjWin(VOID * args)
{
  HWND ObjectHwnd;
  HAB hab2;
  HMQ hmq2;
  QMSG qmsg2;

  hab2 = WinInitialize(0);
  if (hab2) {
    hmq2 = WinCreateMsgQueue(hab2, 512);
    if (hmq2) {
      DosError(FERR_DISABLEHARDERR);
      WinRegisterClass(hab2,
		       WC_OBJECTWINDOW,
		       ObjectWndProc, 0, sizeof(PVOID));
      ObjectHwnd = WinCreateWindow(HWND_OBJECT,
				   WC_OBJECTWINDOW,
				   (PSZ) NULL,
				   0,
				   0L,
				   0L,
				   0L,
				   0L, 0L, HWND_TOP, OBJ_FRAME, NULL, NULL);
      if (!ObjectHwnd)
	Win_Error2(HWND_OBJECT, HWND_DESKTOP, pszSrcFile, __LINE__,
		   IDS_WINCREATEWINDOW);
      else {
#	ifdef FORTIFY
	Fortify_EnterScope();
#	endif
	WinSetWindowPtr(ObjectHwnd, QWL_USER, args);
	/* initially populate container */
	// 18 Jul 08 SHL fixme to know if this really kills WM_CREATE
	WinSendMsg(ObjectHwnd, UM_SETUP, MPVOID, MPVOID);
	PostMsg(ObjectHwnd, UM_RESCAN, MPVOID, MPVOID);
	priority_normal();
	while (WinGetMsg(hab2, &qmsg2, (HWND) 0, 0, 0))
	  WinDispatchMsg(hab2, &qmsg2);
	WinDestroyWindow(ObjectHwnd);
#	ifdef FORTIFY
	{
	  // Allow container to close and free data
	  HWND hwndCnr = ((DIRCNRDATA *)args)->hwndCnr;
	  USHORT i;
	  for (i = 0; WinIsWindow(hab2, hwndCnr) && i < 10; i++)
	    DosSleep(50);
	  for (;;) {
	    UCHAR scope = Fortify_LeaveScope();
	    if ((CHAR)scope == 0)
	      break;
	    Runtime_Error(pszSrcFile, __LINE__, "Attempting to exit thread %u with scope non-zero (%u)",
			  GetTidForThread(), scope);
	    if ((CHAR)scope < 0)
	      break;
	  }
	}
#	endif
      }
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
}

#pragma alloc_text(OBJWIN,ObjectWndProc,MakeObjWin)
