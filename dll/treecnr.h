
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(TREECNR_H)
#define TREECNR_H

MRESULT EXPENTRY OpenButtonProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID ShowTreeRec(HWND hwndCnr, CHAR * dirname, BOOL collapsefirst,
		 BOOL maketop);
HWND StartTreeCnr(HWND hwndParent, ULONG flags);
MRESULT EXPENTRY TreeClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
MRESULT EXPENTRY TreeObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY TreeStatProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


#endif // TREECNR_H
