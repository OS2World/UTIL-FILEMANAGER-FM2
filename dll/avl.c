
/***********************************************************************

  $Id$

  archiver.bb2 search, load, save and date parse

  Copyright (c) 1993, 1998 M. Kimes
  Copyright (c) 2004, 2006 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  13 Aug 05 SHL Beautify with indent
  13 Aug 05 SHL find_type: correct no sig exists bypass logic
  13 Aug 05 SHL SBoxDlgProc: avoid dereferencing NULL signature
  18 Aug 05 SHL Comments
  31 Dec 05 SHL indent -i2
  08 Dec 05 SHL load_archivers: allow empty startlist
  30 Dec 05 SHL load_archivers: use get_archiver_line?(), clean nits
  29 May 06 SHL SBoxDlgProc: support move, add, delete
  30 May 06 SHL load_archivers: add reload support
  16 Jun 06 SHL load_archivers: support signatures containing 0s

***********************************************************************/

#define INCL_WIN
#define INCL_WINSTDDRAG
#define INCL_DOS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <share.h>
#include <ctype.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma alloc_text(MISC9,quick_find_type,find_type)

static void free_arc_type(ARC_TYPE *pat);
static void fill_listbox(HWND hwnd, BOOL fShowAll, SHORT sOldSelect);

//=== quick_find_type() ===

ARC_TYPE *quick_find_type(CHAR * filespec, ARC_TYPE * topsig)
{
  ARC_TYPE *info, *found = NULL;
  CHAR *p;

  if (!arcsigsloaded)
    load_archivers();
  p = strrchr(filespec, '.');
  if (p)
  {
    p++;
    info = (topsig) ? topsig : arcsighead;
    while (info)
    {
      if (info -> ext &&
	  *(info -> ext) &&
	  !stricmp(p, info -> ext))
      {
	found = find_type(filespec, topsig);
	break;
      }
      info = info -> next;
    }
  }
  return found;
}

//=== fill_listbox() fill or refill listbox from current archiver definitions ===

static VOID fill_listbox(HWND hwnd, BOOL fShowAll, SHORT sOldSelect)
{
  ARC_TYPE *pat;
  BOOL found = FALSE;
  SHORT sSelect;

  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);

  for (pat = arcsighead; pat; pat = pat -> next)
  {
    /*
     * this inner loop tests for a dup signature entry and assures
     * that only the entry at the top of the list gets used for
     * conversion; editing any is okay
     */
    if (!fShowAll)
    {
      ARC_TYPE *pat2;
      BOOL isDup = FALSE;
      for (pat2 = arcsighead;
	   pat2 && pat -> siglen && pat2 != pat && !isDup;
	   pat2 = pat2 -> next)
      {
	isDup = pat2 -> siglen == pat -> siglen &&
		!memcmp(pat2 -> signature, pat -> signature, pat -> siglen);
      } // for
      if (isDup)
	continue;
    }

    // If caller is editing archivers or entry useful to caller, show in listbox
    if (fShowAll || (pat -> id && pat -> extract && pat -> create))
    {
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_INSERTITEM,
					  MPFROM2SHORT(LIT_END, 0),
					  MPFROMP(pat -> id ?
						  pat -> id : "?"));
      if (!found && *szDefArc && pat -> id && !strcmp(szDefArc, pat -> id))
      {
	// Highlight default
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			  MPFROMSHORT(sSelect), MPFROMSHORT(TRUE));
	found = TRUE;
      }
    }
    else
    {
      // Complain about odd entry
      if (!pat -> id || !*pat -> id)
      {
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0),
			  MPFROMP(GetPString(IDS_UNKNOWNUNUSABLETEXT)));
      }
      else
      {
	CHAR s[81];
	sprintf(s, "%0.12s %s", pat -> id, GetPString(IDS_UNUSABLETEXT));
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0),
			  MPFROMP(s));
      }
    }
  } // while scanning

  // Try to reselect last selection unless user wants default selection
  if (sOldSelect != LIT_NONE && !found) {
    SHORT sItemCount = (SHORT)WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_QUERYITEMCOUNT,
					        MPVOID,MPVOID);
    if (sOldSelect >= sItemCount)
      sOldSelect = sItemCount - 1;
    if (sOldSelect >= 0) {
      WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
		        MPFROMSHORT(sOldSelect), MPFROMSHORT(TRUE));
    }
  }

  if (found)
    PosOverOkay(hwnd);
}

ARC_TYPE *find_type(CHAR * filespec, ARC_TYPE * topsig)
{
  HFILE handle;
  ULONG action;
  ULONG len;
  ULONG l;
  ARC_TYPE *info;
  CHAR *p;
  CHAR buffer[80];

  if (!arcsigsloaded)
    load_archivers();
  if (!topsig)
    topsig = arcsighead;
  DosError(FERR_DISABLEHARDERR);
  if (DosOpen(filespec,
	      &handle,
	      &action,
	      0L,
	      0L,
	      OPEN_ACTION_FAIL_IF_NEW |
	      OPEN_ACTION_OPEN_IF_EXISTS,
	      OPEN_FLAGS_FAIL_ON_ERROR |
	      OPEN_FLAGS_NOINHERIT |
	      OPEN_FLAGS_RANDOMSEQUENTIAL |
	      OPEN_SHARE_DENYNONE |
	      OPEN_ACCESS_READONLY,
	      0L))
    return NULL;
  // Scan signatures
  for (info = topsig; info; info = info -> next)
  {
    if (info -> siglen == 0)
    {
      // No signature -- check extension
      p = strrchr(filespec, '.');
      if (p)
      {
	p++;
	if (info -> ext &&
	    *(info -> ext) &&
	    !stricmp(p, info -> ext))
	  break;			// Matched

      }
      continue;				// Next sig

    }
    // Try signature match
    l = info -> siglen;
    l = min(l, 79);
    if (!DosChgFilePtr(handle,
		       abs(info -> file_offset),
		       (info -> file_offset >= 0L) ?
		       FILE_BEGIN :
		       FILE_END,
		       &len))
    {
      if (!DosRead(handle,
		   buffer,
		   l,
		   &len) &&
	  len == l)
      {
	if (!memcmp(info -> signature,
		    buffer,
		    l))
	  break;			// Matched

      }
    }
  }					// for

  DosClose(handle);			/* Either way, we're done for now */
  return info;				/* Return signature, if any */
}

//=== free_arc_type() free allocated ARC_TYPE ===

static void free_arc_type(ARC_TYPE *pat)
{
  if (pat)
  {
    if (pat -> id)
      free(pat -> id);
    if (pat -> ext)
      free(pat -> ext);
    if (pat -> list)
      free(pat -> list);
    if (pat -> extract)
      free(pat -> extract);
    if (pat -> create)
      free(pat -> create);
    if (pat -> move)
      free(pat -> move);
    if (pat -> delete)
      free(pat -> delete);
    if (pat -> signature)
      free(pat -> signature);
    if (pat -> startlist)
      free(pat -> startlist);
    if (pat -> endlist)
      free(pat -> endlist);
    if (pat -> exwdirs)
      free(pat -> exwdirs);
    if (pat -> test)
      free(pat -> test);
    if (pat -> createrecurse)
      free(pat -> createrecurse);
    if (pat -> createwdirs)
      free(pat -> createwdirs);
    if (pat -> movewdirs)
      free(pat -> movewdirs);
    free(pat);
  }
}

#pragma alloc_text(AVL,load_archivers, get_archiver_line)

//=== get_archiver_line() read line, strip comments and whitespace ===

#define ARCHIVER_LINE_BYTES	256

static PSZ get_archiver_line(PSZ pszIn, FILE * pf)
{
  PSZ psz = fgets(pszIn, ARCHIVER_LINE_BYTES, pf);
  PSZ psz2;

  if (psz)
  {
    psz2 = strchr(pszIn, ';');
    if (psz2)
      *psz2 = 0;			// Chop comment
    bstripcr(pszIn);			// Strip leading white and trailing white and CR/LF

  }
  return psz;
}

//=== get_archiver_line2() read line, strip whitespace ===

static PSZ get_archiver_line2(PSZ pszIn, FILE * pf)
{
  PSZ psz = fgets(pszIn, ARCHIVER_LINE_BYTES, pf);

  if (psz)
    bstripcr(pszIn);			// Strip lead white and trailing white and CR/LF

  return psz;
}

//=== load_archivers() load or reload archive definitions from archiver.bb2 ===

INT load_archivers(VOID)
{
  FILE *handle;
  CHAR s[ARCHIVER_LINE_BYTES + 1];
  CHAR *p;
  ARC_TYPE *pat;
  ARC_TYPE *patLast;
  INT numlines = NUMLINES;
  INT i;

  if (arcsighead) {
    for (pat = arcsighead; pat;) {
      patLast = pat;
      pat = pat->next;
      free_arc_type(patLast);
    }
    arcsighead = NULL;
  }

  arcsigsmodified = FALSE;

  DosEnterCritSec();
  p = searchpath(GetPString(IDS_ARCHIVERBB2));
  if (!p || !*p)
  {
    DosExitCritSec();
    return -1;
  }
  handle = _fsopen(p, "r", SH_DENYWR);
  DosExitCritSec();
  if (!handle)
    return -2;
  strcpy(archiverbb2, p);
  // Get lines per record count
  if (!get_archiver_line(s, handle))
  {
    fclose(handle);
    return -3;
  }
  if (*s)
    numlines = atoi(s);
  if (!*s || numlines < NUMLINES)
    return -3;
  // Parse rest
  pat = NULL;
  patLast = NULL;
  while (!feof(handle))
  {
    if (!get_archiver_line(s, handle))
      break;				// EOF
    // 1st non-blank line starts definition
    // fixme to preserve comments
    // fixme to avoid allocating empty fields

    if (*s)
    {
      pat = malloc(sizeof(ARC_TYPE));
      if (!pat)
	break;				// fixme to complain

      memset(pat, 0, sizeof(ARC_TYPE));
      pat -> id = strdup(s);
      if (!get_archiver_line(s, handle))	// line 2

	break;
      if (*s)
	pat -> ext = strdup(s);
      else
	pat -> ext = NULL;
      if (!get_archiver_line(s, handle))	// line 3

	break;
      pat -> file_offset = atol(s);
      if (!get_archiver_line(s, handle))	// line 4

	break;
      if (*s)
	pat -> list = strdup(s);
      else
	pat -> list = NULL;
      if (!pat -> list)
	break;
      if (!get_archiver_line(s, handle))	// line 5

	break;
      if (*s)
	pat -> extract = strdup(s);
      else
	pat -> extract = NULL;
      if (!get_archiver_line(s, handle))	// line 6

	break;
      if (*s)
	pat -> exwdirs = strdup(s);
      else
	pat -> exwdirs = NULL;
      if (!get_archiver_line(s, handle))	// line 7

	break;
      if (*s)
	pat -> test = strdup(s);
      else
	pat -> test = NULL;
      if (!get_archiver_line(s, handle))	// line 8

	break;
      if (*s)
	pat -> create = strdup(s);
      else
	pat -> create = NULL;
      if (!get_archiver_line(s, handle))	// line 9

	break;
      if (*s)
	pat -> createwdirs = strdup(s);
      else
	pat -> createwdirs = NULL;
      if (!get_archiver_line(s, handle))	// line 10

	break;
      if (*s)
	pat -> createrecurse = strdup(s);
      else
	pat -> createrecurse = NULL;
      if (!get_archiver_line(s, handle))	// line 11

	break;
      if (*s)
	pat -> move = strdup(s);
      else
	pat -> move = NULL;
      if (!get_archiver_line(s, handle))	// line 12

	break;
      if (*s)
	pat -> movewdirs = strdup(s);
      else
	pat -> movewdirs = NULL;
      if (!get_archiver_line(s, handle))	// line 13

	break;
      pat -> delete = strdup(s);

      if (!get_archiver_line2(s, handle))	// line 14
	break;
      i = literal(s);			// Translate \ escapes
      if (i)
      {
	pat -> siglen = i;
	pat -> signature = malloc(i);
	if (!pat -> signature)
	  break;
	memcpy(pat -> signature, s, i);	// signature may not be a string
      }
      else {
	pat -> siglen = 0;
	pat -> signature = NULL;
      }
      if (!get_archiver_line2(s, handle))	// line 15

	break;
      if (*s)
	pat -> startlist = strdup(s);
      else
	pat -> startlist = NULL;
      if (!get_archiver_line2(s, handle))	// line 16

	break;
      if (*s)
	pat -> endlist = strdup(s);
      else
	pat -> endlist = NULL;
      if (!get_archiver_line(s, handle))	// line 17

	break;
      pat -> osizepos = atoi(s);
      if (!get_archiver_line(s, handle))	// line 18

	break;
      pat -> nsizepos = atoi(s);
      if (!get_archiver_line(s, handle))	// line 19

	break;
      pat -> fdpos = atoi(s);
      p = strchr(s, ',');
      if (p)
      {
	p++;
	pat -> datetype = atoi(p);
      }
      if (!get_archiver_line(s, handle))	// line 20

	break;
      pat -> fdflds = atoi(s);
      if (!get_archiver_line(s, handle))	// line 21

	break;
      pat -> fnpos = atoi(s);
      p = strchr(s, ',');
      if (p)
      {
	p++;
	pat -> nameislast = (BOOL) (*p && atol(p) == 0) ? FALSE : TRUE;
	p = strchr(p, ',');
	if (p)
	{
	  p++;
	  pat -> nameisnext = (BOOL) (*p && atol(p) == 0) ? FALSE : TRUE;
	  p = strchr(p, ',');
	  if (p)
	  {
	    p++;
	    pat -> nameisfirst = (BOOL) (*p && atol(p) == 0) ? FALSE : TRUE;
	  }
	}
      }
      // Ignore unknown lines - must be newer file format
      for (i = NUMLINES; i < numlines; i++)
      {
	if (!get_archiver_line(s, handle))
	  break;
      }

      pat -> next = NULL;

      if (!arcsighead)
      {
	arcsighead = patLast = pat;
	pat -> prev = NULL;
      }
      else
      {
	patLast -> next = pat;
	pat -> prev = patLast;
	patLast = pat;
      }
      if (pat -> extract && !*pat -> extract)
      {
	free(pat -> extract);
	pat -> extract = NULL;
      }
    } // if got definition

    pat = NULL;
  } // while lines
  fclose(handle);

  free_arc_type(pat);

  if (!arcsighead)
    return -4;

  arcsigsloaded = TRUE;

  return 0;
}

#define TEST_DRAG 0			// fixme to gone

#pragma alloc_text(FMARCHIVE,SBoxDlgProc,SDlgListboxSubclassProc)

static MRESULT EXPENTRY SDlgListboxSubclassProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PFNWP pfnOldProc = (PFNWP)WinQueryWindowPtr(hwnd, QWL_USER);

  PDRAGITEM pditem;
  PDRAGINFO pdinfo;
  BOOL ok;

  static BOOL emphasized = FALSE;
  static PSZ DRMDRF_LBOX = "<DRM_LBOX,DRF_UNKNOWN>";
  static PSZ DRM_LBOX = "DRM_LBOX";

  switch (msg)
  {
  case WM_BEGINDRAG:
    {
      LONG cur_ndx;
      DRAGITEM ditem;
      DRAGIMAGE dimage;
      HWND hwndDrop;

      fprintf(stderr, "SDlgListboxSubclassProc: BEGINDRAG\n");
      cur_ndx = WinQueryLboxSelectedItem(hwnd);

      if (cur_ndx != LIT_NONE) {
	pdinfo = DrgAllocDraginfo(1);
	if(pdinfo) {
	  pdinfo->usOperation = DO_DEFAULT;
	  pdinfo->hwndSource = hwnd;

	  memset(&ditem,0,sizeof(DRAGITEM));
	  ditem.hwndItem = hwnd;
	  ditem.ulItemID = 1;
	  ditem.hstrType = DrgAddStrHandle(DRT_UNKNOWN);
	  ditem.hstrRMF = DrgAddStrHandle(DRMDRF_LBOX);
	  ditem.hstrContainerName = DrgAddStrHandle("");
	  ditem.hstrSourceName = DrgAddStrHandle("");
	  ditem.hstrTargetName = DrgAddStrHandle("");
	  // ditem.fsControl = 0;
	  ditem.fsSupportedOps = DO_MOVEABLE;

	  memset(&dimage,0,sizeof(DRAGIMAGE));
	  dimage.cb = sizeof(DRAGIMAGE);
	  dimage.hImage = hptrFile;
	  dimage.cptl = 0;
	  dimage.fl = DRG_ICON;
	  dimage.sizlStretch.cx = 32;
	  dimage.sizlStretch.cy = 32;
	  dimage.cxOffset = -16;
	  dimage.cyOffset = 0;
	  DrgSetDragitem(pdinfo,
			 &ditem,
			 sizeof(DRAGITEM),
			 0);		/* Index of DRAGITEM */
	  hwndDrop = DrgDrag(hwnd,
			     pdinfo,
			     &dimage,
			     1,		/* One DRAGIMAGE */
			     VK_ENDDRAG,
			     NULL);
	  if (!hwndDrop) {
	    Win_Error(hwnd,hwnd,__FILE__,__LINE__,"DrgDrag");
	  }

	  DrgFreeDraginfo(pdinfo);
	  // WinSetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_ACTIVATE);
	}
      }
      break;
    }

  case DM_DRAGOVER:
    ok = FALSE;
    if (!emphasized)
    {
      POINTL ptl;
      POINTL ptl2;
      emphasized = TRUE;
      ptl.x = SHORT1FROMMP(mp2);
      ptl.y = SHORT2FROMMP(mp2);
      ptl2 = ptl;
      WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl2, 1);
      fprintf(stderr, "DRAGOVER mapped x y %d %d to %d %d\n", ptl.x, ptl.y, ptl2.x, ptl2.y);
      WinPostMsg(hwnd, WM_BUTTON1CLICK,
		 MPFROM2SHORT((SHORT)ptl2.x, (SHORT)ptl2.y),
		 MPFROM2SHORT(HT_NORMAL, KC_NONE));
      fprintf(stderr, "DRAGOVER posted 0x%x WM_BUTTON1CLICK x y %d %d\n", hwnd, ptl2.x, ptl2.y);
    }
    pdinfo = (PDRAGINFO)mp1;		/* Get DRAGINFO pointer */
    if (pdinfo) {
      DrgAccessDraginfo(pdinfo);
      pditem = DrgQueryDragitemPtr(pdinfo,0);
      /* Check valid rendering mechanisms and data format */
      ok = DrgVerifyRMF(pditem, DRM_LBOX, NULL);
      DrgFreeDraginfo(pdinfo);
      if (ok) {
      }
    }
    return ok ? MRFROM2SHORT(DOR_DROP, DO_MOVE) : MRFROM2SHORT(DOR_NEVERDROP, 0);

  case DM_DRAGLEAVE:
    if (emphasized)
    {
      emphasized = FALSE;
      // fixme to draw listbox item emphasized
      // DrawTargetEmphasis(hwnd, emphasized);
      fprintf(stderr, "DRAGLEAVE\n");
      fflush(stderr);
    }
    return 0;

  case DM_DROPHELP:
    DropHelp(mp1, mp2, hwnd, "fixme to give some help");
    return 0;

  case DM_DROP:
    ok = FALSE;
    fprintf(stderr, "DROP\n");
    fflush(stderr);
    if (emphasized)
    {
      emphasized = FALSE;
      // DrawTargetEmphasis(hwnd, emphasized);
    }
    pdinfo = (PDRAGINFO)mp1;		/* Get DRAGINFO pointer */
    if (pdinfo) {
      DrgAccessDraginfo(pdinfo);
      pditem = DrgQueryDragitemPtr(pdinfo,0);
      if (!pditem)
	Win_Error(hwnd,hwnd,__FILE__,__LINE__,"DM_DROP");
      /* Check valid rendering mechanisms and data */
      ok = DrgVerifyRMF(pditem,DRM_LBOX,NULL) && ~pditem->fsControl & DC_PREPARE;
      if (ok) {
	// ret = FullDrgName(pditem,buffer,buflen);
	/* note: targetfail is returned to source for all items */
	DrgSendTransferMsg(pdinfo->hwndSource,DM_ENDCONVERSATION,
			   MPFROMLONG(pditem->ulItemID),
			   MPFROMLONG(DMFL_TARGETSUCCESSFUL));
      }
      DrgDeleteDraginfoStrHandles(pdinfo);
      DrgFreeDraginfo(pdinfo);
    }
    return 0;
  } // switch
  return pfnOldProc ? pfnOldProc(hwnd, msg, mp1, mp2) :
		      WinDefWindowProc(hwnd, msg, mp1, mp2);
}

//=== SBoxDlgProc() Select archiver to use or edit, supports list reorder too ===

MRESULT EXPENTRY SBoxDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ARC_TYPE **ppatReturn;		// Where to return selected archiver
  ARC_TYPE *pat;
  SHORT sSelect;
  SHORT sItemCount;
  CHAR szItemText[256];
  CHAR szPCItemText[256];		// Parent or child item text
  SHORT i;
  APIRET apiret;
  BOOL fShowAll;

  static SHORT sLastSelect = LIT_NONE;

  switch (msg)
  {
  case WM_INITDLG:
    if (!arcsigsloaded)
      load_archivers();
    if (!(ARC_TYPE **)mp2)
    {
      DosBeep(100, 100);
      WinDismissDlg(hwnd, 0);
      break;
    }
    /* Passed arg points to where to return selected archiver definition
     * On input arg value controls selection list content
     * If non-NULL, dup names are suppressed
     * If NULL, all definitions are shown
     */
    ppatReturn = (ARC_TYPE **)mp2;
    fShowAll = *ppatReturn == NULL;
    if (*ppatReturn)
      *ppatReturn = arcsighead;		// Preset to first
    WinSetWindowPtr(hwnd, QWL_USER, (PVOID)ppatReturn);
    fill_listbox(hwnd, fShowAll, sLastSelect);

#ifdef TEST_DRAG			// fixme
    {
      HWND hwnd2 = WinWindowFromID(hwnd, ASEL_LISTBOX);
      PFNWP pfn = WinSubclassWindow(hwnd2,
				    SDlgListboxSubclassProc);
      WinSetWindowPtr(hwnd2, QWL_USER, (PVOID) pfn);
    }
#endif // TEST_DRAG fixme

    break;

  case WM_COMMAND:
    ppatReturn = (ARC_TYPE **) WinQueryWindowPtr(hwnd, QWL_USER);
    switch (SHORT1FROMMP(mp1))
    {
    case DID_OK:
      sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
					ASEL_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST),
					MPVOID);
      if (sSelect == LIT_NONE)
      {
	DosBeep(100, 100);
	return 0;
      }
      pat = arcsighead;
      if (*ppatReturn)
      {
	// If dups hidden, find archiver with matching id
	*szItemText = 0;
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMTEXT,
			  MPFROM2SHORT(sSelect, 255), MPFROMP(szItemText));
	if (!*szItemText)
	  pat = NULL;
	else
	{
	  for (;pat; pat = pat -> next)
	  {
	    if (pat -> id && !strcmp(szItemText, pat -> id))
		break;		// Found it
	  }
	}
      }
      else
      {
	// If dups not hidden, lookup by count
	for (i = 0; pat && i < sSelect; i++, pat = pat -> next)
	  ; // Scan
      }
      if (pat && (!*ppatReturn ||
		  (pat -> id && pat -> extract && pat -> create)))
      {
	*ppatReturn = pat;
      }
      else
      {
	// Refuse to select
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			  MPFROMSHORT(LIT_NONE), FALSE);
	DosBeep(100, 100);
	return 0;
      }
      sLastSelect = sSelect;
      WinDismissDlg(hwnd, TRUE);
      return 0;

    case DID_CANCEL:
      if (arcsigsmodified) {
	if (saymsg(MB_YESNO,
		   hwnd,
		   GetPString(IDS_ADCHANGESINMEMTEXT),
		   GetPString(IDS_ADREWRITETEXT),
		   NullStr) ==
	    MBID_YES) {
	  PSZ ab2 = searchpath(GetPString(IDS_ARCHIVERBB2));	// Rewrite without prompting
	  rewrite_archiverbb2(ab2);
	}
      }
      sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
					ASEL_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST),
					MPVOID);
      if (sSelect != LIT_NONE)
        sLastSelect = sSelect;
      *ppatReturn = NULL;
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);	// fixme to understand why needed
      return 0;

    case ASEL_PB_ADD:
      sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
					ASEL_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST),
					MPVOID);
      if (sSelect != LIT_NONE) {
	ARCDUMP ad;
	memset(&ad,0,sizeof(ARCDUMP));
	ad.info = malloc(sizeof(ARC_TYPE));
	if (ad.info) {
	  memset(ad.info, 0, sizeof(ARC_TYPE));
	  if (!WinDlgBox(HWND_DESKTOP,
			 hwnd,
			 ArcReviewDlgProc,
			 FM3ModHandle,
			 AD_FRAME,
			 MPFROMP(&ad)))
	  {
	    free(ad.info);
	  }
	  else {
	    // Find self - assume all archivers listed since we are editing
	    for (i = 0, pat = arcsighead; pat && i < sSelect; pat = pat->next, i++)
	      ; // Find self

	    if (!pat) {
	      if (arcsighead)
		saymsg(0,NULLHANDLE,"*Debug*","Can not find self at %d at %s::%u", sSelect, __FILE__, __LINE__);
	      else
		arcsighead = ad.info;
	    }
	    else {
	      // Insert before
	      if (pat->prev) {
		ad.info->next = pat;
		ad.info->prev = pat->prev;
		pat->prev->next = ad.info;
		pat->prev = ad.info;
	      }
	      else {
		arcsighead = ad.info;
		ad.info->next = pat;
		pat->prev = ad.info;
	      }
	    }
	    WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_INSERTITEM,
			      MPFROM2SHORT(sSelect, 0),
			      MPFROMP(ad.info -> id ? ad.info -> id : "?"));
	    WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			      MPFROMSHORT(sSelect - 1), MPFROMSHORT(TRUE));
	    arcsigsmodified = TRUE;
	  }
	}
      }
      return 0;
    case ASEL_PB_DELETE:
      sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
					ASEL_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST),
					MPVOID);
      if (sSelect != LIT_NONE) {
	// Find self - assume all archivers listed since we are editing
	for (i = 0, pat = arcsighead; pat && i < sSelect; pat = pat->next, i++)
	  ; // Find self

	if (!pat) {
	  saymsg(0,NULLHANDLE,"*Debug*","Can not find self at %d at %s::%u",sSelect, __FILE__, __LINE__);
	}
	else {
	  // Delete current
	  if (pat->prev) {
	    pat->prev->next = pat->next;
	    if (pat->next)
	      pat->next->prev = pat->prev;
	  }
	  else {
	    arcsighead = pat->next;
	    if (pat->next)
	      pat->next->prev = pat->prev;
	  }
	}
	free_arc_type(pat);
	arcsigsmodified = TRUE;
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_DELETEITEM,
			  MPFROM2SHORT(sSelect, 0),
			  MPVOID);
        sItemCount = (SHORT)WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_QUERYITEMCOUNT,
					      MPVOID,MPVOID);
	if (sSelect >= sItemCount)
	  sSelect--;
	if (sSelect >= 0) {
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			    MPFROMSHORT(sSelect), MPFROMSHORT(TRUE));
	}
      }
      return 0;
    case ASEL_PB_UP:
      sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
					ASEL_LISTBOX,
					LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST),
					MPVOID);
      if (sSelect != LIT_NONE && sSelect > 0) {
	// Find self - assume all archivers listed since we are editing
	for (i = 0, pat = arcsighead; pat && i < sSelect; pat = pat->next, i++)
	  ; // Find self
	if (!pat || !pat->prev) {
	  saymsg(0,NULLHANDLE,"*Debug*","Can not find self at %d at %s::%u",sSelect, __FILE__, __LINE__);
	}
	else {
	  ARC_TYPE *patGDad;
	  ARC_TYPE *patDad;
	  ARC_TYPE *patChild;
	  patChild = pat->next;
	  patDad = pat->prev;
	  patGDad = patDad->prev;
	  patDad->next = patChild;
	  if (patChild)
	    patChild->prev = patDad;
	  patDad->prev = pat;
	  pat->next = patDad;
	  if (patGDad) {
	    patGDad->next = pat;
	    pat->prev = patGDad;
	  }
	  else {
	    arcsighead = pat;
	    pat->prev = NULL;
	  }

	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, 255), MPFROMP(szItemText));
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect - 1, 255), MPFROMP(szPCItemText));
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SETITEMTEXT,
			    MPFROMSHORT(sSelect), MPFROMP(szPCItemText));
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SETITEMTEXT,
			    MPFROMSHORT(sSelect - 1), MPFROMP(szItemText));
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			    MPFROMSHORT(sSelect - 1), MPFROMSHORT(TRUE));
	  arcsigsmodified = TRUE;
	}
      }
      return 0;
    case ASEL_PB_DOWN:
      sSelect = (SHORT)WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_QUERYSELECTION,
					 MPFROMSHORT(LIT_FIRST),MPVOID);
      sItemCount = (SHORT)WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_QUERYITEMCOUNT,
					    MPVOID,MPVOID);
      if (sSelect != LIT_NONE && sSelect < sItemCount - 1) {
	// Find self - assume all archivers listed since we are editing
	for (i = 0, pat = arcsighead; pat && i < sSelect; pat = pat->next, i++)
	  ; // Find self
	if (!pat || !pat->next) {
	  saymsg(0,NULLHANDLE,"*Debug*","Can not find self at %d/%d at %s::%u",sSelect, sItemCount, __FILE__, __LINE__);
	}
	else {
	  ARC_TYPE *patDad;
	  ARC_TYPE *patChild;
	  patDad = pat->prev;
	  patChild = pat->next;
	  pat->next = patChild->next;
	  patChild->next = pat;
	  pat->prev = patChild;
	  patChild->prev = patDad;
	  if (patDad) {
	    patDad->next = patChild;
	    patChild->prev=patDad;
	  }
	  else {
	    arcsighead = patChild;
	    patChild->prev=NULL;
	  }

	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, 255), MPFROMP(szItemText));
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect + 1, 255), MPFROMP(szPCItemText));
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SETITEMTEXT,
			    MPFROMSHORT(sSelect), MPFROMP(szPCItemText));
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SETITEMTEXT,
			    MPFROMSHORT(sSelect + 1), MPFROMP(szItemText));
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			    MPFROMSHORT(sSelect + 1), MPFROMSHORT(TRUE));
	  arcsigsmodified = TRUE;
	}
      }
      return 0;

    case ASEL_PB_REVERT:
      // Reload without checking in case changed outside
      sSelect = (SHORT)WinSendDlgItemMsg(hwnd,ASEL_LISTBOX,LM_QUERYSELECTION,
					 MPFROMSHORT(LIT_FIRST),MPVOID);
      load_archivers();
      fill_listbox(hwnd, TRUE, sSelect);
      return 0;

    case IDM_HELP:
      if (hwndHelp) {
        WinSendMsg(hwndHelp,
                   HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_EDITARC,0),	// fixme to be HELP_SELARC
                   MPFROMSHORT(HM_RESOURCEID));
      }
    }
    return 0;				// WM_COMMAND

  case WM_CONTROL:
    if (SHORT1FROMMP(mp1) == ASEL_LISTBOX && SHORT2FROMMP(mp1) == LN_ENTER)
      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
    return 0;

  case WM_CLOSE:
    WinDismissDlg(hwnd, FALSE);
    return 0;

  default:
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/*
   see archiver.tmp
   02-08-96  23:55              1
   8 Feb 96 23:55:32            2
   8 Feb 96  11:55p             3
   96-02-08 23:55:32            4
   31-02-98  23:55              5
 */

#pragma alloc_text(ARCCNRS,ArcDateTime)

BOOL ArcDateTime(CHAR * dt, INT type, CDATE * cdate, CTIME * ctime)
{
  INT x;
  BOOL ret = FALSE;
  CHAR *p, *pp, *pd;

  if (dt && cdate && ctime)
  {
    memset(cdate, 0, sizeof(CDATE));
    memset(ctime, 0, sizeof(CTIME));
    if (type)
    {
      p = dt;
      while (*p && *p == ' ')
	p++;
      pd = dt;
      switch (type)
      {
      case 1:
	cdate -> month = atoi(pd);
	p = to_delim(pd, "-/.");
	if (p)
	{
	  p++;
	  cdate -> day = atoi(p);
	  pd = p;
	  p = to_delim(pd, "-/.");
	  if (p)
	  {
	    p++;
	    cdate -> year = atoi(p);
	    if (cdate -> year > 80 && cdate -> year < 1900)
	      cdate -> year += 1900;
	    else if (cdate -> year < 1900)
	      cdate -> year += 2000;
	    ret = TRUE;
	    p = strchr(p, ' ');
	    if (p)
	    {
	      while (*p && *p == ' ')
		p++;
	      ctime -> hours = atoi(p);
	      p = to_delim(pd, ":.");
	      if (p)
	      {
		p++;
		ctime -> minutes = atoi(p);
		p = to_delim(pd, ":.");
		if (p)
		{
		  p++;
		  ctime -> seconds = atoi(p);
		}
	      }
	    }
	  }
	}
	break;

      case 2:
	cdate -> day = atoi(p);
	p = strchr(p, ' ');
	if (p)
	{
	  p++;
	  for (x = 0; x < 12; x++)
	  {
	    if (!strnicmp(p, GetPString(IDS_JANUARY + x), 3))
	      break;
	  }
	  if (x < 12)
	  {
	    cdate -> month = x;
	    p = strchr(p, ' ');
	    if (p)
	    {
	      p++;
	      cdate -> year = atoi(p);
	      if (cdate -> year > 80 && cdate -> year < 1900)
		cdate -> year += 1900;
	      else if (cdate -> year < 1900)
		cdate -> year += 2000;
	      ret = TRUE;
	      p = strchr(p, ' ');
	      if (p)
	      {
		while (*p && *p == ' ')
		  p++;
		ctime -> hours = atoi(p);
		p = to_delim(pd, ":.");
		if (p)
		{
		  p++;
		  ctime -> minutes = atoi(p);
		  p = to_delim(pd, ":.");
		  if (p)
		  {
		    p++;
		    ctime -> seconds = atoi(p);
		  }
		}
	      }
	    }
	  }
	}
	break;

      case 3:
	cdate -> day = atoi(p);
	p = strchr(p, ' ');
	if (p)
	{
	  p++;
	  for (x = 0; x < 12; x++)
	  {
	    if (!strnicmp(p, GetPString(IDS_JANUARY + x), 3))
	      break;
	  }
	  if (x < 12)
	  {
	    cdate -> month = x;
	    p = strchr(p, ' ');
	    if (p)
	    {
	      p++;
	      cdate -> year = atoi(p);
	      if (cdate -> year > 80 && cdate -> year < 1900)
		cdate -> year += 1900;
	      else if (cdate -> year < 1900)
		cdate -> year += 2000;
	      ret = TRUE;
	      p = strchr(p, ' ');
	      if (p)
	      {
		while (*p && *p == ' ')
		  p++;
		ctime -> hours = atoi(p);
		p = to_delim(pd, ":.");
		if (p)
		{
		  p++;
		  pp = p;
		  ctime -> minutes = atoi(p);
		  p = to_delim(pd, ":.");
		  if (p)
		  {
		    p++;
		    ctime -> seconds = atoi(p);
		    p += 2;
		    if (toupper(*p) == 'P')
		      ctime -> hours += 12;
		  }
		  else
		  {
		    p = pp;
		    p += 2;
		    if (toupper(*p) == 'P')
		      ctime -> hours += 12;
		  }
		}
	      }
	    }
	  }
	}
	break;

      case 4:
	cdate -> year = atoi(p);
	if (cdate -> year > 80 && cdate -> year < 1900)
	  cdate -> year += 1900;
	else if (cdate -> year < 1900)
	  cdate -> year += 2000;
	p = to_delim(pd, "-/.");
	if (p)
	{
	  p++;
	  cdate -> month = atoi(p);
	  pd = p;
	  p = to_delim(pd, "-/.");
	  if (p)
	  {
	    p++;
	    cdate -> day = atoi(p);
	    ret = TRUE;
	    p = strchr(p, ' ');
	    if (p)
	    {
	      while (*p && *p == ' ')
		p++;
	      ctime -> hours = atoi(p);
	      p = to_delim(pd, ":.");
	      if (p)
	      {
		p++;
		ctime -> minutes = atoi(p);
		p = to_delim(pd, ":.");
		if (p)
		{
		  p++;
		  ctime -> seconds = atoi(p);
		}
	      }
	    }
	  }
	}
	break;

      case 5:
	cdate -> day = atoi(pd);
	p = to_delim(pd, "-/.");
	if (p)
	{
	  p++;
	  cdate -> month = atoi(p);
	  pd = p;
	  p = to_delim(pd, "-/.");
	  if (p)
	  {
	    p++;
	    cdate -> year = atoi(p);
	    if (cdate -> year > 80 && cdate -> year < 1900)
	      cdate -> year += 1900;
	    else if (cdate -> year < 1900)
	      cdate -> year += 2000;
	    ret = TRUE;
	    p = strchr(p, ' ');
	    if (p)
	    {
	      while (*p && *p == ' ')
		p++;
	      ctime -> hours = atoi(p);
	      p = to_delim(pd, ":.");
	      if (p)
	      {
		p++;
		ctime -> minutes = atoi(p);
		p = to_delim(pd, ":.");
		if (p)
		{
		  p++;
		  ctime -> seconds = atoi(p);
		}
	      }
	    }
	  }
	}
	break;

      default:
	break;
      }
    }
  }
  return ret;
}
