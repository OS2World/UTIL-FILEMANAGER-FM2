
/***********************************************************************

  $Id$

  System Interfaces

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2008 Steven H.Levine

  21 Nov 03 SHL Comments
  31 Jul 04 SHL Indent -i2
  01 Aug 04 SHL Rework lstrip/rstrip usage
  17 Jul 06 SHL Use Runtime_Error
  26 Jul 06 SHL Use convert_nl_to_nul
  15 Aug 06 SHL More error popups
  01 Nov 06 SHL runemf2: temp fix for hung windows caused by termq errors
  03 Nov 06 SHL runemf2: rework termination queue logic to work for multiple threads
  07 Jan 07 GKY Move error strings etc. to string file
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  29 Feb 08 GKY Changes to enable user settable command line length
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  26 May 08 SHL Use uiLineNumber correctly
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory or pTmpDir and use MakeTempName

***********************************************************************/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_LONGLONG                   // dircnrs.h

#include "fm3dll.h"
#include "mkdir.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"                    // Dos_Error...
#include "strutil.h"                    // GetPString
#include "notebook.h"                   //targetdirectory
#include "pathutil.h"
#include "cmdline.h"                    // CmdLineDlgProc
#include "shadow.h"			// RunSeamless
#include "systemf.h"
#include "strips.h"			// convert_nl_to_nul, strip_lead_char
#include "dirs.h"			// switch_to
#include "valid.h"			// MakeFullName
#include "misc.h"			// GetCmdSpec
#include "copyf.h"			// MakeTempName
#include "wrappers.h"			// xfopen
#include "fortify.h"

static PSZ pszSrcFile = __FILE__;

//static HAPP Exec(HWND hwndNotify, BOOL child, char *startdir, char *env,
//          PROGTYPE * progt, ULONG fl, char *formatstring, ...);

/**
 * Bring session foreground
 * @return TRUE if OK, else FALSE
 */

BOOL ShowSession(HWND hwnd, PID pid)
{
  HSWITCH hswitch;
  SWCNTRL swctl;
  ULONG rc;

  hswitch = WinQuerySwitchHandle(pid ? (HWND)0 : hwnd, pid);
  if (hswitch) {
    rc = WinQuerySwitchEntry(hswitch, &swctl);
    if (!rc) {
      if (swctl.idProcess == pid && swctl.uchVisibility == SWL_VISIBLE)
        rc = WinSwitchToProgram(hswitch);
      if (!rc)
        return TRUE;
      // else saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"Failed: %lu/%lx",rc,rc);

    }
  }
  return FALSE;
}

/**
 * Invoke runemf2 for command and file/directory list
 * @return command return code or
 *         -1 if runtime error or
 *         -2 if user cancels command line edit dialog
 */

int ExecOnList(HWND hwnd, char *command, int flags, char *tpath,
               char **list, char *prompt, PCSZ pszCallingFile, UINT uiLineNumber)
{
  /* executes the command once for all files in list */

  char path[CCHMAXPATH], *commandline, modpath[CCHMAXPATH], listfile[CCHMAXPATH],
       *p, *pp, drive, *file, *ext, *dot;
  register int x;
  BOOL spaces;

  if (!command || !*command) {
    Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
    return -1;
  }
  commandline = xmalloc(MaxComLineStrg + 1, pszSrcFile, __LINE__);
  if (!commandline)
    return -1; //already complained
  *listfile = 0;
  bstrip(command);

  *path = 0;
  if (tpath && *tpath)
    strcpy(path, tpath);
  else if (*command != '<' || !strchr(command, '>')) {
    strcpy(path, command + (*command == '"'));
    if (*command == '\"')
      p = strchr(path, '\"');
    else
      p = strchr(path, ' ');
    if (p)
      *p = 0;
    p = strrchr(path, '\\');
    if (!p)
      p = strrchr(path, ':');
    if (p) {
      if (*p == ':') {
        p++;
        *p = '\\';
        p++;
      }
      *p = 0;
    }
    else
      *path = 0;
  }
  if (!*path) {
    if (list && list[0])
      strcpy(path, list[0]);
    p = strrchr(path, '\\');
    if (!p)
      p = strrchr(path, ':');
    if (p) {
      if (*p == ':') {
        p++;
        *p = '\\';
        p++;
      }
      *p = 0;
    }
    else
      *path = 0;
  }
  *modpath = 0;
  if (list && list[0])
    strcpy(modpath, list[0]);
  p = strrchr(modpath, '\\');
  if (!p)
    p = strrchr(modpath, ':');
  if (p) {
    if (*p == ':') {
      p++;
      *p = '\\';
      p++;
    }
    *p = 0;
  }
  else
    *modpath = 0;
  if (!*modpath)
    strcpy(modpath, path);
  if (*path)
    MakeFullName(path);
  if (*modpath)
    MakeFullName(modpath);
  if (IsFullName(path))
    drive = toupper(*path);
  else
    drive = 0;

  p = command;                          // substitue for special % sequences

  pp = commandline;
  *commandline = 0;
  while (*p) {
    if (*p == '%') {
      switch (*(p + 1)) {
      case '!':                 /* write list to file, add filename */
        if (list) {
          if (!*listfile) {
            FILE *fp;


            strcpy(listfile, pTmpDir ? pTmpDir : pFM2SaveDirectory);
            MakeTempName(listfile, "$FM2LI$T", 2);
            /*if (listfile[strlen(listfile) - 1] != '\\')
              strcat(listfile, "\\");
            sprintf(&listfile[strlen(listfile)], "%s.%03x",
                    LISTTEMPROOT, (clock() & 4095));*/
            fp = xfopen(listfile, "w",pszSrcFile,__LINE__);
            if (fp) {
              for (x = 0; list[x]; x++)
              {
                fputs(list[x], fp);
                if (list[x + 1])
                  fputc('\n', fp);
              }
              fclose(fp);
            }
          }
          strcpy(pp, listfile);
          pp += strlen(listfile);
        }
        p += 2;
        break;

      case 'c':                 /* add name of command processor */
        {
          char *env = GetCmdSpec(FALSE);

          if (needs_quoting(env) && !strchr(env, '\"')) {
            *pp = '\"';
            pp++;
            spaces = TRUE;
          }
          else
            spaces = FALSE;
          strcpy(pp, env);
          p += 2;
          pp += strlen(env);
          if (spaces) {
            *pp = '\"';
            pp++;
          }
        }
        break;

      case 't':                 /* add Target directory */
        if (needs_quoting(targetdir) && !strchr(targetdir, '\"')) {
          *pp = '\"';
          pp++;
          spaces = TRUE;
        }
        else
          spaces = FALSE;
        strcpy(pp, targetdir);
        p += 2;
        pp += strlen(targetdir);
        if (spaces) {
          *pp = '\"';
          pp++;
        }
        break;

      case '$':                 /* add drive letter */
        if (drive)
          *pp = drive;
        else {
          ULONG ulDriveNum = 3, ulDriveMap;

          DosQCurDisk(&ulDriveNum, &ulDriveMap);
          *pp = (char) (ulDriveNum + '@');
        }
        pp++;
        p += 2;
        break;

      case 'U':                 /* add path of first list component */
      case 'u':
        if (*modpath) {
          if (needs_quoting(modpath) && !strchr(modpath, '\"')) {
            spaces = TRUE;
            *pp = '\"';
            pp++;
          }
          else
            spaces = FALSE;
          if (*(p + 1) == 'u') {
            strcpy(pp, modpath);
            pp += strlen(modpath);
          }
          else {
            strcpy(pp, modpath + 2);
            pp += strlen(modpath + 2);
          }
          if (spaces) {
            if (modpath[strlen(modpath) - 1] == '\\') {
              *pp = '\\';
              pp++;
            }
            *pp = '\"';
            pp++;
          }
        }
        else {
          char temp[CCHMAXPATH];

          strcpy(temp, pFM2SaveDirectory);
          if (needs_quoting(temp) && !strchr(temp, '\"')) {
            spaces = TRUE;
            *pp = '\"';
            pp++;
          }
          else
            spaces = FALSE;
          strcpy(pp, temp);
          pp += strlen(temp);
          if (spaces) {
            if (temp[strlen(temp) - 1] == '\\') {
              *pp = '\\';
              pp++;
            }
            *pp = '\"';
            pp++;
          }
        }
        p += 2;
        break;

      case 'P':                 /* add path of execution */
      case 'p':
        if (*path) {
          if (needs_quoting(path) && !strchr(path, '\"')) {
            spaces = TRUE;
            *pp = '\"';
            pp++;
          }
          else
            spaces = FALSE;
          if (*(p + 1) == 'p') {
            strcpy(pp, path);
            pp += strlen(path);
          }
          else {
            strcpy(pp, path + 2);
            pp += strlen(path + 2);
          }
          if (spaces) {
            if (path[strlen(path) - 1] == '\\') {
              *pp = '\\';
              pp++;
            }
            *pp = '\"';
            pp++;
          }
        }
        else {
          char temp[CCHMAXPATH];

          strcpy(temp, pFM2SaveDirectory);
          if (needs_quoting(temp) && !strchr(temp, '\"')) {
            spaces = TRUE;
            *pp = '\"';
            pp++;
          }
          else
            spaces = FALSE;
          strcpy(pp, temp);
          pp += strlen(temp);
          if (spaces) {
            if (temp[strlen(temp) - 1] == '\\') {
              *pp = '\\';
              pp++;
            }
            *pp = '\"';
            pp++;
          }
        }
        p += 2;
        break;

      case 'D':
        if (hwndMain) {
          PCNRITEM pci;

          pci = (PCNRITEM) WinSendMsg(WinWindowFromID(WinWindowFromID(
                                           hwndTree, FID_CLIENT), TREE_CNR),
                                      CM_QUERYRECORDEMPHASIS,
                                      MPFROMLONG(CMA_FIRST),
                                      MPFROMSHORT(CRA_CURSORED));
          if (pci && (int) pci != -1 && *pci->pszFileName) {
            if (needs_quoting(pci->pszFileName) &&
                !strchr(pci->pszFileName, '\"'))
            {
              *pp = '\"';
              pp++;
              spaces = TRUE;
            }
            else
              spaces = FALSE;
            strcpy(pp, pci->pszFileName);
            pp += strlen(pci->pszFileName);
            if (spaces) {
              *pp = '\"';
              pp++;
            }
          }
        }
        p += 2;
        break;

      case 'd':
        if (hwndMain) {
          HENUM henum;
          char retstr[CCHMAXPATH];
          HWND hwndC, hwndDir;
          USHORT id;
          BOOL first = TRUE;

          henum = WinBeginEnumWindows(hwndMain);
          while ((hwndC = WinGetNextWindow(henum)) != NULLHANDLE) {
            if (hwndC != hwndTree) {
              id = WinQueryWindowUShort(hwndC, QWS_ID);
              if (id) {
                hwndDir = WinWindowFromID(hwndC, FID_CLIENT);
                if (hwndDir) {
                  hwndDir = WinWindowFromID(hwndDir, DIR_CNR);
                  if (hwndDir) {
                    *retstr = 0;
                    WinSendMsg(hwndC, UM_CONTAINERDIR, MPFROMP(retstr), MPVOID);
                    if (*retstr) {
                      if (!first) {
                        *pp = ' ';
                        pp++;
                      }
                      first = FALSE;
                      if (needs_quoting(retstr) && !strchr(retstr, '\"')) {
                        *pp = '\"';
                        pp++;
                        spaces = TRUE;
                      }
                      else
                        spaces = FALSE;
                      strcpy(pp, retstr);
                      pp += strlen(retstr);
                      if (spaces) {
                        *pp = '\"';
                        pp++;
                      }
                    }
                  }
                }
              }
            }
          }
          WinEndEnumWindows(henum);
        }
        p += 2;
        break;

      case '%':
        *pp = '%';
        pp++;
        p += 2;
        break;

      case 'R':
      case 'F':
      case 'A':
      case 'r':
      case 'f':
      case 'a':
      case 'e':
        if (list) {
          for (x = 0; list[x]; x++)
          {
            file = strrchr(list[x], '\\');
            if (!file)
              file = strrchr(list[x], ':');
            if (file)
              file++;
            else
              file = list[x];
            ext = strrchr(file, '.');
            dot = ext;
            if (ext)
              ext++;
            switch (*(p + 1)) {
            case 'R':
            case 'r':
              if (pp + strlen(list[x]) > commandline + MaxComLineStrg)
                goto BreakOut;
              if (*(p + 1) == 'r') {
                strcpy(pp, list[x]);
                pp += strlen(list[x]);
              }
              else {
                strcpy(pp, list[x] + 2);
                pp += strlen(list[x] + 2);
              }
              break;

            case 'F':
            case 'f':
              if (*(p + 1) == 'F' && dot)
                *dot = 0;
              if (pp + strlen(file) > commandline + MaxComLineStrg)
                goto BreakOut;
              if (needs_quoting(file)) {
                spaces = TRUE;
                *pp = '\"';
                pp++;
              }
              else
                spaces = FALSE;
              strcpy(pp, file);
              pp += strlen(file);
              if (*(p + 1) == 'F' && dot)
                *dot = '.';
              if (spaces) {
                if (*(pp - 1) != '\"') {
                  *pp = '\"';
                  pp++;
                }
              }
              break;

            case 'A':
            case 'a':
              if (pp + strlen(list[x]) > commandline + MaxComLineStrg)
                goto BreakOut;
              if (needs_quoting(list[x]) && !strchr(list[x], '\"')) {
                spaces = TRUE;
                *pp = '\"';
                pp++;
              }
              else
                spaces = FALSE;
              if (*(p + 1) == 'a') {
                strcpy(pp, list[x]);
                pp += strlen(list[x]);
              }
              else {
                strcpy(pp, list[x] + 2);
                pp += strlen(list[x] + 2);
              }
              if (spaces) {
                if (list[x][strlen(list[x]) - 1] == '\\') {
                  *pp = '\\';
                  pp++;
                }
                *pp = '\"';
                pp++;
              }
              break;

            case 'e':
              if (ext) {
                if (pp + strlen(ext) > commandline + MaxComLineStrg)
                  goto BreakOut;
                if (needs_quoting(ext)) {
                  spaces = TRUE;
                  *pp = '\"';
                  pp++;
                }
                else
                  spaces = FALSE;
                strcpy(pp, ext);
                pp += strlen(ext);
                if (spaces) {
                  if (*(pp - 1) != '\"') {
                    *pp = '\"';
                    pp++;
                  }
                }
              }
              break;
            }
            if (list[x + 1]) {
              *pp = ' ';
              pp++;
            }
          }
        }
        p += 2;
        break;

      default:
        *pp = *p;
        p++;
        pp++;
        break;
      }
    }
    else {
      *pp = *p;
      pp++;
      p++;
    }
    *pp = 0;
  }

BreakOut:

  {
    EXECARGS ex;
    ULONG size;
    int ret;

    memset(&ex, 0, sizeof(EXECARGS));
    size = sizeof(ex.environment) - 1;
    PrfQueryProfileData(fmprof, FM3Str, command, ex.environment, &size);
    if (flags & PROMPT) {
      /* allow editing command line */
      ex.flags = (flags & (~PROMPT));
      ex.commandline = commandline;
      strcpy(ex.path, path);
      if (prompt)
        strcpy(ex.title, prompt);
      ret = WinDlgBox(HWND_DESKTOP, hwnd, CmdLineDlgProc, FM3ModHandle,
                      EXEC_FRAME, &ex);
      if (ret != 1) {
        free(commandline);
        return (ret == 0) ? -1 : -2;
      }
    }
    else
      ex.flags = flags;
    ex.flags &= (~PROMPT);
    ret = runemf2(ex.flags, hwnd, pszCallingFile, uiLineNumber, path,
                   (*ex.environment) ? ex.environment : NULL,
                   "%s", commandline);
    free(commandline);
    return ret;
  }
}

/** Run requested app
 * @return application return code or -1 if problem starting app
 */

int runemf2(int type, HWND hwnd, PCSZ pszCallingFile, UINT uiLineNumber,
            char *pszDirectory, char *pszEnvironment,
            char *formatstring,...)
{
  /** example:

   * status = runemf2(SEPARATE | WINDOWED,
   *                  hwnd, pszCallingFile, __LINE__,
   *                  NullStr,
   *                  NULL,
   *                  "%s /C %s",
   *                  getenv("COMSPEC"),
   *                  batchfilename);
   *
   * use (HWND)0 for hwnd if window handle not handy.
   * pszCallingFile and __LINE__ are used to determine caller for easier error tracking
   */

  /**
   * type bitmapped flag -- see FM3DLL.H
   */

  va_list parguments;
  int ret = -1;
  RESULTCODES results;
  STARTDATA sdata;
  REQUESTDATA rq;
  ULONG ulSessID;
  ULONG ulLength;
  UINT ctr;
  ULONG ulAppType;
  PID sessPID;
  BOOL wasquote;
  char *pszPgm, *pszArgs = NULL;
  char szObject[32] = "", *p, szSavedir[CCHMAXPATH];
  BOOL useTermQ = FALSE;
  char szTempdir[CCHMAXPATH];

  typedef struct {
    USHORT usSessID;
    USHORT usRC;
  } TERMINFO;

  TERMINFO *pTermInfo;
  BYTE bPriority;
  APIRET rc;
  PIB *ppib;
  TIB *ptib;

  // Shared by all threads
# define TERMQ_BASE_NAME "\\QUEUES\\FM3WAIT"
  static char szTermQName[30];
  static HQUEUE hTermQ;
  static HEV hTermQSem;

  if (pszDirectory && *pszDirectory) {
    if (!DosQueryPathInfo(pszDirectory,
                          FIL_QUERYFULLNAME,
                          szTempdir,
                          sizeof(szTempdir)))
      pszDirectory = szTempdir;
  }

  if (!hwnd)
    hwnd = HWND_DESKTOP;

  rc = DosAllocMem((PVOID)&pszPgm,
                   MaxComLineStrg,
                   PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
  if (rc) {
    Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,GetPString(IDS_OUTOFMEMORY));
    return -1;
  }

  *szSavedir = 0;

  *pszPgm = 0;
  va_start(parguments,
           formatstring);
  vsprintf(pszPgm,
           formatstring,
           parguments);
  va_end(parguments);

  if (pszEnvironment) {
    p = &pszEnvironment[strlen(pszEnvironment)] + 1;
    *p = 0;
    p = pszEnvironment;
    while ((p = convert_nl_to_nul(p)) != NULL)
      ; // loop
  }

  if (!*pszPgm) {
    p = GetCmdSpec(FALSE);
    strcpy(pszPgm, p);
    if (!*pszPgm) {
      Runtime_Error2(pszSrcFile, __LINE__, IDS_NODATATEXT);
      return -1;
    }
  }

  if (*pszPgm) {
    if (*pszPgm == '<' && strchr(pszPgm, '>')) {
      /* is a workplace object */
      HOBJECT hWPSObject;
      char temp;

      p = strchr(pszPgm, '>');
      p++;
      temp = *p;
      if (temp) {
        rc = DosAllocMem((PVOID)&pszArgs,
                         MaxComLineStrg * 2,
                         PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
        if (rc)
          Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,GetPString(IDS_OUTOFMEMORY));
      }
      else
        pszArgs = NULL;
      *p = 0;
      /* Find the handle of the WPS object */
      hWPSObject = WinQueryObject(pszPgm);
      *p = temp;
      if (hWPSObject != NULLHANDLE) {
        if (pszArgs && *p) {
          sprintf(pszArgs,"OPEN=DEFAULT;PARAMETERS=\"%s\"",p);
          WinSetObjectData(hWPSObject,pszArgs);
        }
        else
          WinSetObjectData(hWPSObject,"OPEN=DEFAULT");
        ret = 0;
      }
      goto ObjectInterrupt;
    }

    if ((type & RUNTYPE_MASK) == SYNCHRONOUS ||
        (type & RUNTYPE_MASK) == ASYNCHRONOUS ||
        (type & RUNTYPE_MASK) == DETACHED)
    {
      strip_lead_char(" \t", pszPgm);
      p = pszPgm;
      wasquote = FALSE;
      while (*p &&
             (wasquote ||
              (*p != ' ' &&
               *p != '\t')))
      {
        if (*p == '\"') {
          if (!wasquote) {
            wasquote = TRUE;
            memmove(p,
                    p + 1,
                    strlen(p));
            while (*p == ' ' ||
                   *p == '\t')
              p++;
          }
          else {
            memmove(p,
                    p + 1,
                    strlen(p));
            break;
          }
        }
        else
          p++;
      }
      if (*p) {
        *p = 0;
        p++;
      }
      else
        p = pszPgm;
      p[strlen(p) + 1] = 0;             /* double-terminate args */
      if (*pszPgm) {
        if (!strchr(pszPgm, '\\') &&
            !strchr(pszPgm, ':') &&
            pszDirectory &&
            *pszDirectory)
        {
          strcpy(szSavedir, pFM2SaveDirectory);
          switch_to(pszDirectory);
        }
        rc = DosQueryAppType(pszPgm,&ulAppType);
        if (!strchr(pszPgm, '\\') &&
            !strchr(pszPgm, ':') &&
            pszDirectory &&
            *pszDirectory)
          switch_to(szSavedir);
        if (rc) {
          Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,
                    GetPString(IDS_DOSQAPPTYPEFAILEDTEXT),
                    pszPgm, pszCallingFile, uiLineNumber);      // 26 May 08 SHL
          DosFreeMem(pszPgm);
          if (pszArgs)
            DosFreeMem(pszArgs);
          return -1;
        }
        if (ulAppType) {
          if (ulAppType & FAPPTYP_DLL || ulAppType & FAPPTYP_VIRTDRV ||
              ulAppType & FAPPTYP_PHYSDRV || ulAppType & FAPPTYP_PROTDLL)
          {
            Runtime_Error(pszSrcFile, __LINE__,
                          GetPString(IDS_APPTYPEUNEXPECTEDTEXT),
                          ulAppType, pszPgm, pszCallingFile, uiLineNumber);     // 26 May 08 SHL
            if (pszPgm)
              DosFreeMem(pszPgm);
            if (pszArgs)
              DosFreeMem(pszArgs);
            return -1;
          }
          if (ulAppType & FAPPTYP_DOS || ulAppType & FAPPTYP_WINDOWSREAL ||
              ulAppType & FAPPTYP_WINDOWSPROT || ulAppType & FAPPTYP_WINDOWSPROT31)
          {
            Runtime_Error(pszSrcFile, __LINE__,
                          GetPString(IDS_APPTYPEUNEXPECTEDTEXT),
                          ulAppType, pszPgm, pszCallingFile, uiLineNumber);     // 26 May 08 SHL
            if (pszPgm)
              DosFreeMem(pszPgm);
            if (pszArgs)
              DosFreeMem(pszArgs);
            return -1;
          }
        }
        memset(&results, 0, sizeof(results));
        if (pszDirectory && *pszDirectory) {
          strcpy(szSavedir, pFM2SaveDirectory);
          switch_to(pszDirectory);
        }
        ret = DosExecPgm(szObject, sizeof(szObject),
                         ((type & RUNTYPE_MASK) == ASYNCHRONOUS ?  EXEC_ASYNC : 0) +
                         ((type & RUNTYPE_MASK) == DETACHED ? EXEC_BACKGROUND : 0),
                         pszPgm, pszEnvironment, &results, pszPgm);
        if (pszDirectory && *pszDirectory)
          switch_to(szSavedir);
        if (ret) {
          Dos_Error(MB_ENTER,ret,hwnd,pszSrcFile,__LINE__,
                    GetPString(IDS_DOSEXECPGMFAILEDTEXT), pszPgm,
                    pszCallingFile, uiLineNumber);      // 26 May 08 SHL
        }
      }
    }
    else {
      if (~type & FULLSCREEN)
        type |= WINDOWED;
      rc = DosAllocMem((PVOID) & pszArgs, MaxComLineStrg * 2,
                       PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
      if (rc) {
        Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,GetPString(IDS_OUTOFMEMORY));
        DosFreeMem(pszPgm);
        return -1;
      }
      *pszArgs = 0;
      memset(&sdata, 0, sizeof(sdata));
      strip_lead_char(" \t", pszPgm);
      p = pszPgm;
      wasquote = FALSE;
      while (*p && (wasquote || (*p != ' ' && *p != '\t'))) {
        if (*p == '\"') {
          if (!wasquote) {
            wasquote = TRUE;
            memmove(p, p + 1, strlen(p));
            while (*p == ' ' || *p == '\t')
              p++;
          }
          else {
            memmove(p, p + 1, strlen(p));
            break;
          }
        }
        else
          p++;
      } // while
      if (*p) {
        *p = 0;
        p++;
      }
      else
        p = NullStr;
      if (*p)
        strcpy(pszArgs, p);

      p = strrchr(pszPgm, '.');
      if (p) {
        char temp[CCHMAXPATH + 1];

        if (!stricmp(p, ".BAT")) {
          strcpy(temp, pszPgm);
          strcpy(pszPgm, pszArgs);
          strcpy(pszArgs, "/C ");
          strcat(pszArgs, temp);
          strcat(pszArgs, " ");
          strcat(pszArgs, pszPgm);
          strcpy(pszPgm, GetCmdSpec(TRUE));             // DOS
        }
        else if (!stricmp(p, ".CMD") || !stricmp(p, ".BTM")) {
          // Assume 4OS2 is BTM
          strcpy(temp, pszPgm);
          strcpy(pszPgm, pszArgs);
          strcpy(pszArgs, "/C ");
          strcat(pszArgs, temp);
          strcat(pszArgs, " ");
          strcat(pszArgs, pszPgm);
          strcpy(pszPgm, GetCmdSpec(FALSE));            // OS/2
        }
      }

      // goddamned OS/2 limit

      if (strlen(pszPgm) + strlen(pszArgs) > 1024)
        pszArgs[1024 - strlen(pszPgm)] = 0;

      if (!strchr(pszPgm, '\\') &&
          !strchr(pszPgm, ':') &&
          pszDirectory &&
          *pszDirectory)
      {
        strcpy(szSavedir, pFM2SaveDirectory);
        switch_to(pszDirectory);
      }
      rc = DosQueryAppType(pszPgm,&ulAppType);
      if (!strchr(pszPgm, '\\') &&
          !strchr(pszPgm, ':') &&
          pszDirectory &&
          *pszDirectory)
        switch_to(szSavedir);
      if (rc) {
        Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,
                  GetPString(IDS_DOSQAPPTYPEFAILEDTEXT),
                  pszPgm, pszCallingFile, uiLineNumber);        // 26 May 08 SHL
        DosFreeMem(pszPgm);
        if (pszArgs)
          DosFreeMem(pszArgs);
        return -1;
      }

      if (ulAppType) {
        if (ulAppType & (FAPPTYP_DLL | FAPPTYP_VIRTDRV | FAPPTYP_PHYSDRV | FAPPTYP_PROTDLL))
        {
          Runtime_Error(pszSrcFile, __LINE__,
                        GetPString(IDS_APPTYPEUNEXPECTEDTEXT),
                        pszPgm, pszCallingFile, uiLineNumber);  // 26 May 08 SHL
          DosFreeMem(pszPgm);
          if (pszArgs)
            DosFreeMem(pszArgs);
          return -1;
        }
        ulAppType &= ~FAPPTYP_BOUND;
        if (ulAppType & (FAPPTYP_DOS | FAPPTYP_WINDOWSREAL | FAPPTYP_WINDOWSPROT | FAPPTYP_WINDOWSPROT31))
        {
          if (ulAppType & (FAPPTYP_WINDOWSREAL | FAPPTYP_WINDOWSPROT | FAPPTYP_WINDOWSPROT31))
          {
            if (~type & FULLSCREEN &&
                ulAppType & (FAPPTYP_WINDOWSREAL | FAPPTYP_WINDOWSPROT | FAPPTYP_WINDOWSPROT31))
            {
              ret = RunSeamless(pszPgm, pszArgs, hwnd);
              if (pszPgm)
                DosFreeMem(pszPgm);
              if (pszArgs)
                DosFreeMem(pszArgs);
              return ret ? 0 : -1;
            }
            else {
              strcat(pszPgm, " ");
              strcat(pszPgm, pszArgs);
              *pszArgs = 0;
              if (ulAppType & (FAPPTYP_WINDOWSPROT | FAPPTYP_WINDOWSREAL | FAPPTYP_WINDOWSPROT31))
                strcat(pszArgs, "/3 ");
              strcat(pszArgs, pszPgm);
              strcpy(pszPgm, "WINOS2.COM");
            }
          }
          else {
            if (~type & FULLSCREEN) {
              type |= WINDOWED;
              ulAppType = SSF_TYPE_WINDOWEDVDM;
            }
            else {
              type &= ~WINDOWED;
              ulAppType = SSF_TYPE_VDM;
            }
          }
        }
        else if (ulAppType & FAPPTYP_32BIT) {
          ulAppType &= ~FAPPTYP_32BIT;
          if (ulAppType == FAPPTYP_WINDOWAPI)
            ulAppType = SSF_TYPE_PM;
          else if (ulAppType == FAPPTYP_WINDOWCOMPAT)
            ulAppType = SSF_TYPE_WINDOWABLEVIO;
          else if (ulAppType == FAPPTYP_NOTWINDOWCOMPAT) {
            ulAppType = SSF_TYPE_FULLSCREEN;
            type &= ~WINDOWED;
            type |= FULLSCREEN;
          }
          else                          /* ? */
            ulAppType = SSF_TYPE_WINDOWABLEVIO;
        }
        else if (ulAppType == FAPPTYP_WINDOWAPI)
          ulAppType = SSF_TYPE_PM;
        else if (ulAppType == FAPPTYP_WINDOWCOMPAT)
          ulAppType = SSF_TYPE_WINDOWABLEVIO;
        else if (ulAppType == FAPPTYP_NOTWINDOWCOMPAT) {
          type &= ~WINDOWED;
          ulAppType = SSF_TYPE_FULLSCREEN;
        }
        else
          ulAppType = SSF_TYPE_DEFAULT;
        if ((type & FULLSCREEN || ~type & WINDOWED) &&
            ulAppType == SSF_TYPE_WINDOWABLEVIO)
        {
          ulAppType = SSF_TYPE_FULLSCREEN;
        }
        // fixme parens?
        else if (type & FULLSCREEN ||
                 (type & WINDOWED && ulAppType == SSF_TYPE_WINDOWEDVDM))
        {
          ulAppType = SSF_TYPE_VDM;
        }
      }
      if (ulAppType == SSF_TYPE_WINDOWEDVDM && type & SEPARATEKEEP) {
        type &= ~SEPARATEKEEP;
        type |= SEPARATE;
      }

      DosGetInfoBlocks(&ptib, &ppib);

      if (~type & WAIT)
        useTermQ = FALSE;
      else {
        rc = 0;
        DosEnterCritSec();
        if (!hTermQ) {
          // Create term queue and event semaphore just once
          sprintf(szTermQName, TERMQ_BASE_NAME "_%x", ppib->pib_ulpid);
          rc = DosCreateQueue(&hTermQ, QUE_FIFO | QUE_CONVERT_ADDRESS, szTermQName);
          if (rc) {
            hTermQ = (HQUEUE)0;         // Try to survive
            DosExitCritSec();
            Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,"DosCreateQueue");
          }
          else {
            rc = DosCreateEventSem(NULL,(PHEV)&hTermQSem,0,FALSE);
            if (rc) {
                hTermQSem = (HEV)0;     // Try to survive
                DosCloseQueue(hTermQ);
                hTermQ = (HQUEUE)0;     // Try to survive
                DosExitCritSec();
                Dos_Error(MB_ENTER,rc,HWND_DESKTOP,pszSrcFile,__LINE__,"DoCreateEventSem");
            }
            // if (!rc) fprintf(stderr,"%s %d qcreated ptib %x hTermQ %x\n",__FILE__, __LINE__,ptib,hTermQ);
          }
        } // if 1st time
        useTermQ = hTermQ && hTermQSem;
        if (!rc)
          DosExitCritSec();
      } // if wait

      memset(&sdata,0,sizeof(sdata));
      sdata.Length = sizeof(sdata);
      sdata.Related = type & (WAIT | CHILD) ? SSF_RELATED_CHILD :
                                              SSF_RELATED_INDEPENDENT;
      sdata.FgBg = type & BACKGROUND ? SSF_FGBG_BACK : SSF_FGBG_FORE;
      sdata.TraceOpt = SSF_TRACEOPT_NONE;
      sdata.PgmName = pszPgm;
      if (*pszArgs)
        sdata.PgmInputs = pszArgs;
      if (useTermQ)
        sdata.TermQ = szTermQName;
      sdata.Environment = pszEnvironment;
      sdata.InheritOpt = SSF_INHERTOPT_PARENT;
      sdata.SessionType = ulAppType;
      sdata.ObjectBuffer = szObject;
      sdata.ObjectBuffLen = sizeof(szObject);
      if ((type & RUNTYPE_MASK) == SEPARATEKEEP)
        sdata.PgmControl |= SSF_CONTROL_NOAUTOCLOSE;
      if (type & MAXIMIZED)
        sdata.PgmControl |= SSF_CONTROL_MAXIMIZE;
      if (type & MINIMIZED)
        sdata.PgmControl |= SSF_CONTROL_MINIMIZE;
      if (type & INVISIBLE)
        sdata.PgmControl |= SSF_CONTROL_INVISIBLE;

      if (pszDirectory && *pszDirectory) {
        strcpy(szSavedir, pFM2SaveDirectory);
        switch_to(pszDirectory);
      }

      // printf("%s %d DosStartsession thread 0x%x data\n ",
      //       __FILE__, __LINE__,ptib->tib_ordinal); fflush(stdout);   // 10 Mar 07 SHL hang
      // printf(" %d %d %d %s %s %s %d %d\n %s %x %x\n",
      //       sdata.Length , sdata.Related, sdata.FgBg, sdata.PgmName,
      //     sdata.PgmInputs, sdata.TermQ, sdata.InheritOpt,
      //   sdata.SessionType, szTermQName,
      //   hTermQ, hTermQSem); fflush(stdout);
      ret = DosStartSession(&sdata, &ulSessID, &sessPID);

      // if (type & WAIT) {
      // printf("%s %d DosStartession thread 0x%x rc = %d sess = %u pid = 0x%x\n",
      //        __FILE__, __LINE__, ptib->tib_ordinal,ret, ulSessID, sessPID); fflush(stdout);  // 10 Mar 07 SHL hang
      // }
      // else {
      // printf("%s %d DosStartession thread 0x%x nowait rc = %d\n",
      //      __FILE__, __LINE__, ptib->tib_ordinal,ret); fflush(stdout);       // 10 Mar 07 SHL hang
      // }

      if (pszDirectory && *pszDirectory)
        switch_to(szSavedir);

      if (ret && ret != ERROR_SMG_START_IN_BACKGROUND) {
        Dos_Error(MB_CANCEL,ret,hwnd,pszSrcFile,__LINE__,
                  GetPString(IDS_DOSSTARTSESSIONFAILEDTEXT),pszPgm,pszArgs,
                  pszCallingFile, uiLineNumber);        // 26 May 08 SHL
      }
      else if (type & WAIT) {
        if (!(type & (BACKGROUND | MINIMIZED | INVISIBLE)))
          ShowSession(hwnd, sessPID);

        if (!useTermQ) {
          STATUSDATA sd;
          // Could not create queue - fallback - fixme to be gone?
          // printf("%s %d waiting wo/termq\n", __FILE__, __LINE__); fflush(stdout);    // 12 Mar 07 SHL hang

          memset(&sd, 0, sizeof(sd));
          sd.Length = (USHORT) sizeof(sd);
          sd.SelectInd = SET_SESSION_UNCHANGED;
          sd.BondInd = SET_SESSION_UNCHANGED;
          for (ctr = 0;; ctr++)
          {
            DosSleep(100);//05 Aug 07 GKY 200
            if (DosSetSession(ulSessID, &sd))   // Check if session gone (i.e. finished)
              break;
            if (ctr > 10) {
              //   printf("%s %d thread 0x%x showing slow sess %u pid 0x%x\n",
              //        __FILE__, __LINE__,ptib->tib_ordinal,ulSessID,sessPID); fflush(stdout); // 12 Mar 07 SHL
              ShowSession(hwnd, sessPID);       // Show every 2 seconds
              ctr = 0;
            }
          }
        }
        else {
          for (ctr = 0;; ctr++)
          {
            if (ctr < 20) {
              rc = DosReadQueue(hTermQ, &rq, &ulLength, (PPVOID)&pTermInfo, 0,
                                DCWW_NOWAIT, &bPriority, hTermQSem);
              if (rc == ERROR_QUE_EMPTY) {
                DosSleep(50);//05 Aug 07 GKY 100
                continue;
              }
            }
            else {
              if (ctr == 20) {
                // printf("%s %d thread 0x%x showing slow sess %u pid 0x%x\n",
                //       __FILE__, __LINE__,ptib->tib_ordinal,ulSessID,sessPID); fflush(stdout);
                ShowSession(hwnd, sessPID);             // Show long running session
              }
              rc = DosReadQueue(hTermQ, &rq, &ulLength, (PPVOID)&pTermInfo, 0,
                                DCWW_WAIT, &bPriority, 0);
            }

            if (rc) {
              // Oh heck
              Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,"DosReadQueue");
              DosSleep(100);//05 Aug 07 GKY 500
              continue;
            }

            //  printf("%s %d DosReadQueue thread 0x%x sess %u sessRC %u rq.pid 0x%x rq.data 0x%x\n",
            //       __FILE__, __LINE__,ptib->tib_ordinal,pTermInfo->usSessID,pTermInfo->usRC,rq.pid, rq.ulData); fflush(stdout);

            if (pTermInfo->usSessID == ulSessID)
              break;                    // Our session is done

            // Requeue session for other thread
            {
              static ULONG ulLastSessID;
              // printf("%s %d requeue thread 0x%x our sess %u term sess %u term rc %u\n",
              //       __FILE__, __LINE__,ptib->tib_ordinal,ulSessID,pTermInfo->usSessID,pTermInfo->usRC); fflush(stdout);
              // fixme to be gone when no longer needed for debug?
              if (ulLastSessID) {
                DosSleep(100);//05 Aug 07 GKY 500
                ulLastSessID = pTermInfo->usSessID;
              }
              // requeue term report for other thread and do not free yet
              rc = DosWriteQueue(hTermQ, rq.ulData, ulLength,(PVOID)pTermInfo, bPriority);
              if (rc)
                Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,"DosWriteQueue");
              DosSleep(50); //05 Aug 07 GKY 100         // Let other thread see queue entry
            }
          } // for

          ret = pTermInfo->usRC == 0;           // Set 1 if rc 0 else 0
          // printf("%s %d thread 0x%x term for sess %u\n",
          //      __FILE__, __LINE__,ptib->tib_ordinal,ulSessID);fflush(stdout);
          DosFreeMem(pTermInfo);
        }
      } // if wait
      else if (!(type & (BACKGROUND | MINIMIZED | INVISIBLE)))
        ShowSession(hwnd, sessPID);
    }
  }

ObjectInterrupt:

  if (pszPgm)
    DosFreeMem(pszPgm);
  if (pszArgs)
    DosFreeMem(pszArgs);

  return ret;
}

//== Exec() Start application with WinStartApp ==
#if 0  // JBS	11 Sep 08
HAPP Exec(HWND hwndNotify, BOOL child, char *startdir, char *env,
          PROGTYPE *progt, ULONG fl, char *formatstring,...)
{
  PROGDETAILS pgd;
  register char *p;
  char *parameters = NULL, *executable = NULL;
  HAPP happ = (HAPP)0;
  ULONG ulOptions = SAF_INSTALLEDCMDLINE;
  BOOL wasquote;
  va_list parguments;

  if (child)
    ulOptions |= SAF_STARTCHILDAPP;

  executable = xmallocz(MaxComLineStrg, pszSrcFile, __LINE__);
  if (executable) {
    va_start(parguments, formatstring);
    vsprintf(executable, formatstring, parguments);
    va_end(parguments);
    strip_lead_char(" \t", executable);
    if (*executable) {
      parameters = xmalloc(MaxComLineStrg, pszSrcFile, __LINE__);
      if (parameters) {
        p = executable;
        wasquote = FALSE;
        while (*p && (wasquote || (*p != ' ' && *p != '\t'))) {
          if (*p == '\"') {
            if (!wasquote) {
              wasquote = TRUE;
              memmove(p, p + 1, strlen(p));
              while (*p == ' ' || *p == '\t')
                p++;
            }
            else {
              memmove(p, p + 1, strlen(p));
              break;
            }
          }
          else
            p++;
        }
        if (*p) {
          *p = 0;
          p++;
        }
        else
          p = NullStr;
        if (*p)
          strcpy(parameters, p);

        if (p && (!stricmp(p, ".BAT") || !stricmp(p, ".CMD"))) {
          char *temp;

          temp = xmalloc(CCHMAXPATH * 2,pszSrcFile,__LINE__);
          if (temp) {
            if (!stricmp(p, ".BAT")) {
              strcpy(temp, executable);
              strcpy(executable, parameters);
              strcpy(parameters, "/C ");
              strcat(parameters, temp);
              strcat(parameters, " ");
              strcat(parameters, executable);
              strcpy(executable, GetCmdSpec(TRUE));
            }
            else if (!stricmp(p, ".CMD")) {
              strcpy(temp, executable);
              strcpy(executable, parameters);
              strcpy(parameters, "/C ");
              strcat(parameters, temp);
              strcat(parameters, " ");
              strcat(parameters, executable);
              strcpy(executable, GetCmdSpec(FALSE));
            }
            free(temp);
          }
        }

        memset(&pgd, 0, sizeof(pgd));
        pgd.Length = sizeof(pgd);
        pgd.progt = *progt;
        pgd.swpInitial.fl = fl;
        pgd.pszEnvironment = env;
        pgd.pszStartupDir = startdir;
        pgd.pszParameters = *parameters ? parameters : NULL;
        pgd.pszExecutable = executable;
        pgd.swpInitial.hwndInsertBehind = HWND_TOP;
        happ = WinStartApp(hwndNotify, &pgd, NULL, NULL, ulOptions);
        free(parameters);
      }
    }
    free(executable);
  }
  return happ;
}
#endif
#pragma alloc_text(SYSTEMF,ShowSession,ExecOnList,runemf2)
