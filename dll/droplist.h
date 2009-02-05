
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2009 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(DROPLIST_H)
#define DROPLIST_H

#include "makelist.h"		// typedef for LISTINFO

BOOL AcceptOneDrop(HWND hwnd, MPARAM mp1, MPARAM mp2);
BOOL CheckPmDrgLimit(PDRAGINFO pDInfo);
LISTINFO *DoFileDrop(HWND hwndCnr, PCSZ directory, BOOL arcfilesok,
		     MPARAM mp1, MPARAM mp2);
void DropHelp(MPARAM mp1, MPARAM mp2, HWND hwnd, PCSZ text);
BOOL FullDrgName(PDRAGITEM pDItem, CHAR * buffer, ULONG buflen);
BOOL GetOneDrop(HWND hwnd, MPARAM mp1, MPARAM mp2, char *buffer, ULONG buflen);


#endif // DROPLIST_H
