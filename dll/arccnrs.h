
/***********************************************************************

  $Id: $

  arccnrs common definitions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Move arccnrs.c definitions here

***********************************************************************/

#if !defined(ARCCNRS_H)
#define ARCCNRS_H

#if !defined(OS2_INCLUDED)
#define INCL_WINSTDCNR			// avl.h
#include <os2.h>
#else
#if !defined(INCL_WINSTDCNR)
#error INCL_WINSTDCNR required
#endif
#endif

#include "avl.h"			// ARC_TYPE

MRESULT EXPENTRY ArcClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
HWND StartArcCnr(HWND hwndParent, HWND hwndCaller, CHAR * arcname, INT flags,
		 ARC_TYPE * sinfo);
MRESULT EXPENTRY ArcTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ArcFolderProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ArcObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
PSZ BldQuotedFullPathName(PSZ pszFullPathName, PSZ pszPathName, PSZ pszFileName);
PSZ BldQuotedFileName(PSZ pszQuotedFileName, PSZ pszFileName);

#endif // ARCCNRS_H
