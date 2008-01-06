
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

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_OS2
#define INCL_WIN
#define INCL_LONGLONG			// dircnrs.h

#include "errutil.h"			// Dos_Error...
#include "fm3dll.h"

static PSZ pszSrcFile = __FILE__;

static INT index(const CHAR * s, const CHAR c);

/* Get index of char in string
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

/* literal()
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

  iBuf = 0;                                /* set index to first character */
  while (pszBuf[iBuf]) {
    switch (pszBuf[iBuf]) {
    case '\\':
      switch (pszBuf[iBuf + 1]) {
      case 'x':                        /* hexadecimal */
	wchar = 0;
	iBuf += 2;                        /* get past "\x" */
	if (index(HEX, (CHAR) toupper(pszBuf[iBuf])) != -1) {
	  iBufSave = iBuf;
	  while (((wpos = index(HEX, (CHAR) toupper(pszBuf[iBuf]))) != -1) &&
	         iBuf < iBufSave + 2) {
	    wchar = (CHAR) (wchar << 4) + (CHAR) wpos;
	    iBuf++;
	  }
	}
	else
	  wchar = 'x';                        /* just an x */
	iBuf--;
	*pszOut++ = wchar;
	break;

      case '\\':                        /* we want a "\" */
	iBuf++;
	*pszOut++ = '\\';
	break;

      case 't':                        /* tab CHAR */
	iBuf++;
	*pszOut++ = '\t';
	break;

      case 'n':                        /* new line */
	iBuf++;
	*pszOut++ = '\n';
	break;

      case 'r':                        /* carr return */
	iBuf++;
	*pszOut++ = '\r';
	break;

      case 'b':                        /* back space */
	iBuf++;
	*pszOut++ = '\b';
	break;

      case 'f':                        /* formfeed */
	iBuf++;
	*pszOut++ = '\x0c';
	break;

      case 'a':                        /* bell */
	iBuf++;
	*pszOut++ = '\07';
	break;

      case '\'':                        /* single quote */
	iBuf++;
	*pszOut++ = '\'';
	break;

      case '\"':                        /* double quote */

	iBuf++;
	*pszOut++ = '\"';
	break;

      default:                                /* decimal */
	iBuf++;                                /* get past "\" */
	wchar = 0;
	if (index(DEC, pszBuf[iBuf]) != -1) {
	  iBufSave = iBuf;
	  do {                                /* cvt to binary */
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
  *pszOut = 0;                                /* Always terminate, even if not string */

  cBufBytes = pszOut - pszWork;                /* Calc string length excluding terminator */
  memcpy(pszBuf, pszWork, cBufBytes + 1);        /* Overwrite including terminator */
  free(pszWork);

  return cBufBytes;                        /* Return string length */
}

/* Check wildcard match
 * @parm pszBuf Buffer to check
 * @parm pszWildCard wildcard to match
 * @parm fNotFileSpec TRUE if generic match else filespec match
 * @return TRUE if matched else FALSE
 */

BOOL wildcard(const PSZ pszBuf, const PSZ pszWildCard,
	      const BOOL fNotFileSpec)
{
  const CHAR *fstr = pszBuf;
  PSZ fcard = pszWildCard;
  CHAR *tcard;
  INT wmatch = TRUE;
  BOOL reverse = FALSE;

  while (wmatch && *fcard && *fstr) {
    if (*fcard == '*' && fcard[strlen(fcard) - 1] == '*' && !reverse){
      tcard  = xstrdup(fcard + 1, __FILE__, __LINE__);
      tcard[strlen(tcard) - 1] = 0;
      if (!(strchr(tcard, '?')) && !(strchr(tcard, '*'))){
	if (strstr(fstr, tcard)){ //strstr match for *stuff* pattern no wildcards in "stuff"
	  xfree(tcard);
	  return TRUE;
	}
	else{
	  xfree(tcard);
	  return FALSE;
	}
      }
      xfree(tcard);
    }
    else   //reverse search for *stuff pattern "stuff" can contain wildcards
      if (*fcard == '*' && fcard[strlen(fcard) - 1] != '*'){
	fstr = strrev(pszBuf);
	fcard = strrev(pszWildCard);
	reverse = TRUE;
      }
     switch (*fcard) { //fm2 standard forward search for all other cases
      case '?':                                /* character substitution */
	fcard++;
	if (fNotFileSpec || (*fstr != '.' && *fstr != '/' && *fstr != '\\'))
	  fstr++;                                /* skip (match) next character */
	break;

      case '*':
	/* find next non-wild character in wildcard */
	while (*fcard && (*fcard == '?' || *fcard == '*'))
	  fcard++;
	if (!*fcard){                        /* if last char of wildcard is *, it matches */
	  if (reverse){
	    fstr = strrev(pszBuf);
	    fcard = strrev(pszWildCard);
	  }
	  return TRUE;
	}
	/* skip until partition, match, or eos */
	while (*fstr && toupper(*fstr) != toupper(*fcard) &&
	       (fNotFileSpec || (*fstr != '\\' &&
	                         *fstr != '/' && *fstr != '.')))
	  fstr++;
	if (!fNotFileSpec && !*fstr)        /* implicit '.' */
	  if (*fcard == '.')
	    fcard++;
	break;

      default:
	if (!fNotFileSpec && ((*fstr == '/' || *fstr == '\\') &&
	                      (*fcard == '/' || *fcard == '\\')))
	  wmatch = TRUE;
	else
	  wmatch = (toupper(*fstr) == toupper(*fcard));
	fstr++;
	fcard++;
	break;
      }
  }  //while

  if ((*fcard && *fcard != '*') || *fstr){
    if (reverse){
      fstr = strrev(pszBuf);
      fcard = strrev(pszWildCard);
    }
    return 0;
  }
  else {
    if (reverse){
      fstr = strrev(pszBuf);
      fcard = strrev(pszWildCard);
    }
    return wmatch;
  }
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
	  sprintf(pchOut, "\\x%02x", (UCHAR)*pchIn);
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

