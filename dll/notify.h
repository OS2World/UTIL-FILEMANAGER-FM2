
/***********************************************************************

  $Id: $

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H

***********************************************************************/

#if !defined(NOTIFY_H)

#define NOTIFY_H

BOOL AddNote(CHAR * note);
HWND DoNotify(char *text);
VOID EndNote(VOID);
VOID HideNote(VOID);
HWND Notify(char *text);
VOID NotifyError(CHAR * filename, APIRET error);
MRESULT EXPENTRY NotifyWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID ShowNote(VOID);

#endif	// NOTIFY_H
