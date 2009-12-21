
/***********************************************************************

  $Id$

  Custom commands

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  01 Mar 08 GKY Move from fm3dll.h to here
  22 Jun 08 GKY Moved free_commands here for fortify checking
  21 Dec 09 GKY Fix the environment so it can be saved, deleted and used consistently.

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
VOID free_commands(VOID);

typedef struct LINKCMDS
{
  PSZ pszCmdLine;
  CHAR *title;
  CHAR *env;
  ULONG flags;
  ULONG ID;
  ULONG HotKeyID;
  struct LINKCMDS *next;
  struct LINKCMDS *prev;
}
LINKCMDS;

// Data declarations
extern BOOL cmdloaded;
extern LINKCMDS *cmdhead;

#endif // COMMAND_H
