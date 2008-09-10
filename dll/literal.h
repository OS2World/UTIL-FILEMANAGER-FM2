
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(LITERAL_H)
#define LITERAL_H

PSZ fixup(const PCH pachInBuf, PSZ pszOutBuf, const UINT cBufBytes,
	  const UINT cInBytes);
UINT literal(PSZ pszBuf);
BOOL wildcard(const PSZ pszBuf, const PSZ pszWildCard,
	      const BOOL fNotFileSpec);


#endif // LITERAL_H
