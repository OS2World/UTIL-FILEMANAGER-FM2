
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  Revisions	01 Aug 04 SHL - Rework lstrip/rstrip usage

***********************************************************************/

#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <share.h>
#include "fm3dll.h"
#include "fm3str.h"

#pragma alloc_text(SELECT,UnHilite,SelectAll,DeselectAll,MarkAll,SetMask)
#pragma alloc_text(SELECT,SelectList)
#pragma alloc_text(SELECT1,Deselect,HideAll,RemoveAll,ExpandAll,InvertAll)


VOID UnHilite (HWND hwndCnr,BOOL all,CHAR ***list) {

  PCNRITEM pci;
  INT      numfiles = 0,numalloc = 0;
  INT      attribute = CRA_CURSORED;

  if(all && list && *list) {
    FreeList(*list);
    *list = NULL;
  }
  pci = (PCNRITEM)CurrentRecord(hwndCnr);
  if(pci && (INT)pci != -1) {
    if(pci->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMLONG(CMA_FIRST),
                       MPFROMSHORT(attribute));
    }
    while(pci && (INT)pci != -1) {
      WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                 MPFROM2SHORT(FALSE,CRA_SELECTED));
      if(!all)
        break;
      if(list)
        AddToList(pci->szFileName,list,&numfiles,&numalloc);
      pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,
                                 MPFROMP(pci),
                                 MPFROMSHORT(CRA_SELECTED));
    }
  }
}


VOID SelectList (HWND hwndCnr,BOOL partial,BOOL deselect,BOOL clearfirst,
                 PCNRITEM pciParent,CHAR *filename,CHAR **list) {

  PCNRITEM     pci;
  register INT x;
  BOOL         foundone = FALSE;
  ULONG        errs = 0L;

  if(clearfirst && !deselect)
    UnHilite(hwndCnr,TRUE,NULL);
  if(list && list[0]) {
    for(x = 0;list[x];x++) {
      pci = FindCnrRecord(hwndCnr,
                          list[x],
                          pciParent,
                          partial,
                          partial,
                          TRUE);
      if(pci) {
        WinSendMsg(hwndCnr,
                   CM_SETRECORDEMPHASIS,
                   MPFROMP(pci),
                   MPFROM2SHORT((SHORT)((deselect) ? FALSE : TRUE),
                                CRA_SELECTED));
        foundone = TRUE;
      }
    }
    if(!foundone && !partial) {

    }
    if(!foundone)
      DosBeep(250,50);
  }
  else if(filename && *filename) {

    FILE *fp;
    CHAR input[1024],*p;

    fp = _fsopen(filename,"r",SH_DENYNO);
    if(fp) {
      while(!feof(fp)) {
        if(!fgets(input,1024,fp))
          break;
        input[1023] = 0;
        bstripcr(input);
        if(*input == '\"') {
          memmove(input,input + 1,strlen(input) + 1);
          lstrip(input);
          p = strchr(input,'\"');
          if(p)
            *p = 0;
          rstrip(input);
        }
        else {
          p = strchr(input,' ');
          if(p)
            *p = 0;
        }
        /* input now contains name of file to select */
        pci = FindCnrRecord(hwndCnr,
                            input,
                            pciParent,
                            partial,
                            partial,
                            TRUE);
        if(pci)       /* found it? */
          WinSendMsg(hwndCnr,
                     CM_SETRECORDEMPHASIS,
                     MPFROMP(pci),
                     MPFROM2SHORT((SHORT)((deselect) ? FALSE : TRUE),
                                  CRA_SELECTED));
        else
          errs++;
        if(errs > 50L) {  /* prevent runaway on bad file */

          APIRET ret;

          ret = saymsg(MB_YESNO,
                       hwndCnr,
                       GetPString(IDS_POSSIBLEERRORTEXT),
                       GetPString(IDS_MAYNOTBELISTTEXT),
                       filename);
          if(ret == MBID_NO)
            break;
          errs = 0L;
        }
      }
      fclose(fp);
    }
  }
}


VOID SelectAll (HWND hwndCnr,BOOL files,BOOL dirs,CHAR *mask,
                CHAR *text,BOOL arc) {

  PCNRITEM       pci;
  BOOL           markit;
  register CHAR *file;
  MASK           Mask;
  register INT   x;
  ULONG          textlen = 0;

  if(text)
    textlen = strlen(text);
  memset(&Mask,0,sizeof(Mask));
  if(mask && *mask)
    SetMask(mask,&Mask);
  pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPVOID,
                             MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
  while( pci && (INT)pci != -1 ) {
    markit = FALSE;
    if(!(pci->rc.flRecordAttr & CRA_FILTERED)) {
      if(!arc) {
        if(files && !(pci->attrFile & FILE_DIRECTORY))
          markit = TRUE;
        if(dirs && (pci->attrFile & FILE_DIRECTORY))
          markit = TRUE;
      }
      else
        markit = TRUE;
      if(mask && *mask && markit) {
        markit = FALSE;
        file = strrchr(pci->szFileName,'\\');
        if(!file)
          file = strrchr(pci->szFileName,':');
        if(file)
          file++;
        else
          file = pci->szFileName;
        for(x = 0;Mask.pszMasks[x];x++) {
          if(*Mask.pszMasks[x]) {
            if(*Mask.pszMasks[x] != '/') {
              if(wildcard((strchr(Mask.pszMasks[x],'\\') ||
                           strchr(Mask.pszMasks[x],':')) ?
                           pci->szFileName : file,Mask.pszMasks[x],FALSE))
                markit = TRUE;
            }
            else {
              if(wildcard((strchr(Mask.pszMasks[x],'\\') ||
                           strchr(Mask.pszMasks[x],':'),FALSE) ?
                           pci->szFileName : file,Mask.pszMasks[x] + 1,
                           FALSE)) {
                markit = FALSE;
                break;
              }
            }
          }
        }
      }
    }
    if(markit && text && *text && !(pci->attrFile & FILE_DIRECTORY)) {

      CHAR *input;

      markit = FALSE;
      input = malloc(65537);
      if(input) {

        ULONG pos;
        LONG  len;
        FILE *inputFile;

        if((inputFile = _fsopen(pci->szFileName,"rb",SH_DENYNO)) != NULL) {
          pos = ftell(inputFile);
          while(!feof(inputFile)) {
            if(pos)
              fseek(inputFile,pos - 256,SEEK_SET);
            len = fread(input,1,65536,inputFile);
            if(len >= 0) {
              if(findstring(text,textlen,input,len,FALSE)) {
                markit = TRUE;
                break;
              }
            }
            else
              break;
          }
          fclose(inputFile) ;
        }
        free(input);
        DosSleep(1L);
      }
    }
    else if(markit && text && *text && (pci->attrFile & FILE_DIRECTORY))
      markit = FALSE;
    if(markit)
      WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                 MPFROM2SHORT(TRUE,CRA_SELECTED));
    pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(pci),
                               MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
  }
}


VOID DeselectAll (HWND hwndCnr,BOOL files,BOOL dirs,CHAR *mask,CHAR *text,
                  BOOL arc) {

  PCNRITEM       pci;
  BOOL           unmarkit;
  register CHAR *file;
  MASK           Mask;
  register INT   x;
  ULONG          textlen = 0;

  if(text)
    textlen = strlen(text);
  memset(&Mask,0,sizeof(Mask));
  if(mask && *mask)
    SetMask(mask,&Mask);
  pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPVOID,
                             MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
  while( pci && (INT)pci != -1 ) {
    unmarkit = FALSE;
    if(!(pci->rc.flRecordAttr & CRA_FILTERED)) {
      if(!arc) {
        if(files && !(pci->attrFile & FILE_DIRECTORY))
          unmarkit = TRUE;
        if(dirs && (pci->attrFile & FILE_DIRECTORY))
          unmarkit = TRUE;
      }
      else
        unmarkit = TRUE;
      if(mask && *mask && unmarkit) {
        unmarkit = FALSE;
        file = strrchr(pci->szFileName,'\\');
        if(!file)
          file = strrchr(pci->szFileName,':');
        if(file)
          file++;
        else
          file = pci->szFileName;
        for(x = 0;Mask.pszMasks[x];x++) {
          if(*Mask.pszMasks[x]) {
            if(*Mask.pszMasks[x] != '/') {
              if(wildcard((strchr(Mask.pszMasks[x],'\\') ||
                           strchr(Mask.pszMasks[x],':')) ?
                           pci->szFileName : file,Mask.pszMasks[x],FALSE))
                unmarkit = TRUE;
            }
            else {
              if(wildcard((strchr(Mask.pszMasks[x],'\\') ||
                           strchr(Mask.pszMasks[x],':')) ?
                           pci->szFileName : file,Mask.pszMasks[x] + 1,
                           FALSE)) {
                unmarkit = FALSE;
                break;
              }
            }
          }
        }
      }
    }
    if(unmarkit && text && *text && !(pci->attrFile & FILE_DIRECTORY)) {

      CHAR *input;

      unmarkit = FALSE;
      input = malloc(65537);
      if(input) {

        ULONG pos;
        LONG  len;
        FILE *inputFile;

        if((inputFile = _fsopen(pci->szFileName,"rb",SH_DENYNO)) != NULL) {
          pos = ftell(inputFile);
          while(!feof(inputFile)) {
            if(pos)
              fseek(inputFile,pos - 256,SEEK_SET);
            len = fread(input,1,65536,inputFile);
            if(len >= 0) {
              if(findstring(text,textlen,input,len,FALSE)) {
                unmarkit = TRUE;
                break;
              }
            }
            else
              break;
          }
          fclose(inputFile) ;
        }
        free(input);
        DosSleep(1L);
      }
    }
    else if(unmarkit && text && *text && (pci->attrFile & FILE_DIRECTORY))
      unmarkit = FALSE;
    if(unmarkit)
      WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,pci,
                 MPFROM2SHORT(FALSE,CRA_SELECTED | CRA_CURSORED |
                              CRA_INUSE | CRA_SOURCE));
    pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(pci),
                               MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
  }
}


VOID Deselect (HWND hwndCnr) {

  PCNRITEM pcil;

  pcil = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,
                              MPFROMLONG(CMA_FIRST),
                              MPFROMSHORT(CRA_SELECTED));
  while(pcil && (INT)pcil != -1) {
    WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pcil),
               MPFROM2SHORT(FALSE,CRA_SELECTED));
    pcil = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMP(pcil),
                      MPFROMSHORT(CRA_SELECTED));
  }
}


VOID HideAll (HWND hwndCnr) {

  PCNRITEM pci,pciH;
  INT      attribute = CRA_CURSORED;
  CNRINFO  cnri;
  BOOL     didone = FALSE;

  memset(&cnri,0,sizeof(CNRINFO));
  cnri.cb = sizeof(CNRINFO);
  WinSendMsg(hwndCnr,CM_QUERYCNRINFO,MPFROMP(&cnri),
             MPFROMLONG(sizeof(CNRINFO)));
  pci = (PCNRITEM)CurrentRecord(hwndCnr);
  if(pci && (INT)pci != -1) {
    if(pci->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMLONG(CMA_FIRST),
                       MPFROMSHORT(attribute));
    }
  }
  while(pci && (INT)pci != -1) {
    pciH = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMP(pci),
                      MPFROMSHORT(attribute));
    WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pci),
               MPFROM2SHORT(FALSE,CRA_CURSORED | CRA_SELECTED |
                                  CRA_INUSE | CRA_SOURCE));
    pci->rc.flRecordAttr |= CRA_FILTERED;
    didone = TRUE;
    if(fSyncUpdates) {
      if(cnri.flWindowAttr & CV_DETAIL)
        WinSendMsg(hwndCnr,CM_INVALIDATERECORD,MPVOID,
                   MPFROM2SHORT(0,CMA_REPOSITION | CMA_ERASE));
      else
        WinSendMsg(hwndCnr,CM_INVALIDATERECORD,MPFROMP(&pci),
                   MPFROM2SHORT(1,CMA_REPOSITION | CMA_ERASE));
    }
    pci = pciH;
  }
  if(didone && !fSyncUpdates)
      WinSendMsg(hwndCnr,CM_INVALIDATERECORD,MPVOID,
                 MPFROM2SHORT(0,CMA_ERASE | CMA_REPOSITION));
}


VOID MarkAll (HWND hwndCnr,BOOL quitit,BOOL target,BOOL source) {

  PCNRITEM pci;
  INT      attribute = CRA_CURSORED;

  if(quitit)
    attribute = (target) ? CRA_TARGET : (source) ? CRA_SOURCE : CRA_INUSE;
  pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,
                             MPFROMLONG(CMA_FIRST),
                             MPFROMSHORT(attribute));
  if(pci && (INT)pci != -1) {
    if(attribute == CRA_CURSORED) {
      if(pci->rc.flRecordAttr & CRA_SELECTED) {
        attribute = CRA_SELECTED;
        pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMLONG(CMA_FIRST),
                         MPFROMSHORT(attribute));
      }
    }
  }
  while( pci && (INT)pci != -1) {
    WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pci),
               MPFROM2SHORT(!quitit,
               ((target) ? CRA_TARGET : (source) ? CRA_SOURCE : CRA_INUSE)));
    pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMP(pci),
                     MPFROMSHORT(attribute));
  }
}


VOID RemoveAll (HWND hwndCnr,ULONG *totalbytes,ULONG *totalfiles) {

  PCNRITEM pci;
  INT      attribute = CRA_CURSORED;
  BOOL     didone = FALSE;

  pci = (PCNRITEM)CurrentRecord(hwndCnr);
  if(pci && (INT)pci != -1) {
    if(pci->rc.flRecordAttr & CRA_SELECTED) {
      attribute = CRA_SELECTED;
      pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMLONG(CMA_FIRST),
                       MPFROMSHORT(attribute));
    }
  }
  while(pci && (INT)pci != -1) {
    if(!(pci->rc.flRecordAttr & CRA_FILTERED)) {
      didone = TRUE;
      if(totalfiles)
        *totalfiles--;
      if(totalbytes)
        *totalbytes -= (pci->cbFile + pci->easize);
      WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                 MPFROM2SHORT(0,CRA_SELECTED));
      if(fSyncUpdates)
        WinSendMsg(hwndCnr,CM_REMOVERECORD,MPFROMP(&pci),
                   MPFROM2SHORT(1,CMA_FREE | CMA_INVALIDATE));
      else
        WinSendMsg(hwndCnr,CM_REMOVERECORD,MPFROMP(&pci),
                   MPFROM2SHORT(1,CMA_FREE));
      if(attribute == CRA_CURSORED)
        break;
      pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMLONG(CMA_FIRST),
                       MPFROMSHORT(attribute));
    }
    else
      pci = WinSendMsg(hwndCnr,CM_QUERYRECORDEMPHASIS,MPFROMP(pci),
                       MPFROMSHORT(attribute));
  }
  if(didone && !fSyncUpdates)
      WinSendMsg(hwndCnr,CM_INVALIDATERECORD,MPVOID,
                 MPFROM2SHORT(0,CMA_REPOSITION));
}


VOID SetMask (CHAR *str,MASK *mask) {

  register INT   x;
  register CHAR *p;

  if(str != mask->szMask)
    strcpy(mask->szMask,str);
  strcpy(mask->szMaskCopy,mask->szMask);
  memset(mask->pszMasks,0,sizeof(CHAR *) * 26);
  p = mask->pszMasks[0] = mask->szMaskCopy;
  for(x = 1;x < 24;x++) {
    while(*p && *p != ';')
     p++;
    if(*p) {
      *p = 0;
      p++;
      mask->pszMasks[x] = p;
    }
    else {
      mask->pszMasks[x] = NULL;
      break;
    }
  }
  mask->pszMasks[x] = NULL;
}


VOID ExpandAll (HWND hwndCnr,BOOL expand,PCNRITEM pciParent) {

  PCNRITEM pci;

  if(!pciParent)
    pciParent = WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(NULL),
                           MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
  if(pciParent) {
    if(expand && !(pciParent->rc.flRecordAttr & CRA_EXPANDED))
      WinSendMsg(hwndCnr,CM_EXPANDTREE,MPFROMP(pciParent),MPVOID);
    else if(!expand && (pciParent->rc.flRecordAttr & CRA_EXPANDED))
      WinSendMsg(hwndCnr,CM_COLLAPSETREE,MPFROMP(pciParent),MPVOID);
    pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(pciParent),
                               MPFROM2SHORT(CMA_FIRSTCHILD,CMA_ITEMORDER));
    if(pci)
      DosSleep(1L);
    while(pci && (INT)pci != -1) {
      ExpandAll(hwndCnr,expand,pci);
      pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(pci),
                                 MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
    }
  }
  DosSleep(0L);
}


VOID InvertAll (HWND hwndCnr) {

  PCNRITEM pci;

  pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPVOID,
                             MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
  while( pci && (INT)pci != -1 ) {
    if(!(pci->rc.flRecordAttr & CRA_FILTERED)) {
      if(!(pci->rc.flRecordAttr & CRA_SELECTED))
        WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                   MPFROM2SHORT(TRUE,CRA_SELECTED));
      else
        WinSendMsg(hwndCnr,CM_SETRECORDEMPHASIS,MPFROMP(pci),
                   MPFROM2SHORT(FALSE,CRA_SELECTED));
    }
    pci = (PCNRITEM)WinSendMsg(hwndCnr,CM_QUERYRECORD,MPFROMP(pci),
                               MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
  }
}


#pragma alloc_text (SELECT3,SpecialSelect,CompNames,CompNamesB)
#pragma alloc_text(SELECT4,FreeCnrs,SpecialSelect2,CompSSNames,CompSSNamesB)


static int CompNamesB (const void *s1,const void *s2) {

  PCNRITEM pci = *(PCNRITEM *)s2;

  return stricmp((CHAR *)s1,pci->pszFileName);
}


static int CompNames (const void *s1,const void *s2) {

  PCNRITEM pci1 = *(PCNRITEM *)s1;
  PCNRITEM pci2 = *(PCNRITEM *)s2;

  return stricmp(pci1->pszFileName,pci2->pszFileName);
}


VOID SpecialSelect (HWND hwndCnrS,HWND hwndCnrD,INT action,BOOL reset) {

  PCNRITEM       pciS,pciD,*pciSa = NULL,*pciDa = NULL;
  CNRINFO        cnri;
  BOOL           slow = FALSE;
  register INT   x,numD,numS;

  if(!hwndCnrS || !hwndCnrD)
    return;

  memset(&cnri,0,sizeof(CNRINFO));
  cnri.cb = sizeof(CNRINFO);
  WinSendMsg(hwndCnrD,CM_QUERYCNRINFO,MPFROMP(&cnri),
             MPFROMLONG(sizeof(CNRINFO)));
  numD = (INT)cnri.cRecords;
  memset(&cnri,0,sizeof(CNRINFO));
  cnri.cb = sizeof(CNRINFO);
  WinSendMsg(hwndCnrS,CM_QUERYCNRINFO,MPFROMP(&cnri),
             MPFROMLONG(sizeof(CNRINFO)));
  numS = (INT)cnri.cRecords;
  if(!numD || numS != numD) {
    DosBeep(250,100);
    saymsg(MB_ENTER,
           HWND_DESKTOP,
           DEBUG_STRING,
           "numD (%lu) != numS (%lu)",
           numD,
           numS);
    return;
  }
  pciDa = malloc(sizeof(PCNRITEM) * numD);
  if(!pciDa) {
    DosBeep(250,100);
    return;
  }

  pciSa = malloc(sizeof(PCNRITEM) * numS);
  if(!pciSa) {
    if(pciDa)
      free(pciDa);
    DosBeep(250,100);
    return;
  }

Restart:

  memset(pciDa,0,sizeof(PCNRITEM) * numD);
  memset(pciSa,0,sizeof(PCNRITEM) * numS);

  pciD = (PCNRITEM)WinSendMsg(hwndCnrD,CM_QUERYRECORD,MPVOID,
                              MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
  x = 0;
  while( pciD && (INT)pciD != -1 && x < numD) {
    if(reset)
      pciD->flags = 0;
    pciDa[x] = pciD;
    x++;
    if(!slow)
      pciD = (PCNRITEM)pciD->rc.preccNextRecord;
    else
      pciD = (PCNRITEM)WinSendMsg(hwndCnrD,CM_QUERYRECORD,MPFROMP(pciD),
                                 MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
    if(!(x % 500))
      DosSleep(1L);
    else if(!(x % 50))
      DosSleep(0L);
  }
  if(numD != x) {
    if(!slow) {
      slow = TRUE;
      goto Restart;
    }
    free(pciDa);
    free(pciSa);
    DosBeep(250,100);
    saymsg(MB_ENTER,
           HWND_DESKTOP,
           DEBUG_STRING,
           "numD (%lu) != x (%lu)",
           numD,
           x);
    return;
  }

  pciS = (PCNRITEM)WinSendMsg(hwndCnrS,CM_QUERYRECORD,MPVOID,
                              MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
  x = 0;
  while( pciS && (INT)pciS != -1 && x < numS) {
    if(reset)
      pciS->flags = 0;
    pciSa[x] = pciS;
    x++;
    if(!slow)
      pciS = (PCNRITEM)pciS->rc.preccNextRecord;
    else
      pciS = (PCNRITEM)WinSendMsg(hwndCnrS,CM_QUERYRECORD,MPFROMP(pciS),
                                 MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
    if(!(x % 500))
      DosSleep(1L);
    else if(!(x % 50))
      DosSleep(0L);
  }
  if(numS != x) {
    if(!slow) {
      slow = TRUE;
      goto Restart;
    }
    free(pciSa);
    free(pciDa);
    DosBeep(250,100);
    saymsg(MB_ENTER,
           HWND_DESKTOP,
           DEBUG_STRING,
           "numS (%lu) != x (%lu)",
           numS,
           x);
    return;
  }

  if(reset) {
    for(x = 0;x < numS;x++) {
      if(!*pciSa[x]->szFileName || !*pciDa[x]->szFileName)
        continue;
      pciSa[x]->flags |= CNRITEM_EXISTS;
      pciDa[x]->flags |= CNRITEM_EXISTS;
      if(pciSa[x]->cbFile + pciSa[x]->easize >
         pciDa[x]->cbFile + pciDa[x]->easize) {
        pciSa[x]->flags |= CNRITEM_LARGER;
        pciDa[x]->flags |= CNRITEM_SMALLER;
      }
      else if(pciSa[x]->cbFile + pciSa[x]->easize <
              pciDa[x]->cbFile + pciDa[x]->easize) {
        pciSa[x]->flags |= CNRITEM_SMALLER;
        pciDa[x]->flags |= CNRITEM_LARGER;
      }
      if((pciSa[x]->date.year > pciDa[x]->date.year) ? TRUE :
         (pciSa[x]->date.year < pciDa[x]->date.year) ? FALSE :
         (pciSa[x]->date.month > pciDa[x]->date.month) ? TRUE :
         (pciSa[x]->date.month < pciDa[x]->date.month) ? FALSE :
         (pciSa[x]->date.day > pciDa[x]->date.day) ? TRUE :
         (pciSa[x]->date.day < pciDa[x]->date.day) ? FALSE :
         (pciSa[x]->time.hours > pciDa[x]->time.hours) ? TRUE :
         (pciSa[x]->time.hours < pciDa[x]->time.hours) ? FALSE :
         (pciSa[x]->time.minutes > pciDa[x]->time.minutes) ? TRUE :
         (pciSa[x]->time.minutes < pciDa[x]->time.minutes) ? FALSE :
         (pciSa[x]->time.seconds > pciDa[x]->time.seconds) ? TRUE :
         (pciSa[x]->time.seconds < pciDa[x]->time.seconds) ? FALSE : FALSE) {
        pciSa[x]->flags |= CNRITEM_NEWER;
        pciDa[x]->flags |= CNRITEM_OLDER;
      }
      else if((pciSa[x]->date.year < pciDa[x]->date.year) ? TRUE :
              (pciSa[x]->date.year > pciDa[x]->date.year) ? FALSE :
              (pciSa[x]->date.month < pciDa[x]->date.month) ? TRUE :
              (pciSa[x]->date.month > pciDa[x]->date.month) ? FALSE :
              (pciSa[x]->date.day < pciDa[x]->date.day) ? TRUE :
              (pciSa[x]->date.day > pciDa[x]->date.day) ? FALSE :
              (pciSa[x]->time.hours < pciDa[x]->time.hours) ? TRUE :
              (pciSa[x]->time.hours > pciDa[x]->time.hours) ? FALSE :
              (pciSa[x]->time.minutes < pciDa[x]->time.minutes) ? TRUE :
              (pciSa[x]->time.minutes > pciDa[x]->time.minutes) ? FALSE :
              (pciSa[x]->time.seconds < pciDa[x]->time.seconds) ? TRUE :
              (pciSa[x]->time.seconds > pciDa[x]->time.seconds) ? FALSE :
              FALSE) {
        pciSa[x]->flags |= CNRITEM_OLDER;
        pciDa[x]->flags |= CNRITEM_NEWER;
      }
      if(!(x % 500))
        DosSleep(1L);
      else if(!(x % 50))
        DosSleep(0L);
    }
  }

  switch(action) {
    case IDM_SELECTIDENTICAL:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED)) {
          if(*pciSa[x]->szFileName &&
             (pciSa[x]->flags & CNRITEM_EXISTS) &&
             !(pciSa[x]->flags & CNRITEM_SMALLER) &&
             !(pciSa[x]->flags & CNRITEM_LARGER) &&
             !(pciSa[x]->flags & CNRITEM_NEWER) &&
             !(pciSa[x]->flags & CNRITEM_OLDER)) {
            if(!(pciSa[x]->rc.flRecordAttr & CRA_SELECTED))
              WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                         MPFROM2SHORT(TRUE,CRA_SELECTED));
            if(!(pciDa[x]->rc.flRecordAttr & CRA_SELECTED))
              WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                         MPFROM2SHORT(TRUE,CRA_SELECTED));
          }
          if(!(x % 500))
            DosSleep(1L);
          else if(!(x % 50))
            DosSleep(0L);
        }
      }
      break;

    case IDM_SELECTSAME:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_EXISTS) &&
           !(pciSa[x]->flags & CNRITEM_SMALLER) &&
           !(pciSa[x]->flags & CNRITEM_LARGER)) {
          if(!(pciSa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
          if(!(pciDa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_SELECTBOTH:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_EXISTS)) {
          if(!(pciSa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
          if(!(pciDa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_SELECTONE:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           !(pciSa[x]->flags & CNRITEM_EXISTS)) {
          if(!(pciSa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        else if(*pciDa[x]->szFileName &&
                !(pciDa[x]->flags & CNRITEM_EXISTS)) {
          if(!(pciDa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_SELECTBIGGER:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_LARGER)) {
          if(!(pciSa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        else if(!(pciDa[x]->rc.flRecordAttr & CRA_FILTERED) &&
                *pciDa[x]->szFileName &&
                (pciDa[x]->flags & CNRITEM_LARGER)) {
          if(!(pciDa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_SELECTSMALLER:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_SMALLER)) {
          if(!(pciSa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        else if(!(pciDa[x]->rc.flRecordAttr & CRA_FILTERED) &&
                *pciDa[x]->szFileName &&
                (pciDa[x]->flags & CNRITEM_SMALLER)) {
          if(!(pciDa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_SELECTNEWER:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_NEWER)) {
          if(!(pciSa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        else if(!(pciDa[x]->rc.flRecordAttr & CRA_FILTERED) &&
                *pciDa[x]->szFileName &&
                (pciDa[x]->flags & CNRITEM_NEWER)) {
          if(!(pciDa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_SELECTOLDER:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_OLDER)) {
          if(!(pciSa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        else if(!(pciDa[x]->rc.flRecordAttr & CRA_FILTERED) &&
                *pciDa[x]->szFileName &&
                (pciDa[x]->flags & CNRITEM_OLDER)) {
          if(!(pciDa[x]->rc.flRecordAttr & CRA_SELECTED))
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_DESELECTBOTH:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_EXISTS)) {
          if(pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
          if(pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_DESELECTONE:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           !(pciSa[x]->flags & CNRITEM_EXISTS)) {
          if(pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        else if(*pciDa[x]->szFileName &&
                !(pciDa[x]->flags & CNRITEM_EXISTS)) {
          if(pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_DESELECTBIGGER:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_LARGER)) {
          if(pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        else if(!(pciDa[x]->rc.flRecordAttr & CRA_FILTERED) &&
                *pciDa[x]->szFileName &&
                (pciDa[x]->flags & CNRITEM_LARGER)) {
          if(pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_DESELECTSMALLER:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_SMALLER)) {
          if(pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        else if(!(pciDa[x]->rc.flRecordAttr & CRA_FILTERED) &&
                *pciDa[x]->szFileName &&
                (pciDa[x]->flags & CNRITEM_SMALLER)) {
          if(pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_DESELECTNEWER:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_NEWER)) {
          if(pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        else if(!(pciDa[x]->rc.flRecordAttr & CRA_FILTERED) &&
                *pciDa[x]->szFileName &&
                (pciDa[x]->flags & CNRITEM_NEWER)) {
          if(pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    case IDM_DESELECTOLDER:
      for(x = 0;x < numS;x++) {
        if(!(pciSa[x]->rc.flRecordAttr & CRA_FILTERED) &&
           *pciSa[x]->szFileName &&
           (pciSa[x]->flags & CNRITEM_OLDER)) {
          if(pciSa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrS,CM_SETRECORDEMPHASIS,MPFROMP(pciSa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        else if(!(pciDa[x]->rc.flRecordAttr & CRA_FILTERED) &&
                *pciDa[x]->szFileName &&
                (pciDa[x]->flags & CNRITEM_OLDER)) {
          if(pciDa[x]->rc.flRecordAttr & CRA_SELECTED)
            WinSendMsg(hwndCnrD,CM_SETRECORDEMPHASIS,MPFROMP(pciDa[x]),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        if(!(x % 500))
          DosSleep(1L);
        else if(!(x % 50))
          DosSleep(0L);
      }
      break;

    default:
      break;
  }

  if(reset) {
    while(numS) {
      WinSendMsg(hwndCnrS,CM_INVALIDATERECORD,
                 MPFROMP(pciSa),
                 MPFROM2SHORT((min(numS,65535)),0));
      DosSleep(0L);
      WinSendMsg(hwndCnrD,CM_INVALIDATERECORD,
                 MPFROMP(pciDa),
                 MPFROM2SHORT((min(numD,65535)),0));
      numS -= min(numS,65535);
      if(numS)
        DosSleep(0L);
    }
  }

  free(pciSa);
  free(pciDa);
  DosPostEventSem(CompactSem);
}


struct SS {
  PCNRITEM  pci;
  BOOL      unique,
            all,
            smallest,
            largest,
            newest,
            oldest;
};

struct Cnr {
  HWND       hwndCnr;
  ULONG      numfiles;
  struct SS *ss;
};


static int CompSSNamesB (const void *s1,const void *s2) {

  struct SS *ss2 = (struct SS *)s2;

  return stricmp((CHAR *)s1,ss2->pci->pszFileName);
}


static int CompSSNames (const void *s1,const void *s2) {

  struct SS *ss1 = (struct SS *)s1;
  struct SS *ss2 = (struct SS *)s2;

  return stricmp(ss1->pci->pszFileName,ss2->pci->pszFileName);
}


VOID FreeCnrs (struct Cnr *Cnrs,INT numw) {

  register INT z;

  for(z = 0;z < numw;z++) {
    if(Cnrs[z].ss)
      free(Cnrs[z].ss);
  }
  free(Cnrs);
  DosPostEventSem(CompactSem);
}


VOID SpecialSelect2 (HWND hwndParent,INT action) {

  PCNRITEM     pci;
  HENUM        henum;
  HWND         hwnd;
  register INT numwindows = 0,w,x,z,cmp;
  struct Cnr  *Cnrs = NULL;
  struct SS  *bsres;

  if(!hwndParent)
    return;

  /* count directory containers, build array of hwnds */
  henum = WinBeginEnumWindows(hwndParent);
  while((hwnd = WinGetNextWindow(henum)) != NULLHANDLE) {
    if(WinWindowFromID(WinWindowFromID(hwnd,FID_CLIENT),DIR_CNR)) {
      Cnrs = realloc(Cnrs,(numwindows + 1) * sizeof(struct Cnr));
      if(!Cnrs) {
        Notify(GetPString(IDS_OUTOFMEMORY));
        return;
      }
      memset(&Cnrs[numwindows],0,sizeof(struct Cnr));
      Cnrs[numwindows].hwndCnr = WinWindowFromID(WinWindowFromID(hwnd,
                                                 FID_CLIENT),DIR_CNR);
      numwindows++;
    }
  }
  WinEndEnumWindows(henum);
  if(numwindows < 2) {
    FreeCnrs(Cnrs,numwindows);
    DosBeep(250,100);
    Notify(GetPString(IDS_COMPSEL2ORMORETEXT));
    return;
  }
  if(numwindows > 4) {
    WinSendMsg(Cnrs[0].
               hwndCnr,
               UM_NOTIFY,
               MPFROMP(GetPString(IDS_BUILDINGLISTSTEXT)),
               MPVOID);
    DosSleep(0L);
  }

  /* count records, build array of pointers to records */
  for(z = 0;z < numwindows;z++) {
    pci = (PCNRITEM)WinSendMsg(Cnrs[z].hwndCnr,
                               CM_QUERYRECORD,
                               MPVOID,
                               MPFROM2SHORT(CMA_FIRST,
                                            CMA_ITEMORDER));
    x = 0;
    while(pci && (INT)pci != -1) {
      if(!(pci->rc.flRecordAttr & CRA_FILTERED) &&
         !(pci->attrFile & FILE_DIRECTORY)) {
        Cnrs[z].ss = realloc(Cnrs[z].ss,(x + 1) * sizeof(struct SS));
        if(!Cnrs[z].ss) {
          FreeCnrs(Cnrs,numwindows);
          Notify(GetPString(IDS_OUTOFMEMORY));
          return;
        }
        memset(&Cnrs[z].ss[x],0,sizeof(struct SS));
        Cnrs[z].ss[x].pci = pci;
        x++;
      }
      pci = (PCNRITEM)WinSendMsg(Cnrs[z].hwndCnr,
                                 CM_QUERYRECORD,
                                 MPFROMP(pci),
                                 MPFROM2SHORT(CMA_NEXT,
                                              CMA_ITEMORDER));
    }
    DosSleep(0L);
    Cnrs[z].numfiles = x;
    if(Cnrs[z].numfiles)
      qsort(Cnrs[z].ss,Cnrs[z].numfiles,sizeof(struct SS),CompSSNames);
  }

  for(z = 0;z < numwindows;z++) {
    for(x = 0;x < Cnrs[z].numfiles;x++) {
      Cnrs[z].ss[x].all = Cnrs[z].ss[x].unique = Cnrs[z].ss[x].newest =
        Cnrs[z].ss[x].oldest = Cnrs[z].ss[x].smallest =
        Cnrs[z].ss[x].largest = TRUE;
      for(w = 0;w < numwindows;w++) {
        if(w != z && Cnrs[w].numfiles) {
          bsres = (struct SS *)bsearch(Cnrs[z].ss[x].pci->pszFileName,
                                        Cnrs[w].ss,Cnrs[w].numfiles,
                                        sizeof(struct SS),CompSSNamesB);
          if(bsres) {
            Cnrs[z].ss[x].unique = FALSE;
            if(Cnrs[z].ss[x].pci->cbFile + Cnrs[z].ss[x].pci->easize >
               bsres->pci->cbFile + bsres->pci->easize)
              Cnrs[z].ss[x].smallest = FALSE;
            if(Cnrs[z].ss[x].pci->cbFile + Cnrs[z].ss[x].pci->easize <
               bsres->pci->cbFile + bsres->pci->easize)
              Cnrs[z].ss[x].largest = FALSE;
            cmp = (Cnrs[z].ss[x].pci->date.year > bsres->pci->date.year) ? TRUE :
                  (Cnrs[z].ss[x].pci->date.year < bsres->pci->date.year) ? FALSE :
                  (Cnrs[z].ss[x].pci->date.month > bsres->pci->date.month) ? TRUE :
                  (Cnrs[z].ss[x].pci->date.month < bsres->pci->date.month) ? FALSE :
                  (Cnrs[z].ss[x].pci->date.day > bsres->pci->date.day) ? TRUE :
                  (Cnrs[z].ss[x].pci->date.day < bsres->pci->date.day) ? FALSE :
                  (Cnrs[z].ss[x].pci->time.hours > bsres->pci->time.hours) ? TRUE :
                  (Cnrs[z].ss[x].pci->time.hours < bsres->pci->time.hours) ? FALSE :
                  (Cnrs[z].ss[x].pci->time.minutes > bsres->pci->time.minutes) ? TRUE :
                  (Cnrs[z].ss[x].pci->time.minutes < bsres->pci->time.minutes) ? FALSE :
                  (Cnrs[z].ss[x].pci->time.seconds > bsres->pci->time.seconds) ? TRUE :
                  (Cnrs[z].ss[x].pci->time.seconds < bsres->pci->time.seconds) ? FALSE :
                   FALSE;
            if(!cmp)
              Cnrs[z].ss[x].newest = FALSE;
            cmp = (Cnrs[z].ss[x].pci->date.year < bsres->pci->date.year) ? TRUE :
                  (Cnrs[z].ss[x].pci->date.year > bsres->pci->date.year) ? FALSE :
                  (Cnrs[z].ss[x].pci->date.month < bsres->pci->date.month) ? TRUE :
                  (Cnrs[z].ss[x].pci->date.month > bsres->pci->date.month) ? FALSE :
                  (Cnrs[z].ss[x].pci->date.day < bsres->pci->date.day) ? TRUE :
                  (Cnrs[z].ss[x].pci->date.day > bsres->pci->date.day) ? FALSE :
                  (Cnrs[z].ss[x].pci->time.hours < bsres->pci->time.hours) ? TRUE :
                  (Cnrs[z].ss[x].pci->time.hours > bsres->pci->time.hours) ? FALSE :
                  (Cnrs[z].ss[x].pci->time.minutes < bsres->pci->time.minutes) ? TRUE :
                  (Cnrs[z].ss[x].pci->time.minutes > bsres->pci->time.minutes) ? FALSE :
                  (Cnrs[z].ss[x].pci->time.seconds < bsres->pci->time.seconds) ? TRUE :
                  (Cnrs[z].ss[x].pci->time.seconds > bsres->pci->time.seconds) ? FALSE :
                   FALSE;
            if(!cmp)
              Cnrs[z].ss[x].oldest = FALSE;
            cmp = 0;
            break;
          }
          else
            Cnrs[z].ss[x].all = FALSE;
        }
      }
      if(Cnrs[z].ss[x].unique)
        Cnrs[z].ss[x].oldest = Cnrs[z].ss[x].newest = Cnrs[z].ss[x].all =
          Cnrs[z].ss[x].largest = Cnrs[z].ss[x].smallest = FALSE;
      DosSleep(0L);
    }
    DosSleep(1L);
  }

  switch(action) {
    case IDM_SELECTBOTH:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].all)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_SELECTMORE:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(!Cnrs[z].ss[x].unique)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_SELECTONE:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].unique)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_SELECTNEWER:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].newest)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_SELECTOLDER:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].oldest)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_SELECTBIGGER:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].largest)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_SELECTSMALLER:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].smallest)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(TRUE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;

    case IDM_DESELECTBOTH:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].all)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_DESELECTMORE:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(!Cnrs[z].ss[x].unique)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_DESELECTONE:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].unique)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_DESELECTNEWER:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].newest)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_DESELECTOLDER:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].oldest)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_DESELECTBIGGER:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].largest)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
    case IDM_DESELECTSMALLER:
      for(z = 0;z < numwindows;z++) {
        for(x = 0;x < Cnrs[z].numfiles;x++) {
          if(Cnrs[z].ss[x].smallest)
            WinSendMsg(Cnrs[z].hwndCnr,CM_SETRECORDEMPHASIS,
                       MPFROMP(Cnrs[z].ss[x].pci),
                       MPFROM2SHORT(FALSE,CRA_SELECTED));
        }
        DosSleep(0L);
      }
      break;
  }

  FreeCnrs(Cnrs,numwindows);
}
