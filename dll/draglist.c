
/***********************************************************************

  $Id$

  Drag drop support

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2007 Steven H.Levine

  16 Oct 02 SHL DoFileDrag: don't free stack
  26 Jul 06 SHL Check more run time errors
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits
  06 Apr 07 GKY Add DeleteDragitemStrHandles
  06 Apr 07 GKY Add some error checking in drag/drop
  19 Apr 07 SHL Rework DeleteDragitemStrHandles to be FreeDragInfoData
  19 Apr 07 SHL Add more drag/drop error checking
  19 Apr 07 SHL Optimize DRAGITEM DRAGIMAGE array access
  21 Apr 07 SHL Avoid odd first time drag failure
  12 May 07 SHL Use dcd->ulItemsToUnHilite
  05 Jul 07 SHL FreeDragInfoData: suppress PMERR_SOURCE_SAME_AS_TARGET notices
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Feb 08 GKY Use xmallocz where appropriate

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_SHLERRORS
#define INCL_LONGLONG

#include "errutil.h"			// Dos_Error...
#include "fm3dll.h"
#include "fortify.h"

static PSZ pszSrcFile = __FILE__;

/**
 * Delete drag item string handles.
 * Work around for DrgDeleteDraginfoStrHandles
 * which seems to fail with a large number of strings
 * Assume called after successful DrgAccessDraginfo
 */


// #define USE_FAST_FREE		// Define to let PM do free

VOID FreeDragInfoData (HWND hwnd, PDRAGINFO pDInfo)
{
  PDRAGITEM pDItem;
  ULONG cDitem;
  ULONG curitem;
  APIRET ok;

# ifdef USE_FAST_FREE
  if (!DrgDeleteDraginfoStrHandles(pDInfo)) {
    Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
	      "DrgDeleteDraginfoStrHandles");
  }
# else // The slow way
  cDitem = DrgQueryDragitemCount(pDInfo);
  for (curitem = 0; curitem < cDitem; curitem++) {
    pDItem = DrgQueryDragitemPtr(pDInfo, curitem);
    if (!pDItem) {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		"DrgQueryDragitemPtr(%u)", curitem);
    }
    else {
      ok = DrgDeleteStrHandle(pDItem->hstrType);
      if (!ok) {
	Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      "DrgDeleteStrHandle(0x%x) hstrType",pDItem->hstrType);
      }
      ok = DrgDeleteStrHandle(pDItem->hstrRMF);
      if (!ok) {
	Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      "DrgDeleteStrHandle(0x%x) hstrRMF",pDItem->hstrRMF);
      }
      ok = DrgDeleteStrHandle(pDItem->hstrContainerName);
      if (!ok) {
	Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      "DrgDeleteStrHandle(0x%x) hstrContainerName",pDItem->hstrContainerName);
      }
      ok = DrgDeleteStrHandle(pDItem->hstrSourceName);
      if (!ok) {
	Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      "DrgDeleteStrHandle(0x%x) hstrSourceName",pDItem->hstrSourceName);
      }
      ok = DrgDeleteStrHandle(pDItem->hstrTargetName);
      if (!ok) {
	Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      "DrgDeleteStrHandle(0x%x) hstrTargetName",pDItem->hstrTargetName);
      }
    }
  } // for
# endif
  if (!DrgFreeDraginfo(pDInfo)) {
    // PMERR_SOURCE_SAME_AS_TARGET is not an error if dragging within same fm/2 process
    if (!IsFm2Window(pDInfo->hwndSource, FALSE) ||
	(WinGetLastError(WinQueryAnchorBlock(hwnd)) & 0xffff) != PMERR_SOURCE_SAME_AS_TARGET)
    {
      Win_Error(hwnd, hwnd, pszSrcFile, __LINE__, "DrgFreeDraginfo");
    }
  }
}

HWND DragOne(HWND hwndCnr, HWND hwndObj, CHAR * filename, BOOL moveok)
{

  DRAGITEM DItem;
  HWND hDrop = 0;
  DRAGIMAGE fakeicon;
  PDRAGINFO pDInfo;
  FILESTATUS3 fs3;
  CHAR szDir[CCHMAXPATH], szFile[CCHMAXPATH], *p;

  if (filename && *filename) {
    if ((IsRoot(filename) && IsValidDrive(*filename)) ||
	!DosQueryPathInfo(filename, FIL_STANDARD, &fs3, sizeof(fs3))) {
      strcpy(szDir, filename);
      p = szDir;
      while (*p) {
	if (*p == '/')
	  *p = '\\';
	p++;
      }
      p = strrchr(szDir, '\\');
      if (p) {
	*p = 0;
	p++;
	strcpy(szFile, p);
	strcat(szDir, "\\");
      }
      else {
	strcpy(szFile, filename);
	*szDir = 0;
      }
      memset(&fakeicon, 0, sizeof(DRAGIMAGE));
      fakeicon.hImage = (IsRoot(filename) ||
			 (fs3.attrFile & FILE_DIRECTORY) != 0) ?
	hptrDir : hptrFile;
      memset(&DItem, 0, sizeof(DRAGITEM));
      DItem.hwndItem = (hwndObj) ? hwndObj : hwndCnr;	// Initialize DRAGITEM
      // DItem.hwndItem = hwndCnr;
      DItem.ulItemID = 1;
      DItem.hstrType = DrgAddStrHandle(DRT_UNKNOWN);
      DItem.hstrRMF = DrgAddStrHandle(DRMDRFLIST);
      DItem.hstrContainerName = DrgAddStrHandle(szDir);
      DItem.hstrSourceName = DrgAddStrHandle(szFile);
      if (!DItem.hstrSourceName)
	 Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
	 "DrgQueryStrName");
      DItem.hstrTargetName = DrgAddStrHandle(szFile);
      DItem.fsControl = 0;
      if (IsRoot(filename) || (fs3.attrFile & FILE_DIRECTORY) != 0)
	DItem.fsControl |= DC_CONTAINER;
      if (IsFullName(filename) &&
	  (driveflags[toupper(*filename) - 'A'] & DRIVE_REMOVABLE))
	DItem.fsControl |= DC_REMOVEABLEMEDIA;
      DItem.fsSupportedOps = DO_COPYABLE | DO_LINKABLE;
      if (moveok && IsFullName(filename) &&
	  !(driveflags[toupper(*filename) - 'A'] & DRIVE_NOTWRITEABLE))
	DItem.fsSupportedOps |= DO_MOVEABLE;
      if (IsRoot(filename))
	DItem.fsSupportedOps = DO_LINKABLE;
      fakeicon.cb = sizeof(DRAGIMAGE);
      fakeicon.cptl = 0;
      fakeicon.fl = DRG_ICON;
      fakeicon.sizlStretch.cx = 32;
      fakeicon.sizlStretch.cy = 32;
      fakeicon.cxOffset = -16;
      fakeicon.cyOffset = 0;
      pDInfo = DrgAllocDraginfo(1);
      if (pDInfo) {
	if (IsFullName(filename) &&
	    (driveflags[toupper(*filename) - 'A'] & DRIVE_NOTWRITEABLE))
	  pDInfo->usOperation = DO_COPY;
	else
	  pDInfo->usOperation = DO_DEFAULT;
	if (IsRoot(filename))
	  pDInfo->usOperation = DO_LINK;
	pDInfo->hwndSource = (hwndObj) ? hwndObj : hwndCnr;
	// pDInfo->hwndSource = hwndCnr;
	DrgSetDragitem(pDInfo, &DItem, sizeof(DRAGITEM), 0);
	WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);
	hDrop = DrgDrag(hwndCnr,
			pDInfo,
			&fakeicon,
			1,		// DragImage count
			VK_ENDDRAG,	// Drag end button
			NULL);
	if (hDrop == NULLHANDLE)
	  FreeDragInfoData(hwndCnr, pDInfo);
	WinSetWindowPos(hwndCnr, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE);
      }
    }
  }
  return hDrop;
}

HWND DoFileDrag(HWND hwndCnr, HWND hwndObj, PCNRDRAGINIT pcd, CHAR * arcfile,
		CHAR * directory, BOOL moveok)
{
  // Drag files from a container

  BOOL isdir, rooting = FALSE;
  PCNRITEM pci;
  CHAR *p;
  INT attribute = CRA_CURSORED;
  PDRAGINFO pDInfo = NULL;
  DRAGITEM **ppDItem = NULL, **ppDITest;
  DRAGITEM *pDItem;
  PCNRITEM pciRec = (PCNRITEM) pcd->pRecord;
  HWND hDrop = NULLHANDLE;
  ULONG ulNumfiles = 0, ulNumDIAlloc = 0, ulSelect, ulNumIcon = 0;
  CHAR szFile[CCHMAXPATH], szBuffer[CCHMAXPATH];
  DRAGIMAGE *paDImgIcons = NULL, *pDImg, dimgFakeIcon;
  BOOL ok;
  UINT c;
  DIRCNRDATA *dcd;

  static BOOL first_drag = TRUE;

  // Count items to unhilite, pass to UnHilite when partial unhilite required
  dcd = INSTDATA(hwndCnr);
  dcd->ulItemsToUnHilite = 0;
  if (!pciRec && directory && *directory)
    return DragOne(hwndCnr, hwndObj, directory, moveok);

  if (!pciRec) {
    pci = (PCNRITEM) WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
				MPFROMLONG(CMA_FIRST),
				MPFROMSHORT(attribute));
    if (pci && (INT) pci > -1) {
      if (pci->rc.flRecordAttr & CRA_SELECTED) {
	attribute = CRA_SELECTED;
	pci =
	  WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		     MPFROMSHORT(attribute));
      }
    }
  }
  else {
    pci = pciRec;
    attribute = (pci->rc.flRecordAttr & CRA_SELECTED) ? CRA_SELECTED : 0;
    if (attribute) {
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMLONG(CMA_FIRST),
		       MPFROMSHORT(attribute));
    }
  }

  ulSelect = 0;
  while (pci && (INT) pci > -1) {
    if (!(pci->rc.flRecordAttr & CRA_FILTERED)) {
      if (IsRoot(pci->pszFileName) && !IsValidDrive(*pci->pszFileName))
	goto Continuing;
      if (!arcfile) {
	strcpy(szBuffer, pci->pszFileName);
	p = strrchr(szBuffer, '\\');
	if (p) {
	  p++;
	  strcpy(szFile, p);
	  *p = 0;
	}
	else
	  goto Continuing;
      }
      else
	strcpy(szFile, pci->pszFileName);
    }
    if (!arcfile) {
      // Filesystem object
      isdir = pci->attrFile & FILE_DIRECTORY;
      // fixme to expand smarter - expand fast at first - do same for similar code
      if (ulNumfiles + 2 > ulNumDIAlloc) {
	// Expand
	if (!paDImgIcons) {
	  pDImg =
	    xrealloc(paDImgIcons, sizeof(DRAGIMAGE) * (ulNumDIAlloc + 4L),
		     pszSrcFile, __LINE__);
	  if (!pDImg)
	    break;
	  paDImgIcons = pDImg;
	}
	else if (!ulNumIcon) {
	  pDImg = &paDImgIcons[ulNumfiles];
	  pDImg->cb = sizeof(DRAGIMAGE);
	  pDImg->cptl = 0;
	  pDImg->hImage = hptrLast;
	  pDImg->fl = DRG_ICON;
	  pDImg->sizlStretch.cx = 32;
	  pDImg->sizlStretch.cy = 32;
	  pDImg->cxOffset = -16 + (ulNumfiles * 4);
	  pDImg->cyOffset = 0 + (ulNumfiles * 7);
	  ulNumIcon = ulNumfiles + 1;
	}
	ppDITest =
	  xrealloc(ppDItem, sizeof(DRAGITEM *) * (ulNumDIAlloc + 4L),
		   pszSrcFile, __LINE__);
	if (!ppDITest)
	  break;
	ppDItem = ppDITest;
	ulNumDIAlloc += 4L;
      }
      // Create & Initialize DRAGITEM
      pDItem = xmallocz(sizeof(DRAGITEM), pszSrcFile, __LINE__);
      if (!pDItem)
	break;				// Already complained
      ppDItem[ulNumfiles] = pDItem;
      if (!ulNumIcon) {
	pDImg = &paDImgIcons[ulNumfiles];
	pDImg->cb = sizeof(DRAGIMAGE);
	pDImg->cptl = 0;
	pDImg->hImage = pci->rc.hptrIcon;
	if (!pDImg->hImage)
	  pDImg->hImage = isdir ? hptrDir : hptrFile;
	pDImg->fl = DRG_ICON;
	pDImg->sizlStretch.cx = 32;
	pDImg->sizlStretch.cy = 32;
	pDImg->cxOffset = -16 + (ulNumfiles * 3);
	pDImg->cyOffset = 0 + (ulNumfiles * 6);
      }
      pDItem->hwndItem = (hwndObj) ? hwndObj : hwndCnr;
      pDItem->hwndItem = hwndCnr;
      pDItem->ulItemID = (ULONG) pci;
      pDItem->hstrType = DrgAddStrHandle(DRT_UNKNOWN);
      ok = pDItem->hstrType;
      pDItem->hstrRMF = DrgAddStrHandle(DRMDRFLIST);
      ok = ok && pDItem->hstrRMF;
      pDItem->hstrContainerName = DrgAddStrHandle(szBuffer);
      ok = ok && pDItem->hstrContainerName;
      pDItem->hstrSourceName = DrgAddStrHandle(szFile);
      ok = ok && pDItem->hstrSourceName;
      pDItem->hstrTargetName = DrgAddStrHandle(szFile);
      ok = ok && pDItem->hstrTargetName;
      if (!ok) {
	DbgMsg(pszSrcFile, __LINE__, "DrgAddStrHandle failed at %lu for %s", ulNumfiles, szFile);
	// If we have string handle add overflow, release corrupt DragItem
	// We release 3 more to work around 1st time drag failure reported by Gregg
	// fixme to know why this happens - PM may need to create a handle?
	c = first_drag ? 4 : 1;
	first_drag = FALSE;
	for (; c > 0 && ulNumfiles > 0; c--) {
	  if (pDItem->hstrType)
	    DrgDeleteStrHandle(pDItem->hstrType);
	  if (pDItem->hstrRMF)
	    DrgDeleteStrHandle(pDItem->hstrRMF);
	  if (pDItem->hstrContainerName)
	    DrgDeleteStrHandle(pDItem->hstrContainerName);
	  if (pDItem->hstrSourceName)
	    DrgDeleteStrHandle(pDItem->hstrSourceName);
	  if (pDItem->hstrTargetName)
	    DrgDeleteStrHandle(pDItem->hstrTargetName);
	  xfree(pDItem, pszSrcFile, __LINE__);
	  // Last item not yet count so only decrement by one less than loop count
	  // Unhilite code will adjust this when unhighliting
	  if (c > 1) {
	    ulNumfiles--;
	    pDItem = ppDItem[ulNumfiles];
	  }
	}
	// Set count to actual count + 1 to ensure count non-zero on any failure
	dcd->ulItemsToUnHilite = ulNumfiles + 1;
	break;
      }
      pDItem->fsControl = isdir ? DC_CONTAINER : 0;
      if (IsFullName(pci->pszFileName) &&
	  (driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_REMOVABLE))
	pDItem->fsControl |= DC_REMOVEABLEMEDIA;
      pDItem->fsSupportedOps = DO_COPYABLE | DO_LINKABLE;
      if (moveok && IsFullName(pci->pszFileName) &&
	  !(driveflags[toupper(*pci->pszFileName) - 'A'] &
	    DRIVE_NOTWRITEABLE))
	pDItem->fsSupportedOps |= DO_MOVEABLE;
      if (IsRoot(pci->pszFileName)) {
	pDItem->fsSupportedOps = DO_LINKABLE;
	rooting = TRUE;
      }
      ulNumfiles++;
      // ppDItem[ulNumfiles] = NULL;	// Why bother - can't we count - fixme to be gone?
    } // if filesystem object
    else {
      // Archive object
      if (ulNumfiles + 3L > ulNumDIAlloc) {
	ppDITest =
	  xrealloc(ppDItem, sizeof(DRAGITEM *) * (ulNumDIAlloc + 5L),
		   pszSrcFile, __LINE__);
	if (!ppDITest)
	  break;
	ppDItem = ppDITest;
	ulNumDIAlloc += 5L;
      }
      // Create & Initialize DRAGITEM
      pDItem = xmallocz(sizeof(DRAGITEM), pszSrcFile, __LINE__);
      if (!pDItem)
	break;
      ppDItem[ulNumfiles] = pDItem;
      dimgFakeIcon.hImage = hptrFile;
      pDItem->hwndItem = (hwndObj) ? hwndObj : hwndCnr;
      pDItem->hwndItem = hwndCnr;
      pDItem->ulItemID = (ULONG) pci;
      pDItem->hstrType = DrgAddStrHandle(DRT_UNKNOWN);
      ok = pDItem->hstrType;
      pDItem->hstrRMF = DrgAddStrHandle(DRMDRFOS2FILE);
      ok = ok && pDItem->hstrRMF;
      pDItem->hstrContainerName = DrgAddStrHandle(arcfile);
      ok = ok && pDItem->hstrContainerName;
      pDItem->hstrSourceName = DrgAddStrHandle(szFile);
      ok = ok && pDItem->hstrSourceName;
      pDItem->hstrTargetName = DrgAddStrHandle(szFile);
      ok = ok && pDItem->hstrTargetName;
      if (!ok){
	if (pDItem->hstrType)
	  DrgDeleteStrHandle(pDItem->hstrType);
	if (pDItem->hstrRMF)
	  DrgDeleteStrHandle(pDItem->hstrRMF);
	if (pDItem->hstrContainerName)
	  DrgDeleteStrHandle(pDItem->hstrContainerName);
	if (pDItem->hstrSourceName)
	  DrgDeleteStrHandle(pDItem->hstrSourceName);
	if (pDItem->hstrTargetName)
	  DrgDeleteStrHandle(pDItem->hstrTargetName);
	xfree(pDItem, pszSrcFile, __LINE__);
	dcd->ulItemsToUnHilite = ulNumfiles + 1;	// +1 to ensure non-zero
	break;
     }
      pDItem->fsControl = DC_PREPARE;
      if (IsFullName(arcfile) &&
	  (driveflags[toupper(*arcfile) - 'A'] & DRIVE_REMOVABLE))
	pDItem->fsControl |= DC_REMOVEABLEMEDIA;
      pDItem->fsSupportedOps = DO_COPYABLE;
      ulNumfiles++;
      // Create & Initialize DRAGITEM
      pDItem = xmallocz(sizeof(DRAGITEM), pszSrcFile, __LINE__);
      if (pDItem) {
	ppDItem[ulNumfiles] = pDItem;
	dimgFakeIcon.hImage = hptrFile;
	pDItem->hwndItem = (hwndObj) ? hwndObj : hwndCnr;
	pDItem->hwndItem = hwndCnr;
	pDItem->ulItemID = ulSelect++;
	pDItem->hstrType = DrgAddStrHandle(DRT_UNKNOWN);
	ok = ok && pDItem->hstrType;
	pDItem->hstrRMF = DrgAddStrHandle(DRMDRFFM2ARC);
	ok = ok && pDItem->hstrRMF;
	pDItem->hstrContainerName = DrgAddStrHandle(arcfile);
	ok = ok && pDItem->hstrContainerName;
	pDItem->hstrSourceName = DrgAddStrHandle(szFile);
	ok = ok && pDItem->hstrSourceName;
	pDItem->hstrTargetName = DrgAddStrHandle(szFile);
	ok = ok && pDItem->hstrTargetName;
	if (!ok) {
	  DbgMsg(pszSrcFile, __LINE__, "DrgAddStrHandle failed at %lu for %s", ulNumfiles, szFile);
	  // If we have string handle add overflow, release corrupt DragItem
	  // We release 3 more to work around 1st time drag failure reported by Gregg
	  // fixme to know why this happens - PM may need to create a handle?
	  c = first_drag ? 4 : 1;
	  first_drag = FALSE;
	  for (; c > 0 && ulNumfiles > 0; c--) {
	    if (pDItem->hstrType)
	      DrgDeleteStrHandle(pDItem->hstrType);
	    if (pDItem->hstrRMF)
	      DrgDeleteStrHandle(pDItem->hstrRMF);
	    if (pDItem->hstrContainerName)
	      DrgDeleteStrHandle(pDItem->hstrContainerName);
	    if (pDItem->hstrSourceName)
	      DrgDeleteStrHandle(pDItem->hstrSourceName);
	    if (pDItem->hstrTargetName)
	      DrgDeleteStrHandle(pDItem->hstrTargetName);
	    xfree(pDItem, pszSrcFile, __LINE__);
	    // Last item not yet count so only decrement by one less than loop count
	    if (c > 1) {
	      ulNumfiles--;
	      pDItem = ppDItem[ulNumfiles];
	    }
	  }
	  // Set count to actual count + 1 to ensure count non-zero on any failure
	  // Unhilite code will adjust this when unhighliting
	  dcd->ulItemsToUnHilite = ulNumfiles + 1;
	  break;
	}
	pDItem->fsControl = 0;
	if (IsFullName(arcfile) &&
	    (driveflags[toupper(*arcfile) - 'A'] & DRIVE_REMOVABLE))
	  pDItem->fsControl |= DC_REMOVEABLEMEDIA;
	pDItem->fsSupportedOps = DO_COPYABLE;
	ulNumfiles++;
      }
      // ppDItem[ulNumfiles] = NULL;	// Why bother - fixme to be gone?
    } // if archive object
    WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS, MPFROMP(pci),
	       MPFROM2SHORT(TRUE, CRA_SOURCE));

  Continuing:

    if (!attribute)
      break;
    pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMP(pci),
		     MPFROMSHORT(attribute));
  } // while

  if (ulNumfiles) {
    pDInfo = DrgAllocDraginfo(ulNumfiles);
    if (pDInfo) {
      if ((arcfile && *arcfile) || (IsFullName(szBuffer) &&
				    (driveflags[toupper(*szBuffer) - 'A'] &
				     DRIVE_NOTWRITEABLE)))
	pDInfo->usOperation = DO_COPY;
      else
	pDInfo->usOperation = DO_DEFAULT;
      if ((!arcfile || !*arcfile) && rooting)
	pDInfo->usOperation = DO_LINK;
      pDInfo->hwndSource = (hwndObj) ? hwndObj : hwndCnr;
      // pDInfo->hwndSource = hwndCnr;
      for (ulSelect = 0; ulSelect < ulNumfiles; ulSelect++) {
	DrgSetDragitem(pDInfo, ppDItem[ulSelect], sizeof(DRAGITEM), ulSelect);
	xfree(ppDItem[ulSelect], pszSrcFile, __LINE__);
      }
#ifdef __DEBUG_ALLOC__
      _heap_check();
#endif
      xfree(ppDItem, pszSrcFile, __LINE__);
      ppDItem = NULL;			// Remember gone
      DosPostEventSem(CompactSem);

      if (arcfile) {
	dimgFakeIcon.cb = sizeof(DRAGIMAGE);
	dimgFakeIcon.cptl = 0;
	if (ulNumfiles > 1)
	  dimgFakeIcon.hImage = hptrFile;
	dimgFakeIcon.fl = DRG_ICON;
	dimgFakeIcon.sizlStretch.cx = 32;
	dimgFakeIcon.sizlStretch.cy = 32;
	dimgFakeIcon.cxOffset = -16;
	dimgFakeIcon.cyOffset = 0;
	paDImgIcons = &dimgFakeIcon;
      }
      if (!arcfile) {
	if (!ulNumIcon)
	  ulNumIcon = ulNumfiles;
      }
      else
	ulNumIcon = 1;

      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);
      hDrop = DrgDrag(hwndCnr,
		      pDInfo,
		      paDImgIcons,
		      ulNumIcon,
		      VK_ENDDRAG,	// Drag end button
		      NULL);
      WinSetWindowPos(hwndCnr, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE);
    }
  }

  if (hDrop == NULLHANDLE ) {
    dcd->ulItemsToUnHilite = 0;
    if (pDInfo)
      FreeDragInfoData(hwndCnr, pDInfo);
  }

  if (ppDItem)
    xfree(ppDItem, pszSrcFile, __LINE__);
  if (paDImgIcons && paDImgIcons != &dimgFakeIcon)
    xfree(paDImgIcons, pszSrcFile, __LINE__);
  DosPostEventSem(CompactSem);
  MarkAll(hwndCnr, TRUE, FALSE, TRUE);
  return hDrop;
}

HWND DragList(HWND hwnd, HWND hwndObj, CHAR ** list, BOOL moveok)
{
  // Drag a linked list of files

  BOOL isdir;
  register CHAR *p;
  PDRAGINFO pDInfo = NULL;
  DRAGITEM **ppDItem = NULL, **ppDITest;
  DRAGITEM *pDItem;
  HWND hDrop = (HWND) 0;
  ULONG ulNumfiles = 0, ulNumDIAlloc = 0, ulSelect, ulNumIcon = 0;
  CHAR szFile[CCHMAXPATH], szBuffer[CCHMAXPATH];
  DRAGIMAGE *paDImgIcons = NULL, *pDImg;
  FILESTATUS3 fs3;
  BOOL ok;
  DIRCNRDATA *dcd;

  if (!list || !list[0])
    return hDrop;

  dcd = INSTDATA(hwnd);

  for (ulSelect = 0; list[ulSelect]; ulSelect++) {
    if ((!IsRoot(list[ulSelect]) || !IsValidDrive(*list[ulSelect])) &&
	DosQueryPathInfo(list[ulSelect], FIL_STANDARD, &fs3, sizeof(fs3)))
      continue;
    strcpy(szBuffer, list[ulSelect]);
    p = strrchr(szBuffer, '\\');
    if (p) {
      p++;
      strcpy(szFile, p);
      *p = 0;
    }
    else
      continue;
    if (*szFile) {
      isdir = IsRoot(list[ulSelect]) || fs3.attrFile & FILE_DIRECTORY;
      // fixme to expand smarter - expand fast at first - do same for similar code
      if (ulNumfiles + 2 > ulNumDIAlloc) {
	if (!paDImgIcons) {
	  pDImg =
	    xrealloc(paDImgIcons, sizeof(DRAGIMAGE) * (ulNumDIAlloc + 4L),
		     pszSrcFile, __LINE__);
	  if (!pDImg)
	    break;
	  paDImgIcons = pDImg;
	}
	else if (!ulNumIcon) {
	  pDImg = &paDImgIcons[ulNumfiles];
	  pDImg->cb = sizeof(DRAGIMAGE);
	  pDImg->cptl = 0;
	  pDImg->hImage = hptrLast;
	  pDImg->fl = DRG_ICON;
	  pDImg->sizlStretch.cx = 32;
	  pDImg->sizlStretch.cy = 32;
	  pDImg->cxOffset = -16 + (ulNumfiles * 4);
	  pDImg->cyOffset = 0 + (ulNumfiles * 7);
	  ulNumIcon = ulNumfiles + 1;
	}
	ppDITest =
	  xrealloc(ppDItem, sizeof(DRAGITEM *) * (ulNumDIAlloc + 4L),
		   pszSrcFile, __LINE__);
	if (!ppDITest)
	  break;
	ppDItem = ppDITest;
	ulNumDIAlloc += 4L;
      }
      // Create & Initialize DRAGITEM
      pDItem = xmallocz(sizeof(DRAGITEM), pszSrcFile, __LINE__);
      if (!pDItem)
	break;
      ppDItem[ulNumfiles] = pDItem;
      if (!ulNumIcon) {
	pDImg = &paDImgIcons[ulNumfiles];
	pDImg->cb = sizeof(DRAGIMAGE);
	pDImg->cptl = 0;
	pDImg->hImage = isdir ? hptrDir : hptrFile;
	pDImg->fl = DRG_ICON;
	pDImg->sizlStretch.cx = 32;
	pDImg->sizlStretch.cy = 32;
	pDImg->cxOffset = -16 + (ulNumfiles * 3);
	pDImg->cyOffset = 0 + (ulNumfiles * 6);
      }
      pDItem->hwndItem = (hwndObj) ? hwndObj : hwnd;
      pDItem->ulItemID = (ULONG) ulSelect;
      pDItem->hstrType = DrgAddStrHandle(DRT_UNKNOWN);
      ok = pDItem->hstrType;
      pDItem->hstrRMF = DrgAddStrHandle(DRMDRFLIST);
      ok = ok && pDItem->hstrRMF;
      pDItem->hstrContainerName = DrgAddStrHandle(szBuffer);
      ok = ok && pDItem->hstrContainerName;
      pDItem->hstrSourceName = DrgAddStrHandle(szFile);
      ok = ok && pDItem->hstrSourceName;
      pDItem->hstrTargetName = DrgAddStrHandle(szFile);
      ok = ok && pDItem->hstrTargetName;
      if (!ok) {
	if (pDItem->hstrType)
	  DrgDeleteStrHandle(pDItem->hstrType);
	if (pDItem->hstrRMF)
	  DrgDeleteStrHandle(pDItem->hstrRMF);
	if (pDItem->hstrContainerName)
	  DrgDeleteStrHandle(pDItem->hstrContainerName);
	if (pDItem->hstrSourceName)
	  DrgDeleteStrHandle(pDItem->hstrSourceName);
	if (pDItem->hstrTargetName)
	  DrgDeleteStrHandle(pDItem->hstrTargetName);
	xfree(pDItem, pszSrcFile, __LINE__);
	// pDItem = NULL;	// Why bother, we can count - fixme to be gone
	dcd->ulItemsToUnHilite = ulNumfiles + 1;
	break;
      }
      pDItem->fsControl = isdir ? DC_CONTAINER : 0;
      if (IsFullName(list[ulSelect]) &&
	  (driveflags[toupper(*list[ulSelect]) - 'A'] & DRIVE_REMOVABLE))
	pDItem->fsControl |= DC_REMOVEABLEMEDIA;
      pDItem->fsSupportedOps = DO_COPYABLE | DO_LINKABLE;
      if (moveok && IsFullName(list[ulSelect]) &&
	  !(driveflags[toupper(*list[ulSelect]) - 'A'] & DRIVE_NOTWRITEABLE))
	pDItem->fsSupportedOps |= DO_MOVEABLE;
      if (IsRoot(list[ulSelect]))
	pDItem->fsControl = DO_LINKABLE;
      ulNumfiles++;
      // ppDItem[ulNumfiles] = NULL;	// Why bother - fixme to be gone
    }
  } // for
  if (ulNumfiles) {
    pDInfo = DrgAllocDraginfo(ulNumfiles);
    if (pDInfo) {
      if ((IsFullName(szBuffer) &&
	   (driveflags[toupper(*szBuffer) - 'A'] & DRIVE_NOTWRITEABLE)))
	pDInfo->usOperation = DO_COPY;
      else
	pDInfo->usOperation = DO_DEFAULT;
      if (IsRoot(list[0]))
	pDInfo->usOperation = DO_LINK;
      pDInfo->hwndSource = hwndObj ? hwndObj : hwnd;
      // pDInfo->hwndSource = hwnd;
      for (ulSelect = 0; ulSelect < ulNumfiles; ulSelect++) {
	if (!DrgSetDragitem(pDInfo, ppDItem[ulSelect], sizeof(DRAGITEM), ulSelect)) {
	  Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		   "DrgSetDragitem");
	}
	free(ppDItem[ulSelect]);
      }	// for
#ifdef __DEBUG_ALLOC__
      _heap_check();
#endif
      xfree(ppDItem, pszSrcFile, __LINE__);
      ppDItem = NULL;			// Remember gone
      DosPostEventSem(CompactSem);

      if (!ulNumIcon)
	ulNumIcon = ulNumfiles;

      WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);
      hDrop = DrgDrag(hwnd,
		      pDInfo,
		      paDImgIcons,
		      ulNumIcon,
		      VK_ENDDRAG,	// Drag end button
		      (PVOID) NULL);
      if (hDrop == NULLHANDLE) {
	dcd->ulItemsToUnHilite = 0;
	FreeDragInfoData(hwnd, pDInfo);
      }
      xfree(paDImgIcons, pszSrcFile, __LINE__);
      paDImgIcons = NULL;		// Remember gone
      WinSetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE);
      DosPostEventSem(CompactSem);
    }
  }
  xfree(ppDItem, pszSrcFile, __LINE__);
  xfree(paDImgIcons, pszSrcFile, __LINE__);
  return hDrop;
}

#ifdef NEVER				// fixme to be enabled someday?

BOOL PickUp(HWND hwndCnr, HWND hwndObj, PCNRDRAGINIT pcd)
{

  PCNRITEM pci;
  BOOL loop = TRUE;
  PDRAGINFO pdinfoOld = NULL, pdinfoCurrent = NULL;
  ULONG cditem = 0;
  DRAGITEM ditem;
  DRAGIMAGE dimgFakeIcon;
  CHAR szDir[CCHMAXPATH], szFile[CCHMAXPATH], *p;

  pci = (PCNRITEM) pcd->pRecord;
  if (pci && (INT) pci != -1) {
    if (pci->rc.flRecordAttr & CRA_SELECTED) {
      loop = TRUE;
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
		       MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_SELECTED));
    }
    while (pci && (INT) pci != -1 && *pci->pszFileName) {
      if (pdinfoOld || DrgQueryDragStatus() & DGS_LAZYDRAGINPROGRESS) {
	if (!pdinfoOld)
	  pdinfoOld = DrgQueryDraginfoPtr(NULL);
	if (pdinfoOld) {
	  cditem = pdinfoOld->cditem + 1;
	  pdinfoCurrent = DrgReallocDraginfo(pdinfoOld, cditem);
	  pdinfoOld = pdinfoCurrent;
	}
      }
      else
	pdinfoCurrent = pdinfoOld = DrgAllocDraginfo(1);
      if (pdinfoCurrent) {
	strcpy(szDir, pci->pszFileName);
	p = szDir;
	while (*p) {
	  if (*p == '/')
	    *p = '\\';
	  p++;
	}
	p = strrchr(szDir, '\\');
	if (p) {
	  *p = 0;
	  p++;
	  strcpy(szFile, p);
	  strcat(szDir, "\\");
	}
	else {
	  strcpy(szFile, pci->pszFileName);
	  *szDir = 0;
	}
	ditem.ulItemID = (ULONG) pci;
	ditem.hwndItem = (hwndObj) ? hwndObj : hwndCnr;
	ditem.hstrType = DrgAddStrHandle(DRT_UNKNOWN);
	ditem.hstrRMF = DrgAddStrHandle(DRMDRFLIST);
	ditem.hstrContainerName = DrgAddStrHandle(szDir);
	ditem.hstrSourceName = DrgAddStrHandle(szFile);
	// fixme to check better if code ever enabled
	if (!ditem.hstrSourceName) {
	  Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		   "DrgAddStrHandle");
	}
	ditem.hstrTargetName = DrgAddStrHandle(szFile);
	ditem.fsControl = 0;
	if (IsRoot(pci->pszFileName) || (pci->attrFile & FILE_DIRECTORY) != 0)
	  ditem.fsControl |= DC_CONTAINER;
	if (IsFullName(pci->pszFileName) &&
	    (driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_REMOVABLE))
	  ditem.fsControl |= DC_REMOVEABLEMEDIA;
	ditem.fsSupportedOps = DO_COPYABLE | DO_LINKABLE;
	if (IsFullName(pci->pszFileName) &&
	    !(driveflags[toupper(*pci->pszFileName) - 'A'] &
	      DRIVE_NOTWRITEABLE))
	  ditem.fsSupportedOps |= DO_MOVEABLE;
	if (IsRoot(pci->pszFileName))
	  ditem.fsSupportedOps = DO_LINKABLE;
	memset(&dimgFakeIcon, 0, sizeof(DRAGIMAGE));
	dimgFakeIcon.hImage = pci->rc.hptrIcon;
	dimgFakeIcon.cb = sizeof(DRAGIMAGE);
	dimgFakeIcon.cptl = 0;
	dimgFakeIcon.fl = DRG_ICON;
	dimgFakeIcon.sizlStretch.cx = 32;
	dimgFakeIcon.sizlStretch.cy = 32;
	dimgFakeIcon.cxOffset = -16;
	dimgFakeIcon.cyOffset = 0;
	if (IsFullName(pci->pszFileName) &&
	    (driveflags[toupper(*pci->pszFileName) - 'A'] &
	     DRIVE_NOTWRITEABLE))
	  pdinfoCurrent->usOperation = DO_COPY;
	else
	  pdinfoCurrent->usOperation = DO_DEFAULT;
	if (IsRoot(pci->pszFileName))
	  pdinfoCurrent->usOperation = DO_LINK;
	pdinfoCurrent->hwndSource = (hwndObj) ? hwndObj : hwndCnr;
	DrgSetDragitem(pdinfoCurrent, &ditem, sizeof(DRAGITEM), cditem);
      }
      if (!loop)
	break;
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
		       MPFROMP(pci), MPFROMSHORT(CRA_SELECTED));
    } // while
    if (pdinfoCurrent)
      return DrgLazyDrag(hwndCnr, pdinfoCurrent, &dimgFakeIcon, 1, NULL);
  }
  return FALSE;
}

#endif // NEVER

#pragma alloc_text(DRAGLIST,DragOne,DoFileDrag,DragList,PickUp,FreeDragInfoData)
