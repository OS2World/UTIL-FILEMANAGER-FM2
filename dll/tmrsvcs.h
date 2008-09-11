
/***********************************************************************

  $Id$

  Timing services interface

  Copyright (c) 2008 Steven H. Levine

  05 Jan 08 SHL Baseline

***********************************************************************/

#if !defined(TMRSVCS_H)
#define TMRSVCS_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

typedef struct {
  INT remaining;		// Remaining counts until time check
  UINT estimated;		// Estimated counts per interval
  UINT misses;
  UINT interval_msec;		// msec per interval
  ULONG start_msec;		// Last tick time
} ITIMER_DESC;

VOID InitITimer(ITIMER_DESC *pitd, UINT interval_msec);
BOOL IsITimerExpired(ITIMER_DESC *pitd);
VOID SleepIfNeeded(ITIMER_DESC *pitd, UINT sleepTime);

#endif // TMRSVCS_H
