
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

  04 Oct 08 JBS Make searchapath function non-static
***********************************************************************/

#if !defined(SRCHPATH_H)
#define SRCHPATH_H

INT RunFM2Util(CHAR *appname, CHAR *filename);
CHAR *searchapath(CHAR * path, CHAR * filename);
CHAR *searchpath(CHAR * filename);


#endif // SRCHPATH_H
