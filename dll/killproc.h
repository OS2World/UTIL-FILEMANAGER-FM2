
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(KILLPROC_H)
#define KILLPROC_H

CHAR *GetDosPgmName(PID pid, CHAR * string);
MRESULT EXPENTRY KillDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


#endif // KILLPROC_H
