#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma data_seg(DATA1)
#pragma alloc_text(MISC9,first_path,searchapath,searchpath)

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

CHAR *searchapath(CHAR * path, CHAR * filename)
{

  static CHAR fbuf[CCHMAXPATH];

  if (strchr(filename, '\\') || strchr(filename, '/')
      || strchr(filename, ':')) {

    FILESTATUS3 fsa;

    if (!DosQueryPathInfo(filename, FIL_STANDARD, &fsa, (ULONG) sizeof(fsa)))
      return filename;
    *fbuf = 0;
    return fbuf;
  }
  *fbuf = 0;
  if (DosSearchPath(SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT |
		    SEARCH_CUR_DIRECTORY,
		    path, filename, fbuf, CCHMAXPATH - 1))
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
