
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2015 Steven H. Levine

  Change log
  20 Sep 15 GKY Create CollapseAll and modify ExpandAll to reduce code overhead
                both to try and speed drive expansion. Change ExpandAll to allow it to loop
                in UM_EXPAND until until drive is completely expanded. Changes were need to
                work with Flesh, Stubby and UnFlesh being moved to a thread

***********************************************************************/

#if !defined(SELECT_H)
#define SELECT_H

#include "dircnrs.h"			// typedef for CNRITEM, PCNRITEM

VOID Deselect(HWND hwndCnr);
VOID DeselectAll(HWND hwndCnr, BOOL files, BOOL dirs, CHAR * mask,
		 CHAR * text, BOOL arc);
BOOL ExpandAll(HWND hwndCnr, INT count, PCNRITEM pciParent);
VOID CollapseAll(HWND hwndCnr, PCNRITEM pciParent);
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
