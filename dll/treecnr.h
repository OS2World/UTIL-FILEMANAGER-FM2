
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

// Data declarations
extern ULONG FM3UL;
extern HWND LastDir;
extern HWND TreeCnrMenu;
extern INT TreesortFlags;
extern INT driveserial[26];
extern BOOL fDCOpens;
extern BOOL fDummy;
extern BOOL fFollowTree;
extern BOOL fTopDir;
extern HPOINTER hptrDunno;
extern HWND hwndMainMenu;

#endif // TREECNR_H
