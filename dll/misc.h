
/***********************************************************************

  $Id: $

  Misc utility functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  17 Jul 08 SHL Baseline

***********************************************************************/

#if !defined(MISC_H)
#define MISC_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

#ifdef FORTIFY
INT GetTidForThread(VOID);
INT GetTidForWindow(HWND hwnd);
#endif

SHORT AddToListboxBottom(HWND hwnd, CHAR * str);
BOOL AdjustCnrColRO(HWND hwndCnr, CHAR * title, BOOL readonly, BOOL toggle);
BOOL AdjustCnrColVis(HWND hwndCnr, CHAR * title, BOOL visible, BOOL toggle);
VOID AdjustCnrColsForFSType(HWND hwndCnr, CHAR * directory, DETAILS_SETTINGS * pds);
VOID AdjustCnrColsForPref(HWND hwndCnr, CHAR * directory, DETAILS_SETTINGS * pds,
			  BOOL compare);
VOID AdjustDetailsSwitches(HWND hwnd, HWND hwndMenu, USHORT cmd,
			   CHAR * directory, CHAR * keyroot, DETAILS_SETTINGS * pds,
			   BOOL compare);
void BoxWindow(HWND hwnd, HPS hps, LONG color);
MRESULT CnrDirectEdit(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
void EmphasizeButton(HWND hwnd, BOOL on);
INT ExecFile(HWND hwnd, CHAR * filename);
HWND FindDirCnr(HWND hwndParent);
VOID FixSwitchList(HWND hwnd, CHAR * text);
VOID HeapThread(VOID * dummy);
VOID LoadDetailsSwitches(CHAR * keyroot, DETAILS_SETTINGS * pds);
VOID LoadLibPath(CHAR * str, LONG len);
VOID OpenEdit(HWND hwnd);
void PaintSTextWindow(HWND hwnd, HPS hps);
VOID QuickPopup(HWND hwnd, DIRCNRDATA * dcd, HWND hwndMenu, USHORT id);
void SayFilter(HWND hwnd, MASK * mask, BOOL archive);
void SaySort(HWND hwnd, INT sortflags, BOOL archive);
void SayView(HWND hwnd, ULONG flWindowAttr);
BOOL SetCnrCols(HWND hwndCnr, BOOL compare);
VOID SetConditionalCascade(HWND hwndMenu, USHORT id, USHORT def);
VOID SetDetailsSwitches(HWND hwnd, DETAILS_SETTINGS * pds);
BOOL SetMenuCheck(HWND hwndMenu, USHORT id, BOOL * bool, BOOL toggle,
		  CHAR * savename);
VOID SetSortChecks(HWND hwndMenu, INT sortflags);
VOID SetSysMenu(HWND hwndSysMenu);
void SetViewMenu(HWND hwndMenu, ULONG flWindowAttr);
BOOL SwitchCommand(HWND hwndMenu, USHORT cmd);
BOOL ViewHelp(CHAR * filename);
VOID disable_menuitem(HWND hwndMenu, USHORT id, BOOL enable);

#endif // MISC_H
