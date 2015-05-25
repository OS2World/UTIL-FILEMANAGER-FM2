
/***********************************************************************

  $Id$

  Font support

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2009 Steven H. Levine

  05 Jan 08 SHL Sync
  29 Nov 08 GKY Remove or replace with a mutex semaphore DosEnterCriSec where appropriate.
  10 Jan 09 GKY Removed rotating strings for font samples as part of StringTable conversion

***********************************************************************/

#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG                   // dircnrs.h

#include "fm3str.h"
#include "errutil.h"                    // Dos_Error...
#include "strutil.h"                    // GetPString
#include "fonts.h"
#include "fm3dll.h"
#include "init.h"                       // Global semaphore

//static VOID SetFont(HWND hwnd);

#pragma data_seg(DATA1)


/**
 *   Convert vector font size using point size and fAttrs structure and
 *   return it in that structure.
 */

VOID ConvertVectorFontSize(FIXED fxPointSize, PFATTRS pfattrs)
{

  HPS hps;
  HDC hDC;
  LONG lxFontResolution;
  LONG lyFontResolution;
  SIZEF sizef;

  hps = WinGetScreenPS(HWND_DESKTOP);   // Screen presentation space

  /**
   *   Query device context for the screen and then query
   *   the resolution of the device for the device context.
   */

  hDC = GpiQueryDevice(hps);
  DevQueryCaps(hDC, CAPS_HORIZONTAL_FONT_RES, (LONG) 1, &lxFontResolution);
  DevQueryCaps(hDC, CAPS_VERTICAL_FONT_RES, (LONG) 1, &lyFontResolution);

  /**
   *   Calculate the size of the character box, based on the
   *   point size selected and the resolution of the device.
   *   The size parameters are of type FIXED, NOT int.
   *   NOTE: 1 point == 1/72 of an inch.
   */

  sizef.cx = (FIXED) (((fxPointSize) / 72) * lxFontResolution);
  sizef.cy = (FIXED) (((fxPointSize) / 72) * lyFontResolution);

  pfattrs->lMaxBaselineExt = MAKELONG(HIUSHORT(sizef.cy), 0);
  pfattrs->lAveCharWidth = MAKELONG(HIUSHORT(sizef.cx), 0);
  WinReleasePS(hps);

}                                       // end ConvertVectorPointSize()

VOID SetPresParamFromFattrs(HWND hwnd, FATTRS * fattrs,
			    SHORT sNominalPointSize, FIXED fxPointSize)
{

  CHAR s[CCHMAXPATH * 2];

  if (fattrs->fsFontUse != FATTR_FONTUSE_OUTLINE)
    sprintf(s, "%hd.", sNominalPointSize / 10);
  else {
    sprintf(s, "%hd.", FIXEDINT(fxPointSize));
    if ((((USHORT) FIXEDFRAC(fxPointSize) * 100) / 65536) > 0)
      sprintf(&s[strlen(s)], "%hd.",
	      ((USHORT) FIXEDFRAC(fxPointSize) * 100) / 65536);
  }
  strcat(s, fattrs->szFacename);
  if (fattrs->fsSelection & FATTR_SEL_ITALIC) {
    strcat(s, ".");
    strcat(s, GetPString(IDS_ITALICTEXT));
  }
  if (fattrs->fsSelection & FATTR_SEL_OUTLINE) {
    strcat(s, ".");
    strcat(s, GetPString(IDS_OUTLINETEXT));
  }
  if (fattrs->fsSelection & FATTR_SEL_BOLD) {
    strcat(s, ".");
    strcat(s, GetPString(IDS_BOLDTEXT));
  }
  if (fattrs->fsSelection & FATTR_SEL_UNDERSCORE) {
    strcat(s, ".");
    strcat(s, GetPString(IDS_UNDERSCORETEXT));
  }
  if (fattrs->fsSelection & FATTR_SEL_STRIKEOUT) {
    strcat(s, ".");
    strcat(s, GetPString(IDS_STRIKEOUTTEXT));
  }
  WinSetPresParam(hwnd, PP_FONTNAMESIZE, strlen(s) + 1, s);
}

#if 0   // JBS	11 Sep 08
VOID SetFont(HWND hwnd)
{

  FONTDLG fontdlg;
  HPS hps;
  FONTMETRICS fm;
  CHAR szFamily[CCHMAXPATH], *szTitle = GetPString(IDS_SETFONTTITLETEXT), *szPreview;

  DosRequestMutexSem(hmtxFM2Globals, SEM_INDEFINITE_WAIT);
  szPreview = GetPString(IDS_BLURB1TEXT + counter++);
  if (strcmp(szPreview, "0") == 0) {
    counter = 0;
    szPreview = GetPString(IDS_BLURB1TEXT + counter++);
  }
  DosReleaseMutexSem(hmtxFM2Globals);
  memset(&fontdlg, 0, sizeof(fontdlg)); // initialize all fields
  hps = WinGetPS(hwnd);
  GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm);
  WinReleasePS(hps);
  fontdlg.cbSize = sizeof(FONTDLG);
  fontdlg.hpsScreen = WinGetScreenPS(HWND_DESKTOP);
  fontdlg.hpsPrinter = NULLHANDLE;
  fontdlg.pszTitle = szTitle;
  fontdlg.pszPreview = szPreview;
  fontdlg.pfnDlgProc = NULL;
  strcpy(szFamily, fm.szFamilyname);
  fontdlg.pszFamilyname = szFamily;
  fontdlg.usFamilyBufLen = sizeof(szFamily);
  fontdlg.fxPointSize = MAKEFIXED(fm.sNominalPointSize / 10, 0);
  fontdlg.fl = FNTS_CENTER | FNTS_INITFROMFATTRS;
  fontdlg.sNominalPointSize = fm.sNominalPointSize;
  fontdlg.flType = (LONG) fm.fsType;
  fontdlg.clrFore = CLR_NEUTRAL;
  fontdlg.clrBack = CLR_BACKGROUND;
  fontdlg.usWeight = fm.usWeightClass;
  fontdlg.usWidth = fm.usWidthClass;
  if (!WinFontDlg(HWND_DESKTOP, hwnd, &fontdlg) || fontdlg.lReturn != DID_OK) {
    WinReleasePS(fontdlg.hpsScreen);
    return;
  }
  if (fontdlg.fAttrs.fsFontUse == FATTR_FONTUSE_OUTLINE)
    ConvertVectorFontSize(fontdlg.fxPointSize, &fontdlg.fAttrs);
  WinReleasePS(fontdlg.hpsScreen);
  SetPresParamFromFattrs(hwnd, &fontdlg.fAttrs, fontdlg.sNominalPointSize,
			 fontdlg.fxPointSize);
}
#endif

FATTRS *SetMLEFont(HWND hwndMLE, FATTRS * fattrs, ULONG flags)
{

  /**
   * Flags:
   *
   * 1 = Don't assume MLE (no MLM_* messages, use fattrs only
   * 2 = Fixed width fonts only
   * 4 = No synthesized fonts
   * 8 = No vector fonts
   * 16 = No bitmapped fonts
   *
   */

  FONTDLG fontDlg;
  HPS hps;
  FONTMETRICS fontMetrics;
  CHAR szFamily[CCHMAXPATH];
  PCSZ pcszPreview;
  static FIXED fxPointSize = 0; // keep track of this for vector fonts

  if ((flags & 1) && !fattrs)
    return fattrs;
  DosRequestMutexSem(hmtxFM2Globals, SEM_INDEFINITE_WAIT);
  // 12 Jan 09 SHL fixme to do multiple previews or rename to IDS_BLURBTEXT
  pcszPreview = GetPString(IDS_BLURB1TEXT);
  DosReleaseMutexSem(hmtxFM2Globals);
  memset(&fontDlg, 0, sizeof(fontDlg)); // initialize all fields
  // Get the current font attributes
  hps = WinGetPS(hwndMLE);
  if (!(flags & 1))
    WinSendMsg(hwndMLE, MLM_QUERYFONT,
	       MPFROMP((PFATTRS) & (fontDlg.fAttrs)), NULL);
  else
    memcpy(&fontDlg.fAttrs, fattrs, sizeof(FATTRS));

  // create system default font
  GpiCreateLogFont(hps, (PSTR8) fontDlg.fAttrs.szFacename, 1,
		   &(fontDlg.fAttrs));
  GpiSetCharSet(hps, 1);
  GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fontMetrics);
  GpiSetCharSet(hps, LCID_DEFAULT);
  GpiDeleteSetId(hps, 1);
  WinReleasePS(hps);

  // Initialize the FONTDLG structure with the current font
  fontDlg.cbSize = sizeof(FONTDLG);
  fontDlg.hpsScreen = WinGetScreenPS(HWND_DESKTOP);     // Screen presentation space
  fontDlg.hpsPrinter = NULLHANDLE;                      // Printer presentation space

  fontDlg.pszTitle = (PSZ)GetPString(IDS_SETVIEWERFONTTITLETEXT);
  fontDlg.pszPreview = (PSZ)pcszPreview;
  fontDlg.pszPtSizeList = NULL;                         // Application provided size list
  fontDlg.pfnDlgProc = NULL;                            // Dialog subclass procedure
  strcpy(szFamily, fontMetrics.szFamilyname);           // Family name of font        
  fontDlg.pszFamilyname = szFamily;                     // point to Family name of font
  fontDlg.fxPointSize = fxPointSize;                    // Point size the user selected
  fontDlg.fl = FNTS_CENTER |                            // FNTS_* flags - dialog styles
    FNTS_INITFROMFATTRS;
  if (flags & 2)
    fontDlg.fl |= FNTS_FIXEDWIDTHONLY;
  if (flags & 4)
    fontDlg.fl |= FNTS_NOSYNTHESIZEDFONTS;
  if (flags & 8)
    fontDlg.fl |= FNTS_BITMAPONLY;
  else if (flags & 16)
    fontDlg.fl |= FNTS_VECTORONLY;
  fontDlg.flFlags = 0;                  // FNTF_* state flags
  // Font type option bits
  fontDlg.flType = (LONG) fontMetrics.fsType;
  fontDlg.flTypeMask = 0;               // Mask of which font types to use 
  fontDlg.flStyle = 0;                  // The selected style bits         
  fontDlg.flStyleMask = 0;              // Mask of which style bits to use 
  fontDlg.clrFore = CLR_NEUTRAL;        // Selected foreground color       
  fontDlg.clrBack = CLR_BACKGROUND;     // Selected background color       
  fontDlg.ulUser = 0;                   // Blank field for application     
  fontDlg.lReturn = 0;                  // Return Value of the Dialog      
  fontDlg.lEmHeight = 0;                // Em height of the current font   
  fontDlg.lXHeight = 0;                 // X height of the current font    
  fontDlg.lExternalLeading = 0;         // External Leading of font        
  // Nominal Point Size of font
  fontDlg.sNominalPointSize = fontMetrics.sNominalPointSize;
  fontDlg.usWeight = fontMetrics.usWeightClass; // The boldness of the font
  fontDlg.usWidth = fontMetrics.usWidthClass;   // The width of the font
  fontDlg.x = 0;                                // X coordinate of the dialog
  fontDlg.y = 0;                                // Y coordinate of the dialog
  fontDlg.usFamilyBufLen = sizeof(szFamily);    // Length of family name buffer

  // Bring up the standard Font Dialog

  if (!WinFontDlg(HWND_DESKTOP, hwndMLE, &fontDlg)
      || fontDlg.lReturn != DID_OK) {
    WinReleasePS(fontDlg.hpsScreen);
    return NULL;
  }
  fxPointSize = fontDlg.fxPointSize;    // save point size for next dialog

  /**
   *   If outline font, calculate the maxbaselineext and
   *   avecharwidth for the point size selected
   */

  if (fontDlg.fAttrs.fsFontUse == FATTR_FONTUSE_OUTLINE)
    ConvertVectorFontSize(fontDlg.fxPointSize, &fontDlg.fAttrs);

  WinReleasePS(fontDlg.hpsScreen);
  if (!(flags & 1))
    WinSendMsg(hwndMLE, MLM_SETFONT, MPFROMP(&(fontDlg.fAttrs)), MPVOID);
  if (fattrs)
    memcpy(fattrs, &fontDlg.fAttrs, sizeof(FATTRS));
  return fattrs;

}                                       // End of SetMLEFont()

#pragma alloc_text(FONTS,ConvertVectorFontSize,SetFont,SetMLEFont)
#pragma alloc_text(FONTS,SetPresParamFromFattrs)
