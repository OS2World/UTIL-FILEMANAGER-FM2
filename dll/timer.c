
/***********************************************************************

  $Id$

  Timer thread

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2008 Steven H. Levine

  22 Jul 06 SHL Check more run time errors
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  10 Dec 08 SHL Integrate exception handler support

***********************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "mainwnd.h"			// Data declaration(s)
#include "datamin.h"			// Data declaration(s)
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "timer.h"
#include "misc.h"			// PostMsg
#include "excputil.h"			// xbeginthread

static PSZ pszSrcFile = __FILE__;

static HEV hevTimerSem;

static void TimerThread(void *args)
{
  HAB hab2;
  HMQ hmq2;
  ULONG cntr = 0;

  priority_bumped();
  hab2 = WinInitialize(0);
  if (hab2) {
    hmq2 = WinCreateMsgQueue(hab2, 0);
    if (hmq2) {
      WinCancelShutdown(hmq2, TRUE);
      if (!DosCreateEventSem(NULL, &hevTimerSem, 0, FALSE)) {
	for (;;) {
	  if (DosWaitEventSem(hevTimerSem, 3000) != ERROR_TIMEOUT)
	    break;
	  cntr++;
	  if (hwndTree && !(cntr % 3))
	    PostMsg(hwndTree, UM_TIMER, MPVOID, MPVOID);
	  if (hwndBubble && WinIsWindowVisible(hwndBubble))
	    PostMsg(hwndBubble, UM_TIMER, MPVOID, MPVOID);
	  if (DataHwnd)
	    PostMsg(DataHwnd, UM_TIMER, MPVOID, MPVOID);
	}
	DosCloseEventSem(hevTimerSem);
      }
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
}

//== StartTimer() return TRUE can start thread ==

BOOL StartTimer(void)
{
  INT rc = xbeginthread(TimerThread,
			32768,
			(PVOID)0,
			pszSrcFile,
			__LINE__);

  return rc != -1;
}

void StopTimer(void)
{
  DosPostEventSem(hevTimerSem);
}

#pragma alloc_text(TIMER,TimerThread,StartTimer,StopTimer)
