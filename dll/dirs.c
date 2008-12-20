
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2008 Steven H.Levine

  Directory manipulation

  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_LONGLONG                   // dircnrs.h

#include "fm3dll.h"
#include "init.h"			// Data declaration(s)
#include "dirs.h"
#include "valid.h"                      // IsValidDir

APIRET save_dir2(CHAR * curdir)
{
  CHAR *env = getenv("FM3INI");

  if (env && *env) {
    strncpy(curdir, env, CCHMAXPATH);
    curdir[CCHMAXPATH - 1] = 0;
    if (IsValidDir(curdir))
      return 0;
    else {
      env = strrchr(curdir, '\\');
      if (env) {
        *env = 0;
        if (IsValidDir(curdir))
          return 0;
      }
    }
  }
  return save_dir(curdir);
}

APIRET save_dir(CHAR * curdir)
{
  APIRET ret;
  ULONG curdirlen, curdrive, drivemap;

  *curdir = 0;
  ret = DosQCurDisk(&curdrive, &drivemap);
  curdirlen = CCHMAXPATH - 4;           /* NOTE!!!!!!!!! */
  ret += DosQCurDir(curdrive, (PBYTE)&curdir[3], &curdirlen);
  *curdir = (CHAR) ('@' + (INT) curdrive);
  curdir[1] = ':';
  curdir[2] = '\\';
  return ret;
}

APIRET switch_to(CHAR * s)
{

  APIRET ret;
  FILESTATUS3 fsa;
  CHAR path[CCHMAXPATH + 1], *p;

  strcpy(path, s);
  while (*path) {
    ret = DosQueryPathInfo(path, FIL_STANDARD, &fsa, sizeof(fsa));
    if (ret || !(fsa.attrFile & FILE_DIRECTORY)) {
      p = strrchr(path, '\\');
      if (p)
        *p = 0;
      else {
        strcpy(path, s);
        break;
      }
    }
    else
      break;
  }
  if (isalpha(*path) && path[1] == ':') {

    ULONG curdrive, drivemap;

    if (!DosQCurDisk(&curdrive, &drivemap)) {
      if ((CHAR) ((CHAR) curdrive + '@') != (CHAR) toupper(*HomePath) &&
          (CHAR) ((CHAR) curdrive + '@') != (CHAR) toupper(*path))
        DosChDir("\\");
    }
    ret = DosSelectDisk(toupper(*path) - '@');
    return (ret) ? ret : DosChDir(path);
  }
  return DosChDir(path);
}

#pragma alloc_text(MISC9,save_dir,save_dir2,switch_to)
