#define INCL_OS2
#define INCL_WIN

#include <os2.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fm3dll.h"

#pragma alloc_text(LITERAL,literal,index,fixup,wildcard)


INT index (const CHAR *s,const CHAR c) {

    CHAR *p;

    p = strchr(s,c);
    if(p == NULL || !*p)
      return -1;
    return (INT)(p - s);
}


/*
 * literal.c
 * Translate a string with tokens in it according to several conventions:
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
 */

#define HEX "0123456789ABCDEF"
#define DEC "0123456789"

INT literal (CHAR *fsource) {

  register INT wpos,w;
  INT          len,oldw;
  CHAR        *fdestin,*freeme,wchar;

  if(!fsource ||
     !*fsource)
    return 0;
  len = strlen(fsource) + 1;
  freeme = fdestin = malloc(len + 1);
  memset(fdestin,0,len);              /* start out with zero'd string */

  w = 0;                              /* set index to first character */
  while(fsource[w]) {                 /* until end of string */
    switch(fsource[w]) {
      case '\\':
        switch(fsource[w + 1]) {
          case 'x' :                  /* hexadecimal */
            wchar = 0;
            w += 2;                   /* get past "\x" */
            if(index(HEX,(CHAR)toupper(fsource[w])) != -1) {
              oldw = w;
              while(((wpos = index(HEX,(CHAR)toupper(fsource[w]))) != -1) &&
                    w < oldw + 2) {
                wchar = (CHAR)(wchar << 4) + (CHAR)wpos;
                w++;
              }
            }
            else
              wchar = 'x';            /* just an x */
            w--;
            *fdestin++ = wchar;
            break;

          case '\\' :                 /* we want a "\" */
            w++;
            *fdestin++ = '\\';
            break;

          case 't' :                  /* tab CHAR */
            w++;
            *fdestin++ = '\t';
            break;

          case 'n' :                  /* new line */
            w++;
            *fdestin++ = '\n';
            break;

          case 'r' :                  /* carr return */
            w++;
            *fdestin++ = '\r';
            break;

          case 'b' :                  /* back space */
            w++;
            *fdestin++ = '\b';
            break;

          case 'f':                   /* formfeed */
            w++;
            *fdestin++ = '\x0c';
            break;

          case 'a':                   /* bell */
            w++;
            *fdestin++ = '\07';
            break;

          case '\'' :                 /* single quote */
            w++;
            *fdestin++ = '\'';
            break;

          case '\"' :                 /* double quote */
            w++;
            *fdestin++ = '\"';
            break;

          default :                   /* decimal */
            w++;                      /* get past "\" */
            wchar = 0;
            if(index(DEC,fsource[w]) != -1) {
              oldw = w;
              do {                    /* cvt to binary */
                wchar = (CHAR)(wchar * 10 + (fsource[w++] - 48));
              } while (index(DEC,fsource[w]) != -1 && w < oldw + 3);
              w--;
            }
            else
              wchar = fsource[w];
            *fdestin ++ = wchar;
            break;
        }
        break;

      default :
        *fdestin++ = fsource[w];
        break;
   }
   w++;
  }
  *fdestin = 0;                               /* terminate the string */

  len = fdestin - freeme;
  memcpy(fsource,freeme,len + 1);             /* swap 'em */
  free(freeme);

  return len;                                 /* return length of string */
}



INT wildcard (const CHAR *fstra,const CHAR *fcarda,const BOOL notfile) {

  /* returns TRUE if match */

  register const CHAR *fstr = fstra,*fcard = fcarda;
  register INT         wmatch = TRUE;

  while(wmatch && *fcard && *fstr) {
    switch(*fcard) {
      case '?' :                        /* character substitution */
         fcard++;
         if(notfile || (*fstr != '.' && *fstr != '/' && *fstr != '\\'))
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
               (notfile || (*fstr != '\\' &&
               *fstr != '/' && *fstr != '.')))
           fstr++;
         if(!notfile && !*fstr)                   /* implicit '.' */
           if(*fcard == '.')
             fcard++;
         break;

      default  :
         if(!notfile && ((*fstr == '/' || *fstr == '\\') &&
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


CHAR * fixup (const CHAR *orig,CHAR *dest,const INT maxlen,const INT datalen) {

  register const CHAR *o = orig;
  register CHAR       *d = dest,*tp;
  CHAR                 temp[33] = "\\x";

  *d = 0;
  if(orig) {
    while((o - orig) <
           datalen &&
           (d - dest) < maxlen) {
      if(!isprint(*o)) {
        if(*o == '\r') {
          *d = '\\';
          d++;
          *d = 'r';
          d++;
        }
        else if(*o == '\n') {
          *d = '\\';
          d++;
          *d = 'n';
          d++;
        }
        else if(*o == '\b') {
          *d = '\\';
          d++;
          *d = 'b';
          d++;
        }
        else {
          sprintf(&temp[2],"%02hx",*o);
          tp = temp;
          while(*tp)
            *d++ = *tp++;
        }
        o++;
      }
      else {
        if(*o == '\\') {
          *d = '\\';
          d++;
          *d = '\\';
          d++;
          o++;
        }
        else
          *d++ = *o++;
      }
      *d = 0;
    }
  }
  return dest;
}

