
/***********************************************************************

  $Id$

  Error reporting

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  Revisions	12 Aug 04 SHL Comments

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
#pragma alloc_text(FMINPUT,General_Error,Dos_Error)

// fixme to have MiscError instead of saymsg

// fixme to be Win_Error
// fixme to pass hwndError rather hab

VOID General_Error(HAB hab,HWND hwndOwner, PSZ ErrModule,LONG ErrLine,CHAR *s,...)
{
  PERRINFO         pErrInfoBlk;    /* Pointer to ERRINFO structure that is filled
                                      by WinGetErrorInfo */
  PSZ              pszOffset;      /* Pointer to the current error message returned
                                      by WinGetErrorInfo */
  CHAR             ErrBuffer[4096]; /* The error message that is displayed to
                                       the user via WinMessageBox */
  PSZ		   psz;
  va_list          ap;

  // Format callers message
  va_start(ap,s);
  vsprintf(ErrBuffer,s,ap);
  va_end(ap);

  // Append file name and line number
  sprintf(ErrBuffer + strlen(ErrBuffer),
          GetPString(IDS_GENERR1TEXT),
          ErrModule, ErrLine, "  ");

  /* Get last PM error for the current thread */
  pErrInfoBlk = WinGetErrorInfo(hab);
  if(pErrInfoBlk != NULL) {
    if(!hwndOwner)
      hwndOwner = HWND_DESKTOP;
    /* Find message offset in array of message offsets
       Assume 1 message - fixme?
     */
    pszOffset = ((PSZ)pErrInfoBlk) + pErrInfoBlk->offaoffszMsg;
    /* Address error message in array of messages and
       append error message to source code linenumber
     */
    psz = ErrBuffer + strlen(ErrBuffer);
    sprintf(psz,"#0x%04x  \"", ERRORIDERROR(pErrInfoBlk->idError));
    psz += strlen(psz);
    strcpy(psz,((PSZ)pErrInfoBlk) + *(PSHORT)pszOffset);
    psz += strlen(psz);
    strcpy(psz,"\"");
    WinFreeErrorInfo(pErrInfoBlk);	/* Free resource segment */

    WinMessageBox(HWND_DESKTOP,	        	/* Parent window */
                  hwndOwner,			/* Owner window */
                  ErrBuffer,			/* Formatted message */
                  GetPString(IDS_GENERR2TEXT),	/* Titlebar message */
                  0,				/* Message identifier */
                  MB_ENTER | MB_ICONEXCLAMATION | MB_MOVEABLE);
  }
}


INT Dos_Error(INT type,ULONG Error,HWND hwndOwner, PSZ ErrModule,
              LONG ErrLine,CHAR *s,...)
{
  CHAR             MsgBuffer[4096]; /* The whole error message that
                                       is displayed to
                                       the user via WinMessageBox */
  ULONG            Class = 17;      /* Error class */
  ULONG            action = 9;      /* Error action */
  ULONG            Locus = 7;       /* Error location */
  ULONG            len;
  CHAR            *pszMsgStart;
  CHAR		  *psz;
  va_list          ap;

  if(Error != 0) {
    strset(MsgBuffer,0);		// fixme?
    if(!hwndOwner)
      hwndOwner = HWND_DESKTOP;

    // Format caller's message
    va_start(ap,s);
    vsprintf(MsgBuffer,s,ap);
    va_end(ap);

    DosErrClass(Error,&Class,&action,&Locus);

    sprintf(&MsgBuffer[strlen(MsgBuffer)],
            GetPString(IDS_DOSERR1TEXT),
            ErrModule,
            ErrLine,
            Error,
            GetPString(IDS_ERRCLASS1TEXT + (Class - 1)),
            GetPString(IDS_ERRACTION1TEXT + (action - 1)),
            GetPString(IDS_ERRLOCUS1TEXT + (Locus - 1)));
    pszMsgStart = MsgBuffer + strlen(MsgBuffer) + 1;
    // Get mesasge leaving space for NL separator
    if(!DosGetMessage(NULL,0L,(PCHAR)pszMsgStart + 1,1024,Error,"OSO001.MSG",&len) ||
       !DosGetMessage(NULL,0L,(PCHAR)pszMsgStart + 1,1024,Error,"OSO001H.MSG",&len))
    {
      // Got message
      pszMsgStart[len + 1] = 0;		// Terminate
      *(pszMsgStart - 1) = '\n';	// Stuff NL before message text
      *pszMsgStart = '\"';		// Prefix message text with quote
      psz = pszMsgStart + len;		// Point at last char
      // Chop trailing NL CR TAB
      while (*psz &&
             (*psz == '\r' || *psz == '\n' || *psz == ' ' || *psz == '\t')) {
        *psz = 0;
        psz--;
      }
      strcat(pszMsgStart,"\"");		// Append trailing quote
      // Convert CR and NL combos to single space
      psz = pszMsgStart;
      while (*psz) {
        if (*psz == '\n' || *psz == '\r') {
          while (*(psz + 1) == '\n' || *(psz + 1) == '\r')
            memmove(psz,psz + 1,strlen(psz));
          *psz = ' ';
        }
        else
          psz++;
      }
    }
    return WinMessageBox(HWND_DESKTOP,          	/* Parent window */
                         hwndOwner,             	/* Owner window */
                         MsgBuffer,     		/* Formatted message */
                         GetPString(IDS_DOSERR2TEXT),	/* Title bar message */
                         0,                     	/* Message identifier */
                         type | MB_ICONEXCLAMATION | MB_MOVEABLE);
  }
  return MBID_ENTER;
}
