
/***********************************************************************

  $Id$

  Edit presentation parameters

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006 Steven H.Levine

  22 Jul 06 SHL Check more run time errors

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "fm3dll.h"

// static PSZ pszSrcFile = __FILE__;

#pragma alloc_text(PRESPARAM,CopyPresParams,SetPresParams,IfNoParam)
#pragma alloc_text(PRESPARAM,PresParamChanged,RestorePresParams)
#pragma alloc_text(PRESPARAM,StoreWndPresParams)

#ifdef NEVER
/*
 * this routine will fill a buffer with all defined window pres params
 * the PRESPARAMS structure returned is suitable to be passed as
 * pPresParams ie. WinCreateWindow(,,,,,,,,,,,, PVOID pPresParams)
 */
VOID StoreWndPresParams(HWND hwnd, CHAR * tagname, HINI prof)
{
  PARAM *pparam;
  PRESPARAMS *ppresparams;

  if (!tagname || !*tagname || !prof)
    return;
  /* setup memory access */
  ppresparams = (PRESPARAMS *) xmalloc(PP_MAXBUF, pszSrcFile, __LINE__);
  if (!ppresparams)
    return;
  ppresparams->cb = 0;			/* no entries yet */
  pparam = ppresparams->aparam;		/* cast structure onto memory */

  /*
   * query every possible presentation parameter...
   */

  /* foreground color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_FOREGROUNDCOLOR,
				 PP_FOREGROUNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {			/* was the param found? */
    ppresparams->cb += 12;		/* used 12 bytes */
    pparam = (PPARAM) (((ULONG) pparam) + 12);	/* advance 12 bytes to next memory location */
  }

  /* background color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_BACKGROUNDCOLOR,
				 PP_BACKGROUNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* hilited foreground color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_HILITEFOREGROUNDCOLOR,
				 PP_HILITEFOREGROUNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* hilited background color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_HILITEBACKGROUNDCOLOR,
				 PP_HILITEBACKGROUNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* disabled foreground color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_DISABLEDFOREGROUNDCOLOR,
				 PP_DISABLEDFOREGROUNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* disabled background color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_DISABLEDBACKGROUNDCOLOR,
				 PP_DISABLEDBACKGROUNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* border color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_BORDERCOLOR,
				 PP_BORDERCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* font name & size -- maximum 44 bytes (32 for face name, 4 for point size) + 8 for data */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_FONTNAMESIZE,
				 0,
				 &pparam->id,
				 36, (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += (pparam->cb + 8);
    pparam = (PPARAM) (((ULONG) pparam) + pparam->cb + 8);
  }

  /* active color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_ACTIVECOLOR,
				 PP_ACTIVECOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* inactive color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_INACTIVECOLOR,
				 PP_INACTIVECOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* active text foreground color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_ACTIVETEXTFGNDCOLOR,
				 PP_ACTIVETEXTFGNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab,
				 QPF_NOINHERIT | QPF_PURERGBCOLOR);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* active text background color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_ACTIVETEXTBGNDCOLOR,
				 PP_ACTIVETEXTBGNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab,
				 QPF_NOINHERIT | QPF_PURERGBCOLOR);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* inactive text foreground color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_INACTIVETEXTFGNDCOLOR,
				 PP_INACTIVETEXTFGNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab,
				 QPF_NOINHERIT | QPF_PURERGBCOLOR);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* inactive text background color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_INACTIVETEXTBGNDCOLOR,
				 PP_INACTIVETEXTBGNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab,
				 QPF_NOINHERIT | QPF_PURERGBCOLOR);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* shadow color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_SHADOW, 0,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* menu foreground color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_MENUFOREGROUNDCOLOR,
				 PP_MENUFOREGROUNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* menu background color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_MENUBACKGROUNDCOLOR,
				 PP_MENUBACKGROUNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* menu hilite foreground color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_MENUHILITEFGNDCOLOR,
				 PP_MENUHILITEFGNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* menu hilite background color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_MENUHILITEBGNDCOLOR,
				 PP_MENUHILITEBGNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* menu disabled foreground color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_MENUDISABLEDFGNDCOLOR,
				 PP_MENUDISABLEDFGNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  /* menu disabled background color -- takes 12 bytes */
  pparam->cb = WinQueryPresParam(hwnd,
				 PP_MENUDISABLEDBGNDCOLOR,
				 PP_MENUDISABLEDBGNDCOLORINDEX,
				 &pparam->id,
				 sizeof(LONG),
				 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  if (ppresparams->cb)
    PrfWriteProfileData(prof,
			appname, tagname, ppresparams, ppresparams->cb + 4);

  free(ppresparams);
}

#endif // NEVER

VOID CopyPresParams(HWND target, HWND source)
{
  /*
   * Copy presentation parameters of interest to us from one window
   * to another
   */

  ULONG AttrFound, AttrValue[64], cbRetLen, x = 0,
    AttrName[] = { PP_FONTNAMESIZE, PP_BACKGROUNDCOLOR,
    PP_FOREGROUNDCOLOR, PP_HILITEBACKGROUNDCOLOR,
    PP_HILITEFOREGROUNDCOLOR, PP_BORDERCOLOR,
    0
  };

  while (AttrName[x]) {
    cbRetLen = WinQueryPresParam(source,
				 AttrName[x],
				 0,
				 &AttrFound,
				 (ULONG) sizeof(AttrValue), &AttrValue, 0);
    if (cbRetLen)
      WinSetPresParam(target, AttrName[x], cbRetLen, (PVOID) AttrValue);
    x++;
  }
}

VOID SetPresParams(HWND hwnd, RGB2 * back, RGB2 * fore, RGB2 * border,
		   CHAR * font)
{
  if (font)
    WinSetPresParam(hwnd, PP_FONTNAMESIZE, strlen(font) + 1, (PVOID) font);
  if (back)
    WinSetPresParam(hwnd, PP_BACKGROUNDCOLOR, sizeof(RGB2), (PVOID) back);
  if (fore)
    WinSetPresParam(hwnd, PP_FOREGROUNDCOLOR, sizeof(RGB2), (PVOID) fore);
  if (border)
    WinSetPresParam(hwnd, PP_BORDERCOLOR, sizeof(RGB2), (PVOID) border);
}

VOID IfNoParam(HWND hwnd, CHAR * keyroot, ULONG size, PVOID attrvalue)
{
  ULONG fsize = 0;
  CHAR s[81];

  sprintf(s, "%s", keyroot);
  if (!PrfQueryProfileSize(fmprof, appname, s, &fsize) || !fsize)
    WinSetPresParam(hwnd, PP_FONTNAMESIZE, size, (PVOID) attrvalue);
}

VOID PresParamChanged(HWND hwnd, CHAR * keyroot, MPARAM mp1, MPARAM mp2)
{
  ULONG AttrFound, AttrValue[64], cbRetLen;

  cbRetLen = WinQueryPresParam(hwnd, (ULONG) mp1, 0, &AttrFound,
			       (ULONG) sizeof(AttrValue), &AttrValue, 0);
  if (cbRetLen) {

    CHAR s[133];

    *s = 0;
    switch (AttrFound) {
    case PP_BACKGROUNDCOLOR:
      sprintf(s, "%s.Backgroundcolor", keyroot);
      break;
    case PP_FOREGROUNDCOLOR:
      sprintf(s, "%s.Foregroundcolor", keyroot);
      break;
    case PP_HILITEBACKGROUNDCOLOR:
      sprintf(s, "%s.Hilitebackgroundcolor", keyroot);
      break;
    case PP_HILITEFOREGROUNDCOLOR:
      sprintf(s, "%s.Hiliteforegroundcolor", keyroot);
      break;
    case PP_BORDERCOLOR:
      sprintf(s, "%s.Bordercolor", keyroot);
      break;
    case PP_FONTNAMESIZE:
      sprintf(s, "%s.Fontnamesize", keyroot);
      break;
    default:
      break;
    }
    if (*s)
      PrfWriteProfileData(fmprof, appname, s, (PVOID) AttrValue, cbRetLen);
  }
}

VOID RestorePresParams(HWND hwnd, CHAR * keyroot)
{
  CHAR s[81];
  ULONG AttrValue[64], size;

  size = sizeof(AttrValue);
  sprintf(s, "%s.Backgroundcolor", keyroot);
  if (PrfQueryProfileData(fmprof, appname, s, (PVOID) AttrValue, &size)
      && size)
    WinSetPresParam(hwnd, PP_BACKGROUNDCOLOR, size, (PVOID) AttrValue);
  size = sizeof(AttrValue);
  sprintf(s, "%s.Foregroundcolor", keyroot);
  if (PrfQueryProfileData(fmprof, appname, s, (PVOID) AttrValue, &size)
      && size)
    WinSetPresParam(hwnd, PP_FOREGROUNDCOLOR, size, (PVOID) AttrValue);
  size = sizeof(AttrValue);
  sprintf(s, "%s.Hilitebackgroundcolor", keyroot);
  if (PrfQueryProfileData(fmprof, appname, s, (PVOID) AttrValue, &size)
      && size)
    WinSetPresParam(hwnd, PP_HILITEBACKGROUNDCOLOR, size, (PVOID) AttrValue);
  size = sizeof(AttrValue);
  sprintf(s, "%s.Hiliteforegroundcolor", keyroot);
  if (PrfQueryProfileData(fmprof, appname, s, (PVOID) AttrValue, &size)
      && size)
    WinSetPresParam(hwnd, PP_HILITEFOREGROUNDCOLOR, size, (PVOID) AttrValue);
  size = sizeof(AttrValue);
  sprintf(s, "%s.Bordercolor", keyroot);
  if (PrfQueryProfileData(fmprof, appname, s, (PVOID) AttrValue, &size)
      && size)
    WinSetPresParam(hwnd, PP_BORDERCOLOR, size, (PVOID) AttrValue);
  size = sizeof(AttrValue);
  sprintf(s, "%s.Fontnamesize", keyroot);
  if (PrfQueryProfileData(fmprof,
			  appname, s, (PVOID) AttrValue, &size) && size)
    WinSetPresParam(hwnd, PP_FONTNAMESIZE, size, (PVOID) AttrValue);
}
