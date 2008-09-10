
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(VIEWER_H)
#define VIEWER_H

MRESULT EXPENTRY MLEEditorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartMLEEditor(HWND hwnd, INT flags, CHAR * filename, HWND hwndRestore);


#endif // VIEWER_H
