
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H
  29 Nov 08 GKY Add flag to tell CheckListProc file is in an archive so it won't try to open it.

***********************************************************************/

#if !defined(CHKLIST_H)

#define CHKLIST_H

#define CHECK_FILES 1    //CHECKLIST flag
#define CHECK_ARCHIVE 2  //CHECKLIST flag file in archive

typedef struct
{
  USHORT size;
  ULONG flags;
  ULONG cmd;
  USHORT dummy;
  CHAR *prompt;
  CHAR **list;
}
CHECKLIST;

VOID CenterOverWindow(HWND hwnd);
MRESULT EXPENTRY CheckListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DropListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL PopupMenu(HWND hwndParent, HWND hwndOwner, HWND hwndMenu);
VOID PosOverOkay(HWND hwnd);

#endif	// CHKLIST_H
