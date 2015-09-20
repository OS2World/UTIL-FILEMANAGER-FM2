
/***********************************************************************

  $Id$

  Container item selection support routines

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2015 Steven H. Levine

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
  12 Jan 08 SHL Localize SpecialSelect in comp.c
  29 Feb 08 GKY Use xfree where appropriate
  31 May 11 SHL Ensure mask->pszMasks[1] initialize to NULL if not used
  13 Jun 15 GKY Fixed compare selection replaced pszFileNames with pszDisplayNames
  02 Aug 15 GKY Remove unneed SubbyScan code and improve suppression of blank lines and
                duplicate subdirectory name caused by running Stubby in worker threads.
  20 Sep 15 GKY Create CollapseAll and modify ExpandAll to reduce code overhead
                both to try and speed drive expansion. Change ExpandAll to allow it to loop
                in UM_EXPAND until until drive is completely expanded. Changes were need to
                work with Flesh, Stubby and UnFlesh being moved to a thread

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <io.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "select.h"
#include "notebook.h"			// Data declaration(s)
#include "newview.h"			// Data declarations
#include "fm3str.h"
#include "filldir.h"			// RemoveCnrItems
#include "makelist.h"			// AddToList
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "valid.h"			// TestCDates
#include "misc.h"			// CurrentRecord
#include "findrec.h"			// FindCnrRecord
#include "notify.h"			// Notify
#include "literal.h"			// wildcard
#include "wrappers.h"			// xrealloc
#include "strips.h"			// bstrip
#include "stristr.h"			// findstring
#include "fortify.h"
#include "flesh.h"
#if 0
#define  __PMPRINTF__
#include "PMPRINTF.H"
#endif

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
    CHAR *moder = "r";

    fp = xfsopen(filename, moder, SH_DENYNO, pszSrcFile, __LINE__, TRUE);
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
	// Input now contains name of file to select
	pci = FindCnrRecord(hwndCnr,
			    input, pciParent, partial, partial, TRUE);
	if (pci)			// Found it?
	  WinSendMsg(hwndCnr,
		     CM_SETRECORDEMPHASIS,
		     MPFROMP(pci),
		     MPFROM2SHORT((SHORT) ((deselect) ? FALSE : TRUE),
				  CRA_SELECTED));
	else
	  errs++;
	if (errs > 50) {		// Prevent runaway on bad file

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
	      if (wildcard(pszToMatch, Mask.pszMasks[x], TRUE)) {
		markit = TRUE;
	      }
	    }
	    else {
	      if (wildcard(pszToMatch, Mask.pszMasks[x] + 1, TRUE)) {
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
	  CHAR *moderb = "rb";

	  if ((inputFile = xfsopen(pci->pszFileName, moderb, SH_DENYNO,
				   pszSrcFile, __LINE__, TRUE)) != NULL) {
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
	      if (wildcard(pszToMatch, Mask.pszMasks[x], TRUE))
		unmarkit = TRUE;
	    }
	    else {
	      if (wildcard(pszToMatch, Mask.pszMasks[x] + 1, TRUE)) {
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
	  CHAR *moderb = "rb";

	  if ((inputFile = xfsopen(pci->pszFileName, moderb, SH_DENYNO,
				   pszSrcFile, __LINE__, TRUE)) != NULL) {
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
      WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
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

VOID SetMask(PSZ maskstr, MASK *mask)
{
  UINT x;
  PSZ p;

  DosEnterCritSec();
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
      *p = 0;				// Replace ; will nul to terminate string
      p++;
    }
  }					// for
  mask->pszMasks[x] = NULL;		// Mark end
  if (!x)
    mask->pszMasks[1] = NULL;		// Need 1 more for multiple mask detect 2011-05-31 SHL
  DosExitCritSec();
}

BOOL ExpandAll(HWND hwndCnr, INT count, PCNRITEM pciParent)
{
  PCNRITEM pci;
  static BOOL fExpanding = FALSE;
  static INT counter = 1;

  if (count != counter && count != 0) {
    if (count > counter) {
      fExpanding = FALSE;
      counter++;
    }
    else if (count < counter) {
      fExpanding = FALSE;
      counter = 1;
    }
  }
  if (!pciParent)
    pciParent = WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(NULL),
			   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  if (pciParent) {
    pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pciParent),
                                MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    // Only expand items that have childern
    if (pci  && ~pciParent->rc.flRecordAttr & CRA_EXPANDED) {
      WinSendMsg(hwndCnr, CM_EXPANDTREE, MPFROMP(pciParent), MPVOID);
      if (count != 0) {
        fExpanding = TRUE;
        if (!IsFleshWorkListEmpty()) {
          WaitFleshWorkListEmpty(NULL); // Let it expand
        }
      }
    }
    while (pci && (INT)pci != -1) {
      ExpandAll(hwndCnr, count, pci);
      if (!IsFleshWorkListEmpty())
        WaitFleshWorkListEmpty(NULL); // Wait for container to catch up
      pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pci),
                                  MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    }
  }
  //DosSleep(0);
  return fExpanding;
}

VOID CollapseAll(HWND hwndCnr, PCNRITEM pciParent)
{
  PCNRITEM pci;

  if (!pciParent)
    pciParent = WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(NULL),
			   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  if (pciParent) {
    pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pciParent),
                                MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    if ((pciParent->rc.flRecordAttr & CRA_EXPANDED))
      WinSendMsg(hwndCnr, CM_COLLAPSETREE, MPFROMP(pciParent), MPVOID);
    while (pci && (INT)pci != -1) {
      CollapseAll(hwndCnr, pci);
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

  return stricmp((PSZ)s1, ss2->pci->pszDisplayName);
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
    xfree(Cnrs[z].ss, pszSrcFile, __LINE__);
  }
  xfree(Cnrs, pszSrcFile, __LINE__);
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

  // Count directory containers, build array of hwnds
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
    DosSleep(0);			// Allow other windows to update
  }

  // Count records, build array of pointers to records
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
    DosSleep(0);			// Allow other windows to update
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
	  bsres = (struct SS *)bsearch(Cnrs[z].ss[x].pci->pszDisplayName,
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
	    if (cmp != 1)
	      Cnrs[z].ss[x].newest = FALSE;
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
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
      DosSleep(0);			// Allow other windows to update
    }
    break;
  }

  FreeCnrs(Cnrs, numwindows);
}

#pragma alloc_text(SELECT,UnHilite,SelectAll,DeselectAll,MarkAll,SetMask)
#pragma alloc_text(SELECT,SelectList)
#pragma alloc_text(SELECT1,Deselect,HideAll,RemoveAll,ExpandAll,InvertAll)
#pragma alloc_text(SELECT4,FreeCnrs,SpecialSelect2,CompSSNames,CompSSNamesB)
