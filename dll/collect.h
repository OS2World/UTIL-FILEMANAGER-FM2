
/***********************************************************************

  $Id: $

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H

***********************************************************************/

#if !defined(COLLECT_H)

#define COLLECT_H

MRESULT EXPENTRY CollectorClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
					MPARAM mp2);
MRESULT EXPENTRY CollectorObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
                                     MPARAM mp2);
MRESULT EXPENTRY CollectorTextProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
HWND StartCollector(HWND hwndParent, INT flags);

#endif	// COLLECT_H
