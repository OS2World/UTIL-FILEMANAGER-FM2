
/***********************************************************************

  $Id$

  Drive tree container management

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2015 Steven H. Levine

  07 Aug 15 SHL Rework to use AddFleshWorkRequest rather than direct calls to Stubby/Flesh/Unflesh

***********************************************************************/

#if !defined(FLESH_H)
#define FLESH_H

BOOL StartFleshWorkThread(VOID);

typedef enum {eStubby, eFlesh, eFleshEnv, eUnFlesh, eFillDir} FLESHWORKACTION;

BOOL IsFleshWorkListEmpty();

VOID SetFleshFocusDrive(CHAR chDriveLetter);

#if 0 // 2015-08-03 SHL FIXME debug
VOID WaitFleshWorkListEmpty();
#else
#define WaitFleshWorkListEmpty() WaitFleshWorkListEmptyDbg(__FILE__, __LINE__)
VOID WaitFleshWorkListEmptyDbg(PCSZ pszSrcFile, UINT uSrcLineNo);
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
