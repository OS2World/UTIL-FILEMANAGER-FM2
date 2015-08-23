
/***********************************************************************

  $Id$

  Worker thread

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2014 Steven H. Levine

  16 Oct 02 SHL Comments
  18 Oct 02 SHL MassAction:Archive - force extension so file found
  06 Jun 05 SHL Indent -i2
  06 Jun 05 SHL Rework Action for VAC3.65 compat
  27 Jul 05 SHL	IDM_DOITYOURSELF - avoid need to strip in ExecOnList
  22 Jul 06 SHL	Comments
  22 Jul 06 SHL Check more run time errors
  03 Nov 06 SHL Renames
  03 Nov 06 SHL Count thread usage
  21 Apr 07 GKY Find FM2Utils by path or utils directory
  16 Jun 07 SHL Update for OpenWatcom
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  07 Aug 07 SHL Use BldQuotedFileName
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  26 Aug 07 SHL Revert to DosSleep(0)
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  22 Jun 08 GKY Made Felete move to xworkplace trash can on systems that have it
  16 JUL 08 GKY Use TMP directory for temp files
  20 Jul 08 GKY Add save/append filename to clipboard.
  02 Aug 08 GKY Limit use of "trash can" to local writable fixed drives or to trash can supported
		drives list if it exists. Fix ability to deselect use of trash can.
  01 Sep 08 GKY Add code to retry on Netdrives "pipe error"
  04 Dec 08 GKY Add a DosSleep to allow file extract to complete before rescan
  04 Dec 08 GKY Add mutex semaphore and disable fSyncUpdates for file deletes to prevent the creation
		on dead CNRITEMS.
  10 Dec 08 SHL Integrate exception handler support
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  25 Dec 08 GKY Add DRIVE_RSCANNED flag to monitor for the first recursive drive scan per session
		to prevent duplicate directory names in tree following a copy before initial scan.
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)
  08 Mar 09 GKY Additional strings move to PCSZs
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.
  26 Jul 09 GKY Fix failure of containers to update when Tree container isn't open in FM2 lite
  13 Dec 09 GKY Attempt to fix container update issues with FM/2 lite
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10).
                Mostly cast CHAR CONSTANT * as CHAR *.
  20 Nov 10 GKY Check that pTmpDir IsValid and recreate if not found; Fixes hangs caused
                by temp file creation failures.
  12 Nov 11 GKY Fixed extract failure caused by spaces in the arc file name.
  04 Aug 12 GKY Changes to use Unlock to unlock files if Unlock.exe is in path both from menu/toolbar and as part of
                copy, move and delete operations
  04 Aug 12 GKY Changes to allow copy and move over readonly files with a warning dialog; also added a warning dialog
                for delete of readonly files
  10 Mar 13 GKY Improvrd readonly check on delete to allow cancel and don't ask again options
                Added saymsg2 for this purpose
  09 Feb 14 GKY Modified wipeallf to allow suppression of the readonly warning on delete
                of temporary files
  16 Feb 14 GKY Fixed move to trashcan to work when the trashcan::drives key hadn't been
                written to OS2.INI. INI check is now only if fTrashCan is TRUE.
  16 Feb 14 GKY Rework readonly check on delete code so it actually works in a logical way
                and so it works with move to trashcan inabled.
  22 Feb 14 GKY Cleanup of readonly check code suppress spurious error on blocked directory
                delete and eliminated the check on additional temp file deletes
  22 Feb 14 GKY Fix warn readonly yes don't ask to work when recursing directories.
  02 May 15 GKY Changes to allow a JAVA executable object to be created using "Real object"
                menu item on a jar file.
  24 Jun 15 GKY Corrected failure to show error message when locked non-exe/dll file fails
                delete
  12 Aug 15 JBS Ticket #522: Ensure no "highmem-unsafe" functions are called directly
                Calls to unsafe Dos... functions have been changed to call the wrapped xDos... functions
  22 Aug 15 GKY Remove recurse scan code.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <process.h>			// _beginthread	// 10 Dec 08 SHL
#include <time.h>

#define INCL_DOS
#define INCL_DOSERRORS
// #define INCL_WINPROGRAMLIST		// 13 Jul 09 SHL dropped
// #define INCL_WINHELP			// 13 Jul 09 SHL dropped
#define INCL_LONGLONG
#define INCL_WINPOINTERS		// WinSetFileIcon
// #define INCL_WINWORKPLACE		// 13 Jul 09 SHL dropped
#define INCL_WINSHELLDATA		// PrfQueryProfileData

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "mainwnd2.h"			// Data declaration(s)
#include "arccnrs.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "defview.h"			// Data declaration(s)
#include "newview.h"			// Data declarations
#include "fm3dlg.h"
#include "fm3str.h"
#include "comp.h"			// FCOMPARE
#include "pathutil.h"			// BldQuotedFileName
#include "makelist.h"			// AddToList
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "notebook.h"			// External viewers
#include "worker.h"			// Action
#include "notify.h"			// AddNote
#include "copyf.h"			// AdjustWildcardName, make_deleteable
#include "attribs.h"			// AttrListDlgProc
#include "chklist.h"			// CheckListProc
#include "info.h"			// DrvInfoProc
#include "extract.h"			// ExtractDlgProc
#include "info.h"			// FileInfoProc
#include "valid.h"			// GetDesktopName, IsNewer
#include "saveclip.h"			// ListToClipboardHab
#include "shadow.h"			// MakeShadows
#include "mkdir.h"			// MassMkdir
#include "uudecode.h"			// MergeDlgProc
#include "objcnr.h"			// ObjCnrDlgProc
#include "printer.h"			// PrintDlgProc, PrintListThread
#include "rename.h"			// RenameProc
#include "srchpath.h"			// RunFM2Util
#include "mainwnd.h"			// TopWindowName
#include "uudecode.h"			// UUD
#include "walkem.h"			// WalkCopyDlgProc, WalkDlgProc, WalkMoveDlgProc
#include "archive.h"			// ArchiveDlgProc
#include "misc.h"			// Broadcast
#include "common.h"			// DecrThreadUsage, IncrThreadUsage
#include "eas.h"			// DisplayEAsProc
#include "systemf.h"			// ExecOnList
#include "avl.h"			// SBoxDlgProc
#include "subj.h"			// Subject
#include "stristr.h"			// stristr
#include "wrappers.h"			// xfopen
#include "fortify.h"
#include "excputil.h"			// 06 May 08 SHL added
#include "getnames.h"                   // insert_filename

// Data definitions
#pragma data_seg(GLOBAL2)
FILE *LogFileHandle;
BOOL fUnlock;

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

#ifdef UNDO

static VOID LINFO undo;

VOID FreeUndo(VOID)
{
  if (undo->list)
    FreeList(undo->list);
  memset(&undo, 0, sizeof(undo));
}

VOID Undo(HWND hwndCnr, HWND hwndFrame, HWND hwndClient, HWND hwndParent)
{
  LISTINFO *li;
  WORKER *wk;

  if (undo->type && undo->list && undo->list[0]) {
    switch (undo->type) {
    case IDM_MOVE:
    case IDM_COPY:
    case IDM_EXTRACT:
      {
#     ifdef FORTIFY
      Fortify_EnterScope();
#      endif
	li = xmallocz(sizeof(LISTINFO), pszSrcFile, __LINE__);
	if (li) {
	  wk = xmallocz(sizeof(WORKER), pszSrcFile, __LINE__);
	  if (wk) {
	    wk->size = sizeof(WORKER);
	    wk->hwndCnr = hwndCnr;
	    wk->hwndParent = hwndParent;
	    wk->hwndFrame = hwndFrame;
	    wk->hwndClient = hwndClient;
	    wk->li = li;
	    *wk->li = *undo;
	    switch (undo->type) {
	    case IDM_COPY:
	    case IDM_EXTRACT:
	      li->type = IDM_PERMDELETE;
	      break;
	    }
	    if (xbeginthread(MassAction,
			     122880,
			     wk,
			     pszSrcFile,
			     __LINE__) == -1)
	    {
	      FreeListInfo(wk->li);
	      free(wk);
#	      ifdef FORTIFY
	      Fortify_LeaveScope();
#	       endif
	    }
	  }
	  else
	    FreeListInfo(li);
	}
      }
      break;
    } // switch
  }
  FreeUndo();
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

#endif // defined(UNDO)

/**
 * Apply action to file list
 * Action repeated for each item in list
 */

VOID Action(VOID * args)
{
  WORKER *wk = (WORKER *)args;
  HAB hab2;
  HMQ hmq2;
  CHAR **files = NULL;
  INT plen = 0;
  CHAR *p, *pp;
  CHAR szQuotedDirName[CCHMAXPATH];
  CHAR szQuotedFileName[CCHMAXPATH];


  if (wk) {
#   ifdef FORTIFY
    Fortify_EnterScope();
#    endif
    if (wk->li && wk->li->list && wk->li->list[0]) {
      hab2 = WinInitialize(0);
      if (hab2) {
	hmq2 = WinCreateMsgQueue(hab2, 0);
	if (hmq2) {
	  CHAR message[(CCHMAXPATH * 2) + 80], wildname[CCHMAXPATH];
	  UINT x;
          BOOL dontask = FALSE, wildcarding = FALSE, overold = FALSE;
          BOOL overnew = FALSE, usedtarget;
          BOOL noreadonlywarn = FALSE;

	  WinCancelShutdown(hmq2, TRUE);
	  IncrThreadUsage();
	  *wildname = 0;
	  // Do action specific preprocessing
	  switch (wk->li->type) {
	  case IDM_MERGE:
	    if (wk->li->type == IDM_MERGE) {
	      if (TestBinary(wk->li->list[0]))
		wk->li->type = IDM_MERGEBINARY;
	      else
		wk->li->type = IDM_MERGETEXT;
	    }
	    strcpy(wk->li->targetpath, wk->li->list[0]);
	    p = strrchr(wk->li->targetpath, '\\');
	    if (p) {
	      p++;
	      *p = 0;
	    }
	    else
	      strcat(wk->li->targetpath, PCSZ_BACKSLASH);
	    sprintf(wk->li->targetpath + strlen(wk->li->targetpath),
		    "MERGE.%03x", (clock() & 4095L));
	    if (!WinDlgBox(HWND_DESKTOP,
			   wk->hwndFrame,
			   MergeDlgProc, FM3ModHandle, MRG_FRAME, (PVOID) wk))
	      goto Abort;
	    if (!wk->li->type ||
		!*wk->li->targetpath || !wk->li->list || !wk->li->list[0])
	      goto Abort;
	    if (IsFile(wk->li->targetpath) != 1 && !wk->li->list[1]) {
	      saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
		     wk->hwndFrame,
		     GetPString(IDS_AHEMTEXT),
		     GetPString(IDS_SILLYMERGETEXT));
	      goto Abort;
	    }
	    break;
	  case IDM_WILDMOVE:
	    wildcarding = TRUE;
	    wk->li->type = IDM_MOVE;
	    break;
	  case IDM_WILDRENAME:
	    wildcarding = TRUE;
	    wk->li->type = IDM_RENAME;
	    break;
	  case IDM_WILDCOPY:
	    wildcarding = TRUE;
	    wk->li->type = IDM_COPY;
	    break;
	  case IDM_MOVEPRESERVE:
	    {
	      CHAR preserve[CCHMAXPATH], *end;

	      wk->li->type = IDM_MOVE;
	      strcpy(preserve, wk->li->list[0] + 2);
	      end = strrchr(preserve, '\\');
	      if (end) {
		end++;
		for (x = 1; wk->li->list[x]; x++) {
		  p = preserve;
		  pp = wk->li->list[x] + 2;
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

	      wk->li->type = IDM_COPY;
	      strcpy(preserve, wk->li->list[0] + 2);
	      end = strrchr(preserve, '\\');
	      if (end) {
		end++;
		for (x = 1; wk->li->list[x]; x++) {
		  p = preserve;
		  pp = wk->li->list[x] + 2;
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
	  }
	  // Process each list item
	  if (wk->li && wk->li->list && wk->li->list[0]) {
	    UINT cFilesModified = 0;	// Required for AddToList
	    UINT cItemsAllocated = 0;	// Required for AddToList
	    UINT cItemsInList;
	    for (cItemsInList = 0; wk->li->list[cItemsInList]; cItemsInList++);	// Count
	    for (x = 0; wk->li->list[x]; x++) {
	      switch (wk->li->type) {
	      case IDM_COLLECTFROMFILE:
		if (Collector) {

		  CHAR *temp = xstrdup(wk->li->list[x], pszSrcFile, __LINE__);

		  if (temp) {
		    if (!PostMsg(Collector,
				 UM_COLLECTFROMFILE, MPFROMP(temp), MPVOID))
		      free(temp);
		  }
		}
		break;

	      case IDM_MERGEBINARY:
	      case IDM_MERGETEXT:
	      case IDM_MERGEBINARYAPPEND:
	      case IDM_MERGETEXTAPPEND:
		{
		  FILE *in, *out;
		  CHAR *moder, *modew;
		  int c;

		  switch (wk->li->type) {
		  case IDM_MERGEBINARY:
		    moder = "rb";
		    modew = "wb";
		    break;
		  case IDM_MERGEBINARYAPPEND:
		    moder = "rb";
		    modew = "a+b";
		    break;
		  case IDM_MERGETEXTAPPEND:
		    moder = "r";
		    modew = "a+";
		    break;
		  default:
		    moder = "r";
		    modew = "w";
		    break;
		  }
		  in = xfsopen(wk->li->list[x], moder, SH_DENYWR, pszSrcFile, __LINE__, TRUE);
		  if (!in) {
		    if (saymsg(MB_ENTERCANCEL,
			       HWND_DESKTOP,
			       GetPString(IDS_MERGEERRORTEXT),
			       GetPString(IDS_CANTOPENINPUTTEXT),
			       wk->li->list[x]) == MBID_CANCEL)
		      goto Abort;
		  }
		  else {
                    out = xfsopen(wk->li->targetpath, modew, SH_DENYWR,
                                  pszSrcFile, __LINE__, TRUE);
		    if (out) {
		      fseek(out, 0L, SEEK_END);
		      switch (wk->li->type) {
		      case IDM_MERGEBINARY:
			wk->li->type = IDM_MERGEBINARYAPPEND;
			break;
		      default:
			wk->li->type = IDM_MERGETEXTAPPEND;
			break;
		      }
		      sprintf(message,
			      GetPString(IDS_MERGINGTEXT),
			      wk->li->list[x], wk->li->targetpath);
		      AddNote(message);
		      while ((c = fgetc(in)) != EOF)
			fputc(c, out);
		      fclose(out);
		      sprintf(message,
			      GetPString(IDS_MERGECOMPLETETEXT),
			      wk->li->list[x], wk->li->targetpath);
		      AddNote(message);
		    }
		    else {
		      saymsg(MB_CANCEL,
			     HWND_DESKTOP,
			     GetPString(IDS_MERGEERRORTEXT),
			     GetPString(IDS_CANTOPENOUTPUTTEXT),
			     wk->li->targetpath);
		      fclose(in);
		      goto Abort;
		    }
		    fclose(in);
		  }
		}
		break;

	      case IDM_UUDECODE:
		{
		  CHAR outname[CCHMAXPATH + 2];

		  sprintf(message,
			  GetPString(IDS_UUDECODINGTEXT), wk->li->list[x]);
		  AddNote(message);
		  if (UUD(wk->li->list[x], outname) && *outname) {
		    sprintf(message,
			    GetPString(IDS_UUDECODECOMPLETETEXT),
			    wk->li->list[x]);
		    AddNote(message);
		    if (//fSyncUpdates ||
			AddToList(outname, &files, &cFilesModified, &cItemsAllocated))
		      Broadcast(hab2,
				wk->hwndCnr,
				UM_UPDATERECORD, MPFROMP(outname), MPVOID);
		  }
		  else {
		    sprintf(message,
			    GetPString(IDS_UUDECODEABORTEDTEXT),
			    wk->li->list[x]);
		    AddNote(message);
		  }
		}
                break;

              case IDM_UNLOCKFILE:
                if (IsFile(wk->li->list[x]) > 0 && fUnlock) {
                  runemf2(SEPARATE | INVISIBLE | BACKGROUND | WAIT,
                          HWND_DESKTOP, pszSrcFile, __LINE__,
                          NULL, NULL, "%s %s", PCSZ_UNLOCKEXE, wk->li->list[x]);
                }
                break;

	      case IDM_VIEWARCHIVE:
		if (IsFile(wk->li->list[x]) > 0) {

		  ARC_TYPE *info = NULL;	// Say calling for editing - fixme to know why?

		  if (WinDlgBox(HWND_DESKTOP,
				wk->hwndFrame,
				SBoxDlgProc,
				FM3ModHandle,
				ASEL_FRAME, (PVOID) & info) && info) {
		    WinSendMsg(wk->hwndCnr,
			       UM_OPENWINDOWFORME,
			       MPFROMP(wk->li->list[x]), MPFROMP(info));
		  }
		}
		break;

	      case IDM_EXTRACT:
		{
		  EXTRDATA ex;
                  BOOL maskspaces = FALSE;
                  CHAR arcname[CCHMAXPATH];

		  memset(&ex, 0, sizeof(EXTRDATA));
		  ex.info = find_type(wk->li->list[x], NULL);
		  if (!ex.info || (!ex.info->extract && !ex.info->exwdirs))
		    break;
                  ex.size = sizeof(EXTRDATA);
                  BldQuotedFileName(arcname, wk->li->list[x]);
                  ex.arcname = arcname;
                  if (!*ex.masks)
                    strcpy(ex.masks, "*");
		  strcpy(ex.extractdir, wk->li->targetpath);
		  if (!WinDlgBox(HWND_DESKTOP,
				 wk->hwndFrame,
				 ExtractDlgProc,
				 FM3ModHandle,
				 EXT_FRAME,
				 (PVOID) & ex) ||
		      !ex.ret ||
		      !*ex.command || !*ex.arcname || !*ex.extractdir)
		    goto Abort;
		  {
		    FILESTATUS3 fsa;

		    DosError(FERR_DISABLEHARDERR);
		    if (DosQueryPathInfo(ex.extractdir,
					 FIL_STANDARD,
					 &fsa,
					 (ULONG) sizeof(FILESTATUS3)) ||
		    !(fsa.attrFile & FILE_DIRECTORY))
		      goto Abort;
		  }
		  if (needs_quoting(ex.masks) && !strchr(ex.masks, '\"'))
		    maskspaces = TRUE;
		  if (!runemf2(SEPARATE | WINDOWED | WAIT |
			       fArcStuffVisible ? 0 : (BACKGROUND | MINIMIZED),
			       HWND_DESKTOP, pszSrcFile, __LINE__, ex.extractdir, NULL,
			       "%s %s %s%s%s",
			       ex.command,
			       ex.arcname,
			       maskspaces ? "\"" : NullStr,
			       *ex.masks ? ex.masks : "\"*\"",
			       maskspaces ? "\"" : NullStr) &&
		      !stricmp(ex.extractdir, wk->directory)) {
		    DosSleep(100); // wait for runemf2 to complete so rescan will actually show something
		    if (WinIsWindow((HAB) 0, wk->hwndCnr))
		      WinSendMsg(wk->hwndCnr,
				 WM_COMMAND,
				 MPFROM2SHORT(IDM_RESCAN, 0), MPVOID);
		  }
		}
		break;

	      case IDM_SUBJECT:
		{
		  INT ret;

		  ret = Subject(wk->hwndFrame, wk->li->list[x]);
		  if (!ret)
		    goto Abort;
		  if (ret == 1) {
		    if (//fSyncUpdates ||
			AddToList(wk->li->list[x],
				  &files, &cFilesModified, &cItemsAllocated))
		      Broadcast(hab2,
				wk->hwndCnr,
				UM_UPDATERECORD,
				MPFROMP(wk->li->list[x]), MPVOID);
		  }
		}
		break;

	      case IDM_INFO:
		if (IsFullName(wk->li->list[x]) &&
		    !(driveflags[toupper(*wk->li->list[x]) - 'A'] &
		      DRIVE_INVALID)) {
		  if (!IsRoot(wk->li->list[x])) {

		    CHAR *list[2];

		    list[0] = wk->li->list[x];
		    list[1] = NULL;
		    if (!WinDlgBox(HWND_DESKTOP,
				   HWND_DESKTOP,
				   FileInfoProc,
				   FM3ModHandle, FLE_FRAME, (PVOID) list)) {
		      goto Abort;
		    }
		  }
		  else {
		    if (!WinDlgBox(HWND_DESKTOP,
				   HWND_DESKTOP,
				   DrvInfoProc,
				   FM3ModHandle,
				   INFO_FRAME, (PVOID) wk->li->list[x]))
		      goto Abort;
		  }
		}
		break;

	      case IDM_OPENWINDOW:
		if (!IsFile(wk->li->list[x]) &&
		    WinIsWindow(hab2, wk->hwndCnr))
		  WinSendMsg(wk->hwndCnr,
			     UM_OPENWINDOWFORME,
			     MPFROMP(wk->li->list[x]), MPVOID);
		break;

	      case IDM_OPENICON:
	      case IDM_OPENDETAILS:
	      case IDM_OPENTREE:
		{
		  FILESTATUS3 fsa;

		  DosError(FERR_DISABLEHARDERR);
		  if (DosQueryPathInfo(wk->li->list[x],
				       FIL_STANDARD,
				       &fsa,
				       (ULONG) sizeof(FILESTATUS3)) ||
		  !(fsa.attrFile & FILE_DIRECTORY))
		    break;
		}
		// else intentional fallthru
	      case IDM_OPENDEFAULT:
	      case IDM_OPENSETTINGS:
		{
		  CHAR *s;

		  switch (wk->li->type) {
		  case IDM_OPENICON:
		    s = (PSZ) PCSZ_ICON;
		    break;
		  case IDM_OPENDETAILS:
		    s = (PSZ) Details;
		    break;
		  case IDM_OPENTREE:
		    s = (PSZ) PCSZ_TREE;
		    break;
		  case IDM_OPENSETTINGS:
		    s = (PSZ) Settings;
		    break;
		  default:
		    s = (PSZ) Default;
		    break;
		  }
		  OpenObject(wk->li->list[x], s, wk->hwndFrame);
		}
		break;

	      case IDM_WPSMOVE:
	      case IDM_WPSCOPY:
	      case IDM_MOVE:
	      case IDM_COPY:
	      case IDM_RENAME:
		{

		  // Select target
		  if (!*wk->li->targetpath && (wk->li->type == IDM_MOVE ||
					       wk->li->type == IDM_COPY ||
					       wk->li->type == IDM_WPSMOVE ||
					       wk->li->type == IDM_WPSCOPY)) {

		    APIRET rc = 1;
		    usedtarget = FALSE;
		    if (hwndMain) {
		      if (!*targetdir)
			TopWindowName(hwndMain,
				      wk->hwndFrame, wk->li->targetpath);
		      else {
			strcpy(wk->li->targetpath, targetdir);
			usedtarget = TRUE;
		      }
		    }
		    if (!*wk->li->targetpath)
		      strcpy(wk->li->targetpath, wk->directory);
		    if (!*wk->li->targetpath) {
		      strcpy(wk->li->targetpath, wk->li->list[0]);
		      p = strrchr(wk->li->targetpath, '\\');
		      if (p) {
			if (*(p - 1) == ':')
			  p++;
			*p = 0;
		      }
		    }
		    MakeValidDir(wk->li->targetpath);
		    if (fConfirmTarget ||
			(!*targetdir && strcmp(realappname, "FM/4"))) {
		    RetryPath:
		      // Confirm target
		      usedtarget = FALSE;
		      if (wk->li->type == IDM_MOVE ||
			  wk->li->type == IDM_WPSMOVE) {
			rc = WinDlgBox(HWND_DESKTOP,
				       wk->hwndFrame,
				       WalkMoveDlgProc,
				       FM3ModHandle,
				       WALK_FRAME, MPFROMP(wk->li->targetpath));
		      }
		      else if (wk->li->type == IDM_COPY ||
			       wk->li->type == IDM_WPSCOPY) {
			rc = WinDlgBox(HWND_DESKTOP,
				       wk->hwndFrame,
				       WalkCopyDlgProc,
				       FM3ModHandle,
				       WALK_FRAME, MPFROMP(wk->li->targetpath));
		      }
		      else
			rc = WinDlgBox(HWND_DESKTOP,
				       wk->hwndFrame,
				       WalkDlgProc,
				       FM3ModHandle,
				       WALK_FRAME, MPFROMP(wk->li->targetpath));
		    }
		    if (!rc || !*wk->li->targetpath)
		      goto Abort;
		    // Check target OK
		    if (driveflags[toupper(*wk->li->targetpath) - 'A'] &
			DRIVE_NOTWRITEABLE) {
		      saymsg(MB_CANCEL,
			     wk->hwndFrame,
			     GetPString(IDS_ERRORTEXT),
			     GetPString(IDS_NOTWRITENOTARGETTEXT));
		      goto RetryPath;
		    }
		  }
		Retry:
		  {
		    // Target OK so far
		    CHAR newname[CCHMAXPATH];
                    APIRET rc;
                    INT ret;
		    FILESTATUS4L fs4;
		    BOOL isnewer;
		    BOOL existed;
		    BOOL fResetVerify = FALSE;
		    INT type = wk->li->type == IDM_RENAME ?
		      MOVE :
		      wk->li->type == IDM_MOVE ?
			MOVE : (wk->li->type == IDM_WPSMOVE) ?
			  WPSMOVE : wk->li->type == IDM_WPSCOPY ?
			    WPSCOPY : COPY;
		    PCSZ moving = wk->li->type == IDM_RENAME ?
		      GetPString(IDS_RENAMINGTEXT) :
		      wk->li->type == IDM_MOVE || wk->li->type == IDM_WPSMOVE ?
			GetPString(IDS_MOVINGTEXT) : GetPString(IDS_COPYINGTEXT);
		    PCSZ move = wk->li->type == IDM_RENAME ?
		      GetPString(IDS_RENAMETEXT) :
		      wk->li->type == IDM_MOVE ||
		       wk->li->type == IDM_WPSMOVE ?
			GetPString(IDS_MOVETEXT) : GetPString(IDS_COPYTEXT);
		    PCSZ moved = wk->li->type == IDM_RENAME ?
		      GetPString(IDS_RENAMEDTEXT) :
		      wk->li->type == IDM_MOVE || wk->li->type == IDM_WPSMOVE ?
			GetPString(IDS_MOVEDTEXT) : GetPString(IDS_COPIEDTEXT);

		    if (*wk->li->targetpath) {
		      strcpy(newname, wk->li->targetpath);
		      AddBackslashToPath(newname);
		      if (plen)
			p = wk->li->list[x] + plen;
		      else {
			p = strrchr(wk->li->list[x], '\\');
			if (p)
			  p++;
			else
			  p = wk->li->list[x];
		      }
		      strcat(newname, p);
		    }
		    else
		      strcpy(newname, wk->li->list[x]);
		    if ((wildcarding || wk->li->type == IDM_RENAME) &&
			*wildname)
		    {
		      CHAR testname[CCHMAXPATH];
		      strcpy(testname, wildname);
		      if (AdjustWildcardName(newname, testname))
			strcpy(newname, testname);
		    }
		    existed = IsFile(newname) != -1;
		    isnewer = IsNewer(wk->li->list[x], newname);
		    if (existed && wk->li->type != IDM_RENAME && dontask) {
		      if (!overold && !overnew)
			break;
		      if (!overold && !isnewer)
			break;
		      if (!overnew && isnewer)
			break;
		    }
		    // Confirm overwrite unless bypassed
		    if ((wk->li->type == IDM_RENAME &&
			 (!dontask || !*wildname)) ||
			(!dontask && existed) ||
			(!dontask && wildcarding) ||
			(IsFile(newname) == 0 && IsFile(wk->li->list[x]) > 0))
		    {
		      MOVEIT mv;
		      memset(&mv, 0, sizeof(MOVEIT));
		      mv.rename = (wk->li->type == IDM_RENAME);
		      mv.source = wk->li->list[x];
		      strcpy(mv.target, newname);
		      rc = WinDlgBox(HWND_DESKTOP,
				     wk->hwndFrame,
				     RenameProc,
				     FM3ModHandle, REN_FRAME, (PVOID) & mv);
		      if (!rc)
			goto Abort;

		      DosSleep(1);
		      if (mv.skip || !*mv.target)
			break;
		      if (mv.dontask)
			dontask = TRUE;
		      if (mv.overold)
			overold = TRUE;
		      if (mv.overnew)
                        overnew = TRUE;
                      if (mv.noreadonlywarn)
                        noreadonlywarn = TRUE;
		      if (wildcarding || wk->li->type == IDM_RENAME) {
			p = strrchr(mv.target, '\\');
			if (p && (strchr(p, '*') || strchr(p, '?'))) {
			  strcpy(wildname, mv.target);
			  AdjustWildcardName(wk->li->list[x], mv.target);
			}
			else
			  *wildname = 0;
		      }
		      strcpy(newname, mv.target);
		      existed = (IsFile(newname) != -1);
		      isnewer = IsNewer(wk->li->list[x], newname);
		      if (!mv.overwrite) {
			if (existed && wk->li->type != IDM_RENAME && dontask) {
			  if (!overold && !overnew)
			    break;
			  if (!overold && !isnewer)
			    break;
			  if (!overnew && isnewer)
			    break;
			}
		      }
		    }
		    if (!strcmp(wk->li->list[x], newname) ||
			(wk->li->type == IDM_COPY &&
			 !stricmp(wk->li->list[x], newname)))
		      break;
		    sprintf(message,
			    " %s \"%s\" %s\"%s\"%s [%u %s%u]",
			    moving,
			    wk->li->list[x],
			    GetPString(IDS_TOTEXT),	// Has trailing space
			    newname,
			    usedtarget ?
			      GetPString(IDS_TOTARGETTEXT) :
			      NullStr,
			    x + 1,
			    GetPString(IDS_OFTEXT),	// Has trailing space
			    cItemsInList);
		    AddNote(message);
		    if (fVerify && (driveflags[toupper(*wk->li->targetpath) - 'A'] & DRIVE_WRITEVERIFYOFF) |
			(driveflags[toupper(*wk->li->list[x]) - 'A'] & DRIVE_WRITEVERIFYOFF)) {
		      DosSetVerify(FALSE);
		      fResetVerify = TRUE;
		    }
		    if (plen) {
		      // make directory/ies, if required

		      CHAR dirpart[CCHMAXPATH];

		      strcpy(dirpart, newname);
		      p = strrchr(dirpart, '\\');
		      if (p) {
			*p = 0;
			if (p > dirpart + 3)
			  MassMkdir((hwndMain) ? hwndMain : wk->hwndCnr,
				    dirpart);
		      }
		    }
		    if (fRealIdle)
		      priority_idle();

                    rc = docopyf(type, wk->li->list[x], newname);
                    if (rc == ERROR_ACCESS_DENIED || rc == ERROR_SHARING_VIOLATION) {
                      ret = make_deleteable(newname, rc, noreadonlywarn);
                      if (ret == SM2_CANCEL)
                        break;
                      if (ret == SM2_DONTASK)
                        noreadonlywarn = TRUE;
                      if (ret == SM2_NO)
                        continue;
                      rc = docopyf(type, wk->li->list[x], newname);
                    }
                    if (rc == ERROR_ACCESS_DENIED || (rc == ERROR_SHARING_VIOLATION && fUnlock))
                      rc = NO_ERROR;
		    if (fResetVerify) {
		      DosSetVerify(fVerify);
		      fResetVerify = FALSE;
		    }
                    priority_normal();
		    if (rc) {
		      if ((rc == ERROR_DISK_FULL ||
			   rc == ERROR_HANDLE_DISK_FULL) &&
			  isalpha(*newname) &&
			  (driveflags[toupper(*newname) - 'A'] &
			   DRIVE_REMOVABLE)
			  && !(driveflags[toupper(*newname) - 'A'] &
			       DRIVE_NOTWRITEABLE)
			  && toupper(*newname) != toupper(*wk->li->list[x])
			  && !DosQueryPathInfo(wk->li->list[x], FIL_QUERYEASIZEL,
					       &fs4, sizeof(fs4))
			  && !(fs4.attrFile & FILE_DIRECTORY)) {

			FSALLOCATE fsa;
			ULONGLONG ullFreeBytes;
			CHAR *ptr;
			INT cntr;

			Notify(GetPString(IDS_FITTINGTEXT));
			DosError(FERR_DISABLEHARDERR);
			if (!DosQueryFSInfo(toupper(*newname) - '@',
					    FSIL_ALLOC,
					    &fsa, sizeof(FSALLOCATE))) {
			  // Assume large file support
			  ullFreeBytes = (ULONGLONG) fsa.cUnitAvail * fsa.cSectorUnit *
			    fsa.cbSector;
			  if (ullFreeBytes) {
			    // Find item that will fit in available space
			    for (cntr = x + 1; wk->li->list[cntr]; cntr++) {
			      DosError(FERR_DISABLEHARDERR);
			      if (!DosQueryPathInfo(wk->li->list[cntr],
						    FIL_QUERYEASIZEL,
						    &fs4,
						    sizeof(fs4)) &&
				  !(fs4.attrFile & FILE_DIRECTORY) &&
				  // fixme to use CBLIST_TO_EASIZE?
				  fs4.cbFile + fs4.cbList <= ullFreeBytes) {
				// Swap with failing item
				ptr = wk->li->list[x];
				wk->li->list[x] = wk->li->list[cntr];
				wk->li->list[cntr] = ptr;
				goto Retry;
			      }
			    }
			    Notify(GetPString(IDS_COULDNTFITTEXT));
			  }
			}
			rc = saymsg(MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION,
				    wk->hwndFrame,
				    GetPString(IDS_DISKFULLTEXT),
				    GetPString(IDS_ANOTHERDISKTEXT));
			if (rc == MBID_RETRY)
			  goto Retry;
			if (rc == MBID_ABORT)
			  goto Abort;
		      }
		      else {
			if (LogFileHandle) {
			  fprintf(LogFileHandle,
				  GetPString(IDS_LOGTOFAILEDTEXT),
				  move, wk->li->list[x], newname, rc);
			}
			rc = Dos_Error(MB_ENTERCANCEL,
				       rc,
				       wk->hwndFrame,
				       pszSrcFile,
				       __LINE__,
				       "%s %s\"%s\" %s\"%s\" %s.",
				       move,			// move, copy, rename etc.
				       GetPString(IDS_OFTEXT),	// "of " - note trailing space
				       wk->li->list[x],
				       GetPString(IDS_TOTEXT),	// "to "
				       newname, GetPString(IDS_FAILEDTEXT));	// "failed"
			if (rc == MBID_CANCEL)
			  goto Abort;
		      }
		    }
		    else {
		      if (LogFileHandle) {
			fprintf(LogFileHandle,
				"%s \"%s\" %s\"%s\"\n",
				moved,
				wk->li->list[x],
				GetPString(IDS_TOTEXT), newname);
		      }
		      if (!strcmp(realappname, "FM/4") &&
			  AddToList(wk->li->list[x],
				    &files, &cFilesModified, &cItemsAllocated))
			Broadcast(hab2,
                                  wk->li->type == IDM_RENAME ? wk->hwndParent : wk->hwndCnr,
				  UM_UPDATERECORD,
				  MPFROMP(wk->li->list[x]), MPVOID);
		      if (!strcmp(realappname, "FM/4") &&
			  AddToList(newname, &files, &cFilesModified, &cItemsAllocated))
			Broadcast(hab2,
				  wk->li->type == IDM_RENAME ? wk->hwndParent : wk->hwndCnr,
				  UM_UPDATERECORD, MPFROMP(newname), MPVOID);
		    }
		  }
		  break;
		}

	      case IDM_COMPARE:
		if ((!IsFile(wk->li->targetpath) ||
		     IsRoot(wk->li->targetpath)) &&
		    (!IsFile(wk->li->list[x]) || IsRoot(wk->li->list[x]))) {
		  if (!*dircompare && WinIsWindow(hab2, wk->hwndCnr))
		    WinSendMsg(wk->hwndCnr,
			       UM_COMPARE,
			       MPFROMP(wk->li->targetpath),
			       MPFROMP(wk->li->list[x]));
		  else {
		    runemf2(SEPARATE,
			    HWND_DESKTOP, pszSrcFile, __LINE__,
			    NULL, NULL,
			    "%s %s %s",
			    dircompare,
			    BldQuotedFileName(szQuotedDirName, wk->li->targetpath),
			    BldQuotedFileName(szQuotedFileName, wk->li->list[x]));
		  }
		}
		else if (*compare) {
		  CHAR *fakelist[3];

		  fakelist[0] = wk->li->list[x];
		  fakelist[1] = wk->li->targetpath;
		  fakelist[2] = NULL;
		  ExecOnList(wk->hwndFrame,
			     compare,
			     WINDOWED | SEPARATEKEEP, NULL, NULL, fakelist, NULL,
			     pszSrcFile, __LINE__);
		}
		else {
		  FCOMPARE fc;

		  memset(&fc, 0, sizeof(fc));
		  fc.size = sizeof(fc);
		  fc.hwndParent = wk->hwndParent;
		  strcpy(fc.file1, wk->li->list[x]);
		  strcpy(fc.file2, wk->li->targetpath);
		  if (WinDlgBox(HWND_DESKTOP,
				wk->hwndFrame,
				CFileDlgProc,
				FM3ModHandle, FCMP_FRAME, (PVOID) & fc))
		    goto Abort;
		}
		break;
	      }	// switch
	      DosSleep(0);
	    } // for list

	    // Do action specific post-processing
	    switch (wk->li->type) {
	    case IDM_MOVE:
	    case IDM_COPY:
	    case IDM_WPSMOVE:
	    case IDM_WPSCOPY:
	    case IDM_RENAME:
	      sprintf(message,
		      GetPString(IDS_OPSCOMPLETETEXT),
		      wk->li->type == IDM_MOVE ?
			GetPString(IDS_MOVETEXT) :
			wk->li->type == IDM_COPY ?
			  GetPString(IDS_COPYTEXT) :
			  wk->li->type == IDM_WPSMOVE ?
			    GetPString(IDS_WPSMOVETEXT) :
			    wk->li->type == IDM_WPSCOPY ?
			      GetPString(IDS_WPSCOPYTEXT) :
			      GetPString(IDS_RENAMETEXT),
		      &"s"[x == 1],		// s or nul
		      (wk->li->type == IDM_MOVE ||
		       wk->li->type == IDM_COPY ||
		       wk->li->type == IDM_WPSMOVE ||
		       wk->li->type == IDM_WPSCOPY) ?
			GetPString(IDS_TOTEXT) : NullStr,
		      (wk->li->type == IDM_MOVE ||
		       wk->li->type == IDM_COPY ||
		       wk->li->type == IDM_WPSMOVE ||
		       wk->li->type == IDM_WPSCOPY) ?
			wk->li->targetpath : NullStr,
		      GetPString(x != 1 ? IDS_ARETEXT : IDS_ISTEXT));
	      Notify(message);
	      if (toupper(*wk->li->targetpath) < 'C' && !fAlertBeepOff)
		DosBeep(1000, 25);	// Wake up user
	      DosSleep(16);//05 Aug 07 GKY 33
	      if (wk->li->type == IDM_WPSMOVE || wk->li->type == IDM_WPSCOPY)
		DosSleep(48);//05 Aug 07 GKY 96
	      break;
	    default:
	      break;
	    }
	  } // if have non-empty list

	Abort:

	  if (files) {
            if (!strcmp(realappname, "FM/4") || !hwndTree ||
                (driveflags[*wk->li->targetpath - 'A'] & DRIVE_RSCANNED))
	      Broadcast(hab2,
			wk->hwndCnr,
			UM_UPDATERECORDLIST, MPFROMP(files), MPVOID);
	    FreeList(files);
	  }

	  if (WinIsWindow(hab2, wk->hwndCnr))
	    PostMsg(wk->hwndCnr, UM_RESCAN, MPVOID, MPVOID);

	  WinDestroyMsgQueue(hmq2);
	} // if queue
	DecrThreadUsage();
	WinTerminate(hab2);
      } // if hab2
    } // if list not empty

    if (wk->li)
      FreeListInfo(wk->li);
    free(wk);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
    DosPostEventSem(CompactSem);
  }
}

/**
 * Apply file list to action
 * All items in list processed as a group, if possible
 */

VOID MassAction(VOID * args)
{
  WORKER *wk = (WORKER *) args;
  HAB hab2;
  HMQ hmq2;
  CHAR **files = NULL;
  CHAR *p, *pp;
  UINT numfiles = 0, numalloc = 0;


  if (wk) {
#   ifdef FORTIFY
    // Fortify_BecomeOwner(wk);
    Fortify_EnterScope();
#   endif
    if (wk->li && wk->li->list && wk->li->list[0]) {
      hab2 = WinInitialize(0);
      if (hab2) {
	hmq2 = WinCreateMsgQueue(hab2, 0);
	if (hmq2) {
	  WinCancelShutdown(hmq2, TRUE);
	  IncrThreadUsage();
	  DosError(FERR_DISABLEHARDERR);
	  if (IsRoot(wk->li->list[0]) || !IsFile(wk->li->list[0])) {
	    if (wk->li->type == IDM_VIEW)
	      wk->li->type = IDM_INFO;
	    if (wk->li->type == IDM_EDIT)
	      wk->li->type = IDM_EAS;
	  }
	  switch (wk->li->type) {
	  case IDM_INFO:
	    if (WinDlgBox(HWND_DESKTOP,
			  wk->hwndFrame,
			  FileInfoProc,
			  FM3ModHandle, FLE_FRAME, (PVOID) wk->li->list) != 2)
	    {
	      break;
	    }
	    // else intentional fallthru
	  case IDM_UPDATE:
	    Broadcast(hab2,
		      wk->hwndCnr,
		      UM_UPDATERECORDLIST, MPFROMP(wk->li->list), MPVOID);
	    break;

	  case IDM_EAS:
	    if (WinDlgBox(HWND_DESKTOP,
			  wk->hwndFrame,
			  DisplayEAsProc,
			  FM3ModHandle, EA_FRAME, (PVOID) wk->li->list))
	      Broadcast(hab2,
			wk->hwndCnr,
			UM_UPDATERECORDLIST, MPFROMP(wk->li->list), MPVOID);
	    break;

	  case IDM_DOITYOURSELF:
	    ExecOnList(wk->hwndFrame,
		       "%a",
		       WINDOWED | SEPARATE | PROMPT, NULL,
		       NULL, wk->li->list, GetPString(IDS_DOITYOURSELFTEXT),
		       pszSrcFile, __LINE__);
	    break;

	  case IDM_MCIPLAY:
	    {
	      UINT x;
	      UINT MaxFM2playStrLen = 24;
	      ULONG total;
	      CHAR fbuf[CCHMAXPATH];

	      if (DosSearchPath(SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT |
				SEARCH_CUR_DIRECTORY,
				(CHAR *) PCSZ_PATH, (CHAR *) PCSZ_FM2PLAYEXE, (PBYTE)fbuf, CCHMAXPATH - 1))
		total = MaxFM2playStrLen;
	      else
		total = strlen(fbuf);
	      for (x = 0; wk->li->list[x]; x++)
		total += (strlen(wk->li->list[x]) + 1 +
			  (needs_quoting(wk->li->list[x]) * 2));
	      if (total > 1000) {

		FILE *fp;
                CHAR szTempFile[CCHMAXPATH];
                CHAR *modew = "w";

                if (pTmpDir && !IsValidDir(pTmpDir))
                  DosCreateDir(pTmpDir, 0);
                BldFullPathName(szTempFile, pTmpDir, PCSZ_FM2PLAYTEMP);
		fp = xfopen(szTempFile, modew, pszSrcFile, __LINE__, FALSE);
		if (fp) {
		  fprintf(fp, "%s", ";AV/2-built FM2Play listfile\n");
		  for (x = 0; wk->li->list[x]; x++)
		    fprintf(fp, "%s\n", wk->li->list[x]);
		  fprintf(fp, ";end\n");
		  fclose(fp);
		  strrev(szTempFile);
		  strcat(szTempFile, "@/");
		  strrev(szTempFile);
		  RunFM2Util(PCSZ_FM2PLAYEXE, szTempFile);
		}
	      }
	    }
	    // intentional fallthru
	  case IDM_FAKEEXTRACT:
	  case IDM_FAKEEXTRACTM:
	    if (wk->li->type == IDM_MCIPLAY ||
		(*wk->li->arcname && wk->li->info &&
		 wk->li->info->extract && *wk->li->targetpath)) {

	      CHAR szBuffer[1025];
	      CHAR fbuf[CCHMAXPATH];
	      UINT x;

	      if (wk->li->type == IDM_FAKEEXTRACT ||
		  wk->li->type == IDM_FAKEEXTRACTM) {
		strcpy(szBuffer,
		       (wk->li->info->exwdirs) ?
		       wk->li->info->exwdirs : wk->li->info->extract);
		strcat(szBuffer, " ");
		BldQuotedFileName(szBuffer + strlen(szBuffer), wk->li->arcname);
	      }
	      else {
		if (DosSearchPath(SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT |
				  SEARCH_CUR_DIRECTORY,
				  (CHAR *) PCSZ_PATH, (CHAR *) PCSZ_FM2PLAYEXE, (PBYTE)fbuf, CCHMAXPATH - 1))
		  strcpy(szBuffer, "UTILS\\FM2PLAY.EXE");
		else
		  strcpy(szBuffer, PCSZ_FM2PLAYEXE);
	      }
	      p = &szBuffer[strlen(szBuffer)];
	      strcat(szBuffer, " ");
	      x = 0;
	      while (wk->li->list[x]) {
		pp = wk->li->list[x];
		while (*pp) {
		  if (*pp == '/')
		    *pp = '\\';
		  pp++;
		}
		BldQuotedFileName(szBuffer + strlen(szBuffer), wk->li->list[x]);
		x++;
		if (!wk->li->list[x] || strlen(szBuffer) +
		    strlen(wk->li->list[x]) + 5 > 1024) {
		  runemf2(SEPARATE | WINDOWED | BACKGROUND | MINIMIZED | WAIT,
			  HWND_DESKTOP, pszSrcFile, __LINE__,
			  (wk->li->type == IDM_FAKEEXTRACT ||
			   wk->li->type == IDM_FAKEEXTRACTM) ?
			     wk->li->targetpath : NULL,
			   NULL,
			   "%s", szBuffer);
		  DosSleep(1);
		  *p = 0;
		}
		strcat(szBuffer, " ");
	      }
	      if (wk->li->type == IDM_MCIPLAY)
		break;
	      strcpy(szBuffer, wk->li->targetpath);
	      AddBackslashToPath(wk->li->targetpath);
	      //if (wk->li->targetpath[strlen(wk->li->targetpath) - 1] != '\\')
	      //  strcat(szBuffer, "\\");
	      p = szBuffer + strlen(szBuffer);
	      for (x = 0; wk->li->list[x]; x++) {
		strcpy(p, wk->li->list[x]);
		free(wk->li->list[x]);
		wk->li->list[x] = xstrdup(szBuffer, pszSrcFile, __LINE__);
	      }
	      if (wk->li->list[0])
		Broadcast(hab2,
			  wk->hwndCnr,
			  UM_UPDATERECORDLIST, MPFROMP(wk->li->list), MPVOID);
	    }
	    break;

	  case IDM_SETICON:
	    if (*wk->li->targetpath) {

	      ICONINFO ici;

	      memset(&ici, 0, sizeof(ICONINFO));
	      ici.cb = sizeof(ICONINFO);
	      ici.fFormat = ICON_FILE;
	      ici.pszFileName = wk->li->list[0];
	      if (!WinSetFileIcon((PSZ) wk->li->targetpath,
				  (PICONINFO) & ici)) {
		ici.fFormat = ICON_CLEAR;
		WinSetFileIcon((PSZ) wk->li->targetpath, (PICONINFO) & ici);
	      }
	      Broadcast(hab2,
			wk->hwndCnr,
			UM_UPDATERECORD, MPFROMP(wk->li->targetpath), MPVOID);
	    }
	    break;

	  case IDM_APPENDTOCLIP:
	  case IDM_SAVETOCLIP:
	  case IDM_SAVETOCLIPFILENAME:
	  case IDM_APPENDTOCLIPFILENAME:
	    ListToClipboardHab(hab2,
			       wk->li->list,
			       wk->li->type);
	    break;

	  case IDM_ARCHIVEM:
	  case IDM_ARCHIVE:
	    {
	      DIRCNRDATA ad;
	      CHAR szBuffer[1025];
	      ARC_TYPE *info = NULL;
	      char *pch;
	      UINT x;

	      memset(&ad, 0, sizeof(DIRCNRDATA));
	      strcpy(ad.arcname, wk->li->targetpath);
	      if (*wk->li->targetpath && IsFile(wk->li->targetpath) > 0) {
		info = find_type(wk->li->targetpath, NULL);
		ad.namecanchange = 0;
	      }
	      else {
		if (*wk->li->targetpath && !IsFile(wk->li->targetpath))
		  AddBackslashToPath(wk->li->targetpath);
		ad.namecanchange = 1;
	      }
	      strcpy(ad.arcname, wk->li->targetpath);
	      if (wk->li->type == IDM_ARCHIVEM)
		ad.fmoving = TRUE;
	      if (!info) {
		ad.info = arcsighead;	// Hide dups
		if (!WinDlgBox(HWND_DESKTOP,
			       wk->hwndFrame,
			       SBoxDlgProc,
			       FM3ModHandle,
			       ASEL_FRAME, (PVOID) & ad.info) || !ad.info) {
		  break;		// we blew it
		}
	      }
	      else
		ad.info = info;
	      if (!ad.info || (!ad.info->create &&
			       !ad.info->move &&
			       !ad.info->createwdirs &&
			       !ad.info->movewdirs &&
			       !ad.info->createrecurse))
		break;
	      if (!*wk->li->targetpath && *wk->directory) {
		strcpy(ad.arcname, wk->directory);
		AddBackslashToPath(ad.arcname);
	      }
	      if (!WinDlgBox(HWND_DESKTOP, wk->hwndFrame, ArchiveDlgProc, FM3ModHandle,
			     ARCH_FRAME, (PVOID) & ad) || !*ad.arcname || !*ad.command)	// we blew it
		break;
	      // Provide extension so containers work
	      pch = strrchr(ad.arcname, '\\');
	      if (pch)
		pch = strrchr(pch, '.');
	      else
		pch = strrchr(ad.arcname, '.');
	      if (!pch && ad.info->ext) {
		strcat(ad.arcname, ".");
		strcat(ad.arcname, ad.info->ext);
	      }
	      // build the sucker
	      strcpy(szBuffer, ad.command);
	      strcat(szBuffer, " ");
	      BldQuotedFileName(szBuffer + strlen(szBuffer), ad.arcname);
	      p = &szBuffer[strlen(szBuffer)];
	      if (ad.mask.szMask) {
		strcat(szBuffer, " ");
		strcat(szBuffer, ad.mask.szMask);
	      }
	      strcat(szBuffer, " ");
	      x = 0;
	      while (wk->li->list[x]) {
		FILESTATUS3 fsa;
		memset(&fsa, 0, sizeof(FILESTATUS3));
		DosError(FERR_DISABLEHARDERR);
		DosQueryPathInfo(wk->li->list[x],
				 FIL_STANDARD,
				 &fsa, (ULONG) sizeof(FILESTATUS3));
		if (fsa.attrFile & FILE_DIRECTORY) {
		  BldQuotedFullPathName(szBuffer + strlen(szBuffer), wk->li->list[x], "*");
		}
		else
		  BldQuotedFileName(szBuffer + strlen(szBuffer), wk->li->list[x]);
		x++;
		if (!wk->li->list[x] ||
		    strlen(szBuffer) + strlen(wk->li->list[x]) + 5 > 1024) {
		  runemf2(SEPARATE | WINDOWED | WAIT |
			  (fArcStuffVisible ? 0 : (BACKGROUND | MINIMIZED)),
			  HWND_DESKTOP, pszSrcFile, __LINE__, NULL, NULL,
			  "%s", szBuffer);
		  DosSleep(1);
		  *p = 0;
		}
		strcat(szBuffer, " ");
	      }
	      Broadcast(hab2,
			wk->hwndCnr,
			UM_UPDATERECORDLIST, MPFROMP(wk->li->list), MPVOID);
	      Broadcast(hab2,
			wk->hwndCnr,
			UM_UPDATERECORD, MPFROMP(ad.arcname), MPVOID);
	    }
	    break;

	  case IDM_VIEW:
	    if (!TestBinary(wk->li->list[0])) {
	      wk->li->type = IDM_VIEWTEXT;
	      goto SkipViewing;
	    }
	    else
	      wk->li->type = IDM_VIEWBINARY;
	    // intentional fallthru
	  case IDM_VIEWBINARY:
	    if (*binview) {
	      ExecOnList((HWND) 0,
			 binview,
			 WINDOWED | SEPARATE, NULL, NULL, wk->li->list, NULL,
			 pszSrcFile, __LINE__);
	      break;
	    }
	    // else intentional fallthru
	  case IDM_VIEWTEXT:
	  SkipViewing:
	    if (*viewer)
	      ExecOnList((HWND) 0, viewer,
			 WINDOWED | SEPARATE |
			 ((fViewChild) ? CHILD : 0),
			 NULL, NULL, wk->li->list, NULL,
			 pszSrcFile, __LINE__);
	    else {

	      CHAR *temp;
	      UINT x;
	      ULONG viewtype;

	      viewtype = (wk->li->type == IDM_VIEWTEXT) ? 8 :
		(wk->li->type == IDM_VIEWBINARY) ? 16 : 0;
	      for (x = 0; wk->li->list[x]; x++) {
		temp = xstrdup(wk->li->list[x], pszSrcFile, __LINE__);
		if (temp && WinIsWindow(hab2, wk->hwndCnr)) {
		  if (!PostMsg(wk->hwndCnr,
			       UM_LOADFILE,
			       MPFROMLONG(5 + viewtype), MPFROMP(temp)))
		    free(temp);
		}
		DosSleep(1);
	      }
	    }
	    break;

	  case IDM_EDIT:
	    if (!TestBinary(wk->li->list[0])) {
	      wk->li->type = IDM_EDITTEXT;
	      goto SkipEditing;
	    }
	    else
	      wk->li->type = IDM_EDITBINARY;
	    // intentional fallthru
	  case IDM_EDITBINARY:
	    if (*bined) {
	      ExecOnList((HWND) 0,
			 bined,
			 WINDOWED | SEPARATE, NULL, NULL, wk->li->list, NULL,
			 pszSrcFile, __LINE__);
	      break;
	    }
	    // else intentional fallthru
	  case IDM_EDITTEXT:
	  SkipEditing:
	    if (*editor)
	      ExecOnList((HWND) 0,
			 editor,
			 WINDOWED | SEPARATE, NULL,  NULL, wk->li->list, NULL,
			 pszSrcFile, __LINE__);
	    else {

	      CHAR *temp;
	      UINT x;
	      ULONG viewtype;

	      viewtype = (wk->li->type == IDM_EDITTEXT) ? 8 :
		(wk->li->type == IDM_EDITBINARY) ? 16 : 0;
	      for (x = 0; wk->li->list[x]; x++) {
		temp = xstrdup(wk->li->list[x], pszSrcFile, __LINE__);
		if (temp && WinIsWindow(hab2, wk->hwndCnr)) {
		  if (!PostMsg(wk->hwndCnr,
			       UM_LOADFILE,
			       MPFROMLONG(4 + viewtype), MPFROMP(temp)))
		    free(temp);
		}
		DosSleep(1);
	      }
	    }
	    break;

	  case IDM_SHADOW2:
	  case IDM_OBJECT:
	  case IDM_SHADOW:
	    {
	      CHAR objectpath[CCHMAXPATH];
	      APIRET rc;

	      if (!*wk->li->targetpath || IsFile(wk->li->targetpath)) {
		GetDesktopName(objectpath, sizeof(objectpath));
		rc = WinDlgBox(HWND_DESKTOP,
			       wk->hwndFrame,
			       ObjCnrDlgProc,
			       FM3ModHandle,
			       OBJCNR_FRAME, MPFROMP(objectpath));
		if (rc) {
		  if (rc > 1)
		    strcpy(objectpath, "<WP_DESKTOP>");
		}
		else
		  break;
	      }
	      else
		strcpy(objectpath, wk->li->targetpath);
	      AddNote(GetPString(IDS_MAKINGOBJSTEXT));
	      MakeShadows(wk->hwndFrame,
			  wk->li->list,
			  (wk->li->type == IDM_SHADOW) +
			  (wk->li->type == IDM_SHADOW2) * 2,
			  objectpath, NULL);
	      AddNote(GetPString(IDS_MADEOBJSTEXT));
	    }
            break;

          case IDM_JAVAEXE:
            {
            CHAR javaexe[CCHMAXPATH] = {0};

            strcpy(javaexe, PCSZ_STARDOTEXE);
            if (insert_filename(HWND_DESKTOP, javaexe, TRUE, FALSE) &&
                *javaexe && !strchr(javaexe, '*') && !strchr(javaexe, '?'))
              PrfWriteProfileString(fmprof, appname, "JavaExe", javaexe);
            }
            break;

	  case IDM_PRINT:
	    if (WinDlgBox(HWND_DESKTOP,
			  wk->hwndFrame,
			  PrintDlgProc,
			  FM3ModHandle, PRN_FRAME, MPFROMP(wk->li))) {
	      if (wk->li && wk->li->list && wk->li->list[0]) {
		strcpy(wk->li->targetpath, printer);
		if (xbeginthread(PrintListThread,
				 65536,
				 wk->li,
				 pszSrcFile,
				 __LINE__) != -1)
		{
		  wk->li = NULL;	// prevent LISTINFO li from being freed here
		}
	      }
	    }
	    break;

	  case IDM_ATTRS:
	    if (WinDlgBox(HWND_DESKTOP,
			  wk->hwndFrame,
			  AttrListDlgProc,
			  FM3ModHandle, ATR_FRAME, MPFROMP(wk->li))) {
	      if (wk->li && wk->li->list && wk->li->list[0])
		Broadcast(hab2,
			  wk->hwndCnr,
			  UM_UPDATERECORDLIST, MPFROMP(wk->li->list), MPVOID);
	    }
	    break;

	  case IDM_PERMDELETE:
	  case IDM_DELETE:
	    {
	      CHECKLIST cl;
	      INT isdir = 0, sysdir = 0, ro = 0, hs = 0;
	      UINT x;
	      FILESTATUS3 fsa;
	      CHAR prompt[CCHMAXPATH * 3];
	      APIRET error = 0, rc;
	      HOBJECT hObjectdest, hObjectofObject;
	      BYTE G_abSupportedDrives[24] = {0};
              ULONG cbSupportedDrives = sizeof(G_abSupportedDrives);
              INT retrn = 0;

	      for (x = 0; wk->li->list[x]; x++) {
		if (IsRoot(wk->li->list[x])) {
		  wk->li->list = RemoveFromList(wk->li->list,
						wk->li->list[x]);
		  if (!wk->li->list)
		    break;
		  x--;
		  continue;
		}
		DosError(FERR_DISABLEHARDERR);
		if (DosQueryPathInfo(wk->li->list[x],
				     FIL_STANDARD, &fsa,
				     (ULONG) sizeof(FILESTATUS3))) {
		  wk->li->list = RemoveFromList(wk->li->list,
						wk->li->list[x]);
		  if (!wk->li->list)
		    break;
		  x--;
		  continue;
		}
		if (fsa.attrFile & FILE_DIRECTORY) {
		  isdir++;
		  if (stristr(wk->li->list[x], ":\\OS2\\") ||
		      !stricmp(wk->li->list[x] + 1, ":\\OS2"))
		    sysdir++;
		}
		else {
		  if (fsa.attrFile & (FILE_HIDDEN | FILE_SYSTEM))
		    hs++;
		  if (fsa.attrFile & FILE_READONLY)
		    ro++;
		}
	      }
	      if (!wk->li->list)
		break;
	      if (fConfirmDelete || isdir || hs || ro) {
		memset(&cl, 0, sizeof(cl));
		cl.size = sizeof(cl);
		cl.list = wk->li->list;
		cl.prompt = prompt;
		cl.flags |= CHECK_FILES;
		cl.cmd = wk->li->type;
		sprintf(prompt,
			GetPString(IDS_DELPROMPT1TEXT),
			(wk->li->type == IDM_DELETE) ?
			NullStr :
			GetPString(IDS_PERMANENTLYTEXT),
			&"s"[wk->li->list[1] == NULL]);
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
		  DosBeep(300, 100);	// Wake up user
		strcat(prompt, GetPString(IDS_DELPROMPT6TEXT));
		error = WinDlgBox(HWND_DESKTOP,
				  wk->hwndFrame,
				  CheckListProc,
				  FM3ModHandle, CHECK_FRAME, MPFROMP(&cl));
		if (!error || error == 65535)
		  break;
		wk->li->list = cl.list;
		if (!wk->li->list || !wk->li->list[0])
		  break;
	      }
	      if (fVerify && driveflags[toupper(*wk->li->list[0]) - 'A'] & DRIVE_WRITEVERIFYOFF)
		DosSetVerify(FALSE);
	      DosRequestMutexSem(hmtxFM2Delete, SEM_INDEFINITE_WAIT); // Prevent race 12-3-08 GKY
	      for (x = 0; wk->li->list[x]; x++) {
		fsa.attrFile = 0;
		DosError(FERR_DISABLEHARDERR);
		DosQueryPathInfo(wk->li->list[x],
				 FIL_STANDARD,
				 &fsa, (ULONG) sizeof(FILESTATUS3));
		if (fsa.attrFile & FILE_DIRECTORY) {
		  error = (APIRET) wipeallf(ignorereadonly, "%s%s*",
					    wk->li->list[x],
					    (*wk->li->list[x] &&
					     wk->li->
					     list[x][strlen(wk->li->list[x]) - 1]
					     != '\\') ? PCSZ_BACKSLASH : NullStr);
		  DosError(FERR_DISABLEHARDERR);
		  if (!error)
		    error = DosDeleteDir(wk->li->list[x]);
		  else
		    DosDeleteDir(wk->li->list[x]);
		}
		else {

		  DosError(FERR_DISABLEHARDERR);
		  if (wk->li->type == IDM_DELETE) {
		    if (fTrashCan) {
                        hObjectdest = WinQueryObject("<XWP_TRASHCAN>");
                        rc = PrfQueryProfileData(HINI_USER,
                                            "XWorkplace",
                                            "TrashCan::Drives",
                                            G_abSupportedDrives,
                                            &cbSupportedDrives);
                        if (hObjectdest != NULLHANDLE &&
                            (rc ? (G_abSupportedDrives[toupper(*wk->li->list[x]) - 'C'] & 1)
                             :((driveflags[toupper(*wk->li->list[x]) - 'A'] &
                                DRIVE_LOCALHD )))) {
                          hObjectofObject = WinQueryObject(wk->li->list[x]);
                          retrn = make_deleteable(wk->li->list[x], error, ignorereadonly);
                          if (retrn == SM2_CANCEL)
                            break;
                          if (retrn == SM2_DONTASK)
                            ignorereadonly = TRUE;
                          if (retrn == SM2_NO)
                            continue;
                          error = WinMoveObject(hObjectofObject, hObjectdest, 0);
                        }
                        else
                          error = DosDelete(wk->li->list[x]);
                      }
                      else
                        error = DosDelete(wk->li->list[x]);
		  }
		  else
		    error = DosForceDelete(wk->li->list[x]);
                  if (error) {

		    DosError(FERR_DISABLEHARDERR);
                    retrn = make_deleteable(wk->li->list[x], error, ignorereadonly);
                    if (retrn == SM2_CANCEL)
                      break;
                    if (retrn == SM2_DONTASK)
                      ignorereadonly = TRUE;
                    if (retrn == SM2_NO)
                      continue;
                    if (wk->li->type == IDM_DELETE) {
                      if (fTrashCan) {
                        hObjectdest = WinQueryObject("<XWP_TRASHCAN>");
                        rc = PrfQueryProfileData(HINI_USER,
                                            "XWorkplace",
                                            "TrashCan::Drives",
                                            G_abSupportedDrives,
                                            &cbSupportedDrives);
                        if (hObjectdest != NULLHANDLE &&
                            (rc ? (G_abSupportedDrives[toupper(*wk->li->list[x]) - 'C'] & 1)
                             :((driveflags[toupper(*wk->li->list[x]) - 'A'] &
                                DRIVE_LOCALHD )))) {
                            hObjectofObject = WinQueryObject(wk->li->list[x]);
                            error = WinMoveObject(hObjectofObject, hObjectdest, 0);
                        }
                        else
                          error = DosDelete(wk->li->list[x]);
                      }
                      else
                        error = DosDelete(wk->li->list[x]);
		    }
		    else
		      error = xDosForceDelete(wk->li->list[x]);
		  }
		  DosReleaseMutexSem(hmtxFM2Delete);
                }
                //DbgMsg(pszSrcFile, __LINE__, "error %i retrn %i", error, retrn);
                if (fWarnReadOnly && error ==  ERROR_FILE_EXISTS) {
                  retrn = SM2_NO;
                }
                if (error && (retrn == SM2_YES || retrn == SM2_DONTASK || retrn == -1))  {
		  if (LogFileHandle)
		    fprintf(LogFileHandle,
			    GetPString(IDS_DELETEFAILED1TEXT),
			    wk->li->list[x], error);
		  if (Dos_Error(MB_ENTERCANCEL,
				error,
				wk->hwndFrame,
				pszSrcFile,
				__LINE__,
				GetPString(IDS_DELETEFAILED2TEXT),
				wk->li->list[x]) == MBID_CANCEL) {
		    DosSetVerify(fVerify);
		    break;
		  }
		}
		else {
		  if (LogFileHandle)
		    fprintf(LogFileHandle,
			    GetPString(IDS_DELETEDTEXT), wk->li->list[x]);
		  sprintf(prompt,
			  GetPString(IDS_DELETEDTEXT), wk->li->list[x]);
		  AddNote(prompt);
		}
		if (//fSyncUpdates ||
		    AddToList(wk->li->list[x], &files, &numfiles, &numalloc)) {
		  Broadcast(hab2,
			    wk->hwndCnr,
			    UM_UPDATERECORD,
			    MPFROMP(wk->li->list[x]), MPVOID);
                } ;
	      } // for
	    }
	    if (fVerify)
	      DosSetVerify(fVerify);
	    break;
	  } // switch
	  if (files) {
	    Broadcast(hab2,
		      wk->hwndCnr,
		      UM_UPDATERECORDLIST, MPFROMP(files), MPVOID);
	    FreeList(files);
	  }
	  if (WinIsWindow(hab2, wk->hwndCnr))
	    PostMsg(wk->hwndCnr, UM_RESCAN, MPVOID, MPVOID);

	  WinDestroyMsgQueue(hmq2);
	}
	DecrThreadUsage();
	WinTerminate(hab2);
      }
    }
    FreeListInfo(wk->li);
    free(wk);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
    DosPostEventSem(CompactSem);
  }
}

#pragma alloc_text(MASSACTION,MassAction)
#pragma alloc_text(ACTION,Action)
#pragma alloc_text(UNDO,FreeUndo,Undo)
