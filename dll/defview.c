#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_MMIOOS2

#include <os2.h>
#include <os2me.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fm3dll.h"
#include "fm3dlg.h"

#pragma alloc_text(DEFVIEW,DefaultView,ShowMultimedia,DefaultViewKeys)


BOOL ShowMultimedia (CHAR *filename) {

  static BOOL       unavailable = FALSE;
  BOOL              ret = FALSE;
  CHAR              loaderror[CCHMAXPATH];
  HMODULE           MMIOModHandle = (HMODULE)0;
  PMMIOIDENTIFYFILE MMIOIdentifyFile = (PMMIOIDENTIFYFILE)0;
  FOURCC            fccStorageSystem = (FOURCC)0;
  MMFORMATINFO      mmFormatInfo;
  APIRET            rc;

  if(unavailable ||
     !filename ||
     !*filename)
    return ret;

  /* load MMPM/2, if available. */
  *loaderror = 0;
  rc = DosLoadModule(loaderror,
                     sizeof(loaderror),
                     "MMIO",
                     &MMIOModHandle);
  if(rc) {
    unavailable = TRUE;
    return ret;
  }
  else {
    if(DosQueryProcAddr(MMIOModHandle,
                        0,
                        "mmioIdentifyFile",
                        (PFN *)&MMIOIdentifyFile)) {
      DosFreeModule(MMIOModHandle);
      unavailable = TRUE;
      return ret;
    }
  }

  /* attempt to identify the file using MMPM/2 */
  memset(&mmFormatInfo,0,sizeof(MMFORMATINFO));
  mmFormatInfo.ulStructLen = sizeof(MMFORMATINFO);
  rc = MMIOIdentifyFile(filename,
                        0L,
                        &mmFormatInfo,
                        &fccStorageSystem,
                        0L,
                        MMIO_FORCE_IDENTIFY_FF);
  /* free module handle */
  DosFreeModule(MMIOModHandle);

  /* check returns from MMPM/2's identification process */
  if(rc != MMIO_ERROR &&
     mmFormatInfo.fccIOProc != FOURCC_DOS) {
    if(mmFormatInfo.ulMediaType == MMIO_MEDIATYPE_IMAGE &&
       (mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED) != 0) {
      /* is an image */
      runemf2(SEPARATE | WINDOWED,
              HWND_DESKTOP,
              NULL,
              NULL,
              "%sIMAGE.EXE \"%s\"",
              (fAddUtils) ? "UTILS\\" : NullStr,
              filename);
      ret = TRUE;
    }
    else {
      if(mmFormatInfo.ulMediaType != MMIO_MEDIATYPE_IMAGE)
        /* is a multimedia file (WAV, MID, AVI, etc.) */
        runemf2(SEPARATE | WINDOWED,
                HWND_DESKTOP,
                NULL,
                NULL,
                "%sFM2PLAY.EXE \"%s\"",
                (fAddUtils) ? "UTILS\\" : NullStr,
                filename);
      ret = TRUE;
    }
  }

  return ret;
}


VOID DefaultViewKeys (HWND hwnd,HWND hwndFrame,HWND hwndParent,
                      SWP *swp,CHAR *filename) {

  if((shiftstate & (KC_CTRL | KC_SHIFT)) ==
     (KC_CTRL | KC_SHIFT))
    DefaultView(hwnd,
                hwndFrame,
                hwndParent,
                swp,
                4,
                filename);
  else if(shiftstate & KC_CTRL)
    DefaultView(hwnd,
                hwndFrame,
                hwndParent,
                swp,
                2,
                filename);
  else if(shiftstate & KC_SHIFT)
    DefaultView(hwnd,
                hwndFrame,
                hwndParent,
                swp,
                1,
                filename);
  else
    DefaultView(hwnd,
                hwndFrame,
                hwndParent,
                swp,
                0,
                filename);
}


VOID DefaultView (HWND hwnd,HWND hwndFrame,HWND hwndParent,SWP *swp,
                  ULONG flags,CHAR *filename) {

  /*
   * bitmapped flags:
   * ---------------
   * 1  = View directly
   * 2  = Open WPS default view
   * 4  = Open WPS settings view
   * 8  = Edit
   * 16 = Info
   * 32 = No view info
   */

  HWND  hwndArc = (HWND)0;
  char *p,*dummy[3];

  if(!hwndParent)
    hwndParent = HWND_DESKTOP;

  if(flags & 32) {
    flags &= (~16);
    if(!IsFile(filename)) {
      DosBeep(50,100);
      return;
    }
  }

  if(flags & 1)     /* directly view the file */
    goto ViewIt;

  if(flags & 2) {   /* open default WPS view of file */
    OpenObject(filename,
               Default,
               hwnd);
    return;
  }

  if(flags & 4) {   /* open WPS settings notebook for file */
    OpenObject(filename,
               Settings,
               hwnd);
    return;
  }

  if((flags & 16) ||
     !IsFile(filename)) {   /* open info for directories */

    char fullname[CCHMAXPATH];

    if(!IsFullName(filename)) {
      if(!DosQueryPathInfo(filename,
                           FIL_QUERYFULLNAME,
                           fullname,
                           sizeof(fullname)))
        filename = fullname;
    }
    if(IsFullName(filename) &&
       !(driveflags[toupper(*filename) - 'A'] & DRIVE_INVALID)) {
      if(!IsRoot(filename)) {
        dummy[0] = filename;
        dummy[1] = NULL;
        WinDlgBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  FileInfoProc,
                  FM3ModHandle,
                  FLE_FRAME,
                  (PVOID)dummy);
      }
      else
        WinDlgBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  DrvInfoProc,
                  FM3ModHandle,
                  INFO_FRAME,
                  (PVOID)filename);
    }
    return;
  }

  if(flags & 8) {   /* edit file */

    ULONG type = IDM_EDITTEXT;

    dummy[0] = filename;
    dummy[1] = NULL;
    if(TestBinary(filename))
      type = IDM_EDITBINARY;
    switch(type) {
      case IDM_EDITBINARY:
        if(*bined) {
          ExecOnList((HWND)0,
                     bined,
                     WINDOWED | SEPARATE,
                     NULL,
                     dummy,
                     NULL);
          break;
        }
      /* else intentional fallthru */
      case IDM_EDITTEXT:
        if(*editor)
          ExecOnList((HWND)0,
                     editor,
                     WINDOWED | SEPARATE,
                     NULL,
                     dummy,
                     NULL);
        else {
          type = (type == IDM_EDITTEXT) ? 8 :
                  (type == IDM_EDITBINARY) ? 16 :
                   0;
          type |= 4;
          StartMLEEditor(hwndParent,
                         type,
                         filename,
                         hwndFrame);
        }
        break;
    }
    return;
  }

  if(ExecAssociation(hwnd,
                     filename) == -1) {
    hwndArc = StartArcCnr((fExternalArcboxes || !swp ||
                           strcmp(realappname,FM3Str)) ?
                          HWND_DESKTOP :
                          hwndParent,
                          hwndFrame,
                          filename,
                          4,
                          NULL);
    if(!hwndArc) {
      if(!fCheckMM ||
         !ShowMultimedia(filename)) {
        if(!IsExecutable(filename) ||
           !ExecFile(hwnd,filename)) {
          p = strrchr(filename,'.');
          if(!p)
            p = ".";
          if(stricmp(p,".INI") ||
             !StartIniEditor(hwndParent,
                             filename,
                             4)) {
            if(stricmp(p,".HLP") ||
               !ViewHelp(filename)) {
ViewIt:
              if(*viewer) {
                dummy[0] = filename;
                dummy[1] = NULL;
                ExecOnList(hwnd,
                           viewer,
                           WINDOWED | SEPARATE |
                           ((fViewChild) ? CHILD : 0),
                           NULL,
                           dummy,
                           NULL);
              }
              else
                StartMLEEditor(hwndParent,
                               5,
                               filename,
                               hwndFrame);
            }
          }
        }
      }
    }
    else {
      if((swp &&
          !fExternalArcboxes &&
          !strcmp(realappname,FM3Str)) &&
         !ParentIsDesktop(hwnd,hwndParent))
        WinSetWindowPos(hwndArc,
                        HWND_TOP,
                        swp->x,
                        swp->y,
                        swp->cx,
                        swp->cy,
                        SWP_MOVE   | SWP_SIZE     | SWP_SHOW |
                        SWP_ZORDER | SWP_ACTIVATE);
    }
  }
}

