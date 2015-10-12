
/***********************************************************************

  $Id$

  System Interfaces

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2015 Steven H.Levine

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
  03 Jan 09 GKY Check for system that is protectonly to gray out Dos/Win command lines and prevent
		Dos/Win programs from being inserted into the execute dialog with message why.
  12 Jul 09 GKY Allow FM/2 to load in high memory
  21 Dec 09 GKY Added CheckExecutibleFlags to streamline code in command.c assoc.c & cmdline.c
  27 Dec 09 GKY Provide human readable error message when DosQueryAppType fails because it
                couldn't find the exe (such as missing archivers).
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  17 JAN 10 GKY Changes to environment handling in ExecuteOnList to facilitate move of commands to INI and allow
                the same commandline to have different environments (multiple different command titles).
  17 JAN 10 GKY Add ENVIRONMENT_SIZE vaiable to replace multiple (often different hard coded sizes) set to 2048
                (the largest value I found).
  20 Nov 10 GKY Check that pTmpDir IsValid and recreate if not found; Fixes hangs caused
                by temp file creation failures.
  26 Aug 11 GKY Add a low mem version of xDosAlloc* wrappers; move error checking into all the
                xDosAlloc* wrappers.
  16 Feb 14 GKY Add "#" command line switch to workaround problem with blank command shell
                started from fm2 after fm2 has been started with stdout and stderr
                redirected to a file.
  23 Feb 14 GKY Undated one of the error messages in runemf2 to provide the calling line
                and file.
  28 Apr 14 JBS Ticket #522: Ensure use of wrapper functions where needed
  12 Aug 15 JBS Ticket #522: Ensure no "highmem-unsafe" functions are called directly
                Calls to unsafe Dos... functions have been changed to call the wrapped xDos... functions

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
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
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
 * CheckExecutibleFlags checks the dialog controls and returns the appropriate
 * flags to be passed the runemf
 */

ULONG CheckExecutibleFlags(HWND hwnd, INT caller)
{
  /**
   * caller indicates the dialog calling the function:
   * 1 = Associations (ASS_)
   * 2 = CmdLine      (EXEC_)
   * 3 = Commands     (CMD_)
   **/

  ULONG flags = 0;

  if (caller != 2 &&
      WinQueryButtonCheckstate(hwnd, caller == 3 ? CMD_DEFAULT : ASS_DEFAULT))
    flags = 0;
  else if (WinQueryButtonCheckstate(hwnd, caller == 3 ? CMD_FULLSCREEN :
                                    caller == 1 ? ASS_FULLSCREEN : EXEC_FULLSCREEN))
    flags = FULLSCREEN;
  else if (WinQueryButtonCheckstate(hwnd, caller == 3 ? CMD_MINIMIZED :
                                    caller == 1 ? ASS_MINIMIZED : EXEC_MINIMIZED))
    flags = MINIMIZED;
  else if (WinQueryButtonCheckstate(hwnd, caller == 3 ? CMD_MAXIMIZED :
                                    caller == 1 ? ASS_MAXIMIZED : EXEC_MAXIMIZED))
    flags = MAXIMIZED;
  else if (WinQueryButtonCheckstate(hwnd, caller == 3 ? CMD_INVISIBLE :
                                    caller == 1 ? ASS_INVISIBLE : EXEC_INVISIBLE))
    flags = INVISIBLE;
  if (WinQueryButtonCheckstate(hwnd, caller == 3 ? CMD_KEEP : caller == 1 ? ASS_KEEP :
                               EXEC_KEEP))
    flags |= caller == 2 ? SEPARATEKEEP : KEEP;
  else if (caller == 2)
    flags |= SEPARATE;
  if (caller !=2 && WinQueryButtonCheckstate(hwnd, caller == 3 ? CMD_PROMPT : ASS_PROMPT))
    flags |= PROMPT;
  if (caller == 3 && WinQueryButtonCheckstate(hwnd, CMD_ONCE))
    flags |= ONCE;
  if (caller == 1 && WinQueryButtonCheckstate(hwnd, ASS_DIEAFTER))
    flags |= DIEAFTER;
  return flags;
}

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

int ExecOnList(HWND hwnd, PSZ command, int flags, PSZ tpath,  PSZ environment,
	       PSZ *list, PCSZ prompt, PCSZ pszCallingFile, UINT uiLineNumber)
{
  // executes the command once for all files in list

  CHAR path[CCHMAXPATH], *commandline, modpath[CCHMAXPATH], listfile[CCHMAXPATH],
       *p, *pp, drive, *file, *ext, *dot;
  register int x;
  BOOL spaces;

  if (!command || !*command) {
    Runtime_Error(pszSrcFile, __LINE__, NULL);
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
      case '!':                 // write list to file, add filename
	if (list) {
	  if (!*listfile) {
	    FILE *fp;
            CHAR *modew = "w";

            if (pTmpDir && !IsValidDir(pTmpDir))
              DosCreateDir(pTmpDir, 0);
	    strcpy(listfile, pTmpDir ? pTmpDir : pFM2SaveDirectory);
	    MakeTempName(listfile, "$FM2LI$T", 2);
	    fp = xfopen(listfile, modew,pszSrcFile,__LINE__, FALSE);
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

      case 'c':                 // add name of command processor
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

      case 't':                 // add Target directory
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

      case '$':                 // add drive letter
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

      case 'U':                 // add path of first list component
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

      case 'P':                 // add path of execution
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
    int ret;

    memset(&ex, 0, sizeof(EXECARGS));
    if (!environment) {
      ULONG size;

      size = ENVIRONMENT_SIZE;
      PrfQueryProfileData(fmprof, FM3Str, command, ex.environment, &size);
    }
    else
      strcpy(ex.environment, environment);
    if (flags & PROMPT) {
      // allow editing command line
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
    ret = runemf2(ex.flags, hwnd, pszCallingFile, uiLineNumber, path,
		   *ex.environment ? ex.environment : NULL,
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
   *
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
   * type bitmapped flag -- see systemf.h
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
  char *p, *pszPgm, *pszArgs = NULL;
  char szObject[32] = "";
  char szSavedir[CCHMAXPATH];
  BOOL useTermQ = FALSE;
  char szTempdir[CCHMAXPATH];
  BOOL fNoErrorMsg = FALSE;

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
  char szTermTemp[30];
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

  if (xDosAllocMemLow((PVOID)&pszPgm, MaxComLineStrg, pszSrcFile,__LINE__))
    return -1; //already complained
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

  if (!stricmp(pszCallingFile, "init.c"))
    fNoErrorMsg = TRUE;

  if (!*pszPgm) {
    p = GetCmdSpec(FALSE);
    strcpy(pszPgm, p);
    if (!*pszPgm) {
      Runtime_Error(pszSrcFile, __LINE__, NULL);
      return -1;
    }
  }

  if (*pszPgm) {
    if (*pszPgm == '<' && strchr(pszPgm, '>')) {
      // is a workplace object
      HOBJECT hWPSObject;
      char temp;

      p = strchr(pszPgm, '>');
      p++;
      temp = *p;
      if (temp) {
	if (xDosAllocMemLow((PVOID)&pszArgs, MaxComLineStrg * 2, pszSrcFile, __LINE__)) {
          DosFreeMem(pszPgm);
          return -1;   //already complained
        }
      }
      else
	pszArgs = NULL;
      *p = 0;
      // Find the handle of the WPS object
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
      p[strlen(p) + 1] = 0;             // double-terminate args
      if (*pszPgm) {
	if (!strchr(pszPgm, '\\') &&
	    !strchr(pszPgm, ':') &&
	    pszDirectory &&
	    *pszDirectory)
	{
	  strcpy(szSavedir, pFM2SaveDirectory);
	  switch_to(pszDirectory);
	}
	rc = xDosQueryAppType(pszPgm, &ulAppType);
	if (!strchr(pszPgm, '\\') &&
	    !strchr(pszPgm, ':') &&
	    pszDirectory &&
	    *pszDirectory)
	  switch_to(szSavedir);
        if (rc) {
          if (rc == ERROR_FILE_NOT_FOUND || rc == ERROR_PATH_NOT_FOUND ||
              rc == ERROR_INVALID_EXE_SIGNATURE || rc == ERROR_EXE_MARKED_INVALID)
            saymsg(MB_OK, HWND_DESKTOP, NullStr,GetPString(IDS_DOSQAPPTYPEFAILEDTEXT2),
                   pszPgm, pszCallingFile,uiLineNumber);
          else if (rc == ERROR_INVALID_DRIVE || rc == ERROR_DRIVE_LOCKED)
            saymsg(MB_OK, HWND_DESKTOP, NullStr,
                   GetPString(IDS_DOSQAPPTYPEFAILEDTEXT3), pszPgm);
          else
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
	if (ret && !fNoErrorMsg) {
	  Dos_Error(MB_ENTER,ret,hwnd,pszSrcFile,__LINE__,
		    GetPString(IDS_DOSEXECPGMFAILEDTEXT), pszPgm,
		    pszCallingFile, uiLineNumber);      // 26 May 08 SHL
	}
      }
    }
    else {
      if (~type & FULLSCREEN)
	type |= WINDOWED;
      if (xDosAllocMemLow((PVOID) &pszArgs, MaxComLineStrg * 2, pszSrcFile, __LINE__)) {
	DosFreeMem(pszPgm);
	return -1;   //already complained
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

        if (!stricmp(p, PCSZ_DOTBAT)) {
          if (!fProtectOnly) {
            strcpy(temp, pszPgm);
            strcpy(pszPgm, pszArgs);
            strcpy(pszArgs, "/C ");
            strcat(pszArgs, temp);
            strcat(pszArgs, " ");
            strcat(pszArgs, pszPgm);
            strcpy(pszPgm, GetCmdSpec(TRUE));             // DOS
          }
          else
            saymsg(MB_OK,
                   HWND_DESKTOP,
                   NullStr,
                   GetPString(IDS_NOTPROTECTONLYEXE),
                   pszPgm);
	}
	else if (!stricmp(p, PCSZ_DOTCMD) || !stricmp(p, PCSZ_DOTBTM)) {
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
      rc = xDosQueryAppType(pszPgm, &ulAppType);
      if (!strchr(pszPgm, '\\') &&
	  !strchr(pszPgm, ':') &&
	  pszDirectory &&
	  *pszDirectory)
        switch_to(szSavedir);
      if (rc) {
        if (rc == ERROR_FILE_NOT_FOUND || rc == ERROR_PATH_NOT_FOUND ||
            rc == ERROR_INVALID_EXE_SIGNATURE || rc == ERROR_EXE_MARKED_INVALID)
          saymsg(MB_OK, HWND_DESKTOP, NullStr, GetPString(IDS_DOSQAPPTYPEFAILEDTEXT2),
                 pszPgm, pszCallingFile,uiLineNumber);
        else if (rc == ERROR_INVALID_DRIVE || rc == ERROR_DRIVE_LOCKED)
          saymsg(MB_OK, HWND_DESKTOP, NullStr,
                 GetPString(IDS_DOSQAPPTYPEFAILEDTEXT3), pszPgm);
        else
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
			ulAppType, pszPgm, pszCallingFile, uiLineNumber);  // 26 May 08 SHL
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
	  else                          // ?
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

      xDosGetInfoBlocks(&ptib, &ppib);

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
		Dos_Error(MB_ENTER,rc,HWND_DESKTOP,pszSrcFile,__LINE__, PCSZ_DOSCREATEEVENTSEM);
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
	sdata.PgmInputs = (PBYTE)pszArgs;
      if (useTermQ) {
        strcpy(szTermTemp, szTermQName);
        sdata.TermQ = (PBYTE)szTermTemp;
      }
      sdata.Environment = (PBYTE)pszEnvironment;
      if (fUseShellEnv && (!strcmp(GetCmdSpec(TRUE), pszPgm) ||
          !strcmp(GetCmdSpec(FALSE), pszPgm)))
        sdata.InheritOpt = SSF_INHERTOPT_SHELL;
      else
        sdata.InheritOpt = SSF_INHERTOPT_PARENT;
      sdata.SessionType = (USHORT) ulAppType;
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
      ret = xDosStartSession(&sdata, &ulSessID, &sessPID);


      if (pszDirectory && *pszDirectory)
	switch_to(szSavedir);

      if (ret && ret != ERROR_SMG_START_IN_BACKGROUND) {
	if (!fNoErrorMsg)
          Dos_Error(MB_CANCEL,ret,hwnd,pszSrcFile,__LINE__,
		  GetPString(IDS_DOSSTARTSESSIONFAILEDTEXT),pszPgm,pszArgs,
                    pszCallingFile, uiLineNumber);        // 26 May 08 SHL
      }
      else if (type & WAIT) {
	if (!(type & (BACKGROUND | MINIMIZED | INVISIBLE)))
	  ShowSession(hwnd, sessPID);

	if (!useTermQ) {
	  STATUSDATA sd;

	  memset(&sd, 0, sizeof(sd));
	  sd.Length = (USHORT) sizeof(sd);
	  sd.SelectInd = SET_SESSION_UNCHANGED;
	  sd.BondInd = SET_SESSION_UNCHANGED;
	  for (ctr = 0;; ctr++)
	  {
            DosSleep(50);
	    if (DosSetSession(ulSessID, &sd))   // Check if session gone (i.e. finished)
	      break;
	    if (ctr > 10) {
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
		DosSleep(50);
		continue;
	      }
	    }
	    else {
              if (ctr == 20) {
		ShowSession(hwnd, sessPID);             // Show long running session
              }
	      rc = DosReadQueue(hTermQ, &rq, &ulLength, (PPVOID)&pTermInfo, 0,
				DCWW_WAIT, &bPriority, 0);
	    }

	    if (rc) {
	      // Oh heck
	      Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,"DosReadQueue");
	      DosSleep(100);
	      continue;
	    }

	    if (pTermInfo->usSessID == ulSessID)
	      break;                    // Our session is done

	    // Requeue session for other thread
	    {
	      static ULONG ulLastSessID;
	      if (ulLastSessID) {
		DosSleep(100);
		ulLastSessID = pTermInfo->usSessID;
	      }
	      // requeue term report for other thread and do not free yet
	      rc = DosWriteQueue(hTermQ, rq.ulData, ulLength,(PVOID)pTermInfo, bPriority);
	      if (rc)
		Dos_Error(MB_CANCEL,rc,hwnd,pszSrcFile,__LINE__,"DosWriteQueue");
	      DosSleep(50);          // Let other thread see queue entry
	    }
	  } // for

	  ret = pTermInfo->usRC == 0;           // Set 1 if rc 0 else 0
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

        if (p && (!stricmp(p, PCSZ_DOTBAT) || !stricmp(p, PCSZ_DOTCMD) ||
                  !stricmp(p, PCSZ_DOTBTM))) {
	  char *temp;

	  temp = xmalloc(CCHMAXPATH * 2,pszSrcFile,__LINE__);
	  if (temp) {
            if (!stricmp(p, PCSZ_DOTBAT)) {
              if (!fProtectOnly) {
                strcpy(temp, executable);
                strcpy(executable, parameters);
                strcpy(parameters, "/C ");
                strcat(parameters, temp);
                strcat(parameters, " ");
                strcat(parameters, executable);
                strcpy(executable, GetCmdSpec(TRUE)); //DOS
              }
              else
                saymsg(MB_OK,
                       HWND_DESKTOP,
                       NullStr,
                       GetPString(IDS_NOTPROTECTONLYEXE),
                       filename);
	    }
	    else if (!stricmp(p, PCSZ_DOTCMD) || !stricmp(p, PCSZ_DOTBTM)) {
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
