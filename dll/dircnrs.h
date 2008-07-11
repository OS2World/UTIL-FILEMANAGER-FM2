
/***********************************************************************

  $Id: $

  dircnrs common definitions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Move dircnrs.c definitions here
  13 Jan 08 GKY Add variables to DIRCNRDATA struct for Subjectwidth/Subjectleft.
  11 Jul 08 JBS Ticket 230: Simplified code and eliminated some local variables by incorporating
                all the details view settings (both the global variables and those in the
                DIRCNRDATA struct) into a new struct: DETAILS_SETTINGS.

***********************************************************************/

#if !defined(DIRCNRS_H)

#define DIRCNRS_H

#if !defined(OS2_INCLUDED)
#define INCL_WINSTDCNR			// avl.h
#define INCL_LONGLONG
#include <os2.h>
#else
#if !defined(INCL_WINSTDCNR)
#error INCL_WINSTDCNR required by dircnrs.h
#endif
#if !defined(INCL_LONGLONG)
#error INCL_LONGLONG required
#endif
#endif

#if defined(__IBMC__)
#if __IBMC__ != 430
#error VAC365 required for long long support
#endif
#if !defined(_LONG_LONG)
#error Long long support not enabled
#endif
#endif

#include "avl.h"			// ARC_TYPE

#define CBLIST_TO_EASIZE(cb) ((cb) > 4 ? (cb) / 2 : 0)	// FILEFINDBUF4L.cbList to logical EA size

typedef struct _CNRITEM
{				/* CONTAINER RECORD STRUCTURE */
  MINIRECORDCORE rc;		/* Base information */
  HWND hwndCnr;			/* The container holding this record */
  PSZ pszFileName;		// Points to buffer holding full pathname or NullStr
  PSZ pszDisplayName;		// Points to displayable part of path name  - used by CFA_STRING
  CHAR *pszSubject;		// Points subject buffer or Nullstr - used by fm/2 and by CFA_STRING
  CHAR *pszDispAttr;		// Points to szDispAttr - required by CFA_STRING
  CDATE date;			/* Last write date of file */
  CTIME time;			/* Last write time of file */
  CDATE ladate;			/* Last access date of file */
  CTIME latime;			/* Last access time of file */
  CDATE crdate;			/* Creation date of file */
  CTIME crtime;			/* Creation time of file */
  CHAR *pszLongName;		// Points to long name buffer - used by code and by CFA_STRING
  CHAR *pszFmtFileSize;         // Comma formatted file size for large file support
  ULONGLONG cbFile;		/* File size */
  ULONGLONG easize;		// Size of EAs - dirsize uses this - hack cough
  ULONG attrFile;		/* Attributes of this file */
  ULONG flags;
}
CNRITEM, *PCNRITEM;

typedef struct
{
  CHAR szMask[CCHMAXPATH];
  CHAR szMaskCopy[CCHMAXPATH];
  CHAR *pszMasks[26];
  ULONG attrFile;
  ULONG antiattr;
  BOOL fNoAttribs;
  BOOL fShowDirs;
  BOOL fNoDirs;
  BOOL fIsTree;
  BOOL fIsSeeAll;
  BOOL fFilesIncluded;
  BOOL fText;
  CHAR szText[256];
  CHAR prompt[80];
}
MASK;

typedef struct DETAILS_SETTINGS
{
  BOOL detailsladate, detailslatime, detailscrdate, detailscrtime,
    detailslongname, detailsea, detailssize, detailssubject,
    detailslwdate, detailslwtime, detailsattr, detailsicon,
    fSubjectInLeftPane, fSubjectLengthMax;
  ULONG SubjectDisplayWidth;
}
DETAILS_SETTINGS;

typedef struct DIRCNRDATA
{
  USHORT size;
  USHORT id;
  INT type;
  ULONG flWindowAttr;
  HWND hwndParent;
  HWND hwndCnr;
  HWND hwndObject;
  HWND hwndFrame;
  HWND hwndClient;
  HWND hwndLastMenu;
  HWND hwndExtract;
  HWND hwndLastDirCnr;
  HWND hwndRestore;
  CHAR directory[CCHMAXPATH];
  CHAR previous[CCHMAXPATH];
  ULONG fg, bg, hifg, hibg, border;
  PFNWP oldproc;
  CHAR font[CCHMAXPATH];
  MASK mask;
  ULONGLONG ullTotalBytes;
  ULONGLONG selectedbytes;
  ULONG selectedfiles;
  ULONG totalfiles;
  BOOL cnremphasized;
  BOOL dontclose;
  ARC_TYPE *info;
  CHAR arcname[CCHMAXPATH];
  CHAR command[257];
  CHAR stopflag;
  CHAR workdir[CCHMAXPATH];
  CHAR lastfilename[CCHMAXPATH];
  BOOL namecanchange;
  BOOL fmoving;
  BOOL amextracted;
  INT lasthelp;
  INT sortFlags;
  DETAILS_SETTINGS ds;
  CHAR **lastselection;
  USHORT shiftstate;
  USHORT suspendview;
  CHAR szCommonName[CCHMAXPATH];
  ULONG lasttime;
  BOOL arcfilled;
  HMTX filling;
  BOOL firsttree;
  ULONG lastattr;
  ULONG ulItemsToUnHilite;
}
DIRCNRDATA;

MRESULT EXPENTRY DirClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
HWND StartDirCnr(HWND hwndParent, CHAR * directory, HWND hwndRestore,
		 ULONG flags);
MRESULT EXPENTRY DirTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DirFolderProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DirMaxProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DirObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

// 05 Jan 08 SHL fixme for dircnrs.c globals to be here

#ifdef DEFINE_GLOBALS
#define DATADEF
#pragma data_seg(GLOBAL1)
#else
#define DATADEF extern
#endif

#endif // DIRCNRS_H
