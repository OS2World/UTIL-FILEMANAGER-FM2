
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(DROPLIST_H)
#define DROPLIST_H

BOOL AcceptOneDrop(HWND hwnd, MPARAM mp1, MPARAM mp2);
BOOL CheckPmDrgLimit(PDRAGINFO pDInfo);
LISTINFO *DoFileDrop(HWND hwndCnr, CHAR * directory, BOOL arcfilesok,
		     MPARAM mp1, MPARAM mp2);
void DropHelp(MPARAM mp1, MPARAM mp2, HWND hwnd, char *text);
BOOL FullDrgName(PDRAGITEM pDItem, CHAR * buffer, ULONG buflen);
BOOL GetOneDrop(HWND hwnd, MPARAM mp1, MPARAM mp2, char *buffer, ULONG buflen);


#endif // DROPLIST_H
