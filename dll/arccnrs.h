
/***********************************************************************

  $Id$

  arccnrs common definitions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Move arccnrs.c definitions here

***********************************************************************/

#if !defined(ARCCNRS_H)
#define ARCCNRS_H

#if !defined(OS2_INCLUDED)
#define INCL_WINSTDCNR			// avl.h
#include <os2.h>
#else
#if !defined(INCL_WINSTDCNR)
#error INCL_WINSTDCNR required
#endif
#endif

#include "avl.h"			// ARC_TYPE

#pragma pack(1)

typedef struct _ARCITEM
{				// ARCHIVE CONTAINER RECORD STRUCTURE
  MINIRECORDCORE rc;		// Base information
  HWND hwndCnr;			/* Container holding this record */
  PSZ pszFileName;		// Points to full path name or NullStr
  PSZ pszDisplayName;		// Points to displayable part of path name  - used by CFA_STRING
  CHAR szDate[40];		// File's assembled date
  PSZ pszDate;			// Pointer to date
  CDATE date;			// if we know date format
  CTIME time;			// if we know time format
  ULONGLONG cbFile;		// File's original size
  ULONGLONG cbComp;		// File's compressed size
  ULONG flags;
}
ARCITEM, *PARCITEM;

#pragma pack()

MRESULT EXPENTRY ArcClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
HWND StartArcCnr(HWND hwndParent, HWND hwndCaller, CHAR * arcname, INT flags,
		 ARC_TYPE * sinfo);
MRESULT EXPENTRY ArcTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ArcFolderProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ArcObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// Data declarations
extern HWND ArcCnrMenu;
extern HWND ArcMenu;
extern CHAR ArcTempRoot[CCHMAXPATH];
extern BOOL fArcStuffVisible;
extern CHAR lastextractpath[CCHMAXPATH];
extern ULONGLONG ullDATFileSpaceNeeded;

#endif // ARCCNRS_H
