
/***********************************************************************

  $Id$

  Save file list to clipboard

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2007 Steven H. Levine

  12 Feb 03 SHL SaveListDlgProc: standardize EA math
  01 Aug 04 SHL Rework lstrip/rstrip usage
  01 Aug 04 SHL Rework fixup usage
  24 May 05 SHL Rework for CNRITEM.szSubject
  17 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets
  22 Mar 07 GKY Use QWL_USER

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(FMCLIPBOARDIN,SaveToClip,SaveToClipHab)
#pragma alloc_text(FMCLIPBOARDOUT,ListToClipboard,ListToClipboardHab)
#pragma alloc_text(FMCLIPBOARDOUT,ListFromClipboard,ListFromClipboardHab)

BOOL SaveToClip(HWND hwnd, CHAR * text, BOOL append)
{
  HAB hab = WinQueryAnchorBlock(hwnd);

  return SaveToClipHab(hab, text, append);
}

BOOL SaveToClipHab(HAB hab, CHAR * text, BOOL append)
{
  CHAR *clip = NULL, *hold = NULL, *p;
  ULONG len;
  BOOL ret = FALSE;

  if (text) {
    len = strlen(text);
    p = text;
    while (*p) {
      if (*p == '\n' && (p == text || *(p - 1) != '\r'))
	len++;
      p++;
    }
    if (len) {
      if (WinOpenClipbrd(hab)) {
	if (append)
	  clip = (CHAR *) WinQueryClipbrdData(hab, CF_TEXT);
	if (clip)
	  len += strlen(clip) + 1L;
	if (!DosAllocSharedMem((PPVOID) & hold, (PSZ) NULL, len, PAG_COMMIT |
			       OBJ_GIVEABLE | PAG_READ | PAG_WRITE)) {
	  *hold = 0;
	  if (clip)
	    strcpy(hold, clip);
	  p = hold + strlen(hold);
	  strcpy(p, text);
	  while (*p) {
	    if (*p == '\n' && (p == hold || *(p - 1) != '\r')) {
	      memmove(p + 1, p, strlen(p) + 1);
	      *p = '\r';
	    }
	    p++;
	  }
	  WinEmptyClipbrd(hab);
	  if (!WinSetClipbrdData(hab, (ULONG) hold, CF_TEXT, CFI_POINTER))
	    DosFreeMem(hold);
	  else
	    ret = TRUE;
	}
	WinCloseClipbrd(hab);
      }
    }
  }
  return ret;
}

VOID ListToClipboard(HWND hwnd, CHAR ** list, BOOL append)
{
  HAB hab = WinQueryAnchorBlock(hwnd);

  ListToClipboardHab(hab, list, append);
}

VOID ListToClipboardHab(HAB hab, CHAR ** list, BOOL append)
{
  CHAR *text = NULL, **clip = NULL;
  INT x;
  ULONG len = 0L;

  if (list && list[0]) {
    for (x = 0; list[x]; x++)
      len += strlen(list[x]) + 2;
    if (len)
      len++;
    if (len) {
      if (append)
	clip = ListFromClipboardHab(hab);
      if (clip && clip[0]) {
	for (x = 0; clip[x]; x++)
	  len += strlen(clip[x]) + 2;
	len++;
      }
      if (WinOpenClipbrd(hab)) {
	if (!DosAllocSharedMem((PPVOID) & text, (PSZ) NULL, len, PAG_COMMIT |
			       OBJ_GIVEABLE | PAG_READ | PAG_WRITE)) {
	  *text = 0;
	  if (clip && clip[0]) {
	    for (x = 0; clip[x]; x++) {
	      strcat(text, clip[x]);
	      strcat(text, "\r\n");
	    }
	  }
	  for (x = 0; list[x]; x++) {
	    strcat(text, list[x]);
	    strcat(text, "\r\n");
	  }
	  WinEmptyClipbrd(hab);
	  if (!WinSetClipbrdData(hab, (ULONG) text, CF_TEXT, CFI_POINTER))
	    DosFreeMem(text);
	}
	WinCloseClipbrd(hab);
      }
      if (clip)
	FreeList(clip);
    }
  }
}

CHAR **ListFromClipboard(HWND hwnd)
{
  HAB hab = WinQueryAnchorBlock(hwnd);

  return ListFromClipboardHab(hab);
}

CHAR **ListFromClipboardHab(HAB hab)
{
  CHAR *p, *pp, *text = NULL, **list = NULL;
  INT numfiles = 0, numalloced = 0;

  if (WinOpenClipbrd(hab)) {
    p = (CHAR *) WinQueryClipbrdData(hab, CF_TEXT);
    if (p && *p)
      text = xstrdup(p, pszSrcFile, __LINE__);
    WinCloseClipbrd(hab);
    if (text) {
      bstrip(text);
      pp = text;
      p = strchr(pp, '\r');
      if (!p)
	p = strchr(pp, '\n');
      while (p && *p) {
	*p = 0;
	p++;
	while (*p == '\r' || *p == '\n' || *p == ' ' || *p == '\t')
	  p++;
	rstrip(pp);
	if (*pp) {
	  if (AddToList(pp, &list, &numfiles, &numalloced))
	    break;
	}
	pp = p;
	p = strchr(pp, '\r');
	if (!p)
	  p = strchr(pp, '\n');
      }
      free(text);
    }
  }
  return list;
}

#pragma alloc_text(SAVELIST,SaveListDlgProc,SaveAllListDlgProc)

MRESULT EXPENTRY SaveListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  HWND hwndCnr;
  CHAR savename[CCHMAXPATH] = "", pattern[81];

  switch (msg) {
  case WM_INITDLG:
    if (!mp2) {
      Runtime_Error(pszSrcFile, __LINE__, "no data");
      WinDismissDlg(hwnd, 0);
    }
    else {
      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      hwndCnr = *(HWND *) mp2;
      WinSendDlgItemMsg(hwnd,
			SAV_FILENAME,
			EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
      WinSendDlgItemMsg(hwnd,
			SAV_PATTERN,
			EM_SETTEXTLIMIT, MPFROM2SHORT(80, 0), MPVOID);
      *savename = *pattern = 0;
      {
	ULONG size;

	size = CCHMAXPATH;
	PrfQueryProfileData(fmprof,
			    appname, "SaveToListName", savename, &size);
	size = 81;
	PrfQueryProfileData(fmprof,
			    appname, "SaveToListPattern", pattern, &size);
      }
      WinSetDlgItemText(hwnd, SAV_FILENAME, savename);
      if (!*pattern)
	strcpy(pattern, "%F  %s");
      {
	CHAR temp[162];

	fixup(pattern, temp, sizeof(temp), strlen(pattern));
	WinSetDlgItemText(hwnd, SAV_PATTERN, temp);
      }
      {
	FILE *fp;
	CHAR s[CCHMAXPATH + 14];

	save_dir2(s);
	if (s[strlen(s) - 1] != '\\')
	  strcat(s, "\\");
	strcat(s, "PATTERNS.DAT");
	fp = _fsopen(s, "r", SH_DENYWR);
	if (fp) {
	  while (xfgets(s, 81, fp, pszSrcFile, __LINE__)) {
	    stripcr(s);
	    if (*s && *s != ';')
	      WinSendMsg(WinWindowFromID(hwnd, SAV_LISTBOX), LM_INSERTITEM,
			 MPFROM2SHORT(LIT_SORTASCENDING, 0), MPFROMP(s));
	  }
	  fclose(fp);
	}
	if (!WinSendDlgItemMsg(hwnd, SAV_LISTBOX, LM_QUERYITEMCOUNT,
			       MPVOID, MPVOID))
	  WinEnableWindow(WinWindowFromID(hwnd, SAV_LISTBOX), FALSE);
      }
    }
    break;

  case UM_SETDIR:
    {
      SHORT sSelect, sMax;
      CHAR szBuffer[CCHMAXPATH + 14];
      FILE *fp;

      sMax = (SHORT) WinSendDlgItemMsg(hwnd, SAV_LISTBOX,
				       LM_QUERYITEMCOUNT, MPVOID, MPVOID);
      if (sMax > 0) {
	save_dir2(szBuffer);
	if (szBuffer[strlen(szBuffer) - 1] != '\\')
	  strcat(szBuffer, "\\");
	strcat(szBuffer, "PATTERNS.DAT");
	fp = xfopen(szBuffer, "w", pszSrcFile, __LINE__);
	if (fp) {
	  fputs(GetPString(IDS_LISTPATTERNTEXT), fp);
	  for (sSelect = 0; sSelect < sMax; sSelect++) {
	    *szBuffer = 0;
	    WinSendDlgItemMsg(hwnd,
			      SAV_LISTBOX,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect, 81), MPFROMP(szBuffer));
	    if (*szBuffer)
	      fprintf(fp, "%s\n", szBuffer);
	  }
	  fclose(fp);
	}
      }
      else if (!sMax) {
	save_dir2(szBuffer);
	if (szBuffer[strlen(szBuffer) - 1] != '\\')
	  strcat(szBuffer, "\\");
	strcat(szBuffer, "PATTERNS.DAT");
	unlinkf("%s", szBuffer);
      }
    }
    return 0;

  case WM_CONTROL:
    if (SHORT1FROMMP(mp1) == SAV_LISTBOX) {

      SHORT sSelect;
      CHAR szBuffer[81];

      switch (SHORT2FROMMP(mp1)) {
      case LN_SELECT:
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, SAV_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(LIT_FIRST), MPVOID);
	if (sSelect >= 0) {
	  *szBuffer = 0;
	  WinSendDlgItemMsg(hwnd, SAV_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, 81), MPFROMP(szBuffer));
	  if (*szBuffer)
	    WinSetDlgItemText(hwnd, SAV_PATTERN, szBuffer);
	}
	break;

      case LN_ENTER:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      }
    }
    return 0;

  case WM_COMMAND:
    hwndCnr = *(HWND *) INSTDATA(hwnd);
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_SAVETOLIST, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;

    case SAV_FIND:
      {
	*savename = 0;
	WinQueryDlgItemText(hwnd, SAV_FILENAME, CCHMAXPATH, savename);
	if (!*savename)
	  strcpy(savename, "*.LST");
	if (export_filename(hwnd, savename, 1) && *savename) {
	  if (!strchr(savename, '.'))
	    strcat(savename, ".LST");
	  WinSetDlgItemText(hwnd, SAV_FILENAME, savename);
	}
      }
      break;

    case SAV_ADD:
    case SAV_DEL:
    case DID_OK:
      WinEnableWindow(hwnd, FALSE);
      {
	PCNRITEM pci;
	FILE *fp;
	CHAR *p, *pp, temp;
	INT attribute = CRA_CURSORED;
	SHORT sSelect;

	*pattern = 0;
	WinQueryDlgItemText(hwnd, SAV_PATTERN, 80, pattern);
	if (!*pattern) {
	  WinEnableWindow(hwnd, TRUE);
	  DosBeep(150, 100);
	  break;
	}
	{
	  switch (SHORT1FROMMP(mp1)) {
	  case SAV_ADD:
	    sSelect = (SHORT) WinSendDlgItemMsg(hwnd, SAV_LISTBOX,
						LM_SEARCHSTRING,
						MPFROM2SHORT(0, LIT_FIRST),
						MPFROMP(pattern));
	    if (sSelect < 0) {
	      WinSendDlgItemMsg(hwnd, SAV_LISTBOX, LM_INSERTITEM,
				MPFROM2SHORT(LIT_SORTASCENDING, 0),
				MPFROMP(pattern));
	      WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	    }
	    WinEnableWindow(hwnd, TRUE);
	    return 0;

	  case SAV_DEL:
	    sSelect = (SHORT) WinSendDlgItemMsg(hwnd, SAV_LISTBOX,
						LM_QUERYSELECTION,
						MPFROM2SHORT(LIT_FIRST, 0),
						MPVOID);
	    if (sSelect >= 0) {
	      WinSendDlgItemMsg(hwnd, SAV_LISTBOX, LM_DELETEITEM,
				MPFROM2SHORT(sSelect, 0), MPVOID);
	      WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	    }
	    WinEnableWindow(hwnd, TRUE);
	    return 0;
	  }
	}
	literal(pattern);
	if (!*pattern) {
	  WinEnableWindow(hwnd, TRUE);
	  DosBeep(250, 100);
	  break;
	}
	PrfWriteProfileString(fmprof, appname, "SaveToListPattern", pattern);
	*savename = 0;
	WinQueryDlgItemText(hwnd, SAV_FILENAME, CCHMAXPATH, savename);
	bstrip(savename);
	if (!*savename) {
	  WinEnableWindow(hwnd, TRUE);
	  DosBeep(100, 100);
	  break;
	}
	if (stricmp(savename, "PRN") &&
	    strnicmp(savename, "\\DEV\\LPT", 8) && !strchr(savename, '.'))
	  strcat(savename, ".LST");
	PrfWriteProfileString(fmprof, appname, "SaveToListName", savename);
	pci = (PCNRITEM) WinSendMsg(hwndCnr,
				    CM_QUERYRECORDEMPHASIS,
				    MPFROMLONG(CMA_FIRST),
				    MPFROMSHORT(attribute));
	if (pci && (INT) pci != -1) {
	  if (pci->rc.flRecordAttr & CRA_SELECTED) {
	    attribute = CRA_SELECTED;
	    pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS,
			     MPFROMLONG(CMA_FIRST), MPFROMSHORT(attribute));
	  }
	}
	if (!pci || (INT) pci == -1)
	  Runtime_Error(pszSrcFile, __LINE__, "no data");
	else {
	  fp = _fsopen(savename, "r+", SH_DENYWR);
	  if (!fp)
	    Runtime_Error(pszSrcFile, __LINE__, "_fsopen");
	  else {
	    fseek(fp, 0L, SEEK_SET);
	    if (WinQueryButtonCheckstate(hwnd, SAV_APPEND) == 0)
	      DosSetFileSize((HFILE) fileno(fp), 0L);
	    else
	      fseek(fp, 0L, SEEK_END);
	    while (pci && (INT) pci != -1) {
	      if (!(pci->rc.flRecordAttr & CRA_FILTERED)) {
		p = pattern;
		while (*p) {
		  if (*p == '%') {
		    p++;
		    switch (*p) {
		    case 's':
		      fputs(pci->pszSubject, fp);
		      break;
		    case 'S':
		      fprintf(fp, "%-40s", pci->pszSubject);
		      break;
		    case 'Z':
		      fprintf(fp, "%-13lu", pci->cbFile);
		      break;
		    case 'z':
		      fprintf(fp, "%lu", pci->cbFile);
		      break;
		    case 'E':
		      fprintf(fp, "%-5u", pci->easize);
		      break;
		    case 'e':
		      fprintf(fp, "%u", pci->easize);
		      break;
		    case 'd':
		    case 'D':
		      fprintf(fp,
			      "%04u/%02u/%02u",
			      pci->date.year, pci->date.month, pci->date.day);
		      break;
		    case 't':
		    case 'T':
		      fprintf(fp,
			      "%02u:%02u:%02u",
			      pci->time.hours,
			      pci->time.minutes, pci->time.seconds);
		      break;
		    case 'l':
		      fputs(pci->pszLongname, fp);
		      break;
		    case 'L':
		      fprintf(fp, "%-40s", pci->pszLongname);
		      break;
		    case 'F':
		    case 'f':
		      if (IsRoot(pci->pszFileName))
			pp = pci->pszFileName;
		      else {
			pp = strrchr(pci->pszFileName, '\\');
			if (pp)
			  pp++;
			else
			  pp = pci->pszFileName;
		      }
		      if (*p == 'F')
			fprintf(fp, "%-13s", pp);
		      else
			fputs(pp, fp);
		      break;
		    case 'p':
		      fputs(pci->pszFileName, fp);
		      break;
		    case 'P':
		      temp = 0;
		      if (!IsRoot(pci->pszFileName)) {
			pp = strrchr(pci->pszFileName, '\\');
			if (pp) {
			  temp = *pp;
			  *pp = 0;
			}
		      }
		      fputs(pci->pszFileName, fp);
		      if (temp)
			*pp = temp;
		      break;
		    case '$':
		      fputc(*pci->pszFileName, fp);
		      break;
		    case '%':
		      fputc('%', fp);
		      break;
		    }
		  }
		  else
		    fputc(*p, fp);
		  p++;
		}
		fputs("\n", fp);
	      }
	      pci = WinSendMsg(hwndCnr, CM_QUERYRECORDEMPHASIS, MPFROMP(pci),
			       MPFROMSHORT(attribute));
	    }
	    fclose(fp);
	  }
	}
      }
      WinEnableWindow(hwnd, TRUE);
      WinDismissDlg(hwnd, 1);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY SaveAllListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				    MPARAM mp2)
{

  CHAR **list;
  CHAR savename[CCHMAXPATH] = "", pattern[81];

  switch (msg) {
  case WM_INITDLG:
    if (!mp2) {
      Runtime_Error(pszSrcFile, __LINE__, "no data");
      WinDismissDlg(hwnd, 0);
    }
    else {
      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      list = (CHAR **) mp2;
      WinSendDlgItemMsg(hwnd,
			SAV_FILENAME,
			EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
      WinSendDlgItemMsg(hwnd,
			SAV_PATTERN,
			EM_SETTEXTLIMIT, MPFROM2SHORT(80, 0), MPVOID);
      *savename = *pattern = 0;
      {
	ULONG size;

	size = CCHMAXPATH;
	PrfQueryProfileData(fmprof,
			    appname, "SaveToListName", savename, &size);
	size = 81;
	PrfQueryProfileData(fmprof,
			    appname, "SaveToListPattern", pattern, &size);
      }
      WinSetDlgItemText(hwnd, SAV_FILENAME, savename);
      if (!*pattern)
	strcpy(pattern, "%F  %s");
      {
	CHAR temp[162];

	fixup(pattern, temp, sizeof(temp), strlen(pattern));
	WinSetDlgItemText(hwnd, SAV_PATTERN, temp);
      }
      {
	FILE *fp;
	CHAR s[CCHMAXPATH + 14];

	save_dir2(s);
	if (s[strlen(s) - 1] != '\\')
	  strcat(s, "\\");
	strcat(s, "PATTERNS.DAT");
	fp = _fsopen(s, "r", SH_DENYWR);
	if (fp) {
	  while (xfgets(s, 81, fp, pszSrcFile, __LINE__)) {
	    stripcr(s);
	    if (*s && *s != ';')
	      WinSendMsg(WinWindowFromID(hwnd, SAV_LISTBOX), LM_INSERTITEM,
			 MPFROM2SHORT(LIT_SORTASCENDING, 0), MPFROMP(s));
	  }
	  fclose(fp);
	}
	if (!WinSendDlgItemMsg(hwnd, SAV_LISTBOX, LM_QUERYITEMCOUNT,
			       MPVOID, MPVOID))
	  WinEnableWindow(WinWindowFromID(hwnd, SAV_LISTBOX), FALSE);
      }
    }
    break;

  case UM_SETDIR:
    {
      SHORT sSelect, sMax;
      CHAR szBuffer[CCHMAXPATH + 14];
      FILE *fp;

      sMax = (SHORT) WinSendDlgItemMsg(hwnd,
				       SAV_LISTBOX,
				       LM_QUERYITEMCOUNT, MPVOID, MPVOID);
      if (sMax > 0) {
	save_dir2(szBuffer);
	if (szBuffer[strlen(szBuffer) - 1] != '\\')
	  strcat(szBuffer, "\\");
	strcat(szBuffer, "PATTERNS.DAT");
	fp = xfopen(szBuffer, "w", pszSrcFile, __LINE__);
	if (fp) {
	  fputs(GetPString(IDS_LISTPATTERNTEXT), fp);
	  for (sSelect = 0; sSelect < sMax; sSelect++) {
	    *szBuffer = 0;
	    WinSendDlgItemMsg(hwnd,
			      SAV_LISTBOX,
			      LM_QUERYITEMTEXT,
			      MPFROM2SHORT(sSelect, 81), MPFROMP(szBuffer));
	    if (*szBuffer)
	      fprintf(fp, "%s\n", szBuffer);
	  }
	  fclose(fp);
	}
      }
      else if (!sMax) {
	save_dir2(szBuffer);
	if (szBuffer[strlen(szBuffer) - 1] != '\\')
	  strcat(szBuffer, "\\");
	strcat(szBuffer, "PATTERNS.DAT");
	unlinkf("%s", szBuffer);
      }
    }
    return 0;

  case WM_CONTROL:
    if (SHORT1FROMMP(mp1) == SAV_LISTBOX) {

      SHORT sSelect;
      CHAR szBuffer[81];

      switch (SHORT2FROMMP(mp1)) {
      case LN_SELECT:
	sSelect = (SHORT) WinSendDlgItemMsg(hwnd, SAV_LISTBOX,
					    LM_QUERYSELECTION,
					    MPFROMSHORT(LIT_FIRST), MPVOID);
	if (sSelect >= 0) {
	  *szBuffer = 0;
	  WinSendDlgItemMsg(hwnd, SAV_LISTBOX, LM_QUERYITEMTEXT,
			    MPFROM2SHORT(sSelect, 81), MPFROMP(szBuffer));
	  if (*szBuffer)
	    WinSetDlgItemText(hwnd, SAV_PATTERN, szBuffer);
	}
	break;

      case LN_ENTER:
	PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
	break;
      }
    }
    return 0;

  case WM_COMMAND:
    list = (CHAR **) INSTDATA(hwnd);
    switch (SHORT1FROMMP(mp1)) {
    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
	WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
		   MPFROM2SHORT(HELP_SAVETOLIST, 0),
		   MPFROMSHORT(HM_RESOURCEID));
      break;

    case SAV_FIND:
      {
	*savename = 0;
	WinQueryDlgItemText(hwnd, SAV_FILENAME, CCHMAXPATH, savename);
	if (!*savename)
	  strcpy(savename, "*.LST");
	if (export_filename(hwnd, savename, 1) && *savename) {
	  if (!strchr(savename, '.'))
	    strcat(savename, ".LST");
	  WinSetDlgItemText(hwnd, SAV_FILENAME, savename);
	}
      }
      break;

    case SAV_ADD:
    case SAV_DEL:
    case DID_OK:
      WinEnableWindow(hwnd, FALSE);
      {
	FILE *fp;
	CHAR *p, *pp, temp;
	INT x = 0;
	SHORT sSelect;
	FILEFINDBUF4 ffb4;
	ULONG nm;
	HDIR hdir;
	CHAR longname[CCHMAXPATH], subject[42];

	*pattern = 0;
	WinQueryDlgItemText(hwnd, SAV_PATTERN, 80, pattern);
	if (!*pattern) {
	  WinEnableWindow(hwnd, TRUE);
	  DosBeep(150, 100);
	  break;
	}
	{
	  switch (SHORT1FROMMP(mp1)) {
	  case SAV_ADD:
	    sSelect = (SHORT) WinSendDlgItemMsg(hwnd, SAV_LISTBOX,
						LM_SEARCHSTRING,
						MPFROM2SHORT(0, LIT_FIRST),
						MPFROMP(pattern));
	    if (sSelect < 0) {
	      WinSendDlgItemMsg(hwnd, SAV_LISTBOX, LM_INSERTITEM,
				MPFROM2SHORT(LIT_SORTASCENDING, 0),
				MPFROMP(pattern));
	      WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	    }
	    WinEnableWindow(hwnd, TRUE);
	    return 0;

	  case SAV_DEL:
	    sSelect = (SHORT) WinSendDlgItemMsg(hwnd, SAV_LISTBOX,
						LM_QUERYSELECTION,
						MPFROM2SHORT(LIT_FIRST, 0),
						MPVOID);
	    if (sSelect >= 0) {
	      WinSendDlgItemMsg(hwnd, SAV_LISTBOX, LM_DELETEITEM,
				MPFROM2SHORT(sSelect, 0), MPVOID);
	      WinSendMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
	    }
	    WinEnableWindow(hwnd, TRUE);
	    return 0;
	  }
	}
	literal(pattern);
	if (!*pattern) {
	  WinEnableWindow(hwnd, TRUE);
	  DosBeep(250, 100);
	  break;
	}
	PrfWriteProfileString(fmprof, appname, "SaveToListPattern", pattern);
	*savename = 0;
	WinQueryDlgItemText(hwnd, SAV_FILENAME, CCHMAXPATH, savename);
	bstrip(savename);
	if (!*savename) {
	  WinEnableWindow(hwnd, TRUE);
	  DosBeep(100, 100);
	  break;
	}
	if (stricmp(savename, "PRN") &&
	    strnicmp(savename, "\\DEV\\LPT", 8) && !strchr(savename, '.'))
	  strcat(savename, ".LST");
	PrfWriteProfileString(fmprof, appname, "SaveToListName", savename);
	if (!list || !list[0])
	  Runtime_Error(pszSrcFile, __LINE__, "no data");
	else {
	  fp = _fsopen(savename, "r+", SH_DENYWR);
	  if (!fp)
	    Runtime_Error(pszSrcFile, __LINE__, "_fsopen");
	  else {
	    fseek(fp, 0L, SEEK_SET);
	    if (WinQueryButtonCheckstate(hwnd, SAV_APPEND) == 0)
	      DosSetFileSize((HFILE) fileno(fp), 0L);
	    else
	      fseek(fp, 0L, SEEK_END);
	    while (list[x]) {
	      hdir = HDIR_CREATE;
	      nm = 1L;
	      *subject = *longname = 0;
	      if (!DosFindFirst(list[x], &hdir,
				FILE_NORMAL | FILE_DIRECTORY |
				FILE_READONLY | FILE_ARCHIVED |
				FILE_HIDDEN | FILE_SYSTEM,
				&ffb4, sizeof(ffb4), &nm, FIL_QUERYEASIZE)) {
		/* load the object's Subject, if required */
		if (ffb4.cbList > 4L) {
		  APIRET rc;
		  EAOP2 eaop;
		  PGEA2LIST pgealist;
		  PFEA2LIST pfealist;
		  PGEA2 pgea;
		  PFEA2 pfea;
		  CHAR *value;

		  pgealist =
		    xmallocz(sizeof(GEA2LIST) + 64, pszSrcFile, __LINE__);
		  if (pgealist) {
		    pgea = &pgealist->list[0];
		    strcpy(pgea->szName, SUBJECT);
		    pgea->cbName = strlen(pgea->szName);
		    pgea->oNextEntryOffset = 0L;
		    pgealist->cbList = sizeof(GEA2LIST) + pgea->cbName;
		    pfealist = xmallocz(1024, pszSrcFile, __LINE__);
		    if (pfealist) {
		      pfealist->cbList = 1024;
		      eaop.fpGEA2List = pgealist;
		      eaop.fpFEA2List = pfealist;
		      eaop.oError = 0L;
		      rc = DosQueryPathInfo(list[x],
					    FIL_QUERYEASFROMLIST,
					    (PVOID) & eaop,
					    (ULONG) sizeof(EAOP2));
		      if (!rc) {
			pfea = &eaop.fpFEA2List->list[0];
			value = pfea->szName + pfea->cbName + 1;
			value[pfea->cbValue] = 0;
			if (*(USHORT *) value == EAT_ASCII)
			  strncpy(subject, value + (sizeof(USHORT) * 2), 40);
			subject[40] = 0;
		      }
		      free(pfealist);
		    }
		    free(pgealist);
		  }
		}
		/* load the object's longname */
		if (ffb4.cbList > 4L) {
		  APIRET rc;
		  EAOP2 eaop;
		  PGEA2LIST pgealist;
		  PFEA2LIST pfealist;
		  PGEA2 pgea;
		  PFEA2 pfea;
		  CHAR *value;

		  pgealist =
		    xmallocz(sizeof(GEA2LIST) + 64, pszSrcFile, __LINE__);
		  if (pgealist) {
		    pgea = &pgealist->list[0];
		    strcpy(pgea->szName, LONGNAME);
		    pgea->cbName = strlen(pgea->szName);
		    pgea->oNextEntryOffset = 0L;
		    pgealist->cbList = sizeof(GEA2LIST) + pgea->cbName;
		    pfealist = xmallocz(1024, pszSrcFile, __LINE__);
		    if (pfealist) {
		      pfealist->cbList = 1024;
		      eaop.fpGEA2List = pgealist;
		      eaop.fpFEA2List = pfealist;
		      eaop.oError = 0L;
		      rc = DosQueryPathInfo(list[x],
					    FIL_QUERYEASFROMLIST,
					    (PVOID) & eaop,
					    (ULONG) sizeof(EAOP2));
		      if (!rc) {
			pfea = &eaop.fpFEA2List->list[0];
			value = pfea->szName + pfea->cbName + 1;
			value[pfea->cbValue] = 0;
			if (*(USHORT *) value == EAT_ASCII)
			  strncpy(longname, value +
				  (sizeof(USHORT) * 2), CCHMAXPATHCOMP);
			longname[CCHMAXPATHCOMP - 1] = 0;
		      }
		      free(pfealist);
		    }
		    free(pgealist);
		  }
		}

		p = pattern;
		while (*p) {
		  if (*p == '%') {
		    p++;
		    switch (*p) {
		    case 's':
		      fputs(subject, fp);
		      break;
		    case 'S':
		      fprintf(fp, "%-40s", subject);
		      break;
		    case 'Z':
		      fprintf(fp, "%-13lu", ffb4.cbFile);
		      break;
		    case 'z':
		      fprintf(fp, "%lu", ffb4.cbFile);
		      break;
		    case 'E':
		      fprintf(fp, "%-5u", CBLIST_TO_EASIZE(ffb4.cbList));
		      break;
		    case 'e':
		      fprintf(fp, "%u", CBLIST_TO_EASIZE(ffb4.cbList));
		      break;
		    case 'd':
		    case 'D':
		      fprintf(fp,
			      "%04u/%02u/%02u",
			      ffb4.fdateLastWrite.year + 1980,
			      ffb4.fdateLastWrite.month,
			      ffb4.fdateLastWrite.day);
		      break;
		    case 't':
		    case 'T':
		      fprintf(fp,
			      "%02u:%02u:%02u",
			      ffb4.ftimeLastWrite.hours,
			      ffb4.ftimeLastWrite.minutes,
			      ffb4.ftimeLastWrite.twosecs * 2);
		      break;
		    case 'l':
		      fputs(longname, fp);
		      break;
		    case 'L':
		      fprintf(fp, "%-40s", longname);
		      break;
		    case 'F':
		    case 'f':
		      if (IsRoot(list[x]))
			pp = list[x];
		      else {
			pp = strrchr(list[x], '\\');
			if (pp)
			  pp++;
			else
			  pp = list[x];
		      }
		      if (*p == 'F')
			fprintf(fp, "%-13s", pp);
		      else
			fputs(pp, fp);
		      break;
		    case 'p':
		      fputs(list[x], fp);
		      break;
		    case 'P':
		      temp = 0;
		      if (!IsRoot(list[x])) {
			pp = strrchr(list[x], '\\');
			if (pp) {
			  temp = *pp;
			  *pp = 0;
			}
		      }
		      fputs(list[x], fp);
		      if (temp)
			*pp = temp;
		      break;
		    case '$':
		      fputc(*list[x], fp);
		      break;
		    case '%':
		      fputc('%', fp);
		      break;
		    }
		  }
		  else
		    fputc(*p, fp);
		  p++;
		}
		fputs("\n", fp);
		DosFindClose(hdir);
	      }
	      x++;
	    }
	    fclose(fp);
	  }
	}
      }
      WinEnableWindow(hwnd, TRUE);
      WinDismissDlg(hwnd, 1);
      break;
    }
    return 0;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
