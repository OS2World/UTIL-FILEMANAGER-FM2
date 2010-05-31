
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log
  28 Dec 08 GKY Check for LVM.EXE and remove Refresh removable media menu item as appropriate
  28 Dec 08 GKY Rework partition submenu to gray out unavailable items (check for existence of files)
                and have no default choice.

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
extern BOOL fLVMGui;
extern BOOL fDFSee;
extern BOOL fFDisk;
extern BOOL fMiniLVM;
extern BOOL fLVM;
extern HPOINTER hptrDunno;
extern HWND hwndMainMenu;

#endif // TREECNR_H
