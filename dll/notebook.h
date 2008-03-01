
/***********************************************************************

  $Id: notebook.h $

  Configuration notebook

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  14 Feb 08 SHL Refactor from fm3dll.h
  29 Feb 08 GKY Refactor global command line variables to notebook.h

***********************************************************************/

#if !defined(NOTEBOOK_H)
#define NOTEBOOK_H

#if !defined(OS2_INCLUDED)
#define INCL_LONGLONG
#include <os2.h>
#else
#if !defined(INCL_LONGLONG)
#error INCL_LONGLONG required by grep.h
#endif
#endif

MRESULT EXPENTRY CfgDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

VOID CfgMenuInit(HWND hwndMenu, BOOL fIsLite);

#ifdef DEFINE_GLOBALS
#define DATADEF
#else
#define DATADEF extern
#endif

DATADEF CHAR *editor, *viewer, *virus, *compare, *binview, *bined,
  *dircompare, *ftprun, *httprun, *mailrun;
DATADEF CHAR ftprundir[CCHMAXPATH], httprundir[CCHMAXPATH],
  mailrundir[CCHMAXPATH], targetdir[CCHMAXPATH];

#endif // NOTEBOOK_H
