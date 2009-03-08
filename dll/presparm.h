
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log
  08 Mar 09 GKY Additional strings move to PCSZs in init.c (Delcare changes to PCSZ)

***********************************************************************/

#if !defined(PRESPARM_H)
#define PRESPARM_H

VOID CopyPresParams(HWND target, HWND source);
VOID PresParamChanged(HWND hwnd, PCSZ keyroot, MPARAM mp1, MPARAM mp2);
VOID RestorePresParams(HWND hwnd, PCSZ keyroot);
VOID SavePresParams(HWND hwnd, PCSZ keyroot);
#ifdef INCL_GPI
VOID SetPresParams(HWND hwnd, RGB2 * back, RGB2 * fore, RGB2 * border,
		   PCSZ font);
#endif


#endif // PRESPARM_H
