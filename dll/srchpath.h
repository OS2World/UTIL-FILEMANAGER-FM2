
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

  04 Oct 08 JBS Make searchapath function non-static
  01 Mar 14 JBS Ticket #524: Made "searchapath" thread-safe. Function names and signatures were changed.

***********************************************************************/

#if !defined(SRCHPATH_H)
#define SRCHPATH_H

INT RunFM2Util(PCSZ appname, CHAR *filename);
APIRET SearchPathForFile(PCSZ pPathname,
                         PCSZ pFilename,
                         PCHAR pFullFilename);
APIRET SearchMultiplePathsForFile(PCSZ pFilename,
                                  PSZ pFullFilename);

#endif // SRCHPATH_H
