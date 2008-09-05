
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(SEEALL_H)
#define SEEALL_H

MRESULT EXPENTRY SeeAllWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY SeeStatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartSeeAll(HWND hwndParent, BOOL standalone, CHAR * startpath);


#endif // SEEALL_H
