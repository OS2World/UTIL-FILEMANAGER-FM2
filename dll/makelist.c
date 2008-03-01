
/***********************************************************************

  $Id$

  Make file lists

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2008 Steven H.Levine

  12 Feb 03 SHL AddToFileList: standardize EA math
  22 Jul 06 SHL Use Runtime_Error
  22 Jul 06 SHL AddToList optimize
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Feb 08 GKY Use xfree where appropriate

***********************************************************************/

#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3str.h"
#include "makelist.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "dircnrs.h"
#include "fm3dll.h"			// 05 Jan 08 SHL fixme to be gone

static PSZ pszSrcFile = __FILE__;

VOID SortList(LISTINFO *li)
{
  /* bubble-sort entries by size, descending */

  UINT x;
  CHAR *s;
  ULONG l;
  BOOL swapped;

  if (li && li->list && li->list[0] && li->cbFile) {
    do {
      swapped = FALSE;
      for (x = 0; li->list[x] && li->list[x + 1]; x++) {
	if (li->cbFile[x] < li->cbFile[x + 1]) {
	  s = li->list[x];
	  li->list[x] = li->list[x + 1];
	  li->list[x + 1] = s;
	  l = li->cbFile[x];
	  li->cbFile[x] = li->cbFile[x + 1];
	  li->cbFile[x + 1] = l;
	  if (li->ulitemID) {
	    l = li->ulitemID[x];
	    li->ulitemID[x] = li->ulitemID[x + 1];
	    li->ulitemID[x + 1] = l;
	  }
	  swapped = TRUE;
	}
      }
    } while (swapped);
  }
}

VOID FreeListInfo(LISTINFO *li)
{
  if (li) {
    xfree(li->ulitemID);
    xfree(li->cbFile);
    if (li->list)
      FreeList(li->list);
    xfree(li);
  }
}

VOID FreeList(CHAR **list)
{
  UINT x;

  if (list) {
    for (x = 0; list[x]; x++) {
#ifdef __DEBUG_ALLOC__
      _heap_check();
#endif
      free(list[x]);
    }
#ifdef __DEBUG_ALLOC__
    _heap_check();
#endif
    xfree(list);
  }
  DosPostEventSem(CompactSem);
}

INT AddToFileList(CHAR *string, FILEFINDBUF4L *ffb4, FILELIST ***list,
		  UINT *pnumfiles, UINT *pnumalloced)
{
  FILELIST *pfl;

  if (string && ffb4) {
    // Ensure room for NULL entry
    if (((*pnumfiles) + 3) > *pnumalloced) {
      FILELIST **pflArray;

      // Use plain realloc for speed
      // 06 Aug 07 SHL fixme to know why + 6
      pflArray = realloc(*list, (*pnumalloced + 6) * sizeof(FILELIST *));
      if (!pflArray) {
	Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_OUTOFMEMORY));
	return 1;
      }
      (*pnumalloced) += 6;
      *list = pflArray;
    }
    // Use plain malloc for speed
    pfl = malloc(sizeof(FILELIST) + strlen(string));
    if (!pfl) {
      Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_OUTOFMEMORY));
      return 2;
    }
    pfl->attrFile = ffb4->attrFile;
    pfl->date = ffb4->fdateLastWrite;
    pfl->time = ffb4->ftimeLastWrite;
    pfl->ladate = ffb4->fdateLastAccess;
    pfl->latime = ffb4->ftimeLastAccess;
    pfl->crdate = ffb4->fdateCreation;
    pfl->crtime = ffb4->ftimeCreation;
    pfl->cbFile = ffb4->cbFile;
    pfl->easize = CBLIST_TO_EASIZE(ffb4->cbList);
    strcpy(pfl->fname, string);
    (*list)[*pnumfiles] = pfl;
    (*pnumfiles)++;
    // Ensure list always ends with two NULL entries
    // 06 Aug 07 SHL fixme to know why
    (*list)[*pnumfiles] = NULL;
    (*list)[(*pnumfiles) + 1] = NULL;
#ifdef __DEBUG_ALLOC__
    _heap_check();
#endif
  }
  return 0;
}

/**
 * Add string to string list
 * Enlarges as needed
 * Ensures 2 NULL end markers exist
 */

INT AddToList(CHAR *string, CHAR ***list, UINT *pnumfiles, UINT *pnumalloced)
{
  CHAR **ppsz;
  PSZ psz;

  if (string) {
    if (((*pnumfiles) + 3) > *pnumalloced) {
      // Use plain realloc for speed
      ppsz = realloc(*list, (*pnumalloced + 6) * sizeof(CHAR *));
      if (!ppsz) {
	Runtime_Error(pszSrcFile, __LINE__, "realloc");
	return 1;
      }
      (*pnumalloced) += 6;
      *list = ppsz;
    }
    // Use plain malloc for speed
    psz = malloc(strlen(string) + 1);
    if (!psz) {
      Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_OUTOFMEMORY));
      return 2;
    }
    (*list)[*pnumfiles] = psz;
    strcpy((*list)[*pnumfiles], string);	// Add entry
    (*pnumfiles)++;
    (*list)[*pnumfiles] = NULL;		// Add end marker
    (*list)[(*pnumfiles) + 1] = NULL;	// Add 2nd end marker - fixme to know why?
#ifdef __DEBUG_ALLOC__
    _heap_check();
#endif
  }
  return 0;
}

CHAR **BuildList(HWND hwndCnr)
{
  PCNRITEM pci;
  CHAR **list = NULL, **test;
  UINT numfiles = 0, numalloc = 0;
  INT error = 0, attribute = CRA_CURSORED;

  pci = (PCNRITEM) CurrentRecord(hwndCnr);
  if (pci && (INT) pci != -1 && !(pci->flags & RECFLAGS_ENV)) {
    if (pci->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		       MPFROMSHORT(attribute));
    }
  }
  while (pci && (INT) pci != -1 && !error) {
    if (!(pci->rc.flRecordAttr & CRA_FILTERED))
      error = AddToList(pci->pszFileName, &list, &numfiles, &numalloc);
    pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMP(pci),
		     MPFROMSHORT(attribute));
  }
  if (numalloc > numfiles + 1) {
    // Use plain realloc for speed
    test = realloc(list, sizeof(CHAR *) * (numfiles + 1));
    if (!test)
      Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_OUTOFMEMORY));
    else
      list = test;
  } // while
  return list;
}

CHAR **BuildArcList(HWND hwndCnr)
{
  PARCITEM pai;
  CHAR **list = NULL;
  UINT numfiles = 0, numalloc = 0;
  INT error = 0, attribute = CRA_CURSORED;

  pai = (PARCITEM) CurrentRecord(hwndCnr);
  if (pai && (INT) pai != -1) {
    if (pai->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pai = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		       MPFROMSHORT(attribute));
    }
  }
  while (pai && (INT) pai != -1 && !error) {
    if (!(pai->rc.flRecordAttr & CRA_FILTERED))
      error = AddToList(pai->pszFileName, &list, &numfiles, &numalloc);
    pai = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMP(pai),
		     MPFROMSHORT(attribute));
  }
  return list;
}

CHAR **RemoveFromList(CHAR **list, CHAR *item)
{
  UINT x, y;

  if (list && list[0] && item) {
    for (x = 0; list[x]; x++) {
      if (item == list[x]) {
	free(list[x]);
	list[x] = NULL;
	for (y = x;; y++) {
	  if (y != x && !list[y])
	    break;
	  list[y] = list[y + 1];
	}
	if (!list[0]) {
	  FreeList(list);
	  list = NULL;
	}
	break;
      }
    }
  }
  return list;
}

CHAR **CombineLists(CHAR **prime, CHAR **add)
{
  UINT x;
  UINT numalloc, numfiles = 0;

  if (add && add[0]) {
    if (prime) {
      for (x = 0; prime[x]; x++)
	numfiles++;
    }
    numalloc = numfiles;
    for (x = 0; add[x]; x++) {
      if (*add[x])
	AddToList(add[x], &prime, &numfiles, &numalloc);
    }
    FreeList(add);
  }
  return prime;
}

#pragma alloc_text(MAKELIST,AddToList,AddToFileList,BuildList,FreeListInfo,FreeList)
#pragma alloc_text(MAKELIST,SortList,BuildArcList,RemoveFromList,CombineLists)
