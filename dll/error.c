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


VOID General_Error(HAB hab,HWND hwnd, PSZ ErrModule,LONG ErrLine,CHAR *s,...) {

  PERRINFO         pErrInfoBlk;    /* Pointer to ERRINFO structure that is filled
                                      by WinGetErrorInfo */
  PSZ              pszOffset;      /* Pointer to the current error message returned
                                      by WinGetErrorInfo */
  CHAR             ErrBuffer[4096]; /* The whole error message that is displayed to
                                       the user via WinMessageBox */
  va_list          ap;

  va_start(ap,s);
  vsprintf(ErrBuffer,s,ap);
  va_end(ap);
  sprintf(&ErrBuffer[strlen(ErrBuffer)],
          GetPString(IDS_GENERR1TEXT),
          ErrModule,
          ErrLine,
          "  ");
  /* Get last error for the current thread. */
  pErrInfoBlk = WinGetErrorInfo(hab);
  if(pErrInfoBlk != (PERRINFO)NULL) {
    if(!hwnd)
      hwnd = HWND_DESKTOP;
    /* Find offset in array of message offsets */
    pszOffset = ((PSZ)pErrInfoBlk) + pErrInfoBlk->offaoffszMsg;
    /* Address error message in array of messages and
       append error message to source code linenumber */
    sprintf(&ErrBuffer[strlen(ErrBuffer)],"#0x%04x  \"",
            ERRORIDERROR(pErrInfoBlk->idError));
    strcat(ErrBuffer,(((PSZ)pErrInfoBlk) + *((PSHORT)pszOffset)));
    strcat(ErrBuffer,"\"");
    WinFreeErrorInfo(pErrInfoBlk);            /* Free resource segment */
    WinMessageBox(HWND_DESKTOP,               /* Parent window is DESKTOP */
                  hwnd,                       /* Owner window is DESKTOP */
                  (PSZ)ErrBuffer,             /* General_Error message */
                                              /* Title bar message */
                  GetPString(IDS_GENERR2TEXT),
                  0,                          /* Message identifier */
                  MB_ENTER | MB_ICONEXCLAMATION | MB_MOVEABLE);
  }
}


INT Dos_Error(INT type,ULONG Error,HWND hwnd, PSZ ErrModule,
              LONG ErrLine,CHAR *s,...) {

  CHAR             MsgBuffer[4096]; /* The whole error message that
                                       is displayed to
                                       the user via WinMessageBox */
  ULONG            Class = 17;      /* Error class */
  ULONG            action = 9;      /* Error action */
  ULONG            Locus = 7;       /* Error location */
  ULONG            len;
  CHAR            *p,*pp;
  va_list          ap;

  if(Error != 0) {
    strset(MsgBuffer,0);
    if(!hwnd)
      hwnd = HWND_DESKTOP;
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
    p = MsgBuffer + strlen(MsgBuffer) + 1;
    if(!DosGetMessage(NULL,0L,(PCHAR)p + 1,1024,Error,"OSO001.MSG",&len) ||
       !DosGetMessage(NULL,0L,(PCHAR)p + 1,1024,Error,"OSO001H.MSG",&len)) {
      p[len + 1] = 0;
      *(p - 1) = '\n';
      *p = '\"';
      pp = p + len;
      while(*pp && (*pp == '\r' || *pp == '\n' || *pp == ' ' || *pp == '\t')) {
        *pp = 0;
        pp--;
      }
      strcat(p,"\"");
      pp = p;
      while(*pp) {
        if(*pp == '\n' || *pp == '\r') {
          while(*(pp + 1) == '\n' || *(pp + 1) == '\r')
            memmove(pp,pp + 1,strlen(pp));
          *pp = ' ';
        }
        else
          pp++;
      }
    }
    return WinMessageBox(HWND_DESKTOP,            /* Parent window is DESKTOP */
                         hwnd,                    /* Owner window */
                         (PSZ)MsgBuffer,          /* DOS API error message */
                                                  /* Title bar message */
                         GetPString(IDS_DOSERR2TEXT),
                         0,                       /* Message identifier */
                         type | MB_ICONEXCLAMATION | MB_MOVEABLE);
  }
  return MBID_ENTER;
}

