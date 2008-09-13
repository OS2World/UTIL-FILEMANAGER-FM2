
/***********************************************************************

  $Id$

  Window List Dialog

  Copyright (c) 1993-97 M. Kimes
  Copyright (c) 2005, 2006 Steven H.Levine

  23 May 05 SHL Use QWL_USER
  22 Jul 06 SHL Check more run time errors
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN

#include "fm3dll.h"
#include "killproc.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "newview.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "mainwnd.h"			// GetNextWindowPos
#include "winlist.h"
#include "wrappers.h"			// xmalloc
#include "misc.h"			// PostMsg
#include "fortify.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;


MRESULT EXPENTRY WinListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SHORT sSelect;

  static HWND Me = (HWND) 0;

  switch (msg) {
  case WM_INITDLG:
    if (Me || !mp2) {
      if (Me)
        PostMsg(Me, UM_FOCUSME, MPVOID, MPVOID);
      WinDismissDlg(hwnd, 0);
    }
    else {

      HENUM henum;
      HWND hwndChild;
      USHORT id;
      CHAR wtext[CCHMAXPATH + 1];

      Me = hwnd;
      WinSetWindowULong(hwnd, QWL_USER, *(HWND *) mp2);
      henum = WinBeginEnumWindows(*(HWND *) mp2);
      while ((hwndChild = WinGetNextWindow(henum)) != NULLHANDLE) {
        id = WinQueryWindowUShort(hwndChild, QWS_ID);
        if (!id)
          continue;
        *wtext = ' ';
        WinQueryWindowText(hwndChild, CCHMAXPATH, wtext + 1);
        wtext[CCHMAXPATH] = 0;
        sSelect =
          (SHORT) WinSendDlgItemMsg(hwnd, WLIST_LISTBOX, LM_INSERTITEM,
                                    MPFROM2SHORT(LIT_SORTASCENDING, 0),
                                    MPFROMP(wtext));
        if (sSelect >= 0)
          WinSendDlgItemMsg(hwnd, WLIST_LISTBOX, LM_SETITEMHANDLE,
                            MPFROM2SHORT(sSelect, 0),
                            MPFROMLONG((ULONG) hwndChild));
      }
      WinEndEnumWindows(henum);

      {
        PSWBLOCK pswb;
        ULONG ulSize, ulcEntries;
        register INT i, y;

        /* Get the switch list information */
        ulcEntries = WinQuerySwitchList(0, NULL, 0);
        ulSize = sizeof(SWBLOCK) + sizeof(HSWITCH) + (ulcEntries + 4) *
          (LONG) sizeof(SWENTRY);
        /* Allocate memory for list */
        pswb = xmalloc((unsigned)ulSize, pszSrcFile, __LINE__);
        if (pswb) {
          /* Put the info in the list */
          ulcEntries = WinQuerySwitchList(0, pswb, ulSize - sizeof(SWENTRY));
          /* do the dirty deed */
          y = 0;
          for (i = 0; i < pswb->cswentry; i++) {
            if (pswb->aswentry[i].swctl.uchVisibility == SWL_VISIBLE &&
                pswb->aswentry[i].swctl.fbJump == SWL_JUMPABLE &&
                ((pswb->aswentry[i].swctl.idProcess == mypid &&
                  (strnicmp(pswb->aswentry[i].swctl.szSwtitle,
                            "FM/2", 4))) ||
                 !strnicmp(pswb->aswentry[i].swctl.szSwtitle, "AV/2", 4) ||
                 !stricmp(pswb->aswentry[i].swctl.szSwtitle, "File Manager/2")
                 || !stricmp(pswb->aswentry[i].swctl.szSwtitle, "Collector")
                 || !strnicmp(pswb->aswentry[i].swctl.szSwtitle, "VTree", 5)
                 || !strnicmp(pswb->aswentry[i].swctl.szSwtitle, "VDir", 4)
                 || (!strnicmp(pswb->aswentry[i].swctl.szSwtitle, FM2Str, 4)
                     && strnicmp(pswb->aswentry[i].swctl.szSwtitle, "FM/2",
                                 4)))) {
              *wtext = '*';
              wtext[1] = 0;
              strcat(wtext, pswb->aswentry[i].swctl.szSwtitle);
              wtext[CCHMAXPATH] = 0;
              sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
                                                  WLIST_LISTBOX,
                                                  LM_INSERTITEM,
                                                  MPFROM2SHORT
                                                  (LIT_SORTASCENDING, 0),
                                                  MPFROMP(wtext));
              if (sSelect >= 0)
                WinSendDlgItemMsg(hwnd,
                                  WLIST_LISTBOX,
                                  LM_SETITEMHANDLE,
                                  MPFROM2SHORT(sSelect, 0),
                                  MPFROMLONG(pswb->aswentry[i].swctl.hwnd));
            }
            y++;
          }
          free(pswb);
          DosPostEventSem(CompactSem);
        }
      }

      sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
                                          WLIST_LISTBOX,
                                          LM_QUERYITEMCOUNT, MPVOID, MPVOID);
      if (sSelect <= 0)
        WinDismissDlg(hwnd, 0);
    }
    PostMsg(hwnd, UM_FOCUSME, MPVOID, MPVOID);
    break;

  case UM_FOCUSME:
    PostMsg(hwnd, UM_SETUP, MPVOID, MPVOID);
    return 0;

  case UM_SETUP:
    PostMsg(hwnd, UM_SETUP2, MPVOID, MPVOID);
    return 0;

  case UM_SETUP2:
    PostMsg(hwnd, UM_SETUP3, MPVOID, MPVOID);
    return 0;

  case UM_SETUP3:
    WinSetActiveWindow(HWND_DESKTOP, hwnd);
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case WLIST_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
        PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(DID_OK, 0), MPVOID);
        break;
      }
      break;
    }
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case WLIST_MINIMIZE:
    case WLIST_CLOSE:
    case DID_OK:
      {
        HWND hwndActive = (HWND) WinQueryWindowULong(hwnd, QWL_USER);

        hwndActive = WinQueryActiveWindow(hwndActive);
        sSelect = (SHORT) WinSendDlgItemMsg(hwnd,
                                            WLIST_LISTBOX,
                                            LM_QUERYSELECTION,
                                            MPFROM2SHORT(LIT_FIRST, 0),
                                            MPVOID);
        while (sSelect >= 0) {

          HWND HwndC;

          HwndC = (HWND) WinSendDlgItemMsg(hwnd,
                                           WLIST_LISTBOX,
                                           LM_QUERYITEMHANDLE,
                                           MPFROM2SHORT(sSelect, 0), MPVOID);
          if (HwndC) {

            SWP swp;

            WinQueryWindowPos(HwndC, &swp);
            if (SHORT1FROMMP(mp1) == DID_OK) {
              if (!(swp.fl & SWP_MINIMIZE) && (swp.cx == 0 || swp.cy == 0)) {
                GetNextWindowPos((HWND) WinQueryWindowULong(hwnd, QWL_USER),
                                 &swp, NULL, NULL);
                WinSetWindowPos(HwndC, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy,
                                SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ZORDER |
                                SWP_ACTIVATE | SWP_FOCUSACTIVATE);
              }
              else
                WinSetWindowPos(HwndC, HWND_TOP, 0, 0, 0, 0,
                                SWP_RESTORE | SWP_SHOW | SWP_ZORDER |
                                SWP_ACTIVATE | SWP_FOCUSACTIVATE);
            }
            else if (SHORT1FROMMP(mp1) == WLIST_MINIMIZE) {
              WinSetWindowPos(HwndC, HWND_BOTTOM, 0, 0, 0, 0,
                              SWP_MINIMIZE | SWP_DEACTIVATE |
                              SWP_FOCUSDEACTIVATE | SWP_ZORDER);
              if (hwndActive == HwndC) {
                WinSetWindowPos(WinWindowFromID(WinQueryWindow(hwndActive,
                                                               QW_PARENT),
                                                TREE_FRAME), HWND_TOP, 0, 0,
                                0, 0,
                                SWP_SHOW | SWP_RESTORE | SWP_ACTIVATE |
                                SWP_FOCUSACTIVATE | SWP_ZORDER);
                hwndActive = (HWND) 0;
              }
            }
            else if (WinQueryWindowUShort(HwndC, QWS_ID) != TREE_FRAME)
              PostMsg(HwndC, WM_CLOSE, MPVOID, MPVOID);
          }
          sSelect = (SHORT) WinSendDlgItemMsg(hwnd, WLIST_LISTBOX,
                                              LM_QUERYSELECTION,
                                              MPFROM2SHORT(sSelect, 0),
                                              MPVOID);
        }
      }
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_WINLIST, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;
    }
    return 0;

  case WM_DESTROY:
    if (Me == hwnd)
      Me = (HWND) 0;
    else
      WinSetWindowPos(Me, HWND_TOP, 0, 0, 0, 0,
                      SWP_SHOW | SWP_RESTORE | SWP_ZORDER | SWP_ACTIVATE);
    break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

VOID WindowList(HWND hwnd)
{
  WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, WinListDlgProc, FM3ModHandle,
            WLIST_FRAME, MPFROMP(&hwnd));
}

#pragma alloc_text(WINLIST,WindowList,WinListDlgProc)
