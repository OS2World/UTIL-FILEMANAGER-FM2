
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(INIS_H)
#define INIS_H

#include "colors.h"			// typedef RGB2

MRESULT EXPENTRY IniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartIniEditor(HWND hwnd, CHAR * filename, INT flags);

// Data declarations
extern CHAR *DRF_FM2INI;
extern CHAR *DRM_FM2INIRECORD;
extern RGB2 RGBBLACK;
extern HELPINIT hini;

#endif // INIS_H
