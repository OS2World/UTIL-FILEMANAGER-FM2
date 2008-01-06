
/***********************************************************************

  $Id$

  Initialization

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  11 Jun 02 SHL Add CheckVersion
  11 Jun 03 SHL Add JFS and FAT32 support
  25 Nov 03 SHL InitFM3DLL: correct strings error mesage
  23 May 05 SHL Use datamin.h
  26 May 05 SHL Comments
  06 Jun 05 SHL indent -i2
  06 Jun 05 SHL Rework FindSwapperDat for VAC3.65 compat
  13 Jul 06 SHL Use Runtime_Error
  13 Jul 06 SHL Sync with current style
  29 Jul 06 SHL Use xfgets
  22 Oct 06 GKY Add NDFS32 support
  18 Feb 07 GKY Add ISOFS, RAMFS support
  30 Mar 07 GKY Defined golbals for removing GetPString for window class names
  21 Apr 07 GKY Find FM2Utils by path or utils directory eleminate fAddUtils global
  15 Jun 07 SHL Make OpenWatcom compatible
  23 Jun 07 GKY Fix WORPLACE_PROCESS enviroment check logic
  28 Jun 07 SHL Rework WORKPLACE_PROCESS check to match reality
  03 Aug 07 GKY Enlarged and made setable everywhere Findbuf (speed file loading)
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  13 Aug 07 SHL Move #pragma alloc_text to end for OpenWatcom compat
  18 Aug 07 JBS Added code to read Details* keys from the INI file (Ticket 118)
  19 Aug 07 SHL Ensure FilesToGet in valid range
  21 Aug 07 GKY Make Subject column in dircnr sizable and movable from the rigth to the left pane
  23 Aug 07 SHL InitFM3DLL: report INI file DosSetPathInfo error correctly
  23 Aug 07 SHL Use BldFullPathName
  25 Aug 07 SHL Work around DosSetPathInfo kernel defect
  01 Sep 07 GKY Use xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundry
  10 Nov 07 GKY Get thousands separator from country info for file sizes.
  26 Nov 07 GKY Eliminate check of ext path on start up
  17 Dec 07 GKY Make WPURLDEFAULTSETTINGS the fall back for ftp/httprun

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>
#include <process.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_MMIOOS2
#define INCL_GPI
#define INCL_DOSERRORS
#define INCL_LONGLONG
#define INCL_DOSNLS
#include <os2.h>
#include <os2me.h>

#define DEFINE_GLOBALS 1

#include "fm3dlg.h"
#include "datamin.h"
#include "tools.h"
#include "fm3str.h"
#include "version.h"
#include "pathutil.h"			// BldFullPathName
#include "arccnrs.h"			// ArcClientWndProc
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "fm3dll.h"

#ifdef __IBMC__
#pragma alloc_text(INIT,LibMain,InitFM3DLL,DeInitFM3DLL)
#pragma alloc_text(INIT1,StartFM3,FindSwapperDat)
#endif

extern int _CRT_init(void);
extern void _CRT_term(void);

static PSZ pszSrcFile = __FILE__;

VOID FindSwapperDat(VOID)
{
  CHAR filename[] = "C:\\CONFIG.SYS";
  CHAR input[8192];
  CHAR *p;
  CHAR *pp;
  FILE *fp;
  FILEFINDBUF3L ffb;
  ULONG nm;
  ULONG size = sizeof(SwapperDat);
  HDIR hdir = HDIR_CREATE;
  APIRET rc = 1;

  *SwapperDat = 0;
  // Check already known
  PrfQueryProfileData(fmprof, FM3Str, "SwapperDat", SwapperDat, &size);
  if (*SwapperDat) {
    nm = 1;
    rc = DosFindFirst(SwapperDat,
		      &hdir,
		      FILE_NORMAL | FILE_ARCHIVED |
		      FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
		      &ffb, sizeof(ffb), &nm, FIL_STANDARDL);
    if (rc && rc != ERROR_FILE_NOT_FOUND && rc != ERROR_PATH_NOT_FOUND) {
      FILEFINDBUF3 ffb;
      rc = DosFindFirst(SwapperDat,
			&hdir,
			FILE_NORMAL | FILE_ARCHIVED |
			FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
			&ffb, sizeof(ffb), &nm, FIL_STANDARD);
      fNoLargeFileSupport = TRUE;
    }
    if (!rc) {
      DosFindClose(hdir);
      fp = fopen(SwapperDat, "r");
      if (fp) {
	fclose(fp);
	*SwapperDat = 0;
	rc = 1;				// Force config.sys scan
      }
    }
    else
      *SwapperDat = 0;
  }
  // If not defined in INI or INI wrong, scan config.sys for SWAPPATH statement
  if (rc) {
    if (DosQuerySysInfo(QSV_BOOT_DRIVE,
			QSV_BOOT_DRIVE,
			&nm,
			sizeof(ULONG))) {
      nm = 3;				// Assume drive C:
    }
    *filename = (CHAR) nm + '@';
    fp = xfsopen(filename, "r", SH_DENYNO, pszSrcFile, __LINE__);
    if (fp) {
      while (!feof(fp)) {
	if (!xfgets(input, sizeof(input), fp, pszSrcFile, __LINE__))
	  break;
	lstrip(input);
	if (!strnicmp(input, "SWAPPATH", 8)) {
	  p = input + 8;
	  while (*p == ' ')
	    p++;
	  if (*p == '=') {
	    p++;
	    stripcr(p);
	    rstrip(p);
	    while (*p == ' ')
	      p++;
	    if (*p == '\"') {
	      p++;
	      pp = p;
	      while (*pp && *pp != '\"')
		*pp += 1;
	      if (*pp)
		*pp = 0;
	    }
	    else {
	      pp = strchr(p, ' ');
	      if (pp)
		*pp = 0;
	    }
	    if (*p) {
	      strncpy(SwapperDat, p, CCHMAXPATH);
	      SwapperDat[CCHMAXPATH - 1] = 0;
	      BldFullPathName(SwapperDat, SwapperDat, "SWAPPER.DAT");
	      hdir = HDIR_CREATE;
	      nm = 1;
	      rc = DosFindFirst(SwapperDat,
				&hdir,
				FILE_NORMAL | FILE_ARCHIVED |
				FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
				&ffb, sizeof(ffb), &nm, FIL_STANDARD);
	      if (rc){
		FILEFINDBUF3 ffb;
		rc = DosFindFirst(SwapperDat,
				  &hdir,
				  FILE_NORMAL | FILE_ARCHIVED |
				  FILE_HIDDEN | FILE_SYSTEM | FILE_READONLY,
				  &ffb, sizeof(ffb), &nm, FIL_STANDARD);
		fNoLargeFileSupport = TRUE;
	      }
	      if (!rc) {
		DosFindClose(hdir);
		PrfWriteProfileString(fmprof,
				      FM3Str, "SwapperDat", SwapperDat);
	      }
	      else
		*SwapperDat = 0;
	      break;
	    }
	  }
	}				// if SWAPPATH
      }					// while
      fclose(fp);
    }
  }
}

#ifdef __WATCOMC__

unsigned APIENTRY LibMain(unsigned hModule,
			  unsigned ulFlag)
{
  CHAR *env;
  CHAR stringfile[CCHMAXPATH];
  FILESTATUS3 fsa;
  APIRET rc;

  switch (ulFlag) {
  case 0:
    // 14 Jun 07 SHL Already done for us
    // if (_CRT_init() == -1)
    //   return 0UL;
    FM3DllHandle = hModule;
    strcpy(stringfile, "FM3RES.STR");
    env = getenv("FM3INI");
    if (env) {
      DosError(FERR_DISABLEHARDERR);
      rc = DosQueryPathInfo(env, FIL_STANDARD, &fsa, sizeof(fsa));
      if (!rc) {
	if (fsa.attrFile & FILE_DIRECTORY) {
	  BldFullPathName(stringfile, env, "FM3RES.STR");
	  DosError(FERR_DISABLEHARDERR);
	  if (DosQueryPathInfo(stringfile, FIL_STANDARD, &fsa, sizeof(fsa)))
	    strcpy(stringfile, "FM3RES.STR");
	}
      }
    }
    LoadStrings(stringfile);

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
      "<DRM_DISCARD,DRF_UNKNOWN>," "<DRM_PRINT,DRF_UNKNOWN>";
    DRMDRFOS2FILE = "<DRM_OS2FILE,DRF_UNKNOWN>";
    DRMDRFFM2ARC = "<DRM_FM2ARCMEMBER,DRF_FM2ARCHIVE>";
    DRM_FM2INIRECORD = "DRM_FM2INIRECORD";
    DRF_FM2INI = "DRF_FM2INI";
    SUBJECT = ".SUBJECT";
    LONGNAME = ".LONGNAME";
    HPFS = "HPFS";
    JFS = "JFS";
    CDFS = "CDFS";
    ISOFS = "ISOFS";
    FAT32 = "FAT32";
    HPFS386 = "HPFS386";
    CBSIFS = "CBSIFS";
    NDFS32 = "NDFS32";
    RAMFS = "RAMFS";
    NTFS = "NTFS";
    WC_OBJECTWINDOW    =  "WC_OBJECTWINDOW";
    WC_BUBBLE          =  "WC_BUBBLE";
    WC_TOOLBUTTONS     =  "WC_TOOLBUTTONS";
    WC_DRIVEBUTTONS    =  "WC_DRIVEBUTTONS";
    WC_DIRCONTAINER    =  "WC_DIRCONTAINER";
    WC_DIRSTATUS       =  "WC_DIRSTATUS";
    WC_TREECONTAINER   =  "WC_TREECONTAINER";
    WC_TREEOPENBUTTON  =  "WC_TREEOPENBUTTON";
    WC_TREESTATUS      =  "WC_TREESTATUS";
    WC_MAINWND         =  "WC_MAINWND";
    WC_MAINWND2        =  "WC_MAINWND2";
    WC_AUTOVIEW        =  "WC_AUTOVIEW";
    WC_LED             =  "WC_LED";
    WC_COLLECTOR       =  "WC_COLLECTOR";
    WC_COLSTATUS       =  "WC_COLSTATUS";
    WC_STATUS          =  "WC_STATUS";
    WC_TOOLBACK        =  "WC_TOOLBACK";
    WC_DRIVEBACK       =  "WC_DRIVEBACK";
    WC_ARCCONTAINER    =  "WC_ARCCONTAINER";
    WC_ARCSTATUS       =  "WC_ARCSTATUS";
    WC_MLEEDITOR       =  "WC_MLEEDITOR";
    WC_INIEDITOR       =  "WC_INIEDITOR";
    WC_SEEALL          =  "WC_SEEALL";
    WC_NEWVIEW         =  "WC_NEWVIEW";
    WC_SEESTATUS       =  "WC_SEESTATUS";
    WC_VIEWSTATUS      =  "WC_VIEWSTATUS";
    WC_ERRORWND        =  "WC_ERRORWND";
    WC_MINITIME        =  "WC_MINITIME";
    WC_DATABAR         =  "WC_DATABAR";

    /* end of strings */
    memset(&RGBBLACK, 0, sizeof(RGB2));
    RGBGREY.bRed = RGBGREY.bGreen = RGBGREY.bBlue = (BYTE)204;
    RGBGREY.fcOptions = 0;
    FM3UL = *(ULONG *)FM3Str;
    DEBUG_STRING = "Debug -- please report to author";
    break;
  case 1:
    StopPrinting = 1;
    if (fmprof)
      PrfCloseProfile(fmprof);
    DosError(FERR_ENABLEHARDERR);
    // 14 Jun 07 SHL Already done for us
    // _CRT_term();
    break;
  default:
    return 0UL;
  }
  return 1UL;
}

#else // __IBMC__

unsigned long _System _DLL_InitTerm(unsigned long hModule,
				    unsigned long ulFlag)
{
  CHAR *env;
  CHAR stringfile[CCHMAXPATH];
  FILESTATUS3 fsa;
  APIRET rc;

  switch (ulFlag) {
  case 0:
    if (_CRT_init() == -1)
      return 0UL;
    FM3DllHandle = hModule;
    strcpy(stringfile, "FM3RES.STR");
    env = getenv("FM3INI");
    if (env) {
      DosError(FERR_DISABLEHARDERR);
      rc = DosQueryPathInfo(env, FIL_STANDARD, &fsa, sizeof(fsa));
      if (!rc) {
	if (fsa.attrFile & FILE_DIRECTORY) {
	  BldFullPathName(stringfile, env, "FM3RES.STR");
	  DosError(FERR_DISABLEHARDERR);
	  if (DosQueryPathInfo(stringfile, FIL_STANDARD, &fsa, sizeof(fsa)))
	    strcpy(stringfile, "FM3RES.STR");
	}
      }
    }
    LoadStrings(stringfile);

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
      "<DRM_DISCARD,DRF_UNKNOWN>," "<DRM_PRINT,DRF_UNKNOWN>";
    DRMDRFOS2FILE = "<DRM_OS2FILE,DRF_UNKNOWN>";
    DRMDRFFM2ARC = "<DRM_FM2ARCMEMBER,DRF_FM2ARCHIVE>";
    DRM_FM2INIRECORD = "DRM_FM2INIRECORD";
    DRF_FM2INI = "DRF_FM2INI";
    SUBJECT = ".SUBJECT";
    LONGNAME = ".LONGNAME";
    HPFS = "HPFS";
    JFS = "JFS";
    CDFS = "CDFS";
    ISOFS = "ISOFS";
    FAT32 = "FAT32";
    HPFS386 = "HPFS386";
    CBSIFS = "CBSIFS";
    NDFS32 = "NDFS32";
    RAMFS = "RAMFS";
    NTFS = "NTFS";
    WC_OBJECTWINDOW    =  "WC_OBJECTWINDOW";
    WC_BUBBLE          =  "WC_BUBBLE";
    WC_TOOLBUTTONS     =  "WC_TOOLBUTTONS";
    WC_DRIVEBUTTONS    =  "WC_DRIVEBUTTONS";
    WC_DIRCONTAINER    =  "WC_DIRCONTAINER";
    WC_DIRSTATUS       =  "WC_DIRSTATUS";
    WC_TREECONTAINER   =  "WC_TREECONTAINER";
    WC_TREEOPENBUTTON  =  "WC_TREEOPENBUTTON";
    WC_TREESTATUS      =  "WC_TREESTATUS";
    WC_MAINWND         =  "WC_MAINWND";
    WC_MAINWND2        =  "WC_MAINWND2";
    WC_AUTOVIEW        =  "WC_AUTOVIEW";
    WC_LED             =  "WC_LED";
    WC_COLLECTOR       =  "WC_COLLECTOR";
    WC_COLSTATUS       =  "WC_COLSTATUS";
    WC_STATUS          =  "WC_STATUS";
    WC_TOOLBACK        =  "WC_TOOLBACK";
    WC_DRIVEBACK       =  "WC_DRIVEBACK";
    WC_ARCCONTAINER    =  "WC_ARCCONTAINER";
    WC_ARCSTATUS       =  "WC_ARCSTATUS";
    WC_MLEEDITOR       =  "WC_MLEEDITOR";
    WC_INIEDITOR       =  "WC_INIEDITOR";
    WC_SEEALL          =  "WC_SEEALL";
    WC_NEWVIEW         =  "WC_NEWVIEW";
    WC_SEESTATUS       =  "WC_SEESTATUS";
    WC_VIEWSTATUS      =  "WC_VIEWSTATUS";
    WC_ERRORWND        =  "WC_ERRORWND";
    WC_MINITIME        =  "WC_MINITIME";
    WC_DATABAR         =  "WC_DATABAR";

    /* end of strings */
    memset(&RGBBLACK, 0, sizeof(RGB2));
    RGBGREY.bRed = RGBGREY.bGreen = RGBGREY.bBlue = (BYTE)204;
    RGBGREY.fcOptions = 0;
    FM3UL = *(ULONG *)FM3Str;
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

#endif // __IBMC__

VOID APIENTRY DeInitFM3DLL(ULONG why)
{
  /* cleanup */
  static CHAR s[CCHMAXPATH];
  CHAR *enddir;
  HDIR search_handle;
  ULONG num_matches;
  FILEFINDBUF3 ffb;

  StopTimer();
  StopPrinting = 1;

  if (LogFileHandle)
    fclose(LogFileHandle);

  if (fmprof) {
    PrfCloseProfile(fmprof);
    fmprof = (HINI) 0;
    if (fIniExisted) {
      DosError(FERR_DISABLEHARDERR);
      DosCopy("FM3.INI", "FM3INI.BAK", DCPY_EXISTING);
    }
  }

  if (fToolsChanged)
    save_tools(NULL);

#  ifdef __IBMC__
  _fcloseall();
# else // __WATCOMC__
  fcloseall();
# endif

  save_dir(s);
  if (s[strlen(s) - 1] != '\\')
    strcat(s, "\\");
  enddir = &s[strlen(s)];
  if (*ArcTempRoot) {
    strcat(s, ArcTempRoot);
    strcat(s, "*");
    search_handle = HDIR_CREATE;
    num_matches = 1L;
    if (!DosFindFirst(s,
		      &search_handle,
		      FILE_NORMAL | FILE_DIRECTORY |
		      FILE_SYSTEM | FILE_READONLY | FILE_HIDDEN |
		      FILE_ARCHIVED,
		      &ffb, sizeof(ffb), &num_matches, FIL_STANDARD)) {
      do {
	strcpy(enddir, ffb.achName);
	if (ffb.attrFile & FILE_DIRECTORY) {
	  wipeallf("%s\\*", s);
	  DosDeleteDir(s);
	}
	else
	  unlinkf("%s", s);
      }
      while (!DosFindNext(search_handle,
			  &ffb, sizeof(ffb), &num_matches));
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
  num_matches = 1;
  if (!DosFindFirst(s,
		    &search_handle,
		    FILE_NORMAL | FILE_DIRECTORY |
		    FILE_SYSTEM | FILE_READONLY | FILE_HIDDEN |
		    FILE_ARCHIVED,
		    &ffb, sizeof(ffb), &num_matches, FIL_STANDARD)) {
    do {
      if (!(ffb.attrFile & FILE_DIRECTORY)) {
	strcpy(enddir, ffb.achName);
	unlinkf("%s", s);
      }
    }
    while (!DosFindNext(search_handle,
			&ffb, sizeof(ffb), &num_matches));
    DosFindClose(search_handle);
  }

  DosForceDelete("$FM2PLAY.$$$");

  EndNote();

  if (FM3ModHandle)
    DosFreeModule(FM3ModHandle);

  DosExitList(EXLST_REMOVE, DeInitFM3DLL);
}

BOOL InitFM3DLL(HAB hab, int argc, char **argv)
{
  /*
   * this function should be called by any application using this DLL right
   * after setting up a message queue
   */

  CLASSINFO clinfo;
  APIRET rc;
  APIRET rcl = 1;
  APIRET rcq = 1;
  PFN pfnResVersion = (PFN)NULL;
  ULONG RVMajor = 0;
  ULONG RVMinor = 0;
  ULONG ret = 0;
  FILESTATUS3 fs3;			// 25 Aug 07 SHL
  PSZ env;
  CHAR dllfile[CCHMAXPATH];
  ULONG size;

  if (!StringsLoaded()) {
    saymsg(MB_ENTER,
	   HWND_DESKTOP,
	   "Error",
	   "FM3RES.STR isn't in right format, at least "
	   "for this version of FM/2.");
    return FALSE;
  }

  strcpy(dllfile, "FM3RES");
  env = getenv("FM3INI");
  if (env) {
    DosError(FERR_DISABLEHARDERR);
    rc = DosQueryPathInfo(env, FIL_STANDARD, &fs3, sizeof(fs3));
    if (!rc) {
      if (fs3.attrFile & FILE_DIRECTORY) {
	BldFullPathName(dllfile, env, "FM3RES");	// 23 Aug 07 SHL
	DosError(FERR_DISABLEHARDERR);
	if (DosQueryPathInfo(dllfile, FIL_STANDARD, &fs3, sizeof(fs3)))
	  strcpy(dllfile, "FM3RES");
      }
    }
  }
  rcl = DosLoadModule(NULL, 0, dllfile, &FM3ModHandle);
  if (rcl) {
    saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
	   HWND_DESKTOP,
	   GetPString(IDS_ERRORTEXT), GetPString(IDS_FM3RESERROR1TEXT));
    return FALSE;
  }
  else {
    rc = DosExitList(EXLST_ADD, DeInitFM3DLL);
    if (rc) {
      Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	       "DosExitList");
    }
    rcq = DosQueryProcAddr(FM3ModHandle, 1, "ResVersion", &pfnResVersion);
    if (!rcq)
      ret = pfnResVersion(&RVMajor, &RVMinor);
  }

  if (RVMajor < VERMAJOR || (RVMajor == VERMAJOR && RVMinor < VERMINOR)) {
    saymsg(MB_ENTER,
	   HWND_DESKTOP,
	   GetPString(IDS_ERRORTEXT),
	   GetPString(IDS_FM3RESERROR2TEXT),
	   !rcq ?
	   GetPString(IDS_FM3RESERROR3TEXT) :
	   !rcl ?
	   GetPString(IDS_FM3RESERROR4TEXT) :
	   GetPString(IDS_FM3RESERROR5TEXT), RVMajor, RVMinor, rcl, rcq, ret);
    return FALSE;
  }

  ArgDriveFlags(argc, argv);
  FillInDriveFlags(NULL);

  if (!*profile)
    strcpy(profile, "FM3.INI");
  mypid = getpid();
  /* give default appname if none set by caller */
  if (!*appname)
    strcpy(appname, FM3Str);
  /* save appname; may be reset below */
  strcpy(realappname, appname);
  if (!strcmp(appname, FM3Str))
    DosSetMaxFH(100);
  else if (!strcmp(appname, "VDir") ||
	   !strcmp(appname, "VTree") ||
	   !strcmp(appname, "VCollect") ||
	   !strcmp(appname, "SEEALL") || !strcmp(appname, "FM/4"))
    DosSetMaxFH(60);
  else
    DosSetMaxFH(40);

  if (DosQuerySysInfo(QSV_VERSION_MAJOR,
		      QSV_VERSION_MINOR,
		      OS2ver,
		      sizeof(OS2ver))) {
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
  if (_beginthread(HeapThread, NULL, 32768, NULL) == -1) {
    Runtime_Error(pszSrcFile, __LINE__,
		  GetPString(IDS_COULDNTSTARTTHREADTEXT));
    return FALSE;
  }
  /* timer messages are sent from a separate thread -- start it */
  if (!StartTimer()) {
    Runtime_Error(pszSrcFile, __LINE__,
		  GetPString(IDS_COULDNTSTARTTHREADTEXT));
    return FALSE;
  }

  /* Are we the workplace shell? */
  env = getenv("WORKPLACE_PROCESS");
  fWorkPlace = env != NULL &&
	       (stricmp(env, "YES") == 0 || atoi(env) == 1);

  if ((!strchr(profile, '\\') && !strchr(profile, ':')) ||
      !(fmprof = PrfOpenProfile((HAB)0, profile)))
  {
    /* figure out where to put INI file... */
    CHAR inipath[CCHMAXPATH];
    // PSZ env;

    DosError(FERR_DISABLEHARDERR);
    save_dir2(HomePath);
    DosError(FERR_DISABLEHARDERR);
    memset(driveserial, -1, sizeof(driveserial));
    *inipath = 0;
    env = getenv("FM3INI");
    if (env) {
      strcpy(inipath, env);
      DosError(FERR_DISABLEHARDERR);
      rc = DosQueryPathInfo(inipath, FIL_STANDARD, &fs3, sizeof(fs3));
      if (!rc) {
	if (fs3.attrFile & FILE_DIRECTORY)
	  BldFullPathName(inipath, inipath, profile);
      }
    }
    if (!env) {
      env = searchpath(profile);
      if (!env)
	env = profile;
      strcpy(inipath, env);
    }

    /* in some odd cases the INI file can get set to readonly status */
    /* here we test it and reset the readonly bit if necessary */
    if (!*inipath)
      strcpy(inipath, profile);
    DosError(FERR_DISABLEHARDERR);

    if (!DosQueryPathInfo(inipath, FIL_STANDARD, &fs3, sizeof(fs3))) {
      fIniExisted = TRUE;
      if (fs3.attrFile & (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM)) {
	fs3.attrFile &= ~(FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM);
	rc = xDosSetPathInfo(inipath, FIL_STANDARD, &fs3, sizeof(fs3), 0);
	if (rc) {
	  Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		      GetPString(IDS_INIREADONLYTEXT), inipath);
	  }

      }
    }

    fmprof = PrfOpenProfile((HAB)0, inipath);
    if (!fmprof) {
      strcpy(inipath, "FM3.INI");
      fmprof = PrfOpenProfile((HAB)0, inipath);
    }

    if (!fmprof) {
      Win_Error(NULLHANDLE, NULLHANDLE, pszSrcFile, __LINE__,
		"PrfOpenProfile");
      return FALSE;
    }
  }

  FindSwapperDat();

  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof,
		      FM3Str,
		      "SeparateParms",
		      &fSeparateParms,
		      &size);
  if (!fSeparateParms)
    strcpy(appname, FM3Str);

  /* start help */
  memset(&hini, 0, sizeof(HELPINIT));
  hini.cb = sizeof(HELPINIT);
  hini.ulReturnCode = 0;
  hini.pszTutorialName = NULL;
  hini.phtHelpTable = (PHELPTABLE) MAKELONG(ID_HELPTABLE, 0xffff);
  hini.hmodAccelActionBarModule = (HMODULE) 0;
  hini.idAccelTable = 0;
  hini.idActionBar = 0;
  hini.pszHelpWindowTitle = GetPString(IDS_FM2HELPTITLETEXT);
  hini.fShowPanelId = CMIC_HIDE_PANEL_ID;
  hini.pszHelpLibraryName = "FM3.HLP";
  hwndHelp = WinCreateHelpInstance(hab, &hini);
  if (!hwndHelp) {
    static CHAR helppath[CCHMAXPATH];	// fixme to be local?

    env = getenv("FM3INI");
    if (env) {
      strcpy(helppath, env);
      DosError(FERR_DISABLEHARDERR);
      rc = DosQueryPathInfo(helppath, FIL_STANDARD, &fs3, sizeof(fs3));
      if (!rc) {
	if (fs3.attrFile & FILE_DIRECTORY) {
	  BldFullPathName(helppath, helppath, "FM3.HLP");
	  hini.pszHelpLibraryName = helppath;
	  hwndHelp = WinCreateHelpInstance(hab, &hini);
	}
      }
    }
  }
  if (!hwndHelp) {
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
  if (!PFNWPCnr || !PFNWPFrame || !PFNWPButton || !PFNWPStatic || !PFNWPMLE) {
    Runtime_Error(pszSrcFile, __LINE__, "WinQueryClassInfo");
    return FALSE;
  }

  /* register window classes we use */
  WinRegisterClass(hab,
		   WC_MAINWND,
		   MainWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID) * 8);
  WinRegisterClass(hab,
		   WC_MAINWND2,
		   MainWndProc2,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID) * 4);
  WinRegisterClass(hab,
		   WC_TREECONTAINER,
		   TreeClientWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   WC_DIRCONTAINER,
		   DirClientWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   WC_COLLECTOR,
		   CollectorClientWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   WC_ARCCONTAINER,
		   ArcClientWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   WC_MLEEDITOR,
		   MLEEditorProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   WC_INIEDITOR,
		   IniProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   WC_TOOLBACK,
		   ToolBackProc, CS_SIZEREDRAW, sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_DRIVEBACK,
		   DriveBackProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_SEEALL,
		   SeeAllWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_NEWVIEW,
		   ViewWndProc,
		   CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_TOOLBUTTONS,
		   ChildButtonProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_DRIVEBUTTONS,
		   DriveProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_BUBBLE,
		   BubbleProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG) * 2);
  WinRegisterClass(hab,
		   WC_STATUS,
		   StatusProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   WC_DIRSTATUS,
		   DirTextProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   WC_TREESTATUS,
		   TreeStatProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   WC_ARCSTATUS,
		   ArcTextProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   WC_COLSTATUS,
		   CollectorTextProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   WC_SEESTATUS,
		   SeeStatusProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   WC_VIEWSTATUS,
		   ViewStatusProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(ULONG));
  WinRegisterClass(hab,
		   WC_ERRORWND,
		   NotifyWndProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_MINITIME,
		   MiniTimeProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID) * 2);
  WinRegisterClass(hab,
		   WC_DATABAR,
		   DataProc, CS_SIZEREDRAW, sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_TREEOPENBUTTON,
		   OpenButtonProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_AUTOVIEW,
		   AutoViewProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));
  WinRegisterClass(hab,
		   WC_LED,
		   LEDProc,
		   CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP,
		   sizeof(PVOID));

  /*
   * set some defaults (note: everything else automatically initialized
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
#if 0 // 06 Oct 07 SHL fixme to be gone after wrapper testing finished
    fNoLargeFileSupport = TRUE;
#endif
  ulCnrType = CCS_EXTENDSEL;
  FilesToGet = FILESTOGET_MIN;
  AutoviewHeight = 48;
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

  //Get default Country info
  {
  COUNTRYCODE Country    = {0};
  ULONG ulInfoLen  = 0;
  COUNTRYINFO CtryInfo   = {0};

  DosQueryCtryInfo(sizeof(CtryInfo), &Country,
                   &CtryInfo, &ulInfoLen);
  *ThousandsSeparator = CtryInfo.szThousandsSeparator[0];
  }

  // load preferences from profile (INI) file
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "ShowTarget", &fShowTarget, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "CheckMM", &fCheckMM, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "ChangeTarget", &fChangeTarget, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "ConfirmTarget", &fConfirmTarget, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "CustomFileDlg", &fCustomFileDlg, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "SaveMiniCmds", &fSaveMiniCmds, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "SaveBigCmds", &fSaveBigCmds, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "NoFoldMenu", &fNoFoldMenu, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "ThreadNotes", &fThreadNotes, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "Prnpagenums", &prnpagenums, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "Prnalt", &prnalt, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "Prnformat", &prnformat, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "Prnformfeedbefore",
		      &prnformfeedbefore, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str,
		      "Prnformfeedafter",&prnformfeedafter, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "Prntabspaces", &prntabspaces, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "Prnwidth", &prnwidth, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "Prnlength", &prnlength, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "Prntmargin", &prntmargin, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "Prnbmargin", &prnbmargin, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "Prnlmargin", &prnlmargin, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "Prnrmargin", &prnrmargin, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "Prnspacing", &prnspacing, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "NoDead", &fNoDead, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "NoFinger", &fNoFinger, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "SwitchTree", &fSwitchTree, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "SwitchTreeExpand",
		      &fSwitchTreeExpand, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "SwitchTreeOnFocus",
		      &fSwitchTreeOnFocus, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "CollapseFirst",
		      &fCollapseFirst, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "FilesInTree",
		      &fFilesInTree, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "TopDir", &fTopDir, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "LookInDir", &fLookInDir, &size);
  PrfQueryProfileString(fmprof, appname, "DefArc", NULL, szDefArc,
			sizeof(szDefArc));
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "AutoviewHeight",
		      &AutoviewHeight, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "KeepCmdLine", &fKeepCmdLine, &size);
  if (strcmp(realappname, "FM/4")) {
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "MoreButtons", &fMoreButtons, &size);
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, FM3Str, "Drivebar", &fDrivebar, &size);
  }
  else
    fDrivebar = fMoreButtons = TRUE;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "NoSearch", &fNoSearch, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "GuessType", &fGuessType, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "ViewChild", &fViewChild, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "ShowEnv", &fShowEnv, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "LeaveTree", &fLeaveTree, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "Comments", &fComments, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, appname, "WS_ANIMATE", &fwsAnimate, &size);
  if (fwsAnimate)
    fwsAnimate = WS_ANIMATE;
  size = sizeof(ULONG);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "SelectedAlways",
		      &fSelectedAlways, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "ToolbarHelp", &fToolbarHelp, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "OtherHelp", &fOtherHelp, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "DrivebarHelp", &fDrivebarHelp, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "AutoAddDirs", &fAutoAddDirs, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname,
		      "AutoAddAllDirs", &fAutoAddAllDirs, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "UserListSwitches",
		      &fUserListSwitches, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "UseNewViewer",
		      &fUseNewViewer, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DefaultDeletePerm",
		      &fDefaultDeletePerm, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "ExternalINIs",
		      &fExternalINIs, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "ExternalCollector",
		      &fExternalCollector, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "ExternalArcboxes",
		      &fExternalArcboxes, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "ExternalViewer",
		      &fExternalViewer, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "UseQProcStat",
		      &fUseQProcStat, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "UseQSysState",
		      &fUseQSysState, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "DataMin", &fDataMin, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DataToFore", &fDataToFore, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DataShowDrives",
		      &fDataShowDrives, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DataInclRemote",
		      &fDataInclRemote, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "SplitStatus", &fSplitStatus, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "FolderAfterExtract",
		      &fFolderAfterExtract, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "DullDatabar", &fDullMin, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "BlueLED", &fBlueLED, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "ConfirmDelete",
		      &fConfirmDelete, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "SaveState", &fSaveState, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "SyncUpdates", &fSyncUpdates, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "LoadSubject", &fLoadSubject, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "UnHilite", &fUnHilite, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "TileBackwards", &fTileBackwards, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "LoadLongname", &fLoadLongnames, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "VerifyWrites", &fVerify, &size);
  DosSetVerify(fVerify);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DontMoveMouse", &fDontMoveMouse, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "NoIconsFiles", &fNoIconsFiles, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "NoIconsDirs", &fNoIconsDirs, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "ForceUpper", &fForceUpper, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "ForceLower", &fForceLower, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "TextTools", &fTextTools, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "ToolTitles", &fToolTitles, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DoubleClickOpens", &fDCOpens, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "LinkSetsIcon", &fLinkSetsIcon, &size);
  size = sizeof(INT);
  PrfQueryProfileData(fmprof, appname, "Sort", &sortFlags, &size);
  size = sizeof(INT);
  PrfQueryProfileData(fmprof, appname, "TreeSort", &TreesortFlags, &size);
  size = sizeof(INT);
  PrfQueryProfileData(fmprof, appname,
		      "CollectorSort", &CollectorsortFlags, &size);
  size = sizeof(targetdir);
  PrfQueryProfileData(fmprof, appname, "Targetdir", targetdir, &size);
  if (!IsValidDir(targetdir))
    *targetdir = 0;
  size = sizeof(extractpath);
  PrfQueryProfileData(fmprof, appname, "ExtractPath", extractpath, &size);
  //if (!IsValidDir(extractpath))
  //  *extractpath = 0;
  size = sizeof(printer);
  PrfQueryProfileData(fmprof, appname, "Printer", printer, &size);
  size = sizeof(dircompare);
  PrfQueryProfileData(fmprof, appname, "DirCompare", dircompare,
		      &size);
  size = sizeof(viewer);
  PrfQueryProfileData(fmprof, appname, "Viewer", viewer, &size);
  size = sizeof(editor);
  PrfQueryProfileData(fmprof, appname, "Editor", editor, &size);
  size = sizeof(binview);
  PrfQueryProfileData(fmprof, appname, "BinView", binview, &size);
  size = sizeof(bined);
  PrfQueryProfileData(fmprof, appname, "BinEd", bined, &size);
  size = sizeof(compare);
  PrfQueryProfileData(fmprof, appname, "Compare", compare, &size);
  size = sizeof(virus);
  PrfQueryProfileData(fmprof, appname, "Virus", virus, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "FtpRunWPSDefault", &fFtpRunWPSDefault, &size);
  size = sizeof(ftprun);
  PrfQueryProfileData(fmprof, appname, "FTPRun", ftprun, &size);
  if (!*ftprun){
    fFtpRunWPSDefault = TRUE;
  }
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "HttpRunWPSDefault", &fHttpRunWPSDefault, &size);
  size = sizeof(httprun);
  PrfQueryProfileData(fmprof, appname, "HTTPRun", httprun, &size);
  if (!*httprun){
    fHttpRunWPSDefault = TRUE;
  }
  size = sizeof(mailrun);
  PrfQueryProfileData(fmprof, appname, "MailRun", mailrun, &size);
  size = sizeof(ftprundir);
  PrfQueryProfileData(fmprof, appname, "FtpRunDir", ftprundir, &size);
  size = sizeof(httprundir);
  PrfQueryProfileData(fmprof, appname, "HttpRunDir", httprundir, &size);
  size = sizeof(mailrundir);
  PrfQueryProfileData(fmprof, appname, "MailRunDir", mailrundir, &size);
  size = sizeof(lasttoolbox);
  PrfQueryProfileData(fmprof, FM3Str, "LastToolBox", lasttoolbox,
                      &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "LibPathStrictHttpRun", &fLibPathStrictHttpRun,
                      &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "LibPathStrictFtpRun", &fLibPathStrictFtpRun,
                      &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "LibPathStrictMailRun", &fLibPathStrictMailRun,
                      &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "NoMailtoMailRun", &fNoMailtoMailRun,
		      &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "FollowTree", &fFollowTree,
		      &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "StartMaximized",
		      &fStartMaximized, &size);
  if (!fStartMaximized) {
    size = sizeof(BOOL);
    PrfQueryProfileData(fmprof, appname, "StartMinimized",
			&fStartMinimized, &size);
  }
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DefaultCopy", &fCopyDefault, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "IdleCopy", &fRealIdle, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "ArcStuffVisible",
		      &fArcStuffVisible, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "NoTreeGap", &fNoTreeGap, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "VTreeOpensWPS",
		      &fVTreeOpensWPS, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "RemoteBug", &fRemoteBug, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "Drag&DropDlg",
		      &fDragndropDlg, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "UserComboBox", &fUserComboBox, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "MinDirOnOpen", &fMinOnOpen, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "QuickArcFind",
		      &fQuickArcFind, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "NoRemovableScan",
		      &fNoRemovableScan, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, FM3Str, "NoBrokenNotify",
		      &NoBrokenNotify, &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, appname, "ContainerType", &ulCnrType,
		      &size);
  size = sizeof(ULONG);
  PrfQueryProfileData(fmprof, appname, "FilesToGet", &FilesToGet, &size);
  if (FilesToGet < FILESTOGET_MIN)
    FilesToGet = FILESTOGET_MIN;
  else if (FilesToGet > FILESTOGET_MAX)
    FilesToGet = FILESTOGET_MAX;
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "AutoView", &fAutoView, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, FM3Str, "FM2Deletes", &fFM2Deletes, &size);

  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsLWDate", &detailslwdate, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsLWTime", &detailslwtime, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsLADate", &detailsladate, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsLATime", &detailslatime, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsCRDate", &detailscrdate, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsCRTime", &detailscrtime, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsLongname", &detailslongname, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsEA", &detailsea, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsSize", &detailssize, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsSubject", &detailssubject, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsAttr", &detailsattr, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "DetailsIcon", &detailsicon, &size);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "SubjectInLeftPane", &fSubjectInLeftPane,
		      &size);
  size = sizeof(ULONG);
  size = sizeof(BOOL);
  PrfQueryProfileData(fmprof, appname, "SubjectLengthMax", &fSubjectLengthMax,
                      &size);
  if (fSubjectLengthMax)
    SubjectDisplayWidth = 0;
  else {
    PrfQueryProfileData(fmprof, appname, "SubjectDisplayWidth",
              	        &SubjectDisplayWidth, &size);
    if (SubjectDisplayWidth < 50)
      SubjectDisplayWidth = 0;
    else if (SubjectDisplayWidth > 1000)
      SubjectDisplayWidth = 1000;
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
  hptrVirtual = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, VIRTUAL_ICON);
  hptrRamdisk = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, RAMDISK_ICON);
  if (!fNoDead)
    hptrFinger = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FINGER_ICON);
  else
    hptrFinger = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FINGER2_ICON);
  hptrApp = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, APP_ICON);
  hptrDunno = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, DUNNO_ICON);
  hptrEnv = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ENV_ICON);
  hptrZipstrm = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, ZIPSTREAM_ICON);

  // set up color array used by seeall.c and newview.c color dialog

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
  HWND hwndFrame;
  HWND hwndClient;
  UINT x;
  ULONG FrameFlags = FCF_TITLEBAR | FCF_SYSMENU |
    FCF_SIZEBORDER | FCF_MINMAX |
    FCF_ACCELTABLE | FCF_MENU | FCF_ICON | FCF_TASKLIST | FCF_NOBYTEALIGN;

  for (x = 1; x < argc; x++) {
    if (*argv[x] == '~' && !argv[x][1])
      fReminimize = TRUE;
    if (*argv[x] == '+' && !argv[x][1])
      fLogFile = TRUE;
    if (*argv[x] == '-') {
      if (!argv[x][1])
	fNoSaveState = TRUE;
      else
	strcpy(profile, &argv[x][1]);
    }
  }

  hwndFrame = WinCreateStdWindow(HWND_DESKTOP,
				 WS_VISIBLE,
				 &FrameFlags,
				 WC_MAINWND,
				 NULL,
				 WS_VISIBLE | WS_ANIMATE,
				 FM3ModHandle, MAIN_FRAME, &hwndClient);
  if (hwndFrame) {
    WinSetWindowUShort(hwndFrame, QWS_ID, MAIN_FRAME);
    hwndMainMenu = WinWindowFromID(hwndFrame, FID_MENU);
    if (!WinRestoreWindowPos(FM2Str, "MainWindowPos", hwndFrame)) {

      ULONG fl = SWP_MOVE | SWP_SIZE;
      RECTL rcl;
      ULONG icz = WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 3L;
      ULONG bsz = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);

      WinQueryWindowRect(HWND_DESKTOP, &rcl);
      rcl.yBottom += icz;
      rcl.yTop -= bsz;
      rcl.xLeft += bsz;
      rcl.xRight -= bsz;
      WinSetWindowPos(hwndFrame,
		      HWND_TOP,
		      rcl.xLeft,
		      rcl.yBottom,
		      rcl.xRight - rcl.xLeft, rcl.yTop - rcl.yBottom, fl);
    }
    if (fLogFile)
      LogFileHandle = _fsopen("FM2.LOG", "a+", SH_DENYWR);
    if (hwndHelp)
      WinAssociateHelpInstance(hwndHelp, hwndFrame);
    PostMsg(hwndClient, UM_SETUP, MPFROMLONG(argc), MPFROMP(argv));
  }
  return hwndFrame;
}

int CheckVersion(int vermajor, int verminor)
{
  int ok = 0;

  // fixme to do useful check - was missing in base source

#if 0
  if (vermajor && verminor) {
    *vermajor = VERMAJOR;
    *verminor = VERMINOR;
    ok = 1;
  }
#endif

  ok = 1;

  return ok;
}

#ifdef __WATCOMC__
#pragma alloc_text(INIT,LibMain,InitFM3DLL,DeInitFM3DLL)
#pragma alloc_text(INIT1,StartFM3,FindSwapperDat)
#endif
