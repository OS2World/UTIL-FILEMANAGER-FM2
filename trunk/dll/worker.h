
/***********************************************************************

  $Id$

  Worker thread interface

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2009 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H
  14 Sep 09 SHL Drop experimental code
  04 Aug 12 GKY Changes to use Unlock to unlock files if Unlock.exe is in path both from menu/toolbar and as part of
                copy, move and delete operations
  04 Aug 12 GKY Changes to allow copy and move over readonly files with a warning dialog; also added a warning dialog
                for delete of readonly files
  06 Jan 13 GKY Added optional confirmation dialogs for delete move and copy to compare dir Ticket 277

***********************************************************************/

#if !defined(WORKER_H)

#define WORKER_H

#include "makelist.h"			// typedef LISTINFO

typedef struct {

  CHAR *source;
  CHAR target[CCHMAXPATH];
  BOOL rename;
  BOOL skip;
  BOOL dontask;
  BOOL overold;
  BOOL overnew;
  BOOL overwrite;
  BOOL noreadonlywarn;
  BOOL compare;
}
MOVEIT;

typedef struct
{
  USHORT size;
  USHORT dummy;
  CHAR directory[CCHMAXPATH];
  HWND hwndParent;
  HWND hwndFrame;
  HWND hwndClient;
  HWND hwndCnr;
  LISTINFO *li;
}
WORKER;

// Data declarations
extern FILE *LogFileHandle;
extern BOOL fUnlock;

VOID Action(VOID * args);
VOID MassAction(VOID * args);

#endif	// WORKER_H
