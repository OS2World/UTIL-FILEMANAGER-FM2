
/***********************************************************************

  $Id$

  Grep support

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2014 Steven H. Levine

  04 Nov 06 SHL Renames
  15 Aug 07 SHL Drop obsoletes
  04 Jan 08 SHL Allow standalone usage
  08 Feb 14 SHL Add ignoreSVN

***********************************************************************/

#if !defined(GREP_H)
#define GREP_H

#if !defined(OS2_INCLUDED)
#define INCL_LONGLONG
#include <os2.h>
#else
#if !defined(INCL_LONGLONG)
#error INCL_LONGLONG required by grep.h
#endif
#endif

ULONG SecsSince1980(FDATE * date, FTIME * time);
VOID GrepThread(VOID * arg);

typedef struct DUPES
{
  CHAR *name;
  ULONG size;
  FDATE date;
  FTIME time;
  INT flags;
  LONG CRC;
  struct DUPES *next;
}
DUPES;

#define GF_INSERTED 1
#define GF_SKIPME   2

typedef struct
{
  USHORT size;
  CHAR fileMasks[8192];			// ; separated
  CHAR fileMask[CCHMAXPATH + 14];
  CHAR curdir[CCHMAXPATH];
  LONG fileCount;
  HWND hwnd;
  HWND hwndFiles;
  HWND hwndCurFile;
  BOOL caseFlag;
  BOOL absFlag;
  BOOL dirFlag;
  BOOL sayfiles;
  BOOL searchEAs;
  BOOL searchFiles;
  BOOL finddupes;
  BOOL CRCdupes;
  BOOL ignoreSVN;
  BOOL nosizedupes;
  BOOL ignoreextdupes;
  BOOL findifany;
  BOOL anyexcludes;
  ULONG greaterthan;
  ULONG lessthan;
  ULONG olderthan;
  ULONG newerthan;
  ULONG numfiles;
  HAB ghab;
  CHAR *stopflag;
  ULONG toinsert;
  ULONGLONG insertedbytes;
  FILEFINDBUF4L **insertffb;
  CHAR **dir;
  ULONG attrFile;
  ULONG antiattr;
  DUPES *dupehead, *dupelast, **dupenames, **dupesizes;
  CHAR searchPattern[4096];
  ULONG numlines;
  CHAR *matched;
}
GREP;

// Data declarations
extern volatile CHAR diegrep;
extern HWND hwndStatus;

#endif // GREP_H
