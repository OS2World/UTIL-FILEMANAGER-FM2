
/***********************************************************************

  $Id: $

  Custom commands

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  01 Mar 08 GKY Move from fm3dll.h to here

***********************************************************************/

#if !defined(COMMAND_H)
#define COMMAND_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

VOID RunCommand(HWND hwnd, INT cx);
VOID EditCommands(HWND hwnd);
CHAR *command_title(INT cx);
VOID load_commands(VOID);

typedef struct LINKCMDS
{
  PSZ pszCmdLine;
  CHAR *title;
  ULONG flags;
  struct LINKCMDS *next;
  struct LINKCMDS *prev;
}
LINKCMDS;

// 01 Mar 08 GKY fixme for command.c globals to be here

#ifdef DEFINE_GLOBALS
#pragma data_seg(GLOBAL1)
#endif

#endif // COMMAND_H
