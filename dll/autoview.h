
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H

***********************************************************************/

#if !defined(AUTOVIEW_H)

#define AUTOVIEW_H

MRESULT EXPENTRY AttrListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
                                 MPARAM mp2);
MRESULT EXPENTRY AutoViewProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
ULONG CreateHexDump(CHAR * value, ULONG cbValue, CHAR * ret, ULONG retlen,
                    ULONG startval, BOOL longlead);

#endif	// AUTOVIEW_H
