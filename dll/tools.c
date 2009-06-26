
/***********************************************************************

  $Id$

  Toolbar support routines

  Copyright (c) 1994-97 M. Kimes
  Copyright (c) 2004, 2008 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  22 Jul 06 SHL Check more run time errors
  29 Jul 06 SHL Use xfgets, xfgets_bstripcr
  18 Aug 06 SHL Report more runtime errors
  05 Sep 06 SHL docopyf filename args must be variables
  05 Sep 06 SHL Sync with standard source formatting
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Feb 08 GKY Use xfree where appropriate
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory and use BldFullPathName
  24 Aug 08 GKY Warn full drive on save of .DAT & .TLS files; prevent loss of existing file
  26 Aug 08 GKY Require unique ID plus text and help strings for all tools save toolbar on button delete
  01 Sep 08 GKY Save toolbars immediately on change.
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "tools.h"
#include "arccnrs.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "pathutil.h"                   // BldFullPathName
#include "fortify.h"
#include "loadbmp.h"			// LoadBitmapFromFileNum
#include "copyf.h"			// docopyf
#include "literal.h"			// literal
#include "wrappers.h"			// xfgets
#include "misc.h"			// CheckDriveSpaceAvail
#include "srchpath.h"			// searchpath
#include "stristr.h"			// stristr
#include "valid.h"			// IsFile
#include "systemf.h"			// runemf2
#include "dirs.h"			// save_dir2
#include "strips.h"			// bstrip

// Data definitions
#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

#pragma data_seg(GLOBAL1)
BOOL fToolsChanged;
TOOL *toolhead = NULL;

#pragma data_seg(GLOBAL2)
CHAR lasttoolbar[CCHMAXPATH];
BOOL qtloaded;
CHAR *quicktool[50];

//== load_quicktools() build *.tls array ==

VOID load_quicktools(VOID)
{
  FILE *fp;
  CHAR s[CCHMAXPATH + 14];
  INT x;

  qtloaded = TRUE;
  for (x = 0; x < 50 && quicktool[x]; x++) {
    free(quicktool[x]);
    quicktool[x] = NULL;
  }
  if (!fToolbar) {
    qtloaded = FALSE;
    return;
  }
  BldFullPathName(s, pFM2SaveDirectory, PCSZ_QUICKTLSDAT);
  fp = _fsopen(s, "r", SH_DENYWR);
  if (fp) {
    x = 0;
    while (!feof(fp)) {
      if (!xfgets_bstripcr(s, CCHMAXPATH + 2, fp, pszSrcFile, __LINE__))
	break;
      if (!*s || *s == ';')
	continue;
      if (x >= 50) {
	Runtime_Error(pszSrcFile, __LINE__, "add");
	break;
      }
      quicktool[x] = xstrdup(s, pszSrcFile, __LINE__);
      if (!quicktool[x])
	break;
      x++;
    }
    fclose(fp);
  }
}

VOID save_quicktools(VOID)
{
  FILE *fp;
  INT x = 0;
  CHAR s[CCHMAXPATH + 14];

  if (!quicktool[0])
    return;
  BldFullPathName(s, pFM2SaveDirectory, PCSZ_QUICKTLSDAT);
  if (CheckDriveSpaceAvail(s, ullDATFileSpaceNeeded, 1) == 2)
    return; //already gave error msg
  fp = xfopen(s, "w", pszSrcFile, __LINE__);
  if (fp) {
    for (x = 0; quicktool[x] && x < 50; x++)
      fprintf(fp, "%s\n", quicktool[x]);
    fclose(fp);
  }
}

//== load_tools() Build tools list given .tls filename ==

TOOL *load_tools(CHAR * filename)
{
  FILE *fp;
  CHAR help[80], text[80], flagstr[80], idstr[80], *fname;
  TOOL *info;

  if (!fToolbar) {
    toolhead = free_tools();
    return toolhead;
  }
  if (!filename || !*filename)
    filename = (*lasttoolbar) ? lasttoolbar : "CMDS.TLS";
  if (*filename)
    fname = searchpath(filename);
  if (!fname || !*fname)
    fname = (PSZ) PCSZ_FM3TOOLSDAT;
  if (fname && *fname) {
    filename = fname;
    strcpy(lasttoolbar, filename);
    fp = _fsopen(filename, "r", SH_DENYWR);
    if (fp) {
      toolhead = free_tools();
      while (!feof(fp)) {
	do {
	  if (!xfgets(help, sizeof(help), fp, pszSrcFile, __LINE__))
	    break;
	} while (*help == ';' && !feof(fp));
	stripcr(help);
	if (!xfgets(text, sizeof(text), fp, pszSrcFile, __LINE__))
	  break;
	stripcr(text);
	if (!xfgets(flagstr, sizeof(flagstr), fp, pszSrcFile, __LINE__))
	  break;
	if (!xfgets(idstr, sizeof(idstr), fp, pszSrcFile, __LINE__))
	  break;
	if (!(USHORT) atoi(idstr))
	  continue;
	info = xmallocz(sizeof(TOOL), pszSrcFile, __LINE__);
	if (info) {
#	  ifdef FORTIFY
          Fortify_SetOwner(info, 1);
          Fortify_SetScope(info, 1);
#	  endif
	  if (*help) {
	    literal(help);
	    if (*help) {
	      info->help = xstrdup(help, pszSrcFile, __LINE__);
#	      ifdef FORTIFY
              Fortify_SetOwner(info->help, 1);
              Fortify_SetScope(info->help, 1);
#	      endif
	    }
	  }
	  if (*text) {
	    info->text = xstrdup(text, pszSrcFile, __LINE__);
#	    ifdef FORTIFY
            Fortify_SetOwner(info->text, 1);
            Fortify_SetScope(info->text, 1);
#	    endif
	  }
	  info->flags = (atoi(flagstr) & (~(T_TEXT | T_EMPHASIZED)));
	  info->id = (USHORT) atoi(idstr);
	  info->next = NULL;
	  add_tool(info);
	}
      }
      fclose(fp);
      fToolsChanged = FALSE;
    }
  }
  return toolhead;
}

VOID save_tools(CHAR * filename)
{
  FILE *fp;
  CHAR *fname;
  TOOL *info;

  if (!filename)
    filename = lasttoolbar;
  if (*filename)
    fname = searchpath(filename);
  if (fname && *fname)
    filename = fname;
  else {
    if (*lasttoolbar)
      filename = lasttoolbar;
    else
      filename = "FM3TOOLS.TLS";
    fname = searchpath(filename);
    if (fname && *fname)
      filename = fname;
  }

  if (stristr(filename, PCSZ_FM3TOOLSDAT))
    filename = "FM3TOOLS.TLS";
  if (toolhead && filename && *filename) {
    strcpy(lasttoolbar, filename);
    PrfWriteProfileString(fmprof, FM3Str, "LastToolbar", filename);
  }
  if (!toolhead) {
    unlinkf(filename);
    return;
  }
  if (CheckDriveSpaceAvail(filename, ullDATFileSpaceNeeded, 1) == 2)
    return; //already gave error msg
  fp = xfopen(filename, "w", pszSrcFile, __LINE__);
  if (fp) {
    fprintf(fp, GetPString(IDS_TOOLFILETEXT), filename);
    info = toolhead;
    while (info) {
      fprintf(fp,
	      "%s\n%s\n%u\n%u\n;\n",
	      (info->help) ? info->help : NullStr,
	      (info->text) ? info->text : NullStr,
	      (info->flags & (~(T_EMPHASIZED | T_TEXT))), info->id);
      info = info->next;
    }
    fclose(fp);
    fToolsChanged = FALSE;
  }
  if (hwndMain)
    PostMsg(hwndMain, UM_FILLBUTTONLIST, MPVOID, MPVOID);
}

TOOL *add_tool(TOOL * tool)
{
  TOOL *info;

  if (tool) {
    info = toolhead;
    if (info) {
      while (info->next)
	info = info->next;
    }
    if (info)
      info->next = tool;
    else
      toolhead = tool;
    fToolsChanged = TRUE;
  }
  return toolhead;
}

TOOL *insert_tool(TOOL * tool, TOOL * after)
{
  if (tool) {
    if (!toolhead)
      return add_tool(tool);
    if (!after) {
      tool->next = toolhead;
      toolhead = tool;
      fToolsChanged = TRUE;
    }
    else {
      tool->next = after->next;
      after->next = tool;
      fToolsChanged = TRUE;
    }
  }
  return toolhead;
}

TOOL *del_tool(TOOL * tool)
{
  TOOL *info, *prev = NULL;

  if (tool) {
    info = toolhead;
    while (info) {
      if (info == tool) {
	if (info == toolhead)
	  toolhead = info->next;
	if (prev)
	  prev->next = info->next;
	xfree(info->help, pszSrcFile, __LINE__);
	xfree(info->text, pszSrcFile, __LINE__);
	free(info);
	fToolsChanged = TRUE;
	break;
      }
      prev = info;
      info = info->next;
    }
  }
  return toolhead;
}

TOOL *find_tool(USHORT id)
{
  TOOL *tool;

  if (id) {
    tool = toolhead;
    while (tool) {
      if (id && tool->id == id)
	return tool;
      tool = tool->next;
    }
  }
  return NULL;
}

TOOL *next_tool(TOOL * tool, BOOL skipinvisible)
{
  while (tool) {
    if (tool->next && (skipinvisible && (tool->next->flags & T_INVISIBLE)))
      tool = tool->next;
    else
      return (tool->next) ? tool->next : toolhead;
  }
  return NULL;
}

TOOL *prev_tool(TOOL * tool, BOOL skipinvisible)
{
  TOOL *info;

Again:
  while (tool) {
    info = toolhead;
    while (info) {
      if (info->next == tool) {
	if (skipinvisible && (info->flags & T_INVISIBLE)) {
	  tool = info;
	  goto Again;
	}
	return info;
      }
      if (!info->next && tool == toolhead)
	return info;
      info = info->next;
    }
    return toolhead;
  }
  return NULL;
}

TOOL *swap_tools(TOOL * tool1, TOOL * tool2)
{
  TOOL *prev1 = NULL, *prev2 = NULL, *info;

  if (tool1 && tool2 && tool1 != tool2) {
    info = toolhead;
    while (info && !prev1 && !prev2) {
      if (info->next == tool1)
	prev1 = info;
      else if (info->next == tool2)
	prev2 = info;
      info = info->next;
    }
    info = tool2;
    tool2 = tool1;
    tool1 = info;
    info = tool2->next;
    if (prev1)
      prev1->next = tool2;
    if (prev2)
      prev2->next = tool1;
    tool2->next = tool1->next;
    tool1->next = info;
    fToolsChanged = TRUE;
  }
  return toolhead;
}

TOOL *free_tools(VOID)
{
  TOOL *tool, *next;

  tool = toolhead;
  while (tool) {
    next = tool->next;
    xfree(tool->help, pszSrcFile, __LINE__);
    xfree(tool->text, pszSrcFile, __LINE__);
    free(tool);
    tool = next;
  }
  toolhead = NULL;
  return toolhead;
}

MRESULT EXPENTRY ReOrderToolsProc(HWND hwnd, ULONG msg, MPARAM mp1,
				  MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    if (!toolhead || !toolhead->next)
      WinDismissDlg(hwnd, 0);
    WinSetWindowText(hwnd, GetPString(IDS_RETOOLTEXT));
    {
      TOOL *tool;
      CHAR s[133];
      SHORT sSelect;

      tool = toolhead;
      while (tool) {
	sprintf(s, "%-5u  %s", tool->id, (tool->help) ? tool->help : "?");
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
					    RE_ADDLISTBOX,
					    LM_INSERTITEM,
					    MPFROMSHORT(LIT_END), MPFROMP(s));
	if (sSelect >= 0)
	  WinSendDlgItemMsg(hwnd,
			    RE_ADDLISTBOX,
			    LM_SETITEMHANDLE,
			    MPFROMSHORT(sSelect), MPFROMLONG((ULONG) tool));
	tool = tool->next;
      }
    }
    break;

  case WM_CONTROL:
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case DID_OK:
      {
	TOOL *tool, *thead = NULL, *last = NULL;
	SHORT sSelect = 0, numitems;

	numitems = (SHORT) WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX,
					     LM_QUERYITEMCOUNT,
					     MPVOID, MPVOID);
	while (numitems) {
	  tool = (TOOL *) WinSendDlgItemMsg(hwnd, RE_REMOVELISTBOX,
					    LM_QUERYITEMHANDLE,
					    MPFROMSHORT(sSelect++), MPVOID);
	  if (tool) {
	    if (!thead)
	      thead = tool;
	    else
	      last->next = tool;
	    last = tool;
	  }
	  numitems--;
	}
	sSelect = 0;
	numitems = (SHORT) WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX,
					     LM_QUERYITEMCOUNT,
					     MPVOID, MPVOID);
	while (numitems) {
	  tool = (TOOL *) WinSendDlgItemMsg(hwnd, RE_ADDLISTBOX,
					    LM_QUERYITEMHANDLE,
					    MPFROMSHORT(sSelect++), MPVOID);
	  if (tool) {
	    if (!thead)
	      thead = tool;
	    else
	      last->next = tool;
	    last = tool;
	  }
	  numitems--;
	}
	if (last)
	  last->next = NULL;
	toolhead = thead;
      }
      save_tools(NULL);
      WinDismissDlg(hwnd, 1);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_REORDERBUTTONS, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;

    case RE_ADD:
      {
	SHORT sSelect, sSelect2;
	CHAR s[133];
	TOOL *tool;

	sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
					     RE_ADDLISTBOX,
					     LM_QUERYSELECTION,
					     MPFROMSHORT(LIT_FIRST), MPVOID);
	while (sSelect >= 0) {
	  tool = (TOOL *) WinSendDlgItemMsg(hwnd,
					    RE_ADDLISTBOX,
					    LM_QUERYITEMHANDLE,
					    MPFROMSHORT(sSelect), MPVOID);
	  if (tool) {
	    sprintf(s, "%-5u  %s", tool->id, (tool->help) ? tool->help : "?");
	    sSelect2 = (SHORT) WinSendDlgItemMsg(hwnd,
						 RE_REMOVELISTBOX,
						 LM_INSERTITEM,
						 MPFROM2SHORT(LIT_END, 0),
						 MPFROMP(s));
	    if (sSelect2 >= 0)
	      WinSendDlgItemMsg(hwnd,
				RE_REMOVELISTBOX,
				LM_SETITEMHANDLE,
				MPFROMSHORT(sSelect2),
				MPFROMLONG((ULONG) tool));
	    WinSendDlgItemMsg(hwnd,
			      RE_ADDLISTBOX,
			      LM_DELETEITEM, MPFROMSHORT(sSelect), MPVOID);
	  }
	  else
	    WinSendDlgItemMsg(hwnd,
			      RE_ADDLISTBOX,
			      LM_SELECTITEM,
			      MPFROMSHORT(sSelect), MPFROMSHORT(FALSE));
	  sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
					       RE_ADDLISTBOX,
					       LM_QUERYSELECTION,
					       MPFROMSHORT(LIT_FIRST),
					       MPVOID);
	}
      }
      break;

    case RE_REMOVE:
      {
	SHORT sSelect, sSelect2;
	CHAR s[133];
	TOOL *tool;

	sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
					     RE_REMOVELISTBOX,
					     LM_QUERYSELECTION,
					     MPFROMSHORT(LIT_FIRST), MPVOID);
	while (sSelect >= 0) {
	  tool = (TOOL *) WinSendDlgItemMsg(hwnd,
					    RE_REMOVELISTBOX,
					    LM_QUERYITEMHANDLE,
					    MPFROMSHORT(sSelect), MPVOID);
	  if (tool) {
	    sprintf(s, "%-5u  %s", tool->id, (tool->help) ? tool->help : "?");
	    sSelect2 = (SHORT) WinSendDlgItemMsg(hwnd,
						 RE_ADDLISTBOX,
						 LM_INSERTITEM,
						 MPFROM2SHORT(LIT_END, 0),
						 MPFROMP(s));
	    if (sSelect2 >= 0)
	      WinSendDlgItemMsg(hwnd,
				RE_ADDLISTBOX,
				LM_SETITEMHANDLE,
				MPFROMSHORT(sSelect2),
				MPFROMLONG((ULONG) tool));
	    WinSendDlgItemMsg(hwnd,
			      RE_REMOVELISTBOX,
			      LM_DELETEITEM, MPFROMSHORT(sSelect), MPVOID);
	  }
	  else
	    WinSendDlgItemMsg(hwnd,
			      RE_REMOVELISTBOX,
			      LM_SELECTITEM,
			      MPFROMSHORT(sSelect), MPFROMSHORT(FALSE));
	  sSelect = (USHORT) WinSendDlgItemMsg(hwnd,
					       RE_REMOVELISTBOX,
					       LM_QUERYSELECTION,
					       MPFROMSHORT(LIT_FIRST),
					       MPVOID);
	}
      }
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY AddToolProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSetWindowPtr(hwnd, QWL_USER, mp2);
    if (mp2) {
      WinSetWindowText(hwnd, GetPString(IDS_EDITTOOLTEXT));
      WinSendDlgItemMsg(hwnd, ADDBTN_ID, EM_SETREADONLY,
			MPFROM2SHORT(TRUE, 0), MPVOID);
    }
    WinSendDlgItemMsg(hwnd, ADDBTN_HELP, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(80, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, ADDBTN_TEXT, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(80, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, ADDBTN_ID, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(5, 0), MPVOID);
    if (!mp2)
      WinCheckButton(hwnd, ADDBTN_VISIBLE, TRUE);
    else {
      TOOL *tool = (TOOL *) mp2;
      CHAR s[33];

      if (tool->help)
	WinSetDlgItemText(hwnd, ADDBTN_HELP, tool->help);
      if (tool->text)
	WinSetDlgItemText(hwnd, ADDBTN_TEXT, tool->text);
      if (tool->flags & T_MYICON)
	WinCheckButton(hwnd, ADDBTN_MYICON, TRUE);
      else
	WinEnableWindow(WinWindowFromID(hwnd, ADDBTN_EDITBMP), FALSE);
      if (tool->flags & T_DROPABLE)
	WinCheckButton(hwnd, ADDBTN_DROPABLE, TRUE);
      if (!(tool->flags & T_INVISIBLE))
	WinCheckButton(hwnd, ADDBTN_VISIBLE, TRUE);
      if (tool->flags & T_SEPARATOR)
	WinCheckButton(hwnd, ADDBTN_SEPARATOR, TRUE);
      if (tool->flags & T_TEXT)
	WinCheckButton(hwnd, ADDBTN_SHOWTEXT, TRUE);
      sprintf(s, "%u", tool->id);
      WinSetDlgItemText(hwnd, ADDBTN_ID, s);
      WinEnableWindow(WinWindowFromID(hwnd, ADDBTN_SHOWTEXT), FALSE);
    }
    WinShowWindow(WinWindowFromID(hwnd, ADDBTN_SHOWTEXT), FALSE);
    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, ADDBTN_HELPME), (HPS) 0, FALSE,
			TRUE);
    PaintRecessedWindow(WinWindowFromID(hwnd, ADDBTN_BMP), (HPS) 0, TRUE,
			FALSE);
    return 0;

  case UM_SETUP:
    {
      HBITMAP hbm = (HBITMAP) 0, hbmd, hbmdd;
      HPS hps;
      CHAR idstr[7];
      USHORT id;

      *idstr = 0;
      WinQueryDlgItemText(hwnd, ADDBTN_ID, 6, idstr);
      id = atoi(idstr);
      if (id) {
	hps = WinGetPS(WinWindowFromID(hwnd, ADDBTN_BMP));
	if (!WinQueryButtonCheckstate(hwnd, ADDBTN_MYICON))
	  hbm = GpiLoadBitmap(hps, 0, id, 28, 28);
	if (!hbm)
	  hbm = LoadBitmapFromFileNum(id);
	if (hbm) {
	  hbmd = (HBITMAP) WinSendDlgItemMsg(hwnd, ADDBTN_BMP, SM_QUERYHANDLE,
					     MPVOID, MPVOID);
	  hbmdd = (HBITMAP) WinSendDlgItemMsg(hwnd, ADDBTN_BMP, SM_SETHANDLE,
					      MPFROMLONG(hbm), MPVOID);
	  if (hbmdd && hbmd && hbmd != hbmdd)
	    GpiDeleteBitmap(hbmd);
	}
      }
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case ADDBTN_HELP:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ADDBTN_HELPME, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, ADDBTN_HELPME,
			  GetPString(IDS_ADDTOOLQUICKHELPTEXT));
      break;

    case ADDBTN_TEXT:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS)
	WinSetDlgItemText(hwnd, ADDBTN_HELPME, NullStr);
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd, ADDBTN_HELPME,
			  GetPString(IDS_ADDTOOLBUTTONTEXT));
      break;

    case ADDBTN_ID:
      if (SHORT2FROMMP(mp1) == EN_KILLFOCUS) {
	WinSetDlgItemText(hwnd, ADDBTN_HELPME, NullStr);
	PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      }
      if (SHORT2FROMMP(mp1) == EN_SETFOCUS)
	WinSetDlgItemText(hwnd,
			  ADDBTN_HELPME, GetPString(IDS_ADDTOOLBUTTONIDTEXT));
      break;

    case ADDBTN_MYICON:
      PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
      WinEnableWindow(WinWindowFromID(hwnd, ADDBTN_EDITBMP),
		      WinQueryButtonCheckstate(hwnd, ADDBTN_MYICON));
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      {
	CHAR help[81], text[81], idstr[7];
	BOOL invisible, dropable, separator, istext, myicon;
        TOOL *tool;
        BOOL BadID = FALSE;

        help[0] = text[0] = 0;
	WinQueryDlgItemText(hwnd, ADDBTN_HELP, 80, help);
	WinQueryDlgItemText(hwnd, ADDBTN_TEXT, 80, text);
	if (WinQueryButtonCheckstate(hwnd, ADDBTN_DROPABLE))
	  dropable = TRUE;
	else
	  dropable = FALSE;
	myicon = WinQueryButtonCheckstate(hwnd, ADDBTN_MYICON);
	if (WinQueryButtonCheckstate(hwnd, ADDBTN_VISIBLE))
	  invisible = FALSE;
	else
	  invisible = TRUE;
	if (WinQueryButtonCheckstate(hwnd, ADDBTN_SEPARATOR))
	  separator = TRUE;
	else
	  separator = FALSE;
	if (WinQueryButtonCheckstate(hwnd, ADDBTN_SHOWTEXT))
	  istext = TRUE;
	else
	  istext = FALSE;
	tool = INSTDATA(hwnd);
	if (tool) {			/* just editing strings... */
	  istext = ((tool->flags & T_TEXT) != 0);
	  xfree(tool->help, pszSrcFile, __LINE__);
	  tool->help = NULL;
	  xfree(tool->text, pszSrcFile, __LINE__);
	  tool->text = NULL;
          if (*help && *text && help && text) {
	    tool->help = xstrdup(help, pszSrcFile, __LINE__);
            tool->text = xstrdup(text, pszSrcFile, __LINE__);
          }
          else {
            saymsg(MB_ENTER,
		   hwnd,
		   GetPString(IDS_MISSINGTEXT),
		   GetPString(IDS_TOOLHELPTEXTBLANK));
            WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, ADDBTN_HELP));
            break;
          }
	  tool->flags = (((dropable) ? T_DROPABLE : 0) |
			 ((invisible) ? T_INVISIBLE : 0) |
			 ((separator) ? T_SEPARATOR : 0) |
			 ((myicon) ? T_MYICON : 0) | ((istext) ? T_TEXT : 0));
          save_tools(NULL);
	  WinDismissDlg(hwnd, 1);
	  break;
	}
	*idstr = 0;
	WinQueryDlgItemText(hwnd, ADDBTN_ID, 6, idstr);
	if (!(USHORT) atoi(idstr)) {
          if (!fAlertBeepOff)
	    DosBeep(250, 100);
	  break;
	}
	tool = toolhead;
	while (tool) {
          if (tool->id == (USHORT) atoi(idstr)) { // && tool != tool) {
	    saymsg(MB_ENTER,
		   hwnd,
		   GetPString(IDS_DUPLICATETEXT),
		   GetPString(IDS_TOOLIDEXISTS));
	    WinSetDlgItemText(hwnd, ADDBTN_ID, NullStr);
            WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, ADDBTN_ID));
            BadID =TRUE;
	    break;
	  }
	  tool = tool->next;
        }
        if (BadID)
          break;
	tool = xmallocz(sizeof(TOOL), pszSrcFile, __LINE__);
	if (tool) {
	  if (*help)
	    tool->help = xstrdup(help, pszSrcFile, __LINE__);
	  if (*text)
	    tool->text = xstrdup(text, pszSrcFile, __LINE__);
	  tool->id = (USHORT) atoi(idstr);
	  tool->flags = (((dropable) ? T_DROPABLE : 0) |
			 ((invisible) ? T_INVISIBLE : 0) |
			 ((separator) ? T_SEPARATOR : 0) |
			 ((myicon) ? T_MYICON : 0) | ((istext) ? T_TEXT : 0));
          add_tool(tool);
          save_tools(NULL);
	  WinDismissDlg(hwnd, tool->id);
	}
      }
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case ADDBTN_EDITBMP:
      {
	CHAR idstr[6], filename[34];

	*idstr = 0;
	WinQueryDlgItemText(hwnd, ADDBTN_ID, 6, idstr);
	if (!(USHORT) atoi(idstr)) {
          if (!fAlertBeepOff)
	    DosBeep(250, 100);
	  break;
	}
	sprintf(filename, "%u.BMP", atoi(idstr));
	if (IsFile(filename) != 1) {
	  CHAR s[CCHMAXPATH] = "EMPTY.BMP";

	  docopyf(COPY, s, filename);
	}
	runemf2(SEPARATE | WINDOWED,
                hwnd, pszSrcFile, __LINE__,
                NULL, NULL, "ICONEDIT.EXE %s", filename);
      }
      break;

    case IDM_HELP:
      if (hwndHelp) {
	if (INSTDATA(hwnd))
	  WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_CHANGEBUTTON, 0),
		     MPFROMSHORT(HM_RESOURCEID));
	else
	  WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_ADDBUTTON, 0),
		     MPFROMSHORT(HM_RESOURCEID));
      }
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY PickToolProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    if (mp2) {
      CHAR s[133];

      sprintf(s, GetPString(IDS_PICKTOOLTITLETEXT), (CHAR *)mp2);
      WinSetWindowText(hwnd, s);
    }
    {
      TOOL *tool;
      CHAR s[133];

      tool = toolhead;
      while (tool) {
	sprintf(s, "%-5u  %s", tool->id, (tool->help) ? tool->help : "?");
	WinSendDlgItemMsg(hwnd,
			  PICKBTN_LISTBOX,
			  LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(s));
	tool = tool->next;
      }
    }
    break;

  case WM_CONTROL:
    if (SHORT1FROMMP(mp1) == PICKBTN_LISTBOX) {
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    case DID_OK:
      {
	SHORT sSelect;
	CHAR s[33];

	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, PICKBTN_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(LIT_FIRST), MPVOID);
	if (sSelect >= 0) {
	  *s = 0;
	  WinSendDlgItemMsg(hwnd, PICKBTN_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, 32), MPFROMP(s));
	  if (*s)
	    WinDismissDlg(hwnd, (USHORT) atoi(s));
	}
      }
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ToolIODlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    if (mp2)
      WinSetWindowULong(hwnd, QWL_USER, TRUE);
    else {
      WinSetWindowULong(hwnd, QWL_USER, FALSE);
      WinSetWindowText(hwnd, GetPString(IDS_LOADTOOLBARTITLETEXT));
    }
    WinSendDlgItemMsg(hwnd,
		      SVBTN_ENTRY,
		      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    {
      FILEFINDBUF3 findbuf;
      HDIR hDir;
      ULONG ulSearchCount, x = 0;
      CHAR *masks[] = { "*.TLS", "FM3TOOLS.DAT", NULL };

      if (mp2)
	masks[1] = NULL;
      while (masks[x]) {
	hDir = HDIR_CREATE;
	ulSearchCount = 1;
	DosError(FERR_DISABLEHARDERR);
	if (!DosFindFirst(masks[x],
			  &hDir,
			  FILE_ARCHIVED,
			  &findbuf,
			  sizeof(FILEFINDBUF3),
			  &ulSearchCount, FIL_STANDARD)) {
	  do {
	    priority_bumped();
	    WinSendMsg(WinWindowFromID(hwnd,
				       SVBTN_LISTBOX),
		       LM_INSERTITEM,
		       MPFROM2SHORT(LIT_SORTASCENDING, 0),
		       MPFROMP(findbuf.achName));
	    ulSearchCount = 1;
	  } while (!DosFindNext(hDir,
				&findbuf,
				sizeof(FILEFINDBUF3), &ulSearchCount));
	  DosFindClose(hDir);
	  priority_bumped();
	}
	x++;
      }
      DosError(FERR_DISABLEHARDERR);
    }
    if (!WinSendDlgItemMsg(hwnd,
			   SVBTN_LISTBOX,
			   LM_QUERYITEMCOUNT, MPVOID, MPVOID)) {
      WinEnableWindow(WinWindowFromID(hwnd, SVBTN_LISTBOX), FALSE);
      PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    }
    WinSetDlgItemText(hwnd,
		      SVBTN_CURRENT,
		      (*lasttoolbar) ? lasttoolbar : PCSZ_FM3TOOLSDAT);
    break;

  case UM_SETUP:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, SVBTN_ENTRY));
    return 0;

  case WM_CONTROL:
    if (SHORT1FROMMP(mp1) == SVBTN_LISTBOX) {
      SHORT sSelect;
      CHAR szBuffer[CCHMAXPATH];

      switch (SHORT2FROMMP(mp1)) {
      case LN_SELECT:
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, SVBTN_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(LIT_FIRST), MPVOID);
	if (sSelect >= 0) {
	  *szBuffer = 0;
	  WinSendDlgItemMsg(hwnd, SVBTN_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, CCHMAXPATH),
			    MPFROMP(szBuffer));
	  if (*szBuffer)
	    WinSetDlgItemText(hwnd, SVBTN_ENTRY, szBuffer);
	}
	break;

      case LN_ENTER:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_HELP:
      if (hwndHelp) {
	if (INSTDATA(hwnd))
	  WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_SAVETOOLS, 0),
		     MPFROMSHORT(HM_RESOURCEID));
	else
	  WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		     MPFROM2SHORT(HELP_LOADTOOLS, 0),
		     MPFROMSHORT(HM_RESOURCEID));
      }
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case DID_OK:
      {
	BOOL saving = WinQueryWindowULong(hwnd, QWL_USER);
	CHAR temptools[CCHMAXPATH];

	strcpy(temptools, lasttoolbar);
	if (fToolsChanged)
	  save_tools(NULL);
	WinQueryDlgItemText(hwnd,
			    SVBTN_ENTRY, sizeof(lasttoolbar), lasttoolbar);
	if (*lasttoolbar) {
	  if (!strchr(lasttoolbar, '.'))
	    strcat(lasttoolbar, PCSZ_DOTTLS);
	}
	if (saving && *lasttoolbar)
	  save_tools(NULL);
	else {
	  if (!load_tools(NULL)) {
	    strcpy(lasttoolbar, temptools);
	    if (!load_tools(NULL)) {
	      *lasttoolbar = 0;
	      load_tools(NULL);
	    }
	  }
	}
	PrfWriteProfileString(fmprof, FM3Str, "LastToolbar", lasttoolbar);
      }
      WinDismissDlg(hwnd, 1);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(TOOLS,load_tools,save_tools,add_tool,insert_tool,del_tool,free_tools,swap_tools)
#pragma alloc_text(TOOLS,load_quicktools,save_quicktools)
#pragma alloc_text(TOOLS1,ReOrderToolsProc,PickToolProc,AddToolProc,ToolIODlgProc)
