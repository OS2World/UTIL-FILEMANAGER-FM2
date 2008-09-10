
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(GETNAMES_H)
#define GETNAMES_H

MRESULT EXPENTRY CustomFileDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL export_filename(HWND hwnd, CHAR * filename, INT overwrite);
BOOL insert_filename(HWND hwnd, CHAR * filename, INT loadit, BOOL newok);


#endif // GETNAMES_H
