
/***********************************************************************

  $Id$

  Fill Directory Tree Containers

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

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
  13 Aug 07 SHL Sync code with other FilesToGet usage and optimize
  13 Aug 07 SHL Move #pragma alloc_text to end for OpenWatcom compat
  04 Nov 07 GKY Use commaFmtULL to display large file sizes
  29 Feb 08 GKY Use xfree where appropriate
  07 Jul 08 SHL Use NULL rather than NullStr in FreeCnrItemData
  16 JUL 08 GKY Use TMP directory for temp files
  20 Jul 08 JBS Ticket 114: Support user-selectable env. strings in Tree container.
  21 Jul 08 JBS Ticket 114: Change env var separator from blank to semicolon
  02 Aug 08 GKY Remove redundant strcpys from inner loop
  23 Aug 08 GKY Free pszDisplayName when appropriate
  01 Sep 08 GKY Updated FreeCnrItemData to prevent trap in strrchr if pci->pszFileName is NULL.
  05 Sep 08 SHL Correct FreeCnrItemData pszDisplayName pointer overlap check
  08 Sep 08 SHL Remove extra pszLongName logic in FreeCnrItemData
  18 Oct 08 GKY Scan drives in 4 passes (local, virtual, remote, flagged slow) to speed tree scans
  19 Nov 08 SHL Correct and sanitize 4 pass scan logic
  21 Nov 08 SHL FillTreeCnr: ensure any unchecked drives checked in pass 4
  24 Nov 08 GKY Add StubyScanThread to treecnr scan/rescan drives all on separate threads
  24 Nov 08 GKY Replace 4 pass drive scan code with StubyScanThread multithread scan
  28 Nov 08 SHL FreeCnrItemData: optimize and always NULL pointers
  28 Nov 08 SHL StubbyScanThread: add else lost in translation
  30 Nov 08 SHL StubbyScanThread: restore else - we want all drives listed
  30 Nov 08 SHL FreeCnrItemData: report double free with Runtime_Error
  04 Dec 08 GKY Use event semaphore to prevent scan of "last" directory container prior to
		tree scan completion; prevents duplicate directory names in tree.
  10 Dec 08 SHL Integrate exception handler support
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  25 Dec 08 GKY Add ProcessDirectoryThread to allow optional recursive drive scan at startup.
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  08 Mar 09 GKY Additional strings move to PCSZs
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)
  14 Mar 09 GKY Prevent execution of UM_SHOWME while drive scan is occuring
  06 Jun 09 GKY Add option to show file system type or drive label in tree
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.
  06 Jul 09 SHL Refactor .LONGNAME and .SUBJECT EA fetch to FetchCommonEAs
  12 Jul 09 GKY Add szFSType to FillInRecordFromFSA use to bypass EA scan and size formatting
                for tree container

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <malloc.h>			// _msize _heapchk
#include <ctype.h>
// #include <process.h>			// _beginthread

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSERRORS
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "draglist.h"			// Data declaration(s)
#include "treecnr.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "newview.h"			// Data declaration(s)
#include "fm3str.h"
#include "filldir.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "misc.h"			// GetTidForWindow
#include "fortify.h"			// 06 May 08 SHL
#include "notebook.h"			// INI file fields
#include "flesh.h"			// FleshEnv, Stubby
#include "update.h"			// SelectDriveIcon
#include "valid.h"			// CheckDrive
#include "filter.h"			// Filter
#include "subj.h"			// Subject
#include "copyf.h"			// unlinkf
#include "literal.h"			// wildcard
#include "i18nutil.h"			// CommaFmtULL
#include "wrappers.h"			// xDosFindNext
#include "init.h"			// GetTidForWindow
#include "common.h"			// IncrThreadUsage
#include "excputil.h"			// xbeginthread
#include "fm3dlg.h"                     // INFO_LABEL
#include "pathutil.h"                   // AddBackslashToPath

VOID StubbyScanThread(VOID * arg);

// Data definitions
static PSZ pszSrcFile = __FILE__;
static BOOL fFirstTime;

#pragma data_seg(GLOBAL1)
HPOINTER hptrEnv;
HPOINTER hptrHidden;
HPOINTER hptrReadonly;
HPOINTER hptrSystem;

#pragma data_seg(GLOBAL2)
PCSZ FM3Tools   = "<FM3_Tools>";
PCSZ WPProgram  = "WPProgram";
volatile INT StubbyScanCount;
volatile INT ProcessDirCount;

typedef struct {
  PCNRITEM    pci;
  HWND        hwndCnr;			// hwnd you want the message posted to
  HWND        hwndDrivesList;
  //BOOL        RamDrive;
}
STUBBYSCAN;

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

VOID StubbyScanThread(VOID * arg)
{
  STUBBYSCAN *StubbyScan;
  HAB thab;
  HMQ hmq = (HMQ) 0;
  BOOL ret;

  DosError(FERR_DISABLEHARDERR);

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif

  StubbyScan = (STUBBYSCAN *)arg;
  if (StubbyScan && StubbyScan->pci && StubbyScan->pci->pszFileName && StubbyScan->hwndCnr) {
    thab = WinInitialize(0);
    if (thab) {
      hmq = WinCreateMsgQueue(thab, 0);
      if (hmq) {
        StubbyScanCount++;
	IncrThreadUsage();
	priority_normal();
        ret = Stubby(StubbyScan->hwndCnr, StubbyScan->pci);
	//DbgMsg(pszSrcFile, __LINE__, "Stubby %i ", ret);
        if (ret == 1) {
          if (WinIsWindow((HAB)0, StubbyScan->hwndCnr)) {
	    WinSendMsg(StubbyScan->hwndCnr,
		       CM_INVALIDATERECORD,
		       MPFROMP(&StubbyScan->pci),
                       MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
            if ((fRScanLocal &&
                (!(driveflags[toupper(*StubbyScan->pci->pszFileName) - 'A'] &
                   ((DRIVE_REMOTE | DRIVE_VIRTUAL)))) ||
                 (fRScanRemote &&
                  (driveflags[toupper(*StubbyScan->pci->pszFileName) - 'A'] &
                   DRIVE_REMOTE)) ||
                 (fRScanVirtual &&
                  (driveflags[toupper(*StubbyScan->pci->pszFileName) - 'A'] &
                   DRIVE_VIRTUAL))) && fInitialDriveScan) {
              if (!(driveflags[toupper(*StubbyScan->pci->pszFileName) - 'A'] &
                    ((fRScanNoWrite ? 0 : DRIVE_NOTWRITEABLE) |
                     (fRScanSlow ? 0 : DRIVE_SLOW)))) {
                WinSendMsg(StubbyScan->hwndCnr, CM_EXPANDTREE, MPFROMP(StubbyScan->pci), MPVOID);
                //DbgMsg(pszSrcFile, __LINE__, "expanded %x %p", StubbyScan->hwndCnr, StubbyScan->pci);
                WinSendMsg(StubbyScan->hwndCnr, CM_COLLAPSETREE, MPFROMP(StubbyScan->pci), MPVOID);
              }
            }
          }
        }
	if (WinIsWindow((HAB)0, StubbyScan->hwndDrivesList)) {
	  WinSendMsg(StubbyScan->hwndDrivesList,
		     LM_INSERTITEM,
		     MPFROM2SHORT(LIT_SORTASCENDING, 0),
		     MPFROMP(StubbyScan->pci->pszFileName));
        }
        StubbyScanCount--;
	WinDestroyMsgQueue(hmq);
      }
      DecrThreadUsage();
      WinTerminate(thab);
    }
    free(StubbyScan);
  } // if StubbyScan
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif

  // _endthread();			// 10 Dec 08 SHL
}

VOID ProcessDirectoryThread(VOID * arg)
{
  PROCESSDIR *ProcessDir;
  HAB thab;
  HMQ hmq = (HMQ) 0;
  //BOOL ret;

  DosError(FERR_DISABLEHARDERR);

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif

  ProcessDir = (PROCESSDIR *)arg;
  if (ProcessDir && ProcessDir->pciParent && ProcessDir->pciParent->pszFileName && ProcessDir->hwndCnr) {
    thab = WinInitialize(0);
    if (thab) {
      hmq = WinCreateMsgQueue(thab, 0);
      if (hmq) {
        ProcessDirCount ++;
	IncrThreadUsage();
        priority_normal();
        ProcessDirectory(ProcessDir->hwndCnr,
                         ProcessDir->pciParent,
                         ProcessDir->szDirBase,
                         ProcessDir->filestoo,
                         ProcessDir->recurse,
                         ProcessDir->partial,
                         ProcessDir->stopflag,
                         ProcessDir->dcd,	                // Optional
                         ProcessDir->pulTotalFiles,	// Optional
                         ProcessDir->pullTotalBytes);	// Optional
        ProcessDirCount --;
	WinDestroyMsgQueue(hmq);
      }
      DecrThreadUsage();
      WinTerminate(thab);
    }
    free(ProcessDir);
  } // if ProcessDir
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif

  // _endthread();			// 10 Dec 08 SHL
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

    if (cmp == *(ULONG *) PCSZ_DOTEXE || cmp == *(ULONG *) PCSZ_DOTCMD ||
        cmp == *(ULONG *) PCSZ_DOTBAT || cmp == *(ULONG *) PCSZ_DOTCOM ||
        cmp == *(ULONG *) PCSZ_DOTBTM)
      hptr = hptrApp;
    else if (cmp == *(ULONG *) ".ZIP" || cmp == *(ULONG *) ".LZH" ||
	     cmp == *(ULONG *) ".ARJ" || cmp == *(ULONG *) ".ARC" ||
	     cmp == *(ULONG *) ".ZOO" || cmp == *(ULONG *) ".RAR" ||
	     cmp == *(ULONG *) ".TAR" || cmp == *(ULONG *) ".TGZ" ||
	     cmp == *(ULONG *) ".GZ"  || cmp == *(ULONG *) ".Z"   ||
	     cmp == *(ULONG *) ".CAB" || cmp == *(ULONG *) ".BZ2")
      hptr = hptrArc;
    else if (cmp == *(ULONG *) PCSZ_DOTBMP  ||
             cmp == *(ULONG *) PCSZ_DOTICO  ||
             cmp == *(ULONG *) PCSZ_DOTPTR  ||
             cmp == *(ULONG *) PCSZ_DOTJPEG ||
             cmp == *(ULONG *) PCSZ_DOTJPG  ||
             cmp == *(ULONG *) ".GIF" ||
	     cmp == *(ULONG *) ".TIF" || cmp == *(ULONG *) ".PCX" ||
	     cmp == *(ULONG *) ".TGA" || cmp == *(ULONG *) ".XBM" ||
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

    if (pTmpDir) {
      psz = pTmpDir;
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
	unlinkf(szFileName);
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

static VOID FetchCommonEAs(PCNRITEM pci)
{
  ULONG flags = driveflags[toupper(*pci->pszFileName) - 'A'];
  BOOL fLoadSubjectForDrive = fLoadSubject && ~flags & DRIVE_NOLOADSUBJS;
  BOOL fLoadLongNameForDrive = fLoadLongnames &&  ~flags & DRIVE_NOLONGNAMES &&
			       ~flags & DRIVE_NOLOADLONGS;
  if (fLoadSubjectForDrive || fLoadLongNameForDrive) {
    // Allocate space to hold GEA2s and .SUBJECT and .LONGNAME strings
    PGEA2LIST pgealist = xmallocz(sizeof(GEA2LIST) + sizeof(GEA2) + 64, pszSrcFile, __LINE__);
    if (pgealist) {
      APIRET rc;
      PFEA2LIST pfealist;
      ULONG offset;
      PGEA2 pgeaPrev = NULL;
      PGEA2 pgea = pgealist->list;	// Point at first available
      EAOP2 eaop;
      UINT state;
      //DbgMsg(pszSrcFile, __LINE__, "pszFileName %s", pci->pszFileName);
      for (state = 0; state < 2; state++) {
	PCSZ pcsz;
	switch (state) {
	case 0:
	  pcsz = fLoadSubjectForDrive ? SUBJECT : NULL;
	  break;
	case 1:
	  pcsz = fLoadLongNameForDrive ? LONGNAME : NULL;
	  break;
	}
	if (pcsz) {
	  if (pgeaPrev) {
	    pgeaPrev->oNextEntryOffset = (PSZ)pgea - (PSZ)pgeaPrev;
	    //DbgMsg(pszSrcFile, __LINE__, "pgea %p oNextEntryOffset %u", pgeaPrev, pgeaPrev->oNextEntryOffset);
	  }
	  strcpy(pgea->szName, pcsz);
	  pgea->cbName = strlen(pgea->szName);
	  pgea->oNextEntryOffset = 0;
	  //DbgMsg(pszSrcFile, __LINE__, "pgea %p cbName %u szName %s", pgea, pgea->cbName, pgea->szName);
	  offset = sizeof(GEA2) + pgea->cbName;	// Calc length including null
	  offset = (offset + 3) & ~3;		// Doubleword align
	  pgeaPrev = pgea;
	  pgea = (PGEA2)((PSZ)pgea + offset);	// Point at next available
	}
      } // for

      pgealist->cbList = (PSZ)pgea - (PSZ)pgealist;
      //DbgMsg(pszSrcFile, __LINE__, "pgealist %p cbList %u", pgealist, pgealist->cbList);

      pfealist = xmallocz(4096, pszSrcFile, __LINE__);
      if (pfealist) {
	pfealist->cbList = 4096;
	eaop.fpGEA2List = pgealist;
	eaop.fpFEA2List = pfealist;
	eaop.oError = 0;
	rc = DosQueryPathInfo(pci->pszFileName, FIL_QUERYEASFROMLIST,
			      (PVOID) &eaop, (ULONG) sizeof(EAOP2));
        if (rc) {
          CHAR s[80];
          sprintf(s, "%s %s",PCSZ_DOSQUERYPATHINFO, "%s");
          Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
                    s, pci->pszFileName);
        }
	  //DbgMsg(pszSrcFile, __LINE__, "DosQueryPathInfo %s failed with rc %u ", pci->pszFileName, rc);
	else {
	  PFEA2 pfea = eaop.fpFEA2List->list;
	  while (pfea) {
	    if (pfea->cbValue) {
	      CHAR *achValue = pfea->szName + pfea->cbName + 1;
              if (*(USHORT *)achValue != EAT_ASCII)
                Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
                          GetPString(IDS_ERROREATYPETEXT),
                          achValue, pfea->cbName, pfea->szName);
		//DbgMsg(pszSrcFile, __LINE__, "EA type 0x%x unexpected for %.*s", achValue, pfea->cbName, pfea->szName);
	      else {
		CHAR ch = achValue[pfea->cbValue];
		PSZ pszValue;
		achValue[pfea->cbValue] = 0;
		pszValue = xstrdup(achValue + (sizeof(USHORT) * 2), pszSrcFile, __LINE__);
		achValue[pfea->cbValue] = ch;
		//DbgMsg(pszSrcFile, __LINE__, "pfea %p %.*s cbValue %u %s", pfea, pfea->cbName, pfea->szName, pfea->cbValue, pszValue);
		if (strncmp(pfea->szName, LONGNAME, pfea->cbName) == 0)
		  pci->pszLongName = pszValue;
		else if (strncmp(pfea->szName, SUBJECT, pfea->cbName) == 0)
		  pci->pszSubject = pszValue;
                else
                  Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		            GetPString(IDS_ERROREATYPETEXT), pfea, pfea->cbName, pfea->szName);
		  //DbgMsg(pszSrcFile, __LINE__, "pfea %p EA %.*s unexpected", pfea, pfea->cbName, pfea->szName);
	      }
	    }
	    if (!pfea->oNextEntryOffset)
	      break;
	    pfea = (PFEA2)((PSZ)pfea + pfea->oNextEntryOffset);
	  } // while
	}
	free(pfealist);
      }
      free(pgealist);
    }
  }
} // FetchCommonEAs

ULONGLONG FillInRecordFromFFB(HWND hwndCnr,
			      PCNRITEM pci,
			      const PSZ pszDirectory,
			      const PFILEFINDBUF4L pffb,
                              const BOOL partial,
			      DIRCNRDATA *dcd)
{
  // fill in a container record from a FILEFINDBUF4L structure

  CHAR *p;
  HPOINTER hptr;
  ULONG flags;

  pci->hwndCnr = hwndCnr;

  /* note that we cheat below, and accept the full pathname in pszDirectory
     if !*pffb->achName.  This speeds up and simplifies processing elsewhere
     (like in update.c)
   */
  if (!*pffb->achName) {
    pci->pszFileName = xstrdup(pszDirectory, pszSrcFile, __LINE__);
    //strcpy(pci->pszFileName, pszDirectory);
  }
  else {
    INT c = strlen(pszDirectory);
    INT c2 = pffb->cchName + 1;
    if (pszDirectory[c - 1] != '\\')
      c2++;
    pci->pszFileName = xmalloc(c + c2, pszSrcFile, __LINE__);
#   ifdef FORTIFY
    {
      if (dcd->type != TREE_FRAME)
	Fortify_ChangeScope(pci->pszFileName, -1);
      else {
	Fortify_SetOwner(pci->pszFileName, 1);
	Fortify_SetScope(pci->pszFileName, 2);
      }
    }
#   endif
    memcpy(pci->pszFileName, pszDirectory, c + 1);
    p = pci->pszFileName + c - 1;
    if (*p != '\\') {
      p++;
      *p = '\\';
    }
    p++;
    memcpy(p, pffb->achName, pffb->cchName + 1);
  }

  // load Subject and/or Longname EAs, if required and have EAs
  if (pffb->cbList > 4L && dcd)
    FetchCommonEAs(pci);

  if (!pci->pszSubject)
    pci->pszSubject = NullStr;

  // load the object's longname
  if (!pci->pszLongName)
    pci->pszLongName = NullStr;

  // do anything required to case of filename
  if (fForceUpper)
    strupr(pci->pszFileName);
  else if (fForceLower)
    strlwr(pci->pszFileName);

  flags = driveflags[toupper(*pci->pszFileName) - 'A'];

  // get an icon to use with it
  if (pffb->attrFile & FILE_DIRECTORY) {
    // is directory
    if (fNoIconsDirs ||
	(flags & DRIVE_NOLOADICONS) ||
	!isalpha(*pci->pszFileName)) {
      hptr = (HPOINTER) 0;
    }
    else
      hptr = WinLoadFileIcon(pci->pszFileName, FALSE);
  }
  else {
    // is file
    if (fNoIconsFiles ||
	(flags & DRIVE_NOLOADICONS) ||
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
	   hptrSystem : pffb->attrFile & FILE_HIDDEN ?
	   hptrHidden : pffb->attrFile & FILE_READONLY ?
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

  //comma format the file size for large file support
  {
    CHAR szBuf[30];
    CommaFmtULL(szBuf, sizeof(szBuf), pffb->cbFile, ' ');
    pci->pszFmtFileSize = xstrdup(szBuf, pszSrcFile, __LINE__);
#   ifdef FORTIFY
    {
      unsigned tid = GetTidForWindow(hwndCnr);
      if (tid == 1)
	Fortify_ChangeScope(pci->pszFmtFileSize, -1);
      else
	Fortify_SetOwner(pci->pszFmtFileSize, 1);
    }
#   endif
  }

  // now fill the darned thing in...
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

  // check to see if record should be visible
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

ULONGLONG FillInRecordFromFSA(HWND hwndCnr,
			      PCNRITEM pci,
			      const PSZ pszFileName,
			      const PFILESTATUS4L pfsa4,
                              const BOOL partial,
                              CHAR *szFSType,
			      DIRCNRDATA *dcd)	// Optional
{
  HPOINTER hptr;
  ULONG flags;
  CHAR *p;
  CHAR szBuf[80];

  // fill in a container record from a FILESTATUS4L structure

  pci->hwndCnr = hwndCnr;
  pci->pszFileName = xstrdup(pszFileName, pszSrcFile, __LINE__);

  if (pfsa4->cbList > 4L && dcd && *szFSType == 0)
    FetchCommonEAs(pci);

  if (!pci->pszSubject)
    pci->pszSubject = NullStr;
  if (!pci->pszLongName)
    pci->pszLongName = NullStr;

  if (fForceUpper)
    strupr(pci->pszFileName);
  else if (fForceLower)
    strlwr(pci->pszFileName);

  flags = driveflags[toupper(*pci->pszFileName) - 'A'];

  if (pfsa4->attrFile & FILE_DIRECTORY) {
    if (fNoIconsDirs ||
	(flags & DRIVE_NOLOADICONS) ||
	!isalpha(*pci->pszFileName)) {
      hptr = (HPOINTER) 0;
    }
    else
      hptr = WinLoadFileIcon(pci->pszFileName, FALSE);
  }
  else {
    if (fNoIconsFiles ||
	(flags & DRIVE_NOLOADICONS) ||
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
  if (szFSType && (fShowFSTypeInTree || fShowDriveLabelInTree)) {
    strcpy(szBuf, p);
    strcat(szBuf, " [");
    strcat(szBuf, szFSType);
    strcat(szBuf, "]");
    pci->pszDisplayName = xstrdup(szBuf, pszSrcFile, __LINE__);
  }
  else
    pci->pszDisplayName = p;

  //comma format the file size for large file support
  if (*szFSType == 0) {
    CHAR szBuf[30];
    CommaFmtULL(szBuf, sizeof(szBuf), pfsa4->cbFile, ' ');
    pci->pszFmtFileSize = xstrdup(szBuf, pszSrcFile, __LINE__);
#   ifdef FORTIFY
    {
      if (dcd && dcd->type == TREE_FRAME) {
	// Will be freed in TreeCnrWndProc WM_DESTROY
	// Fortify_SetOwner(pci->pszFmtFileSize, 1);
	Fortify_SetScope(pci->pszFmtFileSize, 2);
      }
    }
#   endif
  }
  else
    pci->pszFmtFileSize = NullStr;
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
  //INT t;
  PFILEFINDBUF4L paffbFound;
  PFILEFINDBUF4L *papffbSelected;
  PFILEFINDBUF4L pffbFile;
  PFILEFINDBUF4L paffbTotal = NULL;
  PFILEFINDBUF4L paffbTemp;
  HDIR hdir = HDIR_CREATE;
  ULONG ulFindCnt;
  ULONG ulFindMax;
  ULONG ulSelCnt;
  ULONG ulTotal = 0;
  ULONGLONG ullBytes;
  ULONGLONG ullTotalBytes;
  ULONG ulReturnFiles = 0;
  ULONGLONG ullReturnBytes = 0;
  //PCH pchEndPath;
  APIRET rc;
  PCNRITEM pci;
  PCNRITEM pciFirst;
  RECORDINSERT ri;
  BOOL ok = TRUE;
  ULONG ulBufBytes;
  ULONG x;

  if (isalpha(*szDirBase) && szDirBase[1] == ':' && szDirBase[2] == '\\') {
    if ((driveflags[toupper(*szDirBase) - 'A'] & DRIVE_REMOTE) && fRemoteBug)
      ulFindMax = 1;			// file system gets confused
    else if (driveflags[toupper(*szDirBase) - 'A'] & DRIVE_ZIPSTREAM)
      ulFindMax = min(FilesToGet, 225);	// anything more is wasted
    else
      ulFindMax = FilesToGet;		// full-out
  }
  else
    ulFindMax = FilesToGet;

  if (OS2ver[0] == 20 && OS2ver[1] < 30)
    ulFindMax = min(ulFindMax, (65535 / sizeof(FILEFINDBUF4L)));

  ulBufBytes = ulFindMax * sizeof(FILEFINDBUF4L);

  pszFileSpec = xmalloc(CCHMAXPATH + 2, pszSrcFile, __LINE__);
  paffbFound = xmalloc(ulBufBytes, pszSrcFile, __LINE__);
  papffbSelected = xmalloc(sizeof(PFILEFINDBUF4L) * ulFindMax, pszSrcFile, __LINE__);

  if (paffbFound && papffbSelected && pszFileSpec) {
    //t = strlen(szDirBase);
    strcpy(pszFileSpec, szDirBase);
    AddBackslashToPath(pszFileSpec);
    //pchEndPath = pszFileSpec + t;
    //if (*(pchEndPath - 1) != '\\') {
    //  memcpy(pchEndPath, "\\", 2);
    //  pchEndPath++;
    //}
    strcat(pszFileSpec, "*");
    DosError(FERR_DISABLEHARDERR);
    ulFindCnt = ulFindMax;
    rc = xDosFindFirst(pszFileSpec,
		       &hdir,
		       FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
		       FILE_SYSTEM | FILE_HIDDEN |
		       (filestoo ? FILE_DIRECTORY : MUST_HAVE_DIRECTORY),
		       paffbFound,
		       ulBufBytes,
		       &ulFindCnt,
		       FIL_QUERYEASIZEL);
    priority_normal();
    pszFileSpec[strlen(pszFileSpec) - 1] = 0;     // Chop off wildcard
    //*pchEndPath = 0;			
    if (!rc) {
      do {
	/*
	 * remove . and .. from list if present
	 * also counter file system bugs that sometimes
	 * allows normal files to slip through when
	 * only directories should appear (only a few
	 * network file systems exhibit such a problem).
	 */

	if (stopflag && *stopflag)
	  goto Abort;
	pffbFile = paffbFound;
	ulSelCnt = 0;
	for (;;) {
	  if (!*pffbFile->achName ||
	      (!filestoo && ~pffbFile->attrFile & FILE_DIRECTORY) ||
	      (pffbFile->attrFile & FILE_DIRECTORY &&
	       pffbFile->achName[0] == '.' &&
	       (!pffbFile->achName[1] ||
		(pffbFile->achName[1] == '.' && !pffbFile->achName[2])))) {
	    // ulFindCnt--;		// Got . or .. or file to be skipped
	  }
	  else
	    papffbSelected[ulSelCnt++] = pffbFile;	// Remember selected file
	  if (!pffbFile->oNextEntryOffset) {
	    // ulFindCnt = ulSelCnt;	// Remember number selected
	    break;
	  }
	  pffbFile = (PFILEFINDBUF4L)((PBYTE)pffbFile + pffbFile->oNextEntryOffset);
	} // for
	if (ulSelCnt) {
	  // One or more entries selected
	  if (stopflag && *stopflag)
	    goto Abort;
	  if (fSyncUpdates) {
	    pciFirst = WinSendMsg(hwndCnr, CM_ALLOCRECORD,
				  MPFROMLONG(EXTRA_RECORD_BYTES),
				  MPFROMLONG(ulSelCnt));
	    if (!pciFirst) {
	      Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
		        GetPString(IDS_CMALLOCRECERRTEXT));
	      ok = FALSE;
	      ullTotalBytes = 0;
	    }
	    else {
	      // 04 Jan 08 SHL fixme like comp.c to handle less than ulSelCnt records
	      pci = pciFirst;
	      ullTotalBytes = 0;
	      // Insert selected in container
	      for (x = 0; x < ulSelCnt; x++) {
		pffbFile = papffbSelected[x];
		ullBytes = FillInRecordFromFFB(hwndCnr, pci, pszFileSpec,
					       pffbFile, partial, dcd);
		pci = (PCNRITEM) pci->rc.preccNextRecord;
		ullTotalBytes += ullBytes;
	      } // for
	      memset(&ri, 0, sizeof(RECORDINSERT));
	      ri.cb = sizeof(RECORDINSERT);
	      ri.pRecordOrder = (PRECORDCORE) CMA_END;
	      ri.pRecordParent = (PRECORDCORE) pciParent;
	      ri.zOrder = (ULONG) CMA_TOP;
	      ri.cRecordsInsert = ulSelCnt;
	      ri.fInvalidateRecord = TRUE;
	       // !fSyncUpdates && dcd && dcd->type == DIR_FRAME ? FALSE : TRUE; //fSyncUpdates always TRUE 12-1-08 GKY
	      if (!WinSendMsg(hwndCnr,
			      CM_INSERTRECORD,
			      MPFROMP(pciFirst), MPFROMP(&ri))) {
		DosSleep(10);		// Give GUI time to work
		WinSetFocus(HWND_DESKTOP, hwndCnr);
		if (!WinSendMsg(hwndCnr,
				CM_INSERTRECORD,
				MPFROMP(pciFirst), MPFROMP(&ri))) {
		  Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
			    GetPString(IDS_CMINSERTERRTEXT));
		  ok = FALSE;
		  ullTotalBytes = 0;
		  if (WinIsWindow((HAB) 0, hwndCnr))
		    FreeCnrItemList(hwndCnr, pciFirst);
		}
	      // }
	      }
	    }
	    if (ok) {
	      ullReturnBytes += ullTotalBytes;
	      ulReturnFiles += ulSelCnt;
	    }
	  } // if sync updates
	  else {
	    // Append newly selected entries to aggregate list
	    paffbTemp = xrealloc(paffbTotal,
				 sizeof(FILEFINDBUF4L) * (ulSelCnt + ulTotal),
				 pszSrcFile, __LINE__);
	    if (paffbTemp) {
	      // 13 Aug 07 SHL fixme to optimize copy
	      paffbTotal = paffbTemp;
	      for (x = 0; x < ulSelCnt; x++)
		paffbTotal[x + ulTotal] = *papffbSelected[x];
	      ulTotal += ulSelCnt;
	    }
	    else {
	      saymsg(MB_ENTER,
		     HWND_DESKTOP,
		     GetPString(IDS_ERRORTEXT), GetPString(IDS_OUTOFMEMORY));
	      break;
	    }
	  }
	} // if entries selected
	if (stopflag && *stopflag)
	  goto Abort;
	DosError(FERR_DISABLEHARDERR);
	ulFindCnt = ulFindMax;
	rc = xDosFindNext(hdir, paffbFound, ulBufBytes, &ulFindCnt, FIL_QUERYEASIZEL);
	priority_normal();
	if (rc)
	  DosError(FERR_DISABLEHARDERR);
      } while (!rc);

      DosFindClose(hdir);
      xfree(paffbFound, pszSrcFile, __LINE__);
      paffbFound = NULL;
      xfree(papffbSelected, pszSrcFile, __LINE__);
      papffbSelected = NULL;

      if (ulTotal && paffbTotal) {

	if (stopflag && *stopflag)
	  goto Abort;

	pciFirst = WinSendMsg(hwndCnr, CM_ALLOCRECORD,
			      MPFROMLONG(EXTRA_RECORD_BYTES), MPFROMLONG(ulTotal));
	if (!pciFirst) {
	  Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
		    GetPString(IDS_CMALLOCRECERRTEXT));
	  ok = FALSE;
	  ullTotalBytes = 0;
	}
	else {
	  // 04 Jan 08 SHL fixme like comp.c to handle less than ulSelCnt records
	  pci = pciFirst;
	  ullTotalBytes = 0;
	  pffbFile = paffbTotal;
	  for (x = 0; x < ulTotal; x++) {
	    ullBytes = FillInRecordFromFFB(hwndCnr, pci, pszFileSpec,
					   pffbFile, partial, dcd);
	    pci = (PCNRITEM) pci->rc.preccNextRecord;
	    ullTotalBytes += ullBytes;
	    // Can not use offset since we have merged lists - this should be equivalent
	    pffbFile = (PFILEFINDBUF4L)((PBYTE)pffbFile + sizeof(FILEFINDBUF4L));
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
	      DosSleep(10);		// Give GUI time to work
	      WinSetFocus(HWND_DESKTOP, hwndCnr);
	      if (!WinSendMsg(hwndCnr, CM_INSERTRECORD,
			      MPFROMP(pciFirst), MPFROMP(&ri))) {
		Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
			  GetPString(IDS_CMINSERTERRTEXT));
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
	  ulReturnFiles += ulFindCnt;
	}
      }
    }

    /**
     * DosFind for subdirectories of a read-only directory on a FAT volume
     * returns path not found if there are no subdirectories
     * FAT FS seems to ignore . and .. in this case
     * Map to no more files
     * We could verify that directory is marked read-only, it's probably not
     * worth the extra code since we do verify 2 out of 3 prerequisites
     * 15 Jan 08 SHL
     */
    if (rc == ERROR_PATH_NOT_FOUND && !filestoo) {
      ULONG ulDriveType = 0;
      CHAR szFSType[CCHMAXPATH];
      INT removable = CheckDrive(*pszFileSpec, szFSType, &ulDriveType);
      if (removable != -1 && strcmp(szFSType, "FAT") == 0)
	rc = ERROR_NO_MORE_FILES;
    }

    if (rc && rc != ERROR_NO_MORE_FILES) {
      Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		GetPString(IDS_CANTFINDDIRTEXT), pszFileSpec);
    }

    if (!fSyncUpdates && dcd && dcd->type == DIR_FRAME)
      WinSendMsg(hwndCnr, CM_INVALIDATERECORD, MPVOID,
		 MPFROM2SHORT(0, CMA_ERASE));
  }
Abort:
  xfree(paffbTotal, pszSrcFile, __LINE__);
  xfree(pszFileSpec, pszSrcFile, __LINE__);
  xfree(paffbFound, pszSrcFile, __LINE__);
  xfree(papffbSelected, pszSrcFile, __LINE__);

  if (recurse) {
    pci = WinSendMsg(hwndCnr, CM_QUERYRECORD, MPFROMP(pciParent),
		     MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER));
    while (pci && (INT)pci != -1) {
      if ((pci->attrFile & FILE_DIRECTORY))
        if (!fInitialDriveScan)
          Stubby(hwndCnr, pci);
        else {
          while (StubbyScanCount != 0)
            DosSleep(50);
          Stubby(hwndCnr, pci);
        }
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
		DIRCNRDATA * dcd,
		PULONGLONG pullTotalBytes)
{
  ProcessDirectory(hwndCnr,
		   (PCNRITEM)NULL,
		   pszDirectory,
		   TRUE,		// filestoo
		   FALSE,		// recurse
                   TRUE,		// partial
                   dcd ? &dcd->stopflag : NULL,
                   dcd,
		   NULL,		// total files
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
  ULONG ulCurDriveNum, ulDriveMap, numtoinsert = 0;
  ULONG ulDriveType;
  PCNRITEM pci, pciFirst = NULL, pciNext, pciParent = NULL;
  INT x, removable;
  CHAR suggest[32];
  CHAR szDrive[] = " :\\";
  CHAR szFSType[CCHMAXPATH]; 
  FILESTATUS4L fsa4;
  APIRET rc;
  BOOL drivesbuilt = FALSE;
  ULONG startdrive = 3;
  static BOOL didonce;

  fDummy = TRUE;
  *suggest = 0;
  for (x = 0; x < 26; x++) {
    driveflags[x] &= (DRIVE_IGNORE | DRIVE_NOPRESCAN | DRIVE_NOLOADICONS |
		      DRIVE_NOLOADSUBJS | DRIVE_NOLOADLONGS |
                      DRIVE_INCLUDEFILES | DRIVE_SLOW | DRIVE_NOSTATS |
                      DRIVE_WRITEVERIFYOFF);
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
              pszSrcFile, __LINE__, PCSZ_FILLDIRQCURERRTEXT);
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
    Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__, GetPString(IDS_CMALLOCRECERRTEXT));
    // 04 Jan 08 SHL fixme not just up and die
    exit(0);
  }

  // 04 Jan 08 SHL fixme like comp.c to handle less than ulSelCnt records
  pci = pciFirst;
  for (x = 0; x < 26; x++) {
    if ((ulDriveMap & (1L << x)) && !(driveflags[x] & DRIVE_IGNORE)) {

      CHAR s[80];
      ULONG flags = 0;
      ULONG size = sizeof(ULONG);
      BOOL FSInfo = FALSE;
      struct {
              ULONG serial;
              CHAR volumelength;
              CHAR volumelabel[CCHMAXPATH];
             } volser;

      *szDrive = (CHAR)x + 'A';		// Build path spec

      sprintf(s, "%c.DriveFlags", toupper(*szDrive));
      if (PrfQueryProfileData(fmprof, appname, s, &flags, &size)) {
	driveflags[toupper(*szDrive) - 'A'] |= flags;
      }

      if (x > 1) {
	// Hard drive (2..N)
	if (!(driveflags[x] & DRIVE_NOPRESCAN)) {
	  *szFSType = 0;
	  ulDriveType = 0;
	  removable = CheckDrive(*szDrive, szFSType, &ulDriveType);
	  driveserial[x] = -1;
          if (removable != -1) {
            DosError(FERR_DISABLEHARDERR);
            if (!DosQueryFSInfo((ULONG) x + 1,
                                FSIL_VOLSER, &volser, sizeof(volser))) {
              driveserial[x] = volser.serial;
              FSInfo = TRUE;
            }

	  }
	  else
	    driveflags[x] |= DRIVE_INVALID;

	  memset(&fsa4, 0, sizeof(FILESTATUS4L));
	  driveflags[x] |= removable == -1 || removable == 1 ? DRIVE_REMOVABLE : 0;
	  if (ulDriveType & DRIVE_REMOTE)
	    driveflags[x] |= DRIVE_REMOTE;
	  if (!stricmp(szFSType,RAMFS)) {
	    driveflags[x] |= DRIVE_RAMDISK;
	    driveflags[x] &= ~DRIVE_REMOTE;
	  }
	  if (!stricmp(szFSType,NDFS32)) {
	    driveflags[x] |= DRIVE_VIRTUAL;
	    driveflags[x] &= ~DRIVE_REMOTE;
	  }
	  if (!stricmp(szFSType,NTFS))
	    driveflags[x] |= DRIVE_NOTWRITEABLE;
	  if (strcmp(szFSType, HPFS) &&
	      strcmp(szFSType, JFS) &&
	      strcmp(szFSType, ISOFS) &&
	      strcmp(szFSType, CDFS) &&
	      strcmp(szFSType, FAT32) &&
	      strcmp(szFSType, NDFS32) &&
	      strcmp(szFSType, RAMFS) &&
              strcmp(szFSType, NTFS) &&
	      strcmp(szFSType, HPFS386)) {
	    driveflags[x] |= DRIVE_NOLONGNAMES;
	  }

	  if (!strcmp(szFSType, CDFS) || !strcmp(szFSType,ISOFS)) {
	    removable = 1;
	    driveflags[x] |= DRIVE_REMOVABLE | DRIVE_NOTWRITEABLE | DRIVE_CDROM;
	  }
	  if (!stricmp(szFSType, CBSIFS)) {
	    driveflags[x] |= DRIVE_ZIPSTREAM;
	    driveflags[x] &= ~DRIVE_REMOTE;
	    if (ulDriveType & DRIVE_REMOVABLE)
	      driveflags[x] |= DRIVE_REMOVABLE;
	    if (!(ulDriveType & DRIVE_NOLONGNAMES))
	      driveflags[x] &= ~DRIVE_NOLONGNAMES;
          }
          if (!fVerifyOffChecked[x]) {
            if (driveflags[x] & DRIVE_REMOVABLE)
              driveflags[x] |= DRIVE_WRITEVERIFYOFF;
            if (!(driveflags[x] & DRIVE_INVALID)) {
              CHAR Key[80];

              sprintf(Key, "%c.VerifyOffChecked", (CHAR) (x + 'A'));
              fVerifyOffChecked[x] = TRUE;
              PrfWriteProfileData(fmprof, appname, Key, &fVerifyOffChecked[x], sizeof(BOOL));
            }
          }
          if (strcmp(volser.volumelabel, NullStr) != 0 && FSInfo && fShowDriveLabelInTree)
            strcpy(szFSType, volser.volumelabel);
	  pci->rc.flRecordAttr |= CRA_RECORDREADONLY;
	  if ((ULONG)(toupper(*szDrive) - '@') == ulCurDriveNum)
	    pci->rc.flRecordAttr |= (CRA_CURSORED | CRA_SELECTED);

	  if (removable == 0) {
	    // Fixed volume
	    pci->attrFile |= FILE_DIRECTORY;
	    DosError(FERR_DISABLEHARDERR);
	    rc = DosQueryPathInfo(szDrive,
				  FIL_QUERYEASIZEL,
				  &fsa4, (ULONG) sizeof(FILESTATUS4L));
	    if (rc == ERROR_BAD_NET_RESP) {
	      DosError(FERR_DISABLEHARDERR);
	      rc = DosQueryPathInfo(szDrive,
				    FIL_STANDARDL,
				    &fsa4, (ULONG) sizeof(FILESTATUS4L));
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
              if (fShowFSTypeInTree || fShowDriveLabelInTree) {
                strcat(szDrive, " [");
                strcat(szDrive, szFSType);
                strcat(szDrive, "]");
              }
	      pci->pszDisplayName = xstrdup(szDrive, pszSrcFile, __LINE__);
              szDrive[3] = 0;
	      pci->rc.pszIcon = pci->pszDisplayName;
	      pci->attrFile = FILE_DIRECTORY;
	      pci->pszDispAttr = FileAttrToString(pci->attrFile);
	      driveserial[x] = -1;
	    }
            else
              FillInRecordFromFSA(hwndCnr, pci, szDrive, &fsa4, TRUE, szFSType, NULL);
	  }
	  else {
            // Removable volume
            pci->pszFileName = xstrdup(szDrive, pszSrcFile, __LINE__);
            if (fShowFSTypeInTree || fShowDriveLabelInTree) {
              strcat(szDrive, " [");
              strcat(szDrive, szFSType);
              strcat(szDrive, "]");
            }
	    pci->pszDisplayName = xstrdup(szDrive, pszSrcFile, __LINE__);
            szDrive[3] = 0;
	    pci->rc.pszIcon = pci->pszDisplayName;
	    pci->attrFile = FILE_DIRECTORY;
	    pci->pszDispAttr = FileAttrToString(pci->attrFile);
	  }
	  SelectDriveIcon(pci);
#	  ifdef FORTIFY
	  // Will be freed by TreeCnrWndProc WM_DESTROY
	  Fortify_SetScope(pci->pszFileName, 2);
#	  endif
	}
	else {
	  *szFSType = 0;
          pci->rc.hptrIcon = hptrDunno;
          pci->pszFileName = xstrdup(szDrive, pszSrcFile, __LINE__);
          if (fShowFSTypeInTree || fShowDriveLabelInTree) {
            strcat(szDrive, " [?]");
          }
	  pci->pszDisplayName = xstrdup(szDrive, pszSrcFile, __LINE__);
          szDrive[3] = 0;
#	  ifdef FORTIFY
	  // Will be freed by TreeCnrWndProc WM_DESTROY
	  Fortify_SetScope(pci->pszFileName, 2);
#	  endif
	  pci->rc.pszIcon = pci->pszDisplayName;
	  pci->attrFile = FILE_DIRECTORY;
	  pci->pszDispAttr = FileAttrToString(pci->attrFile);
	  driveserial[x] = -1;
	}
	}
      else {
	// diskette drive (A or B)
        pci->rc.hptrIcon = hptrFloppy;
        pci->pszFileName = xstrdup(szDrive, pszSrcFile, __LINE__);
        if (fShowFSTypeInTree || fShowDriveLabelInTree)
          strcat(szDrive, "  [Floppy]");
	pci->pszDisplayName = xstrdup(szDrive, pszSrcFile, __LINE__);
        szDrive[3] = 0;
	pci->rc.pszIcon = pci->pszDisplayName;
	pci->attrFile = FILE_DIRECTORY;
	pci->pszDispAttr = FileAttrToString(pci->attrFile);
	driveflags[x] |= (DRIVE_REMOVABLE | DRIVE_NOLONGNAMES);
	driveserial[x] = -1;
      }
      pci->rc.flRecordAttr |= CRA_RECORDREADONLY;
      pci = (PCNRITEM) pci->rc.preccNextRecord;	// next rec
    }
    else if (!(ulDriveMap & (1L << x)))
      driveflags[x] |= DRIVE_INVALID;
  } // for drives

  PostMsg(hwndMain, UM_BUILDDRIVEBAR, MPVOID, MPVOID);
  drivesbuilt = TRUE;

  // insert the drives
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
      Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
	        GetPString(IDS_CMINSERTERRTEXT));
    }
  }

  // move cursor onto the default drive rather than the first drive
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

	p = pszTreeEnvVarList;
	while (*p == ';')
	  p++;
	while (*p) {
	  *szFSType = 0;
	  pp = szFSType;
	  while (*p && *p != ';')
	    *pp++ = *p++;
	  *pp = 0;
	  while (*p == ';')
	    p++;
	  if (*szFSType &&
	      (!stricmp(szFSType, PCSZ_LIBPATH) || getenv(szFSType))) {
	    pci = WinSendMsg(hwndCnr,
			     CM_ALLOCRECORD,
			     MPFROMLONG(EXTRA_RECORD_BYTES),
			     MPFROMLONG(1));
	    if (pci) {
	      CHAR fname[CCHMAXPATH];
	      pci->flags |= RECFLAGS_ENV;
	      sprintf(fname, "%%%s%%", szFSType);
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
		Win_Error(hwndCnr, HWND_DESKTOP, pszSrcFile, __LINE__,
			  GetPString(IDS_CMINSERTERRTEXT));
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
  {
    STUBBYSCAN *stubbyScan;
    HWND hwndDrivesList = WinWindowFromID(WinQueryWindow(hwndParent, QW_PARENT),
					  MAIN_DRIVELIST);

    pci = (PCNRITEM) WinSendMsg(hwndCnr,
				CM_QUERYRECORD,
				MPVOID,
                                MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
    StubbyScanCount ++;
    while (pci && (INT)pci != -1) {
      stubbyScan = xmallocz(sizeof(STUBBYSCAN), pszSrcFile, __LINE__);
      if (!stubbyScan)
	break;
      stubbyScan->pci = pci;
      stubbyScan->hwndCnr = hwndCnr;
      stubbyScan->hwndDrivesList = hwndDrivesList;
      //stubbyScan->RamDrive = FALSE;
      pciNext = (PCNRITEM) WinSendMsg(hwndCnr,
				      CM_QUERYRECORD,
				      MPFROMP(pci),
				      MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
      if (~pci->flags & RECFLAGS_ENV) {
	ULONG drvNum = toupper(*pci->pszFileName) - 'A';	// 0..25
	if (drvNum == ulCurDriveNum || drvNum >= 2) {
	  ULONG flags = driveflags[drvNum];	// Speed up
	  if (~flags & DRIVE_INVALID &&
	      ~flags & DRIVE_NOPRESCAN &&
	      (!fNoRemovableScan || ~flags & DRIVE_REMOVABLE))
	  {
	    if (xbeginthread(StubbyScanThread,
			     65536,
			     stubbyScan,
			     pszSrcFile,
			     __LINE__) == -1)
	    {
	      xfree(stubbyScan, pszSrcFile, __LINE__);
	    }
	  } // if drive for scanning
	  else
	    WinSendMsg(hwndDrivesList,
		       LM_INSERTITEM,
		       MPFROM2SHORT(LIT_SORTASCENDING, 0),
		       MPFROMP(pci->pszFileName));
	}
	else {
	  WinSendMsg(hwndCnr,
		     CM_INVALIDATERECORD,
		     MPFROMP(&pci),
		     MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
	  WinSendMsg(hwndDrivesList,
		     LM_INSERTITEM,
		     MPFROM2SHORT(LIT_SORTASCENDING, 0),
		     MPFROMP(pci->pszFileName));
	}
      }
      pci = pciNext;
    } // while
    StubbyScanCount--;
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
  DosSleep(16);				// 05 Aug 07 GKY 33
  fDummy = FALSE;
  DosPostEventSem(CompactSem);

  {
    BYTE info;
    BOOL includesyours = FALSE;

    // 10 Jan 08 SHL fixme to understand fFirstTime
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
  if (fInitialDriveScan) {
    HWND hwndDrivesList = WinWindowFromID(WinQueryWindow(hwndParent, QW_PARENT),
					  MAIN_DRIVELIST);
    while (StubbyScanCount != 0 || ProcessDirCount != 0)
      DosSleep(50);
    WinShowWindow(hwndCnr, TRUE);
    WinShowWindow(hwndDrivesList, TRUE);
    fInitialDriveScan = FALSE;
  }
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
  if (pci->pszSubject) {
    if (pci->pszSubject != NullStr)
      free(pci->pszSubject);
    pci->pszSubject = NULL;		// Catch illegal references
  }

  // 08 Sep 08 SHL Remove excess logic
  if (pci->pszLongName) {
    if (pci->pszLongName != NullStr)
      free(pci->pszLongName);
    pci->pszLongName = NULL;		// Catch illegal references
  }

  // Bypass free if pszDisplayName points into pszFileName buffer
  // 05 Sep 08 SHL Correct pointer overlap compare logic
  if (pci->pszDisplayName) {
    if (pci->pszDisplayName != NullStr) {
      if (!pci->pszFileName ||
	  pci->pszDisplayName < pci->pszFileName ||
	  pci->pszDisplayName >= pci->pszFileName + _msize(pci->pszFileName))
      {
	free(pci->pszDisplayName);
      }
    }
    pci->pszDisplayName = NULL;		// Catch illegal references
  }

  if (!pci->pszFileName)
    Runtime_Error(pszSrcFile, __LINE__, "FreeCnrItemData attempting free %p data twice", pci);
  else {
    if (pci->pszFileName != NullStr)
      free(pci->pszFileName);
    pci->pszFileName = NULL;		// Catch illegal references
  }

  // 08 Sep 08 SHL Remove excess logic
  if (pci->pszLongName) {
    if (pci->pszLongName != NullStr)
      free(pci->pszLongName);
    pci->pszLongName = NULL;		// Catch illegal references
  }

  if (pci->pszFmtFileSize) {
    if (pci->pszFmtFileSize != NullStr)
      free(pci->pszFmtFileSize);
    pci->pszFmtFileSize = NULL;		// Catch illegal references
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
	// 12 Sep 07 SHL dwg drivebar crash testing - ticket# ???
	static PCNRITEM pciLast;	// 12 Sep 07 SHL
	ULONG ulSize = sizeof(*pci);
	ULONG ulAttr;
	APIRET apiret = DosQueryMem((PVOID)pci, &ulSize, &ulAttr);
	if (apiret)
	  Dos_Error(MB_ENTER, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
		    "DosQueryMem failed pci %p pciLast %p", pci, pciLast);
	FreeCnrItemData(pci);
	pciLast = pci;
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
      Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,"CM_REMOVERECORD hwnd %x pci %p cnt %u", hwnd, pciFirst, usCnt);
    }
  }

  return remaining;
}

#pragma alloc_text(FILLDIR,FillInRecordFromFFB,FillInRecordFromFSA,IDFile)
#pragma alloc_text(FILLDIR1,ProcessDirectory,FillDirCnr,FillTreeCnr,FileAttrToString,StubbyScanThread)
#pragma alloc_text(EMPTYCNR,EmptyCnr,FreeCnrItemData,FreeCnrItem,FreeCnrItemList,RemoveCnrItems)

