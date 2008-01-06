
/***********************************************************************

  $Id$

  Container item selection support routines

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2008 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  25 May 05 SHL Rework for ULONGLONG
  06 Jun 05 SHL Drop unused code
  06 Jul 06 SHL Support compare content (IDM_SELECTSAMECONTENT)
  13 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets_bstripcr
  15 Aug 06 SHL Rework SetMask args and logic
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits
  19 Apr 07 SHL Sync with NumItemsToUnhilite mods
  12 May 07 SHL Use dcd->ulItemsToUnHilite
  14 Jun 07 SHL SelectAll: make odd expression go away
  02 Aug 07 SHL Sync with CNRITEM mods
  04 Aug 07 SHL Use Runtime_Error
  05 Aug 07 SHL Rework SpecialSelect to use CNRITEM_EXISTS and
	       not use pszFileName since CNRITEM_EXISTS set implies
	       pszFileName not null
  14 Aug 07 SHL Revert ExpandAll DosSleep to 0
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  26 Aug 07 GKY DosSleep(1) in loops changed to (0)

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <io.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3str.h"
#include "filldir.h"			// RemoveCnrItems
#include "makelist.h"			// AddToList
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "fm3dll.h"

static PSZ pszSrcFile = __FILE__;

VOID UnHilite(HWND hwndCnr, BOOL all, CHAR *** list, ULONG ulItemsToUnHilite)
{
  PCNRITEM pci;
  UINT numfiles = 0, numalloc = 0;
  UINT x = 0;
  INT attribute = CRA_CURSORED;

  if (all && list && *list) {
    FreeList(*list);
    *list = NULL;
  }
  pci = (PCNRITEM) CurrentRecord(hwndCnr);
  if (pci && (INT)pci != -1) {
    if (pci->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		       MPFROMSHORT(attribute));
    }
    while (pci && (INT)pci != -1) {
      WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		 MPFROM2SHORT(FALSE, CRA_SELECTED));
      if (!all)
	  break;
      // Count is one extra to ensure non-zero elsewhere
      // x is 0 based index
      if (x + 2 == ulItemsToUnHilite)
	break;
      if (list)
	AddToList(pci->pszFileName, list, &numfiles, &numalloc);
      pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
				  MPFROMP(pci), MPFROMSHORT(CRA_SELECTED));
      x++;
    }
  }
}

VOID SelectList(HWND hwndCnr, BOOL partial, BOOL deselect, BOOL clearfirst,
		PCNRITEM pciParent, PSZ filename, CHAR ** list)
{

  PCNRITEM pci;
  register INT x;
  BOOL foundone = FALSE;
  ULONG errs = 0;

  if (clearfirst && !deselect)
    UnHilite(hwndCnr, TRUE, NULL, 0);
  if (list && list[0]) {
    for (x = 0; list[x]; x++) {
      pci = FindCnrRecord(hwndCnr,
			  list[x], pciParent, partial, partial, TRUE);
      if (pci) {
	WinSendMsg(hwndCnr,
		   CM_SETRECORDEMPHASIS,
		   MPFROMP(pci),
		   MPFROM2SHORT((SHORT) ((deselect) ? FALSE : TRUE),
				CRA_SELECTED));
	foundone = TRUE;
      }
    }
    if (!foundone)
      Runtime_Error(pszSrcFile, __LINE__, "select failed");
  }
  else if (filename && *filename) {

    FILE *fp;
    CHAR input[1024], *p;

    fp = _fsopen(filename, "r", SH_DENYNO);
    if (fp) {
      while (!feof(fp)) {
	if (!xfgets_bstripcr(input, sizeof(input), fp, pszSrcFile, __LINE__))
	  break;
	if (*input == '\"') {
	  memmove(input, input + 1, strlen(input) + 1);
	  lstrip(input);
	  p = strchr(input, '\"');
	  if (p)
	    *p = 0;
	  rstrip(input);
	}
	else {
	  p = strchr(input, ' ');
	  if (p)
	    *p = 0;
	}
	/* input now contains name of file to select */
	pci = FindCnrRecord(hwndCnr,
			    input, pciParent, partial, partial, TRUE);
	if (pci)			/* found it? */
	  WinSendMsg(hwndCnr,
		     CM_SETRECORDEMPHASIS,
		     MPFROMP(pci),
		     MPFROM2SHORT((SHORT) ((deselect) ? FALSE : TRUE),
				  CRA_SELECTED));
	else
	  errs++;
	if (errs > 50) {		/* prevent runaway on bad file */

	  APIRET ret;

	  ret = saymsg(MB_YESNO,
		       hwndCnr,
		       GetPString(IDS_POSSIBLEERRORTEXT),
		       GetPString(IDS_MAYNOTBELISTTEXT), filename);
	  if (ret == MBID_NO)
	    break;
	  errs = 0;
	}
      }
      fclose(fp);
    }
  }
}

VOID SelectAll(HWND hwndCnr, BOOL files, BOOL dirs, PSZ maskstr,
	       PSZ text, BOOL is_arc)
{

  PCNRITEM pci;
  BOOL markit;
  PSZ file;
  PSZ pszToMatch;
  MASK Mask;
  INT x;
  ULONG textlen = 0;

  if (text)
    textlen = strlen(text);
  memset(&Mask, 0, sizeof(Mask));
  if (maskstr)
    SetMask(maskstr, &Mask);
  pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPVOID,
			      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  while (pci && (INT)pci != -1) {

    markit = FALSE;

    if (~pci->rc.flRecordAttr & CRA_FILTERED) {
      if (!is_arc) {
	if (files && ~pci->attrFile & FILE_DIRECTORY)
	  markit = TRUE;
	if (dirs && pci->attrFile & FILE_DIRECTORY)
	  markit = TRUE;
      }
      else
	markit = TRUE;
      if (maskstr && *maskstr && markit) {
	markit = FALSE;
	// Point a filename part
	file = strrchr(pci->pszFileName, '\\');
	if (!file)
	  file = strrchr(pci->pszFileName, ':');
	if (file)
	  file++;
	else
	  file = pci->pszFileName;
	for (x = 0; Mask.pszMasks[x]; x++) {
	  if (*Mask.pszMasks[x]) {
	    if ((strchr(Mask.pszMasks[x], '\\') ||
		strchr(Mask.pszMasks[x], ':')))
	      pszToMatch = pci->pszFileName;
	    else
	      pszToMatch = file;
	    if (*Mask.pszMasks[x] != '/') {
	      if (wildcard(pszToMatch, Mask.pszMasks[x], FALSE)) {
		markit = TRUE;
	      }
	    }
	    else {
	      if (wildcard(pszToMatch, Mask.pszMasks[x] + 1, FALSE)) {
		markit = FALSE;
		break;
	      }
	    }
	  }
	} // for
      }
    }

    if (markit && text && *text) {
      if (~pci->attrFile & FILE_DIRECTORY) {
	PSZ input;
	markit = FALSE;
	input = xmalloc(65537, pszSrcFile, __LINE__);
	if (input) {
	  ULONG pos;
	  LONG len;
	  FILE *inputFile;

	  if ((inputFile = _fsopen(pci->pszFileName, "rb", SH_DENYNO)) != NULL) {
	    pos = ftell(inputFile);
	    while (!feof(inputFile)) {
	      if (pos)
		fseek(inputFile, pos - 256, SEEK_SET);
	      len = fread(input, 1, 65536, inputFile);
	      if (len >= 0) {
		if (findstring(text, textlen, input, len, FALSE)) {
		  markit = TRUE;
		  break;
		}
	      }
	      else
		break;
	    } // while
	    fclose(inputFile);
	  }
	  free(input);
	  DosSleep(1);
	}
      }
      else
	markit = FALSE;
    }

    if (markit)
      WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		 MPFROM2SHORT(TRUE, CRA_SELECTED));
    pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pci),
				MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
  }					// while
}

VOID DeselectAll(HWND hwndCnr, BOOL files, BOOL dirs, PSZ maskstr,
		 PSZ text, BOOL is_arc)
{
  PCNRITEM pci;
  BOOL unmarkit;
  PSZ file;
  PSZ pszToMatch;
  MASK Mask;
  register INT x;
  ULONG textlen = 0;

  if (text)
    textlen = strlen(text);
  memset(&Mask, 0, sizeof(Mask));
  if (maskstr && *maskstr)
    SetMask(maskstr, &Mask);
  pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPVOID,
			      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  while (pci && (INT)pci != -1) {
    unmarkit = FALSE;
    if (~pci->rc.flRecordAttr & CRA_FILTERED) {
      if (!is_arc) {
	if (files && ~pci->attrFile & FILE_DIRECTORY)
	  unmarkit = TRUE;
	if (dirs && (pci->attrFile & FILE_DIRECTORY))
	  unmarkit = TRUE;
      }
      else
	unmarkit = TRUE;
      if (maskstr && *maskstr && unmarkit) {
	unmarkit = FALSE;
	file = strrchr(pci->pszFileName, '\\');
	if (!file)
	  file = strrchr(pci->pszFileName, ':');
	if (file)
	  file++;
	else
	  file = pci->pszFileName;
	for (x = 0; Mask.pszMasks[x]; x++) {
	  if (*Mask.pszMasks[x]) {
	    if (strchr(Mask.pszMasks[x], '\\') ||
		strchr(Mask.pszMasks[x], ':'))
	      pszToMatch = pci->pszFileName;
	    else
	      pszToMatch = file;
	    if (*Mask.pszMasks[x] != '/') {
	      if (wildcard(pszToMatch, Mask.pszMasks[x], FALSE))
		unmarkit = TRUE;
	    }
	    else {
	      if (wildcard(pszToMatch, Mask.pszMasks[x] + 1, FALSE)) {
		unmarkit = FALSE;
		break;
	      }
	    }
	  }
	}
      }
    }

    if (unmarkit && text && *text) {
      if (~pci->attrFile & FILE_DIRECTORY) {
	PSZ input;
	unmarkit = FALSE;
	input = xmalloc(65537, pszSrcFile, __LINE__);
	if (input) {
	  ULONG pos;
	  LONG len;
	  FILE *inputFile;

	  if ((inputFile = _fsopen(pci->pszFileName, "rb", SH_DENYNO)) != NULL) {
	    pos = ftell(inputFile);
	    while (!feof(inputFile)) {
	      if (pos)
		fseek(inputFile, pos - 256, SEEK_SET);
	      len = fread(input, 1, 65536, inputFile);
	      if (len >= 0) {
		if (findstring(text, textlen, input, len, FALSE)) {
		  unmarkit = TRUE;
		  break;
		}
	      }
	      else
		break;
	    } // while
	    fclose(inputFile);
	  }
	  free(input);
	  DosSleep(1);
	}
      }
      else
	unmarkit = FALSE;
    }

    if (unmarkit)
      WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, pci,
		 MPFROM2SHORT(FALSE, CRA_SELECTED | CRA_CURSORED |
			      CRA_INUSE | CRA_SOURCE));
    pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pci),
				MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
  }
}

VOID Deselect(HWND hwndCnr)
{
  PCNRITEM pcil;

  pcil = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
			       MPFROMLONG(CMA_FIRST),
			       MPFROMSHORT(CRA_SELECTED));
  while (pcil && (INT)pcil != -1) {
    WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pcil),
	       MPFROM2SHORT(FALSE, CRA_SELECTED));
    pcil = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMP(pcil),
		      MPFROMSHORT(CRA_SELECTED));
  }
}

//=== HideAll() Hide all selected records ===

VOID HideAll(HWND hwndCnr)
{
  PCNRITEM pci, pciH;
  INT attribute = CRA_CURSORED;
  CNRINFO cnri;
  BOOL didone = FALSE;

  memset(&cnri, 0, sizeof(CNRINFO));
  cnri.cb = sizeof(CNRINFO);
  WinSendMsg(hwndCnr, CM_QUERYCNRINFO, MPFROMP(&cnri),
	     MPFROMLONG(sizeof(CNRINFO)));
  pci = (PCNRITEM) CurrentRecord(hwndCnr);
  if (pci && (INT)pci != -1) {
    if (pci->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		       MPFROMSHORT(attribute));
    }
  }
  while (pci && (INT)pci != -1) {
    pciH = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMP(pci),
		      MPFROMSHORT(attribute));
    WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
	       MPFROM2SHORT(FALSE, CRA_CURSORED | CRA_SELECTED |
			    CRA_INUSE | CRA_SOURCE));
    pci->rc.flRecordAttr |= CRA_FILTERED;
    didone = TRUE;
    if (fSyncUpdates) {
      if (cnri.flWindowAttr & CV_DETAIL)
	WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPVOID,
		   MPFROM2SHORT(0, CMA_REPOSITION | CMA_ERASE));
      else
	WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPFROMP(&pci),
		   MPFROM2SHORT(1, CMA_REPOSITION | CMA_ERASE));
    }
    pci = pciH;
  }
  if (didone && !fSyncUpdates)
    WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPVOID,
	       MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
}

VOID MarkAll(HWND hwndCnr, BOOL quitit, BOOL target, BOOL source)
{
  PCNRITEM pci;
  INT attribute = CRA_CURSORED;

  if (quitit)
    attribute = target ? CRA_TARGET : source ? CRA_SOURCE : CRA_INUSE;
  pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
			      MPFROMLONG(CMA_FIRST), MPFROMSHORT(attribute));
  if (pci && (INT)pci != -1) {
    if (attribute == CRA_CURSORED) {
      if (pci->rc.flRecordAttr & CRA_SELECTED) {
	attribute = CRA_SELECTED;
	pci =
	  WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		     MPFROMSHORT(attribute));
      }
    }
  }
  while (pci && (INT)pci != -1) {
    WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
	       MPFROM2SHORT(!quitit,
			    target ? CRA_TARGET : source ? CRA_SOURCE :
			     CRA_INUSE));
    pci =
      WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMP(pci),
		 MPFROMSHORT(attribute));
  }
}

VOID RemoveAll(HWND hwndCnr, ULONGLONG * pullTotalBytes,
	       ULONG * pulTotalFiles)
{
  PCNRITEM pci;
  INT attribute = CRA_CURSORED;
  BOOL didone = FALSE;

  pci = (PCNRITEM) CurrentRecord(hwndCnr);
  if (pci && (INT)pci != -1) {
    if (pci->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		       MPFROMSHORT(attribute));
    }
  }
  while (pci && (INT)pci != -1) {
    if (~pci->rc.flRecordAttr & CRA_FILTERED) {
      didone = TRUE;
      if (pulTotalFiles)
	*pulTotalFiles -= 1;
      if (pullTotalBytes)
	*pullTotalBytes -= (pci->cbFile + pci->easize);
      WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		 MPFROM2SHORT(0, CRA_SELECTED));
      if (fSyncUpdates)
	RemoveCnrItems(hwndCnr, pci, 1, CMA_FREE | CMA_INVALIDATE);
      else
	RemoveCnrItems(hwndCnr, pci, 1, CMA_FREE);
      if (attribute == CRA_CURSORED)
	break;
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		       MPFROMSHORT(attribute));
    }
    else
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMP(pci),
		       MPFROMSHORT(attribute));
  }
  if (didone && !fSyncUpdates)
    WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPVOID,
	       MPFROM2SHORT(0, CMA_REPOSITION));
}

//== SetMask() Convert mask string to array of pointers to masks ==

VOID SetMask(PSZ maskstr, MASK * mask)
{
  UINT x;
  PSZ p;

  if (maskstr)
    strcpy(mask->szMask, maskstr);	// Got new mask string
  // Build array of pointers
  p = mask->szMaskCopy;
  strcpy(p, mask->szMask);
  // Allow up to 25 masks - ignore extras
  for (x = 0; *p && x < 25; x++) {
    mask->pszMasks[x] = p;
    while (*p && *p != ';')
      p++;				// Find separator
    if (*p) {
      *p = 0;				// Replace ;
      p++;
    }
  }					// for
  mask->pszMasks[x] = NULL;		// Mark end
}

VOID ExpandAll(HWND hwndCnr, BOOL expand, PCNRITEM pciParent)
{
  PCNRITEM pci;

  if (!pciParent)
    pciParent = WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(NULL),
			   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  if (pciParent) {
    if (expand && ~pciParent->rc.flRecordAttr & CRA_EXPANDED)
      WinSendMsg(hwndCnr, CM_EXPANDTREE, MPFROMP(pciParent), MPVOID);
    else if (!expand && (pciParent->rc.flRecordAttr & CRA_EXPANDED))
      WinSendMsg(hwndCnr, CM_COLLAPSETREE, MPFROMP(pciParent), MPVOID);
    pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pciParent),
				MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    if (pci)
      DosSleep(0);
    while (pci && (INT)pci != -1) {
      ExpandAll(hwndCnr, expand, pci);
      pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pci),
				  MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    }
  }
  DosSleep(0);
}

VOID InvertAll(HWND hwndCnr)
{
  PCNRITEM pci;

  pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPVOID,
			      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  while (pci && (INT)pci != -1) {
    if (~pci->rc.flRecordAttr & CRA_FILTERED) {
      if (~pci->rc.flRecordAttr & CRA_SELECTED)
	WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		   MPFROM2SHORT(TRUE, CRA_SELECTED));
      else
	WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
		   MPFROM2SHORT(FALSE, CRA_SELECTED));
    }
    pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pci),
				MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
  }
}

/**
 * Do select actions for compare directories containers
 *
 */

VOID SpecialSelect(HWND hwndCnrS, HWND hwndCnrD, INT action, BOOL reset)
{
  PCNRITEM pciS, pciD, *pciSa = NULL, *pciDa = NULL;
  CNRINFO cnri;
  BOOL slow = FALSE;
  UINT x, numD, numS;
  INT ret = 0;

  if (!hwndCnrS || !hwndCnrD) {
    Runtime_Error(pszSrcFile, __LINE__, "hwndCnrS %p hwndCnrD %p", hwndCnrS, hwndCnrD);
    return;
  }

  memset(&cnri, 0, sizeof(CNRINFO));
  cnri.cb = sizeof(CNRINFO);
  WinSendMsg(hwndCnrD, CM_QUERYCNRINFO, MPFROMP(&cnri),
	     MPFROMLONG(sizeof(CNRINFO)));
  numD = cnri.cRecords;
  memset(&cnri, 0, sizeof(CNRINFO));
  cnri.cb = sizeof(CNRINFO);
  WinSendMsg(hwndCnrS, CM_QUERYCNRINFO, MPFROMP(&cnri),
	     MPFROMLONG(sizeof(CNRINFO)));
  numS = cnri.cRecords;
  if (!numD || numS != numD) {
    Runtime_Error(pszSrcFile, __LINE__, "numD %u != numS %u", numD, numS);
    return;
  }
  pciDa = xmalloc(sizeof(PCNRITEM) * numD, pszSrcFile, __LINE__);
  if (!pciDa)
    return;

  pciSa = xmalloc(sizeof(PCNRITEM) * numS, pszSrcFile, __LINE__);
  if (!pciSa) {
    free(pciDa);
    return;
  }

Restart:

  memset(pciDa, 0, sizeof(PCNRITEM) * numD);
  memset(pciSa, 0, sizeof(PCNRITEM) * numS);

  pciD = (PCNRITEM)WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPVOID,
			       MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  x = 0;
  while (pciD && (INT)pciD != -1 && x < numD) {
    if (reset)
      pciD->flags = 0;
    pciDa[x] = pciD;
    x++;
    if (!slow)
      pciD = (PCNRITEM) pciD->rc.preccNextRecord;
    else
      pciD = (PCNRITEM) WinSendMsg(hwndCnrD, CM_QUERYRECORD, MPFROMP(pciD),
				   MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    if (!(x % 500))
      DosSleep(0);  //26 Aug 07 GKY 1
    // else if (!(x % 50))
    //  DosSleep(0);
  } // while

  if (numD != x) {
    if (!slow) {
      slow = TRUE;
      goto Restart;
    }
    free(pciDa);
    free(pciSa);
    Runtime_Error(pszSrcFile, __LINE__, "numD %u != x %lu", numD, x);
    return;
  }

  pciS = (PCNRITEM) WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPVOID,
			       MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  x = 0;
  while (pciS && (INT)pciS != -1 && x < numS) {
    if (reset)
      pciS->flags = 0;
    pciSa[x] = pciS;
    x++;
    if (!slow)
      pciS = (PCNRITEM) pciS->rc.preccNextRecord;
    else
      pciS = (PCNRITEM) WinSendMsg(hwndCnrS, CM_QUERYRECORD, MPFROMP(pciS),
				   MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    if (!(x % 500))
      DosSleep(0);  //26 Aug 07 GKY 1
    // else if (!(x % 50))
    //  DosSleep(0);
  } // while

  if (numS != x) {
    if (!slow) {
      slow = TRUE;
      goto Restart;
    }
    free(pciSa);
    free(pciDa);
    Runtime_Error(pszSrcFile, __LINE__, "numS (%lu) != x (%lu)", numS, x);
    return;
  }

  // 05 Aug 07 SHL fixme to know what sets reset
  if (reset) {
    // Update flags for files that exist on both sides
    for (x = 0; x < numS; x++) {

      // 05 Aug 07 SHL fixme to know if should clear first
      if (!*pciSa[x]->pszFileName || !*pciDa[x]->pszFileName)
	continue;

      pciSa[x]->flags |= CNRITEM_EXISTS;	// File exists on both sides
      pciDa[x]->flags |= CNRITEM_EXISTS;
      if (pciSa[x]->cbFile + pciSa[x]->easize >
	  pciDa[x]->cbFile + pciDa[x]->easize) {
	pciSa[x]->flags |= CNRITEM_LARGER;
	pciDa[x]->flags |= CNRITEM_SMALLER;
      }
      else if (pciSa[x]->cbFile + pciSa[x]->easize <
	       pciDa[x]->cbFile + pciDa[x]->easize) {
	pciSa[x]->flags |= CNRITEM_SMALLER;
	pciDa[x]->flags |= CNRITEM_LARGER;
      }
      ret = TestCDates(&pciDa[x]->date, &pciDa[x]->time,
                       &pciSa[x]->date, &pciSa[x]->time);
      if (ret == 1)
        /*((pciSa[x]->date.year > pciDa[x]->date.year) ? TRUE :
	  (pciSa[x]->date.year < pciDa[x]->date.year) ? FALSE :
	  (pciSa[x]->date.month > pciDa[x]->date.month) ? TRUE :
	  (pciSa[x]->date.month < pciDa[x]->date.month) ? FALSE :
	  (pciSa[x]->date.day > pciDa[x]->date.day) ? TRUE :
	  (pciSa[x]->date.day < pciDa[x]->date.day) ? FALSE :
	  (pciSa[x]->time.hours > pciDa[x]->time.hours) ? TRUE :
	  (pciSa[x]->time.hours < pciDa[x]->time.hours) ? FALSE :
	  (pciSa[x]->time.minutes > pciDa[x]->time.minutes) ? TRUE :
	  (pciSa[x]->time.minutes < pciDa[x]->time.minutes) ? FALSE :
	  (pciSa[x]->time.seconds > pciDa[x]->time.seconds) ? TRUE :
	  (pciSa[x]->time.seconds < pciDa[x]->time.seconds) ? FALSE : FALSE)*/ {
	pciSa[x]->flags |= CNRITEM_NEWER;
	pciDa[x]->flags |= CNRITEM_OLDER;
      }
      else if (ret == -1)
        /*((pciSa[x]->date.year < pciDa[x]->date.year) ? TRUE :
	       (pciSa[x]->date.year > pciDa[x]->date.year) ? FALSE :
	       (pciSa[x]->date.month < pciDa[x]->date.month) ? TRUE :
	       (pciSa[x]->date.month > pciDa[x]->date.month) ? FALSE :
	       (pciSa[x]->date.day < pciDa[x]->date.day) ? TRUE :
	       (pciSa[x]->date.day > pciDa[x]->date.day) ? FALSE :
	       (pciSa[x]->time.hours < pciDa[x]->time.hours) ? TRUE :
	       (pciSa[x]->time.hours > pciDa[x]->time.hours) ? FALSE :
	       (pciSa[x]->time.minutes < pciDa[x]->time.minutes) ? TRUE :
	       (pciSa[x]->time.minutes > pciDa[x]->time.minutes) ? FALSE :
	       (pciSa[x]->time.seconds < pciDa[x]->time.seconds) ? TRUE :
	       (pciSa[x]->time.seconds > pciDa[x]->time.seconds) ? FALSE :
	       FALSE)*/ {
	pciSa[x]->flags |= CNRITEM_OLDER;
	pciDa[x]->flags |= CNRITEM_NEWER;
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    } // for
  } // if reset

  switch (action) {
  case IDM_SELECTIDENTICAL:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED &&
	  pciSa[x]->flags & CNRITEM_EXISTS &&
	  ~pciSa[x]->flags & CNRITEM_SMALLER &&
	  ~pciSa[x]->flags & CNRITEM_LARGER &&
	  ~pciSa[x]->flags & CNRITEM_NEWER &&
	  ~pciSa[x]->flags & CNRITEM_OLDER) {
	if (~pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	  WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
	if (~pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	  WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    } // for
    break;

  case IDM_SELECTSAME:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED &&
	  pciSa[x]->flags & CNRITEM_EXISTS &&
	  ~pciSa[x]->flags & CNRITEM_SMALLER &&
	  ~pciSa[x]->flags & CNRITEM_LARGER) {
	if (~pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	  WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
	if (~pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	  WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_SELECTSAMECONTENT:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED &&
	  pciSa[x]->flags & CNRITEM_EXISTS)
      {
	FILE *fp1 = NULL;
	FILE *fp2 = NULL;
	BOOL gotMatch = FALSE;
	UINT errLineNo = 0;
	UINT compErrno = 0;
	CHAR buf1[1024];
	CHAR buf2[1024];
	HAB hab = WinQueryAnchorBlock(hwndCnrS);

	if (!*pciSa[x]->pszFileName ||
	    !*pciDa[x]->pszFileName) {
	  Runtime_Error(pszSrcFile, __LINE__,
			"CNRITEM_EXISTS set with null file name for index %u", x);
	  break;
	}

	fp1 = _fsopen(pciSa[x]->pszFileName, "rb", SH_DENYNO);
	if (!fp1) {
	  errLineNo = __LINE__;
	  compErrno = errno;
	}
	else {
	  fp2 = _fsopen(pciDa[x]->pszFileName, "rb", SH_DENYNO);
	  if (!fp2) {
	    errLineNo = __LINE__;
	    compErrno = errno;
	  }
	  else {
	    size_t len1 = filelength(fileno(fp1));
	    size_t len2 = filelength(fileno(fp2));

	    if (len1 == len2) {
	      setbuf(fp1, NULL);
	      setbuf(fp2, NULL);
	      while (WinIsWindow(hab, hwndCnrS)) {
		size_t numread1 = fread(buf1, 1, 1024, fp1);
		size_t numread2 = fread(buf2, 1, 1024, fp2);

		if (!numread1 || !numread2 || numread1 != numread2) {
		  if (ferror(fp1) || ferror(fp2)) {
		    errLineNo = __LINE__;
		    compErrno = errno;
		  }
		  else if (feof(fp1) && feof(fp2))
		    gotMatch = TRUE;
		  break;
		}
		else if (memcmp(buf1, buf2, numread1))
		  break;
	      }	// while
	    } // same len
	  }
	}

	if (fp1)
	  fclose(fp1);

	if (fp2)
	  fclose(fp2);

	if (errLineNo) {
	  Runtime_Error(pszSrcFile, errLineNo,
			"error %d while comparing", compErrno);
	}

	if (gotMatch) {
	  if (~pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	  if (~pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    } // for
    break;

  case IDM_SELECTBOTH:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED &&
	  pciSa[x]->flags & CNRITEM_EXISTS) {
	if (~pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	  WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
	if (~pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	  WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_SELECTONE:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED &&
	  ~pciSa[x]->flags & CNRITEM_EXISTS) {
	if (*pciSa[x]->pszFileName) {
	  if (~pciSa[x]->rc.flRecordAttr & CRA_SELECTED) {
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	  }
	}
	else if (~pciDa[x]->rc.flRecordAttr & CRA_SELECTED) {
	  WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0);  //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_SELECTBIGGER:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED) {
	if (pciSa[x]->flags & CNRITEM_LARGER) {
	  if (~pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
	else if (pciDa[x]->flags & CNRITEM_LARGER) {
	  if (~pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_SELECTSMALLER:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED) {
	if (pciSa[x]->flags & CNRITEM_SMALLER) {
	  if (~pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
	else if (pciDa[x]->flags & CNRITEM_SMALLER) {
	  if (~pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_SELECTNEWER:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED) {
	if (pciSa[x]->flags & CNRITEM_NEWER) {
	  if (~pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
	else if (pciDa[x]->flags & CNRITEM_NEWER) {
	  if (~pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_SELECTOLDER:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED) {
	if (pciSa[x]->flags & CNRITEM_OLDER) {
	  if (~pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
	else if (pciDa[x]->flags & CNRITEM_OLDER) {
	  if (~pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(TRUE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_DESELECTBOTH:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED &&
	  pciSa[x]->flags & CNRITEM_EXISTS) {
	if (pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	  WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		     MPFROM2SHORT(FALSE, CRA_SELECTED));
	if (pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	  WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		     MPFROM2SHORT(FALSE, CRA_SELECTED));
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_DESELECTONE:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED) {
	if (~pciSa[x]->flags & CNRITEM_EXISTS) {
	if (*pciSa[x]->pszFileName) {
	  if (pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
	else if (pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_DESELECTBIGGER:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED) {
	if (pciSa[x]->flags & CNRITEM_LARGER) {
	  if (pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
	else if (pciDa[x]->flags & CNRITEM_LARGER) {
	  if (pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_DESELECTSMALLER:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED) {
	if (pciSa[x]->flags & CNRITEM_SMALLER) {
	  if (pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
	else if (pciDa[x]->flags & CNRITEM_SMALLER) {
	  if (pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_DESELECTNEWER:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED) {
	if (pciSa[x]->flags & CNRITEM_NEWER) {
	  if (pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
	else if (pciDa[x]->flags & CNRITEM_NEWER) {
	  if (pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  case IDM_DESELECTOLDER:
    for (x = 0; x < numS; x++) {
      if (~pciSa[x]->rc.flRecordAttr & CRA_FILTERED) {
	if (pciSa[x]->flags & CNRITEM_OLDER) {
	  if (pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrS, CM_SETRECORDEMPHASIS, MPFROMP(pciSa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
	else if (pciDa[x]->flags & CNRITEM_OLDER) {
	  if (pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
	    WinSendMsg(hwndCnrD, CM_SETRECORDEMPHASIS, MPFROMP(pciDa[x]),
		       MPFROM2SHORT(FALSE, CRA_SELECTED));
	}
      }
      if (!(x % 500))
	DosSleep(0); //26 Aug 07 GKY 1
      // else if (!(x % 50))
      //	DosSleep(0);
    }
    break;

  default:
    break;
  }

  if (reset) {
    while (numS) {
      WinSendMsg(hwndCnrS, CM_INVALIDATERECORD,
		 MPFROMP(pciSa), MPFROM2SHORT((min(numS, 65535)), 0));
      DosSleep(0); //26 Aug 07 GKY 1
      WinSendMsg(hwndCnrD, CM_INVALIDATERECORD,
		 MPFROMP(pciDa), MPFROM2SHORT((min(numD, 65535)), 0));
      numS -= min(numS, 65535);
      if (numS)
	DosSleep(0); //26 Aug 07 GKY 1
    }
  }

  free(pciSa);
  free(pciDa);
  DosPostEventSem(CompactSem);
}

struct SS
{
  PCNRITEM pci;
  BOOL unique, all, smallest, largest, newest, oldest;
};

struct Cnr
{
  HWND hwndCnr;
  ULONG numfiles;
  struct SS *ss;
};

static int CompSSNamesB(const void *s1, const void *s2)
{
  struct SS *ss2 = (struct SS *)s2;

  return stricmp((PSZ)s1, ss2->pci->pszFileName);
}

static int CompSSNames(const void *s1, const void *s2)
{
  struct SS *ss1 = (struct SS *)s1;
  struct SS *ss2 = (struct SS *)s2;

  return stricmp(ss1->pci->pszFileName, ss2->pci->pszFileName);
}

VOID FreeCnrs(struct Cnr * Cnrs, INT numw)
{
  register INT z;

  for (z = 0; z < numw; z++) {
    if (Cnrs[z].ss)
      free(Cnrs[z].ss);
  }
  free(Cnrs);
  DosPostEventSem(CompactSem);
}

/**
 * Do select actions for single container
 *
 */

VOID SpecialSelect2(HWND hwndParent, INT action)
{
  PCNRITEM pci;
  HENUM henum;
  HWND hwnd;
  INT numwindows = 0, w, x, z, cmp = 0;
  struct Cnr *Cnrs = NULL;
  struct SS *bsres;

  if (!hwndParent)
    return;

  /* count directory containers, build array of hwnds */
  henum = WinBeginEnumWindows(hwndParent);
  while ((hwnd = WinGetNextWindow(henum)) != NULLHANDLE) {
    if (WinWindowFromID(WinWindowFromID(hwnd, FID_CLIENT), DIR_CNR)) {
      Cnrs =
	xrealloc(Cnrs, (numwindows + 1) * sizeof(struct Cnr), pszSrcFile,
		 __LINE__);
      if (!Cnrs) {
	Notify(GetPString(IDS_OUTOFMEMORY));
	return;
      }
      memset(&Cnrs[numwindows], 0, sizeof(struct Cnr));
      Cnrs[numwindows].hwndCnr = WinWindowFromID(WinWindowFromID(hwnd,
								 FID_CLIENT),
						 DIR_CNR);
      numwindows++;
    }
  }
  WinEndEnumWindows(henum);
  if (numwindows < 2) {
    FreeCnrs(Cnrs, numwindows);
    Runtime_Error(pszSrcFile, __LINE__, "expected two windows");
    Notify(GetPString(IDS_COMPSEL2ORMORETEXT));
    return;
  }
  if (numwindows > 4) {
    WinSendMsg(Cnrs[0].
	       hwndCnr,
	       UM_NOTIFY, MPFROMP(GetPString(IDS_BUILDINGLISTSTEXT)), MPVOID);
    DosSleep(0); //26 Aug 07 GKY 1
  }

  /* count records, build array of pointers to records */
  for (z = 0; z < numwindows; z++) {
    pci = (PCNRITEM) WinSendMsg(Cnrs[z].hwndCnr,
				CM_QUERYRECORD,
				MPVOID,
				MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
    x = 0;
    while (pci && (INT)pci != -1) {
      if (~pci->rc.flRecordAttr & CRA_FILTERED &&
	  ~pci->attrFile & FILE_DIRECTORY) {
	Cnrs[z].ss =
	  xrealloc(Cnrs[z].ss, (x + 1) * sizeof(struct SS), pszSrcFile,
		   __LINE__);
	if (!Cnrs[z].ss) {
	  FreeCnrs(Cnrs, numwindows);
	  Notify(GetPString(IDS_OUTOFMEMORY));
	  return;
	}
	memset(&Cnrs[z].ss[x], 0, sizeof(struct SS));
	Cnrs[z].ss[x].pci = pci;
	x++;
      }
      pci = (PCNRITEM) WinSendMsg(Cnrs[z].hwndCnr,
				  CM_QUERYRECORD,
				  MPFROMP(pci),
				  MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    }
    DosSleep(0);  //26 Aug 07 GKY 1
    Cnrs[z].numfiles = x;
    if (Cnrs[z].numfiles)
      qsort(Cnrs[z].ss, Cnrs[z].numfiles, sizeof(struct SS), CompSSNames);
  }

  for (z = 0; z < numwindows; z++) {
    for (x = 0; x < Cnrs[z].numfiles; x++) {
      Cnrs[z].ss[x].all = Cnrs[z].ss[x].unique = Cnrs[z].ss[x].newest =
	Cnrs[z].ss[x].oldest = Cnrs[z].ss[x].smallest =
	Cnrs[z].ss[x].largest = TRUE;
      for (w = 0; w < numwindows; w++) {
	if (w != z && Cnrs[w].numfiles) {
	  bsres = (struct SS *)bsearch(Cnrs[z].ss[x].pci->pszFileName,
				       Cnrs[w].ss, Cnrs[w].numfiles,
				       sizeof(struct SS), CompSSNamesB);
	  if (bsres) {
	    Cnrs[z].ss[x].unique = FALSE;
	    if (Cnrs[z].ss[x].pci->cbFile + Cnrs[z].ss[x].pci->easize >
		bsres->pci->cbFile + bsres->pci->easize)
	      Cnrs[z].ss[x].smallest = FALSE;
	    if (Cnrs[z].ss[x].pci->cbFile + Cnrs[z].ss[x].pci->easize <
		bsres->pci->cbFile + bsres->pci->easize)
	      Cnrs[z].ss[x].largest = FALSE;
	    cmp = TestCDates(&bsres->pci->date, &bsres->pci->time,
                             &Cnrs[z].ss[x].pci->date, &Cnrs[z].ss[x].pci->time);
	      /*(Cnrs[z].ss[x].pci->date.year >
	       bsres->pci->date.year) ? TRUE : (Cnrs[z].ss[x].pci->date.year <
						bsres->pci->date.
						year) ? FALSE : (Cnrs[z].
								 ss[x].pci->
								 date.month >
								 bsres->pci->
								 date.
								 month) ? TRUE
	      : (Cnrs[z].ss[x].pci->date.month <
		 bsres->pci->date.month) ? FALSE : (Cnrs[z].ss[x].pci->date.
						    day >
						    bsres->pci->date.
						    day) ? TRUE : (Cnrs[z].
								   ss[x].pci->
								   date.day <
								   bsres->
								   pci->date.
								   day) ?
	      FALSE : (Cnrs[z].ss[x].pci->time.hours >
		       bsres->pci->time.hours) ? TRUE : (Cnrs[z].ss[x].pci->
							 time.hours <
							 bsres->pci->time.
							 hours) ? FALSE
	      : (Cnrs[z].ss[x].pci->time.minutes >
		 bsres->pci->time.minutes) ? TRUE : (Cnrs[z].ss[x].pci->time.
						     minutes <
						     bsres->pci->time.
						     minutes) ? FALSE
	      : (Cnrs[z].ss[x].pci->time.seconds >
		 bsres->pci->time.seconds) ? TRUE : (Cnrs[z].ss[x].pci->time.
						     seconds <
						     bsres->pci->time.
						     seconds) ? FALSE : FALSE;*/
	    if (cmp != 1)
	      Cnrs[z].ss[x].newest = FALSE;
	    /*cmp =
	      (Cnrs[z].ss[x].pci->date.year <
	       bsres->pci->date.year) ? TRUE : (Cnrs[z].ss[x].pci->date.year >
						bsres->pci->date.
						year) ? FALSE : (Cnrs[z].
								 ss[x].pci->
								 date.month <
								 bsres->pci->
								 date.
								 month) ? TRUE
	      : (Cnrs[z].ss[x].pci->date.month >
		 bsres->pci->date.month) ? FALSE : (Cnrs[z].ss[x].pci->date.
						    day <
						    bsres->pci->date.
						    day) ? TRUE : (Cnrs[z].
								   ss[x].pci->
								   date.day >
								   bsres->
								   pci->date.
								   day) ?
	      FALSE : (Cnrs[z].ss[x].pci->time.hours <
		       bsres->pci->time.hours) ? TRUE : (Cnrs[z].ss[x].pci->
							 time.hours >
							 bsres->pci->time.
							 hours) ? FALSE
	      : (Cnrs[z].ss[x].pci->time.minutes <
		 bsres->pci->time.minutes) ? TRUE : (Cnrs[z].ss[x].pci->time.
						     minutes >
						     bsres->pci->time.
						     minutes) ? FALSE
	      : (Cnrs[z].ss[x].pci->time.seconds <
		 bsres->pci->time.seconds) ? TRUE : (Cnrs[z].ss[x].pci->time.
						     seconds >
						     bsres->pci->time.
						     seconds) ? FALSE : FALSE;*/
	    if (cmp != -1)
	      Cnrs[z].ss[x].oldest = FALSE;
	    cmp = 0;
	    break;
	  }
	  else
	    Cnrs[z].ss[x].all = FALSE;
	}
      }
      if (Cnrs[z].ss[x].unique)
	Cnrs[z].ss[x].oldest = Cnrs[z].ss[x].newest = Cnrs[z].ss[x].all =
	  Cnrs[z].ss[x].largest = Cnrs[z].ss[x].smallest = FALSE;
      DosSleep(1);
    }
    DosSleep(1);
  }

  switch (action) {
  case IDM_SELECTBOTH:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].all)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_SELECTMORE:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (!Cnrs[z].ss[x].unique)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      DosSleep(0);  //26 Aug 07 GKY 1
    }
    break;
  case IDM_SELECTONE:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].unique)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      DosSleep(0);  //26 Aug 07 GKY 1
    }
    break;
  case IDM_SELECTNEWER:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].newest)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_SELECTOLDER:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].oldest)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_SELECTBIGGER:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].largest)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_SELECTSMALLER:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].smallest)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(TRUE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;

  case IDM_DESELECTBOTH:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].all)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(FALSE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_DESELECTMORE:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (!Cnrs[z].ss[x].unique)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(FALSE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_DESELECTONE:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].unique)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(FALSE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_DESELECTNEWER:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].newest)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(FALSE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_DESELECTOLDER:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].oldest)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(FALSE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_DESELECTBIGGER:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].largest)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(FALSE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  case IDM_DESELECTSMALLER:
    for (z = 0; z < numwindows; z++) {
      for (x = 0; x < Cnrs[z].numfiles; x++) {
	if (Cnrs[z].ss[x].smallest)
	  WinSendMsg(Cnrs[z].hwndCnr, CM_SETRECORDEMPHASIS,
		     MPFROMP(Cnrs[z].ss[x].pci),
		     MPFROM2SHORT(FALSE, CRA_SELECTED));
      }
      DosSleep(0); //26 Aug 07 GKY 1
    }
    break;
  }

  FreeCnrs(Cnrs, numwindows);
}

#pragma alloc_text(SELECT,UnHilite,SelectAll,DeselectAll,MarkAll,SetMask)
#pragma alloc_text(SELECT,SelectList)
#pragma alloc_text(SELECT1,Deselect,HideAll,RemoveAll,ExpandAll,InvertAll)
#pragma alloc_text(SELECT3,SpecialSelect)
#pragma alloc_text(SELECT4,FreeCnrs,SpecialSelect2,CompSSNames,CompSSNamesB)
