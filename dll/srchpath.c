
/***********************************************************************

  $Id$

  Path search functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2008 Steven H. Levine

  22 Apr 07 GKY Add RunFM2Util to find and run apps from the FM2Utilities
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  23 Aug 07 SHL Comments
  04 Oct 08 JBS Make searchapath non-static

***********************************************************************/

#include <string.h>

#define INCL_WIN
#define INCL_DOS
#define INCL_LONGLONG			// dircnrs.h

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "srchpath.h"
#include "pathutil.h"			// BldQuotedFileName
#include "errutil.h"			// Dos_Error...
#include "systemf.h"			// runemf2
#include "notebook.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)

static PSZ pszSrcFile = __FILE__;

// static CHAR *first_path(CHAR * path, CHAR * ret);

#pragma data_seg(DATA1)

//== RunFM2Util() Find and run an app from the FM2utilities ==
//== Search PATH plus 2 default install dirs ==

INT RunFM2Util(PCSZ appname, CHAR *filename)
{
    CHAR fbuf[CCHMAXPATH];
    CHAR szQuotedFileName[CCHMAXPATH];
    APIRET rc, ret = -1;

    rc = DosSearchPath(SEARCH_IGNORENETERRS |SEARCH_ENVIRONMENT |
		       SEARCH_CUR_DIRECTORY, PCSZ_PATH,
		       appname, (PBYTE)fbuf, CCHMAXPATH - 1);
      if (rc != 0) {
	if (rc != 2){
	Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		  PCSZ_DOSSEARCHPATH, appname);
	return ret;
	}
	else {
	rc = DosSearchPath(0, "UTILS;..\\FM2Utils",
			   appname, (PBYTE)fbuf, CCHMAXPATH - 1);
	    if (rc != 0 && rc != 2){
	      Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
			PCSZ_DOSSEARCHPATH, appname);
	      return ret;
	    }
      }
    }
    ret = runemf2(SEPARATE | WINDOWED,
                  HWND_DESKTOP, pszSrcFile, __LINE__,
                  NULL,
                  NULL,
                  "%s %s",
		  fbuf, BldQuotedFileName(szQuotedFileName, filename));
    return ret;
}

#if 0	// JBS	11 Sep 08
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
#endif

/**
 * Search for file in name PATH env variable
 * 23 Aug 07 SHL fixme to be MT safe
 */

CHAR *searchapath(PCSZ pathvar, PCSZ filename)
{
  static CHAR fbuf[CCHMAXPATH];

  if (strchr(filename, '\\') || strchr(filename, '/')
      || strchr(filename, ':')) {

    FILESTATUS3 fsa;

    strcpy(fbuf, filename);
    if (!DosQueryPathInfo(fbuf, FIL_STANDARD, &fsa, (ULONG) sizeof(fsa)))
      return fbuf;
    *fbuf = 0;
    return fbuf;
  }
  *fbuf = 0;
  if (DosSearchPath(SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT |
		    SEARCH_CUR_DIRECTORY,
		    pathvar, filename, (PBYTE)fbuf, CCHMAXPATH - 1))
    *fbuf = 0;
  return fbuf;
}

CHAR *searchpath(PCSZ filename)
{
  CHAR *found;

  if (!filename)
    return NullStr;
  found = searchapath(PCSZ_PATH, filename);
  if (!*found) {
    found = searchapath("DPATH", filename);
    if (!*found)
      found = searchapath("XPATH", filename);
  }
  return found;
}

#pragma alloc_text(MISC9,first_path,searchapath,searchpath,RunFM2Util)
