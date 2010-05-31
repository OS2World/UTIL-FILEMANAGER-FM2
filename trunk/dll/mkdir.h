
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(MKDIR_H)
#define MKDIR_H

APIRET MassMkdir(HWND hwndClient, CHAR * dir);
BOOL PMMkDir(HWND hwnd, CHAR * filename, BOOL copy);
APIRET SetDir(HWND hwndClient, HWND hwnd, CHAR * dir, INT flags);
void SetTargetDir(HWND hwnd, BOOL justshow);

// Data declarations
extern CHAR targetdir[CCHMAXPATH];

#endif // MKDIR_H
