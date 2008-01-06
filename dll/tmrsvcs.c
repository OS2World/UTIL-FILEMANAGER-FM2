
/***********************************************************************

  $Id: $

  Timer services

  Copyright (c) 2008 Steven H. Levine

  05 Jan 08 SHL Baseline

***********************************************************************/

#define INCL_DOS			// QSV_MS_COUNT

// #include "errutil.h"			// DbgMsg // 05 Jan 08 SHL fixme debug
#include "tmrsvcs.h"

// static PSZ pszSrcFile = __FILE__;	// 05 Jan 08 SHL fixme debug

/**
 * Prepare interval timer descriptor for use
 * @param pTD point to interval timer descriptor
 * @param interval_msec is the timer interval in msec or 0 to retain existing value
 */

VOID InitITimer(ITIMER_DESC *pitd, UINT interval_msec)
{
  if (interval_msec)
    pitd->interval_msec = interval_msec;
  pitd->remaining = pitd->estimated;
  DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &pitd->start_msec, sizeof(pitd->start_msec));
}

/**
 * Check timer interval expired
 * @return TRUE if expired
 */

BOOL IsITimerExpired(ITIMER_DESC *pitd)
{
  INT err_msec;
  ULONG cur_msec;
  UINT elapsed_msec;
  INT cnt;

  if (--pitd->remaining < 0) {

    DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &cur_msec, sizeof(cur_msec));
    elapsed_msec = cur_msec - pitd->start_msec;
    if (elapsed_msec == pitd->interval_msec) {
      pitd->remaining = pitd->estimated;
      pitd->start_msec = cur_msec;
      pitd->misses = 0;
      return TRUE;			// Say interval expired
    }

    pitd->misses++;
    err_msec = (cur_msec - pitd->start_msec) - pitd->interval_msec;
    // Estimate counts per msec
#if 0 // 05 Jan 08 SHL fixme to be gone when no longer needed for testing
    DbgMsg(pszSrcFile, __LINE__,
	   "err_msec %d elapsed_msec %d estimated %u misses %u",
	   err_msec, elapsed_msec, pitd->estimated, pitd->misses);
#endif
    if (err_msec > 0) {
      // Late - need to reduce estimated count
      if (elapsed_msec)
	pitd->estimated = pitd->estimated * pitd->interval_msec / elapsed_msec;
      else {
	// Should not occur
	if (pitd->estimated)
	  pitd->estimated--;
      }
      // Calc counts for next interval
      pitd->remaining = pitd->estimated -
			(pitd->estimated * err_msec / pitd->interval_msec);
      pitd->start_msec += pitd->interval_msec;
      return TRUE;
    }

    // Early - need to increase estimated count
    cnt = pitd->estimated * (-err_msec) / pitd->interval_msec;
    if (!cnt) {
      if (pitd->estimated)
	cnt = pitd->estimated * 2;
      else
	cnt = 1;
    }
    pitd->estimated += cnt;
    pitd->remaining = cnt;
  }
  return FALSE;				// Keep waiting
}

VOID SleepIfNeeded(ITIMER_DESC *pitd, UINT sleepTime)
{
  if (IsITimerExpired(pitd)) {
    DosSleep(sleepTime);
    InitITimer(pitd, 0);
  }
}

#pragma alloc_text(TMRSVCS,InitITimer,IsITimerExpired,SleepIfNeeded)
