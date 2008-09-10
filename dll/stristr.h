
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(STRISTR_H)
#define STRISTR_H

CHAR *findstring(CHAR * findthis, ULONG lenthis, CHAR * findin,
		 ULONG lenin, BOOL insensitive);
CHAR *stristr(const register CHAR * t, const CHAR * s);
CHAR *strnstr(register CHAR * t, CHAR * s, LONG len);


#endif // STRISTR_H
