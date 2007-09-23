
/***********************************************************************

  $Id$

  Path search functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2007 Steven H. Levine

  22 Apr 07 GKY Add RunFM2Util to find and run apps from the FM2Utilities
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  23 Aug 07 SHL Comments

***********************************************************************/
#define INCL_WIN
#define INCL_WINERRORS
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_LONGLONG

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

static PSZ pszSrcFile = __FILE__;

#pragma data_seg(DATA1)

//== RunFM2Util() Find and run an app from the FM2utilities ==
//== Search PATH plus 2 default install dirs ==

INT RunFM2Util(CHAR *appname, CHAR *filename)
{
    CHAR fbuf[CCHMAXPATH];
    APIRET rc, ret = -1;

    rc = DosSearchPath(SEARCH_IGNORENETERRS |SEARCH_ENVIRONMENT |
		       SEARCH_CUR_DIRECTORY,"PATH",
		       appname, fbuf, CCHMAXPATH - 1);
      if (rc != 0) {
	if (rc != 2){
	Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		  "DosSearchPath", appname);
	return ret;
	}
	else {
	rc = DosSearchPath(0, "UTILS;..\\FM2Utils",
			   appname, fbuf, CCHMAXPATH - 1);
	    if (rc != 0 && rc != 2){
	      Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
			"DosSearchPath", appname);
	      return ret;
	    }
      }
    }
    ret = runemf2(SEPARATE | WINDOWED,
                  HWND_DESKTOP,
                  NULL,
                  NULL,
                  "%s \"%s\"",
		  fbuf, filename);
    return ret;
}

CHAR *first_path(CHAR * path, CHAR * ret)
{

  CHAR *p, *pp;

  if (!path || !ret)
    return ret;
  strcpy(ret, path);
  p = strchr(ret, ';');
  if (p) {
    *p = 0;
    p++;
    if (*ret == '.') {			/* skip initial "cur dir" */
      pp = strchr(p, ';');
      if (pp)
	*pp = 0;
      if (*p)
	memmove(ret, p, strlen(p) + 1);
    }
  }
  return ret;
}

/**
 * Search for file in name PATH env variable
 * 23 Aug 07 SHL fixme to be MT safe
 */

CHAR *searchapath(CHAR *pathvar, CHAR *filename)
{
  static CHAR fbuf[CCHMAXPATH];

  if (strchr(filename, '\\') || strchr(filename, '/')
      || strchr(filename, ':')) {

    FILESTATUS3L fsa;

    if (!DosQueryPathInfo(filename, FIL_STANDARDL, &fsa, (ULONG) sizeof(fsa)))
      return filename;
    *fbuf = 0;
    return fbuf;
  }
  *fbuf = 0;
  if (DosSearchPath(SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT |
		    SEARCH_CUR_DIRECTORY,
		    pathvar, filename, fbuf, CCHMAXPATH - 1))
    *fbuf = 0;
  return fbuf;
}

CHAR *searchpath(CHAR * filename)
{
  CHAR *found;

  if (!filename)
    return "";
  found = searchapath("PATH", filename);
  if (!*found) {
    found = searchapath("DPATH", filename);
    if (!*found)
      found = searchapath("XPATH", filename);
  }
  return found;
}

#pragma alloc_text(MISC9,first_path,searchapath,searchpath,RunFM2Util)
