
/***********************************************************************

  $Id$

  Edit presentation parameters

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2008 Steven H.Levine

  22 Jul 06 SHL Check more run time errors
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  22 Nov 07 GKY Use CopyPresParams to fix presparam inconsistencies in menus
  10 Dec 07 GKY Updated CopyPresParams to copy all parameter types
  08 Mar 09 GKY Additional strings move to PCSZs in init.c
  19 Mar 09 GKY Moved DeletePresParams from mainwnd.c

***********************************************************************/

#include <string.h>

#define INCL_WIN
#define INCL_LONGLONG                   // dircnrs.h

#include "fm3dll.h"
#include "colors.h"			// typedef RGB2
#include "presparm.h"
#include "notebook.h"			// Data declaration(s)
#include "wrappers.h"			// xmalloc
#include "fortify.h"

// static PSZ pszSrcFile = __FILE__;

//static VOID IfNoParam(HWND hwnd, CHAR * keyroot, ULONG size, PVOID attrvalue);

//static VOID StoreWndPresParams(HWND hwnd, CHAR * tagname, HINI prof);

#ifdef NEVER
/**
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
  // setup memory access
  ppresparams = (PRESPARAMS *) xmalloc(PP_MAXBUF, pszSrcFile, __LINE__);
  if (!ppresparams)
    return;
  ppresparams->cb = 0;                  // no entries yet
  pparam = ppresparams->aparam;         // cast structure onto memory

  // query every possible presentation parameter...

  // foreground color -- takes 12 bytes
  pparam->cb = WinQueryPresParam(hwnd,
                                 PP_FOREGROUNDCOLOR,
                                 PP_FOREGROUNDCOLORINDEX,
                                 &pparam->id,
                                 sizeof(LONG),
                                 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {                             // was the param found?
    ppresparams->cb += 12;                      // used 12 bytes
    pparam = (PPARAM) (((ULONG) pparam) + 12);  // advance 12 bytes to next memory location
  }

  // background color -- takes 12 bytes
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

  // hilited foreground color -- takes 12 bytes
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

  // hilited background color -- takes 12 bytes
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

  // disabled foreground color -- takes 12 bytes
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

  // disabled background color -- takes 12 bytes
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

  // border color -- takes 12 bytes
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

  // font name & size -- maximum 44 bytes (32 for face name, 4 for point size) + 8 for data
  pparam->cb = WinQueryPresParam(hwnd,
                                 PP_FONTNAMESIZE,
                                 0,
                                 &pparam->id,
                                 36, (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += (pparam->cb + 8);
    pparam = (PPARAM) (((ULONG) pparam) + pparam->cb + 8);
  }

  // active color -- takes 12 bytes
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

  // inactive color -- takes 12 bytes
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

  // active text foreground color -- takes 12 bytes
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

  // active text background color -- takes 12 bytes
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

  // inactive text foreground color -- takes 12 bytes
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

  // inactive text background color -- takes 12 bytes
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

  // shadow color -- takes 12 bytes
  pparam->cb = WinQueryPresParam(hwnd,
                                 PP_SHADOW, 0,
                                 &pparam->id,
                                 sizeof(LONG),
                                 (PVOID) pparam->ab, QPF_NOINHERIT);
  if (pparam->cb) {
    ppresparams->cb += 12;
    pparam = (PPARAM) (((ULONG) pparam) + 12);
  }

  // menu foreground color -- takes 12 bytes
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

  // menu background color -- takes 12 bytes
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

  // menu hilite foreground color -- takes 12 bytes
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

  // menu hilite background color -- takes 12 bytes
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

  // menu disabled foreground color -- takes 12 bytes
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

  // menu disabled background color -- takes 12 bytes
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
  /**
   * Copy presentation parameters of interest to us from one window
   * to another
   */

  ULONG AttrFound, AttrValue[64], cbRetLen, x = 0,
    AttrName[] = { PP_FONTNAMESIZE, PP_FOREGROUNDCOLOR,
    PP_BACKGROUNDCOLOR, PP_HILITEBACKGROUNDCOLOR,
    PP_HILITEFOREGROUNDCOLOR, PP_BORDERCOLOR, PP_MENUFOREGROUNDCOLOR,
    PP_MENUBACKGROUNDCOLOR, PP_FOREGROUNDCOLORINDEX, PP_BACKGROUNDCOLORINDEX,
    PP_HILITEFOREGROUNDCOLORINDEX, PP_HILITEBACKGROUNDCOLORINDEX,
    PP_DISABLEDFOREGROUNDCOLOR, PP_DISABLEDBACKGROUNDCOLOR,
    PP_DISABLEDFOREGROUNDCOLORINDEX, PP_DISABLEDBACKGROUNDCOLORINDEX,
    PP_BORDERCOLOR, PP_BORDERCOLORINDEX, PP_FONTHANDLE, PP_ACTIVECOLOR,
    PP_ACTIVECOLORINDEX, PP_INACTIVECOLOR, PP_INACTIVECOLORINDEX,
    PP_ACTIVETEXTFGNDCOLOR, PP_ACTIVETEXTFGNDCOLORINDEX, PP_ACTIVETEXTBGNDCOLOR,
    PP_ACTIVETEXTBGNDCOLORINDEX, PP_INACTIVETEXTFGNDCOLOR,
    PP_INACTIVETEXTFGNDCOLORINDEX, PP_INACTIVETEXTBGNDCOLOR,
    PP_INACTIVETEXTBGNDCOLORINDEX, PP_SHADOW, PP_MENUFOREGROUNDCOLORINDEX,
    PP_MENUBACKGROUNDCOLORINDEX, PP_MENUHILITEFGNDCOLOR, PP_MENUHILITEFGNDCOLORINDEX,
    PP_MENUHILITEBGNDCOLOR, PP_MENUHILITEBGNDCOLORINDEX, PP_MENUDISABLEDFGNDCOLOR,
    PP_MENUDISABLEDFGNDCOLORINDEX, PP_MENUDISABLEDBGNDCOLOR,
    PP_MENUDISABLEDBGNDCOLORINDEX, PP_SHADOWTEXTCOLOR, PP_SHADOWTEXTCOLORINDEX,
    PP_SHADOWHILITEFGNDCOLOR, PP_SHADOWHILITEFGNDCOLORINDEX, PP_SHADOWHILITEBGNDCOLOR,
    PP_SHADOWHILITEBGNDCOLORINDEX, PP_ICONTEXTBACKGROUNDCOLOR,
    PP_ICONTEXTBACKGROUNDCOLORINDEX, PP_BORDERLIGHTCOLOR, PP_BORDERDARKCOLOR,
    PP_BORDER2COLOR, PP_BORDER2LIGHTCOLOR, PP_BORDER2DARKCOLOR, PP_BORDERDEFAULTCOLOR,
    PP_FIELDBACKGROUNDCOLOR, PP_BUTTONBACKGROUNDCOLOR, PP_BUTTONBORDERLIGHTCOLOR,
    PP_BUTTONBORDERDARKCOLOR, PP_ARROWCOLOR, PP_ARROWBORDERLIGHTCOLOR,
    PP_ARROWBORDERDARKCOLOR, PP_ARROWDISABLEDCOLOR, PP_CHECKLIGHTCOLOR,
    PP_CHECKMIDDLECOLOR, PP_CHECKDARKCOLOR, PP_PAGEFOREGROUNDCOLOR,
    PP_PAGEBACKGROUNDCOLOR, PP_MAJORTABFOREGROUNDCOLOR, PP_MAJORTABBACKGROUNDCOLOR,
    PP_MINORTABFOREGROUNDCOLOR, PP_MINORTABBACKGROUNDCOLOR, PP_USER, 0 };

  while (AttrName[x]) {
    cbRetLen = WinQueryPresParam(source,
                                 AttrName[x],
                                 0,
                                 &AttrFound,
                                 sizeof(AttrValue),
                                 &AttrValue, 0);
    if (cbRetLen)
      WinSetPresParam(target, AttrName[x], cbRetLen, (PVOID) AttrValue);
    x++;
  } //while
}

VOID SetPresParams(HWND hwnd, RGB2 * back, RGB2 * fore, RGB2 * border,
                   PCSZ font)
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

#if 0   // JBS	11 Sep 08
VOID IfNoParam(HWND hwnd, CHAR * keyroot, ULONG size, PVOID attrvalue)
{
  ULONG fsize = 0;
  CHAR s[81];

  sprintf(s, "%s", keyroot);
  if (!PrfQueryProfileSize(fmprof, appname, s, &fsize) || !fsize)
    WinSetPresParam(hwnd, PP_FONTNAMESIZE, size, (PVOID) attrvalue);
}
#endif

VOID PresParamChanged(HWND hwnd, PCSZ keyroot, MPARAM mp1, MPARAM mp2)
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

VOID RestorePresParams(HWND hwnd, PCSZ keyroot)
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

  /** SavePresParams
   * Save the presentation parameters used by RestorePresParams
   */
VOID SavePresParams(HWND hwnd, PCSZ keyroot)
{


  ULONG AttrFound, AttrValue[64], cbRetLen, x = 0,
    AttrName[] = { PP_FONTNAMESIZE, PP_FOREGROUNDCOLOR,
    PP_BACKGROUNDCOLOR, PP_HILITEBACKGROUNDCOLOR,
    PP_HILITEFOREGROUNDCOLOR, PP_BORDERCOLOR, 0 };

  while (AttrName[x]) {
    cbRetLen = WinQueryPresParam(hwnd,
                                 AttrName[x],
                                 0,
                                 &AttrFound,
                                 sizeof(AttrValue),
                                 &AttrValue, 0);
    if (cbRetLen){
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
    x++;
  } //while
}

VOID DeletePresParams(PCSZ Keyroot)
{
  PSZ apszPPNames[] =
  {
    "Backgroundcolor",
    "Foregroundcolor",
    "Hilitebackgroundcolor",
    "Hiliteforegroundcolor",
    "Bordercolor",
    "Fontnamesize"
  };

  ULONG ulSize, ulArraySize = sizeof(apszPPNames) / sizeof(PSZ), x;
  CHAR  pchKeyroot[CCHMAXPATH];
  CHAR  *eos;

  strcpy(pchKeyroot, Keyroot);
  eos = pchKeyroot + strlen(pchKeyroot);

  for (x = 0; x < ulArraySize; x++) {
    strcpy(eos, apszPPNames[x]);
    if (PrfQueryProfileSize(fmprof, appname, pchKeyroot, &ulSize) && ulSize) {
      PrfWriteProfileData(fmprof, appname, pchKeyroot, NULL, ulSize);
    }
  }
}

#pragma alloc_text(PRESPARAM,CopyPresParams,SetPresParams)
#pragma alloc_text(PRESPARAM,PresParamChanged,RestorePresParams,SavePresParams)
#pragma alloc_text(PRESPARAM,StoreWndPresParams)
