
/***********************************************************************

  $Id$

  Drop support

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2007 Steven H.Levine

  22 Nov 02 SHL Baseline
  08 Feb 03 SHL DropHelp: calc EA size consistently
  21 Jul 06 SHL Drop dup code
  22 Jul 06 SHL Check more run time errors
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits
  06 Apr 07 GKY Add some error checking in drag/drop
  19 Apr 07 SHL Use FreeDragInfoData
  19 Apr 07 SHL Add more drag/drop error checking
  21 Apr 07 SHL Add debug code to track down reason for PMERR_SOURCE_SAME_AS_TARGET
  10 Jun 07 GKY Add CheckPmDrgLimit including IsFm2Window as part of work around PM drag limit
  02 Aug 07 SHL Lock in DoFileDrop sanity checks
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "avl.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "droplist.h"
#include "draglist.h"			// FreeDragInfoData
#include "misc.h"			// IsFm2Window
#include "wrappers.h"			// xrealloc

#include "fortify.h"

static PSZ pszSrcFile = __FILE__;

static ULONG GetDropCount(HWND hwnd, MPARAM mp1);
static BOOL TwoDrgNames(PDRAGITEM pDItem, CHAR * buffer1, ULONG buflen1,
		 char *buffer2, ULONG buflen2);

BOOL CheckPmDrgLimit(PDRAGINFO pDInfo)
{
    /*
     * Checks for FM2 source window then checks window words
     * for ulItemsToUnHilite and if it is not zero displays a
     * message to the users that not all items are being dragged
     * returns TRUE on success
     */
	  if (!DrgAccessDraginfo(pDInfo)) {
	    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		      "DrgAccessDraginfo");
	    return FALSE;
	  }
	  else if (IsFm2Window(pDInfo->hwndSource, FALSE)) {
	    DIRCNRDATA *dcdsrc = INSTDATA(pDInfo->hwndSource);
	    if (dcdsrc->ulItemsToUnHilite) {
	    saymsg(MB_OK | MB_INFORMATION,
		   HWND_DESKTOP,
		   GetPString(IDS_ERRORTEXT),
		   GetPString(IDS_EXCEEDPMDRGLMT));
	    }
	    DrgFreeDraginfo(pDInfo);
	  }
	  return TRUE;
}

BOOL TwoDrgNames(PDRAGITEM pDItem, CHAR * buffer1, ULONG buflen1,
		 char *buffer2, ULONG buflen2)
{
  /*
   * Gets archive name from directory field, file name from file field
   * Returns FALSE on error, TRUE on success.
   */

  register ULONG len;
  BOOL ret = FALSE;

  if (pDItem && buffer2 && buflen2) {	/* else error calling function */
    if (buffer1 && buflen1)		/* zero buffers */
      *buffer1 = 0;
    *buffer2 = 0;

    if (buffer1 && buflen1) {
      len = DrgQueryStrName(pDItem->hstrContainerName, buflen1, buffer1);
      buffer1[len] = 0;
      if (len) {			/* be sure we get full pathname of arc file */

	char szTemp[CCHMAXPATH + 2];

	if (!DosQueryPathInfo(buffer1,
			      FIL_QUERYFULLNAME, szTemp, sizeof(szTemp))) {
	  strncpy(buffer1, szTemp, buflen1);
	  buffer1[buflen1 - 1] = 0;
	}
      }
      {					/* be sure that file/directory is accessible */
	FILESTATUS3 fsa3;

	DosError(FERR_DISABLEHARDERR);
	if (DosQueryPathInfo(buffer1,
			     FIL_STANDARD,
			     &fsa3,
			     sizeof(fsa3)) ||
	    fsa3.attrFile & FILE_DIRECTORY) {
	  *buffer1 = 0;
	  return ret;
	}
      }
    }

    len = DrgQueryStrName(pDItem->hstrSourceName, buflen2, buffer2);
    buffer2[len] = 0;
    if (len)
      ret = TRUE;
  }
  return ret;
}

BOOL FullDrgName(PDRAGITEM pDItem, CHAR * buffer, ULONG buflen)
{
  /*
   * Gets full name of file from a dragged item.
   * Returns FALSE on error, TRUE on success.
   */

  ULONG len, blen;
  BOOL ret = FALSE;
  APIRET rc;
  FILESTATUS3 fsa3;

  if (pDItem && buffer && buflen) {	/* else error calling function */
    *buffer = 0;			/* zero buffer */

    blen = DrgQueryStrName(pDItem->hstrContainerName, buflen, buffer);
    if(!blen)
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
	      "DrgQueryStrName");
    else {
      if (*(buffer + (blen - 1)) != '\\') {
	*(buffer + blen) = '\\';
	blen++;
      }
    }
    buffer[blen] = 0;
    len = DrgQueryStrName(pDItem->hstrSourceName,
			  buflen - blen, buffer + blen);
    if(!len) {
      // DbgMsg(pszSrcFile, __LINE__, "0x%x %d %d", pDItem->hstrSourceName,
      //           buflen - blen, buffer + blen);
      // Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
      //         "DrgQueryStrName");
    }
    buffer[blen + len] = 0;
    {					/* be sure we get full pathname of file/directory */
      char szTemp[CCHMAXPATH + 2];
      rc = DosQueryPathInfo(buffer,
			    FIL_QUERYFULLNAME, szTemp, sizeof(szTemp));
      if (!rc) {
	strncpy(buffer, szTemp, buflen);
	buffer[buflen - 1] = 0;
      }
      else
       Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		  "DosQueryPathInfo");
    }

    rc = DosQueryPathInfo(buffer, FIL_STANDARD, &fsa3, sizeof(fsa3));
    if (!rc)
      ret = TRUE;
    else {
	Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		  "DosQueryPathInfo");
	*buffer = 0;
    }
  }
  return ret;
}

BOOL GetOneDrop(HWND hwnd, MPARAM mp1, MPARAM mp2, char *buffer, ULONG buflen)
{
  PDRAGITEM pDItem;			/* DRAGITEM struct ptr */
  PDRAGINFO pDInfo;			/* DRAGINFO struct ptr */
  ULONG numitems;
  register ULONG curitem;
  BOOL ret = FALSE;

  if (buffer && buflen)
    *buffer = 0;			/* zero buffer field */

  pDInfo = (PDRAGINFO) mp1;		/* Get DRAGINFO pointer */
  if (pDInfo) {
    if (!DrgAccessDraginfo(pDInfo)) {
      Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		"DrgAccessDraginfo");
    }
    else {
      numitems = DrgQueryDragitemCount(pDInfo);
      pDItem = DrgQueryDragitemPtr(pDInfo,0);
      if (buflen && buffer) {
	if (DrgVerifyRMF(pDItem,	/* Check valid rendering */
			 DRM_OS2FILE,	/* mechanisms and data */
			 NULL) && !(pDItem->fsControl & DC_PREPARE))
	  ret = FullDrgName(pDItem, buffer, buflen);
      }
      // Return targetfail to source for all items - fixme to know why
      for (curitem = 0; curitem < numitems; curitem++) {
	pDItem = DrgQueryDragitemPtr(pDInfo,curitem);
	DrgSendTransferMsg(pDInfo->hwndSource, DM_ENDCONVERSATION,
			   MPFROMLONG(pDItem->ulItemID),
			   MPFROMLONG(DMFL_TARGETFAIL));
      }
      FreeDragInfoData(hwnd, pDInfo);
    }
  }

  return ret;
}

BOOL AcceptOneDrop(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
  PDRAGITEM pDItem;			/* Pointer to DRAGITEM */
  PDRAGINFO pDInfo;			/* Pointer to DRAGINFO */
  BOOL ret = FALSE;

  pDInfo = (PDRAGINFO) mp1;		/* Get DRAGINFO pointer */
  if (pDInfo) {
    if (!DrgAccessDraginfo(pDInfo)) {
      Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		"DrgAccessDraginfo");
    }
    else {
      pDItem = DrgQueryDragitemPtr(pDInfo, 0);

      /* Check valid rendering mechanisms and data formats */
      if (DrgVerifyRMF(pDItem, DRM_OS2FILE, NULL))
	ret = TRUE;

      // FreeDragInfoData(hwnd, pDInfo);	// 20 Apr 07 SHL
      DrgFreeDraginfo(pDInfo);
    }
  }
  return ret;
}

static ULONG GetDropCount(HWND hwnd, MPARAM mp1)
{
  PDRAGINFO pDInfo;
  ULONG numitems;

  pDInfo = mp1;
  if (pDInfo) {
    if (!DrgAccessDraginfo(pDInfo)) {
      Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
	      "DrgAccessDraginfo");
      numitems = 0;			// Oh well
    }
    else {
      numitems = DrgQueryDragitemCount(pDInfo);
      FreeDragInfoData(hwnd, pDInfo);
    }
  }
  return numitems;
}

void DropHelp(MPARAM mp1, MPARAM mp2, HWND hwnd, char *text)
{
  ULONG numitems;

  numitems = GetDropCount(hwnd, mp1);
  saymsg(MB_ENTER | MB_ICONASTERISK,
	 hwnd, GetPString(IDS_FM2DROPHELPTEXT), text, numitems, numitems);
}

LISTINFO *DoFileDrop(HWND hwndCnr, CHAR * directory, BOOL arcfilesok,
		     MPARAM mp1, MPARAM mp2)
{
  /* builds a list from the dropped files */

  BOOL isArc = FALSE, arctest = FALSE;
  PDRAGITEM pDItem;
  PDRAGINFO pDInfo;
  PCNRITEM pci;
  CHAR szFrom[CCHMAXPATH + 1], szArc[CCHMAXPATH + 1];
  CHAR **files = NULL;
  UINT numfiles = 0, numalloc = 0;
  ULONG curitem, numitems;
  PULONGLONG pcbFile = NULL;
  PULONG pulitemID = NULL;
  LISTINFO *li = NULL;
  ARC_TYPE *arcinfo = NULL;
  USHORT Operation;

  UINT numok = 0;			// 02 Aug 07 SHL fixme to be gone someday
  UINT numfail = 0;

  *szArc = 0;
  pci = (PCNRITEM) ((PCNRDRAGINFO) mp2)->pRecord;
  pDInfo = ((PCNRDRAGINFO) mp2)->pDragInfo;
  if (!pDInfo) {
    Runtime_Error(pszSrcFile, __LINE__, "No drag info");
    return NULL;
  }
  if (!DrgAccessDraginfo(pDInfo)) {
    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
	    "DrgAccessDraginfo");
    numitems = 0;			// Avoid death
  }
  else {
    numitems = DrgQueryDragitemCount(pDInfo);
    Operation = pDInfo->usOperation;
    pDItem = DrgQueryDragitemPtr(pDInfo, 0);
    if (!pDItem) {
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		"DrgQueryDragitemPtr");
    }
    else {
      if (Operation == DO_MOVE && !(pDItem->fsSupportedOps & DO_MOVEABLE)) {
	saymsg(MB_ENTER, HWND_DESKTOP, GetPString(IDS_WARNINGTEXT),
	       GetPString(IDS_FORCINGCOPYTEXT));
	Operation = DO_COPY;
      }
    }
  }

  for (curitem = 0; curitem < numitems; curitem++) {
    pDItem = DrgQueryDragitemPtr(pDInfo, curitem);
    if (!pDItem) {
      Win_Error(hwndCnr, hwndCnr, pszSrcFile, __LINE__,
		"DrgQueryDragitemPtr(%u)", curitem);
      break;
    }

    /* ambiguous drop request -- see what's allowed */
    if (Operation == DO_DEFAULT || Operation >= DO_UNKNOWN) {
      if (pDItem->fsSupportedOps & DO_COPYABLE)
	Operation = DO_COPY;
      else if (pDItem->fsSupportedOps & DO_MOVEABLE)
	Operation = DO_MOVE;
      else if (pDItem->fsSupportedOps & DO_LINKABLE)
	Operation = DO_LINK;
    }
    else {
      /* ignore object if selected command not allowed for it */
      BOOL ok;
      switch (Operation) {
      case DO_MOVE:
	ok = pDItem->fsSupportedOps & DO_MOVEABLE;
	break;
      case DO_COPY:
	ok = pDItem->fsSupportedOps & DO_COPYABLE;
	break;
      case DO_LINK:
	ok = pDItem->fsSupportedOps & DO_LINKABLE;
	break;
      default:
	ok = FALSE;
      }
      if (!ok) {
	// Fail request
	DrgSendTransferMsg(pDItem->hwndItem,
			   DM_ENDCONVERSATION,
			   MPFROMLONG(pDItem->ulItemID),
			   MPFROMLONG(DMFL_TARGETFAIL));
	numfail++;
	continue;
      }
    }

    if (DrgVerifyRMF(pDItem,
		     DRM_OS2FILE,
		     NULL) ||
	(arcfilesok &&
	 ((arctest = DrgVerifyRMF(pDItem,
				  DRM_FM2ARCMEMBER,
				  DRF_FM2ARCHIVE)) != FALSE))) {
      if (pDItem->fsControl & DC_PREPARE) {
	DrgSendTransferMsg(pDItem->hwndItem,
			   DM_ENDCONVERSATION,
			   MPFROMLONG(pDItem->ulItemID),
			   MPFROMLONG(DMFL_TARGETFAIL));
	numfail++;
	continue;
      }

      if (arctest || isArc) {
	if (!isArc) {
	  if (TwoDrgNames(pDItem,
			  szArc,
			  sizeof(szArc),
			  szFrom, sizeof(szFrom)) && *szArc && *szFrom) {
	    isArc = TRUE;
	    arcinfo = find_type(szArc, arcsighead);
	  }
	  if (!arcinfo || !arcinfo->extract || !*arcinfo->extract) {
	    *szArc = *szFrom = 0;
	    isArc = FALSE;
	  }
	}
	else
	  TwoDrgNames(pDItem, NULL, 0, szFrom, sizeof(szFrom));
      }
      else
	FullDrgName(pDItem, szFrom, sizeof(szFrom));

      if (!*szFrom) {
	DrgSendTransferMsg(pDItem->hwndItem,
			   DM_ENDCONVERSATION,
			   MPFROMLONG(pDItem->ulItemID),
			   MPFROMLONG(DMFL_TARGETFAIL));
	numfail++;
	continue;
      }

      if (numfiles + 2 > numalloc) {

	CHAR **test;
        PULONG pltest;
        PULONGLONG plltest;

	numalloc += 12;
	test =
	  xrealloc(files, numalloc * sizeof(CHAR *), pszSrcFile, __LINE__);
	if (!test)
	  break;
	files = test;
	plltest =
	  xrealloc(pcbFile, numalloc * sizeof(ULONGLONG), pszSrcFile, __LINE__);
	if (!plltest)
	  break;
	pcbFile = plltest;
	pltest =
	  xrealloc(pulitemID, numalloc * sizeof(ULONG), pszSrcFile, __LINE__);
	if (!pltest)
	  break;
	pulitemID = pltest;
      }
      pcbFile[numfiles] = 0;
      if (!isArc) {

	FILESTATUS4L fsa4;

	if (!DosQueryPathInfo(szFrom, FIL_QUERYEASIZEL, &fsa4, sizeof(fsa4)))
	  pcbFile[numfiles] = fsa4.cbFile + CBLIST_TO_EASIZE(fsa4.cbList);
      }
      pulitemID[numfiles] = pDItem->ulItemID;
      files[numfiles] = xstrdup(szFrom, pszSrcFile, __LINE__);
      files[numfiles + 1] = NULL;
      if (!files[numfiles])
	break;
      numfiles++;
      DrgSendTransferMsg(pDItem->hwndItem,
			 DM_ENDCONVERSATION,
			 MPFROMLONG(pDItem->ulItemID),
			 MPFROMLONG(DMFL_TARGETSUCCESSFUL));
      numok++;
    }
    else {
      DrgSendTransferMsg(pDItem->hwndItem,
			 DM_ENDCONVERSATION,
			 MPFROMLONG(pDItem->ulItemID),
			 MPFROMLONG(DMFL_TARGETFAIL));
      numfail++;
    }
  } // for curitem

  if (files && numfiles && files[0] && pcbFile && pulitemID) {
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
    li = xmallocz(sizeof(LISTINFO), pszSrcFile, __LINE__);
    if (li) {
      li->type = Operation;
      li->hwnd = hwndCnr;
      li->list = files;
      li->cbFile = pcbFile;
      li->ulitemID = pulitemID;
      li->hwndS = pDInfo->hwndSource;
      if (!pci && directory)
	strcpy(li->targetpath, directory);
      else if (pci)
	strcpy(li->targetpath, pci->pszFileName);
      if (isArc) {
	strcpy(li->arcname, szArc);
	li->info = arcinfo;
      }
      SortList(li);
    }
    else {
      xfree(pcbFile, pszSrcFile, __LINE__);
      xfree(pulitemID, pszSrcFile, __LINE__);
      if (files)
	FreeList(files);
    }
  }
  else {
    xfree(pcbFile, pszSrcFile, __LINE__);
    xfree(pulitemID, pszSrcFile, __LINE__);
    if (files)
      FreeList(files);
  }

  FreeDragInfoData(hwndCnr, pDInfo);
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif

  // 02 Aug 07 SHL fixme to be gone someday or use Runtime_Error if really an error
  if (numfail || numok == 0)
    DbgMsg(pszSrcFile, __LINE__, "calling FreeDragInfoData with %u ok, %u failed", numok, numfail);

  return li;
}

#pragma alloc_text(DROPLIST,DoFileDrop,FullDrgName,TwoDrgNames,GetOneDrop,CheckPmDrgLimit)
