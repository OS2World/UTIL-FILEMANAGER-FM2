
/***********************************************************************

  $Id$

  Compare directories

  Copyright (c) 1993-02 M. Kimes
  Copyright (c) 2003, 2006 Steven H. Levine

  16 Oct 02 MK Baseline
  04 Nov 03 SHL Force window refresh after subdir toggle
  01 Aug 04 SHL Rework lstrip/rstrip usage
  24 May 05 SHL Rework Win_Error usage
  24 May 05 SHL Rework for CNRITEM.szSubject
  25 May 05 SHL Rework with ULONGLONG
  06 Jun 05 SHL Drop unused
  12 Jul 06 SHL Renames and comments
  13 Jul 06 SHL Use Runtime_Error

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_LONGLONG
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <share.h>
#include <io.h>

#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma alloc_text(COMPAREDIR,FillCnrsThread,FillDirList,CompNames)
#pragma alloc_text(COMPAREDIR1,CompareDlgProc)
#pragma alloc_text(COMPAREDIR2,SelectCnrsThread,ActionCnrThread)
#pragma alloc_text(COMPAREFILE,CFileDlgProc,CompareFilesThread)
#pragma alloc_text(SNAPSHOT,SnapShot,StartSnap)

typedef struct {
  CHAR  filename[CCHMAXPATH];
  CHAR  dirname[CCHMAXPATH];
  BOOL  recurse;
} SNAPSTUFF;

static PSZ pszSrcFile = __FILE__;

//=== SnapShot() Write directory tree to file and recurse if requested ===

static VOID SnapShot (char *path,FILE *fp,BOOL recurse)
{
  FILEFINDBUF4 *fb;
  char         *mask,*enddir;
  HDIR          hdir = HDIR_CREATE;
  ULONG         nm = 1L;

  fb = xmalloc(sizeof(FILEFINDBUF4),pszSrcFile,__LINE__);
  if(fb) {
    mask = xmalloc(CCHMAXPATH,pszSrcFile,__LINE__);
    if(mask) {
      sprintf(mask,
              "%s%s*",
              path,
              (path[strlen(path) - 1] != '\\') ? "\\" : NullStr);
      enddir = strrchr(mask,'\\');
      enddir++;
      if(!DosFindFirst(mask,
                       &hdir,
                       FILE_NORMAL | FILE_DIRECTORY |
                       FILE_ARCHIVED | FILE_READONLY | FILE_HIDDEN |
                       FILE_SYSTEM,
                       fb,
                       sizeof(FILEFINDBUF4),
                       &nm,
                       FIL_QUERYEASIZE)) {
        do {
          strcpy(enddir,fb->achName);
          if(!(fb->attrFile & FILE_DIRECTORY))
            fprintf(fp,
                    "\"%s\",%u,%lu,%04u/%02u/%02u,%02u:%02u:%02u,%lu,%lu,N\n",
                    mask,
                    enddir - mask,
                    fb->cbFile,
                    (fb->fdateLastWrite.year + 1980),
                    fb->fdateLastWrite.month,
                    fb->fdateLastWrite.day,
                    fb->ftimeLastWrite.hours,
                    fb->ftimeLastWrite.minutes,
                    fb->ftimeLastWrite.twosecs,
                    fb->attrFile,
                    (fb->cbList > 4L) ? (fb->cbList / 2L) : 0L);
          else if(recurse && (*fb->achName != '.' ||
                  (fb->achName[1] && fb->achName[1] != '.')))
            SnapShot(mask,fp,recurse);
          nm = 1L;
        } while(!DosFindNext(hdir,fb,sizeof(FILEFINDBUF4),&nm));
        DosFindClose(hdir);
      }
      free(mask);
    }
    free(fb);
  }
}

//=== StartSnap() Write directory tree to snapshot file ===

static VOID StartSnap (VOID *dummy)
{
  SNAPSTUFF  *sf = (SNAPSTUFF *)dummy;
  FILE       *fp;
  CHAR       *p;

  if(sf) {
    if(*sf->dirname && *sf->filename) {
      priority_normal();
      p = sf->dirname;
      while(*p) {
        if(*p == '/')
          *p = '\\';
        p++;
      }
      if(*(p - 1) != '\\') {
        *p = '\\';
        p++;
      }
      fp = xfopen(sf->filename,"w",pszSrcFile,__LINE__);
      if (fp) {
        fprintf(fp,"\"%s\"\n",sf->dirname);
        SnapShot(sf->dirname,fp,sf->recurse);
        fclose(fp);
      }
    }
    free(sf);
  }
}

//=== CompareFilesThread() Compare files and update container select flags ===

static VOID CompareFilesThread (VOID *args)
{
  FCOMPARE fc;
  HAB      hab2;
  HMQ      hmq2;
  FILE    *fp1,*fp2;
  ULONG    len1,len2,offset = 0L;
  LONG     numread1,numread2;
  CHAR     s[1024],ss[1024],*p1,*p2;

  if(args) {
    fc = *(FCOMPARE *)args;
    hab2 = WinInitialize(0);
    if(hab2) {
      hmq2 = WinCreateMsgQueue(hab2,0);
      if(hmq2) {
        WinCancelShutdown(hmq2,TRUE);
        if(!IsFile(fc.file1) || IsRoot(fc.file1)) {
          p1 = strrchr(fc.file2,'\\');
          if(p1) {
            if(fc.file1[strlen(fc.file1) - 1] == '\\')
              p1++;
            strcat(fc.file1,p1);
          }
        }
        else if(!IsFile(fc.file2) || IsRoot(fc.file2)) {
          p1 = strrchr(fc.file1,'\\');
          if(p1) {
            if(fc.file2[strlen(fc.file2) - 1] == '\\')
              p1++;
            strcat(fc.file2,p1);
          }
        }
        sprintf(s,GetPString(IDS_COMPCOMPARETEXT),fc.file1);
        AddToListboxBottom(fc.hwndList,s);
        sprintf(s,GetPString(IDS_COMPTOTEXT),fc.file2);
        AddToListboxBottom(fc.hwndList,s);
        fp1 = _fsopen(fc.file1,"rb",SH_DENYNO);
        if (!fp1) {
          sprintf(s,GetPString(IDS_COMPCANTOPENTEXT),fc.file1);
          AddToListboxBottom(fc.hwndList,s);
          WinSetWindowText(fc.hwndHelp,GetPString(IDS_ERRORTEXT));
	}
	else {
          fp2 = _fsopen(fc.file2,"rb",SH_DENYNO);
          if (!fp2) {
            sprintf(s,GetPString(IDS_COMPCANTOPENTEXT),fc.file2);
            AddToListboxBottom(fc.hwndList,s);
            WinSetWindowText(fc.hwndHelp,GetPString(IDS_ERRORTEXT));
	  }
	  else {
            len1 = filelength(fileno(fp1));
            len2 = filelength(fileno(fp2));
            if(len1 != len2) {
              strcpy(s,GetPString(IDS_COMPDIFSIZESTEXT));
              AddToListboxBottom(fc.hwndList,s);
              sprintf(s,GetPString(IDS_COMPVSBYTESTEXT),len1,len2);
              AddToListboxBottom(fc.hwndList,s);
              WinSetWindowText(fc.hwndHelp,GetPString(IDS_COMPDONTMATCHTEXT));
            }
            else {
              WinSetWindowText(fc.hwndHelp,GetPString(IDS_COMPCOMPARINGTEXT));
              while(WinIsWindow(hab2,fc.hwndList)) {
                numread1 = fread(s,1,1024,fp1);
                numread2 = fread(ss,1,1024,fp2);
                if(numread1 != numread2 || feof(fp1) != feof(fp2)) {
                  sprintf(s,GetPString(IDS_COMPREADERRORTEXT),
                          offset,offset);
                  AddToListboxBottom(fc.hwndList,s);
                  WinSetWindowText(fc.hwndHelp,GetPString(IDS_ERRORTEXT));
                  break;
                }
                else if(!numread1 && feof(fp1) && feof(fp2)) {
                  AddToListboxBottom(fc.hwndList,
                                     GetPString(IDS_COMPFILESMATCHTEXT));
                  if(!stricmp(fc.file1,fc.file2))
                    AddToListboxBottom(fc.hwndList,
                                       GetPString(IDS_COMPWONDERWHYTEXT));
                  WinSetWindowText(fc.hwndHelp,
                                   GetPString(IDS_COMPCOMPLETETEXT));
                  break;
                }
                else if(numread1 <= 0 || numread2 <= 0) {
                  if(offset == len1)
                    break;
                  else {
                    sprintf(s,GetPString(IDS_COMPMATCHREADERRORTEXT),
                            offset,offset);
                    WinSetWindowText(fc.hwndHelp,
                                     GetPString(IDS_COMPODDERRORTEXT));
                    AddToListboxBottom(fc.hwndList,s);
                    break;
                  }
                }
                else if(memcmp(s,ss,numread1)) {
                  p1 = s;
                  p2 = ss;
                  while(p1 < s + numread1) {
                    if(*p1 != *p2) {
                      sprintf(s,GetPString(IDS_COMPMISMATCHERRORTEXT),
                              offset + (p1 - s),offset + (p1 - s));
                      AddToListboxBottom(fc.hwndList,s);
                      WinSetWindowText(fc.hwndHelp,
                                       GetPString(IDS_COMPDONTMATCHTEXT));
                      break;
                    }
                    p1++;
                    p2++;
                  }
                  break;
                }
                offset += numread1;
              }
            }
            fclose(fp2);
          }
          fclose(fp1);
        }
        WinDestroyMsgQueue(hmq2);
      }
      WinTerminate(hab2);
    }
  }
}

//=== CFileDlgProc() Select directories to compare dialog procedure ===

MRESULT EXPENTRY CFileDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  FCOMPARE *fc;

  switch(msg) {
    case WM_INITDLG:
      if(!mp2)
        WinDismissDlg(hwnd,0);
      else {
        WinSetWindowPtr(hwnd,0,mp2);
        fc = (FCOMPARE *)mp2;
        fc->hwndReport = hwnd;
        fc->hwndList = WinWindowFromID(hwnd,FCMP_LISTBOX);
        fc->hwndHelp = WinWindowFromID(hwnd,FCMP_HELP);
        if(!*fc->file1 || !fc->file2) {
          WinDismissDlg(hwnd,0);
          break;
        }
        MakeFullName(fc->file1);
        MakeFullName(fc->file2);
        if(!stricmp(fc->file1,fc->file2)) {
          saymsg(MB_CANCEL,hwnd,
                 GetPString(IDS_COMPSILLYALERTTEXT),
                 GetPString(IDS_COMPTOITSELFTEXT));
          WinDismissDlg(hwnd,0);
          break;
        }
        if (_beginthread(CompareFilesThread,NULL,65536,(PVOID)fc) == -1) {
          Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
          WinDismissDlg(hwnd,0);
        }
      }
      break;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,
              UM_SETDIR,
              MPVOID,
              MPVOID);
      break;

    case UM_SETDIR:
      PaintRecessedWindow(WinWindowFromID(hwnd,FCMP_HELP),
                          (HPS)0,
                          FALSE,
                          TRUE);
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          WinDismissDlg(hwnd,0);
          break;
        case DID_CANCEL:
          WinDismissDlg(hwnd,1);
          break;
      }
      return 0;

    case WM_DESTROY:
      DosSleep(100L);
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

//=== ActionCnrThread() Do requested action on container contents ===

static VOID ActionCnrThread (VOID *args)
{
  COMPARE *cmp = (COMPARE *)args;
  HAB      hab;
  HMQ      hmq;
  HWND     hwndCnrS,hwndCnrD;
  PCNRITEM pci,pciO,pcin,pciOn;
  CHAR     newname[CCHMAXPATH],dirname[CCHMAXPATH],*p;
  APIRET   rc;

  if(!cmp)
    return;

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize(0);
  if(hab) {
    hmq = WinCreateMsgQueue(hab,0);
    if(hmq) {
      WinCancelShutdown(hmq,TRUE);
      priority_normal();
      switch(cmp->action) {
        case COMP_DELETELEFT:
          hwndCnrS = WinWindowFromID(cmp->hwnd,COMP_LEFTDIR);
          hwndCnrD = WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR);
          cmp->action = IDM_DELETE;
          break;
        case COMP_DELETERIGHT:
          hwndCnrS = WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR);
          hwndCnrD = WinWindowFromID(cmp->hwnd,COMP_LEFTDIR);
          cmp->action = IDM_DELETE;
          break;
        case COMP_MOVELEFT:
          cmp->action = IDM_MOVE;
          hwndCnrS = WinWindowFromID(cmp->hwnd,COMP_LEFTDIR);
          hwndCnrD = WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR);
          break;
        case COMP_MOVERIGHT:
          cmp->action = IDM_MOVE;
          hwndCnrS = WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR);
          hwndCnrD = WinWindowFromID(cmp->hwnd,COMP_LEFTDIR);
          break;
        case COMP_COPYLEFT:
          cmp->action = IDM_COPY;
          hwndCnrS = WinWindowFromID(cmp->hwnd,COMP_LEFTDIR);
          hwndCnrD = WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR);
          break;
        case COMP_COPYRIGHT:
          cmp->action = IDM_COPY;
          hwndCnrS = WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR);
          hwndCnrD = WinWindowFromID(cmp->hwnd,COMP_LEFTDIR);
          break;
        default:
	  Runtime_Error(pszSrcFile, __LINE__, "bad case %u", cmp->action);
          goto Abort;
      }

      pci = WinSendMsg(hwndCnrS,CM_QUERYRECORD,MPVOID,
                       MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
      pciO = WinSendMsg(hwndCnrD,CM_QUERYRECORD,MPVOID,
                        MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
      while(pci && (INT)pci != -1 && pciO && (INT)pciO != -1) {

        pcin = WinSendMsg(hwndCnrS,CM_QUERYRECORD,MPFROMP(pci),
                          MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
        pciOn = WinSendMsg(hwndCnrD,CM_QUERYRECORD,MPFROMP(pciO),
                           MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
        if(*pci->szFileName && (pci->rc.flRecordAttr & CRA_SELECTED)) {
          switch(cmp->action) {
            case IDM_DELETE:
              if(!unlinkf("%s",pci->szFileName)) {
                WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                           MPFROM2SHORT(FALSE,CRA_SELECTED));
                if(!*pciO->szFileName) {
                  WinSendMsg(hwndCnrS,CM_REMOVERECORD,MPFROMP(&pci),
                             MPFROM2SHORT(1,CMA_FREE | CMA_INVALIDATE));
                  if(pciO->rc.flRecordAttr & CRA_SELECTED)
                    WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciO),
                               MPFROM2SHORT(FALSE,CRA_SELECTED));
                  WinSendMsg(hwndCnrD,CM_REMOVERECORD,MPFROMP(&pciO),
                             MPFROM2SHORT(1,CMA_FREE | CMA_INVALIDATE));
                }
                else {
                  *pci->szFileName = 0;
                  pci->pszFileName = pci->szFileName;
                  pci->flags = 0;
                  WinSendMsg(hwndCnrS,CM_INVALIDATERECORD,MPFROMP(&pci),
                             MPFROM2SHORT(1,CMA_ERASE | CMA_TEXTCHANGED));
                }
                if(hwndCnrS == WinWindowFromID(cmp->hwnd,COMP_LEFTDIR))
                  cmp->cmp->totalleft--;
                else
                  cmp->cmp->totalright--;
                DosSleep(0L);
              }
              break;

            case IDM_MOVE:
              if(hwndCnrS == WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR))
                sprintf(newname,"%s%s%s",cmp->leftdir,
                        cmp->leftdir[strlen(cmp->leftdir) - 1] == '\\' ? NullStr : "\\",
                        pci->pszFileName);
              else
                sprintf(newname,"%s%s%s",cmp->rightdir,
                        cmp->rightdir[strlen(cmp->rightdir) - 1] == '\\' ? NullStr : "\\",
                        pci->pszFileName);
              /* make directory if required */
              strcpy(dirname,newname);
              p = strrchr(dirname,'\\');
              if(p) {
                if(p > dirname + 2)
                  p++;
                *p = 0;
                if(IsFile(dirname) == -1)
                  MassMkdir(hwndMain,dirname);
              }
              rc = docopyf(MOVE,pci->szFileName,"%s",newname);
              if(!rc && stricmp(pci->szFileName,newname)) {
                WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                           MPFROM2SHORT(FALSE,CRA_SELECTED));
                if(pciO->rc.flRecordAttr & CRA_SELECTED)
                  WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciO),
                             MPFROM2SHORT(FALSE,CRA_SELECTED));
                strcpy(pciO->szFileName,newname);
                if(hwndCnrS == WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR)) {
                  pciO->pszFileName = pciO->szFileName + strlen(cmp->leftdir);
                  if(cmp->leftdir[strlen(cmp->leftdir) - 1] != '\\')
                    pciO->pszFileName++;
                }
                else {
                  pciO->pszFileName = pciO->szFileName + strlen(cmp->rightdir);
                  if(cmp->rightdir[strlen(cmp->rightdir) - 1] != '\\')
                    pciO->pszFileName++;
                }
                strcpy(pciO->szDispAttr,pci->szDispAttr);
                pciO->attrFile   = pci->attrFile;
                pciO->flags      = 0;
                pciO->date       = pci->date;
                pciO->time       = pci->time;
                pciO->ladate     = pci->ladate;
                pciO->latime     = pci->latime;
                pciO->crdate     = pci->crdate;
                pciO->crtime     = pci->crtime;
                pciO->cbFile     = pci->cbFile;
                pciO->easize     = pci->easize;
                *pciO->szSubject = 0;
                *pci->szFileName = 0;
                pci->pszFileName = pci->szFileName;
                pci->flags = 0;
                WinSendMsg(hwndCnrS,CM_INVALIDATERECORD,MPFROMP(&pci),
                           MPFROM2SHORT(1,CMA_ERASE | CMA_TEXTCHANGED));
                WinSendMsg(hwndCnrD,CM_INVALIDATERECORD,MPFROMP(&pciO),
                           MPFROM2SHORT(1,CMA_ERASE | CMA_TEXTCHANGED));
              }
              else if (rc) {
                rc = Dos_Error(MB_ENTERCANCEL,
                               rc,
                               HWND_DESKTOP,
                               pszSrcFile,
                               __LINE__,
                               GetPString(IDS_COMPMOVEFAILEDTEXT),
                               pci->szFileName,
                               newname);
                if(rc == MBID_CANCEL) /* cause loop to break */
                  pcin = NULL;
              }
              break;

            case IDM_COPY:
              if(hwndCnrS == WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR))
                sprintf(newname,"%s%s%s",cmp->leftdir,
                        cmp->leftdir[strlen(cmp->leftdir) - 1] == '\\' ? NullStr : "\\",
                        pci->pszFileName);
              else
                sprintf(newname,"%s%s%s",cmp->rightdir,
                        cmp->rightdir[strlen(cmp->rightdir) - 1] == '\\' ? NullStr : "\\",
                        pci->pszFileName);
              /* make directory if required */
              strcpy(dirname,newname);
              p = strrchr(dirname,'\\');
              if(p) {
                if(p > dirname + 2)
                  p++;
                *p = 0;
                if(IsFile(dirname) == -1)
                  MassMkdir(hwndMain,dirname);
              }
              rc = docopyf(COPY,pci->szFileName,"%s",newname);
              if (rc) {
                rc = Dos_Error(MB_ENTERCANCEL,
                               rc,
                               HWND_DESKTOP,
                               pszSrcFile,
                               __LINE__,
                               GetPString(IDS_COMPCOPYFAILEDTEXT),
                               pci->szFileName,
                               newname);
                if(rc == MBID_CANCEL)
                  pcin = NULL;		 /* cause loop to break */
	      }
	      else {
                WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                           MPFROM2SHORT(FALSE,CRA_SELECTED));
                if(pciO->rc.flRecordAttr & CRA_SELECTED)
                  WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciO),
                             MPFROM2SHORT(FALSE,CRA_SELECTED));
                strcpy(pciO->szFileName,newname);
                if(hwndCnrS == WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR)) {
                  pciO->pszFileName = pciO->szFileName + strlen(cmp->leftdir);
                  if(cmp->leftdir[strlen(cmp->leftdir) - 1] != '\\')
                    pciO->pszFileName++;
                }
                else {
                  pciO->pszFileName = pciO->szFileName + strlen(cmp->rightdir);
                  if(cmp->rightdir[strlen(cmp->rightdir) - 1] != '\\')
                    pciO->pszFileName++;
                }
                strcpy(pciO->szDispAttr,pci->szDispAttr);
                pciO->attrFile   = pci->attrFile;
                pciO->flags      = CNRITEM_EXISTS;
                pciO->date       = pci->date;
                pciO->time       = pci->time;
                pciO->ladate     = pci->ladate;
                pciO->latime     = pci->latime;
                pciO->crdate     = pci->crdate;
                pciO->crtime     = pci->crtime;
                pciO->cbFile     = pci->cbFile;
                pciO->easize     = pci->easize;
                *pci->szSubject  = 0;
                pci->flags       = CNRITEM_EXISTS;
                WinSendMsg(hwndCnrS,CM_INVALIDATERECORD,MPFROMP(&pci),
                           MPFROM2SHORT(1,CMA_ERASE | CMA_TEXTCHANGED));
                WinSendMsg(hwndCnrD,CM_INVALIDATERECORD,MPFROMP(&pciO),
                           MPFROM2SHORT(1,CMA_ERASE | CMA_TEXTCHANGED));
              }
              break;

            default:
              break;
          } // switch
        }
        pci = pcin;
        pciO = pciOn;
      } // while
Abort:
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  PostMsg(cmp->hwnd,UM_CONTAINER_FILLED,MPFROMLONG(1L),MPVOID);
  PostMsg(cmp->hwnd,WM_COMMAND,MPFROM2SHORT(IDM_DESELECTALL,0),MPVOID);
  free(cmp);
}

//=== SelectCnrsThread() Update container selection flags thread ===

static VOID SelectCnrsThread (VOID *args)
{
  COMPARE *cmp = (COMPARE *)args;
  HAB      hab;
  HMQ      hmq;

  if(!cmp)
    return;

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize(0);
  if(hab) {
    hmq = WinCreateMsgQueue(hab,0);
    if(hmq) {
      WinCancelShutdown(hmq,TRUE);
      priority_normal();
      switch(cmp->action) {
        case IDM_INVERT:
          InvertAll(WinWindowFromID(cmp->hwnd,COMP_LEFTDIR));
          InvertAll(WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR));
          break;

        case IDM_DESELECTALL:
          Deselect(WinWindowFromID(cmp->hwnd,COMP_LEFTDIR));
          Deselect(WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR));
          break;

        default:
          SpecialSelect(WinWindowFromID(cmp->hwnd,COMP_LEFTDIR),
                        WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR),
                        cmp->action,cmp->reset);
          break;
      }
      if(!PostMsg(cmp->hwnd,UM_CONTAINER_FILLED,MPFROMLONG(1L),MPVOID))
        WinSendMsg(cmp->hwnd,UM_CONTAINER_FILLED,MPFROMLONG(1L),MPVOID);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  free(cmp);
}

static VOID FillDirList (CHAR *str,INT skiplen,BOOL recurse,
                         FILELIST ***list,INT *numfiles,INT *numalloc) {

  register BYTE *fb;
  register CHAR *enddir;
  register ULONG x;
  CHAR          *maskstr;
  FILEFINDBUF4  *ffb4,*pffb;
  HDIR           hDir;
  ULONG          nm,fl = 0,ulM = 64;
  APIRET         rc;

  if(!str || !*str)
    return;
  if(!recurse)
    ulM = 128;
  maskstr = xmalloc(CCHMAXPATH,pszSrcFile,__LINE__);
  if(!maskstr)
    return;
  ffb4 = xmalloc(sizeof(FILEFINDBUF4) * ulM,pszSrcFile,__LINE__);
  if(!ffb4) {
    free(maskstr);
    return;
  }
  x = strlen(str);
  memcpy(maskstr,str,x + 1);
  enddir = maskstr + x;
  if(*(enddir - 1) != '\\') {
    *enddir = '\\';
    enddir++;
    *enddir = 0;
  }
  *enddir = '*';
  *(enddir + 1) = 0;
  hDir = HDIR_CREATE;
  nm = ulM;
  if(recurse)
    fl = FILE_DIRECTORY;
  DosError(FERR_DISABLEHARDERR);
  rc = DosFindFirst(maskstr, &hDir,
                    (FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
                     FILE_SYSTEM | FILE_HIDDEN) | fl,
                    ffb4, sizeof(FILEFINDBUF4) * nm,
                    &nm, FIL_QUERYEASIZE);
  if(!rc) {
    while(!rc) {
      fb = (BYTE *)ffb4;
      x = 0;
      while(x < nm) {
        pffb = (FILEFINDBUF4 *)fb;
        if(pffb->attrFile & FILE_DIRECTORY) {
          if(recurse && (*pffb->achName != '.' && (pffb->achName[1] &&
             pffb->achName[1] != '.'))) {
            if(fForceUpper)
              strupr(pffb->achName);
            else if(fForceLower)
              strlwr(pffb->achName);
            memcpy(enddir,pffb->achName,pffb->cchName + 1);
            FillDirList(maskstr,skiplen,recurse,list,numfiles,numalloc);
          }
        }
        else {
          if(fForceUpper)
            strupr(pffb->achName);
          else if(fForceLower)
            strlwr(pffb->achName);
          memcpy(enddir,pffb->achName,pffb->cchName + 1);
          if(AddToFileList(maskstr + skiplen,pffb,list,numfiles,numalloc))
            goto Abort;
        }
        fb += pffb->oNextEntryOffset;
        x++;
      }
      nm = ulM;
      DosError(FERR_DISABLEHARDERR);
      rc = DosFindNext(hDir,ffb4,sizeof(FILEFINDBUF4) * nm,&nm);
    }
Abort:
    DosFindClose(hDir);
    DosSleep(0L);
  }
  free(maskstr);
  free(ffb4);
}

//=== CompNames() Compare names for qsort ===

static int CompNames (const void *n1,const void *n2)
{
  FILELIST *fl1 = *(FILELIST **)n1;
  FILELIST *fl2 = *(FILELIST **)n2;

  return stricmp(fl1->fname,fl2->fname);
}

//=== FillCnrsThread() Fill left and right containers ===

static VOID FillCnrsThread (VOID *args)
{
  COMPARE    *cmp = (COMPARE *)args;
  HAB         hab;
  HMQ         hmq;
  BOOL        notified = FALSE;
  static CHAR attrstring[] = "RHS\0DA";
  HWND        hwndLeft,hwndRight;

  if(!cmp)
    _endthread();

  DosError(FERR_DISABLEHARDERR);

  hab = WinInitialize(0);
  if(!hab)
    Win_Error(NULLHANDLE,NULLHANDLE,pszSrcFile,__LINE__,"WinInitialize");
  else {
    hmq = WinCreateMsgQueue(hab,0);
    if(!hmq)
      Win_Error(NULLHANDLE,NULLHANDLE,pszSrcFile,__LINE__,"WinCreateMsgQueue");
    else {
      INT             x;
      INT             l;
      INT             r;
      INT             y;
      ULONG           cntr;
      FILELIST        **filesl = NULL;
      FILELIST        **filesr = NULL;
      INT             numfilesl = 0;
      INT             numfilesr = 0;
      INT             numallocl = 0;
      INT             numallocr = 0;
      INT             lenl;			// Directory prefix length
      INT             lenr;
      UINT            recsNeeded;		// fixme to check ovf
      PCNRITEM        pcilFirst;
      PCNRITEM        pcirFirst;
      PCNRITEM        pcil;
      PCNRITEM        pcir;
      PCNRITEM        pcit;
      RECORDINSERT    ri;
      CHAR            *pch;

      WinCancelShutdown(hmq,TRUE);
      hwndLeft = WinWindowFromID(cmp->hwnd,COMP_LEFTDIR);
      hwndRight = WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR);
      lenl = strlen(cmp->leftdir);
      if(cmp->leftdir[strlen(cmp->leftdir) - 1] != '\\')
        lenl++;
      lenr = strlen(cmp->rightdir);
      if(cmp->rightdir[strlen(cmp->rightdir) - 1] != '\\')
        lenr++;
      priority_normal();
      /* clear containers */
      WinSendMsg(hwndRight,CM_REMOVERECORD,
                 MPVOID,MPFROM2SHORT(0,CMA_FREE | CMA_INVALIDATE));
      WinSendMsg(hwndLeft,CM_REMOVERECORD,
                 MPVOID,MPFROM2SHORT(0,CMA_FREE | CMA_INVALIDATE));
      cmp->cmp->totalleft = cmp->cmp->totalright = 0;

      /* build list of all files in left directory */
      if(fForceLower)
        strlwr(cmp->leftdir);
      else if(fForceUpper)
        strupr(cmp->leftdir);
      FillDirList(cmp->leftdir,lenl,cmp->includesubdirs,
                  &filesl,&numfilesl,&numallocl);

      if(filesl)
        qsort(filesl,numfilesl,sizeof(CHAR *),CompNames);
      /* build list of all files in right directory */
      if(!*cmp->rightlist) {
        if(fForceLower)
          strlwr(cmp->rightdir);
        else if(fForceUpper)
          strupr(cmp->rightdir);
        FillDirList(cmp->rightdir,lenr,cmp->includesubdirs,
                    &filesr,&numfilesr,&numallocr);
      }
      else
      {
        /* use snapshot file */
        FILE        *fp;
        FILEFINDBUF4 fb4;
        CHAR         str[CCHMAXPATH * 2],*p;

        memset(&fb4,0,sizeof(fb4));
        fp = fopen(cmp->rightlist,"r");
        if(!fp)
	  Runtime_Error(pszSrcFile, __LINE__, "can not open %s (%d)", cmp->rightlist, errno);
	else {
          while(!feof(fp)) {
	    /* first get name of directory */
            if(!fgets(str,sizeof(str) - 1,fp))
              break;		// EOF
            str[sizeof(str) - 1] = 0;
            bstrip(str);
            p = str;
            if(*p == '\"') {
	      /* Quoted */
              p++;
              if(*p && *p != '\"') {
                p = strchr(p,'\"');
                if(p) {
                  *p = 0;
                  if(*(str + 1)) {
                    strcpy(cmp->rightdir,str + 1);
                    if(fForceUpper)
                      strupr(cmp->rightdir);
                      else if(fForceLower)
                      strlwr(cmp->rightdir);
                    p = cmp->rightdir + (strlen(cmp->rightdir) - 1);
                    if(p - cmp->rightdir > 3 && *p == '\\')
                      *p = 0;		// Chop trailing slash
                    break;
                  }
                }
              }
            }
          } // while !EOF
          {
            CNRINFO cnri;

            memset(&cnri,0,sizeof(cnri));
            cnri.cb = sizeof(cnri);
            cnri.pszCnrTitle = cmp->rightdir;
            WinSendMsg(hwndRight,CM_SETCNRINFO,
                       MPFROMP(&cnri),
                       MPFROMLONG(CMA_CNRTITLE));
          }
          if(*cmp->rightdir) {
            lenr = strlen(cmp->rightdir) +
                     (cmp->rightdir[strlen(cmp->rightdir) - 1] != '\\');
            while(!feof(fp)) {
              if(!fgets(str,sizeof(str) - 1,fp))
                break;
              str[sizeof(str) - 1] = 0;
              bstrip(str);
              p = str;
              if(*p == '\"') {
                p++;
                if(*p && *p != '\"') {
                  p = strchr(p,'\"');
                  if(p) {
                    *p = 0;
                    p++;
                    if(*p == ',') {
                      p++;
                      if(!cmp->includesubdirs && atol(p) > lenr)
                        continue;
                      p = strchr(p,',');
                      if(p) {
                        p++;
                        fb4.cbFile = atol(p);
                        p = strchr(p,',');
                        if(p) {
                          p++;
                          fb4.fdateLastWrite.year = atol(p) - 1980;
                          p = strchr(p,'/');
                          if(p) {
                            p++;
                            fb4.fdateLastWrite.month = atol(p);
                            p = strchr(p,'/');
                            if(p) {
                              p++;
                              fb4.fdateLastWrite.day = atol(p);
                              p = strchr(p,',');
                              if(p) {
                                p++;
                                fb4.ftimeLastWrite.hours = atol(p);
                                p = strchr(p,':');
                                if(p) {
                                  p++;
                                  fb4.ftimeLastWrite.minutes = atol(p);
                                  p = strchr(p,':');
                                  if(p) {
                                    p++;
                                    fb4.ftimeLastWrite.twosecs = atol(p);
                                    p = strchr(p,',');
                                    if(p) {
                                      p++;
                                      fb4.attrFile = atol(p);
                                      p = strchr(p,',');
                                      if(p) {
                                        p++;
                                        fb4.cbList = atol(p) * 2;
                                        if(fForceUpper)
                                          strupr(str + 1);
                                        else if(fForceLower)
                                          strlwr(str + 1);
                                        if(AddToFileList((str + 1) + lenr,
                                                         &fb4,
                                                         &filesr,
                                                         &numfilesr,
                                                         &numallocr))
                                          break;
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            } // while
          } // if have rightdir
          fclose(fp);
        }
      } // if snapshot file

      if(filesr)
        qsort(filesr,numfilesr,sizeof(CHAR *),CompNames);

      /* we now have two lists of files, both sorted. */
      /* first, count total number of container entries required */
      l = r = 0;
      recsNeeded = 0;
      while((filesl && filesl[l]) || (filesr && filesr[r])) {
        if((filesl && filesl[l]) && (filesr && filesr[r])) {
          x = stricmp(filesl[l]->fname,filesr[r]->fname);
          if(!x) {
            l++;			// In both
            r++;
          }
          else if(x < 0)
            l++;			// In left only
          else
            r++;			// In right only
        }
        else if(filesl && filesl[l])
          l++;				// In left only
        else    /* filesr && filesr[r] */
          r++;				// In right only
        recsNeeded++; /* keep count of how many entries req'd */
      }
      WinSendMsg(cmp->hwnd,UM_CONTAINERHWND,MPVOID,MPVOID);
      /* now insert records into the containers */
      cntr = 0;
      l = r = 0;
      if(recsNeeded) {
        pcilFirst = WinSendMsg(hwndLeft,
                               CM_ALLOCRECORD,
                               MPFROMLONG(EXTRA_RECORD_BYTES2),
                               MPFROMLONG(recsNeeded));
        if (!pcilFirst) {
	  Runtime_Error(pszSrcFile, __LINE__, "CM_ALLOCRECORD %u failed", recsNeeded);
          recsNeeded = 0;
        }
      }
      if (recsNeeded) {
        pcirFirst = WinSendMsg(hwndRight,CM_ALLOCRECORD,
                               MPFROMLONG(EXTRA_RECORD_BYTES2),
                               MPFROMLONG(recsNeeded));
        if (!pcirFirst) {
	  Runtime_Error(pszSrcFile, __LINE__, "CM_ALLOCRECORD %u failed", recsNeeded);
          recsNeeded = 0;
          pcil = pcilFirst;
          while(pcil) {
            pcit = (PCNRITEM)pcil->rc.preccNextRecord;
            WinSendMsg(hwndLeft,CM_FREERECORD,
                       MPFROMP(&pcil),MPFROMSHORT(1));
            pcil = pcit;
          }
        }
      }
      if (recsNeeded) {
        pcil = pcilFirst;
        pcir = pcirFirst;
        while((filesl && filesl[l]) || (filesr && filesr[r])) {
          pcir->hwndCnr = hwndRight;
          pcir->pszFileName = pcir->szFileName;
          pcir->rc.pszIcon = pcir->pszFileName;
          pcir->rc.hptrIcon = (HPOINTER)0;
          pcir->pszSubject = pcir->szSubject;
          pcir->pszLongname = pcir->szLongname;
          pcir->pszDispAttr = pcir->szDispAttr;
          pcil->hwndCnr = hwndLeft;
          pcil->pszFileName = pcil->szFileName;
          pcil->rc.pszIcon = pcil->pszFileName;
          pcil->rc.hptrIcon = (HPOINTER)0;
          pcil->pszDispAttr = pcil->szDispAttr;
          pcil->pszSubject = pcil->szSubject;
          pcil->pszLongname = pcil->szLongname;
          if((filesl && filesl[l]) && (filesr && filesr[r])) {
            x = stricmp(filesl[l]->fname,filesr[r]->fname);
            if(!x) {
	      // Same
              sprintf(pcil->szFileName,"%s%s%s",cmp->leftdir,
                      (cmp->leftdir[strlen(cmp->leftdir) - 1] == '\\') ?
                      NullStr : "\\",filesl[l]->fname);
              // pcil->rc.hptrIcon    = hptrFile;
              pcil->pszFileName    = pcil->szFileName + lenl;
              pcil->attrFile       = filesl[l]->attrFile;
              y = 0;
              for(x = 0;x < 6;x++) {
                if(attrstring[x])
                  pcil->szDispAttr[y++] = (CHAR)((pcil->attrFile & (1 << x)) ?
                                                 attrstring[x] : '-');
	      }
              pcil->szDispAttr[5]  = 0;
              pcil->cbFile         = filesl[l]->cbFile;
              pcil->easize         = filesl[l]->easize;
              pcil->date.day       = filesl[l]->date.day;
              pcil->date.month     = filesl[l]->date.month;
              pcil->date.year      = filesl[l]->date.year + 1980;
              pcil->time.seconds   = filesl[l]->time.twosecs * 2;
              pcil->time.minutes   = filesl[l]->time.minutes;
              pcil->time.hours     = filesl[l]->time.hours;
              pcil->ladate.day     = filesl[l]->ladate.day;
              pcil->ladate.month   = filesl[l]->ladate.month;
              pcil->ladate.year    = filesl[l]->ladate.year + 1980;
              pcil->latime.seconds = filesl[l]->latime.twosecs * 2;
              pcil->latime.minutes = filesl[l]->latime.minutes;
              pcil->latime.hours   = filesl[l]->latime.hours;
              pcil->crdate.day     = filesl[l]->crdate.day;
              pcil->crdate.month   = filesl[l]->crdate.month;
              pcil->crdate.year    = filesl[l]->crdate.year + 1980;
              pcil->crtime.seconds = filesl[l]->crtime.twosecs * 2;
              pcil->crtime.minutes = filesl[l]->crtime.minutes;
              pcil->crtime.hours   = filesl[l]->crtime.hours;
              if (*cmp->dcd.mask.szMask) {
                if(!Filter((PMINIRECORDCORE)pcil,(PVOID)&cmp->dcd.mask)) {
                  pcil->rc.flRecordAttr |= CRA_FILTERED;
                  pcir->rc.flRecordAttr |= CRA_FILTERED;
                }
              }
              sprintf(pcir->szFileName,"%s%s%s",cmp->rightdir,
                      (cmp->rightdir[strlen(cmp->rightdir) - 1] == '\\') ?
                      NullStr : "\\",filesr[r]->fname);
              pcir->pszFileName    = pcir->szFileName + lenr;
              pcir->attrFile       = filesr[r]->attrFile;
              // pcir->rc.hptrIcon    = hptrFile;
              y = 0;
              for(x = 0;x < 6;x++)
                if(attrstring[x])
                  pcir->szDispAttr[y++] = (CHAR)((pcir->attrFile & (1 << x)) ?
                                                 attrstring[x] : '-');
              pcir->szDispAttr[5]  = 0;
              pcir->cbFile         = filesr[r]->cbFile;
              pcir->easize         = filesr[r]->easize;
              pcir->date.day       = filesr[r]->date.day;
              pcir->date.month     = filesr[r]->date.month;
              pcir->date.year      = filesr[r]->date.year + 1980;
              pcir->time.seconds   = filesr[r]->time.twosecs * 2;
              pcir->time.minutes   = filesr[r]->time.minutes;
              pcir->time.hours     = filesr[r]->time.hours;
              pcir->ladate.day     = filesr[r]->ladate.day;
              pcir->ladate.month   = filesr[r]->ladate.month;
              pcir->ladate.year    = filesr[r]->ladate.year + 1980;
              pcir->latime.seconds = filesr[r]->latime.twosecs * 2;
              pcir->latime.minutes = filesr[r]->latime.minutes;
              pcir->latime.hours   = filesr[r]->latime.hours;
              pcir->crdate.day     = filesr[r]->crdate.day;
              pcir->crdate.month   = filesr[r]->crdate.month;
              pcir->crdate.year    = filesr[r]->crdate.year + 1980;
              pcir->crtime.seconds = filesr[r]->crtime.twosecs * 2;
              pcir->crtime.minutes = filesr[r]->crtime.minutes;
              pcir->crtime.hours   = filesr[r]->crtime.hours;
              pcil->flags |= CNRITEM_EXISTS;
              pcir->flags |= CNRITEM_EXISTS;
              pch = pcil->szSubject;
              if(pcil->cbFile + pcil->easize >
                 pcir->cbFile + pcir->easize) {
                pcil->flags |= CNRITEM_LARGER;
                pcir->flags |= CNRITEM_SMALLER;
                strcpy(pch,GetPString(IDS_LARGERTEXT));
                pch += 6;
              }
              else if(pcil->cbFile + pcil->easize <
                      pcir->cbFile + pcir->easize) {
                pcil->flags |= CNRITEM_SMALLER;
                pcir->flags |= CNRITEM_LARGER;
                strcpy(pch,GetPString(IDS_SMALLERTEXT));
                pch += 7;
              }
              if((pcil->date.year > pcir->date.year) ? TRUE :
                 (pcil->date.year < pcir->date.year) ? FALSE :
                 (pcil->date.month > pcir->date.month) ? TRUE :
                 (pcil->date.month < pcir->date.month) ? FALSE :
                 (pcil->date.day > pcir->date.day) ? TRUE :
                 (pcil->date.day < pcir->date.day) ? FALSE :
                 (pcil->time.hours > pcir->time.hours) ? TRUE :
                 (pcil->time.hours < pcir->time.hours) ? FALSE :
                 (pcil->time.minutes > pcir->time.minutes) ? TRUE :
                 (pcil->time.minutes < pcir->time.minutes) ? FALSE :
                 (pcil->time.seconds > pcir->time.seconds) ? TRUE :
                 (pcil->time.seconds < pcir->time.seconds) ? FALSE : FALSE) {
                pcil->flags |= CNRITEM_NEWER;
                pcir->flags |= CNRITEM_OLDER;
                if(pch != pcil->szSubject) {
                  strcpy(pch,", ");
                  pch += 2;
                }
                strcpy(pch,GetPString(IDS_NEWERTEXT));
                pch += 5;
              }
              else if((pcil->date.year < pcir->date.year) ? TRUE :
                      (pcil->date.year > pcir->date.year) ? FALSE :
                      (pcil->date.month < pcir->date.month) ? TRUE :
                      (pcil->date.month > pcir->date.month) ? FALSE :
                      (pcil->date.day < pcir->date.day) ? TRUE :
                      (pcil->date.day > pcir->date.day) ? FALSE :
                      (pcil->time.hours < pcir->time.hours) ? TRUE :
                      (pcil->time.hours > pcir->time.hours) ? FALSE :
                      (pcil->time.minutes < pcir->time.minutes) ? TRUE :
                      (pcil->time.minutes > pcir->time.minutes) ? FALSE :
                      (pcil->time.seconds < pcir->time.seconds) ? TRUE :
                      (pcil->time.seconds > pcir->time.seconds) ? FALSE :
                      FALSE) {
                pcil->flags |= CNRITEM_OLDER;
                pcir->flags |= CNRITEM_NEWER;
                if(pch != pcil->szSubject) {
                  strcpy(pch,", ");
                  pch += 2;
                }
                strcpy(pch,GetPString(IDS_OLDERTEXT));
                pch += 5;
              }
              *pch = 0;
              r++;
              l++;
            }
            else if(x < 0) {
	      // Just on left
              sprintf(pcil->szFileName,"%s%s%s",cmp->leftdir,
                      (cmp->leftdir[strlen(cmp->leftdir) - 1] == '\\') ?
                      NullStr : "\\",filesl[l]->fname);
              pcil->pszFileName = pcil->szFileName + lenl;
              pcil->attrFile       = filesl[l]->attrFile;
	      // pcil->rc.hptrIcon    = hptrFile;
              y = 0;
              for(x = 0;x < 6;x++)
                if(attrstring[x])
                  pcil->szDispAttr[y++] = (CHAR)((pcil->attrFile & (1 << x)) ?
                                                 attrstring[x] : '-');
              pcil->szDispAttr[5]  = 0;
              pcil->cbFile         = filesl[l]->cbFile;
              pcil->easize         = filesl[l]->easize;
              pcil->date.day       = filesl[l]->date.day;
              pcil->date.month     = filesl[l]->date.month;
              pcil->date.year      = filesl[l]->date.year + 1980;
              pcil->time.seconds   = filesl[l]->time.twosecs * 2;
              pcil->time.minutes   = filesl[l]->time.minutes;
              pcil->time.hours     = filesl[l]->time.hours;
              pcil->ladate.day     = filesl[l]->ladate.day;
              pcil->ladate.month   = filesl[l]->ladate.month;
              pcil->ladate.year    = filesl[l]->ladate.year + 1980;
              pcil->latime.seconds = filesl[l]->latime.twosecs * 2;
              pcil->latime.minutes = filesl[l]->latime.minutes;
              pcil->latime.hours   = filesl[l]->latime.hours;
              pcil->crdate.day     = filesl[l]->crdate.day;
              pcil->crdate.month   = filesl[l]->crdate.month;
              pcil->crdate.year    = filesl[l]->crdate.year + 1980;
              pcil->crtime.seconds = filesl[l]->crtime.twosecs * 2;
              pcil->crtime.minutes = filesl[l]->crtime.minutes;
              pcil->crtime.hours   = filesl[l]->crtime.hours;
              if (*cmp->dcd.mask.szMask) {
                if (!Filter((PMINIRECORDCORE)pcil,(PVOID)&cmp->dcd.mask)) {
                  pcil->rc.flRecordAttr |= CRA_FILTERED;
                  pcir->rc.flRecordAttr |= CRA_FILTERED;
                }
              }
              free(filesl[l]);
              l++;
            }
            else {
	      // Just on right
              sprintf(pcir->szFileName,"%s%s%s",cmp->rightdir,
                      (cmp->rightdir[strlen(cmp->rightdir) - 1] == '\\') ?
                      NullStr : "\\",filesr[r]->fname);
              pcir->pszFileName    = pcir->szFileName + lenr;
              pcir->attrFile       = filesr[r]->attrFile;
	      // pcir->rc.hptrIcon    = hptrFile;
              y = 0;
              for (x = 0;x < 6;x++) {
                if (attrstring[x])
                  pcir->szDispAttr[y++] = (CHAR)((pcir->attrFile & (1 << x)) ?
                                                 attrstring[x] : '-');
	      }
              pcir->szDispAttr[5]  = 0;
              pcir->cbFile         = filesr[r]->cbFile;
              pcir->easize         = filesr[r]->easize;
              pcir->date.day       = filesr[r]->date.day;
              pcir->date.month     = filesr[r]->date.month;
              pcir->date.year      = filesr[r]->date.year + 1980;
              pcir->time.seconds   = filesr[r]->time.twosecs * 2;
              pcir->time.minutes   = filesr[r]->time.minutes;
              pcir->time.hours     = filesr[r]->time.hours;
              pcir->ladate.day     = filesr[r]->ladate.day;
              pcir->ladate.month   = filesr[r]->ladate.month;
              pcir->ladate.year    = filesr[r]->ladate.year + 1980;
              pcir->latime.seconds = filesr[r]->latime.twosecs * 2;
              pcir->latime.minutes = filesr[r]->latime.minutes;
              pcir->latime.hours   = filesr[r]->latime.hours;
              pcir->crdate.day     = filesr[r]->crdate.day;
              pcir->crdate.month   = filesr[r]->crdate.month;
              pcir->crdate.year    = filesr[r]->crdate.year + 1980;
              pcir->crtime.seconds = filesr[r]->crtime.twosecs * 2;
              pcir->crtime.minutes = filesr[r]->crtime.minutes;
              pcir->crtime.hours   = filesr[r]->crtime.hours;
              if(*cmp->dcd.mask.szMask) {
                if(!Filter((PMINIRECORDCORE)pcir,(PVOID)&cmp->dcd.mask)) {
                  pcir->rc.flRecordAttr |= CRA_FILTERED;
                  pcil->rc.flRecordAttr |= CRA_FILTERED;
                }
              }
              free(filesr[r]);
              r++;
            }
          }
          else if(filesl && filesl[l]) {
	    // Just on left
            sprintf(pcil->szFileName,"%s%s%s",cmp->leftdir,
                    (cmp->leftdir[strlen(cmp->leftdir) - 1] == '\\') ?
                    NullStr : "\\",filesl[l]->fname);
            pcil->pszFileName = pcil->szFileName + lenl;
            pcil->attrFile       = filesl[l]->attrFile;
	    // pcil->rc.hptrIcon    = hptrFile;
            y = 0;
            for(x = 0;x < 6;x++)
              if(attrstring[x])
                pcil->szDispAttr[y++] = (CHAR)((pcil->attrFile & (1 << x)) ?
                                               attrstring[x] : '-');
            pcil->szDispAttr[5]  = 0;
            pcil->cbFile         = filesl[l]->cbFile;
            pcil->easize         = filesl[l]->easize;
            pcil->date.day       = filesl[l]->date.day;
            pcil->date.month     = filesl[l]->date.month;
            pcil->date.year      = filesl[l]->date.year + 1980;
            pcil->time.seconds   = filesl[l]->time.twosecs * 2;
            pcil->time.minutes   = filesl[l]->time.minutes;
            pcil->time.hours     = filesl[l]->time.hours;
            pcil->ladate.day     = filesl[l]->ladate.day;
            pcil->ladate.month   = filesl[l]->ladate.month;
            pcil->ladate.year    = filesl[l]->ladate.year + 1980;
            pcil->latime.seconds = filesl[l]->latime.twosecs * 2;
            pcil->latime.minutes = filesl[l]->latime.minutes;
            pcil->latime.hours   = filesl[l]->latime.hours;
            pcil->crdate.day     = filesl[l]->crdate.day;
            pcil->crdate.month   = filesl[l]->crdate.month;
            pcil->crdate.year    = filesl[l]->crdate.year + 1980;
            pcil->crtime.seconds = filesl[l]->crtime.twosecs * 2;
            pcil->crtime.minutes = filesl[l]->crtime.minutes;
            pcil->crtime.hours   = filesl[l]->crtime.hours;
            if(*cmp->dcd.mask.szMask) {
              if(!Filter((PMINIRECORDCORE)pcil,(PVOID)&cmp->dcd.mask)) {
                pcil->rc.flRecordAttr |= CRA_FILTERED;
                pcir->rc.flRecordAttr |= CRA_FILTERED;
              }
            }
            free(filesl[l]);
            l++;
          }
          else {
	    /* filesr && filesr[r] */
	    // Just on right
            sprintf(pcir->szFileName,"%s%s%s",cmp->rightdir,
                    (cmp->rightdir[strlen(cmp->rightdir) - 1] == '\\') ?
                    NullStr : "\\",filesr[r]->fname);
            pcir->pszFileName    = pcir->szFileName + lenr;
            pcir->attrFile       = filesr[r]->attrFile;
	    // pcir->rc.hptrIcon    = hptrFile;
            y = 0;
            for (x = 0;x < 6;x++) {
              if(attrstring[x])
                pcir->szDispAttr[y++] = (CHAR)((pcir->attrFile & (1 << x)) ?
                                               attrstring[x] : '-');
	    }
            pcir->szDispAttr[5]  = 0;
            pcir->cbFile         = filesr[r]->cbFile;
            pcir->easize         = filesr[r]->easize;
            pcir->date.day       = filesr[r]->date.day;
            pcir->date.month     = filesr[r]->date.month;
            pcir->date.year      = filesr[r]->date.year + 1980;
            pcir->time.seconds   = filesr[r]->time.twosecs * 2;
            pcir->time.minutes   = filesr[r]->time.minutes;
            pcir->time.hours     = filesr[r]->time.hours;
            pcir->ladate.day     = filesr[r]->ladate.day;
            pcir->ladate.month   = filesr[r]->ladate.month;
            pcir->ladate.year    = filesr[r]->ladate.year + 1980;
            pcir->latime.seconds = filesr[r]->latime.twosecs * 2;
            pcir->latime.minutes = filesr[r]->latime.minutes;
            pcir->latime.hours   = filesr[r]->latime.hours;
            pcir->crdate.day     = filesr[r]->crdate.day;
            pcir->crdate.month   = filesr[r]->crdate.month;
            pcir->crdate.year    = filesr[r]->crdate.year + 1980;
            pcir->crtime.seconds = filesr[r]->crtime.twosecs * 2;
            pcir->crtime.minutes = filesr[r]->crtime.minutes;
            pcir->crtime.hours   = filesr[r]->crtime.hours;
            if (*cmp->dcd.mask.szMask) {
              if (!Filter((PMINIRECORDCORE)pcir,(PVOID)&cmp->dcd.mask)) {
                pcir->rc.flRecordAttr |= CRA_FILTERED;
                pcil->rc.flRecordAttr |= CRA_FILTERED;
              }
            }
            free(filesr[r]);
            r++;
          }
          if(!(cntr % 500))
            DosSleep(1L);
          else if(!(cntr % 50))
            DosSleep(0L);
          cntr++;
          pcil = (PCNRITEM)pcil->rc.preccNextRecord;
          pcir = (PCNRITEM)pcir->rc.preccNextRecord;
        } // while
        if(filesl)
          free(filesl);			// Free header - have already freed elements
        filesl = NULL;
        if(filesr)
          free(filesr);
        filesr = NULL;
        /* insert 'em */
        WinSendMsg(cmp->hwnd,UM_CONTAINERDIR,MPVOID,MPVOID);
        memset(&ri, 0, sizeof(RECORDINSERT));
        ri.cb                 = sizeof(RECORDINSERT);
        ri.pRecordOrder       = (PRECORDCORE)CMA_END;
        ri.pRecordParent      = (PRECORDCORE)NULL;
        ri.zOrder             = (ULONG)CMA_TOP;
        ri.cRecordsInsert     = recsNeeded;
        ri.fInvalidateRecord  = FALSE;
        if (!WinSendMsg(hwndLeft,CM_INSERTRECORD,
                       MPFROMP(pcilFirst),MPFROMP(&ri))) {
          pcil = pcilFirst;
          while (pcil) {
            pcit = (PCNRITEM)pcil->rc.preccNextRecord;
            WinSendMsg(hwndLeft,CM_FREERECORD,
                       MPFROMP(&pcil),MPFROMSHORT(1));
            pcil = pcit;
          }
          numfilesl = 0;
        }
        memset(&ri, 0, sizeof(RECORDINSERT));
        ri.cb                 = sizeof(RECORDINSERT);
        ri.pRecordOrder       = (PRECORDCORE)CMA_END;
        ri.pRecordParent      = (PRECORDCORE)NULL;
        ri.zOrder             = (ULONG)CMA_TOP;
        ri.cRecordsInsert     = recsNeeded;
        ri.fInvalidateRecord  = FALSE;
        if (!WinSendMsg(hwndRight,CM_INSERTRECORD,
                       MPFROMP(pcirFirst),MPFROMP(&ri))) {
          WinSendMsg(hwndLeft,CM_REMOVERECORD,
                     MPVOID,MPFROM2SHORT(0,CMA_FREE | CMA_INVALIDATE));
          pcir = pcirFirst;
          while (pcir) {
            pcit = (PCNRITEM)pcir->rc.preccNextRecord;
            WinSendMsg(hwndRight,CM_FREERECORD,
                       MPFROMP(&pcir),MPFROMSHORT(1));
            pcir = pcit;
          }
          numfilesr = 0;
        }
        cmp->cmp->totalleft = numfilesl;
        cmp->cmp->totalright = numfilesr;
      } // if recsNeeded
      Deselect(hwndLeft);
      Deselect(hwndRight);
      if (!PostMsg(cmp->hwnd,UM_CONTAINER_FILLED,MPVOID,MPVOID))
        WinSendMsg (cmp->hwnd,UM_CONTAINER_FILLED,MPVOID,MPVOID);
      notified = TRUE;
      if (filesl)
        FreeList((CHAR **)filesl);	// Must have failed to create container
      if (filesr)
        FreeList((CHAR **)filesr);
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  if (!notified)
    PostMsg(cmp->hwnd,UM_CONTAINER_FILLED,MPVOID,MPVOID);
  free(cmp);
  DosPostEventSem(CompactSem);
}

#define hwndLeft  (WinWindowFromID(hwnd,COMP_LEFTDIR))
#define hwndRight (WinWindowFromID(hwnd,COMP_RIGHTDIR))

//=== CompareDlgProc() Compare directories dialog procedure ===

MRESULT EXPENTRY CompareDlgProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  COMPARE        *cmp;

  static HPOINTER hptr = (HPOINTER)0;

  switch(msg) {
    case WM_INITDLG:
      cmp = (COMPARE *)mp2;
      if (!cmp) {
	Runtime_Error(pszSrcFile, __LINE__, "no data");
        WinDismissDlg(hwnd,0);
      }
      else {
        if (!hptr)
          hptr = WinLoadPointer(HWND_DESKTOP,FM3ModHandle,COMPARE_ICON);
        WinDefDlgProc(hwnd,WM_SETICON,MPFROMLONG(hptr),MPVOID);
        cmp->hwnd = hwnd;
        WinSetWindowPtr(hwnd,QWL_USER,(PVOID)cmp);
        SetCnrCols(hwndLeft,TRUE);
        SetCnrCols(hwndRight,TRUE);
        WinSendMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
        WinSendMsg(hwnd,UM_SETDIR,MPVOID,MPVOID);
        PostMsg(hwnd,UM_STRETCH,MPVOID,MPVOID);
        {
          USHORT       ids[] = {COMP_LEFTDIR,COMP_RIGHTDIR,COMP_TOTALLEFT,
                                COMP_TOTALRIGHT,COMP_SELLEFT,COMP_SELRIGHT,
                                0};
          register INT x;

          for(x = 0;ids[x];x++)
            SetPresParams(WinWindowFromID(hwnd,ids[x]),
                          &RGBGREY,
                          &RGBBLACK,
                          &RGBBLACK,
                          GetPString(IDS_8HELVTEXT));
        }
      }
      break;

    case UM_STRETCH:
      {
        SWP  swp,swpC;
        LONG titl,szbx,szby,sz;
        HWND hwndActive;

        WinQueryWindowPos(hwnd,&swp);
        if(!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
          hwndActive = WinQueryFocus(HWND_DESKTOP);
          szbx = SysVal(SV_CXSIZEBORDER);
          szby = SysVal(SV_CYSIZEBORDER);
          titl = SysVal(SV_CYTITLEBAR);
          titl += 26;
          swp.cx -= (szbx * 2);
          sz = (swp.cx / 8);
          WinQueryWindowPos(WinWindowFromID(hwnd,COMP_LEFTDIR),&swpC);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_LEFTDIR),HWND_TOP,
                          szbx + 6,
                          swpC.y,
                          (swp.cx / 2) - (szbx + 6),
                          ((swp.cy - swpC.y) - titl) - szby,
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_RIGHTDIR),HWND_TOP,
                          (swp.cx / 2) + (szbx + 6),
                          swpC.y,
                          (swp.cx / 2) - (szbx + 6),
                          ((swp.cy - swpC.y) - titl) - szby,
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_TOTALLEFTHDR),HWND_TOP,
                          szbx + 6,
                          ((swp.cy - titl) - szby) + 4,
                          sz - (szbx + 6),
                          20,
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_TOTALLEFT),HWND_TOP,
                          sz + (szbx + 6),
                          ((swp.cy - titl) - szby) + 4,
                          sz - (szbx + 6),
                          20,
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_SELLEFTHDR),HWND_TOP,
                          (sz * 2) + (szbx + 6),
                          ((swp.cy - titl) - szby) + 4,
                          sz - (szbx + 6),
                          20,
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_SELLEFT),HWND_TOP,
                          (sz * 3) + (szbx + 6),
                          ((swp.cy - titl) - szby) + 4,
                          sz - (szbx + 6),
                          20,
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_TOTALRIGHTHDR),HWND_TOP,
                          (sz * 4) + (szbx + 6),
                          ((swp.cy - titl) - szby) + 4,
                          sz - (szbx + 6),
                          20,
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_TOTALRIGHT),HWND_TOP,
                          (sz * 5) + (szbx + 6),
                          ((swp.cy - titl) - szby) + 4,
                          sz - (szbx + 6),
                          20,
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_SELRIGHTHDR),HWND_TOP,
                          (sz * 6) + (szbx + 6),
                          ((swp.cy - titl) - szby) + 4,
                          sz - (szbx + 6),
                          20,
                          SWP_MOVE | SWP_SIZE);
          WinSetWindowPos(WinWindowFromID(hwnd,COMP_SELRIGHT),HWND_TOP,
                          (sz * 7) + (szbx + 6),
                          ((swp.cy - titl) - szby) + 4,
                          sz - (szbx + 6),
                          20,
                          SWP_MOVE | SWP_SIZE);
          PaintRecessedWindow(WinWindowFromID(hwnd,COMP_TOTALLEFT),
                              (HPS)0,FALSE,FALSE);
          PaintRecessedWindow(WinWindowFromID(hwnd,COMP_SELLEFT),
                              (HPS)0,FALSE,FALSE);
          PaintRecessedWindow(WinWindowFromID(hwnd,COMP_TOTALRIGHT),
                              (HPS)0,FALSE,FALSE);
          PaintRecessedWindow(WinWindowFromID(hwnd,COMP_SELRIGHT),
                              (HPS)0,FALSE,FALSE);
          PaintRecessedWindow(hwndLeft,(HPS)0,
                              (hwndActive == hwndLeft),
                              TRUE);
          PaintRecessedWindow(hwndRight,(HPS)0,
                              (hwndActive == hwndRight),
                              TRUE);
        }
      }
      return 0;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,UM_STRETCH,MPVOID,MPVOID);
      break;

    case UM_SETUP:
      {
        CNRINFO cnri;
        BOOL    tempsubj;

        cmp = INSTDATA(hwnd);
        if(cmp) {
          cmp->dcd.size = sizeof(DIRCNRDATA);
          cmp->dcd.type = DIR_FRAME;
          cmp->dcd.hwndFrame = hwnd;
          cmp->dcd.hwndClient = hwnd;
          cmp->dcd.mask.attrFile = (FILE_DIRECTORY | FILE_ARCHIVED |
                                    FILE_READONLY  | FILE_SYSTEM   |
                                    FILE_HIDDEN);
          LoadDetailsSwitches("DirCmp",&cmp->dcd);
          cmp->dcd.detailslongname = FALSE;
          cmp->dcd.detailsicon     = FALSE; /* TRUE; */
        }
        memset(&cnri,0,sizeof(CNRINFO));
        cnri.cb = sizeof(CNRINFO);
        WinSendDlgItemMsg(hwnd,COMP_LEFTDIR,CM_QUERYCNRINFO,
                          MPFROMP(&cnri),MPFROMLONG(sizeof(CNRINFO)));
        cnri.flWindowAttr |= (CA_OWNERDRAW | CV_MINI);
        cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 68;
        WinSendDlgItemMsg(hwnd,COMP_LEFTDIR,CM_SETCNRINFO,MPFROMP(&cnri),
                          MPFROMLONG(CMA_FLWINDOWATTR | CMA_XVERTSPLITBAR));
        memset(&cnri,0,sizeof(CNRINFO));
        cnri.cb = sizeof(CNRINFO);
        WinSendDlgItemMsg(hwnd,COMP_RIGHTDIR,CM_QUERYCNRINFO,
                          MPFROMP(&cnri),MPFROMLONG(sizeof(CNRINFO)));
        cnri.flWindowAttr |= (CA_OWNERDRAW | CV_MINI);
        cnri.xVertSplitbar = DIR_SPLITBAR_OFFSET - 54;
        WinSendDlgItemMsg(hwnd,COMP_RIGHTDIR,CM_SETCNRINFO,MPFROMP(&cnri),
                          MPFROMLONG(CMA_FLWINDOWATTR | CMA_XVERTSPLITBAR));
        AdjustCnrColRO(hwndLeft,GetPString(IDS_FILENAMECOLTEXT),
                       TRUE,FALSE);
        AdjustCnrColRO(hwndLeft,GetPString(IDS_LONGNAMECOLTEXT),
                       TRUE,FALSE);
        AdjustCnrColRO(hwndRight,GetPString(IDS_FILENAMECOLTEXT),
                       TRUE,FALSE);
        AdjustCnrColRO(hwndRight,GetPString(IDS_LONGNAMECOLTEXT),
                       TRUE,FALSE);
        AdjustCnrColsForPref(hwndLeft,
                             cmp->leftdir,&cmp->dcd,TRUE);
        tempsubj = cmp->dcd.detailssubject;
        cmp->dcd.detailssubject = FALSE;
        AdjustCnrColsForPref(hwndRight,
                             cmp->rightdir,&cmp->dcd,TRUE);
        if(*cmp->rightlist) {
          AdjustCnrColVis(hwndRight,GetPString(IDS_LADATECOLTEXT),FALSE,FALSE);
          AdjustCnrColVis(hwndRight,GetPString(IDS_LATIMECOLTEXT),FALSE,FALSE);
          AdjustCnrColVis(hwndRight,GetPString(IDS_CRDATECOLTEXT),FALSE,FALSE);
          AdjustCnrColVis(hwndRight,GetPString(IDS_CRTIMECOLTEXT),FALSE,FALSE);
        }
        cmp->dcd.detailssubject = tempsubj;
      }
      return 0;

    case WM_DRAWITEM:
      if(mp2) {

        POWNERITEM       pown = (POWNERITEM)mp2;
        PCNRDRAWITEMINFO pcown;
        PCNRITEM         pci;

        pcown = (PCNRDRAWITEMINFO)pown->hItem;
        if(pcown) {
          pci = (PCNRITEM)pcown->pRecord;
          if(pci && (INT)pci != -1 && !*pci->szFileName)
            return MRFROMLONG(TRUE);
        }
      }
      return 0;

    case UM_CONTAINERHWND:
      WinSetDlgItemText(hwnd,COMP_NOTE,
                        GetPString(IDS_COMPHOLDBLDLISTTEXT));
      return 0;

    case UM_CONTAINERDIR:
      WinSetDlgItemText(hwnd,COMP_NOTE,
                        GetPString(IDS_COMPHOLDFILLCNRTEXT));
      return 0;

    case UM_CONTAINER_FILLED:
      cmp = INSTDATA(hwnd);
      if (!cmp) {
	Runtime_Error(pszSrcFile, __LINE__, "pCompare NULL");
        WinDismissDlg(hwnd,0);
      }
      else {
        cmp->filling = FALSE;
        WinEnableWindow(hwndLeft,TRUE);
        WinEnableWindow(hwndRight,TRUE);
        WinEnableWindowUpdate(hwndLeft,TRUE);
        WinEnableWindowUpdate(hwndRight,TRUE);
        {
          CHAR s[81];

          sprintf(s," %d",cmp->totalleft);
          WinSetDlgItemText(hwnd,COMP_TOTALLEFT,s);
          sprintf(s," %d",cmp->totalright);
          WinSetDlgItemText(hwnd,COMP_TOTALRIGHT,s);
          sprintf(s," %d",cmp->selleft);
          WinSetDlgItemText(hwnd,COMP_SELLEFT,s);
          sprintf(s," %d",cmp->selright);
          WinSetDlgItemText(hwnd,COMP_SELRIGHT,s);
        }
        WinEnableWindow(WinWindowFromID(hwnd,DID_OK),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,DID_CANCEL),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,COMP_COLLECT),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTBOTH),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTONE),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTNEWER),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTOLDER),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTBIGGER),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSMALLER),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTBOTH),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTONE),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTNEWER),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTOLDER),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTBIGGER),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTSMALLER),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTALL),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSAMECONTENT),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTIDENTICAL),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSAME),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,IDM_INVERT),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,COMP_SETDIRS),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,COMP_DELETELEFT),TRUE);
        WinEnableWindow(WinWindowFromID(hwnd,COMP_FILTER),TRUE);
        if(!*cmp->rightlist) {
          WinEnableWindow(WinWindowFromID(hwnd,COMP_COPYLEFT),TRUE);
          WinEnableWindow(WinWindowFromID(hwnd,COMP_MOVELEFT),TRUE);
          WinEnableWindow(WinWindowFromID(hwnd,COMP_DELETERIGHT),TRUE);
          WinEnableWindow(WinWindowFromID(hwnd,COMP_COPYRIGHT),TRUE);
          WinEnableWindow(WinWindowFromID(hwnd,COMP_MOVERIGHT),TRUE);
        }
        WinEnableWindow(WinWindowFromID(hwnd,COMP_INCLUDESUBDIRS),TRUE);
        if(*cmp->dcd.mask.szMask)
          WinSetDlgItemText(hwnd,COMP_NOTE,
                            GetPString(IDS_COMPREADYFILTEREDTEXT));
        else
          WinSetDlgItemText(hwnd,COMP_NOTE,
                            GetPString(IDS_COMPREADYTEXT));
      }
      break;

    case WM_INITMENU:
      cmp = INSTDATA(hwnd);
      if(cmp) {
        switch(SHORT1FROMMP(mp1)) {
          case IDM_COMMANDSMENU:
            SetupCommandMenu(cmp->dcd.hwndLastMenu,hwnd);
            break;
        }
      }
      break;

    case WM_MENUEND:
      cmp = INSTDATA(hwnd);
      if(cmp) {
        if((HWND)mp2 == cmp->dcd.hwndLastMenu) {
          MarkAll(hwndLeft,TRUE,FALSE,TRUE);
          MarkAll(hwndRight,TRUE,FALSE,TRUE);
          WinDestroyWindow(cmp->dcd.hwndLastMenu);
          cmp->dcd.hwndLastMenu = (HWND)0;
        }
      }
      break;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case COMP_INCLUDESUBDIRS:
          switch(SHORT2FROMMP(mp1)) {
            case BN_CLICKED:
              cmp = INSTDATA(hwnd);
	      if (cmp)
                *cmp->rightlist = 0;
              PostMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
              PostMsg(hwnd,UM_SETDIR,MPVOID,MPVOID);
              break;
          }
          break;
        case COMP_HIDENOTSELECTED:
          switch(SHORT2FROMMP(mp1)) {
            case BN_CLICKED:
              WinSendMsg(hwnd,UM_HIDENOTSELECTED,MPVOID,MPVOID);
              break;
          }
          break;

        case COMP_LEFTDIR:
        case COMP_RIGHTDIR:
          switch(SHORT2FROMMP(mp1)) {
            case CN_KILLFOCUS:
              PaintRecessedWindow(WinWindowFromID(hwnd,SHORT1FROMMP(mp1)),
                                  (HPS)0,FALSE,TRUE);
              break;

            case CN_SETFOCUS:
              PaintRecessedWindow(WinWindowFromID(hwnd,SHORT1FROMMP(mp1)),
                                  (HPS)0,TRUE,TRUE);
              break;

            case CN_ENTER:
              if(mp2) {

                PCNRITEM pci = (PCNRITEM)((PNOTIFYRECORDENTER)mp2)->pRecord;
                HWND     hwndCnr = WinWindowFromID(hwnd,SHORT1FROMMP(mp1));

                SetShiftState();
                if(pci) {
                  if((pci->rc.flRecordAttr & CRA_INUSE) || !*pci->szFileName)
                    break;
                  WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                             MPFROM2SHORT(TRUE,CRA_INUSE));
                  if(pci->attrFile & FILE_DIRECTORY) {
                    if((shiftstate & (KC_CTRL | KC_SHIFT)) ==
                       (KC_CTRL | KC_SHIFT))
                      OpenObject(pci->szFileName,Settings,hwnd);
                    else
                      OpenObject(pci->szFileName,Default,hwnd);
                  }
                  else
                    DefaultViewKeys(hwnd,hwnd,HWND_DESKTOP,NULL,
                                    pci->szFileName);
                  WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,
                             MPFROMP(pci),
                             MPFROM2SHORT(FALSE,CRA_INUSE |
                                          ((fUnHilite) ? CRA_SELECTED : 0)));
                }
              }
              break;

            case CN_CONTEXTMENU:
              cmp = INSTDATA(hwnd);
              if(cmp) {

                PCNRITEM pci = (PCNRITEM)mp2;
                USHORT   id = COMP_CNRMENU;

                if(cmp->dcd.hwndLastMenu)
                  WinDestroyWindow(cmp->dcd.hwndLastMenu);
                cmp->dcd.hwndLastMenu = (HWND)0;
                cmp->hwndCalling = WinWindowFromID(hwnd,SHORT1FROMMP(mp1));
                if(pci) {
                  if(!*pci->szFileName || *cmp->rightlist)
                    break;
                  id = COMP_MENU;
                  WinSendMsg(cmp->hwndCalling,CM_SETRECORDEMPHASIS,
                             MPFROMP(pci),MPFROM2SHORT(TRUE,CRA_CURSORED));
                }
                cmp->dcd.hwndLastMenu = WinLoadMenu(HWND_DESKTOP,FM3ModHandle,
                                                    id);
                if(cmp->dcd.hwndLastMenu) {
                  if(id == COMP_CNRMENU) {
                    if(SHORT1FROMMP(mp1) == COMP_RIGHTDIR)
                      WinSendMsg(cmp->dcd.hwndLastMenu,MM_DELETEITEM,
                                 MPFROM2SHORT(IDM_SHOWSUBJECT,FALSE),MPVOID);
                    SetDetailsSwitches(cmp->dcd.hwndLastMenu,&cmp->dcd);
                    if(SHORT1FROMMP(mp1) == COMP_LEFTDIR)
                      WinSendMsg(cmp->dcd.hwndLastMenu,MM_DELETEITEM,
                                 MPFROM2SHORT(IDM_LOADLISTFILE,0),MPVOID);
                    else if(*cmp->rightlist)
                      WinSendMsg(cmp->dcd.hwndLastMenu,MM_DELETEITEM,
                                 MPFROM2SHORT(IDM_SAVELISTFILE,0),MPVOID);
                  }
                  PopupMenu(hwnd,hwnd,cmp->dcd.hwndLastMenu);
                }
              }
              break;

            case CN_INITDRAG:
              cmp = INSTDATA(hwnd);
              if(*cmp->rightlist && SHORT1FROMMP(mp1) == COMP_RIGHTDIR)
                break;
              DoFileDrag(WinWindowFromID(hwnd,SHORT1FROMMP(mp1)),
                         (HWND)0,
                         mp2,
                         NULL,
                         NULL,
                         TRUE);
              break;

	    // fixme to be gone - field edits not allowed
            case CN_BEGINEDIT:
              {
                PFIELDINFO pfi = ((PCNREDITDATA)mp2)->pFieldInfo;
                PCNRITEM   pci = (PCNRITEM)((PCNREDITDATA)mp2)->pRecord;

                if (pfi || pci) {
                  PostMsg(hwnd,
                          CM_CLOSEEDIT,
                          MPVOID,
                          MPVOID);
                  // DosBeep(250,100);		// fixme
	          Runtime_Error(pszSrcFile, __LINE__, "CN_BEGINEDIT unexpected");
                }
              }
              break;

	    // fixme to be gone - field edits not allowed
            case CN_REALLOCPSZ:
              cmp = INSTDATA(hwnd);
              if (!cmp)
	        Runtime_Error(pszSrcFile, __LINE__, "no data");
	      else {
                PFIELDINFO  pfi = ((PCNREDITDATA)mp2)->pFieldInfo;
                PCNRITEM    pci = (PCNRITEM)((PCNREDITDATA)mp2)->pRecord;
                HWND        hwndMLE;
                CHAR        szData[CCHMAXPATH],testname[CCHMAXPATH],*p;

	        Runtime_Error(pszSrcFile, __LINE__, "CN_REALLOCPSZ unexpected");
                if (!pci && !pfi) {
                  hwndMLE = WinWindowFromID(WinWindowFromID(hwnd,
                                            SHORT1FROMMP(mp1)),CID_MLE);
                  WinQueryWindowText(hwndMLE,
                                     sizeof(szData),
                                     szData);
                  p = strchr(szData,'\n');
                  if (p)
                    *p = 0;
                  p = strchr(szData,'\r');
                  if (p)
                    *p = 0;
                  bstrip(szData);
                  if (*szData) {
                    if (!DosQueryPathInfo(szData,
                                         FIL_QUERYFULLNAME,
                                         testname,
                                         sizeof(testname))) {
                      if (!SetDir(cmp->hwndParent,
                                 hwnd,
                                 testname,
                                 1)) {
                        if (SHORT1FROMMP(mp1) == COMP_LEFTDIR)
                          strcpy(cmp->leftdir,testname);
                        else {
                          strcpy(cmp->rightdir,testname);
                          *cmp->rightlist = 0;
                        }
                        PostMsg(hwnd,
                                UM_SETUP,
                                MPVOID,
                                MPVOID);
                        PostMsg(hwnd,
                                UM_SETDIR,
                                MPVOID,
                                MPVOID);
                      }
                    }
                  }
                }
              }
              break;

            case CN_EMPHASIS:
              {
                PNOTIFYRECORDEMPHASIS pre = mp2;
                PCNRITEM              pci;

                if(pre->fEmphasisMask & CRA_SELECTED) {
                  pci = (PCNRITEM)pre->pRecord;
                  if(pci) {
                    if(!*pci->szFileName) {
                      if(pci->rc.flRecordAttr & CRA_SELECTED)
                        WinSendDlgItemMsg(hwnd,SHORT1FROMMP(mp1),
                                          CM_SETRECORDEMPHASIS,
                                          MPFROMP(pci),
                                          MPFROM2SHORT(FALSE,CRA_SELECTED));
                    }
                    else {

                      CHAR s[81];

                      cmp = INSTDATA(hwnd);
                      if(pci->rc.flRecordAttr & CRA_SELECTED) {
                        if(SHORT1FROMMP(mp1) == COMP_LEFTDIR)
                          cmp->selleft++;
                        else
                          cmp->selright++;
                      }
                      else {
                        if(SHORT1FROMMP(mp1) == COMP_LEFTDIR) {
                          if(cmp->selleft)
                            cmp->selleft--;
                        }
                        else {
                          if(cmp->selright)
                            cmp->selright--;
                        }
                      }
                      if(SHORT1FROMMP(mp1) == COMP_LEFTDIR) {
                        if(WinIsWindowEnabled(hwndLeft) ||
                           !(cmp->selleft % 50)) {
                          sprintf(s," %d",cmp->selleft);
                          WinSetDlgItemText(hwnd,COMP_SELLEFT,s);
                        }
                      }
                      else {
                        if(WinIsWindowEnabled(hwndRight) ||
                           !(cmp->selright % 50)) {
                          sprintf(s," %d",cmp->selright);
                          WinSetDlgItemText(hwnd,COMP_SELRIGHT,s);
                        }
                      }
                    }
                  }
                }
              }
              break;

            case CN_SCROLL:
              cmp = INSTDATA(hwnd);
              if(!cmp->forcescroll) {

                PNOTIFYSCROLL pns = mp2;

                if(pns->fScroll & CMA_VERTICAL) {
                  cmp->forcescroll = TRUE;
                  WinSendDlgItemMsg(hwnd,(SHORT1FROMMP(mp1) == COMP_LEFTDIR) ?
                                    COMP_RIGHTDIR : COMP_LEFTDIR,
                                    CM_SCROLLWINDOW,MPFROMSHORT(CMA_VERTICAL),
                                    MPFROMLONG(pns->lScrollInc));
                  cmp->forcescroll = FALSE;
                }
              }
              break;
          }
          break;			// COMP_RIGHTDIR
      }
      return 0;				// WM_CONTROL

    case UM_SETDIR:
      cmp = INSTDATA(hwnd);
      if(cmp) {

        COMPARE *forthread;
        CNRINFO  cnri;

        cmp->includesubdirs = WinQueryButtonCheckstate(hwnd,
                                                       COMP_INCLUDESUBDIRS);
        memset(&cnri,0,sizeof(CNRINFO));
        cnri.cb = sizeof(CNRINFO);
        cnri.pszCnrTitle = cmp->leftdir;
        cnri.flWindowAttr = CV_DETAIL | CV_MINI |
                            CA_CONTAINERTITLE | CA_TITLESEPARATOR |
                            CA_DETAILSVIEWTITLES | CA_OWNERDRAW;
        WinSendDlgItemMsg(hwnd,COMP_LEFTDIR,CM_SETCNRINFO,MPFROMP(&cnri),
                          MPFROMLONG(CMA_CNRTITLE | CMA_FLWINDOWATTR));
        cnri.pszCnrTitle = cmp->rightdir;
        WinSendDlgItemMsg(hwnd,COMP_RIGHTDIR,CM_SETCNRINFO,MPFROMP(&cnri),
                          MPFROMLONG(CMA_CNRTITLE | CMA_FLWINDOWATTR));
        cmp->filling = TRUE;
        forthread = xmalloc(sizeof(COMPARE),pszSrcFile,__LINE__);
        if(!forthread)
          WinDismissDlg(hwnd,0);
	else {
          *forthread = *cmp;
          forthread->cmp = cmp;
          if (_beginthread(FillCnrsThread,NULL,122880,(PVOID)forthread) == -1) {
	    Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
            WinDismissDlg(hwnd,0);
            free(forthread);
	  }
	  else {
            WinEnableWindowUpdate(hwndLeft,FALSE);
            WinEnableWindowUpdate(hwndRight,FALSE);
            cmp->selleft = cmp->selright = 0;
            WinSetDlgItemText(hwnd,COMP_SELLEFT,"0");
            WinSetDlgItemText(hwnd,COMP_SELRIGHT,"0");
            WinSetDlgItemText(hwnd,COMP_TOTALLEFT,"0");
            WinSetDlgItemText(hwnd,COMP_TOTALRIGHT,"0");
            WinSetDlgItemText(hwnd,COMP_NOTE,
                              GetPString(IDS_COMPHOLDREADDISKTEXT));
            WinEnableWindow(hwndRight,FALSE);
            WinEnableWindow(hwndLeft,FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,DID_OK),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,DID_CANCEL),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_COLLECT),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTBOTH),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTONE),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTNEWER),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTOLDER),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTBIGGER),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSMALLER),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTBOTH),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTONE),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTNEWER),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTOLDER),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTBIGGER),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTSMALLER),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTALL),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_INCLUDESUBDIRS),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_SETDIRS),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_DELETELEFT),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_DELETERIGHT),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_COPYLEFT),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_MOVELEFT),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_COPYRIGHT),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_MOVERIGHT),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSAMECONTENT),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTIDENTICAL),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSAME),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,IDM_INVERT),FALSE);
            WinEnableWindow(WinWindowFromID(hwnd,COMP_FILTER),FALSE);
          }
        }
      }
      return 0;

    case UM_FILTER:
      cmp = INSTDATA(hwnd);
      if(cmp) {
        if(mp1) {
          DosEnterCritSec();
           SetMask((CHAR *)mp1,&cmp->dcd.mask);
          DosExitCritSec();
        }
        cmp->dcd.suspendview = 1;
        WinSendMsg(hwndLeft,CM_FILTER,MPFROMP(Filter),MPFROMP(&cmp->dcd.mask));
        WinSendMsg(hwndRight,CM_FILTER,MPFROMP(Filter),MPFROMP(&cmp->dcd.mask));
        cmp->dcd.suspendview = 0;
        if(*cmp->dcd.mask.szMask)
          WinSetDlgItemText(hwnd,COMP_NOTE,
                            GetPString(IDS_COMPREADYFILTEREDTEXT));
        else
          WinSetDlgItemText(hwnd,COMP_NOTE,
                            GetPString(IDS_COMPREADYTEXT));
      }
      return 0;

    case UM_HIDENOTSELECTED:
      cmp = INSTDATA(hwnd);
      if(cmp) {
        USHORT wantHide = WinQueryButtonCheckstate(hwnd,
                                                   COMP_HIDENOTSELECTED);

        cmp->dcd.suspendview = 1;
	if (wantHide) {
	  BOOL needRefresh = FALSE;
          HWND hwndl = WinWindowFromID(cmp->hwnd,COMP_LEFTDIR);
          HWND hwndr = WinWindowFromID(cmp->hwnd,COMP_RIGHTDIR);
          PCNRITEM pcil = WinSendMsg(hwndl,CM_QUERYRECORD,MPVOID,
                                     MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
          PCNRITEM pcir = WinSendMsg(hwndr,CM_QUERYRECORD,MPVOID,
                                     MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
          while(pcil && (INT)pcil != -1 && pcir && (INT)pcir != -1) {
	    if (~pcil->rc.flRecordAttr & CRA_SELECTED &&
	        ~pcir->rc.flRecordAttr & CRA_SELECTED) {
	      pcil->rc.flRecordAttr |= CRA_FILTERED;
	      pcir->rc.flRecordAttr |= CRA_FILTERED;
	      needRefresh = TRUE;
	    }
            pcil = WinSendMsg(hwndl,CM_QUERYRECORD,MPFROMP(pcil),
                              MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
            pcir = WinSendMsg(hwndr,CM_QUERYRECORD,MPFROMP(pcir),
                              MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
          } // while
	  if (needRefresh) {
            WinSendMsg(hwndl,CM_INVALIDATERECORD,
                     MPVOID,MPFROM2SHORT(0,CMA_REPOSITION));
            WinSendMsg(hwndr,CM_INVALIDATERECORD,
                     MPVOID,MPFROM2SHORT(0,CMA_REPOSITION));
	  }
	}
	else {
          WinSendMsg(hwndLeft,CM_FILTER,MPFROMP(Filter),MPFROMP(&cmp->dcd.mask));
          WinSendMsg(hwndRight,CM_FILTER,MPFROMP(Filter),MPFROMP(&cmp->dcd.mask));
	}
        cmp->dcd.suspendview = 0;
        if(*cmp->dcd.mask.szMask)
          WinSetDlgItemText(hwnd,COMP_NOTE,
                            GetPString(IDS_COMPREADYFILTEREDTEXT));
        else
          WinSetDlgItemText(hwnd,COMP_NOTE,
                            GetPString(IDS_COMPREADYTEXT));
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_COMPARE:
          cmp = INSTDATA(hwnd);
          if(cmp) {

            PCNRITEM pci;
            CHAR     ofile[CCHMAXPATH];

            pci = (PCNRITEM)WinSendMsg(cmp->hwndCalling,
                                       CM_QUERYRECORDEMPHASIS,
                                       MPFROMLONG(CMA_FIRST),
                                       MPFROMSHORT(CRA_CURSORED));
            if(pci) {
              if(cmp->hwndCalling == hwndLeft)
                strcpy(ofile,cmp->rightdir);
              else
                strcpy(ofile,cmp->leftdir);
              if(ofile[strlen(ofile) - 1] != '\\')
                strcat(ofile,"\\");
              strcat(ofile,pci->pszFileName);
              if(*compare) {

                CHAR *fakelist[3];

                fakelist[0] = pci->szFileName;
                fakelist[1] = ofile;
                fakelist[2] = NULL;
                ExecOnList(hwnd,compare,
                           WINDOWED | SEPARATEKEEP,
                           NULL,fakelist,NULL);
              }
              else {

                FCOMPARE fc;

                memset(&fc,0,sizeof(fc));
                fc.size = sizeof(fc);
                fc.hwndParent = hwnd;
                strcpy(fc.file1,pci->szFileName);
                strcpy(fc.file2,ofile);
                WinDlgBox(HWND_DESKTOP,hwnd,
                          CFileDlgProc,FM3ModHandle,
                          FCMP_FRAME,(PVOID)&fc);
              }
            }
          }
          break;

        case COMP_FILTER:
        case IDM_FILTER:
          cmp = INSTDATA(hwnd);
          if(cmp) {

            BOOL     empty = FALSE;
            PCNRITEM pci;
            CHAR    *p;
            BOOL temp;

            if(!*cmp->dcd.mask.szMask) {
              empty = TRUE;
              temp = fSelectedAlways;
              fSelectedAlways = TRUE;
              pci = (PCNRITEM)CurrentRecord(hwnd);
              fSelectedAlways = temp;
              if(pci && !(pci->attrFile & FILE_DIRECTORY)) {
                p = strrchr(pci->szFileName,'\\');
                if(p) {
                  p++;
                  strcpy(cmp->dcd.mask.szMask,p);
                }
              }
            }
            cmp->dcd.mask.fNoAttribs = TRUE;
            cmp->dcd.mask.attrFile = ALLATTRS;
            cmp->dcd.mask.antiattr = 0;
            if(WinDlgBox(HWND_DESKTOP,hwnd,PickMaskDlgProc,
                         FM3ModHandle,MSK_FRAME,MPFROMP(&cmp->dcd.mask))) {
              cmp->dcd.mask.attrFile = ALLATTRS;
              cmp->dcd.mask.antiattr = 0;
              WinSendMsg(hwnd,UM_FILTER,MPVOID,MPVOID);
            }
            else if(empty) {
              *cmp->dcd.mask.szMask = 0;
              cmp->dcd.mask.attrFile = ALLATTRS;
              cmp->dcd.mask.antiattr = 0;
            }
          }
          break;

        case IDM_SHOWSUBJECT:
        case IDM_SHOWEAS:
        case IDM_SHOWSIZE:
        case IDM_SHOWLWDATE:
        case IDM_SHOWLWTIME:
        case IDM_SHOWLADATE:
        case IDM_SHOWLATIME:
        case IDM_SHOWCRDATE:
        case IDM_SHOWCRTIME:
        case IDM_SHOWATTR:
          cmp = INSTDATA(hwnd);
          if(cmp) {

            DIRCNRDATA dcd1;
            BOOL       tempsubj;

            dcd1 = cmp->dcd;
            AdjustDetailsSwitches(hwndLeft,
                                  (HWND)0,SHORT1FROMMP(mp1),
                                  cmp->leftdir,"DirCmp",&cmp->dcd,
                                  TRUE);
            tempsubj = cmp->dcd.detailssubject;
            cmp->dcd = dcd1;
            cmp->dcd.detailssubject = FALSE;
            AdjustDetailsSwitches(hwndRight,
                                  cmp->dcd.hwndLastMenu,SHORT1FROMMP(mp1),
                                  cmp->rightdir,"DirCmp",&cmp->dcd,TRUE);
            cmp->dcd.detailssubject = tempsubj;
          }
          break;

        case IDM_LOADLISTFILE:
          cmp = INSTDATA(hwnd);
          if(cmp) {

            CHAR fullname[CCHMAXPATH];

            strcpy(fullname,"*.PMD");
            if(insert_filename(HWND_DESKTOP,fullname,TRUE,FALSE) &&
               *fullname && !strchr(fullname,'*') && !strchr(fullname,'?')) {
              strcpy(cmp->rightlist,fullname);
              PostMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
              PostMsg(hwnd,UM_SETDIR,MPVOID,MPVOID);
            }
          }
          break;

        case IDM_SAVELISTFILE:
          cmp = INSTDATA(hwnd);
          if(cmp) {

            SNAPSTUFF *sf;
            CHAR       fullname[CCHMAXPATH];

            strcpy(fullname,"*.PMD");
            if(export_filename(HWND_DESKTOP,fullname,1) && *fullname &&
               !strchr(fullname,'*') && !strchr(fullname,'?')) {
              sf = xmallocz(sizeof(SNAPSTUFF),pszSrcFile,__LINE__);
              if (sf) {
                strcpy(sf->filename,fullname);
                if(hwndLeft == cmp->hwndCalling)
                  strcpy(sf->dirname,cmp->leftdir);
                else
                  strcpy(sf->dirname,cmp->rightdir);
                sf->recurse = cmp->includesubdirs;
                if (_beginthread(StartSnap,NULL,65536,(PVOID)sf) == -1) {
	          Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
                  free(sf);
                }
              }
            }
          }
          break;

        case COMP_SETDIRS:
          cmp = INSTDATA(hwnd);
          if(cmp) {

            WALK2 wa;

            memset(&wa,0,sizeof(wa));
            wa.size = sizeof(wa);
            strcpy(wa.szCurrentPath1,cmp->leftdir);
            strcpy(wa.szCurrentPath2,cmp->rightdir);
            if(WinDlgBox(HWND_DESKTOP,hwnd,WalkTwoCmpDlgProc,
                         FM3ModHandle,WALK2_FRAME,
                         MPFROMP(&wa)) &&
               !IsFile(wa.szCurrentPath1) &&
               !IsFile(wa.szCurrentPath2)) {
              strcpy(cmp->leftdir,wa.szCurrentPath1);
              strcpy(cmp->rightdir,wa.szCurrentPath2);
              *cmp->rightlist = 0;
              PostMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
              PostMsg(hwnd,UM_SETDIR,MPVOID,MPVOID);
            }
          }
          break;

        case COMP_COPYLEFT:
        case COMP_MOVELEFT:
        case COMP_COPYRIGHT:
        case COMP_MOVERIGHT:
        case COMP_DELETELEFT:
        case COMP_DELETERIGHT:
          cmp = INSTDATA(hwnd);
          if(cmp) {

            COMPARE *forthread;

            cmp->filling = TRUE;
            forthread = xmalloc(sizeof(COMPARE),pszSrcFile,__LINE__);
            if (forthread) {
              *forthread = *cmp;
              forthread->cmp = cmp;
              forthread->action = SHORT1FROMMP(mp1);
              if (_beginthread(ActionCnrThread,NULL,122880,(PVOID)forthread) == -1) {
	        Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
                free(forthread);
	      }
	      else {
                WinEnableWindowUpdate(hwndLeft,FALSE);
                WinEnableWindowUpdate(hwndRight,FALSE);
                switch(SHORT1FROMMP(mp1)) {
                  case COMP_DELETELEFT:
                  case COMP_DELETERIGHT:
                    WinSetDlgItemText(hwnd,COMP_NOTE,
                                      GetPString(IDS_COMPHOLDDELETINGTEXT));
                    break;
                  case COMP_MOVELEFT:
                  case COMP_MOVERIGHT:
                    WinSetDlgItemText(hwnd,COMP_NOTE,
                                      GetPString(IDS_COMPHOLDMOVINGTEXT));
                    break;
                  case COMP_COPYLEFT:
                  case COMP_COPYRIGHT:
                    WinSetDlgItemText(hwnd,COMP_NOTE,
                                      GetPString(IDS_COMPHOLDCOPYINGTEXT));
                    break;
                  default:
                    WinSetDlgItemText(hwnd,COMP_NOTE,
                                      GetPString(IDS_COMPHOLDDUNNOTEXT));
                    break;
                }
                WinEnableWindow(hwndRight,FALSE);
                WinEnableWindow(hwndLeft,FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,DID_OK),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,DID_CANCEL),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_COLLECT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTBOTH),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTONE),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTNEWER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTOLDER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTBIGGER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSMALLER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTBOTH),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTONE),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTNEWER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTOLDER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTBIGGER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTSMALLER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTALL),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_INCLUDESUBDIRS),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_SETDIRS),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_DELETELEFT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_DELETERIGHT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_COPYLEFT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_MOVELEFT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_COPYRIGHT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_MOVERIGHT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSAMECONTENT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTIDENTICAL),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSAME),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_INVERT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_FILTER),FALSE);
              }
            }
          }
          break;

        case DID_OK:
          WinDismissDlg(hwnd,0);
          break;
        case DID_CANCEL:
          WinDismissDlg(hwnd,1);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_COMPARE,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case IDM_DESELECTALL:
        case IDM_SELECTNEWER:
        case IDM_SELECTOLDER:
        case IDM_SELECTBIGGER:
        case IDM_SELECTSMALLER:
        case IDM_DESELECTNEWER:
        case IDM_DESELECTOLDER:
        case IDM_DESELECTBIGGER:
        case IDM_DESELECTSMALLER:
        case IDM_DESELECTONE:
        case IDM_DESELECTBOTH:
        case IDM_SELECTBOTH:
        case IDM_SELECTONE:
        case IDM_SELECTSAMECONTENT:
        case IDM_SELECTIDENTICAL:		// name, size and time
        case IDM_SELECTSAME:			// name and size
        case IDM_INVERT:
          cmp = INSTDATA(hwnd);
          if (!cmp)
	      Runtime_Error(pszSrcFile, __LINE__, "no data");
	  else {
            COMPARE *forthread;

            cmp->filling = TRUE;
            forthread = xmalloc(sizeof(COMPARE),pszSrcFile,__LINE__);
            if (forthread) {
              *forthread = *cmp;
              forthread->cmp = cmp;
              forthread->action = SHORT1FROMMP(mp1);
              if (_beginthread(SelectCnrsThread,NULL,65536,(PVOID)forthread) == -1) {
	        Runtime_Error(pszSrcFile, __LINE__, GetPString(IDS_COULDNTSTARTTHREADTEXT));
                free(forthread);
	      }
	      else {
                WinEnableWindowUpdate(hwndLeft,FALSE);
                WinEnableWindowUpdate(hwndRight,FALSE);
                switch(SHORT1FROMMP(mp1)) {
                  case IDM_DESELECTALL:
                  case IDM_DESELECTNEWER:
                  case IDM_DESELECTOLDER:
                  case IDM_DESELECTBIGGER:
                  case IDM_DESELECTSMALLER:
                  case IDM_DESELECTONE:
                  case IDM_DESELECTBOTH:
                    WinSetDlgItemText(hwnd,COMP_NOTE,
                                      GetPString(IDS_COMPHOLDDESELTEXT));
                    break;
                  case IDM_INVERT:
                    WinSetDlgItemText(hwnd,COMP_NOTE,
                                      GetPString(IDS_COMPHOLDINVERTTEXT));
                    break;
                  default:
                    WinSetDlgItemText(hwnd,COMP_NOTE,
                                      GetPString(IDS_COMPHOLDSELTEXT));
                    break;
                }
                WinEnableWindow(hwndRight,FALSE);
                WinEnableWindow(hwndLeft,FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,DID_OK),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,DID_CANCEL),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_COLLECT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTBOTH),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTONE),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTNEWER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTOLDER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTBIGGER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSMALLER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTBOTH),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTONE),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTNEWER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTOLDER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTBIGGER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTSMALLER),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_DESELECTALL),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_INCLUDESUBDIRS),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_SETDIRS),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_DELETELEFT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_DELETERIGHT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_COPYLEFT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_MOVELEFT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_COPYRIGHT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_MOVERIGHT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSAMECONTENT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTIDENTICAL),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_SELECTSAME),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,IDM_INVERT),FALSE);
                WinEnableWindow(WinWindowFromID(hwnd,COMP_FILTER),FALSE);
              }
            }
          }
          break;

        case COMP_COLLECT:
          cmp = INSTDATA(hwnd);
          if(cmp) {

            CHAR **listl,**listr = NULL;

            if(!Collector) {

              SWP  swp;
              HWND hwndC;

              if(!fAutoTile && !ParentIsDesktop(hwnd,cmp->hwndParent) &&
                 (!fExternalCollector && !strcmp(realappname,FM3Str)))
                GetNextWindowPos(cmp->hwndParent,&swp,NULL,NULL);
              hwndC = StartCollector((fExternalCollector ||
                                      strcmp(realappname,FM3Str)) ?
                                     HWND_DESKTOP :
                                     cmp->hwndParent,4);
              if(hwndC) {
                if(!fAutoTile && !ParentIsDesktop(hwnd,cmp->hwndParent) &&
                   (!fExternalCollector && !strcmp(realappname,FM3Str)))
                  WinSetWindowPos(hwndC,HWND_TOP,swp.x,swp.y,
                                  swp.cx,swp.cy,SWP_MOVE | SWP_SIZE |
                                  SWP_SHOW | SWP_ZORDER);
                else if(!ParentIsDesktop(hwnd,cmp->hwndParent) && fAutoTile &&
                        !strcmp(realappname,FM3Str))
                  TileChildren(cmp->hwndParent,TRUE);
                DosSleep(64L);
                PostMsg(hwnd,WM_COMMAND,MPFROM2SHORT(COMP_COLLECT,0),
                           MPVOID);
                break;
              }
            }
            else
              StartCollector(cmp->hwndParent,4);
            {
              BOOL temp;

              temp = fSelectedAlways;
              fSelectedAlways = TRUE;
              listl = BuildList(hwndLeft);
              if(!*cmp->rightlist)
                listr = BuildList(hwndRight);
              fSelectedAlways = temp;
            }
            if(listl || listr) {
              if(Collector) {
                if(listl) {
                  if(!PostMsg(Collector,WM_COMMAND,
                                 MPFROM2SHORT(IDM_COLLECTOR,0),
                                 MPFROMP(listl)))
                    FreeList(listl);
                }
                if(listr) {
                  if(!PostMsg(Collector,WM_COMMAND,
                                 MPFROM2SHORT(IDM_COLLECTOR,0),
                                 MPFROMP(listr)))
                    FreeList(listr);
                }
                WinSetWindowPos(WinQueryWindow(WinQueryWindow(Collector,
                                QW_PARENT),QW_PARENT),HWND_TOP,
                                0,0,0,0,SWP_ACTIVATE);
              }
              else {
                FreeList(listl);
                FreeList(listr);
              }
            }
          }
          break;
      }
      return 0;

    case WM_CLOSE:
      WinDismissDlg(hwnd,0);
      return 0;

    case WM_DESTROY:
      cmp = INSTDATA(hwnd);
      if(cmp) {
        if(cmp->dcd.hwndLastMenu)
          WinDestroyWindow(cmp->dcd.hwndLastMenu);
        if(cmp->dcd.hwndObject) {
          WinSetWindowPtr(cmp->dcd.hwndObject,0,(PVOID)NULL);
          if(!PostMsg(cmp->dcd.hwndObject,WM_CLOSE,MPVOID,MPVOID))
            WinSendMsg(cmp->dcd.hwndObject,WM_CLOSE,MPVOID,MPVOID);
        }
        free(cmp);
      }
      EmptyCnr(hwndLeft);
      EmptyCnr(hwndRight);
      DosPostEventSem(CompactSem);
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}
