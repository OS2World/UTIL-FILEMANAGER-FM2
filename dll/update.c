
/***********************************************************************

  $Id$

  Update Container record/list

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2008 Steven H. Levine

  12 Feb 03 SHL Standardize EA math
  10 Jan 04 SHL Add some intermin large drive error avoidance
  25 May 05 SHL Rework for ULONGLONG
  25 May 05 SHL Rework for FillInRecordFromFFB
  06 Jun 05 SHL Drop unused code
  22 Jul 06 SHL Use wrappers
  20 Feb 07 GKY Add SelectDriveIcon()
  09 Mar 07 GKY Cleanup SelectDriveIcon using "driveflag =" from Steven
  02 Aug 07 SHL Sync with CNRITEM mods
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "filldir.h"			// FillInRecordFromFFB
#include "dircnrs.h"
#include "fm3dll.h"
#include "fortify.h"

static PSZ pszSrcFile = __FILE__;

HPOINTER SelectDriveIcon(PCNRITEM pci)
{
    UINT driveflag = driveflags[toupper(*pci->pszFileName) - 'A'];
    *pci->pszFileName = toupper(*pci->pszFileName);
	      if (isalpha(*pci->pszFileName) &&
		  toupper(*pci->pszFileName) > 'B') {
		if (driveflag & DRIVE_CDROM)
		  pci->rc.hptrIcon = hptrCDROM;
		else
		  pci->rc.hptrIcon =
		     (driveflag & DRIVE_REMOVABLE) ? hptrRemovable
		    :(driveflag & DRIVE_VIRTUAL) ? hptrVirtual
		    :(driveflag & DRIVE_REMOTE) ? hptrRemote
		    :(driveflag & DRIVE_RAMDISK) ? hptrRamdisk
		    :(driveflag & DRIVE_ZIPSTREAM) ? hptrZipstrm : hptrDrive;
	      }
	      else
		pci->rc.hptrIcon = hptrFloppy;
		return pci->rc.hptrIcon;
}
PCNRITEM UpdateCnrRecord(HWND hwndCnr, CHAR * filename, BOOL partial,
			 DIRCNRDATA * dcd)
{
  PCNRITEM pci;
  FILEFINDBUF4L ffb;
  HDIR hDir = HDIR_CREATE;
  ULONG nm = 1;
  ULONG oldemphasis = 0;
  APIRET status;

#ifdef DEBUG
  BOOL existed = FALSE, updated = FALSE, added = FALSE, deleted =
    FALSE, found = FALSE;
#endif

  if (!filename || !*filename)
    return (PCNRITEM) NULL;
  if (IsFullName(filename)) {
    if (driveflags[toupper(*filename) - 'A'] & DRIVE_NOTWRITEABLE)
      /* ignore non-writeable drives */
      return (PCNRITEM) NULL;
  }
  status = xDosFindFirst(filename,
			 &hDir,
			 FILE_NORMAL | FILE_DIRECTORY |
			 FILE_ARCHIVED | FILE_READONLY |
			 FILE_HIDDEN | FILE_SYSTEM,
			 &ffb, sizeof(ffb), &nm, FIL_QUERYEASIZEL);
  if (!status) {
#ifdef DEBUG
    existed = TRUE;
#endif
    /* file exists */
    DosFindClose(hDir);
    if (!dcd)
      dcd = INSTDATA(hwndCnr);
/*
    if(dcd->type == TREE_FRAME &&
       !(ffb.attrFile & FILE_DIRECTORY))
      return (PCNRITEM)NULL;
*/
    if (dcd->type == ARC_FRAME)
      return (PCNRITEM) NULL;
    if (*dcd->directory) {

      CHAR *p, temp;

      p = strrchr(filename, '\\');
      if (p) {
	if (p < filename + 3)
	  p++;
	temp = *p;
	*p = 0;
	if (stricmp(filename, dcd->directory)) {
	  *p = temp;
	  return (PCNRITEM) NULL;
	}
	*p = temp;
      }
      else
	return (PCNRITEM) NULL;
    }
    pci = FindCnrRecord(hwndCnr,
			filename, (PCNRITEM) NULL, partial, FALSE, TRUE);
  Update:
    if (pci) {				/* update record? */
#ifdef DEBUG
      found = TRUE;
#endif
      if ((!fForceUpper && !fForceLower && strcmp(pci->pszFileName, filename)) ||
	  pci->cbFile != ffb.cbFile || pci->attrFile != ffb.attrFile ||
	  pci->easize != CBLIST_TO_EASIZE(ffb.cbList) || pci->date.day !=
	  ffb.fdateLastWrite.day || pci->date.month != ffb.fdateLastWrite.month ||
	  pci->date.year != ffb.fdateLastWrite.year + 1980 || pci->time.seconds !=
	  ffb.ftimeLastWrite.twosecs * 2 || pci->time.minutes != ffb.ftimeLastWrite.minutes ||
	  pci->time.hours != ffb.ftimeLastWrite.hours || pci->ladate.day !=
	  ffb.fdateLastAccess.day || pci->ladate.month != ffb.fdateLastAccess.month ||
	  pci->ladate.year != ffb.fdateLastAccess.year + 1980 || pci->latime.seconds !=
	  ffb.ftimeLastAccess.twosecs * 2 || pci->latime.minutes !=
	  ffb.ftimeLastAccess.minutes || pci->latime.hours != ffb.ftimeLastAccess.hours) {	/* changed; update */
#ifdef DEBUG
	updated = TRUE;
#endif
	*ffb.achName = 0;
	ffb.cchName = 0;
	FillInRecordFromFFB(hwndCnr, pci, filename, &ffb, partial, dcd);
	if (strlen(pci->pszFileName) < 4)
	SelectDriveIcon(pci);
	oldemphasis = pci->rc.flRecordAttr & (CRA_SELECTED | CRA_CURSORED);
	if (oldemphasis)
	  WinSendMsg(hwndCnr,
		     CM_SETRECORDEMPHASIS,
		     MPFROMP(pci), MPFROM2SHORT(FALSE, oldemphasis));
	WinSendMsg(hwndCnr,
		   CM_INVALIDATERECORD, MPFROMP(&pci), MPFROM2SHORT(1,
								    /* CMA_ERASE | */
								    CMA_TEXTCHANGED));
	if (oldemphasis)
	  WinSendMsg(hwndCnr,
		     CM_SETRECORDEMPHASIS,
		     MPFROMP(pci), MPFROM2SHORT(TRUE, oldemphasis));
      }
      else				/* existed, unchanged, do nothing but return */
	return pci;
    }
    else {				/* add record */
#ifdef DEBUG
      added = TRUE;
#endif
      if (dcd->type == DIR_FRAME) {

	RECORDINSERT ri;
	ULONGLONG ullTotalBytes;

	pci = WinSendMsg(hwndCnr,
			 CM_ALLOCRECORD,
			 MPFROMLONG(EXTRA_RECORD_BYTES), MPFROMLONG(1));
	if (pci) {
	  *ffb.achName = 0;
	  ullTotalBytes = FillInRecordFromFFB(hwndCnr,
					      pci,
					      filename, &ffb, partial, dcd);
	  if (strlen(pci->pszFileName) < 4)
	  SelectDriveIcon(pci);
	  memset(&ri, 0, sizeof(RECORDINSERT));
	  ri.cb = sizeof(RECORDINSERT);
	  ri.pRecordOrder = (PRECORDCORE) CMA_END;
	  ri.pRecordParent = (PRECORDCORE) NULL;
	  ri.zOrder = (USHORT) CMA_TOP;
	  ri.cRecordsInsert = 1;
	  ri.fInvalidateRecord = TRUE;
	  if (WinSendMsg(hwndCnr,
			 CM_INSERTRECORD,
			 MPFROMP(pci), MPFROMP(&ri)) && ullTotalBytes) {
	    dcd->ullTotalBytes += ullTotalBytes;
	    PostMsg(hwndCnr, UM_RESCAN, MPVOID, MPVOID);
	    if (pci->attrFile & FILE_DIRECTORY)
	      Stubby(hwndCnr, pci);
	  }
	}
      }
      else if (ffb.attrFile & FILE_DIRECTORY) {

	/* check all parts and insert as required */
	CHAR *p, temp;
	PCNRITEM pciParent = NULL, pciT;

	p = strchr(filename, '\\');
	if (p) {
	  while (p && *p) {
	    if (p < filename + 3)
	      p++;
	    temp = *p;
	    *p = 0;
	    pciT = FindCnrRecord(hwndCnr,
				 filename, NULL, partial, FALSE, TRUE);
	    if (!pciT || (INT) pciT == -1) {
	      pci = WinSendMsg(hwndCnr,
			       CM_ALLOCRECORD,
			       MPFROMLONG(EXTRA_RECORD_BYTES),
			       MPFROMLONG(1));
	      if (pci) {

		RECORDINSERT ri;

		*ffb.achName = 0;
		FillInRecordFromFFB(hwndCnr,
				    pci, filename, &ffb, partial, dcd);
		if (strlen(pci->pszFileName) < 4)
		SelectDriveIcon(pci);
		memset(&ri, 0, sizeof(RECORDINSERT));
		ri.cb = sizeof(RECORDINSERT);
		ri.pRecordOrder = (PRECORDCORE) CMA_END;
		ri.pRecordParent = (PRECORDCORE) pciParent;
		ri.zOrder = (USHORT) CMA_TOP;
		ri.cRecordsInsert = 1;
		ri.fInvalidateRecord = TRUE;
		if (WinSendMsg(hwndCnr,
			       CM_INSERTRECORD, MPFROMP(pci), MPFROMP(&ri))) {
		  Flesh(hwndCnr, pci);
		  *p = temp;
		  pci = FindCnrRecord(hwndCnr,
				      filename, pciT, partial, FALSE, TRUE);
		  if (pci)
		    goto Update;
		}
	      }
	    }
	    else {
	      pciParent = pciT;
	      if (!(pciT->rc.flRecordAttr & CRA_EXPANDED)) {
		Flesh(hwndCnr, pciT);
		*p = temp;
		pci = FindCnrRecord(hwndCnr,
				    filename, pciT, partial, FALSE, TRUE);
		if (pci)
		  goto Update;
	      }
	    }
	    *p = temp;
	    p = strchr(p + ((temp == '\\') ? 1 : 0), '\\');
	  }
	}
	pci = WinSendMsg(hwndCnr,
			 CM_ALLOCRECORD,
			 MPFROMLONG(EXTRA_RECORD_BYTES), MPFROMLONG(1));
	if (pci) {

	  RECORDINSERT ri;
	  ULONGLONG ullTotalBytes;

	  *ffb.achName = 0;
	  ullTotalBytes = FillInRecordFromFFB(hwndCnr,
					      pci,
					      filename, &ffb, partial, dcd);
	  if (strlen(pci->pszFileName) < 4)
	  SelectDriveIcon(pci);
	  memset(&ri, 0, sizeof(RECORDINSERT));
	  ri.cb = sizeof(RECORDINSERT);
	  ri.pRecordOrder = (PRECORDCORE) CMA_END;
	  ri.pRecordParent = (PRECORDCORE) pciParent;
	  ri.zOrder = (USHORT) CMA_TOP;
	  ri.cRecordsInsert = 1;
	  ri.fInvalidateRecord = TRUE;
	  if (WinSendMsg(hwndCnr,
			 CM_INSERTRECORD,
			 MPFROMP(pci), MPFROMP(&ri)) && ullTotalBytes) {
	    if (dcd->type == DIR_FRAME) {
	      dcd->ullTotalBytes += ullTotalBytes;
	    }
	    Stubby(hwndCnr, pci);
	  }
	}
      }
    }
  }
  else if ((pci = FindCnrRecord(hwndCnr,
				filename,
				(PCNRITEM) NULL,
				partial,
				FALSE,
				TRUE)) !=
	   NULL && (INT) pci != -1 && strlen(pci->pszFileName) > 3) {
    /* file doesn't exist; delete record */
#ifdef DEBUG
    found = TRUE;
    deleted = TRUE;
#endif
    if (!dcd)
      dcd = INSTDATA(hwndCnr);
    if (pci->rc.flRecordAttr & CRA_SELECTED)
      WinSendMsg(hwndCnr,
		 CM_SETRECORDEMPHASIS,
		 MPFROMP(pci), MPFROM2SHORT(FALSE, CRA_SELECTED));
    if (dcd->type == DIR_FRAME)
      dcd->ullTotalBytes -= pci->cbFile + pci->easize;
    RemoveCnrItems(hwndCnr, pci, 1, CMA_FREE | CMA_INVALIDATE);
    pci = NULL;
    PostMsg(hwndCnr, UM_RESCAN, MPVOID, MPVOID);
  }
#ifdef DEBUG
  {
    char s[CCHMAXPATH + 80];

    sprintf(s, "%s:%s%s%s%s%s", filename, (existed) ? " Existed" : "",
	    (updated) ? " Updated" : "", (added) ? " Added" : "",
	    (deleted) ? " Deleted" : "", (found) ? " Found" : "");
    WinSetWindowText(WinQueryWindow(hwndMain, QW_PARENT), s);
  }
#endif
  return pci;
}

BOOL UpdateCnrList(HWND hwndCnr, CHAR ** filename, INT howmany, BOOL partial,
		   DIRCNRDATA * dcd)
{
  PCNRITEM pci, *pciList = NULL;
  FILEFINDBUF4L ffb;
  HDIR hDir;
  ULONG nm = (ULONG) howmany;
  INT x;
  INT numlist = 0;
  INT numremain;
  BOOL repos = FALSE;
  BOOL ret = FALSE;
  APIRET status;

  if (!dcd)
    dcd = INSTDATA(hwndCnr);
  if (!dcd) {
    Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
    return ret;
  }
  if (!filename || !howmany || !filename[0])
    return ret;
  {
    CNRINFO cnri;

    memset(&cnri, 0, sizeof(CNRINFO));
    cnri.cb = sizeof(CNRINFO);
    WinSendMsg(hwndCnr,
	       CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
    numremain = cnri.cRecords;
  }
  pciList = xmalloc(sizeof(PCNRITEM) * howmany, pszSrcFile, __LINE__);
  if (pciList) {
    for (x = 0; filename[x] && x < howmany; x++) {
      if (IsFullName(filename[x])) {
	if (driveflags[toupper(*filename[x]) - 'A'] & DRIVE_NOTWRITEABLE)
	  /* ignore non-writeable drives */
	  continue;
      }
      hDir = HDIR_CREATE;
      status = xDosFindFirst(filename[x],
			     &hDir,
			     FILE_NORMAL | FILE_DIRECTORY |
			     FILE_ARCHIVED | FILE_READONLY |
			     FILE_HIDDEN | FILE_SYSTEM,
			     &ffb, sizeof(ffb), &nm, FIL_QUERYEASIZEL);
      if (!status) {
	/* file exists */
	DosFindClose(hDir);
	//      if(dcd->type == TREE_FRAME && !(ffb.attrFile & FILE_DIRECTORY))
	//        continue;
	if (dcd->type == DIR_FRAME && *dcd->directory) {

	  CHAR *p, temp;

	  p = strrchr(filename[x], '\\');
	  if (p) {
	    if (p < filename[x] + 3)
	      p++;
	    temp = *p;
	    *p = 0;
	    if (stricmp(filename[x], dcd->directory)) {
	      *p = temp;
	      continue;
	    }
	    *p = temp;
	  }
	  else
	    continue;
	}
	ret = TRUE;
	pci = FindCnrRecord(hwndCnr,
			    filename[x],
			    (PCNRITEM) NULL, partial, FALSE, TRUE);
	if (pci) {
	  /* update record? */
	  if ((!fForceUpper && !fForceLower &&
	       strcmp(pci->pszFileName, filename[x])) ||
	      pci->cbFile != ffb.cbFile || pci->attrFile != ffb.attrFile ||
	      pci->easize != CBLIST_TO_EASIZE(ffb.cbList) ||
	      pci->date.day != ffb.fdateLastWrite.day ||
	      pci->date.month != ffb.fdateLastWrite.month ||
	      pci->date.year != ffb.fdateLastWrite.year + 1980 ||
	      pci->time.seconds != ffb.ftimeLastWrite.twosecs * 2 ||
	      pci->time.minutes != ffb.ftimeLastWrite.minutes ||
	      pci->time.hours != ffb.ftimeLastWrite.hours ||
	      pci->ladate.day != ffb.fdateLastAccess.day ||
	      pci->ladate.month != ffb.fdateLastAccess.month ||
	      pci->ladate.year != ffb.fdateLastAccess.year + 1980 ||
	      pci->latime.seconds != ffb.ftimeLastAccess.twosecs * 2 ||
	      pci->latime.minutes != ffb.ftimeLastAccess.minutes ||
	      pci->latime.hours != ffb.ftimeLastAccess.hours) {
	    /* changed; update */
	    pciList[numlist++] = pci;
	    *ffb.achName = 0;
	    ffb.cchName = 0;
	    FillInRecordFromFFB(hwndCnr,
				pci, filename[x], &ffb, partial, dcd);
	    if (IsRoot(pci->pszFileName))
	    SelectDriveIcon(pci);
	    WinSendMsg(hwndCnr,
		       CM_SETRECORDEMPHASIS,
		       MPFROMP(pci),
		       MPFROM2SHORT(FALSE, CRA_SELECTED | CRA_CURSORED));
	  }
	}
	else {
	  /* add record */
	  if (dcd->type == DIR_FRAME) {
	    RECORDINSERT ri;
	    ULONGLONG ullTotalBytes;

	    pci = WinSendMsg(hwndCnr,
			     CM_ALLOCRECORD,
			     MPFROMLONG(EXTRA_RECORD_BYTES), MPFROMLONG(1));
	    if (pci) {
	      ret = TRUE;
	      *ffb.achName = 0;
	      ullTotalBytes = FillInRecordFromFFB(hwndCnr,
						  pci,
						  filename[x],
						  &ffb, partial, dcd);
	      if (strlen(pci->pszFileName) < 4)
	        SelectDriveIcon(pci);
	      memset(&ri, 0, sizeof(RECORDINSERT));
	      ri.cb = sizeof(RECORDINSERT);
	      ri.pRecordOrder = (PRECORDCORE) CMA_END;
	      ri.pRecordParent = (PRECORDCORE) NULL;
	      ri.zOrder = (USHORT) CMA_TOP;
	      ri.cRecordsInsert = 1;
	      ri.fInvalidateRecord = FALSE;
	      if (WinSendMsg(hwndCnr,
			     CM_INSERTRECORD, MPFROMP(pci), MPFROMP(&ri))) {
		if (ullTotalBytes) {
		  dcd->ullTotalBytes += ullTotalBytes;
		  numremain++;
		}
		repos = TRUE;
		if (pci->attrFile & FILE_DIRECTORY)
		  Stubby(hwndCnr, pci);
	      }
	      else
		FreeCnrItem(hwndCnr, pci);
	    }
	  }
	  else if (ffb.attrFile & FILE_DIRECTORY) {
	    /* check all parts and insert as required */
	    CHAR *p, temp;
	    PCNRITEM pciParent = NULL, pciT;

	    p = strchr(filename[x], '\\');
	    if (p) {
	      while (p && *p) {
		if (p < filename[x] + 3)
		  p++;
		temp = *p;
		*p = 0;
		pciT = FindCnrRecord(hwndCnr,
				     filename[x], NULL, partial, FALSE, TRUE);
		if (!pciT || (INT) pciT == -1) {
		  pci = WinSendMsg(hwndCnr,
				   CM_ALLOCRECORD,
				   MPFROMLONG(EXTRA_RECORD_BYTES),
				   MPFROMLONG(1));
		  if (pci) {

		    RECORDINSERT ri;
		    ULONGLONG ullTotalBytes;

		    ret = TRUE;
		    *ffb.achName = 0;
		    ullTotalBytes = FillInRecordFromFFB(hwndCnr,
							pci,
							filename[x],
							&ffb, partial, dcd);
		    if (strlen(pci->pszFileName) < 4)
		      SelectDriveIcon(pci);
		    memset(&ri, 0, sizeof(RECORDINSERT));
		    ri.cb = sizeof(RECORDINSERT);
		    ri.pRecordOrder = (PRECORDCORE) CMA_END;
		    ri.pRecordParent = (PRECORDCORE) pciParent;
		    ri.zOrder = (USHORT) CMA_TOP;
		    ri.cRecordsInsert = 1;
		    ri.fInvalidateRecord = FALSE;
		    if (WinSendMsg(hwndCnr,
				   CM_INSERTRECORD,
				   MPFROMP(pci), MPFROMP(&ri))) {
		      if (ullTotalBytes) {
			numremain++;
			if (dcd->type == DIR_FRAME)
			  dcd->ullTotalBytes += ullTotalBytes;
		      }
		      repos = TRUE;
		    }
		    else
		      FreeCnrItem(hwndCnr, pci);
		  }
		}
		else
		  pciParent = pciT;
		*p = temp;
		p = strchr(p + ((temp == '\\') ? 1 : 0), '\\');
	      }
	    }
	    {
	      pci = WinSendMsg(hwndCnr,
			       CM_ALLOCRECORD,
			       MPFROMLONG(EXTRA_RECORD_BYTES),
			       MPFROMLONG(1));
	      if (pci) {

		RECORDINSERT ri;
		ULONGLONG ullTotalBytes;

		ret = TRUE;
		*ffb.achName = 0;
		ullTotalBytes = FillInRecordFromFFB(hwndCnr,
						    pci,
						    filename[x],
						    &ffb, partial, dcd);
		if (strlen(pci->pszFileName) < 4)
		  SelectDriveIcon(pci);
		memset(&ri, 0, sizeof(RECORDINSERT));
		ri.cb = sizeof(RECORDINSERT);
		ri.pRecordOrder = (PRECORDCORE) CMA_END;
		ri.pRecordParent = (PRECORDCORE) pciParent;
		ri.zOrder = (USHORT) CMA_TOP;
		ri.cRecordsInsert = 1;
		ri.fInvalidateRecord = FALSE;
		if (WinSendMsg(hwndCnr,
			       CM_INSERTRECORD, MPFROMP(pci), MPFROMP(&ri))) {
		  if (ullTotalBytes) {
		    numremain++;
		    if (dcd->type == DIR_FRAME)
		      dcd->ullTotalBytes += ullTotalBytes;
		  }
		  repos = TRUE;
		  Stubby(hwndCnr, pci);
		}
		else
		  FreeCnrItem(hwndCnr, pci);
	      }
	    }
	  }
	}
      }
      else if ((pci = FindCnrRecord(hwndCnr,
				    filename[x],
				    (PCNRITEM) NULL,
				    partial,
				    FALSE,
				    TRUE)) != NULL &&
	       (INT) pci != -1 && !IsRoot(pci->pszFileName)) {
	/* file doesn't exist; delete record */
	if (pci->rc.flRecordAttr & CRA_SELECTED)
	  WinSendMsg(hwndCnr,
		     CM_SETRECORDEMPHASIS,
		     MPFROMP(pci), MPFROM2SHORT(FALSE, CRA_SELECTED));
	if (dcd->type == DIR_FRAME)
	  dcd->ullTotalBytes -= (pci->cbFile + pci->easize);
	// 02 Aug 07 SHL rc check was wrong
	if (RemoveCnrItems(hwndCnr, pci, 1,
			   CMA_FREE |
			     numremain == 1 ? CMA_INVALIDATE : 0) != -1) {
	  numremain--;
	  repos = TRUE;
	}
      }
    } // for x
  }
  if (repos || (pciList && numlist)) {
    QUERYRECORDRECT qrr;
    RECTL rCnr, rCItem;

    pci = WinSendMsg(hwndCnr,
		     CM_QUERYRECORDEMPHASIS,
		     MPFROMLONG(CMA_FIRST), MPFROMLONG(CRA_CURSORED));
    if (pci && (INT) pci != -1) {
      memset(&qrr, 0, sizeof(QUERYRECORDRECT));
      qrr.cb = sizeof(QUERYRECORDRECT);
      qrr.pRecord = (PRECORDCORE) pci;
      qrr.fRightSplitWindow = FALSE;
      qrr.fsExtent = CMA_TEXT;
      if (!WinSendMsg(hwndCnr,
		      CM_QUERYRECORDRECT, MPFROMP(&rCItem), MPFROMP(&qrr)))
	qrr.cb = 0;
    }
    if (pciList && numlist && !repos) {
      WinSendMsg(hwndCnr,
		 CM_INVALIDATERECORD,
		 MPFROMP(pciList),
		 MPFROM2SHORT(numlist,
			      (repos ? CMA_NOREPOSITION :
			       CMA_REPOSITION | CMA_ERASE)));
    }
    if (repos)
      WinSendMsg(hwndCnr,
		 CM_INVALIDATERECORD,
		 MPVOID, MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
    if (pci && (INT) pci != -1 && qrr.cb) {
      WinSendMsg(hwndCnr,
		 CM_QUERYVIEWPORTRECT,
		 MPFROMP(&rCnr), MPFROM2SHORT(CMA_WINDOW, (SHORT) FALSE));
      WinSendMsg(hwndCnr,
		 CM_SCROLLWINDOW,
		 MPFROMSHORT(CMA_VERTICAL),
		 MPFROMLONG(rCnr.yTop - rCItem.yTop));
    }
  }
  PostMsg(hwndCnr, UM_RESCAN, MPVOID, MPVOID);
  if (pciList) {
    free(pciList);
    DosPostEventSem(CompactSem);
  }
  return ret;
}

#pragma alloc_text(UPDATECNR,UpdateCnrRecord,UpdateCnrList)
