
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(FILTER_H)
#define FILTER_H

INT APIENTRY Filter(PMINIRECORDCORE rmini, PVOID arg);
MRESULT EXPENTRY PickMaskDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);

#endif // FILTER_H
