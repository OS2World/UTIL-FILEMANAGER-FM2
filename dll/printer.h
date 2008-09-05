
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(PRINTER_H)
#define PRINTER_H

MRESULT EXPENTRY PrintDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID PrintListThread(VOID * arg);


#endif // PRINTER_H
