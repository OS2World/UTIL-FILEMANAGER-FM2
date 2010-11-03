
/***********************************************************************

  $Id$

  filldir.c definitions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Split from fm3dll.h
  25 Dec 08 GKY Add ProcessDirectoryThread to allow optional recursive drive scan at startup.
  08 Mar 09 GKY Additional strings move to PCSZs
  06 Jun 09 GKY Add option to show file system type or drive label in tree
  22 Jul 09 GKY Code changes to use semaphores to serialize drive scanning

***********************************************************************/

#if !defined(FILLDIR_H)

#define FILLDIR_H

#if !defined(OS2_INCLUDED)
#define INCL_WINSTDCNR			// dircnrs.h
#include <os2.h>
#else
#if !defined(INCL_WINSTDCNR)
#error INCL_WINSTDCNR required
#endif
#endif

#include "dircnrs.h"			// DIRCNRDATA

VOID EmptyCnr(HWND hwnd);
const PSZ FileAttrToString(ULONG fileAttr);
VOID FillDirCnr(HWND hwndCnr, CHAR *pszDirectory, DIRCNRDATA *pdcd,
		PULONGLONG pullBytes);
VOID FillTreeCnr(HWND hwndCnr, HWND hwndParent);
VOID ProcessDirectory(const HWND hwndCnr, const PCNRITEM pciParent,
		      const CHAR *szDirBase, const BOOL filestoo,
		      const BOOL recurse, const BOOL partial,
		      CHAR *stopflag, DIRCNRDATA *pdcd,
		      PULONG pullTotalFiles, PULONGLONG pullTotalBytes);
ULONGLONG FillInRecordFromFFB(HWND hwndCnr, PCNRITEM pci,
			      const PSZ pszDirectory,
			      const PFILEFINDBUF4L pffb, const BOOL partial,
			      DIRCNRDATA *pdcd);
ULONGLONG FillInRecordFromFSA(HWND hwndCnr, PCNRITEM pci,
			      const PSZ pszFileName, const PFILESTATUS4L pfsa4,
			      const BOOL partial, CHAR *szFSType, DIRCNRDATA *pdcd);
VOID FreeCnrItem(HWND hwnd, PCNRITEM pci);
VOID FreeCnrItemList(HWND hwnd, PCNRITEM pciFirst);
VOID FreeCnrItemData(PCNRITEM pci);
INT RemoveCnrItems(HWND hwnd, PCNRITEM pci, USHORT usCnt, USHORT usFlags);
VOID ProcessDirectoryThread(VOID * arg);

// Data declarations
extern PCSZ FM3Tools;
extern PCSZ WPProgram;
extern HPOINTER hptrEnv;
extern HPOINTER hptrHidden;
extern HPOINTER hptrReadonly;
extern HPOINTER hptrSystem;
extern INT FixedVolume;

typedef struct {

  HWND     hwndCnr;
  PCNRITEM pciParent;
  CHAR     *szDirBase;
  BOOL     filestoo;
  BOOL     recurse;
  BOOL     partial;
  CHAR           *stopflag;
  DIRCNRDATA     *dcd;	              // Optional
  ULONG          *pulTotalFiles;       // Optional
  PULONGLONG     pullTotalBytes;      // Optional
}
PROCESSDIR;

#endif // FILLDIR_H
