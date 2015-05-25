
/***********************************************************************

  $Id$

  Process dump facility interface

  Copyright (c) 2008 Steven H. Levine

  06 Dec 08 SHL Baseline (Ticket 307)

***********************************************************************/

#if !defined(DUMPUTIL_H)
#define DUMPUTIL_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

VOID DbgDumpProcess(VOID);

#endif // DUMPUTIL_H
