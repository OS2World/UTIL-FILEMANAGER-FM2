
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

#define DATE_BUF_BYTES 11

size_t CommaFmtUL(char *pszBuf, UINT cBufSize, ULONG ullNumber,
		  CHAR chPreferred);
size_t CommaFmtULL(char *pszBuf, UINT cBufSize, ULONGLONG ullNumber,
		   CHAR chPreferred);
size_t commafmt(PSZ pszBuf, UINT cBufSize, LONG lNumber);

VOID DateFormat(CHAR szBuf[DATE_BUF_BYTES], CDATE Date);

VOID FDateFormat(CHAR szBuf[DATE_BUF_BYTES], FDATE Date);

VOID DTDateFormat(CHAR szBuf[DATE_BUF_BYTES], DATETIME Date);

#endif // COMMAFMT_H
