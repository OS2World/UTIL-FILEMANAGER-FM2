
/***********************************************************************

  $Id$

  Process dump facility interface

  Copyright (c) 2008 Steven H. Levine

  06 Dec 08 SHL Baseline (Ticket 307)

***********************************************************************/

#define INCL_DOSMISC			// DosDumpProcess
#include <os2.h>

#include "errutil.h"
#include "dumputil.h"

// 06 Dec 08 SHL fixme to be in OpenWatcom bsedos.h
APIRET APIENTRY DosDumpProcess (ULONG Flag, ULONG Drive, PID Pid);

static PSZ pszSrcFile = __FILE__;

/**
 * Generate process dump if dump facility enabled
 * @note Need to import DosDumpProcess = DOSCALLS.113
 */

VOID DbgDumpProcess(VOID)
{
  APIRET apiret = DosDumpProcess(DDP_PERFORMPROCDUMP, 0, 0);
  // Use DbgMsg to report errors - Dos_Error probably unsafe here
  if (apiret) {
    DbgMsg(pszSrcFile, __LINE__, "DosDumpProcess DDP_PERFORMPROCDUMP reported %u", apiret);
  }
}

#pragma alloc_text(DUMPPROCESS, DumpProcess)
