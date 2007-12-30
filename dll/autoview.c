
/***********************************************************************

  $Id$

  Auto view

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2006 Steven H.Levine

  12 Sep 02 SHL AutoObjProc: catch buff2 overflows
  25 Oct 02 SHL CreateHexDump: catch buffer overflow
  12 Feb 03 SHL AutoObjProc: standardize EA math
  23 May 05 SHL Use QWL_USER
  29 May 06 SHL Sync with archiver.bb2 mods
  22 Jul 06 SHL Check more run time errors
  15 Aug 06 SHL Use Runtime_Error more
  03 Nov 06 SHL Renames
  30 Mar 07 GKY Remove GetPString for window class names
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  01 Sep 07 GKY Use xDosSetPathInfo to fix case where FS3 buffer crosses 64k boundry
  27 Sep 07 SHL Correct ULONGLONG size formatting
  22 Nov 07 GKY Use CopyPresParams to fix presparam inconsistencies in menus
  30 Dec 07 GKY Use CommaFmtULL

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <process.h>			// _beginthread

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"

#pragma data_seg(DATA1)

static PSZ pszSrcFile = __FILE__;

static HWND hwndAutoObj;
static CHAR stopflag;
static CHAR currfile[CCHMAXPATH];

BOOL WriteEA(HWND hwnd, CHAR * filename, CHAR * eaname, USHORT type,
	     CHAR * data)
{
  /* save an ea to disk */

  FEA2LIST *pfealist = NULL;
  EAOP2 eaop;
  APIRET rc;
  ULONG ealen;
  USHORT len, *num, *plen, usCodepage;
  CHAR *p, *eaval;
  BOOL ret = FALSE;

  if (!filename || !eaname)
    return ret;
  usCodepage = (USHORT) WinQueryCp(WinQueryWindowULong(hwnd, QWL_HMQ));
  len = (data) ? strlen(data) : 0;
  ealen = sizeof(FEA2LIST) + 24L + strlen(eaname) + 1L + (ULONG) len + 4L;
  switch (type) {
  case EAT_EA:
  case EAT_ASCII:
    break;
  case EAT_MVST:
    if (data) {
      ealen += sizeof(USHORT) * 5;
      p = data;
      while (*p) {
	if (*p == '\n' && *(p + 1))
	  ealen += sizeof(USHORT);
	p++;
      }
    }
    break;
  case EAT_MVMT:
    if (data) {
      ealen += sizeof(USHORT) * 5;
      p = data;
      while (*p) {
	if (*p == '\n' && *(p + 1))
	  ealen += (sizeof(USHORT) * 2);
	p++;
      }
    }
    break;
  default:
    return ret;
  }

  rc = DosAllocMem((PPVOID) & pfealist, ealen, PAG_COMMIT | PAG_READ |
		   PAG_WRITE | OBJ_TILE);
  if (rc || !pfealist)
    Dos_Error(MB_CANCEL, rc, hwnd, pszSrcFile, __LINE__,
	      GetPString(IDS_OUTOFMEMORY));
  else {
    memset(pfealist, 0, ealen);
    pfealist->list[0].cbName = strlen(eaname);
    memcpy(pfealist->list[0].szName, eaname, pfealist->list[0].cbName + 1);
    eaval = pfealist->list[0].szName + pfealist->list[0].cbName + 1;
    switch (type) {
    case EAT_EA:
    case EAT_ASCII:
      if (data) {
	*(USHORT *) eaval = (USHORT) type;
	eaval += sizeof(USHORT);
	*(USHORT *) eaval = (USHORT) len;
	eaval += sizeof(USHORT);
	memcpy(eaval, data, len);
	eaval += len;
      }
      break;
    case EAT_MVST:
      if (data) {
	*(USHORT *) eaval = (USHORT) EAT_MVST;
	eaval += sizeof(USHORT);
	*(USHORT *) eaval = usCodepage;
	eaval += sizeof(USHORT);
	num = (USHORT *) eaval;
	*num = 0;
	eaval += sizeof(USHORT);
	*(USHORT *) eaval = (USHORT) EAT_ASCII;
	eaval += sizeof(USHORT);
	plen = (USHORT *) eaval;
	*plen = 0;
	eaval += sizeof(USHORT);
	p = data;
	while (*p) {
	  while (*p) {
	    if (*p == '\n') {
	      p++;
	      break;
	    }
	    *eaval++ = *p++;
	    (*plen)++;
	  }
	  if (*p || *plen)
	    (*num)++;
	  if (*p) {
	    plen = (USHORT *) eaval;
	    *plen = 0;
	    eaval += sizeof(USHORT);
	  }
	}
      }
      break;
    case EAT_MVMT:
      if (data) {
	*(USHORT *) eaval = (USHORT) EAT_MVMT;
	eaval += sizeof(USHORT);
	*(USHORT *) eaval = usCodepage;
	eaval += sizeof(USHORT);
	num = (USHORT *) eaval;
	*num = 0;
	eaval += sizeof(USHORT);
	*(USHORT *) eaval = (USHORT) EAT_ASCII;
	eaval += sizeof(USHORT);
	plen = (USHORT *) eaval;
	*plen = 0;
	eaval += sizeof(USHORT);
	p = data;
	while (*p) {
	  while (*p) {
	    if (*p == '\n') {
	      p++;
	      break;
	    }
	    *eaval++ = *p++;
	    (*plen)++;
	  }
	  if (*p || *plen)
	    (*num)++;
	  if (*p) {
	    *(USHORT *) eaval = (USHORT) EAT_ASCII;
	    eaval += sizeof(USHORT);
	    plen = (USHORT *) eaval;
	    *plen = 0;
	    eaval += sizeof(USHORT);
	  }
	}
      }
      break;
    }
    pfealist->list[0].cbValue = (ULONG) (eaval -
					 (pfealist->list[0].szName +
					  pfealist->list[0].cbName + 1));
    memset(&eaop, 0, sizeof(eaop));
    eaop.fpFEA2List = pfealist;
    pfealist->cbList = 13 + (ULONG) pfealist->list[0].cbName +
      (ULONG) pfealist->list[0].cbValue;
    rc = xDosSetPathInfo(filename, FIL_QUERYEASIZE,
			 &eaop, sizeof(eaop), DSPI_WRTTHRU);
    DosFreeMem(pfealist);
    if (!rc)
      ret = TRUE;
  }
  return ret;
}

BOOL PutComments(HWND hwnd, CHAR * filename, CHAR * comments)
{
  register CHAR *p;

  if (comments) {			/* check -- is it empty? */
    p = comments;
    while (*p && isspace(*p))
      p++;
    if (!*p)
      comments = NULL;
  }
  return WriteEA(hwnd, filename, ".COMMENTS", EAT_MVMT, comments);
}

static PSZ pszBufOvfMsg = "Buffer overflow";

ULONG CreateHexDump(CHAR * pchInBuf, ULONG cbInBuf,
		    CHAR * pchOutBuf, ULONG cbOutBuf,
		    ULONG cOffset, BOOL fLongAddr)
{
  register CHAR *pchIn, *pchReset, *pchOut;
  register ULONG ibIn = 0, ibIn2, ibInFill;
  ULONG cbOutLine = 6 + (fLongAddr ? 4 : 0) + 16 * 3 + 1 + 16 + 1;

  if (pchInBuf && cbInBuf && pchOutBuf && cbOutBuf) {
    pchIn = pchInBuf;
    pchOut = pchOutBuf;
    if (cOffset)
      *pchOut++ = '\n';
    while (ibIn < cbInBuf) {
      if (pchOut - pchOutBuf + cbOutLine >= cbOutBuf) {
	Runtime_Error(pszSrcFile, __LINE__, pszBufOvfMsg);
	break;
      }
      pchReset = pchIn;
      ibIn2 = ibIn;
      if (fLongAddr)
	sprintf(pchOut, "%08lx  ", ibIn + cOffset);
      else
	sprintf(pchOut, "%04lx  ", ibIn + cOffset);
      pchOut += 6 + (fLongAddr ? 4 : 0);
      do {
	sprintf(pchOut, "%02x ", (UCHAR)*pchIn);
	pchOut += 3;
	pchIn++;
	ibIn++;
      } while (ibIn < cbInBuf && (ibIn % 16));
      if (ibIn % 16) {
	ibInFill = ibIn;
	while (ibInFill % 16) {
	  *pchOut++ = ' ';
	  *pchOut++ = ' ';
	  *pchOut++ = ' ';
	  ibInFill++;
	}
      }
      *pchOut++ = ' ';
      pchIn = pchReset;
      do {
	if (*pchIn && *pchIn != '\n' && *pchIn != '\r' && *pchIn != '\t'
	    && *pchIn != '\x1a')
	  *pchOut++ = *pchIn++;
	else {
	  *pchOut++ = '.';
	  pchIn++;
	}
	ibIn2++;
      } while (ibIn2 < ibIn);
      if (ibIn < cbInBuf)
	*pchOut++ = '\n';
    }					// while ibIn
    *pchOut = 0;
    return (ULONG) (pchOut - pchOutBuf);
  }
  return 0L;
}

MRESULT EXPENTRY AutoObjProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg) {
  case UM_LOADFILE:
    *currfile = 0;
    stopflag--;
    if (fAutoView) {
      WinSetWindowText((fComments) ? hwndAutoMLE : hwndAutoview, "");
      MLEsetreadonly(hwndAutoMLE, TRUE);
      MLEsetchanged(hwndAutoMLE, FALSE);
    }
    if (mp1) {
      if (fAutoView) {
	strcpy(currfile, (CHAR *) mp1);
	if (!fComments) {
	  if (IsFile((CHAR *) mp1) == 1) {

	    HFILE handle;
	    ULONG olen, ibufflen, action, obufflen, l;
	    CHAR *ibuff, *obuff, *p;
	    // 06 Oct 07 SHL Protect against NTFS driver small buffer defect
	    // CHAR buffer[80];
	    CHAR buffer[4096];
	    ARC_TYPE *info;

	    if (!DosOpen((CHAR *) mp1,
			 &handle,
			 &action,
			 0,
			 0,
			 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
			 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NOINHERIT |
			 OPEN_FLAGS_RANDOMSEQUENTIAL | OPEN_SHARE_DENYNONE |
			 OPEN_ACCESS_READONLY, 0)) {
	      ibufflen = AutoviewHeight < 96 ? 512 : 3072;
	      // 06 Oct 07 SHL protect against NTFS driver small buffer defect
	      // ibuff = xmalloc(ibufflen + 2, pszSrcFile, __LINE__);	// 05 Nov 07 SHL
	      ibuff = xmalloc(max(ibufflen + 2, 4096), pszSrcFile, __LINE__);
	      if (ibuff) {
		// Depends on CreateHexDump line width
		obufflen = (ibufflen / 16) * (6 + 3 * 16 + 1 + 16 + 1) + 80;
		obuff = xmalloc(obufflen + 1, pszSrcFile, __LINE__);
		if (obuff) {
		  *obuff = 0;
		  if (!DosRead(handle, ibuff, ibufflen, &ibufflen) &&
			       ibufflen)
		  {
		    ibuff[ibufflen] = 0;
		    p = obuff;
		    if (IsBinary(ibuff, ibufflen)) {
		      olen = ibufflen;
		      // Check archive
		      if (!arcsigsloaded)
			load_archivers();
		      info = arcsighead;
		      while (info) {
			if (info->signature && *info->signature) {
			  l = strlen(info->signature);
			  l = min(l, 79);
			  if (!DosChgFilePtr(handle, abs(info->file_offset),
					     (info->file_offset >= 0L) ?
					     FILE_BEGIN :
					     FILE_END, &ibufflen)) {
			    if (!DosRead(handle,
					 buffer,
					 l, &ibufflen) && ibufflen == l) {
			      if (!memcmp(info->signature, buffer, l))
				break;
			    }
			  }
			}
			info = info->next;
		      }
		      if (info) {
			sprintf(p, "**%s%s%s\n",
				info->id ? info->id : "",
				info->id ? " " : "",
				GetPString(IDS_ARCHIVETEXT));
			p += strlen(p);
		      }
		      CreateHexDump(ibuff,	// Input buffer
				    olen,	// Input buffer size
				    p,	// Output buffer
				    obufflen - (p - obuff),	// Output buffer size
				    0,	// Address offest
				    FALSE);	// Short addresses
		    }
		    else {
		      // Text file
		      register CHAR *src, *dest;
		      CHAR *endsrc;

		      src = ibuff;
		      dest = obuff;
		      endsrc = ibuff + ibufflen;
		      while (src < endsrc) {
			if (dest == obuff && (*src == '\t' || *src == ' ' ||
					      *src == '\r' || *src == '\n')) {
			  src++;
			}
			else if (*src == '\t' || !*src) {
			  *dest = ' ';
			  dest++;
			  src++;
			}
			else if (*src == '\r' || *src == '\n') {
			  *dest = *src;
			  while (*(src + 1) == '\r' || *(src + 1) == '\n' ||
				 *(src + 1) == ' ' || *(src + 1) == '\t')
			    src++;
			  dest++;
			  src++;
			}
			else {
			  *dest = *src;
			  src++;
			  dest++;
			}
		      }			// while
		      *dest = 0;
		      if (dest - obuff >= obufflen)
			Runtime_Error(pszSrcFile, __LINE__, pszBufOvfMsg);
		    }
		    if (*obuff)
		      WinSetWindowText(hwndAutoview, obuff);
		  }
		  free(obuff);
		}
		free(ibuff);
	      }
	      DosClose(handle);
	    }
	  }
	  else if (!IsFile(currfile)) {

	    static FILEFINDBUF4L ffb[130];
	    CHAR fullname[CCHMAXPATH + 4], szCmmaFmtFileSize[81];
	    HDIR hdir = HDIR_CREATE;
	    ULONG x, nm, ml, mc, bufflen;
	    PBYTE fb;
	    PFILEFINDBUF4L pffbFile;
	    PSZ pszBuf;
	    PSZ p;
	    APIRET rc;

	    BldFullPathName(fullname, currfile, "*");
	    //sprintf(fullname,
	    //        "%s%s*",
	    //        currfile,
	    //        (currfile[strlen(currfile) - 1] == '\\') ? "" : "\\");
	    DosError(FERR_DISABLEHARDERR);
	    nm = sizeof(ffb) / sizeof(FILEFINDBUF4L);
	    if (AutoviewHeight < 96)
	      nm /= 2;
	    rc = xDosFindFirst(fullname,
			       &hdir,
			       FILE_NORMAL | FILE_DIRECTORY |
			       FILE_READONLY | FILE_ARCHIVED |
			       FILE_SYSTEM | FILE_HIDDEN,
			       &ffb, sizeof(ffb), &nm, FIL_QUERYEASIZEL);
	    if (!rc && nm) {
	      fb = (PBYTE) & ffb;
	      x = ml = 0;
	      while (x < nm) {
		pffbFile = (PFILEFINDBUF4L) fb;
		mc = (ULONG) pffbFile->cchName +
		  ((pffbFile->attrFile & FILE_DIRECTORY) != 0);
		ml = max(ml, mc);
		if (!pffbFile->oNextEntryOffset)
		  break;
		fb += pffbFile->oNextEntryOffset;
		x++;
	      }
	      bufflen = (CCHMAXPATHCOMP + 42) * nm;
	      pszBuf = xmalloc(bufflen, pszSrcFile, __LINE__);
	      if (pszBuf) {
		p = pszBuf;
		*p = 0;
		fb = (PBYTE) & ffb;
		x = 0;
		while (x < nm) {
		  pffbFile = (PFILEFINDBUF4L) fb;
		  if (!(!*pffbFile->achName ||
			(((pffbFile->attrFile & FILE_DIRECTORY) &&
			  pffbFile->achName[0] == '.') &&
			 (!pffbFile->achName[1] ||
			  (pffbFile->achName[1] == '.' &&
			   !pffbFile->achName[2]))))) {
                    CommaFmtULL(szCmmaFmtFileSize,
                                sizeof(szCmmaFmtFileSize),
                                pffbFile->cbFile + CBLIST_TO_EASIZE(pffbFile->cbList),
                                ' ');
		    sprintf(p,
			    "%s%-*.*s  %-8s  [%s%s%s%s]  %04lu/%02lu/%02lu "
			      "%02lu:%02lu:%02lu\r",
			    pffbFile->attrFile & FILE_DIRECTORY ? "\\" : " ",
			    ml,
			    ml,
                            pffbFile->achName,
                            szCmmaFmtFileSize,
			    pffbFile->attrFile & FILE_READONLY ? "R" : "-",
			    pffbFile->attrFile & FILE_ARCHIVED ? "A" : "-",
			    pffbFile->attrFile & FILE_HIDDEN ? "H" : "-",
			    pffbFile->attrFile & FILE_SYSTEM ? "S" : "-",
			    pffbFile->fdateLastWrite.year + 1980,
			    pffbFile->fdateLastWrite.month,
			    pffbFile->fdateLastWrite.day,
			    pffbFile->ftimeLastWrite.hours,
			    pffbFile->ftimeLastWrite.minutes,
			    pffbFile->ftimeLastWrite.twosecs * 2);
		    p += strlen(p);
		  }
		  if (!pffbFile->oNextEntryOffset)
		    break;
		  fb += pffbFile->oNextEntryOffset;
		  x++;
		}			// while
		if (p - pszBuf >= bufflen)
		  Runtime_Error(pszSrcFile, __LINE__, pszBufOvfMsg);
		if (*pszBuf)
		  WinSetWindowText(hwndAutoview, pszBuf);
		free(pszBuf);
	      }
	    }
	    if (!rc)
	      DosFindClose(hdir);
	  }
	}
	else {

	  APIRET rc;
	  EAOP2 eaop;
	  PGEA2LIST pgealist;
	  PFEA2LIST pfealist;
	  PGEA2 pgea;
	  PFEA2 pfea;
	  CHAR *value, *pszBuf, *p, *data;
	  USHORT len, type, plen, dlen;
	  BOOL readonly = FALSE;

	  pgealist = xmallocz(sizeof(GEA2LIST) + 64, pszSrcFile, __LINE__);
	  if (pgealist) {
	    pgea = &pgealist->list[0];
	    strcpy(pgea->szName, ".COMMENTS");
	    pgea->cbName = strlen(pgea->szName);
	    pgea->oNextEntryOffset = 0L;
	    pgealist->cbList = (sizeof(GEA2LIST) + pgea->cbName);
	    pfealist = xmallocz(65536, pszSrcFile, __LINE__);
	    if (pfealist) {
	      pfealist->cbList = 65536;
	      eaop.fpGEA2List = pgealist;
	      eaop.fpFEA2List = pfealist;
	      eaop.oError = 0L;
	      rc = DosQueryPathInfo((CHAR *) mp1, FIL_QUERYEASFROMLIST,
				    (PVOID) & eaop, (ULONG) sizeof(EAOP2));
	      free(pgealist);
	      if (!rc) {
		pfea = &eaop.fpFEA2List->list[0];
		if (pfea->cbName && pfea->cbValue) {
		  value = pfea->szName + pfea->cbName + 1;
		  value[pfea->cbValue] = 0;
		  if (*(USHORT *) value == EAT_MVMT) {
		    pszBuf = xmalloc(65536, pszSrcFile, __LINE__);
		    if (pszBuf) {
		      p = pszBuf;
		      *pszBuf = 0;
		      data = value + (sizeof(USHORT) * 3);
		      type = *(USHORT *) data;
		      data += sizeof(USHORT);
		      len = *(USHORT *) data;
		      data += sizeof(USHORT);
		      while ((data - value) - len < pfea->cbValue) {
			if (type == EAT_ASCII) {
			  dlen = plen = 0;
			  while (dlen < len) {
			    if (data[dlen] == '\r') {
			      dlen++;
			      if (dlen < len && data[dlen] != '\n')
				p[plen++] = '\n';
			    }
			    else
			      p[plen++] = data[dlen++];
			  }
			  if ((!len || !plen || p[plen - 1] != '\n') &&
			      (data - value) + len < pfea->cbValue)
			    p[plen++] = '\n';
			  p += plen;
			  *p = 0;
			}
			else
			  readonly = TRUE;
			data += len;
			if (data - value >= pfea->cbValue)
			  break;
			type = *(USHORT *) data;
			data += sizeof(USHORT);
			len = *(USHORT *) data;
			data += sizeof(USHORT);
		      }			// while
		      if (p - pszBuf >= 65536) {
			Runtime_Error(pszSrcFile, __LINE__, pszBufOvfMsg);
			pszBuf[65535] = 0;	// Try to stay alive
			break;
		      }
		      WinSetWindowText(hwndAutoMLE, pszBuf);
		      free(pszBuf);
		    }
		  }
		  else
		    readonly = TRUE;
		}
		/* else EA not present */
		MLEsetchanged(hwndAutoMLE, FALSE);
		MLEsetreadonly(hwndAutoMLE, readonly);
	      }
	      else {
		MLEsetchanged(hwndAutoMLE, FALSE);
		MLEsetreadonly(hwndAutoMLE, FALSE);
	      }
	      free(pfealist);
	    }
	  }
	}
      }
      free((CHAR *) mp1);
    }
    return 0;

  case UM_CLOSE:
    if (!PostMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID))
      WinSendMsg((HWND) 0, WM_QUIT, MPVOID, MPVOID);
    WinDestroyWindow(hwnd);
    return 0;
  }
  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

static VOID MakeAutoWinThread(VOID * args)
{
  HAB hab2;
  HMQ hmq2;
  HWND hwndParent = (HWND) args;
  QMSG qmsg2;

  hab2 = WinInitialize(0);
  if (hab2) {
    hmq2 = WinCreateMsgQueue(hab2, 128);
    if (hmq2) {
      DosError(FERR_DISABLEHARDERR);
      WinRegisterClass(hab2,
		       WC_OBJECTWINDOW,
		       AutoObjProc, 0, sizeof(PVOID));
      hwndAutoObj = WinCreateWindow(HWND_OBJECT,
				    WC_OBJECTWINDOW,
				    (PSZ) NULL,
				    0,
				    0L,
				    0L,
				    0L,
				    0L, 0L, HWND_TOP, OBJ_FRAME, NULL, NULL);
      if (!hwndAutoObj) {
	Win_Error2(HWND_OBJECT, HWND_DESKTOP, pszSrcFile, __LINE__,
		   IDS_WINCREATEWINDOW);
	if (!PostMsg(hwndParent, UM_CLOSE, MPVOID, MPVOID))
	  WinSendMsg(hwndParent, UM_CLOSE, MPVOID, MPVOID);
      }
      else {
	WinSetWindowULong(hwndAutoObj, QWL_USER, hwndParent);
	priority_normal();
	while (WinGetMsg(hab2, &qmsg2, (HWND) 0, 0, 0))
	  WinDispatchMsg(hab2, &qmsg2);
	WinDestroyWindow(hwndAutoObj);
	hwndAutoObj = (HWND) 0;
      }
      WinDestroyMsgQueue(hmq2);
    }
    else
      WinTerminate(hab2);
  }
}

MRESULT EXPENTRY AutoViewProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  USHORT id = WinQueryWindowUShort(hwnd, QWS_ID);

  switch (msg) {
  case WM_CREATE:
    {
      MRESULT mr;

      if (!hwndAutoObj) {
	if (_beginthread(MakeAutoWinThread, NULL, 65536, (PVOID) hwnd) == -1) {
	  Runtime_Error(pszSrcFile, __LINE__,
			GetPString(IDS_COULDNTSTARTTHREADTEXT));
	  PostMsg(hwnd, UM_CLOSE, MPVOID, MPVOID);
	}
      }
      mr = PFNWPStatic(hwnd, msg, mp1, mp2);
      SetPresParams(hwnd,
		    &RGBGREY,
		    &RGBBLACK, &RGBGREY, GetPString(IDS_4SYSTEMVIOTEXT));
      stopflag = 0;
      return mr;
    }

  case UM_SETUP:
    MLEsetlimit(hwnd, 32768);
    MLEsetformat(hwnd, MLFIE_NOTRANS);
    MLEsetchanged(hwnd, FALSE);
    return 0;

  case WM_BUTTON1DOWN:
    {
      SWP swp;

      WinQueryWindowPos(hwnd, &swp);
      if (SHORT2FROMMP(mp1) > swp.cy - 4) {

	HPS hps = WinGetPS(WinQueryWindow(hwnd, QW_PARENT));
	SWP swpC;
	TRACKINFO track;

	if (hps) {
	  WinQueryWindowPos(WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
					    FID_CLIENT), &swpC);
	  track.cxBorder = 4;
	  track.cyBorder = 4;
	  track.cxGrid = 1;
	  track.cyGrid = 8;
	  track.cxKeyboard = 8;
	  track.rclTrack.yBottom = swp.y;
	  track.rclTrack.yTop = swp.y + swp.cy;
	  track.rclTrack.xLeft = swp.x;
	  track.rclTrack.xRight = swp.x + swp.cx;
	  track.rclBoundary = track.rclTrack;
	  track.rclBoundary.yTop = (swpC.cy + swp.y + swp.cy) - 116;
	  track.ptlMinTrackSize.x = swp.x + swp.cx;
	  track.ptlMinTrackSize.y = 36;
	  track.ptlMaxTrackSize.x = swp.x + swp.cx;
	  track.ptlMaxTrackSize.y = (swpC.cy + swp.cy) - 116;
	  track.fs = TF_TOP;
	  if (WinTrackRect(hwnd, hps, &track)) {
	    AutoviewHeight = track.rclTrack.yTop - track.rclTrack.yBottom;
	    PrfWriteProfileData(fmprof,
				FM3Str,
				"AutoviewHeight",
				&AutoviewHeight, sizeof(ULONG));
	    WinSendMsg(WinQueryWindow(hwnd, QW_PARENT),
		       WM_UPDATEFRAME, MPFROMLONG(FCF_SIZEBORDER), MPVOID);
	  }
	  WinReleasePS(hps);
	}
	return (MRESULT) TRUE;
      }
      else if (id != MAIN_AUTOVIEWMLE)
	return CommonTextButton(hwnd, msg, mp1, mp2);
    }
    break;

  case WM_BUTTON3DOWN:
  case WM_BUTTON1UP:
  case WM_BUTTON3UP:
    if (id != MAIN_AUTOVIEWMLE)
      return CommonTextButton(hwnd, msg, mp1, mp2);
    break;

  case WM_MOUSEMOVE:
    shiftstate = (SHORT2FROMMP(mp2) & (KC_ALT | KC_SHIFT | KC_CTRL));
    {
      SWP swp;

      WinQueryWindowPos(hwnd, &swp);
      if (SHORT2FROMMP(mp1) > swp.cy - 4) {
	WinSetPointer(HWND_DESKTOP, hptrNS);
	return (MRESULT) TRUE;
      }
    }
    break;

  case WM_PAINT:
    PostMsg(hwnd, UM_PAINT, MPVOID, MPVOID);
    break;

  case UM_PAINT:
    PaintRecessedWindow(hwnd, (HPS) 0, TRUE, TRUE);
    return 0;

  case WM_SETFOCUS:
    switch (id) {
    case MAIN_AUTOVIEWMLE:
      if (!mp2 && !AutoMenu) {
	if (*currfile) {
	  if (MLEgetchanged(hwnd)) {
	    CHAR *ea = xmalloc(32768, pszSrcFile, __LINE__);

	    if (ea) {
	      *ea = 0;
	      WinQueryWindowText(hwnd, 32767, ea);
	      PutComments(hwnd, currfile, ea);
	      PostMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_RESCAN, 0), MPVOID);
	      free(ea);
	    }
	  }
	}
      }
      break;
    default:
      if (mp2)
	PostMsg(hwnd, UM_FOCUSME, mp1, mp2);
      break;
    }
    break;

  case UM_FOCUSME:
    if (mp2) {

      PID pid;
      TID tid;

      if (WinQueryWindowProcess((HWND) mp1, &pid, &tid) && pid == mypid)
	WinSetFocus(HWND_DESKTOP, (HWND) mp1);
      else
	WinSetFocus(HWND_DESKTOP, hwndTree);
    }
    return 0;

  case UM_CLICKED:
  case UM_CLICKED3:
    if (id != MAIN_AUTOVIEWMLE) {
      PostMsg(hwnd,
	      WM_COMMAND,
	      MPFROM2SHORT(((msg == UM_CLICKED3) ?
			    IDM_EAS : IDM_VIEWORARC), 0), MPVOID);
    }
    return 0;

  case WM_COMMAND:
    PostMsg(hwnd, UM_COMMAND, mp1, mp2);
    return 0;

  case UM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_RESCAN:
      if (*currfile) {

	CHAR *cf = xstrdup(currfile, pszSrcFile, __LINE__);

	if (cf) {
	  stopflag++;
	  if (!PostMsg(hwndAutoObj, UM_LOADFILE, MPFROMP(cf), MPVOID))
	    free(cf);
	}
      }
      break;

    case IDM_INFO:
      DefaultView(hwnd, (HWND) 0, hwndMain, NULL, 16, currfile);
      break;

    case IDM_VIEW:
      DefaultView(hwnd, (HWND) 0, hwndMain, NULL, 0, currfile);
      break;

    case IDM_EDIT:
      DefaultView(hwnd, (HWND) 0, hwndMain, NULL, 8, currfile);
      break;

    case IDM_EAS:
      {
	char *list[2];

	list[0] = currfile;
	list[1] = NULL;

	WinDlgBox(HWND_DESKTOP,
		  hwndMain,
		  DisplayEAsProc, FM3ModHandle, EA_FRAME, (PVOID) list);
      }
      break;

    default:
      PostMsg(hwndMain, msg, mp1, mp2);
    }
    return 0;

  case WM_MENUEND:
    if ((HWND) mp2 == AutoMenu) {
      WinDestroyWindow(AutoMenu);
      AutoMenu = (HWND) 0;
    }
    break;

  case WM_CONTEXTMENU:
    CheckMenu(hwnd, &AutoMenu, (id == MAIN_AUTOVIEWMLE) ?
	      IDM_AUTOVIEWMLE : IDM_AUTOVIEW);
    WinCheckMenuItem(AutoMenu, IDM_AUTOVIEWFILE, !fComments);
    WinCheckMenuItem(AutoMenu, IDM_AUTOVIEWCOMMENTS, fComments);
    WinEnableMenuItem(AutoMenu, IDM_VIEW, (IsFile(currfile) == 1));
    WinEnableMenuItem(AutoMenu, IDM_EDIT, (IsFile(currfile) == 1));
    WinEnableMenuItem(AutoMenu, IDM_INFO, (*currfile != 0));
    PopupMenu(hwnd, hwnd, AutoMenu);
    break;

  case UM_LOADFILE:
    stopflag++;
    if (!PostMsg(hwndAutoObj, msg, mp1, mp2)) {
      if (mp1)
	free((CHAR *) mp1);
    }
    return 0;

  case UM_CLOSE:
    if (AutoMenu) {
      WinDestroyWindow(AutoMenu);
      AutoMenu = (HWND) 0;
    }
    WinDestroyWindow(hwnd);
    return 0;

  case WM_CLOSE:
    WinSendMsg(hwnd, UM_CLOSE, MPVOID, MPVOID);
    return 0;

  case WM_DESTROY:
    if (id != MAIN_AUTOVIEWMLE) {
      if (hwndAutoObj)
	if (!PostMsg(hwndAutoObj, WM_CLOSE, MPVOID, MPVOID))
	  WinSendMsg(hwndAutoObj, WM_CLOSE, MPVOID, MPVOID);
      break;
    }
    break;
  }

  if (id == MAIN_AUTOVIEWMLE)
    return PFNWPMLE(hwnd, msg, mp1, mp2);
  return PFNWPStatic(hwnd, msg, mp1, mp2);
}

#pragma alloc_text(AUTOVIEW,AutoViewProc,CreateHexDump,AutoObjProc)
#pragma alloc_text(AUTOVIEW2,MakeAutoWinThread,WriteEA,PutComments)
