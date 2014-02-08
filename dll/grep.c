
/***********************************************************************

  $Id$

  grep tools

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2014 Steven H. Levine

  12 Feb 03 SHL InsertGrepFile: standardize EA math
  12 Feb 03 SHL doonefile: standardize EA math
  25 May 05 SHL Rework for ULONGLONG
  25 May 05 SHL Rework for FillInRecordFromFFB
  06 Jun 05 SHL Drop unused code
  24 Oct 05 SHL dononefile: do not free EA list twice
  22 Jul 06 SHL Use Runtime_Error
  26 Jul 06 SHL Check more run time errors
  19 Oct 06 SHL Correct . and .. detect
  03 Nov 06 SHL Count thread usage
  03 Aug 07 GKY Enlarged and made setable everywhere Findbuf (speed file loading)
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  13 Aug 07 SHL Avoid pointer errors; sanitize code
  13 Aug 07 SHL Move #pragma alloc_text to end for OpenWatcom compat
  15 Aug 07 SHL Use FilesToGet directly
  26 Aug 07 GKY Improved performance of FillDups
  26 Aug 07 GKY DosSleep(1) in loops changed to (0)
  21 Sep 07 GKY Fix trap on search that includes filenames that exceed maxpath
  07 Feb 08 SHL Use ITIMER_DESC to control sleeps and reporting
  29 Feb 08 GKY Use xfree where appropriate
  29 Nov 08 GKY Remove or replace with a mutex semaphore DosEnterCriSec where appropriate.
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  08 Mar 09 GKY Additional strings move to String Table
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10).
		Mostly cast CHAR CONSTANT * as CHAR *.
  29 May 10 GKY Suppress ERROR_FILENAME_EXCED_RANGE error because of problem with NTFS
  30 May 11 GKY Fixed potential trap caused by passing a nonexistant pci to FillInRecordFromFFB
		in DoInsertion because pci is limited to 65535 files. (nRecord is a USHORT)
		SHL's single loop fix.
  05 Aug 12 GKY Replace SleepIfNeeded with IdleIfNeeded to improve IU response during long searches; it
		will switch between normal and idle priority and back.
  05 Aug 12 GKY Always sort "Find Dups" by filename in the collector.
  08 Feb 14 SHL Support Ignore SVN option

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <limits.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "notebook.h"			// Data declaration(s)
#include "newview.h"			// Data declarations
#include "fm3str.h"
#include "grep.h"
#include "pathutil.h"                   // BldFullPathName
#include "filldir.h"                    // FillInRecordFromFFB
#include "makelist.h"                   // AddToList
#include "errutil.h"                    // Dos_Error...
#include "strutil.h"                    // GetPString
#include "tmrsvcs.h"                    // ITIMER_DESC
#include "common.h"			// DecrThreadUsage, IncrThreadUsage
#include "valid.h"			// MakeFullName
#include "literal.h"			// wildcard
#include "wrappers.h"			// xDosFindNext
#include "eas.h"                        // Free_FEAList
#include "stristr.h"			// findstring
#include "misc.h"			// PostMsg
#include "fortify.h"
#include "init.h"                       // Golbal semaphore
#include "sortcnr.h"			// SortCollectorCnr
#include "collect.h"

static VOID DoAllSubdirs(GREP *grep,
			 CHAR *searchPath,
			 BOOL recursing,
			 char **fileMasks,
			 UINT numFileMasks,
			 ITIMER_DESC *pitdSleep,
			 ITIMER_DESC *pitdReport);
static INT DoMatchingFiles(GREP *grep,
			   CHAR *path,
			   CHAR **fileMasks,
			   UINT numFileMasks,
			   ITIMER_DESC *pitdSleep,
			   ITIMER_DESC *pitdReport);
static BOOL DoOneFile(GREP *grep,
		      CHAR *fileName,
		      FILEFINDBUF4L *pffb,
		      ITIMER_DESC *pitdSleep,
		      ITIMER_DESC *pitdReport);
static BOOL DoInsertion(GREP *grep,
			ITIMER_DESC *pitdSleep,
			ITIMER_DESC *pitdReport);
static BOOL InsertDupe(GREP *grep, CHAR *dir, FILEFINDBUF4L *pffb);
static VOID FillDupes(GREP *grep,
		      ITIMER_DESC *pitdSleep,
		      ITIMER_DESC *pitdReport);

static VOID FreeDupes(GREP *grep);

#define GREPCHARS "*?[] \\"

#define isleap(year) ((((year%4)==0) && ((year%100)!=0)) || \
	((year%400)==0))

// Data definitions
#pragma data_seg(DATA2)
static PSZ pszSrcFile = __FILE__;
static INT monthdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#pragma data_seg(GLOBAL1)
HWND hwndStatus;


ULONG SecsSince1980(FDATE *date, FTIME *time)
{
  ULONG total = 0;
  UINT x;

  for (x = 1980; x < date->year + 1980; x++) {
    if (isleap(x))
      total += (366 * (24 * 60 * 60));
    else
      total += (365 * (24 * 60 * 60));
  }
  for (x = 1; x < date->month; x++) {
    if (x == 2 && isleap(date->year + 1980))
      total += (29 * (24 * 60 * 60));
    else
      total += ((long)monthdays[x - 1] * (24 * 60 * 60));
  }
  total += (((long)date->day - 1) * (24 * 60 * 60));
  total += ((long)time->hours * (60 * 60));
  total += ((long)time->minutes * 60);
  total += ((long)time->twosecs * 2);
  return total;
}

/**
 * this function originally from C_ECHO's Snippets -- modified
 * brute force methodology
 */

static BOOL m_match(CHAR *string, CHAR *pattern, BOOL absolute, BOOL ignore,
		    LONG len)
{
  // return TRUE if pattern found in string
  register CHAR *tn = pattern;
  register LONG len2 = 0;
  LONG lastlen = 0;
  CHAR lo, hi;

  if (len && string && pattern) {
    if (absolute)                       // no pattern matching
      return (findstring(pattern, strlen(pattern), string, len,
			 (ignore == FALSE)) != NULL);

    while (*tn && len2 < len) {
      switch (*tn) {
      case ' ':
	while (*tn == ' ')
	  tn++;
	while (len2 < len && isspace(string[len2]))
	  len2++;
	break;

      case '*':
	while (*tn == '*' || *tn == '?')
	  tn++;
	if (!*tn)
	  return TRUE;
	if (ignore) {
	  while (len2 < len && string[len2] != *tn)
	    len2++;
	}
	else {
	  while (len2 < len && toupper(string[len2] != *tn))
	    len2++;
	}
	break;

      case '[':
	tn++;
	if (!*tn)
	  return FALSE;
	lo = *tn;
	tn++;
	if (*tn != '-')
	  return FALSE;
	tn++;
	if (!*tn)
	  return FALSE;
	hi = *tn;
	tn++;
	if (*tn != ']')
	  return FALSE;
	tn++;
	if (ignore) {
	  if ((toupper(string[len2]) >= toupper(lo)) &&
	      (toupper(string[len2]) <= toupper(hi)))
	    len2++;
	  else {
	    tn = pattern;
	    len2 = lastlen = lastlen + 1;
	  }
	}
	else {
	  if ((string[len2] >= lo) && (string[len2] <= hi))
	    len2++;
	  else {
	    tn = pattern;
	    len2 = lastlen = lastlen + 1;
	  }
	}
	break;

      case '?':
	tn++;
	len2++;
	break;

      case '\\':
	tn++;
	if (!*tn)
	  return FALSE;
	// else intentional fallthru
      default:
	if (ignore) {
	  if (toupper(*tn) == toupper(string[len2])) {
	    tn++;
	    len2++;
	  }
	  else {
	    tn = pattern;
	    len2 = lastlen = lastlen + 1;
	  }
	}
	else {
	  if (*tn == string[len2]) {
	    tn++;
	    len2++;
	  }
	  else {
	    tn = pattern;
	    len2 = lastlen = lastlen + 1;
	  }
	}
	break;
      }
    }
    while (*tn == '*')
      tn++;

    if (!*tn)
      return TRUE;
  }
  return FALSE;
}

static BOOL match(CHAR *string, CHAR *patterns, BOOL absolute, BOOL ignore,
		  LONG len, ULONG numlines, CHAR *matched, BOOL matchall)
{
  BOOL ret = FALSE;
  register CHAR *p;
  register ULONG x = 0;

  p = patterns;
  while (!ret && *p) {
    ret = m_match(string, p, absolute, ignore, len);
    if (matchall && ret)
      break;
    if (matched && ret && x < numlines)
      matched[x] = 1;
    p += strlen(p);                     // check each pattern in 0-terminated list
    p++;
    x++;
  }
  return ret;
}

VOID GrepThread(VOID *arg)
{
  HAB ghab;
  HMQ ghmq;
  GREP grep;
  UINT x;
  UINT numFileMasks;
  static CHAR *fileMasks[512];                // 06 Feb 08 SHL FIXME to not be static
  CHAR *p, *pp, searchPath[CCHMAXPATH * 2];

  ITIMER_DESC itdSleep = { 0 };         // 06 Feb 08 SHL
  ITIMER_DESC itdReport = { 0 };

  if (!arg) {
    Runtime_Error(pszSrcFile, __LINE__, NULL);
    return;
  }

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif
  grep = *(GREP *)arg;
  *grep.stopflag = 0;                   // reset thread-killing flag
  DosError(FERR_DISABLEHARDERR);
  priority_normal();

  ghab = WinInitialize(0);
  if (ghab) {
    grep.ghab = ghab;
    ghmq = WinCreateMsgQueue(ghab, 0);
    if (ghmq) {
      WinCancelShutdown(ghmq, TRUE);
      IncrThreadUsage();
      // DosSleep(100); //05 Aug 07 GKY 128     // 07 Feb 08 SHL
      // hwndStatus does not exist for applet
      WinSetWindowText(hwndStatus ? hwndStatus : grep.hwndCurFile,
		       (CHAR *) GetPString(grep.finddupes ? IDS_GREPDUPETEXT :
						   IDS_GREPSCANTEXT));

      pp = grep.searchPattern;
      while (*pp) {
	if (!grep.absFlag) {
	  p = GREPCHARS;                // see if any sense in pattern matching
	  while (*p) {
	    if (strchr(pp, *p))
	      break;
	    p++;
	  }
	  if (!*p)                      // nope, turn it off
	    grep.absFlag = TRUE;
	}
	pp = pp + strlen(pp) + 1;
      }

      grep.attrFile &= (~FILE_DIRECTORY);
      grep.antiattr &= (~FILE_DIRECTORY);
      if (grep.antiattr & FILE_READONLY)
	grep.antiattr |= MUST_HAVE_READONLY;
      if (grep.antiattr & FILE_HIDDEN)
	grep.antiattr |= MUST_HAVE_HIDDEN;
      if (grep.antiattr & FILE_SYSTEM)
	grep.antiattr |= MUST_HAVE_SYSTEM;
      if (grep.antiattr & FILE_ARCHIVED)
	grep.antiattr |= MUST_HAVE_ARCHIVED;

      grep.anyexcludes = FALSE;
      numFileMasks = 0;
      fileMasks[numFileMasks++] = strtok(grep.fileMasks, ";");

      while ((fileMasks[numFileMasks] = strtok(NULL, ";")) != NULL && numFileMasks < 511) {
	if (*fileMasks[numFileMasks] == '/')
	  grep.anyexcludes = TRUE;
	numFileMasks++;
      }

      InitITimer(&itdSleep, 100);       // Sleep every 100 mSec (was 500)  GKY 8/11/13
      InitITimer(&itdReport, 2000);     // Report every 2 sec

      // loop through search masks
      for (x = 0; x < numFileMasks; x++) {

	 // Ignore exclude masks here
	if (*fileMasks[x] == '/')
	  goto ExcludeSkip;

	// Split directory pathname from mask
	p = (char *)(fileMasks[x] + (strlen(fileMasks[x]) - 1));
	while (*p != '\\' && *p != ':' && p != fileMasks[x])
	  --p;

	if (p == fileMasks[x]) {              // no path
	  strcpy(searchPath, grep.curdir);
	  strncpy(grep.fileMask, fileMasks[x], CCHMAXPATH);
	  grep.fileMask[CCHMAXPATH - 1] = 0;
	}
	else {                          // got to deal with a path
	  if (*p == ':') {              // just a drive, start in root dir
	    *p = 0;
	    p++;
	    strncpy(searchPath, fileMasks[x], CCHMAXPATH - 2);
	    searchPath[CCHMAXPATH - 3] = 0;
	    strcat(searchPath, ":\\");
	    strcpy(grep.fileMask, p);
	  }
	  if (*p == '\\') {
	    // got a 'full' path
	    CHAR temp;

	    p++;
	    temp = *p;		// Save for restore
	    *p = 0;		// Chop after backslash
	    strncpy(searchPath, fileMasks[x], CCHMAXPATH);
	    searchPath[CCHMAXPATH - 1] = 0;
	    *p = temp;
	    strcpy(grep.fileMask, p);
	  }
	  if (!*grep.fileMask)
	    strcpy(grep.fileMask, "*");
	}
	if (*grep.stopflag)
	  break;
	// do top level directory
	DoMatchingFiles(&grep, searchPath, fileMasks, numFileMasks, &itdSleep, &itdReport);
	// Recurse subdirectories if requested
	if (grep.dirFlag)
	  DoAllSubdirs(&grep, searchPath, FALSE, fileMasks, numFileMasks, &itdSleep, &itdReport);

      ExcludeSkip:
	if (*grep.stopflag)
	  break;
	if (WinIsWindow(grep.ghab, grep.hwndFiles))
	  DoInsertion(&grep, &itdSleep, &itdReport);    // insert any remaining objects
      } // for

      if (WinIsWindow(grep.ghab, grep.hwndFiles))
	DoInsertion(&grep, &itdSleep, &itdReport);      // insert any remaining objects

      if (WinIsWindow(grep.ghab, grep.hwndFiles) &&
	  grep.finddupes &&
	  !*grep.stopflag)
      {
	FillDupes(&grep, &itdSleep, &itdReport);
	CollectorsortFlags = 0;
	CollectorsortFlags |= SORT_FILENAME;
	WinSendMsg(grep.hwndFiles, CM_SORTRECORD, MPFROMP(SortCollectorCnr),
		   MPFROMLONG(CollectorsortFlags));
	SaySort(WinWindowFromID(WinQueryWindow(grep.hwndFiles, QW_PARENT),
				DIR_SORT), CollectorsortFlags, FALSE);
      }
      if (!PostMsg(grep.hwndFiles, UM_CONTAINER_FILLED, MPVOID, MPVOID))        // tell window we're done
	WinSendMsg(grep.hwndFiles, UM_CONTAINER_FILLED, MPVOID, MPVOID);
      WinDestroyMsgQueue(ghmq);
    }
    DecrThreadUsage();
    WinTerminate(ghab);
  }
  if (!ghmq || !ghab)
    WinPostMsg(grep.hwndFiles, UM_CONTAINER_FILLED, MPVOID, MPVOID);
  if (grep.dupehead)
    FreeDupes(&grep);
  if (grep.numlines && grep.matched) {
    free(grep.matched);
  }
  // 07 Feb 08 SHL FIXME to free grep here when not static
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
  DosPostEventSem(CompactSem);
}

static BOOL IsExcluded(CHAR *name, CHAR **fileMasks, UINT numFileMasks)
{
  UINT x;
  CHAR *n;

  n = strrchr(name, '\\');
  if (!n)
    n = strrchr(name, ':');
  if (n)
    n++;
  else
    n = name;
  for (x = 0; x < numFileMasks; x++) {
    if (*fileMasks[x] == '/' &&
	wildcard((strchr(fileMasks[x], '\\') ||
		  strchr(fileMasks[x], ':')) ? name : n, fileMasks[x] + 1, FALSE))
      return TRUE;
  }
  return FALSE;
}

/**
 * Recurse though subdirectories selecting files
 */

static VOID DoAllSubdirs(GREP *grep,
			 CHAR *searchPath,
			 BOOL recursing,
			 CHAR **fileMasks,
			 UINT numFileMasks,
			 ITIMER_DESC *pitdSleep,
			 ITIMER_DESC *pitdReport)
{
  FILEFINDBUF4 ffb;
  HDIR findHandle = HDIR_CREATE;
  ULONG ulFindCnt = 1;
  CHAR *p = NULL;

  // Append wildcard mask to directory pathname
  AddBackslashToPath(searchPath);
  strcat(searchPath, "*");
  DosError(FERR_DISABLEHARDERR);
  // Find first directory
  if (!DosFindFirst(searchPath,
		    &findHandle,
		    (MUST_HAVE_DIRECTORY | FILE_ARCHIVED | FILE_SYSTEM |
		     FILE_HIDDEN | FILE_READONLY),
		    &ffb,
		    sizeof(ffb),
		    &ulFindCnt,
		    FIL_QUERYEASIZE))
  {
    // Point p at appended wildcard pattern to speed up pathname build
    p = strrchr(searchPath, '\\');
    if (p)
      p++;
    else
      p = searchPath;
    do {                                // Process each directory that matches the mask
      int skip;
      if (*grep->stopflag)
	break;
      // 2014-02-08 SHL
      // Skip . and ..
      skip = ffb.achName[0] == '.' &&
	     (ffb.achName[1] == 0 ||
	      (ffb.achName[1] == '.' && ffb.achName[2] == 0));
      if (!skip && grep->ignoreSVN) {
	// Skip version control meta data directories
	skip = stricmp(ffb.achName, ".svn") == 0 ||
	       stricmp(ffb.achName, ".git") == 0 ||
	       stricmp(ffb.achName, ".hg") == 0 ||
	       stricmp(ffb.achName, "CVS") == 0;
      }
      if (!skip) {
	strcpy(p, ffb.achName);		// Build full directory pathname
	skip = grep->anyexcludes && IsExcluded(searchPath, fileMasks, numFileMasks);
      }
      if (!skip) {
	// Directory is selected
	// 07 Feb 08 SHL
	if (IsITimerExpired(pitdReport)) {
	  if (!hwndStatus)
	    WinSetWindowText(grep->hwndCurFile, searchPath);
	  else if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles) {
	    CHAR s[CCHMAXPATH + 64];
	    sprintf(s, "%s %s", GetPString(IDS_SCANNINGTEXT), searchPath);
	    WinSetWindowText(hwndStatus, s);
	  }
	}
	// Select files from directory
	DoMatchingFiles(grep, searchPath, fileMasks, numFileMasks, pitdSleep, pitdReport);
	// 07 Feb 08 SHL
	if (IsITimerExpired(pitdReport)) {
	  priority_normal();
	  if (!hwndStatus)
	    WinSetWindowText(grep->hwndCurFile, searchPath);
	  else {
	    if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles) {
	      CHAR s[CCHMAXPATH + 64];
	      sprintf(s, "%s %s", GetPString(IDS_SCANNINGTEXT), searchPath);
	      WinSetWindowText(hwndStatus, s);
	    }
	  };
	}
	// Recurse
	DoAllSubdirs(grep, searchPath, TRUE, fileMasks, numFileMasks, pitdSleep, pitdReport);
      } // if !skip
      ulFindCnt = 1;
      // Get next directory
    } while (!DosFindNext(findHandle,
			  &ffb,
			  sizeof(ffb),
			  &ulFindCnt));
    DosFindClose(findHandle);
  } // if at least one directory
  if (p)                                // Restore orginal directory pathname
    *p = 0;
}

/**
 * Select files matching wildcard masks in single directory
 */

static INT DoMatchingFiles(GREP *grep,
			   CHAR *path,
			   CHAR **fileMasks,
			   UINT numFileMasks,
			   ITIMER_DESC *pitdSleep,
			   ITIMER_DESC *pitdReport)
{
  PFILEFINDBUF4L pffbArray;
  PFILEFINDBUF4L pffbFile;
  ULONG x;
  HDIR findHandle = HDIR_CREATE;
  ULONG ulFindCnt;
  CHAR szFindPath[CCHMAXPATH];
  PSZ p;
  APIRET rc;
  ULONG ulBufBytes = FilesToGet * sizeof(FILEFINDBUF4L);
  static BOOL fDone;

  pffbArray = xmalloc(ulBufBytes, pszSrcFile, __LINE__);
  if (!pffbArray)
    return 0;

  BldFullPathName(szFindPath, path, grep->fileMask);

  MakeFullName(szFindPath);

  // Point p at wildcard mask to speed up pathname build
  p = strrchr(szFindPath, '\\');
  if (p)
    p++;
  else
    p = szFindPath;

  // Step through matching files
  DosError(FERR_DISABLEHARDERR);
  ulFindCnt = FilesToGet;
  rc = xDosFindFirst(szFindPath,
		     &findHandle,
		     FILE_NORMAL | grep->attrFile | grep->antiattr,
		     pffbArray,
		     ulBufBytes,
		     &ulFindCnt,
		     FIL_QUERYEASIZEL);
  if (!rc) {
    do {
      // Process each file that matches the mask
      pffbFile = pffbArray;
      for (x = 0; x < ulFindCnt; x++) {
	if (*grep->stopflag)
	  break;
	// 2014-02-08 SHL FIXME to know if really need to skip . and .. here
	// We should only be selecting files
	if (*pffbFile->achName != '.' ||
	    (pffbFile->achName[1] && pffbFile->achName[1] != '.')) {
	  strcpy(p, pffbFile->achName); // build full file pathname
	  if (strlen(szFindPath) > CCHMAXPATH){
	    // Complain if pathnames exceeds max
	    DosFindClose(findHandle);
	    if (!fDone) {
	      fDone = TRUE;
	      saymsg(MB_OK | MB_ICONASTERISK,
		     HWND_DESKTOP,
		     GetPString(IDS_WARNINGTEXT),
		     GetPString(IDS_LENGTHEXCEEDSMAXPATHTEXT));
	    }
	    return 1;
	  }

	  // 07 Feb 08 SHL
	  if (IsITimerExpired(pitdReport)) {
	    if (!hwndStatus)
	      WinSetWindowText(grep->hwndCurFile, szFindPath);
	    else {
	      if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles) {
		CHAR s[CCHMAXPATH + 64];
		sprintf(s, "%s %s", GetPString(IDS_SCANNINGTEXT), szFindPath);
		WinSetWindowText(hwndStatus, s);
	      }
	    }
	  }

	  if (!grep->anyexcludes || !IsExcluded(szFindPath, fileMasks, numFileMasks)) {
	    // File selected
	    if (!grep->finddupes)
	      DoOneFile(grep, szFindPath, pffbFile, pitdSleep, pitdReport);
	    else if (!InsertDupe(grep, szFindPath, pffbFile)) {
	      DosFindClose(findHandle);
	      free(pffbArray);
#             ifdef FORTIFY
	      Fortify_LeaveScope();
#              endif
	      return 1;
	    }
	  }
	}
	if (!pffbFile->oNextEntryOffset)
	  break;
	pffbFile = (PFILEFINDBUF4L)((PBYTE)pffbFile + pffbFile->oNextEntryOffset);
      } // for
      if (*grep->stopflag)
	break;
      IdleIfNeeded(pitdSleep, 30);
      ulFindCnt = FilesToGet;
      // Next file
      rc = xDosFindNext(findHandle, pffbArray, ulBufBytes, &ulFindCnt, FIL_QUERYEASIZEL);
    } while (!rc);

    DosFindClose(findHandle);
  } // if at least one file

  if (rc && rc != ERROR_NO_MORE_FILES) {
    if (rc == ERROR_FILENAME_EXCED_RANGE) {
      CHAR FileSystem[CCHMAXPATH];

      CheckDrive(toupper(*szFindPath), FileSystem, NULL);
      if (strcmp(FileSystem, NTFS))
	Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		  GetPString(IDS_CANTFINDDIRTEXT), szFindPath);
    }
    else
      Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		GetPString(IDS_CANTFINDDIRTEXT), szFindPath);
  }

  free(pffbArray);
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
  return 0;
}

static VOID freegreplist(GREP *grep)
{
  UINT x;

  if (grep) {
    if (grep->insertffb) {
      for (x = 0; grep->insertffb[x]; x++) {
	free(grep->insertffb[x]);
      }
      free(grep->insertffb);
    }
    if (grep->dir) {
      for (x = 0; grep->dir[x]; x++) {
	free(grep->dir[x]);
      }
      free(grep->dir);
    }
    grep->dir = NULL;
    grep->insertffb = NULL;
    grep->toinsert = 0L;
    grep->insertedbytes = 0L;
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
  }
}

/**
 * Insert one or more records into container
 */

static BOOL DoInsertion(GREP *grep,
			ITIMER_DESC *pitdSleep,
			ITIMER_DESC *pitdReport)
{
  RECORDINSERT ri;
  DIRCNRDATA *dcd;
  PCNRITEM pci, pciFirst;
  UINT x;
  ULONG ulRecsToInsert;

  if (!grep || !grep->toinsert || !grep->insertffb || !grep->dir)
    return FALSE;

  pci = NULL;
  dcd = INSTDATA(grep->hwndFiles);
  for (x = 0; grep->insertffb[x]; x++) {
    if (pci == NULL) {
      ulRecsToInsert = grep->toinsert - x < USHRT_MAX  ? grep->toinsert - x : USHRT_MAX;
      pciFirst = WinSendMsg(grep->hwndFiles, CM_ALLOCRECORD,
		       MPFROMLONG(EXTRA_RECORD_BYTES),
		       MPFROMLONG(ulRecsToInsert));
      if (!pciFirst) {
	Win_Error(grep->hwndFiles, grep->hwndFiles, pszSrcFile, __LINE__,
		  PCSZ_CM_ALLOCRECORD);
	freegreplist(grep);
	return FALSE;
      }
      else {
	pci = pciFirst;
	if (grep->sayfiles) {
	  if (!hwndStatus)
	    WinSetWindowText(grep->hwndCurFile, (CHAR *) GetPString(IDS_GREPINSERTINGTEXT));
	  else {
	    if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
	      WinSetWindowText(hwndStatus, (CHAR *) GetPString(IDS_GREPINSERTINGTEXT));
	  }
	}
      }
    }
    FillInRecordFromFFB(grep->hwndFiles,
			pci, grep->dir[x], grep->insertffb[x], FALSE, dcd);
    pci = (PCNRITEM) pci->rc.preccNextRecord;
    //SleepIfNeeded(pitdSleep, 1);
    if (pci == NULL && ulRecsToInsert) {
      memset(&ri, 0, sizeof(RECORDINSERT));
      ri.cb = sizeof(RECORDINSERT);
      ri.pRecordOrder = (PRECORDCORE) CMA_END;
      ri.pRecordParent = (PRECORDCORE) NULL;
      ri.zOrder = (USHORT) CMA_TOP;
      ri.cRecordsInsert = ulRecsToInsert;
      ri.fInvalidateRecord = TRUE;
      WinSendMsg(grep->hwndFiles,
		 CM_INSERTRECORD, MPFROMP(pciFirst), MPFROMP(&ri));
      if (dcd) {
	DosRequestMutexSem(hmtxFM2Globals, SEM_INDEFINITE_WAIT);
	dcd->ullTotalBytes += grep->insertedbytes;
	DosReleaseMutexSem(hmtxFM2Globals);
      }
      pciFirst = NULL;
    }
    SleepIfNeeded(pitdSleep, 1);
  }//for
    // if (grep->toinsert == FilesToGet)        // 07 Feb 08 SHL
    //  DosSleep(0);  //26 Aug 07 GKY 1 // 07 Feb 08 SHL
    freegreplist(grep);
    PostMsg(grep->hwndFiles, UM_RESCAN, MPVOID, MPVOID);
    return TRUE;
}

/**
 * Insert file ffb and directory name into lists
 */

static BOOL InsertGrepFile(GREP *grep,
			   CHAR *pszFileName,
			   PFILEFINDBUF4L pffb,
			   ITIMER_DESC *pitdSleep,
			   ITIMER_DESC *pitdReport)
{
  PSZ p;
  CHAR szDirectory[CCHMAXPATH];

  if (!WinIsWindow(grep->ghab, grep->hwndFiles)) {
    // Window closed - clean up and go away
    freegreplist(grep);
  }
  else {
    grep->numfiles++;
    strcpy(szDirectory, pszFileName);
    p = strrchr(szDirectory, '\\');

    if (p) {
      // Got directory
      if (p < szDirectory + 4)
	p++;                            // Include root backslash
      *p = 0;

      if (!grep->insertffb) {
	// Allocate 1 extra for end marker?
	grep->insertffb = xmallocz(sizeof(PFILEFINDBUF4L) * (FilesToGet + 1),
				   pszSrcFile, __LINE__);
	if (!grep->insertffb)
	  return FALSE;
	grep->dir = xmallocz(sizeof(CHAR *) * (FilesToGet + 1),
			     pszSrcFile, __LINE__);
	if (!grep->dir) {
	  free(grep->insertffb);
#         ifdef FORTIFY
	  Fortify_LeaveScope();
#          endif
	  return FALSE;
	}
      }

      grep->insertffb[grep->toinsert] =
	xmalloc(sizeof(FILEFINDBUF4L), pszSrcFile, __LINE__);
      if (!grep->insertffb[grep->toinsert])
	return FALSE;
      memcpy(grep->insertffb[grep->toinsert], pffb, sizeof(FILEFINDBUF4L));

      grep->dir[grep->toinsert] = xstrdup(szDirectory, pszSrcFile, __LINE__);
      if (!grep->dir) {
	free(grep->insertffb[grep->toinsert]);
#       ifdef FORTIFY
	Fortify_LeaveScope();
#        endif
	return FALSE;
      }

      grep->insertedbytes += pffb->cbFile + CBLIST_TO_EASIZE(pffb->cbList);
      grep->toinsert++;
      // 07 Oct 09 SHL honor sync updates
      if (grep->toinsert == FilesToGet || fSyncUpdates)
	return DoInsertion(grep, pitdSleep, pitdReport);
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Check file matches search criteria
 */

static BOOL DoOneFile(GREP *grep,
		      CHAR *pszFileName,
		      FILEFINDBUF4L *pffb,
		      ITIMER_DESC *pitdSleep,
		      ITIMER_DESC *pitdReport)
{
  // process a single file
  CHAR *input;
  FILE *inputFile;
  ULONG pos;
  BOOL ret = FALSE, strmatch = FALSE;

  grep->fileCount++;
  if (grep->sayfiles) {
    if (!hwndStatus)
      WinSetWindowText(grep->hwndCurFile, pszFileName);
    else {
      if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
	WinSetWindowText(hwndStatus, pszFileName);
    }
  }

  if (grep->greaterthan || grep->lessthan) {

    BOOL keep = TRUE;
    ULONG adjsize;

    adjsize = pffb->cbFile + (grep->searchEAs ? CBLIST_TO_EASIZE(pffb->cbList) : 0);
    if (grep->greaterthan) {
      if (adjsize < grep->greaterthan)
	keep = FALSE;
    }
    if (keep && grep->lessthan) {
      if (adjsize > grep->lessthan)
	keep = FALSE;
    }
    if (!keep)
      return ret;
  }

  if (grep->newerthan || grep->olderthan) {

    BOOL keep = TRUE;
    ULONG numsecs;

    numsecs = SecsSince1980(&pffb->fdateLastWrite, &pffb->ftimeLastWrite);
    if (grep->newerthan) {
      if (numsecs < grep->newerthan)
	keep = FALSE;
    }
    if (keep && grep->olderthan) {
      if (numsecs > grep->olderthan)
	keep = FALSE;
    }
    if (!keep)
      return ret;
  }

  if ((!grep->searchEAs && !grep->searchFiles) || !*grep->searchPattern)        // just a find
    return InsertGrepFile(grep, pszFileName, pffb, pitdSleep, pitdReport);

  if (grep->searchEAs) {

    HOLDFEA *head, *info;
    USHORT type, len;
    BOOL alltext;
    CHAR *data, temp;

    head = GetFileEAs(pszFileName, FALSE, TRUE);
    if (head) {
      info = head;
      while (info && !strmatch) {
	alltext = TRUE;
	switch (*(USHORT *)info->value) {
	case EAT_ASCII:
	  if (match(info->value + (sizeof(USHORT) * 2),
		    grep->searchPattern, grep->absFlag,
		    grep->caseFlag == FALSE,
		    info->cbValue - (sizeof(USHORT) * 2),
		    grep->numlines,
		    grep->matched,
		    !grep->findifany)) {
	    strmatch = TRUE;
	  }
	  break;
	case EAT_MVST:
	  type = *(USHORT *)(info->value + (sizeof(USHORT) * 3));
	  if (type == EAT_ASCII) {
	    data = info->value + (sizeof(USHORT) * 4);
	    len = *(USHORT *) data;
	    data += sizeof(USHORT);
	    while ((data - info->value) + len <= info->cbValue) {
	      temp = *(data + len);
	      *(data + len) = 0;
	      if (match(data,
			grep->searchPattern,
			grep->absFlag,
			(grep->caseFlag == FALSE),
			len,
			grep->numlines, grep->matched, !grep->findifany)) {
		strmatch = TRUE;
		break;
	      }
	      data += len;
	      if (data - info->value >= info->cbValue)
		break;
	      *data = temp;
	      len = *(USHORT *) data;
	      data += sizeof(USHORT);
	    }
	  }
	  break;
	case EAT_MVMT:
	  data = info->value + (sizeof(USHORT) * 3);
	  type = *(USHORT *) data;
	  data += sizeof(USHORT);
	  len = *(USHORT *) data;
	  data += sizeof(USHORT);
	  while ((data - info->value) - len <= info->cbValue) {
	    if (type != EAT_ASCII) {
	      alltext = FALSE;
	      break;
	    }
	    data += len;
	    if (data - info->value >= info->cbValue)
	      break;
	    type = *(USHORT *) data;
	    data += sizeof(USHORT);
	    len = *(USHORT *) data;
	    data += sizeof(USHORT);
	  }
	  if (alltext) {
	    data = info->value + (sizeof(USHORT) * 3);
	    type = *(USHORT *) data;
	    data += sizeof(USHORT);
	    len = *(USHORT *) data;
	    data += sizeof(USHORT);
	    while ((data - info->value) - len <= info->cbValue) {
	      temp = *(data + len);
	      *(data + len) = 0;
	      if (match(data,
			grep->searchPattern,
			grep->absFlag,
			(grep->caseFlag == FALSE),
			len,
			grep->numlines, grep->matched, !grep->findifany)) {
		strmatch = TRUE;
		break;
	      }
	      data += len;
	      *data = temp;
	      if (data - info->value >= info->cbValue)
		break;
	      type = *(USHORT *) data;
	      data += sizeof(USHORT);
	      len = *(USHORT *) data;
	      data += sizeof(USHORT);
	    }
	  }
	  break;
	default:
	  break;
	}
	info = info->next;
      }                                 // while
      Free_FEAList(head);
      // DosSleep(1);                   // 07 Feb 08 SHL
    }
  }

  if (grep->searchFiles) {
    input = xmalloc(65537, pszSrcFile, __LINE__);
    if (input) {
      LONG len;
      CHAR *moderb = "rb";

      inputFile = xfsopen(pszFileName, moderb, SH_DENYNO, pszSrcFile, __LINE__, TRUE);
      if (inputFile) {
	pos = ftell(inputFile);
	while (!feof(inputFile)) {
	  if (pos)
	    fseek(inputFile, pos - 1024, SEEK_SET);
	  len = fread(input, 1, 65536, inputFile);
	  if (len >= 0) {
	    if (*grep->stopflag)
	      break;
	    if (match(input,
		      grep->searchPattern,
		      grep->absFlag,
		      (grep->caseFlag == FALSE),
		      len, grep->numlines, grep->matched, !grep->findifany)) {
	      strmatch = TRUE;
	      break;
	    }
	  }
	  else
	    break;
	}
	fclose(inputFile);
      }
      free(input);
#     ifdef FORTIFY
      Fortify_LeaveScope();
#      endif
      // DosSleep(1);                   // 07 Feb 08 SHL
    }
  } // if

  if (strmatch)
    ret = InsertGrepFile(grep, pszFileName, pffb, pitdSleep, pitdReport);
  return ret;
}

static LONG cr3tab[] = {                // CRC polynomial 0xEDB88320

  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
  0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
  0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
  0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
  0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
  0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
  0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
  0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
  0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
  0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
  0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
  0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
  0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
  0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
  0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
  0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
  0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
  0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
  0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
  0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
  0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
  0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
  0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
  0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
  0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
  0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
  0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
  0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
  0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
  0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
  0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
  0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
  0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
  0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
  0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
  0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
  0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
  0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
  0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
  0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
  0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
  0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

LONG CRCBlock(register CHAR *str, register INT blklen, register LONG crc)
{
  while (blklen--) {
    crc =
      cr3tab[((INT) crc ^ *str) & 0xff] ^ (((ULONG) crc >> 8) & 0x00FFFFFF);
    str++;
  }
  return crc;
}

LONG CRCFile(CHAR *pszFileName, INT *error)
{
  LONG CRC = -1L, len;
  FILE *fp;
  CHAR *buffer;
  CHAR *moderb = "rb";

  *error = 0;
  buffer = xmalloc(65535, pszSrcFile, __LINE__);
  if (!buffer)
    *error = -1;
  else {
    fp = xfsopen(pszFileName, moderb, SH_DENYNO, pszSrcFile, __LINE__, TRUE);
    if (!fp)
      *error = -2;
    else {
      while (!feof(fp)) {
	len = fread(buffer, 1, 65535, fp);
	if (len && len < 65536L)
	  CRC = CRCBlock(buffer, len, CRC);
	else
	  break;
	// DosSleep(0); //26 Aug 07 GKY 1       // 07 Feb 08 SHL
      }
      fclose(fp);
      // DosSleep(1);                   // 07 Feb 08 SHL
    }
    free(buffer);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
  }
  return CRC;
}

static VOID FreeDupes(GREP *grep)
{
  DUPES *i, *next;

  i = grep->dupehead;
  while (i) {
    next = i->next;
    if (i->name) {
      free(i->name);
    }
    free(i);
    i = next;
  }
  grep->dupehead = grep->dupelast = NULL;
  xfree(grep->dupenames, pszSrcFile, __LINE__);
  xfree(grep->dupesizes, pszSrcFile, __LINE__);
  grep->dupesizes = grep->dupenames = NULL;
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
}

INT comparenamesq(const VOID *v1, const VOID *v2)
{
  DUPES *d1 = *(DUPES **) v1;
  DUPES *d2 = *(DUPES **) v2;
  CHAR *p1, *p2;

  p1 = strrchr(d1->name, '\\');
  if (p1)
    p1++;
  else
    p1 = d1->name;
  p2 = strrchr(d2->name, '\\');
  if (p2)
    p2++;
  else
    p2 = d2->name;
  return stricmp(p1, p2);
}

INT comparenamesqe(const VOID *v1, const VOID *v2)
{
  DUPES *d1 = *(DUPES **) v1;
  DUPES *d2 = *(DUPES **) v2;
  CHAR *p1, *p2, *p1e, *p2e, e1, e2;
  INT ret;

  p1 = strrchr(d1->name, '\\');
  if (p1)
    p1++;
  else
    p1 = d1->name;
  p1e = strrchr(p1, '.');
  if (p1e) {
    e1 = *p1e;
    *p1e = 0;
  }
  p2 = strrchr(d2->name, '\\');
  if (p2)
    p2++;
  else
    p2 = d2->name;
  p2e = strrchr(p2, '.');
  if (p2e) {
    e2 = *p2e;
    *p2e = 0;
  }
  ret = stricmp(p1, p2);
  if (p1e)
    *p1e = e1;
  if (p2e)
    *p2e = e2;
  return ret;
}

INT comparesizesq(const void *v1, const void *v2)
{
  DUPES *d1 = *(DUPES **) v1;
  DUPES *d2 = *(DUPES **) v2;

  return (d1->size > d2->size) ? 1 : (d1->size == d2->size) ? 0 : -1;
}

INT comparenamesb(const void *v1, const void *v2)
{
  DUPES *d1 = (DUPES *) v1;
  DUPES *d2 = *(DUPES **) v2;
  CHAR *p1, *p2;

  p1 = strrchr(d1->name, '\\');
  if (p1)
    p1++;
  else
    p1 = d1->name;
  p2 = strrchr(d2->name, '\\');
  if (p2)
    p2++;
  else
    p2 = d2->name;
  return stricmp(p1, p2);
}

INT comparenamesbe(const VOID *v1, const VOID *v2)
{
  DUPES *d1 = (DUPES *) v1;
  DUPES *d2 = *(DUPES **) v2;
  CHAR *p1, *p2, *p1e, *p2e, e1, e2;
  INT ret;

  p1 = strrchr(d1->name, '\\');
  if (p1)
    p1++;
  else
    p1 = d1->name;
  p1e = strrchr(p1, '.');
  if (p1e) {
    e1 = *p1e;
    *p1e = 0;
  }
  p2 = strrchr(d2->name, '\\');
  if (p2)
    p2++;
  else
    p2 = d2->name;
  p2e = strrchr(p2, '.');
  if (p2e) {
    e2 = *p2e;
    *p2e = 0;
  }
  ret = stricmp(p1, p2);
  if (p1e)
    *p1e = e1;
  if (p2e)
    *p2e = e2;
  return ret;
}

INT comparesizesb(const VOID *v1, const VOID *v2)
{
  DUPES *d1 = (DUPES *) v1;
  DUPES *d2 = *(DUPES **) v2;

  return (d1->size > d2->size) ? 1 : (d1->size == d2->size) ? 0 : -1;
}

static VOID FillDupes(GREP *grep,
		      ITIMER_DESC *pitdSleep,
		      ITIMER_DESC *pitdReport)
{
  DUPES *c, *i, **r;
  register CHAR *pc, *pi;
  CHAR **list = NULL;
  UINT numfiles = 0, numalloced = 0;
  INT error;
  ULONG x;
  ULONG y;
  // ULONG cntr = 1000;                 // 09 Feb 08 SHL

  // if (grep->CRCdupes)                // 09 Feb 08 SHL
  //  cntr = 100;                       // 09 Feb 08 SHL
  x = 0;
  for (i = grep->dupehead; i; i = i->next)
    x++;                                // Count

  if (x) {
    if (!hwndStatus)
      WinSetWindowText(grep->hwndCurFile, (CHAR *) GetPString(IDS_GREPDUPESORTINGTEXT));
    else if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
      WinSetWindowText(hwndStatus, (CHAR *) GetPString(IDS_GREPDUPESORTINGTEXT));
    // DosSleep(0);  //26 Aug 07 GKY 1  // 07 Feb 08 SHL
    grep->dupenames = xmalloc(sizeof(DUPES *) * (x + 1), pszSrcFile, __LINE__);
    if (!grep->nosizedupes)
      grep->dupesizes = xmalloc(sizeof(DUPES *) * (x + 1), pszSrcFile, __LINE__);
    if (grep->dupenames && (grep->nosizedupes || grep->dupesizes)) {
      y = 0;
      for (i = grep->dupehead; i; i = i->next) {
	grep->dupenames[y] = i;
	if (!grep->nosizedupes)
	  grep->dupesizes[y] = i;
	y++;
      }
      grep->dupenames[y] = NULL;        // Mark end
      if (!grep->nosizedupes)
	grep->dupesizes[y] = NULL;

      InitITimer(pitdSleep, 0);         // Reset rate estimator
      SleepIfNeeded(pitdSleep, 1);
      // DosSleep(0); //26 Aug 07 GKY 1 // 07 Feb 08 SHL

      qsort(grep->dupenames,
	    x,
	    sizeof(DUPES *),
	    grep->ignoreextdupes ? comparenamesqe : comparenamesq);
      SleepIfNeeded(pitdSleep, 1);
      // DosSleep(0); //26 Aug 07 GKY 1 // 07 Feb 08 SHL
      if (!grep->nosizedupes) {
	qsort(grep->dupesizes, x, sizeof(DUPES *), comparesizesq);
	SleepIfNeeded(pitdSleep, 1);
	// DosSleep(0); //26 Aug 07 GKY 1       // 07 Feb 08 SHL
      }

      if (!hwndStatus)
	WinSetWindowText(grep->hwndCurFile, (CHAR *) GetPString(IDS_GREPDUPECOMPARINGTEXT));
      else if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
	WinSetWindowText(hwndStatus, (CHAR *) GetPString(IDS_GREPDUPECOMPARINGTEXT));

      InitITimer(pitdSleep, 0);         // Reset rate estimator
      i = grep->dupehead;
      y = 0;
      while (i) {
	if (*grep->stopflag)
	  break;
	SleepIfNeeded(pitdSleep, 1);    // 07 Feb 08 SHL
	if (!(i->flags & GF_SKIPME)) {
	  r = (DUPES **) bsearch(i, grep->dupenames, x, sizeof(DUPES *),
				 ((grep->ignoreextdupes) ? comparenamesbe :
				  comparenamesb));
	  if (r) {
	    while (r > grep->dupenames && ((grep->ignoreextdupes) ?
					!comparenamesqe((r - 1), &i) :
					!comparenamesq((r - 1), &i)))
	      r--;
	    while (*r && ((grep->ignoreextdupes) ?
			  !comparenamesqe(r, &i) : !comparenamesq(r, &i))) {
	      if (*r == i || ((*r)->flags & (GF_INSERTED | GF_SKIPME))) {
		r++;
		continue;
	      }
	      if (grep->CRCdupes) {
		if ((*r)->CRC == -1L) {
		  (*r)->CRC = CRCFile((*r)->name, &error);
		  if (error)
		    (*r)->CRC = -1L;
		  else if ((*r)->CRC == -1L)
		    (*r)->CRC = 0L;
		}
		if (i->CRC == -1L) {
		  i->CRC = CRCFile(i->name, &error);
		  if (error)
		    i->CRC = -1L;
		  else if (i->CRC == -1L)
		    i->CRC = 0L;
		}
		if (((*r)->size != i->size) || ((*r)->CRC != -1L &&
						i->CRC != -1L
						&& (*r)->CRC != i->CRC)) {
		  r++;
		  continue;
		}
	      }
	      if (!AddToList((*r)->name, &list, &numfiles, &numalloced)) {
		(*r)->flags |= GF_INSERTED;
		if (grep->sayfiles) {
		  if (!hwndStatus)
		    WinSetWindowText(grep->hwndFiles, (*r)->name);
		  else if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
		    WinSetWindowText(hwndStatus, (*r)->name);
		}
		if ((*r)->size == i->size &&
		    (i->date.year == (*r)->date.year &&
		     i->date.month == (*r)->date.month &&
		     i->date.day == (*r)->date.day &&
		     i->time.hours == (*r)->time.hours &&
		     i->time.minutes == (*r)->time.minutes &&
		     i->time.twosecs == (*r)->time.twosecs))
		  (*r)->flags |= GF_SKIPME;
	      }
	      if (!(i->flags & (GF_INSERTED | GF_SKIPME))) {
		if (!AddToList(i->name, &list, &numfiles, &numalloced)) {
		  i->flags |= GF_INSERTED;
		  if ((*r)->flags & GF_SKIPME)
		    i->flags |= GF_SKIPME;
		}
	      }
	      r++;
	    }
	  }
	  if (!grep->nosizedupes) {
	    r = (DUPES **) bsearch(i,
				   grep->dupesizes,
				   x, sizeof(DUPES *), comparesizesb);
	    if (r) {
	      while (r > grep->dupesizes && !comparesizesq((r - 1), &i))
		r--;
	      while (*r && !comparesizesq(r, &i)) {
		if (*r == i || ((*r)->flags & (GF_INSERTED | GF_SKIPME)) ||
		    (i->date.year != (*r)->date.year ||
		     i->date.month != (*r)->date.month ||
		     i->date.day != (*r)->date.day ||
		     i->time.hours != (*r)->time.hours ||
		     i->time.minutes != (*r)->time.minutes ||
		     i->time.twosecs != (*r)->time.twosecs)) {
		  r++;
		  continue;
		}
		if (grep->CRCdupes) {
		  if ((*r)->CRC == -1L) {
		    (*r)->CRC = CRCFile((*r)->name, &error);
		    if (error)
		      (*r)->CRC = -1L;
		    else if ((*r)->CRC == -1L)
		      (*r)->CRC = 0L;
		  }
		  if (i->CRC == -1L) {
		    i->CRC = CRCFile(i->name, &error);
		    if (error)
		      i->CRC = -1L;
		    else if (i->CRC == -1L)
		      i->CRC = 0L;
		  }
		  if ((*r)->CRC != -1L && i->CRC != -1L &&
		      (*r)->CRC != i->CRC) {
		    *r += 1;
		    continue;
		  }
		}
		if (!AddToList((*r)->name, &list, &numfiles, &numalloced)) {
		  if (grep->sayfiles) {
		    if (!hwndStatus)
		      WinSetWindowText(grep->hwndCurFile, (*r)->name);
		    else if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
		      WinSetWindowText(hwndStatus, (*r)->name);
		  }
		  (*r)->flags |= GF_INSERTED;
		  if (((grep->ignoreextdupes) ?
		       comparenamesqe(r, &i) : comparenamesq(r, &i)))
		    (*r)->flags |= GF_SKIPME;
		}
		if (!(i->flags & (GF_INSERTED | GF_SKIPME))) {
		  if (!AddToList(i->name, &list, &numfiles, &numalloced)) {
		    i->flags |= GF_INSERTED;
		    if ((*r)->flags & GF_SKIPME)
		      i->flags |= GF_SKIPME;
		  }
		}
		r++;
	      }
	    }
	  }
	}
	i = i->next;
	y++;
	// 08 Feb 08 SHL
	if (IsITimerExpired(pitdReport)) {
	  CHAR s[44];
	  sprintf(s, GetPString(IDS_GREPDUPECHECKPROGTEXT), y, grep->numfiles);
	  if (!hwndStatus)
	    WinSetWindowText(grep->hwndCurFile, s);
	  else {
	    if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
	      WinSetWindowText(hwndStatus, s);
	  }
	}
      } // while
    }
    else {
      // Insufficient memory - fall back to slow method - FIXME to saymsg?
      if (!fErrorBeepOff)
	DosBeep(50, 100);
      if (!hwndStatus)
	WinSetWindowText(grep->hwndCurFile, (CHAR *) GetPString(IDS_GREPDUPECOMPARINGTEXT));
      else if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
	WinSetWindowText(hwndStatus, (CHAR *) GetPString(IDS_GREPDUPECOMPARINGTEXT));
      x = y = 0;
      xfree(grep->dupenames, pszSrcFile, __LINE__);
      grep->dupenames = NULL;
      xfree(grep->dupesizes, pszSrcFile, __LINE__);
      grep->dupesizes = NULL;
#     ifdef FORTIFY
      Fortify_LeaveScope();
#      endif

      InitITimer(pitdSleep, 0);         // Reset rate estimator
      i = grep->dupehead;
      while (i) {
	if (*grep->stopflag)
	  break;
	SleepIfNeeded(pitdSleep, 1);
	if (!(i->flags & GF_SKIPME)) {
	  if (IsITimerExpired(pitdReport)) {
	    // if (!(y % cntr)) { }
	    CHAR s[44];
	    sprintf(s, GetPString(IDS_GREPDUPECHECKPROGTEXT), y, grep->numfiles);
	    if (!hwndStatus)
	      WinSetWindowText(grep->hwndCurFile, s);
	    else if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
	      WinSetWindowText(hwndStatus, s);
	    // DosSleep(0); //26 Aug 07 GKY 1   // 07 Feb 08 SHL
	  }
	  y++;
	  pi = strrchr(i->name, '\\');
	  if (pi)
	    pi++;
	  else
	    pi = i->name;
	  c = grep->dupehead;
	  while (c) {
	    if (*grep->stopflag)
	      break;
	    if (c != i && !(c->flags & (GF_INSERTED | GF_SKIPME))) {
	      x++;
	      pc = strrchr(c->name, '\\');
	      if (pc)
		pc++;
	      else
		pc = c->name;
	      if ((!grep->nosizedupes && i->size == c->size && i->date.year == c->date.year && i->date.month == c->date.month && i->date.day == c->date.day && i->time.hours == c->time.hours && i->time.minutes == c->time.minutes && i->time.twosecs == c->time.twosecs) || !stricmp(pc, pi)) {       // potential dupe
		if (grep->CRCdupes) {
		  if (grep->CRCdupes) {
		    if (c->CRC == -1L) {
		      c->CRC = CRCFile(c->name, &error);
		      if (error)
			c->CRC = -1L;
		      else if (c->CRC == -1L)
			c->CRC = 0L;
		    }
		    if (i->CRC == -1L) {
		      i->CRC = CRCFile(i->name, &error);
		      if (error)
			i->CRC = -1L;
		      else if (i->CRC == -1L)
			i->CRC = 0L;
		    }
		    if ((c->size != i->size) || (c->CRC != -1L &&
						 i->CRC != -1L
						 && c->CRC != i->CRC)) {
		      c = c->next;
		      continue;
		    }
		  }
		}
		if (AddToList(c->name, &list, &numfiles, &numalloced))
		  goto BreakOut;        // Failed
		if (!(i->flags & GF_INSERTED)) {
		  if (AddToList(i->name, &list, &numfiles, &numalloced))
		    goto BreakOut;      // Failed
		}
		if (grep->sayfiles) {
		  if (!hwndStatus)
		    WinSetWindowText(grep->hwndCurFile, pc);
		  else if (WinQueryFocus(HWND_DESKTOP) == grep->hwndFiles)
		    WinSetWindowText(hwndStatus, pc);
		}
		c->flags |= GF_INSERTED;
		i->flags |= GF_INSERTED;
		if (!stricmp(pc, pi)) {
		  c->flags |= GF_SKIPME;
		  i->flags |= GF_SKIPME;
		}
	      }
	      // else if (!(x % 100))   // 07 Feb 08 SHL
	      //        DosSleep(0);  //26 Aug 07 GKY 1 // 07 Feb 08 SHL
	    }
	    c = c->next;
	  }
	}
	i = i->next;
      } // while
    }
  }
BreakOut:
  FreeDupes(grep);
  if (numfiles && list) {
    if (!PostMsg(grep->hwndFiles,
		 WM_COMMAND,
		 MPFROM2SHORT(IDM_COLLECTOR, 0),
		 MPFROMP(list)))
      FreeList(list);
  }
  else
    DosPostEventSem(CompactSem);
}

static BOOL InsertDupe(GREP *grep, CHAR *dir, FILEFINDBUF4L *pffb)
{
  DUPES *info;

  if (*dir) {
    info = xmallocz(sizeof(DUPES), pszSrcFile, __LINE__);
    if (!info)
      return FALSE;

    info->name = xstrdup(dir, pszSrcFile, __LINE__);
    if (!info->name) {
      free(info);
#     ifdef FORTIFY
      Fortify_LeaveScope();
#      endif
      return FALSE;
    }

    info->size = pffb->cbFile;
    info->date = pffb->fdateLastWrite;
    info->time = pffb->ftimeLastWrite;
    info->CRC = -1L;
    grep->numfiles++;
    if (!grep->dupehead)
      grep->dupehead = info;
    if (grep->dupelast)
      grep->dupelast->next = info;
    grep->dupelast = info;
    info->next = NULL;
  }
  return TRUE;
}

#pragma alloc_text(GREP,InsertGrepFile,DoOneFile,DoInsertion,freegreplist)
#pragma alloc_text(GREP,SecsSince1980,match,mmatch,GrepThread)
#pragma alloc_text(GREP,DoAllSubdirs,DoMatchingFiles,InsertDupes,FreeDupes)

#pragma alloc_text(DUPES,InsertDupe,FillDupes,FreeDupes,CRCFile,CRCBlock)
#pragma alloc_text(DUPES,comparenamesq,comparenamesqe,comparenamesb)
#pragma alloc_text(DUPES,comparenamesbe,comparesizesq,comparesizesb)
