
/***********************************************************************

  $Id: $

  Strings table utilities interface

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  05 Jan 08 SHL Move from fm3dll.h to here

***********************************************************************/

#if !defined(STRUTIL_H)
#define STRUTIL_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

BOOL LoadStrings(PSZ filename);
PSZ GetPString(ULONG id);
BOOL StringsLoaded(void);

#endif // STRUTIL_H
