
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(NEWVIEW_H)
#define NEWVIEW_H

MRESULT EXPENTRY ViewStatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ViewWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartViewer(HWND hwndParent, USHORT flags, CHAR * filename,
		 HWND hwndRestore);

// Data declarations
extern HEV CompactSem;
extern BOOL fFtpRunWPSDefault;
extern BOOL fHttpRunWPSDefault;
extern LONG standardcolors[16];
extern CHAR *httprun;
extern CHAR *mailrun;

#endif // NEWVIEW_H
