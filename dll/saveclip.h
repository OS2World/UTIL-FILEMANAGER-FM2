
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(SAVECLIP_H)
#define SAVECLIP_H

CHAR **ListFromClipboard(HWND hwnd);
VOID ListToClipboardHab(HAB hab, CHAR ** list, ULONG append);
MRESULT EXPENTRY SaveAllListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				    MPARAM mp2);
MRESULT EXPENTRY SaveListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
BOOL SaveToClip(HWND hwnd, CHAR * text, BOOL append);


#endif // SAVECLIP_H
