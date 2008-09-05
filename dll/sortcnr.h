
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(SORTCNR_H)
#define SORTCNR_H

SHORT APIENTRY SortCollectorCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2,
				PVOID pStorage);
SHORT APIENTRY SortDirCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2,
			  PVOID pStorage);
SHORT APIENTRY SortTreeCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2,
			   PVOID pStorage);


#endif // SORTCNR_H
