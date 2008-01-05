
/***********************************************************************

  $Id$

  Error reporting

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2007 Steven H. Levine

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

***********************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "fm3dll.h"
#include "fm3str.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static VOID formatWinError(PSZ pszBuf, UINT cBufBytes, HWND hwndErr, HWND hwndOwner,
			   PCSZ pszSrcFile, UINT uSrcLineNo,
			   PCSZ pszFmt, va_list pva);

static APIRET showMsg(ULONG mb_type, HWND hwnd, PCSZ pszTitle, PCSZ pszMsg, BOOL wantLog);

//=== DbgMsg: output debug message stderr ===

VOID DbgMsg(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...)
{
  va_list va;

#if 1 // fixme to be selectable

  static ULONG ul1stMSec;
  // static ULONG ulLastMSec;

  ULONG msec = GetMSecTimer();
  ULONG delta;

  if (!ul1stMSec) {
    ul1stMSec = msec;
    // ulLastMSec = msec;			// Avoid big delta 1st time
  }

  delta = msec - ul1stMSec;
  // ulLastMSec = msec;
  fprintf(stderr, "%03lu.%03lu ", delta / 1000, delta % 1000);

#endif

  // OK for source file name to be null
  fprintf(stderr, "%s %u", pszSrcFile ? pszSrcFile : "n/a", uSrcLineNo);
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

//== Dos_Error: report Dos...() error using passed message string ===

INT Dos_Error(ULONG mb_type, ULONG ulRC, HWND hwndOwner,
	      PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...)
{
  CHAR szMsg[4096];
  ULONG Class;				// Error class
  ULONG action;				// Error action
  ULONG Locus;				// Error location
  ULONG ulMsgLen;
  CHAR *pszMsgStart;
  CHAR *psz;
  va_list va;

  if (!ulRC)
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

  if (strchr(szMsg, ' ') == NULL)
    strcat(szMsg, " failed");		// Assume simple function name

  DosErrClass(ulRC, &Class, &action, &Locus);

  sprintf(szMsg + strlen(szMsg),
	  GetPString(IDS_DOSERR1TEXT),
	  pszSrcFile,
	  uSrcLineNo,
	  ulRC,
	  GetPString(IDS_ERRCLASS1TEXT + (Class - 1)),
	  GetPString(IDS_ERRACTION1TEXT + (action - 1)),
	  GetPString(IDS_ERRLOCUS1TEXT + (Locus - 1)));
  pszMsgStart = szMsg + strlen(szMsg) + 1;
  // Get message leaving space for NL separator
  if (!DosGetMessage
      (NULL, 0L, (PCHAR) pszMsgStart + 1, 1024, ulRC, "OSO001.MSG", &ulMsgLen)
      || !DosGetMessage(NULL, 0L, (PCHAR) pszMsgStart + 1, 1024, ulRC,
			"OSO001H.MSG", &ulMsgLen)) {
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

//== Dos_Error2: report Dos...() error using passed message id ===

INT Dos_Error2(ULONG mb_type, ULONG ulRC, HWND hwndOwner,
	       PCSZ pszSrcFile, UINT uSrcLineNo, UINT idMsg)
{
  return Dos_Error(mb_type, ulRC, hwndOwner, pszSrcFile, uSrcLineNo,
		   GetPString(idMsg));
} // Dos_Error2

/**
 * Format last PM error into passed buffer
 */

static VOID formatWinError(PSZ pszBuf, UINT cBufBytes,
			   HWND hwndErr, HWND hwndOwner,
			   PCSZ pszSrcFile, UINT uSrcLineNo,
			   PCSZ pszFmt, va_list pva)
{
  PERRINFO pErrInfoBlk;		/* Pointer to ERRINFO structure filled
				   by WinGetErrorInfo */
  PSZ pszOffset;		/* Pointer to current error message returned
				   by WinGetErrorInfo */
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
    fprintf(stderr, "Buffer overflow in formatWinError - need %u bytes\n", strlen(pszBuf) + 1);
    fflush(stderr);
  }

  if (strchr(pszBuf, ' ') == NULL)
    strcat(pszBuf, " failed");		// Assume simple function name

  // Append file name and line number and trailing space
  sprintf(pszBuf + strlen(pszBuf),
	  GetPString(IDS_GENERR1TEXT), pszSrcFile, uSrcLineNo);

  // Get last PM error for the current thread
  pErrInfoBlk = WinGetErrorInfo(hab);
  if (!pErrInfoBlk) {
    ERRORID id = WinGetLastError(hab);	// 03 Jan 08 SHL fixme debug
    psz = pszBuf + strlen(pszBuf);
    sprintf(psz, " WinGetErrorInfo failed (%u)", id);
  }
  else {
    if (!hwndOwner)
      hwndOwner = HWND_DESKTOP;
    /* Find message offset in array of message offsets
       Assume 1 message - fixme?
     */
    pszOffset = ((PSZ) pErrInfoBlk) + pErrInfoBlk->offaoffszMsg;
    /* Address error message in array of messages and
       append error message to source code linenumber
     */
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

//== Runtime_Error: report runtime library error using passed message string ===

VOID Runtime_Error(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt, ...)
{
  CHAR szMsg[4096];
  va_list va;

  // Format caller's message
  va_start(va, pszFmt);
  szMsg[sizeof(szMsg) - 1] = 0;
  vsprintf(szMsg, pszFmt, va);
  va_end(va);

  if (szMsg[sizeof(szMsg) - 1]) {
    fprintf(stderr, "Buffer overflow in Runtime_Error - need %u bytes\n", strlen(szMsg) + 1);
    fflush(stderr);
  }

  if (strchr(szMsg, ' ') == NULL)
    strcat(szMsg, " failed");		// Assume simple function name

  sprintf(szMsg + strlen(szMsg),
	  GetPString(IDS_GENERR1TEXT), pszSrcFile, uSrcLineNo);

  showMsg(MB_ICONEXCLAMATION, HWND_DESKTOP, DEBUG_STRING, szMsg, TRUE);

} // Runtime_Error

//== Runtime_Error2: report runtime library error using passed message id ===

VOID Runtime_Error2(PCSZ pszSrcFile, UINT uSrcLineNo, UINT idMsg)
{
  Runtime_Error(pszSrcFile, uSrcLineNo, GetPString(idMsg));

} // Runtime_Error2

// fixme to be rename to Misc_Error

//=== saymsg: report misc error using passed message ===

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

//=== showMsg: display error popup ===

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

  DosBeep(250, 100);

  return WinMessageBox(HWND_DESKTOP,	// Parent
		       hwndOwner,
		       (PSZ) pszMsg, (PSZ) pszTitle, 0,	// help id
		       mb_type | MB_MOVEABLE);
} // showMsg

//== Win_Error: report Win...() error using passed message string ===

VOID Win_Error(HWND hwndErr, HWND hwndOwner,
	       PCSZ pszSrcFile, UINT uSrcLineNo,
	       PCSZ pszFmt, ...)
{
  CHAR szMsg[4096];
  va_list va;

  // Format callers message
  va_start(va, pszFmt);
  formatWinError(szMsg, sizeof(szMsg), hwndErr, hwndOwner, pszSrcFile, uSrcLineNo, pszFmt, va);
  va_end(va);

  showMsg(MB_ENTER | MB_ICONEXCLAMATION, hwndOwner, GetPString(IDS_GENERR2TEXT),
	  szMsg, TRUE);

} // Win_Error

//== Win_Error2: report Win...() error using passed message id ===

VOID Win_Error2(HWND hwndErr, HWND hwndOwner,
		PCSZ pszSrcFile, UINT uSrcLineNo, UINT idMsg)
{
  Win_Error(hwndErr, hwndOwner, pszSrcFile, uSrcLineNo, GetPString(idMsg));

} // Win_Error2

/**
  * Output PM error messsage to stderr
  * This does to same reporting as Win_Error, but bypasses the
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
  formatWinError(szMsg, sizeof(szMsg), hwndErr, hwndOwner, pszSrcFile, uSrcLineNo, pszFmt, va);
  va_end(va);

  fputs(szMsg, stderr);
  fputc('\n', stderr);
  fputc('\n', stderr);
  fflush(stderr);

  DosBeep(250, 100);

} // Win_Error_NoMsgBox

#pragma alloc_text(ERROR,Win_Error,Dos_Error,saymsg,showMsg,GetHiresTimeer)
