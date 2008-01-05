
/***********************************************************************

  $Id$

  Menu support

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  04 Jan 08 SHL Allow standalone usage

***********************************************************************/

#if !defined(MENU_H)
#define MENU_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

typedef struct MENU
{
  USHORT size;
  USHORT cmd;
  USHORT type;
  USHORT dummy;
  CHAR *text;
  struct MENU *next;
}
MENU;

#define ACTION    0
#define SUBMENU   1
#define SEPARATOR 2

#endif // MENU_H
