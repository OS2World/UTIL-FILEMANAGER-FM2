
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2008 Steven H.Levine

  Default file viewer

  20 Nov 03 SHL ShowMultimedia: try to convince fmplay to not play exes (Gregg Young)
  14 Jul 06 SHL Use Runtime_Error
  18 Mar 07 GKY Fixed misindentifycation of nonmultimedia files by ShowMultiMedia
  18 Mar 07 GKY Open mp3, ogg & flac files with OS2 object default since fm2play fails
  21 Apr 07 GKY Find FM2Utils by path or utils directory
  09 Jun 07 SHL ShowMultimedia: Initialize hwnd so that OpenObject might work
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  20 Dec 07 GKY Open jpg files with OS2 object default since image.exe fails
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  25 Aug 08 GKY Check TMP directory space warn if lee than 5 MiB prevent archiver from opening if
                less than 10 KiB (It hangs and can't be closed)

***********************************************************************/

#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_MMIOOS2
#define INCL_LONGLONG			// dircnrs.h
#include <os2.h>
#include <os2me.h>

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "mainwnd2.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "arccnrs.h"			// StartArcCnr
#include "errutil.h"			// Dos_Error...
#include "notebook.h"                   // external viewers
#include "defview.h"
#include "info.h"			// DrvInfoProc
#include "assoc.h"			// ExecAssociation
#include "info.h"			// FileInfoProc
#include "valid.h"			// IsExecutable
#include "srchpath.h"			// RunFM2Util
#include "inis.h"			// StartIniEditor
#include "systemf.h"			// ExecOnList
#include "shadow.h"			// OpenObject
#include "viewer.h"			// StartMLEEditor
#include "newview.h"			// StartViewer
#include "mainwnd.h"			// Data declaration(s)
#include "misc.h"			// ExecFile, ViewHelp

// Data definitions
static PSZ pszSrcFile = __FILE__;

#pragma data_seg(GLOBAL2)
CHAR *Default;

BOOL ShowMultimedia(CHAR * filename)
{

  static BOOL no_mmos2 = FALSE;
  BOOL played = FALSE;
  CHAR loaderror[CCHMAXPATH];
  HMODULE MMIOModHandle = NULLHANDLE;
  PMMIOIDENTIFYFILE pMMIOIdentifyFile = NULL;
  PMMIOGETINFO pMMIOGetInfo = NULL;
  PMMIOCLOSE pMMIOClose = NULL;
  PMMIOOPEN pMMIOOpen = NULL;
  MMIOINFO mmioinfo;
  HMMIO hmmio;
  FOURCC fccStorageSystem = 0;
  MMFORMATINFO mmFormatInfo;
  APIRET rc, rc1;
  HWND hwnd = HWND_DESKTOP;
  char *p;

  if (no_mmos2 || !filename || !*filename)
    return played;

  /* load MMPM/2, if available. */
  *loaderror = 0;
  rc = DosLoadModule(loaderror, sizeof(loaderror), "MMIO", &MMIOModHandle);
  if (rc) {
    no_mmos2 = TRUE;
    return played;
  }
  else {
    if (DosQueryProcAddr(MMIOModHandle,
			 0,
			 "mmioIdentifyFile", (PFN *) & pMMIOIdentifyFile)) {
      DosFreeModule(MMIOModHandle);
      no_mmos2 = TRUE;
      return played;
    }
    if (DosQueryProcAddr(MMIOModHandle,
			 0,
			 "mmioGetInfo", (PFN *) & pMMIOGetInfo)) {
      DosFreeModule(MMIOModHandle);
      no_mmos2 = TRUE;
      return played;
    }
    if (DosQueryProcAddr(MMIOModHandle,
			 0,
			 "mmioClose", (PFN *) & pMMIOClose)) {
      DosFreeModule(MMIOModHandle);
      no_mmos2 = TRUE;
      return played;
    }
    if (DosQueryProcAddr(MMIOModHandle,
			 0,
			 "mmioOpen", (PFN *) & pMMIOOpen)) {
      DosFreeModule(MMIOModHandle);
      no_mmos2 = TRUE;
      return played;
    }
  }

  /* attempt to identify the file using MMPM/2 */
  //printf("%s %d\n ", __FILE__, __LINE__); fflush(stdout);
  memset( &mmioinfo, '\0', sizeof(MMIOINFO) );
  /*Eliminate non multimedia files*/
  hmmio = pMMIOOpen(filename,
	            &mmioinfo,
	            MMIO_READ);
#if 0
  printf("%s %d %d %d %d %d\n",
	  __FILE__, __LINE__,mmioinfo.ulFlags, mmioinfo.ulErrorRet,
	 mmioinfo.pIOProc, mmioinfo.aulInfo); fflush(stdout);
#endif
	 if (!hmmio) {
	     p = strrchr(filename, '.'); //Added to save mp3, ogg & flac which fail above test
	  if (!p)
	      p = ".";
	     /* printf("%s %d %s\n",
	      __FILE__, __LINE__, p); fflush(stdout);*/
	  if (!stricmp(p, ".OGG") || !stricmp(p, ".MP3") || !stricmp(p, ".FLAC") ||
	       !stricmp(p, ".JPG") || !stricmp(p, ".JPEG")){
	     hmmio = pMMIOOpen(filename,
	            &mmioinfo,
	            MMIO_READ | MMIO_NOIDENTIFY);
	     if (!hmmio){
	         DosFreeModule(MMIOModHandle);
	         //printf("%s %d\n ", __FILE__, __LINE__); fflush(stdout);
	         return played;
	     }
	  }
	  else {
	     DosFreeModule(MMIOModHandle);
	        // printf("%s %d\n ", __FILE__, __LINE__); fflush(stdout);
	         return played;
	  }
	 }
	 if (!hmmio) {
	         DosFreeModule(MMIOModHandle);
	        // printf("%s %d\n ", __FILE__, __LINE__); fflush(stdout);
	         return played;
	     }

  rc1 = pMMIOGetInfo(hmmio, &mmioinfo, 0L);
  // printf("%s %d\n ", __FILE__, __LINE__); fflush(stdout);
  memset(&mmFormatInfo, 0, sizeof(MMFORMATINFO));
  mmFormatInfo.ulStructLen = sizeof(MMFORMATINFO);
  rc = pMMIOIdentifyFile(filename,
			 &mmioinfo,
			 &mmFormatInfo,
	                 &fccStorageSystem, 0L,
	                 MMIO_FORCE_IDENTIFY_FF);
#if 0
   printf("%s %d %d %d %d\n %d %d %d %s\n",
	  __FILE__, __LINE__,mmioinfo.ulFlags,
	  mmioinfo.pIOProc, mmioinfo.aulInfo,
	  mmFormatInfo.fccIOProc, mmFormatInfo.fccIOProc,
	  mmFormatInfo.ulIOProcType, mmFormatInfo.szDefaultFormatExt); fflush(stdout);
#endif
  /* free module handle */
  rc1 = pMMIOClose(hmmio, 0L);
  DosFreeModule(MMIOModHandle);

  /* if identified and not FOURCC_DOS */
  if (!rc && mmFormatInfo.fccIOProc != FOURCC_DOS) {
    if (mmFormatInfo.ulMediaType == MMIO_MEDIATYPE_IMAGE &&
	(mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED)) {
      p = strrchr(filename, '.');
	  if (!p)
	      p = ".";
	     /* printf("%s %d %s\n",
	      __FILE__, __LINE__, p); fflush(stdout);*/
	  if (!stricmp(p, ".JPG") || !stricmp(p, ".JPEG"))
	    OpenObject(filename, Default, hwnd);  //Image fails to display these
	  else       // is an image that can be translated
	    RunFM2Util("IMAGE.EXE", filename);
	  played = TRUE;
    }
    else if (mmFormatInfo.ulMediaType != MMIO_MEDIATYPE_IMAGE) {
	/* is a multimedia file (WAV, MID, AVI, etc.) */
	p = strrchr(filename, '.');
	  if (!p)
	      p = ".";
	     /* printf("%s %d %s\n",
	      __FILE__, __LINE__, p); fflush(stdout);*/
	  if (!stricmp(p, ".OGG") || !stricmp(p, ".MP3") || !stricmp(p, ".FLAC"))
	      OpenObject(filename, Default, hwnd);  //FM2Play fails to play these
	  else
	    RunFM2Util("FM2PLAY.EXE", filename);
	  played = TRUE;
    }
  }

  return played;
}

VOID DefaultViewKeys(HWND hwnd, HWND hwndFrame, HWND hwndParent,
		     SWP * swp, CHAR * filename)
{
  if ((shiftstate & (KC_CTRL | KC_SHIFT)) == (KC_CTRL | KC_SHIFT))
    DefaultView(hwnd, hwndFrame, hwndParent, swp, 4, filename);
  else if (shiftstate & KC_CTRL)
    DefaultView(hwnd, hwndFrame, hwndParent, swp, 2, filename);
  else if (shiftstate & KC_SHIFT)
    DefaultView(hwnd, hwndFrame, hwndParent, swp, 1, filename);
  else
    DefaultView(hwnd, hwndFrame, hwndParent, swp, 0, filename);
}

VOID DefaultView(HWND hwnd, HWND hwndFrame, HWND hwndParent, SWP * swp,
		 ULONG flags, CHAR * filename)
{
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

  HWND hwndArc = (HWND) 0;
  char *p, *dummy[3];

  if (!hwndParent)
    hwndParent = HWND_DESKTOP;

  if (flags & 32) {
    flags &= (~16);
    if (!IsFile(filename)) {
      Runtime_Error(pszSrcFile, __LINE__, "%s not found", filename);
      return;
    }
  }

  if (flags & 1)			/* directly view the file */
    goto ViewIt;

  if (flags & 2) {			/* open default WPS view of file */
    OpenObject(filename, Default, hwnd);
    return;
  }

  if (flags & 4) {			/* open WPS settings notebook for file */
    OpenObject(filename, Settings, hwnd);
    return;
  }

  if ((flags & 16) || !IsFile(filename)) {	/* open info for directories */

    char fullname[CCHMAXPATH];

    if (!IsFullName(filename)) {
      if (!DosQueryPathInfo(filename,
			    FIL_QUERYFULLNAME, fullname, sizeof(fullname)))
	filename = fullname;
    }
    if (IsFullName(filename) &&
	!(driveflags[toupper(*filename) - 'A'] & DRIVE_INVALID)) {
      if (!IsRoot(filename)) {
	dummy[0] = filename;
	dummy[1] = NULL;
	WinDlgBox(HWND_DESKTOP,
		  HWND_DESKTOP,
		  FileInfoProc, FM3ModHandle, FLE_FRAME, (PVOID) dummy);
      }
      else
	WinDlgBox(HWND_DESKTOP,
		  HWND_DESKTOP,
		  DrvInfoProc, FM3ModHandle, INFO_FRAME, (PVOID) filename);
    }
    return;
  }

  if (flags & 8) {			/* edit file */

    ULONG type = IDM_EDITTEXT;

    dummy[0] = filename;
    dummy[1] = NULL;
    if (TestBinary(filename))
      type = IDM_EDITBINARY;
    switch (type) {
    case IDM_EDITBINARY:
      if (*bined) {
	ExecOnList((HWND) 0, bined, WINDOWED | SEPARATE, NULL, dummy, NULL,
	           pszSrcFile, __LINE__);
	break;
      }
      /* else intentional fallthru */
    case IDM_EDITTEXT:
      if (*editor)
	ExecOnList((HWND) 0, editor, WINDOWED | SEPARATE, NULL, dummy, NULL,
	           pszSrcFile, __LINE__);
      else {
	type = (type == IDM_EDITTEXT) ? 8 : (type == IDM_EDITBINARY) ? 16 : 0;
	type |= 4;
	StartMLEEditor(hwndParent, type, filename, hwndFrame);
      }
      break;
    }
    return;
  }

  if (ExecAssociation(hwnd, filename) == -1 &&
      CheckDriveSpaceAvail(ArcTempRoot, ullDATFileSpaceNeeded, ullTmpSpaceNeeded) != 2) {
    hwndArc = StartArcCnr((fExternalArcboxes || !swp ||
			   strcmp(realappname, FM3Str)) ?
			  HWND_DESKTOP :
			  hwndParent, hwndFrame, filename, 4, NULL);
    if (!hwndArc) {
      if (!fCheckMM || !ShowMultimedia(filename)) {
	if (!IsExecutable(filename) || !ExecFile(hwnd, filename)) {
	  p = strrchr(filename, '.');
	  if (!p)
	    p = ".";
	  if (stricmp(p, ".INI") || !StartIniEditor(hwndParent, filename, 4)) {
	    if (stricmp(p, ".HLP") || !ViewHelp(filename)) {
            ViewIt:
              if (TestBinary(filename)) {
                if (*binview) {
                  dummy[0] = filename;
                  dummy[1] = NULL;
                  ExecOnList(hwnd,
                             binview,
                             WINDOWED | SEPARATE |
                             ((fViewChild) ? CHILD : 0), NULL, dummy, NULL,
                             pszSrcFile, __LINE__);
                }
                else if (fUseNewViewer) {
                  if (fExternalViewer || strcmp(realappname, FM3Str))
                    hwndParent = HWND_DESKTOP;
                  StartViewer(hwndParent, 5, filename, hwndFrame);
                }
                else
                  StartMLEEditor(hwndParent, 5, filename, hwndFrame);
              }
              else {
                if (*viewer) {
                  dummy[0] = filename;
                  dummy[1] = NULL;
                  ExecOnList(hwnd,
                             viewer,
                             WINDOWED | SEPARATE |
                             ((fViewChild) ? CHILD : 0), NULL, dummy, NULL,
                             pszSrcFile, __LINE__);
                }
                else if (fUseNewViewer) {
                  if (fExternalViewer || strcmp(realappname, FM3Str))
                    hwndParent = HWND_DESKTOP;
                  StartViewer(hwndParent, 5, filename, hwndFrame);
                }
                else
                  StartMLEEditor(hwndParent, 5, filename, hwndFrame);
              }
	    }
	  }
	}
      }
    }
    else {
      if ((swp &&
	   !fExternalArcboxes &&
	   !strcmp(realappname, FM3Str)) &&
	  !ParentIsDesktop(hwnd, hwndParent))
	WinSetWindowPos(hwndArc,
			HWND_TOP,
			swp->x,
			swp->y,
			swp->cx,
			swp->cy,
			SWP_MOVE | SWP_SIZE | SWP_SHOW |
			SWP_ZORDER | SWP_ACTIVATE);
    }
  }
}

#pragma alloc_text(DEFVIEW,DefaultView,ShowMultimedia,DefaultViewKeys)
