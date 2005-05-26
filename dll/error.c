
/***********************************************************************

  $Id$

  Error reporting

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2005 Steven H. Levine

  12 Aug 04 SHL Comments
  23 May 05 SHL Move saymsg here
  24 May 05 SHL Rename General_Error to more accurate Win_Error
  24 May 05 SHL Rename saymsg to more accurate Misc_Error
  24 May 05 SHL Rework Win_Error args and clean up logic

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
#pragma alloc_text(FMINPUT,Win_Error,Dos_Error,saymsg)

// fixme to pass hwndError rather hab

//== Win_Error: report Win...() error ===

VOID Win_Error(HWND hwndErr, HWND hwndOwner, PSZ pszFileName, ULONG ulLineNo, CHAR *pszFmt,...)
{
  PERRINFO pErrInfoBlk;		/* Pointer to ERRINFO structure that is filled
				   by WinGetErrorInfo */
  PSZ pszOffset;		/* Pointer to the current error message returned
				   by WinGetErrorInfo */
  CHAR szErrBuffer[4096];	/* The error message that is displayed to
				   the user via WinMessageBox */
  PSZ psz;
  HAB hab;
  va_list va;

  if (hwndErr == NULLHANDLE)
    hab = (HAB) 0;
  else
    hab = WinQueryAnchorBlock(hwndErr);

  // Format callers message
  va_start(va, pszFmt);
  vsprintf(szErrBuffer, pszFmt, va);
  va_end(va);

  // Append file name and line number
  sprintf(szErrBuffer + strlen(szErrBuffer),
	  GetPString(IDS_GENERR1TEXT),
	  pszFileName, ulLineNo, "  ");

  /* Get last PM error for the current thread */
  pErrInfoBlk = WinGetErrorInfo(hab);
  if (pErrInfoBlk != NULL)
  {
    if (!hwndOwner)
      hwndOwner = HWND_DESKTOP;
    /* Find message offset in array of message offsets
       Assume 1 message - fixme?
     */
    pszOffset = ((PSZ) pErrInfoBlk) + pErrInfoBlk -> offaoffszMsg;
    /* Address error message in array of messages and
       append error message to source code linenumber
     */
    psz = szErrBuffer + strlen(szErrBuffer);
    sprintf(psz, "#0x%04x  \"", ERRORIDERROR(pErrInfoBlk -> idError));
    psz += strlen(psz);
    strcpy(psz, ((PSZ) pErrInfoBlk) + *(PSHORT) pszOffset);
    psz += strlen(psz);
    strcpy(psz, "\"");
    WinFreeErrorInfo(pErrInfoBlk);	/* Free resource segment */

    WinMessageBox(HWND_DESKTOP,		/* Parent */
		  hwndOwner,		/* Owner */
		  szErrBuffer,		/* Formatted message */
		  GetPString(IDS_GENERR2TEXT),	/* Titlebar message */
		  0,			/* Message identifier */
		  MB_ENTER | MB_ICONEXCLAMATION | MB_MOVEABLE);
  }

} // Win_Error

//== Dos_Error: report Dos...() error ===

INT Dos_Error(ULONG mb_type, ULONG ulRC, HWND hwndOwner, PSZ pszFileName,
	      ULONG ulLineNo, CHAR *pszFmt,...)
{
  CHAR szMsgBuffer[4096];	/* The whole error message that
				   is displayed to
				   the user via WinMessageBox */
  ULONG Class = 17;		/* Error class - fixme to not init? */
  ULONG action = 9;		/* Error action */
  ULONG Locus = 7;		/* Error location */
  ULONG ulMsgLen;
  CHAR *pszMsgStart;
  CHAR *psz;
  va_list va;

  if (!ulRC)
    return MBID_ENTER;			// Should not have been called

  if (!hwndOwner)
    hwndOwner = HWND_DESKTOP;

  // Format caller's message
  va_start(va, pszFmt);
  vsprintf(szMsgBuffer, pszFmt, va);
  va_end(va);

  DosErrClass(ulRC, &Class, &action, &Locus);

  sprintf(szMsgBuffer + strlen(szMsgBuffer),
	  GetPString(IDS_DOSERR1TEXT),
	  pszFileName,
	  ulLineNo,
	  ulRC,
	  GetPString(IDS_ERRCLASS1TEXT + (Class - 1)),
	  GetPString(IDS_ERRACTION1TEXT + (action - 1)),
	  GetPString(IDS_ERRLOCUS1TEXT + (Locus - 1)));
  pszMsgStart = szMsgBuffer + strlen(szMsgBuffer) + 1;
  // Get message leaving space for NL separator
  if (!DosGetMessage(NULL, 0L, (PCHAR) pszMsgStart + 1, 1024, ulRC, "OSO001.MSG", &ulMsgLen) ||
      !DosGetMessage(NULL, 0L, (PCHAR) pszMsgStart + 1, 1024, ulRC, "OSO001H.MSG", &ulMsgLen))
  {
    // Got message
    pszMsgStart[ulMsgLen + 1] = 0;	// Terminate
    *(pszMsgStart - 1) = '\n';	// Stuff NL before message text
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

  return WinMessageBox(HWND_DESKTOP,	/* Parent */
		       hwndOwner,	/* Owner */
		       szMsgBuffer,	/* Formatted message */
		       GetPString(IDS_DOSERR2TEXT),	/* Title bar text */
		       0,		/* Message identifier */
		       mb_type | MB_ICONEXCLAMATION | MB_MOVEABLE);

} // Dos_Error

// fixme to be Misc_Error instead of saymsg

//=== saymsg: report misc error ===

APIRET saymsg(ULONG mb_type, HWND hwnd, CHAR *pszTitle, CHAR *pszFmt,...)
{
  CHAR szBuffer[4096];
  va_list va;

  va_start(va, pszFmt);
  vsprintf(szBuffer, pszFmt, va);
  va_end(va);

  if (!hwnd)
    hwnd = HWND_DESKTOP;

  return WinMessageBox(HWND_DESKTOP, hwnd, szBuffer, pszTitle,
		       0, mb_type | MB_MOVEABLE);
} // saymsg
