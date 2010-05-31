
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2009 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H
  04 Feb 09 SHL Convert args to const to match GetPString mods
  08 Mar 09 GKY Additional strings move to PCSZs & String Table
  21 Jun 09 GKY Added drive letter to bitmap buttons in drive bar; Eliminate static drive
                letter windows; Use button ID to identify drive letter for processing.

***********************************************************************/

#if !defined(MAINWND_H)

#define MAINWND_H

#include "colors.h"		// typedef for RGB2

#define DRIVE_BUTTON_WIDTH 28
#define	DRIVE_BUTTON_HEIGHT 21

void BubbleHelp(HWND hwnd, BOOL other, BOOL data, BOOL above, PCSZ help);
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
VOID MakeBubble(HWND hwnd, BOOL above, PCSZ help);
VOID MakeMainObjWin(VOID * args);
VOID ResizeDrives(HWND hwndT, long xwidth);
INT SaveDirCnrState(HWND hwndClient, PCSZ name);
MRESULT EXPENTRY StatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID TileChildren(HWND hwndClient, BOOL absolute);
MRESULT EXPENTRY ToolBackProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND TopWindow(HWND hwndParent, HWND exclude);
HWND TopWindowName(HWND hwndParent, HWND exclude, CHAR * ret);

// Data declarations
extern ULONG AutoviewHeight;
extern ULONG DriveLines;
extern HMODULE FM3ModHandle;
extern PCSZ FM3Str;
extern HWND MainPopupMenu;
extern HWND MainObjectHwnd;
extern BOOL MenuInvisible;
extern PFNWP PFNWPStatic;
extern PFNWP PFNWPButton;
extern RGB2 RGBGREY;
extern BOOL fAmClosing;
extern BOOL fAutoTile;
extern BOOL fAutoView;
extern BOOL fComments;
extern BOOL fDrivebar;
extern BOOL fFreeTree;
extern BOOL fMoreButtons;
extern BOOL fNoFinger;
extern BOOL fNoSaveState;
extern BOOL fNoTileUpdate;
extern BOOL fRunning;
extern BOOL fSplitStatus;
extern BOOL fTextTools;
extern BOOL fToolTitles;
extern BOOL fToolbar;
extern BOOL fUserComboBox;
extern HBITMAP hbmLEDoff;
extern HBITMAP hbmLEDon;
extern HPOINTER hptrFinger;
extern HWND hwndAutoview;
extern HWND hwndBubble;
extern HWND hwndName;
extern HWND hwndStatelist;
extern HWND hwndToolback;
extern HWND hwndTree;
extern USHORT shiftstate;

#define STATE_NAME_MAX_BYTES 256

#endif	// MAINWND_H
