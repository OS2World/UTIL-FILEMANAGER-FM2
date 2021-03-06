
/***********************************************************************

  $Id$

  Tree containers

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2015 Steven H. Levine

  16 Oct 02 SHL Handle large partitions
  11 Jun 03 SHL Add JFS and FAT32 support
  25 May 05 SHL Rename comnam to szCommonName and fix typo
  25 May 05 SHL Use ULONGLONG and CommaFmtULL
  26 May 05 SHL More large file formatting updates
  05 Jun 05 SHL Use QWL_USER
  06 Aug 05 SHL Renames
  08 Dec 05 SHL TreeCnrWndProc: disable menu items if drive not ready
  17 Jul 06 SHL Use Runtime_Error
  15 Aug 06 SHL Rework SetMask args
  31 Aug 06 JS  Add more partitioning menu items
  22 Oct 06 GKY Add NDFS32 support
  29 Dec 06 GKY Fixed menu gray out for remote drives (added variable "remote")
  29 Dec 06 GKY Enabled edit of drive flags on "not ready" drives
  18 Feb 07 GKY More drive type and icon support
  08 Mar 07 SHL Ensure drive icon updates after drive flags change
  09 Mar 07 GKY Use SelectDriveIcon
  30 Mar 07 GKY Remove GetPString for window class names
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limits
  06 Apr 07 GKY Add some error checking in drag/drop
  19 Apr 07 SHL Sync with AcceptOneDrop GetOneDrop mods
  19 Apr 07 SHL Add more drag/drop error checking
  12 May 07 SHL Use dcd->ulItemsToUnHilite; sync with UnHilite arg mods
  10 Jun 07 GKY Add CheckPmDrgLimit including IsFm2Window as part of work around PM drag limit
  10 Jun 07 GKY Mouse button 3 white space click to fail silently
  05 Jul 07 SHL Disable leftover debug code
  02 Aug 07 SHL Sync with CNRITEM mods
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  14 Aug 07 SHL Revert ShowTreeRec DosSleep to 0
  14 Aug 07 SHL Optimze ShowTreeRec collapse - was really slow
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  22 Aug 07 SHL Disable DbgMsgs shipped with 3.0.8beta1
  26 Aug 07 SHL Revert to DosSleep(0)
  22 Nov 07 GKY Use CopyPresParams to fix presparam inconsistencies in menus
  10 Jan 08 SHL Sync with CfgDlgProc mods
  15 Feb 08 SHL Sync with settings menu rework
  15 Feb 08 SHL Avoid death if tree container 0 width
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory
  02 Aug 08 GKY Always pass temp variable point to UM_SHOWME to avoid freeing pci->pszFileName early
  19 Oct 08 GKY Fixed logic for greying menu items (Format etc) on remote and virtual drives (it was reversed)
  19 Oct 08 GKY Fixed context menu to be "drives" menu on unformatted drives
  28 Nov 08 GKY Remove unneeded DosEnterCriSec calls
  10 Dec 08 SHL Integrate exception handler support
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  26 Dec 08 GKY Implemented DROPHELP for the tree container
  27 Dec 08 GKY Add refresh removable media to tree container menus
  28 Dec 08 GKY Rework partition submenu to gray out unavailable items (check for existence of files)
		and have no default choice.
  01 Jan 09 GKY Add Seek and Scan to drives & directory context menus pass drive/dir as search root
  11 Jan 09 GKY Replace font names in the string file with global set at compile in init.c
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  08 Mar 09 GKY Renamed commafmt.h i18nutil.h
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  12 Mar 09 SHL Use common SearchContainer
  14 Mar 09 GKY Prevent execution of UM_SHOWME while drive scan is occuring
  06 Jun 09 GKY Add option to show file system type or drive label in tree
  06 Jun 09 GKY Status line to show file sys/label not shown in tree; shortened to fit split status
  07 Jun 09 GKY Fixed double names in tree container when collapsed tree is accessed
		before recursive scan
  12 Jul 09 GKY Add option to show file system type or drive label in tree
		(get NOPRESCAN drives working)
  22 Jul 09 GKY Code changes to use semaphores to serialize drive scanning
  22 Jul 09 GKY Consolidated driveflag setting code in DriveFlagsOne
  22 Jul 09 GKY Streamline scanning code for faster Tree rescans
  14 Sep 09 SHL Drop experimental code
  15 Sep 09 SHL Use UM_GREP when passing pathname
  15 Nov 09 GKY Add semaphore to fix double names in tree container caused by UM_SHOWME
		before scan completes
  22 Nov 09 GKY Add LVM.EXE to partition submenu
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast
		CHAR CONSTANT * as CHAR *.
  11 Apr 10 GKY Fix drive tree rescan failure and program hang caused by event sem
		never being posted
  20 Nov 10 GKY Rework scanning code to remove redundant scans, prevent double directory
		entries in the tree container, fix related semaphore performance using
		combination of event and mutex semaphores
  04 Aug 12 GKY Fix trap reported by Ben
  30 Dec 12 GKY Changed refresh removable media to query LVM directly to call Rediscover_PRMs (Ticket 472);
		Also added a tree rescan following volume detach.
  22 Feb 14 GKY Fix warn readonly yes don't ask to work when recursing directories.
  07 Sep 14 GKY Fix tree container mis-draws (stacked icons with RWS) The problem was magnified
		by RWS but I think the occasional extra blank directory or duplicating
		directories is related.
  16 Mar 15 GKY Add semaphore hmtxFiltering to prevent freeing dcd while filtering. Prevents
		a trap when FM2 is shutdown or the container is closed while tree
		container is still populating
  02 May 15 GKY Changes to allow a JAVA executable object to be created using "Real object"
		menu item on a jar file.
  12 Jul 15 GKY Fixed trap caused by pci->pszFileName being NullStr
  07 Aug 15 SHL Rework to use AddFleshWorkRequest rather than direct calls to Stubby/Flesh/Unflesh
  20 Aug 15 SHL Sync with SetFleshFocusPath mods
  22 Aug 15 GKY Improve ability of maketop to get directory position in tree correct on first
		open of states with large and/or deep tree structures
  24 Aug 15 GKY Remove fDummy code
  20 Sep 15 GKY Get expand and switch code to work with Flesh, UnFlesh and Stubby running on
                a thread. Loop and idle ExpandAll; Add CollapseAll; Move tree expand to a
                thread; Have ShowTreeRec wait for the Flesh thread.
  26 Sep 15 GKY Adjustments to ShowTreeRec to eliminate failures and reduce retries and container
                noise on tree switches.
  26 Sep 15 GKY Remove fInitialDriveScan code
  26 Sep 15 GKY Changes to speed up ExpandAll
  26 Sep 15 GKY Put UM_TOPDIR in the object window so it can call WaitFleshWorkListEmpty
                while avoiding thread 1
  27 Sep 15 GKY DosSleep times in WaitFleshWorkListEmpty set by caller
  04 Oct 15 GKY Move the eUnflesh call from UM_ENTER to the object window so WaitFleshWorkListEmpty
                can be used (UM_ENTER is on TID 1); Prevent eUnflesh from running if no child
                directories are present; treat floppies like invalid drives on rescan (IDM_UPDATE
                to avoid them seen as directories and having random subdirectories attached to
                them.
  10 Oct 15 GKY Eliminate some unnecessary Flesh and UnFlesh calls
  10 Oct 15 GKY Update icon and display name on CD/DVD eject in all cases.
  10 Oct 15 GKY Don't use Flesh thread for floppy drive scans fix them getting mistakenly identified
                as directories and add nonexistent subdirectories.
  12 Oct 15 GKY Increase ExpandAll waits for removable drives avoids directory name corruption

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG
#define INCL_DOSERRORS

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "treecnr.h"
#include "mainwnd2.h"			// Data declaration(s)
#include "grep.h"			// Data declaration(s)
#include "dircnrs.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"
#include "comp.h"			// COMPARE
#include "filldir.h"			// RemoveCnrItems...
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "notebook.h"			// CfgDlgProc
#include "command.h"			// RunCommand
#include "worker.h"			// Action, MassAction
#include "mainwnd.h"			// BubbleHelp, FindDirCnrByName, GetNextWindowPos
#include "misc.h"			// CnrDirectEdit, EmphasizeButton, FindDirCnr
					// FindDirCnr, FixSwitchList, OpenEdit, QuickPopup
					// SetSortChecks, SwitchCommand, CheckMenu
					// CurrentRecord, IsFm2Window
#include "common.h"			// CommonCnrProc, CommonDriveCmd, CommonFrameWndProc
					// CommonTextProc
#include "valid.h"			// CheckDrive, DriveFlagsOne, IsValidDrive
#include "chklist.h"			// DropListProc
#include "select.h"			// ExpandAll
#include "findrec.h"			// FindCnrRecord, FindParentRecord, ShowCnrRecord
#include "flesh.h"			// AddFleshWorkRequest
#include "notify.h"			// HideNote
#include "objwin.h"			// MakeObjWin
#include "notify.h"			// NotifyError
#include "remap.h"			// RemapDlgProc
#include "saveclip.h"			// SaveListDlgProc
#include "update.h"			// SelectDriveIcon, UpdateCnrList, UpdateCnrRecord
#include "sortcnr.h"			// SortTreeCnr
#include "droplist.h"			// AcceptOneDrop, CheckPmDrgLimit, DropHelp, GetOneDrop
#include "presparm.h"			// CopyPresParams
#include "defview.h"			// DefaultViewKeys
#include "draglist.h"			// DoFileDrag
#include "filter.h"			// Filter
#include "shadow.h"			// OpenObject
#include "mkdir.h"			// PMMkDir
#include "collect.h"			// StartCollector
#include "viewer.h"			// StartMLEEditor
#include "newview.h"			// StartViewer
#include "walkem.h"			// WalkAllDlgProc
#include "i18nutil.h"			// CommaFmtULL
#include "wrappers.h"			// xDosFindFirst
#include "systemf.h"			// runemf2
#include "dirs.h"			// save_dir2
#include "fortify.h"
#include "init.h"			// NullStr etc.
#include "excputil.h"			// xbeginthread
#include "copyf.h"			// ignorereadonly

// Data definitions

#pragma data_seg(GLOBAL1)
HWND LastDir;
HWND TreeCnrMenu;
INT driveserial[26];
BOOL fDCOpens;
BOOL fFollowTree;
BOOL fTopDir;
BOOL fLVMGui;
BOOL fDFSee;
BOOL fFDisk;
BOOL fMiniLVM;
BOOL fLVM;
BOOL fExpandAll;
HPOINTER hptrDunno;
HWND hwndMainMenu;

#pragma data_seg(GLOBAL2)
ULONG FM3UL;
INT TreesortFlags;

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;
static BOOL fOkayMinimize;
static HMQ hmqExpandTree;

APIRET16 APIENTRY16 Dos16MemAvail(PULONG pulAvailMem);

typedef struct APPNOTIFY
{
  HAPP happ;
  CHAR device;
  struct APPNOTIFY *next;
  struct APPNOTIFY *prev;
}
APPNOTIFY;

MRESULT EXPENTRY OpenButtonProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static BOOL emphasized = FALSE;

  switch (msg) {
  case WM_CREATE:
    {
      MRESULT rc;

      rc = PFNWPButton(hwnd, msg, mp1, mp2);
      //fixme to allow user to change presparams 1-10-09 GKY
      WinSetPresParam(hwnd, PP_FONTNAMESIZE,
		      strlen(FNT_8TIMESNEWROMAN) + 1,
		      (PVOID) FNT_8TIMESNEWROMAN);
      return rc;
    }

  case WM_MOUSEMOVE:
    BubbleHelp(hwnd, TRUE, FALSE, TRUE, GetPString(IDS_OPENBUTTONHELP));
    break;

  case WM_CONTEXTMENU:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT),
	    WM_COMMAND, MPFROM2SHORT(IDM_OPENWALK, 0), MPVOID);
    return 0;

  case DM_DRAGOVER:
    if (!emphasized) {
      emphasized = TRUE;
      EmphasizeButton(hwnd, emphasized);
    }
    if (AcceptOneDrop(hwnd, mp1, mp2))
      return MRFROM2SHORT(DOR_DROP, DO_MOVE);
    return MRFROM2SHORT(DOR_NEVERDROP, 0);

  case DM_DRAGLEAVE:
    if (emphasized) {
      emphasized = FALSE;
      EmphasizeButton(hwnd, emphasized);
    }
    break;

  case DM_DROPHELP:
    DropHelp(mp1, mp2, hwnd, GetPString(IDS_OPENDROPHELP));
    return 0;

  case DM_DROP:
    {
      char szFrom[CCHMAXPATH + 2];

      if (emphasized) {
	emphasized = FALSE;
	EmphasizeButton(hwnd, emphasized);
      }
      if (GetOneDrop(hwnd, mp1, mp2, szFrom, sizeof(szFrom))) {
	MakeValidDir(szFrom);
	WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
		   UM_OPENWINDOWFORME, MPFROMP(szFrom), MPVOID);
      }
    }
    return 0;

  }
  return PFNWPButton(hwnd, msg, mp1, mp2);
}

/**
 * Find a record in tree view, move it so it shows in container and
 * make it the current record
 * @param hwndCnr is container which must be in tree view
 * @param pszDir_ is full path name to find
 */

VOID ShowTreeRec(HWND hwndCnr,
		 PCSZ pszDir_,
		 BOOL collapsefirst,
		 BOOL maketop)
{
  PCNRITEM pci;
  PCNRITEM pciToSelect;
  PCNRITEM pciP;
  UINT retries;
  BOOL quickbail = FALSE;
  PSZ p;
  BOOL found;
  CHAR szDir[CCHMAXPATH];
  CHAR szDirArg[CCHMAXPATH];		// Copy of passed value
  CHAR chSaved;

  strcpy(szDirArg, pszDir_);	// Cache here in case arg content changed by some other thread
  
  // already positioned to requested record?
  pci = WinSendMsg(hwndCnr,
		   CM_QUERYRECORDEMPHASIS,
		   MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
  if (pci && (INT)pci != -1 && !stricmp(pci->pszFileName, szDirArg)) {
    quickbail = TRUE;			// Already at requested record - bypass repositioning
    goto MakeTop;
  }
  // 2015-08-13 SHL add retry logic 2015-08-22 GKY increase retries from 10 to 100 to
  // eliminate switch failures on deep or large tree state switches
  for (found = FALSE, retries = 0; !found && retries < 100; retries++) {

    pci = FindCnrRecord(hwndCnr,
			szDirArg,
			NULL,		// pciParent
			TRUE,		// partial
			FALSE,		// partmatch
			TRUE);		// noenv

    if (pci && (INT)pci != -1) {
      found = TRUE;
      break;			// Found it
    }

    // Walk down directory tree, expanding as needed
    strcpy(szDir, szDirArg);
    p = szDir + 3;			// Point after root backslash
    chSaved = *p;			// Remember for restore
    *p = 0;				// Chop after backslash

    for (;;) {
      // Try to match path prefix
      pciP = FindCnrRecord(hwndCnr,
			   szDir,
			   NULL,		// pciParent
			   TRUE,		// partial
			   FALSE,		// partmatch
			   TRUE);		// noenv
      if (!pciP || (INT)pciP == -1) {
	WaitFleshWorkListEmpty(szDirArg, 240);	// 2015-08-23 SHL
	break;					// No match
      }
      if (!stricmp(szDirArg, pciP->pszFileName)) {
	pci = pciP;
	found = TRUE;
	break;			// Got full match
      }

      // Got partial match

      if (~pciP->rc.flRecordAttr & CRA_EXPANDED) {
	WinSendMsg(hwndCnr, CM_EXPANDTREE, MPFROMP(pciP), MPVOID);
      }

      // Add next component to path unless no more components
      if (p) {
	*p = chSaved;			// Restore
	if (chSaved) {
	  if (chSaved == '\\')
	    p++;			// Get past last backslash
	  p = strchr(p, '\\');		// Find next backslash
	  if (p) {
	    chSaved = *p;
	    *p = 0;			// Truncate at backslash
	  }
	}
      }
      WaitFleshWorkListEmpty(NULL, 240);     // 2015-09-26 GKY Let Flesh thread catch up
    } // while expanding

  } // for
  if (found) {
    // Found it
    if (~pci->rc.flRecordAttr & CRA_CURSORED) {
      if (collapsefirst) {
        WaitFleshWorkListEmpty(NULL, 240);
	pciP = WinSendMsg(hwndCnr,
			  CM_QUERYRECORD,
			  MPVOID, MPFROM2SHORT(CMA_FIRST, CMA_ITEMORDER));
	while (pciP && (INT) pciP != -1) {
	  if (pciP->rc.flRecordAttr & CRA_EXPANDED) {
	    // collapse top level of all branches
	    WinSendMsg(hwndCnr, CM_COLLAPSETREE, MPFROMP(pciP), MPVOID);
	  }
	  pciP = WinSendMsg(hwndCnr,
			    CM_QUERYRECORD,
			    MPFROMP(pciP),
			    MPFROM2SHORT(CMA_NEXT, CMA_ITEMORDER));
	} // while
      } // if collapse

      // Expand parent branches
      // 2015-08-06 SHL FIXME to bypass if we did not collapse since search already expanded - maybe?
      pciToSelect = pci;
      for (;;) {
	pciP = WinSendMsg(hwndCnr,
			  CM_QUERYRECORD,
			  MPFROMP(pciToSelect),
			  MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER));
	if (!pciP || (INT)pciP == -1)
	  break;			// Done
	// Got parent
	if (~pciP->rc.flRecordAttr & CRA_EXPANDED)
	  WinSendMsg(hwndCnr, CM_EXPANDTREE, MPFROMP(pciP), MPVOID);
	pciToSelect = pciP;
	DosSleep(0);			// Let GUI update
      } // for
    } // if not cursored

  MakeTop:
    // make record visible
    pciToSelect = pci;
    if (pciToSelect && (INT) pciToSelect != -1) {
      if (fSwitchTreeExpand && ~pciToSelect->rc.flRecordAttr & CRA_EXPANDED) {
	WinSendMsg(hwndCnr, CM_EXPANDTREE, MPFROMP(pciToSelect), MPVOID);
      }
      if (maketop || fTopDir) {
        if (fCollapseFirst && !quickbail) {
          WaitFleshWorkListEmpty(NULL, 240); //Let the root expand first otherwise it makes top
        }
	ShowCnrRecord(hwndCnr, (PMINIRECORDCORE)pciToSelect);
      }

      if (!quickbail) {
	WaitFleshWorkListEmpty(szDirArg, 240);	// 2015-08-19 SHL try to ensure contents stable
	WinSendMsg(hwndCnr,
		   CM_SETRECORDEMPHASIS,
		   MPFROMP(pciToSelect),
		   MPFROM2SHORT(TRUE, CRA_SELECTED | CRA_CURSORED));
      }
    }
  }
}

MRESULT EXPENTRY TreeTitleWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg) {
  case WM_CONTEXTMENU:
    return WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
		      UM_CONTEXTMENU, mp1, mp2);
  }
  return oldproc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY TreeStatProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_CREATE:
    return CommonTextProc(hwnd, msg, mp1, mp2);

  case WM_CONTEXTMENU:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT), msg, mp1, mp2);
    return 0;

  case WM_PAINT:
    {
      MRESULT mr = PFNWPStatic(hwnd, msg, mp1, mp2);

      PaintRecessedWindow(hwnd, (HPS) 0, FALSE, FALSE);
      return mr;
    }

  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinQueryWindow(hwnd, QW_PARENT));
    return 0;
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY TreeFrameWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2)
{
  switch (msg) {
  case UM_RESCAN:
    PostMsg(WinQueryWindow(hwnd, QW_PARENT), msg, mp1, mp2);
    return 0;

  case WM_ADJUSTWINDOWPOS:
    {
      SWP *pswp;

      pswp = (SWP *) mp1;
      if (ParentIsDesktop(hwnd, (HWND) 0)) {
	if (pswp->fl & (SWP_HIDE | SWP_MINIMIZE))
	  HideNote();
      }
    }
    break;

  case WM_TRACKFRAME:
    if (!fFreeTree && !ParentIsDesktop(hwnd, (HWND) 0)) {
      switch (SHORT1FROMMP(mp1) & TF_MOVE) {
      case TF_MOVE:
      case TF_LEFT:
      case TF_TOP:
      case (TF_LEFT | TF_BOTTOM):
      case (TF_LEFT | TF_TOP):
	{
	  SWP swp;

	  WinQueryWindowPos(hwnd, &swp);
	  if (!(swp.fl & SWP_ACTIVATE))
	    WinSetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
			    SWP_ZORDER | SWP_ACTIVATE);
	}
	return 0;
      }
    }
    break;

  case WM_CALCFRAMERECT:
    if (*(ULONG *) realappname != FM3UL) {

      MRESULT mr;
      PRECTL prectl;

      mr = CommonFrameWndProc(TREE_CNR, hwnd, msg, mp1, mp2);

      /**
       * Calculate the position of the client rectangle.
       * Otherwise,  we'll see a lot of redraw when we move the
       * client during WM_FORMATFRAME.
       */

      if (mr && mp2) {
	prectl = (PRECTL) mp1;
	prectl->yTop -= 22;
      }
      return mr;
    }
    break;

  case WM_FORMATFRAME:
    {
      SHORT sCount;
      PSWP pswp, pswpClient, pswpNew;

      sCount = (SHORT) CommonFrameWndProc(TREE_CNR, hwnd, msg, mp1, mp2);

      // Reformat the frame to "squeeze" the client

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
      pswpNew->hwnd = WinWindowFromID(hwnd, MAIN_STATUS);
      if (*(ULONG *) realappname == FM3UL) {

	PSWP pswpTitlebar = (PSWP) 0, pswpMinbutton = (PSWP) 0;
	SHORT x;

	pswpNew->hwnd = WinWindowFromID(hwnd, IDM_OPENWINDOW);
	pswp = (PSWP) mp1;
	for (x = 0; x < sCount; x++) {
	  if (WinQueryWindowUShort(pswp->hwnd, QWS_ID) == FID_TITLEBAR)
	    pswpTitlebar = pswp;
	  else if (WinQueryWindowUShort(pswp->hwnd, QWS_ID) == FID_MINMAX)
	    pswpMinbutton = pswp;
	  if (pswpTitlebar && pswpMinbutton)
	    break;
	  pswp++;
	}
	pswpNew->cy = pswpMinbutton->cy + 3;
	pswpNew->cx = min(pswpNew->cy, (pswpMinbutton->cx / 2) + 3);
	pswpTitlebar->cx -= (pswpNew->cx + 1);
	pswpNew->x = pswpTitlebar->x + pswpTitlebar->cx;
	pswpNew->y = pswpMinbutton->y - 1;
      }
      else {
	pswpNew->x = pswpClient->x + 3;
	pswpNew->y = (pswpClient->y + pswpClient->cy) - 20;
	pswpNew->cx = pswpClient->cx - 6;
	pswpNew->cy = 18;
	pswpClient->cy -= 22;
      }
      sCount++;
      return MRFROMSHORT(sCount);
    }

  case WM_QUERYFRAMECTLCOUNT:
    {
      SHORT sCount;

      sCount = (SHORT) CommonFrameWndProc(TREE_CNR, hwnd, msg, mp1, mp2);
      sCount++;
      return MRFROMSHORT(sCount);
    }
  }
  return CommonFrameWndProc(TREE_CNR, hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY TreeClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2)
{
  switch (msg) {
  case UM_CONTAINERHWND:
    return MRFROMLONG(WinWindowFromID(hwnd, TREE_CNR));

  case UM_VIEWSMENU:
    return MRFROMLONG(CheckMenu(hwndMainMenu, &TreeCnrMenu, TREECNR_POPUP));

  case UM_TIMER:
  case UM_ACTION:
  case UM_SHOWME:
  case UM_OPENWINDOWFORME:
  case UM_MINIMIZE:
  case UM_MAXIMIZE:
  case WM_INITMENU:
  case UM_INITMENU:
  case UM_FILTER:
  case UM_FILESMENU:
  case UM_UPDATERECORD:
  case UM_UPDATERECORDLIST:
  case MM_PORTHOLEINIT:
  case UM_DRIVECMD:
  case WM_CLOSE:
  case WM_CONTROL:
  case UM_COMMAND:
  case WM_COMMAND:
    return WinSendMsg(WinWindowFromID(hwnd, TREE_CNR), msg, mp1, mp2);

  case WM_PSETFOCUS:
  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, TREE_CNR));
    break;

  case WM_ERASEBACKGROUND:
    WinFillRect((HPS) mp1, (PRECTL) mp2, 0x00d0d0d0);
    return 0;

  case WM_PAINT:
    {
      HPS hps;
      RECTL rcl;

      hps = WinBeginPaint(hwnd, (HPS) 0, NULL);
      if (hps) {
	WinQueryWindowRect(hwnd, &rcl);
	WinFillRect(hps, &rcl, CLR_PALEGRAY);
	PaintRecessedWindow(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					    MAIN_STATUS), hps, FALSE, FALSE);
	WinEndPaint(hps);
      }
    }
    break;

  case WM_SIZE:
    WinSetWindowPos(WinWindowFromID(hwnd, TREE_CNR),
		    HWND_TOP,
		    0,
		    0,
		    SHORT1FROMMP(mp2),
		    SHORT2FROMMP(mp2), SWP_SHOW | SWP_MOVE | SWP_SIZE);
    if (hwndMain)
      PostMsg(hwndMain, UM_SIZE, MPVOID, MPVOID);
    break;

  case WM_CONTEXTMENU:
  case UM_CONTEXTMENU:
    PostMsg(WinWindowFromID(hwnd, TREE_CNR),
	    WM_CONTROL, MPFROM2SHORT(TREE_CNR, CN_CONTEXTMENU), MPVOID);
    return 0;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

ULONG ulScanPostCnt;

MRESULT EXPENTRY TreeObjWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  DIRCNRDATA *dcd;

  switch (msg) {
  case UM_SHOWME:
    if (mp1) {
#     ifdef FORTIFY
      Fortify_BecomeOwner(mp1);
#     endif
      dcd = INSTDATA(hwnd);
      if (dcd) {

	/* Hold off if switching on focus change and
	   RestoreDirCnrState has restored one or directory directory containers
	   See RestoreDirCnrState()
	*/
	if (cDirectoriesRestored > 0)
	  cDirectoriesRestored--;

	if (!cDirectoriesRestored) {
	  BOOL tempsusp = dcd->suspendview;
	  BOOL tempfollow = fFollowTree;
	  dcd->suspendview = TRUE;
	  fFollowTree = FALSE;
          priority_idle(); // 2015-09-26 GKY Majority of work done by Flesh and UI threads
          ShowTreeRec(dcd->hwndCnr, (CHAR *)mp1, fCollapseFirst, TRUE);
          priority_normal();
	  PostMsg(hwndTree, WM_COMMAND, MPFROM2SHORT(IDM_UPDATE, 0), MPVOID);

	  dcd->suspendview = (USHORT)tempsusp;	// Restore
          fFollowTree = tempfollow;		// Restore
	}
      }
      free((CHAR *)mp1);
    }
    return 0;

  case UM_TOPDIR:
    if (mp1) {
      dcd = INSTDATA(hwnd);
      if (dcd) {
        PCNRITEM pci = (PCNRITEM) mp1;
        WaitFleshWorkListEmpty(pci->pszFileName, 240);
        ShowCnrRecord(dcd->hwndCnr, (PMINIRECORDCORE) pci);
      }
    }
    return 0;

  case DM_PRINTOBJECT:
    return MRFROMLONG(DRR_TARGET);

  case DM_DISCARDOBJECT:
    dcd = INSTDATA(hwnd);
    if (fFM2Deletes && dcd) {

      LISTINFO *li;
      CNRDRAGINFO cni;

      cni.pRecord = NULL;
      cni.pDragInfo = (PDRAGINFO) mp1;
      li = DoFileDrop(dcd->hwndCnr,
		      dcd->directory, FALSE, MPVOID, MPFROMP(&cni));
      CheckPmDrgLimit(cni.pDragInfo);
      if (li) {
	li->type = ((fDefaultDeletePerm) ? IDM_PERMDELETE : IDM_DELETE);
	if (!PostMsg(hwnd, UM_MASSACTION, MPFROMP(li), MPVOID))
	  FreeListInfo(li);
	else
	  return MRFROMLONG(DRR_SOURCE);
      }
    }
    return MRFROMLONG(DRR_TARGET);

  case UM_EXPAND:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!dcd)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else {
      BOOL tempsusp = dcd->suspendview;
      INT x = 0;
      BOOL fExpanding = TRUE;
      PCNRITEM pci = (PCNRITEM) mp2;

      if (!pci) {
        Runtime_Error(pszSrcFile, __LINE__, NULL);
        return 0;
      }
      dcd->suspendview = TRUE;
      priority_idle();
      if (SHORT1FROMMP(mp1) == IDM_EXPAND) {
        if (!(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_REMOVABLE))
          fExpandAll = TRUE;
        while (fExpanding) { // Not serialized not practical to wait on very large directories
          x++;
          if (!IsFleshWorkListEmpty()) {
            WaitFleshWorkListEmpty(NULL, fExpandAll ? 1 : 50); // Let it expand
          }
          fExpanding = ExpandAll(dcd->hwndCnr, x, pci);
          DosSleep(240);
        }
        fExpandAll = FALSE;
      }
      else
        CollapseAll(dcd->hwndCnr, pci);
      priority_normal();
      DosSleep(1); // Fixes tree epansion (dir text and icons all placed on
		       // the same line as the drive) failure on startup using RWS
      dcd->suspendview = (USHORT) tempsusp;
      PostMsg(dcd->hwndCnr, UM_FILTER, MPVOID, MPVOID);
    }
    return 0;

  case UM_UPDATERECORDLIST:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!dcd || !mp1)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else {
      INT numentries = 0;
      CHAR **list = (CHAR **) mp1;

      while (list[numentries])
	numentries++;
      if (numentries)
	UpdateCnrList(dcd->hwndCnr, list, numentries, TRUE, dcd);
    }
    return 0;

  case UM_SETUP2:
    {
      PCNRITEM pci = (PCNRITEM) mp1;

      if (pci) {
        if ((INT) mp2 == 21 && pci->rc.hptrIcon == hptrCDROM) {
          if (fEjectCDScan)
            PostMsg(hwndTree, WM_COMMAND, MPFROM2SHORT(IDM_RESCAN, 0), MPVOID);
          else {
            driveflags[toupper(*pci->pszFileName) - 'A'] |= DRIVE_INVALID;
            PostMsg(hwndTree, WM_COMMAND, MPFROM2SHORT(IDM_UPDATE, 0), MPVOID);
          }
        }
        NotifyError(pci->pszFileName, (ULONG) mp2);
      }
    }
    return 0;

  case UM_SETUP:
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!dcd)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else {
#     ifdef FORTIFY
      Fortify_BecomeOwner(dcd);
#     endif
      dcd->hwndObject = hwnd;
      if (ParentIsDesktop(hwnd, dcd->hwndParent))
	DosSleep(100); 
    }
    return 0;

  case UM_RESCAN2:
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!dcd)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    // Bypass if not running integrated (i.e if vtree)
    else if (hwndStatus &&
	     dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent)) {
      CHAR s[CCHMAXPATH * 2];
      PCNRITEM pci = (PCNRITEM) mp1;
      FSALLOCATE fsa;
      struct
      {
	ULONG serial;
	CHAR volumelength;
	CHAR volumelabel[CCHMAXPATH];
      }
      volser;
      CHAR tb[64];
      CHAR szFree[64];
      CNRINFO cnri;
      CHAR FileSystem[CCHMAXPATH * 2];
      CHAR szTmpLabel[CCHMAXPATH];
      ULONG type;

      strcpy(s, GetPString(IDS_TREETEXT));
      memset(&cnri, 0, sizeof(CNRINFO));
      cnri.cb = sizeof(CNRINFO);
      WinSendMsg(dcd->hwndCnr,
		 CM_QUERYCNRINFO,
		 MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
      if (cnri.cRecords) {
	sprintf(s, GetPString(IDS_NUMDRIVESTEXT), cnri.cRecords);
	if (pci && (INT) pci != -1) { 
	  if (!(driveflags[toupper(*pci->pszFileName) - 'A'] &
		DRIVE_REMOVABLE) ||
	      driveserial[toupper(*pci->pszFileName) - 'A'] != -1) {
	    memset(&volser, 0, sizeof(volser));
	    DosError(FERR_DISABLEHARDERR);
	    if (!DosQueryFSInfo(toupper(*pci->pszFileName) - '@',
				FSIL_VOLSER,
				&volser,
				(ULONG) sizeof(volser)) &&
		dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent)) {
	      DosError(FERR_DISABLEHARDERR);
	      if (!DosQueryFSInfo(toupper(*pci->pszFileName) - '@',
				  FSIL_ALLOC, &fsa, sizeof(FSALLOCATE))) {
		CommaFmtULL(tb, sizeof(tb),
			    (ULONGLONG) fsa.cUnitAvail * (fsa.cSectorUnit *
							  fsa.cbSector), 'M');
		sprintf(szFree, "  %s %s", tb, GetPString(IDS_FREETEXT));
	      }
	      else
		*szFree = 0;
	      //Show information on status line not shown in the tree container
	      driveserial[toupper(*pci->pszFileName) - 'A'] = volser.serial;
	      if (CheckDrive(toupper(*pci->pszFileName), FileSystem, &type) == -1 ||
		  fShowFSTypeInTree)
		strcpy(FileSystem, NullStr);
	      if (fShowDriveLabelInTree)
		strcpy(szTmpLabel, NullStr);
	      else
		strcpy(szTmpLabel, volser.volumelabel);
	      if (fSplitStatus) {
		CHAR temp[CCHMAXPATH] = " [";

		strcat(temp, s);
		strcat(temp, "]");
		sprintf(s,
			GetPString(fShowFSTypeInTree ? IDS_TREESTATUSSTART1TEXT :
				   fShowDriveLabelInTree ? IDS_TREESTATUSSTART2TEXT :
				   IDS_TREESTATUSSTARTTEXT), toupper(*pci->pszFileName),
			FileSystem, szTmpLabel, volser.serial, szFree);
		strcat(s, temp);
	      }
	      else {
		strcat(s, " [");
		sprintf(&s[strlen(s)],
			GetPString(fShowFSTypeInTree ? IDS_TREESTATUSSTART1TEXT :
				   fShowDriveLabelInTree ? IDS_TREESTATUSSTART2TEXT :
				   IDS_TREESTATUSSTARTTEXT), toupper(*pci->pszFileName),
			FileSystem, szTmpLabel, volser.serial, szFree);
		strcat(s, "]");
	      }
	      if (!fMoreButtons) {
		if (*dcd->mask.szMask ||
		    (dcd->mask.attrFile != ALLATTRS ||
		     ((fFilesInTree ||
		       (driveflags[toupper(*pci->pszFileName)] &
			DRIVE_INCLUDEFILES)) ?
		      dcd->mask.antiattr :
		      (dcd->mask.antiattr &&
		       dcd->mask.antiattr != FILE_DIRECTORY)))) {
		  sprintf(&s[strlen(s)],
			  " (%s)",
			  (*dcd->mask.szMask) ?
			  dcd->mask.szMask : GetPString(IDS_ATTRTEXT));
		}
	      }
	    }
	  }
          else {
	    // find root record and strip it if needed
            pci = FindParentRecord(dcd->hwndCnr, pci);
            driveserial[toupper(*pci->pszFileName) - 'A'] = -1;
            if (pci->fleshed) {
              WaitFleshWorkListEmpty(pci->pszFileName, 240);	// 2015-08-19 SHL in case pci still in work list
              if ((toupper(*pci->pszFileName) - 'A') > 1)  {
                AddFleshWorkRequest(hwnd, pci, eUnFlesh);
              }
              else
                UnFlesh(hwnd, pci);
            }
	  }
	}
      }
      // 21 Sep 09 SHL FIXME to know why checking again - focus change?
      if (dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent))
	WinSetWindowText(hwndStatus, s);
    }
    return 0;

  case UM_RESCAN:
    // Populate container
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!dcd)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else {
      RemoveCnrItems(dcd->hwndCnr, NULL, 0, CMA_FREE | CMA_INVALIDATE | CMA_ERASE);
      WinSendMsg(dcd->hwndCnr,
		 CM_SCROLLWINDOW, MPFROMSHORT(CMA_VERTICAL), MPFROMLONG(-1));
      WinSendMsg(dcd->hwndCnr,
		 CM_SCROLLWINDOW,
		 MPFROMSHORT(CMA_HORIZONTAL), MPFROMLONG(-1));
      FillTreeCnr(dcd->hwndCnr, dcd->hwndParent);
      if (fOkayMinimize) {
	PostMsg(dcd->hwndCnr, UM_MINIMIZE, MPVOID, MPVOID);
	fOkayMinimize = FALSE;
      }
      WinSendMsg(dcd->hwndCnr,
		 CM_INVALIDATERECORD,
		 MPVOID, MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION));
      PostMsg(dcd->hwndCnr, UM_RESCAN, MPVOID, MPVOID);
    }
    return 0;

  case UM_COMMAND:
    if (mp1) {

      LISTINFO *li = (LISTINFO *) mp1;

      switch (li->type) {
      case IDM_DOITYOURSELF:
      case IDM_APPENDTOCLIP:
      case IDM_SAVETOCLIP:
      case IDM_ARCHIVE:
      case IDM_VIEW:
      case IDM_EDIT:
      case IDM_OBJECT:
      case IDM_SHADOW:
      case IDM_SHADOW2:
      case IDM_JAVAEXE:
      case IDM_PRINT:
      case IDM_ATTRS:
      case IDM_DELETE:
      case IDM_PERMDELETE:
	if (li->type == IDM_DELETE)
	  ignorereadonly = FALSE;
	if (PostMsg(hwnd, UM_MASSACTION, mp1, mp2))
	  return (MRESULT) TRUE;
      default:
	if (PostMsg(hwnd, UM_ACTION, mp1, mp2))
	  return (MRESULT) TRUE;
      }
    }
    return 0;

  case UM_MASSACTION:
    if (mp1) {

      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (!dcd)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	WORKER *wk;
#	ifdef FORTIFY
	Fortify_EnterScope();
#	endif
	wk = xmallocz(sizeof(WORKER), pszSrcFile, __LINE__);
	if (!wk)
	  FreeListInfo((LISTINFO *) mp1);
	else {
	  wk->size = sizeof(WORKER);
	  wk->hwndCnr = dcd->hwndCnr;
	  wk->hwndParent = dcd->hwndParent;
	  wk->hwndFrame = dcd->hwndFrame;
	  wk->hwndClient = dcd->hwndClient;
	  wk->li = (LISTINFO *) mp1;
	  strcpy(wk->directory, dcd->directory);
	  if (xbeginthread(MassAction,
			   122880,
			   wk,
			   pszSrcFile,
			   __LINE__) == -1)
	  {
	    free(wk);
	    FreeListInfo((LISTINFO *) mp1);
	  }
	}
#	ifdef FORTIFY
	Fortify_LeaveScope();
#	endif
      }
    }
    return 0;

  case UM_ACTION:
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif
    if (mp1) {
#     ifdef FORTIFY
      Fortify_BecomeOwner(mp1);
#     endif
      dcd = WinQueryWindowPtr(hwnd, QWL_USER);
      if (!dcd)
	Runtime_Error(pszSrcFile, __LINE__, NULL);
      else {
	WORKER *wk;
	wk = xmallocz(sizeof(WORKER), pszSrcFile, __LINE__);
	if (!wk)
	  FreeListInfo((LISTINFO *) mp1);
	else {
	  wk->size = sizeof(WORKER);
	  wk->hwndCnr = dcd->hwndCnr;
	  wk->hwndParent = dcd->hwndParent;
	  wk->hwndFrame = dcd->hwndFrame;
	  wk->hwndClient = dcd->hwndClient;
	  wk->li = (LISTINFO *) mp1;
	  strcpy(wk->directory, dcd->directory);
	  if (xbeginthread(Action,
			   122880,
			   wk,
			   pszSrcFile,
			   __LINE__) == -1)
	  {
	    free(wk);
	    FreeListInfo((LISTINFO *) mp1);
	  }
	}
      }
    }
#   ifdef FORTIFY
    Fortify_LeaveScope();
#   endif
    return 0;

  case WM_CLOSE:
    WinDestroyWindow(hwnd);
    break;

  case WM_DESTROY:
    hwndTree = (HWND) 0;
    dcd = WinQueryWindowPtr(hwnd, QWL_USER);
    if (dcd) {
      WinSendMsg(dcd->hwndCnr,
		 UM_CLOSE, MPFROMLONG(dcd->dontclose != FALSE), MPVOID);
      WinSetWindowPtr(dcd->hwndCnr, QWL_USER, NULL);	// 13 Apr 10 SHL Set NULL before freeing dcd
      free(dcd);
#     ifdef FORTIFY
      Fortify_LeaveScope();
#     endif
    }
    xDosPostEventSem(CompactSem);
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    break;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY TreeCnrWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static APPNOTIFY *apphead = NULL, *apptail = NULL;
  DIRCNRDATA *dcd = INSTDATA(hwnd);
  PCNRITEM pci;
  APIRET rc;

  switch (msg) {
  case DM_PRINTOBJECT:
    return MRFROMLONG(DRR_TARGET);

  case DM_DISCARDOBJECT:
    if (dcd)
      return WinSendMsg(dcd->hwndObject, msg, mp1, mp2);
    else
      return MRFROMLONG(DRR_TARGET);

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (SHORT1FROMMP(mp1) & KC_KEYUP)
      return (MRESULT) TRUE;
    if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY) {
      switch (SHORT2FROMMP(mp2)) {
      case VK_INSERT:
	if ((shiftstate & KC_CTRL) == KC_CTRL)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_MKDIR, 0), MPVOID);
	break;
      case VK_DELETE:
	if ((shiftstate & KC_CTRL) == KC_CTRL)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_PERMDELETE, 0), MPVOID);
	else if ((shiftstate & KC_SHIFT) == KC_SHIFT)
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_SAVETOCLIP, 0), MPVOID);
	else
	  PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_DELETE, 0), MPVOID);
	break;
      } // switch
    }

    if (SearchContainer(hwnd, msg, mp1, mp2))
      return (MRESULT)TRUE;		// Avoid default handler

    break;				// Let default handler see key

  case WM_MOUSEMOVE:
  case WM_BUTTON1UP:
  case WM_BUTTON2UP:
  case WM_BUTTON3UP:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case UM_TIMER:
    if (dcd && dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent) &&
	hwndStatus2) {
      FILEFINDBUF3L ffb;
      ULONG nm = 1;
      HDIR hdir = HDIR_CREATE;

      if (*SwapperDat) {
	if (!xDosFindFirst(SwapperDat,
			   &hdir,
			   FILE_NORMAL | FILE_HIDDEN |
			   FILE_SYSTEM | FILE_ARCHIVED | FILE_READONLY,
			   &ffb, sizeof(ffb), &nm, FIL_STANDARDL)) {
	  CHAR tb[39], tm[39], tpm[39], s[163];
	  ULONG amem;

	  priority_bumped();
	  DosFindClose(hdir);
	  if (!DosQuerySysInfo(QSV_TOTAVAILMEM,
			       QSV_TOTAVAILMEM,
			       (PVOID) & amem, sizeof(amem))) {
	    CommaFmtULL(tpm, sizeof(tpm), amem, 'M');
	  }
	  else
	    *tpm = 0;
	  if (!Dos16MemAvail(&amem))
	    CommaFmtULL(tm, sizeof(tm), amem, 'M');
	  else
	    *tm = 0;
	  CommaFmtULL(tb, sizeof(tb), ffb.cbFile, 'M');
	  sprintf(s, " %s %s%s%s%s%s",
		  GetPString(IDS_SWAPFILETEXT),
		  tb,
		  *tm ? GetPString(IDS_TREEMEMTEXT) : NullStr,
		  tm, *tpm ? "/" : NullStr, tpm);
	  WinSetWindowText(hwndStatus2, s);
	}
	else
	  WinSetWindowText(hwndStatus2, NullStr);
      }
      else
	WinSetWindowText(hwndStatus2, NullStr);
    }
    // 13 Jul 09 SHL FIXME to make sense
    if (msg == UM_TIMER)
      return 0;
    break;

  case WM_PRESPARAMCHANGED:
    PresParamChanged(hwnd, PCSZ_TREECNR, mp1, mp2);
    break;

  case UM_FILESMENU:
    {
      HWND menuHwnd = (HWND) 0;
      FSALLOCATE fsa;

      pci = (PCNRITEM)CurrentRecord(hwnd);
      if (pci && (INT)pci != -1) {
	if (IsRoot(pci->pszFileName) || !DosQueryFSInfo(toupper(*pci->pszFileName) - '@',
						       FSIL_ALLOC, &fsa,
						       sizeof(FSALLOCATE)))
	  menuHwnd = CheckMenu(hwndMainMenu, &TreeMenu, TREE_POPUP);
	else {
	  menuHwnd = CheckMenu(hwndMainMenu, &DirMenu, DIR_POPUP);
	}
	if (!(pci->attrFile & FILE_DIRECTORY))
	  menuHwnd = CheckMenu(hwndMainMenu, &FileMenu, FILE_POPUP);
      }
      return MRFROMLONG(menuHwnd);
    }

  case UM_COMPARE:
    if (dcd && mp1 && mp2) {

      COMPARE *cmp;
      CHAR *leftdir = (CHAR *)mp1, *rightdir = (CHAR *)mp2;

      if (!IsFile(leftdir) && !IsFile(rightdir)) {
	cmp = xmallocz(sizeof(COMPARE), pszSrcFile, __LINE__);
	if (cmp) {
	  cmp->size = sizeof(COMPARE);
	  strcpy(cmp->leftdir, leftdir);
	  strcpy(cmp->rightdir, rightdir);
	  cmp->hwndParent = dcd->hwndParent;
	  cmp->dcd.hwndParent = dcd->hwndParent;
	  WinDlgBox(HWND_DESKTOP,
		    HWND_DESKTOP,
		    CompareDlgProc, FM3ModHandle, COMP_FRAME, MPFROMP(cmp));
	}
      }
    }
    return 0;

  case UM_UPDATERECORDLIST:
    if (dcd && mp1)
      WinSendMsg(dcd->hwndObject, msg, mp1, mp2);
    return 0;

  case UM_UPDATERECORD:
    if (dcd && mp1) {
      CHAR *filename;
      filename = mp1;
      if (filename) {
	UpdateCnrRecord(hwnd, filename, TRUE, dcd);
      }
    }
    return 0;

  case WM_SETFOCUS:
    if (dcd && hwndStatus && mp2) {
      WinSendMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      if (hwndMain)
	PostMsg(hwndMain, UM_ADVISEFOCUS, MPFROMLONG(dcd->hwndFrame), MPVOID);
    }
    break;

  case UM_RESCAN:

    if (dcd && dcd->hwndFrame == WinQueryActiveWindow(dcd->hwndParent)) {
      // put name of our window on status line

      PCNRITEM pci = NULL;
      CHAR str[CCHMAXPATH + 6];

      if (fAutoView && hwndMain) {
	pci = WinSendMsg(hwnd,
			 CM_QUERYRECORDEMPHASIS,
			 MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	if (pci && (INT) pci != -1 && fComments &&
	    !(driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_SLOW))
	  WinSendMsg(hwndMain, UM_LOADFILE, MPFROMP(pci->pszFileName), MPVOID);
	else
	  WinSendMsg(hwndMain, UM_LOADFILE, MPVOID, MPVOID);
      }
      if (!fAutoView || !hwndMain)
	pci = (PCNRITEM) WinSendMsg(hwnd,
				    CM_QUERYRECORDEMPHASIS,
				    MPFROMLONG(CMA_FIRST),
				    MPFROMSHORT(CRA_CURSORED));
      if ((INT) pci == -1)
	pci = NULL;
      if (pci) {
	if (*(ULONG *) realappname == FM3UL) {
	  sprintf(str, "%s %s", GetPString(IDS_DTTEXT), pci->pszFileName);
	  WinSetWindowText(dcd->hwndFrame, str);
	  WinSetWindowText(WinWindowFromID(dcd->hwndFrame, FID_TITLEBAR),
			   str);
	}
	else
	  WinSetWindowText(WinWindowFromID(dcd->hwndFrame,
					   MAIN_STATUS), pci->pszFileName);
	if (fMoreButtons && hwndName) {
	  CHAR szDate[DATE_BUF_BYTES];

	  DateFormat(szDate, pci->date);
	  WinSetWindowText(hwndName, pci->pszFileName);
	  sprintf(str, "%s %02u%s%02u%s%02u", szDate,
		  pci->time.hours, TimeSeparator,
		  pci->time.minutes, TimeSeparator, pci->time.seconds);
	  WinSetWindowText(hwndDate, str);
	  WinSetWindowText(hwndAttr, pci->pszDispAttr);
	}
      }
      PostMsg(dcd->hwndObject, UM_RESCAN2, MPFROMP(pci), MPVOID);
      if (hwndStatus2)
	PostMsg(hwnd, UM_TIMER, MPVOID, MPVOID);
    }
    return 0;

  case UM_SETUP:
#   ifdef FORTIFY
    // Balance WM_DESTROY
    Fortify_EnterScope();
#   endif

    if (!dcd) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      return 0;
    }
    else {
      if (!dcd->hwndObject) {
	// first time through -- set things up
	CNRINFO cnri;

#	ifdef FORTIFY
	Fortify_EnterScope();
#	endif

	RestorePresParams(hwnd, PCSZ_TREECNR);
	memset(&cnri, 0, sizeof(CNRINFO));
	cnri.cb = sizeof(CNRINFO);
	WinSendMsg(hwnd,
		   CM_QUERYCNRINFO,
		   MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
	cnri.cyLineSpacing = 0;
	cnri.cxTreeIndent = 12;
	cnri.pSortRecord = (PVOID) SortTreeCnr;
	cnri.flWindowAttr &= (~(CV_NAME | CV_DETAIL | CV_TEXT));
	cnri.flWindowAttr |= (CV_TREE | CA_TREELINE | CV_ICON | CV_MINI);
	{
	  ULONG size = sizeof(ULONG);

	  PrfQueryProfileData(fmprof,
			      appname,
			      "TreeflWindowAttr",
			      (PVOID) & cnri.flWindowAttr, &size);
	  size = sizeof(MASK);
	  *dcd->mask.prompt = 0;
	  if (!*dcd->mask.szMask && !dcd->mask.attrFile) {
	    if (PrfQueryProfileSize(fmprof,
				    appname, "TreeFilter", &size) && size) {
	      PrfQueryProfileData(fmprof,
				  appname, "TreeFilter", &dcd->mask, &size);
	      SetMask(NULL, &dcd->mask);
	    }
	    else
	      dcd->mask.attrFile = (FILE_READONLY | FILE_NORMAL |
				    FILE_ARCHIVED | FILE_DIRECTORY |
				    FILE_HIDDEN | FILE_SYSTEM);
	  }
	  dcd->mask.attrFile |= FILE_DIRECTORY;
	}
	cnri.flWindowAttr &= (~(CA_MIXEDTARGETEMPH | CA_ORDEREDTARGETEMPH));
	cnri.flWindowAttr |= CV_FLOW;
	dcd->flWindowAttr = cnri.flWindowAttr;
	WinSendMsg(hwnd,
		   CM_SETCNRINFO,
		   MPFROMP(&cnri),
		   MPFROMLONG(CMA_FLWINDOWATTR | CMA_LINESPACING |
			      CMA_CXTREEINDENT | CMA_PSORTRECORD));
	if (xbeginthread(MakeObjWin,
			 327680,
			 dcd,
			 pszSrcFile,
			 __LINE__) == -1)
	{
	  PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
	}
	else
	  DosSleep(1);
#	ifdef FORTIFY
	Fortify_LeaveScope();
#	endif
      }
    }
    return 0;

  case WM_BUTTON3CLICK:
  case WM_CHORD:
    {
      PCNRITEM pci = NULL;
      QUERYRECFROMRECT pqr;
      NOTIFYRECORDENTER nr;
      BOOL tbool = fDCOpens;
      RECTL rectl;
      POINTL ptl;

      shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
      if (msg == WM_CHORD) {
	if (!WinQueryPointerPos(HWND_DESKTOP, &ptl))
	  break;
	WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1);
      }
      else {
	ptl.x = SHORT1FROMMP(mp1);
	ptl.y = SHORT2FROMMP(mp1);
      }
      memset(&rectl, 0, sizeof(rectl));
      memset(&pqr, 0, sizeof(pqr));
      pqr.cb = sizeof(pqr);
      pqr.rect.xLeft = ptl.x - 1;
      pqr.rect.xRight = ptl.x + 1;
      pqr.rect.yTop = ptl.y + 1;
      pqr.rect.yBottom = ptl.y - 1;
      pqr.fsSearch = CMA_PARTIAL;
      pci = (PCNRITEM) WinSendMsg(hwnd,
				  CM_QUERYRECORDFROMRECT,
				  MPFROMLONG(CMA_FIRST), MPFROMP(&pqr));
      if (!pci || (INT) pci == -1)
	break; //Probable B3 click on white space
      else {
	memset(&nr, 0, sizeof(nr));
	nr.hwndCnr = hwnd;
	nr.pRecord = (PRECORDCORE) pci;
	fDCOpens = TRUE;
	WinSendMsg(hwnd,
		   WM_CONTROL,
		   MPFROM2SHORT(WinQueryWindowUShort(hwnd,
						     QWS_ID),
				CN_ENTER), MPFROMP(&nr));
	PostMsg(hwnd, UM_RESTOREDC, MPFROMLONG(tbool), MPVOID);
      }
    }
    break;

  case UM_RESTOREDC:
    fDCOpens = (BOOL) mp1;
    return 0;

  case WM_CONTROL:
    DosError(FERR_DISABLEHARDERR);
    if (dcd) {
      switch (SHORT2FROMMP(mp1)) {
      case CN_BEGINEDIT:
      case CN_REALLOCPSZ:
      case CN_ENDEDIT:
	{
	  MRESULT mre;

	  mre = CnrDirectEdit(hwnd, msg, mp1, mp2);
	  if (mre != (MRESULT) - 1)
	    return mre;
	}
	break;

      case CN_DRAGLEAVE:
	if (mp2) {

	  PDRAGINFO pDInfo;

	  // fixme to know why - seems superfluous
	  pDInfo = ((PCNRDRAGINFO) mp2)->pDragInfo;
	  DrgAccessDraginfo(pDInfo);
	  DrgFreeDraginfo(pDInfo);
	}
	return 0;

      case CN_DROPHELP:
	if (mp2) {

	  PDRAGINFO pDInfo;
	  PCNRITEM pci;
	  ULONG numitems;
	  USHORT usOperation;

	  pci = (PCNRITEM) ((PCNRDRAGINFO) mp2)->pRecord;
	  pDInfo = (PDRAGINFO) ((PCNRDRAGINFO) mp2)->pDragInfo;
	  if (!DrgAccessDraginfo(pDInfo)) {
	    Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      GetPString(IDS_DROPERRORTEXT));
	  }
	  else {
	    numitems = DrgQueryDragitemCount(pDInfo);
	    usOperation = pDInfo->usOperation;
	    if (usOperation == DO_DEFAULT)
	      usOperation = fCopyDefault ? DO_COPY : DO_MOVE;
	    FreeDragInfoData(hwnd, pDInfo);
	    saymsg(MB_ENTER | MB_ICONASTERISK,
		   hwnd,
		   GetPString(IDS_DROPHELPHDRTEXT),
		   GetPString(IDS_DROPHELPTEXT),
		   numitems,
		   &"s"[numitems == 1L],
		   pci ? NullStr : GetPString(IDS_NOTEXT),
		   pci ? NullStr : " ",
		   pci ? pci->pszFileName : NullStr,
		   pci ? " " : NullStr,
		   GetPString((usOperation == DO_MOVE) ?
			      IDS_MOVETEXT :
			      (usOperation == DO_LINK) ?
			      IDS_LINKTEXT : IDS_COPYTEXT));
	  }
	}
	return 0;

      case CN_DRAGAFTER:
      case CN_DRAGOVER:
	if (mp2) {

	  PDRAGITEM pDItem;
	  PDRAGINFO pDInfo;
	  PCNRITEM pci;
	  USHORT uso;

	  pDInfo = ((PCNRDRAGINFO) mp2)->pDragInfo;
	  if (!DrgAccessDraginfo(pDInfo)) {
	    Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
		      PCSZ_DRGACCESSDRAGINFO);
	    return (MRFROM2SHORT(DOR_NODROP, 0));	// Drop not valid
	  }
	  pci = (PCNRITEM) ((PCNRDRAGINFO) mp2)->pRecord;
	  if ((INT) pci == -1)
	    pci = NULL;
	  if (pci && (pci->flags & (RECFLAGS_ENV | RECFLAGS_NODROP))) {
	    DrgFreeDraginfo(pDInfo);
	    return MRFROM2SHORT(DOR_NODROP, 0);
	  }
	  if (!WinIsWindowEnabled(dcd->hwndFrame)) {
	    DrgFreeDraginfo(pDInfo);
	    return MRFROM2SHORT(DOR_NODROP, 0);
	  }
	  if (pci) {
	    uso = pDInfo->usOperation;
	    if (uso == DO_DEFAULT)
	      uso = (fCopyDefault) ? DO_COPY : DO_MOVE;
	    if (!(pci->attrFile & FILE_DIRECTORY)) {
	      if (uso != DO_LINK && uso != DO_COPY && uso != DO_MOVE) {
		DrgFreeDraginfo(pDInfo);
		return (MRFROM2SHORT(DOR_NODROP, 0));
	      }
	      if (uso != DO_LINK &&
		  !(driveflags[toupper(*pci->pszFileName) - 'A'] &
		    DRIVE_NOTWRITEABLE)) {

		ARC_TYPE *info;

		if (!fQuickArcFind &&
		    !(driveflags[toupper(*pci->pszFileName) - 'A'] &
		      DRIVE_SLOW))
		  info = find_type(pci->pszFileName, NULL);
		else
		  info = quick_find_type(pci->pszFileName, NULL);
		if (!info || ((uso == DO_MOVE && !info->move) ||
			      (uso == DO_COPY && !info->create))) {
		  DrgFreeDraginfo(pDInfo);
		  return (MRFROM2SHORT(DOR_NODROP, 0));
		}
	      }
	    }
	  }
	  pDItem = DrgQueryDragitemPtr(pDInfo,		// Access DRAGITEM
				       0);		// Index to DRAGITEM
	  if (DrgVerifyRMF(pDItem,			// Check valid rendering
			   (CHAR *) DRM_OS2FILE,	// mechanisms and data
			   NULL) || DrgVerifyRMF(pDItem,
						 (CHAR *) DRM_FM2ARCMEMBER,
						 (CHAR *) DRF_FM2ARCHIVE)) {	// formats
	    DrgFreeDraginfo(pDInfo);			// Free DRAGINFO
	    if (!pci || (INT) pci == -1)
	      return MRFROM2SHORT(DOR_DROP, DO_MOVE);
	    if (driveflags[toupper(*pci->pszFileName) - 'A'] &
		DRIVE_NOTWRITEABLE)
	      return MRFROM2SHORT(DOR_DROP, DO_LINK);
	    if (toupper(*pci->pszFileName) < 'C')
	      return MRFROM2SHORT(DOR_DROP, DO_COPY);
	    return MRFROM2SHORT(DOR_DROP,		// Return okay to drop
				((fCopyDefault) ? DO_COPY : DO_MOVE));
	  }
	  DrgFreeDraginfo(pDInfo);			// Free DRAGINFO
	}
	return MRFROM2SHORT(DOR_NODROP, 0);		// Drop not valid

      case CN_INITDRAG:
	{
	  PCNRDRAGINIT pcd = (PCNRDRAGINIT) mp2;
	  PCNRITEM pci;

	  if (!pcd) {
	    Runtime_Error(pszSrcFile, __LINE__, NULL);
	    break;
	  }
	  else {
	    pci = (PCNRITEM) pcd->pRecord;
	    if (!pci || (INT) pci == -1) {
	      Runtime_Error(pszSrcFile, __LINE__, NULL);
	      break;
	    }
	    if (pci->flags & (RECFLAGS_ENV | RECFLAGS_NODRAG)) {
	      Runtime_Error(pszSrcFile, __LINE__, "drag not allowed");
	      break;
	    }
	    if (hwndStatus2) {
	      WinSetWindowText(hwndStatus2, (IsRoot(pci->pszFileName)) ?
			       (CHAR *) GetPString(IDS_DRAGROOTTEXT) :
			       (pci->attrFile & FILE_DIRECTORY) ?
			       (CHAR *) GetPString(IDS_DRAGDIRTEXT) :
			       (CHAR *) GetPString(IDS_DRAGFILETEXT));
	    }
	    DoFileDrag(hwnd, dcd->hwndObject, mp2, NULL, NULL, TRUE);
	    if (hwndStatus2) {
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	  }
	}
	return 0;

      case CN_DROP:
	{
	  LISTINFO *li;
	  ULONG action = UM_ACTION;

	  li = DoFileDrop(hwnd, NULL, TRUE, mp1, mp2);
	  CheckPmDrgLimit(((PCNRDRAGINFO)mp2)->pDragInfo);
	  if (li) {
	    if (!*li->targetpath) {
	      if (li->list[0])
		PMMkDir(dcd->hwndParent, li->list[0], FALSE);
	      FreeListInfo(li);
	      return 0;
	    }
	    if (li->list && li->list[0] && IsRoot(li->list[0]))
	      li->type = DO_LINK;
	    else if (fDragndropDlg && (!*li->arcname || !li->info)) {

	      CHECKLIST cl;

	      memset(&cl, 0, sizeof(cl));
	      cl.size = sizeof(cl);
	      cl.flags = li->type;
	      cl.list = li->list;
	      cl.cmd = li->type;
	      cl.prompt = li->targetpath;
	      li->type = WinDlgBox(HWND_DESKTOP,
				   dcd->hwndParent,
				   DropListProc,
				   FM3ModHandle, DND_FRAME, MPFROMP(&cl));
	      if (li->type == DID_ERROR)
		  Win_Error(hwnd, HWND_DESKTOP, pszSrcFile, __LINE__,
			    GetPString(IDS_DRAGDROPDIALOGTEXT));
	      if (!li->type) {
		FreeListInfo(li);
		return 0;
	      }
	      li->list = cl.list;
	      if (!li->list || !li->list[0]) {
		FreeListInfo(li);
		return 0;
	      }
	    }
	    switch (li->type) {
	    case DO_LINK:
	      if (fLinkSetsIcon) {
		li->type = IDM_SETICON;
		action = UM_MASSACTION;
	      }
	      else
		li->type = IDM_COMPARE;
	      break;
	    case DND_EXTRACT:
	      if (*li->targetpath && !IsFile(li->targetpath))
		li->type = IDM_EXTRACT;
	      break;
	    case DND_MOVE:
	      li->type = IDM_MOVE;
	      if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = IDM_ARCHIVEM;
	      }
	      break;
	    case DND_WILDMOVE:
	      li->type = IDM_WILDMOVE;
	      if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = IDM_ARCHIVEM;
	      }
	      break;
	    case DND_OBJECT:
	      li->type = IDM_OBJECT;
	      action = UM_MASSACTION;
	      break;
	    case DND_SHADOW:
	      li->type = IDM_SHADOW;
	      action = UM_MASSACTION;
	      break;
	    case DND_COMPARE:
	      li->type = IDM_COMPARE;
	      break;
	    case DND_SETICON:
	      action = UM_MASSACTION;
	      li->type = IDM_SETICON;
	      break;
	    case DND_COPY:
	      li->type = IDM_COPY;
	      if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = IDM_ARCHIVE;
	      }
	      break;
	    case DND_WILDCOPY:
	      li->type = IDM_WILDCOPY;
	      if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = IDM_ARCHIVE;
	      }
	      break;
	    default:
	      if (*li->arcname && li->info) {
		action = UM_MASSACTION;
		li->type = (li->type == DO_MOVE) ?
		  IDM_FAKEEXTRACTM : IDM_FAKEEXTRACT;
	      }
	      else if (*li->targetpath && IsFile(li->targetpath) == 1) {
		action = UM_MASSACTION;
		li->type = (li->type == DO_MOVE) ? IDM_ARCHIVEM : IDM_ARCHIVE;
	      }
	      else
		li->type = (li->type == DO_MOVE) ? IDM_MOVE : IDM_COPY;
	      break;
	    }
	    if (!li->list || !li->list[0])
	      FreeListInfo(li);
	    else if (!PostMsg(dcd->hwndObject, action, MPFROMP(li), MPVOID))
	      FreeListInfo(li);
	    else {

	      USHORT usop = 0;

	      switch (li->type) {
	      case IDM_COPY:
	      case IDM_WILDCOPY:
		usop = DO_COPY;
		break;
	      case IDM_MOVE:
	      case IDM_WILDMOVE:
	      case IDM_ARCHIVEM:
		usop = DO_MOVE;
		break;
	      }
	      if (usop)
		return MRFROM2SHORT(DOR_DROP, usop);
	    }
	  }
	}
	return 0;

      case CN_EMPHASIS:
        {
          PNOTIFYRECORDEMPHASIS pre = mp2;

          if (pre->fEmphasisMask & CRA_SELECTED) {
            if (pre->pRecord->flRecordAttr & CRA_SELECTED) {
              if (((PCNRITEM) (pre->pRecord))->attrFile & FILE_DIRECTORY) {
                PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
                if (fFollowTree &&
                    !(driveflags
                      [toupper(*((PCNRITEM) pre->pRecord)->pszFileName) -
                       'A'] & DRIVE_INVALID)) {
                  if (!LastDir && !ParentIsDesktop(hwnd, dcd->hwndParent))
                    LastDir = FindDirCnr(dcd->hwndParent);
                  if (LastDir) {
  
                    NOTIFYRECORDENTER pri;
                    BOOL tbool = fDCOpens;
  
                    fDCOpens = FALSE;
                    memset(&pri, 0, sizeof(pri));
                    pri.hwndCnr = hwnd;
                    pri.fKey = FALSE;
                    pri.pRecord = pre->pRecord;
                    WinSendMsg(hwnd,
                               WM_CONTROL,
                               MPFROM2SHORT(SHORT1FROMMP(mp1),
                                            CN_ENTER), MPFROMP(&pri));
                    fDCOpens = tbool;
                  }
                }
                if (*(ULONG *) realappname != FM3UL)
                  WinSetWindowText(WinWindowFromID(dcd->hwndFrame,
                                                   MAIN_STATUS),
                                   ((PCNRITEM) (pre->pRecord))->pszFileName);
              }
            }
          }
        }
        break;

      case CN_CONTEXTMENU:
	{
	  PCNRITEM pci = (PCNRITEM)mp2;
	  BOOL wasFollowing;

	  wasFollowing = fFollowTree;
	  fFollowTree = FALSE;
	  if (pci && (INT)pci != -1 && !(pci->flags & RECFLAGS_ENV)) {
	    // 2015-08-09 SHL try to ensure contents stable
	    if (!IsFleshWorkListEmpty())
	      WinPostMsg(hwnd, msg, mp1, mp2);		// Try again later
	    else {
	      WinSendMsg(hwnd,
			 CM_SETRECORDEMPHASIS,
			 MPFROMP(pci), MPFROM2SHORT(TRUE, CRA_CURSORED));
	      MarkAll(hwnd, FALSE, FALSE, TRUE);
	      if (!(pci->attrFile & FILE_DIRECTORY))
		dcd->hwndLastMenu = CheckMenu(hwndMainMenu, &FileMenu, FILE_POPUP);
	      else if (!IsRoot(pci->pszFileName))
		dcd->hwndLastMenu = CheckMenu(hwndMainMenu, &DirMenu, DIR_POPUP);
	      else
		dcd->hwndLastMenu = CheckMenu(hwndMainMenu, &TreeMenu, TREE_POPUP);
	    }
	  }
	  else {
	    dcd->hwndLastMenu = CheckMenu(hwndMainMenu, &TreeCnrMenu, TREECNR_POPUP);
	    if (dcd->hwndLastMenu && !dcd->cnremphasized) {
	      WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
			 MPFROM2SHORT(TRUE, CRA_SOURCE));
	      dcd->cnremphasized = TRUE;
	    }
	  }
	  if (dcd->hwndLastMenu) {
	    if (dcd->hwndLastMenu == DirMenu)
	      WinEnableMenuItem(DirMenu, IDM_TREE, FALSE);
	    if (dcd->hwndLastMenu == TreeCnrMenu) {
	      if (dcd->flWindowAttr & CV_MINI)
		WinCheckMenuItem(dcd->hwndLastMenu, IDM_MINIICONS, TRUE);
	    }
	    if (!PopupMenu(hwnd, hwnd, dcd->hwndLastMenu)) {
	      if (dcd->cnremphasized) {
		WinSendMsg(hwnd, CM_SETRECORDEMPHASIS, MPVOID,
			   MPFROM2SHORT(FALSE, CRA_SOURCE));
		dcd->cnremphasized = FALSE;
	      }
	      if (dcd->hwndLastMenu != TreeCnrMenu)
		MarkAll(hwnd, TRUE, FALSE, TRUE);
	    }
	  }
	  fFollowTree = wasFollowing;
	}
	break;

      case CN_ENTER:
	if (mp2) {
	  PCNRITEM pci = (PCNRITEM) ((PNOTIFYRECORDENTER) mp2)->pRecord;

	  PostMsg(hwnd, UM_ENTER, MPFROMP(pci), MPVOID);
	}
	break;

      case CN_COLLAPSETREE:
      case CN_EXPANDTREE:
        WinPostQueueMsg(hmqExpandTree, CN_EXPANDTREE ? UM_EXPANDTREE :UM_COLLAPSETREE,
                        mp2, MPFROMP(dcd));
	break;
      }					// switch WM_CONTROL
    }
    return 0;

  case UM_ACTION:
    if (mp1) {

      LISTINFO *li = mp1;
      ULONG action = (ULONG) mp2;

      if (!li->list || !li->list[0] ||
	  !PostMsg(dcd->hwndObject, action, MPFROMP(li), MPVOID))
	FreeListInfo(li);
    }
    return 0;

  case UM_SHOWME:
    if (mp1 && dcd) {
      CHAR *dir = xstrdup((CHAR *)mp1, pszSrcFile, __LINE__);

      if (dir) {
	if (!PostMsg(dcd->hwndObject, UM_SHOWME, MPFROMP(dir), MPVOID))
	  free(dir);
	else
	  SetFleshFocusPath(dir);
      }
    }
    return 0;

  case UM_TOPDIR:
    if (mp1) {
      PostMsg(dcd->hwndObject, UM_TOPDIR, mp1, MPVOID);
    }
    return 0;

  case UM_ENTER:
    {
      FILEFINDBUF3 ffb;
      HDIR hDir = HDIR_CREATE;
      ULONG nm = 1;
      BOOL IsOk = FALSE;
      ULONG ulDriveNum, ulDriveMap;
      PCNRITEM pciP, pciL, pci;
      ULONG fl = SWP_ACTIVATE;
      INT x;

      if (fFollowTree)
	fl = 0;
      SetShiftState();
      pci = (PCNRITEM) mp1;
      if (pci &&
	  (INT) pci != -1 &&
	  !(pci->rc.flRecordAttr & CRA_INUSE) &&
	  !(pci->flags & RECFLAGS_ENV) && IsFullName(pci->pszFileName)) {
	x = (INT) (toupper(*pci->pszFileName) - 'A');
	if (driveflags[x] & DRIVE_INVALID) {
	  if (!fAlertBeepOff)
	    DosBeep(50, 100);
	  if (hwndStatus)
	    WinSetWindowText(hwndStatus, (CHAR *) GetPString(IDS_RESCANSUGTEXT));
	  return 0;
	}
	DosError(FERR_DISABLEHARDERR);
	if (!DosQCurDisk(&ulDriveNum, &ulDriveMap)) {
	  if (!(ulDriveMap & 1 << x)) {
	    pciL = pciP = pci;
	    for (;;) {
	      pciP = WinSendMsg(hwnd,
				CM_QUERYRECORD,
				MPFROMP(pciL),
				MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER));
	      if (pciP && (INT) pciP != -1)
		pciL = pciP;
	      else {
		pciP = pciL;
		break;
	      }
	    } // for
	    RemoveCnrItems(hwnd, pciP, 1, CMA_FREE | CMA_INVALIDATE);
	    return 0;
	  }
	}
	if (driveflags[x] & (DRIVE_REMOVABLE | DRIVE_NOPRESCAN)) {

	  struct
	  {
	    ULONG serial;
	    CHAR volumelength;
	    CHAR volumelabel[CCHMAXPATH];
	  }
	  volser;
	  CHAR FileSystem[CCHMAXPATH];
	  CHAR szBuf[CCHMAXPATH];

	  pciL = pciP = pci;
	  for (;;) {
	    pciP = WinSendMsg(hwnd,
			      CM_QUERYRECORD,
			      MPFROMP(pciL),
			      MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER));
	    if (pciP && (INT) pciP != -1)
	      pciL = pciP;
	    else {
	      pciP = pciL;
	      break;
	    }
	  }
	  if ((driveflags[x] & DRIVE_NOPRESCAN) || (toupper(*pci->pszFileName) > 'B' &&
	       !(driveflags[x] & DRIVE_CDROM))) {
	    DriveFlagsOne(x, FileSystem, &volser);
	    SelectDriveIcon(pciP);
	    if (hwndMain)
	      PostMsg(hwndMain, UM_BUILDDRIVEBAR, MPVOID, MPVOID);
	  }
	  memset(&volser, 0, sizeof(volser));
	  DosError(FERR_DISABLEHARDERR);
	  rc = DosQueryFSInfo(toupper(*pci->pszFileName) - '@',
			      FSIL_VOLSER, &volser,
			      (ULONG) sizeof(volser));
	  if (!rc) {
	    if (!volser.serial || driveserial[x] != volser.serial) {
	      AddFleshWorkRequest(hwnd, pciP, eFlesh);	// forceFlesh
	      driveserial[x] = volser.serial;
	    }
	    if (!pciP->fleshed) {
	      AddFleshWorkRequest(hwnd, pciP, eFlesh);	// forceFlesh
	    }
	    if ((fShowFSTypeInTree || fShowDriveLabelInTree) &&
		strlen(pciP->pszFileName) < 4) {
	      strcpy(szBuf, pciP->pszFileName);
	      strcat(szBuf, " [");
	      strcat(szBuf, fShowFSTypeInTree ? FileSystem : volser.volumelabel);
	      strcat(szBuf, "]");
	      pciP->pszDisplayName = xstrdup(szBuf, pszSrcFile, __LINE__);
	      pciP->rc.pszIcon = pciP->pszDisplayName;
	    }
	    WinSendMsg(hwnd,
		       CM_INVALIDATERECORD,
		       MPFROMP(&pciP),
		       MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
	  }
	  else {
	    driveserial[x] = -1;
            PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
            if (dcd)
              PostMsg(dcd->hwndObject, UM_SETUP2, MPFROMP(pci), MPFROMLONG(rc));
	    return 0;
	  }
	}
	rc = 0;
	IsOk = (IsRoot(pci->pszFileName) &&
		IsValidDrive(toupper(*pci->pszFileName)));
	if (!IsOk) {
	  DosError(FERR_DISABLEHARDERR);
	  rc = DosFindFirst(pci->pszFileName, &hDir,
			    FILE_NORMAL | FILE_DIRECTORY |
			    FILE_ARCHIVED | FILE_READONLY |
			    FILE_HIDDEN | FILE_SYSTEM,
			    &ffb, sizeof(ffb), &nm, FIL_STANDARD);
	  priority_bumped();
	}
	if (!rc) {
	  if (!IsOk)
	    DosFindClose(hDir);
	  if (IsOk || (ffb.attrFile & FILE_DIRECTORY)) {
	    if ((shiftstate & (KC_CTRL | KC_ALT)) == (KC_CTRL | KC_ALT)) {
	      PostMsg(hwnd,
		      WM_COMMAND, MPFROM2SHORT(IDM_SHOWALLFILES, 0), MPVOID);
	      return 0;
	    }
	    if ((shiftstate & (KC_CTRL | KC_SHIFT)) == (KC_CTRL | KC_SHIFT)) {
	      OpenObject(pci->pszFileName, Settings, dcd->hwndFrame);
	      return 0;
	    }
	    if (!(shiftstate & (KC_CTRL | KC_SHIFT))) {
	      if (!ParentIsDesktop(hwnd, dcd->hwndParent)) {
		if (FindDirCnrByName(pci->pszFileName, TRUE)) {
		  return 0;
		}
	      }
	    }
	    if ((shiftstate & KC_CTRL) ||
		(!(shiftstate & KC_SHIFT) &&
		 ParentIsDesktop(hwnd, dcd->hwndParent) && fVTreeOpensWPS)) {

	      ULONG size = sizeof(ULONG), flWindowAttr = CV_ICON;
	      CHAR s[33];

	      strcpy(s, PCSZ_ICON);
	      PrfQueryProfileData(fmprof,
				  appname,
				  "DirflWindowAttr",
				  (PVOID) & flWindowAttr, &size);
	      if (flWindowAttr & CV_DETAIL) {
		if (IsRoot(pci->pszFileName))
		  strcpy(s, PCSZ_TREE);
		else
		  strcpy(s, Details);
	      }
	      OpenObject(pci->pszFileName, s, dcd->hwndFrame);
	      return 0;
	    }
	    if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
		!fDCOpens && !LastDir && !(shiftstate & KC_SHIFT))
	      LastDir = FindDirCnr(dcd->hwndParent);
	    if (LastDir && !fDCOpens && !(shiftstate & KC_SHIFT)) {
	      WinSendMsg(LastDir,
			 UM_SETDIR, MPFROMP(pci->pszFileName), MPVOID);
	      WinSetWindowPos(WinQueryWindow(WinQueryWindow(LastDir,
							    QW_PARENT),
					     QW_PARENT),
			      HWND_TOP, 0, 0, 0, 0, SWP_ZORDER | fl);
	    }
	    else
	      OpenDirCnr(hwnd,
			 dcd->hwndParent,
			 dcd->hwndFrame, FALSE, pci->pszFileName);
	  }
	  else {
	    if (!(driveflags[x] & DRIVE_INCLUDEFILES))
	      RemoveCnrItems(hwnd, pci, 1, CMA_FREE | CMA_INVALIDATE);
	    else {

	      SWP swp;

	      WinQueryWindowPos(dcd->hwndFrame, &swp);
	      DefaultViewKeys(hwnd,
			      dcd->hwndFrame,
			      dcd->hwndParent, &swp, pci->pszFileName);
	    }
	  }
	}
	else {
          if (!IsRoot(pci->pszFileName)) {
            NotifyError(pci->pszFileName, rc);
	    RemoveCnrItems(hwnd, pci, 1, CMA_FREE | CMA_INVALIDATE);
	  }
	}
      }
      else if (!pci)
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_MKDIR, 0), MPVOID);
      if (fFollowTree)
	WinSetFocus(HWND_DESKTOP, hwnd);
    }
    return 0;

  case WM_MENUEND:
    if (dcd) {

      HWND hwndMenu = (HWND) mp2;

      if (hwndMenu == TreeCnrMenu || hwndMenu == TreeMenu ||
	  hwndMenu == DirMenu) {
	MarkAll(hwnd, TRUE, FALSE, TRUE);
	if (dcd->cnremphasized) {
	  WinSendMsg(hwnd,
		     CM_SETRECORDEMPHASIS,
		     MPVOID, MPFROM2SHORT(FALSE, CRA_SOURCE));
	  dcd->cnremphasized = FALSE;
	}
      }
    }
    break;

  case UM_OPENWINDOWFORME:
    if (dcd) {
      if (mp1 && !IsFile((CHAR *)mp1))
	OpenDirCnr(hwnd, dcd->hwndParent, dcd->hwndFrame, FALSE, (char *)mp1);
    }
    return 0;

  case MM_PORTHOLEINIT:
    if (dcd) {
      switch (SHORT1FROMMP(mp1)) {
      case 0:
      case 1:
	{
	  ULONG wmsg;

	  wmsg = ((SHORT1FROMMP(mp1) == 0) ? UM_FILESMENU : UM_VIEWSMENU);
	  PortholeInit((HWND) WinSendMsg(dcd->hwndClient,
					 wmsg, MPVOID, MPVOID), mp1, mp2);
	}
	break;
      }
    }
    break;

  case UM_INITMENU:
  case WM_INITMENU:
    if (dcd) {

      switch (SHORT1FROMMP(mp1)) {
      case IDM_FILESMENU:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1) {
	    BOOL rdy;
	    BOOL writeable;
	    BOOL removable;
	    BOOL local;
	    BOOL underenv;
	    CHAR chDrvU;
	    CHAR szDrv[CCHMAXPATH];

            strcpy(szDrv, pci->pszFileName);
	    chDrvU = *pci->pszFileName;
            chDrvU = toupper(chDrvU);
	    MakeValidDir(szDrv);
	    rdy = *szDrv == chDrvU;	// Drive not ready if MakeValidDir changes drive letter
	    removable = rdy
	      && (driveflags[chDrvU - 'A'] & DRIVE_REMOVABLE) != 0;
	    writeable = rdy
	      && !(driveflags[chDrvU - 'A'] & DRIVE_NOTWRITEABLE);
	    local = rdy && (!(driveflags[chDrvU - 'A'] & (DRIVE_REMOTE | DRIVE_VIRTUAL)));
	    underenv = (pci->flags & RECFLAGS_UNDERENV) != 0;
	    CopyPresParams((HWND) mp2, hwndMainMenu);
	    WinEnableMenuItem((HWND) mp2, IDM_INFO, rdy);

	    WinEnableMenuItem((HWND) mp2, IDM_ATTRS, writeable);
	    WinEnableMenuItem((HWND) mp2, IDM_EAS, writeable);
	    WinEnableMenuItem((HWND) mp2, IDM_SUBJECT, writeable);
	    WinEnableMenuItem((HWND) mp2, IDM_DRVFLAGS, 1);	// fixme to allow if not ready

	    WinEnableMenuItem((HWND) mp2, IDM_ARCHIVE, rdy);

	    WinEnableMenuItem((HWND) mp2, IDM_UPDATE, !underenv);
	    WinEnableMenuItem((HWND) mp2, IDM_EXPANDSUBMENU, !underenv);
	    WinEnableMenuItem((HWND) mp2, IDM_EXPAND, !underenv);
	    WinEnableMenuItem((HWND) mp2, IDM_COLLAPSE, !underenv);

	    WinEnableMenuItem((HWND) mp2, IDM_SIZES, rdy);
	    WinEnableMenuItem((HWND) mp2, IDM_MKDIR, writeable);
	    WinEnableMenuItem((HWND) mp2, IDM_SHOWALLFILES, rdy);
	    WinEnableMenuItem((HWND) mp2, IDM_UNDELETE, writeable);

	    WinEnableMenuItem((HWND) mp2, IDM_CHKDSK, writeable && local);
	    WinEnableMenuItem((HWND) mp2, IDM_FORMAT, writeable && local);
	    WinEnableMenuItem((HWND) mp2, IDM_OPTIMIZE, writeable && local);
	    WinEnableMenuItem((HWND) mp2, IDM_PARTITIONSMENU, local);
	    WinEnableMenuItem((HWND) mp2, IDM_PARTITION, fMiniLVM);
	    WinEnableMenuItem((HWND) mp2, IDM_PARTITIONDF, fDFSee);
	    WinEnableMenuItem((HWND) mp2, IDM_PARTITIONLVMG, fLVMGui);
	    WinEnableMenuItem((HWND) mp2, IDM_PARTITIONLVM, fLVM);
	    WinEnableMenuItem((HWND) mp2, IDM_PARTITIONFD, fFDisk);

	    WinEnableMenuItem((HWND) mp2, IDM_DETACH, !local);

	    WinEnableMenuItem((HWND) mp2, IDM_EJECT, removable);

	    WinEnableMenuItem((HWND) mp2, IDM_LOCK, removable);
	    WinEnableMenuItem((HWND) mp2, IDM_UNLOCK, removable);

	    WinEnableMenuItem((HWND) mp2, IDM_DELETE, !underenv && writeable);
	    WinEnableMenuItem((HWND) mp2, IDM_PERMDELETE, !underenv
			      && writeable);
	    WinEnableMenuItem((HWND) mp2, IDM_DELETESUBMENU, !underenv
			      && writeable);
	    WinEnableMenuItem((HWND) mp2, IDM_MOVEMENU, !underenv
			      && writeable);
	    WinEnableMenuItem((HWND) mp2, IDM_RENAME, !underenv && writeable);

	  }
	}
	break;

      case IDM_VIEWSMENU:
	WinCheckMenuItem((HWND) mp2,
			 IDM_MINIICONS, ((dcd->flWindowAttr & CV_MINI) != 0));
	CopyPresParams((HWND) mp2, hwndMainMenu);
	WinEnableMenuItem((HWND) mp2, IDM_RESELECT, FALSE);
	WinEnableMenuItem((HWND) mp2, IDM_PARTITION, fMiniLVM);
	WinEnableMenuItem((HWND) mp2, IDM_PARTITIONDF, fDFSee);
	WinEnableMenuItem((HWND) mp2, IDM_PARTITIONLVMG, fLVMGui);
	WinEnableMenuItem((HWND) mp2, IDM_PARTITIONLVM, fLVM);
	WinEnableMenuItem((HWND) mp2, IDM_PARTITIONFD, fFDisk);
	break;

      case IDM_COMMANDSMENU:
	SetupCommandMenu((HWND) mp2, hwnd);
	CopyPresParams((HWND) mp2, hwndMainMenu);
	break;

      case IDM_SORTSUBMENU:
	SetSortChecks((HWND) mp2, TreesortFlags);
	CopyPresParams((HWND) mp2, hwndMainMenu);
	break;

      case IDM_WINDOWSMENU:
	SetupWinList((HWND) mp2,
		     (hwndMain) ? hwndMain : (HWND) 0, dcd->hwndFrame);
	CopyPresParams((HWND) mp2, hwndMainMenu);
	break;
      }
      dcd->hwndLastMenu = (HWND) mp2;
    }
    if (msg == WM_INITMENU)
      break;
    return 0;

  case UM_COMMAND:
    if (!mp1)
      Runtime_Error(pszSrcFile, __LINE__, NULL);
    else {
      if (!dcd) {
	Runtime_Error(pszSrcFile, __LINE__, NULL);
	FreeListInfo((LISTINFO *) mp1);
      }
      else {
	if (!PostMsg(dcd->hwndObject, UM_COMMAND, mp1, mp2)) {
	  Runtime_Error(pszSrcFile, __LINE__, PCSZ_POSTMSG);
	  FreeListInfo((LISTINFO *) mp1);
	}
	else
	  return (MRESULT) TRUE;
      }
    }
    return 0;

  case UM_LOADFILE:
    if (dcd && mp2) {

      HWND hwnd;

      if ((INT)mp1 == 5 || (INT)mp1 == 13 || (INT)mp1 == 21)
	hwnd = StartViewer(HWND_DESKTOP, (INT)mp1,
			   (CHAR *)mp2, dcd->hwndFrame);
      else
	hwnd = StartMLEEditor(dcd->hwndParent,
			      (INT)mp1, (CHAR *)mp2, dcd->hwndFrame);
      free((CHAR *)mp2);
      return MRFROMLONG(hwnd);
    }
    return 0;

  case UM_FIXCNRMLE:
  case UM_FIXEDITNAME:
    return CommonCnrProc(hwnd, msg, mp1, mp2);

  case UM_NOTIFY:
    if (mp2)
      Notify((CHAR *)mp2);
    return 0;

  case UM_FILTER:
    if (dcd) {

      BOOL tempsusp = dcd->suspendview;

      if (mp1)
	SetMask((CHAR *)mp1, &dcd->mask);

      dcd->suspendview = TRUE;
      dcd->mask.attrFile |= FILE_DIRECTORY;
      WinSendMsg(hwnd, CM_FILTER, MPFROMP(Filter), MPFROMP(&dcd->mask));
      dcd->suspendview = (USHORT) tempsusp;
      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    }
    return 0;

  case UM_DRIVECMD:
    if (mp1) {
      ShowTreeRec(hwnd, (CHAR *)mp1, FALSE, TRUE);
      PostMsg(hwndTree, WM_COMMAND, MPFROM2SHORT(IDM_UPDATE, 0), MPVOID);
    }
    return 0;

  case WM_APPTERMINATENOTIFY:
    {
      APPNOTIFY *info;
      PCNRITEM pci;
      CHAR s[] = " :\\";

      if (!mp2) {
	if (hwndMain)
	  PostMsg(hwndMain, UM_BUILDDRIVEBAR, MPVOID, MPVOID);
      }
      info = apphead;
      while (info) {
	if (info->happ == (HAPP) mp1) {
	  *s = info->device;
	  pci = FindCnrRecord(hwnd, s, NULL, FALSE, FALSE, TRUE);
	  if (pci && (INT) pci != -1) {
	    INT x = info->device - 'A';
	    CHAR  FileSystem[CCHMAXPATH];

	    driveserial[x] = -1;
	    DriveFlagsOne(x, FileSystem, NULL);
	    if (driveflags[x] &
		(DRIVE_INVALID | DRIVE_IGNORE))
	      RemoveCnrItems(hwnd, pci, 1, CMA_FREE);
	    else
	      AddFleshWorkRequest(hwnd, pci, eFlesh);
	  }
	  if (info->prev)
	    info->prev->next = info->next;
	  if (info->next)
	    info->next->prev = info->prev;
	  if (apphead == info)
	    apphead = info->next;
	  if (apptail == info)
	    apptail = info->prev;
	  free(info);
	  break;
	}
	info = info->next;
      }
    }
    break;

  case WM_COMMAND:
    DosError(FERR_DISABLEHARDERR);
    if (dcd) {
      if (SwitchCommand(dcd->hwndLastMenu, SHORT1FROMMP(mp1)))
	return 0;
      switch (SHORT1FROMMP(mp1)) {
      case IDM_SETTARGET:
	SetTargetDir(hwnd, FALSE, NULL);
	break;

      case IDM_DETACH:
	{
	  CHAR d[3] = " :";
	  PCNRITEM pci;
	  PROGDETAILS pgd;
	  CHAR params[368], *p;
	  HAPP happ;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1 && isalpha(*pci->pszFileName)) {
	    *d = toupper(*pci->pszFileName);
	    p = GetCmdSpec(FALSE);
	    memset(&pgd, 0, sizeof(pgd));
	    pgd.Length = sizeof(pgd);
	    pgd.progt.progc = PROG_WINDOWABLEVIO;
	    pgd.progt.fbVisible = SHE_VISIBLE;
	    pgd.pszTitle = (PSZ)GetPString(IDS_DETACHREQUESTTEXT);
	    pgd.pszExecutable = p;
	    pgd.pszParameters = params;
	    pgd.pszStartupDir = NULL;
	    pgd.pszIcon = NULL;
	    pgd.pszEnvironment = NULL;
	    pgd.swpInitial.hwndInsertBehind = HWND_TOP;
	    pgd.swpInitial.hwnd = hwnd;
	    pgd.swpInitial.fl = SWP_SHOW | SWP_ACTIVATE;
	    sprintf(params, "/C NET USE %s /D", d);
	    happ = WinStartApp(hwnd, &pgd, pgd.pszParameters,
			       NULL, SAF_MAXIMIZED);
	    if (!happ) {
	      saymsg(MB_CANCEL | MB_ICONEXCLAMATION, hwnd,
		     GetPString(IDS_ERRORTEXT),
		     GetPString(IDS_CANTSTARTTEXT), p, params);
	    }
	    else {
	      APPNOTIFY *info;

	      info = xmallocz(sizeof(APPNOTIFY), pszSrcFile, __LINE__);
	      if (info) {
		info->happ = happ;
		info->device = *d;
		if (!apphead)
		  apphead = info;
		else {
		  apptail->next = info;
		  info->prev = apptail;
		}
		apptail = info;
	      }
	      PostMsg(hwndTree, WM_COMMAND, MPFROM2SHORT(IDM_RESCAN, 0), MPVOID);
	    }
	  }
	}
	break;

      case IDM_REMAP:
	WinDlgBox(HWND_DESKTOP, hwnd, RemapDlgProc,
		  FM3ModHandle, MAP_FRAME, NULL);
	break;

      case IDM_CONTEXTMENU:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  PostMsg(hwnd, WM_CONTROL, MPFROM2SHORT(DIR_CNR, CN_CONTEXTMENU),
		  MPFROMP(pci));
	}
	break;

      case IDM_FINDINTREE:
	{
	  PSZ pszTempDir;
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1) {
	    pszTempDir = xstrdup(pci->pszFileName, pszSrcFile, __LINE__);
	    if (pszTempDir)
	      MakeValidDir(pszTempDir);
	  }
	  else
	    pszTempDir = xstrdup(pFM2SaveDirectory, pszSrcFile, __LINE__);
	  if (pszTempDir) {
	    if (WinDlgBox(HWND_DESKTOP, dcd->hwndParent,
			  WalkAllDlgProc,
			  FM3ModHandle, WALK_FRAME, MPFROMP(pszTempDir))) {
	      if (!WinSendMsg(hwnd, UM_SHOWME, MPFROMP(pszTempDir), MPFROMLONG(1)))
		free(pszTempDir);
	    }
	    else
	      free(pszTempDir);
	  }
	}
	break;

      case IDM_BEGINEDIT:
	OpenEdit(hwnd);
	break;

      case IDM_ENDEDIT:
	WinSendMsg(hwnd, CM_CLOSEEDIT, MPVOID, MPVOID);
	break;

      case IDM_FILTER:
	{
	  BOOL empty = FALSE;
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (!*dcd->mask.szMask)
	    empty = TRUE;
	  dcd->mask.fIsTree = TRUE;
	  *dcd->mask.prompt = 0;
	  if (pci && (INT) pci != -1)
	    dcd->mask.fFilesIncluded =
	      ((driveflags[toupper(*pci->pszFileName) - 'A'] &
		DRIVE_INCLUDEFILES) != 0);
	  else
	    dcd->mask.fFilesIncluded = FALSE;
	  if (WinDlgBox(HWND_DESKTOP, hwnd, PickMaskDlgProc,
			FM3ModHandle, MSK_FRAME, MPFROMP(&dcd->mask)))
	    WinSendMsg(hwnd, UM_FILTER, MPVOID, MPVOID);
	  else if (empty)
	    *dcd->mask.szMask = 0;
	  PrfWriteProfileData(fmprof, appname, "TreeFilter", &dcd->mask,
			      sizeof(MASK));
	}
	break;

      case IDM_SHOWSORT:
	QuickPopup(hwnd, dcd, CheckMenu(hwndMainMenu, &TreeCnrMenu, TREECNR_POPUP),
		   IDM_SORTSUBMENU);
	break;

      case IDM_SHOWSELECT:
	QuickPopup(hwnd, dcd, CheckMenu(hwndMainMenu, &TreeCnrMenu, TREECNR_POPUP),
		   IDM_SELECTSUBMENU);
	break;

      case IDM_TREECNRVIEWSETTINGS:
	if (!ParentIsDesktop(dcd->hwndParent, dcd->hwndParent))
	  PostMsg(dcd->hwndParent, msg, MPFROMLONG(IDM_TREECNRVIEWSETTINGS), mp2);
	else {
	  WinDlgBox(HWND_DESKTOP,
		    hwnd,
		    CfgDlgProc,
		    FM3ModHandle,
		    CFG_FRAME,
		    MPFROMLONG(IDM_TREECNRVIEWSETTINGS));
	}
	break;

      case IDM_WALKDIR:
      case IDM_OPENWALK:
	{
	  CHAR newpath[CCHMAXPATH];
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1) {
	    strcpy(newpath, pci->pszFileName);
	    MakeValidDir(newpath);
	  }
	  else
	    strcpy(newpath, pFM2SaveDirectory);
	  if (!WinDlgBox(HWND_DESKTOP, dcd->hwndParent, WalkAllDlgProc,
			 FM3ModHandle, WALK_FRAME,
			 MPFROMP(newpath)) || !*newpath)
	    break;
	  WinSendMsg(hwnd, UM_OPENWINDOWFORME, MPFROMP(newpath), MPVOID);
	}
	break;

      case IDM_HELP:
	if (hwndHelp) {
	  if (!ParentIsDesktop(dcd->hwndFrame, dcd->hwndParent))
	    PostMsg(dcd->hwndParent, UM_COMMAND, mp1, mp2);
	  else
	    WinSendMsg(hwndHelp, HM_HELP_CONTENTS, MPVOID, MPVOID);
	}
	break;

      case IDM_PARTITION:
	runemf2(SEPARATE | WINDOWED, HWND_DESKTOP, pszSrcFile, __LINE__,
		NULL, NULL,
		"%s", PCSZ_MINILVMEXE);
	break;

      case IDM_PARTITIONDF:
	runemf2(SEPARATE | WINDOWED, HWND_DESKTOP, pszSrcFile, __LINE__,
		NULL, NULL,
		"%s", PCSZ_DFSOS2EXE);
	break;

      case IDM_PARTITIONLVMG:
	runemf2(SEPARATE | WINDOWED, HWND_DESKTOP, pszSrcFile, __LINE__,
		NULL, NULL,
		"%s", PCSZ_LVMGUICMD);
	break;

      case IDM_PARTITIONLVM:
	runemf2(SEPARATE | WINDOWED, HWND_DESKTOP, pszSrcFile, __LINE__,
		NULL, NULL,
		"%s", PCSZ_LVMEXE);
	break;

      case IDM_PARTITIONFD:
	runemf2(SEPARATE | WINDOWED, HWND_DESKTOP, pszSrcFile, __LINE__,
		NULL, NULL,
		"%s", PCSZ_FDISKPMEXE);
	break;

      case IDM_REFRESHREMOVABLES:
	{
	PFN Rediscover_PRMs;
	HMODULE hmod = 0;
	CHAR objerr[CCHMAXPATH];

	rc = DosLoadModule(objerr, sizeof(objerr), "LVM", &hmod);
	if (!rc) {
	  rc = DosQueryProcAddr(hmod, 70, NULL, &Rediscover_PRMs);
	  if (!rc)
	    Rediscover_PRMs(&rc);
	  DosFreeModule(hmod);
	}
	if (!rc)
	  PostMsg(hwndTree, WM_COMMAND, MPFROM2SHORT(IDM_RESCAN, 0), MPVOID);
	break;
	}

      case IDM_SORTNAME:
      case IDM_SORTFILENAME:
      case IDM_SORTSIZE:
      case IDM_SORTEASIZE:
      case IDM_SORTFIRST:
      case IDM_SORTLAST:
      case IDM_SORTLWDATE:
      case IDM_SORTLADATE:
      case IDM_SORTCRDATE:
	TreesortFlags &= (SORT_REVERSE | SORT_DIRSFIRST | SORT_DIRSLAST);
      case IDM_SORTDIRSFIRST:
      case IDM_SORTDIRSLAST:
      case IDM_SORTREVERSE:
	switch (SHORT1FROMMP(mp1)) {
	case IDM_SORTFILENAME:
	  TreesortFlags |= SORT_FILENAME;
	  break;
	case IDM_SORTSIZE:
	  TreesortFlags |= SORT_SIZE;
	  break;
	case IDM_SORTEASIZE:
	  TreesortFlags |= SORT_EASIZE;
	  break;
	case IDM_SORTFIRST:
	  TreesortFlags |= SORT_FIRSTEXTENSION;
	  break;
	case IDM_SORTLAST:
	  TreesortFlags |= SORT_LASTEXTENSION;
	  break;
	case IDM_SORTLWDATE:
	  TreesortFlags |= SORT_LWDATE;
	  break;
	case IDM_SORTLADATE:
	  TreesortFlags |= SORT_LADATE;
	  break;
	case IDM_SORTCRDATE:
	  TreesortFlags |= SORT_CRDATE;
	  break;
	case IDM_SORTDIRSFIRST:
	  if (TreesortFlags & SORT_DIRSFIRST)
	    TreesortFlags &= (~SORT_DIRSFIRST);
	  else {
	    TreesortFlags |= SORT_DIRSFIRST;
	    TreesortFlags &= (~SORT_DIRSLAST);
	  }
	  break;
	case IDM_SORTDIRSLAST:
	  if (TreesortFlags & SORT_DIRSLAST)
	    TreesortFlags &= (~SORT_DIRSLAST);
	  else {
	    TreesortFlags |= SORT_DIRSLAST;
	    TreesortFlags &= (~SORT_DIRSFIRST);
	  }
	  break;
	case IDM_SORTREVERSE:
	  if (TreesortFlags & SORT_REVERSE)
	    TreesortFlags &= (~SORT_REVERSE);
	  else
	    TreesortFlags |= SORT_REVERSE;
	  break;
	}
	PrfWriteProfileData(fmprof, appname, "TreeSort", &TreesortFlags,
			    sizeof(INT));
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(SortTreeCnr), MPVOID);
	break;

      case IDM_COLLECT:
      case IDM_GREP:
	if (!Collector) {

	  HWND hwndC;
	  SWP swp;

	  if (!ParentIsDesktop(hwnd, dcd->hwndParent) &&
	      !fAutoTile &&
	      (!fExternalCollector && *(ULONG *) realappname == FM3UL))
	    GetNextWindowPos(dcd->hwndParent, &swp, NULL, NULL);
	  hwndC = StartCollector((fExternalCollector ||
				  *(ULONG *) realappname != FM3UL) ?
				 HWND_DESKTOP : dcd->hwndParent, 4);
	  if (hwndC) {
	    if (!ParentIsDesktop(hwnd,
				 dcd->hwndParent) &&
		!fAutoTile &&
		(!fExternalCollector && *(ULONG *) realappname == FM3UL))
	      WinSetWindowPos(hwndC,
			      HWND_TOP,
			      swp.x,
			      swp.y,
			      swp.cx,
			      swp.cy,
			      SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER);
	    else if (!ParentIsDesktop(hwnd,
				      dcd->hwndParent) &&
		     fAutoTile && *(ULONG *) realappname == FM3UL)
	      TileChildren(dcd->hwndParent, TRUE);
	  }
	  WinSetWindowPos(hwndC, HWND_TOP, 0, 0, 0, 0, SWP_ACTIVATE);
	  DosSleep(100);
	}
	else
	  StartCollector(dcd->hwndParent, 4);
	if (SHORT1FROMMP(mp1) == IDM_GREP) {
	  PCNRITEM pci = NULL;

	  pci = WinSendMsg(hwnd,
			   CM_QUERYRECORDEMPHASIS,
			   MPFROMLONG(CMA_FIRST), MPFROMSHORT(CRA_CURSORED));
	  if (pci && (INT) pci != -1)
	    PostMsg(Collector, WM_COMMAND,
		    MPFROM2SHORT(UM_GREP, 0), MPFROMP(pci->pszFileName));
	  else
	    PostMsg(Collector, WM_COMMAND,
		    MPFROM2SHORT(IDM_GREP, 0), MPVOID);
	}
	else
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_COLLECTOR, 0), MPVOID);
	break;

      case IDM_COLLECTOR:
	DosSleep(32);
	{
	  CHAR **list;

	  list = BuildList(hwnd);
	  if (list) {
	    if (Collector) {
	      if (!PostMsg(Collector, WM_COMMAND,
			   MPFROM2SHORT(IDM_COLLECTOR, 0), MPFROMP(list)))
		FreeList(list);
	    }
	    else
	      FreeList(list);
	  }
	}
	break;

      case IDM_COLLAPSEALL:
	WinSendMsg(hwnd, CM_COLLAPSETREE, MPVOID, MPVOID);
	break;

      case IDM_COLLAPSE:
      case IDM_EXPAND:
	{
	  PCNRITEM pci = NULL;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1) {
	    if (pci->flags & RECFLAGS_UNDERENV)
	      break;
	    PostMsg(dcd->hwndObject, UM_EXPAND, mp1, MPFROMP(pci));
	  }
	}
	break;

      case IDM_UPDATE:
	{
	  // 2015-08-07 SHL FIXME select
	  PCNRITEM pci;
	  if (!IsFleshWorkListEmpty())
	    break;			// 2015-08-07 SHL hold off until stable
	  pci = (PCNRITEM)CurrentRecord(hwnd);
	  if (pci && (INT)pci != -1) {
	    struct
	    {
	      ULONG serial;
	      CHAR volumelength;
	      CHAR volumelabel[CCHMAXPATH];
	    }
	    volser;
	    INT x = toupper(*pci->pszFileName) - 'A';
	    CHAR FileSystem[CCHMAXPATH], szBuf[CCHMAXPATH];

	    UINT driveflag = driveflags[x];
	    if (pci->attrFile & FILE_DIRECTORY) {
	      if (pci->flags & RECFLAGS_UNDERENV)
                break;
              if (pci->fleshed) {
                if (x > 1) {
                  AddFleshWorkRequest(hwnd, pci, eUnFlesh);
                }
                else
                  UnFlesh(hwnd, pci);
              }

	      // Check if drive type might need update
	      if ((driveflag & (DRIVE_INVALID | DRIVE_NOPRESCAN)) ||
		  (~driveflag & DRIVE_NOPRESCAN && pci->rc.hptrIcon == hptrDunno)) {
		DriveFlagsOne(x, FileSystem, &volser);
		driveflag = driveflags[x];
                if (driveflag & DRIVE_INVALID)
                  if (driveflag & DRIVE_REMOVABLE) {
                    pci->rc.hptrIcon = hptrRemovable;
                    if (fShowFSTypeInTree || fShowDriveLabelInTree) {
                      strcpy(szBuf, pci->pszFileName);
                      strcat(szBuf, " [");
                      strcat(szBuf, "]");
                      pci->pszDisplayName = xstrdup(szBuf, pszSrcFile, __LINE__);
                      pci->rc.pszIcon = pci->pszDisplayName;
                    }
                  }
                  else 
                    pci->rc.hptrIcon = hptrDunno;
		else if (strlen(pci->pszFileName) < 4) {
		  SelectDriveIcon(pci);
		  if (fShowFSTypeInTree || fShowDriveLabelInTree) {
		    strcpy(szBuf, pci->pszFileName);
		    strcat(szBuf, " [");
		    strcat(szBuf, fShowFSTypeInTree ? FileSystem : volser.volumelabel);
		    strcat(szBuf, "]");
		    pci->pszDisplayName = xstrdup(szBuf, pszSrcFile, __LINE__);
		    pci->rc.pszIcon = pci->pszDisplayName;
		  }
		}
		WinSendMsg(hwnd,
			   CM_INVALIDATERECORD,
			   MPFROMP(&pci),
			   MPFROM2SHORT(1, CMA_ERASE | CMA_REPOSITION));
		if (hwndMain)
		  PostMsg(hwndMain, UM_BUILDDRIVEBAR, MPVOID, MPVOID);
              }
              if (~driveflag & DRIVE_INVALID) {
                if (x > 1)  { 
                  // Note the UnFlesh above may not have completed when this is called
                  // We need to keep it behind the UnFlesh in line
                  AddFleshWorkRequest(hwnd, pci, eFlesh);
                }
                else
                  Flesh(hwnd, pci);
              }
	    }
	  }
	}
	break;

      case IDM_RESCAN:
	PostMsg(dcd->hwndObject, UM_RESCAN, MPVOID, MPVOID);
	break;

      case IDM_RESORT:
	WinSendMsg(hwnd, CM_SORTRECORD, MPFROMP(SortTreeCnr), MPVOID);
	break;

      case IDM_TEXT:
      case IDM_MINIICONS:
	{
	  CNRINFO cnri;

	  memset(&cnri, 0, sizeof(CNRINFO));
	  cnri.cb = sizeof(CNRINFO);
	  WinSendMsg(hwnd,
		     CM_QUERYCNRINFO,
		     MPFROMP(&cnri), MPFROMLONG(sizeof(CNRINFO)));
	  if (SHORT1FROMMP(mp1) == IDM_MINIICONS) {
	    if (cnri.flWindowAttr & CV_MINI)
	      cnri.flWindowAttr &= (~CV_MINI);
	    else
	      cnri.flWindowAttr |= CV_MINI;
	  }
	  else {
	    if (cnri.flWindowAttr & CV_TEXT) {
	      cnri.flWindowAttr &= (~CV_TEXT);
	      cnri.flWindowAttr |= CV_ICON;
	    }
	    else {
	      cnri.flWindowAttr &= (~CV_ICON);
	      cnri.flWindowAttr |= CV_TEXT;
	    }
	  }
	  dcd->flWindowAttr = cnri.flWindowAttr;
	  PrfWriteProfileData(fmprof,
			      appname,
			      "TreeflWindowAttr",
			      &cnri.flWindowAttr, sizeof(ULONG));
	  WinSendMsg(hwnd,
		     CM_SETCNRINFO,
		     MPFROMP(&cnri),
		     MPFROMLONG(CMA_FLWINDOWATTR | CMA_TREEICON |
				CMA_SLTREEBITMAPORICON));
	}
	break;

      case IDM_SIZES:
      case IDM_DRVFLAGS:
      case IDM_SHOWALLFILES:
      case IDM_UNDELETE:
      case IDM_OPTIMIZE:
      case IDM_CHKDSK:
      case IDM_FORMAT:
      case IDM_MKDIR:
      case IDM_LOCK:
      case IDM_UNLOCK:
      case IDM_EJECT:
      case IDM_CLOSETRAY:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1)
	    CommonDriveCmd(hwnd, pci->pszFileName, SHORT1FROMMP(mp1));
	}
	break;

      case IDM_SAVETOLIST:
	WinDlgBox(HWND_DESKTOP,
		  hwnd,
		  SaveListDlgProc, FM3ModHandle, SAV_FRAME, MPFROMP(&hwnd));
	break;

      case IDM_DELETE:
      case IDM_PERMDELETE:
      case IDM_MOVE:
      case IDM_WPSMOVE:
      case IDM_WILDMOVE:
      case IDM_RENAME:
	{
	  PCNRITEM pci;

	  pci = (PCNRITEM) CurrentRecord(hwnd);
	  if (pci && (INT) pci != -1) {
	    if (pci->flags & RECFLAGS_UNDERENV)
	      break;
	  }
	}
	// else intentional fallthru
      case IDM_ATTRS:
      case IDM_INFO:
      case IDM_COPY:
      case IDM_WPSCOPY:
      case IDM_WILDCOPY:
      case IDM_DOITYOURSELF:
      case IDM_OPENWINDOW:
      case IDM_OPENSETTINGS:
      case IDM_OPENDEFAULT:
      case IDM_OPENICON:
      case IDM_OPENDETAILS:
      case IDM_OPENTREE:
      case IDM_SHADOW:
      case IDM_SHADOW2:
      case IDM_COMPARE:
      case IDM_VIEW:
      case IDM_VIEWTEXT:
      case IDM_VIEWBINARY:
      case IDM_EDIT:
      case IDM_EDITTEXT:
      case IDM_EDITBINARY:
      case IDM_EAS:
      case IDM_SUBJECT:
      case IDM_APPENDTOCLIP:
      case IDM_SAVETOCLIP:
      case IDM_ARCHIVE:
      case IDM_MCIPLAY:
      case IDM_UUDECODE:
	{
	  LISTINFO *li;
	  ULONG action = UM_ACTION;
#	  ifdef FORTIFY
	  Fortify_EnterScope();
#	  endif
	  li = xmallocz(sizeof(LISTINFO), pszSrcFile, __LINE__);
	  if (li) {
	    li->type = SHORT1FROMMP(mp1);
	    li->hwnd = hwnd;
	    li->list = BuildList(hwnd);
	    if (!li->list || !li->list[0]) {
	      free(li);
	      break;
	    }
	    if (IsRoot(li->list[0])) {
	      switch (SHORT1FROMMP(mp1)) {
	      case IDM_MOVE:
	      case IDM_COPY:
	      case IDM_WILDCOPY:
	      case IDM_WILDMOVE:
	      case IDM_WPSMOVE:
	      case IDM_WPSCOPY:
	      case IDM_RENAME:
	      case IDM_DELETE:
	      case IDM_PERMDELETE:
		mp1 = MPFROM2SHORT(IDM_INFO, SHORT2FROMMP(mp1));
		li->type = IDM_INFO;
	      }
	    }
	    switch (SHORT1FROMMP(mp1)) {
	    case IDM_APPENDTOCLIP:
	    case IDM_SAVETOCLIP:
	    case IDM_ARCHIVE:
	    case IDM_DELETE:
	    case IDM_PERMDELETE:
	    case IDM_ATTRS:
	    case IDM_SHADOW:
	    case IDM_SHADOW2:
	    case IDM_DOITYOURSELF:
	    case IDM_EAS:
	    case IDM_VIEW:
	    case IDM_VIEWTEXT:
	    case IDM_VIEWBINARY:
	    case IDM_EDIT:
	    case IDM_EDITTEXT:
	    case IDM_EDITBINARY:
	    case IDM_MCIPLAY:
	      action = UM_MASSACTION;
	    }
	    if (li->type == IDM_DELETE)
	      ignorereadonly = FALSE;
	    if (SHORT1FROMMP(mp1) == IDM_SHADOW ||
		SHORT1FROMMP(mp1) == IDM_SHADOW2)
	      *li->targetpath = 0;
	    if (!PostMsg(dcd->hwndObject, action, MPFROMP(li), MPVOID)) {
	      Runtime_Error(pszSrcFile, __LINE__, PCSZ_POSTMSG);
	      FreeListInfo(li);
	    }
	  }
#	  ifdef FORTIFY
	  Fortify_LeaveScope();
#	  endif
	}
	break;

      default:
	if (SHORT1FROMMP(mp1) >= IDM_COMMANDSTART &&
	    SHORT1FROMMP(mp1) < IDM_QUICKTOOLSTART) {

	  INT x;

	  if (!cmdloaded)
	    load_commands();
	  x = SHORT1FROMMP(mp1);
	  if (x >= 0) {
	    RunCommand(hwnd, x);
	    if (fUnHilite)
	      UnHilite(hwnd, TRUE, &dcd->lastselection, 0);
	  }
	}
	break;
      }
    }
    return 0;

  case WM_SAVEAPPLICATION:
    if (dcd && !ParentIsDesktop(hwnd, dcd->hwndParent)) {

      SWP swp, swpP;
      INT ratio;

      WinQueryWindowPos(dcd->hwndFrame, &swp);
      if (!(swp.fl & (SWP_MINIMIZE | SWP_MAXIMIZE | SWP_HIDE))) {
	WinQueryWindowPos(dcd->hwndParent, &swpP);
	if (swp.cx) {
	  ratio = (swpP.cx * 100) / swp.cx;
	  if (ratio > 0)
	    PrfWriteProfileData(fmprof, appname, "TreeWindowRatio",
				&ratio, sizeof(INT));
	}
      }
    }
    else if (dcd && ParentIsDesktop(hwnd, dcd->hwndParent)) {

      SWP swp;

      WinQueryWindowPos(dcd->hwndFrame, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE | SWP_MAXIMIZE)))
	WinStoreWindowPos((CHAR *) FM2Str, "VTreeWindowPos", dcd->hwndFrame);
    }
    break;

  case UM_MINIMIZE:
    if (dcd && hwndMain) {
      fOkayMinimize = TRUE;
      if (dcd->hwndObject) {
	DosSleep(50);
	  fOkayMinimize = FALSE;
	  WinSetWindowPos(((hwndMain) ? WinQueryWindow(hwndMain, QW_PARENT) :
			   dcd->hwndFrame), HWND_TOP, 0, 0, 0, 0,
			  SWP_MINIMIZE | SWP_DEACTIVATE);
      }
    }
    return 0;

  case UM_MAXIMIZE:
    if (dcd || hwndMain)
      WinSetWindowPos(((hwndMain) ? WinQueryWindow(hwndMain, QW_PARENT) :
		       dcd->hwndFrame), HWND_TOP, 0, 0, 0, 0, SWP_MAXIMIZE |
		      SWP_SHOW);
    return 0;

  case UM_CLOSE:
    {
      HWND hwndParent = WinQueryWindow(WinQueryWindow(WinQueryWindow(hwnd,
								     QW_PARENT),
						      QW_PARENT), QW_PARENT);

      if (!mp1) {
	if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
	  WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
	if (hwndParent && !ParentIsDesktop(hwnd, hwndParent))
	  WinDestroyWindow(hwndParent);
      }
      else
	WinDestroyWindow(WinQueryWindow(WinQueryWindow(hwnd, QW_PARENT),
					QW_PARENT));
    }
    return 0;

  case WM_CLOSE:
    WinSendMsg(hwnd, WM_SAVEAPPLICATION, MPVOID, MPVOID);
    if (dcd)
      dcd->stopflag++;
    if (dcd && dcd->hwndObject) {
      // kill object window
      if (WinIsWindow((HAB) 0, dcd->hwndObject)) {
	if (!PostMsg(dcd->hwndObject, WM_CLOSE, MPVOID, MPVOID))
	  WinSendMsg(dcd->hwndObject, WM_CLOSE, MPVOID, MPVOID);
      }
    }
    else
      WinSendMsg(hwnd, UM_CLOSE, MPFROMLONG(1), MPVOID);
    return 0;

  case WM_DESTROY:
#   ifdef FORTIFY
    DbgMsg(pszSrcFile, __LINE__, "WM_DESTROY hwnd %x TID %u", hwnd, GetTidForThread());	// 18 Jul 08 SHL FIXME
#   endif
    if (TreeCnrMenu)
      WinDestroyWindow(TreeCnrMenu);
    if (DirMenu)
      WinDestroyWindow(DirMenu);
    if (FileMenu)
      WinDestroyWindow(FileMenu);
    TreeCnrMenu = FileMenu = DirMenu = (HWND) 0;
    EmptyCnr(hwnd);
    if (apphead) {
      APPNOTIFY *info, *next;

      info = apphead;
      while (info) {
	next = info->next;
	free(info);
	info = next;
      }
      apphead = apptail = NULL;
    }
#   ifdef FORTIFY
    Fortify_LeaveScope();
#   endif
    break; // WM_DESTROY
  } // switch
  if (dcd && dcd->oldproc){
    return dcd->oldproc(hwnd, msg, mp1, mp2);
  }
  else
    return PFNWPCnr(hwnd, msg, mp1, mp2);
}

/**
 * Start drive tree container
 * @returns tree handle or NULLHANDLE
 */

HWND StartTreeCnr(HWND hwndParent, ULONG flags)
{
  /**
   * bitmapped flags:
   * 0x00000001 = don't close app when window closes
   * 0x00000002 = no frame controls
   * 0x00000004 = no close or move button
   */

  HWND hwndFrame = NULLHANDLE;
  HWND hwndSysMenu = NULLHANDLE;
  HWND hwndClient;
  ULONG FrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
    FCF_SIZEBORDER | FCF_MINMAX | FCF_ICON | FCF_NOBYTEALIGN | FCF_ACCELTABLE;
  DIRCNRDATA *dcd;

  if (!hwndParent)
    hwndParent = HWND_DESKTOP;
  if (ParentIsDesktop(hwndParent, hwndParent))
    FrameFlags |= (FCF_TASKLIST | FCF_MENU);
  if (flags & 2)
    FrameFlags &= (~(FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
		     FCF_MINMAX | FCF_ICON));
  hwndFrame = WinCreateStdWindow(hwndParent,
				 WS_VISIBLE,
				 &FrameFlags,
				 (CHAR *) WC_TREECONTAINER,
				 NULL,
				 WS_VISIBLE | fwsAnimate,
				 FM3ModHandle, TREE_FRAME, &hwndClient);
  if (hwndParent != HWND_DESKTOP) {
    hwndSysMenu = WinWindowFromID(hwndFrame, FID_SYSMENU);
    if (hwndSysMenu != NULLHANDLE)
      WinSendMsg(hwndSysMenu, MM_SETITEMATTR,
		 MPFROM2SHORT(SC_CLOSE, TRUE),
		 MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
    if (!fFreeTree)
      WinSendMsg(hwndSysMenu, MM_SETITEMATTR,
		 MPFROM2SHORT(SC_MOVE, TRUE),
		 MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
  }
  if (hwndFrame && hwndClient) {
#   ifdef FORTIFY
    Fortify_EnterScope();
#   endif
    dcd = xmallocz(sizeof(DIRCNRDATA), pszSrcFile, __LINE__);
    if (!dcd) {
      Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_OUTOFMEMORY));
      PostMsg(hwndClient, WM_CLOSE, MPVOID, MPVOID);
      hwndFrame = (HWND) 0;
    }
    else {
      SWP swp;
      WinQueryWindowPos(hwndFrame, &swp);
      if (*(ULONG *) realappname == FM3UL) {
	if (!WinCreateWindow(hwndFrame,
			     (CHAR *) WC_TREEOPENBUTTON,
			     "#303",
			     WS_VISIBLE | BS_PUSHBUTTON | BS_NOPOINTERFOCUS | BS_BITMAP,
			     ((swp.cx -
			       WinQuerySysValue(HWND_DESKTOP,
						SV_CXMINMAXBUTTON)) -
			      WinQuerySysValue(HWND_DESKTOP,
					       SV_CXMINMAXBUTTON) / 2) -
			      WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER),
			     (swp.cy -
			      WinQuerySysValue(HWND_DESKTOP,
					       SV_CYMINMAXBUTTON)) -
			      WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER),
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CXMINMAXBUTTON) / 2,
			     WinQuerySysValue(HWND_DESKTOP,
					      SV_CYMINMAXBUTTON), hwndFrame,
					      HWND_TOP, IDM_OPENWINDOW, NULL, NULL)) {
	  Win_Error(hwndFrame, hwndParent, pszSrcFile, __LINE__,
		    PCSZ_WINCREATEWINDOW);
	}
      }
      else {
	if (!WinCreateWindow(hwndFrame,
			     (CHAR *) WC_TREESTATUS,
			     (CHAR *) GetPString(IDS_YOUAREHERETEXT),
			     WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER,
			     swp.x + 4 + WinQuerySysValue(HWND_DESKTOP,
							  SV_CXSIZEBORDER),
			     swp.cy - (22 + WinQuerySysValue(HWND_DESKTOP,
							     SV_CYSIZEBORDER)),
			     (swp.cx - 8) - (WinQuerySysValue(HWND_DESKTOP,
							      SV_CXSIZEBORDER)
					     * 2), 22, hwndFrame, HWND_TOP,
			     MAIN_STATUS, NULL, NULL)) {
	  Win_Error(hwndFrame, hwndParent, pszSrcFile, __LINE__,
		    PCSZ_WINCREATEWINDOW);
	}
      }
      memset(dcd, 0, sizeof(DIRCNRDATA));
      dcd->size = sizeof(DIRCNRDATA);
      dcd->type = TREE_FRAME;
      dcd->dontclose = (flags & 1) != 0;
      dcd->hwndParent = hwndParent ? hwndParent : HWND_DESKTOP;
      dcd->hwndClient = hwndClient;
      dcd->hwndFrame = hwndFrame;
      {
	PFNWP oldproc;

	oldproc = WinSubclassWindow(hwndFrame, TreeFrameWndProc);
	WinSetWindowPtr(hwndFrame, QWL_USER, (PVOID) oldproc);
	oldproc = WinSubclassWindow(WinWindowFromID(hwndFrame, FID_TITLEBAR),
				    (PFNWP) TreeTitleWndProc);
	WinSetWindowPtr(WinWindowFromID(hwndFrame, FID_TITLEBAR),
			QWL_USER, (PVOID) oldproc);
      }
      dcd->hwndCnr = WinCreateWindow(hwndClient,
				     WC_CONTAINER,
				     NULL,
				     CCS_AUTOPOSITION | CCS_MINIICONS |
				     CCS_MINIRECORDCORE,
				     0,
				     0,
				     0,
				     0,
				     hwndClient,
				     HWND_TOP, (ULONG) TREE_CNR, NULL, NULL);
      if (!dcd->hwndCnr) {
	Win_Error(hwndClient, hwndClient, pszSrcFile, __LINE__,
		  PCSZ_WINCREATEWINDOW);
	PostMsg(hwndClient, WM_CLOSE, MPVOID, MPVOID);
	free(dcd);
	dcd = NULL;
	hwndFrame = (HWND) 0;
      }
      else {
	WinSetWindowPtr(dcd->hwndCnr, QWL_USER, (PVOID) dcd);
	if (ParentIsDesktop(hwndFrame, hwndParent)) {
	  WinSetWindowText(WinWindowFromID(hwndFrame, FID_TITLEBAR), "VTree");
	  FixSwitchList(hwndFrame, "VTree");
	}
	else {
	  WinSetWindowText(hwndFrame, (CHAR *) GetPString(IDS_TREETEXT));
	  WinSetWindowText(WinWindowFromID(hwndFrame, FID_TITLEBAR),
			   (CHAR *) GetPString(IDS_TREETEXT));
	}
	dcd->oldproc = WinSubclassWindow(dcd->hwndCnr, TreeCnrWndProc);
	// fixme to document 01 test?
	if (dcd->oldproc == 0)
	  Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		    "WinSubclassWindow");
	if (!PostMsg(dcd->hwndCnr, UM_SETUP, MPVOID, MPVOID))
	  WinSendMsg(dcd->hwndCnr, UM_SETUP, MPVOID, MPVOID);
      }
    }
#   ifdef FORTIFY
    if (dcd)
      Fortify_ChangeScope(dcd, -1);
    Fortify_LeaveScope();
    if (dcd)
      Fortify_ChangeScope(dcd, +1);
#   endif
  }
  WinShowWindow(hwndFrame, FALSE);
  return hwndFrame;
}

static VOID ExpandTreeThread(VOID *args)
{
  QMSG   qmsg;
  DIRCNRDATA *dcd;
  HAB hab = WinInitialize(0);
# ifdef FORTIFY
  Fortify_EnterScope();
#  endif
  if (hab) {
    hmqExpandTree = WinCreateMsgQueue(hab, 0);
    if (hmqExpandTree) {
      while (WinGetMsg(hab, &qmsg, (HWND) 0, UM_COLLAPSETREE, UM_EXPANDTREE)) {
        dcd = (DIRCNRDATA *) qmsg.mp2;
        if (!dcd)
          Runtime_Error(pszSrcFile, __LINE__, NULL);
        else {
          PCNRITEM pci = (PCNRITEM) qmsg.mp1;

          if (pci && (INT) pci != -1 && !(pci->flags & RECFLAGS_ENV)) {
            if (driveflags[toupper(*pci->pszFileName) - 'A'] & DRIVE_REMOVABLE) {
    
              struct
              {
                ULONG serial;
                CHAR volumelength;
                CHAR volumelabel[CCHMAXPATH];
              }
              volser;
    
              memset(&volser, 0, sizeof(volser));
              DosError(FERR_DISABLEHARDERR);
              if (!DosQueryFSInfo(toupper(*pci->pszFileName) - '@',
                                  FSIL_VOLSER, &volser,
                                  (ULONG) sizeof(volser))) {
                if (qmsg.msg == UM_COLLAPSETREE &&
                    !volser.serial ||
                    driveserial[toupper(*pci->pszFileName) - 'A'] !=
                    volser.serial)
                {
                  if (pci->fleshed) {
                    WaitFleshWorkListEmpty(pci->pszFileName, 10);	// 2015-08-19 SHL in case pci still in work list
                    AddFleshWorkRequest(dcd->hwndCnr, pci, eUnFlesh);
                  }
                }
                if (qmsg.msg != UM_COLLAPSETREE ||
                    (!volser.serial ||
                     driveserial[toupper(*pci->pszFileName) - 'A'] !=
                     volser.serial)) {
                  WaitFleshWorkListEmpty(pci->pszFileName, 10);	// 2015-08-19 SHL in case pci still in work list
                  if (qmsg.msg == UM_EXPANDTREE && AddFleshWorkRequest(dcd->hwndCnr, pci, eFlesh)
                      && !dcd->suspendview  && fTopDir) {
                    PostMsg(dcd->hwndCnr, UM_TOPDIR, MPFROMP(pci), MPVOID);
                  }
                }
                driveserial[toupper(*pci->pszFileName) - 'A'] = volser.serial;
              }
              else {
                driveserial[toupper(*pci->pszFileName) - 'A'] = -1;
                if (pci->fleshed) {
                  WaitFleshWorkListEmpty(pci->pszFileName, 10);	// 2015-08-19 SHL in case pci still in work list
                  AddFleshWorkRequest(dcd->hwndCnr, pci, eUnFlesh);
                }
                PostMsg(dcd->hwndCnr, UM_RESCAN, MPVOID, MPVOID);
                if (!fAlertBeepOff)
                  DosBeep(250, 100);
              }
            }
            else if (qmsg.msg == UM_EXPANDTREE) {
              if (fExpandAll)
                DosSleep(1);
              else {
                WaitFleshWorkListEmpty(pci->pszFileName, 10);	// 2015-08-19 SHL in case pci still in work list
              }
              AddFleshWorkRequest(dcd->hwndCnr, pci, eFlesh);	// forceFlesh
              if (!dcd->suspendview && fTopDir) {
                PostMsg(dcd->hwndCnr, UM_TOPDIR, MPFROMP(pci), MPVOID);
              }
            }
            if (qmsg.msg == UM_EXPANDTREE && !dcd->suspendview) {
              WinSendMsg(dcd->hwndCnr, UM_FILTER, MPVOID, MPVOID);
            }
          }
        }
      }
    }
    WinDestroyMsgQueue(hmqExpandTree);
  }
  WinTerminate(hab);
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

BOOL StartExpandTreeThread(VOID)
{
  TID tid;
  tid = xbeginthread(ExpandTreeThread,
		     65536,
		     NULL,
		     pszSrcFile, __LINE__);
  return tid != -1;

}

#pragma alloc_text(TREECNR,TreeCnrWndProc,TreeObjWndProc,TreeClientWndProc)
#pragma alloc_text(TREECNR,TreeFrameWndProc,TreeTitleWndProc,ShowTreeRec)
#pragma alloc_text(TREECNR,TreeStatProc,OpenButtonProc)
#pragma alloc_text(STARTUP,StartTreeCnr)
