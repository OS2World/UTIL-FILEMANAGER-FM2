
/***********************************************************************

  $Id: $

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H

***********************************************************************/

#if !defined(MAINWND_H)

#define MAINWND_H

void BubbleHelp(HWND hwnd, BOOL other, BOOL data, BOOL above, char *help);
MRESULT EXPENTRY BubbleProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID BuildDriveBarButtons(HWND hwndT);
MRESULT EXPENTRY ChildButtonProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
BOOL CloseChildren(HWND hwndClient);
ULONG CountDirCnrs(HWND hwndParent);
MRESULT EXPENTRY DriveBackProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DriveProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID FillClient(HWND hwndClient, PSWP pswp, PRECTL prectl, BOOL avoidtree);
HWND FindDirCnrByName(CHAR * directory, BOOL restore);
VOID GetNextWindowPos(HWND hwndClient, PSWP pswp, ULONG * ulCntR,
		      ULONG * ulNumMinChildrenR);
MRESULT EXPENTRY LEDProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY MainWMCommand(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY MainWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID MakeBubble(HWND hwnd, BOOL above, CHAR * help);
VOID MakeMainObjWin(VOID * args);
VOID ResizeDrives(HWND hwndT, long xwidth);
INT SaveDirCnrState(HWND hwndClient, CHAR * name);
MRESULT EXPENTRY StatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID TileChildren(HWND hwndClient, BOOL absolute);
MRESULT EXPENTRY ToolBackProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND TopWindow(HWND hwndParent, HWND exclude);
HWND TopWindowName(HWND hwndParent, HWND exclude, CHAR * ret);

#endif	// MAINWND_H
