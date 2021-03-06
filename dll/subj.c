
/***********************************************************************

  $Id$

  Edit .subject EAs

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004, 2008 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  17 Jul 06 SHL Use Runtime_Error
  06 Aug 07 GKY Increase Subject EA to 1024
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  01 Sep 07 GKY Use xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundry
  08 Mar 09 GKY Additional strings move to PCSZs
  12 Jul 09 GKY Add xDosQueryAppType and xDosAlloc... to allow FM/2 to load in high memory
  26 Aug 11 GKY Add a low mem version of xDosAlloc* wrappers; move error checking into all the
                xDosAlloc* wrappers.

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_LONGLONG

#include "fm3dll.h"
#include "info.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "init.h"			// Data declaration(s)
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "input.h"			// InputDlgProc
#include "subj.h"
#include "wrappers.h"			// xDosSetPathInfo
#include "strips.h"			// bstrip
#include "fortify.h"

// Data definitions
static PSZ pszSrcFile = __FILE__;

#pragma data_seg(GLOBAL2)
PCSZ SUBJECT  = ".SUBJECT";

INT Subject(HWND hwnd, CHAR * filename)
{
  APIRET rc;
  EAOP2 eaop;
  PGEA2LIST pgealist;
  PFEA2LIST pfealist;
  PGEA2 pgea;
  PFEA2 pfea;
  CHAR *value, subject[1024], oldsubject[1024];
  STRINGINPARMS sip;
  INT ret = 0;

  *subject = 0;
  pgealist = xmallocz(sizeof(GEA2LIST) + 64, pszSrcFile, __LINE__);
  if (pgealist) {
    pgea = &pgealist->list[0];
    strcpy(pgea->szName, SUBJECT);
    pgea->cbName = strlen(pgea->szName);
    pgea->oNextEntryOffset = 0;
    pgealist->cbList = (sizeof(GEA2LIST) + pgea->cbName);
    pfealist = xmallocz(1024, pszSrcFile, __LINE__);
    if (pfealist)
    free(pgealist);
    else {
      pfealist->cbList = 1024;
      eaop.fpGEA2List = pgealist;
      eaop.fpFEA2List = pfealist;
      eaop.oError = 0;
      rc = DosQueryPathInfo(filename, FIL_QUERYEASFROMLIST,
			    (PVOID) & eaop, (ULONG) sizeof(EAOP2));
      free(pgealist);
      if (!rc) {
	pfea = &eaop.fpFEA2List->list[0];
	value = pfea->szName + pfea->cbName + 1;
	value[pfea->cbValue] = 0;
	if (*(USHORT *) value == EAT_ASCII)
	  strncpy(subject, value + (sizeof(USHORT) * 2), 1023);
	subject[1023] = 0;
      }
      free(pfealist);
      if (rc == ERROR_SHARING_VIOLATION || rc == ERROR_ACCESS_DENIED) {
	saymsg(MB_CANCEL,
	       hwnd,
	       GetPString(IDS_OOPSTEXT),
	       GetPString(IDS_EASBUSYTEXT), filename);
	return 2;			// Error
      }
      else if (rc) {
	Dos_Error(MB_CANCEL, rc, hwnd, pszSrcFile, __LINE__,
		  PCSZ_DOSQUERYPATHINFO);
	return 2;			// Error
      }
    }
  }
  memset(&sip, 0, sizeof(sip));
  strcpy(oldsubject, subject);
  sip.help = GetPString(IDS_SUBJECTINPUTHELPTEXT);
  sip.ret = subject;
  sip.prompt = GetPString(IDS_SUBJECTINPUTPROMPTTEXT);
  sip.inputlen = 1024;
  sip.title = filename;
  if (WinDlgBox
      (HWND_DESKTOP, hwnd, InputDlgProc, FM3ModHandle, STR_FRAME, &sip)
      && isalpha(*filename)
      && !(driveflags[toupper(*filename) - 'A'] & DRIVE_NOTWRITEABLE)) {
    subject[1023] = 0;
    bstrip(subject);
    if (strcmp(oldsubject, subject)) {

      ULONG ealen;
      USHORT len;
      CHAR *eaval;

      len = strlen(subject);
      if (len)
	ealen = sizeof(FEA2LIST) + 9 + len + 4;
      else
	ealen = sizeof(FEALIST) + 9;
      if (!xDosAllocMem((PPVOID) & pfealist, ealen + 1L, pszSrcFile, __LINE__)) {
	memset(pfealist, 0, ealen + 1);
	pfealist->cbList = ealen;
	pfealist->list[0].oNextEntryOffset = 0L;
	pfealist->list[0].fEA = 0;
	pfealist->list[0].cbName = 8;
	strcpy(pfealist->list[0].szName, SUBJECT);
	if (len) {
	  eaval = pfealist->list[0].szName + 9;
	  *(USHORT *) eaval = (USHORT) EAT_ASCII;
	  eaval += sizeof(USHORT);
	  *(USHORT *) eaval = (USHORT) len;
	  eaval += sizeof(USHORT);
	  memcpy(eaval, subject, len);
	  pfealist->list[0].cbValue = len + (sizeof(USHORT) * 2);
	}
	else
	  pfealist->list[0].cbValue = 0;
	eaop.fpGEA2List = (PGEA2LIST) 0;
	eaop.fpFEA2List = pfealist;
	eaop.oError = 0;
	rc = xDosSetPathInfo(filename, FIL_QUERYEASIZE,
			     &eaop, sizeof(eaop), DSPI_WRTTHRU);
	DosFreeMem(pfealist);
	if (rc) {
	  Dos_Error(MB_ENTER, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
		    GetPString(IDS_ERRORSETTINGSUBJECTTEXT), filename);
	}
	else
	  ret = 1;			// OK
      }
    }
  }
  return ret;				// No change?
}

#pragma alloc_text(FMINPUT,Subject)
