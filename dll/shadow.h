
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log
  08 Mar 09 GKY Additional strings move to PCSZs declare change

***********************************************************************/

#if !defined(SHADOW_H)
#define SHADOW_H

VOID MakeShadows(HWND hwnd, CHAR **list, ULONG Shadows, CHAR *cnr,
		 CHAR *foldername);
VOID OpenObject(CHAR *filename, PCSZ type, HWND hwnd);
BOOL RunSeamless(CHAR *exename, CHAR *args, HWND hwnd);


#endif // SHADOW_H
