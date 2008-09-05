
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(UPDATE_H)
#define UPDATE_H

HPOINTER SelectDriveIcon(PCNRITEM pci);
BOOL UpdateCnrList(HWND hwndCnr, CHAR ** filename, INT howmany, BOOL partial,
		   DIRCNRDATA * dcd);
PCNRITEM UpdateCnrRecord(HWND hwndCnr, CHAR * filename, BOOL partial,
			 DIRCNRDATA * dcd);


#endif // UPDATE_H
