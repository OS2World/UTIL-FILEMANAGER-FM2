
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(SHADOW_H)
#define SHADOW_H

VOID MakeShadows(HWND hwnd, CHAR ** list, ULONG Shadows, CHAR * cnr,
		 CHAR * foldername);
BOOL RunSeamless(CHAR * exename, CHAR * args, HWND hwnd);


#endif // SHADOW_H
