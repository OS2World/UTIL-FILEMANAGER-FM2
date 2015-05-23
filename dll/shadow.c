
/***********************************************************************

  $Id$

  Desktop shadows

  Copyright (c) 1993-97 M. Kimes
  Copyright (c) 2006, 2010 Steven H. Levine

  22 Jul 06 SHL Check more run time errors
  16 Jun 07 SHL Update for OpenWatcom
  06 Aug 07 SHL Use BldFullPathName
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  08 Mar 09 GKY Additional strings move to PCSZs declare change
  12 Jul 09 GKY Add xDosQueryAppType and xDosAlloc... to allow FM/2 to load in high memory
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  28 Jun 14 GKY Fix errors identified with CPPCheck;
  02 May 15 GKY Changes to allow a JAVA executable object to be created using "Real object"
                menu item on a jar file.
  23 May 15 GKY Option to restart desktop to prevent icon loss from JAVA object

***********************************************************************/

#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "filldir.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "pathutil.h"			// BldFullPathName
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "shadow.h"
#include "input.h"			// InputDlgProc
#include "defview.h"			// Data declaration(s)
#include "valid.h"			// IsFile
#include "wrappers.h"			// xmalloc
#include "fortify.h"
#include "init.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "getnames.h"                   // insert_filename
#include "srchpath.h"                   // SearchMultiplePathsForFile

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static HOBJECT CreateDataObject(CHAR * objtitle, CHAR * location, CHAR * path,
			 CHAR * cnr);
static HOBJECT CreateFolderObject(CHAR * objtitle, CHAR * cnr);

static HOBJECT CreateProgramObject(CHAR * objtitle, CHAR * location, CHAR * path,
                                   CHAR * cnr);
static HOBJECT CreateJAVAProgramObject(CHAR * objtitle, CHAR * location, CHAR * path,
			    CHAR * cnr);
static HOBJECT CreateShadowObject(CHAR * objtitle, CHAR * location, CHAR * path,
                                  BOOL executable, CHAR * cnr);
BOOL32 EXPENTRY WinRestartWorkplace(VOID);

HOBJECT CreateProgramObject(CHAR * objtitle, CHAR * location, CHAR * path,
			    CHAR * cnr)
{
  HOBJECT obj = (HOBJECT) 0;
  CHAR *s;
  CHAR objecttmp[CCHMAXPATH];

  if (!cnr)
    return obj;
  strcpy(objecttmp, objtitle);
  s = strchr(objecttmp, '.');
  if (s)
    *s = 0;
  obj = WinCreateObject((CHAR *) WPProgram,
			objecttmp,
			"NODELETE=NO;TEMPLATE=NO;NOCOPY=NO;NOMOVE=NO",
			(location) ? location : cnr, CO_FAILIFEXISTS);
  if (obj) {
    s = xmalloc(5192, pszSrcFile, __LINE__);
    if (s) {
      sprintf(s,
	      "EXENAME=%s%s%s%s%s;PARAMETERS=%%*;OBJECTID=<FM2_%s>",
	      (path) ? path : NullStr,
	      (path) ? PCSZ_BACKSLASH : NullStr,
	      objtitle,
	      (path) ? ";STARTUPDIR=" : NullStr, (path) ? path : NullStr, objtitle);
      WinSetObjectData(obj, s);
      free(s);
    }
  }
  return obj;
}

HOBJECT CreateJAVAProgramObject(CHAR * objtitle, CHAR * location, CHAR * path,
			    CHAR * cnr)
{
  HOBJECT obj = (HOBJECT) 0;
  CHAR *s;
  CHAR objecttmp[CCHMAXPATH];
  CHAR javaexe[CCHMAXPATH] = {0};
  CHAR icon[CCHMAXPATH] = {0};

  if (!cnr)
    return obj;
  if (!PrfQueryProfileString(fmprof, appname, "JavaExe", NULL, javaexe, CCHMAXPATH - 1)) {
    strcpy(javaexe, PCSZ_STARDOTEXE);
    if (insert_filename(HWND_DESKTOP, javaexe, TRUE, FALSE) &&
        *javaexe && !strchr(javaexe, '*') && !strchr(javaexe, '?'))
      PrfWriteProfileString(fmprof, appname, "JavaExe", javaexe);
    else
      return obj;
  }
  strcpy(objecttmp, objtitle);
  s = strchr(objecttmp, '.');
  if (s)
    *s = 0;
  strcpy(icon, path);
  strcat(icon, "\\*.ico");
  insert_filename(HWND_DESKTOP, icon, TRUE, FALSE);
  obj = WinCreateObject((CHAR *) WPProgram,
			objecttmp,
			"NOPRINT=YES;DEFAULTVIEW=RUNNING",
			(location) ? location : cnr, CO_FAILIFEXISTS);
  if (obj) {
    s = xmalloc(5192, pszSrcFile, __LINE__);
    if (s) {
      sprintf(s,
              "%s%s;EXENAME=%s%s%s;PARAMETERS= %s%s%s%s %%*;%sOBJECTID=<FM2_%s>",
              "ICONFILE=",
              icon,
              javaexe,
              (path) ? ";STARTUPDIR=" : NullStr,
              (path) ? path : NullStr,
              "-jar ",
	      (path) ? path : NullStr,
	      (path) ? PCSZ_BACKSLASH : NullStr,
              objtitle,
              "PROGTYPE=PM;",
	      objecttmp);
      WinSetObjectData(obj, s);
      free(s);
    }
  }
  return obj;
}

HOBJECT CreateDataObject(CHAR * objtitle, CHAR * location, CHAR * path,
			 CHAR * cnr)
{

  HOBJECT obj = (HOBJECT) 0;
  CHAR s[1050], s2[1050], *p,
    *type[] = { "WPDataFile", "WPIcon", "WPBitmap" };

  if (!cnr)
    return obj;
  BldFullPathName(s, path, objtitle);
  p = strrchr(objtitle, '.');
  if (p) {
    if (!stricmp(p, PCSZ_DOTICO))
      p = type[1];
    else if (!stricmp(p, PCSZ_DOTBMP))
      p = type[2];
    else
      p = type[0];
  }
  else
    p = type[0];
  obj = WinCreateObject(p,
			s,
			"NODELETE=NO;TEMPLATE=NO;NOCOPY=NO;NOMOVE=NO",
			(location) ? location : cnr, CO_FAILIFEXISTS);
  if (obj) {
    sprintf(s2,
	    "%s%s%s%sOBJECTID=<FM2_%s>",
	    objtitle,
	    (p == type[1]) ? "ICONFILE=" : NullStr,
	    (p == type[1]) ? s : NullStr, (p == type[1]) ? ";" : NullStr, objtitle);
    WinSetObjectData(obj, s2);
  }
  return obj;
}

HOBJECT CreateFolderObject(CHAR * objtitle, CHAR * cnr)
{
  HOBJECT obj = (HOBJECT) 0;
  CHAR s[1050];

  if (!cnr)
    return obj;
  obj = WinCreateObject("WPFolder",
			objtitle,
			"NODELETE=NO;TEMPLATE=NO;NOCOPY=NO;NOMOVE=NO",
			cnr, CO_FAILIFEXISTS);
  if (obj) {
    sprintf(s, "OBJECTID=<FM2_%s>", objtitle);
    WinSetObjectData(obj, s);
  }
  return obj;
}

HOBJECT CreateShadowObject(CHAR * objtitle, CHAR * location, CHAR * path,
			   BOOL executable, CHAR * cnr)
{

  HOBJECT obj = (HOBJECT) 0;
  CHAR *s;

  if (!cnr)
    return obj;
  s = xmalloc(5192, pszSrcFile, __LINE__);
  if (s) {
    sprintf(s,
	    "SHADOWID=%s%s%s",
	    (path) ? path : NullStr, (path) ? PCSZ_BACKSLASH : NullStr, objtitle);
    {					// find an icon for it if possible
      CHAR *p, temp[CCHMAXPATH + 1];

      BldFullPathName(temp, path, objtitle);
      p = strrchr(temp, '.');
      if (p) {
	*p = 0;
	strcat(p, PCSZ_DOTICO);
	if (IsFile(temp) == 1)
	  sprintf(&s[strlen(s)], ";ICONFILE=%s", temp);
      }
    }
    if (executable)
      sprintf(&s[strlen(s)],
	      ";EXENAME=%s%s%s%s%s;PARAMETERS=%%*",
	      (path) ? path : NullStr,
	      (path) ? PCSZ_BACKSLASH : NullStr,
	      objtitle, (path) ? ";STARTUPDIR=" : NullStr, (path) ? path : NullStr);
    strcat(s, ";NODELETE=NO;TEMPLATE=NO;NOCOPY=NO;NOMOVE=NO");
    sprintf(&s[strlen(s)], ";OBJECTID=<FM2_%s>", objtitle);
    obj = WinCreateObject("WPShadow",
			  objtitle,
			  s, (location) ? location : cnr, CO_FAILIFEXISTS);
    free(s);
  }
  return obj;
}

VOID MakeShadows(HWND hwnd, CHAR ** list, ULONG Shadows, CHAR * cnr,
		 CHAR * foldername)
{
  INT x = 0;
  CHAR szBuff[CCHMAXPATH + 8];
  HOBJECT obj = (HOBJECT) 0;
  FILESTATUS3 fsa;
  BOOL JAVA = FALSE;

  *szBuff = 0;
  if (foldername)
    strcpy(szBuff, foldername);
  if (list) {
    if ((list[0] && list[1]) || Shadows > 1) {

      STRINGINPARMS sip;

      sip.help = GetPString(IDS_MAKESHADOWHELPTEXT);
      sip.ret = szBuff;
      sip.prompt = GetPString(IDS_MAKESHADOWPROMPTTEXT);
      sip.inputlen = CCHMAXPATHCOMP;
      sip.title = GetPString(IDS_MAKESHADOWTITLETEXT);
      if (WinDlgBox(HWND_DESKTOP,
		    hwnd, InputDlgProc, FM3ModHandle, STR_FRAME, &sip)) {
	if (*szBuff) {
	  obj = CreateFolderObject(szBuff, cnr);
	  if (!obj) {
	    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
		   hwnd,
		   GetPString(IDS_ERRORTEXT),
		   GetPString(IDS_MAKESHADOWCREATEFAILEDTEXT), szBuff);
	    return;
	  }
	}
      }
      else
	return;
    }
    while (list[x]) {
      if (!DosQueryPathInfo(list[x],
			    FIL_STANDARD, &fsa,
			    (ULONG) sizeof(FILESTATUS3))) {

	ULONG apt;
	CHAR *p, *pp, szDir[CCHMAXPATH + 1], szBuffer[CCHMAXPATH + 1];

	if (xDosQueryAppType(list[x], &apt))
	  apt = 0;
	p = strrchr(list[x], '.');
	if (p) {
          if (!stricmp(p, PCSZ_DOTBAT) || !stricmp(p, PCSZ_DOTCMD) ||
              !stricmp(p, PCSZ_DOTBTM))
            apt |= FAPPTYP_BOUND;
          else if(!stricmp(p, ".jar"))
            JAVA = TRUE;
	}
	*szBuffer = 0;
	p = strrchr(list[x], '\\');
	if (!p)
	  p = strrchr(list[x], ':');
	if (p)
	  p++;
	else
	  p = list[x];
	strcpy(szDir, list[x]);
	pp = strrchr(szDir, '\\');
	if (!pp) {
	  pp = strrchr(szDir, ':');
	  if (pp) {
	    pp++;
	    *pp = '\\';
	    pp++;
	  }
	}
	if (pp)
	  *pp = 0;
	else
	  *szDir = 0;
	if (obj && *szBuff)
	  sprintf(szBuffer, "<FM2_%s>", szBuff);
	else
	  *szBuffer = 0;
	if ((fsa.attrFile & FILE_DIRECTORY) || Shadows)
          CreateShadowObject(p, (obj) ? szBuffer : NULL, szDir, 0, cnr);
        else if (JAVA)
          if (CreateJAVAProgramObject(p, (obj) ? szBuffer : NULL, szDir, cnr)) {
            ULONG ulResult;

            apt |= FAPPTYP_BOUND;
            ulResult = saymsg(MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1, HWND_DESKTOP,
                              GetPString(IDS_RESTARTDESKTOP),
			      GetPString(IDS_SETTINGLOSEICON));
            if (ulResult == MBID_OK){
              WinRestartWorkplace();
            }
          }
        else if (!(apt & (FAPPTYP_NOTWINDOWCOMPAT | FAPPTYP_WINDOWCOMPAT | FAPPTYP_WINDOWAPI |
                          FAPPTYP_BOUND | FAPPTYP_DOS | FAPPTYP_WINDOWSREAL |
                          FAPPTYP_WINDOWSPROT | 0x1000)))	// not an executable app?
	  CreateDataObject(p, (obj) ? szBuffer : NULL, szDir, cnr);
	else
	  CreateProgramObject(p, (obj) ? szBuffer : NULL, szDir, cnr);
      }
      x++;
      DosSleep(1);
    }
  }
}

VOID OpenObject(CHAR *filename, PCSZ type, HWND hwnd)
{
  HOBJECT hWPSObject;

  if (!type)
    type = Default;
  if ((*filename == '<' &&
       filename[strlen(filename) - 1] == '>') || IsFile(filename) != -1) {
    hWPSObject = WinQueryObject(filename);
    if (hWPSObject != NULLHANDLE) {	// got something; try to to open it

      CHAR s[CCHMAXPATH];
      HWND hwndDesktop;

      if (hwnd) {
	hwndDesktop = WinQueryDesktopWindow((HAB) 0, NULLHANDLE);
	WinSetFocus(HWND_DESKTOP, hwndDesktop);
      }
      sprintf(s, "OPEN=%s", type);
      if (!WinSetObjectData(hWPSObject, s) && hwnd)
	WinSetFocus(HWND_DESKTOP, hwnd);
    }
  }
}

BOOL RunSeamless(CHAR * exename, CHAR * args, HWND hwnd)
{
  CHAR settings[1024 + CCHMAXPATH + 80];
  BOOL ret;

  sprintf(settings,
	  "EXENAME=%s;PROGTYPE=SEAMLESS;PARAMETERS=%s;OPEN=DEFAULT",
	  exename, args);
  if (hwnd)
    WinSetFocus(HWND_DESKTOP, HWND_DESKTOP);
  ret = WinCreateObject((CHAR *) WPProgram,
			exename,
			settings, "<WP_NOWHERE>", CO_REPLACEIFEXISTS);
  if (!ret && hwnd)
    WinSetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER | SWP_ACTIVATE);
  return ret;
}

#pragma alloc_text(SHADOW,CreateProgramObject,CreateDataObject,CreateFolderObject,CreateShadowObject)
#pragma alloc_text(SHADOW2,MakeShadows,OpenObject)
