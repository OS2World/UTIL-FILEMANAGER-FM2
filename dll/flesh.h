
/***********************************************************************

  $Id$

  Drive tree container management

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2015 Steven H. Levine

  07 Aug 15 SHL Rework to use AddFleshWorkRequest rather than direct calls to Stubby/Flesh/Unflesh
  19 Aug 15 SHL Allow WaitFleshWorkListEmpty to wait for dependent items
  22 Aug 15 GKY Remove recurse scan code.
  27 Sep 15 GKY DosSleep times in WaitFleshWorkListEmpty set by caller
  10 Oct 15 GKY Don't use Flesh thread for floppy drive scans fix them getting mistakenly identified
                as directories and add nonexistent subdirectories.

***********************************************************************/

#if !defined(FLESH_H)
#define FLESH_H

BOOL StartFleshWorkThread(VOID);

typedef enum {eStubby, eFlesh, eFleshEnv, eUnFlesh} FLESHWORKACTION;

BOOL IsFleshWorkListEmpty(VOID);

VOID SetFleshFocusPath(PCSZ pszPath);
BOOL Flesh(HWND hwndCnr, PCNRITEM pciParent);
VOID UnFlesh(HWND hwndCnr, PCNRITEM pciParent);
VOID WaitFleshWorkListEmpty(PCSZ pszDirName, ULONG ulSleep);
BOOL AddFleshWorkRequest(HWND hwndCnr, PCNRITEM pci, FLESHWORKACTION action);

// Data declarations
extern ULONG NoBrokenNotify;
extern BOOL fFilesInTree;


#endif // FLESH_H
