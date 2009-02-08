
/***********************************************************************

  $Id$

  Save file list to clipboard

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2008 Steven H. Levine

  12 Feb 03 SHL SaveListDlgProc: standardize EA math
  01 Aug 04 SHL Rework lstrip/rstrip usage
  01 Aug 04 SHL Rework fixup usage
  24 May 05 SHL Rework for CNRITEM.szSubject
  17 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets
  22 Mar 07 GKY Use QWL_USER
  06 Aug 07 GKY Increase Subject EA to 1024
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  27 Sep 07 SHL Correct ULONGLONG size formatting
  16 Nov 07 SHL Ensure fixup buffer sufficiently large
  30 Dec 07 GKY Use CommaFmtULL
  16 Feb 08 GKY Changed _fsopen flag so a new list file can be created
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory and use BldFullPathName
  20 Jul 08 GKY Modify ListtoClipHab to provide either fullpath name or filename for save to clipboard
  24 Aug 08 GKY Warn full drive on save of .DAT file; prevent loss of existing file
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Move repeated strings to PCSZs.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>

#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "arccnrs.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "saveclip.h"
#include "makelist.h"			// AddToList
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "pathutil.h"                   // BldFullPathName
#include "literal.h"			// fixup
#include "subj.h"			// Subject
#include "getnames.h"			// export_filename
#include "copyf.h"			// unlinkf
#include "wrappers.h"			// xfgets
#include "strips.h"			// bstrip
#include "misc.h"			// CheckDriveSpaceAvail
#include "commafmt.h"			// CommaFmtULL
#include "valid.h"			// IsRoot
#include "dirs.h"			// save_dir2
#include "fortify.h"

static PSZ pszSrcFile = __FILE__;

//static VOID ListToClipboard(HWND hwnd, CHAR ** list, ULONG append);

static CHAR **ListFromClipboardHab(HAB hab);
static BOOL SaveToClipHab(HAB hab, CHAR * text, BOOL append);

#define MAX_PATTERN_BYTES 80

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
	  clip = (CHAR *)WinQueryClipbrdData(hab, CF_TEXT);
	if (clip)
	  len += strlen(clip) + 1;
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

#if 0	// JBS	11 Sep 08
VOID ListToClipboard(HWND hwnd, CHAR ** list, ULONG append)
{
  HAB hab = WinQueryAnchorBlock(hwnd);

  ListToClipboardHab(hab, list, append);
}
#endif

VOID ListToClipboardHab(HAB hab, CHAR ** list, ULONG append)
{
  CHAR *text = NULL, **clip = NULL, *p = NULL, temp[CCHMAXPATH];
  INT x;
  ULONG len = 0;

  if (list && list[0]) {
    for (x = 0; list[x]; x++) {
      if (append == IDM_SAVETOCLIPFILENAME ||
          append == IDM_APPENDTOCLIPFILENAME) {
        p = strrchr(list[x], '\\');
        if (p) {
          p++;
          strcpy(temp, p);
          free(list[x]);
          list[x] = xstrdup(temp, __FILE__, __LINE__);
        }
      }
      len += strlen(list[x]) + 2;
    }
    if (len)
      len++;
    if (len) {
      if (append == IDM_APPENDTOCLIP ||
          append == IDM_APPENDTOCLIP2 ||
          append == IDM_APPENDTOCLIPFILENAME)
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
          text[strlen(text) - 2] = 0;
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
  UINT numfiles = 0, numalloced = 0;

  if (WinOpenClipbrd(hab)) {
    p = (CHAR *)WinQueryClipbrdData(hab, CF_TEXT);
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
	size = MAX_PATTERN_BYTES + 1;
	PrfQueryProfileData(fmprof,
			    appname, "SaveToListPattern", pattern, &size);
      }
      WinSetDlgItemText(hwnd, SAV_FILENAME, savename);
      if (!*pattern)
	strcpy(pattern, "%F  %s");
      {
	CHAR temp[MAX_PATTERN_BYTES * 4 + 1];

	fixup(pattern, temp, sizeof(temp), strlen(pattern));
	WinSetDlgItemText(hwnd, SAV_PATTERN, temp);
      }
      {
	FILE *fp;
	CHAR s[CCHMAXPATH + 14];

        BldFullPathName(s, pFM2SaveDirectory, "PATTERNS.DAT");
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
        BldFullPathName(szBuffer, pFM2SaveDirectory, "PATTERNS.DAT");
        if (CheckDriveSpaceAvail(szBuffer, ullDATFileSpaceNeeded, 1) == 2)
          break; //already gave error msg
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
        BldFullPathName(szBuffer, pFM2SaveDirectory, "PATTERNS.DAT");
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
	  strcpy(savename, PCSZ_STARDOTLST);
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
        CHAR szCmmaFmtFileSize[81];
	INT attribute = CRA_CURSORED;
	SHORT sSelect;

	*pattern = 0;
	WinQueryDlgItemText(hwnd, SAV_PATTERN, 80, pattern);
	if (!*pattern) {
	  WinEnableWindow(hwnd, TRUE);
          if (!fAlertBeepOff)
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
          if (!fAlertBeepOff)
	    DosBeep(250, 100);
	  break;
	}
	PrfWriteProfileString(fmprof, appname, "SaveToListPattern", pattern);
	*savename = 0;
	WinQueryDlgItemText(hwnd, SAV_FILENAME, CCHMAXPATH, savename);
	bstrip(savename);
	if (!*savename) {
	  WinEnableWindow(hwnd, TRUE);
          if (!fAlertBeepOff)
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
	  fp = _fsopen(savename, "w+", SH_DENYWR);
	  if (!fp)
	    Runtime_Error(pszSrcFile, __LINE__, "_fsopen");
	  else {
	    fseek(fp, 0, SEEK_SET);
	    if (WinQueryButtonCheckstate(hwnd, SAV_APPEND) == 0)
	      DosSetFileSize((HFILE) fileno(fp), 0);
	    else
	      fseek(fp, 0, SEEK_END);
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
                      CommaFmtULL(szCmmaFmtFileSize,
                      sizeof(szCmmaFmtFileSize), pci->cbFile, ' ');
		      fprintf(fp, "%-13s", szCmmaFmtFileSize);
		      break;
		    case 'z':
                      CommaFmtULL(szCmmaFmtFileSize,
                      sizeof(szCmmaFmtFileSize), pci->cbFile, ' ');
		      fprintf(fp, "%s", szCmmaFmtFileSize);
		      break;
		    case 'E':
		      fprintf(fp, "%-5u", pci->easize);
		      break;
		    case 'e':
		      fprintf(fp, "%u", pci->easize);
		      break;
		    case 'd':
                    case 'D':
                      {
                        CHAR szDate[DATE_BUF_BYTES];

                        DateFormat(szDate, pci->date);
		        fprintf(fp,"%s", szDate);
                        break;
                      }
		    case 't':
		    case 'T':
		      fprintf(fp,
			      "%02u%s%02u%s%02u",
			      pci->time.hours, TimeSeparator,
			      pci->time.minutes, TimeSeparator, pci->time.seconds);
		      break;
		    case 'l':
		      fputs(pci->pszLongName, fp);
		      break;
		    case 'L':
		      fprintf(fp, "%-40s", pci->pszLongName);
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
	size = MAX_PATTERN_BYTES + 1;
	PrfQueryProfileData(fmprof,
			    appname, "SaveToListPattern", pattern, &size);
      }
      WinSetDlgItemText(hwnd, SAV_FILENAME, savename);
      if (!*pattern)
	strcpy(pattern, "%F  %s");
      {
	CHAR temp[MAX_PATTERN_BYTES * 4 + 1];

	fixup(pattern, temp, sizeof(temp), strlen(pattern));
	WinSetDlgItemText(hwnd, SAV_PATTERN, temp);
      }
      {
	FILE *fp;
	CHAR s[CCHMAXPATH + 14];

        BldFullPathName(s, pFM2SaveDirectory, "PATTERNS.DAT");
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
        BldFullPathName(szBuffer, pFM2SaveDirectory, "PATTERNS.DAT");
        if (CheckDriveSpaceAvail(szBuffer, ullDATFileSpaceNeeded, 1) == 2)
          break; //already gave error msg
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
        BldFullPathName(szBuffer, pFM2SaveDirectory, "PATTERNS.DAT");
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
	  strcpy(savename, PCSZ_STARDOTLST);
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
	FILEFINDBUF4L ffb4;
	ULONG nm;
	HDIR hdir;
	CHAR longname[CCHMAXPATH], subject[1024];

	*pattern = 0;
	WinQueryDlgItemText(hwnd, SAV_PATTERN, 80, pattern);
	if (!*pattern) {
	  WinEnableWindow(hwnd, TRUE);
          if (!fAlertBeepOff)
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
          if (!fAlertBeepOff)
	    DosBeep(250, 100);
	  break;
	}
	PrfWriteProfileString(fmprof, appname, "SaveToListPattern", pattern);
	*savename = 0;
	WinQueryDlgItemText(hwnd, SAV_FILENAME, CCHMAXPATH, savename);
	bstrip(savename);
	if (!*savename) {
	  WinEnableWindow(hwnd, TRUE);
          if (!fAlertBeepOff)
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
	    fseek(fp, 0, SEEK_SET);
	    if (WinQueryButtonCheckstate(hwnd, SAV_APPEND) == 0)
	      DosSetFileSize((HFILE) fileno(fp), 0);
	    else
	      fseek(fp, 0, SEEK_END);
	    while (list[x]) {
	      hdir = HDIR_CREATE;
	      nm = 1;
	      *subject = *longname = 0;
	      if (!xDosFindFirst(list[x], &hdir,
				 FILE_NORMAL | FILE_DIRECTORY |
				 FILE_READONLY | FILE_ARCHIVED |
				 FILE_HIDDEN | FILE_SYSTEM,
				 &ffb4, sizeof(ffb4), &nm, FIL_QUERYEASIZEL)) {
		/* load the object's Subject, if required */
		if (ffb4.cbList > 4) {
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
		    pgea->oNextEntryOffset = 0;
		    pgealist->cbList = sizeof(GEA2LIST) + pgea->cbName;
		    pfealist = xmallocz(1024, pszSrcFile, __LINE__);
		    if (pfealist) {
		      pfealist->cbList = 1024;
		      eaop.fpGEA2List = pgealist;
		      eaop.fpFEA2List = pfealist;
		      eaop.oError = 0;
		      rc = DosQueryPathInfo(list[x],
					    FIL_QUERYEASFROMLIST,
					    (PVOID) & eaop,
					    (ULONG) sizeof(EAOP2));
		      if (!rc) {
			pfea = &eaop.fpFEA2List->list[0];
			value = pfea->szName + pfea->cbName + 1;
			value[pfea->cbValue] = 0;
			if (*(USHORT *) value == EAT_ASCII)
			  strncpy(subject, value + (sizeof(USHORT) * 2), 1023);
			subject[1023] = 0;
		      }
		      free(pfealist);
		    }
		    free(pgealist);
		  }
		}
		/* load the object's longname */
		if (ffb4.cbList > 4) {
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
		    pgea->oNextEntryOffset = 0;
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
                      CHAR szCmmaFmtFileSize[81];
		    case 's':
		      fputs(subject, fp);
		      break;
		    case 'S':
		      fprintf(fp, "%-40s", subject);
		      break;
		    case 'Z':
                      CommaFmtULL(szCmmaFmtFileSize,
                      sizeof(szCmmaFmtFileSize), ffb4.cbFile, ' ');
		      fprintf(fp, "%-13s", szCmmaFmtFileSize);
		      break;
		    case 'z':
                      CommaFmtULL(szCmmaFmtFileSize,
                      sizeof(szCmmaFmtFileSize), ffb4.cbFile, ' ');
		      fprintf(fp, "%s", szCmmaFmtFileSize);
		      break;
		    case 'E':
		      fprintf(fp, "%-5u", CBLIST_TO_EASIZE(ffb4.cbList));
		      break;
		    case 'e':
		      fprintf(fp, "%u", CBLIST_TO_EASIZE(ffb4.cbList));
		      break;
		    case 'd':
                    case 'D':
                      {
                        CHAR szDate[DATE_BUF_BYTES];

                        FDateFormat(szDate, ffb4.fdateLastWrite);
		        fprintf(fp,"%s", szDate);
                        break;
                      }
		    case 't':
		    case 'T':
		      fprintf(fp,
			      "%02u%s%02u%s%02u",
                              ffb4.ftimeLastWrite.hours,
                              TimeSeparator,
                              ffb4.ftimeLastWrite.minutes,
                              TimeSeparator,
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

#pragma alloc_text(FMCLIPBOARDIN,SaveToClip,SaveToClipHab)
#pragma alloc_text(FMCLIPBOARDOUT,ListToClipboardHab)
#pragma alloc_text(FMCLIPBOARDOUT,ListFromClipboard,ListFromClipboardHab)
#pragma alloc_text(SAVELIST,SaveListDlgProc,SaveAllListDlgProc)
