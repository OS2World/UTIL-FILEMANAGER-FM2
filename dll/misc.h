
/***********************************************************************

  $Id: $

  Misc utility functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  17 Jul 08 SHL Baseline

***********************************************************************/

#if !defined(MISC_H)
#define MISC_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

#ifdef FORTIFY
INT GetTidForThread(VOID);
INT GetTidForWindow(HWND hwnd);
#endif

#endif // MISC_H
