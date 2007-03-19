
/***********************************************************************

  $Id$

  Common definitions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2007 Steven H. Levine

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

***********************************************************************/

#if __IBMC__ != 430
#error VAC365 required for long long support
#endif

#if !defined(_LONG_LONG)
#error Long long support not enabled
#endif

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
#ifndef PMBITMAP_INCLUDED
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

 /**************************************************/
 /* Lazy Drag API's.                               */
 /**************************************************/
BOOL APIENTRY DrgLazyDrag(HWND hwndSource,
			  PDRAGINFO pdinfo,
			  PDRAGIMAGE pdimg, ULONG cdimg, PVOID pRsvd);

BOOL APIENTRY DrgCancelLazyDrag(VOID);

BOOL APIENTRY DrgLazyDrop(HWND hwndTarget,
			  ULONG ulOperation, PPOINTL pptlDrop);

PDRAGINFO APIENTRY DrgQueryDraginfoPtr(PDRAGINFO pRsvd);

PDRAGINFO APIENTRY DrgQueryDraginfoPtrFromHwnd(HWND hwndSource);

PDRAGINFO APIENTRY DrgQueryDraginfoPtrFromDragitem(PDRAGITEM pditem);

ULONG APIENTRY DrgQueryDragStatus(VOID);

PDRAGINFO APIENTRY DrgReallocDraginfo(PDRAGINFO pdinfoOld, ULONG cditem);

 /* Drag Status Flags */
#define DGS_DRAGINPROGRESS         0x0001	/* Standard Drag in Progress. */
#define DGS_LAZYDRAGINPROGRESS     0x0002	/* Lazy Drag in Progress.     */

#define LINES_PER_ARCSIG        21	// Lines in each archiver.bb2 definition
#define CON_COLS                6
#define INSTDATA(h)             WinQueryWindowPtr(h,QWL_USER)
#define DIR_SPLITBAR_OFFSET     18 * 12	/* Pixel offset of details splitbar */
#define CONTAINER_COLUMNS       13	/* Number of columns in details view */
#define RGBFROMPARTS(r,g,b)     (((r) * 65536) + ((g) * 256) + (b))
#define EXTRA_RECORD_BYTES2     (sizeof(CNRITEM) - sizeof(MINIRECORDCORE))
#define EXTRA_RECORD_BYTES      (EXTRA_RECORD_BYTES2 + (CCHMAXPATHCOMP - 1))
#define EXTRA_ARCRECORD_BYTES   (sizeof(ARCITEM) - sizeof(MINIRECORDCORE))
#define ALLATTRS                (FILE_NORMAL | FILE_DIRECTORY | FILE_ARCHIVED |\
                                 FILE_HIDDEN | FILE_READONLY | FILE_SYSTEM)
#define LISTTEMPROOT            "$FM2LI$T."

#include "fm3dll2.h"			// SHL

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

typedef struct LINKCMDS
{
  CHAR *cl;
  CHAR *title;
  ULONG flags;
  struct LINKCMDS *next;
  struct LINKCMDS *prev;
}
LINKCMDS;

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
  HWND hwnd;
  HWND hwndS;
  USHORT type;
  USHORT id;
  INT flags;
  struct __arc_type__ *info;
  CHAR **list;
  ULONG *ulitemID;
  ULONG *cbFile;
  CHAR targetpath[CCHMAXPATH + 6];
  CHAR arcname[CCHMAXPATH];
  CHAR runfile[CCHMAXPATH];
}
LISTINFO;

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
  USHORT flags;
  USHORT cmd;
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

#define CNRITEM_SMALLER   0x00010000
#define CNRITEM_LARGER    0x00020000
#define CNRITEM_NEWER     0x00040000
#define CNRITEM_OLDER     0x00080000
#define CNRITEM_EXISTS    0x00100000

#define RECFLAGS_ENV      0x00000001
#define RECFLAGS_NODRAG   0x00000002
#define RECFLAGS_NODROP   0x00000004
#define RECFLAGS_UNDERENV 0x00000008

#define ARCFLAGS_REALDIR    0x00000001
#define ARCFLAGS_PSEUDODIR  0x00000002

#define CBLIST_TO_EASIZE(cb) ((cb) > 4 ? (cb) / 2 : 0)	// FILEFINDBUF4.cbList to logical EA size

typedef struct _CNRITEM
{				/* CONTAINER RECORD STRUCTURE */
  MINIRECORDCORE rc;		/* Base information */
  HWND hwndCnr;			/* The container holding this record */
  PSZ pszFileName;		// Points to szFileName  - required by CFA_STRING
  CHAR szFileName[CCHMAXPATH];	// Path name - fixme to rename to szPathName?
  CHAR szSubject[40];		/* Subject string */
  CHAR *pszSubject;		// Points szSubject - required by CFA_STRING
  CHAR *pszDispAttr;		// Points to szDispAttr - required by CFA_STRING
  CDATE date;			/* Last write date of file */
  CTIME time;			/* Last write time of file */
  CDATE ladate;			/* Last access date of file */
  CTIME latime;			/* Last access time of file */
  CDATE crdate;			/* Creation date of file */
  CTIME crtime;			/* Creation time of file */
  CHAR szDispAttr[6];		/* Attrib string for details display */
  CHAR *pszLongname;		// Points to szLongName - required by CFA_STRING
  ULONGLONG cbFile;		/* File size */
  ULONGLONG easize;		// Size of EAs - dirsize uses this - hack cough
  ULONG attrFile;		/* Attributes of this file */
  ULONG flags;
  CHAR szLongname[1];		// Holds .LONGNAME EA or root flag
}
CNRITEM, *PCNRITEM;

typedef struct _ARCITEM
{				// ARCHIVE CONTAINER RECORD STRUCTURE
  MINIRECORDCORE rc;		// Base information
  HWND hwndCnr;			/* Container holding this record */
  PSZ pszFileName;		// Pointer to file name
  CHAR szFileName[CCHMAXPATH];	// File name
  CHAR szDate[40];		// File's assembled date
  PSZ pszDate;			// Pointer to date
  CDATE date;			// if we know date format
  CTIME time;			// if we know time format
  ULONG cbFile;			// File's original size
  ULONG cbComp;			// File's compressed size
  ULONG flags;
}
ARCITEM, *PARCITEM;

#pragma pack()

typedef struct
{
  ULONG attrFile;
  ULONG cbFile;
  ULONG easize;
  FDATE date;
  FTIME time;
  FDATE ladate;
  FTIME latime;
  FDATE crdate;
  FTIME crtime;
  CHAR fname[1];
}
FILELIST;

typedef struct __arc_type__
{
  CHAR *id;			// User id
  CHAR *ext;			// Extension (without leading dot)
  LONG file_offset;		// Offset to signature (0..n)
  CHAR *list;			// List command
  CHAR *extract;		// Extract command
  CHAR *exwdirs;		// Extract with directories command
  CHAR *test;			// Test command
  CHAR *create;			// Create without directories
  CHAR *move;			// Move into archive without directories
  CHAR *createrecurse;		// Create with recurse and directories
  CHAR *createwdirs;		// Create with directories
  CHAR *movewdirs;		// Move into archive with directories
  CHAR *delete;			// Delete from archive
  CHAR *signature;		// Archiver signature
  CHAR *startlist;		// Listing start marker (blank means no start marker)
  CHAR *endlist;		// Listing end marker (blank means next blank line or EOF)
  INT siglen;			// Signature length in bytes
  INT osizepos;			// Original file size position (0..n) or -1
  INT nsizepos;			// Compressed file size position or -1
  INT fdpos;			// File date position or -1
  INT fdflds;			// File date element count (typically 3) or -1
  INT fnpos;			// File name position or -1 if last
  INT datetype;			// Date field format
  UINT comment_line_num;	// Comment start in old sig file (1..n), 0 if none
  UINT defn_line_num;		// Definition start in old sig file (1..n), 0 if none
  BOOL nameislast;		// Name is last item on line
  BOOL nameisnext;		// File name is on next line
  BOOL nameisfirst;		// File name is first item on line
  struct __arc_type__ *next;
  struct __arc_type__ *prev;
}
ARC_TYPE;

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

typedef struct
{
  ARC_TYPE *info;
  CHAR listname[CCHMAXPATH];
  CHAR arcname[CCHMAXPATH];
  CHAR *errmsg;
}
ARCDUMP;

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
  BOOL detailsladate, detailslatime, detailscrdate, detailscrtime,
    detailslongname, detailsea, detailssize, detailssubject,
    detailslwdate, detailslwtime, detailsattr, detailsicon;
  CHAR **lastselection;
  USHORT shiftstate;
  USHORT suspendview;
  CHAR szCommonName[CCHMAXPATH];
  ULONG lasttime;
  BOOL arcfilled;
  HMTX filling;
  BOOL firsttree;
  ULONG lastattr;
}
DIRCNRDATA;

typedef struct
{
  USHORT size;
  HWND hwndCnr;
  CHAR directory[CCHMAXPATH];
  BOOL collapsefirst;
  DIRCNRDATA *dcd;
}
SHOWREC;

typedef struct
{
  USHORT size;
  USHORT dummy;
  CHAR file1[CCHMAXPATH];
  CHAR file2[CCHMAXPATH];
  HWND hwndParent;
  HWND hwndList;
  HWND hwndReport;
  HWND hwndHelp;
}
FCOMPARE;

typedef struct COMPARE
{
  USHORT size;
  HWND hwnd;
  HWND hwndParent;
  CHAR leftdir[CCHMAXPATH + 2];
  CHAR rightdir[CCHMAXPATH + 2];
  BOOL forcescroll;
  BOOL filling;
  BOOL includesubdirs;
  INT action;
  INT selleft;
  INT selright;
  INT totalleft;
  INT totalright;
  CHAR rightlist[CCHMAXPATH];	// Snapshot file name
  BOOL reset;
  HWND hwndCalling;
  struct COMPARE *cmp;		// callers compare defintion
  struct DIRCNRDATA dcd;
}
COMPARE;

/* init.c */
VOID FindSwapperDat(VOID);
BOOL InitFM3DLL(HAB hab, int argc, char **argv);
HWND StartFM3(HAB hab, INT argc, CHAR ** argv);

/* filldir.c */
VOID FillDirCnr(HWND hwndCnr, CHAR * pszDirectory, DIRCNRDATA * pdcd,
		PULONGLONG pullBytes);
VOID FillTreeCnr(HWND hwndCnr, HWND hwndParent);
VOID ProcessDirectory(const HWND hwndCnr, const PCNRITEM pciParent,
		      const CHAR * szDirBase, const BOOL filestoo,
		      const BOOL recurse, const BOOL partial,
		      CHAR * stopflag, DIRCNRDATA * pdcd,
		      PULONG pullTotalFiles, PULONGLONG pullTotalBytes);
ULONGLONG FillInRecordFromFFB(HWND hwndCnr, PCNRITEM pci,
			      const PSZ pszDirectory,
			      const PFILEFINDBUF4 pffb, const BOOL partial,
			      DIRCNRDATA * pdcd);
ULONGLONG FillInRecordFromFSA(HWND hwndCnr, PCNRITEM pci,
			      const PSZ pszFileName, const PFILESTATUS4 pfsa4,
			      const BOOL partial, DIRCNRDATA * pdcd);

/* flesh.c */
BOOL Stubby(HWND hwndCnr, PCNRITEM pciParent);
BOOL Flesh(HWND hwndCnr, PCNRITEM pciParent);
BOOL FleshEnv(HWND hwndCnr, PCNRITEM pciParent);
BOOL UnFlesh(HWND hwndCnr, PCNRITEM pciParent);

/* error.c */
INT Dos_Error(ULONG mb_type, ULONG ulRC, HWND hwndOwner,
	      PCSZ pszFileName, ULONG ulLineNo, PCSZ pszFmt, ...);
INT Dos_Error2(ULONG mb_type, ULONG ulRC, HWND hwndOwner, PCSZ pszFileName,
	       ULONG ulLineNo, UINT idMsg);
VOID Win_Error(HWND hwndErr, HWND hwndOwner,
	       PCSZ pszFileName, ULONG ulLineNo, PCSZ pszFmt, ...);
VOID Win_Error2(HWND hwndErr, HWND hwndOwner, PCSZ pszFileName,
		ULONG ulLineNo, UINT idMsg);
VOID Runtime_Error(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...);
VOID Runtime_Error2(PCSZ pszSrcFile, UINT uSrcLineNo, UINT idMsg);
APIRET saymsg(ULONG mb_type, HWND hwnd, PCSZ pszTitle, PCSZ pszFmt, ...);

/* valid.c */
INT CheckDrive(CHAR Drive, CHAR * FileSystem, ULONG * type);
int TestDates(char *file1, char *file2);
BOOL IsNewer(char *file1, char *file2);
BOOL IsRoot(CHAR * filename);
BOOL IsFileSame(CHAR * filename1, CHAR * filename2);
INT IsFile(CHAR * filename);
BOOL IsFullName(CHAR * filename);
BOOL IsValidDir(CHAR * test);
BOOL IsValidDrive(CHAR drive);
CHAR *MakeValidDir(CHAR * path);
BOOL IsExecutable(CHAR * filename);
VOID FillInDriveFlags(VOID * dummy);
VOID DriveFlagsOne(INT x);
VOID ArgDriveFlags(INT argc, CHAR ** argv);
CHAR *assign_ignores(CHAR * s);
BOOL needs_quoting(CHAR * f);
BOOL IsBinary(CHAR * str, ULONG len);
BOOL TestBinary(CHAR * filename);
BOOL ParentIsDesktop(HWND hwnd, HWND hwndParent);
BOOL IsDesktop(HAB hab, HWND hwnd);
char *IsVowel(char a);
VOID GetDesktopName(CHAR * objectpath, ULONG size);
char *RootName(char *filename);
APIRET MakeFullName(char *filename);

/* misc.c */
VOID SetShiftState(VOID);
void EmphasizeButton(HWND hwnd, BOOL on);
void DrawTargetEmphasis(HWND hwnd, BOOL on);
void BoxWindow(HWND hwnd, HPS hps, LONG color);
VOID PaintRecessedWindow(HWND hwnd, HPS hps, BOOL outtie, BOOL dbl);
void PaintSTextWindow(HWND hwnd, HPS hps);
BOOL AdjustCnrColVis(HWND hwndCnr, CHAR * title, BOOL visible, BOOL toggle);
BOOL AdjustCnrColRO(HWND hwndCnr, CHAR * title, BOOL readonly, BOOL toggle);
VOID AdjustCnrColsForFSType(HWND hwndCnr, CHAR * directory, DIRCNRDATA * dcd);
VOID AdjustCnrColsForPref(HWND hwndCnr, CHAR * directory, DIRCNRDATA * dcd,
			  BOOL compare);
BOOL SetCnrCols(HWND hwndCnr, BOOL compare);
MRESULT CnrDirectEdit(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL SetMenuCheck(HWND hwndMenu, USHORT id, BOOL * bool, BOOL toggle,
		  CHAR * savename);
VOID disable_menuitem(HWND hwndMenu, USHORT id, BOOL enable);
BOOL ViewHelp(CHAR * filename);
VOID CloseHelp(VOID);
INT ExecFile(HWND hwnd, CHAR * filename);
VOID EmptyCnr(HWND hwnd);
VOID SetDetailsSwitches(HWND hwnd, DIRCNRDATA * dcd);
VOID AdjustDetailsSwitches(HWND hwnd, HWND hwndMenu, USHORT cmd,
			   CHAR * directory, CHAR * keyroot, DIRCNRDATA * dcd,
			   BOOL compare);
VOID FreeMallocedMem(VOID * mem);
VOID FcloseFile(FILE * fp);
VOID SetConditionalCascade(HWND hwndMenu, USHORT id, USHORT def);
VOID SetSortChecks(HWND hwndMenu, INT sortflags);
VOID SetupCommandMenu(HWND hwndMenu, HWND hwndCnr);
VOID LoadDetailsSwitches(CHAR * keyroot, DIRCNRDATA * dcd);
HWND FindDirCnr(HWND hwndParent);
VOID HeapThread(VOID * dummy);
VOID FixSwitchList(HWND hwnd, CHAR * text);
VOID QuickPopup(HWND hwnd, DIRCNRDATA * dcd, HWND hwndMenu, USHORT id);
PMINIRECORDCORE CurrentRecord(HWND hwndCnr);
BOOL PostMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID OpenEdit(HWND hwnd);
VOID PortholeInit(HWND hwndNew, MPARAM mp1, MPARAM mp2);
HWND CheckMenu(HWND * hwndMenu, USHORT id);
SHORT AddToListboxBottom(HWND hwnd, CHAR * str);
VOID SetSysMenu(HWND hwndSysMenu);
VOID LoadLibPath(CHAR * str, LONG len);
void SaySort(HWND hwnd, INT sortflags, BOOL archive);
void SayView(HWND hwnd, ULONG flWindowAttr);
void SayFilter(HWND hwnd, MASK * mask, BOOL archive);
void SetViewMenu(HWND hwndMenu, ULONG flWindowAttr);
char *GetCmdSpec(BOOL dos);
void Broadcast(HAB hab, HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void SetupWinList(HWND hwndMenu, HWND hwndTop, HWND hwndFrame);
BOOL SwitchCommand(HWND hwndMenu, USHORT cmd);

/* mainwnd.c */
ULONG CountDirCnrs(HWND hwndParent);
HWND TopWindow(HWND hwndParent, HWND exclude);
HWND TopWindowName(HWND hwndParent, HWND exclude, CHAR * ret);
HWND FindDirCnrByName(CHAR * directory, BOOL restore);
MRESULT EXPENTRY MainWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID GetNextWindowPos(HWND hwndClient, PSWP pswp, ULONG * ulCntR,
		      ULONG * ulNumMinChildrenR);
VOID TileChildren(HWND hwndClient, BOOL absolute);
VOID FillClient(HWND hwndClient, PSWP pswp, PRECTL prectl, BOOL avoidtree);
MRESULT EXPENTRY ToolBackProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DriveBackProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ChildButtonProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
MRESULT EXPENTRY DriveProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY BubbleProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL SaveDirCnrState(HWND hwndClient, CHAR * name);
MRESULT EXPENTRY LEDProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY StatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID BuildDriveBarButtons(HWND hwndT);
VOID ResizeDrives(HWND hwndT, long xwidth);
BOOL CloseChildren(HWND hwndClient);
VOID BuildTools(HWND hwndT, BOOL resize);
void BubbleHelp(HWND hwnd, BOOL other, BOOL data, BOOL above, char *help);
VOID MakeBubble(HWND hwnd, BOOL above, CHAR * help);
MRESULT EXPENTRY MainWMCommand(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID MakeMainObjWin(VOID * args);

/* mainwnd2.c */
MRESULT EXPENTRY MainWndProc2(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY FileListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartFM32(HAB hab, INT argc, CHAR ** argv);

/* treecnr.c */
MRESULT EXPENTRY TreeStatProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY TreeClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
HWND StartTreeCnr(HWND hwndParent, ULONG flags);
MRESULT EXPENTRY TreeObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID ShowTreeRec(HWND hwndCnr, CHAR * dirname, BOOL collapsefirst,
		 BOOL maketop);
MRESULT EXPENTRY OpenButtonProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* dircnrs.c */
MRESULT EXPENTRY DirClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
HWND StartDirCnr(HWND hwndParent, CHAR * directory, HWND hwndRestore,
		 ULONG flags);
MRESULT EXPENTRY DirTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DirFolderProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DirMaxProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DirObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* presparm.c */
VOID StoreWndPresParams(HWND hwnd, CHAR * tagname, HINI prof);

#ifdef INCL_GPI
VOID SetPresParams(HWND hwnd, RGB2 * back, RGB2 * fore, RGB2 * border,
		   CHAR * font);
#endif
VOID CopyPresParams(HWND target, HWND source);
VOID IfNoParam(HWND hwnd, CHAR * keyroot, ULONG size, PVOID attrvalue);
VOID PresParamChanged(HWND hwnd, CHAR * keyroot, MPARAM mp1, MPARAM mp2);
VOID RestorePresParams(HWND hwnd, CHAR * keyroot);

/* dirs.c */
APIRET save_dir2(CHAR * curdir);
APIRET save_dir(CHAR * curdir);
APIRET switch_to(CHAR * s);

/* strips.c */
VOID chop_at_crnl(PSZ pszSrc);
PSZ convert_nl_to_nul(PSZ pszSrc);
void strip_lead_char(char *pszStripChars, char *pszSrc);
void strip_trail_char(char *pszStripChars, char *pszSrc);

#define lstrip(s)         strip_lead_char(" \t",(s))
#define rstrip(s)         strip_trail_char(" \t",(s))
#define stripcr(s)        strip_trail_char("\r\n",(s))
// Strip leading and trailing white
#define bstrip(s)         (strip_lead_char(" \t",(s)),strip_trail_char(" \t",(s)))
// Strip leading and trailing white and trail cr/nl
#define bstripcr(s)       (strip_lead_char(" \t",(s)),strip_trail_char("\r\n \t",(s)))

/* delims.c */
char *skip_delim(char *a, register char *delim);
char *to_delim(char *a, register char *delim);

/* copyf.c */
BOOL AdjustWildcardName(CHAR * oldname, CHAR * newname);
CHAR default_disk(VOID);
APIRET docopyf(INT type, CHAR * oldname, CHAR * newname, ...);

#define COPY 0
#define MOVE 1
#define WPSCOPY 2
#define WPSMOVE 4
INT unlinkf(CHAR * string, ...);
INT unlink_allf(CHAR * string, ...);
INT wipeallf(CHAR * string, ...);
INT make_deleteable(CHAR * filename);
CHAR *TruncName(CHAR * oldname, CHAR * buffer);
CHAR *GetLongName(CHAR * oldname, CHAR * buffer);
BOOL WriteLongName(CHAR * filename, CHAR * longname);

/* mkdir.c */
APIRET SetDir(HWND hwndClient, HWND hwnd, CHAR * dir, INT flags);
APIRET MassMkdir(HWND hwndClient, CHAR * dir);
BOOL PMMkDir(HWND hwnd, CHAR * filename, BOOL copy);
void SetTargetDir(HWND hwnd, BOOL justshow);

/* srchpath.c */
CHAR *first_path(CHAR * path, CHAR * ret);
CHAR *searchapath(CHAR * path, CHAR * filename);
CHAR *searchpath(CHAR * filename);

/* literal.c */
UINT literal(PSZ pszBuf);
BOOL wildcard(const PSZ pszBuf, const PSZ pszWildCard,
	      const BOOL fNotFileSpec);
PSZ fixup(const PCH pachInBuf, PSZ pszOutBuf, const UINT cBufBytes,
	  const UINT cInBytes);

/* stristr.c */
CHAR *stristr(const CHAR * t, const CHAR * s);
CHAR *strnistr(register CHAR * t, CHAR * s, LONG len);
CHAR *strnstr(register CHAR * t, CHAR * s, LONG len);
CHAR *findstring(CHAR * findthis, ULONG lenthis, CHAR * findin,
		 ULONG lenin, BOOL insensitive);

/* avl.c */
ARC_TYPE *quick_find_type(CHAR * filespec, ARC_TYPE * topsig);
ARC_TYPE *find_type(CHAR * filespec, ARC_TYPE * topsig);
INT load_archivers(VOID);
BOOL ArcDateTime(CHAR * dt, INT type, CDATE * cdate, CTIME * ctime);

/* avv.c */
VOID rewrite_archiverbb2(CHAR * archiverbb2);
MRESULT EXPENTRY ArcReviewDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
VOID EditArchiverDefinition(HWND hwnd);

/* systemf.c */
BOOL ShowSession(HWND hwnd, PID pid);
INT ExecOnList(HWND hwnd, CHAR * command, INT flags, CHAR * tpath,
	       CHAR ** list, CHAR * prompt);
INT runemf2(INT type, HWND hwnd, CHAR * directory, CHAR * environment,
	    CHAR * formatstring, ...);
HAPP Exec(HWND hwndNotify, BOOL child, char *startdir, char *env,
	  PROGTYPE * progt, ULONG fl, char *formatstring, ...);
#define SYNCHRONOUS   1
#define ASYNCHRONOUS  2
#define DETACHED      3
#define SEPARATE      4
#define SEPARATEKEEP  5
#define WINDOWED      16
#define MAXIMIZED     32
#define MINIMIZED     64
#define FULLSCREEN    128
#define INVISIBLE     256
#define BACKGROUND    512
#define WAIT          1024
#define PROMPT        2048
#define KEEP          4096
#define ONCE          8192
#define DIEAFTER      16384
#define SEAMLESS      32768
#define CHILD         65536

/* cmdline.c */
BOOL add_cmdline(CHAR * cmdline, BOOL big);
VOID save_cmdlines(BOOL big);
MRESULT EXPENTRY CmdLineDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CmdLine2DlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);

/* makelist.c */
INT AddToList(CHAR * string, CHAR *** list, INT * numfiles, INT * numalloced);
INT AddToFileList(CHAR * string, FILEFINDBUF4 * ffb4, FILELIST *** list,
		  INT * numfiles, INT * numalloced);
CHAR **BuildList(HWND hwndCnr);
VOID FreeListInfo(LISTINFO * li);
VOID FreeList(CHAR ** list);
VOID SortList(LISTINFO * li);
CHAR **BuildArcList(HWND hwndCnr);
CHAR **RemoveFromList(CHAR ** list, CHAR * item);
CHAR **CombineLists(CHAR ** prime, CHAR ** add);

/* chklist.c */
VOID PosOverOkay(HWND hwnd);
VOID CenterOverWindow(HWND hwnd);
BOOL PopupMenu(HWND hwndParent, HWND hwndOwner, HWND hwndMenu);
MRESULT EXPENTRY CheckListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY DropListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* eas.c */
VOID HexDump(HWND hwnd, CHAR * value, ULONG cbValue);
HOLDFEA *GetFileEAs(CHAR * filename, BOOL ishandle, BOOL silentfail);
VOID Free_FEAList(HOLDFEA * pFEA);
MRESULT EXPENTRY DisplayEAsProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
PVOID SaveEA(CHAR * filename, HOLDFEA * current, CHAR * newdata,
	     BOOL silentfail);

/* inis.c */
MRESULT EXPENTRY IniProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartIniEditor(HWND hwnd, CHAR * filename, INT flags);

/* subj.c */
INT Subject(HWND hwnd, CHAR * filename);

/* dirsize.c */
MRESULT EXPENTRY DirSizeProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* getnames.c */
BOOL insert_filename(HWND hwnd, CHAR * filename, INT loadit, BOOL newok);
BOOL export_filename(HWND hwnd, CHAR * filename, INT overwrite);
MRESULT EXPENTRY CustomFileDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* input.c */
MRESULT EXPENTRY InputDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* select.c */
VOID UnHilite(HWND hwndCnr, BOOL all, CHAR *** list);
VOID SelectList(HWND hwndCnr, BOOL partial, BOOL deselect, BOOL clearfirst,
		PCNRITEM pciParent, CHAR * filename, CHAR ** list);
VOID SelectAll(HWND hwndCnr, BOOL files, BOOL dirs, CHAR * mask, CHAR * text,
	       BOOL arc);
VOID DeselectAll(HWND hwndCnr, BOOL files, BOOL dirs, CHAR * mask,
		 CHAR * text, BOOL arc);
VOID Deselect(HWND hwndCnr);
VOID HideAll(HWND hwndCnr);
VOID RemoveAll(HWND hwndCnr, ULONGLONG * ullTotalBytes, ULONG * totalfiles);
VOID MarkAll(HWND hwndCnr, BOOL quitit, BOOL target, BOOL source);
VOID SetMask(CHAR * str, MASK * mask);
VOID ExpandAll(HWND hwndCnr, BOOL expand, PCNRITEM pciParent);
VOID InvertAll(HWND hwndCnr);
VOID SpecialSelect(HWND hwndCnrS, HWND hwndCnrD, INT action, BOOL reset);
VOID SpecialSelect2(HWND hwndParent, INT action);

/* viewer.c */
MRESULT EXPENTRY MLEEditorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartMLEEditor(HWND hwnd, INT flags, CHAR * filename, HWND hwndRestore);

/* codepage.c */
INT PickCodepage(HWND hwnd);

/* fonts.c */
VOID SetFont(HWND hwnd);
FATTRS *SetMLEFont(HWND hwndMLE, FATTRS * fattrs, ULONG flags);
VOID SetPresParamFromFattrs(HWND hwnd, FATTRS * fattrs,
			    SHORT sNominalPointSize, FIXED fxPointSize);

/* saveclip.c */
BOOL SaveToClip(HWND hwnd, CHAR * text, BOOL append);
VOID ListToClipboard(HWND hwnd, CHAR ** list, BOOL append);
CHAR **ListFromClipboard(HWND hwnd);
BOOL SaveToClipHab(HAB hab, CHAR * text, BOOL append);
VOID ListToClipboardHab(HAB hab, CHAR ** list, BOOL append);
CHAR **ListFromClipboardHab(HAB hab);
MRESULT EXPENTRY SaveListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
MRESULT EXPENTRY SaveAllListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				    MPARAM mp2);

/* filter.c */
INT APIENTRY Filter(PMINIRECORDCORE rmini, PVOID arg);
BOOL FilterAttrs(PCNRITEM pci, MASK * mask);
VOID save_masks(VOID);
MRESULT EXPENTRY PickMaskDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);

/* archive.c */
MRESULT EXPENTRY ArchiveDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY SBoxDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* extract.c */
MRESULT EXPENTRY ExtractDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* walkem.c */
VOID load_udirs(VOID);
VOID save_udirs(VOID);
BOOL add_udir(BOOL userdirs, CHAR * inpath);
BOOL remove_udir(CHAR * path);
BOOL remove_ldir(CHAR * path);
VOID load_setups(VOID);
VOID save_setups(VOID);
BOOL add_setup(CHAR * name);
BOOL remove_setup(CHAR * name);
VOID FillPathListBox(HWND hwnd, HWND hwnddrive, HWND hwnddir, CHAR * path,
		     BOOL nounwriteable);
MRESULT EXPENTRY WalkDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY WalkAllDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY WalkCopyDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
MRESULT EXPENTRY WalkMoveDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
MRESULT EXPENTRY WalkExtractDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				    MPARAM mp2);
MRESULT EXPENTRY WalkTargetDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
MRESULT EXPENTRY WalkTwoDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY WalkTwoCmpDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
MRESULT EXPENTRY WalkTwoSetDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);

/* arccnrs.c */
MRESULT EXPENTRY ArcClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
HWND StartArcCnr(HWND hwndParent, HWND hwndCaller, CHAR * arcname, INT flags,
		 ARC_TYPE * sinfo);
MRESULT EXPENTRY ArcTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ArcFolderProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ArcObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* assoc.c */
INT ExecAssociation(HWND hwnd, CHAR * datafile);
VOID EditAssociations(HWND hwnd);
VOID load_associations(VOID);
VOID save_associations(VOID);

/*draglist.c */
HWND DoFileDrag(HWND hwndCnr, HWND hwndObj, PCNRDRAGINIT pcd, CHAR * arcfile,
		CHAR * directory, BOOL moveok);
HWND DragOne(HWND hwndCnr, HWND hwndObj, CHAR * filename, BOOL moveok);
HWND DragList(HWND hwnd, HWND hwndObj, CHAR ** list, BOOL moveok);
BOOL PickUp(HWND hwndCnr, HWND hwndObj, PCNRDRAGINIT pcd);

/* droplist.c */
ULONG FreeDrop(MPARAM mp1, MPARAM mp2);
void DropHelp(MPARAM mp1, MPARAM mp2, HWND hwnd, char *text);
BOOL AcceptOneDrop(MPARAM mp1, MPARAM mp2);
BOOL GetOneDrop(MPARAM mp1, MPARAM mp2, char *buffer, ULONG buflen);
BOOL FullDrgName(PDRAGITEM pDItem, CHAR * buffer, ULONG buflen);
BOOL TwoDrgNames(PDRAGITEM pDItem, CHAR * buffer1, ULONG buflen1,
		 char *buffer2, ULONG buflen2);
LISTINFO *DoFileDrop(HWND hwndCnr, CHAR * directory, BOOL arcfilesok,
		     MPARAM mp1, MPARAM mp2);

/* shadow.c */
HOBJECT CreateProgramObject(CHAR * objtitle, CHAR * location, CHAR * path,
			    CHAR * cnr);
HOBJECT CreateDataObject(CHAR * objtitle, CHAR * location, CHAR * path,
			 CHAR * cnr);
HOBJECT CreateFolderObject(CHAR * objtitle, CHAR * cnr);
HOBJECT CreateShadowObject(CHAR * objtitle, CHAR * location, CHAR * path,
			   BOOL executable, CHAR * cnr);
VOID MakeShadows(HWND hwnd, CHAR ** list, ULONG Shadows, CHAR * cnr,
		 CHAR * foldername);
VOID OpenObject(CHAR * filename, CHAR * type, HWND hwnd);
BOOL RunSeamless(CHAR * exename, CHAR * args, HWND hwnd);

/* printer.c */
BOOL PrinterReady(CHAR * printdevname);
BOOL SayPrinterReady(HWND hwnd);
VOID PrintListThread(VOID * arg);
MRESULT EXPENTRY PrintDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* attribs.c */
MRESULT EXPENTRY AttrListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);

/* rename.c */
MRESULT EXPENTRY RenameProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* comp.c */
MRESULT EXPENTRY CFileDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CompareDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* findrec.c */
PCNRITEM FindCnrRecord(HWND hwndCnr, CHAR * filename, PCNRITEM pciParent,
		       BOOL partial, BOOL partmatch, BOOL noenv);
PCNRITEM FindParentRecord(HWND hwndCnr, PCNRITEM pciC);
VOID ShowCnrRecord(HWND hwndCnr, PMINIRECORDCORE pmi);

/* update.c */
HPOINTER SelectDriveIcon(PCNRITEM pci);
PCNRITEM UpdateCnrRecord(HWND hwndCnr, CHAR * filename, BOOL partial,
			 DIRCNRDATA * dcd);
BOOL UpdateCnrList(HWND hwndCnr, CHAR ** filename, INT howmany, BOOL partial,
		   DIRCNRDATA * dcd);

/* info.c */
MRESULT EXPENTRY DrvInfoProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY FileInfoProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY SetDrvProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* fsopen.c */
FILE *_fsopen(CHAR * filename, CHAR * mode, INT sharemode, ...);

/* seticon.c */
MRESULT EXPENTRY SetIconDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* objcnr.c */
MRESULT EXPENTRY ObjCnrDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* sortcnr.c */
SHORT APIENTRY SortTreeCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2,
			   PVOID pStorage);
SHORT APIENTRY SortDirCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2,
			  PVOID pStorage);
SHORT APIENTRY SortCollectorCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2,
				PVOID pStorage);
SHORT SortCnr(PMINIRECORDCORE p1, PMINIRECORDCORE p2, INT Sortflags);

/* collect.c */
MRESULT EXPENTRY CollectorClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
					MPARAM mp2);
MRESULT EXPENTRY CollectorTextProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
HWND StartCollector(HWND hwndParent, INT flags);
MRESULT EXPENTRY CollectorObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				     MPARAM mp2);

/* command.c */
VOID RunCommand(HWND hwnd, INT cx);
VOID EditCommands(HWND hwnd);
CHAR *command_title(INT cx);
VOID load_commands(VOID);

/* instant.c */
MRESULT EXPENTRY InstantDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* undel.c */
MRESULT EXPENTRY UndeleteDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);

/* killproc.c */
MRESULT EXPENTRY KillDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* sysinfo.c */
MRESULT EXPENTRY SysInfoDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* objwin.c */
MRESULT EXPENTRY ObjectWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID MakeObjWin(VOID * args);

/* progstup.c */
MRESULT EXPENTRY ProgDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* key.c */
MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* notify.c */
MRESULT EXPENTRY NotifyWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND Notify(char *text);
HWND DoNotify(char *text);
VOID NotifyError(CHAR * filename, APIRET error);
VOID StartNotes(CHAR * s);
BOOL AddNote(CHAR * note);
VOID EndNote(VOID);
VOID ShowNote(VOID);
VOID HideNote(VOID);

/* winlist.c */
VOID WindowList(HWND hwnd);

/* viewinf.c */
MRESULT EXPENTRY ViewInfProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* notebook.c */
MRESULT EXPENTRY CfgDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* commafmt.c */
size_t commafmt(PSZ pszBuf, UINT cBufSize, LONG lNumber);
size_t CommaFmtUL(char *pszBuf, UINT cBufSize, ULONG ullNumber,
		  CHAR chPreferred);
size_t CommaFmtULL(char *pszBuf, UINT cBufSize, ULONGLONG ullNumber,
		   CHAR chPreferred);

/* autoview.c */
BOOL WriteEA(HWND hwnd, CHAR * filename, CHAR * eaname, USHORT type,
	     CHAR * data);
BOOL PutComments(HWND hwnd, CHAR * filename, CHAR * comments);
MRESULT EXPENTRY AutoViewProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
ULONG CreateHexDump(CHAR * value, ULONG cbValue, CHAR * ret, ULONG retlen,
		    ULONG startval, BOOL longlead);

/* menu.c */
BOOL AddToMenu(CHAR * filename, HWND hwndMenu);

/* worker.c */
VOID MassAction(VOID * args);
VOID Action(VOID * args);

/* fm2cmd.c */
BOOL FM2Command(CHAR * directory, CHAR * command);

/* seeall.c */
MRESULT EXPENTRY SeeAllWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY SeeStatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartSeeAll(HWND hwndParent, BOOL standalone, CHAR * startpath);

/* newview.c */
MRESULT EXPENTRY ViewWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY ViewStatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND StartViewer(HWND hwndParent, USHORT flags, CHAR * filename,
		 HWND hwndRestore);

/* colors.c */
MRESULT EXPENTRY ColorDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* defview.c */
BOOL ShowMultimedia(CHAR * filename);
VOID DefaultView(HWND hwnd, HWND hwndFrame, HWND hwndParent, SWP * swp,
		 ULONG flags, CHAR * filename);
VOID DefaultViewKeys(HWND hwnd, HWND hwndFrame, HWND hwndParent,
		     SWP * swp, CHAR * filename);
#define QuickView(h,f) DefaultView(h,(HWND)0,(HWND)0,NULL,0,f)
#define QuickEdit(h,f) DefaultView(h,(HWND)0,(HWND)0,NULL,8,f)

/* catalog.c */
MRESULT EXPENTRY CatalogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* uudecode.c */
int UUD(char *filename, char *outname);
MRESULT EXPENTRY MergeDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* loadbmp.c */
HBITMAP LoadBitmapFromFileNum(USHORT id);
HBITMAP LoadBitmapFromFile(CHAR * pszFileName);

/* remap.c */
MRESULT EXPENTRY RemapDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* timer.c */
BOOL StartTimer(void);
void StopTimer(void);

/* grep2.c */
MRESULT EXPENTRY GrepDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* common.c */
MRESULT EXPENTRY CommonFrameWndProc(USHORT id,
				    HWND hwnd,
				    ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CommonTextProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void CommonTextPaint(HWND hwnd, HPS hps);
void CommonCreateTextChildren(HWND hwnd, char *class, USHORT * ids);
void CommonDriveCmd(HWND hwnd, char *drive, USHORT cmd);
void CommonCreateMainChildren(HWND hwnd, SWP * swp);
MRESULT EXPENTRY CommonMainWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
MRESULT EXPENTRY CommonTextButton(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2);
MRESULT EXPENTRY CommonCnrProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HWND OpenDirCnr(HWND hwnd, HWND hwndParent, HWND hwndRestore,
		BOOL noautotile, char *directory);
VOID IncrThreadUsage(VOID);
VOID DecrThreadUsage(VOID);

/* string.c */
BOOL LoadStrings(char *filename);
char *GetPString(ULONG id);
BOOL StringsLoaded(void);

/* wrappers.c */
PSZ xfgets(PSZ pszBuf, size_t cMaxBytes, FILE * fp, PCSZ pszSrcFile,
	   UINT uiLineNumber);
PSZ xfgets_bstripcr(PSZ pszBuf, size_t cMaxBytes, FILE * fp, PCSZ pszSrcFile,
		    UINT uiLineNumber);
FILE *xfopen(PCSZ pszFileName, PCSZ pszMode, PCSZ pszSrcFile,
	     UINT uiLineNumber);
FILE *xfsopen(PCSZ pszFileName, PCSZ pszMode, INT fSharemode, PCSZ pszSrcFile,
	      UINT uiLineNumber);
VOID xfree(PVOID pv);
PVOID xmalloc(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber);
PVOID xmallocz(size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber);
PVOID xrealloc(PVOID pvIn, size_t cBytes, PCSZ pszSrcFile, UINT uiLineNumber);
PVOID xstrdup(PCSZ pszIn, PCSZ pszSrcFile, UINT uiLineNumber);

//=====================================================================

#ifdef DEFINE_GLOBALS
#define DATADEF
#else
#define DATADEF extern
#endif

DATADEF ARC_TYPE *arcsighead;
DATADEF BOOL arcsigsloaded;
DATADEF BOOL arcsigsmodified;
DATADEF UINT arcsigs_header_lines;	// Header comments line count in archiver.bb2
DATADEF UINT arcsigs_trailer_line_num;	// Trailer comments start line number (1..n)

DATADEF USHORT nodes, shiftstate;
DATADEF HEV CompactSem;
DATADEF HWND hwndMain, hwndTree, hwndStatus, hwndStatus2, hwndTrash,
  hwndButtonlist, hwndDrivelist, hwndStatelist, hwndUserlist,
  hwndAutoview, hwndAttr, hwndDate, hwndName, hwndBack,
  hwndLED, hwndLEDHdr, hwndAutoMLE, hwndCmdlist;
DATADEF HBITMAP hbmLEDon, hbmLEDoff;
DATADEF CHAR ArcTempRoot[9];
DATADEF HPOINTER hptrArrow, hptrBusy, hptrLast, hptrDir, hptrFile, hptrRemote,
  hptrFloppy, hptrDrive, hptrRemovable, hptrCDROM,hptrVirtual,hptrRamdisk,
  hptrFinger, hptrApp, hptrDunno, hptrSystem, hptrHidden,
  hptrReadonly, hptrNS, hptrZipstrm, hptrArc, hptrArt, hptrEW,
  hptrCommon, hptrEnv;
DATADEF PFNWP PFNWPCnr, PFNWPFrame, PFNWPButton, PFNWPStatic, PFNWPMLE;
DATADEF BOOL fLoadSubject, fLoadLongnames, fForceUpper, fForceLower,
  fSyncUpdates, fAutoTile, fDontMoveMouse, loadedudirs,
  fUnHilite, fWorkPlace, fConfirmDelete, fToolbar,
  fToolsChanged, MenuInvisible, fFreeTree, fFollowTree,
  fNoIconsFiles, fNoIconsDirs, fVerify, fDCOpens,
  fLinkSetsIcon, fSaveState, fTextTools, fCopyDefault,
  fToolTitles, fLogFile, fRealIdle, fNoSaveState,
  fSplitStatus, fArcStuffVisible, fUseMCI, fAmAV2,
  fNoTreeGap, fDummy, fVTreeOpensWPS, fUseQProcStat,
  fStartMinimized, fStartMaximized, fRemoteBug, fReminimize,
  fDragndropDlg, fMinOnOpen, fUserComboBox, loadedsetups,
  fQuickArcFind, fNoRemovableScan, fAutoView, fDataMin,
  fDataToFore, fDataShowDrives, fDataInclRemote,
  fExternalArcboxes, fExternalViewer, fExternalCollector,
  fExternalINIs, fDefaultDeletePerm, fIniExisted, fUseNewViewer,
  fTileBackwards, fFolderAfterExtract, fUserListSwitches,
  fGuessType, fAutoAddDirs, fUdirsChanged, fSelectedAlways,
  fToolbarHelp, fComments, fMoreButtons, fNoSearch, fOtherHelp,
  fKeepCmdLine, fAmClosing, fSeparateParms, fTopDir,
  fLookInDir, fSwitchTree, fSwitchTreeOnFocus, fDrivebar,
  fSwitchTreeExpand, fCollapseFirst, fFilesInTree, fNoDead,
  fThreadNotes, fOkayMinimize, fRunning, fDullMin, fBlueLED,
  fViewChild, fShowEnv, fLeaveTree, fAddUtils, fNoFoldMenu,
  fCustomFileDlg, fSaveMiniCmds, fSaveBigCmds, fNoTileUpdate,
  fFM2Deletes, fAutoAddAllDirs, fConfirmTarget, fChangeTarget,
  fFirstTime, fShowTarget, fNoFinger, fDrivebarHelp, fCheckMM;
DATADEF BOOL detailsladate, detailslatime, detailscrdate, detailscrtime,
  detailslongname, detailsea, detailssize, detailssubject,
  detailslwdate, detailslwtime, detailsattr, detailsicon;
DATADEF PID mypid;
DATADEF INT driveflags[26], driveserial[26];
DATADEF ULONG NoBrokenNotify, fwsAnimate, OS2ver[2], DriveLines;
DATADEF HINI fmprof;
DATADEF HELPINIT hini;
DATADEF HWND hwndHelp, LastDir, AboutBox, DirMenu, FileMenu, TreeMenu,
  ArcMenu, DirCnrMenu, TreeCnrMenu, ArcCnrMenu,
  CollectorCnrMenu, CollectorFileMenu, CollectorDirMenu,
  Collector, MainPopupMenu, DataHwnd, AutoMenu, hwndBubble,
  hwndToolback, MainObjectHwnd;
#ifdef DEFINE_GLOBALS
#pragma data_seg(GLOBAL2)
#endif
DATADEF CHAR *DEBUG_STRING, *FM3Str, *FM2Str, *NullStr, *Default, *Settings,
  *DRM_OS2FILE, *DRM_FM2ARCMEMBER, *DRF_FM2ARCHIVE,
  *DRMDRFLIST, *DRMDRFOS2FILE, *DRMDRFFM2ARC,
  *DRM_FM2INIRECORD, *DRF_FM2INI, *SUBJECT, *LONGNAME,
  *HPFS, *JFS, *CDFS, *FAT32, *HPFS386, *NDFS32, *CBSIFS, *ISOFS, *RAMFS, *NTFS,
  *WPProgram, *FM3Folder, *FM3Tools;
DATADEF RGB2 RGBGREY, RGBBLACK;
DATADEF CHAR archiverbb2[CCHMAXPATH], StopPrinting, profile[CCHMAXPATH];
DATADEF CHAR appname[12], realappname[12];
DATADEF CHAR editor[CCHMAXPATH], viewer[CCHMAXPATH],
  virus[CCHMAXPATH], printer[CCHMAXPATH],
  compare[CCHMAXPATH], extractpath[CCHMAXPATH],
  lastextractpath[CCHMAXPATH], lasttoolbox[CCHMAXPATH],
  HomePath[CCHMAXPATH], SwapperDat[CCHMAXPATH],
  binview[CCHMAXPATH], bined[CCHMAXPATH],
  dircompare[CCHMAXPATH], szDefArc[CCHMAXPATH],
  ftprun[CCHMAXPATH], httprun[CCHMAXPATH], targetdir[CCHMAXPATH];
DATADEF HMODULE FM3DllHandle, FM3ModHandle;
DATADEF CHAR *quicktool[50];
DATADEF BOOL qtloaded;
DATADEF INT sortFlags, TreesortFlags, CollectorsortFlags;
DATADEF INT butxsize, butysize;
DATADEF FILE *LogFileHandle;
DATADEF ULONG ulCnrType, FilesToGet, AutoviewHeight, TreeWidth, FM3UL;
DATADEF long prnwidth, prnlength, prntmargin, prnbmargin, prnlmargin,
  prnrmargin, prnspacing, prntabspaces;
DATADEF BOOL prnpagenums, prnformat, prnformfeedbefore, prnformfeedafter,
  prnalt;
DATADEF LINKDIRS *udirhead, *ldirhead;
DATADEF LINKCMDS *cmdhead, *cmdtail;
DATADEF BOOL cmdloaded;

#ifdef DEFINE_GLOBALS
#pragma data_seg(GLOBAL3)
#endif
#define MAXNUMSETUPS  100
DATADEF CHAR lastsetups[MAXNUMSETUPS][13];
DATADEF INT lastsetup;
DATADEF LONG standardcolors[16];

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

#ifdef DEFINE_GLOBALS
#pragma data_seg(GLOBAL4)
#endif
DATADEF HSWITCH switches[499];
DATADEF ULONG numswitches;

#define priority_idle()     DosSetPriority(PRTYS_THREAD,PRTYC_IDLETIME,30L,0L)
#define priority_normal()   DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,0L,0L)
#define priority_tweaked()  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,1L,0L)
#define priority_bumped()   DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,3L,0L)
#define priority_critical() DosSetPriority(PRTYS_THREAD,PRTYC_FOREGROUNDSERVER,2L,0L)
#define priority_max()      DosSetPriority(PRTYS_THREAD,PRTYC_FOREGROUNDSERVER,31L,0L)
#define SysVal(value)       WinQuerySysValue(HWND_DESKTOP, (value))
