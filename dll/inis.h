
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(INIS_H)
#define INIS_H

MRESULT EXPENTRY IniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartIniEditor(HWND hwnd, CHAR * filename, INT flags);


#endif // INIS_H
