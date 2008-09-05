
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(DRAGLIST_H)
#define DRAGLIST_H

HWND DragList(HWND hwnd, HWND hwndObj, CHAR ** list, BOOL moveok);
HWND DragOne(HWND hwndCnr, HWND hwndObj, CHAR * filename, BOOL moveok);
BOOL PickUp(HWND hwndCnr, HWND hwndObj, PCNRDRAGINIT pcd);


#endif // DRAGLIST_H
