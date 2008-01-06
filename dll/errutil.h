
/***********************************************************************

  $Id: $

  Error reporting utilities interface

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  05 Jan 08 SHL Move from fm3dll.h to here

***********************************************************************/

#if !defined(ERRUTIL_H)
#define ERRUTIL_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

VOID DbgMsg(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);
INT Dos_Error(ULONG mb_type, ULONG ulRC, HWND hwndOwner,
	      PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);
INT Dos_Error2(ULONG mb_type, ULONG ulRC, HWND hwndOwner, PCSZ pszSrcFile,
	       UINT uSrcLineNo, UINT idMsg);
ULONG GetMSecTimer(void);
VOID Runtime_Error(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);
VOID Runtime_Error2(PCSZ pszSrcFile, UINT uSrcLineNo, UINT idMsg);
APIRET saymsg(ULONG mb_type, HWND hwnd, PCSZ pszTitle, PCSZ pszFmt, ...);
VOID Win_Error(HWND hwndErr, HWND hwndOwner,
	       PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);
VOID Win_Error2(HWND hwndErr, HWND hwndOwner, PCSZ pszSrcFile,
		UINT uSrcLineNo, UINT idMsg);
VOID Win_Error_NoMsgBox(HWND hwndErr, HWND hwndOwner,
			PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);

extern PSZ DEBUG_STRING;

#endif // ERRUTIL_H
