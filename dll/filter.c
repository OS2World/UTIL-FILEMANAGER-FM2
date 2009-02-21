
/***********************************************************************

  $Id$

  Filter mask select dialog

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2008 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  22 Jul 06 SHL Check more run time errors
  29 Jul 06 SHL Use xfgets_bstripcr
  22 Mar 07 GKY Use QWL_USER
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Feb 08 GKY Use xfree where appropriate
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory and use BldFullPathName
  24 Aug 08 GKY Warn full drive on save of .DAT file; prevent loss of existing file
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>

#define INCL_WIN
#define INCL_DOS
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "arccnrs.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "pathutil.h"                   // BldFullPathName
#include "filter.h"
#include "select.h"			// SetMask
#include "literal.h"			// wildcard
#include "strips.h"			// bstrip
#include "misc.h"			// CheckDriveSpaceAvail
#include "wrappers.h"			// xfgets_bstripcr
#include "dirs.h"			// save_dir2
#include "fortify.h"

#pragma data_seg(FILTER_DATA)

static PSZ pszSrcFile = __FILE__;

#define MAXNUMMASKS 50

typedef struct LINKMASKS
{
  CHAR *mask;
  struct LINKMASKS *next;
}
LINKMASKS;

static LINKMASKS *maskhead = NULL;
static BOOL loadedmasks = FALSE;

static VOID save_masks(VOID);

INT APIENTRY Filter(PMINIRECORDCORE rmini, PVOID arg)
{

  MASK *mask = (MASK *) arg;
  PCNRITEM r;
  register INT x;
  INT ret = FALSE;
  CHAR *file;

  if (mask) {
    r = (PCNRITEM) rmini;
    if (!(*(r->pszFileName + 3))
	|| (mask->fShowDirs && (r->attrFile & FILE_DIRECTORY)))
      return TRUE;
    if ((!(mask->attrFile & FILE_HIDDEN) && (r->attrFile & FILE_HIDDEN)) ||
	(!(mask->attrFile & FILE_SYSTEM) && (r->attrFile & FILE_SYSTEM)) ||
	(!(mask->attrFile & FILE_READONLY) && (r->attrFile & FILE_READONLY))
	|| (!(mask->attrFile & FILE_ARCHIVED)
	    && (r->attrFile & FILE_ARCHIVED))
	|| (!(mask->attrFile & FILE_DIRECTORY)
	    && (r->attrFile & FILE_DIRECTORY)))
      return FALSE;
    if (((mask->antiattr & FILE_HIDDEN) && !(r->attrFile & FILE_HIDDEN)) ||
	((mask->antiattr & FILE_SYSTEM) && !(r->attrFile & FILE_SYSTEM)) ||
	((mask->antiattr & FILE_READONLY) && !(r->attrFile & FILE_READONLY))
	|| ((mask->antiattr & FILE_ARCHIVED)
	    && !(r->attrFile & FILE_ARCHIVED))
	|| ((mask->antiattr & FILE_DIRECTORY)
	    && !(r->attrFile & FILE_DIRECTORY)))
        return FALSE;
    if (*mask->szMask) {
      file = strrchr(r->pszFileName, '\\');
      if (!file)
        file = strrchr(r->pszFileName, ':');
      if (file)
        file++;
      else
        file = r->pszFileName;
      if (mask->pszMasks[1]) {
        for (x = 0; mask->pszMasks[x]; x++) {
          if (*mask->pszMasks[x]) {
            if (*mask->pszMasks[x] != '/') {
              if (wildcard((strchr(mask->pszMasks[x], '\\') ||
                            strchr(mask->pszMasks[x], ':')) ?
                           r->pszFileName : file, mask->pszMasks[x], FALSE))
                ret = TRUE;
            }
            else {
              if (wildcard((strchr(mask->pszMasks[x], '\\') ||
                            strchr(mask->pszMasks[x], ':')) ?
                           r->pszFileName : file, mask->pszMasks[x] + 1,
                           FALSE)) {
                ret = FALSE;
                break;
              }
            }
          }
        }
      }
      else {
        if (wildcard((strchr(mask->szMask, '\\') ||
                      strchr(mask->szMask, ':')) ?
                     r->pszFileName : file, mask->szMask, FALSE))
          ret = TRUE;
      }
    }
    else
      ret = TRUE;
  }
  else
    ret = TRUE;
  return ret;
}

VOID load_masks(VOID)
{

  /* load linked list of filter masks from FILTERS.DAT file */

  FILE *fp;
  LINKMASKS *info, *last = NULL;
  CHAR s[CCHMAXPATH + 24];

  loadedmasks = TRUE;
  BldFullPathName(s, pFM2SaveDirectory, PCSZ_FILTERSDAT);
  fp = _fsopen(s, "r", SH_DENYWR);
  if (fp) {
    while (!feof(fp)) {
      if (!xfgets_bstripcr(s, sizeof(s), fp, pszSrcFile, __LINE__))
	break;
      if (*s && *s != ';') {
	info = xmalloc(sizeof(LINKMASKS), pszSrcFile, __LINE__);
	if (info) {
	  info->mask = xstrdup(s, pszSrcFile, __LINE__);
	  if (info->mask) {
	    info->next = NULL;
	    if (!maskhead)
	      maskhead = info;
	    else
	      last->next = info;
	    last = info;
	  }
	  else
	    free(info);
	}
      }
    }  //while
    fclose(fp);
  }
}

VOID save_masks(VOID)
{

  /* save linked list of filter masks to FILTERS.DAT file */

  LINKMASKS *info;
  FILE *fp;
  CHAR s[CCHMAXPATH + 14];

  if (!loadedmasks)
    return;
  if (maskhead) {
    BldFullPathName(s, pFM2SaveDirectory, PCSZ_FILTERSDAT);
    if (CheckDriveSpaceAvail(s, ullDATFileSpaceNeeded, 1) == 2)
    return; //already gave error msg
    fp = xfopen(s, "w", pszSrcFile, __LINE__);
    if (fp) {
      fputs(GetPString(IDS_FILTERFILETEXT), fp);
      info = maskhead;
      while (info) {
	fprintf(fp, "%0.*s\n", CCHMAXPATH, info->mask);
	info = info->next;
      }
      fclose(fp);
    }
  }
}

VOID add_mask(CHAR * mask)
{

  LINKMASKS *info, *last = NULL;

  if (!mask || !*mask)
    return;
  if (!loadedmasks)
    load_masks();
  info = maskhead;
  while (info) {
    if (!stricmp(info->mask, mask))
      return;
    last = info;
    info = info->next;
  }
  info = xmalloc(sizeof(LINKMASKS), pszSrcFile, __LINE__);
  if (info) {
    info->mask = xstrdup(mask, pszSrcFile, __LINE__);
    if (info->mask) {
      info->next = NULL;
      if (!maskhead)
	maskhead = info;
      else
	last->next = info;
    }
    else
      free(info);
  }
}

VOID remove_mask(CHAR * mask)
{

  LINKMASKS *info, *last = NULL;

  if (!mask || !*mask)
    return;
  if (!loadedmasks)
    load_masks();
  info = maskhead;
  while (info) {
    if (!stricmp(info->mask, mask)) {
      if (last)
	last->next = info->next;
      else
	maskhead = info->next;
      xfree(info->mask, pszSrcFile, __LINE__);
      free(info);
      break;
    }
    last = info;
    info = info->next;
  }
}

MRESULT EXPENTRY PickMaskDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, MSK_MASK, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    if (!mp2) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    if (!loadedmasks)
      load_masks();
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    {					/* fill list box */
      LINKMASKS *info;

      info = maskhead;
      while (info) {
	WinSendDlgItemMsg(hwnd, MSK_LISTBOX, LM_INSERTITEM,
			  MPFROM2SHORT(LIT_SORTASCENDING, 0),
			  MPFROMP(info->mask));
	info = info->next;
      }
    }
    {
      MASK *mask = (MASK *) mp2;

      if (mask->fNoAttribs) {
	WinEnableWindow(WinWindowFromID(hwnd, MSK_SYSTEM), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_HIDDEN), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_READONLY), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_ARCHIVED), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_DIRECTORY), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTSYSTEM), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTHIDDEN), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTREADONLY), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTARCHIVED), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTDIRECTORY), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_SHOWDIRS), FALSE);
      }
      else {
	WinCheckButton(hwnd, MSK_SYSTEM, (mask->attrFile & FILE_SYSTEM) != 0);
	WinCheckButton(hwnd, MSK_HIDDEN, (mask->attrFile & FILE_HIDDEN) != 0);
	WinCheckButton(hwnd, MSK_READONLY,
		       (mask->attrFile & FILE_READONLY) != 0);
	WinCheckButton(hwnd, MSK_ARCHIVED,
		       (mask->attrFile & FILE_ARCHIVED) != 0);
	WinCheckButton(hwnd, MSK_DIRECTORY,
		       (mask->attrFile & FILE_DIRECTORY) != 0);
	WinCheckButton(hwnd, MSK_MUSTSYSTEM,
		       (mask->antiattr & FILE_SYSTEM) != 0);
	WinCheckButton(hwnd, MSK_MUSTHIDDEN,
		       (mask->antiattr & FILE_HIDDEN) != 0);
	WinCheckButton(hwnd, MSK_MUSTREADONLY,
		       (mask->antiattr & FILE_READONLY) != 0);
	WinCheckButton(hwnd, MSK_MUSTARCHIVED,
		       (mask->antiattr & FILE_ARCHIVED) != 0);
	WinCheckButton(hwnd, MSK_MUSTDIRECTORY,
		       (mask->antiattr & FILE_DIRECTORY) != 0);
	if (mask->fNoDirs)
	  WinEnableWindow(WinWindowFromID(hwnd, MSK_SHOWDIRS), FALSE);
	else
	  WinCheckButton(hwnd, MSK_SHOWDIRS, (mask->fShowDirs != FALSE));
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTSYSTEM),
			(mask->attrFile & FILE_SYSTEM) != 0);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTHIDDEN),
			(mask->attrFile & FILE_HIDDEN) != 0);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTARCHIVED),
			(mask->attrFile & FILE_ARCHIVED) != 0);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTREADONLY),
			(mask->attrFile & FILE_READONLY) != 0);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTDIRECTORY),
			(mask->attrFile & FILE_DIRECTORY) != 0);
      }
      if (*mask->szMask) {

	CHAR s[CCHMAXPATH], *p;

	strcpy(s, mask->szMask);
	if (!strchr(mask->szMask, '?') && !strchr(mask->szMask, '*')) {
	  p = strrchr(mask->szMask, '.');
	  if (p && *(p + 1)) {
	    *s = '*';
	    strcpy(s + 1, p);
	  }
	}
	WinSetDlgItemText(hwnd, MSK_MASK, s);
	WinSendDlgItemMsg(hwnd, MSK_MASK, EM_SETSEL,
			  MPFROM2SHORT(0, CCHMAXPATH), MPVOID);
//          *mask->szMask = 0;
	PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
      }
      if (mask->fIsTree) {
	WinCheckButton(hwnd, MSK_DIRECTORY, TRUE);
	WinCheckButton(hwnd, MSK_SHOWDIRS, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_DIRECTORY), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_SHOWDIRS), FALSE);
      }
      if (mask->fIsSeeAll) {
	WinCheckButton(hwnd, MSK_DIRECTORY, FALSE);
	WinCheckButton(hwnd, MSK_MUSTDIRECTORY, FALSE);
	WinCheckButton(hwnd, MSK_SHOWDIRS, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_DIRECTORY), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTDIRECTORY), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_SHOWDIRS), FALSE);
      }
      if (*mask->prompt)
	WinSetWindowText(hwnd, mask->prompt);
      if (!PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID))
	WinSendMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    }
    break;

  case UM_SETUP:
    {
      MASK *mask = WinQueryWindowPtr(hwnd, QWL_USER);

      if (mask && mask->fText) {

	SWP swpD, swpL, swpE;
	LONG cyScreen;

	cyScreen = SysVal(SV_CYSCREEN);
	WinQueryWindowPos(hwnd, &swpD);
	if (!WinQueryWindowPos(WinWindowFromID(hwnd, MSK_MASK), &swpE))
	  swpE.cy = 18;
	swpE.cy -= 4;
	WinQueryWindowPos(WinWindowFromID(hwnd, MSK_LISTBOX), &swpL);
	WinSetWindowPos(hwnd, HWND_TOP, swpD.x, swpD.y, swpD.cx,
			swpD.cy + swpE.cy + 10, SWP_SIZE);
	WinQueryWindowPos(hwnd, &swpD);
	if (cyScreen && swpD.y + swpD.cy > cyScreen) {
	  swpD.y = (swpD.y + swpD.cy) - cyScreen;
	  WinSetWindowPos(hwnd, HWND_TOP, swpD.x, swpD.y, swpD.cx,
			  swpD.cy, SWP_MOVE);
	}
	if (!WinCreateWindow(hwnd,
			     WC_STATIC,
			     GetPString(IDS_TEXTTITLETEXT),
			     SS_TEXT |
			     DT_VCENTER | DT_RIGHT,
			     swpL.x,
			     swpL.y + swpL.cy + 4,
			     50,
			     swpE.cy, hwnd, HWND_TOP, 65535, NULL, NULL)) {
          Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                    PCSZ_WINCREATEWINDOW);
	}
	if (!WinCreateWindow(hwnd,
			     WC_ENTRYFIELD,
			     NULL,
			     ES_AUTOSCROLL |
			     ES_MARGIN | WS_GROUP | WS_TABSTOP,
			     swpL.x + 54,
			     swpL.y + swpL.cy + 4,
			     swpL.cx - 54,
			     swpE.cy, hwnd, HWND_TOP, MSK_TEXT, NULL, NULL)) {
          Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                    PCSZ_WINCREATEWINDOW);
	}
	WinSendDlgItemMsg(hwnd,
			  MSK_TEXT,
			  EM_SETTEXTLIMIT, MPFROM2SHORT(256, 0), MPVOID);
	if (mask->szText) {
	  WinSetDlgItemText(hwnd, MSK_TEXT, mask->szText);
	  WinSendDlgItemMsg(hwnd,
			    MSK_TEXT,
			    EM_SETSEL, MPFROM2SHORT(0, 256), MPVOID);
	}
      }
      *mask->szText = 0;
    }
    WinShowWindow(hwnd, TRUE);
    return 0;

  case UM_SETDIR:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, MSK_MASK));
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case MSK_SYSTEM:
    case MSK_HIDDEN:
    case MSK_ARCHIVED:
    case MSK_READONLY:
    case MSK_DIRECTORY:
    case MSK_MUSTSYSTEM:
    case MSK_MUSTHIDDEN:
    case MSK_MUSTARCHIVED:
    case MSK_MUSTREADONLY:
    case MSK_MUSTDIRECTORY:
      if (WinQueryButtonCheckstate(hwnd, MSK_SYSTEM))
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTSYSTEM), TRUE);
      else {
	WinCheckButton(hwnd, MSK_MUSTSYSTEM, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTSYSTEM), FALSE);
      }
      if (WinQueryButtonCheckstate(hwnd, MSK_HIDDEN))
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTHIDDEN), TRUE);
      else {
	WinCheckButton(hwnd, MSK_MUSTHIDDEN, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTHIDDEN), FALSE);
      }
      if (WinQueryButtonCheckstate(hwnd, MSK_ARCHIVED))
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTARCHIVED), TRUE);
      else {
	WinCheckButton(hwnd, MSK_MUSTARCHIVED, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTARCHIVED), FALSE);
      }
      if (WinQueryButtonCheckstate(hwnd, MSK_READONLY))
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTREADONLY), TRUE);
      else {
	WinCheckButton(hwnd, MSK_MUSTREADONLY, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTREADONLY), FALSE);
      }
      if (WinQueryButtonCheckstate(hwnd, MSK_DIRECTORY)) {
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTDIRECTORY), TRUE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_SHOWDIRS), TRUE);
      }
      else {
	WinCheckButton(hwnd, MSK_MUSTDIRECTORY, FALSE);
	WinCheckButton(hwnd, MSK_SHOWDIRS, FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTDIRECTORY), FALSE);
	WinEnableWindow(WinWindowFromID(hwnd, MSK_SHOWDIRS), FALSE);
      }
      {
	MASK *mask = INSTDATA(hwnd);

	if (mask) {
	  if (mask->fIsTree) {
	    WinCheckButton(hwnd, MSK_DIRECTORY, TRUE);
	    WinCheckButton(hwnd, MSK_SHOWDIRS, FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_DIRECTORY), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_SHOWDIRS), FALSE);
	  }
	}
      }
      break;

    case MSK_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_SELECT:
	{
	  SHORT sSelect;
	  CHAR tempmask[CCHMAXPATH];

	  sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					      MSK_LISTBOX,
					      LM_QUERYSELECTION,
					      MPFROMSHORT(LIT_FIRST), MPVOID);
	  *tempmask = 0;
	  if (sSelect >= 0)
	    WinSendDlgItemMsg(hwnd,
			      MSK_LISTBOX,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect,
					   CCHMAXPATH), MPFROMP(tempmask));
	  WinSetDlgItemText(hwnd, MSK_MASK, tempmask);
	}
	break;

      case LN_ENTER:
	WinSendDlgItemMsg(hwnd, DID_OK, BM_CLICK, MPFROMSHORT(TRUE), MPVOID);
	break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case MSK_ALL:
      {
	MASK *mask = (MASK *) INSTDATA(hwnd);

	if (mask) {
	  if (!mask->fNoAttribs) {
	    WinCheckButton(hwnd, MSK_SYSTEM, TRUE);
	    WinCheckButton(hwnd, MSK_HIDDEN, TRUE);
	    WinCheckButton(hwnd, MSK_READONLY, TRUE);
	    WinCheckButton(hwnd, MSK_ARCHIVED, TRUE);
	    WinCheckButton(hwnd, MSK_DIRECTORY, TRUE);
	    WinCheckButton(hwnd, MSK_MUSTSYSTEM, FALSE);
	    WinCheckButton(hwnd, MSK_MUSTHIDDEN, FALSE);
	    WinCheckButton(hwnd, MSK_MUSTREADONLY, FALSE);
	    WinCheckButton(hwnd, MSK_MUSTARCHIVED, FALSE);
	    WinCheckButton(hwnd, MSK_MUSTDIRECTORY, FALSE);
	    if (!mask->fNoDirs)
	      WinCheckButton(hwnd, MSK_SHOWDIRS, TRUE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTSYSTEM), TRUE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTHIDDEN), TRUE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTARCHIVED), TRUE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTREADONLY), TRUE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTDIRECTORY), TRUE);
	  }
	  if (mask->fIsTree) {
	    WinCheckButton(hwnd, MSK_DIRECTORY, TRUE);
	    WinCheckButton(hwnd, MSK_MUSTDIRECTORY, FALSE);
	    WinCheckButton(hwnd, MSK_SHOWDIRS, FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_DIRECTORY), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_SHOWDIRS), FALSE);
	  }
	  if (mask->fIsSeeAll) {
	    WinCheckButton(hwnd, MSK_DIRECTORY, FALSE);
	    WinCheckButton(hwnd, MSK_MUSTDIRECTORY, FALSE);
	    WinCheckButton(hwnd, MSK_SHOWDIRS, FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_DIRECTORY), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_MUSTDIRECTORY), FALSE);
	    WinEnableWindow(WinWindowFromID(hwnd, MSK_SHOWDIRS), FALSE);
	  }
	}
      }
      /* intentional fallthru */
    case MSK_CLEAR:
      WinSetDlgItemText(hwnd, MSK_MASK, NullStr);
      break;

    case MSK_DELETE:
    case DID_OK:
      {
	CHAR test[CCHMAXPATH];
	MASK *mask;
	SHORT sSelect;

	mask = INSTDATA(hwnd);
	*test = 0;
	WinQueryDlgItemText(hwnd, MSK_MASK, CCHMAXPATH, test);
	test[CCHMAXPATH - 1] = 0;
	bstrip(test);
	if (SHORT1FROMMP(mp1) == DID_OK) {
	  mask->attrFile =
	    (WinQueryButtonCheckstate(hwnd, MSK_SYSTEM) *
	     FILE_SYSTEM) | (WinQueryButtonCheckstate(hwnd,
						      MSK_HIDDEN) *
			     FILE_HIDDEN) | (WinQueryButtonCheckstate(hwnd,
								      MSK_READONLY)
					     *
					     FILE_READONLY) |
	    (WinQueryButtonCheckstate(hwnd, MSK_ARCHIVED) *
	     FILE_ARCHIVED) | (WinQueryButtonCheckstate(hwnd,
							MSK_DIRECTORY) *
			       FILE_DIRECTORY);
	  mask->antiattr =
	    (WinQueryButtonCheckstate(hwnd, MSK_MUSTSYSTEM) *
	     FILE_SYSTEM) | (WinQueryButtonCheckstate(hwnd,
						      MSK_MUSTHIDDEN) *
			     FILE_HIDDEN) | (WinQueryButtonCheckstate(hwnd,
								      MSK_MUSTREADONLY)
					     *
					     FILE_READONLY) |
	    (WinQueryButtonCheckstate(hwnd, MSK_MUSTARCHIVED) *
	     FILE_ARCHIVED) | (WinQueryButtonCheckstate(hwnd,
							MSK_MUSTDIRECTORY) *
			       FILE_DIRECTORY);
	  mask->fShowDirs =
	    (WinQueryButtonCheckstate(hwnd, MSK_SHOWDIRS) != FALSE);
	  if (mask->fText)
	    WinQueryDlgItemText(hwnd, MSK_TEXT, 256, mask->szText);
	}
	if (*test) {
	  if (SHORT1FROMMP(mp1) == DID_OK) {
	    strcpy(mask->szMask, test);
	    add_mask(test);
	    save_masks();
	    //DosEnterCritSec(); // GKY 11-30-08 moved to SetMask
	    SetMask(mask->szMask, mask);
	    //DosExitCritSec();
	    WinDismissDlg(hwnd, 1);
	  }
	  else {
	    WinSetDlgItemText(hwnd, MSK_MASK, NullStr);
	    remove_mask(test);
	    save_masks();
	    sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
						MSK_LISTBOX,
						LM_SEARCHSTRING,
						MPFROM2SHORT(0, LIT_FIRST),
						MPFROMP(test));
	    if (sSelect >= 0)
	      WinSendDlgItemMsg(hwnd,
				MSK_LISTBOX,
				LM_DELETEITEM,
				MPFROM2SHORT(sSelect, 0), MPVOID);
	  }
	}
	else {
	  if (SHORT1FROMMP(mp1) == DID_OK) {
	    *mask->szMask = 0;
	    //DosEnterCritSec(); // GKY 11-30-08 moved to SetMask
	    SetMask(mask->szMask, mask);
	    //DosExitCritSec();
	    WinDismissDlg(hwnd, 1);
	  }
	  else if (!fAlertBeepOff)
	    DosBeep(50, 100);		// MSK_DELETE
	}
      }
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp,
		   HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_FILTER, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;
  }

  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(FILTER,Filter)
#pragma alloc_text(MASKS,load_masks,save_masks,add_mask,remove_mask,PickMaskDlgProc)
