
/***********************************************************************

  $Id$

  MLE text editor/viewer

  Copyright (c) 1993-97 M. Kimes
  Copyright (c) 2004, 2008 Steven H.Levine

  01 Aug 04 SHL Rework lstrip/rstrip usage
  16 Apr 06 SHL MLEexportfile: rework to avoid wrap problems
  14 Jul 06 SHL Use Runtime_Error
  03 Nov 06 SHL Count thread usage
  22 Mar 07 GKY Use QWL_USER
  06 Aug 07 GKY Reduce DosSleep times (ticket 148)
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  26 Aug 07 GKY DosSleep(1) in loops changed to (0)
  17 Dec 07 GKY Make WPURLDEFAULTSETTINGS the fall back for ftp/httprun
  29 Feb 08 GKY Refactor global command line variables to notebook.h
  22 Jun 08 GKY Fixed memory buffer access after it had been freed
  06 Jul 08 GKY Rework LoadThread logic with Steven's help
  10 Dec 08 SHL Integrate exception handler support

***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
// #include <process.h>			// _beginthread

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG

#include "fm3dll.h"
#include "fm3dll2.h"			// #define's for UM_*, control id's, etc.
#include "mle.h"
#include "init.h"			// Data declaration(s)
#include "mainwnd.h"			// Data declaration(s)
#include "newview.h"			// Data declarations
#include "fm3dlg.h"
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "notebook.h"                   // httprun etc
#include "autoview.h"			// CreateHexDump
#include "saveclip.h"			// SaveToClip
#include "common.h"			// DecrThreadUsage, IncrThreadUsage
#include "chklist.h"			// PosOverOkay
#include "valid.h"			// TestBinary
#include "strips.h"			// bstrip
#include "systemf.h"			// runemf2
#include "wrappers.h"			// xfopen
#include "misc.h"			// PostMsg
#include "fortify.h"
#include "excputil.h"			// xbeginthread

static PSZ pszSrcFile = __FILE__;

#define FAKEROT 1
#define DOROT13(c)     (!isalpha((c)))?(c):((((c) >= (char) 'A') && \
	((c) <= (char) 'M')) || (((c) >= (char) 'a') && ((c) <= (char) 'm')))?((c) + (char) 0xd)\
	:((((c) >= (char) 'N') && ((c) <= (char) 'Z')) || (((c) >= (char) 'n') && ((c) <= (char) 'z')))?\
	((c) - (char) 0xd):(c)

/*((FAKEROT==0)?(c):(FAKEROT==1)?(!isalpha((c)))?(c):((((c) >= (char) 'A') && \
	((c) <= (char) 'M')) || (((c) >= (char) 'a') && ((c) <= (char) 'm')))?((c) + (char) 0xd)\
	:((((c) >= (char) 'N') && ((c) <= (char) 'Z')) || (((c) >= (char) 'n') && ((c) <= (char) 'z')))?\
	((c) - (char) 0xd):(c):((c) >= (char) '!') ? ((((c) + (char) 47) > (char) '~') ? ((c) - (char) 47) :\
	((c) + (char) 47)) : (c))*/

LONG MLEgetlinetext(HWND h, LONG l, CHAR * buf, INT maxlen)
{
  /* get text of line l from MLE */

  IPT s, e;

  s = MLEstartofline(h, l);
  e = MLElinelenleft(h, s);
  return MLEtextatpos(h, s, buf, min((INT) e, maxlen));
}

LONG MLEdeleteline(HWND h, LONG l)
{
  /* delete line l from MLE */

  IPT s, e;

  s = MLEstartofline(h, l);
  e = MLElinelenleft(h, s);
  return MLEdelete(h, s, e);
}

LONG MLEdeletecurline(HWND h)
{
  /* delete current line from MLE */

  LONG l;

  l = MLEcurline(h);
  return MLEdeleteline(h, l);
}

LONG MLEdeletetoeol(HWND h)
{
  /* delete from cursor pos to end of line */

  IPT s, e;

  s = MLEcurpos(h);
  e = MLEcurlenleft(h);
  return MLEdelete(h, s, e);
}

VOID MLEclearall(HWND h)
{
  /* remove all text from MLE */
  LONG len;

  len = MLEgetlen(h);
  if (len)
    MLEdelete(h, 0, len);
}

LONG MLEtextatcursor(HWND h, CHAR * buffer, INT buflen)
{
  /* place up to buflen chars of text from cursor pos into buffer
   * return # of chars imported
   */

  IPT i;

  i = MLEcurpos(h);
  return MLEtextatpos(h, i, buffer, buflen);
}

LONG MLEtextatpos(HWND h, IPT i, CHAR * buffer, INT buflen)
{
  /* place up to buflen chars of text from pos i in buffer
   * return # of chars imported
   */

  WinSendMsg(h, MLM_SETIMPORTEXPORT, MPFROMP(buffer),
	     MPFROMLONG((LONG) buflen));
  return (LONG) WinSendMsg(h, MLM_EXPORT,
			   MPFROMP(&i), MPFROMLONG((PLONG) & buflen));
}

LONG MLEsizeofsel(HWND h)
{
  /* return length of selected text */

  IPT cursor, anchor, test;

  cursor = MLEcurpos(h);
  anchor = MLEancpos(h);
  test = min(cursor, anchor);
  /* MLE fakes us out; get real length in bytes */
  return (LONG) WinSendMsg(h, MLM_QUERYFORMATTEXTLENGTH,
			   MPFROMLONG(test),
			   MPFROMLONG((LONG) ((cursor < anchor) ?
					      (anchor - cursor) :
					      (cursor - anchor))));
}

VOID MLEinternet(HWND h, BOOL ftp)
{
  CHAR *temp = NULL;
  IPT ancpos, curpos, here;
  LONG len, oldlen;
  APIRET rc;
  ULONG size;

  len = MLEsizeofsel(h);
  len = min(2048, len);
  oldlen = len;
  if (len) {
    len++;
    rc = DosAllocMem((PVOID) & temp, 4096,
		     PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
    if (rc || !temp)
      Dos_Error(MB_CANCEL, rc, h, pszSrcFile, __LINE__,
		GetPString(IDS_OUTOFMEMORY));
    else {
      ancpos = MLEancpos(h);
      curpos = MLEcurpos(h);
      here = min(curpos, ancpos);
      WinSendMsg(h, MLM_SETIMPORTEXPORT, MPFROMP(temp), MPFROMLONG(len));
      len = (LONG) WinSendMsg(h, MLM_EXPORT, MPFROMP(&here), MPFROMP(&len));
      if (len <= 1)
	Runtime_Error(pszSrcFile, __LINE__, "len <= 1");
      else {
	if (len > oldlen)
	  len--;
	temp[len] = 0;
	bstripcr(temp);
	if (*temp) {
	  if (ftp) {
	    if (fFtpRunWPSDefault) {
	      CHAR WPSDefaultFtpRun[CCHMAXPATH], WPSDefaultFtpRunDir[CCHMAXPATH];

	      size = sizeof(WPSDefaultFtpRun);
	      PrfQueryProfileData(HINI_USERPROFILE, "WPURLDEFAULTSETTINGS",
				  "DefaultBrowserExe", WPSDefaultFtpRun, &size);
	      size = sizeof(WPSDefaultFtpRunDir);
	      PrfQueryProfileData(HINI_USERPROFILE, "WPURLDEFAULTSETTINGS",
				  "DefaultWorkingDir", WPSDefaultFtpRunDir, &size);
	      runemf2(SEPARATE | WINDOWED,
		      h, pszSrcFile, __LINE__,
		      WPSDefaultFtpRunDir,
		      fLibPathStrictFtpRun ? "SET LIBPATHSTRICT=TRUE" : NULL,
		      "%s %s", WPSDefaultFtpRun, temp);
	    }
	    else
	      runemf2(SEPARATE | WINDOWED,
		      h, pszSrcFile, __LINE__,
		      ftprundir, NULL, "%s %s", ftprun, temp);
	  }
	  else
	    if (fHttpRunWPSDefault) {
	      CHAR WPSDefaultHttpRun[CCHMAXPATH], WPSDefaultHttpRunDir[CCHMAXPATH];

	      size = sizeof(WPSDefaultHttpRun);
	      PrfQueryProfileData(HINI_USERPROFILE, "WPURLDEFAULTSETTINGS",
				  "DefaultBrowserExe", WPSDefaultHttpRun, &size);
	      size = sizeof(WPSDefaultHttpRunDir);
	      PrfQueryProfileData(HINI_USERPROFILE, "WPURLDEFAULTSETTINGS",
				  "DefaultWorkingDir", WPSDefaultHttpRunDir, &size);
	      runemf2(SEPARATE | WINDOWED,
		      h, pszSrcFile, __LINE__,
		      WPSDefaultHttpRunDir,
		      fLibPathStrictHttpRun ? "SET LIBPATHSTRICT=TRUE" : NULL,
		      "%s %s", WPSDefaultHttpRun, temp);
	    }
	    else
	      runemf2(SEPARATE | WINDOWED,
		      h, pszSrcFile, __LINE__,
		      httprundir, NULL, "%s %s", httprun, temp);
	}
      }
      DosFreeMem(temp);
    }
  }
}

BOOL MLEdoblock(HWND h, INT action, CHAR * filename)
{
  /* perform action on text in selection */

  register CHAR *p;
  CHAR *sel, *temp = NULL;
  IPT ancpos, curpos, here;
  LONG sellen, oldlen;
  APIRET rc;

  oldlen = MLEsizeofsel(h);
  if (!oldlen)
    return TRUE;
  sel = xmallocz((size_t) (oldlen + 2), pszSrcFile, __LINE__);
  if (!sel)
    return FALSE;
  rc = DosAllocMem((PVOID) & temp, 32768L,
		   PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
  if (rc || !temp) {
    Dos_Error(MB_CANCEL, rc, h, pszSrcFile, __LINE__,
	      GetPString(IDS_OUTOFMEMORY));
    free(sel);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
    DosPostEventSem(CompactSem);
    return FALSE;
  }

  ancpos = MLEancpos(h);
  curpos = MLEcurpos(h);
  here = min(curpos, ancpos);
  p = sel;
  MLEdisable(h);
  while (oldlen > 0) {
    sellen = min(oldlen + 1, 32701);
    WinSendMsg(h, MLM_SETIMPORTEXPORT, MPFROMP(temp), MPFROMLONG(sellen));
    sellen =
      (LONG) WinSendMsg(h, MLM_EXPORT, MPFROMP(&here), MPFROMP(&sellen));
    if (sellen < 1) {
      Runtime_Error(pszSrcFile, __LINE__, "len < 1");
      free(sel);
#     ifdef FORTIFY
      Fortify_LeaveScope();
#      endif
      DosPostEventSem(CompactSem);
      return FALSE;
    }
    if (sellen > min(oldlen, 32700))
      sellen--;
    memcpy(p, temp, sellen);
    p += sellen;
    oldlen -= sellen;
  }
  switch (action) {
  case APPENDCLIP:
    SaveToClip(h, sel, TRUE);
    DosFreeMem(temp);
    free(sel);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
    MLEenable(h);
    DosPostEventSem(CompactSem);
    return TRUE;

  case WRITE:
    {
      FILE *fp;

      fp = fopen(filename, "a+");
      if (!fp)
	fp = xfopen(filename, "w", pszSrcFile, __LINE__);
      if (fp) {
	fseek(fp, 0L, SEEK_END);
	fwrite(sel, 1, strlen(sel), fp);
	fclose(fp);
      }
#ifdef __DEBUG_ALLOC__
      _heap_check();
#endif
      DosFreeMem(temp);
      free(sel);
#     ifdef FORTIFY
      Fortify_LeaveScope();
#      endif
      MLEenable(h);
      DosPostEventSem(CompactSem);
      return TRUE;
    }

  case UPPERCASE:
    p = sel;
    while (*p) {
      if (isalpha(*p))
	*p = toupper(*p);
      p++;
    }
    break;

  case LOWERCASE:
    p = sel;
    while (*p) {
      if (isalpha(*p))
	*p = tolower(*p);
      p++;
    }
    break;

  case TOGGLECASE:
    p = sel;
    while (*p) {
      if (isalpha(*p)) {
	if (islower(*p))
	  *p = toupper(*p);
	else
	  *p = tolower(*p);
      }
      p++;
    }
    break;

  case ROT13:
    p = sel;
    while (*p) {
      *p = DOROT13(*p); // fixme condition both true and false?
      p++;
    }
    break;

  case XOR:
    p = sel;
    while (*p) {
      *p = (~*p);
      p++;
    }
    break;

  case FORMAT:
    p = sel;
    while (*p) {
      if (*p == '\r') {
	memmove(p, p + 1, strlen(p));
	continue;
      }
      if (*p == '\n') {
	*p = ' ';
	continue;
      }
      p++;
    }
    break;

  default:				/* unknown action */
#ifdef __DEBUG_ALLOC__
    _heap_check();
#endif
    DosFreeMem(temp);
    free(sel);
#   ifdef FORTIFY
    Fortify_LeaveScope();
#    endif
    DosPostEventSem(CompactSem);
    MLEenable(h);
    return FALSE;
  }

  /* replace selection with altered text */
  p = sel;
  here = min(curpos, ancpos);
  MLEclear(h);				/* delete current selection */
  sellen = oldlen = strlen(sel);	/* actual number of bytes */
  while (oldlen > 0) {
    sellen = min(oldlen, 32700);
    memcpy(temp, p, sellen);
    WinSendMsg(h, MLM_SETIMPORTEXPORT, MPFROMP(temp), MPFROMLONG(sellen));
    sellen = (LONG) WinSendMsg(h,
			       MLM_IMPORT,
			       MPFROMP(&here), MPFROMLONG(sellen));
    if (!sellen) {
      Runtime_Error(pszSrcFile, __LINE__, "sellen 0");
      break;
    }
    p += sellen;
    oldlen -= sellen;
    if (oldlen && *p == '\n' /* && *(p - 1) == '\r' */ )
      p--;
  }					// while
  WinSendMsg(h, MLM_SETSEL, MPFROMLONG(ancpos), MPFROMLONG(curpos));
  MLEenable(h);
#ifdef __DEBUG_ALLOC__
  _heap_check();
#endif
  DosFreeMem(temp);
  free(sel);
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
  DosPostEventSem(CompactSem);
  return TRUE;
}

BOOL MLEquotepara(HWND h, CHAR * initials, BOOL fQuoteOld)
{
  LONG num;
  CHAR lineend[2], line[8], *p;

  if (!initials || !*initials)
    initials = " > ";
  num = MLEcurline(h);
  while (MLElinelen(h, num) < 3L && MLEnumlines(h) >= num)
    num++;
  while (MLElinelen(h, num) > 2L && MLEnumlines(h) >= num) {
    memset(line, 0, 8);
    MLEgetlinetext(h, num, line, 7L);
    line[7] = 0;
    if ((p = strchr(line, '>')) == NULL) {
      MLEsetcurpos(h, MLEstartofline(h, num));
      MLEinsert(h, initials);
      MLEsetcurpos(h, (MLEstartofline(h, num) + MLElinelen(h, num)) - 1L);
      MLEtextatcursor(h, lineend, 2L);
      if (*lineend != '\r' && *lineend != '\n')
	MLEinsert(h, "\n");
    }
    else if (fQuoteOld) {
      while (isspace(line[strlen(line) - 1]))
	line[strlen(line) - 1] = 0;
      MLEsetcurpos(h, MLEstartofline(h, num) + (p - line));
      MLEinsert(h, ">");
    }
    num++;
  }
  MLEsetcurpos(h, MLEstartofline(h, num));
  return TRUE;
}

BOOL MLEAutoLoad(HWND h, CHAR * filename)
{
  XMLEWNDPTR *vw;

  vw = (XMLEWNDPTR *) WinQueryWindowPtr(WinQueryWindow(h, QW_PARENT), QWL_USER);
  if (vw && vw->size != sizeof(XMLEWNDPTR))
    vw = NULL;
  if (TestBinary(filename)) {
    if (vw)
      vw->hex = 1;
    return MLEHexLoad(h, filename);
  }
  if (vw)
    vw->hex = 2;
  return MLEloadfile(h, filename);
}

BOOL MLEHexLoad(HWND h, CHAR * filename)
{
  /* insert a file into the current position in the MLE */

  HAB hab;
  CHAR *buffer = NULL, *hexbuff = NULL;
  IPT iptOffset = -1;
  ULONG numread, howmuch, numimport, action, len, left = 0;
  BOOL ret = TRUE, first = TRUE;
  CHAR titletext[512];
  HWND grandpa;
  XMLEWNDPTR *vw;
  HFILE handle;
  APIRET rc;

  *titletext = 0;
  hab = WinQueryAnchorBlock(h);
  vw = (XMLEWNDPTR *) WinQueryWindowPtr(WinQueryWindow(h, QW_PARENT), QWL_USER);
  if (vw && vw->size != sizeof(XMLEWNDPTR))
    vw = NULL;
  grandpa = GrandparentOf(h);
  *titletext = 0;
  WinQueryWindowText(grandpa, 512, titletext);
  rc = DosOpen(filename, &handle, &action, 0, 0,
	       OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
	       OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NOINHERIT |
	       OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYNONE |
	       OPEN_ACCESS_READONLY, 0);
  if (rc) {
    ret = FALSE;
  }
  else {
    DosChgFilePtr(handle, 0, FILE_END, &len);
    DosChgFilePtr(handle, 0, FILE_BEGIN, &action);
    if (len) {
      rc = DosAllocMem((PVOID) & hexbuff, 50001,
		       PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
      if (rc || !hexbuff) {
	Dos_Error(MB_CANCEL, rc, h, pszSrcFile, __LINE__,
		  GetPString(IDS_OUTOFMEMORY));
	ret = FALSE;
      }
      else {
	buffer = xmalloc(10000, pszSrcFile, __LINE__);
	if (!buffer)
	  ret = FALSE;
	else {
	  MLEclearall(h);
	  WinSendMsg(h, MLM_SETIMPORTEXPORT, MPFROMP(hexbuff),
		     MPFROMLONG(50000L));
	  if (!DosRead(handle, buffer, min(10000, len), &numread) && numread) {

	    CHAR s[81];

	    MLEsetwrap(h, FALSE);
	    WinSetSysValue(HWND_DESKTOP, SV_INSERTMODE, FALSE);
	    *hexbuff = 0;
	    numimport = CreateHexDump(buffer,
				      numread, hexbuff, 50000, left, TRUE);
	    while (len && numimport) {	/* import entire file */
	      left += numread;
	      len -= numread;
	      if (!WinIsWindow(hab, h) || (vw && vw->killme))
		break;
	      howmuch = (INT) WinSendMsg(h,
					 MLM_IMPORT,
					 MPFROMP(&iptOffset),
					 MPFROMLONG(numimport));
	      if (first && len) {
		MLEdisable(h);
		WinEnableWindowUpdate(h, FALSE);
		first = FALSE;
	      }
// fprintf(stderr,"%d bytes of %d imported\n",howmuch,numimport);
	      if (howmuch < 1) {
		numimport = 0;
		break;
	      }
	      while (howmuch < numimport) {
		numimport -= howmuch;
		memmove(hexbuff, hexbuff + howmuch, numimport);
		DosSleep(0);  //26 Aug 07 GKY 1
		if (!WinIsWindow(hab, h) || (vw && vw->killme))
		  break;
		howmuch = (INT) WinSendMsg(h,
					   MLM_IMPORT,
					   MPFROMP(&iptOffset),
					   MPFROMLONG((LONG) numimport));
		if (howmuch < 1)
		  break;
	      }
	      if (DosRead(handle, buffer, min(10000, len), &numread)
		  || !numread) {
		numimport = 0;
		break;
	      }
	      *hexbuff = 0;
	      numimport =
		CreateHexDump(buffer, numread, hexbuff, 50000, left, TRUE);
	      if (numimport < 1 || !WinIsWindow(hab, h) || (vw && vw->killme)) {
		numimport = 0;
		break;
	      }
	      sprintf(s, GetPString(IDS_LOADINGMLETEXT), len);
	      WinSetWindowText(grandpa, s);
	    }
	    DosSleep(1);
	  }
	  else
	    ret = FALSE;
	  free(buffer);
#         ifdef FORTIFY
	  Fortify_LeaveScope();
#          endif
	}
	DosFreeMem(hexbuff);
      }
    }
    if (WinIsWindow(hab, h))
      WinSetWindowText(grandpa, titletext);
    DosClose(handle);
  }
  if (!first) {
    WinEnableWindowUpdate(h, TRUE);
    MLEenable(h);
  }
  MLEsetchanged(h, FALSE);
  return ret;
}

//== MLEinsertfile() insert a file into the current position in the MLE ==

BOOL MLEinsertfile(HWND h, CHAR * filename)
{

  HAB hab;
  FILE *fp;
  CHAR *buffer = NULL;
  INT len;
  IPT iptOffset = -1L;
  INT numread, howmuch, tempnum, x;
  BOOL ret = TRUE, first = TRUE, once = FALSE, dont = FALSE;
  CHAR titletext[512];
  HWND grandpa;
  XMLEWNDPTR *vw;
  APIRET rc;

  *titletext = 0;
  hab = WinQueryAnchorBlock(h);
  vw = (XMLEWNDPTR *) WinQueryWindowPtr(WinQueryWindow(h, QW_PARENT), QWL_USER);
  if (vw && vw->size != sizeof(XMLEWNDPTR))
    vw = NULL;
  grandpa = GrandparentOf(h);
  *titletext = 0;
  WinQueryWindowText(grandpa, 512, titletext);
  fp = _fsopen(filename, "r", SH_DENYNO);
  if (!fp)
    ret = FALSE;
  else {
    setvbuf(fp, NULL, _IONBF, 0);
    fseek(fp, 0L, SEEK_END);
    len = (INT) ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    if (len && len != -1) {
      rc = DosAllocMem((PVOID) & buffer,
		       50000L, PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
      if (rc || !buffer) {
	Dos_Error(MB_CANCEL, rc, h, pszSrcFile, __LINE__,
		  GetPString(IDS_OUTOFMEMORY));
	ret = FALSE;
      }
      else {
	WinSendMsg(h,
		   MLM_SETIMPORTEXPORT, MPFROMP(buffer), MPFROMLONG(50000L));
	numread = fread(buffer, 1, min(50000, len), fp);
	if (numread < 1)
	  ret = FALSE;
	while (len && numread > 0) {	/* here we go... */

	  CHAR s[81];

	  while (numread > 0) {		/* import entire file */
	    if (!WinIsWindow(hab, h) || (vw && vw->killme))
	      break;
	    if (strlen(buffer) < numread) {
	      if (!once && !dont)
		rc = saymsg(MB_YESNOCANCEL,
			    HWND_DESKTOP,
			    GetPString(IDS_WARNINGTEXT),
			    GetPString(IDS_TEXTNULSTEXT));
	      else if (once)
		rc = MBID_YES;
	      else if (dont)
		rc = MBID_NO;
	      if (rc == MBID_YES) {
		once = FALSE;
		for (x = 0; x < numread; x++) {
		  if (!buffer[x])
		    buffer[x] = ' ';
		}
	      }
	      else if (rc == MBID_CANCEL) {
		len = 0;
		numread = 0;
		saymsg(MB_ENTER,
		       HWND_DESKTOP,
		       GetPString(IDS_OBEYTEXT),
		       GetPString(IDS_LOADCANCELLEDTEXT));
		break;
	      }
	      else if (rc == MBID_NO)
		dont = TRUE;
	    }
	    howmuch = (INT) WinSendMsg(h,
				       MLM_IMPORT,
				       MPFROMP(&iptOffset),
				       MPFROMLONG((LONG) numread));
	    if (first && !feof(fp)) {
	      MLEdisable(h);
	      WinEnableWindowUpdate(h, FALSE);
	      first = FALSE;
	    }
// fprintf(stderr,"%d bytes of %d imported\n",howmuch,numread);
	    if (howmuch < 1) {
	      numread = 0;
	      break;
	    }
	    len -= howmuch;
	    if (howmuch < numread) {
	      numread -= howmuch;
	      memmove(buffer, buffer + howmuch, numread);
	      if (numread && len) {
		tempnum = numread;
		numread = fread(buffer + tempnum,
				1, min(50000 - tempnum, len), fp);
		if (numread > 1)
		  numread += tempnum;
		else
		  numread = tempnum;
	      }
	      DosSleep(0); //26 Aug 07 GKY 1
	    }
	    else
	      numread = fread(buffer, 1, min(50000, len), fp);
	    if (numread < 1 || !WinIsWindow(hab, h) || (vw && vw->killme)) {
	      numread = 0;
	      break;
	    }
	    sprintf(s, GetPString(IDS_LOADINGMLETEXT), len);
	    WinSetWindowText(grandpa, s);
	  }
	  DosSleep(0); //26 Aug 07 GKY 1
	}
	DosFreeMem(buffer);
      }
    }
    if (WinIsWindow(hab, h))
      WinSetWindowText(grandpa, titletext);
    fclose(fp);
  }
  if (!first) {
    WinEnableWindowUpdate(h, TRUE);
    MLEenable(h);
  }
  return ret;
}

typedef struct
{
  USHORT size;
  USHORT hex;
  HWND h;
  CHAR filename[CCHMAXPATH];
  HWND hwndReport;
  HWND msg;
}
BKGLOAD;

VOID LoadThread(VOID * arg)
{
  BKGLOAD *bkg;
  BOOL fSuccess;
  HAB thab;
  HMQ thmq;

  DosError(FERR_DISABLEHARDERR);

# ifdef FORTIFY
  Fortify_EnterScope();
#  endif

  bkg = (BKGLOAD *) arg;
  if (bkg) {
    thab = WinInitialize(0);
    if (thab) {
      thmq = WinCreateMsgQueue(thab, 0);
      if (thmq) {
	WinCancelShutdown(thmq, TRUE);
	IncrThreadUsage();
	priority_normal();
	if (bkg->hex == 1)
	  fSuccess = MLEHexLoad(bkg->h, bkg->filename);
	else if (bkg->hex == 2)
	  fSuccess = MLEloadfile(bkg->h, bkg->filename);
	else
	  fSuccess = MLEAutoLoad(bkg->h, bkg->filename);
	priority_bumped();
	if (bkg->hwndReport && WinIsWindow(thab, bkg->hwndReport))
	  PostMsg(bkg->hwndReport, bkg->msg, MPFROMLONG(fSuccess),
		  MPFROMP(bkg->filename));
#ifdef __DEBUG_ALLOC__
	_heap_check();
#endif
	WinDestroyMsgQueue(thmq);
      }
      else
	PostMsg(bkg->hwndReport, bkg->msg, MPVOID, MPVOID);
      DecrThreadUsage();
      WinTerminate(thab);
    }
    else {
      // fixme to be gone? - can not PostMsg without HAB
      PostMsg(bkg->hwndReport, bkg->msg, MPVOID, MPVOID);
    }
    free(bkg);
  } // if bkg
# ifdef FORTIFY
  Fortify_LeaveScope();
#  endif
  // _endthread();			// 10 Dec 08 SHL
}

INT MLEbackgroundload(HWND hwndReport, ULONG msg, HWND h, CHAR * filename,
		      INT hex)
{
  /* load a file into the MLE in the background (via a separate thread)
   * return _beginthread status
   */

  BKGLOAD *bkg;

  bkg = xmallocz(sizeof(BKGLOAD), pszSrcFile, __LINE__);
  if (!bkg)
    return -1;
  bkg->size = sizeof(BKGLOAD);
  bkg->hex = (USHORT) hex;
  bkg->hwndReport = hwndReport;
  bkg->msg = msg;
  bkg->h = h;
  strcpy(bkg->filename, filename);
  return xbeginthread(LoadThread,
		      65536,
		      bkg,
		      pszSrcFile,
		      __LINE__);
}

BOOL MLEloadfile(HWND h, CHAR * filename)
{
  /* load a file into the MLE, getting rid of whatever was already
   * there.  Note this returns without erasing existing text if the
   * file to load does not exist
   */

  FILESTATUS3 fsa;
  BOOL ret;

  if (!DosQueryPathInfo(filename, FIL_STANDARD, &fsa, sizeof(fsa)) &&
      ~fsa.attrFile & FILE_DIRECTORY) {
    MLEclearall(h);
    ret = MLEinsertfile(h, filename);
    MLEsetchanged(h, FALSE);
    return ret;
  }
  return FALSE;
}

BOOL MLEexportfile(HWND h, CHAR * filename, INT tabspaces,
		   BOOL striptraillines, BOOL striptrailspaces)
{
  /* save the MLE contents as a file.  Format the output so that
   * the file is CR/LF terminated as presented in the MLE.
   */

  FILE *fp = NULL;
  CHAR *buffer = NULL;
  CHAR *p;
  BOOL ok = TRUE;
  INT blanklines = 0;
  BOOL fWrap = MLEgetwrap(h);
  APIRET rc;

  // saymsg(MB_ENTER,h,DEBUG_STRING,"len = %ld",MLEgetlen(h));
  if (!MLEgetlen(h))			/* nothing to save; forget it */
    return TRUE;

  MLEsetwrap(h, FALSE);			// Need wrap off to export MLFIE_NOTRANS

  if (striptraillines) {

    register LONG x;
    LONG numlines;

    numlines = MLEnumlines(h);
    for (x = numlines; x; x--) {
      if (MLElinelen(h, x - 1L) < 2)
	MLEdeleteline(h, x - 1L);
      else
	break;
    }
    if (!MLEgetlen(h)) {
      /* nothing to save; forget it */
      MLEsetwrap(h, fWrap);		// Restore
      return TRUE;
    }
  }

  rc = DosAllocMem((PVOID) & buffer, 4096L,
		   PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
  if (rc || !buffer) {
    Dos_Error(MB_CANCEL, rc, h, pszSrcFile, __LINE__,
	      GetPString(IDS_OUTOFMEMORY));
    ok = FALSE;
  }
  else {
    fp = fopen(filename, "a+");
    if (!fp)
      fp = xfopen(filename, "w", pszSrcFile, __LINE__);
    if (!fp)
      ok = FALSE;
    else {
      LONG numlines, ret, len, wl, temp;
      IPT s;

      fseek(fp, 0L, SEEK_END);
      numlines = MLEnumlines(h);

      WinSendMsg(h, MLM_SETIMPORTEXPORT, MPFROMP(buffer), MPFROMLONG(4095L));
      for (temp = 0; temp < numlines; temp++) {
	s = MLEstartofline(h, temp);
	wl = len = (LONG) MLElinelenleft(h, s);
	ret = (LONG) WinSendMsg(h, MLM_EXPORT, MPFROMP(&s), MPFROMP(&len));
	if (ret < 0)
	  break;
	wl = min(wl, ret);
	buffer[wl] = 0;
	if (*buffer) {
	  p = buffer + strlen(buffer) - 1;
	  while (p >= buffer && (*p == '\n' || *p == '\r')) {
	    *p = 0;
	    p--;
	  }
	}
	if (tabspaces) {
	  p = buffer;
	  while (*p) {
	    if (*p == '\t') {
	      *p = ' ';
	      memmove((p + tabspaces) - 1, p, strlen(p) + 1);
	      memset(p, ' ', tabspaces);
	    }
	    p++;
	  }
	}
	if (striptrailspaces && *buffer) {
	  p = buffer + strlen(buffer) - 1;
	  while (p >= buffer && (*p == ' ' || *p == '\t')) {
	    *p = 0;
	    p--;
	  }
	}
	if (striptraillines) {
	  if (!*buffer) {
	    blanklines++;
	    continue;
	  }
	  else {
	    while (blanklines) {
	      fwrite("\n", 1, 1, fp);
	      blanklines--;
	    }
	  }
	}
	strcat(buffer, "\n");
	// buffer = translate_out(buffer,4095,h,filename);
	if (fwrite(buffer, 1, strlen(buffer), fp) < 1) {
	  saymsg(MB_ENTER,
		 h, GetPString(IDS_ARGHTEXT), GetPString(IDS_WRITEERRORTEXT));
	  break;
	}
      }					// for lines
    }
  }

  MLEsetwrap(h, fWrap);			// Restore

  if (fp)
    fclose(fp);
  if (buffer)
    DosFreeMem(buffer);

  return ok;
}

MRESULT EXPENTRY SandRDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  /* initiate search(/replace)s in edit mode */

  SRCHPTR *vw;

  if (msg != WM_INITDLG)
    vw = (SRCHPTR *) WinQueryWindowPtr(hwnd, QWL_USER);
  else
    vw = NULL;

  switch (msg) {
  case WM_INITDLG:
    vw = (SRCHPTR *) mp2;
    if (!vw) {
      WinDismissDlg(hwnd, 0);
      break;
    }
    WinSetWindowPtr(hwnd, QWL_USER, (PVOID) mp2);
    WinSendDlgItemMsg(hwnd, SRCH_SEARCH, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(256, 0), MPVOID);
    WinSendDlgItemMsg(hwnd, SRCH_REPLACE, EM_SETTEXTLIMIT,
		      MPFROM2SHORT(256, 0), MPVOID);
    if (*vw->search)
      WinSetDlgItemText(hwnd, SRCH_SEARCH, vw->search);
    if (!MLEgetreadonly(vw->hwndmle)) {
      if (*vw->replace)
	WinSetDlgItemText(hwnd, SRCH_REPLACE, vw->replace);
      WinSendDlgItemMsg(hwnd, SRCH_SANDR, BM_SETCHECK,
			MPFROM2SHORT(vw->sandr, 0), MPVOID);
      WinSendDlgItemMsg(hwnd, SRCH_RALL, BM_SETCHECK,
			MPFROM2SHORT(vw->rall, 0), MPVOID);
    }
    else {
      WinEnableWindow(WinWindowFromID(hwnd, SRCH_SANDR), FALSE);
      WinEnableWindow(WinWindowFromID(hwnd, SRCH_RALL), FALSE);
      WinEnableWindow(WinWindowFromID(hwnd, SRCH_REPLACE), FALSE);
      WinShowWindow(WinWindowFromID(hwnd, SRCH_REPLACE), FALSE);
      *vw->replace = 0;
      vw->sandr = FALSE;
      vw->rall = FALSE;
    }
    memset(&vw->se, 0, sizeof(MLE_SEARCHDATA));
    vw->se.cb = sizeof(MLE_SEARCHDATA);
    vw->se.pchFind = (PCHAR) vw->search;
    vw->se.cchFind = (SHORT) strlen(vw->search);
    vw->se.pchReplace = (PCHAR) vw->replace;
    vw->se.cchReplace = (SHORT) strlen(vw->replace);
    vw->se.iptStart = MLEcurpos(vw->hwndmle);
    vw->se.iptStop = -1L;
    vw->se.cchFound = 0;
    PosOverOkay(hwnd);
    break;

  case WM_CONTROL:
    return 0;

  case WM_COMMAND:
    switch (SHORT1FROMMP(mp1)) {
    case IDM_HELP:
      saymsg(MB_ENTER | MB_ICONASTERISK,
	     hwnd,
	     NullStr,
	     GetPString(IDS_ENTERSEARCHSTRINGTEXT),
	     (MLEgetreadonly(vw->hwndmle)) ?
	     "." : GetPString(IDS_REPLACESTRINGTEXT));
      break;

    case DID_CANCEL:
      WinDismissDlg(hwnd, 0);
      break;

    case DID_OK:
      WinShowWindow(hwnd, FALSE);
      {
	CHAR temp[257];
	APIRET ret;

	if ((USHORT) WinSendDlgItemMsg(hwnd, SRCH_SANDR, BM_QUERYCHECK,
				       MPVOID, MPVOID))
	  vw->sandr = TRUE;
	else
	  vw->sandr = FALSE;
	if ((USHORT) WinSendDlgItemMsg(hwnd, SRCH_RALL, BM_QUERYCHECK,
				       MPVOID, MPVOID))
	  vw->rall = TRUE;
	else
	  vw->rall = FALSE;
	*vw->replace = 0;
	WinQueryDlgItemText(hwnd, SRCH_REPLACE, 256, vw->replace);
	vw->se.cchReplace = (SHORT) strlen(vw->replace);
	WinQueryDlgItemText(hwnd, SRCH_SEARCH, 256, temp);
	if (*temp) {
	  strcpy(vw->search, temp);
	  vw->se.cchFind = (SHORT) strlen(vw->search);
	  if (!WinSendMsg(vw->hwndmle, MLM_SEARCH,
			  MPFROMLONG(MLFSEARCH_SELECTMATCH |
				     (MLFSEARCH_CASESENSITIVE *
				      (vw->fInsensitive ==
				       FALSE)) | (MLFSEARCH_CHANGEALL *
						  (vw->rall != 0))),
			  MPFROMP(&vw->se))) {
	    saymsg(MB_ENTER | MB_ICONASTERISK, hwnd, NullStr,
		   GetPString(IDS_STRINGNOTFOUNDTEXT), vw->search);
	    WinDismissDlg(hwnd, 0);
	    break;
	  }
	  else if (vw->sandr && !vw->rall) {
	    ret = saymsg(MB_YESNOCANCEL,
			 hwnd,
			 NullStr,
			 GetPString(IDS_CONFIRMREPLACETEXT), vw->replace);
	    if (ret == MBID_YES)
	      MLEinsert(vw->hwndmle, vw->replace);
	    else if (ret == MBID_CANCEL) {
	      WinDismissDlg(hwnd, 0);
	      break;
	    }
	    WinDismissDlg(hwnd, 1);
	    break;
	  }
	}
	WinDismissDlg(hwnd, 0);
      }
      break;
    }
    return 0;

  case WM_CLOSE:
    break;
  }

  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

BOOL MLEfindfirst(HWND hwnd, SRCHPTR * vw)
{
  if (MLEsizeofsel(vw->hwndmle) < 256L) {
    MLEgetseltext(vw->hwndmle, vw->search);
    vw->search[255] = 0;
  }
  if (WinDlgBox(HWND_DESKTOP, hwnd, SandRDlgProc, FM3ModHandle,
		SRCH_FRAME, MPFROMP(vw)) && *vw->search)
    return TRUE;
  return FALSE;
}

INT MLEfindnext(HWND hwnd, SRCHPTR * vw)
{
  if (!*vw->search)
    return -1;
  else {
    vw->se.iptStart++;
    if (!WinSendMsg(vw->hwndmle, MLM_SEARCH,
		    MPFROMLONG(MLFSEARCH_SELECTMATCH |
			       (MLFSEARCH_CASESENSITIVE *
				(vw->fInsensitive ==
				 FALSE)) | (MLFSEARCH_CHANGEALL *
					    (vw->rall))), MPFROMP(&vw->se)))
      saymsg(MB_ENTER | MB_ICONASTERISK, hwnd, NullStr,
	     GetPString(IDS_STRINGNOTFOUNDTEXT), vw->search);
    else if (vw->sandr && !vw->rall) {

      APIRET ret;

      ret = saymsg(MB_YESNOCANCEL,
		   hwnd,
		   NullStr, GetPString(IDS_CONFIRMREPLACETEXT), vw->replace);
      if (ret == MBID_YES)
	MLEinsert(vw->hwndmle, vw->replace);
      if (ret != MBID_CANCEL)
	return 1;
    }
  }
  return 0;
}

#pragma alloc_text(FMMLE,MLEgetlinetext,MLEdeleteline,MLEdeletecurline,MLEdeletetoeol)
#pragma alloc_text(FMMLE,MLEclearall,MLEtextatcursor,MLEtextatpos,MLEsizeofsel)
#pragma alloc_text(FMMLE3,MLEdoblock,MLEquotepara,MLEinternet)
#pragma alloc_text(FMMLE4,MLEAutoLoad,MLEHexLoad,MLEinsertfile,LoadThread,MLEbackgroundload)
#pragma alloc_text(FMMLE5,MLEloadfile,MLEexportfile)
#pragma alloc_text(FMMLE3,MLEfindfirst,MLEfindnext,SandRDlgProc)
