
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(DEFVIEW_H)
#define DEFVIEW_H

VOID DefaultView(HWND hwnd, HWND hwndFrame, HWND hwndParent, SWP * swp,
		 ULONG flags, CHAR * filename);
#define QuickView(h,f) DefaultView(h,(HWND)0,(HWND)0,NULL,0,f)
#define QuickEdit(h,f) DefaultView(h,(HWND)0,(HWND)0,NULL,8,f)

BOOL ShowMultimedia(CHAR * filename);


#endif // DEFVIEW_H
