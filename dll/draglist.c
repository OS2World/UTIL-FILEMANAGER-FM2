#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fm3dll.h"

#pragma alloc_text(DRAGLIST,DragOne,DoFileDrag,DragList,PickUp)


HWND DragOne (HWND hwndCnr,HWND hwndObj,CHAR *filename,BOOL moveok) {

  DRAGITEM    DItem;
  HWND        hDrop = 0;
  DRAGIMAGE   fakeicon;
  PDRAGINFO   pDInfo;
  FILESTATUS3 fs3;
  CHAR        szDir[CCHMAXPATH],szFile[CCHMAXPATH],*p;

  if(filename && *filename) {
    if((IsRoot(filename) && IsValidDrive(*filename)) ||
       !DosQueryPathInfo(filename,FIL_STANDARD,&fs3,sizeof(fs3))) {
      strcpy(szDir,filename);
      p = szDir;
      while(*p) {
        if(*p == '/')
          *p = '\\';
        p++;
      }
      p = strrchr(szDir,'\\');
      if(p) {
        *p = 0;
        p++;
        strcpy(szFile,p);
        strcat(szDir,"\\");
      }
      else {
        strcpy(szFile,filename);
        *szDir = 0;
      }
      memset(&fakeicon,0,sizeof(DRAGIMAGE));
      fakeicon.hImage = (IsRoot(filename) ||
                         (fs3.attrFile & FILE_DIRECTORY) != 0) ?
                          hptrDir : hptrFile;
      memset(&DItem,0,sizeof(DRAGITEM));
      DItem.hwndItem = (hwndObj) ? hwndObj : hwndCnr; /* Initialize DRAGITEM */
//      DItem.hwndItem = hwndCnr;
      DItem.ulItemID = 1;
      DItem.hstrType = DrgAddStrHandle(DRT_UNKNOWN);
      DItem.hstrRMF =
         DrgAddStrHandle(DRMDRFLIST);
      DItem.hstrContainerName = DrgAddStrHandle(szDir);
      DItem.hstrSourceName = DrgAddStrHandle(szFile);
      DItem.hstrTargetName = DrgAddStrHandle(szFile);
      DItem.fsControl = 0;
      if(IsRoot(filename) || (fs3.attrFile & FILE_DIRECTORY) != 0)
        DItem.fsControl |= DC_CONTAINER;
      if(IsFullName(filename) &&
         (driveflags[toupper(*filename) - 'A'] & DRIVE_REMOVABLE))
        DItem.fsControl |= DC_REMOVEABLEMEDIA;
      DItem.fsSupportedOps = DO_COPYABLE | DO_LINKABLE;
      if(moveok && IsFullName(filename) &&
         !(driveflags[toupper(*filename) - 'A'] & DRIVE_NOTWRITEABLE))
        DItem.fsSupportedOps |= DO_MOVEABLE;
      if(IsRoot(filename))
        DItem.fsSupportedOps = DO_LINKABLE;
      fakeicon.cb = sizeof(DRAGIMAGE);
      fakeicon.cptl = 0;
      fakeicon.fl = DRG_ICON;
      fakeicon.sizlStretch.cx = 32;
      fakeicon.sizlStretch.cy = 32;
      fakeicon.cxOffset = -16;
      fakeicon.cyOffset = 0;
      pDInfo = DrgAllocDraginfo(1);     /* Allocate DRAGINFO */
      if(pDInfo) {
        if(IsFullName(filename) &&
           (driveflags[toupper(*filename) - 'A'] & DRIVE_NOTWRITEABLE))
          pDInfo->usOperation = DO_COPY;
        else
          pDInfo->usOperation = DO_DEFAULT;
        if(IsRoot(filename))
          pDInfo->usOperation = DO_LINK;
        pDInfo->hwndSource = (hwndObj) ? hwndObj : hwndCnr;
//        pDInfo->hwndSource = hwndCnr;
        DrgSetDragitem(pDInfo,            /* Set item in DRAGINFO  */
                       &DItem,            /* Pointer to DRAGITEM   */
                       sizeof(DRAGITEM),  /* Size of DRAGITEM      */
                       0);                /* Index of DRAGITEM     */
        WinSetFocus(HWND_DESKTOP,HWND_DESKTOP);
        hDrop = DrgDrag(hwndCnr,              /* Initiate drag         */
                        pDInfo,               /* DRAGINFO structure    */
                        &fakeicon,
                        1L,
                        VK_ENDDRAG,           /* End of drag indicator */
                        (PVOID)NULL);         /* Reserved              */

        DrgFreeDraginfo(pDInfo);              /* Free DRAGINFO struct  */
        WinSetWindowPos(hwndCnr,HWND_TOP,0,0,0,0,SWP_ACTIVATE);
      }
    }
  }
  return hDrop;
}


HWND DoFileDrag (HWND hwndCnr,HWND hwndObj,PCNRDRAGINIT pcd,CHAR *arcfile,
                 CHAR *directory,BOOL moveok) {

  /* drag files from a container */

  BOOL           isdir,rooting = FALSE;
  PCNRITEM       pci;
  register CHAR *p;
  INT            attribute = CRA_CURSORED;
  PDRAGINFO      pDInfo = NULL;
  DRAGITEM     **DItem = NULL,**test;
  PCNRITEM       pcs = (PCNRITEM)pcd->pRecord;
  HWND           hDrop = 0;
  register ULONG ulNumfiles = 0L,numdragalloc = 0L,Select,ulNumIcon = 0;
  CHAR           szFile[CCHMAXPATH],szBuffer[CCHMAXPATH];
  DRAGIMAGE     *icon = NULL,*itest,fakeicon;

  if(!pcs && directory && *directory)
    return DragOne(hwndCnr,hwndObj,directory,moveok);

  if(!pcs) {
    pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,
                               MPFROMLONG(CMA_FIRST),
                               MPFROMSHORT(attribute));
    if(pci && (INT)pci > -1) {
      if(pci->rc.flRecordAttr & CRA_SELECTED) {
        attribute = CRA_SELECTED;
        pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMLONG(CMA_FIRST),
                         MPFROMSHORT(attribute));
      }
    }
  }
  else {
    pci = pcs;
    attribute = (pci->rc.flRecordAttr & CRA_SELECTED) ? CRA_SELECTED : 0;
    if(attribute) {
      pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMLONG(CMA_FIRST),
                       MPFROMSHORT(attribute));
    }
  }

  Select = 0L;
  while( pci && (INT)pci > -1) {
    if(!(pci->rc.flRecordAttr & CRA_FILTERED)) {
      if(IsRoot(pci->szFileName) && !IsValidDrive(*pci->szFileName))
        goto Continuing;
      if(!arcfile) {
        strcpy(szBuffer,pci->szFileName);
        p = strrchr(szBuffer,'\\');
        if(p) {
          p++;
          strcpy(szFile,p);
          *p = 0;
        }
        else
          goto Continuing;
      }
      else
        strcpy(szFile,pci->szFileName);
    }
    if(!arcfile) {
      isdir = ((pci->attrFile & FILE_DIRECTORY) != 0);
      if(ulNumfiles + 2L > numdragalloc) {
        if(!icon) {
          itest = realloc(icon,sizeof(DRAGIMAGE) * (numdragalloc + 4L));
          if(itest)
            icon = itest;
          else
            break;
        }
        else if(!ulNumIcon) {
          icon[ulNumfiles].cb = sizeof(DRAGIMAGE);
          icon[ulNumfiles].cptl = 0;
          icon[ulNumfiles].hImage = hptrLast;
          icon[ulNumfiles].fl = DRG_ICON;
          icon[ulNumfiles].sizlStretch.cx = 32;
          icon[ulNumfiles].sizlStretch.cy = 32;
          icon[ulNumfiles].cxOffset = -16 + (ulNumfiles * 4);
          icon[ulNumfiles].cyOffset = 0 + (ulNumfiles * 7);
          ulNumIcon = ulNumfiles + 1;
        }
        test = realloc(DItem,sizeof(DRAGITEM *) * (numdragalloc + 4L));
        if(test) {
          DItem = test;
          numdragalloc += 4L;
        }
        else
          break;
      }
      DItem[ulNumfiles] = malloc(sizeof(DRAGITEM));
      if(DItem[ulNumfiles]) {
        if(!ulNumIcon) {
          icon[ulNumfiles].cb = sizeof(DRAGIMAGE);
          icon[ulNumfiles].cptl = 0;
          icon[ulNumfiles].hImage = pci->rc.hptrIcon;
          if(!icon[ulNumfiles].hImage)
            icon[ulNumfiles].hImage = (isdir) ? hptrDir : hptrFile;
          icon[ulNumfiles].fl = DRG_ICON;
          icon[ulNumfiles].sizlStretch.cx = 32;
          icon[ulNumfiles].sizlStretch.cy = 32;
          icon[ulNumfiles].cxOffset = -16 + (ulNumfiles * 3);
          icon[ulNumfiles].cyOffset = 0 + (ulNumfiles * 6);
        }
        memset(DItem[ulNumfiles],0,sizeof(DRAGITEM));
        DItem[ulNumfiles]->hwndItem = (hwndObj) ? hwndObj : hwndCnr; /* Initialize DRAGITEM   */
          DItem[ulNumfiles]->hwndItem = hwndCnr;
        DItem[ulNumfiles]->ulItemID = (ULONG)pci;
        DItem[ulNumfiles]->hstrType = DrgAddStrHandle(DRT_UNKNOWN);
        DItem[ulNumfiles]->hstrRMF =
           DrgAddStrHandle(DRMDRFLIST);
        DItem[ulNumfiles]->hstrContainerName = DrgAddStrHandle(szBuffer);
        DItem[ulNumfiles]->hstrSourceName = DrgAddStrHandle(szFile);
        DItem[ulNumfiles]->hstrTargetName = DrgAddStrHandle(szFile);
        DItem[ulNumfiles]->fsControl = (isdir) ? DC_CONTAINER : 0;
        if(IsFullName(pci->szFileName) &&
           (driveflags[toupper(*pci->szFileName) - 'A'] & DRIVE_REMOVABLE))
          DItem[ulNumfiles]->fsControl |= DC_REMOVEABLEMEDIA;
        DItem[ulNumfiles]->fsSupportedOps = DO_COPYABLE | DO_LINKABLE;
        if(moveok && IsFullName(pci->szFileName) &&
           !(driveflags[toupper(*pci->szFileName) - 'A'] & DRIVE_NOTWRITEABLE))
          DItem[ulNumfiles]->fsSupportedOps |= DO_MOVEABLE;
        if(IsRoot(pci->szFileName)) {
          DItem[ulNumfiles]->fsSupportedOps = DO_LINKABLE;
          rooting = TRUE;
        }
        ulNumfiles++;
        DItem[ulNumfiles] = NULL;
      }
      else
        break;
    }
    else {
      if(ulNumfiles + 3L > numdragalloc) {
        test = realloc(DItem,sizeof(DRAGITEM *) * (numdragalloc + 5L));
        if(test) {
          DItem = test;
          numdragalloc += 5L;
        }
        else
          break;
      }
      DItem[ulNumfiles] = malloc(sizeof(DRAGITEM));
      if(DItem[ulNumfiles]) {
        fakeicon.hImage = hptrFile;
        memset(DItem[ulNumfiles],0,sizeof(DRAGITEM));
        DItem[ulNumfiles]->hwndItem = (hwndObj) ? hwndObj : hwndCnr; /* Initialize DRAGITEM   */
          DItem[ulNumfiles]->hwndItem = hwndCnr;
        DItem[ulNumfiles]->ulItemID = (ULONG)pci;
        DItem[ulNumfiles]->hstrType = DrgAddStrHandle(DRT_UNKNOWN);
        DItem[ulNumfiles]->hstrRMF =
           DrgAddStrHandle(DRMDRFOS2FILE);
        DItem[ulNumfiles]->hstrContainerName = DrgAddStrHandle(arcfile);
        DItem[ulNumfiles]->hstrSourceName = DrgAddStrHandle(szFile);
        DItem[ulNumfiles]->hstrTargetName = DrgAddStrHandle(szFile);
        DItem[ulNumfiles]->fsControl = DC_PREPARE;
        if(IsFullName(arcfile) &&
           (driveflags[toupper(*arcfile) - 'A'] & DRIVE_REMOVABLE))
          DItem[ulNumfiles]->fsControl |= DC_REMOVEABLEMEDIA;
        DItem[ulNumfiles]->fsSupportedOps = DO_COPYABLE;
        ulNumfiles++;
        DItem[ulNumfiles] = malloc(sizeof(DRAGITEM));
        if(DItem[ulNumfiles]) {
          fakeicon.hImage = hptrFile;
          memset(DItem[ulNumfiles],0,sizeof(DRAGITEM));
          DItem[ulNumfiles]->hwndItem = (hwndObj) ? hwndObj : hwndCnr; /* Initialize DRAGITEM   */
            DItem[ulNumfiles]->hwndItem = hwndCnr;
          DItem[ulNumfiles]->ulItemID = Select++;
          DItem[ulNumfiles]->hstrType = DrgAddStrHandle(DRT_UNKNOWN);
          DItem[ulNumfiles]->hstrRMF =
             DrgAddStrHandle(DRMDRFFM2ARC);
          DItem[ulNumfiles]->hstrContainerName = DrgAddStrHandle(arcfile);
          DItem[ulNumfiles]->hstrSourceName = DrgAddStrHandle(szFile);
          DItem[ulNumfiles]->hstrTargetName = DrgAddStrHandle(szFile);
          DItem[ulNumfiles]->fsControl = 0;
          if(IsFullName(arcfile) &&
             (driveflags[toupper(*arcfile) - 'A'] & DRIVE_REMOVABLE))
            DItem[ulNumfiles]->fsControl |= DC_REMOVEABLEMEDIA;
          DItem[ulNumfiles]->fsSupportedOps = DO_COPYABLE;
          ulNumfiles++;
        }
        DItem[ulNumfiles] = NULL;
      }
      else
        break;
    }
    WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                   MPFROM2SHORT(TRUE,CRA_SOURCE));

Continuing:

    if(!attribute)
      break;
    pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMP(pci),
                     MPFROMSHORT(attribute));
  }

  if(ulNumfiles) {
    pDInfo = DrgAllocDraginfo(ulNumfiles);  /* Allocate DRAGINFO */
    if(pDInfo) {
      if((arcfile && *arcfile) || (IsFullName(szBuffer) &&
         (driveflags[toupper(*szBuffer) - 'A'] & DRIVE_NOTWRITEABLE)))
        pDInfo->usOperation = DO_COPY;
      else
        pDInfo->usOperation = DO_DEFAULT;
      if((!arcfile || !*arcfile) && rooting)
        pDInfo->usOperation = DO_LINK;
      pDInfo->hwndSource = (hwndObj) ? hwndObj : hwndCnr;
//      pDInfo->hwndSource = hwndCnr;
      for(Select = 0L;Select < ulNumfiles;Select++) {
        DrgSetDragitem(pDInfo,            /* Set item in DRAGINFO  */
                       DItem[Select],     /* Pointer to DRAGITEM   */
                       sizeof(DRAGITEM),  /* Size of DRAGITEM      */
                       Select);           /* Index of DRAGITEM     */
        free(DItem[Select]);
      }
#ifdef __DEBUG_ALLOC__
      _heap_check();
#endif
      free(DItem);
      DItem = NULL;
      DosPostEventSem(CompactSem);

      if(arcfile) {
        fakeicon.cb = sizeof(DRAGIMAGE);
        fakeicon.cptl = 0;
        if(ulNumfiles > 1)
          fakeicon.hImage = hptrFile;
        fakeicon.fl = DRG_ICON;
        fakeicon.sizlStretch.cx = 32;
        fakeicon.sizlStretch.cy = 32;
        fakeicon.cxOffset = -16;
        fakeicon.cyOffset = 0;
        icon = &fakeicon;
      }
      if(!arcfile) {
        if(!ulNumIcon)
          ulNumIcon = ulNumfiles;
      }
      else
        ulNumIcon = 1L;

      WinSetFocus(HWND_DESKTOP,HWND_DESKTOP);
      hDrop = DrgDrag(hwndCnr,              /* Initiate drag         */
                      pDInfo,               /* DRAGINFO structure    */
                      icon,
                      ulNumIcon,
                      VK_ENDDRAG,           /* End of drag indicator */
                      (PVOID)NULL);         /* Reserved              */

      DrgFreeDraginfo(pDInfo);              /* Free DRAGINFO struct  */
      free(icon);
      icon = NULL;
      WinSetWindowPos(hwndCnr,HWND_TOP,0,0,0,0,SWP_ACTIVATE);
      DosPostEventSem(CompactSem);
    }
  }
  if(DItem)
    free(DItem);
  if(icon)
    free(icon);
  MarkAll(hwndCnr,TRUE,FALSE,TRUE);
  return hDrop;
}


HWND DragList (HWND hwnd,HWND hwndObj,CHAR **list,BOOL moveok) {

  /* drag a linked list of files */

  BOOL           isdir;
  register CHAR *p;
  PDRAGINFO      pDInfo = NULL;
  DRAGITEM     **DItem = NULL,**test;
  HWND           hDrop = (HWND)0;
  register ULONG ulNumfiles = 0L,numdragalloc = 0L,Select,ulNumIcon = 0;
  CHAR           szFile[CCHMAXPATH],szBuffer[CCHMAXPATH];
  DRAGIMAGE     *icon = NULL,*itest;
  FILESTATUS3    fs3;

  if(!list || !list[0])
    return hDrop;
  for(Select = 0;list[Select];Select++) {
    if((!IsRoot(list[Select]) || !IsValidDrive(*list[Select])) &&
       DosQueryPathInfo(list[Select],FIL_STANDARD,&fs3,sizeof(fs3)))
      continue;
    strcpy(szBuffer,list[Select]);
    p = strrchr(szBuffer,'\\');
    if(p) {
      p++;
      strcpy(szFile,p);
      *p = 0;
    }
    else
      continue;
    if(*szFile) {
      isdir = (IsRoot(list[Select])) ? TRUE :
               ((fs3.attrFile & FILE_DIRECTORY) != 0);
      if(ulNumfiles + 2L > numdragalloc) {
        if(!icon) {
          itest = realloc(icon,sizeof(DRAGIMAGE) * (numdragalloc + 4L));
          if(itest)
            icon = itest;
          else
            break;
        }
        else if(!ulNumIcon) {
          icon[ulNumfiles].cb = sizeof(DRAGIMAGE);
          icon[ulNumfiles].cptl = 0;
          icon[ulNumfiles].hImage = hptrLast;
          icon[ulNumfiles].fl = DRG_ICON;
          icon[ulNumfiles].sizlStretch.cx = 32;
          icon[ulNumfiles].sizlStretch.cy = 32;
          icon[ulNumfiles].cxOffset = -16 + (ulNumfiles * 4);
          icon[ulNumfiles].cyOffset = 0 + (ulNumfiles * 7);
          ulNumIcon = ulNumfiles + 1;
        }
        test = realloc(DItem,sizeof(DRAGITEM *) * (numdragalloc + 4L));
        if(test) {
          DItem = test;
          numdragalloc += 4L;
        }
        else
          break;
      }
      DItem[ulNumfiles] = malloc(sizeof(DRAGITEM));
      if(DItem[ulNumfiles]) {
        if(!ulNumIcon) {
          icon[ulNumfiles].cb = sizeof(DRAGIMAGE);
          icon[ulNumfiles].cptl = 0;
          icon[ulNumfiles].hImage = (isdir) ? hptrDir : hptrFile;
          icon[ulNumfiles].fl = DRG_ICON;
          icon[ulNumfiles].sizlStretch.cx = 32;
          icon[ulNumfiles].sizlStretch.cy = 32;
          icon[ulNumfiles].cxOffset = -16 + (ulNumfiles * 3);
          icon[ulNumfiles].cyOffset = 0 + (ulNumfiles * 6);
        }
        memset(DItem[ulNumfiles],0,sizeof(DRAGITEM));
        DItem[ulNumfiles]->hwndItem = (hwndObj) ? hwndObj : hwnd; /* Initialize DRAGITEM */
//        DItem[ulNumfiles]->hwndItem = hwnd;
        DItem[ulNumfiles]->ulItemID = (ULONG)Select;
        DItem[ulNumfiles]->hstrType = DrgAddStrHandle(DRT_UNKNOWN);
        DItem[ulNumfiles]->hstrRMF =
           DrgAddStrHandle(DRMDRFLIST);
        DItem[ulNumfiles]->hstrContainerName = DrgAddStrHandle(szBuffer);
        DItem[ulNumfiles]->hstrSourceName = DrgAddStrHandle(szFile);
        DItem[ulNumfiles]->hstrTargetName = DrgAddStrHandle(szFile);
        DItem[ulNumfiles]->fsControl = (isdir) ? DC_CONTAINER : 0;
        if(IsFullName(list[Select]) &&
           (driveflags[toupper(*list[Select]) - 'A'] & DRIVE_REMOVABLE))
          DItem[ulNumfiles]->fsControl |= DC_REMOVEABLEMEDIA;
        DItem[ulNumfiles]->fsSupportedOps = DO_COPYABLE | DO_LINKABLE;
        if(moveok && IsFullName(list[Select]) &&
           !(driveflags[toupper(*list[Select]) - 'A'] & DRIVE_NOTWRITEABLE))
          DItem[ulNumfiles]->fsSupportedOps |= DO_MOVEABLE;
        if(IsRoot(list[Select]))
          DItem[ulNumfiles]->fsControl = DO_LINKABLE;
        ulNumfiles++;
        DItem[ulNumfiles] = NULL;
      }
      else
        break;
    }
  }

  if(ulNumfiles) {
    pDInfo = DrgAllocDraginfo(ulNumfiles);  /* Allocate DRAGINFO */
    if(pDInfo) {
      if((IsFullName(szBuffer) &&
         (driveflags[toupper(*szBuffer) - 'A'] & DRIVE_NOTWRITEABLE)))
        pDInfo->usOperation = DO_COPY;
      else
        pDInfo->usOperation = DO_DEFAULT;
      if(IsRoot(list[0]))
        pDInfo->usOperation = DO_LINK;
      pDInfo->hwndSource = (hwndObj) ? hwndObj : hwnd;
//      pDInfo->hwndSource = hwnd;
      for(Select = 0L;Select < ulNumfiles;Select++) {
        DrgSetDragitem(pDInfo,            /* Set item in DRAGINFO  */
                       DItem[Select],     /* Pointer to DRAGITEM   */
                       sizeof(DRAGITEM),  /* Size of DRAGITEM      */
                       Select);           /* Index of DRAGITEM     */
        free(DItem[Select]);
      }
#ifdef __DEBUG_ALLOC__
      _heap_check();
#endif
      free(DItem);
      DItem = NULL;
      DosPostEventSem(CompactSem);

      if(!ulNumIcon)
        ulNumIcon = ulNumfiles;

      WinSetFocus(HWND_DESKTOP,HWND_DESKTOP);
      hDrop = DrgDrag(hwnd,                 /* Initiate drag         */
                      pDInfo,               /* DRAGINFO structure    */
                      icon,
                      ulNumIcon,
                      VK_ENDDRAG,           /* End of drag indicator */
                      (PVOID)NULL);         /* Reserved              */

      DrgFreeDraginfo(pDInfo);              /* Free DRAGINFO struct  */
      free(icon);
      icon = NULL;
      WinSetWindowPos(hwnd,HWND_TOP,0,0,0,0,SWP_ACTIVATE);
      DosPostEventSem(CompactSem);
    }
  }
  if(DItem)
    free(DItem);
  if(icon)
    free(icon);
  return hDrop;
}


#ifdef NEVER

BOOL PickUp (HWND hwndCnr,HWND hwndObj,PCNRDRAGINIT pcd) {

  PCNRITEM  pci;
  BOOL      loop = TRUE;
  PDRAGINFO pdinfoOld = NULL,pdinfoCurrent = NULL;
  ULONG     cditem = 0;
  DRAGITEM  ditem;
  DRAGIMAGE fakeicon;
  CHAR      szDir[CCHMAXPATH],szFile[CCHMAXPATH],*p;

  pci = (PCNRITEM)pcd->pRecord;
  if(pci && (INT)pci != -1) {
    if(pci->rc.flRecordAttr & CRA_SELECTED) {
      loop = TRUE;
      pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,
                       MPFROMLONG(CMA_FIRST),MPFROMSHORT(CRA_SELECTED));
    }
    while(pci && (INT)pci != -1 && *pci->szFileName) {
      if(pdinfoOld || DrgQueryDragStatus() & DGS_LAZYDRAGINPROGRESS) {
        if(!pdinfoOld)
          pdinfoOld = DrgQueryDraginfoPtr(NULL);
        if(pdinfoOld) {
          cditem = pdinfoOld->cditem + 1;
          pdinfoCurrent = DrgReallocDraginfo(pdinfoOld,cditem);
          pdinfoOld = pdinfoCurrent;
        }
      }
      else
        pdinfoCurrent = pdinfoOld = DrgAllocDraginfo(1);
      if(pdinfoCurrent) {
        strcpy(szDir,pci->szFileName);
        p = szDir;
        while(*p) {
          if(*p == '/')
            *p = '\\';
          p++;
        }
        p = strrchr(szDir,'\\');
        if(p) {
          *p = 0;
          p++;
          strcpy(szFile,p);
          strcat(szDir,"\\");
        }
        else {
          strcpy(szFile,pci->szFileName);
          *szDir = 0;
        }
        ditem.ulItemID = (ULONG)pci;
        ditem.hwndItem = (hwndObj) ? hwndObj : hwndCnr;
        ditem.hstrType = DrgAddStrHandle(DRT_UNKNOWN);
        ditem.hstrRMF =
           DrgAddStrHandle(DRMDRFLIST);
        ditem.hstrContainerName = DrgAddStrHandle(szDir);
        ditem.hstrSourceName = DrgAddStrHandle(szFile);
        ditem.hstrTargetName = DrgAddStrHandle(szFile);
        ditem.fsControl = 0;
        if(IsRoot(pci->szFileName) || (pci->attrFile & FILE_DIRECTORY) != 0)
          ditem.fsControl |= DC_CONTAINER;
        if(IsFullName(pci->szFileName) &&
           (driveflags[toupper(*pci->szFileName) - 'A'] & DRIVE_REMOVABLE))
          ditem.fsControl |= DC_REMOVEABLEMEDIA;
        ditem.fsSupportedOps = DO_COPYABLE | DO_LINKABLE;
        if(IsFullName(pci->szFileName) &&
           !(driveflags[toupper(*pci->szFileName) - 'A'] & DRIVE_NOTWRITEABLE))
          ditem.fsSupportedOps |= DO_MOVEABLE;
        if(IsRoot(pci->szFileName))
          ditem.fsSupportedOps = DO_LINKABLE;
        memset(&fakeicon,0,sizeof(DRAGIMAGE));
        fakeicon.hImage = pci->rc.hptrIcon;
        fakeicon.cb = sizeof(DRAGIMAGE);
        fakeicon.cptl = 0;
        fakeicon.fl = DRG_ICON;
        fakeicon.sizlStretch.cx = 32;
        fakeicon.sizlStretch.cy = 32;
        fakeicon.cxOffset = -16;
        fakeicon.cyOffset = 0;
        if(IsFullName(pci->szFileName) &&
           (driveflags[toupper(*pci->szFileName) - 'A'] & DRIVE_NOTWRITEABLE))
          pdinfoCurrent->usOperation = DO_COPY;
        else
          pdinfoCurrent->usOperation = DO_DEFAULT;
        if(IsRoot(pci->szFileName))
          pdinfoCurrent->usOperation = DO_LINK;
        pdinfoCurrent->hwndSource = (hwndObj) ? hwndObj : hwndCnr;
        DrgSetDragitem(pdinfoCurrent,
                       &ditem,
                       sizeof(DRAGITEM),
                       cditem);
      }
      if(!loop)
        break;
      pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,
                       MPFROMP(pci),MPFROMSHORT(CRA_SELECTED));
    }
    if(pdinfoCurrent)
      return DrgLazyDrag(hwndCnr,pdinfoCurrent,&fakeicon,1,NULL);
  }
  return FALSE;
}

#endif
