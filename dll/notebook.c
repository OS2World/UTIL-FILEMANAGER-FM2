
/***********************************************************************

  $Id$

  Configuration notebook

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2008 Steven H. Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  23 May 05 SHL Use QWL_USER
  04 Jun 05 SHL Support Cancel button; make Esc key more consistent
  29 May 06 SHL Comments
  17 Jul 06 SHL Use Runtime_Error
  15 Aug 06 SHL Rework SetMask args
  03 Aug 07 GKY Enlarged and made setable everywhere Findbuf (speed file loading)
  13 Aug 07 SHL Move #pragma alloc_text to end for OpenWatcom compat
  13 Aug 07 SHL Rework FilesToGet min/max to match how DosFindFirst/Next works
  19 Aug 07 SHL Sync with SaveDirCnrState mods
  21 Aug 07 GKY Make Subject column in dircnr sizable and movable from the rigth to the left pane
  26 Nov 07 GKY Allow a currently nonvalid path in the ext path field with warning
  06 Jan 08 GKY Use NormalizeCmdLine to check program strings on entry
  10 Jan 08 SHL Remember last settings page
  10 Jan 08 SHL Rework page select logic
  13 Jan 08 GKY Get Subjectwidth/Subjectleft working in the collector.
  19 Jan 08 JBS Ticket 150: fix/improve save and restore of dir cnr state at FM/2 close/reopen
  15 Feb 08 SHL Rework to support settings menu conditional cascade.  Make more generic
  16 Feb 08 SHL Restore SaveDirCnrState internal state save logic - accidentally removed
  22 Feb 08 JBS Ticket 230: Fix/improve various code related to state or presparam values in the INI file.
  29 Feb 08 GKY Changes to enable user settable command line length
  08 Mar 08 JBS Ticket 230: Replace prefixless INI keys for default directory containers with
                keys using a "DirCnr." prefix
  06 Jul 08 GKY Update delete/undelete to include move to and open XWP trashcan
  11 Jul 08 JBS Ticket 230: Simplified code and eliminated some local variables by incorporating
                all the details view settings (both the global variables and those in the
                DIRCNRDATA struct) into a new struct: DETAILS_SETTINGS.
  19 Jul 08 JBS Ticket 197: Support accelerator keys in setting dialogs.
  20 Jul 08 JBS Ticket 114: Support user-selectable env. strings in Tree container.
  31 Jul 08 JBS Ticket 114: Improved code to avoid traps.
  02 Aug 08 JBS Ticket 114: Improved code to avoid traps.

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG                   // dircnrs.h

#include "fm3dll.h"
#include "dircnrs.h"			// Data declaration(s)
#include "mainwnd2.h"			// Data declaration(s)
#include "comp.h"			// Data declaration(s)
#include "newview.h"			// Data declaration(s)
#include "datamin.h"			// Data declaration(s)
#include "arccnrs.h"			// Data declaration(s)
#include "treecnr.h"			// Data declaration(s)
#include "collect.h"			// Data declarations
#include "fm3dlg.h"
#include "fm3str.h"
#include "pathutil.h"                   // BldQuotedFileName
#include "errutil.h"                    // Dos_Error...
#include "strutil.h"                    // GetPString
#include "notebook.h"
#include "getnames.h"                   // CustomFileDlg
#include "mainwnd.h"                    // SaveDirCnrState
#include "walkem.h"			// WalkExtractDlgProc
#include "valid.h"			// MakeFullName
#include "filter.h"			// PickMaskDlgProc
#include "avl.h"			// SBoxDlgProc
#include "select.h"			// SetMask
#include "mkdir.h"			// SetTargetDir
#include "subj.h"			// Subject
#include "misc.h"                       // LoadDetailsSwitches, SetConditionalCascade
#include "strips.h"			// bstrip
#include "wrappers.h"			// xmalloc
#include "fortify.h"

static VOID SaveLastPageIndex(HWND hwnd);

// Data defintions
#pragma data_seg(GLOBAL1)
HWND Collector;
DETAILS_SETTINGS dsDirCnrDefault;
BOOL fAutoAddAllDirs;
BOOL fAutoAddDirs;
BOOL fBlueLED;
BOOL fCancelAction;
BOOL fCheckMM;
BOOL fCollapseFirst;
BOOL fConfirmDelete;
BOOL fConfirmTarget;
BOOL fCopyDefault;
BOOL fCustomFileDlg;
BOOL fDataMin;
BOOL fDontMoveMouse;
BOOL fDragndropDlg;
BOOL fDrivebarHelp;
BOOL fExternalArcboxes;
BOOL fExternalCollector;
BOOL fExternalINIs;
BOOL fExternalViewer;
BOOL fFM2Deletes;
BOOL fFolderAfterExtract;
BOOL fForceLower;
BOOL fForceUpper;
BOOL fGuessType;
BOOL fLeaveTree;
BOOL fLibPathStrictFtpRun;
BOOL fLibPathStrictHttpRun;
BOOL fLibPathStrictMailRun;
BOOL fLinkSetsIcon;
BOOL fLoadLongnames;
BOOL fLoadSubject;
BOOL fLookInDir;
BOOL fMinOnOpen;
BOOL fNoDead;
BOOL fNoFoldMenu;
BOOL fNoIconsDirs;
BOOL fNoIconsFiles;
BOOL fNoMailtoMailRun;
BOOL fNoRemovableScan;
BOOL fNoSearch;
BOOL fNoTreeGap;
BOOL fOtherHelp;
BOOL fQuickArcFind;
BOOL fRealIdle;
BOOL fRemoteBug;
BOOL fSaveState;
BOOL fSeparateParms;
BOOL fShowEnv;
BOOL fShowTarget;
BOOL fStartMaximized;
BOOL fStartMinimized;
BOOL fSwitchTree;
BOOL fSwitchTreeExpand;
BOOL fSwitchTreeOnFocus;
BOOL fSyncUpdates;
BOOL fTileBackwards;
BOOL fToolbarHelp;
BOOL fTrashCan;
BOOL fUnHilite;
BOOL fUseNewViewer;
BOOL fUserListSwitches;
BOOL fVTreeOpensWPS;
BOOL fVerify;
BOOL fViewChild;
HINI fmprof;
ULONG fwsAnimate;
HWND hwndHelp;
HWND hwndMain;

#pragma data_seg(GLOBAL2)
CHAR *pszTreeEnvVarList;
ULONG FilesToGet;
CHAR appname[12];
CHAR *bined;
CHAR *binview;
CHAR *compare;
CHAR *dircompare;
CHAR *editor;
CHAR extractpath[CCHMAXPATH];
CHAR *ftprun;
CHAR ftprundir[CCHMAXPATH];
CHAR httprundir[CCHMAXPATH];
CHAR mailrundir[CCHMAXPATH];
CHAR szDefArc[CCHMAXPATH];
ULONG ulCnrType;
CHAR *viewer;
CHAR *virus;

#pragma data_seg(DATA2)

static PSZ pszSrcFile = __FILE__;
static HWND hwndNotebook;


MRESULT EXPENTRY CfgADlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  ULONG ulResult;

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CFGA_VIRUS, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGA_EXTRACTPATH, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinEnableWindow(WinWindowFromID(hwnd, CFGA_FIND), FALSE);
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, CFGA_ARCSTUFFVISIBLE, fArcStuffVisible);
    WinCheckButton(hwnd, CFGA_FOLDERAFTEREXTRACT, fFolderAfterExtract);
    WinCheckButton(hwnd, CFGA_QUICKARCFIND, fQuickArcFind);
    WinCheckButton(hwnd, CFGA_DEFARC, (*szDefArc != 0));
    WinSetDlgItemText(hwnd, CFGA_DEFARCNAME, szDefArc);
    WinSetDlgItemText(hwnd, CFGA_VIRUS, virus);
    WinSetDlgItemText(hwnd, CFGA_EXTRACTPATH, extractpath);
    break;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFGA_VIRUS:
    case CFGA_EXTRACTPATH:
      switch (SHORT2FROMMP(mp1)) {
      case EN_KILLFOCUS:
        WinEnableWindow(WinWindowFromID(hwnd, CFGA_FIND), FALSE);
        break;
      case EN_SETFOCUS:
        WinEnableWindow(WinWindowFromID(hwnd, CFGA_FIND), TRUE);
        break;
      }
      break;
    case CFGA_DEFARC:
      switch (SHORT2FROMMP(mp1)) {
      case BN_CLICKED:
        if (WinQueryButtonCheckstate(hwnd, CFGA_DEFARC)) {

          ARC_TYPE *pat = arcsighead;   // Hide dups

          if (!WinDlgBox(HWND_DESKTOP, hwnd,
                         SBoxDlgProc, FM3ModHandle, ASEL_FRAME,
                         (PVOID) & pat) || !pat || !pat->id || !*pat->id) {
            DosBeep(250, 100);          // Complain
            WinCheckButton(hwnd, CFGA_DEFARC, FALSE);
          }
          else
            WinSetDlgItemText(hwnd, CFGA_DEFARCNAME, pat->id);
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
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGA, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case CFGA_FIND:
      {
        CHAR filename[CCHMAXPATH + 9], szfilename[CCHMAXPATH + 9];
        USHORT id;
        HWND hwndFocus;

        strcpy(filename, "*.EXE");
        hwndFocus = WinQueryFocus(HWND_DESKTOP);
        if (hwndFocus) {
          id = WinQueryWindowUShort(hwndFocus, QWS_ID);
          switch (id) {
          case CFGA_VIRUS:
            if (insert_filename(hwnd, filename, 2, FALSE) && *filename) {
              BldQuotedFileName(szfilename, filename);
              strcat(szfilename, " %p");
              WinSetDlgItemText(hwnd, id, szfilename);
            }
            break;
          case CFGA_EXTRACTPATH:
            strcpy(filename, extractpath);
            if (WinDlgBox(HWND_DESKTOP, hwndNotebook,
                          WalkExtractDlgProc, FM3ModHandle, WALK_FRAME,
                          MPFROMP(filename)) && *filename)
              WinSetDlgItemText(hwnd, id, filename);
            break;
          default:
            Runtime_Error(pszSrcFile, __LINE__, "bad case");
            break;
          }
        }
      }
      break;
    }
    return 0;

  case WM_CLOSE:
    fQuickArcFind = WinQueryButtonCheckstate(hwnd, CFGA_QUICKARCFIND);
    PrfWriteProfileData(fmprof,
                        appname,
                        "QuickArcFind", &fQuickArcFind, sizeof(BOOL));
    fArcStuffVisible = WinQueryButtonCheckstate(hwnd, CFGA_ARCSTUFFVISIBLE);
    PrfWriteProfileData(fmprof,
                        appname,
                        "ArcStuffVisible", &fArcStuffVisible, sizeof(BOOL));
    fFolderAfterExtract = WinQueryButtonCheckstate(hwnd,
                                                   CFGA_FOLDERAFTEREXTRACT);
    PrfWriteProfileData(fmprof,
                        appname,
                        "FolderAfterExtract",
                        &fFolderAfterExtract, sizeof(BOOL));
    if (WinQueryButtonCheckstate(hwnd, CFGA_DEFARC)) {

      CHAR temp[CCHMAXPATH];

      *temp = 0;
      WinQueryDlgItemText(hwnd, CFGA_DEFARCNAME, CCHMAXPATH, temp);
      strcpy(szDefArc, temp);
    }
    else
      *szDefArc = 0;
    PrfWriteProfileString(fmprof, appname, "DefArc", szDefArc);
    {
      CHAR *szCLBuf;
      CHAR szPathBuf[CCHMAXPATH];
      PSZ pszWorkBuf;

      szCLBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!szCLBuf)
          return 0; //already complained
      pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
      if (!pszWorkBuf) {
        free(szCLBuf);
        return 0; //already complained
      }
      WinQueryDlgItemText(hwnd, CFGA_VIRUS, MaxComLineStrg, szCLBuf);
      szCLBuf[MaxComLineStrg - 1] = 0;
      if (strcmp(szCLBuf, virus)){
        NormalizeCmdLine(pszWorkBuf, szCLBuf);
        memcpy(virus, pszWorkBuf, strlen(pszWorkBuf) + 1);
        if (!strchr(virus, '%') && strlen(virus) > 3)
          strcat(virus, " %p");
      }
      free(pszWorkBuf);
      free(szCLBuf);
      if (!*virus)
        strcpy(virus, "OS2SCAN.EXE %p /SUB /A");
      WinQueryDlgItemText(hwnd, CFGA_EXTRACTPATH, CCHMAXPATH, szPathBuf);
      szPathBuf[CCHMAXPATH - 1] = 0;
      bstrip(szPathBuf);
      if (strcmp(extractpath, szPathBuf)) {
        memcpy(extractpath, szPathBuf, strlen(szPathBuf) + 1);
        if (*extractpath){
          MakeFullName(extractpath);
          if (IsFile(extractpath)) {
            ulResult = saymsg(MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1, HWND_DESKTOP,
                              GetPString(IDS_WARNINGTEXT),
                              GetPString(IDS_EXTPATHNOTVALIDTEXT),
                              extractpath);
            if (ulResult == MBID_YES)
              *extractpath = 0;
            if (ulResult == MBID_CANCEL){
              WinDlgBox(HWND_DESKTOP,
                        hwnd,
                        CfgDlgProc,
                        FM3ModHandle,
                        CFG_FRAME,
                        MPFROMLONG(IDM_ARCHIVERSETTINGS));
              break;
            }
          }
        }
      }
    }
    PrfWriteProfileString(fmprof, appname, "Virus", virus);
    PrfWriteProfileString(fmprof, appname, "ExtractPath", extractpath);
    break;
  }
  if (fCancelAction) {
    fCancelAction = FALSE;
    WinDlgBox(HWND_DESKTOP,
              hwnd,
              CfgDlgProc,
              FM3ModHandle,
              CFG_FRAME,
              MPFROMLONG(IDM_ARCHIVERSETTINGS));
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgSDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CFGS_FILESTOGET, SPBM_SETTEXTLIMIT,
                      MPFROMSHORT(8), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGS_FILESTOGET, SPBM_OVERRIDESETLIMITS,
                      MPFROMLONG(FILESTOGET_MAX), MPFROMLONG(FILESTOGET_MIN));
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, CFGS_NOICONSFILES, (fNoIconsFiles == FALSE));
    WinCheckButton(hwnd, CFGS_NOICONSDIRS, (fNoIconsDirs == FALSE));
    WinCheckButton(hwnd, CFGS_LOADSUBJECTS, fLoadSubject);
    WinCheckButton(hwnd, CFGS_LOADLONGNAMES, fLoadLongnames);
    WinCheckButton(hwnd, CFGS_FORCELOWER, fForceLower);
    WinCheckButton(hwnd, CFGS_FORCEUPPER, fForceUpper);
    WinCheckButton(hwnd, CFGS_NOREMOVABLESCAN, fNoRemovableScan);
    WinCheckButton(hwnd, CFGS_REMOTEBUG, fRemoteBug);
    WinSendDlgItemMsg(hwnd, CFGS_FILESTOGET, SPBM_SETCURRENTVALUE,
                      MPFROMLONG(FilesToGet), MPVOID);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFGS_FORCEUPPER:
    case CFGS_FORCELOWER:
      {
        BOOL temp;

        temp = WinQueryButtonCheckstate(hwnd, SHORT1FROMMP(mp1));
        if (temp) {
          switch (SHORT1FROMMP(mp1)) {
          case CFGS_FORCEUPPER:
            WinCheckButton(hwnd, CFGS_FORCELOWER, FALSE);
            break;
          case CFGS_FORCELOWER:
            WinCheckButton(hwnd, CFGS_FORCEUPPER, FALSE);
            break;
          }
        }
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGS, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    fLoadLongnames = WinQueryButtonCheckstate(hwnd, CFGS_LOADLONGNAMES);
    PrfWriteProfileData(fmprof, appname, "LoadLongname", &fLoadLongnames,
                        sizeof(BOOL));
    fLoadSubject = WinQueryButtonCheckstate(hwnd, CFGS_LOADSUBJECTS);
    PrfWriteProfileData(fmprof, appname, "LoadSubject", &fLoadSubject,
                        sizeof(BOOL));
    fRemoteBug = WinQueryButtonCheckstate(hwnd, CFGS_REMOTEBUG);
    PrfWriteProfileData(fmprof, appname, "RemoteBug", &fRemoteBug,
                        sizeof(BOOL));
    fNoRemovableScan = WinQueryButtonCheckstate(hwnd, CFGS_NOREMOVABLESCAN);
    PrfWriteProfileData(fmprof, FM3Str, "NoRemovableScan", &fNoRemovableScan,
                        sizeof(BOOL));
    fNoIconsFiles = WinQueryButtonCheckstate(hwnd, CFGS_NOICONSFILES);
    fNoIconsFiles = (fNoIconsFiles) ? FALSE : TRUE;
    PrfWriteProfileData(fmprof, appname, "NoIconsFiles",
                        &fNoIconsFiles, sizeof(BOOL));
    fNoIconsDirs = WinQueryButtonCheckstate(hwnd, CFGS_NOICONSDIRS);
    fNoIconsDirs = (fNoIconsDirs) ? FALSE : TRUE;
    PrfWriteProfileData(fmprof, appname, "NoIconsDirs",
                        &fNoIconsDirs, sizeof(BOOL));
    fForceUpper = WinQueryButtonCheckstate(hwnd, CFGS_FORCEUPPER);
    PrfWriteProfileData(fmprof, appname, "ForceUpper",
                        &fForceUpper, sizeof(BOOL));
    fForceLower = WinQueryButtonCheckstate(hwnd, CFGS_FORCELOWER);
    PrfWriteProfileData(fmprof, appname, "ForceLower",
                        &fForceLower, sizeof(BOOL));
    {
      WinSendDlgItemMsg(hwnd, CFGS_FILESTOGET, SPBM_QUERYVALUE,
                        MPFROMP(&FilesToGet), MPFROM2SHORT(0, SPBQ_DONOTUPDATE));
      if (FilesToGet < FILESTOGET_MIN)
        FilesToGet = FILESTOGET_MIN;
      else if (FilesToGet > FILESTOGET_MAX)
        FilesToGet = FILESTOGET_MAX;
      PrfWriteProfileData(fmprof,
                          appname, "FilesToGet", &FilesToGet, sizeof(ULONG));
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgVDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CFGV_VIEWER, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGV_EDITOR, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGV_BINVIEW, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGV_BINED, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg, 0), MPVOID);
    WinEnableWindow(WinWindowFromID(hwnd, CFGV_FIND), FALSE);
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_UNDO:
    WinSetDlgItemText(hwnd, CFGV_VIEWER, viewer);
    WinSetDlgItemText(hwnd, CFGV_EDITOR, editor);
    WinSetDlgItemText(hwnd, CFGV_BINVIEW, binview);
    WinSetDlgItemText(hwnd, CFGV_BINED, bined);
    WinCheckButton(hwnd, CFGV_USENEWVIEWER, fUseNewViewer);
    WinCheckButton(hwnd, CFGV_GUESSTYPE, fGuessType);
    WinCheckButton(hwnd, CFGV_VIEWCHILD, fViewChild);
    WinCheckButton(hwnd, CFGV_CHECKMM, fCheckMM);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFGV_VIEWER:
    case CFGV_EDITOR:
    case CFGV_BINVIEW:
    case CFGV_BINED:
      switch (SHORT2FROMMP(mp1)) {
      case EN_KILLFOCUS:
        WinEnableWindow(WinWindowFromID(hwnd, CFGV_FIND), FALSE);
        break;
      case EN_SETFOCUS:
        WinEnableWindow(WinWindowFromID(hwnd, CFGV_FIND), TRUE);
        break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGV, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case CFGV_FIND:
      {
        CHAR filename[CCHMAXPATH + 9], szfilename[CCHMAXPATH + 9];
        USHORT id;
        HWND hwndFocus;

        strcpy(filename, "*.EXE");
        hwndFocus = WinQueryFocus(HWND_DESKTOP);
        if (hwndFocus) {
          id = WinQueryWindowUShort(hwndFocus, QWS_ID);
          switch (id) {
          case CFGV_BINVIEW:
          case CFGV_BINED:
          case CFGV_VIEWER:
          case CFGV_EDITOR:
            if (insert_filename(hwnd, filename, 2, FALSE) && *filename) {
              BldQuotedFileName(szfilename, filename);
              strcat(szfilename, " %a");
              WinSetDlgItemText(hwnd, id, szfilename);
            }
            break;
          default:
            Runtime_Error(pszSrcFile, __LINE__, "bad case %d", id);
            break;
          }
        }
      }
      break;
    }
    return 0;

  case WM_CLOSE:
    {
      CHAR *szCLBuf;
      PSZ pszWorkBuf;

      szCLBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!szCLBuf)
          return 0; //already complained
      pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
      if (!pszWorkBuf) {
        free(szCLBuf);
        return 0; //already complained
      }
      WinQueryDlgItemText(hwnd, CFGV_VIEWER, MaxComLineStrg, szCLBuf);
      szCLBuf[MaxComLineStrg - 1] = 0;
      if (strcmp(szCLBuf, viewer)){
        NormalizeCmdLine(pszWorkBuf, szCLBuf);
        memcpy(viewer, pszWorkBuf, strlen(pszWorkBuf) + 1);
        if (!strchr(viewer, '%') && strlen(viewer) > 3)
          strcat(viewer, " %a");
      }
      WinQueryDlgItemText(hwnd, CFGV_EDITOR, MaxComLineStrg, szCLBuf);
      szCLBuf[MaxComLineStrg - 1] = 0;
      if (strcmp(szCLBuf, editor)){
        NormalizeCmdLine(pszWorkBuf, szCLBuf);
        memcpy(editor, pszWorkBuf, strlen(pszWorkBuf) + 1);
        if (!strchr(editor, '%') && strlen(editor) > 3)
          strcat(editor, " %a");
      }
      WinQueryDlgItemText(hwnd, CFGV_BINVIEW, MaxComLineStrg, szCLBuf);
      szCLBuf[MaxComLineStrg - 1] = 0;
      if (strcmp(szCLBuf, binview)){
        NormalizeCmdLine(pszWorkBuf, szCLBuf);
        memcpy(binview, pszWorkBuf, strlen(pszWorkBuf) + 1);
        if (!strchr(binview, '%') && strlen(binview) > 3)
          strcat(binview, " %a");
      }
      WinQueryDlgItemText(hwnd, CFGV_BINED, MaxComLineStrg, szCLBuf);
      szCLBuf[MaxComLineStrg - 1] = 0;
      if (strcmp(szCLBuf, bined)){
        NormalizeCmdLine(pszWorkBuf, szCLBuf);
        memcpy(bined, pszWorkBuf, strlen(pszWorkBuf) + 1);
        if (!strchr(bined, '%') && strlen(bined) > 3)
          strcat(bined, " %a");
      }
      free(pszWorkBuf);
      free(szCLBuf);
      PrfWriteProfileString(fmprof, appname, "Viewer", viewer);
      PrfWriteProfileString(fmprof, appname, "Editor", editor);
      PrfWriteProfileString(fmprof, appname, "BinView", binview);
      PrfWriteProfileString(fmprof, appname, "BinEd", bined);
      fUseNewViewer = WinQueryButtonCheckstate(hwnd, CFGV_USENEWVIEWER);
      PrfWriteProfileData(fmprof, appname, "UseNewViewer", &fUseNewViewer,
                          sizeof(BOOL));
      fGuessType = WinQueryButtonCheckstate(hwnd, CFGV_GUESSTYPE);
      PrfWriteProfileData(fmprof, appname, "GuessType", &fGuessType,
                          sizeof(BOOL));
      fViewChild = WinQueryButtonCheckstate(hwnd, CFGV_VIEWCHILD);
      PrfWriteProfileData(fmprof, appname, "ViewChild", &fViewChild,
                          sizeof(BOOL));
      fCheckMM = WinQueryButtonCheckstate(hwnd, CFGV_CHECKMM);
      PrfWriteProfileData(fmprof, appname, "CheckMM", &fCheckMM, sizeof(BOOL));

      break;
    }
  }
  if (fCancelAction) {
    fCancelAction = FALSE;
    WinDlgBox(HWND_DESKTOP,
              hwnd,
              CfgDlgProc,
              FM3ModHandle,
              CFG_FRAME,
              MPFROMLONG(IDM_VIEWERSETTINGS));
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgHDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CFGH_RUNFTPWORKDIR, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGH_RUNHTTPWORKDIR, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGH_FTPRUN, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGH_HTTPRUN, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGH_MAILRUN, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGH_RUNMAILWORKDIR, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    WinEnableWindow(WinWindowFromID(hwnd, CFGH_FIND), FALSE);
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_UNDO:
    WinSetDlgItemText(hwnd, CFGH_RUNFTPWORKDIR, ftprundir);
    WinSetDlgItemText(hwnd, CFGH_RUNHTTPWORKDIR, httprundir);
    WinSetDlgItemText(hwnd, CFGH_RUNMAILWORKDIR, mailrundir);
    WinSetDlgItemText(hwnd, CFGH_MAILRUN, mailrun);
    WinSetDlgItemText(hwnd, CFGH_FTPRUN, ftprun);
    WinSetDlgItemText(hwnd, CFGH_HTTPRUN, httprun);
    WinCheckButton(hwnd, CFGH_HTTPRUNWPSDEFAULT, fHttpRunWPSDefault);
    WinCheckButton(hwnd, CFGH_FTPRUNWPSDEFAULT, fFtpRunWPSDefault);
    WinCheckButton(hwnd, CFGH_LIBPATHSTRICTHTTPRUN, fLibPathStrictHttpRun);
    WinCheckButton(hwnd, CFGH_LIBPATHSTRICTFTPRUN, fLibPathStrictFtpRun);
    WinCheckButton(hwnd, CFGH_LIBPATHSTRICTMAILRUN, fLibPathStrictMailRun);
    WinCheckButton(hwnd, CFGH_NOMAILTOMAILRUN, fNoMailtoMailRun);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFGH_HTTPRUN:
    case CFGH_FTPRUN:
    case CFGH_RUNFTPWORKDIR:
    case CFGH_RUNHTTPWORKDIR:
    case CFGH_RUNMAILWORKDIR:
    case CFGH_MAILRUN:
      switch (SHORT2FROMMP(mp1)) {
      case EN_KILLFOCUS:
        WinEnableWindow(WinWindowFromID(hwnd, CFGH_FIND), FALSE);
        break;
      case EN_SETFOCUS:
        WinEnableWindow(WinWindowFromID(hwnd, CFGH_FIND), TRUE);
        break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGH, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case CFGH_FIND:
      {
        CHAR filename[CCHMAXPATH + 9], szfilename[CCHMAXPATH + 9];
        USHORT id;
        HWND hwndFocus;

        strcpy(filename, "*.EXE");
        hwndFocus = WinQueryFocus(HWND_DESKTOP);
        if (hwndFocus) {
          id = WinQueryWindowUShort(hwndFocus, QWS_ID);
          switch (id) {
          case CFGH_HTTPRUN:
          case CFGH_FTPRUN:
          case CFGH_MAILRUN:
            if (insert_filename(hwnd, filename, 2, FALSE) && *filename) {
              BldQuotedFileName(szfilename, filename);
              WinSetDlgItemText(hwnd, id, szfilename);
            }
            break;
          case CFGH_RUNFTPWORKDIR:
            strcpy(filename, ftprundir);
            if (WinDlgBox(HWND_DESKTOP, hwndNotebook,
                          WalkExtractDlgProc, FM3ModHandle, WALK_FRAME,
                          MPFROMP(filename)) && *filename)
              WinSetDlgItemText(hwnd, id, filename);
            break;
          case CFGH_RUNHTTPWORKDIR:
            strcpy(filename, httprundir);
            if (WinDlgBox(HWND_DESKTOP, hwndNotebook,
                          WalkExtractDlgProc, FM3ModHandle, WALK_FRAME,
                          MPFROMP(filename)) && *filename)
              WinSetDlgItemText(hwnd, id, filename);
            break;
          case CFGH_RUNMAILWORKDIR:
            strcpy(filename, mailrundir);
            if (WinDlgBox(HWND_DESKTOP, hwndNotebook,
                          WalkExtractDlgProc, FM3ModHandle, WALK_FRAME,
                          MPFROMP(filename)) && *filename)
              WinSetDlgItemText(hwnd, id, filename);
            break;
          default:
            Runtime_Error(pszSrcFile, __LINE__, "bad case %d", id);
            break;
          }
        }
      }
      break;
    }
    return 0;

  case WM_CLOSE:
    {
      CHAR *szCLBuf;
      CHAR szPathBuf[CCHMAXPATH];
      PSZ pszWorkBuf;

      szCLBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!szCLBuf)
          return 0; //already complained
      pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
      if (!pszWorkBuf) {
        free(szCLBuf);
        return 0; //already complained
      }
      WinQueryDlgItemText(hwnd, CFGH_RUNHTTPWORKDIR, CCHMAXPATH, szPathBuf);
      szPathBuf[CCHMAXPATH - 1] = 0;
      bstrip(szPathBuf);
      memcpy(httprundir, szPathBuf, strlen(szPathBuf) + 1);
      WinQueryDlgItemText(hwnd, CFGH_RUNFTPWORKDIR, CCHMAXPATH, szPathBuf);
      szPathBuf[CCHMAXPATH - 1] = 0;
      bstrip(szPathBuf);
      memcpy(ftprundir, szPathBuf, strlen(szPathBuf) + 1);
      WinQueryDlgItemText(hwnd, CFGH_RUNMAILWORKDIR, CCHMAXPATH, szPathBuf);
      szPathBuf[CCHMAXPATH - 1] = 0;
      bstrip(szPathBuf);
      memcpy(mailrundir, szPathBuf, strlen(szPathBuf) + 1);
      WinQueryDlgItemText(hwnd, CFGH_FTPRUN, MaxComLineStrg, szCLBuf);
      szCLBuf[MaxComLineStrg - 1] = 0;
      if (strcmp(szCLBuf, ftprun)){
        NormalizeCmdLine(pszWorkBuf, szCLBuf);
        memcpy(ftprun, pszWorkBuf, strlen(pszWorkBuf) + 1);
      }
      WinQueryDlgItemText(hwnd, CFGH_HTTPRUN, MaxComLineStrg, szCLBuf);
      szCLBuf[MaxComLineStrg - 1] = 0;
      if (strcmp(szCLBuf, httprun)){
        NormalizeCmdLine(pszWorkBuf, szCLBuf);
        memcpy(httprun, pszWorkBuf, strlen(pszWorkBuf) + 1);
      }
      WinQueryDlgItemText(hwnd, CFGH_MAILRUN, MaxComLineStrg, szCLBuf);
      szCLBuf[MaxComLineStrg - 1] = 0;
      if (strcmp(szCLBuf, mailrun)){
        NormalizeCmdLine(pszWorkBuf, szCLBuf);
        memcpy(mailrun, pszWorkBuf, strlen(pszWorkBuf) + 1);
      }
      free(pszWorkBuf);
      free(szCLBuf);
      PrfWriteProfileString(fmprof, appname, "HttpRunDir", httprundir);
      PrfWriteProfileString(fmprof, appname, "FtpRunDir", ftprundir);
      PrfWriteProfileString(fmprof, appname, "MailRunDir", mailrundir);
      PrfWriteProfileString(fmprof, appname, "FTPRun", ftprun);
      PrfWriteProfileString(fmprof, appname, "HTTPRun", httprun);
      PrfWriteProfileString(fmprof, appname, "MailRun", mailrun);
      fHttpRunWPSDefault = WinQueryButtonCheckstate(hwnd, CFGH_HTTPRUNWPSDEFAULT);
      PrfWriteProfileData(fmprof, appname, "HttpRunWPSDefault", &fHttpRunWPSDefault,
                          sizeof(BOOL));
      fFtpRunWPSDefault = WinQueryButtonCheckstate(hwnd, CFGH_FTPRUNWPSDEFAULT);
      PrfWriteProfileData(fmprof, appname, "FtpRunWPSDefault", &fFtpRunWPSDefault,
                          sizeof(BOOL));
      fLibPathStrictHttpRun = WinQueryButtonCheckstate(hwnd, CFGH_LIBPATHSTRICTHTTPRUN);
      PrfWriteProfileData(fmprof, appname, "LibPathStrictHttpRun",
                          &fLibPathStrictHttpRun, sizeof(BOOL));
      fLibPathStrictFtpRun = WinQueryButtonCheckstate(hwnd, CFGH_LIBPATHSTRICTFTPRUN);
      PrfWriteProfileData(fmprof, appname, "LibPathStrictFtpRun",
                          &fLibPathStrictFtpRun, sizeof(BOOL));
      fLibPathStrictMailRun = WinQueryButtonCheckstate(hwnd, CFGH_LIBPATHSTRICTMAILRUN);
      PrfWriteProfileData(fmprof, appname, "LibPathStrictMailRun",
                          &fLibPathStrictMailRun, sizeof(BOOL));
      fNoMailtoMailRun = WinQueryButtonCheckstate(hwnd, CFGH_NOMAILTOMAILRUN);
      PrfWriteProfileData(fmprof, appname, "NoMailtoMailRun",
                          &fNoMailtoMailRun, sizeof(BOOL));
      break;
    }
  }
  if (fCancelAction) {
    fCancelAction = FALSE;
    WinDlgBox(HWND_DESKTOP,
              hwnd,
              CfgDlgProc,
              FM3ModHandle,
              CFG_FRAME,
              MPFROMLONG(IDM_VIEWERSETTINGS));
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgBDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, CFGB_TOOLBARHELP, fToolbarHelp);
    WinCheckButton(hwnd, CFGB_DRIVEBARHELP, fDrivebarHelp);
    WinCheckButton(hwnd, CFGB_OTHERHELP, fOtherHelp);
    return 0;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp,
                   HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGB, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    fToolbarHelp = WinQueryButtonCheckstate(hwnd, CFGB_TOOLBARHELP);
    PrfWriteProfileData(fmprof,
                        FM3Str, "ToolbarHelp", &fToolbarHelp, sizeof(BOOL));
    fDrivebarHelp = WinQueryButtonCheckstate(hwnd, CFGB_DRIVEBARHELP);
    PrfWriteProfileData(fmprof,
                        FM3Str, "DrivebarHelp", &fDrivebarHelp, sizeof(BOOL));
    fOtherHelp = WinQueryButtonCheckstate(hwnd, CFGB_OTHERHELP);
    PrfWriteProfileData(fmprof,
                        FM3Str, "OtherHelp", &fOtherHelp, sizeof(BOOL));
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgTSDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static MASK mask;

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd,
                      CFG5_FILTER,
                      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IDM_HELP));
    return 0;

  case UM_UNDO:
    {
      ULONG flWindowAttr = 0;
      ULONG ulSize = sizeof(ULONG);

      if (!PrfQueryProfileData(fmprof,
                               appname,
                               "TreeflWindowAttr",
                               (PVOID) & flWindowAttr, &ulSize))
        flWindowAttr |= (CV_TREE | CA_TREELINE | CV_ICON | CV_MINI | CV_FLOW);
      WinCheckButton(hwnd, CFG5_ICON, ((flWindowAttr & CV_ICON) != FALSE));
      WinCheckButton(hwnd, CFG5_MINIICONS,
                     ((flWindowAttr & CV_MINI) != FALSE));
      memset(&mask, 0, sizeof(mask));
      mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
        FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
      mask.fIsTree = TRUE;
      ulSize = sizeof(MASK);
      if (PrfQueryProfileData(fmprof, appname, "TreeFilter", &mask, &ulSize)) {
        SetMask(NULL, &mask);
      }
      if (!mask.attrFile)
        mask.attrFile = (FILE_READONLY | FILE_NORMAL |
                         FILE_ARCHIVED | FILE_DIRECTORY |
                         FILE_HIDDEN | FILE_SYSTEM);
      strcpy(mask.prompt, GetPString(IDS_TREEFILTERTITLETEXT));
      WinSetDlgItemText(hwnd, CFG5_FILTER, mask.szMask);
      WinCheckButton(hwnd, CFG6_SORTFIRST, FALSE);
      WinCheckButton(hwnd, CFG6_SORTLAST, FALSE);
      WinCheckButton(hwnd, CFG6_SORTSIZE, FALSE);
      WinCheckButton(hwnd, CFG6_SORTEASIZE, FALSE);
      WinCheckButton(hwnd, CFG6_SORTLWDATE, FALSE);
      WinCheckButton(hwnd, CFG6_SORTLADATE, FALSE);
      WinCheckButton(hwnd, CFG6_SORTCRDATE, FALSE);
      WinCheckButton(hwnd, CFG6_SORTNAME, FALSE);
      WinCheckButton(hwnd, CFG6_SORTREVERSE, FALSE);
      if (TreesortFlags & SORT_FIRSTEXTENSION)
        WinCheckButton(hwnd, CFG6_SORTFIRST, TRUE);
      else if (TreesortFlags & SORT_LASTEXTENSION)
        WinCheckButton(hwnd, CFG6_SORTLAST, TRUE);
      else if (TreesortFlags & SORT_SIZE)
        WinCheckButton(hwnd, CFG6_SORTSIZE, TRUE);
      else if (TreesortFlags & SORT_EASIZE)
        WinCheckButton(hwnd, CFG6_SORTEASIZE, TRUE);
      else if (TreesortFlags & SORT_LWDATE)
        WinCheckButton(hwnd, CFG6_SORTLWDATE, TRUE);
      else if (TreesortFlags & SORT_LADATE)
        WinCheckButton(hwnd, CFG6_SORTLADATE, TRUE);
      else if (TreesortFlags & SORT_CRDATE)
        WinCheckButton(hwnd, CFG6_SORTCRDATE, TRUE);
      else if (TreesortFlags & SORT_FILENAME)
        WinCheckButton(hwnd, CFG6_SORTFILENAME, TRUE);
      else
        WinCheckButton(hwnd, CFG6_SORTNAME, TRUE);
      if (TreesortFlags & SORT_REVERSE)
        WinCheckButton(hwnd, CFG6_SORTREVERSE, TRUE);
    }
    return 0;

  case UM_SETUP5:
    if (WinDlgBox(HWND_DESKTOP, hwndNotebook, PickMaskDlgProc,
                  FM3ModHandle, MSK_FRAME, MPFROMP(&mask))) {
      SetMask(NULL, &mask);
      WinSetDlgItemText(hwnd, CFG5_FILTER, mask.szMask);
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFG5_FILTER:
      switch (SHORT2FROMMP(mp1)) {
      case EN_SETFOCUS:
        PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
        PostMsg(hwnd, UM_SETUP5, MPVOID, MPVOID);
        break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_TREEVIEW, 0),
                   MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    {
      ULONG flWindowAttr = 0;

      if (WinQueryButtonCheckstate(hwnd, CFG5_ICON))
        flWindowAttr |= CV_ICON;
      else
        flWindowAttr |= CV_TEXT;
      if (WinQueryButtonCheckstate(hwnd, CFG5_MINIICONS))
        flWindowAttr |= CV_MINI;
      flWindowAttr |= (CV_TREE | CV_FLOW | CA_TREELINE);
      PrfWriteProfileData(fmprof,
                          appname,
                          "TreeflWindowAttr", &flWindowAttr, sizeof(ULONG));
      if (hwndTree) {

        CNRINFO cnri;

        memset(&cnri, 0, sizeof(cnri));
        cnri.cb = sizeof(cnri);
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(cnri)));
        cnri.flWindowAttr = flWindowAttr;
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_SETCNRINFO, MPFROMP(&cnri),
                   MPFROMLONG(CMA_FLWINDOWATTR));
      }
    }
    TreesortFlags = 0;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTFILENAME))
      TreesortFlags |= SORT_FILENAME;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTSIZE))
      TreesortFlags |= SORT_SIZE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTEASIZE))
      TreesortFlags |= SORT_EASIZE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTFIRST))
      TreesortFlags |= SORT_FIRSTEXTENSION;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTLAST))
      TreesortFlags |= SORT_LASTEXTENSION;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTLWDATE))
      TreesortFlags |= SORT_LWDATE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTLADATE))
      TreesortFlags |= SORT_LADATE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTCRDATE))
      TreesortFlags |= SORT_CRDATE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTDIRSFIRST))
      TreesortFlags |= SORT_DIRSFIRST;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTDIRSLAST))
      TreesortFlags |= SORT_DIRSLAST;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTREVERSE))
      TreesortFlags |= SORT_REVERSE;
    PrfWriteProfileData(fmprof, appname, "TreeSort", &TreesortFlags,
                        sizeof(INT));
    if (hwndTree)
      PostMsg(hwndTree, WM_COMMAND, MPFROM2SHORT(IDM_RESORT, 0), MPVOID);
    *mask.prompt = 0;
    PrfWriteProfileData(fmprof, appname, "TreeFilter", &mask, sizeof(MASK));
    if (hwndTree) {

      DIRCNRDATA *dcd;

      dcd = WinQueryWindowPtr(WinWindowFromID(WinWindowFromID(hwndTree,
                                                              FID_CLIENT),
                                              TREE_CNR), QWL_USER);
      if (dcd && dcd->size == sizeof(DIRCNRDATA)) {
        dcd->mask = mask;
        PostMsg(hwndTree, UM_FILTER, MPVOID, MPVOID);
      }
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgTDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  BOOL fShowEnvChanged = FALSE;
  BOOL fTreeEnvVarListChanged = FALSE;

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CFGT_ENVVARLIST, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, CFGT_FOLLOWTREE, fFollowTree);
    WinCheckButton(hwnd, CFGT_TOPDIR, fTopDir);
    WinCheckButton(hwnd, CFGT_DCOPENS, fDCOpens);
    WinCheckButton(hwnd, CFGT_VTREEOPENSWPS, fVTreeOpensWPS);
    WinCheckButton(hwnd, CFGT_COLLAPSEFIRST, fCollapseFirst);
    WinCheckButton(hwnd, CFGT_SWITCHTREEONFOCUS, fSwitchTreeOnFocus);
    WinCheckButton(hwnd, CFGT_SWITCHTREE, fSwitchTree);
    WinCheckButton(hwnd, CFGT_SWITCHTREEEXPAND, fSwitchTreeExpand);
    WinCheckButton(hwnd, CFGT_SHOWENV, fShowEnv);
    WinSetDlgItemText(hwnd, CFGT_ENVVARLIST, pszTreeEnvVarList);
    return 0;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGT, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    fVTreeOpensWPS = WinQueryButtonCheckstate(hwnd, CFGT_VTREEOPENSWPS);
    PrfWriteProfileData(fmprof, FM3Str, "VTreeOpensWPS", &fVTreeOpensWPS,
                        sizeof(BOOL));
    fCollapseFirst = WinQueryButtonCheckstate(hwnd, CFGT_COLLAPSEFIRST);
    PrfWriteProfileData(fmprof, appname, "CollapseFirst", &fCollapseFirst,
                        sizeof(BOOL));
    fSwitchTreeOnFocus = WinQueryButtonCheckstate(hwnd,
                                                  CFGT_SWITCHTREEONFOCUS);
    PrfWriteProfileData(fmprof, appname, "SwitchTreeOnFocus",
                        &fSwitchTreeOnFocus, sizeof(BOOL));
    fSwitchTreeExpand = WinQueryButtonCheckstate(hwnd, CFGT_SWITCHTREEEXPAND);
    PrfWriteProfileData(fmprof, appname, "SwitchTreeExpand",
                        &fSwitchTreeExpand, sizeof(BOOL));
    fSwitchTree = WinQueryButtonCheckstate(hwnd, CFGT_SWITCHTREE);
    PrfWriteProfileData(fmprof, appname, "SwitchTree", &fSwitchTree,
                        sizeof(BOOL));
    fFollowTree = WinQueryButtonCheckstate(hwnd, CFGT_FOLLOWTREE);
    PrfWriteProfileData(fmprof, appname, "FollowTree", &fFollowTree,
                        sizeof(BOOL));
    fTopDir = WinQueryButtonCheckstate(hwnd, CFGT_TOPDIR);
    PrfWriteProfileData(fmprof, appname, "TopDir", (PVOID) & fTopDir,
                        sizeof(BOOL));
    fDCOpens = WinQueryButtonCheckstate(hwnd, CFGT_DCOPENS);
    PrfWriteProfileData(fmprof, FM3Str, "DoubleClickOpens", &fDCOpens,
                        sizeof(BOOL));
    fShowEnvChanged = (fShowEnv != WinQueryButtonCheckstate(hwnd, CFGT_SHOWENV));
    fShowEnv = WinQueryButtonCheckstate(hwnd, CFGT_SHOWENV);
    PrfWriteProfileData(fmprof, appname, "ShowEnv", &fShowEnv, sizeof(BOOL));
    {
      char * pszTemp = xmalloc(WinQueryDlgItemTextLength(hwnd, CFGT_ENVVARLIST) + 1, pszSrcFile, __LINE__);
      if (pszTemp) {
        WinQueryDlgItemText(hwnd, CFGT_ENVVARLIST, MaxComLineStrg, pszTemp);
        strupr(pszTemp);
        if (strcmp(pszTemp, pszTreeEnvVarList)) {
          fTreeEnvVarListChanged = TRUE;
          strcpy(pszTreeEnvVarList, pszTemp);
          PrfWriteProfileString(fmprof, appname, "TreeEnvVarList", pszTreeEnvVarList);
        }
        free(pszTemp);
      }
      if (hwndTree && (fShowEnvChanged || (fShowEnv && fTreeEnvVarListChanged)))
      {
        PCNRITEM pci = WinSendMsg(WinWindowFromID
                (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR), CM_QUERYRECORDEMPHASIS,
                                  MPFROMLONG(CMA_FIRST),
                                  MPFROMSHORT(CRA_SELECTED));
        PostMsg(WinWindowFromID
                (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR), WM_COMMAND,
                MPFROM2SHORT(IDM_RESCAN, 0), MPVOID);
        pszTemp = xstrdup(pci->pszFileName, pszSrcFile, __LINE__);
        if (pszTemp) {
          if (!PostMsg(hwndTree, UM_SHOWME, MPFROMP(pszTemp), MPVOID))
            free(pszTemp);
          /* pszTemp is freed in the UM_SHOWME code */
        }
      }
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgGDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CFGG_CMDLNLNGTH, SPBM_SETTEXTLIMIT,
                      MPFROMSHORT(8), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGG_CMDLNLNGTH, SPBM_OVERRIDESETLIMITS,
                      MPFROMLONG(CMDLNLNGTH_MAX), MPFROMLONG(CMDLNLNGTH_MIN));
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, CFGG_CONFIRMDELETE, fConfirmDelete);
    WinCheckButton(hwnd, CFGG_VERIFYWRITES, fVerify);
    WinCheckButton(hwnd, CFGG_LINKSETSICON, fLinkSetsIcon);
    WinCheckButton(hwnd, CFGG_DONTMOVEMOUSE, fDontMoveMouse);
    WinCheckButton(hwnd, CFGG_DEFAULTCOPY, fCopyDefault);
    WinCheckButton(hwnd, CFGG_IDLECOPY, fRealIdle);
    WinCheckButton(hwnd, CFGG_DNDDLG, fDragndropDlg);
    WinCheckButton(hwnd, CFGG_DEFAULTDELETEPERM, fDefaultDeletePerm);
    {
      long th = fNoFinger ? 2 : (fNoDead ? 1 : 0);
      WinCheckButton(hwnd, CFGG_NODEAD, th);
    }
    WinCheckButton(hwnd, CFGG_BORING, fDullMin);
    WinCheckButton(hwnd, CFGG_CUSTOMFILEDLG, fCustomFileDlg);
    WinCheckButton(hwnd, CFGG_FM2DELETES, fFM2Deletes);
    WinCheckButton(hwnd, CFGG_CONFIRMTARGET, fConfirmTarget);
    WinSetDlgItemText(hwnd, CFGG_TARGETDIR, targetdir);
    WinCheckButton(hwnd, CFGG_TRASHCAN, fTrashCan);
    WinSendDlgItemMsg(hwnd, CFGG_CMDLNLNGTH, SPBM_SETCURRENTVALUE,
                      MPFROMLONG(MaxComLineStrg), MPVOID);
    return 0;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, IDM_HELP));
    return 0;

  case UM_SETUP5:
    SetTargetDir(hwnd, FALSE);
    WinSetDlgItemText(hwnd, CFGG_TARGETDIR, targetdir);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFGG_TARGETDIR:
      switch (SHORT2FROMMP(mp1)) {
      case EN_SETFOCUS:
        PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
        PostMsg(hwnd, UM_SETUP5, MPVOID, MPVOID);
        break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp,
                   HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGG, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    {
      long test;

      test = WinQueryButtonCheckstate(hwnd, CFGG_NODEAD);
      fNoDead = (test == 1);
      fNoFinger = (test == 2);
      PrfWriteProfileData(fmprof, FM3Str, "NoDead", &fNoDead, sizeof(BOOL));
      PrfWriteProfileData(fmprof,
                          FM3Str, "NoFinger", &fNoFinger, sizeof(BOOL));
      WinDestroyPointer(hptrFinger);
      if (!fNoDead)
        hptrFinger = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FINGER_ICON);
      else
        hptrFinger = WinLoadPointer(HWND_DESKTOP, FM3ModHandle, FINGER2_ICON);
    }
    fLinkSetsIcon = WinQueryButtonCheckstate(hwnd, CFGG_LINKSETSICON);
    PrfWriteProfileData(fmprof,
                        appname,
                        "LinkSetsIcon", &fLinkSetsIcon, sizeof(BOOL));
    fCustomFileDlg = WinQueryButtonCheckstate(hwnd, CFGG_CUSTOMFILEDLG);
    PrfWriteProfileData(fmprof,
                        FM3Str,
                        "CustomFileDlg", &fCustomFileDlg, sizeof(BOOL));
    fDullMin = WinQueryButtonCheckstate(hwnd, CFGG_BORING);
    PrfWriteProfileData(fmprof,
                        FM3Str, "DullDatabar", &fDullMin, sizeof(BOOL));
    fConfirmDelete = WinQueryButtonCheckstate(hwnd, CFGG_CONFIRMDELETE);
    PrfWriteProfileData(fmprof,
                        appname,
                        "ConfirmDelete", &fConfirmDelete, sizeof(BOOL));
    fDontMoveMouse = WinQueryButtonCheckstate(hwnd, CFGG_DONTMOVEMOUSE);
    PrfWriteProfileData(fmprof,
                        appname,
                        "DontMoveMouse", &fDontMoveMouse, sizeof(BOOL));
    fCopyDefault = WinQueryButtonCheckstate(hwnd, CFGG_DEFAULTCOPY);
    PrfWriteProfileData(fmprof, appname, "DefaultCopy",
                        &fCopyDefault, sizeof(BOOL));
    fRealIdle = WinQueryButtonCheckstate(hwnd, CFGG_IDLECOPY);
    PrfWriteProfileData(fmprof, appname, "IdleCopy",
                        &fRealIdle, sizeof(BOOL));
    fDragndropDlg = WinQueryButtonCheckstate(hwnd, CFGG_DNDDLG);
    PrfWriteProfileData(fmprof, appname, "Drag&DropDlg",
                        &fDragndropDlg, sizeof(BOOL));
    fVerify = WinQueryButtonCheckstate(hwnd, CFGG_VERIFYWRITES);
    PrfWriteProfileData(fmprof, appname, "VerifyWrites",
                        &fVerify, sizeof(BOOL));
    DosSetVerify(fVerify);
    fDefaultDeletePerm = WinQueryButtonCheckstate(hwnd,
                                                  CFGG_DEFAULTDELETEPERM);
    PrfWriteProfileData(fmprof, appname, "DefaultDeletePerm",
                        &fDefaultDeletePerm, sizeof(BOOL));
    fFM2Deletes = WinQueryButtonCheckstate(hwnd, CFGG_FM2DELETES);
    PrfWriteProfileData(fmprof, FM3Str, "FM2Deletes",
                        &fFM2Deletes, sizeof(BOOL));
    fTrashCan = WinQueryButtonCheckstate(hwnd, CFGG_TRASHCAN);
    PrfWriteProfileData(fmprof, FM3Str, "TrashCan",
                        &fTrashCan, sizeof(BOOL));
    fConfirmTarget = WinQueryButtonCheckstate(hwnd, CFGG_CONFIRMTARGET);
    PrfWriteProfileData(fmprof, appname, "ConfirmTarget",
                        &fConfirmTarget, sizeof(BOOL));
    {
      WinSendDlgItemMsg(hwnd, CFGG_CMDLNLNGTH, SPBM_QUERYVALUE,
                        MPFROMP(&MaxComLineStrg), MPFROM2SHORT(0, SPBQ_DONOTUPDATE));
      if (MaxComLineStrg < CMDLNLNGTH_MIN)
        MaxComLineStrg = CMDLNLNGTH_MIN;
      else if (MaxComLineStrg > CMDLNLNGTH_MAX)
        MaxComLineStrg = CMDLNLNGTH_MAX;
      PrfWriteProfileData(fmprof,
                          appname, "MaxComLineStrg", &MaxComLineStrg, sizeof(ULONG));
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgCDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CFGC_COMPARE, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, CFGC_DIRCOMPARE, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(MaxComLineStrg, 0), MPVOID);
    WinEnableWindow(WinWindowFromID(hwnd, CFGC_FIND), FALSE);
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_UNDO:
    WinSetDlgItemText(hwnd, CFGC_COMPARE, compare);
    WinSetDlgItemText(hwnd, CFGC_DIRCOMPARE, dircompare);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFGC_COMPARE:
    case CFGC_DIRCOMPARE:
      switch (SHORT2FROMMP(mp1)) {
      case EN_KILLFOCUS:
        WinEnableWindow(WinWindowFromID(hwnd, CFGC_FIND), FALSE);
        break;
      case EN_SETFOCUS:
        WinEnableWindow(WinWindowFromID(hwnd, CFGC_FIND), TRUE);
        break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGC, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case CFGC_FIND:
      {
        CHAR filename[CCHMAXPATH + 9], szfilename[CCHMAXPATH + 9];
        USHORT id;
        HWND hwndFocus;

        strcpy(filename, "*.EXE");
        hwndFocus = WinQueryFocus(HWND_DESKTOP);
        if (hwndFocus) {
          id = WinQueryWindowUShort(hwndFocus, QWS_ID);
          switch (id) {
          case CFGC_COMPARE:
          case CFGC_DIRCOMPARE:
            if (insert_filename(hwnd, filename, 2, FALSE) && *filename) {
              BldQuotedFileName(szfilename, filename);
              strcat(szfilename, " %a");
              WinSetDlgItemText(hwnd, id, szfilename);
            }
            break;
          default:
            Runtime_Error(pszSrcFile, __LINE__, "bad case %d", id);
            break;
          }
        }
      }
      break;
    }
    return 0;

  case WM_CLOSE:
    {
      CHAR *szCLBuf;
      PSZ pszWorkBuf;

      szCLBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
        if (!szCLBuf)
          return 0; //already complained
      pszWorkBuf = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
      if (!pszWorkBuf) {
        free(szCLBuf);
        return 0; //already complained
      }
      WinQueryDlgItemText(hwnd, CFGC_DIRCOMPARE, MaxComLineStrg, szCLBuf);
        szCLBuf[MaxComLineStrg - 1] = 0;
        if (strcmp(szCLBuf, dircompare)){
          NormalizeCmdLine(pszWorkBuf, szCLBuf);
          memcpy(dircompare, pszWorkBuf, strlen(pszWorkBuf) + 1);
          if (!strchr(dircompare, '%') && strlen(dircompare) > 3)
            strcat(dircompare, " %a");
        }
      PrfWriteProfileString(fmprof, appname, "DirCompare", dircompare);
      WinQueryDlgItemText(hwnd, CFGC_COMPARE, MaxComLineStrg, szCLBuf);
      szCLBuf[MaxComLineStrg - 1] = 0;
      if (strcmp(szCLBuf, compare)){
        NormalizeCmdLine(pszWorkBuf, szCLBuf);
        memcpy(compare, pszWorkBuf, strlen(pszWorkBuf) + 1);
        if (!strchr(compare, '%') && strlen(compare) > 3)
          strcat(compare, " %a");
      }
      free(pszWorkBuf);
      free(szCLBuf);
      PrfWriteProfileString(fmprof, appname, "Compare", compare);
      break;
    }
  }
  if (fCancelAction) {
    fCancelAction = FALSE;
    WinDlgBox(HWND_DESKTOP,
              hwnd,
              CfgDlgProc,
              FM3ModHandle,
              CFG_FRAME,
              MPFROMLONG(IDM_COMPARESETTINGS));
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgDDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

  switch (msg) {
  case WM_INITDLG:
    WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, CFGD_UNHILITE, fUnHilite);
    WinCheckButton(hwnd, CFGD_SYNCUPDATES, fSyncUpdates);
    WinCheckButton(hwnd, CFGD_LOOKINDIR, fLookInDir);
    WinCheckButton(hwnd, CFGD_MINONOPEN, fMinOnOpen);
    WinCheckButton(hwnd, CFGD_SELECTEDALWAYS, fSelectedAlways);
    WinCheckButton(hwnd, CFGD_NOSEARCH, fNoSearch);
    WinCheckButton(hwnd, CFGD_EXTENDEDSEL,
                   ((ulCnrType & CCS_EXTENDSEL) != 0));
    WinCheckButton(hwnd, CFGD_MULTIPLESEL,
                   ((ulCnrType & CCS_MULTIPLESEL) != 0));
    WinCheckButton(hwnd, CFGD_LEAVETREE, fLeaveTree);
    WinCheckButton(hwnd, CFGD_NOFOLDMENU, fNoFoldMenu);
    return 0;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGD, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    ulCnrType = 0;
    if (WinQueryButtonCheckstate(hwnd, CFGD_EXTENDEDSEL))
      ulCnrType |= CCS_EXTENDSEL;
    if (WinQueryButtonCheckstate(hwnd, CFGD_MULTIPLESEL))
      ulCnrType |= CCS_MULTIPLESEL;
    PrfWriteProfileData(fmprof, appname, "ContainerType",
                        (PVOID) & ulCnrType, sizeof(BOOL));
    fMinOnOpen = WinQueryButtonCheckstate(hwnd, CFGD_MINONOPEN);
    PrfWriteProfileData(fmprof, FM3Str, "MinDirOnOpen", &fMinOnOpen,
                        sizeof(BOOL));
    fLeaveTree = WinQueryButtonCheckstate(hwnd, CFGD_LEAVETREE);
    PrfWriteProfileData(fmprof, appname, "LeaveTree", &fLeaveTree,
                        sizeof(BOOL));
    fNoFoldMenu = WinQueryButtonCheckstate(hwnd, CFGD_NOFOLDMENU);
    PrfWriteProfileData(fmprof, appname, "NoFoldMenu", &fNoFoldMenu,
                        sizeof(BOOL));
    fSelectedAlways = WinQueryButtonCheckstate(hwnd, CFGD_SELECTEDALWAYS);
    PrfWriteProfileData(fmprof, appname, "SelectedAlways", &fSelectedAlways,
                        sizeof(BOOL));
    fNoSearch = WinQueryButtonCheckstate(hwnd, CFGD_NOSEARCH);
    PrfWriteProfileData(fmprof, appname, "NoSearch", &fNoSearch,
                        sizeof(BOOL));
    fLookInDir = WinQueryButtonCheckstate(hwnd, CFGD_LOOKINDIR);
    PrfWriteProfileData(fmprof, FM3Str, "LookInDir", (PVOID) & fLookInDir,
                        sizeof(BOOL));
    fUnHilite = WinQueryButtonCheckstate(hwnd, CFGD_UNHILITE);
    PrfWriteProfileData(fmprof, appname, "UnHilite",
                        &fUnHilite, sizeof(BOOL));
    {
      BOOL fOldSyncUpdates = WinQueryButtonCheckstate(hwnd, CFGD_SYNCUPDATES);

      if (fOldSyncUpdates != fSyncUpdates) {
        fSyncUpdates = fOldSyncUpdates;
        if (hwndMain && !strcmp(realappname, FM3Str)) {
          // Save state and restore to refresh windows with new settings
          if (SaveDirCnrState(hwndMain, GetPString(IDS_FM2TEMPTEXT)) > 0) {
            PostMsg(MainObjectHwnd, UM_RESTORE, MPVOID, MPFROMLONG(2));
//          PostMsg(hwndMain, UM_RESTORE, MPVOID, MPVOID);
            PostMsg(MainObjectHwnd, UM_RESTORE, GetPString(IDS_FM2TEMPTEXT), MPVOID);
          }
        }
      }
    }
    PrfWriteProfileData(fmprof, appname, "SyncUpdates",
                        &fSyncUpdates, sizeof(BOOL));
    if (!(ulCnrType & (CCS_EXTENDSEL | CCS_MULTIPLESEL)))
      saymsg(MB_ENTER | MB_ICONEXCLAMATION,
             HWND_DESKTOP,
             GetPString(IDS_WARNINGTEXT),
             GetPString(IDS_SELECTTYPEERRORTEXT));
    break;                              // WM_CLOSE
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY CfgMDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, CFGM_EXTERNALINIS, fExternalINIs);
    WinCheckButton(hwnd, CFGM_EXTERNALARCBOXES, fExternalArcboxes);
    WinCheckButton(hwnd, CFGM_EXTERNALVIEWER, fExternalViewer);
    WinCheckButton(hwnd, CFGM_EXTERNALCOLLECTOR, fExternalCollector);
    WinCheckButton(hwnd, CFGM_SAVESTATE, fSaveState);
    WinCheckButton(hwnd, CFGM_AUTOTILE, (fAutoTile));
    WinCheckButton(hwnd, CFGM_FREETREE, (fFreeTree));
    WinCheckButton(hwnd, CFGM_SPLITSTATUS, (fSplitStatus));
    WinCheckButton(hwnd, CFGM_NOTREEGAP, (fNoTreeGap));
    WinCheckButton(hwnd, CFGM_STARTMIN, (fStartMinimized));
    WinCheckButton(hwnd, CFGM_STARTMAX, (fStartMaximized));
    WinCheckButton(hwnd, CFGM_DATAMIN, (fDataMin));
    WinCheckButton(hwnd, CFGM_TILEBACKWARDS, (fTileBackwards));
    {
      long th;

      th = (fAutoAddDirs && fAutoAddAllDirs) ? 2 : (fAutoAddDirs) ? 1 : 0;
      WinCheckButton(hwnd, CFGM_RECENTDIRS, th);
      WinSendMsg(hwnd, UM_RESTORE, MPFROM2SHORT(CFGM_RECENTDIRS, 0), MPVOID);
    }
    WinCheckButton(hwnd, CFGM_USERLISTSWITCHES, fUserListSwitches);
    WinCheckButton(hwnd, CFGM_WSANIMATE, (fwsAnimate != 0));
    WinCheckButton(hwnd, CFGM_SEPARATEPARMS, fSeparateParms);
    WinCheckButton(hwnd, CFGM_BLUELED, fBlueLED);
    WinCheckButton(hwnd, CFGM_SHOWTARGET, fShowTarget);
    WinEnableWindow(WinWindowFromID(hwnd, CFGM_STARTMAX), !(fStartMinimized));
    WinEnableWindow(WinWindowFromID(hwnd, CFGM_STARTMIN), !(fStartMaximized));
    return 0;

  case UM_RESTORE:
    {
      long test;
      BOOL th, oh;
      char s[80];

      test = WinQueryButtonCheckstate(hwnd, SHORT1FROMMP(mp1));
      th = (test != 0);
      oh = (test == 1);
      *s = 0;
      switch (SHORT1FROMMP(mp1)) {
      case CFGM_RECENTDIRS:
        sprintf(s,
                GetPString(IDS_RECENTHELPWHICHTEXT),
                (!oh && th) ?
                GetPString(IDS_RECENTONLYTEXT) :
                (oh && th) ?
                GetPString(IDS_ALLONLYTEXT) : GetPString(IDS_NONE));
        break;
      }
      if (*s)
        WinSetDlgItemText(hwnd, SHORT1FROMMP(mp1), s);
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFGM_RECENTDIRS:
      WinSendMsg(hwnd, UM_RESTORE, mp1, MPVOID);
      break;
    case CFGM_STARTMIN:
      if (WinQueryButtonCheckstate(hwnd, CFGM_STARTMIN)) {
        WinCheckButton(hwnd, CFGM_STARTMAX, FALSE);
        WinEnableWindow(WinWindowFromID(hwnd, CFGM_STARTMAX), FALSE);
      }
      else
        WinEnableWindow(WinWindowFromID(hwnd, CFGM_STARTMAX), TRUE);
      break;
    case CFGM_STARTMAX:
      if (WinQueryButtonCheckstate(hwnd, CFGM_STARTMAX)) {
        WinCheckButton(hwnd, CFGM_STARTMIN, FALSE);
        WinEnableWindow(WinWindowFromID(hwnd, CFGM_STARTMIN), FALSE);
      }
      else
        WinEnableWindow(WinWindowFromID(hwnd, CFGM_STARTMIN), TRUE);
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFGM, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    if (hwndMain && !strcmp(realappname, FM3Str)) {
      if (fFreeTree != WinQueryButtonCheckstate(hwnd, CFGM_FREETREE))
        PostMsg(hwndMain, WM_COMMAND, MPFROM2SHORT(IDM_FREETREE, 0), MPVOID);
      if (fAutoTile != WinQueryButtonCheckstate(hwnd, CFGM_AUTOTILE))
        PostMsg(hwndMain, WM_COMMAND, MPFROM2SHORT(IDM_AUTOTILE, 0), MPVOID);
      if (fSplitStatus != WinQueryButtonCheckstate(hwnd, CFGM_SPLITSTATUS)) {
        fSplitStatus = (fSplitStatus) ? FALSE : TRUE;
        PostMsg(hwndMain, WM_COMMAND, MPFROM2SHORT(IDM_BLINK, 0), MPVOID);
        PrfWriteProfileData(fmprof, FM3Str, "SplitStatus", &fSplitStatus,
                            sizeof(BOOL));
      }
    }
    fUserListSwitches = WinQueryButtonCheckstate(hwnd, CFGM_USERLISTSWITCHES);
    PrfWriteProfileData(fmprof, FM3Str, "UserListSwitches",
                        (PVOID) & fUserListSwitches, sizeof(BOOL));
    fExternalINIs = WinQueryButtonCheckstate(hwnd, CFGM_EXTERNALINIS);
    PrfWriteProfileData(fmprof, FM3Str, "ExternalINIs",
                        (PVOID) & fExternalINIs, sizeof(BOOL));
    fExternalArcboxes = WinQueryButtonCheckstate(hwnd, CFGM_EXTERNALARCBOXES);
    PrfWriteProfileData(fmprof, FM3Str, "ExternalArcboxes",
                        (PVOID) & fExternalArcboxes, sizeof(BOOL));
    fExternalCollector =
      WinQueryButtonCheckstate(hwnd, CFGM_EXTERNALCOLLECTOR);
    PrfWriteProfileData(fmprof, FM3Str, "ExternalCollector",
                        (PVOID) & fExternalCollector, sizeof(BOOL));
    fExternalViewer = WinQueryButtonCheckstate(hwnd, CFGM_EXTERNALVIEWER);
    PrfWriteProfileData(fmprof, FM3Str, "ExternalViewer",
                        (PVOID) & fExternalViewer, sizeof(BOOL));
    {
      long test;

      test = WinQueryButtonCheckstate(hwnd, CFGM_RECENTDIRS);
      fAutoAddDirs = (test != 0);
      fAutoAddAllDirs = (test == 2);
    }
    PrfWriteProfileData(fmprof,
                        appname,
                        "AutoAddDirs", (PVOID) & fAutoAddDirs, sizeof(BOOL));
    PrfWriteProfileData(fmprof,
                        appname,
                        "AutoAddAllDirs",
                        (PVOID) & fAutoAddAllDirs, sizeof(BOOL));
    fwsAnimate = WinQueryButtonCheckstate(hwnd, CFGM_WSANIMATE);
    if (fwsAnimate)
      fwsAnimate = WS_ANIMATE;
    PrfWriteProfileData(fmprof,
                        appname,
                        "WS_ANIMATE", (PVOID) & fwsAnimate, sizeof(ULONG));
    fSaveState = WinQueryButtonCheckstate(hwnd, CFGM_SAVESTATE);
    PrfWriteProfileData(fmprof,
                        FM3Str,
                        "SaveState", (PVOID) & fSaveState, sizeof(BOOL));
    fStartMinimized = WinQueryButtonCheckstate(hwnd, CFGM_STARTMIN);
    PrfWriteProfileData(fmprof,
                        appname,
                        "StartMinimized",
                        (PVOID) & fStartMinimized, sizeof(BOOL));
    fStartMaximized = WinQueryButtonCheckstate(hwnd, CFGM_STARTMAX);
    PrfWriteProfileData(fmprof,
                        appname,
                        "StartMaximized",
                        (PVOID) & fStartMaximized, sizeof(BOOL));
    fDataMin = WinQueryButtonCheckstate(hwnd, CFGM_DATAMIN);
    PrfWriteProfileData(fmprof,
                        FM3Str, "DataMin", (PVOID) & fDataMin, sizeof(BOOL));
    fTileBackwards = WinQueryButtonCheckstate(hwnd, CFGM_TILEBACKWARDS);
    PrfWriteProfileData(fmprof,
                        FM3Str,
                        "TileBackwards",
                        (PVOID) & fTileBackwards, sizeof(BOOL));
    fNoTreeGap = WinQueryButtonCheckstate(hwnd, CFGM_NOTREEGAP);
    PrfWriteProfileData(fmprof,
                        FM3Str,
                        "NoTreeGap", (PVOID) & fNoTreeGap, sizeof(BOOL));
    fBlueLED = WinQueryButtonCheckstate(hwnd, CFGM_BLUELED);
    PrfWriteProfileData(fmprof,
                        appname, "BlueLED", (PVOID) & fBlueLED, sizeof(BOOL));
    {
      BOOL dummy;

      dummy = WinQueryButtonCheckstate(hwnd, CFGM_SHOWTARGET);
      if (dummy != fShowTarget) {
        fShowTarget = dummy;
        PrfWriteProfileData(fmprof,
                            appname,
                            "ShowTarget",
                            (PVOID) & fShowTarget, sizeof(BOOL));
        if (hwndMain)
          PostMsg(WinQueryWindow(hwndMain, QW_PARENT),
                  WM_UPDATEFRAME, MPFROMLONG(FCF_SIZEBORDER), MPVOID);
        SetTargetDir(hwnd, TRUE);
      }
      dummy = WinQueryButtonCheckstate(hwnd, CFGM_SEPARATEPARMS);
      if (dummy != fSeparateParms) {
        fSeparateParms = dummy;
        PrfWriteProfileData(fmprof,
                            FM3Str,
                            "SeparateParms",
                            (PVOID) & fSeparateParms, sizeof(BOOL));
        WinSendMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER),
                   UM_UNDO, MPVOID, MPVOID);
      }
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY Cfg5DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static MASK mask;

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CFG5_SUBJECTDISPLAYWIDTH, SPBM_SETTEXTLIMIT,
                      MPFROMSHORT(8), MPVOID);
    WinSendDlgItemMsg(hwnd, CFG5_SUBJECTDISPLAYWIDTH, SPBM_OVERRIDESETLIMITS,
                      MPFROMLONG(1000), MPFROMLONG(50));
    WinSendDlgItemMsg(hwnd,
                      CFG5_FILTER,
                      EM_SETTEXTLIMIT, MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, CFG5_MINIICONS));
    return 0;

  case UM_UNDO:
    {
      ULONG flWindowAttr = 0;
      ULONG ulSize = sizeof(ULONG);

      if (!PrfQueryProfileData(fmprof,
                               appname,
                               "DirflWindowAttr", &flWindowAttr, &ulSize))
        flWindowAttr = (CV_NAME | CV_MINI | CA_DETAILSVIEWTITLES | CV_FLOW);
      if (flWindowAttr & CV_ICON)
        WinCheckButton(hwnd, CFG5_ICON, TRUE);
      if (flWindowAttr & CV_NAME)
        WinCheckButton(hwnd, CFG5_NAME, TRUE);
      if (flWindowAttr & CV_TEXT)
        WinCheckButton(hwnd, CFG5_TEXT, TRUE);
      if (flWindowAttr & CV_DETAIL)
        WinCheckButton(hwnd, CFG5_DETAIL, TRUE);
      if (flWindowAttr & CV_MINI)
        WinCheckButton(hwnd, CFG5_MINIICONS, TRUE);
      if (flWindowAttr & CA_DETAILSVIEWTITLES)
        WinCheckButton(hwnd, CFG5_SHOWTITLES, TRUE);
      WinCheckButton(hwnd, CFG5_SHOWLNAMES, dsDirCnrDefault.detailslongname);
      WinCheckButton(hwnd, CFG5_SHOWSUBJECT, dsDirCnrDefault.detailssubject);
      WinCheckButton(hwnd, CFG5_SHOWEAS, dsDirCnrDefault.detailsea);
      WinCheckButton(hwnd, CFG5_SHOWSIZE, dsDirCnrDefault.detailssize);
      WinCheckButton(hwnd, CFG5_SHOWICON, dsDirCnrDefault.detailsicon);
      WinCheckButton(hwnd, CFG5_SHOWLWDATE, dsDirCnrDefault.detailslwdate);
      WinCheckButton(hwnd, CFG5_SHOWLWTIME, dsDirCnrDefault.detailslwtime);
      WinCheckButton(hwnd, CFG5_SHOWLADATE, dsDirCnrDefault.detailsladate);
      WinCheckButton(hwnd, CFG5_SHOWLATIME, dsDirCnrDefault.detailslatime);
      WinCheckButton(hwnd, CFG5_SHOWCRDATE, dsDirCnrDefault.detailscrdate);
      WinCheckButton(hwnd, CFG5_SHOWCRTIME, dsDirCnrDefault.detailscrtime);
      WinCheckButton(hwnd, CFG5_SHOWATTR, dsDirCnrDefault.detailsattr);
      memset(&mask, 0, sizeof(mask));
      mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
        FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
      ulSize = sizeof(MASK);
      if (PrfQueryProfileData(fmprof, appname, "DirFilter", &mask, &ulSize))
        SetMask(NULL, &mask);
      if (!mask.attrFile)
        mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
          FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
      strcpy(mask.prompt, GetPString(IDS_DEFDIRFILTERTITLETEXT));
      WinSetDlgItemText(hwnd, CFG5_FILTER, mask.szMask);
      WinCheckButton(hwnd, CFG5_SUBJECTINLEFTPANE, dsDirCnrDefault.fSubjectInLeftPane);
      WinCheckButton(hwnd, CFG5_SUBJECTLENGTHMAX, dsDirCnrDefault.fSubjectLengthMax);
      WinSendDlgItemMsg(hwnd, CFG5_SUBJECTDISPLAYWIDTH, SPBM_SETCURRENTVALUE,
                        MPFROMLONG(dsDirCnrDefault.SubjectDisplayWidth), MPVOID);
    }
    return 0;

  case UM_SETUP5:
    if (WinDlgBox(HWND_DESKTOP, hwndNotebook, PickMaskDlgProc,
                  FM3ModHandle, MSK_FRAME, MPFROMP(&mask))) {
      SetMask(NULL, &mask);
      WinSetDlgItemText(hwnd, CFG5_FILTER, mask.szMask);
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFG5_FILTER:
      switch (SHORT2FROMMP(mp1)) {
      case EN_SETFOCUS:
        PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
        PostMsg(hwnd, UM_SETUP5, MPVOID, MPVOID);
        break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFG5, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    {
      ULONG flWindowAttr = 0;

      if (WinQueryButtonCheckstate(hwnd, CFG5_ICON))
        flWindowAttr |= CV_ICON;
      if (WinQueryButtonCheckstate(hwnd, CFG5_NAME))
        flWindowAttr |= CV_NAME;
      if (WinQueryButtonCheckstate(hwnd, CFG5_TEXT))
        flWindowAttr |= CV_TEXT;
      if (WinQueryButtonCheckstate(hwnd, CFG5_DETAIL))
        flWindowAttr |= CV_DETAIL;
      if (WinQueryButtonCheckstate(hwnd, CFG5_MINIICONS))
        flWindowAttr |= CV_MINI;
      if (WinQueryButtonCheckstate(hwnd, CFG5_SHOWTITLES))
        flWindowAttr |= CA_DETAILSVIEWTITLES;
      flWindowAttr |= CV_FLOW;
      PrfWriteProfileData(fmprof,
                          appname,
                          "DirflWindowAttr", &flWindowAttr, sizeof(ULONG));
    }
    dsDirCnrDefault.detailslongname = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLNAMES);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLongname",
                        &dsDirCnrDefault.detailslongname, sizeof(BOOL));
    dsDirCnrDefault.detailssubject = WinQueryButtonCheckstate(hwnd, CFG5_SHOWSUBJECT);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsSubject",
                        &dsDirCnrDefault.detailssubject, sizeof(BOOL));
    dsDirCnrDefault.detailsea = WinQueryButtonCheckstate(hwnd, CFG5_SHOWSUBJECT);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsEA",
                        &dsDirCnrDefault.detailsea, sizeof(BOOL));
    dsDirCnrDefault.detailssize = WinQueryButtonCheckstate(hwnd, CFG5_SHOWSIZE);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsSize",
                        &dsDirCnrDefault.detailssize, sizeof(BOOL));
    dsDirCnrDefault.detailsicon = WinQueryButtonCheckstate(hwnd, CFG5_SHOWICON);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsIcon",
                        &dsDirCnrDefault.detailsicon, sizeof(BOOL));
    dsDirCnrDefault.detailslwdate = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLWDATE);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLWDate",
                        &dsDirCnrDefault.detailslwdate, sizeof(BOOL));
    dsDirCnrDefault.detailslwtime = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLWTIME);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLWTime",
                        &dsDirCnrDefault.detailslwtime, sizeof(BOOL));
    dsDirCnrDefault.detailsladate = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLADATE);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLADate",
                        &dsDirCnrDefault.detailsladate, sizeof(BOOL));
    dsDirCnrDefault.detailslatime = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLATIME);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLATime",
                        &dsDirCnrDefault.detailslatime, sizeof(BOOL));
    dsDirCnrDefault.detailscrdate = WinQueryButtonCheckstate(hwnd, CFG5_SHOWCRDATE);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsCRDate",
                        &dsDirCnrDefault.detailscrdate, sizeof(BOOL));
    dsDirCnrDefault.detailscrtime = WinQueryButtonCheckstate(hwnd, CFG5_SHOWCRTIME);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsCRTime",
                        &dsDirCnrDefault.detailscrtime, sizeof(BOOL));
    dsDirCnrDefault.detailsattr = WinQueryButtonCheckstate(hwnd, CFG5_SHOWATTR);
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsAttr",
                        &dsDirCnrDefault.detailsattr, sizeof(BOOL));
    dsDirCnrDefault.fSubjectInLeftPane = WinQueryButtonCheckstate(hwnd, CFG5_SUBJECTINLEFTPANE);
    PrfWriteProfileData(fmprof, appname, "DirCnr.SubjectInLeftPane",
                        &dsDirCnrDefault.fSubjectInLeftPane, sizeof(BOOL));
    dsDirCnrDefault.fSubjectLengthMax = WinQueryButtonCheckstate(hwnd, CFG5_SUBJECTLENGTHMAX);
    PrfWriteProfileData(fmprof, appname, "DirCnr.SubjectLengthMax",
                        &dsDirCnrDefault.fSubjectLengthMax, sizeof(BOOL));
    *mask.prompt = 0;
    PrfWriteProfileData(fmprof, appname, "DirFilter", &mask, sizeof(MASK));
    {
        if (!WinQueryButtonCheckstate(hwnd, CFG5_SUBJECTLENGTHMAX)) {
          WinSendDlgItemMsg(hwnd, CFG5_SUBJECTDISPLAYWIDTH, SPBM_QUERYVALUE,
                            MPFROMP(&dsDirCnrDefault.SubjectDisplayWidth), MPFROM2SHORT(0, SPBQ_DONOTUPDATE));
          if (dsDirCnrDefault.SubjectDisplayWidth < 50)
            dsDirCnrDefault.SubjectDisplayWidth = 0;
          else if (dsDirCnrDefault.SubjectDisplayWidth > 1000)
            dsDirCnrDefault.SubjectDisplayWidth = 1000;
        }
        else
          dsDirCnrDefault.SubjectDisplayWidth = 0;
        PrfWriteProfileData(fmprof,
                            appname, "DirCnr.SubjectDisplayWidth",
                            &dsDirCnrDefault.SubjectDisplayWidth, sizeof(ULONG));
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY Cfg6DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, CFG6_SORTFIRST, FALSE);
    WinCheckButton(hwnd, CFG6_SORTLAST, FALSE);
    WinCheckButton(hwnd, CFG6_SORTSIZE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTEASIZE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTLWDATE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTLADATE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTCRDATE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTNAME, FALSE);
    WinCheckButton(hwnd, CFG6_SORTDIRSFIRST, FALSE);
    WinCheckButton(hwnd, CFG6_SORTDIRSLAST, FALSE);
    WinCheckButton(hwnd, CFG6_SORTREVERSE, FALSE);
    if (sortFlags & SORT_FIRSTEXTENSION)
      WinCheckButton(hwnd, CFG6_SORTFIRST, TRUE);
    else if (sortFlags & SORT_LASTEXTENSION)
      WinCheckButton(hwnd, CFG6_SORTLAST, TRUE);
    else if (sortFlags & SORT_SIZE)
      WinCheckButton(hwnd, CFG6_SORTSIZE, TRUE);
    else if (sortFlags & SORT_EASIZE)
      WinCheckButton(hwnd, CFG6_SORTEASIZE, TRUE);
    else if (sortFlags & SORT_LWDATE)
      WinCheckButton(hwnd, CFG6_SORTLWDATE, TRUE);
    else if (sortFlags & SORT_LADATE)
      WinCheckButton(hwnd, CFG6_SORTLADATE, TRUE);
    else if (sortFlags & SORT_CRDATE)
      WinCheckButton(hwnd, CFG6_SORTCRDATE, TRUE);
    else if (sortFlags & SORT_FILENAME)
      WinCheckButton(hwnd, CFG6_SORTFILENAME, TRUE);
    else
      WinCheckButton(hwnd, CFG6_SORTNAME, TRUE);
    if (sortFlags & SORT_DIRSFIRST)
      WinCheckButton(hwnd, CFG6_SORTDIRSFIRST, TRUE);
    else if (sortFlags & SORT_DIRSLAST)
      WinCheckButton(hwnd, CFG6_SORTDIRSLAST, TRUE);
    if (sortFlags & SORT_REVERSE)
      WinCheckButton(hwnd, CFG6_SORTREVERSE, TRUE);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFG6_SORTDIRSFIRST:
    case CFG6_SORTDIRSLAST:
      {
        BOOL temp;

        temp = WinQueryButtonCheckstate(hwnd, SHORT1FROMMP(mp1));
        if (temp) {
          switch (SHORT1FROMMP(mp1)) {
          case CFG6_SORTDIRSFIRST:
            WinCheckButton(hwnd, CFG6_SORTDIRSLAST, FALSE);
            break;
          case CFG6_SORTDIRSLAST:
            WinCheckButton(hwnd, CFG6_SORTDIRSFIRST, FALSE);
            break;
          }
        }
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFG6, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    sortFlags = 0;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTFILENAME))
      sortFlags |= SORT_FILENAME;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTSIZE))
      sortFlags |= SORT_SIZE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTEASIZE))
      sortFlags |= SORT_EASIZE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTFIRST))
      sortFlags |= SORT_FIRSTEXTENSION;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTLAST))
      sortFlags |= SORT_LASTEXTENSION;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTLWDATE))
      sortFlags |= SORT_LWDATE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTLADATE))
      sortFlags |= SORT_LADATE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTCRDATE))
      sortFlags |= SORT_CRDATE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTDIRSFIRST))
      sortFlags |= SORT_DIRSFIRST;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTDIRSLAST))
      sortFlags |= SORT_DIRSLAST;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTREVERSE))
      sortFlags |= SORT_REVERSE;
    PrfWriteProfileData(fmprof, appname, "Sort", &sortFlags, sizeof(INT));
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY Cfg7DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  static MASK mask;

  switch (msg) {
  case WM_INITDLG:
    WinSendDlgItemMsg(hwnd, CFG5_SUBJECTDISPLAYWIDTH, SPBM_SETTEXTLIMIT,
                      MPFROMSHORT(8), MPVOID);
    WinSendDlgItemMsg(hwnd, CFG5_SUBJECTDISPLAYWIDTH, SPBM_OVERRIDESETLIMITS,
                      MPFROMLONG(1000), MPFROMLONG(50));
    WinSendDlgItemMsg(hwnd, CFG5_FILTER, EM_SETTEXTLIMIT,
                      MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_FOCUSME:
    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, CFG5_MINIICONS));
    return 0;

  case UM_UNDO:
    WinCheckButton(hwnd, CFG5_EXTERNALCOLLECTOR, fExternalCollector);
    {
      ULONG flWindowAttr = 0;
      ULONG ulSize = sizeof(ULONG);

      if (!PrfQueryProfileData(fmprof,
                               appname,
                               "CollectorflWindowAttr", &flWindowAttr, &ulSize))
        flWindowAttr = (CV_NAME | CA_DETAILSVIEWTITLES | CV_MINI | CV_FLOW);
      if (flWindowAttr & CV_ICON)
        WinCheckButton(hwnd, CFG5_ICON, TRUE);
      if (flWindowAttr & CV_NAME)
        WinCheckButton(hwnd, CFG5_NAME, TRUE);
      if (flWindowAttr & CV_TEXT)
        WinCheckButton(hwnd, CFG5_TEXT, TRUE);
      if (flWindowAttr & CV_DETAIL)
        WinCheckButton(hwnd, CFG5_DETAIL, TRUE);
      if (flWindowAttr & CV_MINI)
        WinCheckButton(hwnd, CFG5_MINIICONS, TRUE);
      if (flWindowAttr & CA_DETAILSVIEWTITLES)
        WinCheckButton(hwnd, CFG5_SHOWTITLES, TRUE);
      memset(&mask, 0, sizeof(mask));
      mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
        FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
      ulSize = sizeof(MASK);
      if (PrfQueryProfileData(fmprof,
                              appname, "CollectorFilter", &mask, &ulSize)) {
        SetMask(NULL, &mask);
      }
      if (!mask.attrFile)
        mask.attrFile = FILE_DIRECTORY | FILE_ARCHIVED | FILE_HIDDEN |
          FILE_SYSTEM | FILE_NORMAL | FILE_READONLY;
      strcpy(mask.prompt, GetPString(IDS_DEFCOLFILTERTITLETEXT));
      WinSetDlgItemText(hwnd, CFG5_FILTER, mask.szMask);
    }
    {
      DETAILS_SETTINGS ds;

      memset(&ds, 0, sizeof(ds));
      LoadDetailsSwitches("Collector", &ds);
      WinCheckButton(hwnd, CFG5_SHOWLNAMES, ds.detailslongname);
      WinCheckButton(hwnd, CFG5_SHOWSUBJECT, ds.detailssubject);
      WinCheckButton(hwnd, CFG5_SHOWEAS, ds.detailsea);
      WinCheckButton(hwnd, CFG5_SHOWSIZE, ds.detailssize);
      WinCheckButton(hwnd, CFG5_SHOWICON, ds.detailsicon);
      WinCheckButton(hwnd, CFG5_SHOWLWDATE, ds.detailslwdate);
      WinCheckButton(hwnd, CFG5_SHOWLWTIME, ds.detailslwtime);
      WinCheckButton(hwnd, CFG5_SHOWLADATE, ds.detailsladate);
      WinCheckButton(hwnd, CFG5_SHOWLATIME, ds.detailslatime);
      WinCheckButton(hwnd, CFG5_SHOWCRDATE, ds.detailscrdate);
      WinCheckButton(hwnd, CFG5_SHOWCRTIME, ds.detailscrtime);
      WinCheckButton(hwnd, CFG5_SHOWATTR, ds.detailsattr);
      WinCheckButton(hwnd, CFG5_SUBJECTINLEFTPANE, ds.fSubjectInLeftPane);
      WinCheckButton(hwnd, CFG5_SUBJECTLENGTHMAX, ds.fSubjectLengthMax);
      WinSendDlgItemMsg(hwnd, CFG5_SUBJECTDISPLAYWIDTH, SPBM_SETCURRENTVALUE,
                        MPFROMLONG(ds.SubjectDisplayWidth), MPVOID);

    }
    return 0;

  case UM_SETUP5:
    if (WinDlgBox(HWND_DESKTOP, hwndNotebook, PickMaskDlgProc,
                  FM3ModHandle, MSK_FRAME, MPFROMP(&mask))) {
      SetMask(NULL, &mask);
      WinSetDlgItemText(hwnd, CFG5_FILTER, mask.szMask);
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFG5_FILTER:
      switch (SHORT2FROMMP(mp1)) {
      case EN_SETFOCUS:
        PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
        PostMsg(hwnd, UM_SETUP5, MPVOID, MPVOID);
        break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFG7, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    fExternalCollector = WinQueryButtonCheckstate(hwnd,
                                                  CFG5_EXTERNALCOLLECTOR);
    PrfWriteProfileData(fmprof, FM3Str, "ExternalCollector",
                        &fExternalCollector, sizeof(BOOL));
    {
      ULONG flWindowAttr = 0;

      if (WinQueryButtonCheckstate(hwnd, CFG5_ICON))
        flWindowAttr |= CV_ICON;
      if (WinQueryButtonCheckstate(hwnd, CFG5_NAME))
        flWindowAttr |= CV_NAME;
      if (WinQueryButtonCheckstate(hwnd, CFG5_TEXT))
        flWindowAttr |= CV_TEXT;
      if (WinQueryButtonCheckstate(hwnd, CFG5_DETAIL))
        flWindowAttr |= CV_DETAIL;
      if (WinQueryButtonCheckstate(hwnd, CFG5_MINIICONS))
        flWindowAttr |= CV_MINI;
      if (WinQueryButtonCheckstate(hwnd, CFG5_SHOWTITLES))
        flWindowAttr |= CA_DETAILSVIEWTITLES;
      flWindowAttr |= CV_FLOW;
      PrfWriteProfileData(fmprof, appname, "CollectorflWindowAttr",
                          &flWindowAttr, sizeof(ULONG));
    }
    {
      DETAILS_SETTINGS ds;

      memset(&ds, 0, sizeof(ds));
      ds.detailslongname = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLNAMES);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsLongname",
                          &ds.detailslongname, sizeof(BOOL));
      ds.detailssubject = WinQueryButtonCheckstate(hwnd, CFG5_SHOWSUBJECT);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsSubject",
                          &ds.detailssubject, sizeof(BOOL));
      ds.detailsea = WinQueryButtonCheckstate(hwnd, CFG5_SHOWEAS);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsEA",
                          &ds.detailsea, sizeof(BOOL));
      ds.detailssize = WinQueryButtonCheckstate(hwnd, CFG5_SHOWSIZE);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsSize",
                          &ds.detailssize, sizeof(BOOL));
      ds.detailsicon = WinQueryButtonCheckstate(hwnd, CFG5_SHOWICON);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsIcon",
                          &ds.detailsicon, sizeof(BOOL));
      ds.detailslwdate = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLWDATE);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsLWDate",
                          &ds.detailslwdate, sizeof(BOOL));
      ds.detailslwtime = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLWTIME);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsLWTime",
                          &ds.detailslwtime, sizeof(BOOL));
      ds.detailsladate = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLADATE);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsLADate",
                          &ds.detailsladate, sizeof(BOOL));
      ds.detailslatime = WinQueryButtonCheckstate(hwnd, CFG5_SHOWLATIME);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsLATime",
                          &ds.detailslatime, sizeof(BOOL));
      ds.detailscrdate = WinQueryButtonCheckstate(hwnd, CFG5_SHOWCRDATE);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsCRDate",
                          &ds.detailscrdate, sizeof(BOOL));
      ds.detailscrtime = WinQueryButtonCheckstate(hwnd, CFG5_SHOWCRTIME);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsCRTime",
                          &ds.detailscrtime, sizeof(BOOL));
      ds.detailsattr = WinQueryButtonCheckstate(hwnd, CFG5_SHOWATTR);
      PrfWriteProfileData(fmprof, appname, "Collector.DetailsAttr",
                          &ds.detailsattr, sizeof(BOOL));
      ds.fSubjectInLeftPane = WinQueryButtonCheckstate(hwnd, CFG5_SUBJECTINLEFTPANE);
      PrfWriteProfileData(fmprof, appname, "Collector.SubjectInLeftPane",
                          &ds.fSubjectInLeftPane, sizeof(BOOL));
      ds.fSubjectLengthMax = WinQueryButtonCheckstate(hwnd, CFG5_SUBJECTLENGTHMAX);
      PrfWriteProfileData(fmprof, appname, "Collector.SubjectLengthMax",
                          &ds.fSubjectLengthMax, sizeof(BOOL));
      *mask.prompt = 0;
      PrfWriteProfileData(fmprof,
                          appname, "CollectorFilter", &mask, sizeof(MASK));
      {
        if (!WinQueryButtonCheckstate(hwnd, CFG5_SUBJECTLENGTHMAX)) {
          WinSendDlgItemMsg(hwnd, CFG5_SUBJECTDISPLAYWIDTH, SPBM_QUERYVALUE,
                            MPFROMP(&ds.SubjectDisplayWidth), MPFROM2SHORT(0, SPBQ_DONOTUPDATE));
          if (ds.SubjectDisplayWidth < 50)
            ds.SubjectDisplayWidth = 0;
          else if (ds.SubjectDisplayWidth > 1000)
            ds.SubjectDisplayWidth = 1000;
        }
        else
          ds.SubjectDisplayWidth = 0;
        PrfWriteProfileData(fmprof,
                            appname, "Collector.SubjectDisplayWidth",
                            &ds.SubjectDisplayWidth, sizeof(ULONG));
      }
    }
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY Cfg8DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case UM_UNDO:
    WinCheckButton(hwnd, CFG6_SORTFIRST, FALSE);
    WinCheckButton(hwnd, CFG6_SORTLAST, FALSE);
    WinCheckButton(hwnd, CFG6_SORTSIZE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTEASIZE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTLWDATE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTLADATE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTCRDATE, FALSE);
    WinCheckButton(hwnd, CFG6_SORTNAME, FALSE);
    WinCheckButton(hwnd, CFG6_SORTDIRSFIRST, FALSE);
    WinCheckButton(hwnd, CFG6_SORTDIRSLAST, FALSE);
    WinCheckButton(hwnd, CFG6_SORTREVERSE, FALSE);
    if (CollectorsortFlags & SORT_FIRSTEXTENSION)
      WinCheckButton(hwnd, CFG6_SORTFIRST, TRUE);
    else if (CollectorsortFlags & SORT_LASTEXTENSION)
      WinCheckButton(hwnd, CFG6_SORTLAST, TRUE);
    else if (CollectorsortFlags & SORT_SIZE)
      WinCheckButton(hwnd, CFG6_SORTSIZE, TRUE);
    else if (CollectorsortFlags & SORT_EASIZE)
      WinCheckButton(hwnd, CFG6_SORTEASIZE, TRUE);
    else if (CollectorsortFlags & SORT_LWDATE)
      WinCheckButton(hwnd, CFG6_SORTLWDATE, TRUE);
    else if (CollectorsortFlags & SORT_LADATE)
      WinCheckButton(hwnd, CFG6_SORTLADATE, TRUE);
    else if (CollectorsortFlags & SORT_CRDATE)
      WinCheckButton(hwnd, CFG6_SORTCRDATE, TRUE);
    else if (CollectorsortFlags & SORT_FILENAME)
      WinCheckButton(hwnd, CFG6_SORTFILENAME, TRUE);
    else
      WinCheckButton(hwnd, CFG6_SORTNAME, TRUE);
    if (CollectorsortFlags & SORT_DIRSFIRST)
      WinCheckButton(hwnd, CFG6_SORTDIRSFIRST, TRUE);
    else if (CollectorsortFlags & SORT_DIRSLAST)
      WinCheckButton(hwnd, CFG6_SORTDIRSLAST, TRUE);
    if (CollectorsortFlags & SORT_REVERSE)
      WinCheckButton(hwnd, CFG6_SORTREVERSE, TRUE);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case CFG6_SORTDIRSFIRST:
    case CFG6_SORTDIRSLAST:
      {
        BOOL temp;

        temp = WinQueryButtonCheckstate(hwnd, SHORT1FROMMP(mp1));
        if (temp) {
          switch (SHORT1FROMMP(mp1)) {
          case CFG6_SORTDIRSFIRST:
            WinCheckButton(hwnd, CFG6_SORTDIRSLAST, FALSE);
            break;
          case CFG6_SORTDIRSLAST:
            WinCheckButton(hwnd, CFG6_SORTDIRSFIRST, FALSE);
            break;
          }
        }
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFG8, 0), MPFROMSHORT(HM_RESOURCEID));
      break;
    }
    return 0;

  case WM_CLOSE:
    CollectorsortFlags = 0;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTFILENAME))
      CollectorsortFlags |= SORT_FILENAME;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTSIZE))
      CollectorsortFlags |= SORT_SIZE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTEASIZE))
      CollectorsortFlags |= SORT_EASIZE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTFIRST))
      CollectorsortFlags |= SORT_FIRSTEXTENSION;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTLAST))
      CollectorsortFlags |= SORT_LASTEXTENSION;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTLWDATE))
      CollectorsortFlags |= SORT_LWDATE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTLADATE))
      CollectorsortFlags |= SORT_LADATE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTCRDATE))
      CollectorsortFlags |= SORT_CRDATE;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTDIRSFIRST))
      CollectorsortFlags |= SORT_DIRSFIRST;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTDIRSLAST))
      CollectorsortFlags |= SORT_DIRSLAST;
    if (WinQueryButtonCheckstate(hwnd, CFG6_SORTREVERSE))
      CollectorsortFlags |= SORT_REVERSE;
    PrfWriteProfileData(fmprof,
                        appname,
                        "CollectorSort", &CollectorsortFlags, sizeof(INT));
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

MRESULT EXPENTRY Cfg9DlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case WM_INITDLG:
    break;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_CFG9, 0), MPFROMSHORT(HM_RESOURCEID));
      return 0;
    case CFG9_MAXIMUMUI:
      if (hwndMain) {
        if (MenuInvisible)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_HIDEMENU, 0), MPVOID);
        if (!fAutoView)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_AUTOVIEW, 0), MPVOID);
        if (!fDrivebar)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_DRIVEBAR, 0), MPVOID);
        if (!fToolbar)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_TOOLBAR, 0), MPVOID);
        if (!fMoreButtons)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_MOREBUTTONS, 0), MPVOID);
        if (!fUserComboBox)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_USERLIST, 0), MPVOID);
      }
      return 0;
    case CFG9_MINIMUMUI:
      if (hwndMain) {
        if (!MenuInvisible)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_HIDEMENU, 0), MPVOID);
        if (fAutoView)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_AUTOVIEW, 0), MPVOID);
        if (fToolbar)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_TOOLBAR, 0), MPVOID);
        if (fMoreButtons)
          WinSendMsg(hwndMain, WM_COMMAND, MPFROM2SHORT(IDM_MOREBUTTONS, 0),
                     MPVOID);
        if (fUserComboBox)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_USERLIST, 0), MPVOID);
        saymsg(MB_ENTER | MB_ICONASTERISK,
               hwnd,
               GetPString(IDS_DONTFORGETTEXT),
               GetPString(IDS_UNHIDEMENUWARNTEXT));
      }
      return 0;
    case CFG9_MAXINFOPRETTY:
      fLoadSubject = TRUE;
      fLoadLongnames = TRUE;
      fNoIconsFiles = FALSE;
      fNoIconsDirs = FALSE;
      fForceUpper = FALSE;
      fForceLower = FALSE;
      fArcStuffVisible = TRUE;
      fSplitStatus = TRUE;
      fDragndropDlg = TRUE;
      {
        ULONG flWindowAttr;

        flWindowAttr = CV_DETAIL | CV_FLOW | CA_DETAILSVIEWTITLES;
        PrfWriteProfileData(fmprof,
                            appname,
                            "DirflWindowAttr", &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof,
                            appname,
                            "CollectorflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof,
                            appname, "Collector.Fontnamesize", NULL, 0);
      }
      dsDirCnrDefault.detailslongname = TRUE;
      dsDirCnrDefault.detailssubject = TRUE;
      dsDirCnrDefault.detailsea = TRUE;
      dsDirCnrDefault.detailssize = TRUE;
      dsDirCnrDefault.detailsicon = TRUE;
      dsDirCnrDefault.detailslwdate = TRUE;
      dsDirCnrDefault.detailslwtime = TRUE;
      dsDirCnrDefault.detailsladate = TRUE;
      dsDirCnrDefault.detailslatime = TRUE;
      dsDirCnrDefault.detailscrdate = TRUE;
      dsDirCnrDefault.detailscrtime = TRUE;
      dsDirCnrDefault.detailsattr = TRUE;
      if (hwndTree) {

        CNRINFO cnri;
        ULONG flWindowAttr = CV_TREE | CV_ICON | CV_FLOW | CA_TREELINE;

        memset(&cnri, 0, sizeof(cnri));
        cnri.cb = sizeof(cnri);
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(cnri)));
        cnri.flWindowAttr = flWindowAttr;
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_SETCNRINFO, MPFROMP(&cnri),
                   MPFROMLONG(CMA_FLWINDOWATTR));
      }
      break;

    case CFG9_MAXINFOPLAIN:
      fLoadSubject = TRUE;
      fLoadLongnames = TRUE;
      fNoIconsFiles = TRUE;
      fNoIconsDirs = TRUE;
      fForceUpper = FALSE;
      fForceLower = FALSE;
      fArcStuffVisible = TRUE;
      fSplitStatus = TRUE;
      fDragndropDlg = TRUE;
      {
        ULONG flWindowAttr;

        flWindowAttr = CV_DETAIL | CV_FLOW | CV_MINI;
        PrfWriteProfileData(fmprof,
                            appname,
                            "DirflWindowAttr", &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof,
                            appname,
                            "CollectorflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof,
                            appname,
                            "Collector.Fontnamesize",
                            GetPString(IDS_8HELVTEXT),
                            strlen(GetPString(IDS_8HELVTEXT)) + 1);
      }
      dsDirCnrDefault.detailslongname = TRUE;
      dsDirCnrDefault.detailssubject = TRUE;
      dsDirCnrDefault.detailsea = TRUE;
      dsDirCnrDefault.detailssize = TRUE;
      dsDirCnrDefault.detailsicon = TRUE;
      dsDirCnrDefault.detailslwdate = TRUE;
      dsDirCnrDefault.detailslwtime = TRUE;
      dsDirCnrDefault.detailsladate = TRUE;
      dsDirCnrDefault.detailslatime = TRUE;
      dsDirCnrDefault.detailscrdate = TRUE;
      dsDirCnrDefault.detailscrtime = TRUE;
      dsDirCnrDefault.detailsattr = TRUE;
      if (hwndTree) {

        CNRINFO cnri;
        ULONG flWindowAttr = CV_TREE | CV_MINI | CV_TEXT |
          CV_FLOW | CA_TREELINE;

        memset(&cnri, 0, sizeof(cnri));
        cnri.cb = sizeof(cnri);
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(cnri)));
        cnri.flWindowAttr = flWindowAttr;
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_SETCNRINFO, MPFROMP(&cnri),
                   MPFROMLONG(CMA_FLWINDOWATTR));
      }
      break;
    case CFG9_MAXFILENAMES:
      if (hwndMain && fAutoView)
        WinSendMsg(hwndMain, WM_COMMAND,
                   MPFROM2SHORT(IDM_AUTOVIEW, 0), MPVOID);
      fForceUpper = FALSE;
      fForceLower = TRUE;
      fExternalViewer = TRUE;
      fExternalArcboxes = TRUE;
      fExternalCollector = TRUE;
      fExternalINIs = TRUE;
      fLoadSubject = FALSE;
      fLoadLongnames = FALSE;
      fNoIconsFiles = TRUE;
      fNoIconsDirs = TRUE;
      {
        ULONG flWindowAttr;

        flWindowAttr = CV_TEXT | CV_FLOW;
        PrfWriteProfileData(fmprof, appname, "DirflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof, appname, "CollectorflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof, appname, "Collector.Fontnamesize",
                            GetPString(IDS_8HELVTEXT),
                            strlen(GetPString(IDS_8HELVTEXT)) + 1);
      }
      if (hwndTree) {

        CNRINFO cnri;
        ULONG flWindowAttr = CV_TREE | CV_TEXT | CV_FLOW | CA_TREELINE;

        memset(&cnri, 0, sizeof(cnri));
        cnri.cb = sizeof(cnri);
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(cnri)));
        cnri.flWindowAttr = flWindowAttr;
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_SETCNRINFO, MPFROMP(&cnri),
                   MPFROMLONG(CMA_FLWINDOWATTR));
      }
      break;
    case CFG9_MAXSPEED:
      fLoadSubject = FALSE;
      fLoadLongnames = FALSE;
      fVerify = FALSE;
      DosSetVerify(FALSE);
      FilesToGet = FILESTOGET_MAX;
      fQuickArcFind = TRUE;
      fMinOnOpen = TRUE;
      fRealIdle = FALSE;
      fNoIconsFiles = TRUE;
      fNoIconsDirs = TRUE;
      fSyncUpdates = FALSE;
      fArcStuffVisible = FALSE;
      fForceUpper = FALSE;
      fForceLower = FALSE;
      dsDirCnrDefault.detailslongname = FALSE;
      dsDirCnrDefault.detailssubject = FALSE;
      break;

    case CFG9_HECTOR:
      fSwitchTree = TRUE;
      fSwitchTreeOnFocus = FALSE;
      fSwitchTreeExpand = TRUE;
      fCollapseFirst = TRUE;
      fSelectedAlways = FALSE;
      fTileBackwards = FALSE;
      fExternalViewer = FALSE;
      fExternalArcboxes = TRUE;
      fExternalCollector = FALSE;
      fExternalINIs = TRUE;
      fCopyDefault = FALSE;
      fFollowTree = FALSE;
      fLoadSubject = FALSE;
      fLoadLongnames = FALSE;
      fDontMoveMouse = FALSE;
      fUnHilite = TRUE;
      fUserListSwitches = TRUE;
      fDCOpens = FALSE;
      fLinkSetsIcon = FALSE;
      fConfirmDelete = TRUE;
      fSyncUpdates = FALSE;
      fRealIdle = FALSE;
      fNoIconsFiles = FALSE;
      fNoIconsDirs = TRUE;
      fFolderAfterExtract = FALSE;
      fVerify = TRUE;
      DosSetVerify(TRUE);
      fForceUpper = FALSE;
      fForceLower = TRUE;
      fArcStuffVisible = FALSE;
      fVTreeOpensWPS = FALSE;
      fRemoteBug = FALSE;
      fDragndropDlg = TRUE;
      fMinOnOpen = FALSE;
      fQuickArcFind = TRUE;
      fNoRemovableScan = TRUE;
      FilesToGet = FILESTOGET_MIN;
      fFreeTree = FALSE;
      fSplitStatus = TRUE;
      fAutoTile = TRUE;
      fSaveState = TRUE;
      fStartMinimized = FALSE;
      fStartMaximized = FALSE;
      fDataMin = FALSE;
      ulCnrType = CCS_EXTENDSEL | CCS_MULTIPLESEL;
      fNoTreeGap = TRUE;
      {
        ULONG flWindowAttr;

        flWindowAttr = (CV_NAME | CV_MINI | CV_FLOW | CA_DETAILSVIEWTITLES);
        PrfWriteProfileData(fmprof, appname, "DirflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof, appname, "CollectorflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
      }
      dsDirCnrDefault.detailslongname = FALSE;
      dsDirCnrDefault.detailssubject = FALSE;
      dsDirCnrDefault.detailsea = TRUE;
      dsDirCnrDefault.detailssize = TRUE;
      dsDirCnrDefault.detailsicon = TRUE;
      dsDirCnrDefault.detailslwdate = TRUE;
      dsDirCnrDefault.detailslwtime = TRUE;
      dsDirCnrDefault.detailsladate = FALSE;
      dsDirCnrDefault.detailslatime = FALSE;
      dsDirCnrDefault.detailscrdate = FALSE;
      dsDirCnrDefault.detailscrtime = FALSE;
      dsDirCnrDefault.detailsattr = TRUE;
      sortFlags = SORT_FILENAME | SORT_DIRSFIRST;
      CollectorsortFlags = SORT_FILENAME | SORT_DIRSFIRST;
      if (hwndMain) {

        SWP swp;

        if (WinQueryWindowPos(hwndMain, &swp)) {
          WinSetWindowPos(hwndTree, HWND_TOP, 0, 0,
                          swp.cx / 5, swp.cy, SWP_MOVE | SWP_SIZE);
        }
        if (MenuInvisible)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_HIDEMENU, 0), MPVOID);
        if (fAutoView)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_AUTOVIEW, 0), MPVOID);
        if (fToolbar)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_TOOLBAR, 0), MPVOID);
        if (!fDrivebar)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_DRIVEBAR, 0), MPVOID);
        if (!fMoreButtons)
          WinSendMsg(hwndMain, WM_COMMAND, MPFROM2SHORT(IDM_MOREBUTTONS, 0),
                     MPVOID);
        if (!fUserComboBox)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_USERLIST, 0), MPVOID);
      }
      if (hwndTree) {

        CNRINFO cnri;
        ULONG flWindowAttr = (CV_TREE | CV_TEXT | CV_MINI |
                              CV_FLOW | CA_TREELINE);

        memset(&cnri, 0, sizeof(cnri));
        cnri.cb = sizeof(cnri);
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(cnri)));
        cnri.flWindowAttr = flWindowAttr;
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_SETCNRINFO, MPFROMP(&cnri),
                   MPFROMLONG(CMA_FLWINDOWATTR));
      }
      break;

    case CFG9_DEFAULT:
      fSwitchTree = FALSE;
      fSwitchTreeOnFocus = FALSE;
      fSwitchTreeExpand = FALSE;
      fCollapseFirst = FALSE;
      fSelectedAlways = FALSE;
      fTileBackwards = FALSE;
      fExternalViewer = FALSE;
      fExternalArcboxes = FALSE;
      fExternalCollector = FALSE;
      fExternalINIs = FALSE;
      fCopyDefault = FALSE;
      fFollowTree = FALSE;
      fLoadSubject = TRUE;
      fLoadLongnames = TRUE;
      fDontMoveMouse = FALSE;
      fUnHilite = TRUE;
      fUserListSwitches = FALSE;
      fDCOpens = FALSE;
      fLinkSetsIcon = FALSE;
      fConfirmDelete = TRUE;
      fSyncUpdates = FALSE;
      fRealIdle = FALSE;
      fNoIconsFiles = FALSE;
      fNoIconsDirs = FALSE;
      fFolderAfterExtract = FALSE;
      fVerify = TRUE;
      fNoSearch = TRUE;
      DosSetVerify(TRUE);
      fForceUpper = FALSE;
      fForceLower = TRUE;
      fArcStuffVisible = TRUE;
      fVTreeOpensWPS = FALSE;
      fRemoteBug = TRUE;
      fDragndropDlg = TRUE;
      fMinOnOpen = FALSE;
      fQuickArcFind = TRUE;
      fNoRemovableScan = FALSE;
      FilesToGet = FILESTOGET_MAX;
      fFreeTree = FALSE;
      fSplitStatus = TRUE;
      fAutoTile = TRUE;
      fSaveState = TRUE;
      fStartMinimized = FALSE;
      fStartMaximized = FALSE;
      fDataMin = TRUE;
      ulCnrType = CCS_EXTENDSEL;
      fNoTreeGap = FALSE;
      {
        ULONG flWindowAttr;

        flWindowAttr = (CV_NAME | CV_MINI | CV_FLOW | CA_DETAILSVIEWTITLES);
        PrfWriteProfileData(fmprof, appname, "DirflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof, appname, "CollectorflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
      }
      dsDirCnrDefault.detailslongname = FALSE;
      dsDirCnrDefault.detailssubject = FALSE;
      dsDirCnrDefault.detailsea = TRUE;
      dsDirCnrDefault.detailssize = TRUE;
      dsDirCnrDefault.detailsicon = TRUE;
      dsDirCnrDefault.detailslwdate = TRUE;
      dsDirCnrDefault.detailslwtime = TRUE;
      dsDirCnrDefault.detailsladate = FALSE;
      dsDirCnrDefault.detailslatime = FALSE;
      dsDirCnrDefault.detailscrdate = FALSE;
      dsDirCnrDefault.detailscrtime = FALSE;
      dsDirCnrDefault.detailsattr = TRUE;
      sortFlags = SORT_FILENAME | SORT_DIRSFIRST;
      CollectorsortFlags = SORT_FILENAME | SORT_DIRSFIRST;
      if (hwndMain) {

        SWP swp;

        if (WinQueryWindowPos(hwndMain, &swp)) {
          WinSetWindowPos(hwndTree, HWND_TOP, 0, 0,
                          swp.cx / 5,
                          swp.cy -
                          (WinQuerySysValue(HWND_DESKTOP, SV_CYICON) * 2),
                          SWP_MOVE | SWP_SIZE);
        }
      }
      if (hwndTree) {

        CNRINFO cnri;
        ULONG flWindowAttr = (CV_TREE | CV_TEXT |
                              CV_FLOW | CA_TREELINE | CV_MINI);

        memset(&cnri, 0, sizeof(cnri));
        cnri.cb = sizeof(cnri);
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(cnri)));
        cnri.flWindowAttr = flWindowAttr;
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_SETCNRINFO, MPFROMP(&cnri),
                   MPFROMLONG(CMA_FLWINDOWATTR));
      }
      break;

    case CFG9_WINDOZETHINK:
      fTileBackwards = FALSE;
      ulCnrType = CCS_MULTIPLESEL;
      fFollowTree = TRUE;
      fTopDir = FALSE;
      fSwitchTree = TRUE;
      fSwitchTreeOnFocus = FALSE;
      fSwitchTreeExpand = TRUE;
      fCollapseFirst = TRUE;
      fDCOpens = FALSE;
      {
        ULONG flWindowAttr;

        flWindowAttr = CV_NAME | CV_FLOW | CA_DETAILSVIEWTITLES;
        PrfWriteProfileData(fmprof, appname, "DirflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof, appname, "CollectorflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
      }
      fLinkSetsIcon = FALSE;
      fFreeTree = FALSE;
      fNoTreeGap = TRUE;
      fExternalArcboxes = TRUE;
      fExternalViewer = TRUE;
      fExternalCollector = TRUE;
      fExternalINIs = TRUE;
      fUserListSwitches = TRUE;
      WinSendMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), UM_UNDO, MPVOID,
                 MPVOID);
      if (hwndTree) {

        CNRINFO cnri;
        ULONG flWindowAttr = CV_TREE | CV_MINI | CV_ICON |
          CV_FLOW | CA_TREELINE;

        memset(&cnri, 0, sizeof(cnri));
        cnri.cb = sizeof(cnri);
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(cnri)));
        cnri.flWindowAttr = flWindowAttr;
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_SETCNRINFO, MPFROMP(&cnri),
                   MPFROMLONG(CMA_FLWINDOWATTR));
      }
      if (hwndMain) {
        if (fAutoView)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_AUTOVIEW, 0), MPVOID);
        if (!fDrivebar)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_DRIVEBAR, 0), MPVOID);
        {
          SWP swp;

          if (WinQueryWindowPos(hwndMain, &swp)) {
            WinSetWindowPos(hwndTree, HWND_TOP, 0, 0,
                            swp.cx / 5, swp.cy, SWP_MOVE | SWP_SIZE);
          }
        }
        PostMsg(MainObjectHwnd, UM_RESTORE, MPVOID, MPFROMLONG(2L));
        PostMsg(MainObjectHwnd, UM_SETDIR, MPFROMLONG(1L), MPVOID);
      }
      return 0;

    case CFG9_DOSTHINK:
      fTileBackwards = TRUE;
      ulCnrType = CCS_MULTIPLESEL;
      fSwitchTree = TRUE;
      fSwitchTreeOnFocus = FALSE;
      fSwitchTreeExpand = TRUE;
      fCollapseFirst = TRUE;
      fFollowTree = TRUE;
      fTopDir = FALSE;
      if (hwndMain) {
        if (!fTextTools)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_TEXTTOOLS, 0), MPVOID);
      }
      {
        ULONG flWindowAttr;

        flWindowAttr = CV_TEXT | CV_FLOW | CA_DETAILSVIEWTITLES;
        PrfWriteProfileData(fmprof, appname, "DirflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
        PrfWriteProfileData(fmprof, appname, "CollectorflWindowAttr",
                            &flWindowAttr, sizeof(ULONG));
      }
      if (hwndTree) {

        CNRINFO cnri;
        ULONG flWindowAttr = CV_TREE | CV_TEXT | CV_FLOW | CA_TREELINE;

        memset(&cnri, 0, sizeof(cnri));
        cnri.cb = sizeof(cnri);
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(cnri)));
        cnri.flWindowAttr = flWindowAttr;
        WinSendMsg(WinWindowFromID
                   (WinWindowFromID(hwndTree, FID_CLIENT), TREE_CNR),
                   CM_SETCNRINFO, MPFROMP(&cnri),
                   MPFROMLONG(CMA_FLWINDOWATTR));
      }
      /* intentional fallthru */
    case CFG9_1X:
      if (SHORT1FROMMP(mp1) == CFG9_1X) {
        fTileBackwards = FALSE;
        ulCnrType = CCS_MULTIPLESEL | CCS_EXTENDSEL;
        fSwitchTree = FALSE;
        fSwitchTreeOnFocus = FALSE;
        fSwitchTreeExpand = FALSE;
        fCollapseFirst = FALSE;
        fFollowTree = FALSE;
        fNoSearch = TRUE;
      }
      fAutoTile = TRUE;
      fSaveState = TRUE;
      fDCOpens = FALSE;
      fLinkSetsIcon = FALSE;
      fFreeTree = FALSE;
      fNoTreeGap = TRUE;
      fExternalArcboxes = TRUE;
      fExternalViewer = TRUE;
      fExternalCollector = TRUE;
      fExternalINIs = TRUE;
      fUserListSwitches = TRUE;
      WinSendMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), UM_UNDO, MPVOID,
                 MPVOID);
      if (hwndTree) {

        CNRINFO cnri;
        ULONG flWindowAttr = CV_TREE | CV_ICON | CV_FLOW | CA_TREELINE;

        memset(&cnri, 0, sizeof(cnri));
        cnri.cb = sizeof(cnri);
        WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree, FID_CLIENT),
                                   TREE_CNR),
                   CM_QUERYCNRINFO, MPFROMP(&cnri), MPFROMLONG(sizeof(cnri)));
        cnri.flWindowAttr = flWindowAttr;
        WinSendMsg(WinWindowFromID(WinWindowFromID(hwndTree, FID_CLIENT),
                                   TREE_CNR),
                   CM_SETCNRINFO,
                   MPFROMP(&cnri), MPFROMLONG(CMA_FLWINDOWATTR));
      }
      if (hwndMain) {
        if (fAutoView)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_AUTOVIEW, 0), MPVOID);
        if (fUserComboBox)
          WinSendMsg(hwndMain, WM_COMMAND,
                     MPFROM2SHORT(IDM_USERLIST, 0), MPVOID);
        {
          SWP swp;

          if (WinQueryWindowPos(hwndMain, &swp)) {
            WinSetWindowPos(hwndTree, HWND_TOP, 0, 0,
                            swp.cx / 5, swp.cy, SWP_MOVE | SWP_SIZE);
          }
        }
        PostMsg(MainObjectHwnd, UM_RESTORE, MPVOID, MPFROMLONG(2L));
        PostMsg(MainObjectHwnd, UM_SETDIR, MPVOID, MPVOID);
      }
      return 0;

    case DID_CANCEL:
      WinSendMsg(hwnd, UM_UNDO, MPVOID, MPVOID);

    case DID_OK:
      PostMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER), msg, mp1, mp2);
      return 0;

    default:
      return 0;
    }
    // Save new details settings and refresh windows
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLongname",
                        &dsDirCnrDefault.detailslongname, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsSubject",
                        &dsDirCnrDefault.detailssubject, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsEA",
                        &dsDirCnrDefault.detailsea, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsSize",
                        &dsDirCnrDefault.detailssize, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsIcon",
                        &dsDirCnrDefault.detailsicon, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLWDate",
                        &dsDirCnrDefault.detailslwdate, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLWTime",
                        &dsDirCnrDefault.detailslwtime, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLADate",
                        &dsDirCnrDefault.detailsladate, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsLATime",
                        &dsDirCnrDefault.detailslatime, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsCRDate",
                        &dsDirCnrDefault.detailscrdate, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsCRTime",
                        &dsDirCnrDefault.detailscrtime, sizeof(BOOL));
    PrfWriteProfileData(fmprof, appname, "DirCnr.DetailsAttr",
                        &dsDirCnrDefault.detailsattr, sizeof(BOOL));
    if (hwndMain) {
      // Save state and restore to refresh windows with new settings
      if (SaveDirCnrState(hwndMain, GetPString(IDS_FM2TEMPTEXT)) > 0) {
        // Tell window procedure to close container windows
        PostMsg(MainObjectHwnd, UM_RESTORE, MPVOID, MPFROMLONG(2));
        // Restore saved state
        PostMsg(MainObjectHwnd, UM_RESTORE, GetPString(IDS_FM2TEMPTEXT), MPVOID);
      }
    }
    WinSendMsg((HWND) WinQueryWindowULong(hwnd, QWL_USER),
               UM_UNDO, MPVOID, MPVOID);
    return 0;

  case WM_CLOSE:
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

struct
{
  USHORT usMenuId;
  USHORT usFrameId;
  ULONG ulTitle;
  PFNWP pfnPageProc;
  HWND hwnd;
  ULONG ulHelpId;
  ULONG ulPageId;
} np[] = {
  { IDM_DIRCNRSETTINGS, CFGD_FRAME, IDS_NOTEDIRCNRS1TEXT, CfgDDlgProc, 0, 0, 0 },
  { IDM_DIRVIEWSETTINGS, CFG5_FRAME, IDS_NOTEDIRVIEW1TEXT, Cfg5DlgProc, 0, 0, 0},
  { IDM_DIRSORTSETTINGS, CFG6_FRAME, IDS_NOTEDIRSORT1TEXT, Cfg6DlgProc, 0, 0, 0 },
  { IDM_COLLECTORVIEWSETTINGS, CFG5_FRAME, IDS_NOTECOLVIEW1TEXT, Cfg7DlgProc, 0, 0, 0 },
  { IDM_COLLECTORSORTSETTINGS, CFG6_FRAME, IDS_NOTECOLSORT1TEXT, Cfg8DlgProc, 0, 0, 0 },
  { IDM_ARCHIVERSETTINGS, CFGA_FRAME, IDS_NOTEARCHIVER1TEXT, CfgADlgProc, 0, 0, 0 },
  { IDM_TREECNRVIEWSETTINGS, CFGT_FRAME, IDS_NOTETREE1TEXT, CfgTDlgProc, 0, 0, 0 },
  { IDM_TREECNRSORTSETTINGS, CFGTS_FRAME, IDS_NOTETREESORT1TEXT, CfgTSDlgProc, 0, 0, 0 },
  { IDM_VIEWERSETTINGS, CFGV_FRAME, IDS_NOTEVIEWERS1TEXT, CfgVDlgProc, 0, 0, 0 },
  { IDM_VIEWERSETTINGS2, CFGH_FRAME, IDS_NOTEVIEWERS3TEXT, CfgHDlgProc, 0, 0, 0 },
  { IDM_COMPARESETTINGS, CFGC_FRAME, IDS_NOTECOMPARE1TEXT, CfgCDlgProc, 0, 0, 0 },
  { IDM_MONOLITHICSETTINGS, CFGM_FRAME, IDS_NOTEMONOLITHIC1TEXT, CfgMDlgProc, 0, 0, 0 },
  { IDM_GENERALSETTINGS, CFGG_FRAME, IDS_NOTEGENERAL1TEXT, CfgGDlgProc, 0, 0, 0 },
  { IDM_SCANSETTINGS, CFGS_FRAME, IDS_NOTESCANNING1TEXT, CfgSDlgProc, 0, 0, 0 },
  { IDM_BUBBLESSETTINGS, CFGB_FRAME, IDS_NOTEBUBBLE1TEXT, CfgBDlgProc, 0, 0, 0 },
  { IDM_QUICKSETTINGS, CFG9_FRAME, IDS_NOTEQUICK1TEXT, Cfg9DlgProc, 0, 0, 0 },
  { 0, 0, 0, NULL, 0, 0, 0 }    // usFrameId 0 is end marker
};

static PSZ pszIK_LastSettingsPage = "LastSettingsPage";

MRESULT EXPENTRY CfgDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  HWND hwndTemp;
  USHORT attrib = BKA_FIRST;
  INT x;
  ULONG ulPageId;
  UINT uPageIndex;
  HAB hab;
  static HACCEL haccelCfg = NULLHANDLE;

  switch (msg) {
  case WM_INITDLG:
    hwndNotebook = hwnd;
    uPageIndex = 0;
    hab = WinQueryAnchorBlock(hwnd);
    if (haccelCfg == NULLHANDLE) {
      haccelCfg = WinLoadAccelTable(hab, FM3ModHandle, CFG_FRAME);
      if (haccelCfg == NULLHANDLE)
        Win_Error(hwndNotebook, HWND_DESKTOP, pszSrcFile, __LINE__, "WinLoadAccelTable");
    }
    if (haccelCfg != NULLHANDLE) {
      if (!WinSetAccelTable(hab, haccelCfg, hwndNotebook))
        Win_Error(hwndNotebook, HWND_DESKTOP, pszSrcFile, __LINE__, "WinSetAccelTable");
      // else
        // DbgMsg(pszSrcFile, __LINE__, "WinSetAccelTable MAIN_DRIVELIST %x %x", hwndFrame, haccelDriveList);
    }

    // If generic call, try to find conditional cascade default
    // 15 Feb 08 SHL fixme to be gone when/if IDM_NOTEBOOK gone?
    if (mp2 == MPFROMLONG(IDM_NOTEBOOK)) {
      DbgMsg(pszSrcFile, __LINE__, "Trying to translate IDM_NOTEBOOK");
      hwndTemp = WinQueryWindow(hwnd, QW_OWNER);
      if (hwndTemp != HWND_DESKTOP) {
        HWND hwndMenu;
        // Owner is frame if not applet
        hwndMenu = WinWindowFromID(hwndTemp, FID_MENU);
        if (hwndMenu == NULLHANDLE)
          Runtime_Error(pszSrcFile, __LINE__, "FID_MENU");
        else {
          MENUITEM mi;
          BOOL ok;
          memset(&mi, 0, sizeof(mi));
          ok = (BOOL)WinSendMsg(hwndMenu,
                                MM_QUERYITEM,
                                MPFROM2SHORT(IDM_NOTEBOOKSUBMENU, TRUE),
                                MPFROMP(&mi));
          if (!ok)
            Runtime_Error(pszSrcFile, __LINE__, "IDM_NOTEBOOKSUBMENU");
          else {
            mp2 = WinSendMsg(mi.hwndSubMenu, MM_QUERYDEFAULTITEMID, MPVOID, MPVOID);
            if (!mp2)
              Runtime_Error(pszSrcFile, __LINE__, "MM_QUERYDEFAULTITEMID");
          }
        }
      }
    }

    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    WinSendDlgItemMsg(hwnd,
                      CFG_NOTEBOOK,
                      BKM_SETDIMENSIONS,
                      MPFROM2SHORT(82, 24), MPFROMLONG(BKA_MAJORTAB));
    WinSendDlgItemMsg(hwnd,
                      CFG_NOTEBOOK,
                      BKM_SETDIMENSIONS,
                      MPFROM2SHORT(20, 20), MPFROMLONG(BKA_PAGEBUTTON));
    WinSendDlgItemMsg(hwnd,
                      CFG_NOTEBOOK,
                      BKM_SETDIMENSIONS,
                      MPFROM2SHORT(0, 0), MPFROMLONG(BKA_MINORTAB));

    // Build and insert notebook pages
    for (x = 0; np[x].usFrameId; x++) {
      hwndTemp = WinLoadDlg(HWND_DESKTOP,
                            HWND_DESKTOP,
                            np[x].pfnPageProc, FM3ModHandle, np[x].usFrameId, MPVOID);
      if (!hwndTemp) {
        Win_Error(hwnd, hwnd, pszSrcFile, __LINE__,
                  "Can not load settings page %u", x);
      }
      else {
        WinSetWindowULong(hwndTemp, QWL_USER, (ULONG)hwnd);
        np[x].hwnd = hwndTemp;
        np[x].ulPageId = (ULONG)WinSendDlgItemMsg(hwnd,
                                                CFG_NOTEBOOK,
                                                BKM_INSERTPAGE,
                                                MPFROMLONG(BKA_FIRST),
                                                MPFROM2SHORT(BKA_AUTOPAGESIZE |
                                                             BKA_STATUSTEXTON |
                                                             BKA_MAJOR, attrib));
        attrib = BKA_LAST;
        WinSendDlgItemMsg(hwnd,
                          CFG_NOTEBOOK,
                          BKM_SETPAGEWINDOWHWND,
                          MPFROMLONG(np[x].ulPageId), MPFROMLONG(np[x].hwnd));
        WinSendDlgItemMsg(hwnd,
                          CFG_NOTEBOOK,
                          BKM_SETTABTEXT,
                          MPFROMLONG(np[x].ulPageId),
                          MPFROMP(GetPString(np[x].ulTitle)));
        WinSendDlgItemMsg(hwnd,
                          CFG_NOTEBOOK,
                          BKM_SETSTATUSLINETEXT,
                          MPFROMLONG(np[x].ulPageId),
                          MPFROMP(GetPString(np[x].ulTitle + 1)));

        if (LONGFROMMP(mp2) == np[x].usMenuId)
          uPageIndex = x;               // Remember selected page
      }
    } // for pages

    // If quick settings page requested, assume request is for first time init
    // Turn to cfg page and show help
    // Page will not be available if running fm/2 lite or if load error
    // 15 Feb 08 SHL fixme to do just once?
    if (mp2 == MPFROMLONG(IDM_QUICKSETTINGS) &&
        x-- > 0 &&
        np[x].hwnd &&
        np[x].usFrameId == CFG9_FRAME)
    {
      // 10 Jan 08 SHL fixme to document what UM_SETDIR 1 means
      //PostMsg(MainObjectHwnd, UM_SETDIR, MPFROMLONG(1), MPVOID);
      PostMsg(WinWindowFromID(hwnd, CFG_NOTEBOOK),
              BKM_TURNTOPAGE, MPFROMLONG(np[x].ulPageId), MPVOID);
      PostMsg(hwnd, UM_FOCUSME, MPFROMLONG(np[x].hwnd), MPVOID);
      PostMsg(np[x].hwnd, WM_COMMAND, MPFROM2SHORT(IDM_HELP, 0), MPVOID);
    }
    else if (uPageIndex >= x) {
      Runtime_Error(pszSrcFile, __LINE__, "uPageIndex corrupted (%u)",
                    uPageIndex);
      uPageIndex = 0;
    }
    PostMsg(WinWindowFromID(hwnd, CFG_NOTEBOOK),
            BKM_TURNTOPAGE, MPFROMLONG(np[uPageIndex].ulPageId), MPVOID);
    // 15 Feb 08 SHL fixme to put focus on first field of page

  break;

  case UM_FOCUSME:
    if (mp1)
      WinSetActiveWindow(HWND_DESKTOP, (HWND) mp1);
    break;

  case UM_SETUP:
    WinSetActiveWindow(HWND_DESKTOP, WinQueryWindow(hwnd, QW_OWNER));
    WinSetActiveWindow(HWND_DESKTOP, hwnd);
    return 0;

  case WM_HELP:
    PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, mp2);
    break;

  case WM_CHAR:
    if (CHARMSG(&msg)->fs & KC_VIRTUALKEY) {
      switch (CHARMSG(&msg)->vkey) {
        case VK_F3:
        case VK_ENTER:
        case VK_NEWLINE:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_OK, MPVOID);
          return 0;
          break;
        case VK_ESC:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)DID_CANCEL, MPVOID);
          return 0;
          break;
        case VK_F1:
          PostMsg(hwnd, WM_COMMAND, (MPARAM)IDM_HELP, MPVOID);
          return 0;
          break;
      }
    }
    break;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case DID_OK:
      SaveLastPageIndex(hwnd);
      WinDismissDlg(hwnd, 1);
      break;

    case DID_CANCEL:
      // Tell current page to undo itself in case changed and still alive
      SaveLastPageIndex(hwnd);
      ulPageId = (ULONG) WinSendDlgItemMsg(hwnd,
                                         CFG_NOTEBOOK,
                                         BKM_QUERYPAGEID,
                                         MPFROMLONG(0),
                                         MPFROM2SHORT(BKA_TOP, 0));
      hwndTemp = (HWND) WinSendDlgItemMsg(hwnd,
                                          CFG_NOTEBOOK,
                                          BKM_QUERYPAGEWINDOWHWND,
                                          MPFROMLONG(ulPageId), MPVOID);
      if (hwndTemp)
        WinSendMsg(hwndTemp, UM_UNDO, MPVOID, MPVOID);

      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:                      /* relay message to appropriate page's window */
      ulPageId = (ULONG) WinSendDlgItemMsg(hwnd,
                                         CFG_NOTEBOOK,
                                         BKM_QUERYPAGEID,
                                         MPFROMLONG(0),
                                         MPFROM2SHORT(BKA_TOP, 0));
      hwndTemp = (HWND) WinSendDlgItemMsg(hwnd,
                                          CFG_NOTEBOOK,
                                          BKM_QUERYPAGEWINDOWHWND,
                                          MPFROMLONG(ulPageId), MPVOID);
      if (hwndTemp)
        PostMsg(hwndTemp, WM_COMMAND, MPFROM2SHORT(IDM_HELP, 0), MPVOID);
      break;
    }
    return 0;

  case UM_UNDO:
    for (x = 0; np[x].usFrameId; x++) {
      if (np[x].hwnd)
        WinSendMsg(np[x].hwnd, UM_UNDO, MPVOID, MPVOID);
    }
    break;

  case WM_DESTROY:
    if (np[0].usFrameId) {
      for (x = 1; np[x].usFrameId; x++) {
        if (np[x].hwnd) {
          WinSendMsg(np[x].hwnd, WM_CLOSE, MPVOID, MPVOID);
          np[x].hwnd = (HWND) 0;
          np[x].ulPageId = 0;
        }
      }
      WinSendMsg(np[0].hwnd, WM_CLOSE, MPVOID, MPVOID);
      np[0].hwnd = (HWND) 0;
      np[0].ulPageId = 0;
    }
    hwndNotebook = (HWND) 0;
    break;
  case WM_CLOSE:
    //SaveLastPageIndex(hwnd);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
/**
 * Initialize config submenu
 * @param hwndMenu is window handle
 * @param fIsLite is true if called for fm/2 lite
 */

VOID CfgMenuInit(HWND hwndMenu, BOOL fIsLite)
{
  UINT uPageIndex;
  ULONG ulSize = sizeof(uPageIndex);
  UINT x;
  PrfQueryProfileData(fmprof, appname, pszIK_LastSettingsPage, &uPageIndex, &ulSize);
  for (x = 0; x < uPageIndex && np[x].usFrameId; x++)
    ;                                   // Search
  if (np[x].usMenuId)
    SetConditionalCascade(hwndMenu, IDM_NOTEBOOKSUBMENU, np[uPageIndex].usMenuId);
  // If lite, delete excess items from menus and tables
  if (fIsLite) {
    WinSendMsg(hwndMenu,
               MM_DELETEITEM,
               MPFROM2SHORT(IDM_QUICKSETTINGS, FALSE), MPVOID);
    // If fm/2 lite drop quick setting page
    for (x = 0; np[x].usFrameId && np[x].usFrameId != CFG9_FRAME; x++)
      ; // Scan
    np[x].usFrameId = 0;
  }
}

static VOID SaveLastPageIndex(HWND hwnd)
{
  HWND hwndOwner;
  HWND hwndMenu;
  UINT x;
  ULONG ulPageId;

  // Owner is frame if not applet
  hwndOwner = WinQueryWindow(hwnd, QW_OWNER);
  if (hwndOwner != HWND_DESKTOP) {
    ulPageId = (ULONG)WinSendDlgItemMsg(hwnd,
                                       CFG_NOTEBOOK,
                                       BKM_QUERYPAGEID,
                                       MPFROMLONG(0),
                                       MPFROM2SHORT(BKA_TOP, 0));
    if (!ulPageId)
      Runtime_Error(pszSrcFile, __LINE__, "BKM_QUERYPAGEID");
    else {
      for (x = 0; np[x].usMenuId && np[x].ulPageId != ulPageId; x++)
        ; // Scan
      if (!np[x].usMenuId)
        Runtime_Error(pszSrcFile, __LINE__, "bad menu id %lu", ulPageId);
      else {
        PrfWriteProfileData(fmprof, FM3Str, pszIK_LastSettingsPage,
                            (PVOID)&x, sizeof(x));
        hwndMenu = WinWindowFromID(hwndOwner, FID_MENU);
        SetConditionalCascade(hwndMenu, IDM_NOTEBOOKSUBMENU, np[x].usMenuId);
      }
    }
  }
}

#pragma alloc_text(NOTEBOOK,CfgTDlgProc,CfgTSDlgProc,CfgMDlgProc)
#pragma alloc_text(NOTEBOOK2,CfgADlgProc,CfgSDlgProc,CfgVDlgProc,CfgHDlgProc)
#pragma alloc_text(NOTEBOOK3,CfgDDlgProc,Cfg5DlgProc,Cfg6DlgProc)
#pragma alloc_text(NOTEBOOK4,Cfg7DlgProc,Cfg8DlgProc,CfgCDlgProc)
#pragma alloc_text(NOTEBOOK5,CfgGDlgProc,CfgDlgProc,CfgBDlgProc)
#pragma alloc_text(NOTEBOOK5,CfgMenuInit,SaveLastPageIndex)
