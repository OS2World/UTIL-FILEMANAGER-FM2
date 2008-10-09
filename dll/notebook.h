
/***********************************************************************

  $Id$

  Configuration notebook

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  14 Feb 08 SHL Refactor from fm3dll.h
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  20 Jul 08 JBS Ticket 114: Support user-selectable env. strings in Tree container.

***********************************************************************/

#if !defined(NOTEBOOK_H)
#define NOTEBOOK_H

#include "dircnrs.h"		// typedef for DETAILS_SETTINGS

MRESULT EXPENTRY CfgDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

VOID CfgMenuInit(HWND hwndMenu, BOOL fIsLite);

// Data declarations
extern HWND Collector;
extern ULONG FilesToGet;
extern CHAR appname[12];
extern DETAILS_SETTINGS dsDirCnrDefault;
extern CHAR extractpath[CCHMAXPATH];
extern BOOL fAutoAddAllDirs;
extern BOOL fAutoAddDirs;
extern BOOL fBlueLED;
extern BOOL fCancelAction;
extern BOOL fCheckMM;
extern BOOL fCollapseFirst;
extern BOOL fConfirmDelete;
extern BOOL fConfirmTarget;
extern BOOL fCopyDefault;
extern BOOL fCustomFileDlg;
extern BOOL fDataMin;
extern BOOL fDontMoveMouse;
extern BOOL fDragndropDlg;
extern BOOL fDrivebarHelp;
extern BOOL fExternalArcboxes;
extern BOOL fExternalCollector;
extern BOOL fExternalINIs;
extern BOOL fExternalViewer;
extern BOOL fFM2Deletes;
extern BOOL fFolderAfterExtract;
extern BOOL fForceLower;
extern BOOL fForceUpper;
extern BOOL fGuessType;
extern BOOL fLeaveTree;
extern BOOL fLibPathStrictFtpRun;
extern BOOL fLibPathStrictHttpRun;
extern BOOL fLibPathStrictMailRun;
extern BOOL fLinkSetsIcon;
extern BOOL fLoadLongnames;
extern BOOL fLoadSubject;
extern BOOL fLookInDir;
extern BOOL fMinOnOpen;
extern BOOL fNoDead;
extern BOOL fNoFoldMenu;
extern BOOL fNoIconsDirs;
extern BOOL fNoIconsFiles;
extern BOOL fNoMailtoMailRun;
extern BOOL fNoRemovableScan;
extern BOOL fNoSearch;
extern BOOL fNoTreeGap;
extern BOOL fOtherHelp;
extern BOOL fQuickArcFind;
extern BOOL fRealIdle;
extern BOOL fRemoteBug;
extern BOOL fSaveState;
extern BOOL fSeparateParms;
extern BOOL fShowEnv;
extern BOOL fShowTarget;
extern BOOL fStartMaximized;
extern BOOL fStartMinimized;
extern BOOL fSwitchTree;
extern BOOL fSwitchTreeExpand;
extern BOOL fSwitchTreeOnFocus;
extern BOOL fSyncUpdates;
extern BOOL fTileBackwards;
extern BOOL fToolbarHelp;
extern BOOL fTrashCan;
extern BOOL fUnHilite;
extern BOOL fUseNewViewer;
extern BOOL fUserListSwitches;
extern BOOL fVTreeOpensWPS;
extern BOOL fVerify;
extern BOOL fViewChild;
extern HINI fmprof;
extern ULONG fwsAnimate;
extern HWND hwndHelp;
extern HWND hwndMain;
extern CHAR szDefArc[CCHMAXPATH];
extern ULONG ulCnrType;
extern CHAR *bined;
extern CHAR *binview;
extern CHAR *compare;
extern CHAR *dircompare;
extern CHAR *editor;
extern CHAR *ftprun;
extern CHAR ftprundir[CCHMAXPATH];
extern CHAR httprundir[CCHMAXPATH];
extern CHAR mailrundir[CCHMAXPATH];
extern CHAR *pszTreeEnvVarList;
extern CHAR *viewer;
extern CHAR *virus;

#endif // NOTEBOOK_H
