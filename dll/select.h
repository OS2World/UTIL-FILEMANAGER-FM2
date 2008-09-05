
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(SELECT_H)
#define SELECT_H

VOID DeselectAll(HWND hwndCnr, BOOL files, BOOL dirs, CHAR * mask,
		 CHAR * text, BOOL arc);
VOID ExpandAll(HWND hwndCnr, BOOL expand, PCNRITEM pciParent);
VOID HideAll(HWND hwndCnr);
VOID InvertAll(HWND hwndCnr);
VOID RemoveAll(HWND hwndCnr, ULONGLONG * ullTotalBytes, ULONG * totalfiles);
VOID SelectAll(HWND hwndCnr, BOOL files, BOOL dirs, CHAR * mask, CHAR * text,
	       BOOL arc);
VOID SelectList(HWND hwndCnr, BOOL partial, BOOL deselect, BOOL clearfirst,
		PCNRITEM pciParent, CHAR * filename, CHAR ** list);
VOID SpecialSelect2(HWND hwndParent, INT action);


#endif // SELECT_H
