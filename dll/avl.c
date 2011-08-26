
/***********************************************************************

  $Id$

  archiver.bb2 search, load, save and date parse

  Copyright (c) 1993, 1998 M. Kimes
  Copyright (c) 2004, 2008 Steven H.Levine

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
  26 Jun 06 SHL load_archivers: remember where comments are
  14 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets, xfgets_bstripcr
  15 Aug 06 SHL Use Runtime_Error more
  01 Nov 06 SHL Turn off leftover debug code
  06 Apr 07 GKY Work around PM DragInfo and DrgFreeDISH limit
  19 Apr 07 SHL Use FreeDragInfoData
  19 Apr 07 SHL Add more drag/drop error checking
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  25 Aug 07 SHL load_archivers: add missing close on error path
  29 Feb 08 GKY Use xfree where appropriate
  22 Jun 08 GKY Added free_archivers for fortify checking
  19 Jul 08 GKY ifdef Fortify free_archivers
  29 Nov 08 GKY Add ini entry for LastArchiver so the previous archiver is selected in the
                Select archive dialog if no default is provided.
  29 Nov 08 GKY Remove or replace with a mutex semaphore DosEnterCriSec where appropriate.
  11 Jan 08 GKY Replace "ARCHIVER.BB2" in string file with global set at compile in init.c
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  23 Oct 10 GKY Changes to populate and utilize a HELPTABLE for context specific help
  21 Nov 10 GKY Check if archiver.bb2 has been changed on disk before editing
  13 Aug 11 GKY Change to Doxygen comment format
  26 Aug 11 GKY Add the code to correctly format the date time strings for tar.gz archives
                viewed using tar 1.15+

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <sys/stat.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_WINSTDDRAG
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "valid.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "avl.h"
#include "strutil.h"			// GetPString
#include "errutil.h"			// Runtime_Error
#include "avv.h"			// ArcReviewDlgProc, rewrite_archiverbb2
#include "droplist.h"			// DropHelp, FullDrgName
#include "misc.h"			// DrawTargetEmphasis
#include "draglist.h"			// FreeDragInfoData
#include "chklist.h"			// PosOverOkay
#include "literal.h"			// literal
#include "wrappers.h"			// xfgets
#include "strips.h"			// bstrip
#include "srchpath.h"			// searchpath
#include "stristr.h"			// stristr
#include "delims.h"			// to_delim
#include "fortify.h"

// Data definitions
static PSZ pszSrcFile = __FILE__;
static void fill_listbox(HWND hwnd, BOOL fShowAll, SHORT sOldSelect);

#pragma data_seg(GLOBAL1)
ARC_TYPE *arcsighead;
UINT arcsigs_header_lines;		// Header comments line count in archiver.bb2
UINT arcsigs_trailer_line_num;		// Trailer comments start line number (1..n)
BOOL arcsigsloaded;
BOOL arcsigsmodified;
static struct stat Archiverbb2Stats;

#define ARCHIVER_LINE_BYTES	256

//=== quick_find_type() ===
ARC_TYPE *quick_find_type(CHAR * filespec, ARC_TYPE * topsig)
{
  ARC_TYPE *info, *found = NULL;
  CHAR *p;

  if (!arcsigsloaded)
    load_archivers();
  p = strrchr(filespec, '.');
  if (p) {
    p++;
    info = (topsig) ? topsig : arcsighead;
    while (info) {
      if (info->ext && *(info->ext) && !stricmp(p, info->ext)) {
	found = find_type(filespec, topsig);
	break;
      }
      info = info->next;
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

  for (pat = arcsighead; pat; pat = pat->next) {
    /**
     * this inner loop tests for a dup signature entry and assures
     * that only the entry at the top of the list gets used for
     * conversion; editing any is okay
     */
    if (!fShowAll) {
      ARC_TYPE *pat2;
      BOOL isDup = FALSE;

      for (pat2 = arcsighead;
	   pat2 && pat->siglen && pat2 != pat && !isDup; pat2 = pat2->next) {
	isDup = pat2->siglen == pat->siglen &&
	  !memcmp(pat2->signature, pat->signature, pat->siglen);
      }					// for
      if (isDup)
	continue;
    }

    // If caller is editing archivers or entry useful to caller, show in listbox
    if (fShowAll || (pat->id && pat->extract && pat->create)) {
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_INSERTITEM,
					  MPFROM2SHORT(LIT_END, 0),
					  MPFROMP(pat->id ? pat->id : "?"));
      if (!found && *szDefArc && pat->id && !strcmp(szDefArc, pat->id)) {
	// Highlight default
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			  MPFROMSHORT(sSelect), MPFROMSHORT(TRUE));
	found = TRUE;
      }
    }
    else {
      // Complain about odd entry
      if (!pat->id || !*pat->id) {
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0),
			  MPFROMP(GetPString(IDS_UNKNOWNUNUSABLETEXT)));
      }
      else {
	CHAR s[81];

	sprintf(s, "%0.12s %s", pat->id, GetPString(IDS_UNUSABLETEXT));
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0), MPFROMP(s));
      }
    }
  }					// while scanning

  // Try to reselect last selection unless user wants default selection
  if (sOldSelect == LIT_NONE) {
    ULONG size = sizeof(SHORT);

    PrfQueryProfileData(fmprof, appname, "LastArchiver", &sOldSelect, &size);
  }
  if (sOldSelect != LIT_NONE && !found) {
    SHORT sItemCount =
      (SHORT) WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMCOUNT,
				MPVOID, MPVOID);

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
  CHAR buffer[4096];			// 06 Oct 07 SHL Protect against NTFS defect

  if (!arcsigsloaded)
    load_archivers();
  if (!topsig)
    topsig = arcsighead;
  DosError(FERR_DISABLEHARDERR);
  if (DosOpen(filespec,
	      &handle,
	      &action,
	      0,
	      0,
	      OPEN_ACTION_FAIL_IF_NEW |
	      OPEN_ACTION_OPEN_IF_EXISTS,
	      OPEN_FLAGS_FAIL_ON_ERROR |
	      OPEN_FLAGS_NOINHERIT |
	      OPEN_FLAGS_RANDOMSEQUENTIAL |
	      OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0))
    return NULL;
  // Scan signatures
  for (info = topsig; info; info = info->next) {
    if (info->siglen == 0) {
      // No signature -- check extension
      p = strrchr(filespec, '.');
      if (p) {
	p++;
	if (info->ext && *(info->ext) && !stricmp(p, info->ext))
	  break;			// Matched

      }
      continue;				// Next sig

    }
    // Try signature match
    l = info->siglen;
    l = min(l, 79);
    if (!DosChgFilePtr(handle,
		       abs(info->file_offset),
                       (info->file_offset >= 0) ? FILE_BEGIN : FILE_END,
                       &len)) {
      if (!DosRead(handle, buffer, l, &len) && len == l) {
	if (!memcmp(info->signature, buffer, l))
	  break;			// Matched

      }
    }
  }					// for

  DosClose(handle);			// Either way, we're done for now
  return info;				// Return signature, if any
}

# ifdef FORTIFY

VOID free_archivers(VOID)
{
  ARC_TYPE *pat, *next;

  pat = arcsighead;
  while (pat) {
    next = pat->next;
    xfree(pat->id, pszSrcFile, __LINE__);
    xfree(pat->ext, pszSrcFile, __LINE__);
    xfree(pat->list, pszSrcFile, __LINE__);
    xfree(pat->extract, pszSrcFile, __LINE__);
    xfree(pat->create, pszSrcFile, __LINE__);
    xfree(pat->move, pszSrcFile, __LINE__);
    xfree(pat->delete, pszSrcFile, __LINE__);
    xfree(pat->signature, pszSrcFile, __LINE__);
    xfree(pat->startlist, pszSrcFile, __LINE__);
    xfree(pat->endlist, pszSrcFile, __LINE__);
    xfree(pat->exwdirs, pszSrcFile, __LINE__);
    xfree(pat->test, pszSrcFile, __LINE__);
    xfree(pat->createrecurse, pszSrcFile, __LINE__);
    xfree(pat->createwdirs, pszSrcFile, __LINE__);
    xfree(pat->movewdirs, pszSrcFile, __LINE__);
    xfree(pat, pszSrcFile, __LINE__);
    pat = next;
  }
  arcsighead = NULL;
}

//=== free_arc_type() free allocated ARC_TYPE ===

# endif

VOID free_arc_type(ARC_TYPE * pat)
{
  if (pat) {
    xfree(pat->id, pszSrcFile, __LINE__);
    xfree(pat->ext, pszSrcFile, __LINE__);
    xfree(pat->list, pszSrcFile, __LINE__);
    xfree(pat->extract, pszSrcFile, __LINE__);
    xfree(pat->create, pszSrcFile, __LINE__);
    xfree(pat->move, pszSrcFile, __LINE__);
    xfree(pat->delete, pszSrcFile, __LINE__);
    xfree(pat->signature, pszSrcFile, __LINE__);
    xfree(pat->startlist, pszSrcFile, __LINE__);
    xfree(pat->endlist, pszSrcFile, __LINE__);
    xfree(pat->exwdirs, pszSrcFile, __LINE__);
    xfree(pat->test, pszSrcFile, __LINE__);
    xfree(pat->createrecurse, pszSrcFile, __LINE__);
    xfree(pat->createwdirs, pszSrcFile, __LINE__);
    xfree(pat->movewdirs, pszSrcFile, __LINE__);
    free(pat);
  }
}

static UINT cur_line_num;	// Input file line counter

//=== get_line_strip_comments() read line, strip comments and whitespace ===

static PSZ get_line_strip_comments(PSZ pszIn, FILE * fp)
{
  PSZ psz = xfgets(pszIn, ARCHIVER_LINE_BYTES, fp, pszSrcFile, __LINE__);
  PSZ psz2;

  if (psz) {
    cur_line_num++;
    psz2 = strchr(pszIn, ';');
    if (psz2)
      *psz2 = 0;			// Chop comment
    bstripcr(pszIn);			// Strip leading white and trailing white and CR/LF

  }
  return psz;
}

//=== get_line_strip_white() read line, strip whitespace ===

static PSZ get_line_strip_white(PSZ pszIn, FILE * fp)
{
  PSZ psz =
    xfgets_bstripcr(pszIn, ARCHIVER_LINE_BYTES, fp, pszSrcFile, __LINE__);

  if (psz)
    cur_line_num++;

  return psz;
}

//=== load_archivers() load or reload archive definitions from archiver.bb2 ===

INT load_archivers(VOID)
{
  FILE *fp;
  CHAR sz[ARCHIVER_LINE_BYTES + 1];
  CHAR *psz;
  ARC_TYPE *pat = NULL;
  ARC_TYPE *patLast = NULL;
  UINT lines_per_arcsig = LINES_PER_ARCSIG;
  UINT per_sig_comment_line_num = 0;
  INT i;
  CHAR *moder = "r";

  // Free current signatures
  if (arcsighead) {
    for (pat = arcsighead; pat;) {
      patLast = pat;
      pat = pat->next;
      free_arc_type(patLast);
    }
    arcsighead = NULL;
  }

  arcsigsmodified = FALSE;
  arcsigs_header_lines = 0;
  arcsigs_trailer_line_num = 0;

  //DosEnterCritSec(); //GKY 11-29-08
  DosRequestMutexSem(hmtxFM2Globals, SEM_INDEFINITE_WAIT);
  psz = searchpath(PCSZ_ARCHIVERBB2);
  if (!psz || !*psz) {
    DosReleaseMutexSem(hmtxFM2Globals);
    //DosExitCritSec();
    return -1;
  }
  stat(psz, &Archiverbb2Stats);
  fp = xfsopen(psz, moder, SH_DENYWR, pszSrcFile, __LINE__, TRUE);
  DosReleaseMutexSem(hmtxFM2Globals);
  //DosExitCritSec();
  if (!fp)
    return -2;
  strcpy(archiverbb2, psz);		// Remember full path

  cur_line_num = 0;

  // Line 1 must contain number of lines per signature definition
  if (!get_line_strip_comments(sz, fp)) {
    fclose(fp);
    return -3;
  }
  if (*sz)
    lines_per_arcsig = atoi(sz);
  if (!*sz || lines_per_arcsig < LINES_PER_ARCSIG) {
    fclose(fp);				// 25 Aug 07 SHL
    return -3;
  }

  // Parse rest of file
  // 1st non-blank line starts definition
  // Need to determine header size and start of trailer

  while (!feof(fp)) {
    // If reading header
    if (!arcsigs_header_lines) {
      // Reading header - find header size and start of signtures
      if (!get_line_strip_white(sz, fp))
	break;				// Unexpected EOF
      if (stristr(sz, "-- Current Archivers --")) {
	arcsigs_header_lines = cur_line_num;
	continue;
      }
      if (!*sz || *sz == ';')
	continue;			//  Header comment or blank line
      else {
	// Not a comment, must be start of signatures
	PSZ psz2 = strchr(sz, ';');

	if (psz2) {
	  *psz2 = 0;			// Chop trailing comment
	  bstripcr(sz);			// Strip leading white and trailing white and CR/LF
	}
	arcsigs_header_lines = cur_line_num - 1;
      }
    }
    else {
      // Reading defintiions
      if (!get_line_strip_comments(sz, fp))
	break;				// EOF
    }

    // fixme to avoid allocating empty fields

    // Remember start of per sig comments for next definition
    if (per_sig_comment_line_num == 0)
      per_sig_comment_line_num = cur_line_num;

    if (*sz) {
      // At start of defintion

      pat = xmallocz(sizeof(ARC_TYPE), pszSrcFile, __LINE__);
      if (!pat)
	break;
      pat->id = xstrdup(sz, pszSrcFile, __LINE__);

      pat->comment_line_num = per_sig_comment_line_num;
      pat->defn_line_num = cur_line_num;

      if (!get_line_strip_comments(sz, fp))	// line 2 - extension
	break;
      if (*sz)
	pat->ext = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->ext = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 3 - offset to signature
	break;
      pat->file_offset = atol(sz);
      if (!get_line_strip_comments(sz, fp))	// line 4 - list command
	break;
      if (*sz)
	pat->list = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->list = NULL;
      if (!pat->list)
	break;				// Must have list command - fixme to complain
      if (!get_line_strip_comments(sz, fp))	// line 5
	break;
      if (*sz)
	pat->extract = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->extract = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 6
	break;
      if (*sz)
	pat->exwdirs = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->exwdirs = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 7
	break;
      if (*sz)
	pat->test = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->test = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 8
	break;
      if (*sz)
	pat->create = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->create = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 9
	break;
      if (*sz)
	pat->createwdirs = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->createwdirs = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 10
	break;
      if (*sz)
	pat->createrecurse = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->createrecurse = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 11
	break;
      if (*sz)
	pat->move = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->move = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 12
	break;
      if (*sz)
	pat->movewdirs = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->movewdirs = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 13
	break;
      if (*sz)
	pat->delete = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->delete = NULL;
      if (!get_line_strip_white(sz, fp))	// line 14
	break;
      i = literal(sz);			// Translate \ escapes
      if (i) {
	pat->siglen = i;
	pat->signature = xmalloc(i, pszSrcFile, __LINE__);
	if (!pat->signature)
	  break;
	memcpy(pat->signature, sz, i);	// signature may not be a string
      }
      else {
	pat->siglen = 0;
	pat->signature = NULL;
      }
      if (!get_line_strip_white(sz, fp))	// line 15
	break;
      if (*sz)
	pat->startlist = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->startlist = NULL;
      if (!get_line_strip_white(sz, fp))	// line 16
	break;
      if (*sz)
	pat->endlist = xstrdup(sz, pszSrcFile, __LINE__);
      else
	pat->endlist = NULL;
      if (!get_line_strip_comments(sz, fp))	// line 17
	break;
      pat->osizepos = atoi(sz);
      if (!get_line_strip_comments(sz, fp))	// line 18
	break;
      pat->nsizepos = atoi(sz);
      if (!get_line_strip_comments(sz, fp))	// line 19
	break;
      pat->fdpos = atoi(sz);
      psz = strchr(sz, ',');
      if (psz) {
	psz++;
	pat->datetype = atoi(psz);
      }
      if (!get_line_strip_comments(sz, fp))	// line 20
	break;
      pat->fdflds = atoi(sz);
      if (!get_line_strip_comments(sz, fp))	// line 21
	break;
      pat->fnpos = atoi(sz);
      psz = strchr(sz, ',');
      if (psz) {
	psz++;
	pat->nameislast = (BOOL) (*psz && atol(psz) == 0) ? FALSE : TRUE;
	psz = strchr(psz, ',');
	if (psz) {
	  psz++;
	  pat->nameisnext = (BOOL) (*psz && atol(psz) == 0) ? FALSE : TRUE;
	  psz = strchr(psz, ',');
	  if (psz) {
	    psz++;
	    pat->nameisfirst = (BOOL) (*psz && atol(psz) == 0) ? FALSE : TRUE;
	  }
	}
      }
      // Ignore unknown lines - must be newer file format
      for (i = LINES_PER_ARCSIG; i < lines_per_arcsig; i++) {
	if (!get_line_strip_comments(sz, fp))
	  break;			// Unexpected EOF - fixme to complain
      }

      // Add to list, assume next and prev already NULL
      if (!arcsighead)
	arcsighead = patLast = pat;
      else {
	patLast->next = pat;
	pat->prev = patLast;
	patLast = pat;
      }
      pat = NULL;			// Done with this defintion

      arcsigs_trailer_line_num = cur_line_num + 1;	// In case this is last defintion
      per_sig_comment_line_num = 0;
    }					// if got definition

  }					// while more lines

  fclose(fp);

  free_arc_type(pat);			// In case partial definition in progress

  if (!arcsighead)
    return -4;

  arcsigsloaded = TRUE;

  return 0;
}

#define TEST_DRAG 0			// fixme to be gone or to work

static MRESULT EXPENTRY SDlgListboxSubclassProc(HWND hwnd, ULONG msg,
						MPARAM mp1, MPARAM mp2)
{
  PFNWP pfnOldProc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);

  PDRAGITEM pDItem;
  PDRAGINFO pDInfo;
  BOOL ok;

  static BOOL emphasized = FALSE;
  static PSZ DRMDRF_LBOX = "<DRM_LBOX,DRF_UNKNOWN>";
  static PSZ DRM_LBOX = "DRM_LBOX";

  switch (msg) {
  case WM_BEGINDRAG:
    {
      LONG cur_ndx;
      DRAGITEM ditem;
      DRAGIMAGE dimage;
      HWND hwndDrop;

      // fprintf(stderr, "SDlgListboxSubclassProc: BEGINDRAG\n");
      cur_ndx = WinQueryLboxSelectedItem(hwnd);

      if (cur_ndx != LIT_NONE) {
	pDInfo = DrgAllocDraginfo(1);
	if (pDInfo) {
	  pDInfo->usOperation = DO_DEFAULT;
	  pDInfo->hwndSource = hwnd;

	  memset(&ditem, 0, sizeof(DRAGITEM));
	  ditem.hwndItem = hwnd;
	  ditem.ulItemID = 1;
	  ditem.hstrType = DrgAddStrHandle(DRT_UNKNOWN);
	  ditem.hstrRMF = DrgAddStrHandle(DRMDRF_LBOX);
	  ditem.hstrContainerName = DrgAddStrHandle(NullStr);
	  ditem.hstrSourceName = DrgAddStrHandle(NullStr);
	  ditem.hstrTargetName = DrgAddStrHandle(NullStr);
	  ditem.fsSupportedOps = DO_MOVEABLE;

	  memset(&dimage, 0, sizeof(DRAGIMAGE));
	  dimage.cb = sizeof(DRAGIMAGE);
	  dimage.hImage = hptrFile;
	  dimage.cptl = 0;
	  dimage.fl = DRG_ICON;
	  dimage.sizlStretch.cx = 32;
	  dimage.sizlStretch.cy = 32;
	  dimage.cxOffset = -16;
	  dimage.cyOffset = 0;
	  DrgSetDragitem(pDInfo, &ditem, sizeof(DRAGITEM), 0);	// Index of DRAGITEM
	  hwndDrop = DrgDrag(hwnd, pDInfo, &dimage, 1,	// One DRAGIMAGE
			     VK_ENDDRAG, NULL);
	  if (!hwndDrop)
	    Win_Error(hwnd, hwnd, pszSrcFile, __LINE__, "DrgDrag");

	  DrgFreeDraginfo(pDInfo);
	}
      }
      break;
    }

  case DM_DRAGOVER:
    ok = FALSE;
    if (!emphasized) {
      POINTL ptl;
      POINTL ptl2;

      emphasized = TRUE;
      ptl.x = SHORT1FROMMP(mp2);
      ptl.y = SHORT2FROMMP(mp2);
      ptl2 = ptl;
      WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl2, 1);
      // fprintf(stderr, "DRAGOVER mapped x y %d %d to %d %d\n", ptl.x, ptl.y, ptl2.x, ptl2.y);
      WinPostMsg(hwnd, WM_BUTTON1CLICK,
		 MPFROM2SHORT((SHORT) ptl2.x, (SHORT) ptl2.y),
		 MPFROM2SHORT(HT_NORMAL, KC_NONE));
      // fprintf(stderr, "DRAGOVER posted 0x%x WM_BUTTON1CLICK x y %d %d\n", hwnd, ptl2.x, ptl2.y);
    }
    pDInfo = (PDRAGINFO) mp1;		// Get DRAGINFO pointer
    if (pDInfo) {
      if (!DrgAccessDraginfo(pDInfo)) {
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		  PCSZ_DRGACCESSDRAGINFO);
      }
      else {
	pDItem = DrgQueryDragitemPtr(pDInfo, 0);
	// Check valid rendering mechanisms and data format
	ok = DrgVerifyRMF(pDItem, DRM_LBOX, NULL);
	DrgFreeDraginfo(pDInfo);
      }
    }
    return ok ? MRFROM2SHORT(DOR_DROP, DO_MOVE) :
		MRFROM2SHORT(DOR_NEVERDROP, 0);

  case DM_DRAGLEAVE:
    if (emphasized) {
      emphasized = FALSE;
      // fixme to draw listbox item emphasized
      // DrawTargetEmphasis(hwnd, emphasized);
      // fprintf(stderr, "DRAGLEAVE\n");
      fflush(stderr);
    }
    return 0;

  case DM_DROPHELP:
    DropHelp(mp1, mp2, hwnd, "fixme to give some help");
    return 0;

  case DM_DROP:
    ok = FALSE;
    if (emphasized) {
      emphasized = FALSE;
      // DrawTargetEmphasis(hwnd, emphasized);
    }
    pDInfo = (PDRAGINFO) mp1;		// Get DRAGINFO pointer
    if (pDInfo) {
      if (!DrgAccessDraginfo(pDInfo)) {
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		  PCSZ_DRGACCESSDRAGINFO);
      }
      else {
	pDItem = DrgQueryDragitemPtr(pDInfo, 0);
	if (!pDItem)
	  Win_Error(hwnd, hwnd, pszSrcFile, __LINE__, "DM_DROP");
	// Check valid rendering mechanisms and data
	ok = DrgVerifyRMF(pDItem, DRM_LBOX, NULL)
			  && ~pDItem->fsControl & DC_PREPARE;
	if (ok) {
	  // note: targetfail is returned to source for all items
	  DrgSendTransferMsg(pDInfo->hwndSource, DM_ENDCONVERSATION,
			     MPFROMLONG(pDItem->ulItemID),
			     MPFROMLONG(DMFL_TARGETSUCCESSFUL));
	}
	FreeDragInfoData(hwnd, pDInfo);
      }
    }
    return 0;
  } // switch
  return pfnOldProc ? pfnOldProc(hwnd, msg, mp1, mp2) :
    WinDefWindowProc(hwnd, msg, mp1, mp2);
}

//=== SBoxDlgProc() Select archiver to use or edit, supports list reorder too ===

static PSZ pszCantFindMsg = "Can't find item %d";

MRESULT EXPENTRY SBoxDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ARC_TYPE **ppatReturn;	// Where to return selected archiver
  ARC_TYPE *pat;
  SHORT sSelect;
  SHORT sItemCount;
  CHAR szItemText[256];
  CHAR szPCItemText[256];	// Parent or child item text
  SHORT i;
  BOOL fShowAll;

  static SHORT sLastSelect = LIT_NONE;

  switch (msg) {
  case WM_INITDLG:
    if (!arcsigsloaded)
      load_archivers();
    else {
      struct stat Buffer;

      stat(searchpath(PCSZ_ARCHIVERBB2), &Buffer);
      if (Archiverbb2Stats.st_size != Buffer.st_size ||
          Archiverbb2Stats.st_mtime != Buffer.st_mtime)        
        if (saymsg(MB_YESNO,                                   
		   hwnd,
		   GetPString(IDS_ADCHANGESONDISKTEXT),
                   GetPString(IDS_ADRELOADMEMTEXT)) == MBID_YES)
          load_archivers();
    }
    if (!(ARC_TYPE **) mp2) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      WinDismissDlg(hwnd, 0);
      break;
    }
    /** Passed arg points to where to return selected archiver definition
     * On input arg value controls selection list content
     * If non-NULL, dup names are suppressed
     * If NULL, all definitions are shown
     */
    ppatReturn = (ARC_TYPE **) mp2;
    fShowAll = *ppatReturn == NULL;
    if (*ppatReturn)
      *ppatReturn = arcsighead;		// Preset to first
    WinSetWindowPtr(hwnd, QWL_USER, (PVOID) ppatReturn);
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

  case WM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROMSHORT(1), MPFROMSHORT(HM_RESOURCEID));
      break;

  case WM_COMMAND:
    ppatReturn = (ARC_TYPE **) WinQueryWindowPtr(hwnd, QWL_USER);
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					  ASEL_LISTBOX,
					  LM_QUERYSELECTION,
					  MPFROMSHORT(LIT_FIRST), MPVOID);
      if (sSelect == LIT_NONE) {
	Runtime_Error(pszSrcFile, __LINE__, "list empty");
	return 0;
      }
      pat = arcsighead;
      if (*ppatReturn) {
	// If dups hidden, find archiver with matching id
	*szItemText = 0;
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMTEXT,
			  MPFROM2SHORT(sSelect, 255), MPFROMP(szItemText));
	if (!*szItemText)
	  pat = NULL;
        else {
	  for (; pat; pat = pat->next) {
	    if (pat->id && !strcmp(szItemText, pat->id))
	      break;			// Found it
	  }
	}
      }
      else {
	// If dups not hidden, lookup by count
	for (i = 0; pat && i < sSelect; i++, pat = pat->next) ;	// Scan
      }
      if (pat && (!*ppatReturn || (pat->id && pat->extract && pat->create))) {
	*ppatReturn = pat;
      }
      else {
	Runtime_Error(pszSrcFile, __LINE__, "no match");
	// Refuse to select
	WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			  MPFROMSHORT(LIT_NONE), FALSE);
	return 0;
      }
      PrfWriteProfileData(fmprof, appname, "LastArchiver", &sSelect, sizeof(SHORT));
      sLastSelect = sSelect;
      WinDismissDlg(hwnd, TRUE);
      return 0;

    case DID_CANCEL:
      if (arcsigsmodified) {
	if (saymsg(MB_YESNO,
		   hwnd,
		   GetPString(IDS_ADCHANGESINMEMTEXT),
		   GetPString(IDS_ADREWRITETEXT), NullStr) == MBID_YES) {
	  PSZ ab2 = searchpath(PCSZ_ARCHIVERBB2);	// Rewrite without prompting

	  rewrite_archiverbb2(ab2);
	}
      }
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					  ASEL_LISTBOX,
					  LM_QUERYSELECTION,
					  MPFROMSHORT(LIT_FIRST), MPVOID);
      if (sSelect != LIT_NONE) {
        sLastSelect = sSelect;
        PrfWriteProfileData(fmprof, appname, "LastArchiver", &sSelect, sizeof(SHORT));
      }
      *ppatReturn = NULL;
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);	// fixme to understand why needed
      return 0;

    case ASEL_PB_ADD:
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					  ASEL_LISTBOX,
					  LM_QUERYSELECTION,
					  MPFROMSHORT(LIT_FIRST), MPVOID);
      if (sSelect != LIT_NONE) {
	ARCDUMP ad;

	memset(&ad, 0, sizeof(ARCDUMP));
	ad.info = xmallocz(sizeof(ARC_TYPE), pszSrcFile, __LINE__);
	if (ad.info) {
	  if (!WinDlgBox(HWND_DESKTOP,
			 hwnd,
			 ArcReviewDlgProc,
			 FM3ModHandle, AD_FRAME, MPFROMP(&ad))) {
	    free(ad.info);
	  }
	  else {
	    // Find self - assume all archivers listed since we are editing
	    for (i = 0, pat = arcsighead; pat && i < sSelect; pat = pat->next, i++) ;	// Find self

	    if (!pat) {
	      if (arcsighead)
		Runtime_Error(pszSrcFile, __LINE__, pszCantFindMsg, sSelect);
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
			      MPFROMP(ad.info->id ? ad.info->id : "?"));
	    WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			      MPFROMSHORT(sSelect - 1), MPFROMSHORT(TRUE));
	    arcsigsmodified = TRUE;
	  }
	}
      }
      return 0;
    case ASEL_PB_DELETE:
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					  ASEL_LISTBOX,
					  LM_QUERYSELECTION,
					  MPFROMSHORT(LIT_FIRST), MPVOID);
      if (sSelect != LIT_NONE) {
	// Find self - assume all archivers listed since we are editing
	for (i = 0, pat = arcsighead; pat && i < sSelect; pat = pat->next, i++) ;	// Find self

	if (!pat)
	  Runtime_Error(pszSrcFile, __LINE__, pszCantFindMsg, sSelect);
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
			  MPFROM2SHORT(sSelect, 0), MPVOID);
	sItemCount =
	  (SHORT) WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMCOUNT,
				    MPVOID, MPVOID);
	if (sSelect >= sItemCount)
	  sSelect--;
	if (sSelect >= 0) {
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_SELECTITEM,
			    MPFROMSHORT(sSelect), MPFROMSHORT(TRUE));
	}
      }
      return 0;
    case ASEL_PB_UP:
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					  ASEL_LISTBOX,
					  LM_QUERYSELECTION,
					  MPFROMSHORT(LIT_FIRST), MPVOID);
      if (sSelect != LIT_NONE && sSelect > 0) {
	// Find self - assume all archivers listed since we are editing
	for (i = 0, pat = arcsighead; pat && i < sSelect; pat = pat->next, i++) ;	// Find self
	if (!pat || !pat->prev)
	  Runtime_Error(pszSrcFile, __LINE__, pszCantFindMsg, sSelect);
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
			    MPFROM2SHORT(sSelect - 1, 255),
			    MPFROMP(szPCItemText));
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
      sSelect =
	(SHORT) WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYSELECTION,
				  MPFROMSHORT(LIT_FIRST), MPVOID);
      sItemCount =
	(SHORT) WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMCOUNT,
				  MPVOID, MPVOID);
      if (sSelect != LIT_NONE && sSelect < sItemCount - 1) {
	// Find self - assume all archivers listed since we are editing
	for (i = 0, pat = arcsighead; pat && i < sSelect; pat = pat->next, i++) ;	// Find self
	if (!pat || !pat->next)
	  Runtime_Error(pszSrcFile, __LINE__, "Can't find item %d of %d",
			sSelect, sItemCount);
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
	    patChild->prev = patDad;
	  }
	  else {
	    arcsighead = patChild;
	    patChild->prev = NULL;
	  }

	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, 255), MPFROMP(szItemText));
	  WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect + 1, 255),
			    MPFROMP(szPCItemText));
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
      sSelect =
	(SHORT) WinSendDlgItemMsg(hwnd, ASEL_LISTBOX, LM_QUERYSELECTION,
				  MPFROMSHORT(LIT_FIRST), MPVOID);
      load_archivers();
      fill_listbox(hwnd, TRUE, sSelect);
      return 0;

    case IDM_HELP:
      if (hwndHelp) {
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP, MPFROM2SHORT(HELP_EDITARC, 0),	// fixme to be HELP_SELARC
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

/**
 *  see archiver.tmp
 *  02-08-96  23:55              1
 *  8 Feb 96 23:55:32            2
 *  8 Feb 96  11:55p             3
 *  96-02-08 23:55:32            4
 *  31-02-98  23:55              5
 *  Aug 21 23:55 2011            6 Fixes tar.gz date/time formatting when using tar 1..15+
 */

BOOL ArcDateTime(CHAR * dt, INT type, CDATE * cdate, CTIME * ctime)
{
  INT x;
  BOOL ret = FALSE;
  CHAR *p, *pp, *pd;

  if (dt && cdate && ctime) {
    memset(cdate, 0, sizeof(CDATE));
    memset(ctime, 0, sizeof(CTIME));
    if (type) {
      p = dt;
      while (*p && *p == ' ')
	p++;
      pd = dt;
      switch (type) {
      case 1:
	cdate->month = atoi(pd);
	p = to_delim(pd, "-/.");
	if (p) {
	  p++;
	  cdate->day = atoi(p);
	  pd = p;
	  p = to_delim(pd, "-/.");
	  if (p) {
	    p++;
	    cdate->year = atoi(p);
	    if (cdate->year > 80 && cdate->year < 1900)
	      cdate->year += 1900;
	    else if (cdate->year < 1900)
	      cdate->year += 2000;
	    ret = TRUE;
	    p = strchr(p, ' ');
	    if (p) {
	      while (*p && *p == ' ')
		p++;
	      ctime->hours = atoi(p);
	      p = to_delim(pd, ":.");
	      if (p) {
		p++;
		ctime->minutes = atoi(p);
		p = to_delim(pd, ":.");
		if (p) {
		  p++;
		  ctime->seconds = atoi(p);
		}
	      }
	    }
	  }
	}
	break;

      case 2:
	cdate->day = atoi(p);
	p = strchr(p, ' ');
	if (p) {
	  p++;
	  for (x = 0; x < 12; x++) {
	    if (!strnicmp(p, GetPString(IDS_JANUARY + x), 3))
	      break;
	  }
	  if (x < 12) {
	    cdate->month = x + 1;
	    p = strchr(p, ' ');
	    if (p) {
	      p++;
	      cdate->year = atoi(p);
	      if (cdate->year > 80 && cdate->year < 1900)
		cdate->year += 1900;
	      else if (cdate->year < 1900)
		cdate->year += 2000;
	      ret = TRUE;
	      p = strchr(p, ' ');
	      if (p) {
		while (*p && *p == ' ')
		  p++;
		ctime->hours = atoi(p);
		p = to_delim(pd, ":.");
		if (p) {
		  p++;
		  ctime->minutes = atoi(p);
		  p = to_delim(pd, ":.");
		  if (p) {
		    p++;
		    ctime->seconds = atoi(p);
		  }
		}
	      }
	    }
	  }
	}
	break;

      case 3:
	cdate->day = atoi(p);
	p = strchr(p, ' ');
	if (p) {
	  p++;
	  for (x = 0; x < 12; x++) {
	    if (!strnicmp(p, GetPString(IDS_JANUARY + x), 3))
	      break;
	  }
	  if (x < 12) {
	    cdate->month = x + 1;
	    p = strchr(p, ' ');
	    if (p) {
	      p++;
	      cdate->year = atoi(p);
	      if (cdate->year > 80 && cdate->year < 1900)
		cdate->year += 1900;
	      else if (cdate->year < 1900)
		cdate->year += 2000;
	      ret = TRUE;
	      p = strchr(p, ' ');
	      if (p) {
		while (*p && *p == ' ')
		  p++;
		ctime->hours = atoi(p);
		p = to_delim(pd, ":.");
		if (p) {
		  p++;
		  pp = p;
		  ctime->minutes = atoi(p);
		  p = to_delim(pd, ":.");
		  if (p) {
		    p++;
		    ctime->seconds = atoi(p);
		    p += 2;
		    if (toupper(*p) == 'P')
		      ctime->hours += 12;
		  }
		  else {
		    p = pp;
		    p += 2;
		    if (toupper(*p) == 'P')
		      ctime->hours += 12;
		  }
		}
	      }
	    }
	  }
	}
	break;

      case 4:
	cdate->year = atoi(p);
	if (cdate->year > 80 && cdate->year < 1900)
	  cdate->year += 1900;
	else if (cdate->year < 1900)
	  cdate->year += 2000;
	p = to_delim(pd, "-/.");
	if (p) {
	  p++;
	  cdate->month = atoi(p);
	  pd = p;
	  p = to_delim(pd, "-/.");
	  if (p) {
	    p++;
	    cdate->day = atoi(p);
	    ret = TRUE;
	    p = strchr(p, ' ');
	    if (p) {
	      while (*p && *p == ' ')
		p++;
	      ctime->hours = atoi(p);
	      p = to_delim(pd, ":.");
	      if (p) {
		p++;
		ctime->minutes = atoi(p);
		p = to_delim(pd, ":.");
		if (p) {
		  p++;
		  ctime->seconds = atoi(p);
		}
	      }
	    }
	  }
	}
	break;

      case 5:
	cdate->day = atoi(pd);
	p = to_delim(pd, "-/.");
	if (p) {
	  p++;
	  cdate->month = atoi(p);
	  pd = p;
	  p = to_delim(pd, "-/.");
	  if (p) {
	    p++;
	    cdate->year = atoi(p);
	    if (cdate->year > 80 && cdate->year < 1900)
	      cdate->year += 1900;
	    else if (cdate->year < 1900)
	      cdate->year += 2000;
	    ret = TRUE;
	    p = strchr(p, ' ');
	    if (p) {
	      while (*p && *p == ' ')
		p++;
	      ctime->hours = atoi(p);
	      p = to_delim(pd, ":.");
	      if (p) {
		p++;
		ctime->minutes = atoi(p);
		p = to_delim(pd, ":.");
		if (p) {
		  p++;
		  ctime->seconds = atoi(p);
		}
	      }
	    }
	  }
	}
        break;
      case 6:
        for (x = 0; x < 12; x++) {
	    if (!strnicmp(p, GetPString(IDS_JANUARY + x), 3))
	      break;
	  }
	  if (x < 12) {
	    cdate->month = x + 1;
	    p = strchr(p, ' ');
	    if (p) {
	      p++;
              cdate->day = atoi(p);
	      p = strchr(p, ' ');
	      if (p) {
                p++;
                ctime->hours = atoi(p);
	        p = to_delim(pd, ":.");
	        if (p) {
		  p++;
		  ctime->minutes = atoi(p);
                  p = to_delim(pd, ":.");
                  p = strchr(p, ' ');
                  if (p) {
                    p++;
                    cdate->year = atoi(p);
                    if (cdate->year > 80 && cdate->year < 1900)
                      cdate->year += 1900;
                    else if (cdate->year < 1900)
                      cdate->year += 2000;
                    ret = TRUE;
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

#pragma alloc_text(MISC9,quick_find_type,find_type)
#pragma alloc_text(AVL,load_archivers, get_line_strip_comments, get_line_strip_white, free_archivers)
#pragma alloc_text(FMARCHIVE,SBoxDlgProc,SDlgListboxSubclassProc)
#pragma alloc_text(ARCCNRS,ArcDateTime)
