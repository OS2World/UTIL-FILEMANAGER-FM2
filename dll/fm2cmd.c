#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma data_seg(DATA2)
#pragma alloc_text(FM2CMD,FM2Command,fullname,parse)


static VOID fullname (CHAR *directory,CHAR *name) {

  CHAR temp[CCHMAXPATH];

  if(!*name) {
    strcpy(name,directory);
    return;
  }
  if(!strchr(name,':')) {
    if(*name != '\\' && *name != '/') {
      strcpy(temp,directory);
      if(directory[strlen(directory) - 1] != '\\')
        strcat(temp,"\\");
    }
    else {
      *temp = *directory;
      temp[1] = ':';
      temp[2] = 0;
    }
    strcat(temp,name);
    strcpy(name,temp);
  }
  MakeFullName(name);
}


static VOID parse (CHAR *command,CHAR *key,CHAR *rest) {

  CHAR *p;

  *key = *rest = 0;
  strcpy(key,command);
  p = strchr(key,' ');
  if(p) {
    *p = 0;
    p++;
    p = skip_delim(p," \t");
    strcpy(rest,p);
  }
}


BOOL FM2Command (CHAR *directory,CHAR *command) {

  BOOL ret = FALSE;
  CHAR key[CCHMAXPATH],rest[CCHMAXPATH];
  HWND hwnd;

  if(command && *command == '/') {
    parse(command,key,rest);
    if(!stricmp(key,GetPString(IDS_OPENCMDTEXT))) {
      fullname(directory,rest);
      WinSendMsg(hwndTree,UM_OPENWINDOWFORME,MPFROMP(rest),MPVOID);
      ret = TRUE;
    }
    else if(!stricmp(key,GetPString(IDS_CLOSECMDTEXT))) {
      fullname(directory,rest);
      hwnd = FindDirCnrByName(rest,FALSE);
      if(hwnd)
        PostMsg(hwnd,WM_CLOSE,MPVOID,MPVOID);
      ret = TRUE;
    }
    else if(!stricmp(key,GetPString(IDS_HELPCMDTEXT))) {
      saymsg(MB_ENTER,(hwndMain) ? hwndMain : HWND_DESKTOP,
             GetPString(IDS_FM2CMDHELPHDRTEXT),"%s",
             GetPString(IDS_FM2CMDHELPTEXT));
      ret = TRUE;
    }
    else if(!stricmp(key,GetPString(IDS_FILTERCMDTEXT))) {
      hwnd = FindDirCnrByName(directory,FALSE);
      if(hwnd) {
        WinSendMsg(hwnd,UM_FILTER,MPFROMP(rest),MPVOID);
        ret = TRUE;
      }
    }
    else if(!stricmp(key,GetPString(IDS_KEEPCMDTEXT)) ||
            !stricmp(key,GetPString(IDS_NOKEEPCMDTEXT))) {
      if(!stricmp(key,GetPString(IDS_NOKEEPCMDTEXT)))
        fKeepCmdLine = FALSE;
      else
        fKeepCmdLine = TRUE;
      PrfWriteProfileData(fmprof,FM3Str,"KeepCmdLine",&fKeepCmdLine,
                          sizeof(BOOL));
      ret = TRUE;
    }
    else if(!stricmp(key,GetPString(IDS_SAVECMDTEXT)) ||
            !stricmp(key,GetPString(IDS_NOSAVECMDTEXT))) {
      if(!stricmp(key,GetPString(IDS_NOSAVECMDTEXT)))
        fSaveMiniCmds = FALSE;
      else
        fSaveMiniCmds = TRUE;
      PrfWriteProfileData(fmprof,FM3Str,"SaveMiniCmds",&fSaveMiniCmds,
                          sizeof(BOOL));
      ret = TRUE;
    }
  }
  return ret;
}
