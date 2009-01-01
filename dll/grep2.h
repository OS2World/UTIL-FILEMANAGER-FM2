
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(GREP2_H)
#define GREP2_H

MRESULT EXPENTRY GrepDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

typedef struct {

  HWND     *hwnd;
  CHAR     *szGrepPath;
  CHAR     *szGrepText;
}
GREPINFO;


#endif // GREP2_H
