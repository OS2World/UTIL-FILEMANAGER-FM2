
/***********************************************************************

  $Id$

  string quoting utilities
  wildcarding utilities

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2007 Steven H.Levine

  Archive containers

  01 Aug 04 SHL Rework fixup to avoid overflows
  16 Jun 06 SHL liternal: comments
  22 Jul 06 SHL Check more run time errors
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  16 Nov 07 SHL Report fixup buffer overflow
  15 Mar 08 KOMH Fix wildcard for multiple dots
  24 Apr 09 SHL Rework wildcard for clarity

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_OS2
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "errutil.h"			// Dos_Error...
#include "literal.h"
#include "wrappers.h"			// xmalloc
#include "fortify.h"
#include "init.h"                       // Strings

static PSZ pszSrcFile = __FILE__;

static INT index(const CHAR * s, const CHAR c);

/**
 * Get index of char in string
 * @parm s string to search
 * @parm c char to search for
 * @return 0 relative index of c in s or -1
 */

static INT index(const CHAR * s, const CHAR c)
{
  CHAR *p;

  p = strchr(s, c);
  if (p == NULL || !*p)
    return -1;
  return (INT) (p - s);
}

/**
 * literal()
 * Translate a string with \ escape tokens to binary equivalent
 * Translates in place
 *
 * 1.  \x1b translates to CHAR(0x1b)
 * 2.  \27  translates to CHAR(27)
 * 3.  \"   translates to "
 * 4.  \'   translates to '
 * 5.  \\   translates to \
 * 6.  \r   translates to carriage return
 * 7.  \n   translates to linefeed
 * 8.  \b   translates to backspace
 * 9.  \t   translates to tab
 * 10. \a   translates to bell
 * 11. \f   translates to formfeed
 *
 * Synopsis
 *    *s = "this\x20is\32a test of \\MSC\\CSM\7"
 *    literal(s);
 *
 *    ( s now equals "this is a test of \MSC\CSM")
 *
 * Return converted character count like strlen()
 * Count does not include terminating nul
 */

#define HEX "0123456789ABCDEF"
#define DEC "0123456789"

UINT literal(PSZ pszBuf)
{
  INT wpos;
  INT iBuf;
  UINT cBufBytes;
  INT iBufSave;
  PSZ pszOut;
  PSZ pszWork;
  CHAR wchar;

  if (!pszBuf || !*pszBuf)
    return 0;
  cBufBytes = strlen(pszBuf) + 1;
  pszWork = pszOut = xmalloc(cBufBytes + 1, pszSrcFile, __LINE__);
  if (!pszWork)
    return 0;

  iBuf = 0;                                // set index to first character
  while (pszBuf[iBuf]) {
    switch (pszBuf[iBuf]) {
    case '\\':
      switch (pszBuf[iBuf + 1]) {
      case 'x':                        // hexadecimal
	wchar = 0;
	iBuf += 2;                        // get past "\x"
	if (index(HEX, (CHAR) toupper(pszBuf[iBuf])) != -1) {
	  iBufSave = iBuf;
	  while (((wpos = index(HEX, (CHAR) toupper(pszBuf[iBuf]))) != -1) &&
	         iBuf < iBufSave + 2) {
	    wchar = (CHAR) (wchar << 4) + (CHAR) wpos;
	    iBuf++;
	  }
	}
	else
	  wchar = 'x';                        // just an x
	iBuf--;
	*pszOut++ = wchar;
	break;

      case '\\':                        // we want a "\"
	iBuf++;
	*pszOut++ = '\\';
	break;

      case 't':                        // tab CHAR
	iBuf++;
	*pszOut++ = '\t';
	break;

      case 'n':                        // new line
	iBuf++;
	*pszOut++ = '\n';
	break;

      case 'r':                        // carr return
	iBuf++;
	*pszOut++ = '\r';
	break;

      case 'b':                        // back space
	iBuf++;
	*pszOut++ = '\b';
	break;

      case 'f':                        // formfeed
	iBuf++;
	*pszOut++ = '\x0c';
	break;

      case 'a':                        // bell
	iBuf++;
	*pszOut++ = '\07';
	break;

      case '\'':                        // single quote
	iBuf++;
	*pszOut++ = '\'';
	break;

      case '\"':                        // double quote

	iBuf++;
	*pszOut++ = '\"';
	break;

      default:                                // decimal
	iBuf++;                                // get past "\"
	wchar = 0;
	if (index(DEC, pszBuf[iBuf]) != -1) {
	  iBufSave = iBuf;
	  do {                                // cvt to binary
	    wchar = (CHAR) (wchar * 10 + (pszBuf[iBuf++] - 48));
	  } while (index(DEC, pszBuf[iBuf]) != -1 && iBuf < iBufSave + 3);
	  iBuf--;
	}
	else
	  wchar = pszBuf[iBuf];
	*pszOut++ = wchar;
	break;
      }                                        // switch
      break;

    default:
      *pszOut++ = pszBuf[iBuf];
      break;
    }                                        // switch
    iBuf++;
  }                                        // while
  *pszOut = 0;                                // Always terminate, even if not string

  cBufBytes = pszOut - pszWork;                // Calc string length excluding terminator
  memcpy(pszBuf, pszWork, cBufBytes + 1);        // Overwrite including terminator
  free(pszWork);

  return cBufBytes;                        // Return string length
}

/** Check wildcard match
 * @parm pszBuf Buffer to check
 * @parm pszWildCard wildcard to match
 * @parm fIgnorePathSep TRUE if match ignores path separators
 * @return TRUE if matched else FALSE
 * @fixme need to rework to scan from right to left if not ignoring path separators
 */

BOOL wildcard(const PSZ pszBufIn, const PSZ pszWildCardIn,
	      const BOOL fIgnorePathSep)
{
    PSZ pszBuf = pszBufIn;
    PSZ pszWild = pszWildCardIn;

    while (*pszBuf && *pszWild) {
      switch (*pszWild) {
        case '*' :
        {
	  PSZ pszLook;

          // find next non-wild character in wildcard
	  while (*pszWild && ( *pszWild == '*' || *pszWild == '?'))
	    pszWild++;

	  // if last char of wildcard is *, got match
	  if (!*pszWild)
            return TRUE;

	  pszLook = pszBuf;
	  while (*pszLook) {
	    // scan until match, eos or path separator (maybe)
	    while (*pszLook && toupper(*pszLook) != toupper(*pszWild) &&
		   (fIgnorePathSep || ( *pszLook != '/' && *pszLook != '\\')))
	      pszLook++;

	    // If eos or path separator (maybe), stop scan
	    if (!*pszLook || (!fIgnorePathSep && (*pszLook == '/' || *pszLook == '\\')))
              break;

	    // Not ignoring path separators, match next path component
	    if (wildcard(pszLook, pszWild, fIgnorePathSep) == TRUE)
              return TRUE;

	    pszLook++;
	  } // while

	  pszBuf = pszLook;
          break;
        }

        case '?' :          // character substitution
	  pszWild++;

	  if (fIgnorePathSep || (*pszBuf != '.' && *pszBuf != '/' && *pszBuf != '\\'))
	    pszBuf++;     // skip (match) next character
          break;

        default :
	  if (fIgnorePathSep || (*pszBuf  != '/' && *pszBuf  != '\\') ||
	      (*pszWild != '/' && *pszWild != '\\')) {
	    if (toupper( *pszBuf ) != toupper( *pszWild))
              return FALSE;
          }

	  pszWild++;
	  pszBuf++;
          break;
      } // switch
    } // while

    if (!*pszBuf) {
      // Skip trailing * and ?
      while (*pszWild && (*pszWild == '?' || *pszWild == '*'))
	pszWild++;

      if (!fIgnorePathSep) {
        // remove trailing .
	while (*pszWild && *pszWild == '.')
	  pszWild++;
      }
    }

    return (*pszBuf == *pszWild);
}

// fixup - quote literal character array

PSZ fixup(const PCH pachIn, PSZ pszOutBuf, const UINT cBufBytes,
	  const UINT cInBytes)
{
  PCH pchIn = pachIn;
  PCH pchOut = pszOutBuf;

  // input is a character array, not a string - may not be null terminated
  // cBufBytes is buffer size
  if (pachIn) {
    // 16 Nov 07 SHL fixme to optimize counting and speed
    // Ensure room for null and possible \ escape
    while (pchIn - pachIn < cInBytes) {
      if (pchOut - pszOutBuf + 4 >= cBufBytes) {
	*pchOut = 0;
	Runtime_Error(pszSrcFile, __LINE__, "buffer too small for %s", pszOutBuf);
	break;
      }

      if (!isprint(*pchIn)) {
	if (*pchIn == '\r') {
	  *pchOut++ = '\\';
	  *pchOut++ = 'r';
	}
	else if (*pchIn == '\n') {
	  *pchOut++ = '\\';
	  *pchOut++ = 'n';
	}
	else if (*pchIn == '\b') {
	  *pchOut++ = '\\';
	  *pchOut++ = 'b';
	}
	else {
	  sprintf(pchOut, "%sx%02x", PCSZ_BACKSLASH, (UCHAR)*pchIn);
	  pchOut += 4;
	}
	pchIn++;
      }
      else if (*pchIn == '\\') {
	*pchOut++ = '\\';
	*pchOut++ = '\\';
	pchIn++;
      }
      else
	*pchOut++ = *pchIn++;
    } // while

  } // if pachIn
  *pchOut = 0;
  return pszOutBuf;
}

#pragma alloc_text(LITERAL,literal,index,fixup,wildcard)

