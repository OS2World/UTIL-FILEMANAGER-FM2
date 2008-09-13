
/***********************************************************************

  $Id$

  Sort container items

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2007 Steven H. Levine

  24 May 05 SHL Rework for CNRITEM.szSubject
  25 May 05 SHL Rework with ULONGLONG
  22 Mar 07 GKY Use QWL_USER
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#include <string.h>

#define INCL_WIN

#include "fm3dll.h"
#include "dircnrs.h"			// Data declaration(s)
#include "treecnr.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "collect.h"			// Data declarations
#include "sortcnr.h"
#include "valid.h"                      // TestCDates

static SHORT SortCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2, INT Sortflags);

SHORT APIENTRY SortTreeCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2,
                           PVOID pStorage)
{
  return SortCnr(p1, p2, ((pStorage) ? (INT) pStorage : TreesortFlags));
}

SHORT APIENTRY SortDirCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2,
                          PVOID pStorage)
{
  DIRCNRDATA *dcd = WinQueryWindowPtr(((PCNRITEM) p1)->hwndCnr, QWL_USER);

  return SortCnr(p1, p2, ((pStorage) ? (INT) pStorage :
                          (dcd && dcd->size == sizeof(DIRCNRDATA)) ?
                          dcd->sortFlags : sortFlags));
}

SHORT APIENTRY SortCollectorCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2,
                                PVOID pStorage)
{
  return SortCnr(p1, p2, ((pStorage) ? (INT) pStorage : CollectorsortFlags));
}

SHORT SortCnr(PMINIRECORDCORE pRec1, PMINIRECORDCORE pRec2, INT SortFlags)
{
  PCNRITEM pCI1 = (PCNRITEM) pRec1;
  PCNRITEM pCI2 = (PCNRITEM) pRec2;
  SHORT ret = 0;
  CHAR *pch1, *pch2;

  if (SortFlags & SORT_NOSORT)
    return 0;
  if (SortFlags && pCI1->pszFileName + 3 && pCI2->pszFileName + 3) {
    if (SortFlags & SORT_DIRSFIRST) {
      if ((pCI1->attrFile & FILE_DIRECTORY) !=
          (pCI2->attrFile & FILE_DIRECTORY))
        return (pCI1->attrFile & FILE_DIRECTORY) ? -1 : 1;
    }
    else if (SortFlags & SORT_DIRSLAST) {
      if ((pCI1->attrFile & FILE_DIRECTORY) !=
          (pCI2->attrFile & FILE_DIRECTORY))
        return (pCI1->attrFile & FILE_DIRECTORY) ? 1 : -1;
    }
    switch (SortFlags & (~(SORT_DIRSFIRST | SORT_DIRSLAST | SORT_REVERSE))) {
    case SORT_SUBJECT:
      if (*pCI1->pszSubject && *pCI2->pszSubject)
        ret = stricmp(pCI1->pszSubject, pCI2->pszSubject);
      else {
        ret = (*pCI2->pszSubject) ? 1 : (*pCI1->pszSubject) ? -1 : 0;
        if (ret && (SortFlags & SORT_REVERSE))
          ret = (ret > 0) ? -1 : 1;
      }
      break;

    case SORT_FILENAME:
      pch1 = strrchr(pCI1->pszFileName, '\\');
      pch2 = strrchr(pCI2->pszFileName, '\\');
      if (!pch1)
        pch1 = NullStr;
      if (!pch2)
        pch2 = NullStr;
      ret = stricmp(pch1, pch2);
      break;

    case SORT_FIRSTEXTENSION:
      pch1 = strrchr(pCI1->pszFileName, '\\');
      pch2 = strrchr(pCI2->pszFileName, '\\');
      if (!pch1)
        pch1 = pCI1->pszFileName;
      if (!pch2)
        pch2 = pCI2->pszFileName;
      pch1 = strchr(pch1, '.');
      pch2 = strchr(pch2, '.');
      if (!pch1)
        pch1 = NullStr;
      if (!pch2)
        pch2 = NullStr;
      ret = stricmp(pch1, pch2);
      break;

    case SORT_LASTEXTENSION:
      pch1 = strrchr(pCI1->pszFileName, '\\');
      pch2 = strrchr(pCI2->pszFileName, '\\');
      if (!pch1)
        pch1 = pCI1->pszFileName;
      if (!pch2)
        pch2 = pCI2->pszFileName;
      pch1 = strrchr(pch1, '.');
      pch2 = strrchr(pch2, '.');
      if (!pch1)
        pch1 = NullStr;
      if (!pch2)
        pch2 = NullStr;
      ret = stricmp(pch1, pch2);
      break;

    case SORT_SIZE:
      ret =
        (pCI1->cbFile < pCI2->cbFile) ? 1 : (pCI1->cbFile ==
                                             pCI2->cbFile) ? 0 : -1;
      if (!ret)
        ret =
          (pCI1->easize < pCI2->easize) ? 1 : (pCI1->easize ==
                                               pCI2->easize) ? 0 : -1;
      break;

    case SORT_EASIZE:
      ret =
        (pCI1->easize < pCI2->easize) ? 1 : (pCI1->easize ==
                                             pCI2->easize) ? 0 : -1;
      if (!ret)
        ret =
          (pCI1->cbFile < pCI2->cbFile) ? 1 : (pCI1->cbFile ==
                                               pCI2->cbFile) ? 0 : -1;
      break;

    case SORT_LWDATE:
      ret =  TestCDates(&pCI1->date, &pCI1->time,
                        &pCI2->date, &pCI2->time);
        /*(pCI1->date.year < pCI2->date.year) ? 1 :
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
        (pCI1->time.seconds > pCI2->time.seconds) ? -1 : 0;*/
      break;

    case SORT_LADATE:
      ret = TestCDates(&pCI1->ladate, &pCI1->latime,
                       &pCI2->ladate, &pCI2->latime);
        /*(pCI1->ladate.year < pCI2->ladate.year) ? 1 :
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
        (pCI1->latime.seconds > pCI2->latime.seconds) ? -1 : 0;*/
      break;

    case SORT_CRDATE:
      ret = TestCDates(&pCI1->crdate, &pCI1->crtime,
                       &pCI2->crdate, &pCI2->crtime);
        /*(pCI1->crdate.year < pCI2->crdate.year) ? 1 :
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
        (pCI1->crtime.seconds > pCI2->crtime.seconds) ? -1 : 0;*/
      break;
    }

    if (!ret)
      ret = (SHORT) stricmp(pCI1->pszFileName, pCI2->pszFileName);

    if (ret && (SortFlags & SORT_REVERSE))
      ret = (ret > 0) ? -1 : 1;

    return ret;
  }
  return (SHORT) stricmp(pCI1->pszFileName, pCI2->pszFileName);
}

#pragma alloc_text(SORTCNR,SortCnr,SortTreeCnr,SortDirCnr,SortCollectorCnr)
