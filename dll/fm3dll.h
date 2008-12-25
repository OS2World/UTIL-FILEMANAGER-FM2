
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
  xx Sep 08 JBS Ticket 187 (Refactor fm2dll.h): All function declarations and data
                definitions/declarations moved to other include files.

***********************************************************************/

#if !defined(FM3DLL_H)

#define FM3DLL_H

#include <stdio.h>			// FILE
// #include <time.h>			// time_t

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

#define DIR_SPLITBAR_OFFSET     18 * 12	/* Pixel offset of details splitbar */

#define ALLATTRS                (FILE_NORMAL | FILE_DIRECTORY | FILE_ARCHIVED |\
				 FILE_HIDDEN | FILE_READONLY | FILE_SYSTEM)
//#define LISTTEMPROOT            "$FM2LI$T"

#define DRIVE_REMOVABLE     0x00000001
#define DRIVE_NOTWRITEABLE  0x00000002
#define DRIVE_IGNORE        0x00000004
#define DRIVE_CDROM         0x00000008
#define DRIVE_NOLONGNAMES   0x00000010
#define DRIVE_REMOTE        0x00000020
#define DRIVE_BOOT          0x00000040
#define DRIVE_INVALID       0x00000080
#define DRIVE_NOPRESCAN     0x00000100
#define DRIVE_ZIPSTREAM     0x00000200
#define DRIVE_NOLOADICONS   0x00000400
#define DRIVE_NOLOADSUBJS   0x00000800
#define DRIVE_NOLOADLONGS   0x00001000
#define DRIVE_SLOW          0x00002000
#define DRIVE_INCLUDEFILES  0x00004000
#define DRIVE_VIRTUAL       0x00008000
#define DRIVE_NOSTATS       0x00010000
#define DRIVE_RAMDISK       0x00020000
#define DRIVE_WRITEVERIFYOFF 0x00040000
#define DRIVE_RSCANNED      0x00080000

#define SORT_FIRSTEXTENSION 0x00000001
#define SORT_LASTEXTENSION  0x00000002
#define SORT_SIZE           0x00000004
#define SORT_EASIZE         0x00000008
#define SORT_LWDATE         0x00000010
#define SORT_LADATE         0x00000020
#define SORT_CRDATE         0x00000040
#define SORT_DIRSFIRST      0x00000080
#define SORT_DIRSLAST       0x00000100
#define SORT_FILENAME       0x00000200
#define SORT_REVERSE        0x00000400
#define SORT_PATHNAME       0x00000800
#define SORT_NOSORT         0x00001000
#define SORT_SUBJECT        0x00002000


#define RECFLAGS_ENV      0x00000001
#define RECFLAGS_NODRAG   0x00000002
#define RECFLAGS_NODROP   0x00000004
#define RECFLAGS_UNDERENV 0x00000008

#define COPY 0
#define MOVE 1
#define WPSCOPY 2
#define WPSMOVE 4

#define FILESTOGET_MIN  256
#define FILESTOGET_MAX  4096

#define priority_idle()     DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME,30L,0L)
#define priority_normal()   DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,0L,0L)
// #define priority_tweaked()  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,1L,0L)			// Unused, 13 Sep 08 JBS
#define priority_bumped()   DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,3L,0L)
// #define priority_critical() DosSetPriority(PRTYS_THREAD,PRTYC_FOREGROUNDSERVER,2L,0L)	// Unused, 13 Sep 08 JBS
// #define priority_max()      DosSetPriority(PRTYS_THREAD,PRTYC_FOREGROUNDSERVER,31L,0L)	// Unused, 13 Sep 08 JBS

#define INSTDATA(h)         WinQueryWindowPtr((h),QWL_USER)
// #define RGBFROMPARTS(r,g,b)     (((r) * 65536) + ((g) * 256) + (b))	// Unused, 13 Sep 08 JBS
#define SysVal(value)       WinQuerySysValue(HWND_DESKTOP, (value))


#endif // FM3DLL_H
