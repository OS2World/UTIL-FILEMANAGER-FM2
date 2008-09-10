
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(COMMAFMT_H)
#define COMMAFMT_H

size_t CommaFmtUL(char *pszBuf, UINT cBufSize, ULONG ullNumber,
		  CHAR chPreferred);
size_t CommaFmtULL(char *pszBuf, UINT cBufSize, ULONGLONG ullNumber,
		   CHAR chPreferred);
size_t commafmt(PSZ pszBuf, UINT cBufSize, LONG lNumber);

#endif // COMMAFMT_H
