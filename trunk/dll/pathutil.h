
/***********************************************************************

  $Id$

  Path handling utility functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2009 Steven H. Levine

  05 Jan 08 SHL Move from fm3dll.h to here
  29 Feb 08 GKY Changes to enable user settable command line length
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.

***********************************************************************/

#if !defined(PATHUTIL_H)

#define PATHUTIL_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

PSZ AddBackslashToPath(PSZ pszPathName);
PSZ BldFullPathName(PSZ pszFullPathName, PCSZ pszPathName, PCSZ pszFileName);
PSZ BldQuotedFullPathName(PSZ pszFullPathName, PCSZ pszPathName, PCSZ pszFileName);
PSZ BldQuotedFileName(PSZ pszQuotedFileName, PCSZ pszFileName);
PCSZ NormalizeCmdLine(PSZ pszWorkBuf, PSZ pszCmdLine_);

#define MAXCOMLINESTRGDEFAULT (1024)			/* used to build command line strings */
#define CMDLNLNGTH_MIN (299)
#define CMDLNLNGTH_MAX (32768)

#ifdef DEFINE_GLOBALS
#define DATADEF
#else
#define DATADEF extern
#endif

DATADEF ULONG MaxComLineStrg;

#endif // PATHUTIL_H
