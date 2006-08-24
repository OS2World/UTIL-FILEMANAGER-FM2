
/***********************************************************************

  $Id$

  Error reporting

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2006 Steven H. Levine

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
#pragma alloc_text(FMINPUT,Win_Error,Dos_Error,saymsg,showMsg)

static APIRET showMsg(ULONG mb_type, HWND hwnd, PCSZ pszTitle, PCSZ pszMsg);

//== Win_Error: report Win...() error using passed message string ===

VOID Win_Error(HWND hwndErr, HWND hwndOwner, PCSZ pszFileName, ULONG ulLineNo, PCSZ pszFmt,...)
{
  PERRINFO pErrInfoBlk;		/* Pointer to ERRINFO structure filled
				   by WinGetErrorInfo */
  PSZ pszOffset;		/* Pointer to current error message returned
				   by WinGetErrorInfo */
  CHAR szMsg[4096];
  PSZ psz;
  HAB hab;
  va_list va;

  if (hwndErr == NULLHANDLE)
    hab = (HAB)0;
  else
    hab = WinQueryAnchorBlock(hwndErr);

  // Format callers message
  va_start(va, pszFmt);
  vsprintf(szMsg, pszFmt, va);
  va_end(va);

  if (strchr(szMsg, ' ') == NULL)
    strcat(szMsg, " failed.");			// Assume simple function name

  // Append file name and line number
  sprintf(szMsg + strlen(szMsg),
	  GetPString(IDS_GENERR1TEXT),
	  pszFileName, ulLineNo, "  ");

  // Get last PM error for the current thread
  pErrInfoBlk = WinGetErrorInfo(hab);
  if (!pErrInfoBlk) {
    psz = szMsg + strlen(szMsg);
    strcpy(psz, " WinGetError failed.");
  }
  else {
    if (!hwndOwner)
      hwndOwner = HWND_DESKTOP;
    /* Find message offset in array of message offsets
       Assume 1 message - fixme?
     */
    pszOffset = ((PSZ) pErrInfoBlk) + pErrInfoBlk -> offaoffszMsg;
    /* Address error message in array of messages and
       append error message to source code linenumber
     */
    psz = szMsg + strlen(szMsg);
    sprintf(psz, "#0x%04x  \"", ERRORIDERROR(pErrInfoBlk -> idError));
    psz += strlen(psz);
    strcpy(psz, ((PSZ)pErrInfoBlk) + *(PSHORT)pszOffset);
    psz += strlen(psz);
    strcpy(psz, "\"");
    WinFreeErrorInfo(pErrInfoBlk);	// Free resource segment
  }

  showMsg(MB_ENTER | MB_ICONEXCLAMATION,
	  hwndOwner,
	  GetPString(IDS_GENERR2TEXT),	// Titlebar message
	  szMsg);			// Formatted message

} // Win_Error

//== Win_Error2: report Win...() error using passed message id ===

VOID Win_Error2(HWND hwndErr, HWND hwndOwner, PCSZ pszFileName, ULONG ulLineNo, UINT idMsg)
{
  Win_Error(hwndErr, hwndOwner, pszFileName, ulLineNo, GetPString(idMsg));

} // Win_Error2

//== Dos_Error: report Dos...() error using passed message string ===

INT Dos_Error(ULONG mb_type, ULONG ulRC, HWND hwndOwner, PCSZ pszFileName,
	      ULONG ulLineNo, PCSZ pszFmt,...)
{
  CHAR szMsg[4096];
  ULONG Class;			// Error class
  ULONG action;			// Error action
  ULONG Locus;			// Error location
  ULONG ulMsgLen;
  CHAR *pszMsgStart;
  CHAR *psz;
  va_list va;

  if (!ulRC)
    return MBID_ENTER;			// Should not have been called

  // Format caller's message
  va_start(va, pszFmt);
  vsprintf(szMsg, pszFmt, va);
  va_end(va);

  if (strchr(szMsg, ' ') == NULL)
    strcat(szMsg, " failed.");			// Assume simple function name

  DosErrClass(ulRC, &Class, &action, &Locus);

  sprintf(szMsg + strlen(szMsg),
	  GetPString(IDS_DOSERR1TEXT),
	  pszFileName,
	  ulLineNo,
	  ulRC,
	  GetPString(IDS_ERRCLASS1TEXT + (Class - 1)),
	  GetPString(IDS_ERRACTION1TEXT + (action - 1)),
	  GetPString(IDS_ERRLOCUS1TEXT + (Locus - 1)));
  pszMsgStart = szMsg + strlen(szMsg) + 1;
  // Get message leaving space for NL separator
  if (!DosGetMessage(NULL, 0L, (PCHAR)pszMsgStart + 1, 1024, ulRC, "OSO001.MSG", &ulMsgLen) ||
      !DosGetMessage(NULL, 0L, (PCHAR)pszMsgStart + 1, 1024, ulRC, "OSO001H.MSG", &ulMsgLen))
  {
    // Got message
    pszMsgStart[ulMsgLen + 1] = 0;	// Terminate
    *(pszMsgStart - 1) = '\n';		// Stuff NL before message text
    *pszMsgStart = '\"';		// Prefix message text with quote

    psz = pszMsgStart + ulMsgLen;	// Point at last char
    // Chop trailing NL CR TAB
    while (*psz &&
	   (*psz == '\r' || *psz == '\n' || *psz == ' ' || *psz == '\t'))
    {
      *psz-- = 0;
    }
    strcat(psz, "\"");			// Append trailing quote

    // Convert CR and NL combos to single space
    psz = pszMsgStart;
    while (*psz)
    {
      if (*psz == '\n' || *psz == '\r')
      {
	while (*(psz + 1) == '\n' || *(psz + 1) == '\r')
	  memmove(psz, psz + 1, strlen(psz));
	*psz = ' ';
      }
      else
	psz++;
    }
  }

  return showMsg(mb_type | MB_ICONEXCLAMATION,
		 hwndOwner,
		 GetPString(IDS_DOSERR2TEXT),	// Title
		 szMsg);

} // Dos_Error

//== Dos_Error2: report Dos...() error using passed message id ===

INT Dos_Error2(ULONG mb_type, ULONG ulRC, HWND hwndOwner, PCSZ pszFileName,
	      ULONG ulLineNo, UINT idMsg)
{
  return Dos_Error(mb_type, ulRC, hwndOwner, pszFileName,ulLineNo,
                    GetPString(idMsg));
} // Dos_Error2

//== Runtime_Error: report runtime library error using passed message string ===

VOID Runtime_Error(PCSZ pszSrcFile, UINT uSrcLineNo, PCSZ pszFmt,...)
{
  CHAR szMsg[4096];
  va_list va;
  PSZ psz;

  // Format caller's message
  va_start(va, pszFmt);
  vsprintf(szMsg, pszFmt, va);
  va_end(va);

  if (strchr(szMsg, ' ') == NULL)
    strcat(szMsg, " failed.");			// Assume simple function name

  sprintf(szMsg + strlen(szMsg),
	  // GetPString(IDS_DOSERR1TEXT), fixme
	  "\nModule: %s\nLinenumber: %u",
	  pszSrcFile,
	  uSrcLineNo);

  showMsg(MB_ICONEXCLAMATION,HWND_DESKTOP,DEBUG_STRING,szMsg);

} // Runtime_Error

//== Runtime_Error2: report runtime library error using passed message id ===

VOID Runtime_Error2(PCSZ pszSrcFile, UINT uSrcLineNo, UINT idMsg)
{
  Runtime_Error(pszSrcFile, uSrcLineNo, GetPString(idMsg));

} // Runtime_Error2

// fixme to be rename to Misc_Error

//=== saymsg: report misc error using passed message ===

APIRET saymsg(ULONG mb_type, HWND hwnd, PCSZ pszTitle, PCSZ pszFmt,...)
{
  CHAR szMsg[4096];
  va_list va;

  va_start(va, pszFmt);
  vsprintf(szMsg, pszFmt, va);
  va_end(va);

  return showMsg(mb_type,hwnd,pszTitle,szMsg);

} // saymsg

//=== showMsg: display error popup ===

static APIRET showMsg(ULONG mb_type, HWND hwnd, PCSZ pszTitle, PCSZ pszMsg)
{
  if ((mb_type & (MB_YESNO | MB_YESNOCANCEL)) == 0)
  {
    fputs(pszMsg, stderr);
    fputc('\n', stderr);
    fflush(stderr);
  }

  if (!hwnd)
    hwnd = HWND_DESKTOP;

  DosBeep(250,100);

  return WinMessageBox(HWND_DESKTOP,	// Parent
		       hwnd,		// Owner
		       (PSZ)pszMsg,
		       (PSZ)pszTitle,
		       0,		// help id
		       mb_type | MB_MOVEABLE);
} // showMsg
