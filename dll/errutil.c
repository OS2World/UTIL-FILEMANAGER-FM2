
/***********************************************************************

  $Id$

  Error reporting

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2015 Steven H. Levine

  12 Aug 04 SHL Comments
  23 May 05 SHL Move saymsg here
  24 May 05 SHL Rename General_Error to more accurate Win_Error
  24 May 05 SHL Rename saymsg to more accurate Misc_Error
  24 May 05 SHL Rework Win_Error args and clean up logic
  27 May 05 SHL Rework to use common showMsg
  14 Aug 05 SHL showMsg: suppress write to stdout if not error message
  13 Jul 06 SHL Add Runtime_Error
  22 Jul 06 SHL Optimize calling sequences
  26 Jul 06 SHL Add ..._Error2
  16 Aug 06 SHL Tweak message formatting
  07 Jan 07 GKY Move error strings etc. to string file
  18 Apr 07 SHL showMsg: correct selective logging checks
  19 Apr 07 SHL Add DbgMsg
  20 Apr 07 SHL Correct IDS_GENERR1TEXT formatting
  23 Apr 07 SHL Add Win_Error_NoMsgBox.  Rework others
  14 Aug 07 SHL Add GetMSecTimer
  14 Aug 07 SHL Use GetMSecTimer in DbgMsg
  05 Jan 08 SHL Renamed from error.c to match errutil.h
  18 Dec 08 SHL Show thread id in DbgMsg
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  07 Feb 09 GKY Eliminate Win_Error2 by moving function names to PCSZs used in Win_Error
  07 Feb 09 GKY Allow user to turn off alert and/or error beeps in settings notebook.
  08 Mar 09 GKY Remove Dos_Error2 (unused) and Runtime_Error2 (no advantage over using Runtime_Error)
  23 Jul 09 GKY Add low mem buffers for the msg file name so DosGetMessage
		works in HIMEM builds
  01 Dec 10 SHL Dos_Error - remap API errors code that with odd oso001*.msg messages
  10 Mar 13 GKY Improvrd readonly check on delete to allow cancel and don't ask again options
		Added saymsg2 for this purpose
  07 Nov 13 SHL Update comments
  15 Feb 14 GKY Improvements to saymsg2 some code cleanup
  16 Feb 14 GKY Rework readonly check on delete code so it actually works in a logical way
                and so it works with move to trashcan inabled.
  09 Nov 13 SHL Use GetTidForThread in DbgMsg and tweak for for editors that understand file:line

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSERRORS
#define INCL_DOSPROCESS			// PPIB PTIB
#define INCL_LONGLONG

#include "errutil.h"
#include "strutil.h"			// GetPString
#include "fm3str.h"
#include "notebook.h"			// fErrorBeepOff
#include "init.h"			// Data declares
#include "misc.h"			// GetTidForThread
#include "wrappers.h"			// xmallocz
#include "fm3dll2.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static VOID formatWinError(PSZ pszBuf, UINT cBufBytes, HWND hwndErr, HWND hwndOwner,
			   PCSZ pszSrcFile, UINT uSrcLineNo,
			   PCSZ pszFmt, va_list pva);

static APIRET showMsg(ULONG mb_type, HWND hwnd, PCSZ pszTitle, PCSZ pszMsg, BOOL wantLog);

/**
 * Format debug message and output to stderr
 * @note: Local errors also written to stderr
 */

VOID DbgMsg(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...)
{
  ULONG ultid;
  va_list va;

#if 1 // fixme to be selectable

  static ULONG ul1stMSec;
  // static ULONG ulLastMSec;

  ULONG msec = GetMSecTimer();
  ULONG delta;

  if (!ul1stMSec) {
    ul1stMSec = msec;
    // ulLastMSec = msec;		// Avoid big delta 1st time
  }

  delta = msec - ul1stMSec;
  // ulLastMSec = msec;
  fprintf(stderr, "%03lu.%03lu ", delta / 1000, delta % 1000);

#endif

  ultid = GetTidForThread();

  // OK for source file name to be null
  // 2015-08-08 SHL Use file:line for editors that support it
  fprintf(stderr, "%s:%u (%lu)", pszSrcFile ? pszSrcFile : "n/a", uSrcLineNo, ultid);
  // If format null want just file and line
  if (pszFmt) {
    fputc(' ', stderr);
    va_start(va, pszFmt);
    vfprintf(stderr, pszFmt, va);
    va_end(va);
  }
  fputc('\n', stderr);
  fflush(stderr);

} // DbgMsg

/**
 * Format Dos...() error and output using showMsg
 * @note: Local errors written directly to stderr
 */

// 2010-12-01 SHL fixme for ULONG to be APIRET

INT Dos_Error(ULONG mb_type, ULONG apiret, HWND hwndOwner,
	      PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...)
{
  CHAR szMsg[4096];
  CHAR szMsgFile[20], szMsgFileH[20];
  ULONG Class;				// Error class
  ULONG action;				// Error action
  ULONG Locus;				// Error location
  ULONG ulMsgLen;
  APIRET mapped_apiret;
  CHAR *pszMsgStart;
  CHAR *psz;
  va_list va;

  if (!apiret)
    return MBID_ENTER;			// Should not have been called

  // Format caller's message
  va_start(va, pszFmt);
  szMsg[sizeof(szMsg) - 1] = 0;
  vsprintf(szMsg, pszFmt, va);
  va_end(va);

  if (szMsg[sizeof(szMsg) - 1]) {
    fprintf(stderr, "Buffer overflow in Dos_Error - need %u bytes\n", strlen(szMsg) + 1);
    fflush(stderr);
  }

  if (strchr(szMsg, ' ') == NULL) {
    strcat(szMsg, " ");
    strcat(szMsg, GetPString(IDS_FAILEDTEXT));	// Assume simple function name
  }

  DosErrClass(apiret, &Class, &action, &Locus);

  sprintf(szMsg + strlen(szMsg),
	  GetPString(IDS_DOSERR1TEXT),
	  pszSrcFile,
	  uSrcLineNo,
	  apiret,
	  GetPString(IDS_ERRCLASS1TEXT + (Class - 1)),
	  GetPString(IDS_ERRACTION1TEXT + (action - 1)),
	  GetPString(IDS_ERRLOCUS1TEXT + (Locus - 1)));
  pszMsgStart = szMsg + strlen(szMsg) + 1;
  strcpy(szMsgFile, "OSO001.MSG");
  strcpy(szMsgFileH, "OSO001H.MSG");
  // Get message leaving space for NL separator
  // 2010-12-01 SHL Handle cases where message file message does not make sense relative to API error
  switch (apiret) {
  case ERROR_TIMEOUT:
    mapped_apiret = ERROR_SEM_TIMEOUT;	// Assume semaphore timeout
    break;
  default:
    mapped_apiret = apiret;
  }
  if (!DosGetMessage(NULL, 0L, (PCHAR) pszMsgStart + 1, 1024, mapped_apiret, szMsgFile, &ulMsgLen)
      || !DosGetMessage(NULL, 0L, (PCHAR) pszMsgStart + 1, 1024, mapped_apiret,
			szMsgFileH, &ulMsgLen)) {
    // Got message
    pszMsgStart[ulMsgLen + 1] = 0;	// Terminate
    *(pszMsgStart - 1) = '\n';		// Stuff NL before message text
    *pszMsgStart = '\"';		// Prefix message text with quote

    psz = pszMsgStart + ulMsgLen;	// Point at last char
    // Chop trailing NL CR TAB
    while (*psz &&
	   (*psz == '\r' || *psz == '\n' || *psz == ' ' || *psz == '\t')) {
      *psz-- = 0;
    }
    strcat(psz, "\"");			// Append trailing quote

    // Convert CR and NL combos to single space
    psz = pszMsgStart;
    while (*psz) {
      if (*psz == '\n' || *psz == '\r') {
	while (*(psz + 1) == '\n' || *(psz + 1) == '\r')
	  memmove(psz, psz + 1, strlen(psz));
	*psz = ' ';
      }
      else
	psz++;
    }
  }

  return showMsg(mb_type | MB_ICONEXCLAMATION, hwndOwner, GetPString(IDS_DOSERR2TEXT),
		 szMsg, TRUE);

} // Dos_Error

/**
 * Format last PM error into passed buffer
 */

static VOID formatWinError(PSZ pszBuf, UINT cBufBytes,
			   HWND hwndErr, HWND hwndOwner,
			   PCSZ pszSrcFile, UINT uSrcLineNo,
			   PCSZ pszFmt, va_list pva)
{
  PERRINFO pErrInfoBlk;	    // Pointer to ERRINFO structure filled by WinGetErrorInfo
  PSZ pszOffset;	    // Pointer to current error message returned by WinGetErrorInfo
  PSZ psz;
  HAB hab;

  if (hwndErr == NULLHANDLE)
    hab = (HAB)0;
  else
    hab = WinQueryAnchorBlock(hwndErr);

  // Format callers message
  pszBuf[cBufBytes - 1] = 0;
  vsprintf(pszBuf, pszFmt, pva);

  if (pszBuf[cBufBytes - 1]) {
    fprintf(stderr, "Buffer overflow in formatWinError - need %u bytes\n",
            strlen(pszBuf) + 1);
    fflush(stderr);
  }

  if (strchr(pszBuf, ' ') == NULL) {
    strcat(pszBuf, " ");
    strcat(pszBuf, GetPString(IDS_FAILEDTEXT));	// Assume simple function name
  }

  // Append file name and line number and trailing space
  sprintf(pszBuf + strlen(pszBuf),
	  GetPString(IDS_GENERR1TEXT), pszSrcFile, uSrcLineNo);

  // Get last PM error for the current thread
  pErrInfoBlk = WinGetErrorInfo(hab);
  if (!pErrInfoBlk) {
    ERRORID id = WinGetLastError(hab);
    psz = pszBuf + strlen(pszBuf);
    sprintf(psz, " WinGetErrorInfo failed (%u)", id);
  }
  else {
    if (!hwndOwner)
      hwndOwner = HWND_DESKTOP;
    // Find message offset in array of message offsets Assume 1 message - fixme?
    pszOffset = ((PSZ) pErrInfoBlk) + pErrInfoBlk->offaoffszMsg;
    // Address error message in array of messages and append error message to source code linenumber
    psz = pszBuf + strlen(pszBuf);
    sprintf(psz, " #0x%04x \"", ERRORIDERROR(pErrInfoBlk->idError));
    psz += strlen(psz);
    strcpy(psz, ((PSZ) pErrInfoBlk) + *(PSHORT) pszOffset);
    psz += strlen(psz);
    // Chop trailing mush
    psz--;
    while (*psz == '\r' || *psz == '\n' || *psz == ' ')
      *psz-- = 0;
    if (*psz)
      psz++;
    strcpy(psz, "\"");
    WinFreeErrorInfo(pErrInfoBlk);	// Free resource segment
  }

} // formatWinError

/**
 * Return millisecond timer value
 * Resolution is milliseconds, but accuracy will be less
 * depending on systems settings
 */

ULONG GetMSecTimer(void)
{
  ULONG msec;

  APIRET rc = DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
			      &msec, sizeof(msec));
  if (rc) {
    Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      "DosQuerySysInfo");
    msec = 0;
  }
  return msec;
}

/**
 * Format runtime error message and output using showMsg
 * @note: Local errors written directly to stderr
 * @note: If pszFmt is NULL a No Data error message is returned GKY 20 Feb 09 (Replaces Runtime_Error2)
 */

VOID Runtime_Error(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...)
{
  CHAR szMsg[4096];
  va_list va;

  // Format caller's message
  if (!pszFmt)
    pszFmt = PCSZ_NODATA;
  va_start(va, pszFmt);
  szMsg[sizeof(szMsg) - 1] = 0;
  vsprintf(szMsg, pszFmt, va);
  va_end(va);

  if (szMsg[sizeof(szMsg) - 1]) {
    fprintf(stderr, "Buffer overflow in Runtime_Error - need %u bytes\n", strlen(szMsg) + 1);
    fflush(stderr);
  }

  if (strchr(szMsg, ' ') == NULL) {
    strcat(szMsg, " ");
    strcat(szMsg, GetPString(IDS_FAILEDTEXT));	// Assume simple function name
  }

  sprintf(szMsg + strlen(szMsg),
	  GetPString(IDS_GENERR1TEXT), pszSrcFile, uSrcLineNo);

  showMsg(MB_ICONEXCLAMATION, HWND_DESKTOP, GetPString(IDS_DEBUG_STRING), szMsg, TRUE);

} // Runtime_Error

/**
 * Format message and output using showMsg
 * @note: Local errors written directly to stderr
 */

APIRET saymsg(ULONG mb_type, HWND hwnd, PCSZ pszTitle, PCSZ pszFmt, ...)
{
  CHAR szMsg[4096];
  va_list va;

  va_start(va, pszFmt);
  szMsg[sizeof(szMsg) - 1] = 0;
  vsprintf(szMsg, pszFmt, va);
  va_end(va);

  if (szMsg[sizeof(szMsg) - 1]) {
    fprintf(stderr, "Buffer overflow in saymsg - need %u bytes\n", strlen(szMsg) + 1);
    fflush(stderr);
  }

  return showMsg(mb_type, hwnd, pszTitle, szMsg, FALSE);

} // saymsg

/**
 * Format message with custom buttons and output using showMsg
 * Local errors written to stderr
 */

APIRET saymsg2(PCSZ pszButtonNames, int DefaultButton, HWND hwnd,
               PCSZ pszTitle, PCSZ pszFmt, ...)
{
  ULONG i;
  APIRET rc;
  CHAR szMsg[4096];
  va_list va;
  MB2INFO *pmbInfo;
  MB2D mb2dBut[4];
  ULONG ulInfoSize = (sizeof(MB2INFO) + (sizeof(MB2D) * 3));

  va_start(va, pszFmt);
  szMsg[sizeof(szMsg) - 1] = 0;
  vsprintf(szMsg, pszFmt, va);
  va_end(va);

  if (szMsg[sizeof(szMsg) - 1]) {
    fprintf(stderr, "Buffer overflow in saymsg2 - need %u bytes\n", strlen(szMsg) + 1);
    fflush(stderr);
  }

  memset(mb2dBut, 0, sizeof(MB2D) * 4);
  strcpy(mb2dBut[0].achText,GetPString(IDS_MB2DYES));
  strcpy(mb2dBut[1].achText,GetPString(IDS_MB2DYESDONTASK));
  strcpy(mb2dBut[2].achText,GetPString(IDS_MB2DNO));
  strcpy(mb2dBut[3].achText,GetPString(IDS_MB2DCANCELOP));
  mb2dBut[0].idButton = SM2_YES;
  mb2dBut[1].idButton = SM2_DONTASK;
  mb2dBut[2].idButton = SM2_NO;
  mb2dBut[3].idButton = SM2_CANCEL;
  if (DefaultButton)
    mb2dBut[DefaultButton - 1].flStyle = BS_DEFAULT;
  pmbInfo = xmallocz(ulInfoSize, pszSrcFile, __LINE__);
  if (pmbInfo) {
    pmbInfo->cb		= ulInfoSize;
    pmbInfo->hIcon      = 0;
    pmbInfo->cButtons   = 4;
    pmbInfo->flStyle    = MB_MOVEABLE | MB_ICONQUESTION ;
    pmbInfo->hwndNotify = NULLHANDLE;
    for (i = 0; i < 4; i++) {
      memcpy( pmbInfo->mb2d+i , mb2dBut+i , sizeof(MB2D));
    }
    rc = WinMessageBox2(HWND_DESKTOP, hwnd,
			szMsg, pszTitle, SM2_DIALOG,
                        pmbInfo);
    WinSetFocus(HWND_DESKTOP, SM2_DIALOG);
    free(pmbInfo);
    return rc;
  }
  return MBID_ERROR;
}

/**
 * Display message in popup message box
 * Optionally writes formatted message to stderr
 */

static APIRET showMsg(ULONG mb_type, HWND hwndOwner,
		      PCSZ pszTitle, PCSZ pszMsg, BOOL wantLog)
{
  if (wantLog) {
    fputs(pszMsg, stderr);
    fputc('\n', stderr);
    fputc('\n', stderr);
    fflush(stderr);
  }

  if (!hwndOwner)
    hwndOwner = HWND_DESKTOP;
  if (!fErrorBeepOff)
    DosBeep(250, 100);

  return WinMessageBox(HWND_DESKTOP,	// Parent
		       hwndOwner,
		       (PSZ) pszMsg, (PSZ) pszTitle, 0,	// help id
		       mb_type | MB_MOVEABLE);
} // showMsg

/**
 * Format Win...() error and output using showMsg
 */

VOID Win_Error(HWND hwndErr, HWND hwndOwner,
	       PCSZ pszSrcFile, UINT uSrcLineNo,
	       PCSZ pszFmt, ...)
{
  CHAR szMsg[4096];
  va_list va;

  // Format callers message
  va_start(va, pszFmt);
  formatWinError(szMsg, sizeof(szMsg), hwndErr, hwndOwner, pszSrcFile,
                 uSrcLineNo, pszFmt, va);
  va_end(va);

  showMsg(MB_ENTER | MB_ICONEXCLAMATION, hwndOwner, GetPString(IDS_GENERR2TEXT),
	  szMsg, TRUE);

} // Win_Error

/**
  * Format PM error messsage and output to stderr
  * This does the same reporting as Win_Error, but bypasses the
  * message box popup.
  * Use this version when the popup would hang PM.
  */

VOID Win_Error_NoMsgBox(HWND hwndErr, HWND hwndOwner,
			PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...)
{
  CHAR szMsg[4096];
  va_list va;

  // Format callers message
  va_start(va, pszFmt);
  formatWinError(szMsg, sizeof(szMsg), hwndErr, hwndOwner, pszSrcFile, uSrcLineNo,
                 pszFmt, va);
  va_end(va);

  fputs(szMsg, stderr);
  fputc('\n', stderr);
  fputc('\n', stderr);
  fflush(stderr);
  if (!fErrorBeepOff)
    DosBeep(250, 100);

} // Win_Error_NoMsgBox

#pragma alloc_text(ERROR,Win_Error,Dos_Error,saymsg,saymsg2,showMsg,Runtime_Error,GetMSecTimer)
