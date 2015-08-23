
/***********************************************************************

  $Id$

  Find records

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2015 Steven H. Levine

  23 Aug 15 SHL Protect FindCnrRecord filename arg

***********************************************************************/

#if !defined(FINDREC_H)
#define FINDREC_H

#include "dircnrs.h"			// typedef for CNRITEM, PCNRITEM

PCNRITEM FindCnrRecord(HWND hwndCnr, PCSZ filename, PCNRITEM pciParent,
		       BOOL partial, BOOL partmatch, BOOL noenv);
PCNRITEM FindParentRecord(HWND hwndCnr, PCNRITEM pciC);
VOID ShowCnrRecord(HWND hwndCnr, PMINIRECORDCORE pmi);


#endif // FINDREC_H
