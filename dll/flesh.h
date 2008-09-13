
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(FLESH_H)
#define FLESH_H

BOOL Flesh(HWND hwndCnr, PCNRITEM pciParent);
BOOL FleshEnv(HWND hwndCnr, PCNRITEM pciParent);
BOOL Stubby(HWND hwndCnr, PCNRITEM pciParent);
BOOL UnFlesh(HWND hwndCnr, PCNRITEM pciParent);

// Data declarations
extern ULONG NoBrokenNotify;
extern BOOL fFilesInTree;

#endif // FLESH_H
