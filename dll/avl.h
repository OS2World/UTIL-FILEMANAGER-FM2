
/***********************************************************************

  $Id: $

  avl common definitions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Move avl.c definitions here

***********************************************************************/

#if !defined(AVL_H)

#define AVL_H

// #include <stdio.h>			// FILE
// #include <time.h>			// time_t

#if !defined(OS2_INCLUDED)
#define INCL_WINSTDCNR			// CDATE
#include <os2.h>
#else
#if !defined(INCL_WINSTDCNR)
#error INCL_WINSTDCNR required
#endif
#endif

#if defined(__IBMC__)
#if __IBMC__ != 430
#error VAC365 required for long long support
#endif
#if !defined(_LONG_LONG)
#error Long long support not enabled
#endif
#endif

typedef struct __arc_type__
{
  CHAR *id;			// User id
  CHAR *ext;			// Extension (without leading dot)
  LONG file_offset;		// Offset to signature (0..n)
  CHAR *list;			// List command
  CHAR *extract;		// Extract command
  CHAR *exwdirs;		// Extract with directories command
  CHAR *test;			// Test command
  CHAR *create;			// Create without directories
  CHAR *move;			// Move into archive without directories
  CHAR *createrecurse;		// Create with recurse and directories
  CHAR *createwdirs;		// Create with directories
  CHAR *movewdirs;		// Move into archive with directories
  CHAR *delete;			// Delete from archive
  CHAR *signature;		// Archiver signature
  CHAR *startlist;		// Listing start marker (blank means no start marker)
  CHAR *endlist;		// Listing end marker (blank means next blank line or EOF)
  INT siglen;			// Signature length in bytes
  INT osizepos;			// Original file size position (0..n) or -1
  INT nsizepos;			// Compressed file size position or -1
  INT fdpos;			// File date position or -1
  INT fdflds;			// File date element count (typically 3) or -1
  INT fnpos;			// File name position or -1 if last
  INT datetype;			// Date field format
  UINT comment_line_num;	// Comment start in old sig file (1..n), 0 if none
  UINT defn_line_num;		// Definition start in old sig file (1..n), 0 if none
  BOOL nameislast;		// Name is last item on line
  BOOL nameisnext;		// File name is on next line
  BOOL nameisfirst;		// File name is first item on line
  struct __arc_type__ *next;
  struct __arc_type__ *prev;
}
ARC_TYPE;

typedef struct
{
  ARC_TYPE *info;
  CHAR listname[CCHMAXPATH];
  CHAR arcname[CCHMAXPATH];
  CHAR *errmsg;
}
ARCDUMP;

ARC_TYPE *quick_find_type(CHAR * filespec, ARC_TYPE * topsig);
ARC_TYPE *find_type(CHAR * filespec, ARC_TYPE * topsig);
INT load_archivers(VOID);
BOOL ArcDateTime(CHAR * dt, INT type, CDATE * cdate, CTIME * ctime);
VOID free_arc_type(ARC_TYPE * pat);
VOID free_archivers(VOID);

// 05 Jan 08 SHL fixme for avl.c globals to be here

#ifdef DEFINE_GLOBALS
#pragma data_seg(GLOBAL1)
#endif

#endif // AVL_H
