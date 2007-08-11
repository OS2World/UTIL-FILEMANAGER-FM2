
/***********************************************************************

  $Id$

  Fill Directory Tree Containers

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2007 Steven H. Levine

  10 Jan 04 SHL ProcessDirectory: avoid most large drive failures
  24 May 05 SHL Rework Win_Error usage
  24 May 05 SHL Rework for CNRITEM.szSubject
  25 May 05 SHL Rework for ULONGLONG
  25 May 05 SHL Rework FillInRecordFromFFB
  25 May 05 SHL Rework FillTreeCnr
  28 May 05 SHL Drop stale debug code
  05 Jun 05 SHL Comments
  09 Jun 05 SHL Rework WinLoadFileIcon enables
  09 Jun 05 SHL Rework IDFile
  13 Aug 05 SHL Renames
  24 Oct 05 SHL FillInRecordFromFFB: correct longname display enable
  24 Oct 05 SHL FillInRecordFromFSA: correct longname display enable
  24 Oct 05 SHL Drop obsolete code
  22 Jul 06 SHL Check more run time errors
  20 Oct 06 SHL Sync . .. check code
  22 Oct 06 GKY Add NDFS32 support
  17 Feb 07 GKY Additional archive and image file tyoes identifed by extension
  17 Feb 07 GKY Add more drive types
  09 Mar 07 GKY Use SelectDriveIcon
  20 Mar 07 GKY Increase extention check to 4 letters for icon selections
  23 Jun 07 GKY Fixed ram disk without a directory not appearing on states drive list
  23 Jul 07 SHL Sync with CNRITEM updates (ticket#24)
  29 Jul 07 SHL Add CNRITEM free and remove support (ticket#24)
  02 Aug 07 SHL Add FileAttrToString
  03 Aug 07 GKY Enlarged and made setable everywhere Findbuf (speed file loading)
  04 Aug 07 SHL Update #pragma alloc_test for new functions
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG
#include <os2.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <time.h>

#if 1 // fixme to disable or to be configurable
#include <malloc.h>			// _heapchk
#endif

#include "fm3dll.h"
#include "fm3str.h"

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(FILLDIR,FillInRecordFromFFB,FillInRecordFromFSA,IDFile)
#pragma alloc_text(FILLDIR1,ProcessDirectory,FillDirCnr,FillTreeCnr,FileAttrToString)
#pragma alloc_text(EMPTYCNR,EmptyCnr,FreeCnrItemData,FreeCnrItem,FreeCnrItemList,RemoveCnrItems)

/**
 * Return display string given standard file attribute mask
 * @param fileAttr attribute mask in FILEFINDBUF format
 * @return fixed length string for display
 */

const PSZ FileAttrToString(ULONG fileAttr)
{
  // From os2win.h
  // FILE_ATTRIBUTE_READONLY	0x00000001
  // FILE_ATTRIBUTE_HIDDEN	0x00000002
  // FILE_ATTRIBUTE_SYSTEM	0x00000004
  //				0x00000008
  // FILE_ATTRIBUTE_DIRECTORY	0x00000010
  // FILE_ATTRIBUTE_ARCHIVE	0x00000020

  static CHAR *apszAttrString[] = {
  // RHSDA
    "-----",
    "R----",
    "-H---",
    "RH---",
    "--S--",
    "R-S--",
    "-HS--",
    "RHS--",
    "---D-",
    "R--D-",
    "-H-D-",
    "RH-D-",
    "--SD-",
    "R-SD-",
    "-HSD-",
    "RHSD-",
    "----A",
    "R---A",
    "-H--A",
    "RH--A",
    "--S-A",
    "R-S-A",
    "-HS-A",
    "RHS-A",
    "---DA",
    "R--DA",
    "-H-DA",
    "RH-DA",
    "--SDA",
    "R-SDA",
    "-HSDA",
    "RHSDA"
  };

  fileAttr = ((fileAttr & 0x30) >> 1) | (fileAttr & 7);	// Drop don't care bit from index

  return apszAttrString[fileAttr];

}

static HPOINTER IDFile(PSZ p)
{
  HPOINTER hptr;
  ULONG cmp;
  CHAR cmps[5];

  p = strrchr(p, '.');
  if (p && !p[5]) {
    cmps[0] = '.';
    cmps[1] = toupper(p[1]);
    cmps[2] = toupper(p[2]);
    cmps[3] = toupper(p[3]);
    cmps[4] = toupper(p[4]);

    cmp = *(ULONG *) cmps;

    if (cmp == *(ULONG *) ".EXE" || cmp == *(ULONG *) ".CMD" ||
	cmp == *(ULONG *) ".BAT" || cmp == *(ULONG *) ".COM")
      hptr = hptrApp;
    else if (cmp == *(ULONG *) ".ZIP" || cmp == *(ULONG *) ".LZH" ||
	     cmp == *(ULONG *) ".ARJ" || cmp == *(ULONG *) ".ARC" ||
	     cmp == *(ULONG *) ".ZOO" || cmp == *(ULONG *) ".RAR" ||
	     cmp == *(ULONG *) ".TAR" || cmp == *(ULONG *) ".TGZ" ||
	     cmp == *(ULONG *) ".GZ" || cmp == *(ULONG *) ".Z" ||
	     cmp == *(ULONG *) ".CAB" || cmp == *(ULONG *) ".BZ2")
      hptr = hptrArc;
    else if (cmp == *(ULONG *) ".BMP" || cmp == *(ULONG *) ".ICO" ||
	     cmp == *(ULONG *) ".PTR" || cmp == *(ULONG *) ".GIF" ||
	     cmp == *(ULONG *) ".TIF" || cmp == *(ULONG *) ".PCX" ||
	     cmp == *(ULONG *) ".TGA" || cmp == *(ULONG *) ".XBM" ||
	     cmp == *(ULONG *) ".JPEG" || cmp == *(ULONG *) ".JPG" ||
	     cmp == *(ULONG *) ".PNG" || cmp == *(ULONG *) ".PSD" ||
	     cmp == *(ULONG *) ".LGO" || cmp == *(ULONG *) ".EPS" ||
	     cmp == *(ULONG *) ".RLE" || cmp == *(ULONG *) ".RAS" ||
	     cmp == *(ULONG *) ".PLC" || cmp == *(ULONG *) ".MSP" ||
	     cmp == *(ULONG *) ".IFF" || cmp == *(ULONG *) ".FIT" ||
	     cmp == *(ULONG *) ".DCX" || cmp == *(ULONG *) ".MAC" ||
	     cmp == *(ULONG *) ".SFF" || cmp == *(ULONG *) ".SGI" ||
	     cmp == *(ULONG *) ".XWD" || cmp == *(ULONG *) ".XPM" ||
	     cmp == *(ULONG *) ".WPG" || cmp == *(ULONG *) ".CUR" ||
	     cmp == *(ULONG *) ".PNM" || cmp == *(ULONG *) ".PPM" ||
	     cmp == *(ULONG *) ".PGM" || cmp == *(ULONG *) ".PBM")
      hptr = hptrArt;
    else
      hptr = (HPOINTER) 0;
  }
  else
    hptr = (HPOINTER) 0;

  return hptr;
}

static BOOL IsDefaultIcon(HPOINTER hptr)
{
  HPOINTER hptr2;
  HPOINTER hptr3;
  UINT u;

  static HPOINTER hptrPMFile;
  static HPOINTER hptrWPSFile;

  if (!hptrPMFile) {
    hptrPMFile = WinQuerySysPointer(HWND_DESKTOP, SPTR_FILE, FALSE);
  }

  // try to guess WPS default file icon
  hptr2 = (HPOINTER) 0;
  for (u = 0; !hptrWPSFile && u < 10; u++) {
    char szFileName[CCHMAXPATH];
    char *psz;

    psz = getenv("TMP");
    if (!psz && *psz)
      psz = getenv("TEMP");
    if (psz && *psz) {
      strcpy(szFileName, psz);
      psz = szFileName + strlen(szFileName) - 1;
      if (*psz != '\\') {
	psz++;
	*psz++ = '\\';
      }
    }
    else
      psz = szFileName;

    sprintf(psz, "%08x.%03x", rand() & 0xffffffff, rand() & 0xfff);
    if (IsFile(szFileName) != 1) {
      FILE *fp = fopen(szFileName, "w");

      if (fp) {
	fclose(fp);
	hptr3 = WinLoadFileIcon(szFileName, FALSE);
	unlinkf("%s", szFileName);
	if (!hptr2)
	  hptr2 = hptr3;
	else if (hptr3 == hptr3) {
	  hptrWPSFile = hptr3;		// Got same icon twice
	  break;
	}
      }
    }
    DosSleep(rand() % 100);

  } // for

  return hptr == hptrPMFile || hptr == hptrWPSFile;

} // IsDefaultIcon

ULONGLONG FillInRecordFromFFB(HWND hwndCnr,
			      PCNRITEM pci,
			      const PSZ pszDirectory,
			      const PFILEFINDBUF4 pffb,
			      const BOOL partial,
			      DIRCNRDATA *dcd)
{
  /* fill in a container record from a FILEFINDBUF4 structure */

  CHAR *p;
  HPOINTER hptr;

  pci->hwndCnr = hwndCnr;

  /* note that we cheat below, and accept the full pathname in pszDirectory
     if !*pffb->achName.  This speeds up and simplifies processing elsewhere
     (like in update.c)
   */
  if (!*pffb->achName) {
    pci->pszFileName = xstrdup(pszDirectory, pszSrcFile, __LINE__);
    strcpy(pci->pszFileName, pszDirectory);
  }
  else {
    INT c = strlen(pszDirectory);
    INT c2 = pffb->cchName + 1;
    if (pszDirectory[c - 1] != '\\')
      c2++;
    pci->pszFileName = xmalloc(c + c2, pszSrcFile, __LINE__);
    memcpy(pci->pszFileName, pszDirectory, c + 1);
    p = pci->pszFileName + c - 1;
    if (*p != '\\') {
      p++;
      *p = '\\';
    }
    p++;
    memcpy(p, pffb->achName, pffb->cchName + 1);
  }

  /* load the object's Subject, if required */
  pci->pszSubject = NullStr;
  if (pffb->cbList > 4L &&
      dcd && fLoadSubject &&
      (isalpha(*pci->pszFileName) &&
       !(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLOADSUBJS)))
  {
    APIRET rc;
    EAOP2 eaop;
    PGEA2LIST pgealist;
    PFEA2LIST pfealist;
    PGEA2 pgea;
    PFEA2 pfea;
    CHAR *value;

    pgealist = xmallocz(sizeof(GEA2LIST) + 32, pszSrcFile, __LINE__);
    if (pgealist) {
      pgea = &pgealist->list[0];
      strcpy(pgea->szName, SUBJECT);
      pgea->cbName = strlen(pgea->szName);
      pgea->oNextEntryOffset = 0;
      pgealist->cbList = (sizeof(GEA2LIST) + pgea->cbName);
      pfealist = xmallocz(1532, pszSrcFile, __LINE__);
      if (pfealist) {
	pfealist->cbList = 1024;
	eaop.fpGEA2List = pgealist;
	eaop.fpFEA2List = pfealist;
	eaop.oError = 0;
	rc = DosQueryPathInfo(pci->pszFileName, FIL_QUERYEASFROMLIST,
			      (PVOID) & eaop, (ULONG) sizeof(EAOP2));
	if (!rc) {
	  pfea = &eaop.fpFEA2List->list[0];
	  value = pfea->szName + pfea->cbName + 1;
	  value[pfea->cbValue] = 0;
	  if (*(USHORT *) value == EAT_ASCII)
	    pci->pszSubject = xstrdup(value + (sizeof(USHORT) * 2), pszSrcFile, __LINE__);
	}
	free(pfealist);
      }
      free(pgealist);
    }
  }
  if (!pci->pszSubject)
    pci->pszSubject = NullStr;

  /* load the object's longname */
  pci->pszLongName = 0;
  if (fLoadLongnames &&
      dcd &&
      pffb->cbList > 4L &&
      isalpha(*pci->pszFileName) &&
      ~driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLONGNAMES &&
      ~driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLOADLONGS)
  {
    APIRET rc;
    EAOP2 eaop;
    PGEA2LIST pgealist;
    PFEA2LIST pfealist;
    PGEA2 pgea;
    PFEA2 pfea;
    CHAR *value;

    pgealist = xmallocz(sizeof(GEA2LIST) + 32, pszSrcFile, __LINE__);
    if (pgealist) {
      pgea = &pgealist->list[0];
      strcpy(pgea->szName, LONGNAME);
      pgea->cbName = strlen(pgea->szName);
      pgea->oNextEntryOffset = 0;
      pgealist->cbList = (sizeof(GEA2LIST) + pgea->cbName);
      pfealist = xmallocz(1532, pszSrcFile, __LINE__);
      if (pfealist) {
	pfealist->cbList = 1024;
	eaop.fpGEA2List = pgealist;
	eaop.fpFEA2List = pfealist;
	eaop.oError = 0;
	rc = DosQueryPathInfo(pci->pszFileName, FIL_QUERYEASFROMLIST,
			      (PVOID) & eaop, (ULONG) sizeof(EAOP2));
	if (!rc) {
	  pfea = &eaop.fpFEA2List->list[0];
	  value = pfea->szName + pfea->cbName + 1;
	  value[pfea->cbValue] = 0;
	  if (*(USHORT *) value == EAT_ASCII)
	    pci->pszLongName = xstrdup(value + (sizeof(USHORT) * 2), pszSrcFile, __LINE__);
	}
	free(pfealist);
      }
      free(pgealist);
    }
  }
  if (!pci->pszLongName)
    pci->pszLongName = NullStr;

  /* do anything required to case of filename */
  if (fForceUpper)
    strupr(pci->pszFileName);
  else if (fForceLower)
    strlwr(pci->pszFileName);

  /* get an icon to use with it */
  if (pffb->attrFile & FILE_DIRECTORY) {
    // is directory
    if (fNoIconsDirs ||
	(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLOADICONS) ||
	!isalpha(*pci->pszFileName)) {
      hptr = (HPOINTER) 0;
    }
    else
      hptr = WinLoadFileIcon(pci->pszFileName, FALSE);
  }
  else {
    // is file
    if (fNoIconsFiles ||
	(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLOADICONS) ||
	!isalpha(*pci->pszFileName)) {
      hptr = (HPOINTER) 0;
    }
    else
      hptr = WinLoadFileIcon(pci->pszFileName, FALSE);

    if (!hptr || IsDefaultIcon(hptr))
      hptr = IDFile(pci->pszFileName);
  }

  if (!hptr) {
    hptr = pffb->attrFile & FILE_DIRECTORY ?
      hptrDir : pffb->attrFile & FILE_SYSTEM ?
		  hptrSystem :
		    pffb->attrFile & FILE_HIDDEN ?
		    hptrHidden :
		    pffb->attrFile & FILE_READONLY ?
		      hptrReadonly : hptrFile;
  }

  // Tell container what part of pathname to display
  if (partial) {
    p = strrchr(pci->pszFileName, '\\');
    if (!p) {
      p = strrchr(pci->pszFileName, ':');
      if (!p)
	p = pci->pszFileName;
      else
	p++;
    }
    else if ((dcd && dcd->type == TREE_FRAME) ||
	     (!(pffb->attrFile & FILE_DIRECTORY) || !*(p + 1))) {
      p++;
    }
    if (!*p)
      p = pci->pszFileName;
  }
  else
    p = pci->pszFileName;
  pci->pszDisplayName = p;

  /* now fill the darned thing in... */
  pci->date.day = pffb->fdateLastWrite.day;
  pci->date.month = pffb->fdateLastWrite.month;
  pci->date.year = pffb->fdateLastWrite.year + 1980;
  pci->time.seconds = pffb->ftimeLastWrite.twosecs * 2;
  pci->time.minutes = pffb->ftimeLastWrite.minutes;
  pci->time.hours = pffb->ftimeLastWrite.hours;
  pci->ladate.day = pffb->fdateLastAccess.day;
  pci->ladate.month = pffb->fdateLastAccess.month;
  pci->ladate.year = pffb->fdateLastAccess.year + 1980;
  pci->latime.seconds = pffb->ftimeLastAccess.twosecs * 2;
  pci->latime.minutes = pffb->ftimeLastAccess.minutes;
  pci->latime.hours = pffb->ftimeLastAccess.hours;
  pci->crdate.day = pffb->fdateCreation.day;
  pci->crdate.month = pffb->fdateCreation.month;
  pci->crdate.year = pffb->fdateCreation.year + 1980;
  pci->crtime.seconds = pffb->ftimeCreation.twosecs * 2;
  pci->crtime.minutes = pffb->ftimeCreation.minutes;
  pci->crtime.hours = pffb->ftimeCreation.hours;
  pci->easize = CBLIST_TO_EASIZE(pffb->cbList);
  pci->cbFile = pffb->cbFile;
  pci->attrFile = pffb->attrFile;
  pci->pszDispAttr = FileAttrToString(pci->attrFile);
  pci->rc.pszIcon = pci->pszDisplayName;
  pci->rc.hptrIcon = hptr;

  /* check to see if record should be visible */
  if (dcd && (*dcd->mask.szMask || dcd->mask.antiattr ||
	      ((dcd->mask.attrFile &
		(FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY | FILE_ARCHIVED))
	       !=
	       (FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY | FILE_ARCHIVED))))
  {
    if (*dcd->mask.szMask || dcd->mask.antiattr) {
      if (!Filter((PMINIRECORDCORE) pci, (PVOID) & dcd->mask))
	pci->rc.flRecordAttr |= CRA_FILTERED;
    }
    else if ((!(dcd->mask.attrFile & FILE_HIDDEN) &&
	      (pci->attrFile & FILE_HIDDEN)) ||
	     (!(dcd->mask.attrFile & FILE_SYSTEM) &&
	      (pci->attrFile & FILE_SYSTEM)) ||
	     (!(dcd->mask.attrFile & FILE_READONLY) &&
	      (pci->attrFile & FILE_READONLY)) ||
	     (!(dcd->mask.attrFile & FILE_ARCHIVED) &&
	      (pci->attrFile & FILE_ARCHIVED))) {
      pci->rc.flRecordAttr |= CRA_FILTERED;
    }
  }

  return pffb->cbFile + pci->easize;

} // FillInRecordFromFFB

ULONGLONG FillInRecordFromFSA(HWND hwndCnr, PCNRITEM pci, const PSZ pszFileName, const PFILESTATUS4 pfsa4, const BOOL partial, DIRCNRDATA * dcd)	// Optional
{
  HPOINTER hptr;
  CHAR *p;

  /* fill in a container record from a FILESTATUS4 structure */

  pci->hwndCnr = hwndCnr;
  pci->pszFileName = xstrdup(pszFileName, pszSrcFile, __LINE__);
  strcpy(pci->pszFileName, pszFileName);

  /* load the object's Subject, if required */
  pci->pszSubject = NullStr;
  if (pfsa4->cbList > 4L &&
      dcd &&
      fLoadSubject &&
      (!isalpha(*pci->pszFileName) ||
       !(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLOADSUBJS)))
  {
    APIRET rc;
    EAOP2 eaop;
    PGEA2LIST pgealist;
    PFEA2LIST pfealist;
    PGEA2 pgea;
    PFEA2 pfea;
    CHAR *value;

    pgealist = xmallocz(sizeof(GEA2LIST) + 32, pszSrcFile, __LINE__);
    if (pgealist) {
      pgea = &pgealist->list[0];
      strcpy(pgea->szName, SUBJECT);
      pgea->cbName = strlen(pgea->szName);
      pgea->oNextEntryOffset = 0;
      pgealist->cbList = (sizeof(GEA2LIST) + pgea->cbName);
      pfealist = xmallocz(1532, pszSrcFile, __LINE__);
      if (pfealist) {
	pfealist->cbList = 1024;
	eaop.fpGEA2List = pgealist;
	eaop.fpFEA2List = pfealist;
	eaop.oError = 0;
	rc = DosQueryPathInfo(pci->pszFileName, FIL_QUERYEASFROMLIST,
			      (PVOID) & eaop, (ULONG) sizeof(EAOP2));
	if (!rc) {
	  pfea = &eaop.fpFEA2List->list[0];
	  value = pfea->szName + pfea->cbName + 1;
	  value[pfea->cbValue] = 0;
	  if (*(USHORT *) value == EAT_ASCII)
	    pci->pszSubject = xstrdup(value + (sizeof(USHORT) * 2), pszSrcFile, __LINE__);
	}
	free(pfealist);
      }
      free(pgealist);
    }
  }
  if (!pci->pszSubject)
    pci->pszSubject = NullStr;

  pci->pszLongName = 0;
  if (fLoadLongnames &&
      dcd &&
      pfsa4->cbList > 4L &&
      isalpha(*pci->pszFileName) &&
      ~driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLONGNAMES &&
      ~driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLOADLONGS)
  {
    APIRET rc;
    EAOP2 eaop;
    PGEA2LIST pgealist;
    PFEA2LIST pfealist;
    PGEA2 pgea;
    PFEA2 pfea;
    CHAR *value;

    pgealist = xmallocz(sizeof(GEA2LIST) + 32, pszSrcFile, __LINE__);
    if (pgealist) {
      pgea = &pgealist->list[0];
      strcpy(pgea->szName, LONGNAME);
      pgea->cbName = strlen(pgea->szName);
      pgea->oNextEntryOffset = 0;
      pgealist->cbList = (sizeof(GEA2LIST) + pgea->cbName);
      pfealist = xmallocz(1532, pszSrcFile, __LINE__);
      if (pfealist) {
	pfealist->cbList = 1024;
	eaop.fpGEA2List = pgealist;
	eaop.fpFEA2List = pfealist;
	eaop.oError = 0;
	rc = DosQueryPathInfo(pci->pszFileName, FIL_QUERYEASFROMLIST,
			      (PVOID) & eaop, (ULONG) sizeof(EAOP2));
	if (!rc) {
	  pfea = &eaop.fpFEA2List->list[0];
	  value = pfea->szName + pfea->cbName + 1;	// Point at EA value
	  value[pfea->cbValue] = 0;			// Terminate
	  if (*(USHORT *) value == EAT_ASCII) {
	    p = value + sizeof(USHORT) * 2;		// Point at value string
	    pci->pszLongName = xstrdup(p, pszSrcFile, __LINE__);
	  }
	}
	free(pfealist);
      }
      free(pgealist);
    }
  }
  if (!pci->pszLongName)
    pci->pszLongName = NullStr;

  if (fForceUpper)
    strupr(pci->pszFileName);
  else if (fForceLower)
    strlwr(pci->pszFileName);

  if (pfsa4->attrFile & FILE_DIRECTORY) {
    if (fNoIconsDirs ||
	(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLOADICONS) ||
	!isalpha(*pci->pszFileName)) {
      hptr = (HPOINTER) 0;
    }
    else
      hptr = WinLoadFileIcon(pci->pszFileName, FALSE);
  }
  else {
    if (fNoIconsFiles ||
	(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOLOADICONS) ||
	!isalpha(*pci->pszFileName)) {
      hptr = IDFile(pci->pszFileName);
    }
    else
      hptr = WinLoadFileIcon(pci->pszFileName, FALSE);
  }
  if (!hptr) {
    hptr = pfsa4->attrFile & FILE_DIRECTORY ?
      hptrDir :
      pfsa4->attrFile & FILE_SYSTEM ?
      hptrSystem :
      pfsa4->attrFile & FILE_HIDDEN ?
      hptrHidden : pfsa4->attrFile & FILE_READONLY ? hptrReadonly : hptrFile;
  }

  // Tell container what part of pathname to display
  if (partial) {
    p = strrchr(pci->pszFileName, '\\');
    if (!p) {
      p = strrchr(pci->pszFileName, ':');
      if (!p)
	p = pci->pszFileName;
      else
	p++;
    }
    else if ((dcd && dcd->type == TREE_FRAME) ||
	     !(pfsa4->attrFile & FILE_DIRECTORY) || !*(p + 1))
      p++;
    if (!*p)
      p = pci->pszFileName;
  }
  else
    p = pci->pszFileName;
  pci->pszDisplayName = p;

  pci->date.day = pfsa4->fdateLastWrite.day;
  pci->date.month = pfsa4->fdateLastWrite.month;
  pci->date.year = pfsa4->fdateLastWrite.year + 1980;
  pci->time.seconds = pfsa4->ftimeLastWrite.twosecs * 2;
  pci->time.minutes = pfsa4->ftimeLastWrite.minutes;
  pci->time.hours = pfsa4->ftimeLastWrite.hours;
  pci->ladate.day = pfsa4->fdateLastAccess.day;
  pci->ladate.month = pfsa4->fdateLastAccess.month;
  pci->ladate.year = pfsa4->fdateLastAccess.year + 1980;
  pci->latime.seconds = pfsa4->ftimeLastAccess.twosecs * 2;
  pci->latime.minutes = pfsa4->ftimeLastAccess.minutes;
  pci->latime.hours = pfsa4->ftimeLastAccess.hours;
  pci->crdate.day = pfsa4->fdateCreation.day;
  pci->crdate.month = pfsa4->fdateCreation.month;
  pci->crdate.year = pfsa4->fdateCreation.year + 1980;
  pci->crtime.seconds = pfsa4->ftimeCreation.twosecs * 2;
  pci->crtime.minutes = pfsa4->ftimeCreation.minutes;
  pci->crtime.hours = pfsa4->ftimeCreation.hours;
  pci->easize = CBLIST_TO_EASIZE(pfsa4->cbList);
  pci->cbFile = pfsa4->cbFile;
  pci->attrFile = pfsa4->attrFile;
  pci->pszDispAttr = FileAttrToString(pci->attrFile);
  pci->rc.pszIcon = pci->pszDisplayName;
  pci->rc.hptrIcon = hptr;

  if (dcd &&
      (*dcd->mask.szMask || dcd->mask.antiattr ||
       ((dcd->mask.attrFile &
	 (FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY | FILE_ARCHIVED)) !=
	(FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY | FILE_ARCHIVED)))) {
    if (*dcd->mask.szMask || dcd->mask.antiattr) {
      if (!Filter((PMINIRECORDCORE) pci, (PVOID) & dcd->mask))
	pci->rc.flRecordAttr |= CRA_FILTERED;
    }
    else if ((!(dcd->mask.attrFile & FILE_HIDDEN) &&
	      (pci->attrFile & FILE_HIDDEN)) ||
	     (!(dcd->mask.attrFile & FILE_SYSTEM) &&
	      (pci->attrFile & FILE_SYSTEM)) ||
	     (!(dcd->mask.attrFile & FILE_READONLY) &&
	      (pci->attrFile & FILE_READONLY)) ||
	     (!(dcd->mask.attrFile & FILE_ARCHIVED) &&
	      (pci->attrFile & FILE_ARCHIVED)))
      pci->rc.flRecordAttr |= CRA_FILTERED;
  }

  return pfsa4->cbFile + pci->easize;

} // FillInRecordFromFSA

VOID ProcessDirectory(const HWND hwndCnr,
		      const PCNRITEM pciParent,
		      const CHAR *szDirBase,
		      const BOOL filestoo,
		      const BOOL recurse,
		      const BOOL partial,
		      CHAR *stopflag,
		      DIRCNRDATA *dcd,	// Optional
		      ULONG *pulTotalFiles,	// Optional
		      PULONGLONG pullTotalBytes)	// Optional
{
  /* put all the directories (and files if filestoo is TRUE) from a
   * directory into the container.  recurse through subdirectories if
   * recurse is TRUE.
   */

  PSZ pszFileSpec;
  INT t;
  PFILEFINDBUF4 paffbFound;
  PFILEFINDBUF4 *papffbSelected;
  PFILEFINDBUF4 pffbFile;
  PFILEFINDBUF4 paffbTotal = NULL;
  PFILEFINDBUF4 paffbTemp;
  HDIR hdir = HDIR_CREATE;
  ULONG ulFileCnt;
  ULONG ulExtraBytes;
  ULONG ulM = 1;
  ULONG ulTotal = 0;
  ULONGLONG ullBytes;
  ULONGLONG ullTotalBytes;
  ULONG ulReturnFiles = 0;
  ULONGLONG ullReturnBytes = 0;
  PCH pchEndPath;
  APIRET rc;
  PCNRITEM pci;
  PCNRITEM pciFirst;
  RECORDINSERT ri;
  PBYTE pByte;
  PBYTE pByte2;
  BOOL ok = TRUE;

  if (isalpha(*szDirBase) && szDirBase[1] == ':' && szDirBase[2] == '\\') {
    ulExtraBytes = EXTRA_RECORD_BYTES;
    if ((driveflags[toupper(*szDirBase) - 'A'] & DRIVE_REMOTE) && fRemoteBug)
      ulM = 1;				/* file system gets confused */
    else if (driveflags[toupper(*szDirBase) - 'A'] & DRIVE_ZIPSTREAM)
      ulM = min(FilesToGet, 225);	/* anything more is wasted */
    else
      ulM = FilesToGet;			/* full-out */
  }
  else {
    ulExtraBytes = EXTRA_RECORD_BYTES;
    ulM = FilesToGet;
  }
  if (OS2ver[0] == 20 && OS2ver[1] < 30)
    ulM = min(ulM, (65535 / sizeof(FILEFINDBUF4)));

  ulFileCnt = ulM;
  pszFileSpec = xmalloc(CCHMAXPATH + 2, pszSrcFile, __LINE__);
  paffbFound =
    xmalloc((ulM + 1) * sizeof(FILEFINDBUF4), pszSrcFile, __LINE__);
  papffbSelected =
    xmalloc((ulM + 1) * sizeof(PFILEFINDBUF4), pszSrcFile, __LINE__);
  if (paffbFound && papffbSelected && pszFileSpec) {
    t = strlen(szDirBase);
    memcpy(pszFileSpec, szDirBase, t + 1);
    pchEndPath = pszFileSpec + t;
    if (*(pchEndPath - 1) != '\\') {
      memcpy(pchEndPath, "\\", 2);
      pchEndPath++;
    }
    memcpy(pchEndPath, "*", 2);
    DosError(FERR_DISABLEHARDERR);
    rc = DosFindFirst(pszFileSpec, &hdir,
		      FILE_NORMAL | ((filestoo) ? FILE_DIRECTORY :
				     MUST_HAVE_DIRECTORY) | FILE_READONLY |
		      FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN,
		      paffbFound, ulM * sizeof(FILEFINDBUF4),
		      &ulFileCnt, FIL_QUERYEASIZE);
    priority_normal();
    *pchEndPath = 0;
    if (!rc) {
      while (!rc) {
	/*
	 * remove . and .. from list if present
	 * also counter file system bugs that sometimes
	 * allows normal files to slip through when
	 * only directories should appear (only a few
	 * network file systems exhibit such a problem).
	 */
	register ULONG x;

	if (stopflag && *stopflag)
	  goto Abort;
	pByte = (PBYTE) paffbFound;
	for (x = 0; x < ulFileCnt;) {
	  pffbFile = (PFILEFINDBUF4) pByte;
	  if (!*pffbFile->achName ||
	      (!filestoo && !(pffbFile->attrFile & FILE_DIRECTORY)) ||
	      ((pffbFile->attrFile & FILE_DIRECTORY) &&
	       pffbFile->achName[0] == '.' &&
	       (!pffbFile->achName[1] ||
		(pffbFile->achName[1] == '.' && !pffbFile->achName[2])))) {
	    ulFileCnt--;		// Got . or ..
	  }
	  else
	    papffbSelected[x++] = pffbFile;	// Count file
	  if (!pffbFile->oNextEntryOffset) {
	    ulFileCnt = x;		// Adjust count
	    break;
	  }
	  pByte += pffbFile->oNextEntryOffset;
	} // for
	if (ulFileCnt) {
	  if (stopflag && *stopflag)
	    goto Abort;
	  if (fSyncUpdates) {
	    pciFirst = WinSendMsg(hwndCnr, CM_ALLOCRECORD,
				  MPFROMLONG(ulExtraBytes),
				  MPFROMLONG(ulFileCnt));
	    if (!pciFirst) {
	      Win_Error2(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
			 IDS_CMALLOCRECERRTEXT);
	      ok = FALSE;
	      ullTotalBytes = 0;
	    }
	    else {
	      register INT i;

	      pci = pciFirst;
	      ullTotalBytes = 0;
	      for (i = 0; i < ulFileCnt; i++) {
		pffbFile = papffbSelected[i];
		ullBytes = FillInRecordFromFFB(hwndCnr, pci, pszFileSpec,
					       pffbFile, partial, dcd);
		pci = (PCNRITEM) pci->rc.preccNextRecord;
		ullTotalBytes += ullBytes;
	      } // for
	      if (ulFileCnt) {
		memset(&ri, 0, sizeof(RECORDINSERT));
		ri.cb = sizeof(RECORDINSERT);
		ri.pRecordOrder = (PRECORDCORE) CMA_END;
		ri.pRecordParent = (PRECORDCORE) pciParent;
		ri.zOrder = (ULONG) CMA_TOP;
		ri.cRecordsInsert = ulFileCnt;
		ri.fInvalidateRecord = (!fSyncUpdates && dcd &&
					dcd->type == DIR_FRAME) ?
		  FALSE : TRUE;
		if (!WinSendMsg(hwndCnr,
				CM_INSERTRECORD,
				MPFROMP(pciFirst), MPFROMP(&ri))) {
		  DosSleep(10);
		  WinSetFocus(HWND_DESKTOP, hwndCnr);
		  if (!WinSendMsg(hwndCnr,
				  CM_INSERTRECORD,
				  MPFROMP(pciFirst), MPFROMP(&ri))) {
		    Win_Error2(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
			       IDS_CMINSERTERRTEXT);
		    ok = FALSE;
		    ullTotalBytes = 0;
		    if (WinIsWindow((HAB) 0, hwndCnr))
		      FreeCnrItemList(hwndCnr, pciFirst);
		  }
		}
	      }
	    }
	    if (ok) {
	      ullReturnBytes += ullTotalBytes;
	      ulReturnFiles += ulFileCnt;
	    }
	  }
	  else {
	    paffbTemp = xrealloc(paffbTotal,
				 sizeof(FILEFINDBUF4) * (ulFileCnt + ulTotal),
				 pszSrcFile, __LINE__);
	    if (paffbTemp) {
	      paffbTotal = paffbTemp;
	      for (x = 0; x < ulFileCnt; x++)
		paffbTotal[x + ulTotal] = *papffbSelected[x];
	      ulTotal += ulFileCnt;
	    }
	    else {
	      saymsg(MB_ENTER,
		     HWND_DESKTOP,
		     GetPString(IDS_ERRORTEXT), GetPString(IDS_OUTOFMEMORY));
	      break;
	    }
	  }
	}
	if (stopflag && *stopflag)
	  goto Abort;
	ulFileCnt = ulM;
	DosError(FERR_DISABLEHARDERR);
	rc = DosFindNext(hdir, paffbFound, ulM * sizeof(FILEFINDBUF4),
			 &ulFileCnt);
	priority_normal();
	if (rc)
	  DosError(FERR_DISABLEHARDERR);
      }
      DosFindClose(hdir);

      if (paffbFound || papffbSelected) {
	if (paffbFound)
	  free(paffbFound);
	if (papffbSelected)
	  free(papffbSelected);
	papffbSelected = NULL;
	paffbFound = NULL;
      }

      if (ulTotal && paffbTotal) {

	if (stopflag && *stopflag)
	  goto Abort;

	pciFirst = WinSendMsg(hwndCnr, CM_ALLOCRECORD,
			      MPFROMLONG(ulExtraBytes), MPFROMLONG(ulTotal));
	if (!pciFirst) {
	  Win_Error2(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
		     IDS_CMALLOCRECERRTEXT);
	  ok = FALSE;
	  ullTotalBytes = 0;
	}
	else {
	  register INT i;

	  pci = pciFirst;
	  ullTotalBytes = 0;
	  pByte2 = (PBYTE) paffbTotal;
	  for (i = 0; i < ulTotal; i++) {
	    pffbFile = (PFILEFINDBUF4) pByte2;
	    ullBytes = FillInRecordFromFFB(hwndCnr, pci, pszFileSpec,
					   pffbFile, partial, dcd);
	    pci = (PCNRITEM) pci->rc.preccNextRecord;
	    ullTotalBytes += ullBytes;

	    pByte2 += sizeof(FILEFINDBUF4);
	  }
	  if (ulTotal) {
	    memset(&ri, 0, sizeof(RECORDINSERT));
	    ri.cb = sizeof(RECORDINSERT);
	    ri.pRecordOrder = (PRECORDCORE) CMA_END;
	    ri.pRecordParent = (PRECORDCORE) pciParent;
	    ri.zOrder = (ULONG) CMA_TOP;
	    ri.cRecordsInsert = ulTotal;
	    ri.fInvalidateRecord = (!fSyncUpdates && dcd &&
				    dcd->type == DIR_FRAME) ? FALSE : TRUE;
	    if (!WinSendMsg(hwndCnr, CM_INSERTRECORD,
			    MPFROMP(pciFirst), MPFROMP(&ri))) {
	      DosSleep(10);
	      WinSetFocus(HWND_DESKTOP, hwndCnr);
	      if (!WinSendMsg(hwndCnr, CM_INSERTRECORD,
			      MPFROMP(pciFirst), MPFROMP(&ri))) {
		Win_Error2(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
			   IDS_CMINSERTERRTEXT);
		ok = FALSE;
		ullTotalBytes = 0;
		if (WinIsWindow((HAB) 0, hwndCnr))
		  FreeCnrItemList(hwndCnr, pciFirst);
	      }
	    }
	  }
	}
	if (ok) {
	  ullReturnBytes += ullTotalBytes;
	  ulReturnFiles += ulFileCnt;
	}
      }
    }

    if (!fSyncUpdates && dcd && dcd->type == DIR_FRAME)
      WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPVOID,
		 MPFROM2SHORT(0, CMA_ERASE));
  }
Abort:
  if (paffbTotal || papffbSelected || paffbFound || pszFileSpec) {
    if (paffbTotal)
      free(paffbTotal);
    if (pszFileSpec)
      free(pszFileSpec);
    if (paffbFound)
      free(paffbFound);
    if (papffbSelected)
      free(papffbSelected);
  }
  if (recurse) {
    pci = WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pciParent),
		     MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    while (pci && (INT)pci != -1) {
      if (pci->attrFile & FILE_DIRECTORY)
	Stubby(hwndCnr, pci);
      pci = WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pci),
		       MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    }
  }

  if (pulTotalFiles)
    *pulTotalFiles = ulReturnFiles;

  if (pullTotalBytes)
    *pullTotalBytes = ullReturnBytes;

} // ProcessDirectory

VOID FillDirCnr(HWND hwndCnr,
		CHAR * pszDirectory,
		DIRCNRDATA * dcd, PULONGLONG pullTotalBytes)
{
  ProcessDirectory(hwndCnr,
		   (PCNRITEM) NULL,
		   pszDirectory,
		   TRUE,		// filestoo
		   FALSE,		// recurse
		   TRUE,		// partial
		   dcd ? &dcd->stopflag : NULL,
		   dcd,
		   NULL,
		   pullTotalBytes);
  DosPostEventSem(CompactSem);

#if 0 // fixme to be gone or to be configurable
  {
    int state = _heapchk();
    if (state != _HEAPOK)
      Runtime_Error(pszSrcFile, __LINE__, "heap corrupted %d", state);
    else
      DbgMsg(pszSrcFile, __LINE__, "_memavl %u", _memavl());
  }
#endif

} // FillDirCnr

VOID FillTreeCnr(HWND hwndCnr, HWND hwndParent)
{
  ULONG ulCurDriveNum, ulDriveMap, numtoinsert = 0, drvtype;
  PCNRITEM pci, pciFirst = NULL, pciNext, pciParent = NULL;
  INT x, removable;
  CHAR suggest[32];
  CHAR szDrive[] = " :\\";
  CHAR szFileSystem[CCHMAXPATH];
  FILESTATUS4 fsa4;
  APIRET rc;
  BOOL drivesbuilt = FALSE;
  ULONG startdrive = 3;

  static BOOL didonce = FALSE;

  fDummy = TRUE;
  *suggest = 0;
  for (x = 0; x < 26; x++) {
    driveflags[x] &= (DRIVE_IGNORE | DRIVE_NOPRESCAN | DRIVE_NOLOADICONS |
		      DRIVE_NOLOADSUBJS | DRIVE_NOLOADLONGS |
		      DRIVE_INCLUDEFILES | DRIVE_SLOW | DRIVE_NOSTATS);
  }
  memset(driveserial, -1, sizeof(driveserial));

  DosError(FERR_DISABLEHARDERR);
  if (!DosQuerySysInfo(QSV_BOOT_DRIVE,
		       QSV_BOOT_DRIVE,
		       (PVOID) &startdrive,
		       (ULONG) sizeof(ULONG)) &&
      startdrive)
  {
    driveflags[startdrive - 1] |= DRIVE_BOOT;
  }

  DosError(FERR_DISABLEHARDERR);
  rc = DosQCurDisk(&ulCurDriveNum, &ulDriveMap);
  if (rc) {
    Dos_Error(MB_CANCEL,
	      rc,
	      HWND_DESKTOP,
	      pszSrcFile, __LINE__, GetPString(IDS_FILLDIRQCURERRTEXT));
    exit(0);
  }

  // Calc number of drive items to create
  for (x = 0; x < 26; x++) {
    if ((ulDriveMap & (1L << x)) && !(driveflags[x] & DRIVE_IGNORE))
      numtoinsert++;
  }

  if (numtoinsert) {
    pciFirst = WinSendMsg(hwndCnr,
			  CM_ALLOCRECORD,
			  MPFROMLONG(EXTRA_RECORD_BYTES),
			  MPFROMLONG((ULONG) numtoinsert));
  }

  if (!pciFirst) {
    Win_Error2(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__, IDS_CMALLOCRECERRTEXT);
    exit(0);
  }

  pci = pciFirst;
  for (x = 0; x < 26; x++) {
    if ((ulDriveMap & (1L << x)) && !(driveflags[x] & DRIVE_IGNORE)) {

      CHAR s[80];
      ULONG flags = 0;
      ULONG size = sizeof(ULONG);

      *szDrive = (CHAR)x + 'A';		// Build path spec

      sprintf(s, "%c.DriveFlags", toupper(*szDrive));
      if (PrfQueryProfileData(fmprof, appname, s, &flags, &size) &&
	  size == sizeof(ULONG)) {
	driveflags[toupper(*szDrive) - 'A'] |= flags;
      }

      if (x > 1) {
	// Hard drive (2..N)
	if (!(driveflags[x] & DRIVE_NOPRESCAN)) {
	  *szFileSystem = 0;
	  drvtype = 0;
	  removable = CheckDrive(*szDrive, szFileSystem, &drvtype);
	  driveserial[x] = -1;
	  if (removable != -1) {
	    struct {
	      ULONG serial;
	      CHAR volumelength;
	      CHAR volumelabel[CCHMAXPATH];
	    } volser;

	    DosError(FERR_DISABLEHARDERR);
	    if (!DosQueryFSInfo((ULONG) x,
				FSIL_VOLSER, &volser, sizeof(volser))) {
	      driveserial[x] = volser.serial;
	    }
	  }
	  else
	    driveflags[x] |= DRIVE_INVALID;

	  memset(&fsa4, 0, sizeof(FILESTATUS4));
	  driveflags[x] |= removable == -1 || removable == 1 ?
			    DRIVE_REMOVABLE : 0;
	  if (drvtype & DRIVE_REMOTE)
	    driveflags[x] |= DRIVE_REMOTE;
	  if (!stricmp(szFileSystem,RAMFS)) {
	    driveflags[x] |= DRIVE_RAMDISK;
	    driveflags[x] &= ~DRIVE_REMOTE;
	  }
	  if (!stricmp(szFileSystem,NDFS32)) {
	    driveflags[x] |= DRIVE_VIRTUAL;
	    driveflags[x] &= ~DRIVE_REMOTE;
	  }
	  if (!stricmp(szFileSystem,NTFS))
	    driveflags[x] |= DRIVE_NOTWRITEABLE;
	  if (strcmp(szFileSystem, HPFS) &&
	      strcmp(szFileSystem, JFS) &&
	      strcmp(szFileSystem, ISOFS) &&
	      strcmp(szFileSystem, CDFS) &&
	      strcmp(szFileSystem, FAT32) &&
	      strcmp(szFileSystem, NDFS32) &&
	      strcmp(szFileSystem, RAMFS) &&
	      strcmp(szFileSystem, NTFS) &&
	      strcmp(szFileSystem, HPFS386)) {
	    driveflags[x] |= DRIVE_NOLONGNAMES;
	  }

	  if (!strcmp(szFileSystem, CDFS) || !strcmp(szFileSystem,ISOFS)) {
	    removable = 1;
	    driveflags[x] |= DRIVE_REMOVABLE | DRIVE_NOTWRITEABLE |
			     DRIVE_CDROM;
	  }
	  else if (!stricmp(szFileSystem, CBSIFS)) {
	    driveflags[x] |= DRIVE_ZIPSTREAM;
	    driveflags[x] &= ~DRIVE_REMOTE;
	    if (drvtype & DRIVE_REMOVABLE)
	      driveflags[x] |= DRIVE_REMOVABLE;
	    if (!(drvtype & DRIVE_NOLONGNAMES))
	      driveflags[x] &= ~DRIVE_NOLONGNAMES;
	  }

	  pci->rc.flRecordAttr |= CRA_RECORDREADONLY;
	  // if ((ULONG) (toupper(*pci->pszFileName) - '@') == ulCurDriveNum)	// 23 Jul 07 SHL
	  if ((ULONG)(toupper(*szDrive) - '@') == ulCurDriveNum)
	    pci->rc.flRecordAttr |= (CRA_CURSORED | CRA_SELECTED);

	  if (removable == 0) {
	    // Fixed volume
	    pci->attrFile |= FILE_DIRECTORY;
	    DosError(FERR_DISABLEHARDERR);
	    rc = DosQueryPathInfo(szDrive,
				  FIL_QUERYEASIZE,
				  &fsa4, (ULONG) sizeof(FILESTATUS4));
	    // ERROR_BAD_NET_RSP = 58
	    if (rc == 58) {
	      DosError(FERR_DISABLEHARDERR);
	      rc = DosQueryPathInfo(szDrive,
				    FIL_STANDARD,
				    &fsa4, (ULONG) sizeof(FILESTATUS3));
	      fsa4.cbList = 0;
	    }
	    if (rc && !didonce) {
	      // Guess drive letter
	      if (!*suggest) {
		*suggest = '/';
		suggest[1] = 0;
	      }
	      sprintf(suggest + strlen(suggest), "%c" , toupper(*szDrive));
	      pci->pszFileName = xstrdup(szDrive, pszSrcFile, __LINE__);
	      strcpy(pci->pszFileName, szDrive);
	      pci->pszDisplayName = pci->pszFileName;
	      pci->rc.pszIcon = pci->pszDisplayName;
	      pci->attrFile = FILE_DIRECTORY;
	      pci->pszDispAttr = FileAttrToString(pci->attrFile);
	      driveserial[x] = -1;
	    }
	    else
	      FillInRecordFromFSA(hwndCnr, pci, szDrive, &fsa4, TRUE, NULL);
	  }
	  else {
	    // Removable volume
	    pci->pszFileName = xstrdup(szDrive, pszSrcFile, __LINE__);
	    strcpy(pci->pszFileName, szDrive);
	    pci->pszDisplayName = pci->pszFileName;
	    pci->rc.pszIcon = pci->pszDisplayName;
	    pci->attrFile = FILE_DIRECTORY;
	    pci->pszDispAttr = FileAttrToString(pci->attrFile);
	  }
	  SelectDriveIcon(pci);
	}
	else {
	  pci->rc.hptrIcon = hptrDunno;
	  pci->pszFileName = xstrdup(szDrive, pszSrcFile, __LINE__);
	  strcpy(pci->pszFileName, szDrive);
	  pci->pszDisplayName = pci->pszFileName;
	  pci->rc.pszIcon = pci->pszFileName;
	  pci->attrFile = FILE_DIRECTORY;
	  pci->pszDispAttr = FileAttrToString(pci->attrFile);
	  driveserial[x] = -1;
	}
      }
      else {
	// diskette drive (A or B)
	pci->rc.hptrIcon = hptrFloppy;
	pci->pszFileName = xstrdup(szDrive, pszSrcFile, __LINE__);
	strcpy(pci->pszFileName, szDrive);
	pci->pszDisplayName = pci->pszFileName;
	pci->rc.pszIcon = pci->pszDisplayName;
	pci->attrFile = FILE_DIRECTORY;
	pci->pszDispAttr = FileAttrToString(pci->attrFile);
	driveflags[x] |= (DRIVE_REMOVABLE | DRIVE_NOLONGNAMES);
	driveserial[x] = -1;
      }
      pci->rc.flRecordAttr |= CRA_RECORDREADONLY;
      pci = (PCNRITEM) pci->rc.preccNextRecord;	/* next rec */
    }
    else if (!(ulDriveMap & (1L << x)))
      driveflags[x] |= DRIVE_INVALID;
  } // for drives

  PostMsg(hwndMain, UM_BUILDDRIVEBAR, MPVOID, MPVOID);
  drivesbuilt = TRUE;

  /* insert the drives */
  if (numtoinsert && pciFirst) {
    RECORDINSERT ri;

    memset(&ri, 0, sizeof(RECORDINSERT));
    ri.cb = sizeof(RECORDINSERT);
    ri.pRecordOrder = (PRECORDCORE) CMA_END;
    ri.pRecordParent = (PRECORDCORE) NULL;
    ri.zOrder = (ULONG) CMA_TOP;
    ri.cRecordsInsert = numtoinsert;
    ri.fInvalidateRecord = FALSE;
    if (!WinSendMsg(hwndCnr,
		    CM_INSERTRECORD, MPFROMP(pciFirst), MPFROMP(&ri)))
    {
      Win_Error2(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
		 IDS_CMINSERTERRTEXT);
    }
  }

  /* move cursor onto the default drive rather than the first drive */
  if (!fSwitchTree) {
    pci = (PCNRITEM) WinSendMsg(hwndCnr,
				CM_QUERYRECORD,
				MPVOID,
				MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
    while (pci && (INT)pci != -1) {
      if ((ULONG) (toupper(*pci->pszFileName) - '@') == ulCurDriveNum) {
	WinSendMsg(hwndCnr,
		   CM_SETRECORDEMPHASIS,
		   MPFROMP(pci), MPFROM2SHORT(TRUE, CRA_CURSORED));
	break;
      }
      pci = (PCNRITEM) WinSendMsg(hwndCnr,
				  CM_QUERYRECORD,
				  MPFROMP(pci),
				  MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    }
  }

  if (hwndParent) {
    WinSendMsg(WinWindowFromID(WinQueryWindow(hwndParent, QW_PARENT),
			       MAIN_DRIVELIST),
	       LM_DELETEALL, MPVOID, MPVOID);
  }

  if (fShowEnv) {
    RECORDINSERT ri;

    pciParent = WinSendMsg(hwndCnr,
			   CM_ALLOCRECORD,
			   MPFROMLONG(EXTRA_RECORD_BYTES), MPFROMLONG(1));
    if (pciParent) {
      pciParent->flags |= RECFLAGS_ENV;
      pciParent->pszFileName = xstrdup(GetPString(IDS_ENVVARSTEXT), pszSrcFile, __LINE__);
      strcpy(pciParent->pszFileName, GetPString(IDS_ENVVARSTEXT));
      pciParent->pszDisplayName = pciParent->pszFileName;	// 03 Aug 07 SHL
      pciParent->rc.hptrIcon = hptrEnv;
      pciParent->rc.pszIcon = pciParent->pszFileName;
      pciParent->pszDispAttr = FileAttrToString(0);
      memset(&ri, 0, sizeof(RECORDINSERT));
      ri.cb = sizeof(RECORDINSERT);
      ri.pRecordOrder = (PRECORDCORE) CMA_END;
      ri.pRecordParent = (PRECORDCORE) NULL;
      ri.zOrder = (ULONG) CMA_TOP;
      ri.cRecordsInsert = 1;
      ri.fInvalidateRecord = FALSE;
      if (WinSendMsg(hwndCnr,
		     CM_INSERTRECORD, MPFROMP(pciParent), MPFROMP(&ri))) {

	char *p, *pp;

	p = GetPString(IDS_ENVVARNAMES);
	while (*p == ' ')
	  p++;
	while (*p) {
	  *szFileSystem = 0;
	  pp = szFileSystem;
	  while (*p && *p != ' ')
	    *pp++ = *p++;
	  *pp = 0;
	  while (*p == ' ')
	    p++;
	  if (*szFileSystem &&
	      (!stricmp(szFileSystem, "LIBPATH") || getenv(szFileSystem))) {
	    pci = WinSendMsg(hwndCnr,
			     CM_ALLOCRECORD,
			     MPFROMLONG(EXTRA_RECORD_BYTES),
			     MPFROMLONG(1));
	    if (pci) {
	      CHAR fname[CCHMAXPATH];
	      pci->flags |= RECFLAGS_ENV;
	      sprintf(fname, "%%%s%%", szFileSystem);
	      pci->pszFileName = xstrdup(fname, pszSrcFile, __LINE__);
	      pci->rc.hptrIcon = hptrEnv;
	      pci->rc.pszIcon = pci->pszFileName;
	      pci->pszDispAttr = FileAttrToString(0);
	      memset(&ri, 0, sizeof(RECORDINSERT));
	      ri.cb = sizeof(RECORDINSERT);
	      ri.pRecordOrder = (PRECORDCORE) CMA_END;
	      ri.pRecordParent = (PRECORDCORE) pciParent;
	      ri.zOrder = (ULONG) CMA_TOP;
	      ri.cRecordsInsert = 1;
	      ri.fInvalidateRecord = FALSE;
	      if (!WinSendMsg(hwndCnr,
			      CM_INSERTRECORD,
			      MPFROMP(pci), MPFROMP(&ri))) {
		Win_Error2(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
			   IDS_CMINSERTERRTEXT);
		FreeCnrItem(hwndCnr, pci);
	      }
	    }
	  }
	}
	WinSendMsg(hwndCnr,
		   CM_INVALIDATERECORD,
		   MPFROMP(&pciParent),
		   MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
      }
      else
	FreeCnrItem(hwndCnr, pciParent);
    }
  } // if show env

  x = 0;
  pci = (PCNRITEM) WinSendMsg(hwndCnr,
			      CM_QUERYRECORD,
			      MPVOID,
			      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  while (pci && (INT)pci != -1) {
    pciNext = (PCNRITEM) WinSendMsg(hwndCnr,
				    CM_QUERYRECORD,
				    MPFROMP(pci),
				    MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    if (!(pci->flags & RECFLAGS_ENV)) {
      if ((ULONG) (toupper(*pci->pszFileName) - '@') == ulCurDriveNum ||
	  toupper(*pci->pszFileName) > 'B')
      {
	if (!(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_INVALID) &&
	    !(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_NOPRESCAN) &&
	    (!fNoRemovableScan ||
	     !(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_REMOVABLE)))
	{
	  if (!Stubby(hwndCnr, pci) && !DRIVE_RAMDISK) {
	    WinSendMsg(hwndCnr,
		       CM_INVALIDATERECORD,
		       MPFROMP(&pci),
		       MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
	    goto SkipBadRec;
	  }
	}
      }
      else {
	WinSendMsg(hwndCnr,
		   CM_INVALIDATERECORD,
		   MPFROMP(&pci),
		   MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
      }

      WinSendMsg(WinWindowFromID(WinQueryWindow(hwndParent, QW_PARENT),
				 MAIN_DRIVELIST),
		 LM_INSERTITEM,
		 MPFROM2SHORT(LIT_SORTASCENDING, 0),
		 MPFROMP(pci->pszFileName));
    }
  SkipBadRec:
    x++;
    pci = pciNext;
  }
  if (hwndParent)
    WinSendMsg(WinWindowFromID(WinQueryWindow(hwndParent, QW_PARENT),
			       MAIN_DRIVELIST), LM_SELECTITEM,
	       MPFROM2SHORT(0, 0), MPFROMLONG(TRUE));

  pci = (PCNRITEM) WinSendMsg(hwndCnr,
			      CM_QUERYRECORD,
			      MPVOID,
			      MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
  while (pci && (INT)pci != -1) {
    pciNext = (PCNRITEM) WinSendMsg(hwndCnr,
				    CM_QUERYRECORD,
				    MPFROMP(pci),
				    MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
    if (pci->flags & RECFLAGS_ENV) {
      pci = (PCNRITEM) WinSendMsg(hwndCnr,
				  CM_QUERYRECORD,
				  MPFROMP(pci),
				  MPFROM2SHORT(CMA_FIRSTCHILD,
					       CMA_ITEMORDER));
      while (pci && (INT)pci != -1) {
	if (pci->flags & RECFLAGS_ENV)
	  FleshEnv(hwndCnr, pci);
	pci = (PCNRITEM) WinSendMsg(hwndCnr,
				    CM_QUERYRECORD,
				    MPFROMP(pci),
				    MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
      }
      break;
    }
    pci = (PCNRITEM) WinSendMsg(hwndCnr,
				CM_QUERYRECORD,
				MPFROMP(pci),
				MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
  }

  if (!drivesbuilt && hwndMain)
    PostMsg(hwndMain, UM_BUILDDRIVEBAR, MPVOID, MPVOID);
  DosSleep(16);//05 Aug 07 GKY 33
  fDummy = FALSE;
  DosPostEventSem(CompactSem);

  {
    BYTE info;
    BOOL includesyours = FALSE;

    if (*suggest || (!(driveflags[1] & DRIVE_IGNORE) && fFirstTime)) {
      if (!DosDevConfig(&info, DEVINFO_FLOPPY) && info == 1) {
	if (!*suggest) {
	  *suggest = '/';
	  suggest[1] = 0;
	}
	else
	  memmove(suggest + 2, suggest + 1, strlen(suggest));
	suggest[1] = 'B';
      }
    }
    if (*suggest) {
      for (x = 2; x < 26; x++) {
	if (driveflags[x] & DRIVE_IGNORE) {
	  includesyours = TRUE;
	  sprintf(suggest + strlen(suggest), "%c", (char)(x + 'A'));
	}
      }
      strcat(suggest, " %*");
      if (saymsg(MB_YESNO | MB_ICONEXCLAMATION,
		 (hwndParent) ? hwndParent : hwndCnr,
		 GetPString(IDS_SUGGESTTITLETEXT),
		 GetPString(IDS_SUGGEST1TEXT),
		 (includesyours) ? GetPString(IDS_SUGGEST2TEXT) : NullStr,
		 suggest) == MBID_YES) {
	char s[64];

	sprintf(s, "PARAMETERS=%s", suggest);
	WinCreateObject(WPProgram, "FM/2", s, FM3Folder, CO_UPDATEIFEXISTS);
	WinCreateObject(WPProgram,
			"FM/2 Lite", s, FM3Folder, CO_UPDATEIFEXISTS);
	WinCreateObject(WPProgram,
			"Archive Viewer/2", s, FM3Tools, CO_UPDATEIFEXISTS);
	WinCreateObject(WPProgram,
			"Dir Sizes", s, FM3Tools, CO_UPDATEIFEXISTS);
	WinCreateObject(WPProgram,
			"Visual Tree", s, FM3Tools, CO_UPDATEIFEXISTS);
	WinCreateObject(WPProgram,
			"Visual Directory", s, FM3Tools, CO_UPDATEIFEXISTS);
	WinCreateObject(WPProgram,
			"Global File Viewer", s, FM3Tools, CO_UPDATEIFEXISTS);
	WinCreateObject(WPProgram, "Databar", s, FM3Tools, CO_UPDATEIFEXISTS);
      }
    }
  }

  didonce = TRUE;

} // FillTreeCnr


/**
 * Empty all records from a container and free associated storage and
 * Free up field infos
 */

VOID EmptyCnr(HWND hwnd)
{
  PFIELDINFO pfi;

#if 0 // fixme to be gone or to be configurable
  {
    int state = _heapchk();
    if (state != _HEAPOK)
      Runtime_Error(pszSrcFile, __LINE__, "heap corrupted %d", state);
  }
#endif

  // Remove all records
  RemoveCnrItems(hwnd, NULL, 0, CMA_FREE);

  // Remove field info descriptors
  pfi = (PFIELDINFO) WinSendMsg(hwnd, CM_QUERYDETAILFIELDINFO, MPVOID,
				MPFROMSHORT(CMA_FIRST));
  if (pfi &&
      (INT)WinSendMsg(hwnd, CM_REMOVEDETAILFIELDINFO, MPVOID,
	       MPFROM2SHORT(0, CMA_FREE)) == -1) {
    Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,"CM_REMOVEDETAILFIELDINFO hwnd %x", hwnd);
  }
}

/**
 * Free storage associated with container item
 */

VOID FreeCnrItemData(PCNRITEM pci)
{
  PSZ psz;
  // DbgMsg(pszSrcFile, __LINE__, "FreeCnrItemData %p", pci);

  if (pci->pszSubject && pci->pszSubject != NullStr) {

    psz = pci->pszSubject;
    pci->pszSubject = NullStr;
    free(psz);
  }

  // +1 in case long name pointing after last backslash
  if (pci->pszLongName &&
      pci->pszLongName != NullStr &&
      pci->pszLongName != pci->pszFileName &&
      pci->pszLongName != pci->pszDisplayName &&
      pci->pszLongName != pci->pszDisplayName + 1) {
    psz = pci->pszLongName;
    pci->pszLongName = NullStr;
    free(psz);
  }

  if (pci->pszFileName && pci->pszFileName != NullStr) {
    psz = pci->pszFileName;
    pci->pszFileName = NullStr;
    free(psz);
  }
}

/**
 * Free single container item and associated storage
 */

VOID FreeCnrItem(HWND hwnd, PCNRITEM pci)
{
  // DbgMsg(pszSrcFile, __LINE__, "FreeCnrItem hwnd %x pci %p", hwnd, pci);

  FreeCnrItemData(pci);

  if (!WinSendMsg(hwnd, CM_FREERECORD, MPFROMP(&pci), MPFROMSHORT(1))) {
    // Win_Error2(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,IDS_CMFREEERRTEXT);
    Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,
	      "CM_FREERECORD hwnd %x pci %p file %s",
              hwnd, pci,
	      pci && pci->pszFileName ? pci->pszFileName : "n/a");
  }
}

/**
 * Free container item list and associated storage
 */

VOID FreeCnrItemList(HWND hwnd, PCNRITEM pciFirst)
{
  PCNRITEM pci = pciFirst;
  PCNRITEM pciNext;
  USHORT usCount;

  for (usCount = 0; pci; usCount++) {
    pciNext = (PCNRITEM) pci->rc.preccNextRecord;
    FreeCnrItemData(pci);
    pci = pciNext;
  }

  if (usCount) {
    if (!WinSendMsg(hwnd, CM_FREERECORD, MPFROMP(&pci), MPFROMSHORT(usCount))) {
      // Win_Error2(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,IDS_CMFREEERRTEXT);
      Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,"CM_FREERECORD hwnd %x pci %p cnt %u", hwnd, pci, usCount);
    }
  }
}

/**
 * Remove item(s) from container and free associated storage if requested
 * @param pciFirst points to first item to remove or NULL to remove all
 * @param usCnt is remove count or 0 to remove all
 * @returns count of items remaining in container or -1 if error
 */

INT RemoveCnrItems(HWND hwnd, PCNRITEM pciFirst, USHORT usCnt, USHORT usFlags)
{
  INT remaining = usCnt;
  PCNRITEM pci;

  if ((usCnt && !pciFirst) || (!usCnt && pciFirst)) {
      Runtime_Error(pszSrcFile, __LINE__, "pciFirst %p usCnt %u mismatch", pciFirst, usCnt);
      remaining = -1;
  }
  else {
    // Free our buffers if free requested
    if (usFlags & CMA_FREE) {
      if (pciFirst)
	pci = pciFirst;
      else {
	pci = (PCNRITEM)WinSendMsg(hwnd, CM_QUERYRECORD, MPVOID,
				   MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
	if ((INT)pci == -1) {
	  Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,"CM_QUERYRECORD");
	  remaining = -1;
	  pci = NULL;
	}
      }
      while (pci) {
	FreeCnrItemData(pci);
	pci = (PCNRITEM)pci->rc.preccNextRecord;
	if (remaining && --remaining == 0)
	  break;
      }
    }
  }

  // DbgMsg(pszSrcFile, __LINE__, "RemoveCnrItems %p %u %s", pci, usCnt, pci->pszFileName);

  if (remaining != - 1) {
    remaining = (INT)WinSendMsg(hwnd, CM_REMOVERECORD, MPFROMP(&pciFirst), MPFROM2SHORT(usCnt, usFlags));
    if (remaining == -1) {
      // Win_Error2(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,IDS_CMREMOVEERRTEXT);
      Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,"CM_REMOVERECORD hwnd %x pci %p cnt %u", hwnd, pciFirst, usCnt);
    }
  }

  return remaining;
}

