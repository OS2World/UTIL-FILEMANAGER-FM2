
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(MAINWND2_H)
#define MAINWND2_H

MRESULT EXPENTRY MainWndProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartFM32(HAB hab, INT argc, CHAR ** argv);

// Data declarations
extern PFNWP PFNWPFrame;
extern CHAR realappname[12];

#endif // MAINWND2_H
