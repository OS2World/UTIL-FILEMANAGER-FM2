
/***********************************************************************

  $Id$

  Directory size

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2002 Steven H.Levine

  Revisions	16 Oct 02 SHL - Handle large partitions
		12 Feb 03 SHL - Use CBLIST_TO_EASIZE

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma alloc_text(DIRSIZE,ProcessDir,FillCnrs,DirSizeProc)
#pragma alloc_text(DIRSIZE2,PrintToFile,FillInRecSizes,SortSizeCnr)

typedef struct {
  CHAR        *filename;
  HWND         hwndCnr;
  CHAR        *stopflag;
  DIRCNRDATA  *dcd;
} DIRSIZE;

typedef struct {
  CHAR      dirname[CCHMAXPATH];
  CHAR      stopflag;
  BOOL      dying;
  BOOL      working;
  HPOINTER  hptr;
} TEMP;


SHORT APIENTRY SortSizeCnr (PMINIRECORDCORE p1,PMINIRECORDCORE p2,
                            PVOID SortFlags) {

  ULONG size1,size2;

  size1 = ((PCNRITEM)p1)->cbFile + ((PCNRITEM)p1)->easize;
  size2 = ((PCNRITEM)p2)->cbFile + ((PCNRITEM)p2)->easize;
  return (size1 < size2) ? 1 : (size1 == size2) ? 0 : -1;
}


static ULONG ProcessDir (HWND hwndCnr,CHAR *filename,PCNRITEM pciParent,
                         CHAR *stopflag,BOOL top)
{
  CHAR           maskstr[CCHMAXPATH],*pEndMask;
  register char *p,*sp,*pp;
  ULONG          nm,totalbytes = 0L,subbytes = 0L,temp;
  HDIR           hdir;
  FILEFINDBUF4  *ffb;
  APIRET         rc;
  RECORDINSERT   ri;
  PCNRITEM       pciP;

  ffb = malloc(sizeof(FILEFINDBUF4) /* * FilesToGet */);
  if(!ffb)
    return -1L;
  strcpy(maskstr,filename);
  if(maskstr[strlen(maskstr) - 1] != '\\')
    strcat(maskstr,"\\");
  pEndMask = &maskstr[strlen(maskstr)];	// Point after last \
  strcat(maskstr,"*");
//printf("%s\n",maskstr);
  hdir = HDIR_CREATE;
  nm = 1L;
  memset(ffb,0,sizeof(ffb));
  DosError(FERR_DISABLEHARDERR);
//printf("FIND1\n");
  rc = DosFindFirst(filename, &hdir,
                    FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
                    FILE_SYSTEM | FILE_HIDDEN | MUST_HAVE_DIRECTORY,
                    ffb, sizeof(FILEFINDBUF4),&nm, FIL_QUERYEASIZE);
  if(!rc)
    DosFindClose(hdir);

  /*
   * the "|| strlen(filename) < 4 below works around an OS/2 bug
   * that prevents FAT root directories from being found when
   * requesting EASIZE.  sheesh.
   */
  if((!rc && (ffb->attrFile & FILE_DIRECTORY)) || strlen(filename) < 4)
  {
    if(*stopflag) {
      free(ffb);
      return -1L;
    }
    //printf("CM_ALLOCRECORD\n");
    pciP = WinSendMsg(hwndCnr,CM_ALLOCRECORD,MPFROMLONG(EXTRA_RECORD_BYTES2),
                      MPFROMLONG(1L));
    if(!pciP) {
      free(ffb);
      return -1L;
    }
    if(!rc)
      totalbytes = ffb->cbFile + CBLIST_TO_EASIZE(ffb->cbList);
    else
      DosError(FERR_DISABLEHARDERR);
    pciP->pszLongname = pciP->szFileName;
    pciP->rc.hptrIcon = hptrDir;
    *pciP->szDispAttr = *pciP->Longname = *pciP->subject = 0;
    pciP->attrFile = 0L;
  }
  else
  {
    free(ffb);
    Dos_Error(MB_ENTER,
              rc,
              HWND_DESKTOP,
              __FILE__,
              __LINE__,
              GetPString(IDS_CANTFINDDIRTEXT),
              filename);
    return -1L;
  }

  if(strlen(filename) < 4 || top)
    strcpy(pciP->szFileName,filename);
  else {
    p = strrchr(filename,'\\');
    if(!p)
      p = filename;
    else
      p++;
    sp = (strchr(filename,' ') != NULL) ? "\"" : NullStr;
    pp = pciP->szFileName;
    if(*sp) {
      *pp = *sp;
      pp++;
      *pp = 0;
    }
    strcpy(pp,p);
    if(*sp)
      strcat(pp,sp);
  }
  pciP->pszFileName = pciP->szFileName + strlen(pciP->szFileName);
  pciP->rc.pszIcon = pciP->pszLongname;
  pciP->rc.flRecordAttr |= CRA_RECORDREADONLY;
  if(fForceUpper)
    strupr(pciP->szFileName);
  else if(fForceLower)
    strlwr(pciP->szFileName);
  memset(&ri,0,sizeof(RECORDINSERT));
  ri.cb                 = sizeof(RECORDINSERT);
  ri.pRecordOrder       = (PRECORDCORE)CMA_END;
  ri.pRecordParent      = (PRECORDCORE)pciParent;
  ri.zOrder             = (USHORT)CMA_TOP;
  ri.cRecordsInsert     = 1L;
  ri.fInvalidateRecord  = TRUE;
//printf("CM_INSERTRECORD\n");
  if(!WinSendMsg(hwndCnr,CM_INSERTRECORD,MPFROMP(pciP),MPFROMP(&ri))) {
//printf("Insert failed\n");
    free(ffb);
    return -1L;
  }
  hdir = HDIR_CREATE;
  nm = 1L;
//printf("FIND2\n");
  rc = DosFindFirst(maskstr,&hdir,
                    FILE_NORMAL | FILE_READONLY | FILE_ARCHIVED |
                    FILE_SYSTEM | FILE_HIDDEN | FILE_DIRECTORY,
		    ffb,
                    sizeof(FILEFINDBUF4),
                    &nm,
		    FIL_QUERYEASIZE);
  if(!rc)
  {
    register PBYTE fb = (PBYTE)ffb;
    FILEFINDBUF4  *pffbFile;
    ULONG          x;

    while(!rc)
    {
      priority_normal();
      //printf("Found %lu\n",nm);
      for(x = 0L;x < nm;x++)
      {
        pffbFile = (FILEFINDBUF4 *)fb;
        //printf("%s\n",pffbFile->achName);
        //fflush(stdout);
	// Total size skipping . and ..
        if((*pffbFile->achName != '.' ||
	   (pffbFile->achName[1] && pffbFile->achName[1] != '.')) ||
           !(pffbFile->attrFile & FILE_DIRECTORY))
        {
            totalbytes += pffbFile->cbFile +
                          CBLIST_TO_EASIZE(pffbFile->cbList);
          if(!(pffbFile->attrFile & FILE_DIRECTORY))
            pciP->attrFile++;		// Bump file count
          if(*stopflag)
            break;
          if(pffbFile->attrFile & FILE_DIRECTORY) {
	    // Recurse into subdir
            strcpy(pEndMask,pffbFile->achName);	// Append dirname to base dirname
            if(!*stopflag)
	    {
              temp = ProcessDir(hwndCnr,maskstr,pciP,stopflag,FALSE);
              if(temp != (ULONG)-1L)
                subbytes += temp;
	    }
          }
        }
        if(!pffbFile->oNextEntryOffset)
          break;
        fb += pffbFile->oNextEntryOffset;
      } // for matches
      if(*stopflag)
        break;
      DosSleep(0L);
      nm = 1L;	/* FilesToGet */ 
      rc = DosFindNext(hdir,ffb,sizeof(FILEFINDBUF4) ,&nm);
    } // while more found
    DosFindClose(hdir);
    priority_normal();
  }
  pciP->cbFile = totalbytes;
  pciP->easize = subbytes;
  WinSendMsg(hwndCnr,CM_INVALIDATERECORD,MPFROMP(&pciP),
             MPFROM2SHORT(1,CMA_ERASE | CMA_TEXTCHANGED));
  free(ffb);
  return totalbytes + subbytes;
}


VOID FillInRecSizes (HWND hwndCnr,PCNRITEM pciParent,ULONG totalbytes,
                     CHAR *stopflag,BOOL isroot)
{
  PCNRITEM pci = pciParent;
  SHORT    attrib = CMA_FIRSTCHILD;

  if(pci) {

    CHAR           tf[80],tb[80],tt[80],br[80];
    register CHAR	*p;
    register ULONG	cntr,x;
    float		cntra = 0.0;

    commafmt(tf,sizeof(tf),(pci->cbFile > 0L && pci->cbFile < 1024L) ? 1L :
             pci->cbFile / 1024L);
    *br = 0;
    if(totalbytes) {
      if(isroot) {

        FSALLOCATE fsa;
        APIRET     rc;

        memset(&fsa,0,sizeof(fsa));
        rc = DosQueryFSInfo(toupper(*pci->szFileName) - '@',FSIL_ALLOC,&fsa,
                            sizeof(FSALLOCATE));
        if(!rc)
          cntra = (totalbytes * 100.0) /
                    ((float)fsa.cUnit *	(fsa.cSectorUnit * fsa.cbSector));
        pci->Longname[1] = 1;
      }
      else
        cntra = (((float)pci->cbFile + pci->easize) * 100.0) / totalbytes;
      cntr = (ULONG)cntra / 2;
      p = br;
      for(x = 0;x < cntr;x++) {
        *p = '#';
        p++;
      }
      if(cntr * 2 != (ULONG)cntra) {
        *p = '=';
        p++;
        x++;
      }
      for(;x < 50;x++) {
        *p = ' ';
        p++;
      }
      *p = 0;
    }
    pci->flags = (ULONG)cntra;
    commafmt(tb,sizeof(tb),(pci->easize > 0L && pci->easize < 1024L) ? 1L :
             pci->easize / 1024L);
    commafmt(tt,sizeof(tt),(pci->cbFile + pci->easize > 0L &&
                            pci->cbFile + pci->easize < 1024L) ? 1L :
             (pci->cbFile + pci->easize) / 1024L);
    sprintf(&pci->szFileName[strlen(pci->szFileName)],
            "  %sk + %sk = %sk (%.02lf%%%s)\r%s",
            tf,
            tb,
            tt,
            cntra,
            (isroot) ? GetPString(IDS_OFDRIVETEXT) : NullStr,
            br);
    WinSendMsg(hwndCnr,
               CM_INVALIDATERECORD,
               MPFROMP(&pci),
               MPFROM2SHORT(1,0));
    isroot = FALSE;
  }
  else
    attrib = CMA_FIRST;
  pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(pci),
                             MPFROM2SHORT(attrib,CMA_ITEMORDER));
  while(pci && (INT)pci != -1) {
    if(*stopflag)
      break;
    FillInRecSizes(hwndCnr,pci,totalbytes,stopflag,isroot);
    isroot = FALSE;
    pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(pci),
                               MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
  }
}


static VOID PrintToFile (HWND hwndCnr,ULONG indent,PCNRITEM pciParent,
                         FILE *fp) {

  PCNRITEM       pci;
  register CHAR *p;

  if(!pciParent) {
    pciParent = WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(NULL),
                           MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
    indent = 0;
  }
  if(pciParent) {
    p = strchr(pciParent->szFileName,'\r');
    if(p)
      *p = 0;
    fprintf(fp,"%*.*s%s %lu %s%s\n",
            indent * 2,indent * 2," ",
            pciParent->szFileName,
            pciParent->attrFile,
            GetPString(IDS_FILETEXT),
            &"s"[pciParent->attrFile == 1]);
    if(p)
      *p = '\r';
    if(pciParent->rc.flRecordAttr & CRA_EXPANDED) {
      pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(pciParent),
                                 MPFROM2SHORT(CMA_FIRSTCHILD,CMA_ITEMORDER));
      while(pci && (INT)pci != -1) {
        DosSleep(0L);
        PrintToFile(hwndCnr,indent + 1,pci,fp);
        pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(pci),
                                   MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
      }
    }
  }
}


static VOID FillCnrs (VOID *args)
{
  HAB           hab;
  HMQ           hmq;
  DIRSIZE      *dirsize = (DIRSIZE *)args;
  HWND          hwndCnr;
  ULONG         totalbytes;

  if(!dirsize)
    return;
  hwndCnr = dirsize->hwndCnr;

  DosError(FERR_DISABLEHARDERR);

  // priority_normal();
  hab = WinInitialize(0);
  if(hab) {
    hmq = WinCreateMsgQueue(hab,0);
    if(hmq) {
      WinCancelShutdown(hmq,TRUE);
      totalbytes = ProcessDir(hwndCnr,dirsize->filename,
                              (PCNRITEM)NULL,dirsize->stopflag,TRUE);
      DosPostEventSem(CompactSem);
      WinEnableWindowUpdate(hwndCnr,FALSE);
      FillInRecSizes(hwndCnr,NULL,totalbytes,dirsize->stopflag,TRUE);
      WinEnableWindowUpdate(hwndCnr,TRUE);
      WinSendMsg(hwndCnr,CM_INVALIDATERECORD,MPVOID,
                 MPFROM2SHORT(0,CMA_ERASE | CMA_TEXTCHANGED));
      WinDestroyMsgQueue(hmq);
    }
    WinTerminate(hab);
  }
  PostMsg(WinQueryWindow(hwndCnr,QW_PARENT),
          UM_CONTAINER_FILLED,
          MPVOID,MPVOID);
  free(dirsize);
}


MRESULT EXPENTRY DirSizeProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  TEMP *data;

  switch(msg) {
    case WM_INITDLG:
      if(!mp2) {
        WinDismissDlg(hwnd,0);
        break;
      }
      data = malloc(sizeof(TEMP));
      if(!data) {
        WinDismissDlg(hwnd,0);
        break;
      }
      memset(data,0,sizeof(TEMP));
      strcpy(data->dirname,(CHAR *)mp2);
      WinSetWindowPtr(hwnd,0,(PVOID)data);
      data->hptr = WinLoadPointer(HWND_DESKTOP,FM3ModHandle,DIRSIZE_ICON);
      WinDefDlgProc(hwnd,WM_SETICON,MPFROMLONG(data->hptr),MPVOID);
      {
        CHAR s[CCHMAXPATH + 81];

        sprintf(s,
                GetPString(IDS_DIRSIZETITLETEXT),
                data->dirname);
        WinSetWindowText(hwnd,s);
      }
      {
        DIRSIZE *dirsize;

        dirsize = malloc(sizeof(DIRSIZE));
        if(!dirsize) {
          WinDismissDlg(hwnd,0);
          break;
        }
        dirsize->stopflag = (CHAR *)&data->stopflag;
        dirsize->filename = data->dirname;
        dirsize->hwndCnr = WinWindowFromID(hwnd,DSZ_CNR);
        if(_beginthread(FillCnrs,NULL,122880L * 5L,(PVOID)dirsize) == -1) {
          free(dirsize);
          WinDismissDlg(hwnd,0);
          break;
        }
        data->working = TRUE;
      }
      PostMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
      break;

    case UM_SETUP:
      {
        CNRINFO    cnri;
        FSALLOCATE fsa;
        APIRET     rc;

        memset(&cnri,0,sizeof(CNRINFO));
        cnri.cb = sizeof(CNRINFO);
        WinSendDlgItemMsg(hwnd,DSZ_CNR,CM_QUERYCNRINFO,
                          MPFROMP(&cnri),MPFROMLONG(sizeof(CNRINFO)));
        cnri.cyLineSpacing = 0;
        cnri.cxTreeIndent = 12L;
        cnri.flWindowAttr = CV_TREE | CV_FLOW | CA_TREELINE | CA_OWNERDRAW;
        WinSendDlgItemMsg(hwnd,DSZ_CNR,CM_SETCNRINFO,MPFROMP(&cnri),
                          MPFROMLONG(CMA_FLWINDOWATTR | CMA_TREEICON |
                                     CMA_LINESPACING | CMA_CXTREEINDENT));
        data = INSTDATA(hwnd);
        if(data && isalpha(*data->dirname)) {
          memset(&fsa,0,sizeof(fsa));
          rc = DosQueryFSInfo(toupper(*data->dirname) - '@',FSIL_ALLOC,&fsa,
                              sizeof(FSALLOCATE));
          if(!rc) {

            CHAR s[132],tf[80],tb[80],tu[80];

            commafmt(tf,sizeof(tf),
	             (ULONG)(((float)fsa.cUnitAvail *
		       (fsa.cSectorUnit * fsa.cbSector)) / 1024L));
            commafmt(tb,sizeof(tb),
	             (ULONG)(((float)fsa.cUnit *
		       (fsa.cSectorUnit * fsa.cbSector)) / 1024L));
            commafmt(tu,sizeof(tu),
	             (ULONG)(((float)(fsa.cUnit - fsa.cUnitAvail) *
                       (fsa.cSectorUnit * fsa.cbSector)) / 1024L));
            sprintf(s,
                    GetPString(IDS_FREESPACETEXT),
                    tf,
                    tb,
                    tu);
            WinSetDlgItemText(hwnd,
                              DSZ_FREESPACE,
                              s);
          }
          else
            WinSetDlgItemText(hwnd,
                              DSZ_FREESPACE,
                              GetPString(IDS_FREESPACEUTEXT));
        }
      }
      return 0;

    case UM_CONTAINER_FILLED:
      data = INSTDATA(hwnd);
      if(!data || data->dying) {
        data->working = FALSE;
        WinDismissDlg(hwnd,0);
        return 0;
      }
      data->working = FALSE;
      {
        CHAR     tb[44],s[66];
        PCNRITEM pci;

        pci = WinSendDlgItemMsg(hwnd,DSZ_CNR,CM_QUERYRECORD,MPVOID,
                                MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
        if(pci && (INT)pci != -1)
          WinSendDlgItemMsg(hwnd,DSZ_CNR,CM_EXPANDTREE,MPFROMP(pci),MPVOID);
        *s = 0;
        pci = WinSendDlgItemMsg(hwnd,DSZ_CNR,CM_QUERYRECORDEMPHASIS,
                                MPFROMLONG(CMA_FIRST),
                                MPFROMSHORT(CRA_CURSORED));
        if(pci && (INT)pci != -1) {
          commafmt(tb,sizeof(tb),pci->attrFile);
          sprintf(s,
                  "%s %s%s",
                  tb,
                  GetPString(IDS_FILETEXT),
                  &"s"[pci->attrFile == 1]);
        }
        WinSetDlgItemText(hwnd,DSZ_NUMFILES,s);
      }
      WinSendDlgItemMsg(hwnd,DSZ_CNR,CM_SORTRECORD,MPFROMP(SortSizeCnr),
                        MPVOID);
      DosBeep(500,25);
      return 0;

    case WM_ADJUSTWINDOWPOS:
      PostMsg(hwnd,UM_STRETCH,MPVOID,MPVOID);
      break;

    case UM_STRETCH:
      {
        SWP swpC,swp;

        WinQueryWindowPos(hwnd,&swp);
        if(!(swp.fl & (SWP_HIDE | SWP_MINIMIZE))) {
          WinQueryWindowPos(WinWindowFromID(hwnd,DSZ_CNR),&swpC);
          WinSetWindowPos(WinWindowFromID(hwnd,DSZ_CNR),HWND_TOP,
                          SysVal(SV_CXSIZEBORDER),
                          swpC.y,
                          swp.cx - (SysVal(SV_CXSIZEBORDER) * 2),
                          (swp.cy - swpC.y) - (SysVal(SV_CYTITLEBAR) +
                                               SysVal(SV_CYSIZEBORDER)),
                          SWP_MOVE | SWP_SIZE);
        }
      }
      return 0;

    case WM_DRAWITEM:
      if(mp2) {

        OWNERITEM       *oi = mp2;
        CNRDRAWITEMINFO *cnd;
        PCNRITEM         pci;

        if(oi->idItem == CMA_TEXT) {
          cnd = (CNRDRAWITEMINFO *)oi->hItem;
          if(cnd) {
            pci = (PCNRITEM)cnd->pRecord;
            if(pci) {

              POINTL aptl[TXTBOX_COUNT],ptl;
              CHAR  *p;
              LONG   clr,x;

              p = strchr(pci->szFileName,'\r');
              if(p) {
                /* draw text */
                if(!pci->cbFile)  /* no size */
                  GpiSetColor(oi->hps,CLR_DARKGRAY);
                else if(!pci->easize) /* no size below */
                  GpiSetColor(oi->hps,CLR_DARKBLUE);
                else
                  GpiSetColor(oi->hps,CLR_BLACK);
                GpiSetBackMix(oi->hps,BM_LEAVEALONE);
                GpiSetMix(oi->hps,FM_OVERPAINT);
                *p = 0;
                GpiQueryTextBox(oi->hps,strlen(pci->szFileName),
                                pci->szFileName,TXTBOX_COUNT,aptl);
                ptl.x = oi->rclItem.xLeft;
                ptl.y = (oi->rclItem.yTop - aptl[TXTBOX_TOPRIGHT].y);
                GpiMove(oi->hps,&ptl);
                GpiCharString(oi->hps,strlen(pci->szFileName),
                              pci->szFileName);
                *p = '\r';

                /* draw the graph box */
                GpiQueryTextBox(oi->hps,1,"#",TXTBOX_COUNT,aptl);
                /* draw black outline */
                GpiSetColor(oi->hps,CLR_BLACK);
                ptl.x = oi->rclItem.xLeft;
                ptl.y = oi->rclItem.yBottom + 2;
                GpiMove(oi->hps,&ptl);
                ptl.x = oi->rclItem.xLeft + 101;
                ptl.y = (oi->rclItem.yBottom + aptl[TXTBOX_TOPRIGHT].y);
                GpiBox(oi->hps,DRO_OUTLINE,&ptl,0,0);
                /* fill with gray */
                GpiSetColor(oi->hps,CLR_PALEGRAY);
                ptl.x = oi->rclItem.xLeft + 1;
                ptl.y = oi->rclItem.yBottom + 3;
                GpiMove(oi->hps,&ptl);
                ptl.x = oi->rclItem.xLeft + 100;
                ptl.y = (oi->rclItem.yBottom + aptl[TXTBOX_TOPRIGHT].y) - 1;
                GpiBox(oi->hps,DRO_OUTLINEFILL,&ptl,0,0);

                /* draw shadow at bottom & right sides */
                GpiSetColor(oi->hps,CLR_DARKGRAY);
                ptl.x = oi->rclItem.xLeft + 1;
                ptl.y = oi->rclItem.yBottom + 3;
                GpiMove(oi->hps,&ptl);
                ptl.x = oi->rclItem.xLeft + 100;
                GpiLine(oi->hps,&ptl);
                ptl.y = (oi->rclItem.yBottom + aptl[TXTBOX_TOPRIGHT].y) - 1;
                GpiLine(oi->hps,&ptl);

                /* draw highlight at top and left sides */
                GpiSetColor(oi->hps,CLR_WHITE);
                ptl.x = oi->rclItem.xLeft + 1;
                GpiLine(oi->hps,&ptl);
                ptl.y = oi->rclItem.yBottom + 3;
                GpiLine(oi->hps,&ptl);

                /* draw shadow of box */
                GpiSetColor(oi->hps,CLR_DARKGRAY);
                ptl.x = oi->rclItem.xLeft + 2;
                ptl.y = oi->rclItem.yBottom;
                GpiMove(oi->hps,&ptl);
                ptl.x = oi->rclItem.xLeft + 103;
                GpiLine(oi->hps,&ptl);
                ptl.y = (oi->rclItem.yBottom + aptl[TXTBOX_TOPRIGHT].y) - 2;
                GpiLine(oi->hps,&ptl);
                ptl.x--;
                GpiMove(oi->hps,&ptl);
                ptl.y = oi->rclItem.yBottom + 1;
                GpiLine(oi->hps,&ptl);
                ptl.x = oi->rclItem.xLeft + 2;
                GpiLine(oi->hps,&ptl);

                /* fill box with graph bar */
                if(pci->flags) {
                  if(pci->Longname[1] == 1) /* is root record */
                    GpiSetColor(oi->hps,CLR_DARKGREEN);
                  else
                    GpiSetColor(oi->hps,CLR_RED);
                  ptl.x = oi->rclItem.xLeft + 1;
                  ptl.y = oi->rclItem.yBottom + 3;
                  GpiMove(oi->hps,&ptl);
                  ptl.x = oi->rclItem.xLeft + pci->flags;
                  ptl.y = (oi->rclItem.yBottom + aptl[TXTBOX_TOPRIGHT].y) - 1;
                  GpiBox(oi->hps,DRO_OUTLINEFILL,&ptl,0,0);

                  /* draw highlights and shadows on graph */
                  if(pci->Longname[1] == 1)
                    GpiSetColor(oi->hps,CLR_GREEN);
                  else
                    GpiSetColor(oi->hps,CLR_PALEGRAY);
                  if(pci->flags > 5) {
                    ptl.x = oi->rclItem.xLeft + 1;
                    ptl.y = oi->rclItem.yBottom + 3;
                    GpiMove(oi->hps,&ptl);
                    ptl.y = (oi->rclItem.yBottom + aptl[TXTBOX_TOPRIGHT].y) - 1;
                    GpiLine(oi->hps,&ptl);
                  }
                  else {
                    ptl.y = (oi->rclItem.yBottom + aptl[TXTBOX_TOPRIGHT].y) - 1;
                    GpiMove(oi->hps,&ptl);
                  }
                  ptl.x = oi->rclItem.xLeft + pci->flags;
                  GpiLine(oi->hps,&ptl);
                  if(pci->Longname[1] != 1) {
                    GpiSetColor(oi->hps,CLR_DARKRED);
                    ptl.x = oi->rclItem.xLeft + 2;
                    ptl.y = oi->rclItem.yBottom + 3;
                    GpiMove(oi->hps,&ptl);
                    ptl.x = oi->rclItem.xLeft + pci->flags;
                    GpiLine(oi->hps,&ptl);
                  }
                }

                /* draw hash marks in box */
                GpiSetColor(oi->hps,CLR_WHITE);
                clr = CLR_WHITE;
                for(x = 1;x < 10;x++) {
                  if(clr == CLR_WHITE && x * 10 > pci->flags) {
                    clr = CLR_BLACK;
                    GpiSetColor(oi->hps,CLR_BLACK);
                  }
                  ptl.x = (oi->rclItem.xLeft + 1) + (x * 10);
                  ptl.y = oi->rclItem.yBottom + aptl[TXTBOX_TOPRIGHT].y - 1;
                  GpiMove(oi->hps,&ptl);
                  switch(x) {
                    case 1:
                    case 3:
                    case 7:
                    case 9:
                      ptl.y -= 1;
                      break;
                    case 5:
                      ptl.y -= 4;
                      break;
                    case 2:
                    case 4:
                    case 6:
                    case 8:
                      ptl.y -= 2;
                      break;
                  }
                  GpiLine(oi->hps,&ptl);
                }
                return MRFROMLONG(TRUE);
              }
            }
          }
        }
      }
      return FALSE;

    case WM_CONTROL:
      switch(SHORT2FROMMP(mp1)) {
        case CN_ENTER:
          if(mp2) {

            PCNRITEM     pci = (PCNRITEM)((PNOTIFYRECORDENTER)mp2)->pRecord;
            CHAR         filename[CCHMAXPATH],temp[CCHMAXPATH];

            if(pci) {
              *filename = 0;
              while(pci && (INT)pci != -1) {
                memset(temp,0,sizeof(temp));
                strncpy(temp,pci->szFileName,
                        pci->pszFileName - pci->szFileName);
                strrev(temp);
                if(*filename && *temp != '\\')
                  strcat(filename,"\\");
                strcat(filename,temp);
                pci = WinSendDlgItemMsg(hwnd,DSZ_CNR,CM_QUERYRECORD,
                                        MPFROMP(pci),
                                        MPFROM2SHORT(CMA_PARENT,
                                                     CMA_ITEMORDER));
              }
              strrev(filename);
              if(!fVTreeOpensWPS)
                OpenDirCnr((HWND)0,
                           (hwndMain) ? hwndMain : HWND_DESKTOP,
                           hwnd,
                           FALSE,
                           filename);
              else {

                ULONG size = sizeof(ULONG),flWindowAttr = CV_ICON;
                CHAR  s[33];

                strcpy(s,"ICON");
                PrfQueryProfileData(fmprof,appname,"DirflWindowAttr",
                                    (PVOID)&flWindowAttr,&size);
                if(flWindowAttr & CV_DETAIL) {
                  if(IsRoot(filename))
                    strcpy(s,"TREE");
                  else
                    strcpy(s,"DETAILS");
                }
                OpenObject(filename,s,hwnd);
              }
            }
          }
          break;
        case CN_EMPHASIS:
          data = INSTDATA(hwnd);
          if(data && !data->working && mp2) {

            PNOTIFYRECORDEMPHASIS pre = mp2;
            PCNRITEM              pci;
            CHAR                  tb[44],s[66];

            pci = (PCNRITEM)((pre) ? pre->pRecord : NULL);
            if(pci && (pre->fEmphasisMask & CRA_SELECTED) &&
               (pci->rc.flRecordAttr & CRA_SELECTED)) {
              commafmt(tb,sizeof(tb),pci->attrFile);
              sprintf(s,
                      "%s %s%s",
                      tb,
                      GetPString(IDS_FILETEXT),
                      &"s"[pci->attrFile == 1]);
              WinSetDlgItemText(hwnd,
                                DSZ_NUMFILES,
                                s);
            }
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_DIRSIZE,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case DSZ_PRINT:
          data = INSTDATA(hwnd);
          if(data && !data->working) {

            CHAR  filename[CCHMAXPATH];
            FILE *fp;

            save_dir2(filename);
            sprintf(&filename[strlen(filename)],"\\%csizes.Rpt",
                    (data) ? toupper(*data->dirname) : '+');
            if(export_filename(hwnd,filename,FALSE) && *filename) {
              if(stricmp(filename,"PRN") &&
                 strnicmp(filename,"\\DEV\\LPT",8) &&
                 !strchr(filename,'.'))
                strcat(filename,".RPT");
              fp = fopen(filename,"a+");
              if(fp) {
                WinSetPointer(HWND_DESKTOP,hptrBusy);
                PrintToFile(WinWindowFromID(hwnd,DSZ_CNR),0,NULL,fp);
                fclose(fp);
                WinSetPointer(HWND_DESKTOP,hptrArrow);
              }
              else
                saymsg(MB_CANCEL,
                       hwnd,
                       GetPString(IDS_ERRORTEXT),
                       GetPString(IDS_COMPCANTOPENTEXT),
                       filename);
            }
          }
          else
            DosBeep(50,100);
          break;

        case DSZ_EXPAND:
        case DSZ_COLLAPSE:
          {
            PCNRITEM pci;

            pci = (PCNRITEM)WinSendDlgItemMsg(hwnd,DSZ_CNR,
                                              CM_QUERYRECORDEMPHASIS,
                                              MPFROMLONG(CMA_FIRST),
                                              MPFROMSHORT(CRA_CURSORED));
            if(pci)
              ExpandAll(WinWindowFromID(hwnd,DSZ_CNR),
                        (SHORT1FROMMP(mp1) == DSZ_EXPAND),pci);
          }
          break;

        case DID_OK:
        case DID_CANCEL:
          data = INSTDATA(hwnd);
          if(data) {
            if(data->working) {
              data->dying = TRUE;
              data->stopflag = 0xff;
              DosBeep(1000,100);
            }
            else
              WinDismissDlg(hwnd,0);
          }
          break;
      }
      return 0;

    case WM_CLOSE:
      data = INSTDATA(hwnd);
      if(data)
        data->stopflag = 0xff;
      DosSleep(1L);
      break;

    case WM_DESTROY:
      data = INSTDATA(hwnd);
      if(data) {
        data->stopflag = 0xff;
        if(data->hptr)
          WinDestroyPointer(data->hptr);
        DosSleep(33L);
        free(data);
      }
      DosPostEventSem(CompactSem);
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}
