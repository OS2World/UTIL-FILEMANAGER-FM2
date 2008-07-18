
/***********************************************************************

  $Id: $

  makelist common definitions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Move makelist.c definitions here
  22 Jun 08 GKY Change variable type to fix compiler warning
  17 Jul 08 SHL Add SetListOwner for Fortify support

***********************************************************************/

#if !defined(MAKELIST_H)
#define MAKELIST_H

#if defined(__IBMC__)
#if __IBMC__ != 430
#error VAC365 required for long long support
#endif
#if !defined(_LONG_LONG)
#error Long long support not enabled
#endif
#endif

#if !defined(OS2_INCLUDED)
#define INCL_WINSTDCNR			// avl.h
#define INCL_LONGLONG
#include <os2.h>
#else
#if !defined(INCL_WINSTDCNR)
#error INCL_WINSTDCNR required
#endif
#if !defined(INCL_LONGLONG)
#error INCL_LONGLONG required by makelist.h
#endif
#endif

#include "avl.h"			// ARC_TYPE

typedef struct
{
  ULONG attrFile;
  ULONGLONG cbFile;
  ULONGLONG easize;
  FDATE date;
  FTIME time;
  FDATE ladate;
  FTIME latime;
  FDATE crdate;
  FTIME crtime;
  CHAR fname[1];
}
FILELIST;

typedef struct
{
  HWND hwnd;
  HWND hwndS;
  ULONG type;
  USHORT id;
  INT flags;
  struct __arc_type__ *info;
  CHAR **list;
  ULONG *ulitemID;
  ULONGLONG *cbFile;
  CHAR targetpath[CCHMAXPATH + 6];
  CHAR arcname[CCHMAXPATH];
  CHAR runfile[CCHMAXPATH];
}
LISTINFO;

INT AddToList(CHAR *string, CHAR ***list, UINT *pnumfiles, UINT *pnumalloced);
INT AddToFileList(CHAR *string, FILEFINDBUF4L *ffb4, FILELIST ***list,
		  UINT *pnumfiles, UINT *numalloced);
CHAR **BuildList(HWND hwndCnr);
VOID FreeListInfo(LISTINFO *li);
VOID FreeList(CHAR **list);
VOID SortList(LISTINFO *li);
CHAR **BuildArcList(HWND hwndCnr);
CHAR **RemoveFromList(CHAR **list, CHAR *item);
CHAR **CombineLists(CHAR **prime, CHAR **add);

#ifdef FORTIFY
VOID SetListOwner(LISTINFO *li);
#endif // FORTIFY

#endif // MAKELIST_H
