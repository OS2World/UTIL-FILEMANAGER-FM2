
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(PRINTER_H)
#define PRINTER_H

MRESULT EXPENTRY PrintDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID PrintListThread(VOID * arg);

// Data declarations
extern CHAR StopPrinting;
extern CHAR printer[CCHMAXPATH];
extern BOOL prnalt;
extern long prnbmargin;
extern BOOL prnformat;
extern BOOL prnformfeedafter;
extern BOOL prnformfeedbefore;
extern long prnlength;
extern long prnlmargin;
extern BOOL prnpagenums;
extern long prnrmargin;
extern long prnspacing;
extern long prntabspaces;
extern long prntmargin;
extern long prnwidth;

#endif // PRINTER_H
