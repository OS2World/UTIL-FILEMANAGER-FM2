
/***********************************************************************

  $Id$

  Load bitmaps

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2006, 2008 Steven H. Levine

  22 Jul 06 SHL Check more run time errors
  16 Jan 07 SHL Check more run time errors
  16 Jan 07 SHL Sync variable names for sanity
  16 Jan 07 SHL Open bitmap file binary - no wonder the code does not work
  16 Jan 07 SHL Beautify with indent -i2
  18 Apr 08 SHL LoadBitmapFromFile ensure pf initialized if no hPS
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory

***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <share.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG			// dircnrs.h

#include "errutil.h"			// Dos_Error...
#include "loadbmp.h"
#include "fm3dll.h"

static PSZ pszSrcFile = __FILE__;

static HBITMAP LoadBitmapFromFile(CHAR * pszFileName);

HBITMAP LoadBitmapFromFileNum(USHORT id)
{
  char s[CCHMAXPATH];

  strcpy(s, pFM2SaveDirectory);
  sprintf(s + strlen(s), "\\%u.BMP", id);
  return LoadBitmapFromFile(s);
}

HBITMAP LoadBitmapFromFile(CHAR * pszFileName)
{
  HBITMAP hBmp = (HBITMAP) 0;
  FILE *pf;
  ULONG rc;
  USHORT usType;
  PBITMAPARRAYFILEHEADER2 pbmafh2 = NULL;	// Must init for xfree
  PBITMAPFILEHEADER2 pbmfh2;	// No color table
  PBITMAPINFOHEADER2 pbmih2;	// No color table
  PBITMAPINFO2 pbmi2;		// Includes color table
  BOOL is2x;			// Format 1.x or 2.x
  ULONG ulColors;
  ULONG ulRGBOffset;
  PBYTE pData = NULL;		// Must init for xfree
  ULONG ulDataSize;
  SIZEL sizel;
  HPS hPS = WinGetPS(HWND_DESKTOP);

  if (!hPS) {
    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__, "WinGetPS");
    pf = NULL;
    goto ExitLoadBMP;
  }

  pf = _fsopen(pszFileName, "rb", SH_DENYWR);
  if (!pf) {
    // OK for file to not exist - enable following for debug as needed
    // Runtime_Error(pszSrcFile, __LINE__, "_fsopen %s", pszFileName);
    goto ExitLoadBMP;
  }

  // Read image type
  // fixme to just read type2 header - this is silly and wastes time
  rc = fread(&usType, 1, sizeof(usType), pf);
  if (rc != sizeof(usType)) {
    Runtime_Error(pszSrcFile, __LINE__, "fread");
    goto ExitLoadBMP;
  }

  /* Read bitmap info header
     Allocate enough to hold a complete 2.x bitmap array file header
     fixme to support > 256 colors?
   */
  pbmafh2 =
    xmalloc(sizeof(*pbmafh2) + 256 * sizeof(RGB2), pszSrcFile, __LINE__);
  if (!pbmafh2)
    goto ExitLoadBMP;
  /* Assign pointers to the file header and bitmap info header etc.
     Both the 1.x and 2.x structures are assigned to simplify code
     fixme to clean this up - aliased pointers are evil
   */
  pbmfh2 = &pbmafh2->bfh2;
  pbmih2 = &pbmfh2->bmp2;
  pbmi2 = (PBITMAPINFO2) pbmih2;

  switch (usType) {
  case BFT_BMAP:
  case BFT_ICON:
  case BFT_POINTER:
  case BFT_COLORICON:
  case BFT_COLORPOINTER:
    {
      /* Assume image is a 2.0 image and read as a 2.x header
         OK for 1.x file - read will not fail unless file is corrupted
       */
      rc = fseek(pf, 0, SEEK_SET);
      if (rc) {
	Runtime_Error(pszSrcFile, __LINE__, "fseek 0");
	goto ExitLoadBMP;
      }

      rc = fread(pbmfh2, 1, sizeof(*pbmfh2), pf);
      if (rc != sizeof(*pbmfh2)) {
	Runtime_Error(pszSrcFile, __LINE__, "fread");
	goto ExitLoadBMP;
      }

      is2x = pbmih2->cbFix > sizeof(BITMAPINFOHEADER);	// 1.x or 2.x bitmap
      /* We will read the color table later
         Color table follows header but
         location depends on the type of the bitmap (old vs new)
         1.x header is fixed size
         2.x header is variable sized, so offset must be calculated
         cbFix contains actual size of BITMAPINFOHEADER2 in file
       */
      ulRGBOffset = is2x ? sizeof(*pbmfh2) - sizeof(*pbmih2) + pbmih2->cbFix :
	sizeof(BITMAPFILEHEADER);
    }
    break;

  case BFT_BITMAPARRAY:
    {
      /* Now we are dealing with a bitmap array which is a collection of bitmaps
         Each bitmap has its own file header
       */

      ULONG ulCurOffset;
      ULONG clScreenWidth;
      ULONG clScreenHeight;
      ULONG ulDeviceColors;
      ULONG ulSizeDiff;
      ULONG ulOffsetPicked = 0;
      ULONG ulColorsPicked;
      ULONG ulSizeDiffPicked;
      HDC hdc;

      /* Scan the array and chose the bitmap best suited
         for the current display size and color capacities
       */
      hdc = GpiQueryDevice(hPS);
      if (!hdc) {
	Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		  "GpiQueryDevice");
	goto ExitLoadBMP;
      }
      DevQueryCaps(hdc, CAPS_COLORS, 1, (PLONG) & ulDeviceColors);
      DevQueryCaps(hdc, CAPS_WIDTH, 1, (PLONG) & clScreenWidth);
      DevQueryCaps(hdc, CAPS_HEIGHT, 1, (PLONG) & clScreenHeight);
      pbmafh2->offNext = 0;
      do {
	ulCurOffset = pbmafh2->offNext;
	rc = fseek(pf, pbmafh2->offNext, SEEK_SET);
	if (rc) {
	  Runtime_Error(pszSrcFile, __LINE__, "fseek %ld", pbmafh2->offNext);
	  goto ExitLoadBMP;
	}
	rc = fread(pbmafh2, 1, sizeof(*pbmafh2), pf);
	if (rc != sizeof(*pbmafh2)) {
	  Runtime_Error(pszSrcFile, __LINE__, "fread");
	  goto ExitLoadBMP;
	}
	is2x = pbmih2->cbFix > sizeof(BITMAPINFOHEADER);
	if (is2x) {
	  ulColors = 1 << (pbmafh2->bfh2.bmp2.cBitCount *
			   pbmafh2->bfh2.bmp2.cPlanes);
	}
	else {
	  ulColors =
	    1 << (((PBITMAPARRAYFILEHEADER) pbmafh2)->bfh.bmp.cBitCount *
		  ((PBITMAPARRAYFILEHEADER) pbmafh2)->bfh.bmp.cPlanes);
	}
	if (pbmafh2->cxDisplay == 0 && pbmafh2->cyDisplay == 0) {
	  // This is a device independant bitmap - process it as a VGA
	  pbmafh2->cxDisplay = 640;
	  pbmafh2->cyDisplay = 480;
	}
	ulSizeDiff = abs(pbmafh2->cxDisplay - clScreenWidth) +
	  abs(pbmafh2->cyDisplay - clScreenHeight);
	if (ulDeviceColors == ulColors && ulSizeDiff == 0) {
	  // We found the perfect match
	  ulOffsetPicked = ulCurOffset;
	  break;			// Stop scan
	}
	if (ulOffsetPicked == 0 ||	// First time thru
	    ulSizeDiff < ulSizeDiffPicked ||	// Better fit than any previous
	    (ulColors > ulColorsPicked && ulColors < ulDeviceColors) ||	// More colors than prev & less than device
	    (ulColors < ulColorsPicked && ulColors > ulDeviceColors)) {
	  ulOffsetPicked = ulCurOffset;	// Make this our current pick
	  ulColorsPicked = ulColors;
	  ulSizeDiffPicked = ulSizeDiff;
	}
      } while (pbmafh2->offNext != 0);

      // Retrieve the selected bitmap
      rc = fseek(pf, ulOffsetPicked, SEEK_SET);
      if (rc) {
	Runtime_Error(pszSrcFile, __LINE__, "fseek %ld", ulOffsetPicked);
	goto ExitLoadBMP;
      }
      rc = fread(pbmafh2, 1, sizeof(*pbmafh2), pf);
      if (rc != sizeof(*pbmafh2)) {
	Runtime_Error(pszSrcFile, __LINE__, "fread");
	goto ExitLoadBMP;
      }

      is2x = pbmih2->cbFix > sizeof(BITMAPINFOHEADER);
      /* As before, we calculate offset in file stream to color table
         This code must match single bitmap logic
       */
      ulRGBOffset = ulOffsetPicked;
      ulRGBOffset +=
	is2x ? sizeof(*pbmafh2) - sizeof(*pbmih2) +
	pbmih2->cbFix : sizeof(BITMAPARRAYFILEHEADER);
    }
    break;

  default:
    Runtime_Error(pszSrcFile, __LINE__, "Bad type %u", usType);
    goto ExitLoadBMP;
  }					// endswitch

  // Position to color table
  rc = fseek(pf, ulRGBOffset, SEEK_SET);
  if (rc) {
    Runtime_Error(pszSrcFile, __LINE__, "fseek %ld", ulRGBOffset);
    goto ExitLoadBMP;
  }

  // Read color table
  if (is2x) {
    /* For a 2.0 bitmap, read the color table as is
       The bitmap info structure is header + color table
       If we have 24 bits per pel, there is usually no color table, unless
       pbmih2->cclrUsed or pbmih2->cclrImportant are non zero
       fixme to test this
     */
    if (pbmih2->cBitCount < 24) {
      ULONG ulRGBBytes;

      ulColors = 1L << pbmih2->cBitCount;

      if (ulColors > 256) {
	Runtime_Error(pszSrcFile, __LINE__, "RGB exceeds 256 colors: %lu",
		      ulColors);
	goto ExitLoadBMP;
      }
      ulRGBBytes = ulColors * sizeof(RGB2);
      rc = fread(&pbmi2->argbColor[0], 1, ulRGBBytes, pf);
      if (rc != ulRGBBytes) {
	Runtime_Error(pszSrcFile, __LINE__, "fread");
	goto ExitLoadBMP;
      }
    }					// endif
    // Get pointer to bitmap info (header and color table)
    pbmi2 = (PBITMAPINFO2) pbmih2;
  }
  else {
    /* This is a 1.x format bitmap
       Since the current standard format is the 2.0
       convert the header and color table to 2.x format
     */
    ULONG ul;
    RGB rgb;
    PBITMAPINFOHEADER pbmih = &((PBITMAPARRAYFILEHEADER) pbmafh2)->bfh.bmp;

    if (pbmih->cBitCount < 24) {
      ulColors = 1 << pbmih->cBitCount;
      if (ulColors > 256) {
	Runtime_Error(pszSrcFile, __LINE__, "RGB exceeds 256 colors: %lu",
		      ulColors);
	goto ExitLoadBMP;
      }
      // Read in 1.x color table and reformat for 2.x
      for (ul = 0; ul < ulColors; ul++) {
	fread(&rgb, 1, sizeof(rgb), pf);
	pbmi2->argbColor[ul].bRed = rgb.bRed;
	pbmi2->argbColor[ul].bGreen = rgb.bGreen;
	pbmi2->argbColor[ul].bBlue = rgb.bBlue;
	pbmi2->argbColor[ul].fcOptions = 0;	// initialize 2.x extra byte to 0
      }					// for
    }

    // Convert the old style to the new header format
    pbmi2->cbFix = sizeof(BITMAPINFOHEADER2);
    pbmi2->cBitCount = pbmih->cBitCount;
    pbmi2->cPlanes = pbmih->cPlanes;
    pbmi2->cy = pbmih->cy;
    pbmi2->cx = pbmih->cx;
    // set rest to zero
    memset((PCHAR) pbmi2 + 16, 0, sizeof(BITMAPINFOHEADER2) - 16);
  }					// if 1.x

  /* The 2.0 bitmap info structure set up
     Position to start of the bitmap data
   */
  rc = fseek(pf, pbmfh2->offBits, SEEK_SET);
  if (rc) {
    Runtime_Error(pszSrcFile, __LINE__, "fseek %ld", pbmfh2->offBits);
    goto ExitLoadBMP;
  }

  /* Read the bitmap data
     The read size is derived using the magic formula
     Each bitmap scan line is aligned on a doubleword boundary
     The size of the scan line is the number of pels times the bpp
     After aligning it, we divide by 4 to get the number of bytes, and
     multiply by the number of scan lines and the number of pel planes
   */
  if (pbmi2->ulCompression)
    ulDataSize = pbmi2->cbImage;
  else
    ulDataSize = (((pbmi2->cBitCount * pbmi2->cx) + 31) / 32) * 4 *
      pbmi2->cy * pbmi2->cPlanes;
  pData = xmalloc(ulDataSize, pszSrcFile, __LINE__);
  if (!pData)
    goto ExitLoadBMP;
  rc = fread(pData, 1, ulDataSize, pf);
  if (rc != ulDataSize) {
    Runtime_Error(pszSrcFile, __LINE__, "fread");
    goto ExitLoadBMP;
  }

  // Create the GPI bitmap image
  sizel.cx = pbmi2->cx;
  sizel.cy = pbmi2->cy;

  hBmp = GpiCreateBitmap(hPS, (PBITMAPINFOHEADER2) pbmi2, CBM_INIT,
			 pData, pbmi2);
  if (!hBmp)
    Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
	      "GpiCreateBitmap");

ExitLoadBMP:

  xfree(pData, pszSrcFile, __LINE__);
  xfree(pbmafh2, pszSrcFile, __LINE__);
  if (pf)
    fclose(pf);
  if (hPS)
    WinReleasePS(hPS);
  return hBmp;
}

#pragma alloc_text(LOADBITMAP,LoadBitmapFromFile,LoadBitmapFromFileNum)
