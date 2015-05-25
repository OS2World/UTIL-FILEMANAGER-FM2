
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log
  14 Mar 09 GKY Additional strings move to PCSZs

***********************************************************************/

#if !defined(INIS_H)
#define INIS_H

#include "colors.h"			// typedef RGB2

MRESULT EXPENTRY IniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartIniEditor(HWND hwnd, CHAR * filename, INT flags);

// Data declarations
extern PCSZ DRF_FM2INI;
extern PCSZ DRM_FM2INIRECORD;
extern RGB2 RGBBLACK;
extern HELPINIT hini;

#endif // INIS_H
