#define INCL_DOS
#define INCL_WIN
#define INCL_GPI

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <share.h>
#include "fm3dll.h"

#pragma alloc_text(LOADBITMAP,LoadBitmapFromFile,LoadBitmapFromFileNum)


HBITMAP LoadBitmapFromFileNum (USHORT id) {

  char s[CCHMAXPATH];

  save_dir2(s);
  sprintf(s + strlen(s),"\\%u.BMP",id);
  return LoadBitmapFromFile(s);
}


HBITMAP LoadBitmapFromFile (CHAR *pszFileName) {

  HBITMAP                 hBmp = (HBITMAP)0;
  FILE                   *File;
  ULONG                   rc;
  USHORT                  usType = 0;           // #@!!! compiler warnings
  PBITMAPARRAYFILEHEADER2 pbafh2 = NULL;        // (MAM) chng init values to NULL instead of ptr to 0
  PBITMAPFILEHEADER2      pbfh2  = NULL;                       
  PBITMAPINFOHEADER2      pbih2  = NULL;                       
  PBITMAPINFO2            pbmi2  = NULL;                       
  PBITMAPARRAYFILEHEADER  pbafh  = NULL;                       
  PBITMAPFILEHEADER       pbfh   = NULL;                       
  PBITMAPINFOHEADER       pbih   = NULL;                       
  BOOL                    f2;        // format 1.x or 2.x
  ULONG                   ulOffset;
  PBYTE                   pData  = NULL;
  ULONG                   ulDataSize;
  SIZEL                   sizel;
  HPS                     hPS = WinGetPS(HWND_DESKTOP);

  //--- open the file
  File = _fsopen(pszFileName,"r",SH_DENYWR);
  if(!File)
    goto ExitLoadBMP;

  /* Read image type, and reset the stream...................................*/
  /* The image type is a USHORT, so we only read that........................*/
  rc = fread(&usType,1,sizeof(usType),File);
  if(rc != sizeof(usType))
    goto ExitLoadBMP;

  /* Next read the bitmap info header........................................*/
  // we allocate enough to hold a complete bitmap array file header
  pbafh2 = (PBITMAPARRAYFILEHEADER2)malloc(sizeof(*pbafh2) +
                                             256 * sizeof(RGB2));
  if(!pbafh2)
    goto ExitLoadBMP;
  /* Next we assign pointers to the file header and bitmap info header...*/
  /* Both the 1.x and 2.x structures are assigned just in case...........*/
  pbfh2 = &pbafh2->bfh2;
  pbih2 = &pbfh2->bmp2;
  pbmi2 = (PBITMAPINFO2)pbih2;
  pbafh = (PBITMAPARRAYFILEHEADER)pbafh2;
  pbfh  = &pbafh->bfh;
  pbih  = &pbfh->bmp;
  switch (usType) {
    case BFT_BMAP:
    case BFT_ICON:
    case BFT_POINTER:
    case BFT_COLORICON:
    case BFT_COLORPOINTER:
      {
        /* Now we assume the image is a 2.0 image and so we read a bitmap-file-*/
        /* Now we reset the stream, next we'll read the bitmap info header. To do .*/
        /* this we need to reset the stream to 0...................................*/
        fseek(File,0,SEEK_SET);
        /*-header-2 structure..................................................*/
        rc = fread(pbfh2,1,sizeof(*pbfh2),File);
        if(rc != sizeof(*pbfh2))
          goto ExitLoadBMP;

        f2 = pbih2->cbFix > sizeof(*pbih); // 1.x or 2.x bitmap
        /* We will need to read the color table. Thus we position the stream...*/
        /* so that the next read will read IT. This, of course, depends on the.*/
        /* type of the bitmap (old vs new), note that in the NEW case, we can..*/
        /* not be certain of the size of the bitmap header.....................*/
        ulOffset = (f2) ? sizeof(*pbfh2) + pbih->cbFix - sizeof(*pbih2) :
                          sizeof(*pbfh);
      }
      break;

    case BFT_BITMAPARRAY:
      {
        /* Now we are dealing with a bitmap array. This is a collection of ....*/
        /* bitmap files and each has its own file header.......................*/

        BOOL   bBest = FALSE;
        ULONG  ulCurOffset, ulOffsetTemp = 0;
        LONG   lScreenWidth;
        LONG   lScreenHeight;
        LONG   lClrsDev, lClrsTemp = 0;
        LONG   lClrs;
        ULONG  ulSizeDiff, ulSizeDiffTemp = 0xffffffff;
        HDC    hdc;

        // -- We will browse through the array and chose the bitmap best suited
        // -- for the current display size and color capacities.
        hdc = GpiQueryDevice( hPS );
        DevQueryCaps(hdc,CAPS_COLORS,1,&lClrsDev);
        DevQueryCaps(hdc,CAPS_WIDTH, 1,&lScreenWidth);
        DevQueryCaps(hdc,CAPS_HEIGHT,1,&lScreenHeight);
        pbafh2->offNext = 0;
        do {
           ulCurOffset = pbafh2->offNext;
           rc = fseek(File,pbafh2->offNext,SEEK_SET);
           if(rc)
             goto ExitLoadBMP;
           rc = fread(pbafh2,1,sizeof(*pbafh2),File);
           if(rc != sizeof(*pbafh2))
             goto ExitLoadBMP;
           f2 = pbih2->cbFix > sizeof(*pbih);
           if(f2)
             lClrs = 1 << (pbafh2->bfh2.bmp2.cBitCount *
                           pbafh2->bfh2.bmp2.cPlanes);
           else
             lClrs = 1 << (pbafh->bfh.bmp.cBitCount *
                           pbafh->bfh.bmp.cPlanes);
           if((pbafh2->cxDisplay == 0) && (pbafh2->cyDisplay == 0)) {
             // This is a device independant bitmap
             // Process it as a VGA
             pbafh2->cxDisplay = 640;
             pbafh2->cyDisplay = 480;
           } // endif
           ulSizeDiff = abs(pbafh2->cxDisplay - lScreenWidth) +
                            abs(pbafh2->cyDisplay - lScreenHeight);
           if((lClrsDev == lClrs) &&
              (ulSizeDiff == 0)) {
             // We found the perfect match
             bBest = TRUE;
             ulOffsetTemp = ulCurOffset;
           }
           else {
             if((ulOffsetTemp == 0) ||           // First time thru
                (ulSizeDiff < ulSizeDiffTemp) || // Better fit than any previous
                  ((lClrs > lClrsTemp) && (lClrs < lClrsDev)) || // More colors than prev & less than device
                  ((lClrs < lClrsTemp) && (lClrs > lClrsDev))) {
               ulOffsetTemp = ulCurOffset;       // Make this our current pick
               lClrsTemp   = lClrs;
               ulSizeDiffTemp = ulSizeDiff;
             } // endif
           } // endif
        } while((pbafh2->offNext != 0) && !bBest); // enddo

        // Now retrieve the best bitmap
        rc = fseek(File,ulOffsetTemp,SEEK_SET);
        if(rc)
          goto ExitLoadBMP;
        rc = fread(pbafh2,1,sizeof(*pbafh2),File);
        if(rc != sizeof(*pbafh2))
          goto ExitLoadBMP;

        f2 = pbih2->cbFix > sizeof(*pbih);
        /* as before, we calculate where to position the stream in order to ...*/
        /* read the color table information....................................*/
        ulOffset = ulOffsetTemp;
        ulOffset += (f2) ? sizeof(*pbafh2) + pbih2->cbFix - sizeof(*pbih2):
                           sizeof(*pbafh);
      }
      break;

    default:
       goto ExitLoadBMP;
  } /* endswitch */

  /* We now position the stream on the color table information...............*/
  rc = fseek(File,ulOffset,SEEK_SET);
  if(rc)
    goto ExitLoadBMP;

  /* Read the color table....................................................*/
  if(f2) {
    /* For a 2.0 bitmap, all we need to do is read the color table...........*/
    /* The bitmap info structure is just the header + color table............*/
    // If we have 24 bits per pel, there is usually no color table, unless
    // pbih2->cclrUsed or pbih2->cclrImportant are non zero, we should
    // test that !
    if(pbih2->cBitCount < 24) {

      ULONG ul = (1 << pbih2->cBitCount) * sizeof(RGB2);

      rc = fread(&pbmi2->argbColor[0],1,ul,File);
      if(rc != ul)
        goto ExitLoadBMP;
    } // endif
    /* remember the bitmap info we mentioned just above?.....................*/
    pbmi2 = (PBITMAPINFO2)pbih2;
  }
  else {
    /* This is an old format bitmap. Since the common format is the 2.0......*/
    /* We have to convert all the RGB entries to RGB2........................*/

    ULONG ul, cColors;
    RGB   rgb;

    if(pbih->cBitCount <24)
      cColors = 1 << pbih->cBitCount;
    else
    // If there are 24 bits per pel, the 24 bits are assumed to be a RGB value
      cColors = 0;
    /* Loop over the original table and create the new table, the extra byte.*/
    /* has to be 0...........................................................*/
    for(ul = 0; ul < cColors; ul++) {
      fread(&rgb,1,sizeof(rgb),File);
      pbmi2->argbColor[ul].bRed      = rgb.bRed;
      pbmi2->argbColor[ul].bGreen    = rgb.bGreen;
      pbmi2->argbColor[ul].bBlue     = rgb.bBlue;
      pbmi2->argbColor[ul].fcOptions = 0;
    } /* endfor */

    // we have to convert the old to the new version header
    pbmi2->cbFix = sizeof(*pbih2);
    pbmi2->cBitCount = pbih->cBitCount;
    pbmi2->cPlanes = pbih->cPlanes;
    pbmi2->cy = pbih->cy;
    pbmi2->cx = pbih->cx;
    // set rest to zero
    memset((PCHAR)pbmi2 + 16,0,sizeof(*pbih2) - 16);
  } /* endif */

  /* We have the 2.0 bitmap info structure set...............................*/
  /* move to the stream to the start of the bitmap data......................*/
  rc = fseek(File,pbfh2->offBits,SEEK_SET);
  if(rc)
    goto ExitLoadBMP;

  /* Read the bitmap data, the read size is derived using the magic formula..*/
  /* The bitmap scan line is aligned on a doubleword boundary................*/
  /* The size of the scan line is the number of pels times the bpp...........*/
  /* After aligning it, we divide by 4 to get the number of bytes, and.......*/
  /* multiply by the number of scan lines and the number of pel planes.......*/
  if(pbmi2->ulCompression)
    ulDataSize = pbmi2->cbImage;
  else
    ulDataSize = (((pbmi2->cBitCount * pbmi2->cx) + 31) / 32) * 4 *
                  pbmi2->cy * pbmi2->cPlanes;
  pData = (PBYTE)malloc(ulDataSize);
  if(!pData)
    goto ExitLoadBMP;
  rc = fread(pData, 1, ulDataSize, File);
  if(rc != ulDataSize)
    goto ExitLoadBMP;

  /* Now, we create the bitmap...............................................*/
  sizel.cx = pbmi2->cx;
  sizel.cy = pbmi2->cy;

  hBmp = GpiCreateBitmap(hPS,(PBITMAPINFOHEADER2)pbmi2,CBM_INIT,
                         pData,pbmi2);

ExitLoadBMP:
  if(pData)
    free(pData);
  if(pbafh2)
    free(pbafh2);
  fclose(File);
  WinReleasePS(hPS);
  return(hBmp);
}

