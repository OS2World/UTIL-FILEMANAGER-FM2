
/***********************************************************************

  $Id$

  Sort container items

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005 Steven H. Levine

  24 May 05 SHL Rework for CNRITEM.szSubject
  25 May 05 SHL Rework with ULONGLONG

***********************************************************************/

#define INCL_WIN
#define INCL_LONGLONG
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fm3dll.h"

#pragma alloc_text(SORTCNR,SortCnr,SortTreeCnr,SortDirCnr,SortCollectorCnr)


SHORT APIENTRY SortTreeCnr (PMINIRECORDCORE p1,PMINIRECORDCORE p2,
			    PVOID pStorage)
{
  return SortCnr(p1,p2,((pStorage) ? (INT)pStorage : TreesortFlags));
}


SHORT APIENTRY SortDirCnr (PMINIRECORDCORE p1,PMINIRECORDCORE p2,
			   PVOID pStorage)
{
  DIRCNRDATA *dcd = WinQueryWindowPtr(((PCNRITEM)p1)->hwndCnr,0);

  return SortCnr(p1,p2,((pStorage) ? (INT)pStorage :
		 (dcd && dcd->size == sizeof(DIRCNRDATA)) ?
		  dcd->sortFlags : sortFlags));
}


SHORT APIENTRY SortCollectorCnr (PMINIRECORDCORE p1,PMINIRECORDCORE p2,
				 PVOID pStorage)
{
  return SortCnr(p1,p2,((pStorage) ? (INT)pStorage : CollectorsortFlags));
}


SHORT SortCnr (PMINIRECORDCORE pRec1,PMINIRECORDCORE pRec2,INT SortFlags)
{
  PCNRITEM   pCI1 = (PCNRITEM)pRec1;
  PCNRITEM   pCI2 = (PCNRITEM)pRec2;
  SHORT      ret = 0;
  CHAR      *pch1,*pch2;

  if(SortFlags & SORT_NOSORT)
    return 0;
  if(SortFlags && pCI1->szFileName[3] && pCI2->szFileName[3]) {
    if(SortFlags & SORT_DIRSFIRST) {
      if((pCI1->attrFile & FILE_DIRECTORY) != (pCI2->attrFile & FILE_DIRECTORY))
	return (pCI1->attrFile & FILE_DIRECTORY) ? -1 : 1;
    }
    else if(SortFlags & SORT_DIRSLAST) {
      if((pCI1->attrFile & FILE_DIRECTORY) != (pCI2->attrFile & FILE_DIRECTORY))
	return (pCI1->attrFile & FILE_DIRECTORY) ? 1 : -1;
    }
    switch(SortFlags & (~(SORT_DIRSFIRST | SORT_DIRSLAST | SORT_REVERSE))) {
      case SORT_SUBJECT:
	if(*pCI1->szSubject && *pCI2->szSubject)
	  ret = stricmp(pCI1->szSubject,pCI2->szSubject);
	else {
	  ret = (*pCI2->szSubject) ? 1 : (*pCI1->szSubject) ? -1 : 0;
	  if(ret && (SortFlags & SORT_REVERSE))
	    ret = (ret > 0) ? -1 : 1;
	}
	break;

      case SORT_FILENAME:
	pch1 = strrchr(pCI1->szFileName,'\\');
	pch2 = strrchr(pCI2->szFileName,'\\');
	if(!pch1)
	  pch1 = NullStr;
	if(!pch2)
	  pch2 = NullStr;
	ret = stricmp(pch1,pch2);
	break;

      case SORT_FIRSTEXTENSION:
	pch1 = strrchr(pCI1->szFileName,'\\');
	pch2 = strrchr(pCI2->szFileName,'\\');
	if(!pch1)
	  pch1 = pCI1->szFileName;
	if(!pch2)
	  pch2 = pCI2->szFileName;
	pch1 = strchr(pch1,'.');
	pch2 = strchr(pch2,'.');
	if(!pch1)
	  pch1 = NullStr;
	if(!pch2)
	  pch2 = NullStr;
	ret = stricmp(pch1,pch2);
	break;

      case SORT_LASTEXTENSION:
	pch1 = strrchr(pCI1->szFileName,'\\');
	pch2 = strrchr(pCI2->szFileName,'\\');
	if(!pch1)
	  pch1 = pCI1->szFileName;
	if(!pch2)
	  pch2 = pCI2->szFileName;
	pch1 = strrchr(pch1,'.');
	pch2 = strrchr(pch2,'.');
	if(!pch1)
	  pch1 = NullStr;
	if(!pch2)
	  pch2 = NullStr;
	ret = stricmp(pch1,pch2);
	break;

      case SORT_SIZE:
	ret = (pCI1->cbFile < pCI2->cbFile) ? 1 : (pCI1->cbFile == pCI2->cbFile) ?
	      0 : -1;
	if(!ret)
	  ret = (pCI1->easize < pCI2->easize) ? 1 : (pCI1->easize == pCI2->easize) ?
		0 : -1;
	break;

      case SORT_EASIZE:
	ret = (pCI1->easize < pCI2->easize) ? 1 : (pCI1->easize == pCI2->easize) ?
	      0 : -1;
	if(!ret)
	  ret = (pCI1->cbFile < pCI2->cbFile) ? 1 : (pCI1->cbFile == pCI2->cbFile) ?
		0 : -1;
	break;

      case SORT_LWDATE:
	ret = (pCI1->date.year < pCI2->date.year) ? 1 :
	      (pCI1->date.year > pCI2->date.year) ? -1 :
	      (pCI1->date.month < pCI2->date.month) ? 1 :
	      (pCI1->date.month > pCI2->date.month) ? -1 :
	      (pCI1->date.day < pCI2->date.day) ? 1 :
	      (pCI1->date.day > pCI2->date.day) ? -1 :
	      (pCI1->time.hours < pCI2->time.hours) ? 1 :
	      (pCI1->time.hours > pCI2->time.hours) ? -1 :
	      (pCI1->time.minutes < pCI2->time.minutes) ? 1 :
	      (pCI1->time.minutes > pCI2->time.minutes) ? -1 :
	      (pCI1->time.seconds < pCI2->time.seconds) ? 1 :
	      (pCI1->time.seconds > pCI2->time.seconds) ? -1 : 0;
	break;

      case SORT_LADATE:
	ret = (pCI1->ladate.year < pCI2->ladate.year) ? 1 :
	      (pCI1->ladate.year > pCI2->ladate.year) ? -1 :
	      (pCI1->ladate.month < pCI2->ladate.month) ? 1 :
	      (pCI1->ladate.month > pCI2->ladate.month) ? -1 :
	      (pCI1->ladate.day < pCI2->ladate.day) ? 1 :
	      (pCI1->ladate.day > pCI2->ladate.day) ? -1 :
	      (pCI1->latime.hours < pCI2->latime.hours) ? 1 :
	      (pCI1->latime.hours > pCI2->latime.hours) ? -1 :
	      (pCI1->latime.minutes < pCI2->latime.minutes) ? 1 :
	      (pCI1->latime.minutes > pCI2->latime.minutes) ? -1 :
	      (pCI1->latime.seconds < pCI2->latime.seconds) ? 1 :
	      (pCI1->latime.seconds > pCI2->latime.seconds) ? -1 : 0;
	break;

      case SORT_CRDATE:
	ret = (pCI1->crdate.year < pCI2->crdate.year) ? 1 :
	      (pCI1->crdate.year > pCI2->crdate.year) ? -1 :
	      (pCI1->crdate.month < pCI2->crdate.month) ? 1 :
	      (pCI1->crdate.month > pCI2->crdate.month) ? -1 :
	      (pCI1->crdate.day < pCI2->crdate.day) ? 1 :
	      (pCI1->crdate.day > pCI2->crdate.day) ? -1 :
	      (pCI1->crtime.hours < pCI2->crtime.hours) ? 1 :
	      (pCI1->crtime.hours > pCI2->crtime.hours) ? -1 :
	      (pCI1->crtime.minutes < pCI2->crtime.minutes) ? 1 :
	      (pCI1->crtime.minutes > pCI2->crtime.minutes) ? -1 :
	      (pCI1->crtime.seconds < pCI2->crtime.seconds) ? 1 :
	      (pCI1->crtime.seconds > pCI2->crtime.seconds) ? -1 : 0;
	break;
    }

    if(!ret)
      ret = (SHORT)stricmp(pCI1->szFileName,pCI2->szFileName);

    if(ret && (SortFlags & SORT_REVERSE))
      ret = (ret > 0) ? -1 : 1;

    return ret;
  }
  return (SHORT)stricmp(pCI1->szFileName,pCI2->szFileName);
}
