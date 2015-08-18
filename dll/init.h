
/***********************************************************************

  $Id$

  Initialization

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2015 Steven H. Levine

  Change log
  29 Nov 08 GKY Remove or replace with a mutex semaphore DosEnterCriSec where appropriate.
  03 Jan 09 GKY Check for system that is protectonly to gray out Dos/Win command lines and prevent
                Dos/Win programs from being inserted into the execute dialog with message why.
  11 Jan 09 GKY Move strings that shouldn't be translated (font names etc) compile time variables
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  07 Feb 09 GKY Move repeated strings to PCSZs.
  07 Feb 09 GKY Add *DateFormat functions to format dates based on locale
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  22 Jul 09 GKY Drivebar enhancements add refresh removable, rescan all drives, drive button
                loads drive root directory in directory container or expands drive tree
                and rescans drive in tree container depending on container focus, greyed out
                inappropriate menu context choices
  15 Nov 09 GKY Add more PCSZs
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10).
                Mostly cast CHAR CONSTANT * as CHAR *.
  20 Nov 10 GKY Rework scanning code to remove redundant scans, prevent double directory
                entries in the tree container, fix related semaphore performance using
                combination of event and mutex semaphores
  08 Jan 12 GKY Add support for changing PresParams in the notify status window
  04 Aug 12 GKY Changes to use Unlock to unlock files if Unlock.exe is in path both from menu/toolbar and as part of
                copy, move and delete operations
  04 Aug 12 GKY Changes to allow copy and move over readonly files with a warning dialog; also added a warning dialog
                for delete of readonly files
  16 Feb 14 GKY Add "#" command line switch to workaround problem with blank command shell
                started from fm2 after fm2 has been started with stdout and stderr
                redirected to a file.
  02 Mar 14 GKY Fixed typo that reversed the function of the saymsg dialog g/bzip check.
                Added option to suppress message regarding missing bzip2.exe
                or gzip.exe on TAR.B/GZ archives.
  30 Aug 14 GKY Add semaphore hmtxFiltering to prevent freeing dcd while filtering. Prevents
                a trap when FM2 is shutdown while directory containers are still populating
  02 Aug 15 GKY Serialize local hard drive scanning to reduce drive thrashing continue to scan
                all other drive types in separate threads.

***********************************************************************/

#if !defined(INIT_H)
#define INIT_H

BOOL InitFM3DLL(HAB hab, int argc, char **argv);
HWND StartFM3(HAB hab, INT argc, CHAR ** argv);

// Data declarations
extern PCSZ PCSZ_ARCHIVERBB2;
extern PCSZ PCSZ_ASSOCDAT;
extern PCSZ PCSZ_CMDLINESDAT;
extern PCSZ PCSZ_CMDMINIDAT;
extern PCSZ PCSZ_COMMANDSDAT;
extern PCSZ PCSZ_FILTERSDAT;
extern PCSZ PCSZ_GREPMASKDAT;
extern PCSZ PCSZ_PATTERNSDAT;
extern PCSZ PCSZ_RESOURCEDAT;
extern PCSZ PCSZ_QUICKTLSDAT;
extern PCSZ PCSZ_FM3TOOLSDAT;
extern PCSZ PCSZ_USERDIRSDAT;
extern PCSZ PCSZ_FM2PLAYEXE;
extern PCSZ PCSZ_FM2PLAYTEMP;
extern PCSZ PCSZ_LVMGUICMD;
extern PCSZ PCSZ_DFSOS2EXE;
extern PCSZ PCSZ_MINILVMEXE;
extern PCSZ PCSZ_FDISKPMEXE;
extern PCSZ PCSZ_IMAGEEXE;
extern PCSZ PCSZ_LVMEXE;
extern PCSZ PCSZ_UNLOCKEXE;
extern PCSZ PCSZ_ARCCNR;
extern PCSZ PCSZ_COLLECTOR;
extern PCSZ PCSZ_DIRCNR;
extern PCSZ PCSZ_DIRCMP;
extern PCSZ PCSZ_DIRSIZES;
extern PCSZ PCSZ_NOTIFYWND;
extern PCSZ PCSZ_TOOLBAR;
extern PCSZ PCSZ_TREECNR;
extern PCSZ PCSZ_SHUTDOWNSTATE;
extern PCSZ PCSZ_FM2TEMPTEXT;
extern PCSZ DRMDRFFM2ARC;
extern PCSZ DRF_FM2ARCHIVE;
extern PCSZ DRMDRFOS2FILE;
extern PCSZ DRM_FM2ARCMEMBER;
extern PCSZ DRM_OS2FILE;
extern PCSZ FM2Str;
extern PCSZ FM3Folder;
extern CHAR HomePath[CCHMAXPATH];
extern PCSZ LONGNAME;
extern CHAR *NullStr;
extern ULONG OS2ver[2];
extern PFNWP PFNWPCnr;
extern PFNWP PFNWPMLE;
extern PCSZ Settings;
extern PCSZ Details;
extern PCSZ PCSZ_TREE;
extern PCSZ PCSZ_ICON;
extern CHAR SwapperDat[CCHMAXPATH];
extern CHAR ThousandsSeparator[2];
extern CHAR DateSeparator[2];
extern CHAR TimeSeparator[2];
extern ULONG ulTimeFmt;
extern ULONG ulDateFmt;
extern PCSZ FNT_HELVETICA;
extern PCSZ FNT_6HELVETICA;
extern PCSZ FNT_8HELVETICA;
extern PCSZ FNT_8HELVETICABOLD;
extern PCSZ FNT_10SYSTEMMONOTEXT;
extern PCSZ FNT_10SYSTEMPROPORT;
extern PCSZ FNT_2SYSTEMVIO;
extern PCSZ FNT_4SYSTEMVIO;
extern PCSZ FNT_10SYSTEMVIO;
extern PCSZ FNT_8TIMESNEWROMAN;
extern PCSZ PCSZ_CM_ALLOCRECORD;
extern PCSZ PCSZ_QUERYCNRINFO;
extern PCSZ PCSZ_DOSCREATEMUTEXSEM;
extern PCSZ PCSZ_DOSCREATEEVENTSEM;
extern PCSZ PCSZ_DOSDUPHANDLE;
extern PCSZ PCSZ_DOSGETINFOBLOCKS;
extern PCSZ PCSZ_DOSQUERYPATHINFO;
extern PCSZ PCSZ_DOSSEARCHPATH;
extern PCSZ PCSZ_DRGACCESSDRAGINFO;
extern PCSZ PCSZ_POSTMSG;
extern PCSZ PCSZ_WINCREATEWINDOW;
extern PCSZ PCSZ_WINLOADACCELTABLE;
extern PCSZ PCSZ_WINSETACCELTABLE;
extern PCSZ PCSZ_INIQUERYPRFTEXT;
extern PCSZ PCSZ_PRFOPENPROFILEFAILED;
extern PCSZ PCSZ_PRFQUERYPROFILEDATA;
extern PCSZ PCSZ_PRFQUERYPROFILESIZE;
extern PCSZ PCSZ_PRFQUERYPROFILESTRING;
extern PCSZ PCSZ_FILLDIRQCURERRTEXT;
extern PCSZ PCSZ_NODATA;
extern PCSZ PCSZ_BACKSLASH;
extern PCSZ PCSZ_STARDOTEXE;
extern PCSZ PCSZ_STARDOTINI;
extern PCSZ PCSZ_STARDOTLST;
extern PCSZ PCSZ_STARDOTPMD;
extern PCSZ PCSZ_STARDOTTXT;
extern PCSZ PCSZ_FM3DOTINI;
extern PCSZ PCSZ_FM3INI;
extern PCSZ PCSZ_FM3INIDOTBAK;
extern PCSZ PCSZ_FM3INIDOTBAD;
extern PCSZ PCSZ_FM3INIDOTBAD2;
extern PCSZ PCSZ_FM3RES;
extern PCSZ PCSZ_FM3DOTHLP;
extern PCSZ PCSZ_DOTEXE;
extern PCSZ PCSZ_DOTCOM;
extern PCSZ PCSZ_DOTCMD;
extern PCSZ PCSZ_DOTBTM;
extern PCSZ PCSZ_DOTBAT;
extern PCSZ PCSZ_DOTLST;
extern PCSZ PCSZ_DOTTLS;
extern PCSZ PCSZ_DOTHLP;
extern PCSZ PCSZ_DOTINF;
extern PCSZ PCSZ_DOTMP3;
extern PCSZ PCSZ_DOTMPG;
extern PCSZ PCSZ_DOTMPEG;
extern PCSZ PCSZ_DOTOGG;
extern PCSZ PCSZ_DOTFLAC;
extern PCSZ PCSZ_DOTJPEG;
extern PCSZ PCSZ_DOTJPG;
extern PCSZ PCSZ_DOTICO;
extern PCSZ PCSZ_DOTPTR;
extern PCSZ PCSZ_DOTBB2;
extern PCSZ PCSZ_DOTBMP;
extern PCSZ PCSZ_DOTCOMMENTS;
extern PCSZ PCSZ_PATH;
extern PCSZ PCSZ_LIBPATH;
extern PCSZ PCSZ_WPURLDEFAULTSETTINGS;
extern PCSZ WC_ARCCONTAINER;
extern PCSZ WC_ARCSTATUS;
extern PCSZ WC_AUTOVIEW;
extern PCSZ WC_BUBBLE;
extern PCSZ WC_COLLECTOR;
extern PCSZ WC_COLSTATUS;
extern PCSZ WC_DATABAR;
extern PCSZ WC_DIRCONTAINER;
extern PCSZ WC_DIRSTATUS;
extern PCSZ WC_DRIVEBACK;
extern PCSZ WC_DRIVEBUTTONS;
extern PCSZ WC_ERRORWND;
extern PCSZ WC_INIEDITOR;
extern PCSZ WC_LED;
extern PCSZ WC_MAINWND;
extern PCSZ WC_MAINWND2;
extern PCSZ WC_MINITIME;
extern PCSZ WC_MLEEDITOR;
extern PCSZ WC_NEWVIEW;
extern PCSZ WC_OBJECTWINDOW;
extern PCSZ WC_SEEALL;
extern PCSZ WC_SEESTATUS;
extern PCSZ WC_STATUS;
extern PCSZ WC_TOOLBACK;
extern PCSZ WC_TOOLBUTTONS;
extern PCSZ WC_TREECONTAINER;
extern PCSZ WC_TREEOPENBUTTON;
extern PCSZ WC_TREESTATUS;
extern PCSZ WC_VIEWSTATUS;
extern BOOL fDontSuggestAgain;
extern BOOL fInitialDriveScan;
extern BOOL fAmAV2;
extern BOOL fChangeTarget;
extern BOOL fIniExisted;
extern BOOL fLogFile;
extern BOOL fProtectOnly;
extern BOOL fReminimize;
extern BOOL fWantFirstTimeInit;
extern BOOL fUseShellEnv;
extern BOOL fDontAskBzip;
extern BOOL fDontAskGzip;
extern HPOINTER hptrApp;
extern HPOINTER hptrArc;
extern HPOINTER hptrArrow;
extern HPOINTER hptrArt;
extern HPOINTER hptrBusy;
extern HPOINTER hptrCDROM;
extern HPOINTER hptrDrive;
extern HPOINTER hptrEW;
extern HPOINTER hptrFloppy;
extern HPOINTER hptrNS;
extern HPOINTER hptrRamdisk;
extern HPOINTER hptrRemote;
extern HPOINTER hptrRemovable;
extern HPOINTER hptrVirtual;
extern HPOINTER hptrZipstrm;
extern CHAR *pFM2SaveDirectory;
extern CHAR *pTmpDir;
extern CHAR profile[CCHMAXPATH];
extern CHAR *pLibPathStrict;
extern ULONGLONG ullTmpSpaceNeeded;
#ifdef INCL_DOSSEMAPHORES
extern HMTX hmtxFM2Globals;
extern HMTX hmtxFM2Delete;

#if 0 // 2015-08-07 SHL FIXME to be gone
extern HMTX hmtxScanning;
#endif // 2015-08-07 SHL FIXME to be gone

#if 0 // 2015-08-07 SHL FIXME to be gone
extern HMTX hmtxScanningLocalHD;
#endif // 2015-08-07 SHL FIXME to be gone

extern HMTX hmtxFiltering;

#if 0 // 2015-08-04 SHL FIXME to be gone
extern HEV  hevTreeCnrScanComplete;
#endif // 2015-08-04 SHL FIXME to be gone

#endif

#endif // INIT_H
