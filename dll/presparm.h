
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(PRESPARM_H)
#define PRESPARM_H

VOID CopyPresParams(HWND target, HWND source);
VOID PresParamChanged(HWND hwnd, CHAR * keyroot, MPARAM mp1, MPARAM mp2);
VOID RestorePresParams(HWND hwnd, CHAR * keyroot);
VOID SavePresParams(HWND hwnd, CHAR * keyroot);
#ifdef INCL_GPI
VOID SetPresParams(HWND hwnd, RGB2 * back, RGB2 * fore, RGB2 * border,
		   CHAR * font);
#endif


#endif // PRESPARM_H
