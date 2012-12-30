
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2013 Steven H. Levine

  Change log
  30 Dec 12 GKY Enhance traget directory drop to give the option of changing the directory or carrying out an
                operation to the current target; Added an error message for target=None;
                Added parameter to SetTargetDir

***********************************************************************/

#if !defined(MKDIR_H)
#define MKDIR_H

APIRET MassMkdir(HWND hwndClient, CHAR * dir);
BOOL PMMkDir(HWND hwnd, CHAR * filename, BOOL copy);
APIRET SetDir(HWND hwndClient, HWND hwnd, CHAR * dir, INT flags);
void SetTargetDir(HWND hwnd, BOOL justshow, PSZ newtarget);

// Data declarations
extern CHAR targetdir[CCHMAXPATH];

#endif // MKDIR_H
