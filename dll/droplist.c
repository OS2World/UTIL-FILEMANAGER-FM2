#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fm3dll.h"
#include "fm3str.h"

#pragma alloc_text(DROPLIST,DoFileDrop,FullDrgName,TwoDrgNames,GetOneDrop)


BOOL TwoDrgNames (PDRAGITEM pDItem,CHAR *buffer1,ULONG buflen1,
                  char *buffer2,ULONG buflen2) {

  /*
   * Gets archive name from directory field, file name from file field
   * Returns FALSE on error, TRUE on success.
   */

  register ULONG len;
  BOOL           ret = FALSE;

  if(pDItem && buffer2 && buflen2) {     /* else error calling function */
    if(buffer1 && buflen1)               /* zero buffers */
      *buffer1 = 0;
    *buffer2 = 0;

    if(buffer1 && buflen1) {
      len = DrgQueryStrName(pDItem->hstrContainerName,
                            buflen1,buffer1);
      buffer1[len] = 0;
      if(len) { /* be sure we get full pathname of arc file */

        char szTemp[CCHMAXPATH + 2];

        if(!DosQueryPathInfo(buffer1,
                             FIL_QUERYFULLNAME,
                             szTemp,
                             sizeof(szTemp))) {
          strncpy(buffer1,
                  szTemp,
                  buflen1);
          buffer1[buflen1 - 1] = 0;
        }
      }
      { /* be sure that file/directory is accessible */
        FILESTATUS3 fsa3;

        DosError(FERR_DISABLEHARDERR);
        if(DosQueryPathInfo(buffer1,
                            FIL_STANDARD,
                            &fsa3,
                            sizeof(fsa3)) ||
           (fsa3.attrFile & FILE_DIRECTORY) != 0) {
          *buffer1 = 0;
          return ret;
        }
      }
    }

    len = DrgQueryStrName(pDItem->hstrSourceName,
                          buflen2,
                          buffer2);
    buffer2[len] = 0;
    if(len)
      ret = TRUE;
  }
  return ret;
}


BOOL FullDrgName (PDRAGITEM pDItem,CHAR *buffer,ULONG buflen) {

  /*
   * Gets full name of file from a dragged item.
   * Returns FALSE on error, TRUE on success.
   */

  register ULONG len,blen;
  BOOL           ret = FALSE;

  if(pDItem && buffer && buflen) {      /* else error calling function */
    *buffer = 0;                        /* zero buffer */

    blen = DrgQueryStrName(pDItem->hstrContainerName,
                           buflen,buffer);
    if(blen) {
      if(*(buffer + (blen - 1)) != '\\') {
        *(buffer + blen) = '\\';
        blen++;
      }
    }
    buffer[blen] = 0;
    len = DrgQueryStrName(pDItem->hstrSourceName,
                          buflen - blen,buffer + blen);
    buffer[blen + len] = 0;
    { /* be sure we get full pathname of file/directory */
      char szTemp[CCHMAXPATH + 2];

      if(!DosQueryPathInfo(buffer,
                           FIL_QUERYFULLNAME,
                           szTemp,
                           sizeof(szTemp))) {
        strncpy(buffer,
                szTemp,
                buflen);
        buffer[buflen - 1] = 0;
      }
    }
    { /* be sure that file/directory is accessible */
      FILESTATUS3 fsa3;

      if(!DosQueryPathInfo(buffer,
                           FIL_STANDARD,
                           &fsa3,
                           sizeof(fsa3)))
        ret = TRUE;
      else
        *buffer = 0;
    }
  }
  return ret;
}


BOOL GetOneDrop (MPARAM mp1,MPARAM mp2,char *buffer,ULONG buflen) {

  PDRAGITEM      pDItem;                       /* DRAGITEM struct ptr   */
  PDRAGINFO      pDInfo;                       /* DRAGINFO struct ptr   */
  ULONG          numitems;
  register ULONG x;
  BOOL           ret = FALSE;

  if(buffer && buflen)
    *buffer = 0;                               /* zero buffer field     */

  pDInfo = (PDRAGINFO)mp1;                     /* Get DRAGINFO pointer  */
  if(pDInfo) {
    DrgAccessDraginfo(pDInfo);                 /* Access DRAGINFO       */
    numitems = DrgQueryDragitemCount(pDInfo);
    pDItem = DrgQueryDragitemPtr(pDInfo,       /* Access DRAGITEM       */
                                 0);           /* Index to DRAGITEM     */
    if(buflen && buffer) {
      if(DrgVerifyRMF(pDItem,                  /* Check valid rendering */
                      DRM_OS2FILE,             /* mechanisms and data   */
                      NULL) &&
         !(pDItem->fsControl & DC_PREPARE))
        ret = FullDrgName(pDItem,buffer,buflen);
    }
    /* note:  targetfail is returned to source for all items */
    for(x = 0;x < numitems;x++) {
      pDItem = DrgQueryDragitemPtr(pDInfo,       /* Access DRAGITEM   */
                                   x);           /* Index to DRAGITEM */
      DrgSendTransferMsg(pDInfo->hwndSource,DM_ENDCONVERSATION,
                         MPFROMLONG(pDItem->ulItemID),
                         MPFROMLONG(DMFL_TARGETFAIL));
    }
    DrgDeleteDraginfoStrHandles(pDInfo);
    DrgFreeDraginfo(pDInfo);                 /* Free DRAGINFO */
  }

  return ret;
}


BOOL AcceptOneDrop (MPARAM mp1,MPARAM mp2) {

  PDRAGITEM pDItem;                        /* Pointer to DRAGITEM   */
  PDRAGINFO pDInfo;                        /* Pointer to DRAGINFO   */
  BOOL      ret = FALSE;

  pDInfo = (PDRAGINFO)mp1;                 /* Get DRAGINFO pointer  */
  if(pDInfo) {
    DrgAccessDraginfo(pDInfo);               /* Access DRAGINFO       */
    pDItem = DrgQueryDragitemPtr(pDInfo,     /* Access DRAGITEM       */
                                 0);         /* Index to DRAGITEM     */
    if(DrgVerifyRMF(pDItem,                  /* Check valid rendering */
                    DRM_OS2FILE,             /* mechanisms and data   */
                    NULL))                   /* formats               */
      ret = TRUE;
    DrgFreeDraginfo(pDInfo);
  }
  return ret;
}


ULONG FreeDrop (MPARAM mp1,MPARAM mp2) {

  PDRAGINFO pDInfo;
  ULONG     numitems;

  pDInfo = mp1;
  if(pDInfo) {
    DrgAccessDraginfo(pDInfo);
    numitems = DrgQueryDragitemCount(pDInfo);
    DrgDeleteDraginfoStrHandles(pDInfo);
    DrgFreeDraginfo(pDInfo);
  }
  return numitems;
}


void DropHelp (MPARAM mp1,MPARAM mp2,HWND hwnd,char *text) {

  ULONG numitems;

  numitems = FreeDrop(mp1,mp2);
  saymsg(MB_ENTER | MB_ICONASTERISK,
         hwnd,
         GetPString(IDS_FM2DROPHELPTEXT),
         text,
         numitems,
         numitems);
}


LISTINFO * DoFileDrop (HWND hwndCnr, CHAR *directory, BOOL arcfilesok,
                       MPARAM mp1, MPARAM mp2) {

  /* builds a list from the dropped files */

  BOOL            isArc = FALSE,arctest = FALSE;
  PDRAGITEM       pDItem;
  PDRAGINFO       pDInfo;
  PCNRITEM        pci;
  CHAR            szFrom[CCHMAXPATH + 1],
                  szArc[CCHMAXPATH + 1];
  register CHAR **files = NULL;
  INT             numfiles = 0,numalloc = 0;
  register ULONG  curitem = 0L,numitems,*cbFile = NULL,*ulitemID = NULL;
  LISTINFO       *li = NULL;
  ARC_TYPE       *arcinfo = NULL;
  USHORT          Operation;

  *szArc = 0;
  pci = (PCNRITEM)((PCNRDRAGINFO)mp2)->pRecord;
  pDInfo = ((PCNRDRAGINFO)mp2)->pDragInfo;
  if(!pDInfo)
    return NULL;
  DrgAccessDraginfo(pDInfo);
  Operation = pDInfo->usOperation;
  pDItem = DrgQueryDragitemPtr(pDInfo,0L);
  if(Operation == DO_MOVE &&
     !(pDItem->fsSupportedOps & DO_MOVEABLE)) {
    DosBeep(50,100);
    Operation = DO_COPY;
  }
  numitems = DrgQueryDragitemCount(pDInfo);
  while(curitem < numitems) {
    pDItem = DrgQueryDragitemPtr(pDInfo,
                                 curitem);
    if(!pDItem)
      break;

    /* ambiguous drop request -- see what's allowed */
    if(Operation == DO_DEFAULT || Operation >= DO_UNKNOWN) {
      if(pDItem->fsSupportedOps & DO_COPYABLE)
        Operation = DO_COPY;
      else if(pDItem->fsSupportedOps & DO_MOVEABLE)
        Operation = DO_MOVE;
      else if(pDItem->fsSupportedOps & DO_LINKABLE)
        Operation = DO_LINK;
    }
    else {  /* ignore object if selected command not allowed for it */
      switch(Operation) {
        case DO_MOVE:
          if(pDItem->fsSupportedOps & DO_MOVEABLE)
            goto Okay;
          break;
        case DO_COPY:
          if(pDItem->fsSupportedOps & DO_COPYABLE)
            goto Okay;
          break;
        case DO_LINK:
          if(pDItem->fsSupportedOps & DO_LINKABLE)
            goto Okay;
          break;
      }
      DrgSendTransferMsg(pDItem->hwndItem,
                         DM_ENDCONVERSATION,
                         MPFROMLONG(pDItem->ulItemID),
                         MPFROMLONG(DMFL_TARGETFAIL));
      curitem++;
      continue;
    }

Okay:

    if(DrgVerifyRMF(pDItem,
                    DRM_OS2FILE,
                    NULL) ||
       (arcfilesok &&
        ((arctest = DrgVerifyRMF(pDItem,
                                 DRM_FM2ARCMEMBER,
                                 DRF_FM2ARCHIVE)) != FALSE))) {
      if(pDItem->fsControl & DC_PREPARE) {
        DrgSendTransferMsg(pDItem->hwndItem,
                           DM_ENDCONVERSATION,
                           MPFROMLONG(pDItem->ulItemID),
                           MPFROMLONG(DMFL_TARGETFAIL));
        curitem++;
        continue;
      }

      if(arctest ||
         isArc) {
        if(!isArc) {
          if(TwoDrgNames(pDItem,
                         szArc,
                         sizeof(szArc),
                         szFrom,
                         sizeof(szFrom)) &&
             *szArc &&
             *szFrom) {
            isArc = TRUE;
            arcinfo = find_type(szArc,arcsighead);
          }
          if(!arcinfo ||
             !arcinfo->extract ||
             !*arcinfo->extract) {
            *szArc = *szFrom = 0;
            isArc = FALSE;
          }
        }
        else
          TwoDrgNames(pDItem,
                      NULL,
                      0,
                      szFrom,
                      sizeof(szFrom));
      }
      else
        FullDrgName(pDItem,
                    szFrom,
                    sizeof(szFrom));

      if(!*szFrom) {
        DrgSendTransferMsg(pDItem->hwndItem,
                           DM_ENDCONVERSATION,
                           MPFROMLONG(pDItem->ulItemID),
                           MPFROMLONG(DMFL_TARGETFAIL));
        curitem++;
        continue;
      }

      if(numfiles + 2 > numalloc) {

        CHAR  **test;
        ULONG  *ltest;

        numalloc += 12;
        test = realloc(files,numalloc * sizeof(CHAR *));
        if(!test)
          goto AbortDrop;
        files = test;
        ltest = realloc(cbFile,numalloc * sizeof(ULONG));
        if(!ltest)
          goto AbortDrop;
        cbFile = ltest;
        ltest = realloc(ulitemID,numalloc * sizeof(ULONG));
        if(!ltest)
          goto AbortDrop;
        ulitemID = ltest;
      }
      cbFile[numfiles] = 0;
      if(!isArc) {

        FILESTATUS4 fsa4;

        if(!DosQueryPathInfo(szFrom,
                             FIL_QUERYEASIZE,
                             &fsa4,
                             sizeof(fsa4)))
          cbFile[numfiles] = fsa4.cbFile + ((fsa4.cbList > 4L) ?
                                            fsa4.cbList : 0L);
      }
      ulitemID[numfiles] = pDItem->ulItemID;
      files[numfiles] = strdup(szFrom);
      files[numfiles + 1] = NULL;
      if(!files[numfiles])
        goto AbortDrop;
      numfiles++;
      DrgSendTransferMsg(pDItem->hwndItem,
                         DM_ENDCONVERSATION,
                         MPFROMLONG(pDItem->ulItemID),
                         MPFROMLONG(DMFL_TARGETSUCCESSFUL));
    }
    else
      DrgSendTransferMsg(pDItem->hwndItem,
                         DM_ENDCONVERSATION,
                         MPFROMLONG(pDItem->ulItemID),
                         MPFROMLONG(DMFL_TARGETFAIL));
    curitem++;
  }

AbortDrop:

  if(files &&
     numfiles &&
     files[0] &&
     cbFile &&
     ulitemID) {
    li = malloc(sizeof(LISTINFO));
    if(li) {
      memset(li,0,sizeof(LISTINFO));
      li->type  = Operation;
      li->hwnd  = hwndCnr;
      li->list  = files;
      li->cbFile = cbFile;
      li->ulitemID = ulitemID;
      li->hwndS = pDInfo->hwndSource;
      if(!pci &&
         directory)
        strcpy(li->targetpath,directory);
      else if(pci)
        strcpy(li->targetpath,pci->szFileName);
      if(isArc) {
        strcpy(li->arcname,szArc);
        li->info = arcinfo;
      }
      SortList(li);
    }
    else {
      if(cbFile)
        free(cbFile);
      if(ulitemID)
        free(ulitemID);
      if(files)
        FreeList(files);
    }
  }
  else {
    if(cbFile)
      free(cbFile);
    if(ulitemID)
      free(ulitemID);
    if(files)
      FreeList(files);
  }
  DrgDeleteDraginfoStrHandles(pDInfo);
  DrgFreeDraginfo(pDInfo);
  return li;
}

