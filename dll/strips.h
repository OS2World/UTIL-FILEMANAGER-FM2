
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(STRIPS_H)
#define STRIPS_H

VOID chop_at_crnl(PSZ pszSrc);
PSZ convert_nl_to_nul(PSZ pszSrc);
VOID remove_first_occurence_of_character(char *pszRemoveChar, char *pszSrc);

/* strips.c */
void strip_lead_char(char *pszStripChars, char *pszSrc);
void strip_trail_char(char *pszStripChars, char *pszSrc);
#define lstrip(s)         strip_lead_char(" \t",(s))
#define rstrip(s)         strip_trail_char(" \t",(s))
#define stripcr(s)        strip_trail_char("\r\n",(s))
// Strip leading and trailing white
#define bstrip(s)         (strip_lead_char(" \t",(s)),strip_trail_char(" \t",(s)))
// Strip leading and trailing white and trail cr/nl
#define bstripcr(s)       (strip_lead_char(" \t",(s)),strip_trail_char("\r\n \t",(s)))



#endif // STRIPS_H
