
/***********************************************************************

  $Id$

  compare interface

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Split from fm3dll.h
  18 Jan 08 SHL Sync with count update mods

***********************************************************************/

#if !defined(COMP_H)

#define COMP_H

#if !defined(OS2_INCLUDED)
#define INCL_WINSTDCNR			// dircnrs.h
#include <os2.h>
#else
#if !defined(INCL_WINSTDCNR)
#error INCL_WINSTDCNR required
#endif
#endif

#include "dircnrs.h"			// DIRCNRDATA

typedef struct
{
  USHORT size;
  USHORT dummy;
  CHAR file1[CCHMAXPATH];
  CHAR file2[CCHMAXPATH];
  HWND hwndParent;
  HWND hwndList;
  HWND hwndReport;
  HWND hwndHelp;
}
FCOMPARE;

typedef struct COMPARE
{
  USHORT size;
  HWND hwnd;
  HWND hwndParent;
  CHAR leftdir[CCHMAXPATH + 2];
  CHAR rightdir[CCHMAXPATH + 2];
  BOOL forcescroll;
  BOOL filling;
  BOOL includesubdirs;
  INT action;
  UINT selleft;
  UINT selright;
  UINT totalleft;
  UINT totalright;
  UINT uOldSelLeft;
  UINT uOldSelRight;
  UINT uOldTotalLeft;
  UINT uOldTotalRight;
  CHAR rightlist[CCHMAXPATH];	// Snapshot file name
  BOOL reset;
  HWND hwndCalling;
  struct COMPARE *cmp;		// callers compare defintion
  struct DIRCNRDATA dcd;
}
COMPARE;

MRESULT EXPENTRY CFileDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CompareDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// Data declarations
extern BOOL fSelectedAlways;

#endif // COMP_H
