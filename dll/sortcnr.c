#define INCL_WIN

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fm3dll.h"

#pragma alloc_text(SORTCNR,SortCnr,SortTreeCnr,SortDirCnr,SortCollectorCnr)


SHORT APIENTRY SortTreeCnr (PMINIRECORDCORE p1,PMINIRECORDCORE p2,
                            PVOID pStorage) {

  return SortCnr(p1,p2,((pStorage) ? (INT)pStorage : TreesortFlags));
}


SHORT APIENTRY SortDirCnr (PMINIRECORDCORE p1,PMINIRECORDCORE p2,
                           PVOID pStorage) {

  DIRCNRDATA *dcd = WinQueryWindowPtr(((PCNRITEM)p1)->hwndCnr,0);

  return SortCnr(p1,p2,((pStorage) ? (INT)pStorage :
                 (dcd && dcd->size == sizeof(DIRCNRDATA)) ?
                  dcd->sortFlags : sortFlags));
}


SHORT APIENTRY SortCollectorCnr (PMINIRECORDCORE p1,PMINIRECORDCORE p2,
                                 PVOID pStorage) {

  return SortCnr(p1,p2,((pStorage) ? (INT)pStorage : CollectorsortFlags));
}


SHORT SortCnr (PMINIRECORDCORE p1,PMINIRECORDCORE p2,INT SortFlags) {

  PCNRITEM   p = (PCNRITEM)p1,pp = (PCNRITEM)p2;
  SHORT      ret = 0;
  CHAR      *pext,*ppext;

  if(SortFlags & SORT_NOSORT)
    return 0;
  if(SortFlags && p->szFileName[3] && pp->szFileName[3]) {
    if(SortFlags & SORT_DIRSFIRST) {
      if((p->attrFile & FILE_DIRECTORY) != (pp->attrFile & FILE_DIRECTORY))
        return (p->attrFile & FILE_DIRECTORY) ? -1 : 1;
    }
    else if(SortFlags & SORT_DIRSLAST) {
      if((p->attrFile & FILE_DIRECTORY) != (pp->attrFile & FILE_DIRECTORY))
        return (p->attrFile & FILE_DIRECTORY) ? 1 : -1;
    }
    switch(SortFlags & (~(SORT_DIRSFIRST | SORT_DIRSLAST | SORT_REVERSE))) {
      case SORT_SUBJECT:
        if(*p->subject && *pp->subject)
          ret = stricmp(p->subject,pp->subject);
        else {
          ret = (*pp->subject) ? 1 : (*p->subject) ? -1 : 0;
          if(ret && (SortFlags & SORT_REVERSE))
            ret = (ret > 0) ? -1 : 1;
        }
        break;

      case SORT_FILENAME:
        pext = strrchr(p->szFileName,'\\');
        ppext = strrchr(pp->szFileName,'\\');
        if(!pext)
          pext = NullStr;
        if(!ppext)
          ppext = NullStr;
        ret = stricmp(pext,ppext);
        break;

      case SORT_FIRSTEXTENSION:
        pext = strrchr(p->szFileName,'\\');
        ppext = strrchr(pp->szFileName,'\\');
        if(!pext)
          pext = p->szFileName;
        if(!ppext)
          ppext = pp->szFileName;
        pext = strchr(pext,'.');
        ppext = strchr(ppext,'.');
        if(!pext)
          pext = NullStr;
        if(!ppext)
          ppext = NullStr;
        ret = stricmp(pext,ppext);
        break;

      case SORT_LASTEXTENSION:
        pext = strrchr(p->szFileName,'\\');
        ppext = strrchr(pp->szFileName,'\\');
        if(!pext)
          pext = p->szFileName;
        if(!ppext)
          ppext = pp->szFileName;
        pext = strrchr(pext,'.');
        ppext = strrchr(ppext,'.');
        if(!pext)
          pext = NullStr;
        if(!ppext)
          ppext = NullStr;
        ret = stricmp(pext,ppext);
        break;

      case SORT_SIZE:
        ret = (p->cbFile < pp->cbFile) ? 1 : (p->cbFile == pp->cbFile) ?
              0 : -1;
        if(!ret)
          ret = (p->easize < pp->easize) ? 1 : (p->easize == pp->easize) ?
                0 : -1;
        break;

      case SORT_EASIZE:
        ret = (p->easize < pp->easize) ? 1 : (p->easize == pp->easize) ?
              0 : -1;
        if(!ret)
          ret = (p->cbFile < pp->cbFile) ? 1 : (p->cbFile == pp->cbFile) ?
                0 : -1;
        break;

      case SORT_LWDATE:
        ret = (p->date.year < pp->date.year) ? 1 :
              (p->date.year > pp->date.year) ? -1 :
              (p->date.month < pp->date.month) ? 1 :
              (p->date.month > pp->date.month) ? -1 :
              (p->date.day < pp->date.day) ? 1 :
              (p->date.day > pp->date.day) ? -1 :
              (p->time.hours < pp->time.hours) ? 1 :
              (p->time.hours > pp->time.hours) ? -1 :
              (p->time.minutes < pp->time.minutes) ? 1 :
              (p->time.minutes > pp->time.minutes) ? -1 :
              (p->time.seconds < pp->time.seconds) ? 1 :
              (p->time.seconds > pp->time.seconds) ? -1 : 0;
        break;

      case SORT_LADATE:
        ret = (p->ladate.year < pp->ladate.year) ? 1 :
              (p->ladate.year > pp->ladate.year) ? -1 :
              (p->ladate.month < pp->ladate.month) ? 1 :
              (p->ladate.month > pp->ladate.month) ? -1 :
              (p->ladate.day < pp->ladate.day) ? 1 :
              (p->ladate.day > pp->ladate.day) ? -1 :
              (p->latime.hours < pp->latime.hours) ? 1 :
              (p->latime.hours > pp->latime.hours) ? -1 :
              (p->latime.minutes < pp->latime.minutes) ? 1 :
              (p->latime.minutes > pp->latime.minutes) ? -1 :
              (p->latime.seconds < pp->latime.seconds) ? 1 :
              (p->latime.seconds > pp->latime.seconds) ? -1 : 0;
        break;

      case SORT_CRDATE:
        ret = (p->crdate.year < pp->crdate.year) ? 1 :
              (p->crdate.year > pp->crdate.year) ? -1 :
              (p->crdate.month < pp->crdate.month) ? 1 :
              (p->crdate.month > pp->crdate.month) ? -1 :
              (p->crdate.day < pp->crdate.day) ? 1 :
              (p->crdate.day > pp->crdate.day) ? -1 :
              (p->crtime.hours < pp->crtime.hours) ? 1 :
              (p->crtime.hours > pp->crtime.hours) ? -1 :
              (p->crtime.minutes < pp->crtime.minutes) ? 1 :
              (p->crtime.minutes > pp->crtime.minutes) ? -1 :
              (p->crtime.seconds < pp->crtime.seconds) ? 1 :
              (p->crtime.seconds > pp->crtime.seconds) ? -1 : 0;
        break;
    }

    if(!ret)
      ret = (SHORT)stricmp(p->szFileName,pp->szFileName);

    if(ret && (SortFlags & SORT_REVERSE))
      ret = (ret > 0) ? -1 : 1;

    return ret;
  }
  return (SHORT)stricmp(p->szFileName,pp->szFileName);
}

