
/***********************************************************************

  $Id$

  Path search functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2010 Steven H. Levine

  22 Apr 07 GKY Add RunFM2Util to find and run apps from the FM2Utilities
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  23 Aug 07 SHL Comments
  04 Oct 08 JBS Make searchapath non-static
  17 JAN 10 GKY Changes to get working with Watcom 1.9 Beta (1/16/10). Mostly cast CHAR CONSTANT * as CHAR *.
  01 Mar 14 JBS Ticket #524: Made "searchapath" thread-safe. Function names and signatures were changed.

***********************************************************************/

#include <string.h>

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
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
		       SEARCH_CUR_DIRECTORY, (CHAR *) PCSZ_PATH,
		       (CHAR *) appname, (PBYTE)fbuf, CCHMAXPATH - 1);
      if (rc != 0) {
	if (rc != 2){
	Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		  PCSZ_DOSSEARCHPATH, appname);
	return ret;
	}
	else {
	rc = DosSearchPath(0, "UTILS;..\\FM2Utils",
			   (CHAR *) appname, (PBYTE)fbuf, CCHMAXPATH - 1);
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
    if (*ret == '.') {			// skip initial "cur dir"
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
 * SearchPathForFile: Search for a file along a path
 *
 * @param pPathname: the name of a path environment variable (input)
 *    Used only if pFilename has no directory separators or ':' for a drive sepcification
 *
 * @param pFilename: the name of a file to search for (input)
 *    If the file name includes a directory separator or the ':' for a drive specification then
 *        DosQueryPathInfo is used for the search
 *    else
 *        DosSearchPath is used, along with the pPathname parameter
 *
 * @param pFullFilename: address of where to place fully-qulified name if search succeeds (output)
 *    This parameter may be NULL if the fully-qualified filename is not desired.
 *
 * @return Return code from call to DosQueryPathInfo/DosSearchPath
 *
 */
APIRET SearchPathForFile(PCSZ pPathname,
			 PCSZ pFilename,
			 PCHAR pFullFilename)
{
  APIRET rc;
  CHAR szFullFilename[CCHMAXPATH];

  if (!pPathname || !*pPathname || !pFilename || !*pFilename)
    return ERROR_INVALID_PARAMETER;

  if (strchr(pFilename, '\\') || strchr(pFilename, '/')
      || strchr(pFilename, ':')) {
    rc = DosQueryPathInfo(pFilename, FIL_QUERYFULLNAME,
			  (PVOID)szFullFilename, (ULONG) CCHMAXPATH-1);
  }
  else {
   rc = DosSearchPath(SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT |
		      SEARCH_CUR_DIRECTORY,
		      (CHAR *) pPathname,
		      (CHAR *) pFilename,
		      (PBYTE) szFullFilename,
		      CCHMAXPATH - 1);
  }
  if (!rc && pFullFilename) {
    strcpy(pFullFilename, szFullFilename);
  }
  return rc;
}

/**
 * SearchMultiplePathsForFile: Search for a file along multiple paths.
 *   Currently these paths are hard-coded to: PATH, DPATH and XPATH.
 *
 * @param pFilename: the name of a file to search for (input)
 *
 * @param pFullFilename: address of where to place fully-qulified name if search succeeds (output)
 *    This parameter may be NULL if the fully-qualified filename is not desired.
 *
 * @return Return code from call to SearchPathForFile (DosQueryPathInfo/DosSearchPath)
 *
 * @note: The code uses DosSearchPathForFile for all searches. First it searches PATH.
 *	  If this fails it searches DPATH. If this fails it seaches XPATH.
 *
 */
APIRET SearchMultiplePathsForFile(PCSZ pFilename,
				  PSZ pFullFilename)
{
  APIRET rc;

  rc = SearchPathForFile(PCSZ_PATH,
			 pFilename,
			 pFullFilename);
  if (rc && rc != ERROR_INVALID_PARAMETER) {
    rc = SearchPathForFile("DPATH",
			   pFilename,
			   pFullFilename);
    if (rc && rc != ERROR_INVALID_PARAMETER)
      rc = SearchPathForFile("XPATH",
			     pFilename,
			     pFullFilename);
  }
  return rc;
}

//#pragma alloc_text(MISC9,first_path,searchapath,searchpath,RunFM2Util)
// jbs: first_path seems to be unused
#pragma alloc_text(MISC9,first_path,SearchPathForFile,SearchMultiplePathsForFile,RunFM2Util)
