
/* excputil.hpp - exception handlers
   $Id$

   Copyright (c) 2008 Steven H. Levine

   06 Dec 08 SHL Baseline (Ticket #26)

*/

#if !defined(EXCPUTIL_H)
#define EXCPUTIL_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

int xbeginthread(VOID (*pfnThread)(PVOID),
		 PVOID unused,
		 UINT cStackBytes,
		 PVOID pvArgs);

#endif // EXCPUTIL_H
