
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2009 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H

***********************************************************************/

#if !defined(NOTIFY_H)

#define NOTIFY_H

VOID AddNote(PCSZ note);		// 16 Jul 09 SHL
HWND DoNotify(PCSZ text);
VOID EndNote(VOID);
VOID HideNote(VOID);
HWND Notify(PCSZ text);
VOID NotifyError(PCSZ filename, APIRET error);
MRESULT EXPENTRY NotifyWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID ShowNote(VOID);

// Data declarations
extern BOOL fThreadNotes;

#endif	// NOTIFY_H
