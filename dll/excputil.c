/***********************************************************

  $Id$

  exception handlers

  Copyright (c) 2008 Steven H. Levine

  Write to exceptq .trp file or ?:\fm2_trap.log
  where ? is boot volume

  06 Dec 08 SHL Baseline (Ticket #26)

************************************************************/

#include <stdio.h>			// fprintf
#include <string.h>			// strcpy
#include <time.h>			// tm
#include <process.h>			// _beginthread

#define INCL_DOSMODULEMGR		// DosQueryModFromEIP
#define INCL_DOSPROCESS			// PPIB PTIB
#define INCL_DOSEXCEPTIONS		// XCTP_...
#define INCL_DOSMISC			// DosDumpProcess?
#define INCL_DOSERRORS			// NO_ERROR
// #include <os2.h>

#include "wrappers.h"			// xmalloc xfree
#include "errutil.h"			// Dos_Error Runtime_Error
#include "fm3str.h"			// IDS_COULDNTSTARTTHREADTEXT
#include "strutil.h"			// GetPString

#include "excputil.h"
#include "fortify.h"

static PSZ pszSrcFile = __FILE__;

typedef struct {
  VOID (*pfnThread)(PVOID);
  PVOID *pvArgs;
} THREADDATA;

/**
 * Wrapper thread that installs exception handler and invokes
 * function passed to xbeginthread
 */

static VOID WrapperThread(PVOID pvArgs)
{
  EXCEPTIONREGISTRATIONRECORD regRec = { NULL, NULL };
  APIRET apiret;
  THREADDATA *ptd = (THREADDATA*)pvArgs;

# ifdef FORTIFY
  Fortify_EnterScope();
  Fortify_BecomeOwner(pvArgs);
# endif

  regRec.ExceptionHandler = HandleException;
  apiret = DosSetExceptionHandler(&regRec);
  if (apiret != NO_ERROR) {
    Dos_Error(MB_ENTER, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
	      "DosSetExceptionHandler");
  }

#if 0 // 11 Dec 08 SHL fixme tobe gone - debug
  {
    static UINT when;
    if (++when == 2)
      *(char*)0 = 0;
  }
#endif

  (*ptd->pfnThread)(ptd->pvArgs);	// Invoke thread

  xfree(pvArgs, pszSrcFile, __LINE__);

  if (apiret == NO_ERROR) {
    apiret = DosUnsetExceptionHandler(&regRec);
    if (apiret != NO_ERROR) {
      Dos_Error(MB_ENTER, apiret, HWND_DESKTOP, pszSrcFile, __LINE__,
		"DosUnsetExceptionHandler");
    }
  }

# ifdef FORTIFY
  Fortify_LeaveScope();
# endif

  // Time to disappear

}

/**
 * _beginthread wrapper which supplies exception handler support
 */

int xbeginthread(VOID (*pfnThread)(PVOID),
		 UINT cStackBytes,
		 PVOID pvArgs,
		 PSZ pszSrcFile,
		 UINT uiLineNumber)
{
  int rc;
  THREADDATA *ptd = (THREADDATA *)xmalloc(sizeof(THREADDATA), pszSrcFile, __LINE__);
  ptd->pfnThread = pfnThread;
  ptd->pvArgs = pvArgs;
  rc = _beginthread(WrapperThread, NULL, cStackBytes, ptd);
  if (rc == -1)
    Runtime_Error(pszSrcFile, uiLineNumber,
		  GetPString(IDS_COULDNTSTARTTHREADTEXT));
  return rc;
}

/**
 * Handle exception
 * Calls exceptq if available or handles internally
 */

ULONG HandleException(PEXCEPTIONREPORTRECORD pReport,
		      PEXCEPTIONREGISTRATIONRECORD pReg,
		      PCONTEXTRECORD pContext,
		      PVOID pv)
{
  BOOL handled = FALSE;
  ULONG ex = pReport->ExceptionNum;
  time_t now;
  struct tm *ptm;
  char *pszTime;
  PIB *ppib;
  TIB *ptib;
  ULONG ulpid;
  ULONG ultid;
  APIRET apiret;
  HMODULE hmod;

  static unsigned working;
  static PSZ pszExcpMsg = "Caught exception %lx in process %lx (%lu) thread %u at %.24s";

  // Try to report cascading exceptions in handler only once
  // This simple test can get confused if multiple threads trap at same time
  if (++working > 1) {
    if (working == 2) {
      DbgMsg(pszSrcFile, __LINE__, "Caught exception %lx at %p while handler active",
	     ex, pContext->ctx_RegEip);
    }
    working--;
    return XCPT_CONTINUE_SEARCH;
  }

  // Bypass exceptions we don't handle or exceptions we ignore
  // Keep in sync with exceptq selections since exceptq will ignore anyway
  if ((pReport->fHandlerFlags & (EH_UNWINDING | EH_NESTED_CALL)) ||
      (ex & XCPT_SEVERITY_CODE) != XCPT_FATAL_EXCEPTION ||
      ex == XCPT_ASYNC_PROCESS_TERMINATE ||
      ex == XCPT_PROCESS_TERMINATE ||
      ex == XCPT_UNWIND ||
      ex == XCPT_SIGNAL ||
      ex == XCPT_BREAKPOINT ||
      ex == XCPT_SINGLE_STEP)
  {
    working--;
    return XCPT_CONTINUE_SEARCH;
  }

  now = time(NULL);
  ptm = localtime(&now);
  pszTime = asctime(ptm);

  apiret = DosGetInfoBlocks(&ptib, &ppib);
  if (apiret) {
    ulpid = 0;
    ultid = 0;
  }
  else {
    ulpid = ppib->pib_ulpid;
    ultid = ptib->tib_ptib2->tib2_ultid;
  }

  DbgMsg(pszSrcFile, __LINE__, pszExcpMsg,
	 ex, ulpid, ulpid, ultid, pszTime);

  // Report errors with DbgMsg, Dos_Error unsafe here
  apiret = DosLoadModule (0, 0, "exceptq" ,&hmod);
  if (apiret) {
    if (apiret != ERROR_FILE_NOT_FOUND)
      DbgMsg(pszSrcFile, __LINE__, "DosLoadModule(exceptq) reported error %u", apiret);
  }
  else {
    ERR pfn;
    apiret = DosQueryProcAddr(hmod, 0, "MYHANDLER", (PFN*)&pfn);
    if (apiret)
      DbgMsg(pszSrcFile, __LINE__, "DosQueryProcAddr(MYHANDLER) reported error %u", apiret);
    else {
      // DbgMsg(pszSrcFile, __LINE__, "Invoking exceptq handler at %p", pfn);
      (*pfn)(pReport, pReg, pContext, pv);
      handled = TRUE;
    }
    DosFreeModule(hmod);
  }

  // If exceptq not available use local handler
  if (!handled) {
    union {
      struct {
	ULONG ulEBP;
	ULONG ulEIP;
      } stk32;
      struct {
	USHORT usBP;
	USHORT usIP;
	USHORT usCS;			// > 1 and < 0xfff
      } stk16;
    } u;
    ULONG ulObjNum;
    ULONG ulOffset;
    ULONG ulEIP;
    ULONG ulEBP;
    CHAR szFileName[CCHMAXPATH];
    BOOL is32Bit;
    APIRET apiret;
    ULONG flags;
    ULONG cnt;
    ULONG ulOldEBP = 0;
    INT c;
    FILE* fp;
    CHAR *modea = "a";

    handled = TRUE;

    // Write stack trace to log file - let kernel do popuplog.os2
    fp = xfopen("fm2_trap.log", modea, pszSrcFile, __LINE__, TRUE);
    if (!fp)
      fp = stderr;			// Oh well

    if (fp != stderr) {
      fputc('\n', fp);
      fprintf(fp, pszExcpMsg,
	      ex, ulpid, ulpid,	ultid, pszTime);
      fputc('\n', stderr);
    }
    fputc('\n', stderr);

    // fixme to support mixed 32-bit/16-bit stacks, 5b = FLAT_CS
    if (pContext->ctx_SegCs == 0x5b) {
      is32Bit = TRUE;			// Assume 32-bit
      u.stk32.ulEIP = pContext->ctx_RegEip;
      u.stk32.ulEBP = pContext->ctx_RegEbp;
    }
    else {
      is32Bit = FALSE;
      u.stk16.usIP = (SHORT) pContext->ctx_RegEip;
      u.stk16.usBP = (SHORT) pContext->ctx_RegEbp;
    }

    // Walk stack frames
    for (c = 0; c < 100; c++) {
      if (is32Bit) {
	ulEIP = u.stk32.ulEIP;
	ulEBP = u.stk32.ulEBP;
      }
      else {
	ulEIP = ((ULONG) (pContext->ctx_SegCs & ~7) << 13) | u.stk16.usIP;
	ulEBP = ((ULONG) (pContext->ctx_SegSs & ~7) << 13) | u.stk16.usBP;
      }

      apiret = DosQueryModFromEIP(&hmod, &ulObjNum, CCHMAXPATH, szFileName,
			      &ulOffset, ulEIP);
      if (apiret) {
	ulObjNum = 0;
	ulOffset = 0;
	strcpy(szFileName, "n/a");
      }
      else
	ulObjNum++;			// Number from 1..n for display

      fprintf(fp, " Stack frame %u @ %lx:%lx %s %lx:%lx\n",
	      c,
	      pContext->ctx_SegCs, ulEIP,
	      szFileName, ulObjNum, ulOffset);

      if (apiret)
	break;

      if (!ulEBP || ulEBP <= ulOldEBP)
	break;

      cnt = sizeof(u);

      apiret = DosQueryMem((void*)ulEBP, &cnt, &flags);
      if (apiret || (flags & (PAG_COMMIT | PAG_READ)) != (PAG_COMMIT | PAG_READ))
	break;			// We are lost

      ulOldEBP = ulEBP;
      memcpy((void*)&u, (void*)ulEBP, sizeof(u));

    } // for

    if (fp || fp != stderr)
      fclose(fp);

  } // if !handled

  // DbgMsg(pszSrcFile, __LINE__, "We are going to die now");

  working--;

  return XCPT_CONTINUE_SEARCH;		// Let other handlers see exception

}
