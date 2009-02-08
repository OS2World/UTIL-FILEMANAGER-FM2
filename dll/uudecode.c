
/***********************************************************************

  $Id$

  uudecode

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2005, 2008 Steven H. Levine

  06 Jun 05 SHL Indent -i2
  06 Jun 05 SHL Drop unused code
  17 Jul 06 SHL Use Runtime_Error
  29 Jul 06 SHL Use xfgets
  01 Sep 06 SHL Back to fgets for now - avoid excess error messages
  22 Mar 07 GKY Use QWL_USER
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <share.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG                   // dircnrs.h

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "worker.h"			// typedef WORKER
#include "notebook.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "makelist.h"                   // AddToList
#include "errutil.h"                    // Dos_Error...
#include "strutil.h"                    // GetPString
#include "defview.h"
#include "uudecode.h"
#include "getnames.h"			// export_filename
#include "valid.h"			// IsFile
#include "misc.h"			// PaintRecessedWindow
#include "wrappers.h"			// xfgets

static PSZ pszSrcFile = __FILE__;

/* prototypes */
static BOOL decode(FILE * in, FILE * out);
static void outdec(char *p, FILE * f, int n);

/* single character decode */
#define DEC(c)  (((c) - ' ') & 077)

int UUD(char *filename, CHAR * dest)
{
  FILE *in, *out;
  int mode, ret = 0;
  char buf[80];
  char fakedest[CCHMAXPATH];

  if (!dest)
    dest = fakedest;
  in = _fsopen(filename, "r", SH_DENYWR);
  if (!in) {
    saymsg(MB_CANCEL,
           HWND_DESKTOP,
           GetPString(IDS_ERRORTEXT),
           GetPString(IDS_COMPCANTOPENTEXT), filename);
    return ret;
  }

  /* search for header line */
  for (;;) {
    if (!fgets(buf, sizeof(buf), in)) {
      fclose(in);
      saymsg(MB_CANCEL,
             HWND_DESKTOP,
             GetPString(IDS_ERRORTEXT),
             GetPString(IDS_UUDNOBEGINTEXT), filename);
      return ret;
    }
    if (!strncmp(buf, "begin ", 6))
      break;
  }                                     // for
  *dest = 0;
  sscanf(buf, "begin %o %259s", &mode, dest);
  dest[CCHMAXPATH - 1] = 0;
  {
    /* place dest in same directory as filename by default... */
    char build[CCHMAXPATH], *p;

    strcpy(build, filename);
    p = strrchr(build, '\\');
    if (p) {
      p++;
      *p = 0;
    }
    else
      strcat(build, "\\");
    strncat(build, dest, CCHMAXPATH - strlen(dest));
    strcpy(dest, build);
  }

  if (!export_filename(HWND_DESKTOP, dest, FALSE)) {
    fclose(in);
    return ret;
  }

  /* create output file */
  out = _fsopen(dest, "ab+", SH_DENYWR);
  if (!out) {
    fclose(in);
    saymsg(MB_CANCEL,
           HWND_DESKTOP,
           GetPString(IDS_ERRORTEXT),
           GetPString(IDS_UUDCANTOPENFORTEXT), dest, filename);
    return ret;
  }

  ret = 1;
  decode(in, out);

  xfgets(buf, sizeof(buf), in, pszSrcFile, __LINE__);

  fclose(in);
  fclose(out);
  return ret;
}

/*
 * copy from in to out, decoding as you go along.
 */
static BOOL decode(FILE * in, FILE * out)
{
  char buf[80];
  char *bp;
  int n;

  for (;;) {
    /* for each input line */
    if (!xfgets(buf, sizeof(buf), in, pszSrcFile, __LINE__))
      return FALSE;
    n = DEC(buf[0]);
    if (n <= 0)
      break;
    bp = &buf[1];
    while (n > 0) {
      outdec(bp, out, n);
      bp += 4;
      n -= 3;
    }
  }
  return TRUE;
}

/*
 * output a group of 3 bytes (4 input characters).
 * the input chars are pointed to by p, they are to
 * be output to file f.  n is used to tell us not to
 * output all of them at the end of the file.
 */
static void outdec(char *p, FILE * f, int n)
{
  INT c1, c2, c3;

  c1 = DEC(*p) << 2 | (UINT) DEC(p[1]) >> 4;
  c2 = DEC(p[1]) << 4 | (UINT) DEC(p[2]) >> 2;
  c3 = DEC(p[2]) << 6 | DEC(p[3]);
  if (n >= 1)
    putc(c1, f);
  if (n >= 2)
    putc(c2, f);
  if (n >= 3)
    putc(c3, f);
}

MRESULT EXPENTRY MergeDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  WORKER *wk;

  switch (msg) {
  case WM_INITDLG:
    if (mp2) {
      WinSetWindowPtr(hwnd, QWL_USER, mp2);
      wk = (WORKER *) mp2;
      if (wk->li && wk->li->list && wk->li->list[0]) {
        WinSendDlgItemMsg(hwnd, MRG_TARGETNAME, EM_SETTEXTLIMIT,
                          MPFROM2SHORT(CCHMAXPATH, 0), MPVOID);
        PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      }
      else
        WinDismissDlg(hwnd, 0);
    }
    else
      WinDismissDlg(hwnd, 0);
    break;

  case UM_UNDO:
    WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_DELETEALL, MPVOID, MPVOID);
    wk = WinQueryWindowPtr(hwnd, QWL_USER);
    if (wk) {
      INT x, numfiles = 0;
      SHORT start;
      CHAR *p;

      WinSetDlgItemText(hwnd, MRG_TARGETNAME, wk->li->targetpath);
      start = 0;
      p = strrchr(wk->li->targetpath, '\\');
      if (p)
        start = (p + 1) - wk->li->targetpath;
      WinSendDlgItemMsg(hwnd, MRG_TARGETNAME, EM_SETSEL,
                        MPFROM2SHORT(start, CCHMAXPATH), MPVOID);
      for (x = 0; wk->li->list[x]; x++) {
        if (IsFile(wk->li->list[x]) == 1) {
          numfiles++;
          WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_INSERTITEM,
                            MPFROM2SHORT(LIT_END, 0),
                            MPFROMP(wk->li->list[x]));
        }
      }
      WinCheckButton(hwnd, MRG_BINARY, (wk->li->type == IDM_MERGEBINARY));
      if (!numfiles) {
        saymsg(MB_CANCEL | MB_ICONEXCLAMATION,
               hwnd,
               GetPString(IDS_SILLYERRORTEXT),
               GetPString(IDS_MERGEWASTETEXT));
        WinDismissDlg(hwnd, 0);
      }
    }
    return 0;

  case WM_CONTROL:
    switch (SHORT1FROMMP(mp1)) {
    case MRG_LISTBOX:
      switch (SHORT2FROMMP(mp1)) {
      case LN_ENTER:
        {
          SHORT x;
          CHAR szBuffer[CCHMAXPATH];

          x = (SHORT) WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_QUERYSELECTION,
                                        MPFROMSHORT(LIT_FIRST), MPVOID);
          if (x >= 0) {
            *szBuffer = 0;
            WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_QUERYITEMTEXT,
                              MPFROM2SHORT(x, CCHMAXPATH), MPFROMP(szBuffer));
            if (*szBuffer)
              QuickEdit(hwnd, szBuffer);
          }
        }
        break;
      }
      break;
    }
    break;

  case WM_ADJUSTWINDOWPOS:
    PostMsg(hwnd, UM_SETDIR, MPVOID, MPVOID);
    break;

  case UM_SETDIR:
    PaintRecessedWindow(WinWindowFromID(hwnd, MRG_HELP), (HPS) 0, FALSE,
                        TRUE);
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_UNDO:
      PostMsg(hwnd, UM_UNDO, MPVOID, MPVOID);
      break;

    case MRG_CHANGETARGET:
      wk = WinQueryWindowPtr(hwnd, QWL_USER);
      if (wk) {
        CHAR filename[CCHMAXPATH];

        strcpy(filename, wk->li->targetpath);
        if (export_filename(HWND_DESKTOP, filename, FALSE) && *filename) {
          strcpy(wk->li->targetpath, filename);
          WinSetDlgItemText(hwnd, MRG_TARGETNAME, wk->li->targetpath);
        }
      }
      break;

    case MRG_REMOVE:
      {
        SHORT x;

        x = (SHORT) WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_QUERYSELECTION,
                                      MPFROMSHORT(LIT_FIRST), MPVOID);
        if (x >= 0)
          WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_DELETEITEM,
                            MPFROMSHORT(x), MPVOID);
      }
      break;

    case MRG_BOTTOM:
    case MRG_TOP:
      {
        SHORT x;
        CHAR szBuffer[CCHMAXPATH];

        x = (SHORT) WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_QUERYSELECTION,
                                      MPFROMSHORT(LIT_FIRST), MPVOID);
        if (x >= 0) {
          *szBuffer = 0;
          WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_QUERYITEMTEXT,
                            MPFROM2SHORT(x, CCHMAXPATH), MPFROMP(szBuffer));
          if (*szBuffer) {
            WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_DELETEITEM,
                              MPFROMSHORT(x), MPVOID);
            if (SHORT1FROMMP(mp1) == MRG_TOP)
              WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_INSERTITEM,
                                MPFROM2SHORT(0, 0), MPFROMP(szBuffer));
            else
              WinSendDlgItemMsg(hwnd, MRG_LISTBOX, LM_INSERTITEM,
                                MPFROM2SHORT(LIT_END, 0), MPFROMP(szBuffer));
          }
        }
      }
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case IDM_HELP:
      if (hwndHelp)
        WinSendMsg(hwndHelp, HM_DISPLAY_HELP,
                   MPFROM2SHORT(HELP_MERGE, 0), MPFROMSHORT(HM_RESOURCEID));
      break;

    case DID_OK:
      wk = WinQueryWindowPtr(hwnd, QWL_USER);
      if (wk) {
        BOOL append, binary;
        CHAR **list = NULL, **test, szBuffer[CCHMAXPATH];
        UINT numfiles = 0, numalloc = 0;
        INT error;
        SHORT x, y;

        *szBuffer = 0;
        WinQueryDlgItemText(hwnd, MRG_TARGETNAME, CCHMAXPATH, szBuffer);
        if (!*szBuffer) {
          if (!fAlertBeepOff)
            DosBeep(50, 100);
          WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, MRG_TARGETNAME));
          break;
        }
        if (DosQueryPathInfo(szBuffer,
                             FIL_QUERYFULLNAME,
                             wk->li->targetpath, CCHMAXPATH)) {
          if (!fAlertBeepOff)
            DosBeep(50, 100);
          WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwnd, MRG_TARGETNAME));
          break;
        }
        WinSetDlgItemText(hwnd, MRG_TARGETNAME, szBuffer);
        append = WinQueryButtonCheckstate(hwnd, MRG_APPEND);
        binary = WinQueryButtonCheckstate(hwnd, MRG_BINARY);
        wk->li->type = (append && binary) ? IDM_MERGEBINARYAPPEND :
          (append) ? IDM_MERGETEXTAPPEND :
          (binary) ? IDM_MERGEBINARY : IDM_MERGETEXT;
        x = (SHORT) WinSendDlgItemMsg(hwnd,
                                      MRG_LISTBOX,
                                      LM_QUERYITEMCOUNT, MPVOID, MPVOID);
        for (y = 0; y < x; y++) {
          *szBuffer = 0;
          WinSendDlgItemMsg(hwnd,
                            MRG_LISTBOX,
                            LM_QUERYITEMTEXT,
                            MPFROM2SHORT(y, CCHMAXPATH), MPFROMP(szBuffer));
          if (*szBuffer) {
            error = AddToList(szBuffer, &list, &numfiles, &numalloc);
            if (error) {
              Runtime_Error(pszSrcFile, __LINE__, "AddToList");
              break;
            }
          }
        }
        if (numfiles && list && numfiles + 1 < numalloc) {
          test =
            xrealloc(list, sizeof(CHAR *) * (numfiles + 1), pszSrcFile,
                     __LINE__);
          if (test)
            list = test;
        }
        if (!list || !list[0]) {
          Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
          break;
        }
        else {
          FreeList(wk->li->list);
          wk->li->list = list;
        }
      }
      WinDismissDlg(hwnd, 1);
      break;
    }                                   // switch WM_COMMAND mp1
    return 0;
  }                                     // switch msg
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(UUD,UUD,decode,outdec)
#pragma alloc_text(MERGE,MergeDlgProc)
