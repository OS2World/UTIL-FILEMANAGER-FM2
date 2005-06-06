
/***********************************************************************

  $Id$

  Initialization

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2005 Steven H. Levine

  11 Jun 02 SHL Add CheckVersion
  11 Jun 03 SHL Add JFS and FAT32 support
  25 Nov 03 SHL InitFM3DLL: correct strings error mesage
  23 May 05 SHL Use datamin.h
  26 May 05 SHL Comments
  06 Jun 05 SHL indent -i2

***********************************************************************/

#define DEFINE_GLOBALS 1

#define INCL_DOS
#define INCL_WIN
#define INCL_MMIOOS2
#define INCL_GPI

#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <time.h>
#include <process.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "datamin.h"
#include "tools.h"
#include "fm3str.h"
#include "version.h"

#pragma alloc_text(INIT,_DLL_InitTerm,InitFM3DLL,DeInitFM3DLL)
#pragma alloc_text(INIT1,StartFM3,FindSwapperDat)

extern int _CRT_init(void);
extern void _CRT_term(void);

/*
   extern HMODULE FM3ResHandle;
   extern ULONG   RVMajor,RVMinor;
 */

VOID FindSwapperDat(VOID)
{
  CHAR *filename = "C:\\CONFIG.SYS", input[8192], *p, *pp;
  FILE *fp;
  FILEFINDBUF3 ffb;
  ULONG nm = 1L, size = sizeof(SwapperDat);
  HDIR hdir = HDIR_CREATE;
  APIRET rc = 1L;

  *SwapperDat = 0;
  PrfQueryProfileData(fmprof,
		      FM3Str,
		      "SwapperDat",
		      (PVOID) SwapperDat,
		      &size);
  if (*SwapperDat)
  {
    rc = DosFindFirst(SwapperDat,
		      &hdir,
		      FILE_NORMAL | FILE_ARCHIVED |
		      FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
		      &ffb,
		      sizeof(ffb),
		      &nm,
		      FIL_STANDARD);
    if (!rc)
    {
      DosFindClose(hdir);
      fp = fopen(SwapperDat, "r");
      if (fp)
      {
	fclose(fp);
	*SwapperDat = 0;
	rc = 1L;
      }
    }
    else
      *SwapperDat = 0;
  }
  if (rc)
  {
    if (DosQuerySysInfo(QSV_BOOT_DRIVE,
			QSV_BOOT_DRIVE,
			(PVOID) & nm,
			(ULONG) sizeof(ULONG)))
      nm = 3L;
    *filename = (CHAR) nm + '@';
    fp = _fsopen(filename, "r", SH_DENYNO);
    if (fp)
    {
      while (!feof(fp))
      {
	if (!fgets(input, 8192, fp))
	  break;
	input[8191] = 0;
	lstrip(input);
	if (!strnicmp(input, "SWAPPATH", 8))
	{
	  p = input + 8;
	  while (*p == ' ')
	    p++;
	  if (*p == '=')
	  {
	    p++;
	    stripcr(p);
	    rstrip(p);
	    while (*p == ' ')
	      p++;
	    if (*p == '\"')
	    {
	      p++;
	      pp = p;
	      while (*pp && *pp != '\"')
		*pp++;
	      if (*pp)
		*pp = 0;
	    }
	    else
	    {
	      pp = strchr(p, ' ');
	      if (pp)
		*pp = 0;
	    }
	    if (*p)
	    {
	      strncpy(SwapperDat, p, CCHMAXPATH);
	      SwapperDat[CCHMAXPATH - 1] = 0;
	      if (SwapperDat[strlen(SwapperDat) - 1] != '\\')
		strcat(SwapperDat, "\\");
	      strcat(SwapperDat, "SWAPPER.DAT");
	      hdir = HDIR_CREATE;
	      nm = 1L;
	      if (!DosFindFirst(SwapperDat,
				&hdir,
				FILE_NORMAL | FILE_ARCHIVED |
				FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
				&ffb,
				sizeof(ffb),
				&nm,
				FIL_STANDARD))
	      {
		DosFindClose(hdir);
		PrfWriteProfileString(fmprof,
				      FM3Str,
				      "SwapperDat",
				      SwapperDat);
	      }
	      else
		*SwapperDat = 0;
	      break;
	    }
	  }
	}
      }
      fclose(fp);
    }
  }
}

unsigned long _System _DLL_InitTerm(unsigned long hModule,
				    unsigned long ulFlag)
{

  switch (ulFlag)
  {
  case 0:
    if (_CRT_init() == -1)
      return 0UL;
    FM3DllHandle = hModule;
    {
      char *env, stringfile[CCHMAXPATH];

      strcpy(stringfile, "FM3RES.STR");
      env = getenv("FM3INI");
      if (env)
      {

	FILESTATUS3 fsa;
	APIRET rc;

	DosError(FERR_DISABLEHARDERR);
	rc = DosQueryPathInfo(env,
			      FIL_STANDARD,
			      &fsa,
			      (ULONG) sizeof(fsa));
	if (!rc)
	{
	  if (fsa.attrFile & FILE_DIRECTORY)
	  {
	    strcpy(stringfile, env);
	    if (stringfile[strlen(stringfile) - 1] != '\\')
	      strcat(stringfile, "\\");
	    strcat(stringfile, "FM3RES.STR");
	    DosError(FERR_DISABLEHARDERR);
	    if (DosQueryPathInfo(stringfile,
				 FIL_STANDARD,
				 &fsa,
				 sizeof(fsa)))
	      strcpy(stringfile, "FM3RES.STR");
	  }
	}
      }
      LoadStrings(stringfile);
    }
    DosError(FERR_DISABLEHARDERR);
    /* strings here to prevent multiple occurences in DLL */
    FM2Str = "FM/2";
    FM3Str = "FM/3";
    NullStr = "";
    Default = "DEFAULT";
    Settings = "SETTINGS";
    WPProgram = "WPProgram";
    FM3Folder = "<FM3_Folder>";
    FM3Tools = "<FM3_Tools>";
    DRM_OS2FILE = "DRM_OS2FILE";
    DRM_FM2ARCMEMBER = "DRM_FM2ARCMEMBER";
    DRF_FM2ARCHIVE = "DRF_FM2ARCHIVE";
    DRMDRFLIST = "<DRM_OS2FILE,DRF_UNKNOWN>,"
      "<DRM_DISCARD,DRF_UNKNOWN>,"
      "<DRM_PRINT,DRF_UNKNOWN>";
    DRMDRFOS2FILE = "<DRM_OS2FILE,DRF_UNKNOWN>";
    DRMDRFFM2ARC = "<DRM_FM2ARCMEMBER,DRF_FM2ARCHIVE>";
    DRM_FM2INIRECORD = "DRM_FM2INIRECORD";
    DRF_FM2INI = "DRF_FM2INI";
    SUBJECT = ".SUBJECT";
    LONGNAME = ".LONGNAME";
    HPFS = "HPFS";
    JFS = "JFS";
    CDFS = "CDFS";
    FAT32 = "FAT32";
    HPFS386 = "HPFS386";
    CBSIFS = "CBSIFS";
    /* end of strings */
    memset(&RGBBLACK, 0, sizeof(RGB2));
    RGBGREY.bRed = RGBGREY.bGreen = RGBGREY.bBlue = 204;
    RGBGREY.fcOptions = 0;
    FM3UL = *(ULONG *) FM3Str;
    DEBUG_STRING = "Debug -- please report to author";
    break;
  case 1:
    StopPrinting = 1;
    if (fmprof)
      PrfCloseProfile(fmprof);
    DosError(FERR_ENABLEHARDERR);
    _CRT_term();
    break;
  default:
    return 0UL;
  }
  return 1UL;
}

VOID APIENTRY DeInitFM3DLL(ULONG why)
{
  /* cleanup */
  static CHAR s[CCHMAXPATH];
  CHAR *enddir;
  HDIR search_handle;
  ULONG num_matches;
  static FILEFINDBUF3 f;

  StopTimer();
  StopPrinting = 1;

  if (LogFileHandle)
    fclose(LogFileHandle);

  if (fmprof)
  {
    PrfCloseProfile(fmprof);
    fmprof = (HINI) 0;
    if (fIniExisted)
    {
      DosError(FERR_DISABLEHARDERR);
      DosCopy("FM3.INI",
	      "FM3INI.BAK",
	      DCPY_EXISTING);
    }
  }

  if (fToolsChanged)
    save_tools(NULL);

  _fcloseall();

  save_dir(s);
  if (s[strlen(s) - 1] != '\\')
    strcat(s, "\\");
  enddir = &s[strlen(s)];
  if (*ArcTempRoot)
  {
    strcat(s, ArcTempRoot);
    strcat(s, "*");
    search_handle = HDIR_CREATE;
    num_matches = 1L;
    if (!DosFindFirst(s,
		      &search_handle,
		      FILE_NORMAL | FILE_DIRECTORY |
		      FILE_SYSTEM | FILE_READONLY | FILE_HIDDEN |
		      FILE_ARCHIVED,
		      &f,
		      sizeof(FILEFINDBUF3),
		      &num_matches,
		      FIL_STANDARD))
    {
      do
      {
	strcpy(enddir, f.achName);
	if (f.attrFile & FILE_DIRECTORY)
	{
	  wipeallf("%s\\*", s);
	  DosDeleteDir(s);
	}
	else
	  unlinkf("%s", s);
      }
      while (!DosFindNext(search_handle,
			  &f,
			  sizeof(FILEFINDBUF3),
			  &num_matches));
      DosFindClose(search_handle);
    }
  }

  save_dir(s);
  if (s[strlen(s) - 1] != '\\')
    strcat(s, "\\");
  enddir = &s[strlen(s)];
  strcat(s, LISTTEMPROOT);
  strcat(s, "???");
  search_handle = HDIR_CREATE;
  num_matches = 1L;
  if (!DosFindFirst(s,
		    &search_handle,
		    FILE_NORMAL | FILE_DIRECTORY |
		    FILE_SYSTEM | FILE_READONLY | FILE_HIDDEN |
		    FILE_ARCHIVED,
		    &f,
		    sizeof(FILEFINDBUF3),
		    &num_matches,
		    FIL_STANDARD))
  {
    do
    {
      if (!(f.attrFile & FILE_DIRECTORY))
      {
	strcpy(enddir, f.achName);
	unlinkf("%s", s);
      }
    }
    while (!DosFindNext(search_handle,
			&f,
			sizeof(FILEFINDBUF3),
			&num_matches));
    DosFindClose(search_handle);
  }

  DosForceDelete("$FM2PLAY.$$$");

  EndNote();

  if (FM3ModHandle)
    DosFreeModule(FM3ModHandle);

  DosExitList(EXLST_REMOVE,
	      DeInitFM3DLL);
}

BOOL InitFM3DLL(HAB hab, int argc, char **argv)
{
  /*
   * this function should be called by any application using this DLL right
   * after setting up a message queue
   */

  CLASSINFO clinfo;
  BOOL okay = FALSE;

  if (!StringsLoaded())
  {
    DosBeep(50, 100);
    saymsg(MB_ENTER,
	   HWND_DESKTOP,
	   "Error",
	   "FM3RES.STR isn't in right format, at least "
	   "for this version of FM/2.");
    return FALSE;
  }

  {					/* Load resource DLL FM3RES.DLL */
    APIRET rcl = 1, rcq = 1;
    PFN pfnResVersion = (PFN) NULL;
    ULONG RVMajor = 0, RVMinor = 0, ret = 0;
    char *env, dllfile[CCHMAXPATH];

    strcpy(dllfile, "FM3RES");
    env = getenv("FM3INI");
    if (env)
    {

      FILESTATUS3 fsa;
      APIRET rc;

      DosError(FERR_DISABLEHARDERR);
      rc = DosQueryPathInfo(env,
			    FIL_STANDARD,
			    &fsa,
			    (ULONG) sizeof(fsa));
      if (!rc)
      {
	if (fsa.attrFile & FILE_DIRECTORY)
	{
	  strcpy(dllfile, env);
	  if (dllfile[strlen(dllfile) - 1] != '\\')
	    strcat(dllfile, "\\");
	  strcat(dllfile, "FM3RES");
	  DosError(FERR_DISABLEHARDERR);
	  if (DosQueryPathInfo(dllfile,
			       FIL_STANDARD,
			       &fsa,
			       sizeof(fsa)))
	    strcpy(dllfile, "FM3RES");
	}
      }
    }
    rcl = DosLoadModule(NULL,
			0,
			dllfile,
			&FM3ModHandle);
    if (!rcl)
    {
      if (DosExitList(EXLST_ADD,
		      DeInitFM3DLL))
	DosBeep(500, 100);
      rcq = DosQueryProcAddr(FM3ModHandle,
			     1,
			     "ResVersion",
			     &pfnResVersion);
      if (!rcq)
	ret = pfnResVersion(&RVMajor, &RVMinor);
    }
    else
    {
      saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
	     HWND_DESKTOP,
	     GetPString(IDS_ERRORTEXT),
	     GetPString(IDS_FM3RESERROR1TEXT));
      return FALSE;
    }
    if (RVMajor < VERMAJOR ||
	(RVMajor == VERMAJOR && RVMinor < VERMINOR))
    {
      DosBeep(50, 100);
      saymsg(MB_ENTER,
	     HWND_DESKTOP,
	     GetPString(IDS_ERRORTEXT),
	     GetPString(IDS_FM3RESERROR2TEXT),
	     (!rcq) ?
	     GetPString(IDS_FM3RESERROR3TEXT) :
	     (!rcl) ?
	     GetPString(IDS_FM3RESERROR4TEXT) :
	     GetPString(IDS_FM3RESERROR5TEXT),
	     RVMajor,
	     RVMinor,
	     rcl,
	     rcq,
	     ret);
      return FALSE;
    }
  }

  ArgDriveFlags(argc, argv);
  FillInDriveFlags(NULL);

  {					/* try to ensure that FM/2 Utilities are available */
    CHAR curpath[CCHMAXPATH + 8], *env;
    FILESTATUS3 fs3;

    save_dir2(curpath);
    strcat(curpath, "\\UTILS");
    if (!DosQueryPathInfo(curpath,
			  FIL_STANDARD,
			  &fs3,
			  sizeof(fs3)) &&
	(fs3.attrFile & FILE_DIRECTORY))
    {
      env = getenv("PATH");
      if (env)
      {
	if (!stristr(curpath, env))
	  fAddUtils = TRUE;
      }
    }
  }
  if (!*profile)
    strcpy(profile, "FM3.INI");
  mypid = getpid();
  /* give default appname if none set by caller */
  if (!*appname)
    strcpy(appname, FM3Str);
  /* save appname; may be reset below */
  strcpy(realappname, appname);
  if (!strcmp(appname, FM3Str))
    DosSetMaxFH(100L);
  else if (!strcmp(appname, "VDir") ||
	   !strcmp(appname, "VTree") ||
	   !strcmp(appname, "VCollect") ||
	   !strcmp(appname, "SEEALL") ||
	   !strcmp(appname, "FM/4"))
    DosSetMaxFH(60L);
  else
    DosSetMaxFH(40L);

  if (DosQuerySysInfo(QSV_VERSION_MAJOR,
		      QSV_VERSION_MINOR,
		      (PVOID) OS2ver,
		      (ULONG) sizeof(OS2ver)))
  {
    OS2ver[0] = 2;
    OS2ver[1] = 1;
  }

  /* set up default root names for temp archive goodies */
  if (!fAmAV2)
    strcpy(ArcTempRoot, "$FM$ARC$");
  else
    strcpy(ArcTempRoot, "$AV$ARC$");

  /* initialize random number generator */
  srand(time(NULL) + clock());

  priority_bumped();

  /* _heapmin() is done in a separate thread -- start it */
  if (_beginthread(HeapThread,
		   NULL,
		   32768,
		   NULL) ==
      -1 ||
  /* timer messages are sent from a separate thread -- start it */
      !StartTimer())
  {
    DosBeep(50, 100);
    DosSleep(10);
    DosBeep(50, 100);
    return FALSE;
  }

  {					/* are we the workplace? */
    CHAR *env;

    env = getenv("WORKPLACE__PROCESS");
    if (!env || stricmp(env, "NO"))
      fWorkPlace = TRUE;
  }

  if ((!strchr(profile, '\\') && !strchr(profile, ':')) ||
      !(fmprof = PrfOpenProfile((HAB) 0, profile)))
  {

    /* figure out where to put INI file... */
    CHAR *env, inipath[CCHMAXPATH];

    DosError(FERR_DISABLEHARDERR);
    save_dir2(HomePath);
    DosError(FERR_DISABLEHARDERR);
    memset(driveserial, -1, sizeof(driveserial));
    *inipath = 0;
    env = getenv("FM3INI");
    if (env)
    {

      FILESTATUS3 fsa;
      APIRET rc;

      strcpy(inipath, env);
      DosError(FERR_DISABLEHARDERR);
      rc = DosQueryPathInfo(inipath,
			    FIL_STANDARD,
			    &fsa,
			    (ULONG) sizeof(fsa));
      if (!rc)
      {
	if (fsa.attrFile & FILE_DIRECTORY)
	{
	  if (inipath[strlen(inipath) - 1] != '\\')
	    strcat(inipath, "\\");
	  strcat(inipath, profile);
	}
      }
    }
    if (!env)
    {
      env = searchpath(profile);
      if (!env)
	env = profile;
      strcpy(inipath, env);
    }
    /* in some odd cases the INI file can get set to readonly status */
    /* here we test it and reset the readonly bit if necessary */
    {
      FILESTATUS3 fsa;

      if (!*inipath)
	strcpy(inipath, profile);
      DosError(FERR_DISABLEHARDERR);
      if (!DosQueryPathInfo(inipath, FIL_STANDARD, &fsa, (ULONG) sizeof(fsa)))
      {
	fIniExisted = TRUE;
	if (fsa.attrFile & (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM))
	{
	  fsa.attrFile &= (~(FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM));
	  if (DosSetPathInfo(inipath, FIL_STANDARD, (PVOID) & fsa,
			     (ULONG) sizeof(fsa), 0L))
	    saymsg(MB_ENTER,
		   HWND_DESKTOP,
		   GetPString(IDS_ERRORTEXT),
		   GetPString(IDS_INIREADONLYTEXT),
		   inipath);
	}
      }
    }
    fmprof = PrfOpenProfile((HAB) 0, inipath);
    if (!fmprof)
    {
      strcpy(inipath, "FM3.INI");
      fmprof = PrfOpenProfile((HAB) 0, inipath);
    }
    if (!fmprof)
    {
      DosBeep(50, 100);
      DosSleep(10);
      DosBeep(50, 100);
      DosSleep(10);
      DosBeep(50, 100);
      return FALSE;
    }
  }

  FindSwapperDat();

  {
    ULONG size = sizeof(BOOL);

    PrfQueryProfileData(fmprof,
			FM3Str,
			"SeparateParms",
			(PVOID) & fSeparateParms,
			&size);
    if (!fSeparateParms)
      strcpy(appname,
	     FM3Str);
  }

  /* start help */
  memset(&hini, 0, sizeof(HELPINIT));
  hini.cb = sizeof(HELPINIT);
  hini.ulReturnCode = 0L;
  hini.pszTutorialName = NULL;
  hini.phtHelpTable = (PHELPTABLE) MAKELONG(ID_HELPTABLE, 0xffff);
  hini.hmodAccelActionBarModule = (HMODULE) 0;
  hini.idAccelTable = 0;
  hini.idActionBar = 0;
  hini.pszHelpWindowTitle = GetPString(IDS_FM2HELPTITLETEXT);
  hini.fShowPanelId = CMIC_HIDE_PANEL_ID;
  hini.pszHelpLibraryName = "FM3.HLP";
  hwndHelp = WinCreateHelpInstance(hab, &hini);
  if (!hwndHelp)
  {

    CHAR *env;
    static CHAR helppath[CCHMAXPATH];

    env = getenv("FM3INI");
    if (env)
    {

      FILESTATUS3 fsa;
      APIRET rc;

      strcpy(helppath, env);
      DosError(FERR_DISABLEHARDERR);
      rc = DosQueryPathInfo(helppath, FIL_STANDARD, &fsa, (ULONG) sizeof(fsa));
      if (!rc)
      {
	if (fsa.attrFile & FILE_DIRECTORY)
	{
	  if (helppath[strlen(helppath) - 1] != '\\')
	    strcat(helppath, "\\");
	  strcat(helppath, "FM3.HLP");
	  hini.pszHelpLibraryName = helppath;
	  hwndHelp = WinCreateHelpInstance(hab, &hini);
	}
      }
    }
  }
  if (!hwndHelp)
  {
    saymsg(MB_ENTER | MB_ICONEXCLAMATION,
	   HWND_DESKTOP,
	   GetPString(IDS_FM2TROUBLETEXT),
	   GetPString(IDS_CANTLOADHELPTEXT),
	   GetPString(IDS_NOHELPACCEPTTEXT));
  }

  /* a couple of default window procs so we don't have to look them up later */
  if (WinQueryClassInfo(hab, WC_CONTAINER, &clinfo))
    PFNWPCnr = clinfo.pfnWindowProc;
  // saymsg(MB_ENTER,HWND_DESKTOP,"Container flags:","%08lx",clinfo.flClassStyle);
  if (WinQueryClassInfo(hab, WC_FRAME, &clinfo))
    PFNWPFrame = clinfo.pfnWindowProc;
  // saymsg(MB_ENTER,HWND_DESKTOP,"Frame flags:","%08lx",clinfo.flClassStyle);
  if (WinQueryClassInfo(hab, WC_BUTTON, &clinfo))
    PFNWPButton = clinfo.pfnWindowProc;
  // saymsg(MB_ENTER,HWND_DESKTOP,"Button flags:","%08lx",clinfo.flClassStyle);
  if (WinQueryClassInfo(hab, WC_STATIC, &clinfo))
    PFNWPStatic = clinfo.pfnWindowProc;
  // saymsg(MB_ENTER,HWND_DESKTOP,"Static flags:","%08lx",clinfo.flClassStyle);
  if (WinQueryClassInfo(hab, WC_MLE, &clinfo))
    PFNWPMLE = clinfo.pfnWindowProc;
  // saymsg(MB_ENTER,HWND_DESKTOP,"MLE flags:","%08lx",clinfo.flClassStyle);
  if (!PFNWPCnr || !PFNWPFrame || !PFNWPButton || !PFNWPStatic || !PFNWPMLE)
  {
    DosBeep(50, 100);
    DosSleep(10);
    DosBeep(50, 100);
    DosSleep(10);
    DosBeep(50, 100);
    DosSleep(10);
    DosBeep(50, 100);
    return FALSE;
  }

  /* register window classes we use */
  WinRegisterClass(hab,
		   GetPString(IDS_WCMAINWND),
		   MainWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID) * 8);
  WinRegisterClass(hab,
		   GetPString(IDS_WCMAINWND2),
		   MainWndProc2,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID) * 4);
  WinRegisterClass(hab,
		   GetPString(IDS_WCTREECONTAINER),
		   TreeClientWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   GetPString(IDS_WCDIRCONTAINER),
		   DirClientWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   GetPString(IDS_WCCOLLECTOR),
		   CollectorClientWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   GetPString(IDS_WCARCCONTAINER),
		   ArcClientWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   GetPString(IDS_WCMLEEDITOR),
		   MLEEditorProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   GetPString(IDS_WCINIEDITOR),
		   IniProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   GetPString(IDS_WCTOOLBACK),
		   ToolBackProc,
		   CS_SIZEREDRAW,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCDRIVEBACK),
		   DriveBackProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCSEEALL),
		   SeeAllWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCNEWVIEW),
		   ViewWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCTOOLBUTTONS),
		   ChildButtonProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCDRIVEBUTTONS),
		   DriveProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCBUBBLE),
		   BubbleProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG) * 2);
  WinRegisterClass(hab,
		   GetPString(IDS_WCSTATUS),
		   StatusProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   GetPString(IDS_WCDIRSTATUS),
		   DirTextProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   GetPString(IDS_WCTREESTATUS),
		   TreeStatProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   GetPString(IDS_WCARCSTATUS),
		   ArcTextProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   GetPString(IDS_WCCOLSTATUS),
		   CollectorTextProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   GetPString(IDS_WCSEESTATUS),
		   SeeStatusProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   GetPString(IDS_WCVIEWSTATUS),
		   ViewStatusProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   GetPString(IDS_WCERRORWND),
		   NotifyWndProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCMINITIME),
		   MiniTimeProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   GetPString(IDS_WCDATABAR),
		   DataProc,
		   CS_SIZEREDRAW,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCTREEOPENBUTTON),
		   OpenButtonProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCAUTOVIEW),
		   AutoViewProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   GetPString(IDS_WCLED),
		   LEDProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));

  /*
   * set some defaults (note:  everything else automatically initialized
   * to 0)
   */
  detailssize = detailsea = detailslwdate = detailslwtime = detailsattr =
    detailsicon = fAutoTile = fConfirmDelete = fLoadSubject = fUnHilite =
    fLoadLongnames = fToolbar = fSaveState = fGuessType = fToolbarHelp =
    fAutoAddDirs = fUseNewViewer = fDataToFore = fDataShowDrives =
    fSplitStatus = fDragndropDlg = fQuickArcFind = fKeepCmdLine =
    fMoreButtons = fDrivebar = fCollapseFirst = fSwitchTree =
    fSwitchTreeExpand = fNoSearch = fCustomFileDlg = fOtherHelp =
    fSaveMiniCmds = fUserComboBox = fFM2Deletes = fConfirmTarget =
    fShowTarget = fDrivebarHelp = fCheckMM = TRUE;
  ulCnrType = CCS_EXTENDSEL;
  FilesToGet = 128L;
  AutoviewHeight = 48L;
  strcpy(printer, "PRN");
  prnwidth = 80;
  prnlength = 66;
  prntmargin = 6;
  prnbmargin = 6;
  prnlmargin = 6;
  prnrmargin = 3;
  prnspacing = 1;
  prntabspaces = 8;
  CollectorsortFlags = sortFlags = SORT_DIRSFIRST;

  /*
   * load preferences from profile (INI) file
   */
  {
    ULONG size;

    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"ShowTarget",
			(PVOID) & fShowTarget,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"CheckMM",
			(PVOID) & fCheckMM,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"ChangeTarget",
			(PVOID) & fChangeTarget,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"ConfirmTarget",
			(PVOID) & fConfirmTarget,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			FM3Str,
			"CustomFileDlg",
			(PVOID) & fCustomFileDlg,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			FM3Str,
			"SaveMiniCmds",
			(PVOID) & fSaveMiniCmds,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"SaveBigCmds",
			(PVOID) & fSaveBigCmds,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"NoFoldMenu",
			(PVOID) & fNoFoldMenu,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			FM3Str,
			"ThreadNotes",
			(PVOID) & fThreadNotes,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "Prnpagenums", (PVOID) & prnpagenums, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "Prnalt", (PVOID) & prnalt, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "Prnformat", (PVOID) & prnformat, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "Prnformfeedbefore",
			(PVOID) & prnformfeedbefore, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			FM3Str,
			"Prnformfeedafter",
			(PVOID) & prnformfeedafter,
			&size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "Prntabspaces",
			(PVOID) & prntabspaces, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "Prnwidth", (PVOID) & prnwidth, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "Prnlength", (PVOID) & prnlength, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "Prntmargin", (PVOID) & prntmargin, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "Prnbmargin", (PVOID) & prnbmargin, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "Prnlmargin", (PVOID) & prnlmargin, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "Prnrmargin", (PVOID) & prnrmargin, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "Prnspacing", (PVOID) & prnspacing, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "NoDead", (PVOID) & fNoDead, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "NoFinger", (PVOID) & fNoFinger, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "SwitchTree", (PVOID) & fSwitchTree,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "SwitchTreeExpand",
			(PVOID) & fSwitchTreeExpand, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "SwitchTreeOnFocus",
			(PVOID) & fSwitchTreeOnFocus, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "CollapseFirst",
			(PVOID) & fCollapseFirst, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "FilesInTree",
			(PVOID) & fFilesInTree, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "TopDir", (PVOID) & fTopDir, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "LookInDir", (PVOID) & fLookInDir, &size);
    PrfQueryProfileString(fmprof, appname, "DefArc", NULL, szDefArc, sizeof(szDefArc));
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "AutoviewHeight",
			(PVOID) & AutoviewHeight, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "KeepCmdLine", (PVOID) & fKeepCmdLine, &size);
    if (strcmp(realappname, "FM/4"))
    {
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof,
			  FM3Str,
			  "MoreButtons",
			  (PVOID) & fMoreButtons,
			  &size);
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof,
			  FM3Str,
			  "Drivebar",
			  (PVOID) & fDrivebar,
			  &size);
    }
    else
      fDrivebar = fMoreButtons = TRUE;
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"NoSearch",
			(PVOID) & fNoSearch,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"GuessType",
			(PVOID) & fGuessType,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"ViewChild",
			(PVOID) & fViewChild,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"ShowEnv",
			(PVOID) & fShowEnv,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"LeaveTree",
			(PVOID) & fLeaveTree,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "Comments", (PVOID) & fComments, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, appname, "WS_ANIMATE", (PVOID) & fwsAnimate, &size);
    if (fwsAnimate)
      fwsAnimate = WS_ANIMATE;
    size = sizeof(ULONG);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "SelectedAlways",
			(PVOID) & fSelectedAlways, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "ToolbarHelp", (PVOID) & fToolbarHelp, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "OtherHelp", (PVOID) & fOtherHelp, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "DrivebarHelp", (PVOID) & fDrivebarHelp, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"AutoAddDirs",
			(PVOID) & fAutoAddDirs,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof,
			appname,
			"AutoAddAllDirs",
			(PVOID) & fAutoAddAllDirs,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "UserListSwitches",
			(PVOID) & fUserListSwitches, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "UseNewViewer",
			(PVOID) & fUseNewViewer, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "DefaultDeletePerm",
			(PVOID) & fDefaultDeletePerm, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "ExternalINIs",
			(PVOID) & fExternalINIs, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "ExternalCollector",
			(PVOID) & fExternalCollector, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "ExternalArcboxes",
			(PVOID) & fExternalArcboxes, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "ExternalViewer",
			(PVOID) & fExternalViewer, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "UseQProcStat",
			(PVOID) & fUseQProcStat, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "DataMin", (PVOID) & fDataMin, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "DataToFore", (PVOID) & fDataToFore, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "DataShowDrives",
			(PVOID) & fDataShowDrives, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "DataInclRemote",
			(PVOID) & fDataInclRemote, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "SplitStatus", (PVOID) & fSplitStatus, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "FolderAfterExtract",
			(PVOID) & fFolderAfterExtract, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "DullDatabar", (PVOID) & fDullMin, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "BlueLED", (PVOID) & fBlueLED, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "ConfirmDelete", (PVOID) & fConfirmDelete, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "SaveState", (PVOID) & fSaveState, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "SyncUpdates", (PVOID) & fSyncUpdates, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "LoadSubject", (PVOID) & fLoadSubject, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "UnHilite", (PVOID) & fUnHilite, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "TileBackwards", (PVOID) & fTileBackwards,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "LoadLongname", (PVOID) & fLoadLongnames, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "VerifyWrites", (PVOID) & fVerify, &size);
    DosSetVerify(fVerify);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "DontMoveMouse", (PVOID) & fDontMoveMouse, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "NoIconsFiles", (PVOID) & fNoIconsFiles, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "NoIconsDirs", (PVOID) & fNoIconsDirs, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "ForceUpper", (PVOID) & fForceUpper, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "ForceLower", (PVOID) & fForceLower, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "TextTools", (PVOID) & fTextTools, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "ToolTitles", (PVOID) & fToolTitles, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "DoubleClickOpens", (PVOID) & fDCOpens, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "LinkSetsIcon", (PVOID) & fLinkSetsIcon, &size);
    size = sizeof(INT);
    PrfQueryProfileData(fmprof, appname, "Sort", (PVOID) & sortFlags, &size);
    size = sizeof(INT);
    PrfQueryProfileData(fmprof, appname, "TreeSort", (PVOID) & TreesortFlags, &size);
    size = sizeof(INT);
    PrfQueryProfileData(fmprof,
			appname,
			"CollectorSort",
			(PVOID) & CollectorsortFlags,
			&size);
    size = sizeof(targetdir);
    PrfQueryProfileData(fmprof,
			appname,
			"Targetdir",
			(PVOID) targetdir,
			&size);
    if (!IsValidDir(targetdir))
      *targetdir = 0;
    size = sizeof(extractpath);
    PrfQueryProfileData(fmprof,
			appname,
			"ExtractPath",
			(PVOID) extractpath,
			&size);
    if (!IsValidDir(extractpath))
      *extractpath = 0;
    size = sizeof(printer);
    PrfQueryProfileData(fmprof, appname, "Printer", (PVOID) printer, &size);
    size = sizeof(dircompare);
    PrfQueryProfileData(fmprof, appname, "DirCompare", (PVOID) dircompare, &size);
    size = sizeof(viewer);
    PrfQueryProfileData(fmprof, appname, "Viewer", (PVOID) viewer, &size);
    size = sizeof(editor);
    PrfQueryProfileData(fmprof, appname, "Editor", (PVOID) editor, &size);
    size = sizeof(binview);
    PrfQueryProfileData(fmprof, appname, "BinView", (PVOID) binview, &size);
    size = sizeof(bined);
    PrfQueryProfileData(fmprof, appname, "BinEd", (PVOID) bined, &size);
    size = sizeof(compare);
    PrfQueryProfileData(fmprof, appname, "Compare", (PVOID) compare, &size);
    size = sizeof(virus);
    PrfQueryProfileData(fmprof, appname, "Virus", (PVOID) virus, &size);
    size = sizeof(ftprun);
    PrfQueryProfileData(fmprof, appname, "FTPRun", (PVOID) ftprun, &size);
    if (!*ftprun && !size)
      strcpy(ftprun, "ftppm.exe");
    size = sizeof(httprun);
    PrfQueryProfileData(fmprof, appname, "HTTPRun", (PVOID) httprun, &size);
    if (!*httprun && !size)
      strcpy(httprun, "explore.exe -q");
    size = sizeof(lasttoolbox);
    PrfQueryProfileData(fmprof, FM3Str, "LastToolBox", (PVOID) lasttoolbox, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "FollowTree", (PVOID) & fFollowTree, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "StartMaximized", (PVOID) & fStartMaximized, &size);
    if (!fStartMaximized)
    {
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, appname, "StartMinimized", (PVOID) & fStartMinimized, &size);
    }
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "DefaultCopy", (PVOID) & fCopyDefault, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "IdleCopy", (PVOID) & fRealIdle, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "ArcStuffVisible",
			(PVOID) & fArcStuffVisible, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "NoTreeGap", (PVOID) & fNoTreeGap, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "VTreeOpensWPS", (PVOID) & fVTreeOpensWPS,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "RemoteBug", (PVOID) & fRemoteBug, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "Drag&DropDlg", (PVOID) & fDragndropDlg,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "UserComboBox", (PVOID) & fUserComboBox,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "MinDirOnOpen", (PVOID) & fMinOnOpen,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "QuickArcFind", (PVOID) & fQuickArcFind,
			&size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "NoRemovableScan", (PVOID) & fNoRemovableScan,
			&size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, FM3Str, "NoBrokenNotify", (PVOID) & NoBrokenNotify,
			&size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, appname, "ContainerType", (PVOID) & ulCnrType, &size);
    size = sizeof(ULONG);
    PrfQueryProfileData(fmprof, appname, "FilesToGet", (PVOID) & FilesToGet, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "AutoView", (PVOID) & fAutoView, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "FM2Deletes", (PVOID) & fFM2Deletes, &size);
  }

  /* load pointers and icons we use */
  hptrArrow = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE);
  hptrBusy = WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE);
  hptrNS = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZENS, FALSE);
  hptrEW = WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE);
  hptrFloppy = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FLOPPY_ICON);
  hptrDrive = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, DRIVE_ICON);
  hptrRemovable = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, REMOVABLE_ICON);
  hptrCDROM = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, CDROM_ICON);
  hptrFile = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FILE_ICON);
  hptrDir = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, DIR_FRAME);
  hptrArc = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ARC_FRAME);
  hptrArt = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ART_ICON);
  hptrSystem = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FILE_SYSTEM_ICON);
  hptrHidden = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FILE_HIDDEN_ICON);
  hptrReadonly = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FILE_READONLY_ICON);
  hptrLast = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, LASTITEM_ICON);
  hptrRemote = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, REMOTE_ICON);
  if (!fNoDead)
    hptrFinger = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FINGER_ICON);
  else
    hptrFinger = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FINGER2_ICON);
  hptrApp = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, APP_ICON);
  hptrDunno = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, DUNNO_ICON);
  hptrEnv = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ENV_ICON);
  hptrZipstrm = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ZIPSTREAM_ICON);

  /*
   * set up color array used by seeall.c and newview.c color dialog
   */
  standardcolors[0] = CLR_WHITE;
  standardcolors[1] = CLR_BLACK;
  standardcolors[2] = CLR_BLUE;
  standardcolors[3] = CLR_RED;
  standardcolors[4] = CLR_PINK;
  standardcolors[5] = CLR_GREEN;
  standardcolors[6] = CLR_CYAN;
  standardcolors[7] = CLR_YELLOW;
  standardcolors[8] = CLR_DARKGRAY;
  standardcolors[9] = CLR_DARKBLUE;
  standardcolors[10] = CLR_DARKRED;
  standardcolors[11] = CLR_DARKPINK;
  standardcolors[12] = CLR_DARKGREEN;
  standardcolors[13] = CLR_DARKCYAN;
  standardcolors[14] = CLR_BROWN;
  standardcolors[15] = CLR_PALEGRAY;

  return TRUE;
}

HWND StartFM3(HAB hab, INT argc, CHAR ** argv)
{
  HWND hwndFrame, hwndClient;
  ULONG FrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
  FCF_SIZEBORDER | FCF_MINMAX |
  FCF_ACCELTABLE | FCF_MENU |
  FCF_ICON | FCF_TASKLIST |
  FCF_NOBYTEALIGN;

  {
    INT x;

    for (x = 1; x < argc; x++)
    {
      if (*argv[x] == '~' && !argv[x][1])
	fReminimize = TRUE;
      if (*argv[x] == '+' && !argv[x][1])
	fLogFile = TRUE;
      if (*argv[x] == '-')
      {
	if (!argv[x][1])
	  fNoSaveState = TRUE;
	else
	  strcpy(profile, &argv[x][1]);
      }
    }
  }

  hwndFrame = WinCreateStdWindow(HWND_DESKTOP,
				 WS_VISIBLE,
				 &FrameFlags,
				 GetPString(IDS_WCMAINWND),
				 NULL,
				 WS_VISIBLE | WS_ANIMATE,
				 FM3ModHandle,
				 MAIN_FRAME,
				 &hwndClient);
  if (hwndFrame)
  {
    WinSetWindowUShort(hwndFrame,
		       QWS_ID,
		       MAIN_FRAME);
    if (!WinRestoreWindowPos(FM2Str,
			     "MainWindowPos",
			     hwndFrame))
    {

      ULONG fl = SWP_MOVE | SWP_SIZE;
      RECTL rcl;
      ULONG icz = WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 3L;
      ULONG bsz = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);

      WinQueryWindowRect(HWND_DESKTOP,
			 &rcl);
      rcl.yBottom += icz;
      rcl.yTop -= bsz;
      rcl.xLeft += bsz;
      rcl.xRight -= bsz;
      WinSetWindowPos(hwndFrame,
		      HWND_TOP,
		      rcl.xLeft,
		      rcl.yBottom,
		      rcl.xRight - rcl.xLeft,
		      rcl.yTop - rcl.yBottom, fl);
    }
    if (fLogFile)
      LogFileHandle = _fsopen("FM2.LOG",
			      "a+",
			      SH_DENYWR);
    if (hwndHelp)
      WinAssociateHelpInstance(hwndHelp,
			       hwndFrame);
    PostMsg(hwndClient,
	    UM_SETUP,
	    MPFROMLONG(argc),
	    MPFROMP(argv));
  }
  return hwndFrame;
}

int CheckVersion(int vermajor, int verminor)
{
  int ok = 0;

  // fixme to do useful check - was missing in base source

#if 0
  if (vermajor && verminor)
  {
    *vermajor = VERMAJOR;
    *verminor = VERMINOR;
    ok = 1;
  }
#endif

  ok = 1;

  return ok;
}
