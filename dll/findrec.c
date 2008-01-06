
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2007 Steven H.Levine

  Find records

  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#include <string.h>

#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"

PCNRITEM FindCnrRecord(HWND hwndCnr, CHAR * filename, PCNRITEM pciParent,
		       BOOL partial, BOOL partmatch, BOOL noenv)
{
  SEARCHSTRING srch;
  PCNRITEM pci;
  register CHAR *file, *p;

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
  srch.usView = CV_TREE;		/* | CV_EXACTMATCH; */
  if (!pciParent)
    pciParent = (PCNRITEM) CMA_FIRST;
  pci = WinSendMsg(hwndCnr,
		   CM_SEARCHSTRING, MPFROMP(&srch), MPFROMP(pciParent));
  while (pci && (INT) pci != -1) {
    if (!noenv || (pci->flags & (RECFLAGS_ENV | RECFLAGS_UNDERENV)) == 0) {
      if (!partmatch) {			/* full name must match full name */
	if (!stricmp(pci->pszFileName, filename))
	  return pci;			/* success */
      }
      else {				/* only root name must match */
	if (strlen(pci->pszFileName) > 3) {
	  p = strrchr(pci->pszFileName, '\\');
	  if (!p) {
	    p = strrchr(pci->pszFileName, ':');
	    if (p)
	      p++;
	    else
	      p = pci->pszFileName;
	  }
	  else
	    p++;
	}
	else
	  p = pci->pszFileName;
	if (!stricmp(p, file))
	  return pci;			/* success */
      }
    }
    pci = WinSendMsg(hwndCnr, CM_SEARCHSTRING, MPFROMP(&srch), MPFROMP(pci));
  }

  return NULL;				/* failure */
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

  memset(&qrecrct, 0, sizeof(QUERYRECORDRECT));
  qrecrct.cb = sizeof(QUERYRECORDRECT);
  qrecrct.pRecord = (PRECORDCORE) pmi;
  qrecrct.fsExtent = (CMA_ICON | CMA_TEXT | CMA_TREEICON);
  if (!WinSendMsg(hwndCnr,
		  CM_QUERYRECORDRECT, MPFROMP(&rcl), MPFROMP(&qrecrct))) {
    qrecrct.fsExtent = CMA_TEXT | CMA_TREEICON;
    WinSendMsg(hwndCnr, CM_QUERYRECORDRECT, MPFROMP(&rcl), MPFROMP(&qrecrct));
  }
  WinSendMsg(hwndCnr,
	     CM_QUERYVIEWPORTRECT,
	     MPFROMP(&rclViewport), MPFROM2SHORT(CMA_WINDOW, FALSE));
  WinSendMsg(hwndCnr,
	     CM_SCROLLWINDOW,
	     MPFROMSHORT(CMA_VERTICAL),
	     MPFROMLONG((rclViewport.yTop - rcl.yTop) - 4));
  WinSendMsg(hwndCnr,
	     CM_SCROLLWINDOW,
	     MPFROMSHORT(CMA_HORIZONTAL), MPFROMLONG(rcl.xLeft - 4));
}

#pragma alloc_text(FINDREC,FindCnrRecord,FindParentRecord,ShowCnrRecord)
