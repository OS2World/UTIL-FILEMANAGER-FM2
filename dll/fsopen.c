
/***********************************************************************

  $Id$

  _fsopen for IBMC

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2007 Steven H. Levine

  15 Oct 02 SHL Baseline
  05 Jun 07 SHL Update for OpenWatcom

***********************************************************************/

#if defined(__IBMC__)

#define INCL_WIN
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include "fm3dll.h"

#pragma alloc_text(FSOPEN,_fsopen)

FILE *_fsopen(CHAR * filename, CHAR * mode, INT sharemode, ...)
{

  ULONG openflag = OPEN_ACTION_OPEN_IF_EXISTS, openmode = 0, action = 0;
  HFILE handle;
  FILE *fp;
  BOOL text = TRUE;

  if (!stristr(mode, "b"))
    text = FALSE;
  if (stristr(mode, "r"))
    openmode |= OPEN_ACCESS_READONLY;
  else if (stristr(mode, "w")) {
    openmode |= OPEN_ACCESS_WRITEONLY;
    openflag |= (OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW);
  }
  if (stristr(mode, "a"))
    openmode |= OPEN_ACCESS_WRITEONLY;
  if (stristr(mode, "+")) {
    openmode &= (~(OPEN_ACCESS_READONLY | OPEN_ACCESS_WRITEONLY));
    openmode |= OPEN_ACCESS_READWRITE;
    openflag |= OPEN_ACTION_CREATE_IF_NEW;
  }
  if (sharemode == SH_DENYRW)
    openmode |= OPEN_SHARE_DENYREADWRITE;
  else if (sharemode == SH_DENYWR)
    openmode |= OPEN_SHARE_DENYWRITE;
  else if (sharemode == SH_DENYRD)
    openmode |= OPEN_SHARE_DENYREAD;
  else
    openmode |= OPEN_SHARE_DENYNONE;
  openmode |= OPEN_FLAGS_FAIL_ON_ERROR;
  if (text)
    openmode |= OPEN_FLAGS_SEQUENTIAL;
  else
    openmode |= OPEN_FLAGS_RANDOMSEQUENTIAL;
  if (DosOpen(filename, &handle, &action, 0L, FILE_NORMAL, openflag, openmode,
	      (PEAOP2) 0))
    return NULL;
  if (mode[strlen(mode) - 1] == 't')
    mode[strlen(mode) - 1] = 0;		/* bug bug bug */
  fp = fdopen(handle, mode);
  if (!fp) {
    DosClose(handle);
    fp = fopen(filename, mode);		/* last ditch effort */
  }
  if (fp) {
    if (text)				/* line buffer text files */
      setvbuf(fp, NULL, _IOLBF, BUFSIZ * 2);
    else
      setvbuf(fp, NULL, _IOFBF, BUFSIZ * 8);
    if (stristr(mode, "a"))
      fseek(fp, 0L, SEEK_END);
  }
  return fp;
}

#endif
