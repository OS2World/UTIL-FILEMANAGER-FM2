
/***********************************************************************

  $Id$

  External strings file support

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006 Steven H. Levine

  22 Jul 06 SHL Comments

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <share.h>
#include <io.h>

#include "fm3dll.h"
#include "fm3str.h"
#include "version.h"

#pragma alloc_text(STRINGS,LoadStrings,GetPString)

static char **strs, *str;
static ULONG numStr;

//== LoadStrings() load strings from file ==

BOOL LoadStrings(char *filename)
{
  BOOL ok = FALSE;
  ULONG size, len, totalsize;
  USHORT vermajor = 0, verminor = 0;
  register char *p;
  register ULONG x;
  FILE *fp;
  APIRET rc;

  /* Load strings from requested file or FM3RES.STR
   * with some quiet error-checking.
   * Return TRUE on success, FALSE on error.
   */

  if (!filename)
    filename = "FM3RES.STR";
  numStr = 0;
  if (str)
    DosFreeMem(str);
  strs = NULL;
  str = NULL;

  fp = _fsopen(filename, "rb", SH_DENYWR);
  if (fp) {
    if (fread(&numStr,
	      sizeof(numStr),
	      1,
	      fp) &&
	numStr == IDS_NUMSTRS &&
	fread(&len, sizeof(len), 1, fp) &&
	fread(&vermajor, sizeof(vermajor), 1, fp) &&
	fread(&verminor, sizeof(verminor), 1, fp) &&
	(vermajor >= VERMAJORBREAK &&
	 (vermajor > VERMAJORBREAK || verminor >= VERMINORBREAK))) {
      fseek(fp, 0, SEEK_END);
      size = ftell(fp) - ((sizeof(ULONG) * 2) + (sizeof(USHORT) * 2));
      if (size && size == len) {
	fseek(fp, (sizeof(ULONG) * 2) + (sizeof(USHORT) * 2), SEEK_SET);
	/* NOTE:  Make one memory object for both str and strs
	 * for efficiency.
	 */
	totalsize = size + sizeof(ULONG);
	totalsize += (totalsize % sizeof(ULONG));
	len = totalsize;
	totalsize += (numStr * sizeof(char *));
	totalsize += 4;
	rc = DosAllocMem((PPVOID) & str, totalsize,
			 PAG_COMMIT | PAG_READ | PAG_WRITE);
	if (!rc && str) {
	  strs = (char **)(str + len);
	  if (fread(str, 1, size, fp) == size) {
	    p = str;
	    for (x = 0; x < numStr; x++) {
	      if (p - str >= size)
		break;
	      strs[x] = p;
	      while (*p)
		p++;
	      p++;
	    }
	    if (x == numStr)
	      ok = TRUE;
	  }
	  if (ok)
	    /* set pages to readonly */
	    DosSetMem(str, totalsize, PAG_COMMIT | PAG_READ);
	}
      }
    }
    fclose(fp);
  }

  if (!ok) {
    numStr = 0;
    if (str)
      DosFreeMem(str);
    str = NULL;
    strs = NULL;
  }

  return ok;
}

//== GetPString() return a readonly pointer to the requested string in memory ==

char *GetPString(ULONG id)
{
  return id < numStr && str && strs && strs[id] ? strs[id] : NullStr;
}

//== StringsLoaded() return TRUE is strings loaded

BOOL StringsLoaded(void)
{
  return numStr && str && strs;
}
