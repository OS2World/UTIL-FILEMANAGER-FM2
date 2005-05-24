
/***********************************************************************

  $Id$

  Minimized data bar interface

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005 Steven H.Levine

  23 May 05 SHL Pull from fm3dll.h

***********************************************************************/

#if !defined(DATAMIN_INCLUDED)

#define DATAMIN_INCLUDED

#define QWL_DATAMIN_PTR	(QWL_USER + 4)

typedef struct {
  APIRET     qfsa_rc;
  FSALLOCATE fsa;
  APIRET     qfsi_rc;
  ULONG      qfsa_cb;
  FSQBUFFER2 fsqb2;
  CHAR	     ab[256];		// space for fsqb2 variable data
} tDataMin;

MRESULT EXPENTRY MiniTimeProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY DataProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2);
HWND CreateDataBar (HWND hwndParent,ULONG fl);

#endif
