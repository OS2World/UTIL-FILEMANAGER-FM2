
/***********************************************************************

  $Id$

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2004 Steven H.Levine

  Revisions	01 Aug 04 SHL - Rework lstrip/rstrip usage

***********************************************************************/

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"
#include "mle.h"

#pragma data_seg(DATA1)
#pragma alloc_text(EAS,DisplayEAsProc,SaveEA,HexDumpEA,CheckEA,AddEAProc)
#pragma alloc_text(EAS1,HexDump,GetFileEAs,Free_FEAList)

typedef struct {
  CHAR *name;
  INT   type;
} RESERVEDEAS;

typedef struct {
  CHAR    *filename;
  HOLDFEA *head;
} ADDEA;


HOLDFEA * CheckEA (HOLDFEA *head,CHAR *eaname) {

  /* return pointer to ea named eaname if found in linked list */

  register HOLDFEA *info = NULL;

  if(eaname && *eaname) {
    info = head;
    while(info) {
      if(!strcmp(info->name,eaname))
        return info;
      info = info->next;
    }
  }
  return info;
}


MRESULT EXPENTRY AddEAProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  ADDEA              *add;
  HOLDFEA            *head;
  CHAR               *filename;
  static CHAR        *forbidden[] = {".ASSOCTABLE",
                                     ".CLASSINFO",
                                     ".ICON",
                                     ".CODEPAGE",
                                     ""};
  static RESERVEDEAS restypes[] = {".TYPE",EAT_MVMT,
                                   ".SUBJECT",EAT_ASCII,
                                   ".COMMENTS",EAT_MVMT,
                                   ".KEYPHRASES",EAT_MVMT,
                                   ".HISTORY",EAT_MVMT,
                                   ".LONGNAME",EAT_ASCII,
                                   ".VERSION",EAT_ASCII,
                                   "",0};

  switch(msg) {
    case WM_INITDLG:
      if(!mp2) {
        WinDismissDlg(hwnd,0);
        break;
      }
      WinSetWindowPtr(hwnd,0,(PVOID)mp2);
      WinSendDlgItemMsg(hwnd,EAC_NAME,EM_SETTEXTLIMIT,
                        MPFROM2SHORT(255,0),MPVOID);
      WinCheckButton(hwnd,EAC_ASCII,TRUE);
      break;

    case WM_PAINT:
      PostMsg(hwnd,UM_PAINT,MPVOID,MPVOID);
      break;

    case UM_PAINT:
      PaintRecessedWindow(WinWindowFromID(hwnd,EAC_TEXT),(HPS)0,FALSE,FALSE);
      return 0;

    case WM_CONTROL:
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case DID_OK:
          add = INSTDATA(hwnd);
          head = add->head;
          filename = add->filename;
          {
            CHAR   s[257];
            INT    x;
            USHORT type = EAT_ASCII;

            *s = 0;
            WinQueryDlgItemText(hwnd,EAC_NAME,255,s);
            bstrip(s);
            if(!*s)
              WinDismissDlg(hwnd,0);
            else {
              if(CheckEA(head,s)) {
                DosBeep(50,100);
                WinSetDlgItemText(hwnd,EAC_TEXT,
                                  GetPString(IDS_EANAMEEXISTSTEXT));
                break;
              }
              for(x = 0;*forbidden[x];x++) {
                if(!strcmp(forbidden[x],s)) {
                  DosBeep(50,100);
                  WinSetDlgItemText(hwnd,EAC_TEXT,
                                    GetPString(IDS_EANAMERESERVEDTEXT));
                  return 0;
                }
              }
              if(WinQueryButtonCheckstate(hwnd,EAC_MVST))
                type = EAT_MVST;
              else if(WinQueryButtonCheckstate(hwnd,EAC_MVMT))
                type = EAT_MVMT;
              for(x = 0;*restypes[x].name;x++) {
                if(!strcmp(restypes[x].name,s)) {
                  if(type != restypes[x].type) {
                    DosBeep(50,100);
                    WinSetDlgItemText(hwnd,EAC_TEXT,
                                      GetPString(IDS_EAWRONGTYPETEXT));
                    return 0;
                  }
                  break;
                }
              }
              /* if we get here, create dummy ea */
              {
                PFEA2LIST pfealist = NULL;
                EAOP2     eaop;
                ULONG     ealen;
                CHAR     *eaval;

                ealen = sizeof(FEA2LIST) + strlen(s) + 64;
                if(!DosAllocMem((PPVOID)&pfealist,ealen + 1L,
                                 OBJ_TILE | PAG_COMMIT | PAG_READ | PAG_WRITE)) {
                  memset(pfealist,0,ealen + 1);
                  pfealist->cbList = ealen;
                  pfealist->list[0].oNextEntryOffset = 0L;
                  pfealist->list[0].fEA = 0;
                  pfealist->list[0].cbName = strlen(s);
                  strcpy(pfealist->list[0].szName,s);
                  eaval = pfealist->list[0].szName + strlen(s) + 1;
                  *(USHORT *)eaval = (USHORT)type;
                  eaval += sizeof(USHORT);
                  if(type == EAT_MVST || type == EAT_MVMT) {
                    *(USHORT *)eaval = (USHORT)0; /* codepage */
                    eaval += sizeof(USHORT);
                    *(USHORT *)eaval = (USHORT)1; /* number */
                    eaval += sizeof(USHORT);
                    *(USHORT *)eaval = (USHORT)EAT_ASCII; /* type */
                    eaval += sizeof(USHORT);
                  }
                  *(USHORT *)eaval = (USHORT)4;
                  eaval += sizeof(USHORT);
                  memcpy(eaval,GetPString(IDS_FAKETEXT),4);
                  pfealist->list[0].cbValue = 4 + (sizeof(USHORT) * 2) +
                                              ((type == EAT_MVST ||
                                                type == EAT_MVMT) ?
                                              sizeof(USHORT) * 3: 0);
                  eaop.fpGEA2List = (PGEA2LIST)0;
                  eaop.fpFEA2List = pfealist;
                  eaop.oError = 0L;
                  DosSetPathInfo(filename,FIL_QUERYEASIZE,(PVOID)&eaop,
                                 (ULONG)sizeof(EAOP2),DSPI_WRTTHRU);
                  WinDismissDlg(hwnd,1);
                }
              }
            }
          }
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_ADDEA,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;
      }
      return 0;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}

static VOID HexDumpEA (HWND hwnd,HOLDFEA *info) {

  if(info)
    HexDump(WinWindowFromID(hwnd,EA_HEXDUMP),info->value,info->cbValue);
}


VOID HexDump (HWND hwnd,CHAR *value,ULONG cbValue) {

  /* display a hexdump of a binary 'string' in listbox hwnd */

  CHAR           s[132];
  register CHAR *p,*pp,*a;
  register ULONG x = 0,y,z;

  WinSendMsg(hwnd,LM_DELETEALL,MPVOID,MPVOID);
  if(cbValue) {
    pp = p = value;
    while(x < cbValue) {
      y = x;
      sprintf(s,"%04lx  ",x);
      a = s + 6;
      do {
        sprintf(a,"%02hx ",*p);
        a += 3;
        p++;
        x++;
      } while(x < cbValue && (x % 16));
      if(x % 16) {
        z = x;
        while(z % 16) {
          *a++ = ' ';
          *a++ = ' ';
          *a++ = ' ';
          z++;
        }
      }
      *a++ = ' ';
      p = pp;
      do {
        if(*p)
          *a++ = *p++;
        else {
          *a++ = '.';
          p++;
        }
        *a = 0;
        y++;
      } while(y < x);
      if((SHORT)WinSendMsg(hwnd,LM_INSERTITEM,MPFROM2SHORT(LIT_END,0),
                           MPFROMP(s)) < 0)
        break;
      pp = p;
    }
  }
}


typedef struct {
  USHORT   size;
  USHORT   flags;
  HOLDFEA *head,*current;
  CHAR   **list;
  CHAR     filename[CCHMAXPATH];
} EAPROCDATA;


MRESULT EXPENTRY DisplayEAsProc (HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2) {

  EAPROCDATA     *eap;
  HOLDFEA        *info;
  static HPOINTER hptrIcon = (HPOINTER)0;

  if(msg != WM_INITDLG)
    eap = (EAPROCDATA *)WinQueryWindowPtr(hwnd,0);

  switch(msg) {
    case WM_INITDLG:
      if(!mp2) {
        WinDismissDlg(hwnd,0);
        break;
      }
      eap = malloc(sizeof(EAPROCDATA));
      if(!eap) {
        WinDismissDlg(hwnd,0);
        break;
      }
      hptrIcon = WinLoadPointer(HWND_DESKTOP,FM3ModHandle,EA_FRAME);
      WinDefDlgProc(hwnd,WM_SETICON,MPFROMLONG(hptrIcon),MPVOID);
      memset(eap,0,sizeof(EAPROCDATA));
      eap->size = sizeof(EAPROCDATA);
      eap->list = (CHAR **)mp2;
      WinSetWindowPtr(hwnd,0,(PVOID)eap);
      WinSendDlgItemMsg(hwnd,
                        EA_ENTRY,
                        EM_SETTEXTLIMIT,
                        MPFROM2SHORT(40,0),
                        MPVOID);
      MLEsetlimit(WinWindowFromID(hwnd,EA_MLE),
                  32767L);
      MLEsetformat(WinWindowFromID(hwnd,EA_MLE),
                   MLFIE_NOTRANS);
      {
        INT   x;
        SHORT sSelect;
        CHAR  s[CCHMAXPATH];

        for(x = 0;eap->list[x];x++) {
          if(DosQueryPathInfo(eap->list[x],
                              FIL_QUERYFULLNAME,
                              s,
                              sizeof(s)))
            strcpy(s,eap->list[x]);
          if(IsFile(s) != -1)
            WinSendDlgItemMsg(hwnd,
                              EA_NAMES,
                              LM_INSERTITEM,
                              MPFROM2SHORT(LIT_SORTASCENDING,0),
                              MPFROMP(s));
        }
        sSelect = (SHORT)WinSendDlgItemMsg(hwnd,
                                           EA_NAMES,
                                           LM_QUERYITEMCOUNT,
                                           MPVOID,
                                           MPVOID);
        if(sSelect > 0)
          WinSendDlgItemMsg(hwnd,
                            EA_NAMES,
                            LM_SELECTITEM,
                            MPFROM2SHORT(0,0),
                            MPFROMSHORT(TRUE));
        else
          WinDismissDlg(hwnd,0);
      }
      break;


    case UM_SETDIR:
      if(*eap->filename) {
        if(eap->head)
          Free_FEAList(eap->head);
        eap->head = GetFileEAs(eap->filename,
                               FALSE,
                               FALSE);
        if(!isalpha(*eap->filename) ||
           (driveflags[toupper(*eap->filename) - 'A'] & DRIVE_NOTWRITEABLE)) {
          WinEnableWindow(WinWindowFromID(hwnd,EA_CHANGE),FALSE);
          WinEnableWindow(WinWindowFromID(hwnd,EA_ADD),FALSE);
          WinEnableWindow(WinWindowFromID(hwnd,EA_DELETE),FALSE);
        }
        else {
          WinEnableWindow(WinWindowFromID(hwnd,EA_CHANGE),TRUE);
          WinEnableWindow(WinWindowFromID(hwnd,EA_ADD),TRUE);
          WinEnableWindow(WinWindowFromID(hwnd,EA_DELETE),TRUE);
        }
        WinSendMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
      }
      break;

    case UM_SETUP:
      WinSendDlgItemMsg(hwnd,EA_LISTBOX,LM_DELETEALL,MPVOID,MPVOID);
      WinShowWindow(WinWindowFromID(hwnd,EA_ENTRY),FALSE);
      WinSetDlgItemText(hwnd,EA_ENTRY,NullStr);
      WinShowWindow(WinWindowFromID(hwnd,EA_MLE),FALSE);
      MLEclearall(WinWindowFromID(hwnd,EA_MLE));
      WinShowWindow(WinWindowFromID(hwnd,EA_HEXDUMP),FALSE);
      WinSendDlgItemMsg(hwnd,EA_HEXDUMP,LM_DELETEALL,MPVOID,MPVOID);
      WinShowWindow(WinWindowFromID(hwnd,EA_CHANGE),FALSE);
      WinShowWindow(WinWindowFromID(hwnd,EA_DELETE),FALSE);
      eap->current = NULL;
      if(eap->head) {
        WinSetDlgItemText(hwnd,EA_TEXT,NullStr);
        info = eap->head;
        while(info) {
          WinSendDlgItemMsg(hwnd,EA_LISTBOX,LM_INSERTITEM,
                            MPFROM2SHORT(LIT_END,0),
                            MPFROMP(info->name));
          info = info->next;
        }
        WinSendDlgItemMsg(hwnd,EA_LISTBOX,LM_SELECTITEM,
                          MPFROM2SHORT(0,0),MPFROM2SHORT(TRUE,0));
      }
      else
        WinSetDlgItemText(hwnd,EA_TEXT,
                          GetPString(IDS_EANOEAS));
      return 0;

    case WM_PAINT:
      PostMsg(hwnd,UM_PAINT,MPVOID,MPVOID);
      break;

    case UM_PAINT:
      PaintRecessedWindow(WinWindowFromID(hwnd,EA_HELP),(HPS)0,FALSE,TRUE);
      PaintRecessedWindow(WinWindowFromID(hwnd,EA_TEXT),(HPS)0,FALSE,FALSE);
      return 0;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1)) {
        case EA_NAMES:
          switch(SHORT2FROMMP(mp1)) {
            case LN_SETFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,
                                GetPString(IDS_EAFILENAMESHELPTEXT));
              break;
            case LN_KILLFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,NullStr);
              break;
            case LN_ENTER:
            case LN_SELECT:
              {
                CHAR  s[CCHMAXPATH];
                SHORT sSelect;

                sSelect = (SHORT)WinSendDlgItemMsg(hwnd,EA_NAMES,
                                                   LM_QUERYSELECTION,
                                                   MPFROM2SHORT(LIT_FIRST,0),
                                                   MPVOID);
                if(sSelect >= 0) {
                  *s = 0;
                  WinSendDlgItemMsg(hwnd,EA_NAMES,LM_QUERYITEMTEXT,
                                    MPFROM2SHORT(sSelect,CCHMAXPATH),
                                    MPFROMP(s));
                  if(*s) {
                    strcpy(eap->filename,s);
                    if(SHORT2FROMMP(mp1) == LN_SELECT)
                      WinSendMsg(hwnd,UM_SETDIR,MPVOID,MPVOID);
                    else
                      QuickView(hwnd,eap->filename);
                  }
                }
              }
              break;
          }
          break;

        case EA_LISTBOX:
          switch(SHORT2FROMMP(mp1)) {
            case LN_SETFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,
                                GetPString(IDS_EATYPESHELPTEXT));
              break;
            case LN_KILLFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,NullStr);
              break;
            case LN_SELECT:
              {
                CHAR  s[257];
                SHORT sSelect;

                eap->current = NULL;
                if(eap->head) {
                  WinSetDlgItemText(hwnd,EA_TEXT,NullStr);
                  WinShowWindow(WinWindowFromID(hwnd,EA_ENTRY),FALSE);
                  WinSetDlgItemText(hwnd,EA_ENTRY,NullStr);
                  MLEclearall(WinWindowFromID(hwnd,EA_MLE));
                  WinShowWindow(WinWindowFromID(hwnd,EA_MLE),FALSE);
                  WinShowWindow(WinWindowFromID(hwnd,EA_CHANGE),FALSE);
                  WinShowWindow(WinWindowFromID(hwnd,EA_DELETE),FALSE);
                  WinShowWindow(WinWindowFromID(hwnd,EA_HEXDUMP),FALSE);
                  WinSendDlgItemMsg(hwnd,EA_HEXDUMP,LM_DELETEALL,
                                    MPVOID,MPVOID);
                  *s = 0;
                  sSelect = (USHORT)WinSendDlgItemMsg(hwnd,EA_LISTBOX,
                                    LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),
                                    MPVOID);
                  if(sSelect >= 0) {
                    WinSendDlgItemMsg(hwnd,EA_LISTBOX,LM_QUERYITEMTEXT,
                                      MPFROM2SHORT(sSelect,256),
                                      MPFROMP(s));
                    if(*s) {

                      USHORT len,codepage,num,type;
                      CHAR  *data,last = '\n',*linefeed = "\n";
                      BOOL   alltext;
                      IPT    pos = 0L;

                      info = eap->head;
                      while(info) {
                        if(!strcmp(s,info->name)) {
                          eap->current = info;
                          WinShowWindow(WinWindowFromID(hwnd,EA_DELETE),TRUE);
                          switch(*(USHORT *)info->value) {
                            case EAT_EA:
                            case EAT_ASCII:
                              if(!strcmp(info->name,SUBJECT))
                                WinSendDlgItemMsg(hwnd,EA_ENTRY,
                                                  EM_SETTEXTLIMIT,
                                                  MPFROM2SHORT(40,0),MPVOID);
                              else
                                WinSendDlgItemMsg(hwnd,EA_ENTRY,
                                                  EM_SETTEXTLIMIT,
                                                  MPFROM2SHORT(256,0),MPVOID);
                              WinSetDlgItemText(hwnd,EA_ENTRY,
                                                info->value +
                                                (sizeof(USHORT) * 2));
                              WinShowWindow(WinWindowFromID(hwnd,EA_ENTRY),
                                            TRUE);
                              WinEnableWindow(WinWindowFromID(hwnd,EA_CHANGE),
                                              FALSE);
                              WinShowWindow(WinWindowFromID(hwnd,EA_CHANGE),
                                            TRUE);
                              {
                                CHAR str[81];

                                sprintf(str,
                                        GetPString(IDS_DATAANDBYTESTEXT),
                                        (*(USHORT *)info->value == EAT_ASCII) ?
                                         GetPString(IDS_TEXTTEXT) :
                                         GetPString(IDS_EAREFTEXT),
                                        info->cbValue);
                                WinSetDlgItemText(hwnd,
                                                  EA_TEXT,
                                                  str);
                              }
                              break;
                            case EAT_MVST:
                              MLEclearall(WinWindowFromID(hwnd,EA_MLE));
                              codepage = *(USHORT *)(info->value +
                                         sizeof(USHORT));
                              num = *(USHORT *)(info->value +
                                    (sizeof(USHORT) * 2));
                              type = *(USHORT *)(info->value +
                                     (sizeof(USHORT) * 3));
                              if(type == EAT_ASCII) {
                                data = info->value + (sizeof(USHORT) * 4);
                                len = *(USHORT *)data;
                                data += sizeof(USHORT);
                                while((data - info->value) + len <=
                                      info->cbValue) {
                                  if(last != '\n') {
                                    WinSendDlgItemMsg(hwnd,
                                                      EA_MLE,
                                                      MLM_SETIMPORTEXPORT,
                                                      MPFROMP(linefeed),
                                                      MPFROMLONG(1L));
                                    WinSendDlgItemMsg(hwnd,
                                                      EA_MLE,
                                                      MLM_IMPORT,
                                                      MPFROMP(&pos),
                                                      MPFROMLONG(1L));
                                  }
                                  WinSendDlgItemMsg(hwnd,
                                                    EA_MLE,
                                                    MLM_SETIMPORTEXPORT,
                                                    MPFROMP(data),
                                                    MPFROMLONG((ULONG)len));
                                  WinSendDlgItemMsg(hwnd,
                                                    EA_MLE,
                                                    MLM_IMPORT,
                                                    MPFROMP(&pos),
                                                    MPFROMLONG((ULONG)len));
                                  data += len;
                                  last = *(data - 1);
                                  if(data - info->value >= info->cbValue)
                                    break;
                                  len = *(USHORT *)data;
                                  data += sizeof(USHORT);
                                }
                                WinShowWindow(WinWindowFromID(hwnd,EA_MLE),
                                              TRUE);
                                WinEnableWindow(WinWindowFromID(hwnd,EA_CHANGE),
                                                FALSE);
                                WinShowWindow(WinWindowFromID(hwnd,EA_CHANGE),
                                              TRUE);
                              }
                              else {
                                WinShowWindow(WinWindowFromID(hwnd,EA_MLE),
                                              FALSE);
                                HexDumpEA(hwnd,info);
                                WinShowWindow(WinWindowFromID(hwnd,EA_HEXDUMP),
                                              TRUE);
                              }
                              {
                                CHAR str[81];

                                sprintf(str,
                                        GetPString(IDS_MVSTTEXT),
                                        num,
                                        (num == 1) ?
                                         GetPString(IDS_YTEXT) :
                                         GetPString(IDS_IESTEXT),
                                        info->cbValue);
                                WinSetDlgItemText(hwnd,
                                                  EA_TEXT,
                                                  str);
                              }
                              break;
                            case EAT_MVMT:
                              MLEclearall(WinWindowFromID(hwnd,EA_MLE));
                              codepage = *(USHORT *)(info->value +
                                         sizeof(USHORT));
                              num = *(USHORT *)(info->value +
                                    (sizeof(USHORT) * 2));
                              data = info->value + (sizeof(USHORT) * 3);
                              type = *(USHORT *)data;
                              data += sizeof(USHORT);
                              len = *(USHORT *)data;
                              data += sizeof(USHORT);
                              alltext = TRUE;
                              while((data - info->value) - len <=
                                    info->cbValue) {
                                if(type != EAT_ASCII) {
                                  alltext = FALSE;
                                  break;
                                }
                                data += len;
                                if(data - info->value >= info->cbValue)
                                  break;
                                type = *(USHORT *)data;
                                data += sizeof(USHORT);
                                len = *(USHORT *)data;
                                data += sizeof(USHORT);
                              }
                              if(alltext) {
                                data = info->value + (sizeof(USHORT) * 3);
                                type = *(USHORT *)data;
                                data += sizeof(USHORT);
                                len = *(USHORT *)data;
                                data += sizeof(USHORT);
                                while((data - info->value) - len <=
                                      info->cbValue) {
                                  if(last != '\n') {
                                    WinSendDlgItemMsg(hwnd,
                                                      EA_MLE,
                                                      MLM_SETIMPORTEXPORT,
                                                      MPFROMP(linefeed),
                                                      MPFROMLONG(1L));
                                    WinSendDlgItemMsg(hwnd,
                                                      EA_MLE,
                                                      MLM_IMPORT,
                                                      MPFROMP(&pos),
                                                      MPFROMLONG(1L));
                                  }
                                  WinSendDlgItemMsg(hwnd,
                                                    EA_MLE,
                                                    MLM_SETIMPORTEXPORT,
                                                    MPFROMP(data),
                                                    MPFROMLONG((ULONG)len));
                                  WinSendDlgItemMsg(hwnd,
                                                    EA_MLE,
                                                    MLM_IMPORT,
                                                    MPFROMP(&pos),
                                                    MPFROMLONG((ULONG)len));
                                  data += len;
                                  last = *(data - 1);
                                  if(data - info->value >= info->cbValue)
                                    break;
                                  type = *(USHORT *)data;
                                  data += sizeof(USHORT);
                                  len = *(USHORT *)data;
                                  data += sizeof(USHORT);
                                }
                              }
                              if(alltext) {
                                WinShowWindow(WinWindowFromID(hwnd,EA_MLE),
                                              TRUE);
                                WinEnableWindow(WinWindowFromID(hwnd,
                                                EA_CHANGE),FALSE);
                                WinShowWindow(WinWindowFromID(hwnd,EA_CHANGE),
                                              TRUE);
                              }
                              else {
                                WinShowWindow(WinWindowFromID(hwnd,EA_MLE),
                                              FALSE);
                                HexDumpEA(hwnd,info);
                                WinShowWindow(WinWindowFromID(hwnd,EA_HEXDUMP),
                                              TRUE);
                              }
                              {
                                CHAR str[81];

                                sprintf(str,
                                        GetPString(IDS_MVMTTEXT),
                                        num,
                                        (num == 1) ?
                                         GetPString(IDS_YTEXT) :
                                         GetPString(IDS_IESTEXT),
                                        info->cbValue,
                                        (alltext) ?
                                         GetPString(IDS_ALLTEXTTEXT) :
                                         GetPString(IDS_MIXEDTYPESTEXT));
                                WinSetDlgItemText(hwnd,
                                                  EA_TEXT,
                                                  str);
                              }
                              break;
                            default:
                              HexDumpEA(hwnd,info);
                              WinShowWindow(WinWindowFromID(hwnd,EA_HEXDUMP),
                                            TRUE);
                              switch(*(USHORT *)info->value) {
                                case EAT_BINARY:
                                  {
                                    CHAR str[81];

                                    sprintf(str,
                                            GetPString(IDS_BINARYBYTESTEXT),
                                            info->cbValue);
                                    WinSetDlgItemText(hwnd,
                                                      EA_TEXT,
                                                      str);
                                  }
                                  break;
                                case EAT_BITMAP:
                                  {
                                    CHAR str[81];

                                    sprintf(str,
                                            GetPString(IDS_BITMAPBYTESTEXT),
                                            info->cbValue);
                                    WinSetDlgItemText(hwnd,
                                                      EA_TEXT,
                                                      str);
                                  }
                                  break;
                                case EAT_METAFILE:
                                  {
                                    CHAR str[81];

                                    sprintf(str,
                                            GetPString(IDS_METAFILEBYTESTEXT),
                                            info->cbValue);
                                    WinSetDlgItemText(hwnd,
                                                      EA_TEXT,
                                                      str);
                                  }
                                  break;
                                case EAT_ICON:
                                  {
                                    CHAR str[81];

                                    sprintf(str,
                                            GetPString(IDS_ICONBYTESTEXT),
                                            info->cbValue);
                                    WinSetDlgItemText(hwnd,
                                                      EA_TEXT,
                                                      str);
                                  }
                                  break;
                                case EAT_ASN1:
                                  {
                                    CHAR str[81];

                                    sprintf(str,
                                            GetPString(IDS_ASN1BYTESTEXT),
                                            info->cbValue);
                                    WinSetDlgItemText(hwnd,
                                                      EA_TEXT,
                                                      str);
                                  }
                                  break;
                                default:
                                  {
                                    CHAR str[81];

                                    sprintf(str,
                                            GetPString(IDS_UNKNOWNBYTESTEXT),
                                            *(USHORT *)info->value,
                                            info->cbValue);
                                    WinSetDlgItemText(hwnd,
                                                      EA_TEXT,
                                                      str);
                                  }
                                  break;
                              }
                              break;
                          }
                        }
                        info = info->next;
                      }
                    }
                  }
                }
                if(!isalpha(*eap->filename) ||
                   (driveflags[toupper(*eap->filename) - 'A'] & DRIVE_NOTWRITEABLE)) {
                  WinEnableWindow(WinWindowFromID(hwnd,EA_CHANGE),FALSE);
                  WinEnableWindow(WinWindowFromID(hwnd,EA_ADD),FALSE);
                  WinEnableWindow(WinWindowFromID(hwnd,EA_DELETE),FALSE);
                }
                else {
                  WinEnableWindow(WinWindowFromID(hwnd,EA_CHANGE),TRUE);
                  WinEnableWindow(WinWindowFromID(hwnd,EA_ADD),TRUE);
                  WinEnableWindow(WinWindowFromID(hwnd,EA_DELETE),TRUE);
                }
              }
              break;
          }
          break;

        case EA_ENTRY:
          switch(SHORT2FROMMP(mp1)) {
            case EN_SETFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,
                                GetPString(IDS_EADATAHELPTEXT));
              break;
            case EN_KILLFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,NullStr);
              break;
            case EN_CHANGE:
              WinEnableWindow(WinWindowFromID(hwnd,EA_CHANGE),TRUE);
              break;
          }
          break;

        case EA_HEXDUMP:
          switch(SHORT2FROMMP(mp1)) {
            case LN_SETFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,
                                GetPString(IDS_EADATAHELPTEXT));
              break;
            case LN_KILLFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,NullStr);
              break;
          }
          break;

        case EA_MLE:
          switch(SHORT2FROMMP(mp1)) {
            case MLN_SETFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,
                                GetPString(IDS_EADATAHELPTEXT));
              break;
            case MLN_KILLFOCUS:
              WinSetDlgItemText(hwnd,EA_HELP,NullStr);
              break;
            case MLN_CHANGE:
              WinEnableWindow(WinWindowFromID(hwnd,EA_CHANGE),TRUE);
              break;
          }
          break;
      }
      return 0;

    case WM_COMMAND:
      switch(SHORT1FROMMP(mp1)) {
        case EA_ADD:
          {
            ADDEA add;

            add.filename = eap->filename;
            add.head = eap->head;
            if(WinDlgBox(HWND_DESKTOP,hwnd,AddEAProc,FM3ModHandle,
                         EAC_FRAME,&add)) {
              Free_FEAList(eap->head);
              eap->head = GetFileEAs(eap->filename,FALSE,FALSE);
              WinSendMsg(hwnd,UM_SETUP,MPVOID,MPVOID);
            }
          }
          break;
        case EA_CHANGE:
          if(!eap->current)
            WinShowWindow(WinWindowFromID(hwnd,EA_CHANGE),FALSE);
          else {

            CHAR  *s;
            USHORT control;

            if(eap->head && *eap->filename) {
              switch(*(USHORT *)eap->current->value) {
                case EAT_EA:
                case EAT_ASCII:
                  control = EA_ENTRY;
                  break;
                case EAT_MVMT:
                  control = EA_MLE;
                  break;
                case EAT_MVST:
                  control = EA_MLE;
                  break;
                default:
                  DosBeep(250,100);
                  WinShowWindow(WinWindowFromID(hwnd,EA_CHANGE),FALSE);
                  control = 0;
              }
              if(control) {
                s = malloc(32768);
                if(s) {
                  *s = 0;
                  WinQueryDlgItemText(hwnd,control,32767L,(PCH)s);
                  if(*s) {

                    PFEA2LIST pfealist;

                    pfealist = SaveEA(eap->filename,eap->current,s,FALSE);
                    if(pfealist) {

                      PFEA2 pfea;

                      pfea = malloc(pfealist->cbList);
                      if(pfea) {
                        memcpy(pfea,pfealist->list,
                               pfealist->cbList - sizeof(ULONG));
                        free(eap->current->pfea);
                        eap->current->pfea = pfea;
                        eap->current->name = eap->current->pfea->szName;
                        eap->current->cbName = eap->current->pfea->cbName;
                        eap->current->cbValue = eap->current->pfea->cbValue;
                        eap->current->value = eap->current->pfea->szName +
                                         eap->current->pfea->cbName + 1;
                        eap->current->value[eap->current->cbValue] = 0;
                        PostMsg(hwnd,WM_CONTROL,
                                   MPFROM2SHORT(EA_LISTBOX,LN_SELECT),
                                   MPVOID);
                      }
                      DosFreeMem(pfealist);
                    }
                    else
                      DosBeep(250,100);
                  }
                  else
                    DosBeep(500,100);
                  free(s);
                }
                else
                  DosBeep(100,100);
              }
            }
            else
              DosBeep(50,100);
          }
          break;

        case EA_DELETE:
          if(eap->head && eap->current) {

            EAOP2     eaop;
            PFEA2LIST pfealist;
            GEA2LIST  gealist;
            APIRET    rc;
            SHORT     sSelect;

            pfealist = malloc(sizeof(FEA2LIST) + eap->current->cbName + 1);
            if(pfealist) {
              memset(pfealist,0,sizeof(FEA2LIST) + eap->current->cbName + 1);
              pfealist->cbList = sizeof(FEA2LIST) + eap->current->cbName + 1;
              pfealist->list[0].cbName = eap->current->cbName;
              strcpy(pfealist->list[0].szName,eap->current->name);
              pfealist->list[0].cbValue = 0;
              memset(&gealist,0,sizeof(GEA2LIST));
              gealist.cbList = sizeof(GEA2LIST);
              eaop.fpGEA2List = &gealist;
              eaop.fpFEA2List = pfealist;
              eaop.oError = 0L;
              rc = DosSetPathInfo(eap->filename,FIL_QUERYEASIZE,(PVOID)&eaop,
                                  (ULONG)sizeof(EAOP2),DSPI_WRTTHRU);
              free(pfealist);
              if(!rc) {
                sSelect = 0;
                if(eap->current == eap->head) {
                  eap->head = eap->head->next;
                  free(eap->current->pfea);
                  free(eap->current);
                  eap->current = NULL;
                }
                else {
                  info = eap->head;
                  while(info) {
                    if(info->next == eap->current) {
                      sSelect++;
                      info->next = eap->current->next;
                      free(eap->current->pfea);
                      free(eap->current);
                      eap->current = NULL;
                      break;
                    }
                    sSelect++;
                    info = info->next;
                  }
                }
                WinSendDlgItemMsg(hwnd,EA_LISTBOX,LM_DELETEITEM,
                                  MPFROM2SHORT(sSelect,0),MPVOID);
                WinShowWindow(WinWindowFromID(hwnd,EA_ENTRY),FALSE);
                WinShowWindow(WinWindowFromID(hwnd,EA_MLE),FALSE);
                WinShowWindow(WinWindowFromID(hwnd,EA_CHANGE),FALSE);
                WinShowWindow(WinWindowFromID(hwnd,EA_DELETE),FALSE);
                WinShowWindow(WinWindowFromID(hwnd,EA_HEXDUMP),FALSE);
                WinSetDlgItemText(hwnd,EA_ENTRY,NullStr);
                MLEclearall(WinWindowFromID(hwnd,EA_MLE));
                WinSendDlgItemMsg(hwnd,EA_HEXDUMP,LM_DELETEALL,
                                  MPVOID,MPVOID);
                if(sSelect && (SHORT)WinSendDlgItemMsg(hwnd,EA_LISTBOX,
                                                       LM_QUERYITEMCOUNT,
                                                       MPVOID,MPVOID) <=
                                                         sSelect)
                  sSelect--;
                WinSendDlgItemMsg(hwnd,EA_LISTBOX,LM_SELECTITEM,
                                  MPFROM2SHORT(sSelect,0),
                                  MPFROM2SHORT(TRUE,0));
              }
              else
                DosBeep(50,100);
            }
          }
          if(!eap->head)
            WinSetDlgItemText(hwnd,EA_TEXT,GetPString(IDS_EANOEAS));
          break;

        case IDM_HELP:
          if(hwndHelp)
            WinSendMsg(hwndHelp,HM_DISPLAY_HELP,
                       MPFROM2SHORT(HELP_EAS,0),
                       MPFROMSHORT(HM_RESOURCEID));
          break;

        case DID_OK:
          WinDismissDlg(hwnd,1);
          break;

        case DID_CANCEL:
          WinDismissDlg(hwnd,0);
          break;
      }
      return 0;

    case WM_CLOSE:
      break;

    case WM_DESTROY:
      if(eap) {
        if(eap->head)
          Free_FEAList(eap->head);
        free(eap);
        if(hptrIcon)
          WinDestroyPointer(hptrIcon);
        hptrIcon = (HPOINTER)0;
      }
      break;
  }
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
}


PVOID SaveEA (CHAR *filename,HOLDFEA *current,CHAR *newdata,
              BOOL silentfail) {

  /* save an ea to disk */

  PFEA2LIST pfealist = NULL;
  EAOP2     eaop;
  APIRET    rc;
  ULONG     ealen;
  USHORT    len,*num,*plen;
  CHAR     *p,*eaval;

  if(!filename || !current)
    return (PVOID)pfealist;
  len = strlen(newdata);
  ealen = sizeof(FEA2LIST) + 24L + (ULONG)current->cbName + 1L +
          (ULONG)len + 4L;
  switch(*(USHORT *)current->value) {
    case EAT_EA:
    case EAT_ASCII:
      break;
    case EAT_MVST:
      ealen += sizeof(USHORT) * 5;
      p = newdata;
      while(*p == '\n')
        p++;
      while(*p) {
        if(*p == '\n' && *(p + 1))
          ealen += sizeof(USHORT);
        p++;
      }
      break;
    case EAT_MVMT:
      ealen += sizeof(USHORT) * 5;
      p = newdata;
      while(*p == '\n')
        p++;
      while(*p) {
        if(*p == '\n' && *(p + 1))
          ealen += (sizeof(USHORT) * 2);
        p++;
      }
      break;
    default:
      return (PVOID)pfealist;
  }

  if(!DosAllocMem((PPVOID)&pfealist,ealen,
                  OBJ_TILE | PAG_COMMIT | PAG_READ | PAG_WRITE)) {
    memset(pfealist,0,ealen);
    pfealist->list[0].oNextEntryOffset = 0L;
    pfealist->list[0].fEA = 0; //current->fEA;
    pfealist->list[0].cbName = current->cbName;
    memcpy(pfealist->list[0].szName,current->name,pfealist->list[0].cbName + 1);
    eaval = pfealist->list[0].szName + pfealist->list[0].cbName + 1;
    switch(*(USHORT *)current->value) {
      case EAT_EA:
      case EAT_ASCII:
        *(USHORT *)eaval = *(USHORT *)current->value;
        eaval += sizeof(USHORT);
        *(USHORT *)eaval = (USHORT)len;
        eaval += sizeof(USHORT);
        memcpy(eaval,newdata,len);
        eaval += len;
        break;
      case EAT_MVST:
        *(USHORT *)eaval = (USHORT)EAT_MVST;
        eaval += sizeof(USHORT);
        *(USHORT *)eaval = *(USHORT *)(current->value + sizeof(USHORT));
        eaval += sizeof(USHORT);
        num = (USHORT *)eaval;
        *num = 0;
        eaval += sizeof(USHORT);
        *(USHORT *)eaval = (USHORT)EAT_ASCII;
        eaval += sizeof(USHORT);
        plen = (USHORT *)eaval;
        *plen = 0;
        eaval += sizeof(USHORT);
        p = newdata;
        while(*p == '\n')
          p++;
        while(*p) {
          while(*p) {
            if(*p == '\n')
              p++;
            *eaval++ = *p++;
            (*plen)++;
          }
          if(*p || *plen)
            (*num)++;
          if(*p) {
            plen = (USHORT *)eaval;
            *plen = 0;
            eaval += sizeof(USHORT);
          }
        }
        break;
/*
        cbList      nextoffset fea cb cbval name.......
000000  3C 00 00 00 00 00 00 00 6F 0B 24 00 2E 4B 45 59  <       o$ .KEY
        ....................    eat   code  num   eat
000010  50 48 52 41 53 45 53 00 DF FF 00 00 02 00 FD FF  PHRASES ßÿ   ýÿ
        len.. phrase1............................ eat
000020  0C 00 4B 65 79 20 70 68 72 61 73 65 20 31 FD FF   Key phrase 1ýÿ
        len.. phrase2......................
000030  0A 00 4B 65 79 20 70 68 72 61 73 65               Key phrase
*/
      case EAT_MVMT:
        *(USHORT *)eaval = (USHORT)EAT_MVMT;
        eaval += sizeof(USHORT);
        *(USHORT *)eaval = *(USHORT *)(current->value + sizeof(USHORT));
        eaval += sizeof(USHORT);
        num = (USHORT *)eaval;
        *num = 0;
        eaval += sizeof(USHORT);
        *(USHORT *)eaval = (USHORT)EAT_ASCII;
        eaval += sizeof(USHORT);
        plen = (USHORT *)eaval;
        *plen = 0;
        eaval += sizeof(USHORT);
        p = newdata;
        while(*p == '\n')
          p++;
        while(*p) {
          while(*p) {
            if(*p == '\n')
              p++;
            *eaval++ = *p++;
            (*plen)++;
          }
          if(*p || *plen)
            (*num)++;
          if(*p) {
            *(USHORT *)eaval = (USHORT)EAT_ASCII;
            eaval += sizeof(USHORT);
            plen = (USHORT *)eaval;
            *plen = 0;
            eaval += sizeof(USHORT);
          }
        }
        break;
    }
    pfealist->list[0].cbValue = (ULONG)(eaval -
                                (pfealist->list[0].szName +
                                 pfealist->list[0].cbName + 1));
    eaop.fpGEA2List = (PGEA2LIST)0;
    eaop.fpFEA2List = pfealist;
    eaop.oError = 0L;
    pfealist->cbList = 13L + (ULONG)pfealist->list[0].cbName +
                       (ULONG)pfealist->list[0].cbValue;

/*
{
FILE *fp;

fp = fopen("test","wb");
if(fp) {
fwrite(pfealist,pfealist->cbList,1,fp);
fclose(fp);
}
}
*/

    rc = DosSetPathInfo(filename,FIL_QUERYEASIZE,(PVOID)&eaop,
                        (ULONG)sizeof(EAOP2),DSPI_WRTTHRU);
    if(rc) {
      DosFreeMem(pfealist);
      pfealist = NULL;
    }
    if(rc && !silentfail) {
      if(rc == ERROR_ACCESS_DENIED || rc == ERROR_SHARING_VIOLATION)
        saymsg(MB_ENTER,
               HWND_DESKTOP,
               GetPString(IDS_OOPSTEXT),
               GetPString(IDS_CANTWRITEEATEXT),
               current->name,
               filename);
      else
        Dos_Error(MB_ENTER,
                  rc,
                  HWND_DESKTOP,
                  __FILE__,
                  __LINE__,
                  GetPString(IDS_ERRORWRITEEATEXT),
                  current->name,
                  filename,
                  eaop.oError);
    }
  }
  return (PVOID)pfealist;
}


HOLDFEA *GetFileEAs (CHAR *filename,BOOL ishandle,BOOL silentfail) {

  /* load eas from disk into HOLDFEA linked list */

  HOLDFEA    *head = NULL,*info,*last = NULL;
  FILESTATUS4 fsa4;
  HFILE       handle;
  ULONG       action;
  APIRET      rc;

  if(!filename)
    return head;
  if(ishandle || !DosOpen(filename,&handle,&action,0L,0,
                          OPEN_ACTION_FAIL_IF_NEW |
                          OPEN_ACTION_OPEN_IF_EXISTS,
                          OPEN_FLAGS_NOINHERIT |
                          OPEN_SHARE_DENYREADWRITE |
                          OPEN_ACCESS_READWRITE,(PEAOP2)0)) {
    if(ishandle)
      handle = *(HFILE *)filename;
    if(!DosQueryFileInfo(handle,FIL_QUERYEASIZE,(PVOID)&fsa4,
                         (ULONG)sizeof(fsa4)) && fsa4.cbList > 4L) {

      PDENA2    pdena;
      EAOP2     eaop;
      PGEA2LIST pgealist;
      PFEA2LIST pfealist;
      PGEA2     pgea;
      ULONG     x = 1L,ecnt = 1L;

      pdena = malloc(65536 + 1024);
      if(pdena) {
        while(!DosEnumAttribute(ENUMEA_REFTYPE_FHANDLE,&handle,x,(PVOID)pdena,
                                (ULONG)65536L,&ecnt,ENUMEA_LEVEL_NO_VALUE) &&
              ecnt) {
          pgealist = malloc(64 + pdena->cbName);
          if(pgealist) {
            pgealist->cbList = 64 + pdena->cbName;
            pgea = pgealist->list;
            pgea->oNextEntryOffset = 0L;
            pgea->cbName = pdena->cbName;
            memcpy(pgea->szName,pdena->szName,pdena->cbName + 1);
            pfealist = malloc(64 + pdena->cbName + pdena->cbValue);
            if(pfealist) {
              memset(pfealist,0,64 + pdena->cbName + pdena->cbValue);
              pfealist->cbList = 64 + pdena->cbName + pdena->cbValue;
              eaop.fpGEA2List = pgealist;
              eaop.fpFEA2List = pfealist;
              eaop.oError = 0L;
// saymsg(MB_ENTER,HWND_DESKTOP,"Debug1","\"%s\" %ld",pdena->szName,x);
              rc = DosQueryFileInfo(handle,FIL_QUERYEASFROMLIST,(PVOID)&eaop,
                                    (ULONG)sizeof(EAOP2));
              if(!rc) {
                info = malloc(sizeof(HOLDFEA));
                if(info) {
                  info->pfea = malloc(eaop.fpFEA2List->cbList - sizeof(ULONG));
                  memcpy(info->pfea,eaop.fpFEA2List->list,
                         eaop.fpFEA2List->cbList - sizeof(ULONG));
                  info->name = info->pfea->szName;
                  info->cbName = info->pfea->cbName;
                  info->cbValue = info->pfea->cbValue;
                  info->value = info->pfea->szName + info->pfea->cbName + 1;
                  info->value[info->cbValue] = 0;
                  info->next = NULL;
                  if(!head)
                    head = info;
                  else
                    last->next = info;
                  last = info;
                }
              }
              else {
                if(!silentfail)
                  Dos_Error(MB_ENTER,
                            rc,
                            HWND_DESKTOP,
                            __FILE__,
                            __LINE__,
                            GetPString(IDS_ERRORREADEATEXT),
                            pdena->szName);
              }
              free(pfealist);
            }
            free(pgealist);
          }
          x += ecnt;
        }
        free(pdena);
        DosPostEventSem(CompactSem);
      }
    }
    if(!ishandle)
      DosClose(handle);
  }
  else {  /* try it without opening it */
    if(!DosQueryPathInfo(filename,FIL_QUERYEASIZE,(PVOID)&fsa4,
                         (ULONG)sizeof(fsa4)) && fsa4.cbList > 4L) {

      PDENA2    pdena;
      EAOP2     eaop;
      PGEA2LIST pgealist;
      PFEA2LIST pfealist;
      PGEA2     pgea;
      ULONG     x = 1L,ecnt = 1L;

      pdena = malloc(65536 + 1024);
      if(pdena) {
        while(!DosEnumAttribute(ENUMEA_REFTYPE_PATH,filename,x,(PVOID)pdena,
                                (ULONG)65536L,&ecnt,ENUMEA_LEVEL_NO_VALUE) &&
              ecnt) {
          pgealist = malloc(64 + pdena->cbName);
          if(pgealist) {
            pgealist->cbList = 64 + pdena->cbName;
            pgea = pgealist->list;
            pgea->oNextEntryOffset = 0L;
            pgea->cbName = pdena->cbName;
            memcpy(pgea->szName,pdena->szName,pdena->cbName + 1);
            pfealist = malloc(64 + pdena->cbName + pdena->cbValue);
            if(pfealist) {
              memset(pfealist,0,64 + pdena->cbName + pdena->cbValue);
              pfealist->cbList = 64 + pdena->cbName + pdena->cbValue;
              eaop.fpGEA2List = pgealist;
              eaop.fpFEA2List = pfealist;
              eaop.oError = 0L;
// saymsg(MB_ENTER,HWND_DESKTOP,"Debug2","\"%s\" %ld",pdena->szName,x);
              rc = DosQueryPathInfo(filename,FIL_QUERYEASFROMLIST,
                                    (PVOID)&eaop,
                                    (ULONG)sizeof(EAOP2));
              if(!rc) {
                info = malloc(sizeof(HOLDFEA));
                if(info) {
                  info->pfea = malloc(eaop.fpFEA2List->cbList - sizeof(ULONG));
                  memcpy(info->pfea,eaop.fpFEA2List->list,
                         eaop.fpFEA2List->cbList - sizeof(ULONG));
                  info->name = info->pfea->szName;
                  info->cbName = info->pfea->cbName;
                  info->cbValue = info->pfea->cbValue;
                  info->value = info->pfea->szName + info->pfea->cbName + 1;
                  info->value[info->cbValue] = 0;
                  info->next = NULL;
                  if(!head)
                    head = info;
                  else
                    last->next = info;
                  last = info;
                }
                else
                  free(pfealist);
              }
              else {
                free(pfealist);
                if(!silentfail) {
                  if(rc == ERROR_ACCESS_DENIED || rc == ERROR_SHARING_VIOLATION) {
                    rc = saymsg(MB_ENTER | MB_CANCEL,
                                HWND_DESKTOP,
                                GetPString(IDS_OOPSTEXT),
                                GetPString(IDS_CANTREADEATEXT),
                                filename,
                                pdena->szName);
                    if(rc == MBID_CANCEL) {
                      free(pgealist);
                      break;
                    }
                  }
                  else
                    Dos_Error(MB_ENTER,
                              rc,
                              HWND_DESKTOP,
                              __FILE__,
                              __LINE__,
                              GetPString(IDS_ERRORREADEATEXT),
                              pdena->szName);
                }
              }
            }
            free(pgealist);
          }
          x += ecnt;
        }
        free(pdena);
        DosPostEventSem(CompactSem);
      }
    }
  }
  return head;
}


VOID Free_FEAList (HOLDFEA *pFEA) {

  /* free a linked list of HOLDFEAs */

  register HOLDFEA *next;

  while(pFEA) {   /* Free linked list */
    next = pFEA->next;
    free(pFEA->pfea);
    free(pFEA);
    pFEA = next;
  }
  DosPostEventSem(CompactSem);
}

