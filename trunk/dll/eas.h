
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(EAS_H)
#define EAS_H

typedef struct HOLDFEA
{
  PFEA2 pfea;
  CHAR *name;
  CHAR *value;
  BYTE fEA;
  BYTE cbName;
  USHORT cbValue;
  struct HOLDFEA *next;
}
HOLDFEA;

MRESULT EXPENTRY DisplayEAsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID Free_FEAList(HOLDFEA * pFEA);
HOLDFEA *GetFileEAs(CHAR * filename, BOOL ishandle, BOOL silentfail);
VOID HexDump(HWND hwnd, CHAR * value, ULONG cbValue);

#endif // EAS_H
