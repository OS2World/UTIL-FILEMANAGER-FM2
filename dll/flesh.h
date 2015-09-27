
/***********************************************************************

  $Id$

  Drive tree container management

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2015 Steven H. Levine

  07 Aug 15 SHL Rework to use AddFleshWorkRequest rather than direct calls to Stubby/Flesh/Unflesh
  19 Aug 15 SHL Allow WaitFleshWorkListEmpty to wait for dependent items
  22 Aug 15 GKY Remove recurse scan code.
  27 Sep 15 GKY DosSleep times in WaitFleshWorkListEmpty set by caller

***********************************************************************/

#if !defined(FLESH_H)
#define FLESH_H

BOOL StartFleshWorkThread(VOID);

typedef enum {eStubby, eFlesh, eFleshEnv, eUnFlesh} FLESHWORKACTION;

BOOL IsFleshWorkListEmpty();

VOID SetFleshFocusPath(PCSZ pszPath);

#if 0 // 2015-08-03 SHL FIXME debug
VOID WaitFleshWorkListEmpty(PCSZ pszDirName, ULONG ulSleep);
#else
#define WaitFleshWorkListEmpty(pszDirName, ulSleep) WaitFleshWorkListEmptyDbg(pszDirName, ulSleep, __FILE__, __LINE__)
VOID WaitFleshWorkListEmptyDbg(PCSZ pszDirName, ULONG ulSleep, PCSZ pszSrcFile, UINT uSrcLineNo);
#endif

#if 0 // 2015-08-03 SHL FIXME debug
BOOL AddFleshWorkRequest(HWND hwndCnr, PCNRITEM pci, FLESHWORKACTION action);
#else
#define AddFleshWorkRequest(hwnCnr, pciParent, action) AddFleshWorkRequestDbg(hwnCnr, pciParent, action, __FILE__, __LINE__)
BOOL AddFleshWorkRequestDbg(HWND hwndCnr, PCNRITEM pci, FLESHWORKACTION action, PCSZ pszSrcFile, UINT uSrcLineNo);
#endif

// Data declarations
extern ULONG NoBrokenNotify;
extern BOOL fFilesInTree;


#endif // FLESH_H
