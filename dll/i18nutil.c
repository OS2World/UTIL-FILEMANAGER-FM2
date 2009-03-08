
/***********************************************************************

  $Id$

  Command formatting tools

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2005 Steven H. Levine

  06 Jan 04 SHL Disable hundfmt, clean commafmt
  25 May 05 SHL Drop hundfmt
  25 May 05 SHL Add CommaFmtULL, CommaFmtUL
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  05 Nov 07 GKY Use commafmtULL to display file sizes for large file support
  10 Nov 07 GKY Get thousands separator from country info for file sizes.
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale

***********************************************************************/

/*
 * COMMAFMT.C
 * Adapted from public domain code by Bob Stout
 *
 *         2. By making the numeric argument a long and prototyping it before
 *            use, passed numeric arguments will be implicitly cast to longs
 *            thereby avoiding int overflow.
 *         3. Use the thousands grouping and thousands separator from the
 *            ANSI locale to make this more robust.
 */

#include <string.h>

#include "fm3dll.h"
#include "init.h"			// Data declaration(s)
#include "i18nutil.h"

size_t commafmt(char *pszBuf,	// Output buffer
		UINT cBufSize,	// Buffer size, including nul
		long lNumber)	// Number to convert
{
  UINT cChars = 1;		// Number of characters generated (excluding nul)
  UINT cDigits = 1;		// For commas
  INT sign = 1;

  char *pch = pszBuf + cBufSize - 1;

  if (cBufSize < 2)
    goto ABORT;

  *pch-- = 0;				// Stuff terminator
  --cBufSize;
  if (lNumber < 0) {
    sign = -1;
    lNumber = -lNumber;
  }

  for (; cChars <= cBufSize; ++cChars, ++cDigits) {
    *pch-- = (CHAR)(lNumber % 10 + '0');
    lNumber /= 10;
    if (!lNumber)
      break;
    if (cDigits % 3 == 0) {
      *pch-- = *ThousandsSeparator;
      ++cChars;
    }
    if (cChars >= cBufSize)
      goto ABORT;
  }					// for

  if (sign < 0) {
    if (cBufSize == 0)
      goto ABORT;
    *pch-- = '-';
    ++cChars;
  }

  strcpy(pszBuf, ++pch);		// Left align

  return cChars;

ABORT:
  *pszBuf = 0;
  return 0;
}

/**
 * Format long long number with commas and SI unit suffix
 * @param pszBuf Points to output buffer
 * @param cBufSize Buffer size including space for terminating nul
 * @param ullNumber Number to convert
 * @param chPreferred Preferred suffix, blank, K, M
 * @return size of formatting string excluding terminating nul
 */

size_t CommaFmtULL(char *pszBuf,	// Output buffer
		   UINT cBufSize,	// Buffer size, including nul
		   ULONGLONG ullNumber,	// Number to convert
		   CHAR chPreferred)	// Preferred suffix, blank, K, M, G
{
  CHAR chSuffix = ' ';
  size_t c;

  if (ullNumber >= 1ULL << 31 || (chPreferred != ' ' && ullNumber >= 1024)) {
    ullNumber = (ullNumber + 1023) >> 10;
    chSuffix = 'K';
    if (ullNumber >= 1ULL << 31 || (chPreferred == 'M' && ullNumber >= 1024) ||
        (chPreferred == 'G' && ullNumber >= 1024)) {
      ullNumber = (ullNumber + 1023) >> 10;
      chSuffix = 'M';
      if (ullNumber >= 1ULL << 31 || (chPreferred == 'G' && ullNumber >= 1024)) {
        ullNumber = (ullNumber + 1023) >> 10;
        chSuffix = 'G';
      }
    }
  }

  c = commafmt(pszBuf, cBufSize, (LONG)ullNumber);

  if (chSuffix != ' ') {
    if (c + 4 > cBufSize) {
      *pszBuf = 0;
      c = 0;
    }
    else {
      pszBuf += c;
      *pszBuf++ = chSuffix;
      *pszBuf++ = 'i';
      *pszBuf++ = 'B';
      c += 3;
      *pszBuf = 0;
    }
  }
  return c;
}

//=== CommaFmtUL: format unsigned long number with commas and SI unit suffix ===

size_t CommaFmtUL(char *pszBuf,	// Output buffer
		  UINT cBufSize,	// Buffer size, including nul
		  ULONG ulNumber,	// Number to convert
		  CHAR chPreferred)	// Preferred suffix, blank, K, M
{
  CHAR chSuffix = ' ';
  size_t c;

  if (ulNumber >= 1ULL << 31 || (chPreferred != ' ' && ulNumber >= 1024)) {
    ulNumber = (ulNumber + 1023) >> 10;
    chSuffix = 'K';
    if (ulNumber >= 1ULL << 31 || (chPreferred == 'M' && ulNumber >= 1024)) {
      ulNumber = (ulNumber + 1023) >> 10;
      chSuffix = 'M';
    }
  }

  c = commafmt(pszBuf, cBufSize, ulNumber);

  if (chSuffix != ' ') {
    if (c + 4 > cBufSize) {
      *pszBuf = 0;
      c = 0;
    }
    else {
      pszBuf += c;
      *pszBuf++ = chSuffix;
      *pszBuf++ = 'i';
      *pszBuf++ = 'B';
      c += 3;
      *pszBuf = 0;
    }
  }
  return c;
}

VOID DateFormat(CHAR szBuf[DATE_BUF_BYTES], CDATE Date)
{
  switch(ulDateFmt) {

  case 3:

    sprintf(szBuf, "%04u%s%02u%s%02u", Date.year, DateSeparator,
            Date.day, DateSeparator, Date.month);
    break;

  case 2:

    sprintf(szBuf, "%04u%s%02u%s%02u",	Date.year, DateSeparator,
            Date.month, DateSeparator, Date.day);
    break;

  case 1:

    sprintf(szBuf, "%02u%s%02u%s%04u",	Date.day, DateSeparator,
            Date.month, DateSeparator, Date.year);
    break;

  case 0:
  default:

    sprintf(szBuf, "%02u%s%02u%s%04u",	Date.month, DateSeparator,
            Date.day, DateSeparator, Date.year);
    break;
  }
}

VOID FDateFormat(CHAR szBuf[DATE_BUF_BYTES], FDATE Date)
{
  switch(ulDateFmt) {

  case 3:

    sprintf(szBuf, "%04u%s%02u%s%02u",	Date.year + 1980, DateSeparator,
            Date.day, DateSeparator, Date.month);
    break;

  case 2:

    sprintf(szBuf, "%04u%s%02u%s%02u",	Date.year + 1980, DateSeparator,
            Date.month, DateSeparator, Date.day);
    break;

  case 1:

    sprintf(szBuf, "%02u%s%02u%s%04u",	Date.day, DateSeparator,
            Date.month, DateSeparator, Date.year + 1980);
    break;

  case 0:
  default:

    sprintf(szBuf, "%02u%s%02u%s%04u",	Date.month, DateSeparator,
            Date.day, DateSeparator, Date.year + 1980);
    break;
  }
}

VOID DTDateFormat(CHAR szBuf[DATE_BUF_BYTES], DATETIME Date)
{
  switch(ulDateFmt) {

  case 3:

    sprintf(szBuf, "%04u%s%02u%s%02u",	Date.year, DateSeparator,
            Date.day, DateSeparator, Date.month);
    break;

  case 2:

    sprintf(szBuf, "%04u%s%02u%s%02u",	Date.year, DateSeparator,
            Date.month, DateSeparator, Date.day);
    break;

  case 1:

    sprintf(szBuf, "%02u%s%02u%s%04u",	Date.day, DateSeparator,
            Date.month, DateSeparator, Date.year);
    break;

  case 0:
  default:

    sprintf(szBuf, "%02u%s%02u%s%04u",	Date.month, DateSeparator,
            Date.day, DateSeparator, Date.year);
    break;
  }
}

#pragma alloc_text(MISC8,commafmt,CommaFmtUL,CommaFmtULL)
