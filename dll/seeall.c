
/***********************************************************************

  $Id$

  See all matching files

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2010 Steven H. Levine

  16 Oct 02 SHL Handle large partitions
  25 Nov 03 SHL StartSeeAll: avoid forgetting startpath
  06 Dec 03 SHL StartSeeAll: correct malloc arg oops
  23 May 05 SHL Use QWL_USER
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  05 Jun 05 SHL Use QWL_USER
  06 Jun 05 SHL Drop unused code
  29 May 06 SHL Comments
  17 Jul 06 SHL Use Runtime_Error
  19 Oct 06 SHL Correct . and .. detect
  03 Nov 06 SHL Renames
  03 Nov 06 SHL Count thread usage
  30 Mar 07 GKY Remove GetPString for window class names
  03 Aug 07 GKY Enlarged and made setable everywhere Findbuf (speeds file loading)
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  07 Aug 07 SHL Use BldQuotedFullPathName and BldQuotedFileName
  13 Aug 07 SHL Sync code with other FilesToGet usage
  13 Aug 07 SHL Move #pragma alloc_text to end for OpenWatcom compat
  14 Aug 07 SHL Revert to DosSleep(0) to speed up inner loops
  14 Aug 07 SHL Drop afFilesToGet
  26 Aug 07 GKY DosSleep(1) in loops changed to (0)
  27 Sep 07 SHL Correct ULONGLONG size formatting
  30 Dec 07 GKY Use CommaFmtULL
  30 Dec 07 GKY Use TestFDates for comparing by date
  15 Feb 08 GKY Prevent trap on scan of drive containing files that exceed maxpath
  29 Feb 08 GKY Use xfree where appropriate
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  20 Jul 08 GKY Add save/append filename to clipboard.
		Change menu wording to make these easier to find
  10 Dec 08 SHL Integrate exception handler support
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  11 Jan 09 GKY Replace font names in the string file with global set at compile in init.c
  11 Jan 08 GKY Change flag on GetMLEFont to 3 from 11 to give a larger selection of mono spaced fonts
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.
  17 Jan 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  01 Dec 10 SHL Ensure FindAllThread thread quits fast when requested
  04 Aug 12 GKY Changes to use Unlock to unlock files if Unlock.exe is in path both from menu/toolbar and as part of
                copy, move and delete operations
  04 Aug 12 GKY Changes to allow copy and move over readonly files with a warning dialog; also added a warning dialog
                for delete of readonly files

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// #include <process.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "seeall.h"
#include "mainwnd2.h"			// Data declaration(s)
#include "grep.h"			// Data declaration(s)
#include "arccnrs.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "worker.h"			// Data declaration(s)
#include "newview.h"			// Data declarations
#include "fm3dlg.h"
#include "fm3str.h"
#include "pathutil.h"			// BldQuotedFullPathName...
#include "makelist.h"			// AddToList
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "notebook.h"			// targetdirectory
#include "copyf.h"			// AdjustWildcardName, make_deleteable
#include "attribs.h"			// AttrListDlgProc
#include "chklist.h"			// CenterOverWindow, CheckListProc
#include "colors.h"			// ColorDlgProc
#include "draglist.h"			// DragList
#include "extract.h"			// ExtractDlgProc
#include "info.h"			// FileInfoProc
#include "valid.h"			// GetDesktopName, IsNewer, TestFDates
#include "saveclip.h"			// ListToClipboardHab, SaveAllListDlgProc
#include "shadow.h"			// MakeShadows
#include "mkdir.h"			// MassMkdir
#include "objcnr.h"			// ObjCnrDlgProc
#include "codepage.h"			// PickCodepage
#include "printer.h"			// PrintDlgProc, PrintListThread
#include "rename.h"			// RenameProc
#include "uudecode.h"			// UUD
#include "walkem.h"			// WalkCopyDlgProc, WalkMoveDlgProc
#include "archive.h"			// ArchiveDlgProc
#include "common.h"			// CommonTextProc, DecrThreadUsage, IncrThreadUsage
#include "defview.h"			// DefaultViewKeys
#include "eas.h"			// DisplayEAsProc
#include "mainwnd.h"			// GetNextWindowPos
#include "filter.h"			// PickMaskDlgProc
#include "avl.h"			// SBoxDlgProc
#include "collect.h"			// StartCollector
#include "subj.h"			// Subject
#include "i18nutil.h"			// commafmt
#include "literal.h"			// wildcard
#include "wrappers.h"			// xDosFindNext
#include "misc.h"			// SetConditionalCascade
#include "fonts.h"			// SetMLEFont
#include "stristr.h"			// stristr
#include "systemf.h"			// runemf2
#include "fortify.h"
#include "excputil.h"			// xbeginthread

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

typedef struct
{
  CHAR *fullname, *filename;
  USHORT attrFile, flags;
  FDATE date;
  FTIME time;
  ULONGLONG cbFile;
  ULONG CRC;
}
ALLFILES;

#define AF_SELECTED     0x0001
#define AF_DELETED      0x0002
#define AF_FILTERED     0x0004
#define AF_DUPE         0x0008
#define AF_CRCED        0x0010

#define AFM_MARK        0
#define AFM_UNMARK      1
#define AFM_INVERT      2
#define AFM_MARKDELETED 3
#define AFM_FILTER      4

#define DP_NAMES        0x0001
#define DP_DATES        0x0002
#define DP_SIZES        0x0004
#define DP_CRCS         0x0008
#define DP_EXTS         0x0010

#define SEEALLFILECNR_FONT_LCID 15

#define COLORS_MAX                   8

#define COLORS_CURSOREDNORMALBACK    0
#define COLORS_CURSOREDSELECTEDBACK  1
#define COLORS_NORMALBACK            2
#define COLORS_SELECTEDBACK          3
#define COLORS_SELECTEDNORMALFORE    4
#define COLORS_NORMALFORE            5
#define COLORS_READONLYFORE          6
#define COLORS_SYSTEMFORE            7

static LONG Colors[COLORS_MAX] = { COLR_WHITE, COLR_DARKGRAY, COLR_PALEGRAY,
  COLR_BLACK, COLR_WHITE, COLR_BLACK,
  COLR_DARKBLUE, COLR_DARKRED
};

typedef int (FNSORT) (const void *, const void *);
typedef FNSORT *PFNSORT;

typedef struct
{
  USHORT size;
  USHORT dupeflags;
  ALLFILES *afhead;
  ALLFILES **afindex;
  ULONG afheadcnt;
  ULONG afalloc;
  ULONG longest;
  ULONG longestw;
  ULONG topfile;
  ULONG cursored;
  ULONG selected;
  ULONGLONG ullSelectedBytes;
  ULONG afindexcnt;
  ULONG lastselected;
  ULONG lastdirection;
  ULONG multiplier;
  ULONG lasttime;
  CHAR stopflag;
  CHAR abDrvFlags[26];
  CHAR szCommonName[CCHMAXPATH];
  CHAR szFindPath[CCHMAXPATH];
  LONG lMaxAscender;
  LONG lMaxDescender;
  LONG lMaxHeight;
  LONG maxx;
  LONG horzscroll;
  BOOL fullnames;
  BOOL invertsort;
  BOOL mousecaptured;
  HMTX hmtxScan;
  HWND hvscroll;
  HWND hhscroll;
  HWND hwndMenu;
  HWND hwndObj;
  HWND hwndClient;
  HWND hwndPopup;
  HWND hwndStatus;
  HWND hwndFrame;
  HPS hps;
  PFNSORT pfnCompare;
  MASK mask;
  FATTRS fattrs;
  LONG aulColors[8];
  BOOL killme;
}
ALLDATA;

static BOOL Fullnames = FALSE;
static BOOL Firsttime = TRUE;
static BOOL SortReverse;
static USHORT Codepage, SortType;
static FATTRS Fattrs;

extern LONG CRCFile(CHAR * filename, INT * error);

MRESULT EXPENTRY DupeDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    {
      USHORT flags = SHORT1FROMMP(mp2);

      WinCheckButton(hwnd, DUPE_NAMES, ((flags & DP_NAMES) != 0));
      WinCheckButton(hwnd, DUPE_DATES, ((flags & DP_NAMES) != 0));
      WinCheckButton(hwnd, DUPE_SIZES, ((flags & DP_NAMES) != 0));
      WinCheckButton(hwnd, DUPE_CRCS, ((flags & DP_NAMES) != 0));
      if (!(flags & DP_NAMES))
	WinEnableWindow(WinWindowFromID(hwnd, DUPE_EXTS), FALSE);
    }
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case DUPE_NAMES:
      if (WinQueryButtonCheckstate(hwnd, DUPE_NAMES))
	WinEnableWindow(WinWindowFromID(hwnd, DUPE_EXTS), TRUE);
      else {
	WinCheckButton(hwnd, DUPE_EXTS, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, DUPE_EXTS), FALSE);
      }
      break;
    case DUPE_SIZES:
      if (!WinQueryButtonCheckstate(hwnd, DUPE_SIZES))
	WinCheckButton(hwnd, DUPE_CRCS, FALSE);
      break;
    case DUPE_CRCS:
      if (WinQueryButtonCheckstate(hwnd, DUPE_CRCS))
	WinCheckButton(hwnd, DUPE_SIZES, TRUE);
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      {
	USHORT flags = 0;

	if (WinQueryButtonCheckstate(hwnd, DUPE_NAMES)) {
	  flags |= DP_NAMES;
	  if (WinQueryButtonCheckstate(hwnd, DUPE_EXTS))
	    flags |= DP_EXTS;
	}
	if (WinQueryButtonCheckstate(hwnd, DUPE_DATES))
	  flags |= DP_DATES;
	if (WinQueryButtonCheckstate(hwnd, DUPE_SIZES)) {
	  flags |= DP_SIZES;
	  if (WinQueryButtonCheckstate(hwnd, DUPE_CRCS))
	    flags |= (DP_CRCS | DP_SIZES);
	}
	if (!flags)
	  saymsg(MB_ENTER,
		 hwnd,
		 GetPString(IDS_ERRORTEXT),
		 GetPString(IDS_CHECKONETEXT));
	else
	  WinDismissDlg(hwnd, flags);
      }
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_DUPES, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static ULONG NumLines(RECTL * rcl, ALLDATA * ad)
{
  ULONG numlines;

  numlines = (rcl->yTop - rcl->yBottom) / ad->lMaxHeight;
  if (ad->lMaxDescender && numlines &&
      ((rcl->yTop - rcl->yBottom) -
       (numlines * ad->lMaxHeight) <= ad->lMaxDescender))
    numlines--;
  return numlines;
}

MRESULT EXPENTRY SeeObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case UM_MASSACTION:
    {
      CHAR **files = NULL, **list = (CHAR **) mp2, path[CCHMAXPATH];
      UINT numfiles = 0, numalloc = 0;
      INT plen = 0;
      HWND hwndFrame = WinQueryWindowULong(hwnd, QWL_USER);
      CHAR message[CCHMAXPATH * 2], wildname[CCHMAXPATH];
      register INT x;
      register CHAR *p, *pp;
      BOOL dontask = FALSE, wildcarding = FALSE,
	overold = FALSE, overnew = FALSE;

      if (!list || !list[0])
	goto Abort;
      *path = *wildname = 0;

      switch (SHORT1FROMMP(mp1)) {
      case IDM_MOVEPRESERVE:
	{
	  CHAR preserve[CCHMAXPATH], *end;

	  mp1 = MPFROM2SHORT(IDM_MOVE, SHORT2FROMMP(mp1));
	  strcpy(preserve, list[0] + 2);
	  end = strrchr(preserve, '\\');
	  if (end) {
	    end++;
	    for (x = 1; list[x]; x++) {
	      p = preserve;
	      pp = list[x] + 2;
	      while (p < end && toupper(*p) == toupper(*pp)) {
		p++;
		pp++;
	      }
	      if (*p == '\\')
		p++;
	      if (p < end)
		end = p;
	    }
	    *end = 0;
	  }
	  else
	    *preserve = 0;
	  plen = strlen(preserve);
	  if (plen)
	    plen += 2;
	}
	break;
      case IDM_COPYPRESERVE:
	{
	  CHAR preserve[CCHMAXPATH], *end;

	  mp1 = MPFROM2SHORT(IDM_COPY, SHORT2FROMMP(mp1));
	  strcpy(preserve, list[0] + 2);
	  end = strrchr(preserve, '\\');
	  if (end) {
	    end++;
	    for (x = 1; list[x]; x++) {
	      p = preserve;
	      pp = list[x] + 2;
	      while (p < end && toupper(*p) == toupper(*pp)) {
		p++;
		pp++;
	      }
	      if (*p == '\\')
		p++;
	      if (p < end)
		end = p;
	    }
	    *end = 0;
	  }
	  else
	    *preserve = 0;
	  plen = strlen(preserve);
	  if (plen)
	    plen += 2;
	}
	break;
      case IDM_WILDMOVE:
	wildcarding = TRUE;
	mp1 = MPFROM2SHORT(IDM_MOVE, SHORT2FROMMP(mp1));
	break;
      case IDM_WILDRENAME:
	wildcarding = TRUE;
	mp1 = MPFROM2SHORT(IDM_RENAME, SHORT2FROMMP(mp1));
	break;
      case IDM_WILDCOPY:
	wildcarding = TRUE;
	mp1 = MPFROM2SHORT(IDM_COPY, SHORT2FROMMP(mp1));
	break;
      }

      switch (SHORT1FROMMP(mp1)) {
      case IDM_OBJECT:
      case IDM_SHADOW:
	{
	  APIRET rc;

	  GetDesktopName(path, sizeof(path));
	  rc = WinDlgBox(HWND_DESKTOP,
			 hwndFrame,
			 ObjCnrDlgProc,
			 FM3ModHandle, OBJCNR_FRAME, MPFROMP(path));
	  if (rc) {
	    if (rc > 1)
	      strcpy(path, "<WP_DESKTOP>");
	  }
	  else {
	    FreeList(list);
	    break;
	  }
	  MakeShadows(hwndFrame,
		      list, (SHORT1FROMMP(mp1) == IDM_SHADOW), path, NULL);
	}
	FreeList(list);
	break;

      case IDM_PRINT:
	{
	  LISTINFO *li;
#         ifdef FORTIFY
	  Fortify_EnterScope();
#          endif
	  li = xmallocz(sizeof(LISTINFO), pszSrcFile, __LINE__);
	  if (li) {
	    li->hwndS = WinWindowFromID(hwndFrame, FID_CLIENT);
	    li->type = IDM_PRINT;
	    li->list = list;
	    if (WinDlgBox(HWND_DESKTOP,
			  li->hwndS,
			  PrintDlgProc,
			  FM3ModHandle, PRN_FRAME, MPFROMP(li))) {
	      if (li && li->list && li->list[0]) {
		strcpy(li->targetpath, printer);
		if (xbeginthread(PrintListThread,
				 65536,
				 li,
				 pszSrcFile,
				 __LINE__) == -1)
		{
		  FreeListInfo(li);
		}
	      }
	    }
	  }
	}
	break;

      case IDM_EAS:
	if (WinDlgBox(HWND_DESKTOP,
		      hwndFrame,
		      DisplayEAsProc, FM3ModHandle, EA_FRAME, (PVOID) list)) {
	  if (!PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
		       UM_UPDATERECORDLIST, MPFROMP(list), MPVOID))
	    FreeList(list);
	}
	else
	  FreeList(list);
	break;

      case IDM_INFO:
	if (WinDlgBox(HWND_DESKTOP,
		      hwndFrame,
		      FileInfoProc,
		      FM3ModHandle, FLE_FRAME, (PVOID) list) == 2) {
	  if (!PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
		       UM_UPDATERECORDLIST, MPFROMP(list), MPVOID))
	    FreeList(list);
	}
	else
	  FreeList(list);
	break;

      case IDM_ARCHIVE:
	{
	  DIRCNRDATA ad;
	  CHAR szBuffer[1025];

	  memset(&ad, 0, sizeof(ad));
	  ad.namecanchange = 1;
	  ad.info = arcsighead;		// Hide dups
	  if (!WinDlgBox(HWND_DESKTOP,
			 hwndFrame,
			 SBoxDlgProc,
			 FM3ModHandle,
			 ASEL_FRAME, (PVOID) & ad.info) || !ad.info) {
	    // we blew it
	    FreeList(list);
	    break;
	  }
	  if (!ad.info->create &&
	      !ad.info->move &&
	      !ad.info->createwdirs &&
	      !ad.info->movewdirs && !ad.info->createrecurse) {
	    // 14 Aug 07 SHL fixme to tell user why we failed
	    FreeList(list);
	    break;
	  }
	  if (!WinDlgBox(HWND_DESKTOP, hwndFrame, ArchiveDlgProc, FM3ModHandle, ARCH_FRAME, (PVOID) & ad) || !*ad.arcname || !*ad.command) {    /* we blew it */
	    FreeList(list);
	    break;
	  }
	  // Build archiver command line
	  strcpy(szBuffer, ad.command);
	  strcat(szBuffer, " ");
	  strcat(szBuffer, ad.arcname);
	  p = &szBuffer[strlen(szBuffer)];
	  if (ad.mask.szMask) {
	    strcat(szBuffer, " ");
	    strcat(szBuffer, ad.mask.szMask);
	  }
	  strcat(szBuffer, " ");
	  x = 0;
	  while (list[x]) {
	    FILESTATUS3 fsa;
	    memset(&fsa, 0, sizeof(fsa));
	    DosError(FERR_DISABLEHARDERR);
	    DosQueryPathInfo(list[x], FIL_STANDARD, &fsa, sizeof(fsa));
	    if (fsa.attrFile & FILE_DIRECTORY) {
	      BldQuotedFullPathName(szBuffer + strlen(szBuffer),
				    list[x], "*");
	    }
	    else
	      BldQuotedFileName(szBuffer + strlen(szBuffer), list[x]);
	    x++;
	    if (!list[x] || strlen(szBuffer) + strlen(list[x]) + 5 > 1024) {
	      runemf2(SEPARATE | WINDOWED | WAIT |
		      (fArcStuffVisible ? 0 : (BACKGROUND | MINIMIZED)),
		      HWND_DESKTOP, pszSrcFile, __LINE__,
		      NULL, NULL, "%s", szBuffer);
	      DosSleep(1);		// Let archiver get started
	      *p = 0;
	    }
	    strcat(szBuffer, " ");
	  } // while
	  AddToList(ad.arcname, &files, &numfiles, &numalloc);
	}
	if (!PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
		     UM_UPDATERECORDLIST, MPFROMP(list), MPVOID))
	  FreeList(list);
	break;

      case IDM_ATTRS:
	{
	  LISTINFO li;

	  memset(&li, 0, sizeof(li));
	  li.list = list;
	  if (WinDlgBox(HWND_DESKTOP,
			hwndFrame,
			AttrListDlgProc,
			FM3ModHandle, ATR_FRAME, MPFROMP(&li))) {
	    if (li.list && li.list[0]) {
	      if (!PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
			   UM_UPDATERECORDLIST, MPFROMP(li.list), MPVOID))
		FreeList(li.list);
	    }
	  }
	  else
	    FreeList(list);
	}
	break;

      case IDM_MOVE:
      case IDM_COPY:
	if (!*path)
	  strcpy(path, targetdir);
	if (!*path)
	  strcpy(path, list[0]);
	MakeValidDir(path);
      RetryPath:
	if (SHORT1FROMMP(mp1) == IDM_MOVE) {
	  if (!WinDlgBox(HWND_DESKTOP,
			 hwndFrame,
			 WalkMoveDlgProc,
			 FM3ModHandle, WALK_FRAME, MPFROMP(path)) || !*path) {
	    FreeList(list);
	    goto Abort;
	  }
	}
	else {
	  if (!WinDlgBox(HWND_DESKTOP,
			 hwndFrame,
			 WalkCopyDlgProc,
			 FM3ModHandle, WALK_FRAME, MPFROMP(path)) || !*path) {
	    FreeList(list);
	    goto Abort;
	  }
	}
	if (driveflags[toupper(*path) - 'A'] & DRIVE_NOTWRITEABLE) {
	  saymsg(MB_CANCEL,
		 hwndFrame,
		 GetPString(IDS_ERRORTEXT),
		 GetPString(IDS_NOTWRITENOTARGETTEXT));
	  goto RetryPath;
	}
	/* intentional fallthru */
      case IDM_RENAME:
	{
	  CHAR newname[CCHMAXPATH];
	  PCSZ moving, move, moved;
	  APIRET rc;
	  INT type;
	  FILESTATUS4L fs4;
	  BOOL isnewer, existed, fResetVerify = FALSE;

	  for (x = 0; list[x]; x++) {
	  Retry:
	    type = (SHORT1FROMMP(mp1) == IDM_RENAME) ? MOVE :
	      (SHORT1FROMMP(mp1) == IDM_MOVE) ? MOVE :
	      (SHORT1FROMMP(mp1) == IDM_WPSMOVE) ? WPSMOVE :
	      (SHORT1FROMMP(mp1) == IDM_WPSCOPY) ? WPSCOPY : COPY;
	    moving = (SHORT1FROMMP(mp1) == IDM_RENAME) ?
	      GetPString(IDS_RENAMINGTEXT) :
	      (SHORT1FROMMP(mp1) == IDM_MOVE ||
	       SHORT1FROMMP(mp1) == IDM_WPSMOVE) ?
	      GetPString(IDS_MOVINGTEXT) : GetPString(IDS_COPYINGTEXT);
	    move = (SHORT1FROMMP(mp1) == IDM_RENAME) ?
	      GetPString(IDS_RENAMETEXT) :
	      (SHORT1FROMMP(mp1) == IDM_MOVE ||
	       SHORT1FROMMP(mp1) == IDM_WPSMOVE) ?
	      GetPString(IDS_MOVETEXT) : GetPString(IDS_COPYTEXT);
	    moved = (SHORT1FROMMP(mp1) == IDM_RENAME) ?
	      GetPString(IDS_RENAMEDTEXT) :
	      (SHORT1FROMMP(mp1) == IDM_MOVE ||
	       SHORT1FROMMP(mp1) == IDM_WPSMOVE) ?
	      GetPString(IDS_MOVEDTEXT) : GetPString(IDS_COPIEDTEXT);
	    if (*path) {
	      strcpy(newname, path);
	      AddBackslashToPath(newname);
	      //if (newname[strlen(newname) - 1] != '\\')
	      //  strcat(newname, "\\");
	      if (plen)
		p = list[x] + plen;
	      else {
		p = strrchr(list[x], '\\');
		if (p)
		  p++;
		else
		  p = list[x];
	      }
	      strcat(newname, p);
	    }
	    else
	      strcpy(newname, list[x]);
	    if ((wildcarding || SHORT1FROMMP(mp1) == IDM_RENAME) && *wildname) {

	      CHAR testname[CCHMAXPATH];

	      strcpy(testname, wildname);
	      if (AdjustWildcardName(newname, testname))
		strcpy(newname, testname);
	    }
	    existed = (IsFile(newname) != -1);
	    isnewer = IsNewer(list[x], newname);
	    if (existed && SHORT1FROMMP(mp1) != IDM_RENAME && dontask) {
	      if (!overold && !overnew)
		break;
	      if (!overold && !isnewer)
		break;
	      if (!overnew && isnewer)
		break;
	    }
	    if ((SHORT1FROMMP(mp1) == IDM_RENAME &&
		 (!dontask || !*wildname)) ||
		(!dontask && existed) ||
		(!dontask && wildcarding) ||
		(IsFile(newname) == 0 && IsFile(list[x]) == 1)) {

	      MOVEIT mv;

	      memset(&mv, 0, sizeof(mv));
	      mv.rename = (SHORT1FROMMP(mp1) == IDM_RENAME);
	      mv.source = list[x];
	      strcpy(mv.target, newname);
	      rc = WinDlgBox(HWND_DESKTOP,
			     hwndFrame,
			     RenameProc,
			     FM3ModHandle, REN_FRAME, (PVOID) & mv);
	      if (!rc) {
		FreeList(list);
		goto Abort;
	      }
	      DosSleep(0);		//26 Aug 07 GKY 1
	      if (mv.skip || !*mv.target)
		break;
	      if (mv.dontask)
		dontask = TRUE;
	      if (mv.overold)
		overold = TRUE;
	      if (mv.overnew)
		overnew = TRUE;
	      if (wildcarding || SHORT1FROMMP(mp1) == IDM_RENAME) {
		p = strrchr(mv.target, '\\');
		if (p && (strchr(p, '*') || strchr(p, '?'))) {
		  strcpy(wildname, mv.target);
		  AdjustWildcardName(list[x], mv.target);
		}
		else
		  *wildname = 0;
	      }
	      strcpy(newname, mv.target);
	      existed = (IsFile(newname) != -1);
	      isnewer = IsNewer(list[x], newname);
	      if (!mv.overwrite) {
		if (existed && SHORT1FROMMP(mp1) != IDM_RENAME && dontask) {
		  if (!overold && !overnew)
		    break;
		  if (!overold && !isnewer)
		    break;
		  if (!overnew && isnewer)
		    break;
		}
	      }
	    }
	    if (!stricmp(list[x], newname))
	      break;
	    sprintf(message,
		    " %s \"%s\" %s \"%s\"",
		    moving, list[x], GetPString(IDS_TOTEXT), newname);
	    WinSetWindowText(WinWindowFromID(hwndFrame, SEEALL_STATUS),
			     message);
	    if (fRealIdle)
	      priority_idle();
	    if (fVerify && (driveflags[toupper(*list[x]) - 'A'] & DRIVE_WRITEVERIFYOFF ||
			    driveflags[toupper(*newname) - 'A'] & DRIVE_WRITEVERIFYOFF)) {
	      DosSetVerify(FALSE);
	      fResetVerify = TRUE;
	    }
	    if (plen) {
	      /* make directory/ies, if required */

	      CHAR dirpart[CCHMAXPATH];

	      strcpy(dirpart, newname);
	      p = strrchr(dirpart, '\\');
	      if (p) {
		*p = 0;
		if (p > dirpart + 3)
		  MassMkdir((hwndMain) ? hwndMain : (HWND) 0, dirpart);
	      }
	    }
	    rc = docopyf(type, list[x], newname);
	    if (fResetVerify) {
	      DosSetVerify(fVerify);
	      fResetVerify = FALSE;
	    }
	    priority_normal();
	    if (rc) {
	      if ((rc == ERROR_DISK_FULL ||
		   rc == ERROR_HANDLE_DISK_FULL) &&
		  isalpha(*newname) &&
		  (driveflags[toupper(*newname) - 'A'] & DRIVE_REMOVABLE) &&
		  !(driveflags[toupper(*newname) - 'A'] & DRIVE_NOTWRITEABLE)
		  && toupper(*newname) != toupper(*list[x])
		  && !DosQueryPathInfo(list[x], FIL_QUERYEASIZEL, &fs4,
				       sizeof(fs4))
		  && !(fs4.attrFile & FILE_DIRECTORY)) {

		FSALLOCATE fsa;
		ULONGLONG ullFreeBytes;
		CHAR *ptr;
		INT cntr;

		WinSetWindowText(WinWindowFromID(hwndFrame, SEEALL_STATUS),
				 (CHAR *) GetPString(IDS_FITTINGTEXT));
		DosError(FERR_DISABLEHARDERR);
		if (!DosQueryFSInfo(toupper(*newname) - '@',
				    FSIL_ALLOC, &fsa, sizeof(fsa))) {
		  // Assume large file support
		  ullFreeBytes = (ULONGLONG) fsa.cUnitAvail * fsa.cSectorUnit *
				fsa.cbSector;
		  if (ullFreeBytes) {
		    // Find item that will fit in available space
		    for (cntr = x + 1; list[cntr]; cntr++) {
		      DosError(FERR_DISABLEHARDERR);
		      if (!DosQueryPathInfo(list[cntr],
					    FIL_QUERYEASIZEL,
					    &fs4, sizeof(fs4)) &&
			  !(fs4.attrFile & FILE_DIRECTORY) &&
			  // fixme to use CBLIST_TO_EASIZE?
			  fs4.cbFile + fs4.cbList <= ullFreeBytes) {
			// Swap with failing item
			ptr = list[x];
			list[x] = list[cntr];
			list[cntr] = ptr;
			goto Retry;
		      }
		    }
		    WinSetWindowText(WinWindowFromID(hwndFrame,
						     SEEALL_STATUS),
				     (CHAR *) GetPString(IDS_COULDNTFITTEXT));
		  }
		}
		rc = saymsg(MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION,
			    hwndFrame,
			    GetPString(IDS_DISKFULLTEXT),
			    GetPString(IDS_ANOTHERDISKTEXT));
		if (rc == MBID_RETRY)
		  goto Retry;
		if (rc == MBID_ABORT) {
		  FreeList(list);
		  goto Abort;
		}
	      }
	      else {
		if (LogFileHandle)
		  fprintf(LogFileHandle,
			  GetPString(IDS_LOGTOFAILEDTEXT),
			  move, list[x], newname, rc);
		rc = Dos_Error(MB_ENTERCANCEL,
			       rc,
			       hwndFrame,
			       pszSrcFile,
			       __LINE__,
			       "%s %s \"%s\" %s\"%s\" %s.",
			       move,
			       GetPString(IDS_OFTEXT),
			       list[x],
			       GetPString(IDS_TOTEXT),
			       newname, GetPString(IDS_FAILEDTEXT));
		if (rc == MBID_CANCEL) {
		  FreeList(list);
		  goto Abort;
		}
	      }
	    }
	    else {
	      if (LogFileHandle)
		fprintf(LogFileHandle,
			"%s \"%s\" %s \"%s\"\n",
			moved, list[x], GetPString(IDS_TOTEXT), newname);
	      AddToList(newname, &files, &numfiles, &numalloc);
	    }
	  }
	}
	if (SHORT1FROMMP(mp1) != IDM_COPY) {
	  if (!PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
		       UM_UPDATERECORDLIST, MPFROMP(list), MPVOID))
	    FreeList(list);
	}
	else
	  FreeList(list);
	break;

      case IDM_UUDECODE:
	for (x = 0; list[x]; x++)
	  UUD(list[x], NULL);
        break;

      case IDM_UNLOCKFILE:
        for (x = 0; list[x]; x++) {
          if (IsFile(list[x]) > 0 && fUnlock) {
            runemf2(SEPARATE | INVISIBLE | BACKGROUND | WAIT,
                    HWND_DESKTOP, pszSrcFile, __LINE__,
                    NULL, NULL, "%s %s", PCSZ_UNLOCKEXE, list[x]);
          }
        }
        break;

      case IDM_EXTRACT:
	for (x = 0; list[x]; x++) {

	  EXTRDATA ex;
	  BOOL maskspaces = FALSE;

	  memset(&ex, 0, sizeof(ex));
	  ex.info = find_type(list[x], NULL);
	  if (!ex.info || (!ex.info->extract && !ex.info->exwdirs))
	    break;
	  ex.size = sizeof(ex);
	  ex.arcname = list[x];
	  strcpy(ex.masks, "*");
	  if (!WinDlgBox(HWND_DESKTOP,
			 hwndFrame,
			 ExtractDlgProc,
			 FM3ModHandle,
			 EXT_FRAME,
			 (PVOID) & ex) ||
	      !ex.ret || !*ex.command || !*ex.arcname || !*ex.extractdir)
	    break;
	  if (IsFile(ex.extractdir) != 0)
	    Runtime_Error(pszSrcFile, __LINE__, "directory expected");
	  else {
	    if (needs_quoting(ex.masks) && !strchr(ex.masks, '\"'))
	      maskspaces = TRUE;
	    runemf2(SEPARATE | WINDOWED |
		    (fArcStuffVisible ? 0 : (BACKGROUND | MINIMIZED)),
		    HWND_DESKTOP, pszSrcFile, __LINE__,
		    ex.extractdir, NULL,
		    "%s %s %s%s%s",
		    ex.command,
		    ex.arcname,
		    maskspaces ? "\"" : NullStr,
		    *ex.masks ? ex.masks : "*",
		    maskspaces ? "\"" : NullStr);
	  }
	}
	// fixme to not leak?
	if (!PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
		     UM_UPDATERECORDLIST, MPFROMP(list), MPVOID))
	  FreeList(list);
	break;

      case IDM_SUBJECT:
	for (x = 0; list[x]; x++) {

	  INT ret;

	  if (IsFile(list[x]) == 1) {
	    ret = Subject(hwndFrame, list[x]);
	    if (!ret)
	      break;
	  }
	}
	if (!PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
		     UM_UPDATERECORDLIST, MPFROMP(list), MPVOID))
	  FreeList(list);
	break;

      case IDM_OPENDEFAULT:
      case IDM_OPENSETTINGS:
	for (x = 0; list[x]; x++) {
	  if (IsFile(list[x]) != -1) {

	    CHAR *s;

	    switch (SHORT1FROMMP(mp1)) {
	    case IDM_OPENSETTINGS:
	      s = (PSZ) Settings;
	      break;
	    default:
	      s = (PSZ) Default;
	      break;
	    }
	    OpenObject(list[x], s, hwndFrame);
	  }
	}
	FreeList(list);
	break;

      case IDM_DELETE:
      case IDM_PERMDELETE:
	{
	  CHECKLIST cl;
	  INT isdir = 0, sysdir = 0, ro = 0, hs = 0;
	  FILESTATUS3 fsa;
	  CHAR prompt[CCHMAXPATH * 3];
	  APIRET error;

	  for (x = 0; list[x]; x++) {
	    if (IsRoot(list[x])) {
	      list = RemoveFromList(list, list[x]);
	      if (!list)
		break;
	      x--;
	      continue;
	    }
	    DosError(FERR_DISABLEHARDERR);
	    if (DosQueryPathInfo(list[x], FIL_STANDARD, &fsa, sizeof(fsa))) {
	      list = RemoveFromList(list, list[x]);
	      if (!list)
		break;
	      x--;
	      continue;
	    }
	    if (fsa.attrFile & FILE_DIRECTORY) {
	      isdir++;
	      if (stristr(list[x], ":\\OS2\\") ||
		  !stricmp(list[x] + 1, ":\\OS2"))
		sysdir++;
	    }
	    else {
	      if (fsa.attrFile & (FILE_HIDDEN | FILE_SYSTEM))
		hs++;
	      if (fsa.attrFile & FILE_READONLY)
		ro++;
	    }
	  }
	  if (!list)
	    break;
	  if (fConfirmDelete || isdir) {
	    memset(&cl, 0, sizeof(cl));
	    cl.size = sizeof(cl);
	    cl.list = list;
	    cl.prompt = prompt;
	    cl.flags |= CHECK_FILES;
	    cl.cmd = SHORT1FROMMP(mp1);
	    sprintf(prompt,
		    GetPString(IDS_DELPROMPT1TEXT),
		    (SHORT1FROMMP(mp1) == IDM_DELETE) ?
		    NullStr :
		    GetPString(IDS_PERMANENTLYTEXT), &"s"[list[1] == NULL]);
	    if (isdir) {
	      sprintf(&prompt[strlen(prompt)],
		      GetPString(IDS_DELPROMPT2TEXT),
		      isdir,
		      (isdir > 1) ?
		      GetPString(IDS_ARETEXT) :
		      GetPString(IDS_ISTEXT),
		      (isdir == 1) ?
		      GetPString(IDS_ATEXT) :
		      NullStr,
		      (isdir > 1) ?
		      GetPString(IDS_IESTEXT) : GetPString(IDS_YTEXT));
	      if (sysdir)
		sprintf(&prompt[strlen(prompt)],
			GetPString(IDS_DELPROMPT3TEXT),
			sysdir,
			(sysdir == 1) ?
			GetPString(IDS_YTEXT) : GetPString(IDS_IESTEXT));
	    }
	    if (ro)
	      sprintf(&prompt[strlen(prompt)],
		      GetPString(IDS_DELPROMPT4TEXT),
		      ro,
		      &"s"[ro == 1],
		      (ro > 1) ?
		      GetPString(IDS_ARETEXT) : GetPString(IDS_ISTEXT));
	    if (hs)
	      sprintf(&prompt[strlen(prompt)],
		      GetPString(IDS_DELPROMPT5TEXT),
		      hs,
		      &"s"[hs == 1],
		      (hs > 1) ?
		      GetPString(IDS_ARETEXT) : GetPString(IDS_ISTEXT));
	    if ((ro || hs || sysdir) && !fAlertBeepOff)
	      DosBeep(300, 100);
	    strcat(prompt, GetPString(IDS_DELPROMPT6TEXT));
	    if (!WinDlgBox(HWND_DESKTOP,
			   WinWindowFromID(hwndFrame, FID_CLIENT),
			   CheckListProc,
			   FM3ModHandle, CHECK_FRAME, MPFROMP(&cl)))
	      break;
	    list = cl.list;
	    if (!list || !list[0])
	      break;
	  }
	  for (x = 0; list[x]; x++) {
	    fsa.attrFile = 0;
	    DosError(FERR_DISABLEHARDERR);
	    DosQueryPathInfo(list[x], FIL_STANDARD, &fsa, sizeof(fsa));
	    if (fsa.attrFile & FILE_DIRECTORY) {
	      sprintf(prompt, GetPString(IDS_DELETINGTEXT), list[x]);
	      WinSetWindowText(WinWindowFromID(hwndFrame, SEEALL_STATUS),
			       prompt);
	      error = (APIRET) wipeallf("%s%s*",
					list[x],
					(*list[x] &&
					 list[x][strlen(list[x]) - 1] !=
					 '\\') ? PCSZ_BACKSLASH : NullStr);
	      DosError(FERR_DISABLEHARDERR);
	      if (!error)
		error = DosDeleteDir(list[x]);
	      else
		DosDeleteDir(list[x]);
	    }
	    else {
	      DosError(FERR_DISABLEHARDERR);
	      if (SHORT1FROMMP(mp1) == IDM_DELETE)
		error = DosDelete(list[x]);
	      else
		error = DosForceDelete(list[x]);
	      if (error) {
		DosError(FERR_DISABLEHARDERR);
		make_deleteable(list[x], error);
		if (SHORT1FROMMP(mp1) == IDM_DELETE)
		  error = DosDelete(list[x]);
		else
		  error = DosForceDelete(list[x]);
	      }
	    }
	    if (error) {
	      if (LogFileHandle)
		fprintf(LogFileHandle,
			GetPString(IDS_DELETEFAILED1TEXT), list[x], error);
	      if (Dos_Error(MB_ENTERCANCEL,
			    error,
			    hwndFrame,
			    pszSrcFile,
			    __LINE__,
			    GetPString(IDS_DELETEFAILED2TEXT),
			    list[x]) == MBID_CANCEL)
		break;
	    }
	    else if (LogFileHandle)
	      fprintf(LogFileHandle,
		      "%s\n", GetPString(IDS_DELETEDTEXT), list[x]);
	    AddToList(list[x], &files, &numfiles, &numalloc);
	  }
	}
	FreeList(list);
	break;

      case IDM_SAVETOLIST:
	if (list) {
	  WinDlgBox(HWND_DESKTOP,
		    WinWindowFromID(hwndFrame, FID_CLIENT),
		    SaveAllListDlgProc,
		    FM3ModHandle, SAV_FRAME, MPFROMP(list));
	  FreeList(list);
	}
	break;

      case IDM_SAVETOCLIP:
      case IDM_APPENDTOCLIP:
      case IDM_SAVETOCLIPFILENAME:
      case IDM_APPENDTOCLIPFILENAME:
	if (list) {
	  ListToClipboardHab(WinQueryAnchorBlock(hwnd),
			     list, SHORT1FROMMP(mp1));
	  FreeList(list);
	}
	break;

      default:
	if (list)
	  FreeList(list);
	break;
      }

      switch (SHORT1FROMMP(mp1)) {
      case IDM_MOVE:
      case IDM_COPY:
      case IDM_RENAME:
	sprintf(message,
		GetPString(IDS_OPSCOMPLETETEXT),
		SHORT1FROMMP(mp1) == IDM_MOVE ?
		  GetPString(IDS_MOVETEXT) :
		  SHORT1FROMMP(mp1) == IDM_COPY ?
		    GetPString(IDS_COPYTEXT) :
		    SHORT1FROMMP(mp1) == IDM_WPSMOVE ?
		      GetPString(IDS_WPSMOVETEXT) :
		      SHORT1FROMMP(mp1) == IDM_WPSCOPY ?
			GetPString(IDS_WPSCOPYTEXT) :
			GetPString(IDS_RENAMETEXT),
		&"s"[x == 1],
		SHORT1FROMMP(mp1) == IDM_MOVE ||
		SHORT1FROMMP(mp1) == IDM_COPY ||
		SHORT1FROMMP(mp1) == IDM_WPSMOVE ||
		SHORT1FROMMP(mp1) == IDM_WPSCOPY ?
		  GetPString(IDS_TOTEXT) : NullStr,
		SHORT1FROMMP(mp1) == IDM_MOVE ||
		SHORT1FROMMP(mp1) == IDM_COPY ||
		SHORT1FROMMP(mp1) == IDM_WPSCOPY ?
		  path : NullStr,
		x != 1 ? GetPString(IDS_ARETEXT) : GetPString(IDS_ISTEXT));
	WinSetWindowText(WinWindowFromID(hwndFrame, SEEALL_STATUS), message);
	if (toupper(*path) < 'C' && !fAlertBeepOff)
	  DosBeep(1000, 25);
	DosSleep(16);			// 05 Aug 07 GKY 33
	break;

      default:
	break;
      }
    Abort:
      if (files) {
	if (!PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
		     UM_UPDATERECORDLIST, MPFROMP(files), MPVOID))
	  FreeList(files);
      }
      PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
	      UM_RESCAN, MPVOID, MPVOID);
    }
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    break;

  case WM_DESTROY:
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

static VOID MakeSeeObjWinThread(VOID * args)
{
  HAB hab2;
  HMQ hmq2;
  HWND hwndObj;
  ALLDATA *ad = (ALLDATA *) args;
  QMSG qmsg2;

  if (ad) {
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
    hab2 = WinInitialize(0);
    if (hab2) {
      hmq2 = WinCreateMsgQueue(hab2, 256);
      if (hmq2) {
	DosError(FERR_DISABLEHARDERR);
	WinRegisterClass(hab2,
			 (CHAR *) WC_OBJECTWINDOW,
			 SeeObjWndProc, 0, sizeof(PVOID));
	hwndObj = WinCreateWindow(HWND_OBJECT,
				  (CHAR *) WC_OBJECTWINDOW,
				  (PSZ) NULL,
				  0,
				  0,
				  0,
				  0,
				  0, 0, HWND_TOP, SEEALL_OBJ, NULL, NULL);
	if (!hwndObj) {
	  Win_Error(HWND_OBJECT, HWND_DESKTOP, pszSrcFile, __LINE__,
		    PCSZ_WINCREATEWINDOW);
	  if (!PostMsg(ad->hwndClient, WM_CLOSE, MPVOID, MPVOID))
	    WinSendMsg(ad->hwndClient, WM_CLOSE, MPVOID, MPVOID);
	}
	else {
	  ad->hwndObj = hwndObj;
	  WinSetWindowULong(hwndObj, QWL_USER, ad->hwndFrame);
	  priority_normal();
	  while (WinGetMsg(hab2, &qmsg2, (HWND) 0, 0, 0))
	    WinDispatchMsg(hab2, &qmsg2);
	  WinDestroyWindow(hwndObj);
	}
	WinDestroyMsgQueue(hmq2);
      }
      else
	WinTerminate(hab2);
    }
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
  }
}

static VOID SelectMask(HWND hwnd, BOOL deselect)
{
  MASK mask;
  register ULONG x, y, z;
  BOOL ret;
  ALLDATA *pAD = WinQueryWindowPtr(hwnd, QWL_USER);

  memset(&mask, 0, sizeof(mask));
  mask.fNoAttribs = FALSE;
  mask.fNoDirs = TRUE;
  mask.attrFile = pAD->mask.attrFile;
  mask.antiattr = pAD->mask.antiattr;
  mask.fIsSeeAll = TRUE;
  strcpy(mask.prompt,
	 GetPString((!deselect) ?
		    IDS_SELECTFILTERTEXT : IDS_DESELECTFILTERTEXT));
  if (WinDlgBox(HWND_DESKTOP,
		hwnd,
		PickMaskDlgProc,
		FM3ModHandle,
		MSK_FRAME,
		MPFROMP(&mask)) &&
      (*mask.szMask ||
       mask.attrFile != pAD->mask.attrFile ||
       mask.antiattr != pAD->mask.antiattr)) {
    for (x = 0; x < pAD->afindexcnt; x++) {
      y = (pAD->invertsort) ? (pAD->afindexcnt - 1) - x : x;
      ret = FALSE;
      if (mask.pszMasks[1]) {
	for (z = 0; mask.pszMasks[z]; z++) {
	  if (*mask.pszMasks[z]) {
	    if (*mask.pszMasks[z] != '/') {
	      if (wildcard((strchr(mask.pszMasks[z], '\\') ||
			    strchr(mask.pszMasks[z], ':')) ?
			   pAD->afindex[y]->fullname : pAD->afindex[y]->
			   filename, mask.pszMasks[z], FALSE))
		ret = TRUE;
	    }
	    else {
	      if (wildcard((strchr(mask.pszMasks[z], '\\') ||
			    strchr(mask.pszMasks[z], ':')) ?
			   pAD->afindex[y]->fullname : pAD->afindex[y]->
			   filename, mask.pszMasks[y] + 1, FALSE)) {
		ret = FALSE;
		break;
	      }
	    }
	  }
	}
      }
      else if (*mask.szMask) {
	if (wildcard((strchr(mask.szMask, '\\') ||
		      strchr(mask.szMask, ':')) ?
		     pAD->afindex[y]->fullname : pAD->afindex[y]->filename,
		     mask.szMask, FALSE))
	  ret = TRUE;
      }
      else
	ret = TRUE;
      if (ret) {
	if ((!(mask.attrFile & FILE_HIDDEN)
	     && (pAD->afindex[y]->attrFile & FILE_HIDDEN))
	    || (!(mask.attrFile & FILE_SYSTEM)
		&& (pAD->afindex[y]->attrFile & FILE_SYSTEM))
	    || (!(mask.attrFile & FILE_READONLY)
		&& (pAD->afindex[y]->attrFile & FILE_READONLY))
	    || (!(mask.attrFile & FILE_ARCHIVED)
		&& (pAD->afindex[y]->attrFile & FILE_ARCHIVED)))
	  ret = FALSE;
	else
	  if (((mask.antiattr & FILE_HIDDEN)
	       && !(pAD->afindex[y]->attrFile & FILE_HIDDEN))
	      || ((mask.antiattr & FILE_SYSTEM)
		  && !(pAD->afindex[y]->attrFile & FILE_SYSTEM))
	      || ((mask.antiattr & FILE_READONLY)
		  && !(pAD->afindex[y]->attrFile & FILE_READONLY))
	      || ((mask.antiattr & FILE_ARCHIVED)
		  && !(pAD->afindex[y]->attrFile & FILE_ARCHIVED)))
	  ret = FALSE;
      }
      if (ret) {
	if (deselect) {
	  if (pAD->afindex[y]->flags & AF_SELECTED) {
	    pAD->selected--;
	    pAD->ullSelectedBytes -= pAD->afindex[y]->cbFile;
	    pAD->afindex[y]->flags &= ~AF_SELECTED;
	  }
	}
	else {
	  if (~pAD->afindex[y]->flags & AF_SELECTED) {
	    pAD->selected++;
	    pAD->ullSelectedBytes += pAD->afindex[y]->cbFile;
	    pAD->afindex[y]->flags |= AF_SELECTED;
	  }
	}
      }
    }                                   // for
  }
}

static VOID CollectList(HWND hwnd, CHAR ** list)
{
  if (!Collector) {
    if (hwndMain && !fExternalCollector && !strcmp(realappname, FM3Str)) {

      HWND hwndC;
      SWP swp;

      if (!fAutoTile)
	GetNextWindowPos(hwndMain, &swp, NULL, NULL);
      hwndC = StartCollector(hwndMain, 4);
      if (hwndC) {
	if (!fAutoTile)
	  WinSetWindowPos(hwndC, HWND_TOP, swp.x, swp.y,
			  swp.cx, swp.cy, SWP_MOVE | SWP_SIZE |
			  SWP_SHOW | SWP_ZORDER);
	else
	  TileChildren(hwndMain, TRUE);
	WinSetWindowPos(hwndC, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE);
	DosSleep(100);//05 Aug 07 GKY 250
      }
    }
    else {
      StartCollector(HWND_DESKTOP, 4);
      DosSleep(100);//05 Aug 07 GKY 250
    }
  }
  if (!PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_COLLECTOR, 0),
	       MPFROMP(list)))
    FreeList(list);
}

static VOID FreeAllFilesList(HWND hwnd)
{
  ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  register ULONG x;

  if (ad->afhead && ad->afheadcnt) {
    for (x = 0; x < ad->afheadcnt; x++) {
      xfree(ad->afhead[x].fullname, pszSrcFile, __LINE__);
    }
    free(ad->afhead);
    ad->afhead = NULL;
    xfree(ad->afindex, pszSrcFile, __LINE__);
    ad->afindex = NULL;
  }
  DosPostEventSem(CompactSem);
  ad->afalloc = ad->afindexcnt = ad->afheadcnt = ad->longest = ad->longestw =
    ad->maxx = ad->horzscroll = 0;
}

static CHAR **BuildAList(HWND hwnd)
{
  ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  ULONG x;
  ULONG y;
  ULONG z = 0;
  CHAR **list = NULL;
  UINT numfiles = 0;
  UINT numalloc = 0;
  INT error;

  if (ad->selected) {
    for (x = 0; x < ad->afindexcnt; x++) {
      y = (ad->invertsort) ? (ad->afindexcnt - 1) - x : x;
      if (ad->afindex[y]->flags & AF_SELECTED) {
	error = AddToList(ad->afindex[y]->fullname, &list,
			  &numfiles, &numalloc);
	if (error)
	  break;
	z++;
	if (z >= ad->selected)
	  break;
      }
    }
  }
  return list;
}

static BOOL Mark(HWND hwnd, INT command, CHAR ** list)
{
  /* Marks only unfiltered files */

  ALLDATA *pAD = WinQueryWindowPtr(hwnd, QWL_USER);
  register ULONG x, y, z;
  BOOL ret = TRUE;
  BOOL didone = FALSE;

  for (x = 0; x < pAD->afindexcnt; x++) {
    y = (pAD->invertsort) ? (pAD->afindexcnt - 1) - x : x;
    if (list) {
      ret = FALSE;
      for (z = 0; list[z]; z++) {
	if (!stricmp(list[z], pAD->afindex[y]->fullname)) {
	  ret = TRUE;
	  break;
	}
      }
    }
    if (ret) {
      didone = TRUE;
      if (command == AFM_UNMARK) {
	if (pAD->afindex[y]->flags & AF_SELECTED) {
	  pAD->selected--;
	  pAD->ullSelectedBytes -= pAD->afindex[y]->cbFile;
	  pAD->afindex[y]->flags &= ~AF_SELECTED;
	}
      }
      else if (command == AFM_MARK) {
	if (~pAD->afindex[y]->flags & AF_SELECTED) {
	  pAD->selected++;
	  pAD->ullSelectedBytes += pAD->afindex[y]->cbFile;
	  pAD->afindex[y]->flags |= AF_SELECTED;
	}
      }
      else if (command == AFM_INVERT) {
	if (pAD->afindex[y]->flags & AF_SELECTED) {
	  pAD->selected--;
	  pAD->ullSelectedBytes -= pAD->afindex[y]->cbFile;
	  pAD->afindex[y]->flags &= ~AF_SELECTED;
	}
	else {
	  pAD->selected++;
	  pAD->ullSelectedBytes += pAD->afindex[y]->cbFile;
	  pAD->afindex[y]->flags |= AF_SELECTED;
	}
      }
      else if (command == AFM_MARKDELETED) {
	if (pAD->afindex[y]->flags & AF_SELECTED)
	  pAD->afindex[y]->flags |= AF_DELETED;
      }
      else if (command == AFM_FILTER) {
	if (pAD->afindex[y]->flags & AF_SELECTED)
	  pAD->afindex[y]->flags |= AF_FILTERED;
      }
    }
  }                                     // for x
  return didone;
}

static BOOL UpdateList(HWND hwnd, CHAR **list)
{
  /* Updates files in the list */

  ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  ULONG x, z;
  BOOL ret, didone = FALSE;
  FILEFINDBUF3L ffb;
  ULONG ulFindCnt;
  HDIR hdir;
  CHAR *p;

  if (list) {
    for (z = 0; list[z] && !ad->stopflag; z++) {
      ret = FALSE;
      for (x = 0; x < ad->afheadcnt; x++) {
	if (!stricmp(list[z], ad->afhead[x].fullname)) {
	  ret = TRUE;
	  break;
	}
      }
      if (ret) {
	didone = TRUE;
	hdir = HDIR_CREATE;
	ulFindCnt = 1;
	if (!xDosFindFirst(list[z], &hdir, FILE_NORMAL | FILE_ARCHIVED |
			   FILE_DIRECTORY | FILE_READONLY | FILE_SYSTEM |
			   FILE_HIDDEN, &ffb, sizeof(ffb), &ulFindCnt,
			   FIL_STANDARDL)) {
	  DosFindClose(hdir);
	  if (!(ffb.attrFile & FILE_DIRECTORY)) {
	    ad->afhead[x].attrFile = (USHORT) ffb.attrFile;
	    ad->afhead[x].cbFile = ffb.cbFile;
	    ad->afhead[x].date = ffb.fdateLastWrite;
	    ad->afhead[x].time = ffb.ftimeLastWrite;
	  }
	  else
	    ad->afhead[x].flags |= AF_DELETED;
	}
	else
	  ad->afhead[x].flags |= AF_DELETED;
      }
      else if (isalpha(*list[z]) && ad->abDrvFlags[toupper(*list[z]) - 'A']) {
	didone = TRUE;
	hdir = HDIR_CREATE;
	ulFindCnt = 1;
	if (!xDosFindFirst(list[z], &hdir, FILE_NORMAL | FILE_ARCHIVED |
			   FILE_DIRECTORY | FILE_READONLY | FILE_SYSTEM |
			   FILE_HIDDEN, &ffb, sizeof(ffb), &ulFindCnt,
			   FIL_STANDARDL)) {
	  DosFindClose(hdir);
	  if (!(ffb.attrFile & FILE_DIRECTORY)) {
	    if (!ad->afalloc || ad->afheadcnt > ad->afalloc - 1) {

	      ALLFILES *temp, **templ;

	      temp =
		xrealloc(ad->afhead, (ad->afalloc + 1) * sizeof(ALLFILES),
			 pszSrcFile, __LINE__);
	      if (!temp) {
		ad->stopflag = 1;
		break;
	      }
	      else {
		ad->afhead = temp;
		templ =
		  xrealloc(ad->afindex,
			   (ad->afalloc + 1) * sizeof(ALLFILES *), pszSrcFile,
			   __LINE__);
		if (!templ) {
		  ad->stopflag = 1;
		  break;
		}
		else
		  ad->afindex = templ;
		ad->afalloc++;
	      }
	    }
	    ad->afhead[ad->afheadcnt].fullname =
	      xstrdup(list[z], pszSrcFile, __LINE__);
	    if (ad->afhead[ad->afheadcnt].fullname) {
	      p = strrchr(ad->afhead[ad->afheadcnt].fullname, '\\');
	      if (!p)
		p = ad->afhead[ad->afheadcnt].fullname;
	      else
		p++;
	      ad->afhead[ad->afheadcnt].filename = p;
	      ad->afhead[ad->afheadcnt].cbFile = ffb.cbFile;
	      ad->afhead[ad->afheadcnt].date = ffb.fdateLastWrite;
	      ad->afhead[ad->afheadcnt].time = ffb.ftimeLastWrite;
	      ad->afhead[ad->afheadcnt].attrFile = (USHORT) ffb.attrFile;
	      ad->afhead[ad->afheadcnt].flags = 0;
	      if (ad->longest < strlen(ad->afhead[ad->afheadcnt].filename))
		ad->longest = strlen(ad->afhead[ad->afheadcnt].filename);
	      if (ad->longestw < strlen(ad->afhead[ad->afheadcnt].fullname))
		ad->longestw = strlen(ad->afhead[ad->afheadcnt].fullname);

	      ad->afheadcnt++;
	    }
	    else {
	      ad->stopflag = 1;
	      break;
	    }
	  }
	}
      }
    }
  }
  return didone;
}

static int comparefullnames(const void *v1, const void *v2)
{
  ALLFILES *d1 = *(ALLFILES **) v1;
  ALLFILES *d2 = *(ALLFILES **) v2;
  int ret;

  ret = stricmp(d1->fullname, d2->fullname);
  return ret;
}

static int comparenames(const void *v1, const void *v2)
{
  ALLFILES *d1 = *(ALLFILES **) v1;
  ALLFILES *d2 = *(ALLFILES **) v2;
  int ret;

  ret = stricmp(d1->filename, d2->filename);
  if (!ret)
    ret = comparefullnames(v1, v2);
  return ret;
}

static int compareexts(const void *v1, const void *v2)
{
  ALLFILES *d1 = *(ALLFILES **) v1;
  ALLFILES *d2 = *(ALLFILES **) v2;
  register CHAR *p1, *p2;
  int ret;

  p1 = strrchr(d1->filename, '.');
  p2 = strrchr(d2->filename, '.');
  if (!p1)
    p1 = NullStr;
  else
    p1++;
  if (!p2)
    p2 = NullStr;
  else
    p2++;
  ret = stricmp(p1, p2);
  if (!ret)
    ret = comparenames(v1, v2);
  return ret;
}

static int comparesizes(const void *v1, const void *v2)
{
  ALLFILES *d1 = *(ALLFILES **) v1;
  ALLFILES *d2 = *(ALLFILES **) v2;
  int ret;

  ret = (d1->cbFile > d2->cbFile) ? 1 : (d1->cbFile == d2->cbFile) ? 0 : -1;
  if (!ret)
    ret = comparenames(v1, v2);
  return ret;
}

static int comparedates(const void *v1, const void *v2)
{
  ALLFILES *d1 = *(ALLFILES **) v1;
  ALLFILES *d2 = *(ALLFILES **) v2;
  int ret;

  ret = TestFDates(NULL, NULL,
		  &d2->date, &d2->time,
		  &d1->date, &d1->time);

  if (!ret)
    ret = comparenames(v1, v2);
  return ret;
}

static VOID ReSort(HWND hwnd)
{
  ALLDATA *pAD = WinQueryWindowPtr(hwnd, QWL_USER);
  register ULONG x, y;

  pAD->selected = 0;
  pAD->ullSelectedBytes = 0;
  for (x = 0, y = 0; x < pAD->afheadcnt; x++) {
    if (!(pAD->afhead[x].flags & (AF_DELETED | AF_FILTERED))) {
      if (pAD->afhead[x].flags & AF_SELECTED) {
	pAD->selected++;
	pAD->ullSelectedBytes += pAD->afhead[x].cbFile;
      }
      pAD->afindex[y++] = &(pAD->afhead[x]);
    }
  }                                     // for x
  pAD->afindexcnt = y;
  PostMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
  if (!pAD->stopflag && pAD->pfnCompare && pAD->afindexcnt) {
    WinSendMsg(hwnd, UM_RESCAN, MPFROMLONG(1), MPVOID);
    qsort(pAD->afindex, pAD->afindexcnt, sizeof(ALLFILES *), pAD->pfnCompare);
  }
}

VOID FindDupesThread(VOID * args)
{
  register ULONG x, z;
  register CHAR *px, *pz;
  CHAR s[80];
  INT error;
  HWND hwnd = (HWND) args;
  HAB hab2 = (HAB) 0;
  HMQ hmq2 = (HMQ) 0;
  ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif
  if (!DosRequestMutexSem(ad->hmtxScan, SEM_INDEFINITE_WAIT)) {
    priority_normal();
    hab2 = WinInitialize(0);
    if (hab2) {
      hmq2 = WinCreateMsgQueue(hab2, 0);
      if (hmq2) {
	WinCancelShutdown(hmq2, TRUE);
	IncrThreadUsage();
	if (ad->cursored <= ad->afindexcnt) {
	  for (x = 0; x < ad->afheadcnt; x++)
	    ad->afhead[x].flags &= (~(AF_DUPE | AF_SELECTED));
	  DosSleep(0);  //26 Aug 07 GKY 1
	  for (x = 0; x < ad->afheadcnt && !ad->stopflag; x++) {
	    if (!(ad->afhead[x].flags & (AF_DUPE | AF_FILTERED))) {
	      if (!(x % 50)) {
		sprintf(s,
			GetPString(IDS_DUPECHECKINGOFTEXT), x, ad->afheadcnt);
		WinSetWindowText(ad->hwndStatus, s);
	      }
	      for (z = 0; z < ad->afheadcnt && !ad->stopflag; z++) {
		if (x != z &&
		    !(ad->afhead[z].flags & (AF_DUPE | AF_FILTERED)))
		{
		  if (ad->dupeflags & DP_SIZES) {
		    if (ad->afhead[x].cbFile != ad->afhead[z].cbFile)
		      goto SkipNonDupe;
		  }
		  if (ad->dupeflags & DP_DATES) {
		    if (*(INT *) & ad->afhead[x].date !=
			*(INT *) & ad->afhead[z].date ||
			*(INT *) & ad->afhead[x].time !=
			*(INT *) & ad->afhead[z].time)
		      goto SkipNonDupe;
		  }
		  if (ad->dupeflags & DP_NAMES) {
		    if (ad->dupeflags & DP_EXTS) {
		      px = strrchr(ad->afhead[x].filename, '.');
		      pz = strrchr(ad->afhead[z].filename, '.');
		      if ((px || pz) && (!px || !pz))
			goto SkipNonDupe;
		      if (px) {
			*px = 0;
			*pz = 0;
		      }
		    }
		    if (stricmp(ad->afhead[x].filename,
				ad->afhead[z].filename)) {
		      if (ad->dupeflags & DP_EXTS) {
			if (px) {
			  *px = '.';
			  *pz = '.';
			}
		      }
		      goto SkipNonDupe;
		    }
		    if (ad->dupeflags & DP_EXTS) {
		      if (px) {
			*px = '.';
			*pz = '.';
		      }
		    }
		  }
		  if (ad->dupeflags & DP_CRCS) {
		    if (!(ad->afhead[x].flags & AF_CRCED)) {
		      ad->afhead[x].CRC = CRCFile(ad->afhead[x].fullname,
						  &error);
		      if (!error)
			ad->afhead[x].flags |= AF_CRCED;
		    }
		    if (!(ad->afhead[z].flags & AF_CRCED)) {
		      ad->afhead[z].CRC = CRCFile(ad->afhead[z].fullname,
						  &error);
		      if (!error)
			ad->afhead[z].flags |= AF_CRCED;
		    }
		    if ((ad->afhead[x].flags & AF_CRCED) &&
			(ad->afhead[z].flags & AF_CRCED)) {
		      if (ad->afhead[x].CRC != ad->afhead[z].CRC)
			goto SkipNonDupe;
		    }
		    DosSleep(0);
		  }
		  ad->afhead[x].flags |= AF_DUPE;
		  ad->afhead[z].flags |= AF_DUPE;
		SkipNonDupe:
		  ;
		}
	      } // for
	      DosSleep(0);
	    }
	  } // for
	  for (x = 0; x < ad->afheadcnt && !ad->stopflag; x++) {
	    if (!(ad->afhead[x].flags & AF_DUPE))
	      ad->afhead[x].flags |= AF_FILTERED;
	  }
	  ReSort(hwnd);
	  WinInvalidateRect(hwnd, NULL, FALSE);
	}
      }
    }
    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    DosReleaseMutexSem(ad->hmtxScan);
  } // if got sem
  if (hmq2) {
    PostMsg(hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
    WinDestroyMsgQueue(hmq2);
  }
  if (hab2) {
    DecrThreadUsage();
    WinTerminate(hab2);
  }
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

static VOID FilterAll(HWND hwnd, ALLDATA *ad);

static VOID FilterList(HWND hwnd)
{
  ULONG x;
  ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  CHAR *p;

  if (ad->cursored <= ad->afindexcnt) {
    x = ad->cursored - 1;
    x = (ad->invertsort) ? (ad->afindexcnt - 1) - x : x;
    p = strrchr(ad->afindex[x]->filename, '.');
    if (p) {
      strcpy(ad->mask.szMask, "*");
      strcat(ad->mask.szMask, p);
    }
  }
  *(ad->mask.prompt) = 0;
  ad->mask.fIsSeeAll = TRUE;
  if (WinDlgBox(HWND_DESKTOP, hwnd, PickMaskDlgProc,
		FM3ModHandle, MSK_FRAME, MPFROMP(&ad->mask)))
    FilterAll(hwnd, ad);
}

static VOID FilterAll(HWND hwnd, ALLDATA *ad)
{
  ULONG x, z;
  BOOL ret;

  if (ad) {
    for (x = 0; x < ad->afheadcnt; x++) {
      ret = FALSE;
      if (ad->mask.pszMasks[1]) {
	for (z = 0; ad->mask.pszMasks[z]; z++) {
	  if (*ad->mask.pszMasks[z]) {
	    if (*ad->mask.pszMasks[z] != '/') {
	      if (wildcard((strchr(ad->mask.pszMasks[z], '\\') ||
			    strchr(ad->mask.pszMasks[z], ':')) ?
			   ad->afhead[x].fullname : ad->afhead[x].filename,
			   ad->mask.pszMasks[z], FALSE))
		ret = TRUE;
	    }
	    else {
	      if (wildcard((strchr(ad->mask.pszMasks[z], '\\') ||
			    strchr(ad->mask.pszMasks[z], ':')) ?
			   ad->afhead[x].fullname : ad->afhead[x].filename,
			   ad->mask.pszMasks[z] + 1, FALSE)) {
		ret = FALSE;
		break;
	      }
	    }
	  }
	}
      }
      else if (*ad->mask.szMask) {
	if (wildcard((strchr(ad->mask.szMask, '\\') ||
		      strchr(ad->mask.szMask, ':')) ?
		     ad->afhead[x].fullname : ad->afhead[x].filename,
		     ad->mask.szMask, FALSE))
	  ret = TRUE;
      }
      else
	ret = TRUE;

      if (ret) {
	if ((!(ad->mask.attrFile & FILE_HIDDEN)
	     && (ad->afhead[x].attrFile & FILE_HIDDEN))
	    || (!(ad->mask.attrFile & FILE_SYSTEM)
		&& (ad->afhead[x].attrFile & FILE_SYSTEM))
	    || (!(ad->mask.attrFile & FILE_READONLY)
		&& (ad->afhead[x].attrFile & FILE_READONLY))
	    || (!(ad->mask.attrFile & FILE_ARCHIVED)
		&& (ad->afhead[x].attrFile & FILE_ARCHIVED)))
	  ret = FALSE;
	else
	  if (((ad->mask.antiattr & FILE_HIDDEN)
	       && !(ad->afhead[x].attrFile & FILE_HIDDEN))
	      || ((ad->mask.antiattr & FILE_SYSTEM)
		  && !(ad->afhead[x].attrFile & FILE_SYSTEM))
	      || ((ad->mask.antiattr & FILE_READONLY)
		  && !(ad->afhead[x].attrFile & FILE_READONLY))
	      || ((ad->mask.antiattr & FILE_ARCHIVED)
		  && !(ad->afhead[x].attrFile & FILE_ARCHIVED)))
	  ret = FALSE;
      }

      if (!ret)
	ad->afhead[x].flags |= AF_FILTERED;
      else
	ad->afhead[x].flags &= (~AF_FILTERED);
    }
    ReSort(hwnd);
    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    WinInvalidateRect(hwnd, NULL, FALSE);
  }
}

static ULONG RemoveDeleted(HWND hwnd)
{
  ALLDATA *pAD = WinQueryWindowPtr(hwnd, QWL_USER);
  ULONG oldafheadcnt = pAD->afheadcnt;
  register ULONG x, y;

  for (x = 0; x < pAD->afheadcnt; x++) {
    if (pAD->afhead[x].flags & AF_DELETED) {
      for (y = x; y < pAD->afheadcnt; y++) {
	if (~pAD->afhead[y].flags & AF_DELETED)
	  break;
	if (pAD->afhead[y].flags & AF_SELECTED &&
	    ~pAD->afhead[y].flags & AF_FILTERED) {
	  pAD->selected--;
	  pAD->ullSelectedBytes -= pAD->afhead[y].cbFile;
	}
	xfree(pAD->afhead[y].fullname, pszSrcFile, __LINE__);
      }
      memmove(&(pAD->afhead[x]), &(pAD->afhead[y]),
	      (pAD->afheadcnt - y) * sizeof(ALLFILES));
      pAD->afheadcnt -= (y - x);
    }
  }                                     // for x
  if (pAD->afheadcnt != oldafheadcnt) {

    ALLFILES *tempa, **templ;

    if (!pAD->afheadcnt)
      FreeAllFilesList(hwnd);
    else {
      tempa =
	xrealloc(pAD->afhead, pAD->afheadcnt * sizeof(ALLFILES), pszSrcFile,
		 __LINE__);
      if (tempa) {
	pAD->afhead = tempa;
	pAD->afalloc = pAD->afheadcnt;
      }
      templ =
	xrealloc(pAD->afindex, pAD->afheadcnt * sizeof(ALLFILES *), pszSrcFile,
		 __LINE__);
      if (templ)
	pAD->afindex = templ;
      DosPostEventSem(CompactSem);
      ReSort(hwnd);
    }
  }
  return pAD->afheadcnt;
}

static VOID DoADir(HWND hwnd, CHAR * pathname)
{
  ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  CHAR *filename, *enddir;
  PFILEFINDBUF3L pffbArray, pffbFile;
  HDIR hdir = HDIR_CREATE;
  ULONG ulFindCnt;
  ULONG ulFindMax;
  ULONG ulBufBytes;
  ULONG x;
  APIRET rc;
  static BOOL fDone;

  filename = xmalloc(CCHMAXPATH + 100, pszSrcFile, __LINE__);
  if (!filename)
    return;

  ulFindMax = FilesToGet;
  if (fRemoteBug && isalpha(*pathname) && pathname[1] == ':' &&
      pathname[2] == '\\' &&
      (driveflags[toupper(*pathname) - 'A'] & DRIVE_REMOTE))
    ulFindMax = 1;

  ulBufBytes = sizeof(FILEFINDBUF3L) * ulFindMax;
  pffbArray = xmalloc(ulBufBytes, pszSrcFile, __LINE__);
  if (!pffbArray) {
    free(filename);
    return;
  }

  strcpy(filename, pathname);
  enddir = &filename[strlen(filename) - 1];
  if (*enddir != '\\') {
    enddir++;
    *enddir = '\\';
  }
  enddir++;
  strcpy(enddir, "*");
  DosError(FERR_DISABLEHARDERR);
  ulFindCnt = ulFindMax;
  rc = xDosFindFirst(filename, &hdir, FILE_NORMAL | FILE_ARCHIVED |
		     FILE_READONLY | FILE_DIRECTORY | FILE_SYSTEM |
		     FILE_HIDDEN,
		     pffbArray, ulBufBytes, &ulFindCnt, FIL_STANDARDL);
  if (!rc) {
    do {
      priority_normal();
      pffbFile = pffbArray;

      for (x = 0; x < ulFindCnt; x++) {
	if (ad->stopflag)
	  break;
	if (pffbFile->attrFile & FILE_DIRECTORY) {
	  // Skip . and ..
	  if (pffbFile->achName[0] != '.' ||
	      (pffbFile->achName[1] &&
	       (pffbFile->achName[1] != '.' || pffbFile->achName[2]))) {
	    strcpy(enddir, pffbFile->achName);
	    DoADir(hwnd, filename);
	  }
	}
	else {
	  *enddir = 0;
	  strcpy(enddir, pffbFile->achName);
	  if (strlen(filename) > CCHMAXPATH) {
	    // Complain if pathnames exceeds max
	    DosFindClose(hdir);
	    free(pffbArray);
	    free(filename);
	    if (!fDone) {
	      fDone = TRUE;
	      saymsg(MB_OK | MB_ICONASTERISK,
		     HWND_DESKTOP,
		     GetPString(IDS_WARNINGTEXT),
		     GetPString(IDS_LENGTHEXCEEDSMAXPATHTEXT));
	    }
	    return;
	  }
	  if (!ad->afalloc || ad->afheadcnt > ad->afalloc - 1) {

	    ALLFILES *temp;

	    temp = xrealloc(ad->afhead, (ad->afalloc + 1000) *
			    sizeof(ALLFILES), pszSrcFile, __LINE__);
	    if (!temp) {
	      ad->stopflag = 1;
	      break;
	    }
	    else {
	      ad->afhead = temp;
	      if (ad->stopflag)
		break;
	      ad->afalloc += 1000;
	    }
	  }
	  ad->afhead[ad->afheadcnt].fullname =
	    xstrdup(filename, pszSrcFile, __LINE__);
	  if (!ad->afhead[ad->afheadcnt].fullname) {
	    ad->stopflag = 1;
	    break;
	  }
	  else {
	    ad->afhead[ad->afheadcnt].filename =
	      ad->afhead[ad->afheadcnt].fullname + (enddir - filename);
	    ad->afhead[ad->afheadcnt].cbFile = pffbFile->cbFile;
	    ad->afhead[ad->afheadcnt].date = pffbFile->fdateLastWrite;
	    ad->afhead[ad->afheadcnt].time = pffbFile->ftimeLastWrite;
	    ad->afhead[ad->afheadcnt].attrFile = (USHORT) pffbFile->attrFile;
	    ad->afhead[ad->afheadcnt].flags = 0;
	    ad->afheadcnt++;
	    if (ad->longest < pffbFile->cchName)
	      ad->longest = pffbFile->cchName;
	    if (ad->longestw < pffbFile->cchName + (enddir - filename))
	      ad->longestw = pffbFile->cchName + (enddir - filename);
	  }
	}
	pffbFile = (PFILEFINDBUF3L)((PBYTE)pffbFile + pffbFile->oNextEntryOffset);
      } // for

      if (ad->stopflag)
	break;

      ulFindCnt = ulFindMax;
      rc = xDosFindNext(hdir,
			pffbArray,
			sizeof(FILEFINDBUF3L) * ulFindCnt,
			&ulFindCnt,
			FIL_STANDARDL);
    } while (!rc);
    DosFindClose(hdir);
  }

  if (rc && rc != ERROR_NO_MORE_FILES) {
    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      GetPString(IDS_CANTFINDDIRTEXT), filename);
  }

  free(pffbArray);
  free(filename);
}

static VOID FindAllThread(VOID * args)
{
  ULONG ulDriveNum, ulDriveMap, x;
  CHAR startname[] = " :\\";
  HWND hwnd = (HWND) args;
  HAB hab2 = (HAB) 0;
  HMQ hmq2 = (HMQ) 0;
  ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  APIRET apiret;

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif

  // DbgMsg(pszSrcFile, __LINE__, "FindAllThread requesting hmtxScan");
  apiret = DosRequestMutexSem(ad->hmtxScan, SEM_INDEFINITE_WAIT);
  if (apiret != NO_ERROR)
    Dos_Error(MB_CANCEL, apiret, hwnd, pszSrcFile, __LINE__, "DosRequestMutexSem");
  else {
    priority_normal();
    hab2 = WinInitialize(0);

    if (hab2) {
      hmq2 = WinCreateMsgQueue(hab2, 0);
      if (hmq2) {
	WinCancelShutdown(hmq2, TRUE);
	IncrThreadUsage();
	if (!*ad->szFindPath) {
	  DosError(FERR_DISABLEHARDERR);
	  if (!DosQCurDisk(&ulDriveNum, &ulDriveMap)) {
	    for (x = 2; x < 26 && !ad->stopflag; x++) {
	      if ((ulDriveMap & (1 << x)) && ad->abDrvFlags[x]) {
		*startname = (CHAR) (x + 'A');
		DoADir(hwnd, startname);
		if (ad->stopflag)
		  break;
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
		DosSleep(0); //26 Aug 07 GKY 1
	      }
	    } // for
	  }
	}
	else
	  DoADir(hwnd, ad->szFindPath);

	DosPostEventSem(CompactSem);
      }
    }

    if (!ad->stopflag) {
      if (ad->afalloc != ad->afheadcnt) {

	ALLFILES *tempa, **templ;

	tempa =
	  xrealloc(ad->afhead, sizeof(ALLFILES) * ad->afheadcnt, pszSrcFile,
		   __LINE__);
	if (tempa) {
	  ad->afhead = tempa;
	  ad->afalloc = ad->afheadcnt;
	}
	templ =
	  xrealloc(ad->afindex, sizeof(ALLFILES *) * ad->afheadcnt, pszSrcFile,
		   __LINE__);
	if (templ)
	  ad->afindex = templ;
	DosPostEventSem(CompactSem);
      }
      PostMsg(hwnd, UM_RESCAN, MPFROMLONG(1), MPVOID);
      ReSort(hwnd);
    }

    // DbgMsg(pszSrcFile, __LINE__, "FindAllThread releasing hmtxScan");
    DosReleaseMutexSem(ad->hmtxScan);
  }

  if (hmq2) {
    PostMsg(hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
    WinDestroyMsgQueue(hmq2);
  }

  if (hab2) {
    DecrThreadUsage();
    WinTerminate(hab2);
  }

# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

MRESULT EXPENTRY AFDrvsWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    if (mp2) {

      ULONG ulDriveNum, ulDriveMap, x;
      CHAR startname[] = " :";
      SHORT sSelect;
      ALLDATA *ad;

      ad = (ALLDATA *) mp2;
      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      DosError(FERR_DISABLEHARDERR);
      if (!DosQCurDisk(&ulDriveNum, &ulDriveMap)) {
	for (x = 2; x < 26 && !ad->stopflag; x++) {
	  if (!(driveflags[x] & (DRIVE_IGNORE | DRIVE_INVALID))) {
	    if (ulDriveMap & (1 << x)) {
	      *startname = (CHAR) (x + 'A');
	      sSelect = (SHORT) WinSendDlgItemMsg(hwnd, DRVS_LISTBOX,
						  LM_INSERTITEM,
						  MPFROM2SHORT(LIT_END, 0),
						  MPFROMP(startname));
	      if (sSelect >= 0 && ad->abDrvFlags[x])
		WinSendDlgItemMsg(hwnd, DRVS_LISTBOX, LM_SELECTITEM,
				  MPFROM2SHORT(sSelect, 0), MPFROMLONG(TRUE));
	    }
	  }
	}
      }
    }
    else
      WinDismissDlg(hwnd, 0);
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case DRVS_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      {
	INT x;
	SHORT sSelect;
	CHAR filename[3];
	ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);

	memset(ad->abDrvFlags, 0, sizeof(ad->abDrvFlags));
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, DRVS_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROM2SHORT(LIT_FIRST, 0),
					    MPVOID);
	while (sSelect >= 0) {
	  *filename = 0;
	  if (WinSendDlgItemMsg(hwnd, DRVS_LISTBOX, LM_QUERYITEMTEXT,
				MPFROM2SHORT(sSelect, 2),
				MPFROMP(filename)) && *filename)
	    ad->abDrvFlags[*filename - 'A'] = 1;
	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd, DRVS_LISTBOX,
					      LM_QUERYSELECTION,
					      MPFROM2SHORT(sSelect, 0),
					      MPVOID);
	}
	for (x = 2; x < 26; x++) {
	  if (ad->abDrvFlags[x]) {
	    WinDismissDlg(hwnd, 1);
	    return 0;
	  }
	}
      }
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_DRVSWND, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static HPS InitWindow(HWND hwnd)
{
  ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  HPS hps = (HPS) 0;
  SIZEL sizel;
  FONTMETRICS FontMetrics;

  if (ad) {
    sizel.cx = sizel.cy = 0;
    hps = GpiCreatePS(WinQueryAnchorBlock(hwnd), WinOpenWindowDC(hwnd),
		      (PSIZEL) & sizel, PU_PELS | GPIF_DEFAULT | GPIT_MICRO |
		      GPIA_ASSOC);
    if (hps) {
      GpiSetCp(hps, (ULONG) ad->fattrs.usCodePage);
      GpiCreateLogFont(hps, NULL, SEEALLFILECNR_FONT_LCID, &ad->fattrs);
      GpiSetCharSet(hps, SEEALLFILECNR_FONT_LCID);
      GpiQueryFontMetrics(hps, sizeof(FontMetrics), &FontMetrics);
      ad->fattrs.lAveCharWidth = FontMetrics.lAveCharWidth;
      ad->fattrs.lMaxBaselineExt = FontMetrics.lMaxBaselineExt;
      ad->lMaxAscender = max(FontMetrics.lMaxAscender, 0);
      ad->lMaxDescender = max(FontMetrics.lMaxDescender, 0);
      ad->lMaxHeight = ad->lMaxDescender + ad->lMaxAscender;
      if (ad->fattrs.usCodePage != FontMetrics.usCodePage) {
	ad->fattrs.usCodePage = FontMetrics.usCodePage;
	Codepage = ad->fattrs.usCodePage;
	PrfWriteProfileData(fmprof,
			    appname,
			    "Seeall.Codepage",
			    &ad->fattrs.usCodePage, sizeof(USHORT));
      }
      else if (ad->fattrs.usCodePage) {

	HMQ hmq;
	ULONG cps[50], len, x;

	if (!DosQueryCp(sizeof(cps), cps, &len)) {
	  for (x = 0; x < len / sizeof(ULONG); x++) {
	    if (cps[x] == (ULONG) ad->fattrs.usCodePage) {
	      hmq = WinQueryWindowULong(hwnd, QWL_HMQ);
	      WinSetCp(hmq, ad->fattrs.usCodePage);
	      break;
	    }
	  }
	}
	DosSetProcessCp((ULONG) ad->fattrs.usCodePage);
      }
      GpiSetBackMix(hps, BM_OVERPAINT);
    }
  }
  return (hps);
}

static VOID PaintLine(HWND hwnd, HPS hps, ULONG whichfile, ULONG topfile,
		      RECTL * Rectl)
{
  ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  POINTL ptl;
  CHAR szBuff[CCHMAXPATH + 80], szCmmaFmtFileSize[81], szDate[DATE_BUF_BYTES];
  ULONG len, y;

  y = (ad->invertsort) ? (ad->afindexcnt - 1) - whichfile : whichfile;
  ptl.y = (Rectl->yTop -
	   (ad->lMaxHeight * (((whichfile + 1) - topfile) + 1)));
  ptl.x = ad->horzscroll;
  if (ptl.y < Rectl->yBottom || ptl.y > Rectl->yTop || y > ad->afindexcnt)
    return;
  GpiSetBackMix(hps, BM_OVERPAINT);
  if (ad->afindex[y]->flags & AF_SELECTED) {
    GpiSetColor(hps, standardcolors[Colors[COLORS_SELECTEDNORMALFORE]]);
    GpiSetBackColor(hps, (whichfile == ad->cursored - 1) ?
		    standardcolors[Colors[COLORS_CURSOREDSELECTEDBACK]] :
		    standardcolors[Colors[COLORS_SELECTEDBACK]]);
  }
  else {
    GpiSetColor(hps,
		((ad->afindex[y]->attrFile & (FILE_SYSTEM | FILE_HIDDEN)) !=
		 0) ? standardcolors[Colors[COLORS_SYSTEMFORE]] : ((ad->
								    afindex
								    [y]->
								    attrFile &
								    FILE_READONLY)
								   !=
								   0) ?
		standardcolors[Colors[COLORS_READONLYFORE]] :
		standardcolors[Colors[COLORS_NORMALFORE]]);
    GpiSetBackColor(hps,
		    (whichfile ==
		     ad->cursored -
		     1) ? standardcolors[Colors[COLORS_CURSOREDNORMALBACK]] :
		    standardcolors[Colors[COLORS_NORMALBACK]]);
  }
  CommaFmtULL(szCmmaFmtFileSize,
	      sizeof(szCmmaFmtFileSize), ad->afindex[y]->cbFile, ' ');
  FDateFormat(szDate, ad->afindex[y]->date);
  len = sprintf(szBuff,
		"%c%-*.*s  %-12s  %c%c%c%c%c  %s %02u%s%02u%s%02u ",
		whichfile == ad->cursored - 1 ? '>' : ' ',
		ad->fullnames ? ad->longestw : ad->longest,
		ad->fullnames ? ad->longestw : ad->longest,
		ad->fullnames ? ad->afindex[y]->fullname :
		ad->afindex[y]->filename,
		szCmmaFmtFileSize,
		"-A"[((ad->afindex[y]->attrFile & FILE_ARCHIVED) != 0)],
		"-R"[((ad->afindex[y]->attrFile & FILE_READONLY) != 0)],
		"-H"[((ad->afindex[y]->attrFile & FILE_HIDDEN) != 0)],
		"-S"[((ad->afindex[y]->attrFile & FILE_SYSTEM) != 0)],
		"-D"[((ad->afindex[y]->attrFile & FILE_DIRECTORY) != 0)],
		szDate,
		ad->afindex[y]->time.hours, TimeSeparator,
		ad->afindex[y]->time.minutes, TimeSeparator,
		ad->afindex[y]->time.twosecs * 2);
  GpiCharStringAt(hps, &ptl, len, szBuff);
  GpiQueryCurrentPosition(hps, &ptl);
  if (ptl.x + abs(ad->horzscroll) > ad->maxx) {
    ad->maxx = ptl.x + abs(ad->horzscroll);
    WinSendMsg(ad->hhscroll, SBM_SETTHUMBSIZE,
	       MPFROM2SHORT((SHORT) Rectl->xRight, (SHORT) ad->maxx), MPVOID);
  }
}

MRESULT EXPENTRY SeeStatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_CREATE:
    return CommonTextProc(hwnd, msg, mp1, mp2);

  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case WM_PAINT:
    {
      SWP swp;
      POINTL ptl;
      HPS hps;

      PaintRecessedWindow(hwnd, (HPS) 0, FALSE, FALSE);
      hps = WinGetPS(WinQueryWindow(hwnd, QW_PARENT));
      if (hps) {
	WinQueryWindowPos(hwnd, &swp);
	ptl.x = swp.x - 1;
	ptl.y = swp.y + swp.cy + 2;
	GpiMove(hps, &ptl);
	GpiSetColor(hps, CLR_WHITE);
	ptl.x = swp.x + swp.cx;
	GpiLine(hps, &ptl);
	WinReleasePS(hps);
      }
    }
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					      FID_CLIENT));
    return 0;
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY SeeFrameWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg) {
  case WM_BUTTON1UP:
  case WM_BUTTON2UP:
  case WM_BUTTON3UP:
  case WM_MOUSEMOVE:
  case WM_CHORD:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_CALCFRAMERECT:
    {
      MRESULT mr;
      PRECTL prectl;

      mr = oldproc(hwnd, msg, mp1, mp2);

      /*
       * Calculate the position of the client rectangle.
       * Otherwise,  we'll see a lot of redraw when we move the
       * client during WM_FORMATFRAME.
       */

      if (mr && mp2) {
	prectl = (PRECTL) mp1;
	prectl->yBottom += 22;
	prectl->yTop -= 24;
      }
      return mr;
    }

  case WM_FORMATFRAME:
    {
      SHORT sCount;
      PSWP pswp, pswpClient, pswpNew;

      sCount = (SHORT) oldproc(hwnd, msg, mp1, mp2);

      /*
       * Reformat the frame to "squeeze" the client
       * and make room for status window sibling beneath
       */

      pswp = (PSWP) mp1;
      {
	SHORT x;

	for (x = 0; x < sCount; x++) {
	  if (WinQueryWindowUShort(pswp->hwnd, QWS_ID) == FID_CLIENT) {
	    pswpClient = pswp;
	    break;
	  }
	  pswp++;
	}
      }
      pswpNew = (PSWP) mp1 + sCount;
      *pswpNew = *pswpClient;
      pswpNew->hwnd = WinWindowFromID(hwnd, SEEALL_STATUS);
      pswpNew->x = pswpClient->x + 2;
      pswpNew->y = pswpClient->y + 2;
      pswpNew->cx = pswpClient->cx - 3;
      pswpNew->cy = 20;
      pswpClient->y = pswpNew->y + pswpNew->cy + 3;
      pswpClient->cy = (pswpClient->cy - pswpNew->cy) - 5;
      sCount++;
      return MRFROMSHORT(sCount);
    }

  case WM_QUERYFRAMECTLCOUNT:
    {
      SHORT sCount;

      sCount = (SHORT) oldproc(hwnd, msg, mp1, mp2);
      sCount++;
      return MRFROMSHORT(sCount);
    }
  }
  return oldproc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY SeeAllWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ALLDATA *pAD = WinQueryWindowPtr(hwnd, QWL_USER);
  APIRET apiret;

  switch (msg) {
  case WM_CREATE:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc WM_CREATE");
    WinSetWindowPtr(hwnd, QWL_USER, NULL);
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
    pAD = xmallocz(sizeof(ALLDATA), pszSrcFile, __LINE__);
    if (pAD) {
      HWND hwndFrame;

      pAD->size = sizeof(ALLDATA);
      hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
      pAD->hwndFrame = hwndFrame;
      pAD->mask.attrFile = FILE_READONLY | FILE_HIDDEN |
	FILE_SYSTEM | FILE_ARCHIVED;
      pAD->mask.fNoDirs = TRUE;
      *(pAD->mask.prompt) = 0;
      WinSetWindowPtr(hwnd, QWL_USER, (PVOID) pAD);
      pAD->pfnCompare = comparenames;
      if (Firsttime) {

	ULONG size;

	size = sizeof(USHORT);
	PrfQueryProfileData(fmprof,
			    appname,
			    "Seeall.Codepage", (PVOID) & Codepage, &size);
	size = sizeof(BOOL);
	PrfQueryProfileData(fmprof,
			    appname,
			    "Seeall.Fullnames", (PVOID) & Fullnames, &size);
	size = sizeof(USHORT);
	PrfQueryProfileData(fmprof,
			    appname,
			    "Seeall.Sort", (PVOID) & SortType, &size);
	size = sizeof(BOOL);
	PrfQueryProfileData(fmprof,
			    appname,
			    "Seeall.SortReverse",
			    (PVOID) & SortReverse, &size);
	memset(&Fattrs, 0, sizeof(Fattrs));
	size = sizeof(Fattrs);
	Fattrs.usRecordLength = sizeof(Fattrs);
	Fattrs.lMaxBaselineExt = 16;
	Fattrs.lAveCharWidth = 8;
	Fattrs.usCodePage = Codepage;
	strcpy(Fattrs.szFacename, GetPString(IDS_SYSMONOTEXT));
	PrfQueryProfileData(fmprof,
			    appname,
			    "Seeall.Fattrs", (PVOID) & Fattrs, &size);
	size = sizeof(LONG) * COLORS_MAX;
	PrfQueryProfileData(fmprof,
			    appname, "Seeall.Colors", (PVOID) Colors, &size);
	Firsttime = FALSE;
      }
      switch (SortType) {
      case IDM_SORTEASIZE:
	pAD->pfnCompare = (PFNSORT) NULL;
	break;
      case IDM_SORTNAME:
	pAD->pfnCompare = comparefullnames;
	break;
      case IDM_SORTFILENAME:
	pAD->pfnCompare = comparenames;
	break;
      case IDM_SORTSIZE:
	pAD->pfnCompare = comparesizes;
	break;
      case IDM_SORTLWDATE:
	pAD->pfnCompare = comparedates;
	break;
      case IDM_SORTFIRST:
	pAD->pfnCompare = compareexts;
	break;
      }
      pAD->invertsort = SortReverse;
      pAD->fattrs = Fattrs;
      pAD->fullnames = Fullnames;
      pAD->stopflag = 0;
      pAD->cursored = pAD->topfile = 1;
      pAD->fattrs.usCodePage = Codepage;
      memcpy(pAD->aulColors, Colors, sizeof(LONG) * COLORS_MAX);
      pAD->hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
      SetConditionalCascade(pAD->hwndMenu, IDM_DELETESUBMENU,
			    (fDefaultDeletePerm) ?
			    IDM_PERMDELETE : IDM_DELETE);
      SetConditionalCascade(pAD->hwndMenu, IDM_MOVEMENU, IDM_MOVE);
      SetConditionalCascade(pAD->hwndMenu, IDM_COPYMENU, IDM_COPY);
      SetConditionalCascade(pAD->hwndMenu, IDM_OPENSUBMENU, IDM_OPENDEFAULT);
      SetConditionalCascade(pAD->hwndMenu, IDM_OBJECTSUBMENU, IDM_SHADOW);
      if (fWorkPlace) {
	WinSendMsg(pAD->hwndMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OPENSUBMENU, TRUE), MPVOID);
	WinSendMsg(pAD->hwndMenu, MM_DELETEITEM,
		   MPFROM2SHORT(IDM_OBJECTSUBMENU, TRUE), MPVOID);
      }
      pAD->hwndClient = hwnd;
      pAD->hps = InitWindow(hwnd);
      pAD->hvscroll = WinWindowFromID(hwndFrame, FID_VERTSCROLL);
      pAD->hhscroll = WinWindowFromID(hwndFrame, FID_HORZSCROLL);
      pAD->multiplier = 1;
      if (xbeginthread(MakeSeeObjWinThread,
		       122880,
		       pAD,
		       pszSrcFile,
		       __LINE__) != -1)
      {
	if (!DosCreateMutexSem(NULL, &pAD->hmtxScan, 0, FALSE)) {
	  pAD->hwndStatus = WinCreateWindow(hwndFrame,
					    (CHAR *) WC_SEESTATUS,
					    NullStr,
					    WS_VISIBLE | SS_TEXT |
					    DT_LEFT | DT_VCENTER,
					    0,
					    0,
					    0,
					    0,
					    hwndFrame,
					    HWND_TOP,
					    SEEALL_STATUS, NULL, NULL);
	  if (!pAD->hwndStatus)
	    Win_Error(hwndFrame, hwnd, pszSrcFile, __LINE__,
		      PCSZ_WINCREATEWINDOW);
	  else {
	    PFNWP oldproc;

	    oldproc = WinSubclassWindow(hwndFrame, SeeFrameWndProc);
	    WinSetWindowPtr(hwndFrame, QWL_USER, (PVOID) oldproc);
	  }
	  break;
	}
      }
    }
    PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    break;

  case UM_SETUP5:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc UM_SETUP5");
    if (pAD) {
      if (mp1 && *((CHAR *)mp1))
	strcpy(pAD->szFindPath, (CHAR *)mp1);
      else {
	if (!WinDlgBox(HWND_DESKTOP, hwnd, AFDrvsWndProc,
		       FM3ModHandle, DRVS_FRAME, (PVOID) pAD)) {
	  PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	  return 0;
	}
      }
      if (xbeginthread(FindAllThread,
		       524288,
		       (PVOID)hwnd,
		       pszSrcFile,
		       __LINE__) == -1)
      {
	PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      }
      else {
	DosSleep(50);//05 Aug 07 GKY 100
	PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	PostMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);
      }
    }
    else
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
    return 0;

  case UM_UPDATERECORDLIST:
    if (mp1) {

      APIRET rc;

      rc = DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN);
      if (!rc) {
	WinSetPointer(HWND_DESKTOP, hptrBusy);
	if (UpdateList(hwnd, mp1)) {
	  FreeList(mp1);
	  RemoveDeleted(hwnd);
	  ReSort(hwnd);
	  WinInvalidateRect(hwnd, NULL, FALSE);
	}
	DosReleaseMutexSem(pAD->hmtxScan);
	WinSetPointer(HWND_DESKTOP, hptrArrow);
      }
    }
    return 0;

  case UM_SETUP2:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc UM_SETUP2");
    if (pAD) {

      CHAR s[256];
      BOOL once = FALSE;
      ULONG x, ulDriveNum, ulDriveMap;

      strcpy(s, GetPString(IDS_SEEALLTITLETEXT));
      if (!*pAD->szFindPath) {
	DosError(FERR_DISABLEHARDERR);
	if (!DosQCurDisk(&ulDriveNum, &ulDriveMap)) {
	  for (x = 2; x < 26 && !pAD->stopflag; x++) {
	    if ((ulDriveMap & (1 << x)) && pAD->abDrvFlags[x]) {
	      sprintf(&s[strlen(s)], "%s%c:", (once) ? ", " : " (", x + 'A');
	      once = TRUE;
	    }
	  }
	  if (once)
	    strcat(s, ")");
	}
      }
      else {
	strcat(s, " (");
	strcat(s, pAD->szFindPath);
	strcat(s, ")");
      }
      WinSetWindowText(WinQueryWindow(hwnd, QW_PARENT), s);
    }
    return 0;

  case UM_SETUP3:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc UM_SETUP3");
    if (pAD) {
      pAD->multiplier = pAD->afindexcnt / 32767;
      if (pAD->multiplier * 32767 != pAD->afindexcnt)
	pAD->multiplier++;
      if (!pAD->multiplier)
	pAD->multiplier++;
      {
	RECTL Rectl;
	ULONG numlines;

	WinQueryWindowRect(hwnd, &Rectl);
	numlines = NumLines(&Rectl, pAD);
	if (numlines) {
	  WinSendMsg(pAD->hhscroll, SBM_SETTHUMBSIZE,
		     MPFROM2SHORT((SHORT) Rectl.xRight, (SHORT) pAD->maxx),
		     MPVOID);
	  WinSendMsg(pAD->hvscroll, SBM_SETTHUMBSIZE,
		     MPFROM2SHORT((SHORT) numlines,
				  (SHORT) min(pAD->afindexcnt, 32767)),
		     MPFROM2SHORT(1, pAD->afindexcnt + 1));
	  WinSendMsg(pAD->hhscroll, SBM_SETSCROLLBAR,
		     MPFROMSHORT((SHORT) abs(pAD->horzscroll)),
		     MPFROM2SHORT(0, (SHORT) (pAD->maxx - Rectl.xRight)));
	  WinSendMsg(pAD->hvscroll, SBM_SETSCROLLBAR,
		     MPFROMSHORT((SHORT) (pAD->topfile / pAD->multiplier)),
		     MPFROM2SHORT(1,
				  (SHORT) (pAD->afindexcnt / pAD->multiplier) -
				  (numlines - 1)));
	  if (pAD->afindexcnt - pAD->topfile < numlines) {
	    pAD->topfile = ((pAD->afindexcnt - pAD->topfile) - numlines);
	    WinInvalidateRect(hwnd, NULL, FALSE);
	  }
	}
      }
    }
    return 0;

  case UM_SETUP4:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc UM_SETUP4");
    if (pAD)
      pAD->killme = TRUE;
    else
      PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    return 0;

  case UM_RESCAN:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc UM_RESCAN");
    if (pAD && !pAD->stopflag) {
      if (DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {
	// Assume still working - show progress
	CHAR s[CCHMAXPATH + 80], tm[34];

	if (mp1) {
	  strcpy(s, GetPString(IDS_SORTINGTEXT));
	  if (pAD->afindexcnt) {
	    commafmt(tm, sizeof(tm), pAD->afindexcnt);
	    strcat(s, tm);
	  }
	}
	else {
	  strcpy(s, GetPString(IDS_WORKINGTEXT));
	  if (pAD->afheadcnt) {
	    commafmt(tm, sizeof(tm), pAD->afheadcnt);
	    strcat(s, tm);
	  }
	}
	if (mp2) {
	  strcat(s, " ");
	  strcat(s, (CHAR *)mp2);
	}
	WinSetWindowText(pAD->hwndStatus, s);
      }
      else {
	// Assume scan done
	CHAR s[(CCHMAXPATH * 2) + 80], tm[34], ts[34], tb[34];
	ULONG y;

	if (mp1) {
	  strcpy(s, GetPString(IDS_SORTINGTEXT));
	  if (pAD->afindexcnt) {
	    commafmt(tm, sizeof(tm), pAD->afindexcnt);
	    strcat(s, tm);
	  }
	  if (mp2) {
	    strcat(s, " ");
	    strcat(s, (CHAR *)mp2);
	  }
	}
	else if (pAD->afindexcnt) {
	  y = (pAD->invertsort) ? (pAD->afindexcnt - 1) - (pAD->cursored - 1) :
	    pAD->cursored - 1;
	  commafmt(tm, sizeof(tm), pAD->afindexcnt);
	  commafmt(ts, sizeof(ts), pAD->selected);
	  CommaFmtULL(tb, sizeof(tb), pAD->ullSelectedBytes, ' ');
	  sprintf(s,
		  " %s %s%s%s  %s %s (%s)  %s %s",
		  tm,
		  GetPString(IDS_FILETEXT),
		  &"s"[pAD->afindexcnt == 1],
		  (*pAD->mask.szMask ||
		   (pAD->mask.attrFile & (~FILE_DIRECTORY)) !=
		   (ALLATTRS & (~FILE_DIRECTORY)) ||
		   pAD->mask.antiattr) ?
		  GetPString(IDS_FILTEREDTEXT) :
		  NullStr,
		  ts,
		  GetPString(IDS_SELECTEDTEXT),
		  tb, GetPString(IDS_CURRTEXT), pAD->afindex[y]->fullname);
	}
	else
	  sprintf(s,
		  GetPString(IDS_NOFILESPSTEXT),
		  (*pAD->mask.szMask ||
		   (pAD->mask.attrFile & (~FILE_DIRECTORY)) !=
		   (ALLATTRS & (~FILE_DIRECTORY)) ||
		   pAD->mask.antiattr) ?
		  GetPString(IDS_FILTEREDTEXT) : NullStr);
	WinSetWindowText(pAD->hwndStatus, s);
	DosReleaseMutexSem(pAD->hmtxScan);
      }
    }
    return 0;

  case UM_SETUP:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc UM_SETUP");
    if (pAD) {
      WinSendMsg(pAD->hvscroll, SBM_SETTHUMBSIZE, MPFROM2SHORT(1, 1), MPVOID);
      WinSendMsg(pAD->hhscroll, SBM_SETTHUMBSIZE, MPFROM2SHORT(1, 1), MPVOID);
      WinSetActiveWindow(HWND_DESKTOP, WinQueryWindow(hwnd, QW_PARENT));
    }
    return 0;

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (pAD && !(SHORT1FROMMP(mp1) & KC_KEYUP)) {

      register ULONG x;
      ULONG numlines, y, wascursored = pAD->cursored, thistime, len;
      BOOL found = FALSE;
      RECTL rcl;

      WinQueryWindowRect(hwnd, &rcl);
      numlines = NumLines(&rcl, pAD);
      if (numlines) {
	if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY) {
	  pAD->lasttime = 0;
	  *pAD->szCommonName = 0;
	  switch (SHORT2FROMMP(mp2)) {
	  case VK_DELETE:
	    if ((shiftstate & KC_CTRL) == KC_CTRL)
	      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_PERMDELETE, 0),
		      MPVOID);
	    else if ((shiftstate & KC_SHIFT) == KC_SHIFT)
	      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_SAVETOCLIP, 0),
		      MPVOID);
	    else
	      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_DELETE, 0), MPVOID);
	    break;
	  case VK_LEFT:
	    WinSendMsg(hwnd, WM_HSCROLL, MPFROM2SHORT(FID_HORZSCROLL, 0),
		       MPFROM2SHORT(0, SB_LINELEFT));
	    break;
	  case VK_RIGHT:
	    WinSendMsg(hwnd, WM_HSCROLL, MPFROM2SHORT(FID_HORZSCROLL, 0),
		       MPFROM2SHORT(0, SB_LINERIGHT));
	    break;
	  case VK_PAGEUP:
	    WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
		       MPFROM2SHORT(0, SB_PAGEUP));
	    break;
	  case VK_PAGEDOWN:
	    WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
		       MPFROM2SHORT(0, SB_PAGEDOWN));
	    break;
	  case VK_UP:
	    if (pAD->cursored > 1) {
	      if (shiftstate & KC_SHIFT)
		WinSendMsg(hwnd, WM_BUTTON1CLICK,
			   MPFROM2SHORT(pAD->fattrs.lAveCharWidth + 2,
					((rcl.yTop - (pAD->lMaxHeight *
						      ((pAD->cursored) -
						       pAD->topfile))) -
					 pAD->lMaxDescender) - 1),
			   MPFROM2SHORT(TRUE, 0));
	      pAD->cursored--;
	      if (pAD->cursored < pAD->topfile) {
		PaintLine(hwnd, pAD->hps, pAD->cursored, pAD->topfile, &rcl);
		WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
			   MPFROM2SHORT(0, SB_LINEUP));
	      }
	      else {
		PaintLine(hwnd, pAD->hps, pAD->cursored - 1, pAD->topfile,
			  &rcl);
		PaintLine(hwnd, pAD->hps, pAD->cursored, pAD->topfile, &rcl);
	      }
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	    break;
	  case VK_DOWN:
	    if (pAD->cursored < pAD->afindexcnt
		&& pAD->cursored < pAD->topfile + numlines) {
	      if (shiftstate & KC_SHIFT)
		WinSendMsg(hwnd, WM_BUTTON1CLICK,
			   MPFROM2SHORT(pAD->fattrs.lAveCharWidth + 2,
					((rcl.yTop - (pAD->lMaxHeight *
						      ((pAD->cursored) -
						       pAD->topfile))) -
					 pAD->lMaxDescender) - 1),
			   MPFROM2SHORT(TRUE, 0));
	      pAD->cursored++;
	      if (pAD->cursored >= pAD->topfile + numlines) {
		PaintLine(hwnd, pAD->hps, pAD->cursored - 2, pAD->topfile,
			  &rcl);
		WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
			   MPFROM2SHORT(0, SB_LINEDOWN));
	      }
	      else {
		PaintLine(hwnd, pAD->hps, pAD->cursored - 1, pAD->topfile,
			  &rcl);
		PaintLine(hwnd, pAD->hps, pAD->cursored - 2, pAD->topfile,
			  &rcl);
	      }
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	    break;
	  case VK_END:
	    if ((shiftstate & KC_CTRL) ||
		pAD->cursored == (pAD->topfile - 1) + numlines) {
	      pAD->cursored = pAD->afindexcnt;
	      pAD->topfile = (pAD->afindexcnt + 1) - numlines;
	      if (pAD->topfile > pAD->afindexcnt)
		pAD->topfile = 1;
	      WinInvalidateRect(hwnd, NULL, FALSE);
	    }
	    else {
	      pAD->cursored = (pAD->topfile - 1) + numlines;
	      if (pAD->cursored > pAD->afindexcnt)
		pAD->cursored = pAD->afindexcnt;
	      PaintLine(hwnd, pAD->hps, pAD->cursored - 1, pAD->topfile,
			&rcl);
	      PaintLine(hwnd, pAD->hps, wascursored - 1, pAD->topfile, &rcl);
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	    break;
	  case VK_HOME:
	    if ((shiftstate & KC_CTRL) || pAD->cursored == pAD->topfile) {
	      pAD->topfile = 1;
	      pAD->cursored = 1;
	      WinInvalidateRect(hwnd, NULL, FALSE);
	    }
	    else {
	      pAD->cursored = pAD->topfile;
	      PaintLine(hwnd, pAD->hps, pAD->cursored - 1, pAD->topfile,
			&rcl);
	      PaintLine(hwnd, pAD->hps, wascursored - 1, pAD->topfile, &rcl);
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	    break;
	  case VK_SPACE:
	    WinSendMsg(hwnd, WM_BUTTON1CLICK,
		       MPFROM2SHORT(pAD->fattrs.lAveCharWidth + 2,
				    ((rcl.yTop - (pAD->lMaxHeight *
						  ((pAD->cursored) -
						   pAD->topfile))) -
				     pAD->lMaxDescender) - 1),
		       MPFROM2SHORT(TRUE, 0));
	    break;
	  case VK_NEWLINE:
	  case VK_ENTER:
	    WinSendMsg(hwnd, WM_BUTTON1DBLCLK,
		       MPFROM2SHORT(pAD->fattrs.lAveCharWidth + 2,
				    ((rcl.yTop - (pAD->lMaxHeight *
						  ((pAD->cursored) -
						   pAD->topfile))) -
				     pAD->lMaxDescender) - 1), MPFROM2SHORT(0,
									    0));
	    break;
	  }
	}
	else if (SHORT1FROMMP(mp1) & KC_CHAR) {
	  switch (SHORT1FROMMP(mp2)) {
	  case '\x1b':
	  case '\r':
	  case '\n':
	    WinSendMsg(hwnd, WM_BUTTON1DBLCLK,
		       MPFROM2SHORT(pAD->fattrs.lAveCharWidth + 2,
				    (rcl.yTop - (pAD->lMaxHeight *
						 ((pAD->cursored) -
						  pAD->topfile))) - 1),
		       MPFROM2SHORT(0, 0));
	    pAD->lasttime = 0;
	    *pAD->szCommonName = 0;
	    break;
	  default:
	    thistime = WinQueryMsgTime(WinQueryAnchorBlock(hwnd));
	    if (thistime > pAD->lasttime + 1000)
	      *pAD->szCommonName = 0;
	    pAD->lasttime = thistime;
	  KbdRetry:
	    len = strlen(pAD->szCommonName);
	    if (len >= CCHMAXPATH - 1) {
	      *pAD->szCommonName = 0;
	      len = 0;
	    }
	    pAD->szCommonName[len] = toupper(SHORT1FROMMP(mp2));
	    pAD->szCommonName[len + 1] = 0;
	    for (x = pAD->cursored - (len > 0); x < pAD->afindexcnt; x++) {
	      y = (pAD->invertsort) ? (pAD->afindexcnt - 1) - x : x;
	      if (pAD->fullnames) {
		if (!strnicmp(pAD->afindex[y]->fullname, pAD->szCommonName,
			      len + 1)) {
		  found = TRUE;
		  break;
		}
	      }
	      else {
		if (!strnicmp(pAD->afindex[y]->filename, pAD->szCommonName,
			      len + 1)) {
		  found = TRUE;
		  break;
		}
	      }
	    }
	    if (!found) {
	      for (x = 0; x < pAD->cursored - (len > 0); x++) {
		y = (pAD->invertsort) ? (pAD->afindexcnt - 1) - x : x;
		if (pAD->fullnames) {
		  if (!strnicmp(pAD->afindex[y]->fullname, pAD->szCommonName,
				len + 1)) {
		    found = TRUE;
		    break;
		  }
		}
		else {
		  if (!strnicmp(pAD->afindex[y]->filename, pAD->szCommonName,
				len + 1)) {
		    found = TRUE;
		    break;
		  }
		}
	      }
	    }
	    if (found) {
	      if (x + 1 != pAD->cursored) {
		pAD->cursored = x + 1;
		if (pAD->cursored >= pAD->topfile &&
		    pAD->cursored < pAD->topfile + numlines &&
		    wascursored != pAD->cursored) {
		  PaintLine(hwnd, pAD->hps, pAD->cursored - 1, pAD->topfile,
			    &rcl);
		  PaintLine(hwnd, pAD->hps, wascursored - 1, pAD->topfile,
			    &rcl);
		}
		else {
		  if (pAD->cursored < numlines)
		    pAD->topfile = 1;
		  else if (pAD->cursored >
			   (pAD->afindexcnt + 1) - (numlines / 2))
		    pAD->topfile = (pAD->afindexcnt + 1) - numlines;
		  else
		    pAD->topfile = pAD->cursored - (numlines / 2);
		  WinInvalidateRect(hwnd, NULL, FALSE);
		}
	      }
	    }
	    else {
	      *pAD->szCommonName = 0;
	      pAD->lasttime = 0;
	      if (len)                  // retry as first letter if no match
		goto KbdRetry;
	    }
	    break;
	  }
	}
      }
    }
    break;

  case DM_PRINTOBJECT:
    return MRFROMLONG(DRR_TARGET);

  case DM_DISCARDOBJECT:
    return MRFROMLONG(DRR_TARGET);

  case WM_BEGINDRAG:
    {
      CHAR **list;

      list = BuildAList(hwnd);
      if (!list)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	WinSetWindowText(pAD->hwndStatus, (CHAR *) GetPString(IDS_DRAGGINGFILESTEXT));
	DragList(hwnd, (HWND) 0, list, TRUE);
	FreeList(list);
	PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      }
    }
    break;

  case WM_BUTTON1MOTIONSTART:
    if (pAD && !pAD->stopflag &&
	!DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {
      pAD->mousecaptured = TRUE;
      pAD->lastselected = (ULONG) - 1;
      pAD->lastdirection = 0;
      WinSetCapture(HWND_DESKTOP, hwnd);
      DosReleaseMutexSem(pAD->hmtxScan);
      WinSendMsg(hwnd, WM_BUTTON1CLICK, mp1, MPFROM2SHORT(TRUE, 0));
    }
    break;

  case WM_MOUSEMOVE:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (pAD && pAD->mousecaptured) {

      ULONG numlines, whichfile, y, x;
      LONG inc;
      RECTL Rectl;
      POINTS pts;
      BOOL outofwindow = FALSE;

      WinQueryWindowRect(hwnd, &Rectl);
      numlines = NumLines(&Rectl, pAD);
      if (numlines) {
	pts.x = SHORT1FROMMP(mp1);
	pts.y = SHORT2FROMMP(mp1);
	if (pts.y < 0) {
	  WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
		     MPFROM2SHORT(0, SB_LINEDOWN));
	  pts.y = 1;
	  outofwindow = TRUE;
	}
	else if (pts.y > Rectl.yTop - Rectl.yBottom) {
	  WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
		     MPFROM2SHORT(0, SB_LINEUP));
	  pts.y = (SHORT) (Rectl.yTop - Rectl.yBottom) - 1;
	  outofwindow = TRUE;
	}
	whichfile = ((Rectl.yTop - Rectl.yBottom) -
		     ((LONG) pts.y + pAD->lMaxDescender)) / pAD->lMaxHeight;
	if (whichfile > numlines - 1)
	  whichfile = numlines - 1;
	whichfile += (pAD->topfile - 1);
	y = (pAD->invertsort) ? (pAD->afindexcnt - 1) - whichfile : whichfile;
	if (y < pAD->afindexcnt && pAD->lastselected != whichfile) {
	  if (pAD->lastselected != (ULONG) - 1) {
	    inc = (pAD->lastselected < whichfile) ? 1 : -1;
	    for (x = pAD->lastselected + inc;
		 x != whichfile && x < pAD->afindexcnt;
		 (pAD->lastselected < whichfile) ? x++ : x--) {
	      y = (pAD->invertsort) ? (pAD->afindexcnt - 1) - x : x;
	      if (pAD->afindex[y]->flags & AF_SELECTED) {
		pAD->afindex[y]->flags &= ~AF_SELECTED;
		pAD->selected--;
		pAD->ullSelectedBytes -= pAD->afindex[y]->cbFile;
	      }
	      else {
		pAD->afindex[y]->flags |= AF_SELECTED;
		pAD->selected++;
		pAD->ullSelectedBytes += pAD->afindex[y]->cbFile;
	      }
	      PaintLine(hwnd, pAD->hps, x, pAD->topfile, &Rectl);
	    }
	  }
	  WinSendMsg(hwnd, WM_BUTTON1CLICK, MPFROM2SHORT(pts.x, pts.y),
		     MPFROM2SHORT(TRUE, 0));
	}
      }
      if (outofwindow) {

	POINTL ptl;

	WinQueryPointerPos(HWND_DESKTOP, &ptl);
	WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
	if ((SHORT) ptl.y == (SHORT) SHORT2FROMMP(mp1) &&
	    (SHORT) ptl.x == (SHORT) SHORT1FROMMP(mp1) &&
	    ((SHORT) ptl.y < 0 || ptl.y > (Rectl.yTop - Rectl.yBottom))) {
	  PostMsg(hwnd, UM_MOUSEMOVE, mp1, MPVOID);
	  DosSleep(1);
	}
      }
    }
    break;

  case UM_MOUSEMOVE:
    if (pAD && pAD->mousecaptured) {

      POINTL ptl;
      RECTL Rectl;

      WinQueryWindowRect(hwnd, &Rectl);
      WinQueryPointerPos(HWND_DESKTOP, &ptl);
      WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
      if ((SHORT) ptl.y == (SHORT) SHORT2FROMMP(mp1) &&
	  (SHORT) ptl.x == (SHORT) SHORT1FROMMP(mp1) &&
	  ((SHORT) ptl.y < 0 || ptl.y > (Rectl.yTop - Rectl.yBottom))) {
	DosSleep(1);
	PostMsg(hwnd, WM_MOUSEMOVE, mp1, MPFROM2SHORT(TRUE, 0));
      }
    }
    return 0;

  case WM_BUTTON1UP:
  case WM_BUTTON1MOTIONEND:
    if (pAD) {
      pAD->mousecaptured = FALSE;
      pAD->lastselected = (ULONG) - 1;
      pAD->lastdirection = 0;
      WinSetCapture(HWND_DESKTOP, NULLHANDLE);
    }
    break;

  case WM_BUTTON1CLICK:
  case WM_BUTTON1DBLCLK:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (pAD && !pAD->stopflag &&
	!DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {

      ULONG numlines, whichfile, y, wascursored;
      RECTL Rectl;
      POINTS pts;

      if (pAD->afindexcnt) {
	WinQueryWindowRect(hwnd, &Rectl);
	numlines = NumLines(&Rectl, pAD);
	if (numlines) {
	  pts.x = SHORT1FROMMP(mp1);
	  pts.y = SHORT2FROMMP(mp1);
	  whichfile = ((Rectl.yTop - Rectl.yBottom) -
		       ((LONG) pts.y + pAD->lMaxDescender)) / pAD->lMaxHeight;
	  if (whichfile > numlines - 1)
	    whichfile = numlines - 1;
	  whichfile += (pAD->topfile - 1);
	  if (whichfile + 1 > pAD->afindexcnt)
	    break;
	  y = (pAD->invertsort) ? (pAD->afindexcnt - 1) - whichfile : whichfile;
	  wascursored = pAD->cursored;
	  pAD->cursored = whichfile + 1;
	  if (pAD->cursored != wascursored)
	    PaintLine(hwnd, pAD->hps, wascursored - 1, pAD->topfile, &Rectl);
	  if (y < pAD->afindexcnt) {
	    if (msg == WM_BUTTON1CLICK || fUnHilite) {
	      if (pAD->afindex[y]->flags & AF_SELECTED) {
		pAD->afindex[y]->flags &= ~AF_SELECTED;
		pAD->selected--;
		pAD->ullSelectedBytes -= pAD->afindex[y]->cbFile;
	      }
	      else {
		pAD->afindex[y]->flags |= AF_SELECTED;
		pAD->selected++;
		pAD->ullSelectedBytes += pAD->afindex[y]->cbFile;
	      }
	      PaintLine(hwnd, pAD->hps, whichfile, pAD->topfile, &Rectl);
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	  }
	  if (msg == WM_BUTTON1CLICK) {
	    if (pAD->lastselected != (ULONG) - 1) {
	      if (whichfile > pAD->lastselected)
		pAD->lastdirection = 1;
	      else
		pAD->lastdirection = 2;
	    }
	    else
	      pAD->lastdirection = 0;
	    pAD->lastselected = whichfile;
	  }
	  else
	    DefaultViewKeys(hwnd, pAD->hwndFrame, HWND_DESKTOP, NULL,
			    pAD->afindex[y]->fullname);
	}
      }
      DosReleaseMutexSem(pAD->hmtxScan);
    }
    break;

  case WM_MENUEND:
    if (pAD && (HWND) mp2 == pAD->hwndPopup) {
      WinDestroyWindow(pAD->hwndPopup);
      pAD->hwndPopup = (HWND) 0;
    }
    break;

  case WM_CONTEXTMENU:
    if (pAD) {
      if (!pAD->hwndPopup) {
	pAD->hwndPopup =
	  WinLoadMenu(HWND_DESKTOP, FM3ModHandle, SEEALL_POPUP);
	if (pAD->hwndPopup) {
	  //fixme to allow user to change presparams 1-10-09 GKY
	  WinSetPresParam(pAD->hwndPopup, PP_FONTNAMESIZE,
			  strlen(FNT_8HELVETICA) + 1,
			  (PVOID) FNT_8HELVETICA);
	  SetConditionalCascade(pAD->hwndPopup,
				IDM_DELETESUBMENU,
				(fDefaultDeletePerm) ?
				IDM_PERMDELETE : IDM_DELETE);
	  SetConditionalCascade(pAD->hwndPopup, IDM_MOVEMENU, IDM_MOVE);
	  SetConditionalCascade(pAD->hwndPopup, IDM_COPYMENU, IDM_COPY);
	  SetConditionalCascade(pAD->hwndPopup, IDM_OPENSUBMENU,
				IDM_OPENDEFAULT);
	  SetConditionalCascade(pAD->hwndMenu, IDM_OBJECTSUBMENU, IDM_SHADOW);
	  if (fWorkPlace) {
	    WinSendMsg(pAD->hwndPopup, MM_DELETEITEM,
		       MPFROM2SHORT(IDM_OPENSUBMENU, TRUE), MPVOID);
	    WinSendMsg(pAD->hwndPopup, MM_DELETEITEM,
		       MPFROM2SHORT(IDM_OBJECTSUBMENU, TRUE), MPVOID);
	  }
	}
      }
      if (pAD->hwndPopup) {

	APIRET rc;

	rc = DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN);
	WinEnableMenuItem(pAD->hwndPopup, IDM_EAS, (rc == 0 &&
						    pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_UUDECODE, (rc == 0 &&
                                                         pAD->selected != 0));
        WinEnableMenuItem(pAD->hwndPopup, IDM_UNLOCKFILE, (rc == 0 && fUnlock &&
							 pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_EXTRACT, (rc == 0 &&
							pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_ARCHIVE, (rc == 0 &&
							pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_MOVEMENU, (rc == 0 &&
							 pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_COPYMENU, (rc == 0 &&
							 pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_RENAME, (rc == 0 &&
						       pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_PRINT, (rc == 0 &&
						      pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_SUBJECT, (rc == 0 &&
							pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_OPENSUBMENU, (rc == 0 &&
							    pAD->selected !=
							    0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_OBJECTSUBMENU,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_DELETESUBMENU,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_INFO,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_ATTRS,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_COLLECT,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_SAVETOCLIP,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_SAVETOCLIPFILENAME,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_APPENDTOCLIP,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_APPENDTOCLIPFILENAME,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_SAVETOLIST,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_REMOVE,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_HIDEALL,
			  (rc == 0 && pAD->selected != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_SELECTALL,
			  (rc == 0 && pAD->afindexcnt != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_SELECTMASK,
			  (rc == 0 && pAD->afindexcnt != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_DESELECTALL,
			  (rc == 0 && pAD->afindexcnt != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_DESELECTMASK,
			  (rc == 0 && pAD->afindexcnt != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_INVERT,
			  (rc == 0 && pAD->afindexcnt != 0));
	WinEnableMenuItem(pAD->hwndPopup, IDM_FILTER,
			  (rc == 0 && pAD->afheadcnt != 0));
	if (!rc)
	  DosReleaseMutexSem(pAD->hmtxScan);
	if (WinPopupMenu(hwnd, hwnd, pAD->hwndPopup, SHORT1FROMMP(mp1),
			 SHORT2FROMMP(mp1), 0,
			 PU_HCONSTRAIN | PU_VCONSTRAIN |
			 PU_KEYBOARD | PU_MOUSEBUTTON1))
	  CenterOverWindow(pAD->hwndPopup);
      }
    }
    break;

  case UM_CONTAINER_FILLED:
    if (pAD) {
      pAD->stopflag = 0;
      pAD->topfile = 1;
      pAD->cursored = 1;
      pAD->multiplier = 1;
      if (!pAD->afindexcnt) {
	if (!fAlertBeepOff)
	  DosBeep(250, 50);
	PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      }
      else {
	if (!fAlertBeepOff)
	  DosBeep(1000, 25);
	WinInvalidateRect(hwnd, NULL, FALSE);
	PostMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
      }
      WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT), HWND_TOP, 0, 0, 0, 0,
		      SWP_SHOW | SWP_RESTORE | SWP_ACTIVATE | SWP_ZORDER);
    }
    return 0;

  case WM_ERASEBACKGROUND:
    WinFillRect((HPS) mp1, (PRECTL) mp2,
		standardcolors[Colors[COLORS_NORMALBACK]]);
    return 0;

  case WM_PAINT:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc WM_PAINT");
    if (pAD) {

      HPS hpsp;
      RECTL Rectl;
      POINTL ptl;
      register ULONG x;
      ULONG y, len, numlines;
      CHAR szBuff[CCHMAXPATH + 80], szCmmaFmtFileSize[81], szDate[DATE_BUF_BYTES];
      BOOL inverted, hidsys, reado, wascursored;

      hpsp = WinBeginPaint(hwnd, pAD->hps, &Rectl);
      WinFillRect(hpsp, &Rectl, standardcolors[Colors[COLORS_NORMALBACK]]);
      if (!pAD->stopflag &&
	  !DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {
	WinQueryWindowRect(hwnd, &Rectl);
	numlines = NumLines(&Rectl, pAD);
	if (pAD->afindexcnt && numlines) {
	  if (pAD->topfile > (pAD->afindexcnt + 1) - numlines)
	    pAD->topfile = (pAD->afindexcnt + 1) - numlines;
	  if (pAD->topfile > pAD->afindexcnt)
	    pAD->topfile = 1;
	  if (!pAD->topfile)
	    pAD->topfile = 1;
	  if (pAD->cursored < pAD->topfile)
	    pAD->cursored = pAD->topfile;
	  else if (pAD->cursored > (pAD->topfile + numlines) - 1)
	    pAD->cursored = (pAD->topfile + numlines) - 1;
	  if (pAD->cursored > pAD->afindexcnt)
	    pAD->cursored = pAD->afindexcnt;
	}
	else
	  pAD->topfile = pAD->cursored = 1;
	if (numlines && pAD->afindexcnt) {
	  GpiSetBackMix(hpsp, BM_OVERPAINT);
	  wascursored = TRUE;
	  for (x = pAD->topfile - 1; x < pAD->afindexcnt; x++) {
	    ptl.x = pAD->horzscroll;
	    if (wascursored) {          /* reestablish normal colors */
	      GpiSetColor(pAD->hps,
			  standardcolors[Colors[COLORS_NORMALFORE]]);
	      GpiSetBackColor(pAD->hps,
			      standardcolors[Colors[COLORS_NORMALBACK]]);
	      wascursored = inverted = hidsys = reado = FALSE;
	    }
	    if (x == pAD->cursored - 1)
	      wascursored = TRUE;
	    y = (pAD->invertsort) ? (pAD->afindexcnt - 1) - x : x;
	    ptl.y = (Rectl.yTop -
		     (pAD->lMaxHeight * (((x + 1) - pAD->topfile) + 1)));
	    if (ptl.y - pAD->lMaxDescender <= 0)
	      break;
	    if (pAD->afindex[y]->flags & AF_SELECTED) {
	      if (!inverted) {
		GpiSetColor(pAD->hps,
			    standardcolors[Colors
					   [COLORS_SELECTEDNORMALFORE]]);
		GpiSetBackColor(pAD->hps,
				(wascursored) ?
				standardcolors[Colors
					       [COLORS_CURSOREDSELECTEDBACK]]
				:
				standardcolors[Colors[COLORS_SELECTEDBACK]]);
		inverted = TRUE;
	      }
	    }
	    else if (inverted ||
		     ((pAD->afindex[y]->attrFile &
		       (FILE_SYSTEM | FILE_HIDDEN)) != 0) != hidsys ||
		     ((pAD->afindex[y]->attrFile & FILE_READONLY) != 0) !=
		     reado) {
	      if (pAD->afindex[y]->attrFile & (FILE_SYSTEM | FILE_HIDDEN)) {
		GpiSetColor(pAD->hps,
			    standardcolors[Colors[COLORS_SYSTEMFORE]]);
		hidsys = TRUE;
	      }
	      else if ((pAD->afindex[y]->attrFile & FILE_READONLY) != 0) {
		GpiSetColor(pAD->hps,
			    standardcolors[Colors[COLORS_READONLYFORE]]);
		reado = TRUE;
	      }
	      else
		GpiSetColor(pAD->hps,
			    standardcolors[Colors[COLORS_NORMALFORE]]);
	      GpiSetBackColor(pAD->hps,
			      (wascursored) ?
			      standardcolors[Colors
					     [COLORS_CURSOREDNORMALBACK]] :
			      standardcolors[Colors[COLORS_NORMALBACK]]);
	      inverted = FALSE;
	    }
	    else if (wascursored)
	      GpiSetBackColor(pAD->hps,
			      standardcolors[Colors
					     [COLORS_CURSOREDNORMALBACK]]);
	    CommaFmtULL(szCmmaFmtFileSize,
			sizeof(szCmmaFmtFileSize), pAD->afindex[y]->cbFile, ' ');
	    FDateFormat(szDate, pAD->afindex[y]->date);
	    len =
	      sprintf(szBuff,
		      "%c%-*.*s  %-12s  %c%c%c%c%c  %s %02u%s%02u%s%02u ",
		      wascursored ? '>' : ' ',
		      pAD->fullnames ? pAD->longestw : pAD->longest,
		      pAD->fullnames ? pAD->longestw : pAD->longest,
		      (pAD->fullnames) ? pAD->afindex[y]->fullname : pAD->
		      afindex[y]->filename, szCmmaFmtFileSize,
		      "-A"[((pAD->afindex[y]->attrFile & FILE_ARCHIVED) !=
			    0)],
		      "-R"[((pAD->afindex[y]->attrFile & FILE_READONLY) !=
			    0)],
		      "-H"[((pAD->afindex[y]->attrFile & FILE_HIDDEN) != 0)],
		      "-S"[((pAD->afindex[y]->attrFile & FILE_SYSTEM) != 0)],
		      "-D"[((pAD->afindex[y]->attrFile & FILE_DIRECTORY) != 0)],
		      szDate,
		      pAD->afindex[y]->time.hours, TimeSeparator,
		      pAD->afindex[y]->time.minutes, TimeSeparator,
		      pAD->afindex[y]->time.twosecs * 2);
	    GpiCharStringAt(hpsp, &ptl, len, szBuff);
	    GpiQueryCurrentPosition(hpsp, &ptl);
	    if (ptl.x + abs(pAD->horzscroll) > pAD->maxx) {
	      pAD->maxx = ptl.x + abs(pAD->horzscroll);
	      WinSendMsg(pAD->hhscroll, SBM_SETTHUMBSIZE,
			 MPFROM2SHORT((SHORT) Rectl.xRight,
				      (SHORT) pAD->maxx), MPVOID);
	    }
	  }
	}
	DosReleaseMutexSem(pAD->hmtxScan);
      }
      WinEndPaint(hpsp);
      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      if (!pAD->stopflag)
	WinSendMsg(pAD->hvscroll, SBM_SETSCROLLBAR,
		   MPFROMSHORT((SHORT) (pAD->topfile / pAD->multiplier)),
		   MPFROM2SHORT(1,
				(SHORT) (pAD->afindexcnt / pAD->multiplier) -
				(numlines - 1)));
      WinSendMsg(pAD->hhscroll, SBM_SETSCROLLBAR,
		 MPFROMSHORT((SHORT) abs(pAD->horzscroll)),
		 MPFROM2SHORT(0, (SHORT) (pAD->maxx - Rectl.xRight)));
      WinSendMsg(pAD->hhscroll, SBM_SETTHUMBSIZE,
		 MPFROM2SHORT((SHORT) Rectl.xRight, (SHORT) pAD->maxx),
		 MPVOID);
    }
    break;

  case WM_HSCROLL:
    {
      RECTL rectl;
      BOOL invalidate = TRUE;

      WinQueryWindowRect(hwnd, &rectl);
      switch (SHORT2FROMMP(mp2)) {
      case SB_PAGERIGHT:
	if (abs(pAD->horzscroll) < pAD->maxx - rectl.xRight) {
	  pAD->horzscroll -= rectl.xRight;
	  if (abs(pAD->horzscroll) > pAD->maxx - rectl.xRight)
	    pAD->horzscroll = -(pAD->maxx - rectl.xRight);
	}
	break;

      case SB_PAGELEFT:
	if (pAD->horzscroll < 0) {
	  pAD->horzscroll += rectl.xRight;
	  if (pAD->horzscroll > 0)
	    pAD->horzscroll = 0;
	}
	break;

      case SB_LINERIGHT:
	if (abs(pAD->horzscroll) < pAD->maxx - rectl.xRight)
	  pAD->horzscroll -= pAD->fattrs.lAveCharWidth;
	break;

      case SB_LINELEFT:
	if (pAD->horzscroll < 0)
	  pAD->horzscroll += pAD->fattrs.lAveCharWidth;
	break;

      case SB_SLIDERTRACK:
	pAD->horzscroll = SHORT1FROMMP(mp2);
	pAD->horzscroll = -(pAD->horzscroll);
	if (pAD->horzscroll > 0)
	  pAD->horzscroll = 0;
	if (abs(pAD->horzscroll) > pAD->maxx - rectl.xRight)
	  pAD->horzscroll = -(pAD->maxx - rectl.xRight);
	break;

      default:
	invalidate = FALSE;
	break;
      }
      if (invalidate)
	WinInvalidateRect(hwnd, NULL, FALSE);
    }
    break;

  case WM_VSCROLL:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc WM_VSCROLL");
    if (pAD && !pAD->stopflag &&
	!DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {

      ULONG numlines, wascursored;
      RECTL rcl;

      if (pAD->afindexcnt) {
	WinQueryWindowRect(hwnd, &rcl);
	numlines = NumLines(&rcl, pAD);
	if (numlines) {
	  wascursored = pAD->cursored;
	  switch (SHORT2FROMMP(mp2)) {
	  case SB_PAGEUP:
	    if (pAD->topfile > 1) {
	      pAD->topfile -= numlines;
	      if (pAD->topfile > pAD->afindexcnt ||
		  pAD->topfile > (pAD->afindexcnt + 1) - numlines)
		pAD->topfile = 1;
	      if (pAD->cursored > pAD->topfile + numlines)
		pAD->cursored = pAD->topfile + numlines;
	      if (pAD->cursored > pAD->afindexcnt)
		pAD->cursored = pAD->afindexcnt;
	      WinInvalidateRect(hwnd, NULL, FALSE);
	    }
	    break;
	  case SB_PAGEDOWN:
	    if (pAD->topfile <= pAD->afindexcnt - numlines) {
	      pAD->topfile += numlines;
	      if (pAD->topfile > (pAD->afindexcnt + 1) - numlines)
		pAD->topfile = (pAD->afindexcnt + 1) - numlines;
	      if (pAD->cursored < pAD->topfile)
		pAD->cursored = pAD->topfile;
	      if (pAD->cursored > (pAD->topfile + numlines) - 1)
		pAD->cursored = (pAD->topfile + numlines) - 1;
	      if (pAD->cursored > pAD->afindexcnt)
		pAD->cursored = pAD->afindexcnt;
	      WinInvalidateRect(hwnd, NULL, FALSE);
	    }
	    break;
	  case SB_LINEDOWN:
	    if (pAD->topfile <= pAD->afindexcnt - numlines) {

	      RECTL Rectl, iRectl;

	      pAD->topfile++;
	      if (pAD->cursored < pAD->topfile)
		pAD->cursored = pAD->topfile;
	      else if (pAD->cursored > (pAD->topfile + numlines) - 1)
		pAD->cursored = (pAD->topfile + numlines) - 1;
	      if (pAD->cursored > pAD->afindexcnt)
		pAD->cursored = pAD->afindexcnt;
	      WinQueryWindowRect(hwnd, &Rectl);
	      WinScrollWindow(hwnd, 0, pAD->lMaxHeight,
			      NULL, NULL, NULLHANDLE, &iRectl, 0);
	      WinFillRect(pAD->hps, &iRectl,
			  standardcolors[Colors[COLORS_NORMALBACK]]);
	      PaintLine(hwnd, pAD->hps, (pAD->topfile + numlines) - 2,
			pAD->topfile, &Rectl);
	      if (pAD->cursored != pAD->topfile + numlines)
		PaintLine(hwnd, pAD->hps, pAD->cursored - 1, pAD->topfile,
			  &Rectl);
	      if (wascursored != pAD->cursored
		  && wascursored < pAD->topfile + numlines
		  && wascursored >= pAD->topfile)
		PaintLine(hwnd, pAD->hps, wascursored - 1, pAD->topfile,
			  &Rectl);
	      if (wascursored != pAD->cursored)
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	      WinSendMsg(pAD->hhscroll, SBM_SETSCROLLBAR,
			 MPFROMSHORT((SHORT) abs(pAD->horzscroll)),
			 MPFROM2SHORT(0, (SHORT) (pAD->maxx - Rectl.xRight)));
	      WinSendMsg(pAD->hvscroll, SBM_SETSCROLLBAR,
			 MPFROMSHORT((SHORT) (pAD->topfile /
					      pAD->multiplier)),
			 MPFROM2SHORT(1, (SHORT) (pAD->afindexcnt /
						  pAD->multiplier) -
				      (numlines - 1)));
	    }
	    break;
	  case SB_LINEUP:
	    if (pAD->topfile > 1) {

	      RECTL Rectl, iRectl;

	      pAD->topfile--;
	      if (pAD->cursored < pAD->topfile)
		pAD->cursored = pAD->topfile;
	      else if (pAD->cursored > (pAD->topfile + numlines) - 1)
		pAD->cursored = (pAD->topfile + numlines) - 1;
	      if (pAD->cursored > pAD->afindexcnt)
		pAD->cursored = pAD->afindexcnt;
	      WinQueryWindowRect(hwnd, &Rectl);
	      WinScrollWindow(hwnd, 0, -pAD->lMaxHeight,
			      NULL, NULL, NULLHANDLE, &iRectl, 0);
	      WinFillRect(pAD->hps, &iRectl,
			  standardcolors[Colors[COLORS_NORMALBACK]]);
	      iRectl = Rectl;
	      iRectl.yTop -= ((numlines * pAD->lMaxHeight) +
			      pAD->lMaxDescender);
	      WinFillRect(pAD->hps, &iRectl,
			  standardcolors[pAD->aulColors[COLORS_NORMALBACK]]);
	      PaintLine(hwnd, pAD->hps, pAD->topfile - 1, pAD->topfile,
			&Rectl);
	      if (pAD->cursored != pAD->topfile)
		PaintLine(hwnd, pAD->hps, pAD->cursored - 1, pAD->topfile,
			  &Rectl);
	      if (pAD->cursored != wascursored && wascursored >= pAD->topfile
		  && wascursored < pAD->topfile + numlines)
		PaintLine(hwnd, pAD->hps, wascursored - 1, pAD->topfile,
			  &Rectl);
	      if (pAD->cursored != wascursored)
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	      WinSendMsg(pAD->hhscroll, SBM_SETSCROLLBAR,
			 MPFROMSHORT((SHORT) abs(pAD->horzscroll)),
			 MPFROM2SHORT(0, (SHORT) (pAD->maxx - Rectl.xRight)));
	      WinSendMsg(pAD->hvscroll, SBM_SETSCROLLBAR,
			 MPFROMSHORT((SHORT) (pAD->topfile /
					      pAD->multiplier)),
			 MPFROM2SHORT(1, (SHORT) (pAD->afindexcnt /
						  pAD->multiplier) -
				      (numlines - 1)));
	    }
	    break;
	  case SB_SLIDERTRACK:
	    if ((SHORT1FROMMP(mp2) >= 1) ||
		(SHORT1FROMMP(mp2)) <= pAD->afindexcnt) {
	      pAD->topfile = (ULONG) SHORT1FROMMP(mp2) * pAD->multiplier;
	      if (pAD->topfile > (pAD->afindexcnt + 1) - numlines)
		pAD->topfile = (pAD->afindexcnt + 1) - numlines;
	      if (!pAD->topfile)
		pAD->topfile = 1;
	      if (pAD->cursored < pAD->topfile)
		pAD->cursored = pAD->topfile;
	      else if (pAD->cursored > pAD->topfile + numlines)
		pAD->cursored = pAD->topfile + numlines;
	      if (pAD->cursored > pAD->afindexcnt)
		pAD->cursored = pAD->afindexcnt;
	      WinInvalidateRect(hwnd, NULL, FALSE);
	    }
	    else
	      WinAlarm(HWND_DESKTOP, WA_NOTE);
	    break;
	  }
	}
      }
      DosReleaseMutexSem(pAD->hmtxScan);
    }
    break;

  case WM_INITMENU:
    if (pAD) {
      switch (SHORT1FROMMP(mp1)) {
      case IDM_FILESMENU:
	{
	  APIRET rc;

	  rc = DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN);
	  WinEnableMenuItem((HWND) mp2, IDM_DUPES, (rc == 0));
	  WinEnableMenuItem((HWND) mp2, IDM_COLLECT, (rc == 0 &&
						      pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_SAVETOCLIP, (rc == 0 &&
							 pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_SAVETOCLIPFILENAME,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_APPENDTOCLIP,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_APPENDTOCLIPFILENAME,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_SAVETOLIST,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_REMOVE,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_HIDEALL,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_DELETESUBMENU,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_INFO,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_ATTRS,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_EAS,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_UUDECODE,
                            (rc == 0 && pAD->selected != 0));
          WinEnableMenuItem((HWND) mp2, IDM_UNLOCKFILE,
			    (rc == 0 && fUnlock && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_EXTRACT,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_ARCHIVE,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_MOVEMENU,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_COPYMENU,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_RENAME,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_PRINT,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_SUBJECT,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_OPENSUBMENU,
			    (rc == 0 && pAD->selected != 0));
	  WinEnableMenuItem((HWND) mp2, IDM_OBJECTSUBMENU,
			    (rc == 0 && pAD->selected != 0));
	  if (!rc)
	    DosReleaseMutexSem(pAD->hmtxScan);
	}
	break;

      case IDM_SELECTSUBMENU:
	{
	  APIRET rc;

	  rc = DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN);
	  WinEnableMenuItem((HWND) mp2, IDM_SELECTALL, (rc == 0 &&
							pAD->afindexcnt != 0 &&
							(pAD->afindexcnt !=
							 pAD->selected ||
							 !pAD->selected)));
	  WinEnableMenuItem((HWND) mp2, IDM_SELECTMASK, (rc == 0 &&
							 pAD->afindexcnt != 0 &&
							 (pAD->afindexcnt !=
							  pAD->selected ||
							  !pAD->selected)));
	  WinEnableMenuItem((HWND) mp2, IDM_DESELECTALL, (rc == 0 &&
							  pAD->afindexcnt != 0
							  && pAD->selected));
	  WinEnableMenuItem((HWND) mp2, IDM_DESELECTMASK,
			    (rc == 0 && pAD->afindexcnt != 0) && pAD->selected);
	  WinEnableMenuItem((HWND) mp2, IDM_INVERT,
			    (rc == 0 && pAD->afindexcnt != 0));
	  if (!rc)
	    DosReleaseMutexSem(pAD->hmtxScan);
	}
	break;

      case IDM_SORTSUBMENU:
	{
	  APIRET rc;

	  rc = DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN);
	  WinEnableMenuItem((HWND) mp2, IDM_SORTNAME, (rc == 0));
	  WinEnableMenuItem((HWND) mp2, IDM_SORTEASIZE, (rc == 0));
	  WinEnableMenuItem((HWND) mp2, IDM_SORTSIZE, (rc == 0));
	  WinEnableMenuItem((HWND) mp2, IDM_SORTLWDATE, (rc == 0));
	  WinEnableMenuItem((HWND) mp2, IDM_SORTFILENAME, (rc == 0));
	  WinEnableMenuItem((HWND) mp2, IDM_SORTFIRST, (rc == 0));
	  if (!rc)
	    DosReleaseMutexSem(pAD->hmtxScan);
	}
	WinCheckMenuItem((HWND) mp2, IDM_SORTNAME,
			 (pAD->pfnCompare == comparefullnames));
	WinCheckMenuItem((HWND) mp2, IDM_SORTEASIZE,
			 (pAD->pfnCompare == (PFNSORT) NULL));
	WinCheckMenuItem((HWND) mp2, IDM_SORTSIZE,
			 (pAD->pfnCompare == comparesizes));
	WinCheckMenuItem((HWND) mp2, IDM_SORTLWDATE,
			 (pAD->pfnCompare == comparedates));
	WinCheckMenuItem((HWND) mp2, IDM_SORTFILENAME,
			 (pAD->pfnCompare == comparenames));
	WinCheckMenuItem((HWND) mp2, IDM_SORTREVERSE, pAD->invertsort);
	WinCheckMenuItem((HWND) mp2, IDM_SORTFIRST,
			 (pAD->pfnCompare == compareexts));
	break;

      case IDM_VIEWSMENU:
	{
	  APIRET rc;

	  rc = DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN);
	  WinEnableMenuItem((HWND) mp2, IDM_RESCAN, (rc == 0));
	  WinEnableMenuItem((HWND) mp2, IDM_FILTER, (rc == 0 &&
						     pAD->afheadcnt != 0));
	  if (!rc)
	    DosReleaseMutexSem(pAD->hmtxScan);
	}
	WinCheckMenuItem((HWND) mp2, IDM_SHOWLNAMES, pAD->fullnames);
	break;
      }
    }
    break;

  case WM_COMMAND:
    if (!pAD) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      return 0;
    }
    switch (SHORT1FROMMP(mp1)) {
    case IDM_SETTARGET:
      SetTargetDir(hwnd, FALSE);
      break;

    case IDM_DUPES:
      if (!pAD->stopflag &&
	  !DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {
	pAD->dupeflags = (USHORT) WinDlgBox(HWND_DESKTOP,
					    hwnd,
					    DupeDlgProc,
					    FM3ModHandle,
					    DUPE_FRAME,
					    MPFROM2SHORT(pAD->dupeflags, 0));
	if (pAD->dupeflags) {
	  xbeginthread(FindDupesThread,
		       65536,
		       (PVOID)hwnd,
		       pszSrcFile,
		       __LINE__);
	}
	DosReleaseMutexSem(pAD->hmtxScan);
      }
      break;

    case IDM_COLORPALETTE:
      {
	COLORS co;
	LONG temp[COLORS_MAX];

	memset(&co, 0, sizeof(co));
	co.size = sizeof(co);
	co.numcolors = COLORS_MAX;
	co.colors = pAD->aulColors;
	co.descriptions = IDS_SACOLORS1TEXT;
	co.origs = temp;
	co.prompt = IDS_SACOLORSPROMPTTEXT;
	memcpy(temp, pAD->aulColors, sizeof(LONG) * COLORS_MAX);
	if (WinDlgBox(HWND_DESKTOP,
		      hwnd,
		      ColorDlgProc,
		      FM3ModHandle, COLOR_FRAME, (PVOID) & co)) {
	  memcpy(Colors, pAD->aulColors, sizeof(LONG) * COLORS_MAX);
	  PrfWriteProfileData(fmprof,
			      appname,
			      "Seeall.Colors",
			      &pAD->aulColors, sizeof(LONG) * COLORS_MAX);
	  WinInvalidateRect(hwnd, NULL, FALSE);
	}
      }
      break;

    case IDM_CODEPAGE:
      {
	INT cp;

	cp = PickCodepage(hwnd);
	if (cp != -1) {
	  pAD->fattrs.usCodePage = (USHORT) cp;
	  Codepage = pAD->fattrs.usCodePage;
	  PrfWriteProfileData(fmprof,
			      appname,
			      "Seeall.Codepage",
			      &pAD->fattrs.usCodePage, sizeof(USHORT));
	  GpiDeleteSetId(pAD->hps, SEEALLFILECNR_FONT_LCID);
	  GpiAssociate(pAD->hps, 0);
	  GpiDestroyPS(pAD->hps);
	  pAD->hps = InitWindow(hwnd);
	  pAD->maxx = pAD->horzscroll = 0;
	  WinInvalidateRect(hwnd, NULL, FALSE);
	}
      }
      break;

    case IDM_FONTPALETTE:
      SetMLEFont(hwnd, &pAD->fattrs, 3);
      PrfWriteProfileData(fmprof,
			  appname,
			  "Seeall.Fattrs", &pAD->fattrs, sizeof(pAD->fattrs));
      Fattrs = pAD->fattrs;
      GpiDeleteSetId(pAD->hps, SEEALLFILECNR_FONT_LCID);
      GpiAssociate(pAD->hps, 0);
      GpiDestroyPS(pAD->hps);
      pAD->hps = InitWindow(hwnd);
      pAD->maxx = pAD->horzscroll = 0;
      WinInvalidateRect(hwnd, NULL, FALSE);
      break;

    case IDM_SHOWLNAMES:
      pAD->fullnames = (pAD->fullnames) ? FALSE : TRUE;
      PrfWriteProfileData(fmprof,
			  appname,
			  "Seeall.Fullnames", &pAD->fullnames, sizeof(BOOL));
      Fullnames = pAD->fullnames;
      pAD->maxx = pAD->horzscroll = 0;
      WinInvalidateRect(hwnd, NULL, FALSE);
      break;

    case IDM_SORTREVERSE:
      pAD->invertsort = (pAD->invertsort) ? FALSE : TRUE;
      SortReverse = pAD->invertsort;
      PrfWriteProfileData(fmprof,
			  appname,
			  "Seeall.SortReverse",
			  (PVOID) & pAD->invertsort, sizeof(BOOL));
      WinInvalidateRect(hwnd, NULL, FALSE);
      break;

    case IDM_SORTEASIZE:
    case IDM_SORTNAME:
    case IDM_SORTFILENAME:
    case IDM_SORTSIZE:
    case IDM_SORTLWDATE:
    case IDM_SORTFIRST:
      if (!pAD->stopflag &&
	  !DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {
	{
	  USHORT dummy = SHORT1FROMMP(mp1);

	  PrfWriteProfileData(fmprof,
			      appname,
			      "Seeall.Sort", (PVOID) & dummy, sizeof(USHORT));
	  SortType = SHORT1FROMMP(mp1);
	}
	WinSetPointer(HWND_DESKTOP, hptrBusy);
	WinSendMsg(hwnd, UM_RESCAN, MPFROMLONG(1), MPVOID);
	switch (SHORT1FROMMP(mp1)) {
	case IDM_SORTEASIZE:
	  pAD->pfnCompare = (PFNSORT) NULL;
	  ReSort(hwnd);
	  break;

	case IDM_SORTNAME:
	  pAD->pfnCompare = comparefullnames;
	  ReSort(hwnd);
	  break;

	case IDM_SORTFILENAME:
	  pAD->pfnCompare = comparenames;
	  ReSort(hwnd);
	  break;

	case IDM_SORTSIZE:
	  pAD->pfnCompare = comparesizes;
	  ReSort(hwnd);
	  break;

	case IDM_SORTLWDATE:
	  pAD->pfnCompare = comparedates;
	  ReSort(hwnd);
	  break;

	case IDM_SORTFIRST:
	  pAD->pfnCompare = compareexts;
	  ReSort(hwnd);
	  break;
	}
	WinSetPointer(HWND_DESKTOP, hptrArrow);
	DosReleaseMutexSem(pAD->hmtxScan);
	WinInvalidateRect(hwnd, NULL, FALSE);
      }
      break;

    case IDM_FILTER:
      if (!pAD->stopflag &&
	  !DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {
	FilterList(hwnd);
	DosReleaseMutexSem(pAD->hmtxScan);
      }
      break;

    case IDM_RESCAN:
      if (!pAD->stopflag &&
	  !DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {

	CHAR tempflags[26];

	memcpy(tempflags, pAD->abDrvFlags, sizeof(tempflags));
	if (!WinDlgBox(HWND_DESKTOP, hwnd, AFDrvsWndProc, FM3ModHandle,
		       DRVS_FRAME, (PVOID) pAD)) {
	  memcpy(pAD->abDrvFlags, tempflags, sizeof(tempflags));
	  *pAD->szFindPath = 0;
	  DosReleaseMutexSem(pAD->hmtxScan);
	  break;
	}
	WinSendMsg(pAD->hhscroll, SBM_SETTHUMBSIZE, MPFROM2SHORT(1, 1),
		   MPVOID);
	WinSendMsg(pAD->hvscroll, SBM_SETTHUMBSIZE, MPFROM2SHORT(1, 1),
		   MPVOID);
	pAD->topfile = 1;
	pAD->cursored = 1;
	pAD->selected = 0;
	pAD->ullSelectedBytes = 0;
	pAD->maxx = pAD->horzscroll = 0;
	FreeAllFilesList(hwnd);
	pAD->stopflag = 0;
	if (xbeginthread(FindAllThread,
			 524288,
			 (PVOID)hwnd,
			 pszSrcFile,
			 __LINE__) == -1)
	{
	  WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
	  DosReleaseMutexSem(pAD->hmtxScan);
	}
	else {
	  DosReleaseMutexSem(pAD->hmtxScan);
	  DosSleep(50);//05 Aug 07 GKY 100
	  WinInvalidateRect(hwnd, NULL, FALSE);
	  PostMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);
	  PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	}
      }
      break;
    case IDM_UNHIDEALL:
      {
      ALLDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
      FilterAll(hwnd, ad);
      }
      break;

    case IDM_DELETE:
    case IDM_PERMDELETE:
    case IDM_SELECTALL:
    case IDM_DESELECTALL:
    case IDM_INVERT:
    case IDM_SELECTMASK:
    case IDM_DESELECTMASK:
    case IDM_REMOVE:
    case IDM_HIDEALL:
    case IDM_COLLECT:
    case IDM_COLLECTOR:
    case IDM_SAVETOCLIP:
    case IDM_SAVETOCLIPFILENAME:
    case IDM_APPENDTOCLIP:
    case IDM_APPENDTOCLIPFILENAME:
    case IDM_SAVETOLIST:
    case IDM_INFO:
    case IDM_ATTRS:
    case IDM_MOVE:
    case IDM_COPY:
    case IDM_RENAME:
    case IDM_MOVEPRESERVE:
    case IDM_COPYPRESERVE:
    case IDM_WILDMOVE:
    case IDM_WILDCOPY:
    case IDM_SUBJECT:
    case IDM_EAS:
    case IDM_PRINT:
    case IDM_ARCHIVE:
    case IDM_EXTRACT:
    case IDM_UUDECODE:
    case IDM_UNLOCKFILE:
    case IDM_SHADOW:
    case IDM_OBJECT:
    case IDM_OPENSETTINGS:
    case IDM_OPENDEFAULT:
      if (!pAD->stopflag &&
	  !DosRequestMutexSem(pAD->hmtxScan, SEM_IMMEDIATE_RETURN)) {
	switch (SHORT1FROMMP(mp1)) {
	case IDM_SELECTALL:
	case IDM_DESELECTALL:
	case IDM_INVERT:
	case IDM_HIDEALL:
	case IDM_REMOVE:
	  Mark(hwnd, (SHORT1FROMMP(mp1) == IDM_DESELECTALL) ?
	       AFM_UNMARK : (SHORT1FROMMP(mp1) == IDM_INVERT) ?
	       AFM_INVERT : (SHORT1FROMMP(mp1) == IDM_HIDEALL) ?
	       AFM_FILTER : (SHORT1FROMMP(mp1) == IDM_REMOVE) ?
	       AFM_MARKDELETED : 0, NULL);
	  if (SHORT1FROMMP(mp1) == IDM_REMOVE ||
	      SHORT1FROMMP(mp1) == IDM_HIDEALL) {
	    if (SHORT1FROMMP(mp1) == IDM_REMOVE)
	      RemoveDeleted(hwnd);
	    else
	      ReSort(hwnd);
	  }
	  WinInvalidateRect(hwnd, NULL, FALSE);
	  break;

	case IDM_SELECTMASK:
	case IDM_DESELECTMASK:
	  SelectMask(hwnd, (SHORT1FROMMP(mp1) == IDM_DESELECTMASK));
	  WinInvalidateRect(hwnd, NULL, FALSE);
	  break;

	case IDM_DELETE:
	case IDM_PERMDELETE:
	case IDM_APPENDTOCLIP:
	case IDM_APPENDTOCLIPFILENAME:
	case IDM_SAVETOCLIP:
	case IDM_SAVETOCLIPFILENAME:
	case IDM_SAVETOLIST:
	case IDM_COLLECT:
	case IDM_INFO:
	case IDM_ATTRS:
	case IDM_MOVE:
	case IDM_COPY:
	case IDM_RENAME:
	case IDM_MOVEPRESERVE:
	case IDM_COPYPRESERVE:
	case IDM_WILDMOVE:
	case IDM_WILDCOPY:
	case IDM_SUBJECT:
	case IDM_PRINT:
	case IDM_EAS:
	case IDM_ARCHIVE:
	case IDM_EXTRACT:
	case IDM_SHADOW:
	case IDM_OBJECT:
	case IDM_OPENSETTINGS:
	case IDM_OPENDEFAULT:
        case IDM_UUDECODE:
        case IDM_UNLOCKFILE:
	  {
	    CHAR **list = BuildAList(hwnd);

	    if (!list)
	      Runtime_Error(pszSrcFile, __LINE__, NULL);
	    else {
	      switch (SHORT1FROMMP(mp1)) {
	      case IDM_COLLECT:
		CollectList(hwnd, list);
		break;
	      case IDM_DELETE:
	      case IDM_PERMDELETE:
	      case IDM_APPENDTOCLIP:
	      case IDM_APPENDTOCLIPFILENAME:
	      case IDM_SAVETOCLIP:
	      case IDM_SAVETOCLIPFILENAME:
	      case IDM_SAVETOLIST:
	      case IDM_INFO:
	      case IDM_ATTRS:
	      case IDM_MOVE:
	      case IDM_COPY:
	      case IDM_RENAME:
	      case IDM_MOVEPRESERVE:
	      case IDM_COPYPRESERVE:
	      case IDM_WILDMOVE:
	      case IDM_WILDCOPY:
	      case IDM_SUBJECT:
	      case IDM_PRINT:
	      case IDM_EAS:
	      case IDM_ARCHIVE:
	      case IDM_EXTRACT:
              case IDM_UUDECODE:
              case IDM_UNLOCKFILE:
	      case IDM_OBJECT:
	      case IDM_SHADOW:
	      case IDM_OPENSETTINGS:
	      case IDM_OPENDEFAULT:
		if (!PostMsg(pAD->hwndObj, UM_MASSACTION, mp1, MPFROMP(list)))
		  FreeList(list);
		break;
	      }
	      if (fUnHilite) {
		Mark(hwnd, AFM_UNMARK, NULL);
		WinInvalidateRect(hwnd, NULL, FALSE);
	      }
	    }
	  }
	  break;

	case IDM_COLLECTOR:
	  if (mp2) {

	    CHAR **list = mp2;

	    if (Collector) {
	      if (!PostMsg(Collector, WM_COMMAND,
			   MPFROM2SHORT(IDM_COLLECTOR, 0), MPFROMP(list)))
		FreeList(list);
	      else if (fUnHilite) {
		Mark(hwnd, AFM_UNMARK, NULL);
		WinInvalidateRect(hwnd, NULL, FALSE);
	      }
	    }
	    else
	      FreeList(list);
	  }
	  break;
	}
	DosReleaseMutexSem(pAD->hmtxScan);
      }
      else if (SHORT1FROMMP(mp1) == IDM_COLLECTOR) {
	DosSleep(50);//05 Aug 07 GKY 100
	if (!PostMsg(hwnd, msg, mp1, mp2))
	  WinSendMsg(hwnd, msg, mp1, mp2);
      }
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_SEEALL, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_SIZE:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc WM_SIZE");
    PostMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
    break;

  case WM_CLOSE:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc WM_CLOSE");
    if (pAD)
      pAD->stopflag = 1;
    WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
    return 0;

  case WM_DESTROY:
    // DbgMsg(pszSrcFile, __LINE__, "SeeAllWndProc WM_DESTROY");
    if (pAD) {
      pAD->stopflag = 1;
      if (pAD->hmtxScan) {
	apiret = DosRequestMutexSem(pAD->hmtxScan, 2000);
	if (apiret != NO_ERROR)
	  Dos_Error(MB_CANCEL, apiret, hwnd, pszSrcFile, __LINE__, "DosRequestMutexSem");
	DosCloseMutexSem(pAD->hmtxScan);	// Are probably going to die anyway
      }
      if (pAD->hwndPopup)
	WinDestroyWindow(pAD->hwndPopup);
      if (pAD->hwndObj) {
	if (!PostMsg(pAD->hwndObj, WM_CLOSE, MPVOID, MPVOID))
	  WinSendMsg(pAD->hwndObj, WM_CLOSE, MPVOID, MPVOID);
      }
      if (pAD->hps) {
	GpiDeleteSetId(pAD->hps, SEEALLFILECNR_FONT_LCID);
	GpiAssociate(pAD->hps, 0);
	GpiDestroyPS(pAD->hps);
      }
      if (pAD->killme) {
	if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
	  WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
      }
      FreeAllFilesList(hwnd);
      free(pAD);
    }
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
    break;
  }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

HWND StartSeeAll(HWND hwndParent, BOOL standalone,      // called by applet
		 CHAR * pszStartPath)   // pathname or NULL
{
  HWND hwndFrame = (HWND) 0, hwndClient;
  ULONG FrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
    FCF_SIZEBORDER | FCF_MINMAX |
    FCF_NOBYTEALIGN | FCF_VERTSCROLL |
    FCF_MENU | FCF_ICON | FCF_ACCELTABLE | FCF_HORZSCROLL;

  if (ParentIsDesktop(hwndParent, hwndParent))
    FrameFlags |= (FCF_TASKLIST | FCF_SHELLPOSITION);
  hwndFrame = WinCreateStdWindow(hwndParent,
				 WS_VISIBLE,
				 &FrameFlags,
				 (CHAR *) WC_SEEALL,
				 (CHAR *) GetPString(IDS_SEEALLTITLETEXT),
				 WS_VISIBLE | fwsAnimate,
				 FM3ModHandle, SEEALL_FRAME, &hwndClient);
  if (hwndFrame) {
    static CHAR *pszDir;

    if (standalone) {
      if (!PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
		   UM_SETUP4, MPVOID, MPVOID)) {
	PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
	return (HWND) 0;
      }
    }
    if (pszStartPath) {
      // Needs to be static for other thread
      if (!pszDir)
	pszDir = xmalloc(CCHMAXPATH, pszSrcFile, __LINE__);
      if (pszDir) {
	strcpy(pszDir, pszStartPath);
	pszStartPath = pszDir;
      }
    }
    PostMsg(WinWindowFromID(hwndFrame, FID_CLIENT),
	    UM_SETUP5, MPFROMP(pszStartPath), MPVOID);
  }
  else if (standalone)
    PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
  return hwndFrame;
}

#pragma alloc_text(SEEALL,comparefullnames,comparenames,comparesizes)
#pragma alloc_text(SEEALL,comparedates,compareexts,SeeStatusProc)
#pragma alloc_text(SEEALL,InitWindow,PaintLine,SeeAllWndProc)
#pragma alloc_text(SEEALL,UpdateList,CollectList,ReSort,Mark)
#pragma alloc_text(SEEALL,BuildAList,RemoveDeleted,SeeFrameWndProc,FilterList,FilterAll)
#pragma alloc_text(SEEALL2,SeeObjWndProc,MakeSeeObjWinThread,FindDupesThread,DupeDlgProc)
#pragma alloc_text(SEEALL3,FreeAllFilesList,DoADir,FindAllThread,AFDrvsWndProc)
#pragma alloc_text(SEEALL3,StartSeeAll)

