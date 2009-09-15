
/***********************************************************************

  $Id$

  Worker thread interface

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2009 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H
  14 Sep 09 SHL Drop experimental code

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

VOID Action(VOID * args);
VOID MassAction(VOID * args);

#endif	// WORKER_H
