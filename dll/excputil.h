
/* excputil.hpp - exception handlers
   $Id$

   Copyright (c) 2008 Steven H. Levine

   06 Dec 08 SHL Baseline (Ticket #26)

*/

#if !defined(EXCPUTIL_H)
#define EXCPUTIL_H

#if !defined(OS2_INCLUDED)
#define INCL_DOSEXCEPTIONS		// XCTP_...
#include <os2.h>
#endif

int xbeginthread(VOID (*pfnThread)(PVOID),
		 UINT cStackBytes,
		 PVOID pvArgs,
		 PSZ pszSrcFile,
		 UINT uiLineNumber);

_ERR HandleException;

#endif // EXCPUTIL_H
