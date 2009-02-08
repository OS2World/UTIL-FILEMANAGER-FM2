
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log
  07 Feb 09 GKY Add *DateFormat functions to format dates bassed on locale

***********************************************************************/

#if !defined(COMMAFMT_H)
#define COMMAFMT_H

size_t CommaFmtUL(char *pszBuf, UINT cBufSize, ULONG ullNumber,
		  CHAR chPreferred);
size_t CommaFmtULL(char *pszBuf, UINT cBufSize, ULONGLONG ullNumber,
		   CHAR chPreferred);
size_t commafmt(PSZ pszBuf, UINT cBufSize, LONG lNumber);

VOID DateFormat(PSZ pszBuf, CDATE Date);

VOID FDateFormat(PSZ pszBuf, FDATE Date);

VOID DTDateFormat(PSZ pszBuf, DATETIME Date);

#endif // COMMAFMT_H
