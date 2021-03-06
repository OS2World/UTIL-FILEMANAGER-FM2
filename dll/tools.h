
/***********************************************************************

  $Id$

  tools interface

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2008 Steven H. Levine

  26 May 05 SHL Localize toolhead
  04 Jan 08 SHL Allow standalone usage

***********************************************************************/

#if !defined(TOOLS_H)
#define TOOLS_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

typedef struct TOOL
{
  CHAR *help;
  CHAR *text;
  INT flags;
  struct TOOL *next;
  USHORT id;
}
TOOL;

#define T_DROPABLE    0x00000001
#define T_EMPHASIZED  0x00000002
#define T_INVISIBLE   0x00000004
#define T_SEPARATOR   0x00000008
#define T_TEXT        0x00000010
#define T_MYICON      0x00000020


VOID load_quicktools(VOID);
VOID save_quicktools(VOID);
TOOL *load_tools(CHAR * filename);
VOID save_tools(CHAR * filename);
TOOL *add_tool(TOOL * tool);
TOOL *insert_tool(TOOL * tool, TOOL * after);
TOOL *del_tool(TOOL * tool);
TOOL *find_tool(USHORT id);
TOOL *free_tools(VOID);
TOOL *swap_tools(TOOL * tool1, TOOL * tool2);
TOOL *next_tool(TOOL * tool, BOOL skipinvisible);
TOOL *prev_tool(TOOL * tool, BOOL skipinvisible);
MRESULT EXPENTRY ReOrderToolsProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
MRESULT EXPENTRY AddToolProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY PickToolProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ToolIODlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// Data declarations
extern BOOL fToolsChanged;
extern CHAR lasttoolbar[CCHMAXPATH];
extern BOOL qtloaded;
extern CHAR *quicktool[50];
extern TOOL *toolhead;

#endif // TOOLS_H
