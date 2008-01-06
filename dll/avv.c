
/***********************************************************************

  $Id$

  archiver.bb2 editor

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2007 Steven H.Levine

  31 Jul 04 SHL ArcReviewDlgProc: correct nameis... decodes
  01 Aug 04 SHL Localize functions
  01 Aug 04 SHL Rework fixup usage
  06 Jun 05 SHL Drop unused
  14 Aug 05 SHL rewrite_archiverbb2: avoid dereferencing null signature
  14 Aug 05 SHL ArcReviewDlgProc: ensure signature allocated
  29 May 06 SHL EditArchiverDefinition: rework
  26 Jun 06 SHL rewrite_archiverbb2: include user comments
  14 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets
  30 Jul 06 SHL Avoid warnings when editing new definition
  22 Mar 07 GKY Use QWL_USER
  16 Jun 07 SHL Update for OpenWatcom
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "version.h"
#include "fm3str.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static PSZ checkfile(PSZ file, INT * error);
static BOOL check_archiver(HWND hwnd, ARC_TYPE * info);
static INT get_int_from_window(HWND hwnd, USHORT id);
static LONG get_long_from_window(HWND hwnd, USHORT id);
static PSZ nonull(PSZ a);
static PSZ free_and_strdup_from_window(HWND hwnd, USHORT id, PSZ pszDest);
static PSZ free_and_strdup_quoted_from_window(HWND hwnd, USHORT id, PSZ pszDest);

//=== EditArchiverDefinition() Select archiver to edit definition ===

VOID EditArchiverDefinition(HWND hwnd)
{
  ARCDUMP ad;
  ARC_TYPE *pat;

  // Allow multiple edits
  for (;;) {
    pat = NULL;				// Do not hide dups
    if (!WinDlgBox(HWND_DESKTOP,
		   hwnd,
		   SBoxDlgProc,
		   FM3ModHandle, ASEL_EDIT_FRAME, (PVOID) & pat) || !pat) {
      break;				// we are done
    }

    memset(&ad, 0, sizeof(ARCDUMP));
    ad.info = pat;
    WinDlgBox(HWND_DESKTOP,
	      hwnd, ArcReviewDlgProc, FM3ModHandle, AD_FRAME, MPFROMP(&ad));
  }					// for
}

static PSZ free_and_strdup_from_window(HWND hwnd, USHORT id, PSZ pszDest)
{
  CHAR sz[256];

  xfree(pszDest);
  WinQueryDlgItemText(hwnd, id, sizeof(sz), sz);
  if (*sz)
    pszDest = xstrdup(sz, pszSrcFile, __LINE__);
  else
    pszDest = NULL;
  return pszDest;
}

static PSZ free_and_strdup_quoted_from_window(HWND hwnd, USHORT id, PSZ pszDest)
{
  CHAR sz[256], *psz[256];

  xfree(pszDest);
  WinQueryDlgItemText(hwnd, id, sizeof(sz), sz);
  if (*sz){
    *psz = CheckApp_QuoteAddExe(sz);
    pszDest = xstrdup(sz, pszSrcFile, __LINE__);
  }
  else
    pszDest = NULL;
  return pszDest;
}

static INT get_int_from_window(HWND hwnd, USHORT id)
{
  CHAR s[256];

  WinQueryDlgItemText(hwnd, id, sizeof(s), s);
  return atoi(s);
}

static INT get_int2_from_window(HWND hwnd, USHORT id)
{
  CHAR s[256];
  PSZ p;

  WinQueryDlgItemText(hwnd, id, sizeof(s), s);
  p = strchr(s, ',');
  if (p)
    p++;
  return p ? atoi(p) : 0;
}

INT get_int3_from_window(HWND hwnd, USHORT id)
{
  CHAR s[256];
  PSZ p;

  WinQueryDlgItemText(hwnd, id, sizeof(s), s);
  p = strchr(s, ',');
  if (p) {
    p++;
    p = strchr(p, ',');
    if (p)
      p++;
  }
  return p ? atoi(p) : 0;
}

INT get_int4_from_window(HWND hwnd, USHORT id)
{
  CHAR s[256];
  PSZ p;

  WinQueryDlgItemText(hwnd, id, sizeof(s), s);
  p = strchr(s, ',');
  if (p) {
    p++;
    p = strchr(p, ',');
    if (p) {
      p++;
      p = strchr(p, ',');
      if (p)
	p++;
    }
  }
  return p ? atoi(p) : 0;
}

LONG get_long_from_window(HWND hwnd, USHORT id)
{
  CHAR s[256];

  WinQueryDlgItemText(hwnd, id, sizeof(s), s);
  return atol(s);
}

#pragma alloc_text (AVV2,nonull,rewrite_archiverbb2,checkfile)

// nonull - convert NULL pointer to empty string

static PSZ nonull(PSZ psz)
{
  if (!psz)
    psz = NullStr;
  return psz;
}

//=== rewrite_archiverbb2() rewrite archiver.bb2, prompt if arg NULL, merge comments ===

VOID rewrite_archiverbb2(PSZ archiverbb2)
{
  FILE *fpNew;
  FILE *fpOld = NULL;
  UINT entry_num = 0;		// Definition counter
  UINT input_line_num = 0;	// Input file line counter
  ARC_TYPE *pat;
  CHAR sz[258];
  CHAR *psz;
  BOOL needEntryNumber;
  BOOL needReload = FALSE;	// Because line numbers changed
  time_t t;
  struct tm *tm;
  CHAR ch;

  arcsigsmodified = FALSE;

  if (!arcsighead) {
    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
	   HWND_DESKTOP,
	   GetPString(IDS_SAYWHATTEXT), GetPString(IDS_NOINFOTOSAVETEXT));
    return;
  }
  // Alert unless file name passed
  if (!archiverbb2 || !*archiverbb2) {
    saymsg(MB_ENTER | MB_ICONASTERISK,
	   HWND_DESKTOP,
	   GetPString(IDS_NOTETEXT), GetPString(IDS_SAVEARCBB2TEXT));
    archiverbb2 = GetPString(IDS_ARCHIVERBB2);
  }

  /* save a backup */
  psz = strrchr(archiverbb2, '.');
  if (psz && !stricmp(psz, ".BB2")) {
    strcpy(psz, ".BAK");
    DosDelete(archiverbb2);
    strcpy(sz, archiverbb2);
    strcpy(psz, ".BB2");
    DosMove(archiverbb2, sz);
    fpOld = fopen(sz, "r");		// OK for file not to exist
  }

  fpNew = fopen(archiverbb2, "w");

  if (fpNew) {

    fprintf(fpNew, "%u\n", LINES_PER_ARCSIG);
    t = time(NULL);
    tm = localtime(&t);

    fprintf(fpNew,
	    ";\n; %s file written by FM/2 v%d.%02d on %u/%u/%u %u:%02u:%02u\n;\n",
	    GetPString(IDS_ARCHIVERBB2),
	    VERMAJOR, VERMINOR,
	    tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900,
	    tm->tm_hour, tm->tm_min, tm->tm_sec);
    // Rewrite header if known
    if (fpOld && arcsigs_header_lines) {
      needReload = TRUE;
      while (input_line_num < arcsigs_header_lines) {
	psz = xfgets(sz, sizeof(sz), fpOld, pszSrcFile, __LINE__);
	if (!psz)
	  break;
	input_line_num++;
	if (input_line_num == 1)
	  continue;			// Bypass sig count
	fputs(sz, fpNew);
      }
    }
    else {
      // Write default header
      fputs(GetPString(IDS_ARCHIVERBB2TEXT), fpNew);
    }
    pat = arcsighead;
    while (pat) {
      needEntryNumber = TRUE;
      // Rewrite per sig header comments if any exist
      if (fpOld && pat->comment_line_num) {
	needReload = TRUE;		// fixme to optimize
	// Definitions reordered - need to rewind
	if (input_line_num > pat->comment_line_num) {
	  fseek(fpOld, 0, SEEK_SET);
	  input_line_num = 0;
	}
	while (input_line_num + 1 < pat->defn_line_num) {
	  psz = xfgets(sz, sizeof(sz), fpOld, pszSrcFile, __LINE__);
	  if (!psz)
	    break;			// Unexpected EOF
	  input_line_num++;
	  if (input_line_num < pat->comment_line_num)
	    continue;
	  if (needEntryNumber && strnicmp(sz, "; Entry #", 9) == 0) {
	    // Rewrite entry count comment
	    needEntryNumber = FALSE;
	    for (psz = sz + 9; *psz == ' '; psz++) ;	// Find non-blank
	    for (; (ch = *psz) >= '0' && ch <= '9'; psz++) ;	// Find end of entry#
	    fprintf(fpNew, GetPString(IDS_ENTRYCNTRTEXT), ++entry_num);
	    fputs(psz, fpNew);
	  }
	  else {
	    fputs(sz, fpNew);
	  }
	}
      }

      if (needEntryNumber) {
	fputs(";\n", fpNew);
	fprintf(fpNew, GetPString(IDS_ENTRYCNTRTEXT), ++entry_num);
	if (pat->id)
	  fprintf(fpNew, " (%s)", pat->id);
	fputs("\n;\n", fpNew);
      }

      fprintf(fpNew,
	      "%s\n%s\n%ld\n%s\n",
	      nonull(pat->id),
	      nonull(pat->ext), pat->file_offset, nonull(pat->list));
      fprintf(fpNew,
	      "%s\n%s\n%s\n%s\n%s\n%s\n",
	      nonull(pat->extract),
	      nonull(pat->exwdirs),
	      nonull(pat->test),
	      nonull(pat->create),
	      nonull(pat->createwdirs), nonull(pat->createrecurse));
      fprintf(fpNew,
	      "%s\n%s\n%s\n",
	      nonull(pat->move), nonull(pat->movewdirs), nonull(pat->delete));
      fprintf(fpNew,
	      "%s\n%s\n%s\n%d\n%d\n%d,%d\n%d\n%d,%lu,%lu,%lu\n",
	      fixup(pat->signature,
		    sz,
		    sizeof(sz),
		    pat->siglen),
	      nonull(pat->startlist),
	      nonull(pat->endlist),
	      pat->osizepos,
	      pat->nsizepos,
	      pat->fdpos,
	      pat->datetype,
	      pat->fdflds,
	      pat->fnpos, pat->nameislast, pat->nameisnext, pat->nameisfirst);
      pat = pat->next;
    }					// while more sigs

    // Rewrite trailer comments if known
    if (fpOld && arcsigs_trailer_line_num) {
      for (;;) {
	psz = xfgets(sz, sizeof(sz), fpOld, pszSrcFile, __LINE__);
	if (!psz)
	  break;
	input_line_num++;
	if (input_line_num < arcsigs_trailer_line_num)
	  continue;			// Bypass sig count
	fputs(sz, fpNew);
      }
    }

    fclose(fpNew);

  }					// if fpNew open OK

  if (fpOld)
    fclose(fpOld);

  if (needReload)
    load_archivers();			// Resync commend line numbers
}

static PSZ checkfile(PSZ file, INT * error)
{
  CHAR *p, *pp = NULL;
  INT ret;
  ULONG apptype;

  if (!file || !*file) {
    *error = 3;
    return NULL;
  }
  pp = strchr(file, ' ');
  if (pp)
    *pp = 0;
  p = searchpath(file);
  if (!p || !*p)
    *error = 1;
  else {
    ret = (INT) DosQueryAppType(p, &apptype);
    if (ret)
      *error = -1;
    else {
      apptype &= (~FAPPTYP_32BIT);
      if (!apptype ||
	  (apptype == FAPPTYP_NOTWINDOWCOMPAT) ||
	  (apptype == FAPPTYP_WINDOWCOMPAT) ||
	  (apptype & FAPPTYP_BOUND) ||
	  (apptype & FAPPTYP_WINDOWAPI) || (apptype & FAPPTYP_DOS)) {
	*error = 0;
      }
      else
	*error = 2;
    }
  }
  if (pp)
    *pp = ' ';
  return p;
}

#pragma alloc_text (AVV3,check_archiver,ArcReviewDlgProc)

static BOOL check_archiver(HWND hwnd, ARC_TYPE * info)
{
  BOOL noStart = FALSE, noEnd = FALSE, badPos = FALSE;
  INT badList = 0, badCreate = 0, badExtract = 0;
  static PSZ aerrors[3];

  aerrors[0] = GetPString(IDS_STARTLISTEMPTYTEXT);
  aerrors[1] = GetPString(IDS_ENDLISTEMPTYTEXT);
  aerrors[2] = GetPString(IDS_BOGUSNAMETEXT);
  if (!info->startlist || !*info->startlist)
    noStart = TRUE;
  if (!info->endlist || !*info->endlist)
    noEnd = TRUE;
  if (info->fnpos > 50 || info->fnpos < -1)
    badPos = TRUE;
  checkfile(info->list, &badList);
  checkfile(info->create, &badCreate);
  checkfile(info->extract, &badExtract);
  if (!noStart && !noEnd && !badPos && !badList && !badCreate && !badExtract)
    return TRUE;			// OK
  if (!info->id)
    return FALSE;			// Assume new if no id
  saymsg(MB_ENTER | MB_ICONASTERISK,
	 hwnd,
	 GetPString(IDS_WARNINGSTEXT),
	 GetPString(IDS_AVVCHK1TEXT),
	 noStart ? aerrors[0] : NullStr,
	 noEnd ? aerrors[1] : NullStr,
	 badPos ? aerrors[2] : NullStr,
	 badList == 1 ?
	 GetPString(IDS_AVVCHK2TEXT) :
	 badList == -1 ?
	 GetPString(IDS_AVVCHK3TEXT) :
	 badList == 2 ?
	 GetPString(IDS_AVVCHK4TEXT) :
	 badList == 3 ?
	 GetPString(IDS_AVVCHK5TEXT) :
	 NullStr,
	 badCreate == 1 ?
	 GetPString(IDS_AVVCHK6TEXT) :
	 badCreate == -1 ?
	 GetPString(IDS_AVVCHK7TEXT) :
	 badCreate == 2 ?
	 GetPString(IDS_AVVCHK8TEXT) :
	 badCreate == 3 ?
	 GetPString(IDS_AVVCHK9TEXT) :
	 NullStr,
	 badExtract == 1 ?
	 GetPString(IDS_AVVCHK10TEXT) :
	 badExtract == -1 ?
	 GetPString(IDS_AVVCHK11TEXT) :
	 badExtract == 2 ?
	 GetPString(IDS_AVVCHK12TEXT) :
	 badExtract == 3 ? GetPString(IDS_AVVCHK13TEXT) : NullStr);
  if (badList || badExtract)
    return FALSE;			// Problems
  return TRUE;				// OK
}

//=== ArcReviewDlgProc() View/edit single archiver.bb2 setup ===

MRESULT EXPENTRY ArcReviewDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2)
{
  ARCDUMP *admp;
  CHAR s[256];
  SHORT sSelect;

  if (msg != WM_INITDLG)
    admp = (ARCDUMP *) WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg) {
  case WM_INITDLG:
    admp = (ARCDUMP *) mp2;
    if (!admp || !admp->info) {
      WinDismissDlg(hwnd, 0);
      return 0;
    }

    WinSetWindowPtr(hwnd, QWL_USER, mp2);

    WinSendDlgItemMsg(hwnd, AD_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
    for (sSelect = AD_ID; sSelect < AD_ADDWPATHS + 1; sSelect++) {
      WinSendDlgItemMsg(hwnd,
			sSelect,
			EM_SETTEXTLIMIT,
			MPFROM2SHORT(sizeof(s) - 1, 0), MPVOID);
    }
    if (admp->info->id)
      WinSetDlgItemText(hwnd, AD_ID, admp->info->id);
    if (admp->info->ext)
      WinSetDlgItemText(hwnd, AD_EXT, admp->info->ext);
    sprintf(s, "%ld", admp->info->file_offset);
    WinSetDlgItemText(hwnd, AD_SIGPOS, s);
    if (admp->info->siglen) {
      WinSetDlgItemText(hwnd,
			AD_SIG,
			fixup(admp->info->signature,
			      s, sizeof(s), admp->info->siglen));
    }
    if (admp->info->startlist)
      WinSetDlgItemText(hwnd, AD_STARTLIST, admp->info->startlist);
    if (admp->info->endlist)
      WinSetDlgItemText(hwnd, AD_ENDLIST, admp->info->endlist);
    if (admp->info->list)
      WinSetDlgItemText(hwnd, AD_LIST, admp->info->list);
    sprintf(s,
	    "%d,%d,%d,%d",
	    admp->info->fnpos,
	    admp->info->nameislast,
	    admp->info->nameisnext, admp->info->nameisfirst);
    WinSetDlgItemText(hwnd, AD_FNAMEPOS, s);
    sprintf(s, "%d", admp->info->osizepos);
    WinSetDlgItemText(hwnd, AD_OLDSZ, s);
    sprintf(s, "%d", admp->info->nsizepos);
    WinSetDlgItemText(hwnd, AD_NEWSZ, s);
    sprintf(s, "%d,%d", admp->info->fdpos, admp->info->datetype);
    WinSetDlgItemText(hwnd, AD_DATEPOS, s);
    sprintf(s, "%d", admp->info->fdflds);
    WinSetDlgItemText(hwnd, AD_NUMDATEFLDS, s);
    if (admp->info->extract)
      WinSetDlgItemText(hwnd, AD_EXTRACT, admp->info->extract);
    if (admp->info->exwdirs)
      WinSetDlgItemText(hwnd, AD_WDIRS, admp->info->exwdirs);
    if (admp->info->test)
      WinSetDlgItemText(hwnd, AD_TEST, admp->info->test);
    if (admp->info->create)
      WinSetDlgItemText(hwnd, AD_ADD, admp->info->create);
    if (admp->info->move)
      WinSetDlgItemText(hwnd, AD_MOVE, admp->info->move);
    if (admp->info->delete)
      WinSetDlgItemText(hwnd, AD_DELETE, admp->info->delete);
    if (admp->info->createrecurse)
      WinSetDlgItemText(hwnd, AD_ADDRECURSE, admp->info->createrecurse);
    if (admp->info->createwdirs)
      WinSetDlgItemText(hwnd, AD_ADDWPATHS, admp->info->createwdirs);
    if (admp->info->movewdirs)
      WinSetDlgItemText(hwnd, AD_MOVEWPATHS, admp->info->movewdirs);

    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    break;				// WM_INITDLG

  case UM_SETUP:
    if (admp->listname && *admp->listname) {

      FILE *fp = fopen(admp->listname, "r");

      if (!fp) {
	WinSendDlgItemMsg(hwnd,
			  AD_LISTBOX,
			  LM_INSERTITEM,
			  MPFROM2SHORT(LIT_END, 0),
			  MPFROMP(GetPString(IDS_CANTOPENFILETEXT)));
      }
      else {
	while (!feof(fp)) {
	  if (!xfgets(s, sizeof(s), fp, pszSrcFile, __LINE__))
	    break;
	  stripcr(s);
	  WinSendDlgItemMsg(hwnd,
			    AD_LISTBOX,
			    LM_INSERTITEM,
			    MPFROM2SHORT(LIT_END, 0), MPFROMP(s));
	}
	fclose(fp);
      }
    }
    else {
      WinSendDlgItemMsg(hwnd,
			AD_LISTBOX,
			LM_INSERTITEM,
			MPFROM2SHORT(LIT_END, 0),
			MPFROMP(GetPString(IDS_NOTAPPLICABLETEXT)));
    }
    check_archiver(hwnd, admp->info);
    return 0;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, AD_HELP), (HPS) 0, FALSE, TRUE);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case AD_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
	for (sSelect = 0; sSelect < 10; sSelect++)
	  WinSetDlgItemText(hwnd, AD_FLD1 + sSelect, NullStr);
	if (!admp->listname)
	  Runtime_Error(pszSrcFile, __LINE__, "no listname");
	else {
	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      AD_LISTBOX,
					      LM_QUERYSELECTION,
					      MPVOID, MPVOID);
	  WinSendDlgItemMsg(hwnd,
			    AD_LISTBOX,
			    LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, 255), MPFROMP(s));
	  if (!*s)
	    Runtime_Error(pszSrcFile, __LINE__, "no text");
	  else {
	    PSZ p;
	    PSZ pp;

	    p = s;
	    for (sSelect = 0; sSelect < 10; sSelect++) {
	      pp = p;
	      while (*pp == ' ' || *pp == '\t')
		pp++;
	      if (!*pp)
		break;
	      p = pp;
	      while (*p && (*p != ' ' && *p != '\t'))
		p++;
	      if (*p)
		*p++ = 0;
	      WinSetDlgItemText(hwnd, AD_FLD1 + sSelect, pp);
	    }
	  }
	}
	break;

      case LN_KILLFOCUS:
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
	break;

      case LN_SETFOCUS:
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_TEXTARCPRODUCEDTEXT));
	break;
      }
      break;

    case AD_ID:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADIDTEXT));
      break;

    case AD_ADD:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADADDTEXT));
      break;

    case AD_MOVE:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADMOVETEXT));
      break;

    case AD_EXT:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADEXTTEXT));
      break;

    case AD_EXTRACT:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADEXTRACTTEXT));
      break;

    case AD_WDIRS:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADWDIRSTEXT));
      break;

    case AD_SIG:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADSIGTEXT));
      break;

    case AD_LIST:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADLISTTEXT));
      break;

    case AD_TEST:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADTESTTEXT));
      break;

    case AD_ADDWPATHS:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADADDWPATHSTEXT));
      break;

    case AD_MOVEWPATHS:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADMOVEWPATHSTEXT));
      break;

    case AD_ADDRECURSE:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADADDRECURSETEXT));
      break;

    case AD_DELETE:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADDELETETEXT));
      break;

    case AD_SIGPOS:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADSIGPOSTEXT));
      break;

    case AD_FNAMEPOS:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADFNAMEPOSTEXT));
      break;

    case AD_OLDSZ:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADOLDSZTEXT));
      break;

    case AD_NUMDATEFLDS:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADNUMDATEFLDSTEXT));
      break;

    case AD_DATEPOS:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADDATEPOSTEXT));
      break;

    case AD_NEWSZ:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADNEWSZTEXT));
      break;

    case AD_STARTLIST:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADSTARTLISTTEXT));
      break;

    case AD_ENDLIST:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, AD_HELP, GetPString(IDS_ADENDLISTTEXT));
      break;

    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case AD_SEEEXTRACTOR:
    case AD_SEEADDER:
      {
	static CHAR tempargs[1026];

	*tempargs = 0;
	if (SHORT1FROMMP(mp1) == AD_SEEADDER)
	  WinQueryDlgItemText(hwnd, AD_ADD, 255, tempargs);
	else
	  WinQueryDlgItemText(hwnd, AD_EXTRACT, 255, tempargs);
	if (!*tempargs)
	  saymsg(MB_CANCEL,
		 hwnd,
		 GetPString(IDS_BZZZTTEXT), GetPString(IDS_NEEDENTRYTEXT));
	else {

	  PSZ p;

	  lstrip(tempargs);
	  p = strchr(tempargs, ' ');
	  if (p)
	    *p = 0;
	  {
	    EXECARGS ex;

	    ex.flags = SEPARATEKEEP | WINDOWED | MAXIMIZED;
	    ex.commandline = tempargs;
	    *ex.path = 0;
	    *ex.environment = 0;
	    if (WinDlgBox(HWND_DESKTOP,
			  hwnd,
			  CmdLineDlgProc, FM3ModHandle, EXEC_FRAME, &ex) == 1)
	      runemf2(ex.flags,
		      hwnd, pszSrcFile, __LINE__,
		      NULL,
		      (*ex.environment) ? ex.environment : NULL,
		      "%s", tempargs);
	  }
	}
      }
      return 0;

    case DID_OK:
      // fixme to avoid creating empty strings for startlist and endlist
      admp->info->startlist =
	free_and_strdup_from_window(hwnd, AD_STARTLIST,
				    admp->info->startlist);
      admp->info->endlist =
	free_and_strdup_from_window(hwnd, AD_ENDLIST, admp->info->endlist);
      admp->info->id =
	free_and_strdup_from_window(hwnd, AD_ID, admp->info->id);
      admp->info->create =
	free_and_strdup_quoted_from_window(hwnd, AD_ADD, admp->info->create);
      admp->info->createwdirs =
	free_and_strdup_quoted_from_window(hwnd, AD_ADDWPATHS,
				    admp->info->createwdirs);
      admp->info->createrecurse =
	free_and_strdup_quoted_from_window(hwnd, AD_ADDRECURSE,
				    admp->info->createrecurse);
      admp->info->movewdirs =
	free_and_strdup_quoted_from_window(hwnd, AD_MOVEWPATHS,
				    admp->info->movewdirs);
      admp->info->move =
	free_and_strdup_quoted_from_window(hwnd, AD_MOVE, admp->info->move);
      admp->info->delete =
	free_and_strdup_quoted_from_window(hwnd, AD_DELETE, admp->info->delete);
      admp->info->test =
	free_and_strdup_quoted_from_window(hwnd, AD_TEST, admp->info->test);
      admp->info->extract =
	free_and_strdup_quoted_from_window(hwnd, AD_EXTRACT, admp->info->extract);
      admp->info->exwdirs =
	free_and_strdup_quoted_from_window(hwnd, AD_WDIRS, admp->info->exwdirs);
      admp->info->ext =
	free_and_strdup_from_window(hwnd, AD_EXT, admp->info->ext);
      admp->info->signature =
	free_and_strdup_from_window(hwnd, AD_SIG, admp->info->signature);
      admp->info->siglen = literal(admp->info->signature);
      admp->info->list = free_and_strdup_quoted_from_window(hwnd,
						     AD_LIST,
						     admp->info->list);
      admp->info->file_offset = get_long_from_window(hwnd, AD_SIGPOS);
      admp->info->osizepos = get_int_from_window(hwnd, AD_OLDSZ);
      admp->info->nsizepos = get_int_from_window(hwnd, AD_NEWSZ);
      admp->info->fdpos = get_int_from_window(hwnd, AD_DATEPOS);
      admp->info->datetype = get_int2_from_window(hwnd, AD_DATEPOS);
      admp->info->fdflds = get_int_from_window(hwnd, AD_NUMDATEFLDS);
      admp->info->fnpos = get_int_from_window(hwnd, AD_FNAMEPOS);
      admp->info->nameislast =
	(get_int2_from_window(hwnd, AD_FNAMEPOS)) ? TRUE : FALSE;
      admp->info->nameisnext =
	(get_int3_from_window(hwnd, AD_FNAMEPOS)) ? TRUE : FALSE;
      admp->info->nameisfirst =
	(get_int4_from_window(hwnd, AD_FNAMEPOS)) ? TRUE : FALSE;
      {
	INT ok = check_archiver(hwnd, admp->info);

	if (saymsg(MB_YESNO,
		   hwnd,
		   GetPString(IDS_ADCHANGESINMEMTEXT),
		   GetPString(IDS_ADREWRITETEXT),
		   !ok ? GetPString(IDS_NOTRECOMMENDTEXT) : NullStr) ==
	    MBID_YES) {

	  PSZ ab2;

	  ab2 = searchpath(GetPString(IDS_ARCHIVERBB2));	// Rewrite without alerting
	  rewrite_archiverbb2(ab2);
	}
      }
      WinDismissDlg(hwnd, TRUE);
      return 0;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_EDITARC, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, FALSE);
      return 0;

    case AD_TOSTART:
      if (!admp->listname)
	Runtime_Error(pszSrcFile, __LINE__, "no listname");
      else {
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    AD_LISTBOX,
					    LM_QUERYSELECTION,
					    MPVOID, MPVOID);
	WinSendDlgItemMsg(hwnd,
			  AD_LISTBOX,
			  LM_QUERYITEMTEXT,
			  MPFROM2SHORT(sSelect, 255), MPFROMP(s));
	if (*s)
	  WinSetDlgItemText(hwnd, AD_STARTLIST, s);
	else
	BooBoo:
	  saymsg(MB_ENTER,
		 hwnd,
		 GetPString(IDS_OOPSTEXT),
		 GetPString(IDS_SELECTFROMLISTTEXT));
      }
      return 0;

    case AD_TOEND:
      if (!admp->listname)
	Runtime_Error(pszSrcFile, __LINE__, "no listname");
      else {
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    AD_LISTBOX,
					    LM_QUERYSELECTION,
					    MPVOID, MPVOID);
	WinSendDlgItemMsg(hwnd,
			  AD_LISTBOX,
			  LM_QUERYITEMTEXT,
			  MPFROM2SHORT(sSelect, 255), MPFROMP(s));
	if (*s)
	  WinSetDlgItemText(hwnd, AD_ENDLIST, s);
	else
	  goto BooBoo;
      }
      return 0;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(AVV,EditArchiverDefinition,free_and_strdup_from_window)
#pragma alloc_text(AVV,get_int_from_window,get_int2_from_window)
#pragma alloc_text(AVV,get_long_from_window,get_int3_from_window)
#pragma alloc_text(AVV,get_int4_from_window)
