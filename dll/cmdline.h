
/***********************************************************************

  $Id: $

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H

***********************************************************************/

#if !defined(CMDLINE_H)

#define CMDLINE_H

MRESULT EXPENTRY CmdLine2DlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
MRESULT EXPENTRY CmdLineDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL add_cmdline(CHAR * cmdline, BOOL big);
VOID save_cmdlines(BOOL big);

#endif	// CMDLINE_H
