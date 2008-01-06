
/***********************************************************************

  $Id: $

  Path handling utility functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Move from fm3dll.h to here

***********************************************************************/

#if !defined(PATHUTIL_H)

#define PATHUTIL_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#else
#endif

PSZ BldFullPathName(PSZ pszFullPathName, PSZ pszPathName, PSZ pszFileName);
PSZ BldQuotedFullPathName(PSZ pszFullPathName, PSZ pszPathName, PSZ pszFileName);
PSZ BldQuotedFileName(PSZ pszQuotedFileName, PSZ pszFileName);

#endif // PATHUTIL_H
