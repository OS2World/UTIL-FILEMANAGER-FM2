
/***********************************************************************

  $Id$

  New internal viewer

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2006 Steven H. Levine

  01 Dec 03 SHL Comments
  02 Dec 03 SHL Correct WM_VSCROLL math
  23 May 05 SHL Use QWL_USER
  06 Jun 05 SHL Indent -i2
  06 Jun 05 SHL Correct reversed wrap logic
  17 Jul 06 SHL Use Runtime_Error
  26 Jul 06 SHL Use chop_at_crnl and convert_nl_to_nul

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <process.h>
#include <limits.h>
#include <share.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(NEWVIEW,ViewStatusProc,FreeViewerMem,LoadFile)
#pragma alloc_text(NEWVIEW,InitWindow,PaintLine,ViewWndProc)
#pragma alloc_text(NEWVIEW,ViewFrameWndProc,StartViewer,ReLine)
#pragma alloc_text(NEWVIEW,BuildAList,Search,Clipboard,FindStrDlgProc)
#pragma alloc_text(NEWVIEW,BuildAList2,UrlDlgProc)

#define VF_SELECTED     0x01
#define VF_FOUND        0x02
#define VF_HTTP         0x04
#define VF_FTP          0x08

#define FIXED_FONT_LCID 5

#define COLORS_MAX                   12

#define COLORS_CURSOREDNORMALBACK    0
#define COLORS_CURSOREDSELECTEDBACK  1
#define COLORS_NORMALBACK            2
#define COLORS_SELECTEDBACK          3
#define COLORS_NORMALFORE            4
#define COLORS_FOUNDFORE             5
#define COLORS_SELECTEDFORE          6
#define COLORS_SELECTEDFOUNDFORE     7
#define COLORS_HTTPBACK              8
#define COLORS_HTTPFORE              9
#define COLORS_FTPBACK               10
#define COLORS_FTPFORE               11

static LONG Colors[COLORS_MAX] = {COLR_WHITE, COLR_DARKGRAY,
                                  COLR_PALEGRAY, COLR_BLACK,
                                  COLR_BLACK, COLR_RED,
                                  COLR_WHITE, COLR_YELLOW,
                                  COLR_PALEGRAY, COLR_DARKBLUE,
                                  COLR_PALEGRAY, COLR_DARKGREEN};

#define SEARCHSTRINGLEN 1024

typedef struct
{
  USHORT size;
  USHORT flags;
  USHORT cliptype;
  CHAR filename[CCHMAXPATH];
  CHAR *text;
  CHAR **lines, *markedlines;
  CHAR searchtext[SEARCHSTRINGLEN], *lastpos, szFacename[FACESIZE];
  ULONG textsize, numlines, topline, cursored, selected, numalloc, multiplier,
      lastselected, found;
  CHAR stopflag, busy;
  LONG oldwidth, lastdirection, lMaxAscender, lMaxDescender, lMaxHeight,
      maxx, horzscroll;
  BOOL hex, mousecaptured, sensitive, dummy, literalsearch, clientfocused,
      alsoselect, wrapon, relining, httpin, ftpin, ignorehttp, ignoreftp,
      needrefreshing;
  HMTX ScanSem;
  HWND hvscroll, hwndMenu, hwndStatus1, hwndStatus2, hwndStatus3, hwndRestore,
      hwndPopup, hwndListbox, hwndFrame, hwndDrag, hwndParent, hhscroll;
  HPS hps;
  FATTRS fattrs;
  LONG colors[12];
}
VIEWDATA;

typedef struct
{
  USHORT size;
  USHORT dummy;
  ULONG len;
  CHAR *line;
  CHAR url[SEARCHSTRINGLEN];
}
URLDATA;

static BOOL Sensitive = FALSE;
static USHORT Codepage = 0;
static BOOL Firsttime = TRUE;
static BOOL LiteralSearch = FALSE;
static BOOL AlsoSelect = FALSE;
static BOOL WrapOn = FALSE;
static BOOL IgnoreFTP = FALSE;
static BOOL IgnoreHTTP = FALSE;
static FATTRS Fattrs;

MRESULT EXPENTRY UrlDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  URLDATA *urld;

  switch (msg)
  {
  case WM_INITDLG:
    if (mp2)
    {
      CHAR *p, *e, *pp;
      SHORT count;

      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      urld = mp2;
      e = urld -> line + urld -> len + 1;
      p = urld -> line;
      do
      {
	p = strnstr(p, "http://", e - p);
	if (p)
	{
	  strncpy(urld -> url, p, min(e - p, SEARCHSTRINGLEN - 1));
	  urld -> url[min(e - p, SEARCHSTRINGLEN - 1)] = 0;
	  pp = urld -> url;
	  while (*pp && *pp != ' ' && *pp != '\r' && *pp != '\n' &&
		 *pp != '\"')
	    pp++;
	  *pp = 0;
	  WinSendDlgItemMsg(hwnd, URL_LISTBOX, LM_INSERTITEM,
			    MPFROM2SHORT(LIT_END, 0),
			    MPFROMP(urld -> url));
	  p++;
	}
      }
      while (p && *p && p < e);
      p = urld -> line;
      do
      {
	p = strnstr(p, "ftp://", e - p);
	if (p)
	{
	  strncpy(urld -> url, p, min(e - p, SEARCHSTRINGLEN - 1));
	  urld -> url[min(e - p, SEARCHSTRINGLEN - 1)] = 0;
	  pp = urld -> url;
	  while (*pp && *pp != ' ' && *pp != '\r' && *pp != '\n' &&
		 *pp != '\"')
	    pp++;
	  *pp = 0;
	  WinSendDlgItemMsg(hwnd, URL_LISTBOX, LM_INSERTITEM,
			    MPFROM2SHORT(LIT_END, 0),
			    MPFROMP(urld -> url));
	  p++;
	}
      }
      while (p && *p && p < e);
      *urld -> url = 0;
      count = (SHORT) WinSendDlgItemMsg(hwnd, URL_LISTBOX, LM_QUERYITEMCOUNT,
					MPVOID, MPVOID);
      if (count)
      {
	WinSendDlgItemMsg(hwnd, URL_LISTBOX, LM_SELECTITEM,
			  MPFROMSHORT(0), MPFROMSHORT(TRUE));
	if (count == 1)
	  WinSendMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	else
	  PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
	break;
      }
    }
    WinDismissDlg(hwnd, 0);
    break;

  case UM_SETUP:
    WinShowWindow(hwnd, TRUE);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1))
    {
    case URL_LISTBOX:
      switch (SHORT2FROMMP(mp1))
      {
      case LN_ENTER:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1))
    {
    case URL_BOOKMARK:
      WinDismissDlg(hwnd, 3);
      break;

    case DID_OK:
      {
	SHORT select;

	urld = WinQueryWindowPtr(hwnd, QWL_USER);
	if (urld)
	{
	  select = (SHORT) WinSendDlgItemMsg(hwnd, URL_LISTBOX,
					     LM_QUERYSELECTION,
					     MPFROMSHORT(LIT_FIRST),
					     MPVOID);
	  if (select >= 0)
	  {
	    *urld -> url = 0;
	    WinSendDlgItemMsg(hwnd, URL_LISTBOX, LM_QUERYITEMTEXT,
			      MPFROM2SHORT(select, sizeof(urld -> url)),
			      MPFROMP(urld -> url));
	    if (*urld -> url)
	    {
	      if (!strncmp(urld -> url, "http://", 7))
	      {
		WinDismissDlg(hwnd, 1);
		break;
	      }
	      else if (!strncmp(urld -> url, "ftp://", 6))
	      {
		memmove(urld -> url, urld -> url + 6, strlen(urld -> url) + 1);
		if (*urld -> url)
		{
		  WinDismissDlg(hwnd, 2);
		  break;
		}
	      }
	    }
	  }
	}
      }
      Runtime_Error(pszSrcFile, __LINE__, "no data");
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

static ULONG NumLines(RECTL * rcl, VIEWDATA * ad)
{
  ULONG numlines;

  numlines = (rcl -> yTop - rcl -> yBottom) / ad -> lMaxHeight;
  if (ad -> lMaxDescender && numlines &&
      ((rcl -> yTop - rcl -> yBottom) -
       (numlines * ad -> lMaxHeight) <= ad -> lMaxDescender))
    numlines--;
  return numlines;
}

static CHAR **BuildAList(HWND hwnd)
{
  VIEWDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  register ULONG x, y, z = 0;
  ULONG width;
  RECTL Rectl;
  CHAR **list = NULL, s[SEARCHSTRINGLEN], a;
  register CHAR *p, *e;
  INT numlines = 0, numalloc = 0;

  if (ad && ad -> selected)
  {
    WinQueryWindowRect(hwnd, &Rectl);
    width = (Rectl.xRight - Rectl.xLeft) / ad -> fattrs.lAveCharWidth;
    if (!width && !ad -> hex)
      return list;
    for (x = 0; x < ad -> numlines; x++)
    {
      if (ad -> stopflag)
	break;
      if (ad -> markedlines[x] & VF_SELECTED)
      {
	if (ad -> hex)
	{
	  width = ad -> textsize - (x * 16);
	  width = min(width, 16);
	  sprintf(s, "%08lx ", x * 16);
	  p = s + 9;
	  for (y = 0; y < width; y++)
	  {
	    sprintf(p, " %02hx", ad -> text[(x * 16) + y]);
	    p += 3;
	  }
	  *p = ' ';
	  p++;
	  *p = ' ';
	  p++;
	  for (y = 0; y < width; y++)
	  {
	    a = ad -> text[(x * 16) + y];
	    if (a && a != '\n' && a != '\r' && a != '\t' && a != '\x1a')
	      *p = ad -> text[(x * 16) + y];
	    else
	      *p = '.';
	    p++;
	  }
	  *p = 0;
	}
	else
	{
	  if (!ad -> wrapon)
	  {
	    e = p = ad -> lines[x];
	    while (*e != '\r' && *e != '\n' && e < ad -> text + ad -> textsize)
	      e++;
/*
   if((*e == '\r' || *e == '\n') && e > p)
   e--;
 */
	    width = e - p;
	  }
	  else
	  {
	    p = ad -> lines[x];
	    e = p + (width - 1);
	    if (e - ad -> text > ad -> textsize)
	      e = ad -> text + ad -> textsize;
	    while (p < e)
	    {
	      if (*p == '\r' || *p == '\n')
	      {
		e = p;
		break;
	      }
	      p++;
	    }
	  }
	  strncpy(s, ad -> lines[x], e - ad -> lines[x]);
	  s[e - ad -> lines[x]] = 0;
	}
	if (AddToList(s, &list, &numlines, &numalloc))
	  break;
	z++;
	if (z >= ad -> selected)
	  break;
      }
    }
  }
  return list;
}

static CHAR **BuildAList2(HWND hwnd)
{
  VIEWDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  CHAR **list = NULL, s[SEARCHSTRINGLEN];
  SHORT x, z;
  INT numlines = 0, numalloc = 0;

  if (ad)
  {
    z = (SHORT) WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX,
				  LM_QUERYITEMCOUNT, MPVOID, MPVOID);
    z = max(z, 0);
    for (x = 0; x < z; x++)
    {
      if (ad -> stopflag)
	break;
      *s = 0;
      WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX, LM_QUERYITEMTEXT,
			MPFROM2SHORT(x, SEARCHSTRINGLEN), MPFROMP(s));
      if (*s)
	if (AddToList(s, &list, &numlines, &numalloc))
	  break;
    }
  }
  return list;
}

MRESULT EXPENTRY ViewStatusProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg)
  {
  case WM_CREATE:
    return CommonTextProc(hwnd, msg, mp1, mp2);

  case WM_MOUSEMOVE:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);

      if (fOtherHelp)
      {
	if ((!hwndBubble || WinQueryWindowULong(hwndBubble, QWL_USER) != hwnd) &&
	    !WinQueryCapture(HWND_DESKTOP))
	{

	  char *s = NULL;

	  switch (id)
	  {
	  case NEWVIEW_STATUS2:
	    s = GetPString(IDS_NVSTATUS2HELPTEXT);
	    break;
	  case NEWVIEW_STATUS3:
	    s = GetPString(IDS_NVSTATUS3HELPTEXT);
	    break;
	  case NEWVIEW_DRAG:
	    s = GetPString(IDS_NVDRAGHELPTEXT);
	    break;
	  }
	  if (s && *s)
	    MakeBubble(hwnd, TRUE, s);
	  else if (hwndBubble)
	    WinDestroyWindow(hwndBubble);
	}
      }
      switch (id)
      {
      case NEWVIEW_STATUS1:
	break;
      default:
	return CommonTextButton(hwnd, msg, mp1, mp2);
      }
    }
    break;

  case WM_BUTTON3UP:
  case WM_BUTTON1UP:
  case WM_BUTTON1DOWN:
  case WM_BUTTON3DOWN:
    {
      USHORT id;

      id = WinQueryWindowUShort(hwnd, QWS_ID);
      switch (id)
      {
      case NEWVIEW_STATUS1:
	break;
      default:
	return CommonTextButton(hwnd, msg, mp1, mp2);
      }
    }
    break;

  case UM_CLICKED:
  case UM_CLICKED3:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID), cmd = 0;

      switch (id)
      {
      case NEWVIEW_DRAG:
	if (msg == UM_CLICKED)
	  cmd = (msg == UM_CLICKED) ? IDM_HEXMODE : IDM_DESELECTALL;
	break;
      case NEWVIEW_STATUS2:
	cmd = (msg == UM_CLICKED) ? IDM_GOTOLINE : IDM_FINDFIRST;
	break;
      case NEWVIEW_STATUS3:
	cmd = (msg == UM_CLICKED) ? IDM_GOTOOFFSET : IDM_FINDNEXT;
	break;
      default:
	break;
      }
      PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
			      FID_CLIENT),
	      WM_COMMAND,
	      MPFROM2SHORT(cmd, 0),
	      MPVOID);
    }
    return 0;

  case WM_BEGINDRAG:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);

      switch (id)
      {
      case NEWVIEW_STATUS1:
      case NEWVIEW_DRAG:
	{
	  VIEWDATA *ad = WinQueryWindowPtr(WinWindowFromID(
							WinQueryWindow(hwnd,
								 QW_PARENT),
							    FID_CLIENT), 0);

	  if (ad)
	    DragOne(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
			      FID_CLIENT), (HWND) 0, ad -> filename, FALSE);
	}
	break;
      default:
	break;
      }
    }
    break;

  case WM_CONTEXTMENU:
    PostMsg(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
			    FID_CLIENT),
	    UM_CONTEXTMENU,
	    MPVOID,
	    MPVOID);
    break;

  case WM_SETFOCUS:
    if (mp2)
      PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case WM_PAINT:
    {
      USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);
      ULONG color;
      VIEWDATA *ad = WinQueryWindowPtr(WinWindowFromID(WinQueryWindow(hwnd,
						QW_PARENT), FID_CLIENT), 0);
      SWP swp;
      POINTL ptl;
      HPS hps;

      switch (id)
      {
      case NEWVIEW_STATUS1:
	PaintRecessedWindow(hwnd, (HPS) 0, FALSE, FALSE);
	break;
      default:
	PaintRecessedWindow(hwnd, (HPS) 0, TRUE, FALSE);
	break;
      }
      hps = WinGetPS(WinQueryWindow(hwnd, QW_PARENT));
      if (hps)
      {
	WinQueryWindowPos(hwnd, &swp);
	ptl.x = swp.x - 1;
	ptl.y = swp.y + swp.cy + 2;
	GpiMove(hps, &ptl);
	switch (id)
	{
	case NEWVIEW_STATUS1:
	  if (ad)
	    color = (standardcolors[ad -> colors[COLORS_NORMALBACK]] == CLR_WHITE) ?
	      CLR_PALEGRAY : CLR_WHITE;
	  else
	    color = CLR_WHITE;
	  break;
	default:
	  if (ad)
	    color = (standardcolors[ad -> colors[COLORS_NORMALBACK]] == CLR_PALEGRAY) ?
	      CLR_DARKGRAY : CLR_PALEGRAY;
	  else
	    color = CLR_PALEGRAY;
	  break;
	}
	GpiSetColor(hps, color);
	ptl.x = swp.x + swp.cx;
	GpiLine(hps, &ptl);
	WinReleasePS(hps);
      }
    }
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP,
		WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				FID_CLIENT));
    return 0;
  }
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

static VOID FreeViewerMem(HWND hwnd)
{
  VIEWDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);

  if (ad)
  {
    ad -> selected = ad -> textsize = ad -> numlines = ad -> numalloc = 0;
    if (ad -> text)
      free(ad -> text);
    if (ad -> lines)
      free(ad -> lines);
    if (ad -> markedlines)
      free(ad -> markedlines);
    ad -> text = NULL;
    ad -> lines = NULL;
    ad -> markedlines = NULL;
    DosPostEventSem(CompactSem);
  }
}

static HPS InitWindow(HWND hwnd)
{
  VIEWDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  HPS hps = (HPS) 0;
  SIZEL sizel;
  FONTMETRICS FontMetrics;

  if (ad)
  {
    sizel.cx = sizel.cy = 0;
    hps = GpiCreatePS(WinQueryAnchorBlock(hwnd), WinOpenWindowDC(hwnd),
		      (PSIZEL) & sizel, PU_PELS | GPIF_DEFAULT | GPIT_MICRO |
		      GPIA_ASSOC);
    if (hps)
    {
      GpiSetCp(hps, (ULONG) ad -> fattrs.usCodePage);
      GpiCreateLogFont(hps, NULL, FIXED_FONT_LCID, &ad -> fattrs);
      GpiSetCharSet(hps, FIXED_FONT_LCID);
      GpiQueryFontMetrics(hps, (long) sizeof(FONTMETRICS), &FontMetrics);
      ad -> fattrs.lAveCharWidth = FontMetrics.lAveCharWidth;
      ad -> fattrs.lMaxBaselineExt = FontMetrics.lMaxBaselineExt;
      ad -> lMaxAscender = max(FontMetrics.lMaxAscender, 0);
      ad -> lMaxDescender = max(FontMetrics.lMaxDescender, 0);
      ad -> lMaxHeight = ad -> lMaxAscender + ad -> lMaxDescender;
      if (ad -> fattrs.usCodePage != FontMetrics.usCodePage)
      {
	ad -> fattrs.usCodePage = FontMetrics.usCodePage;
	Codepage = ad -> fattrs.usCodePage;
	PrfWriteProfileData(fmprof,
			    appname,
			    "Viewer.Codepage",
			    &ad -> fattrs.usCodePage,
			    sizeof(USHORT));
      }
      else if (ad -> fattrs.usCodePage)
      {

	HMQ hmq;
	ULONG cps[50], len, x;

	if (!DosQueryCp(sizeof(cps), cps, &len))
	{
	  for (x = 0; x < len / sizeof(ULONG); x++)
	  {
	    if (cps[x] == (ULONG) ad -> fattrs.usCodePage)
	    {
	      hmq = WinQueryWindowULong(hwnd, QWL_HMQ);
	      WinSetCp(hmq, ad -> fattrs.usCodePage);
	      break;
	    }
	  }
	}
	DosSetProcessCp((ULONG) ad -> fattrs.usCodePage);
      }
      GpiSetBackMix(hps, BM_OVERPAINT);
      SetPresParamFromFattrs(WinWindowFromID(ad -> hwndFrame, NEWVIEW_LISTBOX),
			     &ad -> fattrs, FontMetrics.sNominalPointSize,
			  MAKEFIXED(FontMetrics.sNominalPointSize / 10, 0));
    }
  }
  return (hps);
}

static VOID PaintLine(HWND hwnd, HPS hps, ULONG whichline, ULONG topline,
		      RECTL * Rectl)
{
  VIEWDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);
  POINTL ptl;
  ULONG width;
  register CHAR *p, *e;
  CHAR marker[] = " >";
  RECTL rcl2;

  if (ad && (ad -> hex || ad -> lines))
  {
    ptl.y = (Rectl -> yTop - (ad -> lMaxHeight *
			      (((whichline + 1) - topline) + 1)));
    ptl.x = 0;
    GpiMove(hps, &ptl);
    GpiSetBackMix(hps, BM_OVERPAINT);
    if (ad -> markedlines)
    {
      if (ad -> markedlines[whichline] & VF_SELECTED)
      {
	GpiSetColor(hps, ((ad -> markedlines[whichline] & VF_FOUND) != 0) ?
		    standardcolors[ad -> colors[COLORS_SELECTEDFOUNDFORE]] :
		    standardcolors[ad -> colors[COLORS_SELECTEDFORE]]);
	GpiSetBackColor(hps, (whichline == ad -> cursored - 1) ?
		 standardcolors[ad -> colors[COLORS_CURSOREDSELECTEDBACK]] :
			standardcolors[ad -> colors[COLORS_SELECTEDBACK]]);
      }
      else if (ad -> markedlines[whichline] & VF_FOUND)
      {
	GpiSetColor(hps, standardcolors[ad -> colors[COLORS_FOUNDFORE]]);
	GpiSetBackColor(hps, (whichline == ad -> cursored - 1) ?
		   standardcolors[ad -> colors[COLORS_CURSOREDNORMALBACK]] :
			standardcolors[ad -> colors[COLORS_NORMALBACK]]);
      }
      else
      {
	GpiSetColor(hps, standardcolors[ad -> colors[COLORS_NORMALFORE]]);
	GpiSetBackColor(hps, (whichline == ad -> cursored - 1) ?
		   standardcolors[ad -> colors[COLORS_CURSOREDNORMALBACK]] :
			standardcolors[ad -> colors[COLORS_NORMALBACK]]);
      }
    }
    else
    {
      GpiSetColor(hps, standardcolors[ad -> colors[COLORS_NORMALFORE]]);
      GpiSetBackColor(hps, (whichline == ad -> cursored - 1) ?
		   standardcolors[ad -> colors[COLORS_CURSOREDNORMALBACK]] :
		      standardcolors[ad -> colors[COLORS_NORMALBACK]]);
    }
    if (!ad -> hex)
    {
      if (ad -> wrapon)
      {
	width = (Rectl -> xRight - Rectl -> xLeft) / ad -> fattrs.lAveCharWidth;
	if (width)
	{
	  GpiCharString(hps, 1, marker + (whichline == ad -> cursored - 1));
	  p = ad -> lines[whichline];
	  e = p + (width - 1);
	  if (e - ad -> text > ad -> textsize)
	    e = ad -> text + ad -> textsize;
	  while (p < e)
	  {
	    if (*p == '\r' || *p == '\n')
	    {
	      e = p;
	      break;
	    }
	    p++;
	  }
	  if (ad -> ftpin && whichline != ad -> cursored - 1 && (!ad -> markedlines ||
	      !(ad -> markedlines[whichline] & (VF_SELECTED | VF_FOUND))) &&
	      strnstr(ad -> lines[whichline], "ftp://", e - ad -> lines[whichline]))
	  {
	    GpiSetColor(hps, standardcolors[ad -> colors[COLORS_FTPFORE]]);
	    GpiSetBackColor(hps, standardcolors[ad -> colors[COLORS_FTPBACK]]);
	  }
	  if (ad -> httpin && whichline != ad -> cursored - 1 && (!ad -> markedlines ||
	      !(ad -> markedlines[whichline] & (VF_SELECTED | VF_FOUND))) &&
	      strnstr(ad -> lines[whichline], "http://", e - ad -> lines[whichline]))
	  {
	    GpiSetColor(hps, standardcolors[ad -> colors[COLORS_HTTPFORE]]);
	    GpiSetBackColor(hps, standardcolors[ad -> colors[COLORS_HTTPBACK]]);
	  }
	  rcl2 = *Rectl;
	  rcl2.yTop = ptl.y + ad -> lMaxAscender;
	  rcl2.yBottom = ptl.y - ad -> lMaxDescender;
	  GpiCharString(hps, e - ad -> lines[whichline], ad -> lines[whichline]);
	  GpiQueryCurrentPosition(hps, &ptl);
	  rcl2.xLeft = ptl.x;
	  WinFillRect(hps, &rcl2,
		      standardcolors[ad -> colors[COLORS_NORMALBACK]]);
	}
      }
      else
      {
	width = (Rectl -> xRight - Rectl -> xLeft) / ad -> fattrs.lAveCharWidth;
	if (width)
	{
	  GpiCharString(hps, 1, marker + (whichline == ad -> cursored - 1));
	  p = ad -> lines[whichline];
	  e = p + (abs(ad -> horzscroll) / ad -> fattrs.lAveCharWidth);
	  if (e - ad -> text > ad -> textsize)
	    e = ad -> text + ad -> textsize;
	  while (p < e)
	  {
	    if (*p == '\r' || *p == '\n')
	      break;
	    p++;
	  }
	  if (*p != '\r' && *p != '\n')
	  {

	    CHAR *pp;

	    e = p + (width - 1);
	    if (e - ad -> text > ad -> textsize)
	      e = ad -> text + ad -> textsize;
	    pp = p;
	    while (pp < e)
	    {
	      if (*pp == '\r' || *pp == '\n')
	      {
		e = pp;
		break;
	      }
	      pp++;
	    }
	  }
	  else
	    e = p;
	  if (ad -> ftpin && whichline != ad -> cursored - 1 && (!ad -> markedlines ||
	      !(ad -> markedlines[whichline] & (VF_SELECTED | VF_FOUND))) &&
	      strnstr(ad -> lines[whichline], "ftp://", e - ad -> lines[whichline]))
	  {
	    GpiSetColor(hps, standardcolors[ad -> colors[COLORS_FTPFORE]]);
	    GpiSetBackColor(hps, standardcolors[ad -> colors[COLORS_FTPBACK]]);
	  }
	  if (ad -> httpin && whichline != ad -> cursored - 1 && (!ad -> markedlines ||
	      !(ad -> markedlines[whichline] & (VF_SELECTED | VF_FOUND))) &&
	      strnstr(ad -> lines[whichline], "http://", e - ad -> lines[whichline]))
	  {
	    GpiSetColor(hps, standardcolors[ad -> colors[COLORS_HTTPFORE]]);
	    GpiSetBackColor(hps, standardcolors[ad -> colors[COLORS_HTTPBACK]]);
	  }
	  rcl2 = *Rectl;
	  rcl2.yTop = ptl.y + ad -> lMaxAscender;
	  rcl2.yBottom = ptl.y - ad -> lMaxDescender;
	  GpiCharString(hps, e - p, p);
	  GpiQueryCurrentPosition(hps, &ptl);
	  rcl2.xLeft = ptl.x;
	  WinFillRect(hps, &rcl2,
		      standardcolors[ad -> colors[COLORS_NORMALBACK]]);
	}
      }
    }
    else
    {

      CHAR s[80];
      register ULONG x;

      rcl2 = *Rectl;
      rcl2.yTop = ptl.y + ad -> lMaxAscender;
      rcl2.yBottom = ptl.y - ad -> lMaxDescender;
      GpiCharString(hps, 1, marker + (whichline == ad -> cursored - 1));
      width = ad -> textsize - (whichline * 16);
      width = min(width, 16);
      sprintf(s,
	      "%08lx ",
	      whichline * 16);
      p = s + 9;
      for (x = 0; x < width; x++)
      {
	sprintf(p,
		" %02hx",
		ad -> text[(whichline * 16) + x]);
	p += 3;
      }
      for (; x < 16; x++)
      {
	*p = ' ';
	p++;
	*p = ' ';
	p++;
	*p = ' ';
	p++;
      }
      *p = ' ';
      p++;
      *p = ' ';
      p++;
      for (x = 0; x < width; x++)
      {
	*p = ad -> text[(whichline * 16) + x];
	p++;
      }
      *p = 0;
      GpiCharString(hps, (p - s) - (abs(ad -> horzscroll) /
				    ad -> fattrs.lAveCharWidth),
		  s + (abs(ad -> horzscroll) / ad -> fattrs.lAveCharWidth));
      GpiQueryCurrentPosition(hps, &ptl);
      if (ptl.x + abs(ad -> horzscroll) + ad -> fattrs.lAveCharWidth + 1 > ad -> maxx)
      {
	ad -> maxx = ptl.x + abs(ad -> horzscroll) + ad -> fattrs.lAveCharWidth + 1;
	WinSendMsg(ad -> hhscroll, SBM_SETTHUMBSIZE,
		   MPFROM2SHORT((SHORT) Rectl -> xRight, (SHORT) ad -> maxx),
		   MPVOID);
      }
      rcl2.xLeft = ptl.x;
      WinFillRect(hps, &rcl2, standardcolors[ad -> colors[COLORS_NORMALBACK]]);
    }
  }
}

static VOID Search(VOID * args)
{
  HWND hwnd = (HWND) args;
  HAB hab2;
  HMQ hmq2;
  VIEWDATA *ad;
  register CHAR *p;
  RECTL Rectl;
  ULONG width, numlines, lastline, whichline, firstline = ULONG_MAX;
  register ULONG x;
  CHAR s[SEARCHSTRINGLEN], s2[SEARCHSTRINGLEN], *t, *n, markwith;

  priority_normal();
  hab2 = WinInitialize(0);
  if (hab2)
  {
    hmq2 = WinCreateMsgQueue(hab2, 0);
    if (hmq2)
    {
      WinCancelShutdown(hmq2, TRUE);
      ad = WinQueryWindowPtr(hwnd, QWL_USER);
      if (ad)
      {
	if (!DosRequestMutexSem(ad -> ScanSem, SEM_INDEFINITE_WAIT))
	{
	  markwith = VF_FOUND | ((ad -> alsoselect) ? VF_SELECTED : 0);
	  strcpy(s, ad -> searchtext);
	  if (*s)
	  {
	    WinQueryWindowRect(hwnd, &Rectl);
	    width = (Rectl.xRight - Rectl.xLeft) / ad -> fattrs.lAveCharWidth;
	    numlines = NumLines(&Rectl, ad);
	    WinSetWindowText(WinWindowFromID(ad -> hwndFrame,
					     NEWVIEW_STATUS1),
			     GetPString(IDS_SEARCHINGTEXT));
	    if (numlines && width && ad -> markedlines && ad -> numlines &&
		ad -> text && ad -> textsize)
	    {
	      for (x = 0; x < ad -> numlines && !ad -> stopflag; x++)
		ad -> markedlines[x] &= (~VF_FOUND);
	      ad -> found = 0;
	      t = s;
	      while (t && !ad -> stopflag)
	      {
		lastline = 1;
		n = convert_nl_to_nul(t);
		if (*t)
		{
		  strcpy(s2, t);
		  if (ad -> literalsearch)
		    literal(s2);
		  p = ad -> text;
		  while (p && !ad -> stopflag)
		  {
		    p = findstring(s2, strlen(s2), p,
				   ad -> textsize - (p - ad -> text),
				   ad -> sensitive);
		    if (p)
		    {
		      if (ad -> hex)
		      {
			whichline = (p - ad -> text) / 16;
			if (whichline < firstline)
			  firstline = whichline;
			if (!(ad -> markedlines[whichline] & VF_FOUND))
			  ad -> found++;
			if (markwith & VF_SELECTED)
			{
			  if (!(ad -> markedlines[whichline] & VF_SELECTED))
			    ad -> selected++;
			}
			ad -> markedlines[whichline] |= markwith;
			if ((p - ad -> text) + strlen(s2) > (whichline + 1) * 16)
			{
			  whichline++;
			  if (!(ad -> markedlines[whichline] & VF_FOUND))
			    ad -> found++;
			  if (markwith & VF_SELECTED)
			  {
			    if (!(ad -> markedlines[whichline] & VF_SELECTED))
			      ad -> selected++;
			  }
			  ad -> markedlines[whichline] |= markwith;
			}
			p = ad -> text + ((whichline + 1) * 16);
			if (p >= ad -> text + ad -> textsize)
			  break;
		      }
		      else
		      {
			for (x = lastline; x < ad -> numlines; x++)
			{
			  if (ad -> lines[x] > p)
			  {
			    if (x - 1 < firstline)
			      firstline = x - 1;
			    if (!(ad -> markedlines[x - 1] & VF_FOUND))
			      ad -> found++;
			    if (markwith & VF_SELECTED)
			    {
			      if (!(ad -> markedlines[x - 1] & VF_SELECTED))
				ad -> selected++;
			    }
			    ad -> markedlines[x - 1] |= markwith;
			    if (x + 1 < ad -> numlines &&
				p + strlen(s2) > ad -> lines[x])
			    {
			      x++;
			      if (!(ad -> markedlines[x - 1] & VF_FOUND))
				ad -> found++;
			      if (markwith & VF_SELECTED)
			      {
				if (!(ad -> markedlines[x - 1] & VF_SELECTED))
				  ad -> selected++;
			      }
			      ad -> markedlines[x - 1] |= markwith;
			    }
			    lastline = x;
			    p = ad -> lines[x];
			    break;
			  }
			}
			if (x >= ad -> numlines)
			{
			  if (markwith & VF_SELECTED)
			  {
			    if (!(ad -> markedlines[numlines - 1] & VF_SELECTED))
			      ad -> selected++;
			    if (!(ad -> markedlines[numlines - 1] & VF_FOUND))
			      ad -> found++;
			  }
			  ad -> markedlines[ad -> numlines - 1] |= markwith;
			  break;
			}
		      }
		    }
		  }
		}
		t = n;
	      }
	    }
	    DosReleaseMutexSem(ad -> ScanSem);
	    if (!ad -> stopflag && firstline == ULONG_MAX)
	    {
	      DosBeep(50, 50);
	      WinSetWindowText(WinWindowFromID(ad -> hwndFrame,
					       NEWVIEW_STATUS1),
			       GetPString(IDS_NOMATCHINGTEXT));
	      DosSleep(1500);
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	      PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	    }
	    else if (!ad -> stopflag)
	    {
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	      PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	      PostMsg(hwnd, UM_CONTAINER_FILLED,
		      MPFROMLONG(firstline + 1),
		      MPFROMLONG(firstline + 1));
	    }
	  }
	  else
	    DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
  DosPostEventSem(CompactSem);
}

static VOID Clipboard(VOID * args)
{
  HWND hwnd = (HWND) args;
  HAB hab2;
  HMQ hmq2;
  VIEWDATA *ad;
  CHAR **list;
  USHORT cmd;
  register ULONG x;
  BOOL released = FALSE;

  priority_normal();
  hab2 = WinInitialize(0);
  if (hab2)
  {
    hmq2 = WinCreateMsgQueue(hab2, 0);
    if (hmq2)
    {
      WinCancelShutdown(hmq2, TRUE);
      ad = WinQueryWindowPtr(hwnd, QWL_USER);
      if (ad)
      {
	if (!DosRequestMutexSem(ad -> ScanSem, SEM_INDEFINITE_WAIT))
	{
	  cmd = ad -> cliptype;
	  if (ad -> numlines && ad -> text && ad -> textsize && ad -> markedlines &&
	      !ad -> stopflag)
	  {
	    WinSetWindowText(WinWindowFromID(ad -> hwndFrame,
					     NEWVIEW_STATUS1),
			     GetPString(IDS_BUILDINGLINELISTTEXT));
	    if (cmd == IDM_SAVETOCLIP || cmd == IDM_APPENDTOCLIP ||
		cmd == IDM_SAVETOLIST)
	      list = BuildAList(hwnd);
	    else
	      list = BuildAList2(hwnd);
	    if (list)
	    {
	      if (!ad -> stopflag)
	      {
		WinSetWindowText(WinWindowFromID(ad -> hwndFrame,
						 NEWVIEW_STATUS1),
				 (cmd == IDM_SAVETOCLIP ||
				  cmd == IDM_SAVETOCLIP2) ?
				 GetPString(IDS_SAVETOCLIPTEXT) :
				 (cmd == IDM_APPENDTOCLIP ||
				  cmd == IDM_APPENDTOCLIP2) ?
				 GetPString(IDS_APPENDTOCLIPTEXT) :
				 GetPString(IDS_WRITETOFILETEXT));
		DosReleaseMutexSem(ad -> ScanSem);
		released = TRUE;
		if (cmd == IDM_SAVETOCLIP || cmd == IDM_APPENDTOCLIP ||
		    cmd == IDM_SAVETOCLIP2 || cmd == IDM_APPENDTOCLIP2)
		  ListToClipboardHab(hab2, list, (cmd == IDM_APPENDTOCLIP ||
						  cmd == IDM_APPENDTOCLIP2));
		else
		{

		  FILE *fp;
		  CHAR filename[CCHMAXPATH];

		  *filename = 0;
		  if (export_filename(hwnd, filename, FALSE))
		  {
		    fp = _fsopen(filename, "a+", SH_DENYWR);
		    if (!fp) {
		      saymsg(MB_CANCEL,
			     hwnd,
			     GetPString(IDS_ERRORTEXT),
			     GetPString(IDS_CANTOPENFORWRITETEXT),
			     filename);
		    }
		    else {
		      fseek(fp, 0L, SEEK_END);
		      for (x = 0; list[x]; x++)
			fprintf(fp,
				"%s\n",
				list[x]);
		      fclose(fp);
		    }
		  }
		}
	      }
	      FreeList(list);
	    }
	    else
	    {
	      DosReleaseMutexSem(ad -> ScanSem);
	      released = TRUE;
	      DosBeep(50, 100);
	      WinSetWindowText(WinWindowFromID(ad -> hwndFrame,
					       NEWVIEW_STATUS1),
			       GetPString(IDS_NVNOLINESSELTEXT));
	      DosSleep(1500);
	    }
	  }
	  if (!released)
	    DosReleaseMutexSem(ad -> ScanSem);
	  PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	}
      }
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
  DosPostEventSem(CompactSem);
}

static VOID ReLine(VOID * args)
{
  HWND hwnd = (HWND) args;
  HAB hab2;
  HMQ hmq2;
  VIEWDATA *ad;
  CHAR *p, *pp, *e, *whereiam = NULL;
  RECTL Rectl;
  ULONG width, numlines, firstline = 1, cursored = 1;

  priority_normal();
  hab2 = WinInitialize(0);
  if (hab2)
  {
    hmq2 = WinCreateMsgQueue(hab2, 0);
    if (hmq2)
    {
      WinCancelShutdown(hmq2, TRUE);
      ad = WinQueryWindowPtr(hwnd, QWL_USER);
      if (ad)
      {
	ad -> relining = TRUE;
	if (!DosRequestMutexSem(ad -> ScanSem, SEM_INDEFINITE_WAIT))
	{
	  ad -> busy++;
	  ad -> maxx = 0;
	  if (ad -> text && ad -> textsize)
	  {
	    if (ad -> hex)
	    {
	      firstline = ad -> topline;
	      cursored = ad -> cursored;
	    }
	    else if (ad -> lines)
	      whereiam = ad -> lines[ad -> cursored - 1];
	    ad -> found = 0;
	    ad -> selected = ad -> numlines = ad -> numalloc = 0;
	    if (ad -> lines)
	      free(ad -> lines);
	    if (ad -> markedlines)
	      free(ad -> markedlines);
	    ad -> lines = NULL;
	    ad -> markedlines = NULL;
	    WinSetWindowText(WinWindowFromID(ad -> hwndFrame,
					     NEWVIEW_STATUS1),
			     GetPString(IDS_FORMATTINGTEXT));
	    if (!ad -> hex)
	    {
	      if (WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX,
				    LM_QUERYITEMCOUNT, MPVOID, MPVOID))
	      {
		WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX, LM_DELETEALL,
				  MPVOID, MPVOID);
		PostMsg(ad -> hwndFrame, WM_UPDATEFRAME,
			MPFROMLONG(FCF_SIZEBORDER), MPVOID);
	      }
	    }
	    WinSetFocus(HWND_DESKTOP, hwnd);
	    if (!ad -> hex)
	    {
	      WinQueryWindowRect(hwnd, &Rectl);
	      width = (Rectl.xRight - Rectl.xLeft) / ad -> fattrs.lAveCharWidth;
	      numlines = NumLines(&Rectl, ad);
	      ad -> oldwidth = width;
	      p = ad -> text;
	      if (width)
	      {
		while (p - ad -> text < ad -> textsize && !ad -> stopflag)
		{
		  if (ad -> wrapon)
		  {
		    e = p + (width - 1);
		    if (e - ad -> text > ad -> textsize)
		      e = ad -> text + ad -> textsize;
		    pp = p;
		    while (pp < e)
		    {
		      if (*pp == '\r' || *pp == '\n')
		      {
			e = pp;
			break;
		      }
		      pp++;
		    }
		  }
		  else
		  {
		    pp = p;
		    while (pp - ad -> text < ad -> textsize &&
			   *pp != '\r' && *pp != '\n')
		      pp++;
		    e = pp;
		    if (ad -> maxx <
			(((e - p) + 1) * ad -> fattrs.lAveCharWidth) + 1)
		      ad -> maxx = (((e - p) + 1) *
				    ad -> fattrs.lAveCharWidth) + 1;
		  }
		  if (whereiam && p >= whereiam && e <= whereiam)
		  {
		    cursored = firstline = ad -> numlines + 1;
		    whereiam = NULL;
		  }
		  /* assign ad->lines[ad->numlines] */
		  if (ad -> numlines + 1 > ad -> numalloc)
		  {

		    CHAR **temp;

		    temp = xrealloc(ad -> lines, sizeof(CHAR *) *
				   (ad -> numalloc + 256),pszSrcFile,__LINE__);
		    if (!temp)
		      break;
		    ad -> lines = temp;
		    ad -> numalloc += 256;
		  }
		  ad -> lines[ad -> numlines] = p;
		  ad -> numlines++;
		  if (ad -> numlines == numlines)
		  {
		    /* display first page */
		    register INT x;

		    for (x = 0; x < ad -> numlines; x++)
		    {
		      if ((LONG) (Rectl.yTop -
				  (ad -> lMaxHeight * (((x + 1) -
						  ad -> topline) + 1))) < 0)
			break;
		      PaintLine(hwnd, ad -> hps, x, 1, &Rectl);
		    }
		  }
		  p = e;
		  if (p - ad -> text < ad -> textsize)
		  {
		    if (*p == '\r')
		      p++;
		  }
		  if (p - ad -> text < ad -> textsize)
		  {
		    if (*p == '\n')
		      p++;
		  }
		}
	      }
	      if (ad -> numalloc != ad -> numlines)
	      {

		CHAR **temp;

		temp = xrealloc(ad -> lines, sizeof(CHAR *) * ad -> numlines,pszSrcFile,__LINE__);
		if (temp)
		{
		  ad -> lines = temp;
		  ad -> numalloc = ad -> numlines;
		}
	      }
	    }
	    else
	    {
	      ad -> numlines = ad -> textsize / 16;
	      if (ad -> numlines * 16 < ad -> textsize)
		ad -> numlines++;
	    }
	    if (ad -> numlines)
	    {
	      ad -> markedlines = xmalloc(ad -> numlines,pszSrcFile,__LINE__);
	      if (ad -> markedlines)
	      {
		memset(ad -> markedlines, 0, ad -> numlines);
		ad -> selected = 0;
	      }
	      if (*ftprun && !ad -> ignoreftp && strstr(ad -> text, "ftp://"))
		ad -> ftpin = TRUE;
	      if (*httprun && !ad -> ignorehttp && strstr(ad -> text, "http://"))
		ad -> httpin = TRUE;
	    }
	  }
	  DosReleaseMutexSem(ad -> ScanSem);
	  PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	  PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	  ad -> busy--;
	}
      }
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
  DosPostEventSem(CompactSem);
  if (ad && !ad -> stopflag)
  {
    PostMsg(hwnd, UM_CONTAINER_FILLED, MPFROMLONG(firstline),
	    MPFROMLONG(cursored));
    ad -> relining = FALSE;
  }
}

static VOID LoadFile(VOID * args)
{
  HWND hwnd = (HWND) args;
  HAB hab2;
  HMQ hmq2;
  VIEWDATA *ad;
  HFILE handle;
  ULONG action, len;
  APIRET rc;
  BOOL error = TRUE;

  hab2 = WinInitialize(0);
  if (hab2)
  {
    hmq2 = WinCreateMsgQueue(hab2, 0);
    if (hmq2)
    {
      WinCancelShutdown(hmq2, TRUE);
      ad = WinQueryWindowPtr(hwnd, QWL_USER);
      if (ad)
      {
	if (!DosRequestMutexSem(ad -> ScanSem, SEM_INDEFINITE_WAIT))
	{
	  ad -> busy++;
	  priority_normal();
	  if (*ad -> filename)
	  {
	    if (ad -> text)
	      free(ad -> text);
	    if (ad -> lines)
	      free(ad -> lines);
	    if (ad -> markedlines)
	      free(ad -> markedlines);
	    ad -> text = NULL;
	    ad -> lines = NULL;
	    ad -> markedlines = NULL;
	    ad -> ftpin = ad -> httpin = FALSE;
	    ad -> selected = ad -> numlines = ad -> textsize = ad -> numalloc = 0;
	    WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX, LM_DELETEALL,
			      MPVOID, MPVOID);
	    PostMsg(ad -> hwndFrame, WM_UPDATEFRAME,
		    MPFROMLONG(FCF_SIZEBORDER), MPVOID);
	    WinSetFocus(HWND_DESKTOP, hwnd);
	    rc = DosOpen(ad -> filename, &handle, &action, 0L, 0L,
		       OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
			 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NOINHERIT |
			 OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYNONE |
			 OPEN_ACCESS_READONLY, 0L);
	    if (rc) {
	      Dos_Error(MB_CANCEL,
			rc,
			hwnd,
			pszSrcFile,
			__LINE__,
			GetPString(IDS_COMPCANTOPENTEXT),
			ad -> filename);
	    }
	    else {
	      DosChgFilePtr(handle, 0L, FILE_END, &len);
	      DosChgFilePtr(handle, 0L, FILE_BEGIN, &action);
	      if (!len) {
		saymsg(MB_CANCEL,
		       hwnd,
		       GetPString(IDS_ERRORTEXT),
		       GetPString(IDS_ZEROLENGTHTEXT),
		       ad -> filename);
	      }
	      else {
		ad -> text = xmalloc(len + 2,pszSrcFile,__LINE__);
		if (ad -> text)
		{
		  *ad -> text = 0;
		  ad -> text[len] = 0;
		  rc = DosRead(handle, ad -> text, len, &ad -> textsize);
		  if (rc) {
		    Dos_Error(MB_CANCEL,
			      rc,
			      hwnd,
			      pszSrcFile,
			      __LINE__,
			      GetPString(IDS_ERRORREADINGTEXT),
			      ad -> filename);
		    free(ad -> text);
		    ad -> text = NULL;
		    ad -> textsize = 0;
		  }
		  else {
		    ad -> text[ad -> textsize] = 0;
		    if (!ad -> hex && !(ad -> flags & (8 | 16)) && ad -> textsize)
		    {
		      ULONG x;
		      x = min(512, ad -> textsize);
		      if (fGuessType && IsBinary(ad -> text, x))
			ad -> hex = TRUE;
		    }
		    if (ad -> textsize) {
		      if (_beginthread(ReLine, NULL, 524288, (PVOID) hwnd) == -1)
                        Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
		      else
		        error = FALSE;
		    }
		  }
		}
	      }
	      DosClose(handle);
	    }
	  }
	  ad -> busy--;
	  DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      WinDestroyMsgQueue(hmq2);
    }
    WinTerminate(hab2);
  }
  if (error)
    PostMsg(hwnd, UM_CONTAINER_FILLED, MPVOID, MPVOID);
  DosPostEventSem(CompactSem);
}

MRESULT EXPENTRY ViewFrameWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PFNWP oldproc = (PFNWP) WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg)
  {
  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1))
    {
    case NEWVIEW_LISTBOX:
      return WinSendMsg(WinWindowFromID(hwnd, FID_CLIENT), UM_CONTROL,
			mp1, mp2);
    }
    break;

  case WM_CALCFRAMERECT:
    {
      MRESULT mr;
      PRECTL prectl;
      SHORT sSelect;

      mr = oldproc(hwnd, msg, mp1, mp2);

      /*
       * Calculate the position of the client rectangle.
       * Otherwise,  we'll see a lot of redraw when we move the
       * client during WM_FORMATFRAME.
       */

      if (mr && mp2)
      {
	prectl = (PRECTL) mp1;
	prectl -> yBottom += 22;
	prectl -> yTop -= 22;
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, NEWVIEW_LISTBOX,
					    LM_QUERYITEMCOUNT,
					    MPVOID, MPVOID);
	if (sSelect > 0)
	  prectl -> yTop -= 48;
      }
      return mr;
    }

  case WM_FORMATFRAME:
    {
      SHORT sCount, soldCount, sSelect;
      PSWP pswp, pswpClient, pswpNew1, pswpNew2, pswpNew3, pswpList, pswpScroll,
          pswpNew4, pswpUp, pswpDn;

      sCount = (SHORT) oldproc(hwnd, msg, mp1, mp2);
      soldCount = sCount;

      /*
       * Reformat the frame to "squeeze" the client
       * and make room for status window sibling beneath
       */

      pswp = (PSWP) mp1;
      {
	SHORT x;

	for (x = 0; x < sCount; x++)
	{
	  if (WinQueryWindowUShort(pswp -> hwnd, QWS_ID) == FID_CLIENT)
	  {
	    pswpClient = pswp;
	    break;
	  }
	  pswp++;
	}
      }
      pswpNew1 = (PSWP) mp1 + soldCount;
      pswpNew2 = (PSWP) mp1 + (soldCount + 1);
      pswpNew3 = (PSWP) mp1 + (soldCount + 2);
      pswpNew4 = (PSWP) mp1 + (soldCount + 3);
      *pswpNew1 = *pswpClient;
      pswpNew1 -> hwnd = WinWindowFromID(hwnd, NEWVIEW_STATUS1);
      pswpNew1 -> x = pswpClient -> x + 2;
      pswpNew1 -> y = pswpClient -> y + 2;
      pswpNew1 -> cx = (pswpClient -> cx / 3) - 3;
      pswpNew1 -> cy = 20;
      pswpClient -> y = pswpNew1 -> y + pswpNew1 -> cy + 3;
      pswpClient -> cy = (pswpClient -> cy - pswpNew1 -> cy) - 5;
      *pswpNew2 = *pswpNew1;
      *pswpNew3 = *pswpNew1;
      *pswpNew4 = *pswpNew1;
      pswpNew2 -> hwnd = WinWindowFromID(hwnd, NEWVIEW_STATUS2);
      pswpNew3 -> hwnd = WinWindowFromID(hwnd, NEWVIEW_STATUS3);
      pswpNew4 -> hwnd = WinWindowFromID(hwnd, NEWVIEW_DRAG);
      pswpNew2 -> x = pswpNew1 -> x + pswpNew1 -> cx + 3;
      pswpNew3 -> x = pswpNew2 -> x + pswpNew2 -> cx + 3;
      pswpNew3 -> cx = ((pswpClient -> x + pswpClient -> cx) - pswpNew3 -> x) - 26;
      pswpNew4 -> x = pswpNew3 -> x + pswpNew3 -> cx + 3;
      pswpNew4 -> cx = 20;
      sCount += 4;
      pswpScroll = (PSWP) mp1;
      while (pswpScroll < pswpClient)
      {
	if (WinQueryWindowUShort(pswpScroll -> hwnd, QWS_ID) == FID_VERTSCROLL)
	  break;
	pswpScroll++;
      }
      if (pswpScroll == pswpClient)
	pswpScroll = NULL;
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd, NEWVIEW_LISTBOX,
					  LM_QUERYITEMCOUNT,
					  MPVOID, MPVOID);
      if (sSelect > 0)
      {
	pswpList = (PSWP) mp1 + (soldCount + 4);
	*pswpList = *pswpClient;
	pswpList -> hwnd = WinWindowFromID(hwnd, NEWVIEW_LISTBOX);
	pswpList -> x = pswpClient -> x;
	pswpList -> cx = pswpClient -> cx;
	if (pswpScroll)
	{
	  pswpList -> cx += pswpScroll -> cx;
	  pswpScroll -> cy -= 48;
	}
	pswpList -> y = (pswpClient -> y + pswpClient -> cy) - 48;
	pswpList -> cy = 48;
	pswpClient -> cy -= 48;
	sCount++;
      }
      WinShowWindow(WinWindowFromID(hwnd, NEWVIEW_LISTBOX), (sSelect > 0));

      if (pswpScroll)
      {
	pswpUp = (PSWP) mp1 + (soldCount + 4 + (sSelect > 0));
	*pswpUp = *pswpClient;
	pswpUp -> hwnd = WinWindowFromID(hwnd, IDM_PREVBLANKLINE);
	pswpUp -> cx = pswpScroll -> cx;
	pswpUp -> x = pswpScroll -> x;
	pswpUp -> cy = WinQuerySysValue(HWND_DESKTOP, SV_CYVSCROLLARROW);
	pswpUp -> y = (pswpScroll -> y + pswpScroll -> cy) - (pswpUp -> cy + 1);
	pswpScroll -> cy -= ((pswpUp -> cy * 2) + 1);
	pswpDn = (PSWP) mp1 + (soldCount + 5 + (sSelect > 0));
	*pswpDn = *pswpUp;
	pswpDn -> y = pswpScroll -> y;
	pswpDn -> hwnd = WinWindowFromID(hwnd, IDM_NEXTBLANKLINE);
	pswpScroll -> y += pswpUp -> cy;
	sCount += 2;
      }
      else
      {
	WinShowWindow(WinWindowFromID(hwnd, IDM_PREVBLANKLINE), FALSE);
	WinShowWindow(WinWindowFromID(hwnd, IDM_NEXTBLANKLINE), FALSE);
      }
      return MRFROMSHORT(sCount);
    }

  case WM_QUERYFRAMECTLCOUNT:
    {
      SHORT sCount, sSelect;

      sCount = (SHORT) oldproc(hwnd, msg, mp1, mp2);
      sCount += 6;
      sSelect = (SHORT) WinSendDlgItemMsg(hwnd, NEWVIEW_LISTBOX,
					  LM_QUERYITEMCOUNT,
					  MPVOID, MPVOID);
      if (sSelect > 0)
	sCount++;
      return MRFROMSHORT(sCount);
    }
  }
  return oldproc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY FindStrDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  VIEWDATA *ad;

  switch (msg)
  {
  case WM_INITDLG:
    if (!mp2)
      WinDismissDlg(hwnd, 0);
    else
    {

      HWND hwndClient = *(HWND *) mp2;

      WinSetWindowULong(hwnd, QWL_USER, (ULONG) hwndClient);
      ad = (VIEWDATA *) WinQueryWindowPtr(hwndClient, QWL_USER);
      MLEsetwrap(WinWindowFromID(hwnd, NEWFIND_MLE), FALSE);
      MLEsetlimit(WinWindowFromID(hwnd, NEWFIND_MLE), SEARCHSTRINGLEN);
      MLEsetformat(WinWindowFromID(hwnd, NEWFIND_MLE), MLFIE_NOTRANS);
      if (*ad -> searchtext)
      {

	IPT here = 0;
	ULONG len = strlen(ad -> searchtext);

	WinSendMsg(WinWindowFromID(hwnd, NEWFIND_MLE),
		   MLM_SETIMPORTEXPORT,
		   MPFROMP(ad -> searchtext),
		   MPFROMLONG(SEARCHSTRINGLEN));
	WinSendMsg(WinWindowFromID(hwnd, NEWFIND_MLE),
		   MLM_IMPORT,
		   MPFROMP(&here),
		   MPFROMLONG(len));
      }
      WinCheckButton(hwnd, NEWFIND_ALSOSELECT, ad -> alsoselect);
      WinCheckButton(hwnd, NEWFIND_SENSITIVE, ad -> sensitive);
      WinCheckButton(hwnd, NEWFIND_LITERAL, ad -> literalsearch);
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1))
    {
    case DID_OK:
      {
	CHAR s[SEARCHSTRINGLEN];
	IPT here = 0;
	ULONG len;
	HWND hwndClient = WinQueryWindowULong(hwnd, QWL_USER);

	ad = (VIEWDATA *) WinQueryWindowPtr(hwndClient, QWL_USER);
	memset(s, 0, SEARCHSTRINGLEN);
	WinSendMsg(WinWindowFromID(hwnd, NEWFIND_MLE),
		   MLM_SETIMPORTEXPORT,
		   MPFROMP(s),
		   MPFROMLONG(SEARCHSTRINGLEN));
	len = SEARCHSTRINGLEN;
	WinSendMsg(WinWindowFromID(hwnd, NEWFIND_MLE),
		   MLM_EXPORT,
		   MPFROMP(&here),
		   MPFROMLONG(&len));
	s[SEARCHSTRINGLEN - 1] = 0;
	if (!*s)
	{
	  DosBeep(250, 100);		// Complain
	  break;
	}
	strcpy(ad -> searchtext, s);
	ad -> sensitive = WinQueryButtonCheckstate(hwnd, NEWFIND_SENSITIVE);
	if (ad -> sensitive != Sensitive)
	{
	  Sensitive = ad -> sensitive;
	  PrfWriteProfileData(fmprof,
			      appname,
			      "Viewer.Sensitive",
			      &ad -> sensitive,
			      sizeof(BOOL));
	}
	ad -> literalsearch = WinQueryButtonCheckstate(hwnd, NEWFIND_LITERAL);
	if (ad -> literalsearch != LiteralSearch)
	{
	  LiteralSearch = ad -> literalsearch;
	  PrfWriteProfileData(fmprof,
			      appname,
			      "Viewer.LiteralSearch",
			      &ad -> literalsearch,
			      sizeof(BOOL));
	}
	ad -> alsoselect = WinQueryButtonCheckstate(hwnd, NEWFIND_ALSOSELECT);
	if (ad -> alsoselect != AlsoSelect)
	{
	  AlsoSelect = ad -> alsoselect;
	  PrfWriteProfileData(fmprof,
			      appname,
			      "Viewer.AlsoSelect",
			      &ad -> alsoselect,
			      sizeof(BOOL));
	}
      }
      WinDismissDlg(hwnd, 1);
      break;
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY ViewWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  VIEWDATA *ad = WinQueryWindowPtr(hwnd, QWL_USER);

  switch (msg)
  {
  case WM_CREATE:
    {
      HWND temphwnd;
      HWND hwndFrame = WinQueryWindow(hwnd, QW_PARENT);

      temphwnd = WinCreateWindow(hwndFrame,
				 WC_BUTTON,
				 "<",
				 WS_VISIBLE |
				 BS_PUSHBUTTON | BS_NOPOINTERFOCUS,
				 0,
				 0,
				 0,
				 0,
				 hwndFrame,
				 HWND_TOP,
				 IDM_PREVBLANKLINE,
				 NULL,
				 NULL);
      if (!temphwnd)
	Win_Error2(hwndFrame,hwnd,pszSrcFile,__LINE__,IDS_WINCREATEWINDOW);
      else {
        WinSetPresParam(temphwnd,
		        PP_FONTNAMESIZE,
		        strlen(GetPString(IDS_8HELVTEXT)) + 1,
		        (PVOID) GetPString(IDS_8HELVTEXT));
      }
      temphwnd = WinCreateWindow(hwndFrame,
				 WC_BUTTON,
				 ">",
				 WS_VISIBLE |
				 BS_PUSHBUTTON | BS_NOPOINTERFOCUS,
				 0,
				 0,
				 0,
				 0,
				 hwndFrame,
				 HWND_TOP,
				 IDM_NEXTBLANKLINE,
				 NULL,
				 NULL);
      if (!temphwnd)
	Win_Error2(hwndFrame,hwnd,pszSrcFile,__LINE__,IDS_WINCREATEWINDOW);
      else {
        WinSetPresParam(temphwnd,
		        PP_FONTNAMESIZE,
		        strlen(GetPString(IDS_8HELVTEXT)) + 1,
		        (PVOID)GetPString(IDS_8HELVTEXT));
      }
      WinStartTimer(WinQueryAnchorBlock(hwnd),
		    hwnd,
		    ID_TIMER5,
		    1000L);
    }
    break;

  case WM_TIMER:
    if (ad &&
	ad -> needrefreshing &&
	!ad -> stopflag &&
	!ad -> relining &&
	!DosRequestMutexSem(ad -> ScanSem,
			    SEM_IMMEDIATE_RETURN))
    {
      ad -> needrefreshing = FALSE;
      DosReleaseMutexSem(ad -> ScanSem);
      WinInvalidateRect(hwnd, NULL, TRUE);
    }
    break;

  case UM_SETUP:
    if (!ad)
      Runtime_Error(pszSrcFile, __LINE__, "no data");
    else {
      CHAR s[CCHMAXPATH + 8];
      APIRET rc;
      ad -> hwndMenu = WinWindowFromID(ad -> hwndFrame, FID_MENU);
      ad -> hvscroll = WinWindowFromID(ad -> hwndFrame, FID_VERTSCROLL);
      ad -> hhscroll = WinWindowFromID(ad -> hwndFrame, FID_HORZSCROLL);
      WinSendMsg(ad -> hhscroll,SBM_SETTHUMBSIZE,MPFROM2SHORT(1, 1),MPVOID);
      WinSendMsg(ad -> hvscroll,SBM_SETTHUMBSIZE,MPFROM2SHORT(1, 1),MPVOID);
      sprintf(s,"%s: %s",FM2Str,ad -> filename);
      WinSetWindowText(ad -> hwndFrame,s);
      rc = DosCreateMutexSem(NULL, &ad -> ScanSem, 0L, FALSE);
      if (rc)
        Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,"DosCreateMutexSem");
      else {
	PFNWP oldproc;
	HWND hwndFrame = ad -> hwndFrame;
	WinSendMsg(ad -> hvscroll,
		   SBM_SETSCROLLBAR,
		   MPFROMSHORT(1),
		   MPFROM2SHORT(1, 1));
	WinSendMsg(ad -> hhscroll, SBM_SETSCROLLBAR, MPFROMSHORT(1),
		   MPFROM2SHORT(1, 1));
	ad -> hwndStatus1 = WinCreateWindow(hwndFrame,
					    GetPString(IDS_WCVIEWSTATUS),
					    GetPString(IDS_LOADINGTEXT),
					    WS_VISIBLE | SS_TEXT |
					    DT_LEFT | DT_VCENTER,
					    0,
					    0,
					    0,
					    0,
					    hwndFrame,
					    HWND_TOP,
					    NEWVIEW_STATUS1,
					    NULL,
					    NULL);
	if (!ad -> hwndStatus1)
          Win_Error2(hwndFrame,hwnd,pszSrcFile,__LINE__,IDS_WINCREATEWINDOW);

	ad -> hwndStatus2 = WinCreateWindow(hwndFrame,
					    GetPString(IDS_WCVIEWSTATUS),
					    NULL,
					    WS_VISIBLE | SS_TEXT |
					    DT_LEFT | DT_VCENTER,
					    0,
					    0,
					    0,
					    0,
					    hwndFrame,
					    HWND_TOP,
					    NEWVIEW_STATUS2,
					    NULL,
					    NULL);
	if (!ad -> hwndStatus2)
          Win_Error2(hwndFrame,hwnd,pszSrcFile,__LINE__,IDS_WINCREATEWINDOW);

	ad -> hwndStatus3 = WinCreateWindow(hwndFrame,
					    GetPString(IDS_WCVIEWSTATUS),
					    NULL,
					    WS_VISIBLE | SS_TEXT |
					    DT_LEFT | DT_VCENTER,
					    0,
					    0,
					    0,
					    0,
					    hwndFrame,
					    HWND_TOP,
					    NEWVIEW_STATUS3,
					    NULL,
					    NULL);
	if (!ad -> hwndStatus3)
          Win_Error2(hwndFrame,hwnd,pszSrcFile,__LINE__,IDS_WINCREATEWINDOW);

	ad -> hwndListbox = WinCreateWindow(hwndFrame,
					    WC_LISTBOX,
					    NULL,
					    LS_NOADJUSTPOS,
					    0,
					    0,
					    0,
					    0,
					    hwndFrame,
					    HWND_TOP,
					    NEWVIEW_LISTBOX,
					    NULL,
					    NULL);
	if (!ad -> hwndListbox)
          Win_Error2(hwndFrame,hwnd,pszSrcFile,__LINE__,IDS_WINCREATEWINDOW);

	ad -> hwndDrag = WinCreateWindow(hwndFrame,
					 GetPString(IDS_WCVIEWSTATUS),
					 "#100",
					 WS_VISIBLE | SS_BITMAP,
					 0,
					 0,
					 0,
					 0,
					 hwndFrame,
					 HWND_TOP,
					 NEWVIEW_DRAG,
					 NULL,
					 NULL);
	if (!ad -> hwndDrag)
          Win_Error2(hwndFrame,hwnd,pszSrcFile,__LINE__,IDS_WINCREATEWINDOW);

	oldproc = WinSubclassWindow(hwndFrame, ViewFrameWndProc);
	WinSetWindowPtr(hwndFrame, QWL_USER, (PVOID)oldproc);
	ad -> hps = InitWindow(hwnd);
	if (_beginthread(LoadFile, NULL, 524288, (PVOID) hwnd) == -1)
          Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
	else {
	  WinSendMsg(hwnd, UM_SETUP5, MPVOID, MPVOID);
	  DosSleep(32L);
	  return (MRESULT)1;
	}
      }
    }
    // Oops
    WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
    return 0;

  case UM_SETUP5:
    if (ad)
    {
      if (ad -> hwndFrame ==
	  WinQueryActiveWindow(WinQueryWindow(ad -> hwndFrame,
					      QW_PARENT)) &&
	  !ParentIsDesktop(ad -> hwndFrame, (HWND) 0))
      {
	if (hwndStatus2)
	  WinSetWindowText(hwndStatus2,
			   (*ad -> filename) ?
			   ad -> filename :
			   GetPString(IDS_UNTITLEDTEXT));
	if (fMoreButtons)
	{
	  WinSetWindowText(hwndName,
			   (*ad -> filename) ?
			   ad -> filename :
			   GetPString(IDS_UNTITLEDTEXT));
	  WinSetWindowText(hwndDate, NullStr);
	  WinSetWindowText(hwndAttr, NullStr);
	}
	if (hwndStatus)
	  WinSetWindowText(hwndStatus,
			   GetPString(IDS_INTERNALVIEWERTITLETEXT));
      }
    }
    return 0;

  case DM_DISCARDOBJECT:
  case DM_PRINTOBJECT:
    return MRFROMLONG(DRR_TARGET);

  case UM_RESCAN:
    if (ad)
    {
      if (!ad -> busy &&
	  !DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
      {
	if (ad -> numlines)
	{

	  CHAR s[80], tb[34], tl[34];

	  commafmt(tb, sizeof(tb), ad -> textsize);
	  commafmt(tl, sizeof(tl), ad -> numlines);
	  sprintf(s,
		  " %s %s%s  %s %s%s",
		  tb,
		  GetPString(IDS_BYTETEXT),
		  &"s"[ad -> textsize == 1],
		  tl,
		  GetPString(IDS_LINETEXT),
		  &"s"[ad -> numlines == 1]);
	  WinSetWindowText(ad -> hwndStatus1, s);
	}
	else
	  WinSetWindowText(ad -> hwndStatus1,
			   GetPString(IDS_NVNOLINESTEXT));
	DosReleaseMutexSem(ad -> ScanSem);
      }
      else
	WinSetWindowText(ad -> hwndStatus1,
			 GetPString(IDS_WORKINGTEXT));
    }
    return 0;

  case UM_SETUP2:
    /*
     * calculate width of client in characters, recalc lines if
     * oldwidth != newwidth, set ad->oldwidth for later comparison
     */
    if (ad)
    {

      BOOL invalidate = FALSE;

      if (!ad -> wrapon && !ad -> hex)
      {
	if (WinQueryWindow(ad -> hhscroll, QW_PARENT) == ad -> hwndFrame)
	{
	  invalidate = TRUE;
	  WinSetOwner(ad -> hhscroll, HWND_OBJECT);
	  WinSetParent(ad -> hhscroll, HWND_OBJECT, TRUE);
	  ad -> maxx = 0;
	  ad -> horzscroll = 0;
	}
      }
      else
      {
	if (WinQueryWindow(ad -> hhscroll, QW_PARENT) != ad -> hwndFrame)
	{
	  invalidate = TRUE;
	  WinSetOwner(ad -> hhscroll, ad -> hwndFrame);
	  WinSetParent(ad -> hhscroll, ad -> hwndFrame, TRUE);
	}
      }
      if (invalidate)
      {
	WinSendMsg(ad -> hwndFrame, WM_UPDATEFRAME, MPFROMLONG(FCF_SIZEBORDER),
		   MPVOID);
	WinInvalidateRect(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					  NEWVIEW_DRAG), NULL, FALSE);
	WinInvalidateRect(ad -> hhscroll, NULL, FALSE);
      }
    }

    if (ad && !ad -> busy &&
	!DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
    {

      RECTL rcl;
      ULONG newwidth;

      WinQueryWindowRect(hwnd, &rcl);
      newwidth = (rcl.xRight - rcl.xLeft) / ad -> fattrs.lAveCharWidth;
      if ((!ad -> hex || ad -> oldwidth == -1) &&
	  newwidth != ad -> oldwidth && ad -> text && ad -> textsize)
      {
	ad -> oldwidth = newwidth;
	if (!ad -> relining)
	{
	  if (_beginthread(ReLine, NULL, 524288, (PVOID) hwnd) == -1)
	  {
            Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
	    DosReleaseMutexSem(ad -> ScanSem);
	    WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
	    return 0;
	  }
	}
      }
      ad -> oldwidth = newwidth;
      DosReleaseMutexSem(ad -> ScanSem);
    }
    return MRFROMLONG(TRUE);

  case WM_CHAR:
    shiftstate = (SHORT1FROMMP(mp1) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (ad && !ad -> busy && !(SHORT1FROMMP(mp1) & KC_KEYUP) &&
	!DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
    {

      ULONG numlines, wascursored = ad -> cursored;
      RECTL rcl;

      WinQueryWindowRect(hwnd, &rcl);
      numlines = NumLines(&rcl, ad);
      if (numlines)
      {
	if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY)
	{
	  switch (SHORT2FROMMP(mp2))
	  {
	  case VK_LEFT:
	    WinSendMsg(hwnd, WM_HSCROLL, MPFROM2SHORT(FID_HORZSCROLL, 0),
		       MPFROM2SHORT(0, SB_LINELEFT));
	    break;
	  case VK_RIGHT:
	    WinSendMsg(hwnd, WM_HSCROLL, MPFROM2SHORT(FID_HORZSCROLL, 0),
		       MPFROM2SHORT(0, SB_LINERIGHT));
	    break;
	  case VK_PAGEUP:
	    PostMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
		    MPFROM2SHORT(0, SB_PAGEUP));
	    break;
	  case VK_PAGEDOWN:
	    PostMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
		    MPFROM2SHORT(0, SB_PAGEDOWN));
	    break;
	  case VK_UP:
	    if (ad -> cursored > 1)
	    {
	      if (shiftstate & KC_SHIFT)
		WinSendMsg(hwnd, WM_BUTTON1CLICK,
			   MPFROM2SHORT(ad -> fattrs.lAveCharWidth + 2,
					((rcl.yTop - (ad -> lMaxHeight *
				      ((ad -> cursored) - ad -> topline))) -
					 ad -> lMaxDescender) - 1),
			   MPFROM2SHORT(TRUE, 0));
	      ad -> cursored--;
	      if (ad -> cursored < ad -> topline)
	      {
		PaintLine(hwnd, ad -> hps, ad -> cursored, ad -> topline, &rcl);
		WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
			   MPFROM2SHORT(0, SB_LINEUP));
	      }
	      else
	      {
		PaintLine(hwnd, ad -> hps, ad -> cursored - 1, ad -> topline, &rcl);
		PaintLine(hwnd, ad -> hps, ad -> cursored, ad -> topline, &rcl);
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	      }
	    }
	    break;
	  case VK_DOWN:
	    if (ad -> cursored < ad -> numlines &&
		ad -> cursored < ad -> topline + numlines)
	    {
	      if (shiftstate & KC_SHIFT)
		WinSendMsg(hwnd, WM_BUTTON1CLICK,
			   MPFROM2SHORT(ad -> fattrs.lAveCharWidth + 2,
					((rcl.yTop - (ad -> lMaxHeight *
				      ((ad -> cursored) - ad -> topline))) -
					 ad -> lMaxDescender) - 1),
			   MPFROM2SHORT(TRUE, 0));
	      ad -> cursored++;
	      if (ad -> cursored >= ad -> topline + numlines)
	      {
		PaintLine(hwnd, ad -> hps, ad -> cursored - 2, ad -> topline, &rcl);
		WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
			   MPFROM2SHORT(0, SB_LINEDOWN));
	      }
	      else
	      {
		PaintLine(hwnd, ad -> hps, ad -> cursored - 1, ad -> topline, &rcl);
		PaintLine(hwnd, ad -> hps, ad -> cursored - 2, ad -> topline, &rcl);
		PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	      }
	    }
	    break;
	  case VK_END:
	    if ((shiftstate & KC_CTRL) ||
		ad -> cursored == (ad -> topline - 1) + numlines)
	    {
	      ad -> cursored = ad -> numlines;
	      ad -> topline = (ad -> numlines + 1) - numlines;
	      if (ad -> topline > ad -> numlines)
		ad -> topline = 1;
	      WinInvalidateRect(hwnd, NULL, FALSE);
	    }
	    else
	    {
	      ad -> cursored = (ad -> topline - 1) + numlines;
	      if (ad -> cursored > ad -> numlines)
		ad -> cursored = ad -> numlines;
	      PaintLine(hwnd, ad -> hps, ad -> cursored - 1, ad -> topline, &rcl);
	      PaintLine(hwnd, ad -> hps, wascursored - 1, ad -> topline, &rcl);
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	    PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	    break;
	  case VK_HOME:
	    if ((shiftstate & KC_CTRL) ||
		ad -> cursored == ad -> topline)
	    {
	      ad -> topline = 1;
	      ad -> cursored = 1;
	      WinInvalidateRect(hwnd, NULL, FALSE);
	    }
	    else
	    {
	      ad -> cursored = ad -> topline;
	      PaintLine(hwnd, ad -> hps, ad -> cursored - 1, ad -> topline, &rcl);
	      PaintLine(hwnd, ad -> hps, wascursored - 1, ad -> topline, &rcl);
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	    PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	    break;
	  case VK_SPACE:
	    WinSendMsg(hwnd, WM_BUTTON1CLICK,
		       MPFROM2SHORT(ad -> fattrs.lAveCharWidth + 2,
				    ((rcl.yTop - (ad -> lMaxHeight *
				      ((ad -> cursored) - ad -> topline))) -
				     ad -> lMaxDescender) - 1),
		       MPFROM2SHORT(TRUE, 0));
	    break;
	  case VK_NEWLINE:
	  case VK_ENTER:
	    WinSendMsg(hwnd, WM_BUTTON1DBLCLK,
		       MPFROM2SHORT(ad -> fattrs.lAveCharWidth + 2,
				    ((rcl.yTop - (ad -> lMaxHeight *
				      ((ad -> cursored) - ad -> topline))) -
				     ad -> lMaxDescender) - 1),
		       MPFROM2SHORT(0, 0));
	    break;
	  }
	}
	else if (SHORT1FROMMP(mp1) & KC_CHAR)
	{
	  switch (SHORT1FROMMP(mp2))
	  {
	  case '\r':
	  case '\n':
	    WinSendMsg(hwnd, WM_BUTTON1DBLCLK,
		       MPFROM2SHORT(ad -> fattrs.lAveCharWidth + 2,
				    (rcl.yTop - (ad -> lMaxHeight *
				  ((ad -> cursored) - ad -> topline))) - 1),
		       MPFROM2SHORT(0, 0));
	    break;
	  default:
	    break;
	  }
	}
	if (wascursored != ad -> cursored)
	  PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
      }
      DosReleaseMutexSem(ad -> ScanSem);
    }
    break;

  case WM_BUTTON1MOTIONSTART:
    WinSetFocus(HWND_DESKTOP, hwnd);
    if (ad && !ad -> stopflag && !ad -> busy &&
	!DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
    {
      ad -> mousecaptured = TRUE;
      ad -> lastselected = ULONG_MAX;
      ad -> lastdirection = 0;
      WinSetCapture(HWND_DESKTOP, hwnd);
      WinSendMsg(hwnd, WM_BUTTON1CLICK, mp1, MPFROM2SHORT(TRUE, 0));
    }
    break;

  case WM_MOUSEMOVE:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_SHIFT | KC_ALT | KC_CTRL));
    if (ad && ad -> mousecaptured)
    {

      ULONG numlines, whichline, x;
      LONG inc;
      RECTL Rectl;
      POINTS pts;
      BOOL outofwindow = FALSE;

      WinQueryWindowRect(hwnd, &Rectl);
      numlines = NumLines(&Rectl, ad);
      if (numlines)
      {
	pts.x = SHORT1FROMMP(mp1);
	pts.y = SHORT2FROMMP(mp1);
	if (pts.y < 0)
	{
	  WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
		     MPFROM2SHORT(0, SB_LINEDOWN));
	  pts.y = 1;
	  outofwindow = TRUE;
	}
	else if (pts.y > Rectl.yTop - Rectl.yBottom)
	{
	  WinSendMsg(hwnd, WM_VSCROLL, MPFROM2SHORT(FID_VERTSCROLL, 0),
		     MPFROM2SHORT(0, SB_LINEUP));
	  pts.y = (Rectl.yTop - Rectl.yBottom) - 1;
	  outofwindow = TRUE;
	}
	whichline = ((Rectl.yTop - Rectl.yBottom) -
		     ((LONG) pts.y + ad -> lMaxDescender)) /
	  ad -> lMaxHeight;
	if (whichline > numlines - 1)
	  whichline = numlines - 1;
	whichline += (ad -> topline - 1);
	if (whichline < ad -> numlines && ad -> lastselected != whichline)
	{
	  if (ad -> lastselected != ULONG_MAX)
	  {
	    inc = (ad -> lastselected < whichline) ? 1 : -1;
	    for (x = ad -> lastselected + inc;
		 x != whichline && x < ad -> numlines;
		 (ad -> lastselected < whichline) ? x++ : x--)
	    {
	      if (ad -> markedlines)
	      {
		if (ad -> markedlines[x] & VF_SELECTED)
		{
		  ad -> markedlines[x] &= (~VF_SELECTED);
		  ad -> selected--;
		}
		else
		{
		  ad -> markedlines[x] |= VF_SELECTED;
		  ad -> selected++;
		}
	      }
	      PaintLine(hwnd, ad -> hps, x, ad -> topline, &Rectl);
	    }
	    WinSendMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	  }
	  WinSendMsg(hwnd, WM_BUTTON1CLICK, MPFROM2SHORT(pts.x, pts.y),
		     MPFROM2SHORT(TRUE, 0));
	}
      }
      if (outofwindow)
      {

	POINTL ptl;

	WinQueryPointerPos(HWND_DESKTOP, &ptl);
	WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1L);
	if ((SHORT) ptl.y == (SHORT) SHORT2FROMMP(mp1) &&
	    (SHORT) ptl.x == (SHORT) SHORT1FROMMP(mp1) &&
	    ((SHORT) ptl.y < 0 || ptl.y > (Rectl.yTop - Rectl.yBottom)))
	{
	  PostMsg(hwnd, UM_MOUSEMOVE, mp1, MPVOID);
	  DosSleep(1L);
	}
      }
    }
    break;

  case UM_MOUSEMOVE:
    if (ad && ad -> mousecaptured)
    {

      POINTL ptl;
      RECTL Rectl;

      WinQueryWindowRect(hwnd, &Rectl);
      WinQueryPointerPos(HWND_DESKTOP, &ptl);
      WinMapWindowPoints(HWND_DESKTOP, hwnd, &ptl, 1L);
      if ((SHORT) ptl.y == (SHORT) SHORT2FROMMP(mp1) &&
	  (SHORT) ptl.x == (SHORT) SHORT1FROMMP(mp1) &&
	  ((SHORT) ptl.y < 0 || ptl.y > (Rectl.yTop - Rectl.yBottom)))
      {
	DosSleep(1L);
	PostMsg(hwnd, WM_MOUSEMOVE, mp1, MPFROM2SHORT(TRUE, 0));
      }
    }
    return 0;

  case WM_BUTTON1UP:
  case WM_BUTTON1MOTIONEND:
    WinSetFocus(HWND_DESKTOP, hwnd);
    if (ad && ad -> mousecaptured)
    {
      ad -> mousecaptured = FALSE;
      ad -> lastselected = ULONG_MAX;
      ad -> lastdirection = 0;
      DosReleaseMutexSem(ad -> ScanSem);
      WinSetCapture(HWND_DESKTOP, NULLHANDLE);
    }
    break;

  case WM_BUTTON1DBLCLK:
  case WM_BUTTON1CLICK:
    WinSetFocus(HWND_DESKTOP, hwnd);
    if (ad && !ad -> stopflag && ad -> numlines && ad -> text && !ad -> busy &&
	!DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
    {

      ULONG numlines, whichline, wascursored, width;
      RECTL Rectl;
      POINTS pts;

      WinQueryWindowRect(hwnd, &Rectl);
      numlines = NumLines(&Rectl, ad);
      if (!numlines)
	break;
      pts.x = SHORT1FROMMP(mp1);
      pts.y = SHORT2FROMMP(mp1);
      whichline = ((Rectl.yTop - Rectl.yBottom) -
		   ((LONG) pts.y + ad -> lMaxDescender)) /
	ad -> lMaxHeight;
      if (whichline > numlines - 1)
	whichline = numlines - 1;
      whichline += (ad -> topline - 1);
      if (whichline + 1 > ad -> numlines)
	break;
      wascursored = ad -> cursored;
      ad -> cursored = whichline + 1;
      if (msg == WM_BUTTON1CLICK)
      {
	if (ad -> lastselected != ULONG_MAX)
	{
	  if (whichline > ad -> lastselected)
	    ad -> lastdirection = 1;
	  else
	    ad -> lastdirection = 2;
	}
	else
	  ad -> lastdirection = 0;
	ad -> lastselected = whichline;
	if (whichline < ad -> numlines)
	{
	  if (ad -> markedlines)
	  {
	    if (ad -> markedlines[whichline] & VF_SELECTED)
	    {
	      ad -> selected--;
	      ad -> markedlines[whichline] &= (~VF_SELECTED);
	    }
	    else
	    {
	      ad -> selected++;
	      ad -> markedlines[whichline] |= VF_SELECTED;
	    }
	  }
	  WinSendMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	}
	PaintLine(hwnd, ad -> hps, whichline, ad -> topline, &Rectl);
	if (ad -> cursored != wascursored)
	{
	  PaintLine(hwnd, ad -> hps, wascursored - 1, ad -> topline, &Rectl);
	  PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	}
      }
      else
      {

	SHORT numsels, sSelect = 0, numinserted;
	ULONG linenum;

	if (!ad -> hex && ad -> lines)
	{

	  CHAR *p, *e;

	  width = (Rectl.xRight - Rectl.xLeft) / ad -> fattrs.lAveCharWidth;
	  e = p = ad -> lines[whichline];
	  while (*e != '\r' && *e != '\n' && e < ad -> text + ad -> textsize)
	  {
	    if (ad -> wrapon && e - p == width)
	      break;
	    e++;
	  }
	  if ((*e == '\r' || *e == '\n') && e > p)
	    e--;
	  width = e - p;
	  if (!width)
	    goto NoAdd;

	  if ((ad -> httpin && *httprun &&
	       strnstr(ad -> lines[whichline], "http://", width)) ||
	      (ad -> ftpin && *ftprun &&
	       strnstr(ad -> lines[whichline], "ftp://", width)))
	  {

	    USHORT ret;
	    URLDATA *urld;

	    urld = xmallocz(sizeof(URLDATA),pszSrcFile,__LINE__);
	    if (urld) {
	      urld -> size = sizeof(URLDATA);
	      urld -> line = ad -> lines[whichline];
	      urld -> len = width;
	      ret = (USHORT) WinDlgBox(HWND_DESKTOP, hwnd, UrlDlgProc,
				       FM3ModHandle, URL_FRAME, urld);
	      switch (ret) {
	      case 0:
		free(urld);
		goto NoAdd;
	      case 1:
		if (*urld -> url)
		  runemf2(SEPARATE | WINDOWED,
			  hwnd,
			  NULL,
			  NULL,
			  "%s %s",
			  httprun,
			  urld -> url);
		free(urld);
		goto NoAdd;
	      case 2:
		if (*urld -> url)
		  runemf2(SEPARATE | WINDOWED,
			  hwnd,
			  NULL,
			  NULL,
			  "%s %s",
			  ftprun,
			  urld -> url);
		free(urld);
		goto NoAdd;
	      default:
		break;
	      }
	      free(urld);
	    }
	  }
	}
	numsels = (SHORT) WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX,
					 LM_QUERYITEMCOUNT, MPVOID, MPVOID);
	if (numsels > 0)
	{
	  for (sSelect = 0; sSelect < numsels; sSelect++)
	  {
	    linenum = (ULONG) WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX,
						LM_QUERYITEMHANDLE,
						MPFROM2SHORT(sSelect, 0),
						MPVOID);
	    if (linenum == whichline)
	      goto NoAdd;
	  }
	}
	{
	  CHAR *s = NULL, *p;

	  if (!ad -> hex && ad -> lines)
	  {
	    s = xmalloc(width + 2,pszSrcFile,__LINE__);
	    if (!s)
	      goto NoAdd;
	    strncpy(s, ad -> lines[whichline], width + 1);
	    s[width + 1] = 0;
	    p = s;
	    while (*p)
	    {
	      if (*p == '\r' || *p == '\n')
	      {
		*p = 0;
		break;
	      }
	      p++;
	    }
	  }
	  else
	  {

	    register ULONG x;

	    width = ad -> textsize - (whichline * 16);
	    width = min(width, 16);
	    s = xmalloc(80,pszSrcFile,__LINE__);
	    if (!s)
	      goto NoAdd;
	    sprintf(s, "%08lx ", whichline * 16);
	    p = s + 9;
	    for (x = 0; x < width; x++)
	    {
	      sprintf(p, " %02hx", ad -> text[(whichline * 16) + x]);
	      p += 3;
	    }
	    *p = ' ';
	    p++;
	    *p = ' ';
	    p++;
	    for (x = 0; x < width; x++)
	    {
	      *p = ad -> text[(whichline * 16) + x];
	      p++;
	    }
	    *p = 0;
	  }
	  if (s)
	  {
	    if (*s)
	    {
	      ad -> dummy = TRUE;
	      numinserted = (SHORT) WinSendDlgItemMsg(ad -> hwndFrame,
						      NEWVIEW_LISTBOX,
						      LM_INSERTITEM,
						   MPFROM2SHORT(LIT_END, 0),
						      MPFROMP(s));
	      ad -> dummy = FALSE;
	      if (numinserted >= 0)
		WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX,
				  LM_SETITEMHANDLE,
				  MPFROM2SHORT(numinserted, 0),
				  MPFROMLONG(whichline));
	    }
	    free(s);
	  }
	}
	if (!numsels)
	  WinSendMsg(ad -> hwndFrame, WM_UPDATEFRAME,
		     MPFROMLONG(FCF_SIZEBORDER), MPVOID);
      }
    NoAdd:
      DosReleaseMutexSem(ad -> ScanSem);
      DosPostEventSem(CompactSem);
    }
    break;

  case WM_MENUEND:
    if (ad && ad -> hwndPopup == (HWND) mp2)
    {
      WinDestroyWindow(ad -> hwndPopup);
      ad -> hwndPopup = (HWND) 0;
    }
    break;

  case UM_CONTEXTMENU:
  case WM_CONTEXTMENU:
    if (ad)
    {
      if (!ad -> hwndPopup)
      {
	ad -> hwndPopup = WinLoadMenu(HWND_DESKTOP, FM3ModHandle, NEWVIEW_POPUP);
	if (ad -> hwndPopup)
	  WinSetPresParam(ad -> hwndPopup,
			  PP_FONTNAMESIZE,
			  strlen(GetPString(IDS_8HELVTEXT)) + 1,
			  GetPString(IDS_8HELVTEXT));
      }
      if (ad -> hwndPopup)
      {

	APIRET rc;
	SHORT sSelect;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	WinEnableMenuItem(ad -> hwndPopup, IDM_SAVETOCLIP, (rc == 0 &&
						      ad -> selected != 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_APPENDTOCLIP, (rc == 0 &&
						      ad -> selected != 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_SAVETOLIST, (rc == 0 &&
						      ad -> selected != 0));
	sSelect = (SHORT) WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX,
					 LM_QUERYITEMCOUNT, MPVOID, MPVOID);
	WinEnableMenuItem(ad -> hwndPopup, IDM_SAVETOCLIP2, (rc == 0 &&
							     sSelect > 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_APPENDTOCLIP2, (rc == 0 &&
							       sSelect > 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_SAVETOLIST2, (rc == 0 &&
							     sSelect > 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_SELECTALL, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines));
	WinEnableMenuItem(ad -> hwndPopup, IDM_DESELECTALL, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
						      ad -> selected != 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_PREVSELECTED, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
						      ad -> selected != 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_NEXTSELECTED, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
						      ad -> selected != 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_SELECTFOUND, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
							 ad -> found != 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_DESELECTFOUND, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
						      ad -> selected != 0 &&
							 ad -> found != 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_INVERT, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines));
	WinEnableMenuItem(ad -> hwndPopup, IDM_FINDFIRST, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines));
	WinEnableMenuItem(ad -> hwndPopup, IDM_FINDNEXT, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
							  ad -> found));
	WinEnableMenuItem(ad -> hwndPopup, IDM_FINDPREV, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
							  ad -> found));
	WinEnableMenuItem(ad -> hwndPopup, IDM_GOTOLINE, (rc == 0 &&
						      ad -> numlines != 0));
	WinEnableMenuItem(ad -> hwndPopup, IDM_GOTOOFFSET, (rc == 0 &&
						      ad -> textsize != 0));
	if (!rc)
	  DosReleaseMutexSem(ad -> ScanSem);
	PopupMenu(hwnd, hwnd, ad -> hwndPopup);
      }
    }
    break;

  case UM_SETUP3:
    if (ad && !ad -> busy &&
	!DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
    {
      ad -> multiplier = ad -> numlines / 32767;
      if (ad -> multiplier * 32767 != ad -> numlines)
	ad -> multiplier++;
      if (!ad -> multiplier)
	ad -> multiplier++;
      {
	RECTL Rectl;
	ULONG numlines;

	WinQueryWindowRect(hwnd, &Rectl);
	numlines = NumLines(&Rectl, ad);
	if (numlines)
	{
	  WinSendMsg(ad -> hhscroll, SBM_SETTHUMBSIZE,
		     MPFROM2SHORT((SHORT) Rectl.xRight, (SHORT) ad -> maxx),
		     MPVOID);
	  WinSendMsg(ad -> hvscroll, SBM_SETTHUMBSIZE,
		     MPFROM2SHORT((SHORT) numlines,
			       (SHORT) min(ad -> numlines, 32767)), MPVOID);
	  if (ad -> multiplier)
	    WinSendMsg(ad -> hvscroll, SBM_SETSCROLLBAR,
		    MPFROMSHORT((SHORT) (ad -> topline / ad -> multiplier)),
		       MPFROM2SHORT(1, (SHORT) ((ad -> numlines + 1) /
					     ad -> multiplier) - numlines));
	  WinSendMsg(ad -> hhscroll, SBM_SETSCROLLBAR,
		     MPFROMSHORT((SHORT) abs(ad -> horzscroll)),
		     MPFROM2SHORT(0, (SHORT) (ad -> maxx - Rectl.xRight)));
	  if (ad -> numlines - ad -> topline < numlines)
	  {
	    ad -> topline = ((ad -> numlines - ad -> topline) - numlines);
	    WinInvalidateRect(hwnd, NULL, FALSE);
	  }
	}
      }
      DosReleaseMutexSem(ad -> ScanSem);
    }
    return 0;

  case UM_SETUP4:
    if (ad && !ad -> busy &&
	!DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
    {

      CHAR s[140], t[34];
      ULONG numlines;
      RECTL Rectl;

      WinQueryWindowRect(hwnd, &Rectl);
      numlines = NumLines(&Rectl, ad);
      commafmt(t, sizeof(t), ad -> cursored);
      strcpy(s, GetPString(IDS_LINECOLONTEXT));
      strcat(s, t);
      if (ad -> selected)
      {
	if (ad -> selected > ad -> numlines)
	  ad -> selected = 0;
	else
	{
	  commafmt(t, sizeof(t), ad -> selected);
	  strcat(s, "  (");
	  strcat(s, t);
	  strcat(s, GetPString(IDS_SELECTEDPARENTEXT));
	}
      }
      if (ad -> found)
      {
	if (ad -> found > ad -> numlines)
	  ad -> found = 0;
	else
	{
	  commafmt(t, sizeof(t), ad -> found);
	  strcat(s, "  (");
	  strcat(s, t);
	  strcat(s, GetPString(IDS_FOUNDPARENTEXT));
	}
      }
      WinSetWindowText(ad -> hwndStatus2, s);
      if (!ad -> hex && ad -> lines)
	commafmt(t, sizeof(t), ad -> lines[ad -> cursored - 1] - ad -> text);
      else
	commafmt(t, sizeof(t), (ad -> cursored - 1) * 16);
      strcpy(s, GetPString(IDS_OFFSETCOLONTEXT));
      strcat(s, t);
      WinSetWindowText(ad -> hwndStatus3, s);
      if (ad -> multiplier)
	WinSendMsg(ad -> hvscroll, SBM_SETSCROLLBAR,
		   MPFROMSHORT((SHORT) (ad -> topline / ad -> multiplier)),
		   MPFROM2SHORT(1, (SHORT) ((ad -> numlines + 1) /
					    ad -> multiplier) - numlines));
      WinSendMsg(ad -> hhscroll, SBM_SETSCROLLBAR,
		 MPFROMSHORT((SHORT) abs(ad -> horzscroll)),
		 MPFROM2SHORT(0, (SHORT) (ad -> maxx - Rectl.xRight)));
      DosReleaseMutexSem(ad -> ScanSem);
    }
    return 0;

  case UM_CONTAINER_FILLED:
    if (ad && !ad -> busy &&
	!DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
    {
      ad -> stopflag = 0;
      ad -> topline = 1;
      ad -> cursored = 1;
      ad -> multiplier = 1;
      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
      WinEnableWindow(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				      IDM_NEXTBLANKLINE), !ad -> hex);
      WinEnableWindow(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
				      IDM_PREVBLANKLINE), !ad -> hex);
      if (!ad -> numlines)
      {
	if (!ad -> text)
          Runtime_Error(pszSrcFile, __LINE__, "no data");
	PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      }
      else
      {
	if (mp1 && (ULONG) mp1 < ad -> numlines + 1)
	{

	  RECTL Rectl;
	  ULONG numlines;

	  WinQueryWindowRect(hwnd, &Rectl);
	  numlines = NumLines(&Rectl, ad);
	  if (numlines)
	  {
	    ad -> topline = (ULONG) mp1;
	    if (ad -> numlines - ad -> topline < numlines)
	      ad -> topline = ad -> numlines - numlines;
	    ad -> cursored = (ULONG) mp1;
	    if (mp2)
	    {
	      ad -> cursored = (ULONG) mp2;
	      if (ad -> cursored > (ad -> topline - 1) + numlines)
		ad -> cursored = (ad -> topline - 1) + numlines;
	    }
	  }
	}
	WinSendMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
	PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	WinInvalidateRect(hwnd, NULL, FALSE);
      }
      DosReleaseMutexSem(ad -> ScanSem);
    }
    else if (ad)
      ad -> needrefreshing = TRUE;
    return 0;

  case WM_ERASEBACKGROUND:
    WinFillRect((HPS) mp1, (PRECTL) mp2,
		standardcolors[ad -> colors[COLORS_NORMALBACK]]);
    return 0;

  case WM_PAINT:
    if (ad)
    {

      HPS hpsp;
      RECTL Rectl;
      register ULONG x;
      ULONG numlines, wascursored = ad -> cursored;

      hpsp = WinBeginPaint(hwnd, ad -> hps, &Rectl);
      WinFillRect(hpsp, &Rectl,
		  standardcolors[ad -> colors[COLORS_NORMALBACK]]);
      if (!ad -> stopflag && !ad -> busy &&
	  !DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
      {
	WinQueryWindowRect(hwnd, &Rectl);
	numlines = NumLines(&Rectl, ad);
	if (numlines)
	{
	  if (ad -> numlines && (ad -> lines || ad -> hex))
	  {
	    if (ad -> topline > (ad -> numlines + 1) - numlines)
	      ad -> topline = (ad -> numlines + 1) - numlines;
	    if (ad -> topline > ad -> numlines)
	      ad -> topline = 1;
	    if (!ad -> topline)
	      ad -> topline = 1;
	    if (ad -> cursored < ad -> topline)
	      ad -> cursored = ad -> topline;
	    else if (ad -> cursored > (ad -> topline + numlines) - 1)
	      ad -> cursored = (ad -> topline + numlines) - 1;
	    if (ad -> cursored > ad -> numlines)
	      ad -> cursored = ad -> numlines;
	    if (wascursored != ad -> cursored)
	      PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	  }
	  else
	    ad -> topline = ad -> cursored = 1;
	  if (ad -> numlines && (ad -> lines || ad -> hex))
	  {
	    for (x = ad -> topline - 1; x < ad -> numlines; x++)
	    {
	      if (((LONG) (Rectl.yTop -
			   (ad -> lMaxHeight *
	      (((x + 1) - ad -> topline) + 1))) - ad -> lMaxDescender) <= 0)
		break;
	      PaintLine(hwnd, hpsp, x, ad -> topline, &Rectl);
	    }
	  }
	}
	if (ad -> multiplier)
	  WinSendMsg(ad -> hvscroll, SBM_SETSCROLLBAR,
		     MPFROMSHORT((SHORT) (ad -> topline / ad -> multiplier)),
		     MPFROM2SHORT(1, (SHORT) ((ad -> numlines + 1) /
					      ad -> multiplier) - numlines));
	WinSendMsg(ad -> hhscroll, SBM_SETSCROLLBAR,
		   MPFROMSHORT((SHORT) abs(ad -> horzscroll)),
		   MPFROM2SHORT(0, (SHORT) (ad -> maxx - Rectl.xRight)));
	WinSendMsg(ad -> hhscroll, SBM_SETTHUMBSIZE,
		   MPFROM2SHORT((SHORT) Rectl.xRight, (SHORT) ad -> maxx),
		   MPVOID);
	DosReleaseMutexSem(ad -> ScanSem);
	ad -> needrefreshing = FALSE;
      }
      else
	ad -> needrefreshing = TRUE;
      WinEndPaint(hpsp);
    }
    else
    {

      HPS hpsp;

      hpsp = WinBeginPaint(hwnd, (HPS) 0, NULL);
      WinEndPaint(hpsp);
    }
    PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
    break;

  case WM_HSCROLL:
    {
      RECTL rectl;
      BOOL invalidate = TRUE;

      WinQueryWindowRect(hwnd, &rectl);
      switch (SHORT2FROMMP(mp2))
      {
      case SB_PAGERIGHT:
	if (abs(ad -> horzscroll) <= ad -> maxx - rectl.xRight)
	{
	  ad -> horzscroll -= rectl.xRight;
	  if (abs(ad -> horzscroll) > ad -> maxx - rectl.xRight)
	    ad -> horzscroll = -((ad -> maxx - rectl.xRight) +
				 ad -> fattrs.lAveCharWidth);
	}
	else
	  invalidate = FALSE;
	break;

      case SB_PAGELEFT:
	if (ad -> horzscroll < 0)
	{
	  ad -> horzscroll += rectl.xRight;
	  if (ad -> horzscroll > 0)
	    ad -> horzscroll = 0;
	}
	else
	  invalidate = FALSE;
	break;

      case SB_LINERIGHT:
	if (abs(ad -> horzscroll) <= ad -> maxx - rectl.xRight)
	  ad -> horzscroll -= ad -> fattrs.lAveCharWidth;
	else
	  invalidate = FALSE;
	break;

      case SB_LINELEFT:
	if (ad -> horzscroll < 0)
	  ad -> horzscroll += ad -> fattrs.lAveCharWidth;
	else
	  invalidate = FALSE;
	break;

      case SB_SLIDERTRACK:
	ad -> horzscroll = (SHORT1FROMMP(mp2) / ad -> fattrs.lAveCharWidth) *
	  ad -> fattrs.lAveCharWidth;
	ad -> horzscroll = -(ad -> horzscroll);
	if (ad -> horzscroll > 0)
	  ad -> horzscroll = 0;
	if (abs(ad -> horzscroll) > (ad -> maxx - rectl.xRight) +
	    ad -> fattrs.lAveCharWidth)
	  ad -> horzscroll = -(ad -> maxx - rectl.xRight);
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
    if (ad && !ad -> stopflag && ad -> text && ad -> numlines && !ad -> busy &&
	!DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
    {

      ULONG numlines, wascursored;
      RECTL rcl;

      WinQueryWindowRect(hwnd, &rcl);
      numlines = NumLines(&rcl, ad);
      if (numlines)
      {
	wascursored = ad -> cursored;
	switch (SHORT2FROMMP(mp2))
	{
	case SB_PAGEUP:
	  if (ad -> topline > 1)
	  {
	    ad -> topline -= numlines;
	    if (ad -> topline > ad -> numlines ||
		ad -> topline + numlines > (ad -> numlines + 1))
	      ad -> topline = 1;
	    if (ad -> cursored > ad -> topline + numlines)
	      ad -> cursored = ad -> topline + numlines;
	    if (ad -> cursored > ad -> numlines)
	      ad -> cursored = ad -> numlines;
	    WinInvalidateRect(hwnd, NULL, FALSE);
	  }
	  break;
	case SB_PAGEDOWN:
	  if (ad -> topline + numlines <= ad -> numlines)
	  {
	    ad -> topline += numlines;
	    if (ad -> topline + numlines > ad -> numlines + 1)
	      ad -> topline = (ad -> numlines + 1) - numlines;
	    if (ad -> cursored < ad -> topline)
	      ad -> cursored = ad -> topline;
	    if (ad -> cursored + 1 > ad -> topline + numlines)
	      ad -> cursored = (ad -> topline + numlines) - 1;
	    if (ad -> cursored > ad -> numlines)
	      ad -> cursored = ad -> numlines;
	    WinInvalidateRect(hwnd, NULL, FALSE);
	  }
	  break;
	case SB_LINEDOWN:
	  if (ad -> topline + numlines <= ad -> numlines)
	  {

	    RECTL Rectl, iRectl;

	    ad -> topline++;
	    if (ad -> cursored < ad -> topline)
	      ad -> cursored = ad -> topline;
	    else if (ad -> cursored + 1 > ad -> topline + numlines)
	      ad -> cursored = (ad -> topline + numlines) - 1;
	    if (ad -> cursored > ad -> numlines)
	      ad -> cursored = ad -> numlines;
	    WinQueryWindowRect(hwnd, &Rectl);
	    WinScrollWindow(hwnd, 0, ad -> lMaxHeight,
			    NULL, NULL, NULLHANDLE, &iRectl, 0);
	    WinFillRect(ad -> hps, &iRectl,
			standardcolors[ad -> colors[COLORS_NORMALBACK]]);
	    PaintLine(hwnd, ad -> hps, (ad -> topline + numlines) - 2,
		      ad -> topline, &Rectl);
	    if (ad -> cursored != ad -> topline + numlines)
	      PaintLine(hwnd, ad -> hps, ad -> cursored - 1, ad -> topline, &Rectl);
	    if (wascursored != ad -> cursored &&
		wascursored < ad -> topline + numlines &&
		wascursored >= ad -> topline)
	      PaintLine(hwnd, ad -> hps, wascursored - 1, ad -> topline, &Rectl);
	    if (numlines >= ad -> numlines)
	      numlines = 0;
	    if (ad -> multiplier)
	      WinSendMsg(ad -> hvscroll, SBM_SETSCROLLBAR,
		    MPFROMSHORT((SHORT) (ad -> topline / ad -> multiplier)),
			 MPFROM2SHORT(1, (SHORT) ((ad -> numlines + 1) /
						  ad -> multiplier) -
				      numlines));
	    WinSendMsg(ad -> hhscroll, SBM_SETSCROLLBAR,
		       MPFROMSHORT((SHORT) abs(ad -> horzscroll)),
		       MPFROM2SHORT(0, (SHORT) (ad -> maxx - Rectl.xRight)));
	  }
	  break;
	case SB_LINEUP:
	  if (ad -> topline > 1)
	  {

	    RECTL Rectl, iRectl;

	    ad -> topline--;
	    if (ad -> cursored < ad -> topline)
	      ad -> cursored = ad -> topline;
	    else if (ad -> cursored + 1 > ad -> topline + numlines)
	      ad -> cursored = (ad -> topline + numlines) - 1;
	    if (ad -> cursored > ad -> numlines)
	      ad -> cursored = ad -> numlines;
	    WinQueryWindowRect(hwnd, &Rectl);
	    WinScrollWindow(hwnd, 0, -ad -> lMaxHeight,
			    NULL, NULL, NULLHANDLE, &iRectl, 0);
	    WinFillRect(ad -> hps, &iRectl,
			standardcolors[ad -> colors[COLORS_NORMALBACK]]);
	    iRectl = Rectl;
	    iRectl.yTop -= ((numlines * ad -> lMaxHeight) +
			    ad -> lMaxDescender);
	    WinFillRect(ad -> hps, &iRectl,
			standardcolors[ad -> colors[COLORS_NORMALBACK]]);
	    PaintLine(hwnd, ad -> hps, ad -> topline - 1, ad -> topline, &Rectl);
	    if (ad -> cursored != ad -> topline)
	      PaintLine(hwnd, ad -> hps, ad -> cursored - 1, ad -> topline, &Rectl);
	    if (ad -> cursored != wascursored &&
		wascursored >= ad -> topline &&
		wascursored < ad -> topline + numlines)
	      PaintLine(hwnd, ad -> hps, wascursored - 1, ad -> topline, &Rectl);
	    if (numlines >= ad -> numlines)
	      numlines = 0;
	    if (ad -> multiplier)
	      WinSendMsg(ad -> hvscroll, SBM_SETSCROLLBAR,
		    MPFROMSHORT((SHORT) (ad -> topline / ad -> multiplier)),
			 MPFROM2SHORT(1, (SHORT) ((ad -> numlines + 1) /
						  ad -> multiplier) -
				      numlines));
	    WinSendMsg(ad -> hhscroll, SBM_SETSCROLLBAR,
		       MPFROMSHORT((SHORT) abs(ad -> horzscroll)),
		       MPFROM2SHORT(0, (SHORT) (ad -> maxx - Rectl.xRight)));
	  }
	  break;
	case SB_SLIDERTRACK:
	  if ((SHORT1FROMMP(mp2) >= 1) ||
	      (SHORT1FROMMP(mp2)) <= ad -> numlines)
	  {
	    ad -> topline = (ULONG) SHORT1FROMMP(mp2) * ad -> multiplier;
	    if (ad -> topline + numlines > ad -> numlines + 1)
	      ad -> topline = (ad -> numlines + 1) - numlines;
	    if (!ad -> topline)
	      ad -> topline = 1;
	    if (ad -> cursored < ad -> topline)
	      ad -> cursored = ad -> topline;
	    else if (ad -> cursored > ad -> topline + numlines)
	      ad -> cursored = ad -> topline + numlines;
	    if (ad -> cursored > ad -> numlines)
	      ad -> cursored = ad -> numlines;
	    WinInvalidateRect(hwnd, NULL, FALSE);
	  }
	  else
	    WinAlarm(HWND_DESKTOP, WA_NOTE);
	  break;
	}
	if (ad -> cursored != wascursored)
	  PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
      }
      DosReleaseMutexSem(ad -> ScanSem);
    }
    break;

  case WM_INITMENU:
    switch (SHORT1FROMMP(mp1))
    {
    case IDM_FILESMENU:
      {
	APIRET rc;
	SHORT sSelect;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	WinEnableMenuItem((HWND) mp2, IDM_SAVETOCLIP, (rc == 0 &&
						       ad -> selected != 0));
	WinEnableMenuItem((HWND) mp2, IDM_APPENDTOCLIP, (rc == 0 &&
						      ad -> selected != 0));
	WinEnableMenuItem((HWND) mp2, IDM_SAVETOLIST, (rc == 0 &&
						       ad -> selected != 0));
	sSelect = (SHORT) WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX,
					 LM_QUERYITEMCOUNT, MPVOID, MPVOID);
	WinEnableMenuItem((HWND) mp2, IDM_SAVETOCLIP2, (rc == 0 &&
							sSelect > 0));
	WinEnableMenuItem((HWND) mp2, IDM_APPENDTOCLIP2, (rc == 0 &&
							  sSelect > 0));
	WinEnableMenuItem((HWND) mp2, IDM_SAVETOLIST2, (rc == 0 &&
							sSelect > 0));
	if (!rc)
	  DosReleaseMutexSem(ad -> ScanSem);
      }
      break;

    case IDM_VIEWSMENU:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	WinEnableMenuItem((HWND) mp2, IDM_FONTPALETTE, (rc == 0));
	WinEnableMenuItem((HWND) mp2, IDM_HEXMODE, (rc == 0));
	WinEnableMenuItem((HWND) mp2, IDM_WRAP, (rc == 0));
	WinEnableMenuItem((HWND) mp2, IDM_CODEPAGE, (rc == 0));
	if (!rc)
	  DosReleaseMutexSem(ad -> ScanSem);
      }
      WinCheckMenuItem((HWND) mp2, IDM_HEXMODE, ad -> hex);
      WinCheckMenuItem((HWND) mp2, IDM_WRAP, ad -> wrapon);
      WinCheckMenuItem((HWND) mp2, IDM_IGNOREFTP, ad -> ignoreftp);
      WinCheckMenuItem((HWND) mp2, IDM_IGNOREHTTP, ad -> ignorehttp);
      break;

    case IDM_SEARCHMENU:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	WinEnableMenuItem((HWND) mp2, IDM_FINDFIRST, (rc == 0 &&
						      ad -> numlines != 0 &&
						      ad -> markedlines));
	WinEnableMenuItem((HWND) mp2, IDM_FINDNEXT, (rc == 0 &&
						     ad -> numlines != 0 &&
						     ad -> markedlines &&
						     ad -> found != 0));
	WinEnableMenuItem((HWND) mp2, IDM_FINDPREV, (rc == 0 &&
						     ad -> numlines != 0 &&
						     ad -> markedlines &&
						     ad -> found != 0));
	WinEnableMenuItem((HWND) mp2, IDM_NEXTBLANKLINE, (rc == 0 &&
						      ad -> numlines != 0 &&
							  !ad -> hex));
	WinEnableMenuItem((HWND) mp2, IDM_PREVBLANKLINE, (rc == 0 &&
						      ad -> numlines != 0 &&
							  !ad -> hex));
	WinEnableMenuItem((HWND) mp2, IDM_GOTOLINE, (rc == 0 &&
						     ad -> numlines != 0));
	WinEnableMenuItem((HWND) mp2, IDM_GOTOOFFSET, (rc == 0 &&
						       ad -> textsize != 0));
	if (!rc)
	  DosReleaseMutexSem(ad -> ScanSem);
      }
      break;

    case IDM_SELECTSUBMENU:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	WinEnableMenuItem((HWND) mp2, IDM_SELECTALL, (rc == 0 &&
						      ad -> numlines != 0 &&
						      ad -> markedlines &&
						      (ad -> selected !=
						       ad -> numlines ||
						       !ad -> selected)));
	WinEnableMenuItem((HWND) mp2, IDM_DESELECTALL, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
						      ad -> selected != 0));
	WinEnableMenuItem((HWND) mp2, IDM_DESELECTFOUND, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
						      ad -> selected != 0 &&
							  ad -> found != 0));
	WinEnableMenuItem((HWND) mp2, IDM_SELECTFOUND, (rc == 0 &&
						      ad -> numlines != 0 &&
							ad -> markedlines &&
							ad -> found != 0 &&
							(ad -> numlines !=
							 ad -> selected ||
							 !ad -> selected)));
	WinEnableMenuItem((HWND) mp2, IDM_NEXTSELECTED, (rc == 0 &&
						      ad -> numlines != 0 &&
							 ad -> markedlines &&
						      ad -> selected != 0));
	WinEnableMenuItem((HWND) mp2, IDM_PREVSELECTED, (rc == 0 &&
						      ad -> numlines != 0 &&
							 ad -> markedlines &&
						      ad -> selected != 0));
	WinEnableMenuItem((HWND) mp2, IDM_INVERT, (rc == 0 &&
						   ad -> numlines != 0 &&
						   ad -> markedlines));
	if (!rc)
	  DosReleaseMutexSem(ad -> ScanSem);
      }
      break;
    }
    break;

  case UM_CONTROL:
    switch (SHORT1FROMMP(mp1))
    {
    case NEWVIEW_LISTBOX:
      switch (SHORT2FROMMP(mp1))
      {
      case LN_SETFOCUS:
	if (ad)
	{
	  if (!ad -> clientfocused)
	  {
	    PostMsg(hwnd,
		    WM_COMMAND,
		    MPFROM2SHORT(IDM_NEXTWINDOW, 0),
		    MPVOID);
	    break;
	  }
	  ad -> clientfocused = FALSE;
	}
	PostMsg(hwnd,
		UM_CONTROL,
		MPFROM2SHORT(NEWVIEW_LISTBOX,
			     LN_SELECT),
		MPVOID);
	break;
      case LN_KILLFOCUS:
	if (ad)
	{
	  ad -> clientfocused = TRUE;
	  WinSetFocus(HWND_DESKTOP, hwnd);
	}
	break;
      case LN_SELECT:
	if (ad && !ad -> dummy)
	{

	  ULONG linenum, numlines;
	  SHORT sSelect;
	  HWND hwndUL = WinWindowFromID(ad -> hwndFrame,
					SHORT1FROMMP(mp1));
	  RECTL Rectl;

	  sSelect = (SHORT) WinSendMsg(hwndUL,
				       LM_QUERYSELECTION,
				       MPFROM2SHORT(LIT_FIRST, 0),
				       MPVOID);
	  if (sSelect >= 0)
	  {
	    linenum = (ULONG) WinSendMsg(hwndUL,
					 LM_QUERYITEMHANDLE,
					 MPFROM2SHORT(sSelect, 0),
					 MPVOID);
	    if (ad -> topline != linenum + 1 &&
		linenum < ad -> numlines)
	    {
	      WinQueryWindowRect(hwnd, &Rectl);
	      numlines = NumLines(&Rectl, ad);
	      ad -> topline = linenum + 1;
	      if (ad -> numlines - ad -> topline < numlines)
		ad -> topline = ad -> numlines - numlines;
	      ad -> cursored = linenum + 1;
	      WinInvalidateRect(hwnd, NULL, FALSE);
	      PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	      PostMsg(hwnd, UM_RESCAN, MPVOID, MPVOID);
	    }
	  }
	  else
	    PostMsg(hwndUL, LM_SELECTITEM, MPFROM2SHORT(0, 0),
		    MPFROM2SHORT(TRUE, 0));
	}
	break;

      case LN_ENTER:
	if (ad)
	{

	  SHORT sSelect;
	  HWND hwndUL = WinWindowFromID(ad -> hwndFrame,
					SHORT1FROMMP(mp1));

	  sSelect = (SHORT) WinSendMsg(hwndUL,
				       LM_QUERYSELECTION,
				       MPFROM2SHORT(LIT_FIRST, 0),
				       MPVOID);
	  if (sSelect >= 0)
	  {
	    ad -> dummy = TRUE;
	    WinSendMsg(hwndUL, LM_DELETEITEM,
		       MPFROM2SHORT(sSelect, 0), MPVOID);
	    ad -> dummy = FALSE;
	    sSelect = (SHORT) WinSendMsg(hwndUL,
					 LM_QUERYITEMCOUNT,
					 MPVOID,
					 MPVOID);
	    if (sSelect <= 0)
	    {
	      PostMsg(ad -> hwndFrame, WM_UPDATEFRAME,
		      MPFROMLONG(FCF_SIZEBORDER), MPVOID);
	      WinSetFocus(HWND_DESKTOP, hwnd);
	    }
	  }
	}
	break;

      default:
	break;
      }
      break;

    default:
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1))
    {
    case IDM_EDIT:
      if (*editor)
      {

	CHAR *dummy[2];

	dummy[0] = ad -> filename;
	dummy[1] = NULL;
	ExecOnList(hwnd, editor, WINDOWED | SEPARATE, NULL, dummy, NULL);
      }
      else
	StartMLEEditor(ad -> hwndParent, 4, ad -> filename,
		       ad -> hwndFrame);
      ad -> hwndRestore = (HWND) 0;
      PostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
      break;

    case IDM_IGNOREFTP:
      ad -> ignoreftp = (ad -> ignoreftp) ? FALSE : TRUE;
      ad -> ftpin = FALSE;
      if (ad -> text && *ftprun && !ad -> ignoreftp &&
	  strstr(ad -> text, "ftp://"))
	ad -> ftpin = TRUE;
      IgnoreFTP = ad -> ignoreftp;
      PrfWriteProfileData(fmprof, appname, "Viewer.IgnoreFTP",
			  &ad -> ignoreftp, sizeof(BOOL));
      WinInvalidateRect(hwnd, NULL, FALSE);
      break;

    case IDM_IGNOREHTTP:
      ad -> ignorehttp = (ad -> ignorehttp) ? FALSE : TRUE;
      ad -> httpin = FALSE;
      if (ad -> text && *httprun && !ad -> ignorehttp &&
	  strstr(ad -> text, "http://"))
	ad -> httpin = TRUE;
      IgnoreHTTP = ad -> ignorehttp;
      PrfWriteProfileData(fmprof, appname, "Viewer.IgnoreHTTP",
			  &ad -> ignorehttp, sizeof(BOOL));
      WinInvalidateRect(hwnd, NULL, FALSE);
      break;

    case IDM_PREVBLANKLINE:
      if (!ad -> hex && ad -> lines)
      {

	ULONG x;

	x = ad -> cursored - 2;
	if (x >= ad -> numlines)
	  x = 0;
	while (x < ad -> numlines &&
	       (*ad -> lines[x] == '\r' || *ad -> lines[x] == '\n'))
	  x--;
	if (x >= ad -> numlines)
	  x = 0;
	for (; x < ad -> numlines; x--)
	{
	  if (*ad -> lines[x] == '\r' || *ad -> lines[x] == '\n')
	  {
	    if (x < ad -> numlines - 1)
	      x++;
	    break;
	  }
	}
	if (x < ad -> numlines)
	{
	  ad -> topline = ad -> cursored = x;
	  WinInvalidateRect(hwnd, NULL, FALSE);
	}
      }
      break;

    case IDM_NEXTBLANKLINE:
      if (!ad -> hex && ad -> lines)
      {

	ULONG x;

	x = ad -> cursored;
	while (x < ad -> numlines &&
	       (*ad -> lines[x] == '\r' || *ad -> lines[x] == '\n'))
	  x++;
	for (; x < ad -> numlines; x++)
	{
	  if (*ad -> lines[x] == '\r' || *ad -> lines[x] == '\n')
	  {
	    if (x < ad -> numlines - 1)
	      x++;
	    break;
	  }
	}
	if (x < ad -> numlines)
	{
	  while (x < ad -> numlines &&
		 (*ad -> lines[x] == '\r' || *ad -> lines[x] == '\n'))
	    x++;
	  if (x < ad -> numlines)
	  {
	    ad -> topline = ad -> cursored = x;
	    WinInvalidateRect(hwnd, NULL, FALSE);
	  }
	}
      }
      break;

    case IDM_VIEW:
    case IDM_OBJECT:
      if (!ad -> hex && ad -> lines)
      {

	CHAR line[CCHMAXPATH], filename[CCHMAXPATH], *p;

	strncpy(line, ad -> lines[ad -> cursored - 1], CCHMAXPATH);
	line[CCHMAXPATH - 1] = 0;
	chop_at_crnl(line);
	if (*line == '\"')
	{
	  memmove(line, line + 1, strlen(line));
	  p = strchr(line, '\"');
	  lstrip(line);
	  if (p)
	    *p = 0;
	  rstrip(line);
	}
	else
	{
	  lstrip(line);
	  p = strchr(line, ' ');
	  if (p)
	    *p = 0;
	  rstrip(line);
	}
	if (!strchr(line, '\\') && !strchr(line, '/') && !strchr(line, ':'))
	{
	  strcpy(filename, ad -> filename);
	  p = strrchr(filename, '\\');
	  if (p)
	    p++;
	  else
	    p = filename;
	  strcpy(p, line);
	}
	else
	  strcpy(filename, line);
	MakeFullName(filename);
	if (*filename &&
	    IsFile(filename) == 1)
	{
	  if (SHORT1FROMMP(mp1) == IDM_OBJECT)
	    OpenObject(filename,
		       Default,
		       ad -> hwndFrame);
	  else
	    DefaultView(hwnd,
			ad -> hwndFrame,
			HWND_DESKTOP,
			NULL,
			0,
			filename);
	}
      }
      break;

    case IDM_COLORPALETTE:
      {
	COLORS co;
	LONG temp[COLORS_MAX];

	memset(&co, 0, sizeof(co));
	co.size = sizeof(co);
	co.numcolors = COLORS_MAX;
	co.colors = ad -> colors;
	co.descriptions = IDS_NVCOLORS1TEXT;
	co.origs = temp;
	co.prompt = IDS_NVCOLORSPROMPTTEXT;
	memcpy(temp,
	       ad -> colors,
	       sizeof(LONG) * COLORS_MAX);
	if (WinDlgBox(HWND_DESKTOP,
		      hwnd,
		      ColorDlgProc,
		      FM3ModHandle,
		      COLOR_FRAME,
		      (PVOID) & co))
	{
	  memcpy(Colors,
		 ad -> colors,
		 sizeof(LONG) * COLORS_MAX);
	  PrfWriteProfileData(fmprof,
			      appname,
			      "Viewer.Colors",
			      &ad -> colors,
			      sizeof(LONG) * COLORS_MAX);
	  WinInvalidateRect(hwnd,
			    NULL,
			    FALSE);
	  WinInvalidateRect(ad -> hwndStatus1,
			    NULL,
			    FALSE);
	  WinInvalidateRect(ad -> hwndStatus2,
			    NULL,
			    FALSE);
	  WinInvalidateRect(ad -> hwndStatus3,
			    NULL,
			    FALSE);
	}
      }
      break;

    case IDM_NEXTWINDOW:
    case IDM_PREVWINDOW:
      {
	SHORT sSelect;

	sSelect = (SHORT) WinSendDlgItemMsg(ad -> hwndFrame,
					    NEWVIEW_LISTBOX,
					    LM_QUERYITEMCOUNT,
					    MPVOID,
					    MPVOID);
	if (sSelect)
	{
	  if (!ad -> clientfocused)
	    WinSetFocus(HWND_DESKTOP, hwnd);
	  else
	    WinSetFocus(HWND_DESKTOP,
			WinWindowFromID(ad -> hwndFrame,
					NEWVIEW_LISTBOX));
	}
	else
	  WinSetFocus(HWND_DESKTOP, hwnd);
      }
      break;

    case IDM_FINDFIRST:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	if (!rc)
	{
	  if (!ad -> busy && ad -> text && ad -> numlines && ad -> markedlines)
	  {

	    ULONG numlines;
	    RECTL Rectl;
	    static char test[SEARCHSTRINGLEN];

	    WinQueryWindowRect(hwnd, &Rectl);
	    numlines = NumLines(&Rectl, ad);
	    if (!numlines)
	      break;
	    strcpy(test, ad -> searchtext);
	    if (WinDlgBox(HWND_DESKTOP, hwnd, FindStrDlgProc, FM3ModHandle,
			  NEWFIND_FRAME, (PVOID) & hwnd))
	    {
	      if (*ad -> searchtext &&
		  strcmp(test, ad -> searchtext))
		PrfWriteProfileString(fmprof,
				      appname,
				      "Viewer.Searchtext",
				      (PVOID) ad -> searchtext);
	      if (_beginthread(Search, NULL, 524288, (PVOID) hwnd) == -1)
                Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
	    }
	  }
	  DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      break;

    case IDM_PREVSELECTED:
    case IDM_NEXTSELECTED:
    case IDM_FINDPREV:
    case IDM_FINDNEXT:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	if (!rc)
	{
	  if (!ad -> busy && ad -> text && ad -> markedlines)
	  {

	    RECTL Rectl;
	    register ULONG x;
	    ULONG numlines;
	    CHAR markedwith;

	    markedwith = (SHORT1FROMMP(mp1) == IDM_FINDNEXT ||
			  SHORT1FROMMP(mp1) == IDM_FINDPREV) ?
	      VF_FOUND : VF_SELECTED;
	    WinQueryWindowRect(hwnd, &Rectl);
	    numlines = NumLines(&Rectl, ad);
	    if (!numlines)
	      break;
	    WinSetPointer(HWND_DESKTOP, hptrBusy);
	    if (SHORT1FROMMP(mp1) == IDM_PREVSELECTED ||
		SHORT1FROMMP(mp1) == IDM_FINDPREV)
	    {
	      for (x = ad -> cursored - 2; x < ULONG_MAX - 1; x--)
	      {
		if (ad -> markedlines[x] & markedwith)
		{
		  ad -> topline = x + 1;
		  if (ad -> numlines - ad -> topline < numlines)
		    ad -> topline = ad -> numlines - numlines;
		  ad -> cursored = x + 1;
		  WinInvalidateRect(hwnd, NULL, FALSE);
		  PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
		  break;
		}
	      }
	    }
	    else
	    {
	      for (x = ad -> cursored; x < ad -> numlines; x++)
	      {
		if (ad -> markedlines[x] & markedwith)
		{
		  ad -> topline = x + 1;
		  if (ad -> numlines - ad -> topline < numlines)
		    ad -> topline = ad -> numlines - numlines;
		  ad -> cursored = x + 1;
		  WinInvalidateRect(hwnd, NULL, FALSE);
		  PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
		  break;
		}
	      }
	    }
	    WinSetPointer(HWND_DESKTOP, hptrArrow);
	    if (x >= ad -> numlines)
	      DosBeep(50, 100);
	  }
	  DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      break;

    case IDM_SELECTFOUND:
    case IDM_DESELECTFOUND:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	if (!rc)
	{
	  if (!ad -> busy && ad -> text && ad -> markedlines)
	  {

	    RECTL Rectl;
	    register ULONG x;
	    ULONG numlines;

	    WinQueryWindowRect(hwnd, &Rectl);
	    numlines = NumLines(&Rectl, ad);
	    if (!numlines)
	      break;
	    WinSetPointer(HWND_DESKTOP, hptrBusy);
	    for (x = 0; x < ad -> numlines; x++)
	    {
	      if (SHORT1FROMMP(mp1) == IDM_SELECTFOUND)
	      {
		if ((ad -> markedlines[x] & VF_FOUND) &&
		    !(ad -> markedlines[x] & VF_SELECTED))
		{
		  ad -> markedlines[x] |= VF_SELECTED;
		  ad -> selected++;
		}
	      }
	      else
	      {
		if ((ad -> markedlines[x] & VF_FOUND) &&
		    (ad -> markedlines[x] & VF_SELECTED))
		{
		  ad -> markedlines[x] &= (~VF_SELECTED);
		  ad -> selected--;
		}
	      }
	    }
	    WinSendMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	    WinSetPointer(HWND_DESKTOP, hptrArrow);
	    WinInvalidateRect(hwnd, NULL, FALSE);
	  }
	  DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      break;

    case IDM_GOTOLINE:
    case IDM_GOTOOFFSET:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	if (!rc)
	{
	  if (!ad -> busy && ad -> numlines)
	  {

	    ULONG numlines, linenum;
	    CHAR s[34], ss[134];
	    STRINGINPARMS sip;
	    RECTL Rectl;
	    register ULONG x;

	    WinQueryWindowRect(hwnd, &Rectl);
	    numlines = NumLines(&Rectl, ad);
	    if (!numlines)
	      break;
	    if (ad -> numlines <= numlines)
	    {
	      DosBeep(500, 100);
	      break;
	    }
	    sip.help = (SHORT1FROMMP(mp1) == IDM_GOTOLINE) ?
	      GetPString(IDS_NVLINEJUMPTEXT) :
	      GetPString(IDS_NVBYTEJUMPTEXT);
	    sip.ret = s;
	    *s = 0;
	    sip.prompt = ss;
	    sip.inputlen = 34;
	    sip.title = (SHORT1FROMMP(mp1) == IDM_GOTOLINE) ?
	      GetPString(IDS_NVLINEJUMPTITLETEXT) :
	      GetPString(IDS_NVBYTEJUMPTITLETEXT);
	    sprintf(sip.prompt,
		    GetPString(IDS_NVJUMPTEXT),
		    (SHORT1FROMMP(mp1) == IDM_GOTOLINE) ?
		    GetPString(IDS_LINETEXT) :
		    GetPString(IDS_OFFSETTEXT),
		    (SHORT1FROMMP(mp1) == IDM_GOTOLINE) ?
		    1 :
		    0,
		    (SHORT1FROMMP(mp1) == IDM_GOTOLINE) ?
		    ad -> numlines :
		    ad -> textsize - 1);
	    WinDlgBox(HWND_DESKTOP,
		      hwnd,
		      InputDlgProc,
		      FM3ModHandle,
		      STR_FRAME,
		      &sip);
	    if (*s)
	    {
	      s[33] = 0;
	      linenum = atol(s);
	      switch (SHORT1FROMMP(mp1))
	      {
	      case IDM_GOTOLINE:
		if (linenum > 0 &&
		    linenum <= ad -> numlines)
		{
		  ad -> topline = linenum;
		  ad -> cursored = ad -> topline;
		  if (ad -> numlines - ad -> topline < numlines)
		    ad -> topline = (ad -> numlines - numlines) + 1;
		  WinInvalidateRect(hwnd,
				    NULL,
				    FALSE);
		}
		break;
	      case IDM_GOTOOFFSET:
		if (linenum < ad -> textsize)
		{
		  if (ad -> hex)
		    ad -> topline = (linenum / 16) + 1;
		  else if (ad -> lines)
		  {
		    ad -> topline = (ULONG) - 1;
		    for (x = 0; x < ad -> numlines; x++)
		    {
		      if (ad -> lines[x] > ad -> text + linenum)
		      {
			ad -> topline = x + 1;
			break;
		      }
		    }
		    if (ad -> topline == (ULONG) - 1)
		      ad -> topline = ad -> numlines;
		  }
		  ad -> cursored = ad -> topline;
		  if (ad -> numlines - ad -> topline < numlines)
		    ad -> topline = (ad -> numlines - numlines) + 1;
		  WinInvalidateRect(hwnd,
				    NULL,
				    FALSE);
		}
		break;
	      }
	    }
	    PostMsg(hwnd,
		    UM_SETUP4,
		    MPVOID,
		    MPVOID);
	  }
	  DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      break;

    case IDM_CODEPAGE:
      {
	INT cp;

	cp = PickCodepage(hwnd);
	if (cp != -1)
	{
	  ad -> fattrs.usCodePage = (USHORT) cp;
	  Codepage = ad -> fattrs.usCodePage;
	  PrfWriteProfileData(fmprof,
			      appname,
			      "Viewer.Codepage",
			      &ad -> fattrs.usCodePage,
			      sizeof(USHORT));
	  GpiDeleteSetId(ad -> hps,
			 FIXED_FONT_LCID);
	  GpiAssociate(ad -> hps, 0);
	  GpiDestroyPS(ad -> hps);
	  ad -> hps = InitWindow(hwnd);
	  WinSendMsg(hwnd,
		     UM_SETUP3,
		     MPVOID,
		     MPVOID);
	  PostMsg(hwnd,
		  UM_SETUP4,
		  MPVOID,
		  MPVOID);
	  WinInvalidateRect(hwnd,
			    NULL,
			    FALSE);
	}
      }
      break;

    case IDM_SAVETOLIST2:
    case IDM_SAVETOCLIP2:
    case IDM_APPENDTOCLIP2:
    case IDM_SAVETOLIST:
    case IDM_SAVETOCLIP:
    case IDM_APPENDTOCLIP:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem,
				SEM_IMMEDIATE_RETURN);
	if (!rc)
	{
	  if (!ad -> busy)
	  {
	    ad -> cliptype = SHORT1FROMMP(mp1);
	    if (_beginthread(Clipboard,NULL,524288,(PVOID) hwnd) == -1)
              Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
	  }
	  DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      break;

    case IDM_SELECTALL:
    case IDM_DESELECTALL:
    case IDM_INVERT:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem,
				SEM_IMMEDIATE_RETURN);
	if (!rc)
	{
	  if (!ad -> busy && ad -> markedlines)
	  {

	    register ULONG x;

	    for (x = 0; x < ad -> numlines; x++)
	    {
	      switch (SHORT1FROMMP(mp1))
	      {
	      case IDM_SELECTALL:
		if (!(ad -> markedlines[x] & VF_SELECTED))
		{
		  ad -> markedlines[x] |= VF_SELECTED;
		  ad -> selected++;
		}
		break;
	      case IDM_DESELECTALL:
		if (ad -> markedlines[x] & VF_SELECTED)
		{
		  ad -> markedlines[x] &= (~VF_SELECTED);
		  ad -> selected--;
		}
		break;
	      case IDM_INVERT:
		if (ad -> markedlines[x] & VF_SELECTED)
		{
		  ad -> markedlines[x] &= (~VF_SELECTED);
		  ad -> selected--;
		}
		else
		{
		  ad -> markedlines[x] |= VF_SELECTED;
		  ad -> selected++;
		}
		break;
	      }
	    }
	    WinSendMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	    WinInvalidateRect(hwnd, NULL, FALSE);
	  }
	  DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      break;

    case IDM_WRAP:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	if (!rc)
	{
	  if (!ad -> busy)
	  {
	    ad -> wrapon = ad -> wrapon ? FALSE : TRUE;
	    WrapOn = ad -> wrapon;
	    PrfWriteProfileData(fmprof, appname, "Viewer.WrapOn",
				&ad -> wrapon, sizeof(BOOL));
	    PostMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);
	    PostMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
	    PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	    if (WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX,
				  LM_QUERYITEMCOUNT, MPVOID, MPVOID))
	      WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX, LM_DELETEALL,
				MPVOID, MPVOID);
	    ad -> oldwidth = -1;
	    WinSendMsg(ad -> hvscroll, SBM_SETTHUMBSIZE,
		       MPFROM2SHORT(1, 1),
		       MPVOID);
	    WinSendMsg(ad -> hvscroll, SBM_SETSCROLLBAR,
		       MPFROMSHORT(1), MPFROM2SHORT(1, 1));
	    WinSendMsg(ad -> hhscroll, SBM_SETTHUMBSIZE,
		       MPFROM2SHORT(1, 1),
		       MPVOID);
	    WinSendMsg(ad -> hhscroll, SBM_SETSCROLLBAR,
		       MPFROMSHORT(1), MPFROM2SHORT(1, 1));
	    WinSendMsg(ad -> hwndFrame, WM_UPDATEFRAME,
		       MPFROMLONG(FCF_SIZEBORDER), MPVOID);
	    WinInvalidateRect(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					      NEWVIEW_DRAG), NULL, FALSE);
	    WinInvalidateRect(ad -> hhscroll, NULL, FALSE);
	  }
	  DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      break;

    case IDM_HEXMODE:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	if (!rc)
	{
	  if (!ad -> busy)
	  {
	    ad -> hex = (ad -> hex) ? FALSE : TRUE;
	    WinEnableWindow(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					    IDM_NEXTBLANKLINE), !ad -> hex);
	    WinEnableWindow(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					    IDM_PREVBLANKLINE), !ad -> hex);
	    PostMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);
	    PostMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
	    PostMsg(hwnd, UM_SETUP4, MPVOID, MPVOID);
	    if (WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX,
				  LM_QUERYITEMCOUNT, MPVOID, MPVOID))
	      WinSendDlgItemMsg(ad -> hwndFrame, NEWVIEW_LISTBOX, LM_DELETEALL,
				MPVOID, MPVOID);
	    ad -> oldwidth = -1;
	    WinSendMsg(ad -> hvscroll, SBM_SETTHUMBSIZE,
		       MPFROM2SHORT(1, 1),
		       MPVOID);
	    WinSendMsg(ad -> hvscroll, SBM_SETSCROLLBAR,
		       MPFROMSHORT(1), MPFROM2SHORT(1, 1));
	    WinSendMsg(ad -> hhscroll, SBM_SETTHUMBSIZE,
		       MPFROM2SHORT(1, 1),
		       MPVOID);
	    WinSendMsg(ad -> hhscroll, SBM_SETSCROLLBAR,
		       MPFROMSHORT(1), MPFROM2SHORT(1, 1));
	    WinSendMsg(ad -> hwndFrame, WM_UPDATEFRAME,
		       MPFROMLONG(FCF_SIZEBORDER), MPVOID);
	    WinInvalidateRect(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					      NEWVIEW_DRAG), NULL, FALSE);
	    WinInvalidateRect(ad -> hhscroll, NULL, FALSE);
	  }
	  DosReleaseMutexSem(ad -> ScanSem);
	}
      }
      break;

    case IDM_FONTPALETTE:
      {
	APIRET rc;

	rc = DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN);
	if (!rc)
	{
	  SetMLEFont(hwnd, &ad -> fattrs, 11);
	  PrfWriteProfileData(fmprof, appname, "Viewer.Fattrs",
			      &ad -> fattrs, sizeof(FATTRS));
	  Fattrs = ad -> fattrs;
	  GpiDeleteSetId(ad -> hps, FIXED_FONT_LCID);
	  GpiAssociate(ad -> hps, 0);
	  GpiDestroyPS(ad -> hps);
	  ad -> hps = InitWindow(hwnd);
	  DosReleaseMutexSem(ad -> ScanSem);
	  WinSendMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);
	  WinInvalidateRect(hwnd, NULL, FALSE);
	}
      }
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_NEWVIEW, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_SETFOCUS:
    if (mp2)
      WinSendMsg(hwnd, UM_SETUP5, MPVOID, MPVOID);
    if (mp2 && ad && ad -> needrefreshing && !ad -> stopflag &&
	!DosRequestMutexSem(ad -> ScanSem, SEM_IMMEDIATE_RETURN))
    {
      ad -> needrefreshing = FALSE;
      DosReleaseMutexSem(ad -> ScanSem);
      WinInvalidateRect(hwnd, NULL, TRUE);
    }
    break;

  case WM_SIZE:
    if (SHORT1FROMMP(mp2) && SHORT2FROMMP(mp2))
    {
      PostMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);
      PostMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
    }
    break;

  case WM_SAVEAPPLICATION:
    if (ad && ParentIsDesktop(hwnd, ad -> hwndParent))
    {

      SWP swp;

      WinQueryWindowPos(ad -> hwndFrame, &swp);
      if (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE | SWP_MAXIMIZE)))
	PrfWriteProfileData(fmprof,
			    appname,
			    "NewViewSizePos",
			    &swp,
			    sizeof(swp));
    }
    break;

  case WM_CLOSE:
    if (ad)
      ad -> stopflag = 1;
    WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
    return 0;

  case WM_DESTROY:
    {
      BOOL dontclose = FALSE;
      HWND hwndRestore = (HWND) 0;

      WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, ID_TIMER5);
      if (ad)
      {
	ad -> stopflag = 1;
	if (ad -> ScanSem)
	{
	  DosRequestMutexSem(ad -> ScanSem, 15000L);
	  DosCloseMutexSem(ad -> ScanSem);
	}
	if (ad -> busy)
	  DosSleep(128L);
	if (ad -> hps)
	{
	  GpiDeleteSetId(ad -> hps, FIXED_FONT_LCID);
	  GpiAssociate(ad -> hps, 0);
	  GpiDestroyPS(ad -> hps);
	}
	hwndRestore = ad -> hwndRestore;
	dontclose = ((ad -> flags & 4) != 0) ? TRUE : FALSE;
	FreeViewerMem(hwnd);
	WinSetWindowPtr(hwnd, QWL_USER, NULL);
	free(ad);
      }
      if (hwndRestore && hwndRestore != HWND_DESKTOP)
      {

	ULONG fl = SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER;
	SWP swp;

	if (WinQueryWindowPos(hwndRestore, &swp))
	{
	  if (!(swp.fl & SWP_MAXIMIZE))
	    fl |= SWP_RESTORE;
	  WinSetWindowPos(hwndRestore, HWND_TOP, 0, 0, 0, 0, fl);
	}
      }
      if (!dontclose &&
	  ParentIsDesktop(hwnd, WinQueryWindow(WinQueryWindow(hwnd,
						    QW_PARENT), QW_PARENT)))
      {
	if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
	  DosExit(EXIT_PROCESS, 1);
      }
    }
    break;
  }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

HWND StartViewer(HWND hwndParent, USHORT flags, CHAR * filename,
		 HWND hwndRestore)
{
  HWND hwndFrame = (HWND) 0, hwndClient;
  VIEWDATA *ad;
  ULONG FrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
  FCF_SIZEBORDER | FCF_MINMAX |
  FCF_NOBYTEALIGN | FCF_VERTSCROLL |
  FCF_MENU | FCF_ICON |
  FCF_ACCELTABLE | FCF_HORZSCROLL;

  if (strcmp(realappname, FM3Str))
    hwndParent = HWND_DESKTOP;
  if (ParentIsDesktop(hwndParent, hwndParent))
    FrameFlags |= FCF_TASKLIST;
// saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"\"%s\"\r\rparent %s desktop",filename,(ParentIsDesktop(hwndParent,hwndParent)) ? "is" : "isn't");
  hwndFrame = WinCreateStdWindow(hwndParent,
				 0,
				 &FrameFlags,
				 GetPString(IDS_WCNEWVIEW),
				 GetPString(IDS_FM2VIEWERTITLETEXT),
				 fwsAnimate,
				 FM3ModHandle,
				 NEWVIEW_FRAME,
				 &hwndClient);
  if (hwndFrame)
  {

    HWND hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);

    if (!fToolbar && hwndMenu)
    {
      WinSendMsg(hwndMenu, MM_DELETEITEM,
		 MPFROM2SHORT(IDM_FINDFIRST, FALSE), MPVOID);
      WinSendMsg(hwndMenu, MM_DELETEITEM,
		 MPFROM2SHORT(IDM_FINDNEXT, FALSE), MPVOID);
      WinSendMsg(hwndMenu, MM_DELETEITEM,
		 MPFROM2SHORT(IDM_FINDPREV, FALSE), MPVOID);
      WinSendMsg(hwndMenu, MM_DELETEITEM,
		 MPFROM2SHORT(IDM_SAVETOCLIP, FALSE), MPVOID);
    }
    ad = xmallocz(sizeof(VIEWDATA),pszSrcFile,__LINE__);
    if (!ad) {
      WinDestroyWindow(hwndFrame);
      hwndFrame = (HWND) 0;
    }
    else {
      ad -> size = sizeof(VIEWDATA);
      ad -> stopflag = 0;
      ad -> multiplier = 1;
      ad -> hwndRestore = hwndRestore;
      ad -> hwndFrame = hwndFrame;
      ad -> hwndParent = hwndParent;
      ad -> clientfocused = TRUE;
      ad -> oldwidth = -1;
      strcpy(ad -> filename, filename);
      ad -> flags = flags;
      if (ad -> flags & 16)
	ad -> hex = TRUE;
      WinSetWindowPtr(hwndClient, QWL_USER, (PVOID) ad);
      if (Firsttime)
      {

	ULONG size;

	size = sizeof(BOOL);
	PrfQueryProfileData(fmprof, appname, "Viewer.Sensitive",
			    (PVOID) & Sensitive, &size);
	size = sizeof(USHORT);
	PrfQueryProfileData(fmprof, appname, "Viewer.Codepage",
			    (PVOID) & Codepage, &size);
	size = sizeof(BOOL);
	PrfQueryProfileData(fmprof, appname, "Viewer.LiteralSearch",
			    (PVOID) & LiteralSearch, &size);
	size = sizeof(BOOL);
	PrfQueryProfileData(fmprof, appname, "Viewer.AlsoSelect",
			    (PVOID) & AlsoSelect, &size);
	size = sizeof(BOOL);
	PrfQueryProfileData(fmprof, appname, "Viewer.WrapOn",
			    (PVOID) & WrapOn, &size);
	size = sizeof(BOOL);
	PrfQueryProfileData(fmprof, appname, "Viewer.IgnoreFTP",
			    (PVOID) & IgnoreFTP, &size);
	size = sizeof(BOOL);
	PrfQueryProfileData(fmprof, appname, "Viewer.IgnoreHTTP",
			    (PVOID) & IgnoreHTTP, &size);
	memset(&Fattrs, 0, sizeof(FATTRS));
	size = sizeof(FATTRS);
	Fattrs.usRecordLength = sizeof(FATTRS);
	Fattrs.lMaxBaselineExt = 16;
	Fattrs.lAveCharWidth = 8;
	Fattrs.usCodePage = Codepage;
	strcpy(Fattrs.szFacename, GetPString(IDS_SYSMONOTEXT));
	PrfQueryProfileData(fmprof, appname, "Viewer.Fattrs",
			    (PVOID) & Fattrs, &size);
	size = sizeof(LONG) * COLORS_MAX;
	PrfQueryProfileData(fmprof, appname, "Viewer.Colors",
			    (PVOID) Colors, &size);
	Firsttime = FALSE;
      }
      {
	ULONG size = sizeof(ad -> searchtext);

	PrfQueryProfileData(fmprof, appname, "Viewer.Searchtext",
			    (PVOID) ad -> searchtext, &size);
	ad -> searchtext[sizeof(ad -> searchtext) - 1] = 0;
      }
      ad -> sensitive = Sensitive;
      ad -> literalsearch = LiteralSearch;
      ad -> fattrs = Fattrs;
      ad -> alsoselect = AlsoSelect;
      ad -> fattrs.usCodePage = Codepage;
      ad -> wrapon = WrapOn;
      ad -> ignorehttp = IgnoreHTTP;
      ad -> ignoreftp = IgnoreFTP;
      memcpy(ad -> colors, Colors, sizeof(LONG) * COLORS_MAX);
      WinSetWindowPtr(hwndClient, QWL_USER, (PVOID) ad);
      if (!WinSendMsg(hwndClient, UM_SETUP, MPVOID, MPVOID))
	hwndFrame = (HWND)0;
      else {
        // DosSleep(64L);
	if (!(FrameFlags & FCF_TASKLIST) && !(flags & 2))
	{
	  SWP swp;
	  FillClient(hwndParent, &swp, NULL, FALSE);
	  WinSetWindowPos(hwndFrame, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
			  SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_RESTORE |
			  SWP_ZORDER | SWP_ACTIVATE);
	}
	else if (FrameFlags & FCF_TASKLIST)
	{

	  SWP swp, swpD;
	  ULONG size = sizeof(swp);
	  LONG cxScreen, cyScreen;

	  WinQueryTaskSizePos(WinQueryAnchorBlock(hwndFrame), 0, &swp);
	  if (PrfQueryProfileData(fmprof,
				  appname,
				  "NewViewSizePos",
				  &swpD,
				  &size))
	  {
	    cxScreen = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
	    cyScreen = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
	    if (swp.x + swpD.cx > cxScreen)
	      swp.x = cxScreen - swpD.cx;
	    if (swp.y + swpD.cy > cyScreen)
	      swp.y = cyScreen - swpD.cy;
	    swp.cx = swpD.cx;
	    swp.cy = swpD.cy;
	  }
	  WinSetWindowPos(hwndFrame, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
			  SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ZORDER |
			  SWP_ACTIVATE);
	}
      }
    }
  }
  return hwndFrame;
}
