
/***********************************************************************

  $Id$

  Common definitions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  12 Feb 03 SHL Add CBLIST_TO_EASIZE
  11 Jun 03 SHL Add JFS and FAT32 support
  06 Jan 04 SHL Drop hundfmt
  01 Aug 04 SHL Optimze strippers
  01 Aug 04 SHL Drop avv local functions
  23 May 05 SHL Split datamin to datamin.h
  24 May 05 SHL Rework Win_Error usage
  25 May 05 SHL Require VAC 3.65
  25 May 05 SHL Rename comnam to szCommonName
  25 May 05 SHL Rework for FillInRecordFromFFB
  25 May 05 SHL Add CommaFmtULL CommaFmtUL
  28 May 05 SHL Drop local functions
  06 Jun 05 SHL Use QWL_USER
  11 Aug 05 SHL Renames
  29 May 06 SHL Rework EditArchiverDefinition
  16 Jun 06 SHL ARC_TYPE: support non-string signatures
  26 Jun 06 SHL ARC_TYPE: support preserving comments
  28 Jun 06 SHL DIRCNRDATA: drop unused
  05 Jul 06 SHL Support Hide not selected
  13 Jul 06 SHL Add Runtime_Error
  22 Jul 06 SHL Add memory.c functions
  26 Jul 06 SHL Add strips.c functions
  26 Jul 06 SHL Add more error.c functions
  29 Jul 06 SHL Add xfgets, xfgets_bstripcr
  22 Oct 06 GKY Add NDFS32 support
  03 Nov 06 SHL Renames
  17 Jan 07 SHL Resize extractdir
  17 Feb 07 GKY Add more file system types
  17 Feb 07 GKY Add SelectDriveIcon to streamline update.c
  18 Mar 07 GKY Add MM import typedefines for fix for files misindentified as multimedia
  30 Mar 07 GKY Defined golbals for removing GetPString for window class names
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits NumItemsToUnhilite & DeleteDragitemStrHandles
  06 Apr 07 GKY Add some error checking in drag/drop
  19 Apr 07 SHL Rework DeleteDragitemStrHandles to be FreeDragInfoData
  19 Apr 07 SHL Add DbgMsg.  Sync with AcceptOneDrop GetOneDrop mods.
  21 Apr 07 GKY Find FM2Utils by path or utils directory eliminate fAddUtils global
  23 Apr 07 SHL Add Win_Error_NoMsgBox
  12 May 07 SHL Add ulItemsToUnHilite to DIRCNRDATA, pass to Unhilite as arg
  05 Jun 07 SHL Update for OpenWatcom
  10 Jun 07 GKY Add CheckPmDrgLimit including IsFm2Window as part of work around PM drag limit
  16 Jun 07 GKY Add CheckPmDrgLimit including IsFm2Window as part of work around PM drag limit
  16 Jun 07 SHL Update more for OpenWatcom
  22 Jul 07 GKY Update CNRITEM to optimize RAM usage
  23 Jul 07 SHL More CNRITEM updates (ticket#24)
  01 Aug 07 SHL More CNRITEM and ARCITEM updates (ticket#24)
  14 Aug 07 SHL Add GetMSecTimer
  14 Aug 07 SHL Delete obsoletes
  16 Aug 07 SHL Update for ticket# 109 - status update
  18 Aug 07 SHL Update for ticket# 31 - states length
  19 Aug 07 SHL Move FILESTOGET_MIN/MAX here
  19 Aug 07 SHL Update SaveDirCnrState return
  21 Aug 07 GKY Make Subject column in dircnr sizable and movable from the right to the left pane
  01 Sep 07 GKY Add xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundary
  04 Nov 07 GKY Add pszFmtFileSize to CNRITEM to display large file sizes
  10 Nov 07 GKY Add ThousandSeparator variable for file sizes NLS tseparator.
  22 Nov 07 GKY Use CopyPresParams in CheckMenu to fix presparam inconsistencies in menus
  17 Dec 07 GKY Add variables for using WPURLDEFAULTSETTINGS as the fall back for ftphttprun
  29 Dec 07 GKY Add remove_first_occurence_of_character
  30 Dec 07 GKY Change TestDates to TestFDates can compare by filename or FDATE/FTIME data
  30 Dec 07 GKY Add TestCDates to compare CNRITEMs by CDATE/CTIME data
  04 Jan 08 SHL Allow standalone usage
  05 Jan 08 SHL Move comp.c definitions to comp.h
  05 Jan 08 SHL Move dircnr.c definitions to dircnr.h
  05 Jan 08 SHL Move makelist.c definitions to makelist.h
  05 Jan 08 SHL Move error.c definitions to errutil.h
  05 Jan 08 SHL Move string.c definitions to strutil.h
  10 Jan 08 SHL Add UM_FIRSTTIME
  12 Jan 08 SHL Localize SpecialSelect to comp.c
  14 Feb 08 SHL Refactor CfgDlgProc to notebook.h
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  22 Jun 08 GKY Changed some variable types to fix compiler warnings
  11 Jul 08 JBS Ticket 230: Simplified code and eliminated some local variables by incorporating
                all the details view settings (both the global variables and those in the
                DIRCNRDATA struct) into a new struct: DETAILS_SETTINGS.
  16 JUL 08 GKY Use TMP directory for temp files
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory and use MakeTempName; Remove LISTTEMPROOT
  xx Sep 08 JBS Ticket 187 (Refactor fm2dll.h): All function declarations moved to other include files.

***********************************************************************/

#if !defined(FM3DLL_H)

#define FM3DLL_H

#include <stdio.h>			// FILE
#include <time.h>			// time_t

#if !defined(OS2_INCLUDED)
#define INCL_WINSTDDRAG
#define INCL_WINSTDCNR
#define INCL_WINPROGRAMLIST
#define INCL_WINHELP
#define INCL_LONGLONG			// 05 Jan 08 SHL fixme to be gone eventually
#include <os2.h>
#else
#if !defined(INCL_WINSTDDRAG)
#error INCL_WINSTDDRAG required
#endif
#if !defined(INCL_WINSTDCNR)
#error INCL_WINSTDCNR required
#endif
#if !defined(INCL_WINPROGRAMLIST)
#error INCL_WINPROGRAMLIST required
#endif
#if !defined(INCL_WINHELP)
#error INCL_WINHELP required
#endif
#if !defined(INCL_LONGLONG)
#error INCL_LONGLONG required
#endif
#endif // OS2_INCLUDED

#if defined(__IBMC__)
#if __IBMC__ != 430
#error VAC365 required for long long support
#endif
#if !defined(_LONG_LONG)
#error Long long support not enabled
#endif
#endif

#include "dircnrs.h"	// 05 Jan 08 SHL fixme to be gone when DIRCNRDATA refs gone
#include "makelist.h"	// 05 Jan 08 SHL fixme to be gone when LISTINFO refs gone
#include "command.h"    // 01 Mar 08 GKY fixme to be gone when LINKCMDS refs gone

#ifdef DEFINE_GLOBALS
#pragma data_seg(GLOBAL1)
#endif

#define PP_MAX    PP_MENUDISABLEBGNDCOLORINDEX
#define PP_MAXBUF 384

#ifndef MM_PORTHOLEINIT
#define MM_PORTHOLEINIT   0x01fb
#endif
#ifndef MS_POPUP
#define MS_POPUP          0x00000010L
#endif
#ifndef CCS_MINIICONS
#define CCS_MINIICONS     0x0800
#endif
#ifndef CRA_SOURCE
#define CRA_SOURCE        0x00004000
#endif
#ifndef CV_EXACTMATCH
#define CV_EXACTMATCH     0x10000000
#endif
#ifndef CBN_SETFOCUS
#define CBN_SETFOCUS      20
#endif
#ifndef CBN_KILLFOCUS
#define CBN_KILLFOCUS     21
#endif
#ifndef CN_VERIFYEDIT
#define CN_VERIFYEDIT     134
#endif
#ifndef CN_PICKUP
#define CN_PICKUP         135
#endif
#ifndef CN_DROPNOTIFY
#define CN_DROPNOTIFY     136
#endif
#ifndef CN_GRIDRESIZED
#define CN_GRIDRESIZED    137
#endif
#ifndef BKS_MERLINSTYLE
#define BKS_MERLINSTYLE   0x0800
#endif

// PMBITMAP_INCLUDED - IBM Toolkit
// INCL_GPIBITMAPS - OpenWatcom Toolkit
#if !defined(PMBITMAP_INCLUDED) && !defined(INCL_GPIBITMAPS)
typedef struct _RGB2		/* rgb2 */
{
  BYTE bBlue;			/* Blue component of the color definition */
  BYTE bGreen;			/* Green component of the color definition */
  BYTE bRed;			/* Red component of the color definition  */
  BYTE fcOptions;		/* Reserved, must be zero                 */
}
RGB2;
typedef RGB2 *PRGB2;
#endif

#define LINES_PER_ARCSIG        21	// Lines in each archiver.bb2 definition
#define CON_COLS                6
#define INSTDATA(h)             WinQueryWindowPtr((h),QWL_USER)
#define DIR_SPLITBAR_OFFSET     18 * 12	/* Pixel offset of details splitbar */
#define CONTAINER_COLUMNS       13	/* Number of columns in details view */
#define RGBFROMPARTS(r,g,b)     (((r) * 65536) + ((g) * 256) + (b))

#define EXTRA_RECORD_BYTES      (sizeof(CNRITEM) - sizeof(MINIRECORDCORE))
#define EXTRA_ARCRECORD_BYTES   (sizeof(ARCITEM) - sizeof(MINIRECORDCORE))

#define ALLATTRS                (FILE_NORMAL | FILE_DIRECTORY | FILE_ARCHIVED |\
				 FILE_HIDDEN | FILE_READONLY | FILE_SYSTEM)
//#define LISTTEMPROOT            "$FM2LI$T"

#include "fm3dll2.h"

#define UM_PAINT            (WM_USER)
#define UM_SETUP            (WM_USER + 1)
#define UM_RESCAN           (WM_USER + 2)
#define UM_INITIALSIZE      (WM_USER + 3)
#define UM_CONTROL          (WM_USER + 4)
#define UM_COMMAND          (WM_USER + 5)
#define UM_SIZE             (WM_USER + 6)
#define UM_FOCUSME          (WM_USER + 7)
#define UM_FIXEDITNAME      (WM_USER + 8)
#define UM_UPDATERECORD     (WM_USER + 9)
#define UM_SETDIR           (WM_USER + 10)
#define UM_CONTAINER_FILLED (WM_USER + 11)
#define UM_STRETCH          (WM_USER + 12)
#define UM_LOADFILE         (WM_USER + 13)
#define UM_MOUSEMOVE        (WM_USER + 14)
#define UM_ENTER            (WM_USER + 15)
#define UM_CLOSE            (WM_USER + 16)
#define UM_ACTION           (WM_USER + 17)
#define UM_MASSACTION       (WM_USER + 18)
#define UM_UPDATERECORDLIST (WM_USER + 19)
#define UM_FILESMENU        (WM_USER + 20)
#define UM_SELECT           (WM_USER + 21)
#define UM_VIEWSMENU        (WM_USER + 22)
#define UM_CONTAINERHWND    (WM_USER + 23)
#define UM_OPENWINDOWFORME  (WM_USER + 24)
#define UM_FOLDUP           (WM_USER + 25)
#define UM_INITMENU         (WM_USER + 26)
#define UM_COMPARE          (WM_USER + 27)
#define UM_EXPAND           (WM_USER + 28)
#define UM_REPLACEFOCUS     (WM_USER + 29)
#define UM_UNDO             (WM_USER + 30)
#define UM_RENDER           (WM_USER + 31)
#define UM_BUTTON2DOWN      (WM_USER + 32)
#define UM_BUTTON2UP        (WM_USER + 33)
#define UM_COLLECTFROMFILE  (WM_USER + 34)
#define UM_TIMER            (WM_USER + 35)
#define UM_HELPON           (WM_USER + 36)
#define UM_SETUP2           (WM_USER + 37)
#define UM_SETUP3           (WM_USER + 38)
#define UM_CONTEXTMENU      (WM_USER + 39)
#define UM_FILLUSERLIST     (WM_USER + 40)
#define UM_CONTAINERDIR     (WM_USER + 41)
#define UM_SETUP4           (WM_USER + 42)
#define UM_FILLSETUPLIST    (WM_USER + 43)
#define UM_ARRANGEICONS     (WM_USER + 44)
#define UM_SETUP5           (WM_USER + 45)
#define UM_NOTIFY           (WM_USER + 46)
#define UM_INSERTRECORD     (WM_USER + 47)
#define UM_ADDTOMENU        (WM_USER + 48)
#define UM_COLLECT          (WM_USER + 49)
#define UM_RESTOREDC        (WM_USER + 50)
#define UM_MINIMIZE         (WM_USER + 51)
#define UM_MAXIMIZE         (WM_USER + 52)
#define UM_BUTTON1MOTIONSTART (WM_USER + 53)
#define UM_SETUP6           (WM_USER + 54)
#define UM_FILLBUTTONLIST   (WM_USER + 55)
#define UM_SETUSERLISTNAME  (WM_USER + 56)
#define UM_FILTER           (WM_USER + 57)
#define UM_SORTRECORD       (WM_USER + 58)
#define UM_SIZE2            (WM_USER + 59)
#define UM_RESTORE          (WM_USER + 60)
#define UM_TOPDIR           (WM_USER + 61)
#define UM_SHOWME           (WM_USER + 62)
#define UM_RESCAN2          (WM_USER + 63)
#define UM_BUILDDRIVEBAR    (WM_USER + 64)
#define UM_THREADUSE        (WM_USER + 65)
#define UM_DRIVECMD         (WM_USER + 66)
#define UM_ADVISEFOCUS      (WM_USER + 67)
#define UM_FIXCNRMLE        (WM_USER + 68)
#define UM_FLESH            (WM_USER + 69)
#define UM_FILLCMDLIST      (WM_USER + 70)
#define UM_CLICKED          (WM_USER + 71)
#define UM_CLICKED3         (WM_USER + 72)
#define UM_HIDENOTSELECTED  (WM_USER + 73)
#define UM_FIRSTTIME	    (WM_USER + 74)

typedef struct
{
  USHORT size;
  USHORT dummy;
  CHAR szCurrentPath1[CCHMAXPATH];
  CHAR szCurrentPath2[CCHMAXPATH];
}
WALK2;

typedef struct LINKDIRS
{
  CHAR *path;
  struct LINKDIRS *next;
}
LINKDIRS;

typedef struct
{
  USHORT size;
  USHORT numcolors;
  USHORT flags;
  USHORT currentcolor;
  ULONG prompt;
  long *colors;
  ULONG descriptions;
  long *origs;
}
COLORS;

typedef struct
{
  CHAR *title;			/* title of dialog */
  CHAR *prompt;			/* prompt to user */
  CHAR *ret;			/* buffer out, default in */
  CHAR *help;			/* help text */
  INT inputlen;			/* max len of ret */
}
STRINGINPARMS;

typedef struct
{
  CHAR *source;
  CHAR target[CCHMAXPATH];
  BOOL rename;
  BOOL skip;
  BOOL dontask;
  BOOL overold;
  BOOL overnew;
  BOOL overwrite;
}
MOVEIT;

typedef struct HOLDFEA
{
  PFEA2 pfea;
  CHAR *name;
  CHAR *value;
  BYTE fEA;
  BYTE cbName;
  USHORT cbValue;
  struct HOLDFEA *next;
}
HOLDFEA;

typedef struct
{
  USHORT size;
  USHORT dummy;
  CHAR directory[CCHMAXPATH];
  HWND hwndParent;
  HWND hwndFrame;
  HWND hwndClient;
  HWND hwndCnr;
  LISTINFO *li;
}
WORKER;

typedef struct
{
  USHORT size;
  ULONG flags;
  ULONG cmd;
  USHORT dummy;
  CHAR *prompt;
  CHAR **list;
}
CHECKLIST;

typedef struct
{
  ULONG flags;
  CHAR *commandline;
  CHAR path[CCHMAXPATH];
  CHAR environment[1001];
  CHAR tempprompt[128];
  CHAR title[80];
  BOOL dropped;
}
EXECARGS;

#pragma pack(1)

// Compare directory flags
#define CNRITEM_SMALLER   0x00010000	// file exists in both containers and this one is smaller
#define CNRITEM_LARGER    0x00020000
#define CNRITEM_NEWER     0x00040000
#define CNRITEM_OLDER     0x00080000
#define CNRITEM_EXISTS    0x00100000	// file exists in both containers

#define RECFLAGS_ENV      0x00000001
#define RECFLAGS_NODRAG   0x00000002
#define RECFLAGS_NODROP   0x00000004
#define RECFLAGS_UNDERENV 0x00000008

#define ARCFLAGS_REALDIR    0x00000001
#define ARCFLAGS_PSEUDODIR  0x00000002

typedef struct _ARCITEM
{				// ARCHIVE CONTAINER RECORD STRUCTURE
  MINIRECORDCORE rc;		// Base information
  HWND hwndCnr;			/* Container holding this record */
  PSZ pszFileName;		// Points to full path name or NullStr
  PSZ pszDisplayName;		// Points to displayable part of path name  - used by CFA_STRING
  CHAR szDate[40];		// File's assembled date
  PSZ pszDate;			// Pointer to date
  CDATE date;			// if we know date format
  CTIME time;			// if we know time format
  ULONGLONG cbFile;		// File's original size
  ULONGLONG cbComp;		// File's compressed size
  ULONG flags;
}
ARCITEM, *PARCITEM;

#pragma pack()

typedef struct
{
  USHORT size;
  ARC_TYPE *info;
  CHAR *arcname;
  CHAR masks[257];
  CHAR command[257];
  CHAR extractdir[CCHMAXPATH];
  INT ret;
}
EXTRDATA;

#define COPY 0
#define MOVE 1
#define WPSCOPY 2
#define WPSMOVE 4

/* systemf.c */
// #if defined(__IBMC__)
// /* fsopen.c */
// FILE *_fsopen(CHAR * filename, CHAR * mode, INT sharemode, ...);
// #endif

//=====================================================================


#define FILESTOGET_MIN  256
#define FILESTOGET_MAX  4096


#ifdef INCL_MMIOOS2
#pragma pack(4)
/* definitions for MMPM/2 imports */
typedef DWORD(APIENTRY MMIOIDENTIFYFILE) (PSZ, PMMIOINFO, PMMFORMATINFO,
					  PFOURCC, DWORD, DWORD);
typedef MMIOIDENTIFYFILE *PMMIOIDENTIFYFILE;
typedef DWORD(APIENTRY MMIOOPEN)( PSZ, PMMIOINFO, DWORD);
typedef MMIOOPEN *PMMIOOPEN;
typedef WORD (APIENTRY MMIOGETINFO)( HMMIO, PMMIOINFO, WORD);
typedef MMIOGETINFO *PMMIOGETINFO;
typedef WORD (APIENTRY MMIOCLOSE)( HMMIO, WORD);
typedef MMIOCLOSE *PMMIOCLOSE;

#pragma pack()
#endif


#define priority_idle()     DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME,30L,0L)
#define priority_normal()   DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,0L,0L)
#define priority_tweaked()  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,1L,0L)
#define priority_bumped()   DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,3L,0L)
#define priority_critical() DosSetPriority(PRTYS_THREAD,PRTYC_FOREGROUNDSERVER,2L,0L)
#define priority_max()      DosSetPriority(PRTYS_THREAD,PRTYC_FOREGROUNDSERVER,31L,0L)
#define SysVal(value)       WinQuerySysValue(HWND_DESKTOP, (value))

#endif // FM3DLL_H
