#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <share.h>
#include <io.h>
#include "fm3dll.h"
#include "fm3str.h"
#include "version.h"

#pragma alloc_text(STRINGS,LoadStrings,GetPString)

static char **strs,*str;
static ULONG  numStr;


BOOL LoadStrings (char *filename) {

  /*
   * Load strings from FM3RES.STR, with some error-checking.
   * Return TRUE on success, FALSE on error.
   */

  BOOL           ret = FALSE;
  ULONG          size,len,totalsize;
  USHORT         vermajor = 0,verminor = 0;
  register char *p;
  register ULONG x;
  FILE          *fp;

  if(!filename)
    filename = "FM3RES.STR";
  numStr = 0;
  if(str)
    DosFreeMem(str);
  strs = NULL;
  str = NULL;

  fp = _fsopen(filename,
               "rb",
               SH_DENYWR);
  if(fp) {
    if(fread(&numStr,
             sizeof(numStr),
             1,
             fp) &&
       numStr == IDS_NUMSTRS &&
       fread(&len,sizeof(len),1,fp) &&
       fread(&vermajor,sizeof(vermajor),1,fp) &&
       fread(&verminor,sizeof(verminor),1,fp) &&
       (vermajor >= VERMAJORBREAK &&
        (vermajor > VERMAJORBREAK ||
         verminor >= VERMINORBREAK))) {
      fseek(fp,0,SEEK_END);
      size = ftell(fp) - ((sizeof(ULONG) * 2) + (sizeof(USHORT) * 2));
      if(size &&
         size == len) {
        fseek(fp,(sizeof(ULONG) * 2) + (sizeof(USHORT) * 2),SEEK_SET);
        /*
         * NOTE:  Make one memory object for both str and strs
         * for efficiency.
         */
        totalsize = size + sizeof(ULONG);
        totalsize += (totalsize % sizeof(ULONG));
        len = totalsize;
        totalsize += (numStr * sizeof(char *));
        totalsize += 4;
        if(!DosAllocMem((PPVOID)&str,totalsize,
                        PAG_COMMIT | PAG_READ | PAG_WRITE) &&
           str) {
          strs = (char **)(str + len);
          if(fread(str,1,size,fp) == size) {
            p = str;
            for(x = 0;x < numStr;x++) {
              if(p - str >= size)
                break;
              strs[x] = p;
              while(*p)
                p++;
              p++;
            }
            if(x == numStr)
              ret = TRUE;
          }
          if(ret)
            /* set pages to readonly */
            DosSetMem(str,
                      totalsize,
                      PAG_COMMIT | PAG_READ);
        }
      }
    }
    fclose(fp);
  }

  if(!ret) {
    numStr = 0;
    if(str)
      DosFreeMem(str);
    str = NULL;
    strs = NULL;
  }

  return ret;
}


char *GetPString (ULONG id) {

  /*
   * return a readonly pointer to the requested string in memory
   */

  return (id < numStr && str && strs && strs[id]) ? strs[id] : NullStr;
}


BOOL StringsLoaded (void) {

  return (numStr && str && strs);
}
