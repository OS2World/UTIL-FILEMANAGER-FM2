#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fm3dll.h"

#pragma alloc_text(MAKELIST,AddToList,AddToFileList,BuildList,FreeListInfo,FreeList)
#pragma alloc_text(MAKELIST,SortList,BuildArcList,RemoveFromList,CombineLists)


VOID SortList (LISTINFO *li) {

  /* bubble-sort entries by size, descending */

  INT   x;
  CHAR *s;
  ULONG l;
  BOOL  swapped;

  if(li && li->list && li->list[0] && li->cbFile) {
    do {
      swapped = FALSE;
      for(x = 0;li->list[x] && li->list[x + 1];x++) {
        if(li->cbFile[x] < li->cbFile[x + 1]) {
          s = li->list[x];
          li->list[x] = li->list[x + 1];
          li->list[x + 1] = s;
          l = li->cbFile[x];
          li->cbFile[x] = li->cbFile[x + 1];
          li->cbFile[x + 1] = l;
          if(li->ulitemID) {
            l = li->ulitemID[x];
            li->ulitemID[x] = li->ulitemID[x + 1];
            li->ulitemID[x + 1] = l;
          }
          swapped = TRUE;
        }
      }
    } while(swapped);
  }
}


VOID FreeListInfo (LISTINFO *li) {

  if(li) {
    if(li->ulitemID)
      free(li->ulitemID);
    if(li->cbFile)
      free(li->cbFile);
    if(li->list)
      FreeList(li->list);
    free(li);
  }
}


VOID FreeList (CHAR **list) {

  register INT x;

  if(list) {
    for(x = 0;list[x];x++) {
#ifdef __DEBUG_ALLOC__
  _heap_check();
#endif
      free(list[x]);
    }
#ifdef __DEBUG_ALLOC__
  _heap_check();
#endif
    free(list);
  }
  DosPostEventSem(CompactSem);
}


INT AddToFileList (CHAR *string,FILEFINDBUF4 *ffb4,FILELIST ***list,
                   INT *numfiles,INT *numalloced) {

  FILELIST **test;

  if(string && ffb4) {
    if(((*numfiles) + 3) > *numalloced) {
      if((test = realloc(*list,(((*numalloced) + 6) *
          sizeof(FILELIST *)))) == NULL)
        return 1;
      (*numalloced) += 6;
      *list = test;
    }
    if(((*list)[*numfiles] = malloc(sizeof(FILELIST) +
                                    strlen(string))) == NULL)
      return 2;
    (*list)[*numfiles]->attrFile = ffb4->attrFile;
    (*list)[*numfiles]->date = ffb4->fdateLastWrite;
    (*list)[*numfiles]->time = ffb4->ftimeLastWrite;
    (*list)[*numfiles]->ladate = ffb4->fdateLastAccess;
    (*list)[*numfiles]->latime = ffb4->ftimeLastAccess;
    (*list)[*numfiles]->crdate = ffb4->fdateCreation;
    (*list)[*numfiles]->crtime = ffb4->ftimeCreation;
    (*list)[*numfiles]->cbFile = ffb4->cbFile;
    (*list)[*numfiles]->easize = (ffb4->cbList > 4L) ? (ffb4->cbList / 2): 0L;
    strcpy((*list)[*numfiles]->fname,string);
    (*numfiles)++;
    (*list)[*numfiles] = NULL;
    (*list)[(*numfiles) + 1] = NULL;
#ifdef __DEBUG_ALLOC__
    _heap_check();
#endif
  }
  return 0;
}


INT AddToList (CHAR *string,CHAR ***list,INT *numfiles,INT *numalloced) {

  CHAR **test;

  if(string) {
    if(((*numfiles) + 3) > *numalloced) {
      if((test = realloc(*list,(((*numalloced) + 6) * sizeof(CHAR *)))) == NULL)
        return 1;
      (*numalloced) += 6;
      *list = test;
    }
    if(((*list)[*numfiles] = malloc(strlen(string) + 1)) == NULL)
      return 2;
    strcpy((*list)[*numfiles],string);
    (*numfiles)++;
    (*list)[*numfiles] = NULL;
    (*list)[(*numfiles) + 1] = NULL;
#ifdef __DEBUG_ALLOC__
    _heap_check();
#endif
  }
  return 0;
}


CHAR ** BuildList (HWND hwndCnr) {

  PCNRITEM pci;
  CHAR   **list = NULL,**test;
  INT      numfiles = 0,numalloc = 0,error = 0,attribute = CRA_CURSORED;

  pci = (PCNRITEM)CurrentRecord(hwndCnr);
  if(pci && (INT)pci != -1 && !(pci->flags & RECFLAGS_ENV)) {
    if(pci->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMLONG(CMA_FIRST),
                       MPFROMSHORT(attribute));
    }
  }
  while(pci && (INT)pci != -1 && !error) {
    if(!(pci->rc.flRecordAttr & CRA_FILTERED))
      error = AddToList(pci->szFileName,&list,&numfiles,&numalloc);
    pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMP(pci),
                     MPFROMSHORT(attribute));
  }
  if(numalloc > numfiles + 1) {
    test = realloc(list,sizeof(CHAR *) * (numfiles + 1));
    if(test)
      list = test;
  }
  return list;
}


CHAR ** BuildArcList (HWND hwndCnr) {

  PARCITEM pai;
  CHAR   **list = NULL;
  INT      numfiles = 0,numalloc = 0,error = 0,attribute = CRA_CURSORED;

  pai = (PARCITEM)CurrentRecord(hwndCnr);
  if(pai && (INT)pai != -1) {
    if(pai->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pai = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMLONG(CMA_FIRST),
                       MPFROMSHORT(attribute));
    }
  }
  while( pai && (INT)pai != -1 && !error) {
    if(!(pai->rc.flRecordAttr & CRA_FILTERED))
      error = AddToList(pai->szFileName,&list,&numfiles,&numalloc);
    pai = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMP(pai),
                     MPFROMSHORT(attribute));
  }
  return list;
}


CHAR ** RemoveFromList (CHAR **list,CHAR *item) {

  register INT x,y;

  if(list && list[0] && item) {
    for(x = 0;list[x];x++) {
      if(item == list[x]) {
        free(list[x]);
        list[x] = NULL;
        for(y = x;;y++) {
          if(y != x && !list[y])
            break;
          list[y] = list[y + 1];
        }
        if(!list[0]) {
          FreeList(list);
          list = NULL;
        }
        break;
      }
    }
  }
  return list;
}


CHAR ** CombineLists (CHAR **prime,CHAR **add) {

  register INT x;
  INT          numalloc,numfiles = 0;

  if(add && add[0]) {
    if(prime) {
      for(x = 0;prime[x];x++)
        numfiles++;
    }
    numalloc = numfiles;
    for(x = 0;add[x];x++) {
      if(*add[x])
        AddToList(add[x],&prime,&numfiles,&numalloc);
    }
    FreeList(add);
  }
  return prime;
}
