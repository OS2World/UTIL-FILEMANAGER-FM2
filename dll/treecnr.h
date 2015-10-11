
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log
  28 Dec 08 GKY Check for LVM.EXE and remove Refresh removable media menu item as appropriate
  28 Dec 08 GKY Rework partition submenu to gray out unavailable items (check for existence of files)
                and have no default choice.
  23 Aug 15 SHL Protect ShowTreeRec dirname arg
  24 AUG 15 GKY Remove fDummy code
  20 Sep 15 GKY Move tree expand to a thread.

***********************************************************************/

#if !defined(TREECNR_H)
#define TREECNR_H

MRESULT EXPENTRY OpenButtonProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID ShowTreeRec(HWND hwndCnr, PCSZ dirname, BOOL collapsefirst, BOOL maketop);
HWND StartTreeCnr(HWND hwndParent, ULONG flags);
MRESULT EXPENTRY TreeClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
MRESULT EXPENTRY TreeObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY TreeStatProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL StartExpandTreeThread(VOID);

// Data declarations
extern ULONG FM3UL;
extern HWND LastDir;
extern HWND TreeCnrMenu;
extern INT TreesortFlags;
extern INT driveserial[26];
extern BOOL fDCOpens;
extern BOOL fFollowTree;
extern BOOL fTopDir;
extern BOOL fLVMGui;
extern BOOL fDFSee;
extern BOOL fFDisk;
extern BOOL fMiniLVM;
extern BOOL fLVM;
extern BOOL fExpandAll;
extern HPOINTER hptrDunno;
extern HWND hwndMainMenu;

#endif // TREECNR_H
