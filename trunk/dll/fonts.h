
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(FONTS_H)
#define FONTS_H

FATTRS *SetMLEFont(HWND hwndMLE, FATTRS * fattrs, ULONG flags);
VOID SetPresParamFromFattrs(HWND hwnd, FATTRS * fattrs,
			    SHORT sNominalPointSize, FIXED fxPointSize);


#endif // FONTS_H
