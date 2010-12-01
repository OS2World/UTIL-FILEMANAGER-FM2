
/***********************************************************************

  $Id$

  Error reporting utilities interface

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2010 Steven H. Levine

  05 Jan 08 SHL Move from fm3dll.h to here
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  08 Mar 09 GKY Remove Dos_Error2 (unused) and Runtime_Error2 (no advantage over using Runtime_Error)

***********************************************************************/

#if !defined(ERRUTIL_H)
#define ERRUTIL_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

VOID DbgMsg(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);
INT Dos_Error(ULONG mb_type, APIRET apiret, HWND hwndOwner,
	      PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);
ULONG GetMSecTimer(void);
VOID Runtime_Error(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);
VOID Runtime_Error2(PCSZ pszSrcFile, UINT uSrcLineNo, UINT idMsg);
APIRET saymsg(ULONG mb_type, HWND hwnd, PCSZ pszTitle, PCSZ pszFmt, ...);
VOID Win_Error(HWND hwndErr, HWND hwndOwner,
	       PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);
VOID Win_Error_NoMsgBox(HWND hwndErr, HWND hwndOwner,
			PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);

#endif // ERRUTIL_H
