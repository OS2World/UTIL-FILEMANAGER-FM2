
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(DRAGLIST_H)
#define DRAGLIST_H

HWND DoFileDrag(HWND hwndCnr, HWND hwndObj, PCNRDRAGINIT pcd, CHAR * arcfile,
		CHAR * directory, BOOL moveok);HWND DragList(HWND hwnd, HWND hwndObj, CHAR ** list, BOOL moveok);
HWND DragOne(HWND hwndCnr, HWND hwndObj, CHAR * filename, BOOL moveok);
VOID FreeDragInfoData (HWND hwnd, PDRAGINFO pDInfo);
BOOL PickUp(HWND hwndCnr, HWND hwndObj, PCNRDRAGINIT pcd);

// Data declarations
extern CHAR *DRMDRFLIST;
extern HPOINTER hptrDir;
extern HPOINTER hptrFile;
extern HPOINTER hptrLast;

#endif // DRAGLIST_H
