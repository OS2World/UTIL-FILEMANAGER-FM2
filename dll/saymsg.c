#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "fm3dll.h"
#include "fm3str.h"

#pragma alloc_text(FMINPUT,saymsg)


APIRET saymsg (APIRET type,HWND hwnd,CHAR *title,CHAR *string,...) {

  CHAR        *buffer;
  va_list     ap;
  APIRET      ret;

  buffer = malloc(4096);
  if(!buffer) {
    WinMessageBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  GetPString(IDS_OUTOFMEMORY),
                  title,
                  0,
                  MB_ENTER);
    return -1;
  }
  va_start(ap,string);
  vsprintf(buffer,string,ap);
  va_end(ap);
  if(!hwnd)
    hwnd = HWND_DESKTOP;
  ret = WinMessageBox(HWND_DESKTOP,hwnd,buffer,title,
                      0,type | MB_MOVEABLE);
  free(buffer);
  return ret;
}
