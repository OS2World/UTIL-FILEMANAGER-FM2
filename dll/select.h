
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(SELECT_H)
#define SELECT_H

VOID Deselect(HWND hwndCnr);
VOID DeselectAll(HWND hwndCnr, BOOL files, BOOL dirs, CHAR * mask,
		 CHAR * text, BOOL arc);
VOID ExpandAll(HWND hwndCnr, BOOL expand, PCNRITEM pciParent);
VOID HideAll(HWND hwndCnr);
VOID InvertAll(HWND hwndCnr);
VOID MarkAll(HWND hwndCnr, BOOL quitit, BOOL target, BOOL source);
VOID RemoveAll(HWND hwndCnr, ULONGLONG * ullTotalBytes, ULONG * totalfiles);
VOID SelectAll(HWND hwndCnr, BOOL files, BOOL dirs, CHAR * mask, CHAR * text,
	       BOOL arc);
VOID SelectList(HWND hwndCnr, BOOL partial, BOOL deselect, BOOL clearfirst,
		PCNRITEM pciParent, CHAR * filename, CHAR ** list);
VOID SetMask(CHAR * str, MASK * mask);
VOID SpecialSelect2(HWND hwndParent, INT action);
VOID UnHilite(HWND hwndCnr, BOOL all, CHAR *** list, ULONG ulItemsToUnHilite);


#endif // SELECT_H
