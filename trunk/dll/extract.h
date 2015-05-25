
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(EXTRACT_H)
#define EXTRACT_H

typedef struct
{
  USHORT size;
  ARC_TYPE *info;
  CHAR *arcname;
  CHAR masks[257];
  CHAR command[257];
  CHAR extractdir[CCHMAXPATH];
  INT ret;
}
EXTRDATA;

MRESULT EXPENTRY ExtractDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


#endif // EXTRACT_H
