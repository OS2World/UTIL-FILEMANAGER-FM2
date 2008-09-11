
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H

***********************************************************************/

#if !defined(AVV_H)

#define AVV_H

MRESULT EXPENTRY ArcReviewDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
VOID EditArchiverDefinition(HWND hwnd);
VOID rewrite_archiverbb2(CHAR * archiverbb2);

#endif	// AVV_H
