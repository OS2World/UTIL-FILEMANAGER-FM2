
/***********************************************************************

  $Id$

  Copy functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2009 Steven H.Levine

  14 Sep 02 SHL Drop obsolete debug code
  14 Oct 02 SHL Drop obsolete debug code
  10 Nov 02 SHL docopyf - don't forget to terminate longname
		optimize longname logic
  01 Aug 04 SHL Rework lstrip/rstrip usage
  28 May 05 SHL Drop debug code
  14 Jul 06 SHL Use Runtime_Error
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  01 Sep 07 GKY Use xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundry
  29 Feb 08 GKY Use xfree where appropriate
  19 Jul 08 GKY Modify MakeTempName for use making temp directory names
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)
  28 Jun 09 GKY Added AddBackslashToPath() to remove repeatative code.
  12 Jul 09 GKY Add xDosQueryAppType and xDosAlloc... to allow FM/2 to load in high memory
  13 Jul 09 SHL Drop obsolete code
  22 Jul 09 GKY Delete .LONGNAME EA if it becomes the filename on a copy or move.
  19 Oct 09 SHL Correct copyf regression when moving to save volume
  31 Mar 10 JBS Correct copyf which was creating 8.4, not 8.3, temporary names
  26 Aug 11 GKY Add a low mem version of xDosAlloc* wrappers; move error checking into all the
                xDosAlloc* wrappers.
  04 Aug 12 GKY Changes to use Unlock to unlock files if Unlock.exe is in path both from menu/toolbar and as part of copy, move and
                delete operations
  04 Aug 12 GKY Changes to allow copy and move over readonly files with a warning dialog; also added a warning dialog for delete of
                readonly files
  10 Mar 13 GKY Improvrd readonly check on delete to allow cancel and don't ask again options

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "killproc.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "arccnrs.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "copyf.h"
#include "literal.h"			// fixup
#include "misc.h"			// Broadcast
#include "valid.h"			// MakeFullName
#include "wrappers.h"			// xDosSetPathInfo
#include "strips.h"			// bstrip
#include "fortify.h"
#include "pathutil.h"			// AddBackslashToPath
#include "worker.h"
#include "systemf.h"

static PSZ pszSrcFile = __FILE__;

static CHAR *GetLongName(CHAR * oldname, CHAR * buffer);
static CHAR *TruncName(CHAR * oldname, CHAR * buffer);

//static CHAR default_disk(VOID);
//static INT unlink_allf(CHAR * string, ...);

#ifndef WinMoveObject
HOBJECT APIENTRY WinMoveObject(HOBJECT hObjectofObject,
			       HOBJECT hObjectofDest, ULONG ulReserved);
#endif
#ifndef WinCopyObject
HOBJECT APIENTRY WinCopyObject(HOBJECT hObjectofObject,
			       HOBJECT hObjectofDest, ULONG ulReserved);
#endif

/**
 * Make temporary file name
 * @param buffer is input directory name and output file name buffer
 * @param temproot is filename root used by type 2
 * @param type is name style
 * @return pointer to name in buffer or NULL if failed
 * @note not MT safe
 */

PSZ MakeTempName(PSZ buffer, PSZ temproot, INT type)
{
  FILESTATUS3 fs3;
  APIRET rc;
  PSZ p;
  PSZ o;

  if (strlen(buffer) > 3)
    AddBackslashToPath(buffer);
  p = o = buffer + strlen(buffer);
  switch (type) {
  case 0:
    sprintf(p, "%08lx.%03lx", (UINT)mypid, (UINT)rand() & 4095L);  		// 4095 = 0x0FFF
    break;
  case 1:
    sprintf(p, "%s%04lx.%03lx", "$FM2", (UINT)mypid, (UINT)rand() & 4095L);	// 4095 = 0x0FFF
    break;
  case 2:
    sprintf(p, "%s.%03x", temproot, (UINT)(rand() & 4095));		  	// 4095 = 0x0FFF
    break;
  default:
    break;
  }
  p = buffer + (strlen(buffer) - 1);
  for (;;) {
    DosError(FERR_DISABLEHARDERR);
    rc = DosQueryPathInfo(buffer, FIL_STANDARD, &fs3, sizeof(fs3));
    if (rc == ERROR_DISK_CHANGE) {
      DosError(FERR_ENABLEHARDERR);
      rc = DosQueryPathInfo(buffer, FIL_STANDARD, &fs3, sizeof(fs3));
    }
    if (rc)
      break;
  Loop:
    if (p < o) {
      *buffer = 0;
      return NULL;
    }
    if ((*p) + 1 < 'Z' + 1) {
      (*p)++;
      while (strchr("*?<>\":/\\|+=;,[]. ", *p))
	(*p)++;
      *p = toupper(*p);
    }
    else {
      p--;
      if (p >= o && *p == '.')
	p--;
      goto Loop;
    }
  }
  return buffer;
}

CHAR *TruncName(CHAR * oldname, CHAR * buffer)
{
  CHAR *p, *f, *s, *o;
  FILESTATUS3 fs3;
  APIRET rc;

  if (!buffer || !oldname || !*oldname) {
    if (buffer)
      *buffer = 0;
    return NULL;
  }
  strcpy(buffer, oldname);
  f = strrchr(buffer, '\\');
  if (!f)
    f = strrchr(buffer, '/');
  if (!f)
    f = buffer;
  else
    f++;
  p = f;
  o = p;
  f = oldname + (f - buffer);
  strupr(buffer);
  while (*f == '.')
    f++;		// skip leading '.'s
  s = f;
  // skip past rootname
  while (*f && *f != '.' && f < s + 8) {
    *p = toupper(*f);
    p++;
    f++;
  }
  while (*f == '.')
    f++;
  s = f;
  f = strrchr(f, '.');
  if (f) {
    while (*f == '.')
      f++;
  }
  if (f && *(f + 1))
    s = f;
  else
    f = s;
  if (*f) {
    *p = '.';
    p++;
    while (*f && *f != '.' && f < s + 3) {
      *p = toupper(*f);
      p++;
      f++;
    }
  }
  *p = 0;

  p = o;
  while (*p) {
    if (strchr("*?<>\":/\\|+=;,[] ", *p) || *p < 0x20)
      *p = '_';
    if (*p == '.' && *(p + 1) == '.')
      *(p + 1) = '_';
    p++;
  }

  p = o + (strlen(o) - 1);
  for (;;) {
    DosError(FERR_DISABLEHARDERR);
    rc = DosQueryPathInfo(buffer, FIL_STANDARD, &fs3, sizeof(fs3));
    if (rc == ERROR_DISK_CHANGE) {
      DosError(FERR_ENABLEHARDERR);
      rc = DosQueryPathInfo(buffer, FIL_STANDARD, &fs3, sizeof(fs3));
    }
    if (rc)
      break;
  Loop:
    if (p < o) {
      *buffer = 0;
      return NULL;
    }
    if ((*p) + 1 < 'Z' + 1) {
      (*p)++;
      while (strchr("*?<>\":/\\|+=;,[]. ", *p))
	(*p)++;
      *p = toupper(*p);
    }
    else {
      p--;
      if (p >= o && *p == '.')
	p--;
      goto Loop;
    }
  }
  return buffer;
}

CHAR *GetLongName(CHAR * oldname, CHAR * longname)
{
  if (!longname)
    return NULL;
  *longname = 0;
  if (!oldname || !*oldname)
    return NULL;
  if (IsFullName(oldname)) {

    APIRET rc;
    EAOP2 eaop;
    PGEA2LIST pgealist;
    PFEA2LIST pfealist;
    PGEA2 pgea;
    PFEA2 pfea;
    CHAR *value;

    strcpy(longname, oldname);
    value = longname;
    while (*value) {
      if (*value == '/')
	*value = '\\';
      value++;
    }
    value = strrchr(longname, '\\');
    if (value) {
      value++;
      *value = 0;
    }
    pgealist = xmallocz(sizeof(GEA2LIST) + 32, pszSrcFile, __LINE__);
    if (pgealist) {
      pgea = &pgealist->list[0];
      strcpy(pgea->szName, LONGNAME);
      pgea->cbName = strlen(pgea->szName);
      pgea->oNextEntryOffset = 0L;
      pgealist->cbList = (sizeof(GEA2LIST) + pgea->cbName);
      pfealist = xmallocz(1536, pszSrcFile, __LINE__);
      if (pfealist) {
	pfealist->cbList = 1024;
	eaop.fpGEA2List = pgealist;
	eaop.fpFEA2List = pfealist;
	eaop.oError = 0L;
	DosError(FERR_DISABLEHARDERR);
	rc = DosQueryPathInfo(oldname,
			      FIL_QUERYEASFROMLIST,
			      (PVOID) & eaop, (ULONG) sizeof(EAOP2));
	if (!rc) {
	  pfea = &eaop.fpFEA2List->list[0];
	  value = pfea->szName + pfea->cbName + 1;
	  value[pfea->cbValue] = 0;
	  if (*(USHORT *) value == EAT_ASCII)
	    strncat(longname,
		    value + (sizeof(USHORT) * 2),
		    CCHMAXPATH - strlen(longname));
	  longname[CCHMAXPATH - 1] = 0;
	}
	free(pfealist);
      }
      free(pgealist);
    }
  }
  return longname;
}

BOOL ZapLongName(PSZ filename)
{
  return WriteLongName(filename, NullStr);
}

BOOL WriteLongName(CHAR * filename, CHAR * longname)
{
  APIRET rc;
  EAOP2 eaop;
  PFEA2LIST pfealist = NULL;
  ULONG ealen;
  USHORT len;
  CHAR *eaval, *p;

  if (!filename || !*filename || !longname)
    return FALSE;
  p = strrchr(longname, '\\');
  if (p)
    memmove(longname, p + 1, strlen(p + 1) + 1);
  p = strrchr(longname, '/');
  if (p)
    memmove(longname, p + 1, strlen(p + 1) + 1);
  bstrip(longname);
  len = strlen(longname);
  if (len)
    ealen = sizeof(FEA2LIST) + 10 + len + 4;
  else
    ealen = sizeof(FEALIST) + 10;
  if (xDosAllocMem((PPVOID) &pfealist, ealen + 32L, pszSrcFile, __LINE__))
    return FALSE;
  else {
    memset(pfealist, 0, ealen + 1);
    pfealist->cbList = ealen;
    pfealist->list[0].oNextEntryOffset = 0;
    pfealist->list[0].fEA = 0;
    pfealist->list[0].cbName = 9;
    strcpy(pfealist->list[0].szName, LONGNAME);
    if (len) {
      eaval = pfealist->list[0].szName + 10;
      *(USHORT *) eaval = (USHORT) EAT_ASCII;
      eaval += sizeof(USHORT);
      *(USHORT *) eaval = (USHORT) len;
      eaval += sizeof(USHORT);
      memcpy(eaval, longname, len);
      pfealist->list[0].cbValue = len + (sizeof(USHORT) * 2);
    }
    else
      pfealist->list[0].cbValue = 0;
    eaop.fpGEA2List = (PGEA2LIST) 0;
    eaop.fpFEA2List = pfealist;
    eaop.oError = 0L;
    DosError(FERR_DISABLEHARDERR);
    rc = xDosSetPathInfo(filename, FIL_QUERYEASIZE,
			 &eaop, sizeof(eaop), DSPI_WRTTHRU);
    DosFreeMem(pfealist);
    if (rc)
      return FALSE;
  }
  return TRUE;
}

BOOL AdjustWildcardName(CHAR * oldname, CHAR * newname)
{
  BOOL ret = FALSE;

  // NOTE: newname should be CCHMAXPATH chars long!

  if (strchr(newname, '*') || strchr(newname, '?')) {

    CHAR srce[CCHMAXPATHCOMP], dest[CCHMAXPATHCOMP], result[CCHMAXPATHCOMP],
	 *p;

    p = strrchr(newname, '\\');
    if (p && *(p + 1)) {
      strcpy(dest, p + 1);
      p = strrchr(oldname, '\\');
      if (p && *(p + 1)) {
	strcpy(srce, p + 1);
	DosError(FERR_DISABLEHARDERR);
	if (!DosEditName(1L, srce, dest, (PBYTE)result, (ULONG)sizeof(result))) {
	  p = strrchr(newname, '\\');
	  p++;
	  strcpy(p, result);
	  ret = TRUE;
	}
      }
    }
  }
  return ret;
}

/** Copy/move file
 * @param type is copy/move type
 * @param oldname is fully qualified source file name
 * @param newname is fully qualified destination file name
 * @return
 *   0:  success
 *  -1:  bad parameter(s)
 *  -2:  source does not exist
 *  -3:  bad copy/move type
 *   anything else: API return
 */

APIRET docopyf(INT type, CHAR *oldname, CHAR *newname)
{
  CHAR longname[CCHMAXPATH], shortname[CCHMAXPATH];
  CHAR olddisk, newdisk, dir[CCHMAXPATH], *p, *pp;
  APIRET ret = -1, rc;
  FILESTATUS3L st, st2, dummy;
  BOOL diskchange = FALSE, zaplong = FALSE;

  *shortname = *dir = 0;

  if (!oldname || !*oldname || !*newname)
    return (APIRET)-1;			// bad args

  DosError(FERR_DISABLEHARDERR);
  if (DosQueryPathInfo(oldname, FIL_STANDARDL, &st, sizeof(FILESTATUS3L)))
    return (APIRET)-2;			// can not access source

  AdjustWildcardName(oldname, newname);
  MakeFullName(oldname);
  MakeFullName(newname);
  olddisk = toupper(*oldname);		// source drive
  newdisk = toupper(*newname);		// destination drive
  if (!(driveflags[toupper(*oldname) - 'A'] & DRIVE_NOLONGNAMES))
    *longname = 0;
  else {
    GetLongName(oldname, longname);
    if (*longname) {
      p = RootName(longname);
      if (p != longname)
	memmove(longname, p, strlen(p) + 1);
    }
  }
  // If root name changed make sure longname EA goes away
  if (*longname) {
    p = RootName(oldname);
    pp = RootName(longname);
    if (stricmp(p, pp)) {
      zaplong = TRUE;
    }
  }

  DosError(FERR_DISABLEHARDERR);
  switch (type) {
  case WPSMOVE:
    {
      HOBJECT hobjsrc;
      HOBJECT hobjdest;

      ret = ERROR_FILE_NOT_FOUND;
      hobjsrc = WinQueryObject(oldname);
      if (hobjsrc) {
	strcpy(dir, newname);
	p = strrchr(dir, '\\');
	if (p < dir + 3)
	  p++;
	*p = 0;
	ret = ERROR_PATH_NOT_FOUND;
	hobjdest = WinQueryObject(dir);
	if (hobjdest) {
	  ret = ERROR_GEN_FAILURE;
	  hobjsrc = WinMoveObject(hobjsrc, hobjdest, 0);
	  if (hobjsrc)
	    ret = 0;
	}
      }
    }
    return ret;

  case WPSCOPY:
    {
      HOBJECT hobjsrc;
      HOBJECT hobjdest;

      ret = ERROR_FILE_NOT_FOUND;
      hobjsrc = WinQueryObject(oldname);
      if (hobjsrc) {
	strcpy(dir, newname);
	p = strrchr(dir, '\\');
	if (p < dir + 3)
	  p++;
	*p = 0;
	ret = ERROR_PATH_NOT_FOUND;
	hobjdest = WinQueryObject(dir);
	if (hobjdest) {
	  ret = ERROR_GEN_FAILURE;
	  hobjsrc = WinCopyObject(hobjsrc, hobjdest, 0);
	  if (hobjsrc)
	    ret = 0;
	}
      }
    }
    return ret;

  case MOVE:
    *dir = 0;
    if (olddisk == newdisk) {
      // Moving to same drive
      // make temporary copy in case move fails
      if (IsFile(newname) != -1 && stricmp(oldname, newname)) {
	// 19 Oct 09 SHL correct regression
	strcpy(dir, newname);
	p = strrchr(dir, '\\');
	if (p)
	  *p = 0;
	AddBackslashToPath(dir);
	MakeTempName(dir, NULL, 0);
	if (DosMove(newname, dir))
	  *dir = 0;			// Avoid trying to restore bad copy
      }
      DosError(FERR_DISABLEHARDERR);
      ret = DosMove(oldname, newname);	// move it
      if (ret && *dir) {		// failed -- clean up
	DosError(FERR_DISABLEHARDERR);
	if (!DosMove(dir, newname))
	  Broadcast((HAB) 0, hwndMain, UM_UPDATERECORD, MPFROMP(dir), MPVOID);
      }
      else if (!ret && *dir) {
	if (!IsFile(dir)) {
	  if (!strchr(dir, '?') && !strchr(dir, '*'))
	    wipeallf("%s\\*", dir);
	  DosError(FERR_DISABLEHARDERR);
	  if (DosDeleteDir(dir)) {
	    make_deleteable(dir, -1, TRUE);
	    DosDeleteDir(dir);
	  }
	}
        else if (IsFile(dir) > 0) {
          APIRET error;

          DosError(FERR_DISABLEHARDERR);
          error = DosForceDelete(dir);
	  if (error) {
	    make_deleteable(dir, error, FALSE);
	    DosForceDelete(dir);
	  }
	  if (zaplong) {
	    ret = ZapLongName(dir);
	  }
	  Broadcast((HAB) 0, hwndMain, UM_UPDATERECORD, MPFROMP(dir), MPVOID);
	}
      }
    }
    else {
      // Moving to different drive
      DosError(FERR_DISABLEHARDERR);
      ret = DosCopy(oldname, newname, DCPY_EXISTING);	// <=-NOTE!
      if (ret == ERROR_DISK_CHANGE) {
	DosError(FERR_ENABLEHARDERR);
	ret = DosCopy(oldname, newname, DCPY_EXISTING);
	diskchange = TRUE;
      }
      if (ret == ERROR_INVALID_NAME || ret == ERROR_FILENAME_EXCED_RANGE) {
	if (TruncName(newname, shortname)) {
	  // make 8.3 filename
	  DosError(FERR_DISABLEHARDERR);
	  ret = DosCopy(oldname, shortname, DCPY_EXISTING);
	  if (!ret) {
	    // success -- write longname ea
	    WriteLongName(shortname, newname);
	    strcpy(newname, shortname);
	    // broadcast fixup msg to windows
	    Broadcast((HAB) 0,
		      hwndMain, UM_UPDATERECORD, MPFROMP(shortname), MPVOID);
	  }
	}
      }
      else if (!ret && *longname) {

	CHAR fixname[CCHMAXPATH];

	strcpy(fixname, newname);
	p = strrchr(fixname, '\\');
	if (p) {
	  p++;
	  *p = 0;
	}
	strcat(fixname, longname);
	DosError(FERR_DISABLEHARDERR);
	DosMove(newname, fixname);
	strcpy(newname, fixname);
	if (zaplong)
	  ZapLongName(fixname);
	Broadcast((HAB) 0,
		  hwndMain, UM_UPDATERECORD, MPFROMP(fixname), MPVOID);
      }
      if (!ret) {
	// double-check success
	DosError(FERR_DISABLEHARDERR);
	rc = DosQueryPathInfo(newname,
			      FIL_STANDARDL, &st2, sizeof(FILESTATUS3L));
	if (rc == ERROR_DISK_CHANGE) {
	  DosError(FERR_ENABLEHARDERR);
	  rc = DosQueryPathInfo(newname,
				FIL_STANDARDL, &st2, sizeof(FILESTATUS3L));
	}
	if (!rc && st2.cbFile == st.cbFile) {
	  // seems to have worked...
	  DosError(FERR_DISABLEHARDERR);
	  if (diskchange) {
	    DosError(FERR_ENABLEHARDERR);
	    DosQueryPathInfo(oldname, FIL_STANDARDL, &dummy, sizeof(FILESTATUS3L));	// force disk change
	  }
	  if (!(st2.attrFile & FILE_DIRECTORY))
	    unlinkf(oldname);		// erase file
	  else {
	    // remove directory
	    wipeallf("%s\\*", oldname);
	    DosError(FERR_DISABLEHARDERR);
	    if (DosDeleteDir(oldname)) {
	      make_deleteable(oldname, -1, TRUE);
	      DosDeleteDir(oldname);
	    }
	  }
	}
      }
    }
    return ret;

  case COPY:
    DosError(FERR_DISABLEHARDERR);
    ret = DosCopy(oldname, newname, DCPY_EXISTING);	// <=-NOTE!
    if (ret == ERROR_DISK_CHANGE) {
      DosError(FERR_ENABLEHARDERR);
      ret = DosCopy(oldname, newname, DCPY_EXISTING);
      diskchange = TRUE;
    }
    if (ret == ERROR_INVALID_NAME || ret == ERROR_FILENAME_EXCED_RANGE) {
      if (TruncName(newname, shortname)) {
	DosError((diskchange) ? FERR_ENABLEHARDERR : FERR_DISABLEHARDERR);
	ret = DosCopy(oldname, shortname, DCPY_EXISTING);
	if (!ret) {
	  WriteLongName(shortname, newname);
	  strcpy(newname, shortname);
	  Broadcast((HAB) 0,
		    hwndMain, UM_UPDATERECORD, MPFROMP(shortname), MPVOID);
	}
      }
    }
    else if (!ret && *longname) {

      CHAR fixname[CCHMAXPATH];

      strcpy(fixname, newname);
      p = strrchr(fixname, '\\');
      if (p) {
	p++;
	*p = 0;
      }
      strcat(fixname, longname);
      DosError(FERR_DISABLEHARDERR);
      DosMove(newname, fixname);
      if (zaplong)
	ZapLongName(fixname);
      Broadcast((HAB) 0, hwndMain, UM_UPDATERECORD, MPFROMP(fixname), MPVOID);
    }
    return ret;

  default:
    // shouldn't happen
    Runtime_Error(pszSrcFile, __LINE__, "bad case %u", type);
    break;
  } // switch type
  Runtime_Error(pszSrcFile, __LINE__, "copy/move type %u unexpected", type);
  return (APIRET)-3;			// bad copy/move type
}

INT make_deleteable(CHAR * filename, INT error, BOOL Dontcheckreadonly)
{
  APIRET rc;
  INT ret = -1;
  INT retrn;
  FILESTATUS3 fsi;

  //DbgMsg(pszSrcFile, __LINE__, "error %i ", error);
  DosError(FERR_DISABLEHARDERR);
  rc = DosQueryPathInfo(filename, FIL_STANDARD, &fsi, sizeof(fsi));
  if (!rc) {
    if (fsi.attrFile & 0x00000001) {
      if (fWarnReadOnly && !Dontcheckreadonly) {
        retrn = saymsg2(NULL, 0,
                       HWND_DESKTOP,
                       GetPString(IDS_READONLYFILEWARNINGTITLE),
                       GetPString(IDS_READONLYFILEWARNING),
                       filename);
        if (retrn == 3)
          ret = 3;
        else if (retrn == 4)
          ret = 2;
        else {
          fsi.attrFile = 0;
          DosError(FERR_DISABLEHARDERR);
          if (!xDosSetPathInfo(filename, FIL_STANDARD, &fsi, sizeof(fsi), 0))
            if (retrn == 1)
              ret = 0;
            else
              ret = 1;
        }
      }
      else
        fsi.attrFile = 0;
        DosError(FERR_DISABLEHARDERR);
        if (!xDosSetPathInfo(filename, FIL_STANDARD, &fsi, sizeof(fsi), 0))
          ret = 0;
    }
  }
  if (error ==  ERROR_SHARING_VIOLATION && fUnlock) {
    retrn = saymsg(MB_YESNO | MB_DEFBUTTON2,
                 HWND_DESKTOP,
                 GetPString(IDS_LOCKEDFILEWARNINGTITLE),
                 GetPString(IDS_LOCKEDFILEWARNING),
                 filename);
    if (retrn == MBID_YES) {
      runemf2(SEPARATE | INVISIBLE | BACKGROUND | WAIT,
              HWND_DESKTOP, pszSrcFile, __LINE__,
              NULL, NULL, "%s %s", PCSZ_UNLOCKEXE, filename);
    }
  }

  return ret;
}

/**
 * unlink everything from directory on down...
 */

INT wipeallf(CHAR *string, ...)
{
  FILEFINDBUF3 *f;
  HDIR search_handle;
  ULONG num_matches;
  CHAR *p, *ss, *str;
  CHAR s[CCHMAXPATH], mask[257];
  va_list ap;
  INT rc;
  static BOOL ignorereadonly = FALSE;

  va_start(ap, string);
  vsprintf(s, string, ap);
  va_end(ap);

  if (!*s)
    return -1;
  p = s;
  while ((p = strchr(p, '/')) != NULL) {
    *p = '\\';
    p++;
  }

  str = xstrdup(s, pszSrcFile, __LINE__);
  if (!str)
    return -1;

  {
    // safety net -- disallow deleting a root dir or partial name
    CHAR temp;
    p = strrchr(str, '\\');
    if (p) {
      p++;
      temp = *p;
      *p = 0;
      if (IsRoot(str) || !IsFullName(str)) {
	// under no circumstances!
	Runtime_Error(pszSrcFile, __LINE__, "bad name %s", str);
	free(str);
	return -1;
      }
      *p = temp;
    }
  }

  p = s;
  p = strrchr(s, '\\');			// strip s to just path
  if (!p)
    p = strrchr(s, ':');
  if (p) {
    p++;
    strncpy(mask, p, 256);
    mask[256] = 0;
    *p = 0;
  }
  else {
    *mask = 0;
    *s = 0;
  }

  ss = xmalloc(CCHMAXPATH, pszSrcFile, __LINE__);
  f = xmalloc(sizeof(FILEFINDBUF3), pszSrcFile, __LINE__);
  if (!ss || !f) {
    xfree(ss, pszSrcFile, __LINE__);
    xfree(f, pszSrcFile, __LINE__);
    free(str);
    return -1;
  }

  search_handle = HDIR_CREATE;
  num_matches = 1;

  DosError(FERR_DISABLEHARDERR);
  if (!DosFindFirst(str, &search_handle, FILE_NORMAL | FILE_DIRECTORY |
		     FILE_SYSTEM | FILE_READONLY | FILE_HIDDEN | FILE_ARCHIVED,
		     f, sizeof(FILEFINDBUF3), &num_matches, FIL_STANDARD)) {

    strcpy(ss, s);
    p = &ss[strlen(ss)];

    do {
      strcpy(p, f->achName);
      if (f->attrFile & FILE_DIRECTORY) {
	if (strcmp(f->achName, ".") && strcmp(f->achName, "..")) {
	  wipeallf("%s/%s", ss, mask);	// recurse to wipe files
	  DosError(FERR_DISABLEHARDERR);
	  // remove directory
	  if (DosDeleteDir(ss)) {
	    make_deleteable(ss, -1, TRUE);	// Try harder
	    DosError(FERR_DISABLEHARDERR);
	    DosDeleteDir(ss);
	  }
	}
      }
      else {
        APIRET error;

        DosError(FERR_DISABLEHARDERR);
        error = DosForceDelete(ss);
        if (error) {
          INT retrn = 0;

          retrn = make_deleteable(ss, error, ignorereadonly);
          if (retrn == 3)
            continue;
          else if (retrn == 1)
            ignorereadonly = TRUE;
	  DosError(FERR_DISABLEHARDERR);
	  rc = (INT) DosForceDelete(ss);
	  if (rc)
	    return rc;
	}
      }
      num_matches = 1;
      DosError(FERR_DISABLEHARDERR);
    } while (!DosFindNext(search_handle, f, sizeof(FILEFINDBUF3),
			  &num_matches));
    DosFindClose(search_handle);
  }

  free(f);
  free(ss);
  free(str);
  ignorereadonly = FALSE;
  return 0;
}

#if 0 // JBS 11 Sep 08 fixme to be gone
INT unlink_allf(CHAR * string, ...)
{
  // wildcard delete
  FILEFINDBUF3 *f;
  HDIR search_handle;
  ULONG num_matches;
  CHAR *p, *ss, *str;
  CHAR s[CCHMAXPATH];
  va_list ap;
  va_start(ap, string);
  vsprintf(s, string, ap);
  va_end(ap);

  if (!*s)
    return -1;
  p = s;
  while ((p = strchr(p, '/')) != NULL) {
    *p = '\\';
    p++;
  }

  str = xstrdup(s, pszSrcFile, __LINE__);
  if (!str)
    return -1;

  p = s;
  p = strrchr(s, '\\');			// strip s to just path
  if (!p)
    p = strrchr(s, ':');
  if (p) {
    p++;
    *p = 0;
  }
  else
    *s = 0;

  ss = xmalloc(CCHMAXPATH, pszSrcFile, __LINE__);
  f = xmalloc(sizeof(FILEFINDBUF3), pszSrcFile, __LINE__);
  if (!ss || !f) {
    xfree(ss, pszSrcFile, __LINE__);
    xfree(f, pszSrcFile, __LINE__);
    free(str);
    return -1;
  }

  search_handle = HDIR_CREATE;
  num_matches = 1;

  DosError(FERR_DISABLEHARDERR);
  if (!DosFindFirst(str, &search_handle, FILE_NORMAL, f,
		    sizeof(FILEFINDBUF3), &num_matches, FIL_STANDARD)) {

    strcpy(ss, s);
    p = &ss[strlen(ss)];

    do {
      strcpy(p, f->achName);
      unlinkf("%s", ss);
      num_matches = 1;
      DosError(FERR_DISABLEHARDERR);
    } while (!DosFindNext(search_handle, f, sizeof(FILEFINDBUF3),
			  &num_matches));
    DosFindClose(search_handle);
  }

  free(f);
  free(ss);
  free(str);
  return 0;
}
#endif

/**
 * Delete file
 * @return OS/2 API error code or 0 if OK
 */

INT unlinkf(CHAR *string)
{
  if (!strstr(string, ArcTempRoot)) {
    DosError(FERR_DISABLEHARDERR);
    if (DosDelete(string)) {
      make_deleteable(string, -1, TRUE);
      DosError(FERR_DISABLEHARDERR);
      return DosDelete(string);
    }
  }
  else {
    APIRET error;

    DosError(FERR_DISABLEHARDERR);
    error = DosForceDelete(string);
    if (error) {
      make_deleteable(string, error, FALSE);
      DosError(FERR_DISABLEHARDERR);
      return DosForceDelete(string);
    }
  }
  return 0;
}

#pragma alloc_text(LONGNAMES,TruncName,GetLongName,WriteLongName)
#pragma alloc_text(LONGNAMES,ZapLongName,AdjustWildcardName)
#pragma alloc_text(COPYF,default_disk,docopyf,MakeTempName)
#pragma alloc_text(UNLINKF,unlinkf,unlink_allf,make_deleteable,wipeallf)
