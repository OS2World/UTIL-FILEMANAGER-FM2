
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2009 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(INPUT_H)
#define INPUT_H

typedef struct
{
  PCSZ title;			/* title of dialog */
  PCSZ prompt;			/* prompt to user */
  CHAR *ret;			/* buffer out, default in */
  PCSZ help;			/* help text */
  INT inputlen;			/* max len of ret */
}
STRINGINPARMS;

MRESULT EXPENTRY InputDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


#endif // INPUT_H
