
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(COMMON_H)
#define COMMON_H

MRESULT EXPENTRY CommonCnrProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void CommonCreateMainChildren(HWND hwnd, SWP * swp);
void CommonCreateTextChildren(HWND hwnd, char *class, USHORT * ids);
void CommonDriveCmd(HWND hwnd, char *drive, USHORT cmd);
MRESULT EXPENTRY CommonFrameWndProc(USHORT id,
				    HWND hwnd,
				    ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CommonMainWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
void CommonTextPaint(HWND hwnd, HPS hps);


#endif // COMMON_H
