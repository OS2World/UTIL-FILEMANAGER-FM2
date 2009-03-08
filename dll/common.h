
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log
  08 Mar 09 GKY Additional strings move to PCSZs in init.c (Declare changes)

***********************************************************************/

#if !defined(COMMON_H)
#define COMMON_H

MRESULT EXPENTRY CommonCnrProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void CommonCreateMainChildren(HWND hwnd, SWP * swp);
void CommonCreateTextChildren(HWND hwnd, PCSZ class, USHORT * ids);
void CommonDriveCmd(HWND hwnd, char *drive, USHORT cmd);
MRESULT EXPENTRY CommonFrameWndProc(USHORT id,
				    HWND hwnd,
				    ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CommonMainWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
MRESULT EXPENTRY CommonTextButton(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
void CommonTextPaint(HWND hwnd, HPS hps);
MRESULT EXPENTRY CommonTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID DecrThreadUsage(VOID);
VOID IncrThreadUsage(VOID);
HWND OpenDirCnr(HWND hwnd, HWND hwndParent, HWND hwndRestore,
		BOOL noautotile, char *directory);

// Data declarations
extern HWND hwndBack;

#endif // COMMON_H
