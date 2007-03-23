
/***********************************************************************

  $Id$

  Print file list

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2006 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  17 Jul 06 SHL Use Runtime_Error
  03 Nov 06 SHL Renames
  03 Nov 06 SHL Count thread usage
  22 Mar 07 GKY Use QWL_USER

***********************************************************************/

#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <io.h>
#include <share.h>
#include <string.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(PRINTER,PrinterReady,SayPrinterReady)
#pragma alloc_text(PRINTER2,PrintListThread)
#pragma alloc_text(PRINTER3,PrintDlgProc)

static HMTX PrintSem = 0;

BOOL PrinterReady(CHAR * printdevname)
{
  FILE *printhandle;
  CHAR param = 0, data = 0;
  ULONG datalen = 1L, parmlen = 1L, htype, flagword;

  if (!fWorkPlace)			/* assume spooler is active */
    return TRUE;
  DosError(FERR_DISABLEHARDERR);
  printhandle = xfopen(printdevname, "a+", pszSrcFile, __LINE__);
  if (printhandle) {
    if (!strnicmp(printdevname, "COM", 3) && isdigit(printdevname[3])) {
      fclose(printhandle);
      return TRUE;
    }
    if (!DosQueryHType(fileno(printhandle), &htype, &flagword) &&
	!(htype & 7)) {
      fclose(printhandle);
      return TRUE;
    }
    if (DosDevIOCtl(fileno(printhandle), 5L, 0x00000066, (PVOID) & param,
		    1L, &parmlen, (PVOID) & data, 1L,
		    &datalen) != 0x00000100) {
      fclose(printhandle);
      return FALSE;
    }
    fclose(printhandle);
    if (data & 32)			/* bit 5 is out of paper */
      return FALSE;
    return TRUE;
  }
  return FALSE;
}

BOOL SayPrinterReady(HWND hwnd)
{
  if (!hwnd)
    hwnd = HWND_DESKTOP;
  if (!PrinterReady(printer)) {
    saymsg(MB_ENTER | MB_ICONEXCLAMATION,
	   hwnd,
	   GetPString(IDS_NOTETEXT), GetPString(IDS_PRINTERNOTREADYTEXT));
    return FALSE;
  }
  return TRUE;
}

//=== PrintListThread - background-print a list of files ===

VOID PrintListThread(VOID * arg)
{
  HAB hab2;
  HMQ hmq2;
  LISTINFO *li = (LISTINFO *) arg;
  register INT x;
  FILE *fpi, *fpo;
  CHAR s[CCHMAXPATH + 80];
  FILESTATUS3 fs3;
  LONG cols, lines, pages, z, lmargin, rmargin;
  BOOL endline, endpage, startpage, skipping, firstpass;
  int c;
  APIRET rc = MBID_YES;

  if (StopPrinting)
    goto Abort;
  if (!PrintSem) {
    if (DosCreateMutexSem(NULL, &PrintSem, 0, FALSE)) {
      Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		"DosCreateMutexSem");
      goto Abort;
    }
  }
  priority_normal();
  hab2 = WinInitialize(0);
  if (hab2) {
    hmq2 = WinCreateMsgQueue(hab2, 0);
    if (hmq2) {
      WinCancelShutdown(hmq2, TRUE);
      IncrThreadUsage();
      if (li && li->list && li->list[0]) {
	AddNote(GetPString(IDS_PRINTINGLISTTEXT));
	for (x = 0; li->list[x]; x++) {
	  if (rc == MBID_CANCEL)
	    break;
	  DosError(FERR_DISABLEHARDERR);
	  if (DosQueryPathInfo
	      (li->list[x], FIL_STANDARD, &fs3, (ULONG) sizeof(fs3))
	      || (fs3.attrFile & FILE_DIRECTORY) || !fs3.cbFile)
	    continue;
	  if (StopPrinting)
	    break;
	  DosRequestMutexSem(PrintSem, 240000L);
	  if (StopPrinting)
	    break;
	  if (!PrinterReady(li->targetpath))
	    Runtime_Error(pszSrcFile, __LINE__, "printer %s error",
			  li->targetpath);
	  else {
	    fpi = _fsopen(li->list[x], "r", SH_DENYWR);
	    if (!fpi)
	      Runtime_Error(pszSrcFile, __LINE__, "cannot open %s",
			    li->list[x]);
	    else {
	      fpo = _fsopen(li->targetpath, "a+", SH_DENYRW);
	      if (!fpo)
		Runtime_Error(pszSrcFile, __LINE__, "cannot open %s",
			      li->targetpath);
	      else {
		sprintf(s, GetPString(IDS_PRINTINGTEXT), li->list[x]);
		AddNote(s);
		firstpass = TRUE;
		skipping = FALSE;
		lmargin = prnlmargin;
		rmargin = prnrmargin;
		if (prnformfeedbefore) {
		  if (!skipping)
		    fputc('\x0c', fpo);
		}
	      Again:
		cols = lines = pages = 0;
		endline = endpage = FALSE;
		startpage = TRUE;
		while ((c = fgetc(fpi)) != EOF) {
		  if (StopPrinting || ferror(fpo))
		    break;
		  if (prnformat) {
		    if (startpage) {
		      pages++;
		      if (prnpagenums) {
			if (!skipping)
			  fprintf(fpo, "%*.*s#%lu",
				  lmargin, lmargin, " ", pages);
		      }
		      for (z = 0; z < prntmargin; z++) {
			if (!skipping)
			  fputc('\n', fpo);
		      }
		      lines = prntmargin;
		      endline = endpage = FALSE;
		      cols = 0;
		      startpage = FALSE;
		    }
		    if (!cols) {
		      for (z = 0; z < lmargin; z++) {
			if (!skipping)
			  fputc(' ', fpo);
		      }
		      cols = lmargin;
		    }
		    if (!skipping) {
		      if (c == '\t') {
			for (z = 0; z < prntabspaces &&
			     cols + z < prnwidth - rmargin; z++)
			  fputc(' ', fpo);
		      }
		      else
			fputc(c, fpo);
		    }
		    if (c == '\x0c')
		      endpage = TRUE;
		    else if (c == '\n') {
		      lines++;
		      cols = 0;
		      for (z = 1; z < prnspacing; z++) {
			if (prnlength - lines <= prnbmargin)
			  break;
			if (!skipping)
			  fputc('\n', fpo);
			lines++;
		      }
		      if (prnlength - lines <= prnbmargin)
			endpage = TRUE;
		    }
		    else {
		      cols++;
		      if (c == '\t')
			c += prntabspaces;
		      if (prnwidth - cols <= rmargin) {
			endline = TRUE;
			if (!skipping)
			  fputc('\n', fpo);
			lines++;
		      }
		      for (z = 1; z < prnspacing; z++) {
			if (prnlength - lines <= prnbmargin)
			  break;
			if (!skipping)
			  fputc('\n', fpo);
			lines++;
		      }
		    }
		    if (endline) {
		      do {
			c = fgetc(fpi);
			if (c == '\n')
			  break;
		      } while (isspace(c) && c != EOF);
		      if (c == EOF)
			break;
		      if (!isspace(c))
			ungetc(c, fpi);
		      endline = FALSE;
		      cols = 0;
		      if (prnlength - lines <= prnbmargin)
			endpage = TRUE;
		    }
		    if (endpage) {
		      if (prnbmargin) {
			if (c != '\x0c') {
			  if (!skipping)
			    fputc('\r\x0c', fpo);
			}
		      }
		      lines = cols = 0;
		      endpage = FALSE;
		      startpage = TRUE;
		      do {
			c = fgetc(fpi);
		      } while (c == '\n');
		      if (c == EOF)
			break;
		      ungetc(c, fpi);
		      if (prnformat && prnalt)
			skipping = (skipping) ? FALSE : TRUE;
		      DosSleep(1L);
		    }
		  }
		  else
		    fputc(c, fpo);
		}
		if (prnalt && prnformat && firstpass && pages > 1) {
		  fclose(fpo);
		  fpo = _fsopen(li->targetpath, "a+", SH_DENYRW);
		  if (fpo) {
		    rewind(fpi);
		    firstpass = FALSE;
		    skipping = TRUE;
		    rc = saymsg(MB_YESNOCANCEL | MB_ICONEXCLAMATION,
				HWND_DESKTOP,
				GetPString(IDS_PRINTTITLETEXT),
				GetPString(IDS_PRINTEVENTEXT), li->list[x]);
		    if (rc == MBID_YES) {
		      lmargin = prnrmargin;
		      rmargin = prnlmargin;
		      fputc('\x0c', fpo);
		      goto Again;
		    }
		  }
		  else
		    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
			   HWND_DESKTOP,
			   GetPString(IDS_PRINTTITLETEXT),
			   GetPString(IDS_PRINTCANTREOPENTEXT),
			   li->targetpath);
		}
		if (prnformfeedafter)
		  fputc('\x0c', fpo);
		if (!StopPrinting) {
		  sprintf(s, GetPString(IDS_PRINTEDTEXT), li->list[x]);
		  AddNote(s);
		}
		fclose(fpo);
	      }
	      fclose(fpi);
	    }
	  }
	  DosReleaseMutexSem(PrintSem);
	  DosSleep(1L);
	}
	if (!StopPrinting)
	  AddNote(GetPString(IDS_PRINTEDLISTTEXT));
      }
      WinDestroyMsgQueue(hmq2);
    }
    DecrThreadUsage();
    WinTerminate(hab2);
  }
Abort:
  if (li)
    FreeListInfo(li);
}

MRESULT EXPENTRY PrintDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  LISTINFO *li;

  switch (msg) {
  case WM_INITDLG:
    if (!mp2)
      WinDismissDlg(hwnd, 0);
    else {
      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      WinSendDlgItemMsg(hwnd, PRN_WIDTH, EM_SETTEXTLIMIT,
			MPFROM2SHORT(3, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, PRN_LENGTH, EM_SETTEXTLIMIT,
			MPFROM2SHORT(3, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, PRN_LMARGIN, EM_SETTEXTLIMIT,
			MPFROM2SHORT(2, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, PRN_RMARGIN, EM_SETTEXTLIMIT,
			MPFROM2SHORT(2, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, PRN_TMARGIN, EM_SETTEXTLIMIT,
			MPFROM2SHORT(2, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, PRN_BMARGIN, EM_SETTEXTLIMIT,
			MPFROM2SHORT(2, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, PRN_SPACING, EM_SETTEXTLIMIT,
			MPFROM2SHORT(1, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, PRN_TABSPACES, EM_SETTEXTLIMIT,
			MPFROM2SHORT(2, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, PRN_PRINTER, EM_SETTEXTLIMIT,
			MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      PosOverOkay(hwnd);
    }
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, PRN_FORMAT, prnformat);
    WinCheckButton(hwnd, PRN_ALT, prnalt);
    WinCheckButton(hwnd, PRN_PAGENUMS, prnpagenums);
    WinCheckButton(hwnd, PRN_FORMBEFORE, prnformfeedbefore);
    WinCheckButton(hwnd, PRN_FORMAFTER, prnformfeedafter);
    {
      CHAR s[33];

      sprintf(s, "%lu", prnwidth);
      WinSetDlgItemText(hwnd, PRN_WIDTH, s);
      sprintf(s, "%lu", prnlength);
      WinSetDlgItemText(hwnd, PRN_LENGTH, s);
      sprintf(s, "%lu", prnlmargin);
      WinSetDlgItemText(hwnd, PRN_LMARGIN, s);
      sprintf(s, "%lu", prnrmargin);
      WinSetDlgItemText(hwnd, PRN_RMARGIN, s);
      sprintf(s, "%lu", prntmargin);
      WinSetDlgItemText(hwnd, PRN_TMARGIN, s);
      sprintf(s, "%lu", prnbmargin);
      WinSetDlgItemText(hwnd, PRN_BMARGIN, s);
      sprintf(s, "%lu", prnspacing);
      WinSetDlgItemText(hwnd, PRN_SPACING, s);
      sprintf(s, "%lu", prntabspaces);
      WinSetDlgItemText(hwnd, PRN_TABSPACES, s);
    }
    WinSetDlgItemText(hwnd, PRN_PRINTER, printer);
    WinSendDlgItemMsg(hwnd, PRN_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
    li = WinQueryWindowPtr(hwnd, QWL_USER);
    if (!li)
      Runtime_Error(pszSrcFile, __LINE__, "no data");
    else {
      INT x;

      for (x = 0; li->list[x]; x++) {
	if (IsFile(li->list[x]) == 1) {
	  WinSendDlgItemMsg(hwnd, PRN_LISTBOX, LM_INSERTITEM,
			    MPFROM2SHORT(LIT_END, 0), MPFROMP(li->list[x]));
	  WinSendDlgItemMsg(hwnd, PRN_LISTBOX, LM_SELECTITEM,
			    MPFROM2SHORT(x, 0), MPFROMSHORT(TRUE));
	}
      }
      if (!(SHORT) WinSendDlgItemMsg(hwnd, PRN_LISTBOX, LM_QUERYITEMCOUNT,
				     MPVOID, MPVOID)) {
	saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
	       hwnd,
	       GetPString(IDS_TWIDDLETEXT), GetPString(IDS_NOPRINTABLETEXT));
	WinDismissDlg(hwnd, 0);
      }
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case PRN_LISTBOX:
      if (SHORT2FROMMP(mp1) == LN_ENTER) {

	SHORT x;
	CHAR szBuffer[CCHMAXPATH];

	x = (SHORT) WinSendDlgItemMsg(hwnd, PRN_LISTBOX, LM_QUERYSELECTION,
				      MPFROMSHORT(LIT_FIRST), MPVOID);
	if (x >= 0) {
	  *szBuffer = 0;
	  WinSendDlgItemMsg(hwnd, PRN_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(x, CCHMAXPATH), MPFROMP(szBuffer));
	  if (*szBuffer)
	    QuickView(hwnd, szBuffer);
	}
      }
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      li = WinQueryWindowPtr(hwnd, QWL_USER);
      if (!li)
	Runtime_Error(pszSrcFile, __LINE__, "no data");
      else {
	prnformat = WinQueryButtonCheckstate(hwnd, PRN_FORMAT);
	PrfWriteProfileData(fmprof, FM3Str, "Prnformat",
			    (PVOID) & prnformat, sizeof(prnformat));
	prnalt = WinQueryButtonCheckstate(hwnd, PRN_ALT);
	PrfWriteProfileData(fmprof, FM3Str, "Prnalt",
			    (PVOID) & prnalt, sizeof(prnalt));
	prnpagenums = WinQueryButtonCheckstate(hwnd, PRN_PAGENUMS);
	PrfWriteProfileData(fmprof, FM3Str, "Prnpagenums",
			    (PVOID) & prnpagenums, sizeof(prnpagenums));
	prnformfeedbefore = WinQueryButtonCheckstate(hwnd, PRN_FORMBEFORE);
	PrfWriteProfileData(fmprof, FM3Str, "Prnformfeedbefore",
			    (PVOID) & prnformfeedbefore,
			    sizeof(prnformfeedbefore));
	prnformfeedafter = WinQueryButtonCheckstate(hwnd, PRN_FORMAFTER);
	PrfWriteProfileData(fmprof, FM3Str, "Prnformfeedafter",
			    (PVOID) & prnformfeedafter,
			    sizeof(prnformfeedafter));
	{
	  CHAR s[33];

	  *s = 0;
	  WinQueryDlgItemText(hwnd, PRN_WIDTH, sizeof(s), s);
	  if (atol(s) < 24 || atol(s) > 4096) {
	    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, PRN_WIDTH));
	    DosBeep(50, 100);
	    break;
	  }
	  prnwidth = atol(s);
	  PrfWriteProfileData(fmprof, FM3Str, "Prnwidth",
			      (PVOID) & prnwidth, sizeof(prnwidth));
	  *s = 0;
	  WinQueryDlgItemText(hwnd, PRN_LENGTH, sizeof(s), s);
	  if (atol(s) < 24 || atol(s) > 4096) {
	    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, PRN_LENGTH));
	    DosBeep(50, 100);
	    break;
	  }
	  prnlength = atol(s);
	  PrfWriteProfileData(fmprof, FM3Str, "Prnlength",
			      (PVOID) & prnlength, sizeof(prnlength));
	  *s = 0;
	  WinQueryDlgItemText(hwnd, PRN_LMARGIN, sizeof(s), s);
	  if (atol(s) < 0 || atol(s) > prnwidth - 1) {
	    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, PRN_LMARGIN));
	    DosBeep(50, 100);
	    break;
	  }
	  prnlmargin = atol(s);
	  PrfWriteProfileData(fmprof, FM3Str, "Prnlmargin",
			      (PVOID) & prnlmargin, sizeof(prnlmargin));
	  *s = 0;
	  WinQueryDlgItemText(hwnd, PRN_RMARGIN, sizeof(s), s);
	  if (atol(s) < 0 || atol(s) > (prnwidth - prnlmargin) - 1) {
	    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, PRN_RMARGIN));
	    DosBeep(50, 100);
	    break;
	  }
	  prnrmargin = atol(s);
	  PrfWriteProfileData(fmprof, FM3Str, "Prnrmargin",
			      (PVOID) & prnrmargin, sizeof(prnrmargin));
	  *s = 0;
	  WinQueryDlgItemText(hwnd, PRN_TABSPACES, sizeof(s), s);
	  if (atol(s) < 1 ||
	      atol(s) > prnwidth - ((prnlmargin + prnrmargin) - 1)) {
	    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, PRN_TABSPACES));
	    DosBeep(50, 100);
	    break;
	  }
	  prntabspaces = atol(s);
	  PrfWriteProfileData(fmprof, FM3Str, "Prntabspaces",
			      (PVOID) & prntabspaces, sizeof(prntabspaces));
	  *s = 0;
	  WinQueryDlgItemText(hwnd, PRN_TMARGIN, sizeof(s), s);
	  if (atol(s) < 0 || atol(s) > prnlength - 1) {
	    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, PRN_TMARGIN));
	    DosBeep(50, 100);
	    break;
	  }
	  prntmargin = atol(s);
	  PrfWriteProfileData(fmprof, FM3Str, "Prntmargin",
			      (PVOID) & prntmargin, sizeof(prntmargin));
	  *s = 0;
	  WinQueryDlgItemText(hwnd, PRN_BMARGIN, sizeof(s), s);
	  if (atol(s) < 0 || atol(s) > (prnlength - prntmargin) - 1) {
	    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, PRN_BMARGIN));
	    DosBeep(50, 100);
	    break;
	  }
	  prnbmargin = atol(s);
	  PrfWriteProfileData(fmprof, FM3Str, "Prnbmargin",
			      (PVOID) & prnbmargin, sizeof(prnbmargin));
	  *s = 0;
	  WinQueryDlgItemText(hwnd, PRN_SPACING, sizeof(s), s);
	  if (atol(s) < 1 ||
	      atol(s) > ((prnlength - prntmargin) - prnbmargin) - 1) {
	    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, PRN_SPACING));
	    DosBeep(50, 100);
	    break;
	  }
	  prnspacing = atol(s);
	  PrfWriteProfileData(fmprof, FM3Str, "Prnspacing",
			      (PVOID) & prnspacing, sizeof(prnspacing));
	  WinQueryDlgItemText(hwnd, PRN_PRINTER, sizeof(printer), printer);
	  bstrip(printer);
	  if (!*printer)
	    strcpy(printer, "PRN");
	  WinSetDlgItemText(hwnd, PRN_PRINTER, printer);
	  PrfWriteProfileString(fmprof, appname, "Printer", printer);
	  SayPrinterReady(hwnd);
	}
	{
	  CHAR szBuffer[CCHMAXPATH + 1];
	  INT numfiles = 0, numalloc = 0, error;
	  SHORT x;

	  if (li->list)
	    FreeList(li->list);
	  li->list = NULL;
	  x = (SHORT) WinSendDlgItemMsg(hwnd, PRN_LISTBOX, LM_QUERYSELECTION,
					MPFROMSHORT(LIT_FIRST), MPVOID);
	  if (x >= 0) {
	    do {
	      *szBuffer = 0;
	      WinSendDlgItemMsg(hwnd, PRN_LISTBOX, LM_QUERYITEMTEXT,
				MPFROM2SHORT(x, CCHMAXPATH),
				MPFROMP(szBuffer));
	      error = AddToList(szBuffer, &li->list, &numfiles, &numalloc);
	      if (error) {
		Runtime_Error(pszSrcFile, __LINE__, "AddToList");
		break;
	      }
	      x = (SHORT) WinSendDlgItemMsg(hwnd, PRN_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(x), MPVOID);
	    } while (x >= 0);
	  }
	}
	if (!li->list || !li->list[0])
	  WinDismissDlg(hwnd, 0);
	else
	  WinDismissDlg(hwnd, 1);
      }
      break;
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_PRINT, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
