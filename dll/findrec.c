
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2015 Steven H.Levine

  Find records

  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  28 Dec 08 GKY Containers will only scroll to the right if needed to show end of selected
		item and will scroll left to eliminate space after a selected item. Ticket 204
  06 Aug 15 SHL Clean up and comment
  23 Aug 15 SHL Protect FindCnrRecord filename arg
  20 Sep 15 GKY Add a correction factor so directories don't get placed above the top of the
                tree container when a large drive has been expanded.

***********************************************************************/

#include <string.h>
#include <stdlib.h>

#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "findrec.h"
#include "errutil.h"			// Dos_Error...

//static PSZ pszSrcFile = __FILE__;

PCNRITEM FindCnrRecord(HWND hwndCnr, PCSZ filename, PCNRITEM pciParent,
		       BOOL partial, BOOL partmatch, BOOL noenv)
{
  SEARCHSTRING srch;
  PCNRITEM pci;
  PCSZ file;
  PCSZ p;

  if (partial) {
    if (strlen(filename) > 3) {
      file = strrchr(filename, '\\');
      if (!file) {
	file = strrchr(filename, ':');
	if (file)
	  file++;
	else
	  file = filename;
      }
      else
	file++;
    }
    else
      file = filename;
  }
  else
    file = filename;
  memset(&srch, 0, sizeof(SEARCHSTRING));
  srch.cb = (ULONG) sizeof(SEARCHSTRING);
  srch.pszSearch = (PSZ) file;
  srch.fsPrefix = FALSE;
  srch.fsCaseSensitive = FALSE;
  srch.usView = CV_TREE;
  if (!pciParent)
    pciParent = (PCNRITEM) CMA_FIRST;
  pci = WinSendMsg(hwndCnr,
                   CM_SEARCHSTRING, MPFROMP(&srch), MPFROMP(pciParent));
  while (pci && (INT) pci != -1) {
    if (!noenv || (pci->flags & (RECFLAGS_ENV | RECFLAGS_UNDERENV)) == 0) {
      // CNRITEM for file/directory
      if (!partmatch) {			// file name must match full name
	if (!stricmp(pci->pszFileName, filename))
	  return pci;			// got full match
      }
      else {				// only root name must match
	// partial match
	if (strlen(pci->pszFileName) <= 3)
	  p = pci->pszFileName;			// Root
	else {
	  p = strrchr(pci->pszFileName, '\\');
	  if (p)
	    p++;			// After slash
	  else {
	    p = strrchr(pci->pszFileName, ':');
	    if (p)
	      p++;			// After colon
	    else
	      p = pci->pszFileName;	// Must be bare file name
	  }
	}
	if (!stricmp(p, file))
	  return pci;			// got partial match
      }
    }
    pci = WinSendMsg(hwndCnr, CM_SEARCHSTRING, MPFROMP(&srch), MPFROMP(pci));
  }

  return NULL;				// failure
}

PCNRITEM FindParentRecord(HWND hwndCnr, PCNRITEM pciC)
{

  PCNRITEM pciP = (PCNRITEM) NULL, pci = pciC;

  if (pci) {
    for (;;) {
      pciP = WinSendMsg(hwndCnr,
			CM_QUERYRECORD,
			MPFROMP(pci),
			MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER));
      if (pciP && (INT) pciP != -1)
	pci = pciP;
      else
	break;
    }
  }
  return pci;
}

VOID ShowCnrRecord(HWND hwndCnr, PMINIRECORDCORE pmi)
{

  QUERYRECORDRECT qrecrct;
  RECTL rcl;
  RECTL rclViewport;
  RECTL rclFirst;
  RECTL rclLast;
  PMINIRECORDCORE pmiFirst;
  PMINIRECORDCORE pmiLast;
  INT correction;

  pmiFirst = WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(NULL),
                        MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  pmiLast = WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(NULL),
                       MPFROM2SHORT(CMA_LAST, CMA_ITEMORDER));
  WinSendMsg(hwndCnr,
	     CM_QUERYVIEWPORTRECT,
	     MPFROMP(&rclViewport), MPFROM2SHORT(CMA_WINDOW , TRUE));
  memset(&qrecrct, 0, sizeof(QUERYRECORDRECT));
  qrecrct.cb = sizeof(QUERYRECORDRECT);
  qrecrct.pRecord = (PRECORDCORE) pmi;
  qrecrct.fsExtent = (CMA_ICON | CMA_TEXT | CMA_TREEICON);
  if (!WinSendMsg(hwndCnr,
		  CM_QUERYRECORDRECT, MPFROMP(&rcl), MPFROMP(&qrecrct))) {
    qrecrct.fsExtent = CMA_TEXT | CMA_TREEICON;
    WinSendMsg(hwndCnr, CM_QUERYRECORDRECT, MPFROMP(&rcl), MPFROMP(&qrecrct));
  }
  qrecrct.pRecord = (PRECORDCORE) pmiFirst;
  WinSendMsg(hwndCnr, CM_QUERYRECORDRECT, MPFROMP(&rclFirst), MPFROMP(&qrecrct));
  qrecrct.pRecord = (PRECORDCORE) pmiLast;
  WinSendMsg(hwndCnr, CM_QUERYRECORDRECT, MPFROMP(&rclLast), MPFROMP(&qrecrct));
  correction = 5 + ((abs(rclFirst.yTop) + abs(rclLast.yTop)) / 22500);
  WinSendMsg(hwndCnr,
	     CM_SCROLLWINDOW,
	     MPFROMSHORT(CMA_VERTICAL),
             MPFROMLONG((rclViewport.yTop - (rcl.yTop) - correction)));
  WinSendMsg(hwndCnr,
	     CM_SCROLLWINDOW,
	     MPFROMSHORT(CMA_HORIZONTAL), MPFROMLONG(rcl.xRight - rclViewport.xRight));
  
}

#pragma alloc_text(FINDREC,FindCnrRecord,FindParentRecord,ShowCnrRecord)
