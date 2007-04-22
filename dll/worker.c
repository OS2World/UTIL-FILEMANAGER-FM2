
/***********************************************************************

  $Id$

  Worker thread

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2007 Steven H. Levine

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

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSERRORS
#include <os2.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <share.h>
#include <time.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(MASSACTION,MassAction)
#pragma alloc_text(ACTION,Action)
#pragma alloc_text(UNDO,FreeUndo,Undo)

BOOL APIENTRY WinQueryObjectPath(HOBJECT hobject,
				 PSZ pszPathName, ULONG ulSize);

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
    case IDM_MOVE case IDM_COPY:
    case IDM_EXTRACT:
      {
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
	    if (_beginthread(MassAction, NULL, 122880, (PVOID) wk) == -1) {
	      Runtime_Error(pszSrcFile, __LINE__,
			    GetPString(IDS_COULDNTSTARTTHREADTEXT));
	      FreeListInfo(wk->li);
	      free(wk);
	    }
	  }
	  else
	    FreeListInfo(li);
	}
      }
      break;
    }
  }
  FreeUndo();
}

#endif // defined(UNDO)

VOID Action(VOID * args)
{
  WORKER *wk = (WORKER *) args;
  HAB hab2;
  HMQ hmq2;
  CHAR **files = NULL;
  INT numfiles = 0, numalloc = 0, plen = 0;
  CHAR *p, *pp;

  if (wk) {
    if (wk->li && wk->li->list && wk->li->list[0]) {
      hab2 = WinInitialize(0);
      if (hab2) {
	hmq2 = WinCreateMsgQueue(hab2, 0);
	if (hmq2) {
	  CHAR message[(CCHMAXPATH * 2) + 80], wildname[CCHMAXPATH];
	  register INT x;
	  BOOL dontask = FALSE, wildcarding = FALSE, overold =
	    FALSE, overnew = FALSE, usedtarget;

	  WinCancelShutdown(hmq2, TRUE);
	  IncrThreadUsage();
	  *wildname = 0;
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
	      strcat(wk->li->targetpath, "\\");
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
	  if (wk->li && wk->li->list && wk->li->list[0]) {
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
		  in = _fsopen(wk->li->list[x], moder, SH_DENYWR);
		  if (!in) {
		    if (saymsg(MB_ENTERCANCEL,
			       HWND_DESKTOP,
			       GetPString(IDS_MERGEERRORTEXT),
			       GetPString(IDS_CANTOPENINPUTTEXT),
			       wk->li->list[x]) == MBID_CANCEL)
		      goto Abort;
		  }
		  else {
		    out = _fsopen(wk->li->targetpath, modew, SH_DENYWR);
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
		    if (fSyncUpdates ||
			AddToList(outname, &files, &numfiles, &numalloc))
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

		  memset(&ex, 0, sizeof(EXTRDATA));
		  ex.info = find_type(wk->li->list[x], NULL);
		  if (!ex.info || (!ex.info->extract && !ex.info->exwdirs))
		    break;
		  ex.size = sizeof(EXTRDATA);
		  ex.arcname = wk->li->list[x];
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
		  if (!runemf2(SEPARATE | WINDOWED |
			       ((fArcStuffVisible) ? 0 :
				(BACKGROUND | MINIMIZED)),
			       HWND_DESKTOP,
			       ex.extractdir,
			       NULL,
			       "%s %s %s%s%s",
			       ex.command,
			       ex.arcname,
			       (maskspaces) ? "\"" : NullStr,
			       (*ex.masks) ? ex.masks : "*",
			       (maskspaces) ? "\"" : NullStr) &&
		      !stricmp(ex.extractdir, wk->directory)) {
		    if (WinIsWindow(hab2, wk->hwndCnr))
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
		    if (fSyncUpdates ||
			AddToList(wk->li->list[x],
				  &files, &numfiles, &numalloc))
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
		/* else intentional fallthru */
	      case IDM_OPENDEFAULT:
	      case IDM_OPENSETTINGS:
		{
		  CHAR *s;

		  switch (wk->li->type) {
		  case IDM_OPENICON:
		    s = "ICON";
		    break;
		  case IDM_OPENDETAILS:
		    s = "DETAILS";
		    break;
		  case IDM_OPENTREE:
		    s = "TREE";
		    break;
		  case IDM_OPENSETTINGS:
		    s = Settings;
		    break;
		  default:
		    s = Default;
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
		  if (driveflags[toupper(*wk->li->targetpath) - 'A'] &
		      DRIVE_NOTWRITEABLE) {
		    saymsg(MB_CANCEL,
			   wk->hwndFrame,
			   GetPString(IDS_ERRORTEXT),
			   "%s", GetPString(IDS_NOTWRITENOTARGETTEXT));
		    goto RetryPath;
		  }
		}
	      Retry:
		{
		  CHAR newname[CCHMAXPATH], *moving, *move, *moved;
		  APIRET rc;
		  INT type;
		  FILESTATUS4 fs4;
		  BOOL isnewer, existed;

		  type = (wk->li->type == IDM_RENAME) ? MOVE :
		    (wk->li->type == IDM_MOVE) ? MOVE :
		    (wk->li->type == IDM_WPSMOVE) ? WPSMOVE :
		    (wk->li->type == IDM_WPSCOPY) ? WPSCOPY : COPY;
		  moving = (wk->li->type == IDM_RENAME) ?
		    GetPString(IDS_RENAMINGTEXT) :
		    (wk->li->type == IDM_MOVE ||
		     wk->li->type == IDM_WPSMOVE) ?
		    GetPString(IDS_MOVINGTEXT) : GetPString(IDS_COPYINGTEXT);
		  move = (wk->li->type == IDM_RENAME) ?
		    GetPString(IDS_RENAMETEXT) :
		    (wk->li->type == IDM_MOVE ||
		     wk->li->type == IDM_WPSMOVE) ?
		    GetPString(IDS_MOVETEXT) : GetPString(IDS_COPYTEXT);
		  moved = (wk->li->type == IDM_RENAME) ?
		    GetPString(IDS_RENAMEDTEXT) :
		    (wk->li->type == IDM_MOVE ||
		     wk->li->type == IDM_WPSMOVE) ?
		    GetPString(IDS_MOVEDTEXT) : GetPString(IDS_COPIEDTEXT);
		  if (*wk->li->targetpath) {
		    strcpy(newname, wk->li->targetpath);
		    if (newname[strlen(newname) - 1] != '\\')
		      strcat(newname, "\\");
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
		      *wildname) {

		    CHAR testname[CCHMAXPATH];

		    strcpy(testname, wildname);
		    if (AdjustWildcardName(newname, testname))
		      strcpy(newname, testname);
		  }
		  existed = (IsFile(newname) != -1);
		  isnewer = IsNewer(wk->li->list[x], newname);
		  /*
		     {
		     char temp[CCHMAXPATH * 3];
		     sprintf(temp,"Target: %s\rSource: %s\rOverold: %lu\rOvernew: %lu\rIsNewer: %lu\rExisted: %lu",newname,wk->li->list[x],overold,overnew,isnewer,existed);
		     saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,temp);
		     }
		   */
		  if (existed && wk->li->type != IDM_RENAME && dontask) {
		    if (!overold && !overnew)
		      break;
		    if (!overold && !isnewer)
		      break;
		    if (!overnew && isnewer)
		      break;
		  }
		  if ((wk->li->type == IDM_RENAME &&
		       (!dontask || !*wildname)) ||
		      (!dontask && existed) ||
		      (!dontask && wildcarding) ||
		      (IsFile(newname) == 0 && IsFile(wk->li->list[x]) > 0)) {

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
		    DosSleep(1L);
		    if (mv.skip || !*mv.target)
		      break;
		    if (mv.dontask)
		      dontask = TRUE;
		    if (mv.overold)
		      overold = TRUE;
		    if (mv.overnew)
		      overnew = TRUE;
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
			  " %s \"%s\" %s\"%s\"%s",
			  moving,
			  wk->li->list[x],
			  GetPString(IDS_TOTEXT),
			  newname,
			  (usedtarget) ? GetPString(IDS_TOTARGETTEXT) :
			  NullStr);
		  AddNote(message);
		  if (plen) {
		    /* make directory/ies, if required */

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
		  rc = docopyf(type, wk->li->list[x], "%s", newname);
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
			&& !DosQueryPathInfo(wk->li->list[x], FIL_QUERYEASIZE,
					     &fs4, sizeof(fs4))
			&& !(fs4.attrFile & FILE_DIRECTORY)) {

		      FSALLOCATE fsa;
		      ULONG clFreeBytes;
		      CHAR *ptr;
		      INT cntr;

		      Notify(GetPString(IDS_FITTINGTEXT));
		      DosError(FERR_DISABLEHARDERR);
		      if (!DosQueryFSInfo(toupper(*newname) - '@',
					  FSIL_ALLOC,
					  &fsa, sizeof(FSALLOCATE))) {
			// Assume <2GB since file did not fit
			clFreeBytes = fsa.cUnitAvail * fsa.cSectorUnit *
			  fsa.cbSector;
			if (clFreeBytes) {
			  // Find item that will fit in available space
			  for (cntr = x + 1; wk->li->list[cntr]; cntr++) {
			    DosError(FERR_DISABLEHARDERR);
			    if (!DosQueryPathInfo(wk->li->list[cntr],
						  FIL_QUERYEASIZE,
						  &fs4,
						  sizeof(fs4)) &&
				!(fs4.attrFile & FILE_DIRECTORY) &&
				// fixme to use CBLIST_TO_EASIZE?
				fs4.cbFile + fs4.cbList <= clFreeBytes) {
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
				  "%s", GetPString(IDS_ANOTHERDISKTEXT));
		      if (rc == MBID_RETRY)
			goto Retry;
		      if (rc == MBID_ABORT)
			goto Abort;
		    }
		    else {
		      if (LogFileHandle)
			fprintf(LogFileHandle,
				GetPString(IDS_LOGTOFAILEDTEXT),
				move, wk->li->list[x], newname, rc);
		      rc = Dos_Error(MB_ENTERCANCEL,
				     rc,
				     wk->hwndFrame,
				     pszSrcFile,
				     __LINE__,
				     "%s %s \"%s\" %s\"%s\" %s.",
				     move,
				     GetPString(IDS_OFTEXT),
				     wk->li->list[x],
				     GetPString(IDS_TOTEXT),
				     newname, GetPString(IDS_FAILEDTEXT));
		      if (rc == MBID_CANCEL)
			goto Abort;
		    }
		  }
		  else {
		    if (LogFileHandle)
		      fprintf(LogFileHandle,
			      "%s \"%s\" %s\"%s\"\n",
			      moved,
			      wk->li->list[x],
			      GetPString(IDS_TOTEXT), newname);
		    if (fSyncUpdates ||
			AddToList(wk->li->list[x],
				  &files, &numfiles, &numalloc))
		      Broadcast(hab2,
				wk->hwndCnr,
				UM_UPDATERECORD,
				MPFROMP(wk->li->list[x]), MPVOID);
		    if (fSyncUpdates ||
			AddToList(newname, &files, &numfiles, &numalloc))
		      Broadcast(hab2,
				wk->hwndCnr,
				UM_UPDATERECORD, MPFROMP(newname), MPVOID);
		  }
		}
		break;

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
		    CHAR d1[] = "\"";
		    CHAR d2[] = "\"";

		    if (!needs_quoting(wk->li->targetpath))
		      *d1 = 0;
		    if (!needs_quoting(wk->li->list[x]))
		      *d2 = 0;
		    runemf2(SEPARATE,
			    HWND_DESKTOP,
			    NULL,
			    NULL,
			    "%s %s%s%s %s%s%s",
			    dircompare,
			    d1,
			    wk->li->targetpath, d1, d2, wk->li->list[x], d2);
		  }
		}
		else if (*compare) {
		  CHAR *fakelist[3];

		  fakelist[0] = wk->li->list[x];
		  fakelist[1] = wk->li->targetpath;
		  fakelist[2] = NULL;
		  ExecOnList(wk->hwndFrame,
			     compare,
			     WINDOWED | SEPARATEKEEP, NULL, fakelist, NULL);
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
	      }				// switch
	      DosSleep(0L);
	    }				// for list

	    switch (wk->li->type) {
	    case IDM_MOVE:
	    case IDM_COPY:
	    case IDM_WPSMOVE:
	    case IDM_WPSCOPY:
	    case IDM_RENAME:
	      sprintf(message,
		      GetPString(IDS_OPSCOMPLETETEXT),
		      (wk->li->type == IDM_MOVE) ?
		      GetPString(IDS_MOVETEXT) :
		      (wk->li->type == IDM_COPY) ?
		      GetPString(IDS_COPYTEXT) :
		      (wk->li->type == IDM_WPSMOVE) ?
		      GetPString(IDS_WPSMOVETEXT) :
		      (wk->li->type == IDM_WPSCOPY) ?
		      GetPString(IDS_WPSCOPYTEXT) :
		      GetPString(IDS_RENAMETEXT),
		      &"s"[x == 1],
		      (wk->li->type == IDM_MOVE ||
		       wk->li->type == IDM_COPY ||
		       wk->li->type == IDM_WPSMOVE ||
		       wk->li->type == IDM_WPSCOPY) ?
		      GetPString(IDS_TOTEXT) :
		      NullStr,
		      (wk->li->type == IDM_MOVE ||
		       wk->li->type == IDM_COPY ||
		       wk->li->type == IDM_WPSMOVE ||
		       wk->li->type == IDM_WPSCOPY) ?
		      wk->li->targetpath :
		      NullStr,
		      (x != 1) ?
		      GetPString(IDS_ARETEXT) : GetPString(IDS_ISTEXT));
	      Notify(message);
	      if (toupper(*wk->li->targetpath) < 'C')
		DosBeep(1000, 25);	// Wake up user
	      DosSleep(33L);
	      if (wk->li->type == IDM_WPSMOVE || wk->li->type == IDM_WPSCOPY)
		DosSleep(96L);
	      break;
	    default:
	      break;
	    }
	  }

	Abort:

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

    if (wk->li)
      FreeListInfo(wk->li);
    free(wk);
    DosPostEventSem(CompactSem);
  }
}

VOID MassAction(VOID * args)
{
  WORKER *wk = (WORKER *) args;
  HAB hab2;
  HMQ hmq2;
  CHAR **files = NULL;
  register CHAR *p, *pp;
  INT numfiles = 0, numalloc = 0;

  if (wk) {
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
	    /* else intentional fallthru */
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
		       WINDOWED | SEPARATE | PROMPT,
		       NULL, wk->li->list, GetPString(IDS_DOITYOURSELFTEXT));
	    break;

	  case IDM_MCIPLAY:
	    {
	      register INT x;
              register ULONG total;
              CHAR fbuf[CCHMAXPATH];

              if (DosSearchPath(SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT |
	                        SEARCH_CUR_DIRECTORY,
                                "PATH", "FM2PLAY.EXE", fbuf, CCHMAXPATH - 1))
                total += strlen("..\\FM2UTILS\\FM2PLAY.EXE ");
              else
                total = strlen(fbuf);
	      for (x = 0; wk->li->list[x]; x++)
		total += (strlen(wk->li->list[x]) + 1 +
			  (needs_quoting(wk->li->list[x]) * 2));
	      if (total > 1000) {

		FILE *fp;

		fp = xfopen("$FM2PLAY.$$$", "w", pszSrcFile, __LINE__);
		if (fp) {
		  fprintf(fp, "%s", ";FM/2-built FM2Play listfile\n");
		  for (x = 0; wk->li->list[x]; x++)
		    fprintf(fp, "%s\n", wk->li->list[x]);
		  fprintf(fp, ";end\n");
                  fclose(fp);
                  RunFM2Util("FM2PLAY.EXE", "/#$FM2PLAY.$$$");
		  break;
		}
	      }
	    }
	    /* intentional fallthru */
	  case IDM_FAKEEXTRACT:
	  case IDM_FAKEEXTRACTM:
	    if (wk->li->type == IDM_MCIPLAY ||
		(*wk->li->arcname && wk->li->info &&
		 wk->li->info->extract && *wk->li->targetpath)) {

              CHAR szBuffer[1025];
              CHAR fbuf[CCHMAXPATH];
	      register INT x;

	      if (wk->li->type == IDM_FAKEEXTRACT ||
		  wk->li->type == IDM_FAKEEXTRACTM) {
		strcpy(szBuffer,
		       (wk->li->info->exwdirs) ?
		       wk->li->info->exwdirs : wk->li->info->extract);
		strcat(szBuffer, " ");
		if (needs_quoting(wk->li->arcname))
		  strcat(szBuffer, "\"");
		strcat(szBuffer, wk->li->arcname);
		if (needs_quoting(wk->li->arcname))
		  strcat(szBuffer, "\"");
	      }
	      else {
                if (DosSearchPath(SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT |
		                  SEARCH_CUR_DIRECTORY,
                                  "PATH", "FM2PLAY.EXE", fbuf, CCHMAXPATH - 1))
		  strcpy(szBuffer, "UTILS\\FM2PLAY.EXE");
		else
		  strcpy(szBuffer, "FM2PLAY.EXE");
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
		if (needs_quoting(wk->li->list[x]))
		  strcat(szBuffer, "\"");
		strcat(szBuffer, wk->li->list[x]);
		if (needs_quoting(wk->li->list[x]))
		  strcat(szBuffer, "\"");
		x++;
		if (!wk->li->list[x] || strlen(szBuffer) +
		    strlen(wk->li->list[x]) + 5 > 1024) {
		  runemf2(SEPARATE | WINDOWED | BACKGROUND | MINIMIZED | WAIT,
			  HWND_DESKTOP,
			  ((wk->li->type == IDM_FAKEEXTRACT ||
			    wk->li->type == IDM_FAKEEXTRACTM) ?
			   wk->li->targetpath : NULL), NULL, "%s", szBuffer);
		  DosSleep(1L);
		  *p = 0;
		}
		strcat(szBuffer, " ");
	      }
	      if (wk->li->type == IDM_MCIPLAY)
		break;
	      strcpy(szBuffer, wk->li->targetpath);
	      if (wk->li->targetpath[strlen(wk->li->targetpath) - 1] != '\\')
		strcat(szBuffer, "\\");
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
	    ListToClipboardHab(hab2,
			       wk->li->list,
			       (wk->li->type == IDM_APPENDTOCLIP));
	    break;

	  case IDM_ARCHIVEM:
	  case IDM_ARCHIVE:
	    {
	      DIRCNRDATA ad;
	      CHAR szBuffer[1025];
	      ARC_TYPE *info = NULL;
	      char *pch;
	      register INT x;

	      memset(&ad, 0, sizeof(DIRCNRDATA));
	      strcpy(ad.arcname, wk->li->targetpath);
	      if (*wk->li->targetpath && IsFile(wk->li->targetpath) > 0) {
		info = find_type(wk->li->targetpath, NULL);
		ad.namecanchange = 0;
	      }
	      else {
		if (*wk->li->targetpath && !IsFile(wk->li->targetpath))
		  if (wk->li->targetpath[strlen(wk->li->targetpath) - 1] !=
		      '\\')
		    strcat(wk->li->targetpath, "\\");
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
		  break;		/* we blew it */
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
		if (ad.arcname[strlen(ad.arcname) - 1] != '\\')
		  strcat(ad.arcname, "\\");
	      }
	      if (!WinDlgBox(HWND_DESKTOP, wk->hwndFrame, ArchiveDlgProc, FM3ModHandle, ARCH_FRAME, (PVOID) & ad) || !*ad.arcname || !*ad.command)	/* we blew it */
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
	      /* build the sucker */
	      strcpy(szBuffer, ad.command);
	      strcat(szBuffer, " ");
	      if (needs_quoting(ad.arcname))
		strcat(szBuffer, "\"");
	      strcat(szBuffer, ad.arcname);
	      if (needs_quoting(ad.arcname))
		strcat(szBuffer, "\"");
	      p = &szBuffer[strlen(szBuffer)];
	      if (ad.mask.szMask) {
		strcat(szBuffer, " ");
		strcat(szBuffer, ad.mask.szMask);
	      }
	      strcat(szBuffer, " ");
	      x = 0;
	      while (wk->li->list[x]) {

		FILESTATUS3 fsa;
		BOOL spaces;

		if (needs_quoting(wk->li->list[x])) {
		  spaces = TRUE;
		  strcat(szBuffer, "\"");
		}
		else
		  spaces = FALSE;
		strcat(szBuffer, wk->li->list[x]);
		memset(&fsa, 0, sizeof(FILESTATUS3));
		DosError(FERR_DISABLEHARDERR);
		DosQueryPathInfo(wk->li->list[x],
				 FIL_STANDARD,
				 &fsa, (ULONG) sizeof(FILESTATUS3));
		if (fsa.attrFile & FILE_DIRECTORY) {
		  if (szBuffer[strlen(szBuffer) - 1] != '\\')
		    strcat(szBuffer, "\\");
		  strcat(szBuffer, "*");
		}
		if (spaces)
		  strcat(szBuffer, "\"");
		x++;
		if (!wk->li->list[x] ||
		    strlen(szBuffer) + strlen(wk->li->list[x]) + 5 > 1024) {
		  runemf2(SEPARATE | WINDOWED |
			  ((fArcStuffVisible) ? 0 :
			   (BACKGROUND | MINIMIZED)) |
			  WAIT, HWND_DESKTOP, NULL, NULL, "%s", szBuffer);
		  DosSleep(1L);
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
	    /* intentional fallthru */
	  case IDM_VIEWBINARY:
	    if (*binview) {
	      ExecOnList((HWND) 0,
			 binview,
			 WINDOWED | SEPARATE, NULL, wk->li->list, NULL);
	      break;
	    }
	    /* else intentional fallthru */
	  case IDM_VIEWTEXT:
	  SkipViewing:
	    if (*viewer)
	      ExecOnList((HWND) 0, viewer,
			 WINDOWED | SEPARATE |
			 ((fViewChild) ? CHILD : 0),
			 NULL, wk->li->list, NULL);
	    else {

	      CHAR *temp;
	      register INT x;
	      ULONG viewtype;

	      viewtype = (wk->li->type == IDM_VIEWTEXT) ? 8 :
		(wk->li->type == IDM_VIEWBINARY) ? 16 : 0;
	      for (x = 0; wk->li->list[x]; x++) {
		temp = xstrdup(wk->li->list[x], pszSrcFile, __LINE__);
		if (temp && WinIsWindow(hab2, wk->hwndCnr)) {
		  if (!PostMsg(wk->hwndCnr,
			       UM_LOADFILE,
			       MPFROMLONG(5L + viewtype), MPFROMP(temp)))
		    free(temp);
		}
		DosSleep(1L);
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
	    /* intentional fallthru */
	  case IDM_EDITBINARY:
	    if (*bined) {
	      ExecOnList((HWND) 0,
			 bined,
			 WINDOWED | SEPARATE, NULL, wk->li->list, NULL);
	      break;
	    }
	    /* else intentional fallthru */
	  case IDM_EDITTEXT:
	  SkipEditing:
	    if (*editor)
	      ExecOnList((HWND) 0,
			 editor,
			 WINDOWED | SEPARATE, NULL, wk->li->list, NULL);
	    else {

	      CHAR *temp;
	      register INT x;
	      ULONG viewtype;

	      viewtype = (wk->li->type == IDM_EDITTEXT) ? 8 :
		(wk->li->type == IDM_EDITBINARY) ? 16 : 0;
	      for (x = 0; wk->li->list[x]; x++) {
		temp = xstrdup(wk->li->list[x], pszSrcFile, __LINE__);
		if (temp && WinIsWindow(hab2, wk->hwndCnr)) {
		  if (!PostMsg(wk->hwndCnr,
			       UM_LOADFILE,
			       MPFROMLONG(4L + viewtype), MPFROMP(temp)))
		    free(temp);
		}
		DosSleep(1L);
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

	  case IDM_PRINT:
	    if (WinDlgBox(HWND_DESKTOP,
			  wk->hwndFrame,
			  PrintDlgProc,
			  FM3ModHandle, PRN_FRAME, MPFROMP(wk->li))) {
	      if (wk->li && wk->li->list && wk->li->list[0]) {
		strcpy(wk->li->targetpath, printer);
		if (_beginthread(PrintListThread, NULL, 65536, (PVOID) wk->li)
		    == -1)
		  Runtime_Error(pszSrcFile, __LINE__,
				GetPString(IDS_COULDNTSTARTTHREADTEXT));
		else
		  wk->li = NULL;	/* prevent LISTINFO li from being freed */
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
	      register INT x;
	      FILESTATUS3 fsa;
	      CHAR prompt[CCHMAXPATH * 3];
	      APIRET error;

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
		if (ro || hs || sysdir)
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
	      for (x = 0; wk->li->list[x]; x++) {
		fsa.attrFile = 0;
		DosError(FERR_DISABLEHARDERR);
		DosQueryPathInfo(wk->li->list[x],
				 FIL_STANDARD,
				 &fsa, (ULONG) sizeof(FILESTATUS3));
		if (fsa.attrFile & FILE_DIRECTORY) {
		  sprintf(prompt,
			  GetPString(IDS_DELETINGTEXT), wk->li->list[x]);
		  AddNote(prompt);
		  error = (APIRET) wipeallf("%s%s*",
					    wk->li->list[x],
					    (*wk->li->list[x] &&
					     wk->li->
					     list[x][strlen(wk->li->list[x]) -
						     1] !=
					     '\\') ? "\\" : NullStr);
		  DosError(FERR_DISABLEHARDERR);
		  if (!error)
		    error = DosDeleteDir(wk->li->list[x]);
		  else
		    DosDeleteDir(wk->li->list[x]);
		}
		else {
		  sprintf(prompt,
			  GetPString(IDS_DELETINGTEXT), wk->li->list[x]);
		  AddNote(prompt);
		  DosError(FERR_DISABLEHARDERR);
		  if (wk->li->type == IDM_DELETE)
		    error = DosDelete(wk->li->list[x]);
		  else
		    error = DosForceDelete(wk->li->list[x]);
		  if (error) {
		    DosError(FERR_DISABLEHARDERR);
		    make_deleteable(wk->li->list[x]);
		    if (wk->li->type == IDM_DELETE)
		      error = DosDelete(wk->li->list[x]);
		    else
		      error = DosForceDelete(wk->li->list[x]);
		  }
		}
		if (error) {
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
				wk->li->list[x]) == MBID_CANCEL)
		    break;
		}
		else {
		  if (LogFileHandle)
		    fprintf(LogFileHandle,
			    GetPString(IDS_DELETEDTEXT), wk->li->list[x]);
		  sprintf(prompt,
			  GetPString(IDS_DELETEDTEXT), wk->li->list[x]);
		  AddNote(prompt);
		}
		if (fSyncUpdates ||
		    AddToList(wk->li->list[x], &files, &numfiles, &numalloc))
		  Broadcast(hab2,
			    wk->hwndCnr,
			    UM_UPDATERECORD,
			    MPFROMP(wk->li->list[x]), MPVOID);
	      }
	    }
	    break;
	  } // switch
	  if (files) {
	    Broadcast(hab2,
		      wk->hwndCnr,
		      UM_UPDATERECORDLIST, MPFROMP(files), MPVOID);
	    FreeList(files);
	  }
	Abort:
	  if (WinIsWindow(hab2, wk->hwndCnr))
	    PostMsg(wk->hwndCnr, UM_RESCAN, MPVOID, MPVOID);

	  WinDestroyMsgQueue(hmq2);
	}
	DecrThreadUsage();
	WinTerminate(hab2);
      }
    }
    if (wk->li)
      FreeListInfo(wk->li);
    free(wk);
    DosPostEventSem(CompactSem);
  }
}
