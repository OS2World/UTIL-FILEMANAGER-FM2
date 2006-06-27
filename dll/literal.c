
/***********************************************************************

  $Id$

  string quoting utilities
  wildcarding utilities

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2006 Steven H.Levine

  Archive containers

  01 Aug 04 SHL Rework fixup to avoid overflows
  16 Jun 06 SHL liternal: comments

***********************************************************************/

#define INCL_OS2
#define INCL_WIN

#include <os2.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fm3dll.h"

static INT index(const CHAR *s,const CHAR c);

#pragma alloc_text(LITERAL,literal,index,fixup,wildcard)

/* Get index of char in string
 * @parm s string to search
 * @parm c char to search for
 * @return 0 relative index of c in s or -1
 */

static INT index(const CHAR *s,const CHAR c)
{
    CHAR *p;

    p = strchr(s,c);
    if(p == NULL || !*p)
      return -1;
    return (INT)(p - s);
}

/* Translate a string with \ escape tokens to binary equivalent
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
 * Return converted character count
 * Does not include terminating nul
 */

#define HEX "0123456789ABCDEF"
#define DEC "0123456789"

UINT literal(PSZ pszBuf)
{
  INT	wpos;
  INT	iBuf;
  UINT  cBufBytes;
  INT   iBufSave;
  PSZ	pszOut;
  PSZ	pszWork;
  CHAR  wchar;

  if(!pszBuf ||	!*pszBuf)
    return 0;
  cBufBytes = strlen(pszBuf) + 1;
  pszWork = pszOut = malloc(cBufBytes + 1);

  iBuf = 0;                             /* set index to first character */
  while(pszBuf[iBuf]) {
    switch(pszBuf[iBuf]) {
      case '\\':
	switch(pszBuf[iBuf + 1]) {
	  case 'x' :			/* hexadecimal */
	    wchar = 0;
	    iBuf += 2;                  /* get past "\x" */
	    if(index(HEX,(CHAR)toupper(pszBuf[iBuf])) != -1) {
	      iBufSave = iBuf;
	      while(((wpos = index(HEX,(CHAR)toupper(pszBuf[iBuf]))) != -1) &&
		    iBuf < iBufSave + 2) {
		wchar = (CHAR)(wchar << 4) + (CHAR)wpos;
		iBuf++;
	      }
	    }
	    else
	      wchar = 'x';		/* just an x */
	    iBuf--;
	    *pszOut++ = wchar;
	    break;

	  case '\\' :			/* we want a "\" */
	    iBuf++;
	    *pszOut++ = '\\';
	    break;

	  case 't' :			/* tab CHAR */
	    iBuf++;
	    *pszOut++ = '\t';
	    break;

	  case 'n' :			/* new line */
	    iBuf++;
	    *pszOut++ = '\n';
	    break;

	  case 'r' :			/* carr return */
	    iBuf++;
	    *pszOut++ = '\r';
	    break;

	  case 'b' :			/* back space */
	    iBuf++;
	    *pszOut++ = '\b';
	    break;

	  case 'f':			/* formfeed */
	    iBuf++;
	    *pszOut++ = '\x0c';
	    break;

	  case 'a':			/* bell */
	    iBuf++;
	    *pszOut++ = '\07';
	    break;

	  case '\'' :			/* single quote */
	    iBuf++;
	    *pszOut++ = '\'';
	    break;

	  case '\"' :			/* double quote */
	    iBuf++;
	    *pszOut++ = '\"';
	    break;

	  default :			/* decimal */
	    iBuf++;                     /* get past "\" */
	    wchar = 0;
	    if(index(DEC,pszBuf[iBuf]) != -1) {
	      iBufSave = iBuf;
	      do {			/* cvt to binary */
		wchar = (CHAR)(wchar * 10 + (pszBuf[iBuf++] - 48));
	      } while (index(DEC,pszBuf[iBuf]) != -1 && iBuf < iBufSave + 3);
	      iBuf--;
	    }
	    else
	      wchar = pszBuf[iBuf];
	    *pszOut ++ = wchar;
	    break;
	} // switch
	break;

      default :
	*pszOut++ = pszBuf[iBuf];
	break;
   } // switch
   iBuf++;
  } // while
  *pszOut = 0;				/* Always terminate, even if not string */

  cBufBytes = pszOut - pszWork;		/* Calc string length excluding terminator */
  memcpy(pszBuf,pszWork,cBufBytes + 1);	/* Overwrite including terminator */
  free(pszWork);

  return cBufBytes;                     /* Return string length */
}

/* Check wildcard match
 * @parm pszBuf Buffer to check
 * @parm pszWildCard wildcard to match
 * @parm fNotFileSpec TRUE if generic match else filespec match
 * @return TRUE if matched else FALSE
 */


BOOL wildcard(const PSZ pszBuf,const PSZ pszWildCard,const BOOL fNotFileSpec)
{
  const CHAR *fstr = pszBuf;
  PSZ fcard = pszWildCard;
  INT         wmatch = TRUE;

  while(wmatch && *fcard && *fstr) {
    switch(*fcard) {
      case '?' :                        /* character substitution */
	 fcard++;
	 if(fNotFileSpec || (*fstr != '.' && *fstr != '/' && *fstr != '\\'))
	   fstr++;                      /* skip (match) next character */
	 break;

      case '*' :
	 /* find next non-wild character in wildcard */
	 while(*fcard && (*fcard == '?' || *fcard == '*'))
	   fcard++;
	 if(!*fcard)   /* if last char of wildcard is *, it matches */
	   return TRUE;
	 /* skip until partition, match, or eos */
	 while(*fstr && toupper(*fstr) != toupper(*fcard) &&
	       (fNotFileSpec || (*fstr != '\\' &&
	       *fstr != '/' && *fstr != '.')))
	   fstr++;
	 if(!fNotFileSpec && !*fstr)    /* implicit '.' */
	   if(*fcard == '.')
	     fcard++;
	 break;

      default  :
	 if(!fNotFileSpec && ((*fstr == '/' || *fstr == '\\') &&
	    (*fcard == '/' || *fcard == '\\')))
	   wmatch = TRUE;
	 else
	   wmatch = (toupper(*fstr) == toupper(*fcard));
	 fstr++;
	 fcard++;
	 break;
    }
  }

  if ((*fcard && *fcard != '*') || *fstr)
    return 0;
  else
    return wmatch;
}


// fixup - quote literal character array

PSZ fixup(const PCH pachIn, PSZ pszOutBuf, const UINT cBufBytes, const UINT cInBytes)
{
  PCH	pchIn = pachIn;
  PCH	pchOut = pszOutBuf;
  PSZ	pszTemp;
  static CHAR	szTemp[5] = "\\x";	// Constant prefix

  // input is a character array, not a string - may not be null terminated
  // cBufBytes is buffer size
  if (pachIn) {
    // Ensure room for null and possible \ escape
    while (pchIn - pachIn < cInBytes &&
	   pchOut - pszOutBuf + 2 < cBufBytes) {
      if(!isprint(*pchIn)) {
	if(*pchIn == '\r') {
	  *pchOut++ = '\\';
	  *pchOut++ = 'r';
	}
	else if(*pchIn == '\n') {
	  *pchOut++ = '\\';
	  *pchOut++ = 'n';
	}
	else if(*pchIn == '\b') {
	  *pchOut++ = '\\';
	  *pchOut++ = 'b';
	}
	else {
	  sprintf(szTemp + 2,"%02hx",*pchIn);
	  for (pszTemp = szTemp; *pszTemp;)
	    *pchOut++ = *pszTemp++;
	}
	pchIn++;
      }
      else if(*pchIn == '\\') {
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

