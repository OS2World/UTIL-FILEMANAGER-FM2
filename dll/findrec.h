
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(FINDREC_H)
#define FINDREC_H

PCNRITEM FindCnrRecord(HWND hwndCnr, CHAR * filename, PCNRITEM pciParent,
		       BOOL partial, BOOL partmatch, BOOL noenv);
PCNRITEM FindParentRecord(HWND hwndCnr, PCNRITEM pciC);
VOID ShowCnrRecord(HWND hwndCnr, PMINIRECORDCORE pmi);


#endif // FINDREC_H
