
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log
  29 Nov 08 GKY Remove or replace with a mutex semaphore DosEnterCriSec where appropriate.

***********************************************************************/

#if !defined(INIT_H)
#define INIT_H

BOOL InitFM3DLL(HAB hab, int argc, char **argv);
HWND StartFM3(HAB hab, INT argc, CHAR ** argv);

// Data declarations
extern CHAR *CBSIFS;
extern CHAR *DRMDRFFM2ARC;
extern CHAR *DRF_FM2ARCHIVE;
extern CHAR *DRMDRFOS2FILE;
extern CHAR *DRM_FM2ARCMEMBER;
extern CHAR *DRM_OS2FILE;
extern CHAR *FM2Str;
extern CHAR *FM3Folder;
extern CHAR HomePath[CCHMAXPATH];
extern CHAR *LONGNAME;
extern CHAR *NullStr;
extern ULONG OS2ver[2];
extern PFNWP PFNWPCnr;
extern PFNWP PFNWPMLE;
extern CHAR *Settings;
extern CHAR SwapperDat[CCHMAXPATH];
extern CHAR ThousandsSeparator[2];
extern CHAR *WC_ARCCONTAINER;
extern CHAR *WC_ARCSTATUS;
extern CHAR *WC_AUTOVIEW;
extern CHAR *WC_BUBBLE;
extern CHAR *WC_COLLECTOR;
extern CHAR *WC_COLSTATUS;
extern CHAR *WC_DATABAR;
extern CHAR *WC_DIRCONTAINER;
extern CHAR *WC_DIRSTATUS;
extern CHAR *WC_DRIVEBACK;
extern CHAR *WC_DRIVEBUTTONS;
extern CHAR *WC_ERRORWND;
extern CHAR *WC_INIEDITOR;
extern CHAR *WC_LED;
extern CHAR *WC_MAINWND2;
extern CHAR *WC_MINITIME;
extern CHAR *WC_MLEEDITOR;
extern CHAR *WC_NEWVIEW;
extern CHAR *WC_OBJECTWINDOW;
extern CHAR *WC_SEEALL;
extern CHAR *WC_SEESTATUS;
extern CHAR *WC_STATUS;
extern CHAR *WC_TOOLBACK;
extern CHAR *WC_TOOLBUTTONS;
extern CHAR *WC_TREECONTAINER;
extern CHAR *WC_TREEOPENBUTTON;
extern CHAR *WC_TREESTATUS;
extern CHAR *WC_VIEWSTATUS;
extern BOOL fAmAV2;
extern BOOL fChangeTarget;
extern BOOL fIniExisted;
extern BOOL fLogFile;
extern BOOL fReminimize;
extern BOOL fWantFirstTimeInit;
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
extern ULONGLONG ullTmpSpaceNeeded;
extern HMTX hmtxFM2Globals;
extern HMTX hmtxFM2Delete;
extern HEV  DriveScanStart;

#endif // INIT_H
