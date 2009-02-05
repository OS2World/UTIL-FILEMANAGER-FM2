
/***********************************************************************

  $Id$

  Strings table utilities interface

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2009 Steven H. Levine

  05 Jan 08 SHL Move from fm3dll.h to here
  03 Feb 09 SHL Switch to STRINGTABLE and const return

***********************************************************************/

#if !defined(STRUTIL_H)
#define STRUTIL_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

PCSZ GetPString(ULONG id);

#endif // STRUTIL_H
