
/***********************************************************************

  $Id$

  Info window

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2005 Steven H. Levine

  12 Feb 03 SHL insert_grepfile: standardize EA math
  12 Feb 03 SHL doonefile: standardize EA math
  25 May 05 SHL Rework for ULONGLONG
  25 May 05 SHL Rework for FillInRecordFromFFB
  06 Jun 05 SHL Drop unused code
  24 Oct 05 SHL dononefile: do not free EA list twice

***********************************************************************/

#define INCL_DOS
#define INCL_WIN
#define INCL_LONGLONG
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <share.h>

#include "fm3dll.h"
#include "fm3str.h"
#include "grep.h"

#pragma data_seg(DATA2)
#pragma alloc_text(GREP,SecsSince1980,match,mmatch,dogrep)
#pragma alloc_text(GREP,doallsubdirs,domatchingfiles)

/*****************************/
/*   Function Prototypes     */
/*****************************/

static VOID  doallsubdirs    (GREP *grep,CHAR *searchPath,BOOL recursing,
                              char **fle,int numfls);
static INT   domatchingfiles (GREP *grep,CHAR *path,char **fle,int numfls);
static BOOL  doonefile       (GREP *grep,CHAR *fileName,FILEFINDBUF4 *f);
static BOOL  doinsertion     (GREP *grep);
static BOOL  InsertDupe      (GREP *grep,CHAR *dir,FILEFINDBUF4 *f);
static VOID  FillDupes       (GREP *g);
static VOID  FreeDupes       (GREP *g);

#define GREPCHARS "*?[] \\"

#define isleap(year) ((((year%4)==0) && ((year%100)!=0)) || \
        ((year%400)==0))


static INT monthdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};


ULONG SecsSince1980 (FDATE *date,FTIME *time)
{
  ULONG        total = 0L;
  register int x;

  for(x = 1980;x < date->year + 1980;x++) {
    if(isleap(x))
      total += (366L * (24L * 60L * 60L));
    else
      total += (365L * (24L * 60L * 60L));
  }
  for(x = 1;x < date->month;x++) {
    if(x == 2 && isleap(date->year + 1980))
      total += (29L * (24L * 60L * 60L));
    else
      total += ((long)monthdays[x - 1] * (24L * 60L * 60L));
  }
  total += (((long)date->day - 1L) * (24L * 60L * 60L));
  total += ((long)time->hours * (60L * 60L));
  total += ((long)time->minutes * 60L);
  total += ((long)time->twosecs * 2L);
  return total;
}


/*
 * this function originally from C_ECHO's Snippets -- modified
 * brute force methodology
 */

static BOOL m_match (CHAR *string, CHAR *pattern, BOOL absolute, BOOL ignore,
                     LONG len) {

  /* return TRUE if pattern found in string */

  register CHAR *tn = pattern;
  register LONG  len2 = 0;
  LONG           lastlen = 0;
  CHAR           lo,hi;

  if(len && string && pattern) {
    if(absolute)                  /* no pattern matching */
      return(findstring(pattern,strlen(pattern),string,len,
                        (ignore == FALSE)) != NULL);

    while(*tn && len2 < len) {
      switch(*tn) {
        case ' ':
          while(*tn == ' ')
            tn++;
          while(len2 < len && isspace(string[len2]))
            len2++;
          break;

        case '*':
          while(*tn == '*' || *tn == '?')
            tn++;
          if(!*tn)
            return TRUE;
          if(ignore) {
            while(len2 < len && string[len2] != *tn)
              len2++;
          }
          else {
            while(len2 < len && toupper(string[len2] != *tn))
              len2++;
          }
          break;

        case '[':
          tn++;
          if(!*tn)
            return FALSE;
          lo = *tn;
          tn++;
          if(*tn != '-')
            return FALSE;
          tn++;
          if(!*tn)
            return FALSE;
          hi = *tn;
          tn++;
          if (*tn != ']')
            return FALSE;
          tn++;
          if(ignore) {
            if ((toupper(string[len2]) >= toupper(lo)) &&
                (toupper(string[len2]) <= toupper(hi)))
              len2++;
            else {
              tn = pattern;
              len2 = lastlen = lastlen + 1;
            }
          }
          else {
            if ((string[len2] >= lo) && (string[len2] <= hi))
              len2++;
            else {
              tn = pattern;
              len2 = lastlen = lastlen + 1;
            }
          }
          break;

        case '?':
          tn++;
          len2++;
          break;

        case '\\':
          tn++;
          if(!*tn)
            return FALSE;
          /* else intentional fallthru */
        default:
          if(ignore) {
            if(toupper(*tn) == toupper(string[len2])) {
              tn++;
              len2++;
            }
            else {
              tn = pattern;
              len2 = lastlen = lastlen + 1;
            }
          }
          else {
            if(*tn == string[len2]) {
              tn++;
              len2++;
            }
            else {
              tn = pattern;
              len2 = lastlen = lastlen + 1;
            }
          }
          break;
      }
    }
    while(*tn == '*')
      tn++;

    if (!*tn)
      return TRUE;
  }
  return FALSE;
}


static BOOL match (CHAR *string,CHAR *patterns,BOOL absolute,BOOL ignore,
                   LONG len,ULONG numlines,CHAR *matched,BOOL matchall) {

  BOOL           ret = FALSE;
  register CHAR *p;
  register ULONG x = 0;

  p = patterns;
  while(!ret && *p) {
    ret = m_match(string,p,absolute,ignore,len);
    if(matchall && ret)
      break;
    if(matched && ret && x < numlines)
      matched[x] = 1;
    p += strlen(p); /* check each pattern in 0-terminated list */
    p++;
    x++;
  }
  return ret;
}


VOID dogrep (VOID *arg)
{
  HAB           ghab;
  HMQ           ghmq;
  GREP          grep;
  register INT  x,numfls;
  static CHAR  *fle[512];
  CHAR         *p,*pp,searchPath[CCHMAXPATH * 2];

  if(!arg)
    return;
  grep = *(GREP *)arg;
  *grep.stopflag = 0;  /* reset thread-killing flag */
  grep.FilesToGet = (grep.dirFlag) ? min(FilesToGet,128) : FilesToGet;
  DosError(FERR_DISABLEHARDERR);
  priority_normal();

  ghab = WinInitialize(0);
  if(ghab) {
    grep.ghab = ghab;
    ghmq = WinCreateMsgQueue(ghab,0);
    if(ghmq) {
      WinCancelShutdown(ghmq,TRUE);
      DosSleep(128L);
      WinSetWindowText(grep.hwndCurFile,
                       GetPString((grep.finddupes) ?
                                  IDS_GREPDUPETEXT : IDS_GREPSCANTEXT));

      pp = grep.searchPattern;
      while(*pp) {
        if(!grep.absFlag) {
          p = GREPCHARS;  /* see if any sense in pattern matching */
          while(*p) {
            if(strchr(pp,*p))
              break;
            p++;
          }
          if(!*p) /* nope, turn it off */
            grep.absFlag = TRUE;
        }
        pp = pp + strlen(pp) + 1;
      }

      grep.attrFile &= (~FILE_DIRECTORY);
      grep.antiattr &= (~FILE_DIRECTORY);
      if(grep.antiattr & FILE_READONLY)
        grep.antiattr |= MUST_HAVE_READONLY;
      if(grep.antiattr & FILE_HIDDEN)
        grep.antiattr |= MUST_HAVE_HIDDEN;
      if(grep.antiattr & FILE_SYSTEM)
        grep.antiattr |= MUST_HAVE_SYSTEM;
      if(grep.antiattr & FILE_ARCHIVED)
        grep.antiattr |= MUST_HAVE_ARCHIVED;

      grep.anyexcludes = FALSE;
      numfls = x = 0;
      fle[numfls++] = strtok(grep.tosearch,";");
      while((fle[numfls] = strtok(NULL,";")) != NULL && numfls < 511) {
        if(*fle[numfls] == '/')
          grep.anyexcludes = TRUE;
        numfls++;
      }

      while(x < numfls) { /* loop through search masks */

        if(*fle[x] == '/')  /* is an exclude mask only */
          goto ExcludeSkip;

        /* first, separate any path from mask */

        p = (char *)(fle[x] + (strlen(fle[x]) - 1));
        while(*p != '\\' && *p != ':' && p != fle[x])
          --p;

        if(p == fle[x]) {  /* no path */
          strcpy(searchPath,grep.curdir);
          strncpy(grep.fileMask,fle[x],CCHMAXPATH);
          grep.fileMask[CCHMAXPATH - 1] = 0;
        }
        else {  /* got to deal with a path */
          if(*p == ':') { /* just a drive, start in root dir */
            *p = 0;
            p++;
            strncpy(searchPath,fle[x],CCHMAXPATH - 2);
            searchPath[CCHMAXPATH - 3] = 0;
            strcat(searchPath,":\\");
            strcpy(grep.fileMask,p);
          }
          if(*p == '\\') {  /* got a 'full' path */

            CHAR temp;

            p++;
            temp = *p;
            *p = 0;
            strncpy(searchPath,fle[x],CCHMAXPATH);
            searchPath[CCHMAXPATH - 1] = 0;
            *p = temp;
            strcpy(grep.fileMask,p);
          }
          if(!*grep.fileMask)
            strcpy(grep.fileMask,"*");
        }
        if(*grep.stopflag)
          break;
        /* do single directory */
        domatchingfiles(&grep,searchPath,fle,numfls);
        if(grep.dirFlag)  /* do subdirs */
          doallsubdirs(&grep,searchPath,FALSE,fle,numfls);
ExcludeSkip:
        if(*grep.stopflag)
          break;
        x++ ;
        if(WinIsWindow(grep.ghab,grep.hwndFiles))
          doinsertion(&grep); /* insert any remaining objects */
      }

ShutDownThread:  /* kill pm connection, end thread */

      if(WinIsWindow(grep.ghab,grep.hwndFiles))
        doinsertion(&grep); /* insert any remaining objects */

      if(WinIsWindow(grep.ghab,grep.hwndFiles) && grep.finddupes &&
         !*grep.stopflag)
        FillDupes(&grep);

      if(!PostMsg(grep.hwndFiles,
                  UM_CONTAINER_FILLED,
                  MPVOID,
                  MPVOID)) /* tell window we're done */
        WinSendMsg(grep.hwndFiles,
                   UM_CONTAINER_FILLED,
                   MPVOID,
                   MPVOID);
      WinDestroyMsgQueue(ghmq);
    }
    WinTerminate(ghab);
  }
  if(!ghmq || !ghab)
    WinPostMsg(grep.hwndFiles,
               UM_CONTAINER_FILLED,
               MPVOID,
               MPVOID);
  if(grep.dupehead)
    FreeDupes(&grep);
  if(grep.numlines &&
     grep.matched)
    free(grep.matched);
  DosPostEventSem(CompactSem);
}


static BOOL IsExcluded (char *name,char **fle,int numfls)
{
  register int x;
  char        *n;

  n = strrchr(name,'\\');
  if(!n)
    n = strrchr(name,':');
  if(n)
    n++;
  else
    n = name;
  for(x = 0;x < numfls;x++) {
    if(*fle[x] == '/' &&
       wildcard((strchr(fle[x],'\\') ||
                 strchr(fle[x],':')) ?
                name : n,fle[x] + 1,FALSE))
      return TRUE;
  }
  return FALSE;
}


static VOID doallsubdirs (GREP *grep,CHAR *searchPath,BOOL recursing,
                          char **fle,int numfls) {

  /* process all subdirectories */

  FILEFINDBUF4 findBuffer;
  HDIR         findHandle  = HDIR_CREATE;
  LONG         findCount   = 1L;
  CHAR         *p = NULL;

  /* add a mask to search path */
  if(searchPath[strlen(searchPath) - 1] != '\\')
    strcat(searchPath,"\\");
  strcat(searchPath,"*");
  /* step through all subdirectories */
  DosError(FERR_DISABLEHARDERR);
  if(!DosFindFirst(searchPath,&findHandle,(MUST_HAVE_DIRECTORY |
                  FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY),
                  &findBuffer,
                  (ULONG)sizeof(findBuffer),
                  (PULONG)&findCount,
                  FIL_QUERYEASIZE)) {

    /* get rid of mask portion, save end-of-directory */

    p = strrchr(searchPath,'\\');
    if(p)
      p++;
    else
      p = searchPath;
    do {   /* Process each directory that matches the mask */
      priority_normal();
      if(*grep->stopflag)
        break;
      if(*findBuffer.achName != '.' ||
         (findBuffer.achName[1] && findBuffer.achName[1] != '.')) {
        strcpy(p,findBuffer.achName) ;
        if(!grep->anyexcludes || !IsExcluded(searchPath,fle,numfls)) {
          domatchingfiles(grep,searchPath,fle,numfls) ;
          doallsubdirs(grep,searchPath,TRUE,fle,numfls);
          DosSleep(0L);
        }
      }
      findCount = 1L;
    } while(!DosFindNext(findHandle,
                         &findBuffer,
                         sizeof(findBuffer),
                         (PULONG)&findCount));
    DosFindClose(findHandle);
    priority_normal();
  }
  if(p)    /* strip off last directory addition */
    *p = 0;
}


static INT domatchingfiles (GREP *grep,CHAR *path,char **fle,int numfls)
{
  /* process all matching files in a directory */

  PFILEFINDBUF4  findBuffer = malloc(grep->FilesToGet * sizeof(FILEFINDBUF4));
  PFILEFINDBUF4  pffbFile;
  register PBYTE fb;
  register ULONG x;
  HDIR           findHandle  = HDIR_CREATE;
  ULONG          findCount = grep->FilesToGet;
  CHAR           newPath[CCHMAXPATH],*p;
  APIRET         rc;

  if(!findBuffer)
    return 0;

  /* build filemask */

  sprintf(newPath,
          "%s%s%s",
          path,
          (path[strlen(path) - 1] == '\\') ?
           NullStr : "\\",
          grep->fileMask);

  MakeFullName(newPath);

  /* find and save end-of-dir position */
  p = strrchr(newPath,'\\');
  if(p)
    p++;
  else
    p = newPath;

  /* step through matching files */
  DosError(FERR_DISABLEHARDERR);
  if(!DosFindFirst(newPath,
                  &findHandle,
                  (FILE_NORMAL | grep->attrFile | grep->antiattr),
                  findBuffer,
                  (ULONG)(grep->FilesToGet * sizeof(FILEFINDBUF4)),
                  (PULONG)&findCount,
                  FIL_QUERYEASIZE)) {

    do {   /* Process each file that matches the mask */
      priority_normal();
      fb = (PBYTE)findBuffer;
      for(x = 0L;x < findCount;x++) {
        pffbFile = (PFILEFINDBUF4)fb;
        if(*grep->stopflag)
          break;
        if(*pffbFile->achName != '.' ||
           (pffbFile->achName[1] && pffbFile->achName[1] != '.')) {
          strcpy(p,pffbFile->achName);  /* build filename */
          if(!grep->anyexcludes ||
             !IsExcluded(newPath,fle,numfls)) {
            if(!grep->finddupes)
              doonefile(grep,
                        newPath,
                        pffbFile);
            else if(!InsertDupe(grep,
                                newPath,
                                pffbFile)) {
              DosFindClose(findHandle);
              free(findBuffer);
              return 1;
            }
          }
        }
        if(!pffbFile->oNextEntryOffset)
          break;
        fb += pffbFile->oNextEntryOffset;
      }
      findCount = grep->FilesToGet;
      rc = DosFindNext(findHandle,
                       findBuffer,
                       (ULONG)(grep->FilesToGet * sizeof(FILEFINDBUF4)),
                       (PULONG)&findCount);
      if(!rc)
        DosSleep(1L);
    } while(!rc);
    DosFindClose(findHandle);
    priority_normal();
  }
  free(findBuffer);
  return 0 ;
}

#pragma alloc_text(GREP,insert_grepfile,doonefile,doinsertion,freegreplist)


static VOID freegreplist (GREP *grep)
{
  register INT x;

  if(grep) {
    if(grep->insertffb) {
      for(x = 0;grep->insertffb[x];x++)
        free(grep->insertffb[x]);
      free(grep->insertffb);
    }
    if(grep->dir) {
      for(x = 0;grep->dir[x];x++)
        free(grep->dir[x]);
      free(grep->dir);
    }
    grep->dir = NULL;
    grep->insertffb = NULL;
    grep->toinsert = 0L;
    grep->insertedbytes = 0L;
  }
}


static BOOL doinsertion (GREP *grep)
{
  RECORDINSERT ri;
  DIRCNRDATA  *dcd;
  PCNRITEM     pci,pciFirst;
  INT          x;

  if(!grep ||
     !grep->toinsert ||
     !grep->insertffb ||
     !grep->dir)
    return FALSE;
  pci = WinSendMsg(grep->hwndFiles,
                   CM_ALLOCRECORD,
                   MPFROMLONG(EXTRA_RECORD_BYTES),
                   MPFROMLONG(grep->toinsert));
  if(pci) {
    if(grep->sayfiles)
      WinSetWindowText(grep->hwndCurFile,
                       GetPString(IDS_GREPINSERTINGTEXT));
    pciFirst = pci;
    dcd = INSTDATA(grep->hwndFiles);
    for(x = 0; grep->insertffb[x]; x++) {
      FillInRecordFromFFB(grep->hwndFiles,
                          pci,
                          grep->dir[x],
                          grep->insertffb[x],
                          FALSE,
                          dcd);
      pci = (PCNRITEM) pci->rc.preccNextRecord;
    }
    memset(&ri,0,sizeof(RECORDINSERT));
    ri.cb                 = sizeof(RECORDINSERT);
    ri.pRecordOrder       = (PRECORDCORE) CMA_END;
    ri.pRecordParent      = (PRECORDCORE)NULL;
    ri.zOrder             = (USHORT) CMA_TOP;
    ri.cRecordsInsert     = grep->toinsert;
    ri.fInvalidateRecord  = TRUE;
    WinSendMsg(grep->hwndFiles,
               CM_INSERTRECORD,
               MPFROMP(pciFirst),
               MPFROMP(&ri));
    if(dcd) {
      DosEnterCritSec();
       dcd->ullTotalBytes += grep->insertedbytes;
      DosExitCritSec();
    }
    if(grep->toinsert == grep->FilesToGet)
      DosSleep(1L);
    freegreplist(grep);
    PostMsg(grep->hwndFiles,
            UM_RESCAN,
            MPVOID,
            MPVOID);
    return TRUE;
  }
  freegreplist(grep);
  return FALSE;
}


static BOOL insert_grepfile (GREP *grep,CHAR *filename,FILEFINDBUF4 *f)
{
  CHAR        *p,szDirectory[CCHMAXPATH];

  if(WinIsWindow(grep->ghab,grep->hwndFiles)) {
    grep->numfiles++;
    strcpy(szDirectory,filename);
    p = strrchr(szDirectory,'\\');
    if(p) {
      if(p < szDirectory + 4)
        p++;
      *p = 0;
      if(!grep->insertffb) {
        grep->insertffb = malloc(sizeof(FILEFINDBUF4 *) *
                                 (grep->FilesToGet + 1));
        if(!grep->insertffb)
          return FALSE;
        memset(grep->insertffb,0,sizeof(FILEFINDBUF4 *) *
               (grep->FilesToGet + 1));
        grep->dir = malloc(sizeof(CHAR *) * (grep->FilesToGet + 1));
        if(!grep->dir) {
          free(grep->insertffb);
          return FALSE;
        }
        memset(grep->dir,0,sizeof(CHAR *) * (grep->FilesToGet + 1));
      }
      grep->insertffb[grep->toinsert] = malloc(sizeof(FILEFINDBUF4));
      if(!grep->insertffb[grep->toinsert])
        return FALSE;
      memcpy(grep->insertffb[grep->toinsert],f,sizeof(FILEFINDBUF4));
      grep->dir[grep->toinsert] = strdup(szDirectory);
      if(!grep->dir) {
        free(grep->insertffb[grep->toinsert]);
        return FALSE;
      }
      grep->insertedbytes += f->cbFile + CBLIST_TO_EASIZE(f->cbList);
      grep->toinsert++;
      if(grep->toinsert == grep->FilesToGet)
        return doinsertion(grep);
      return TRUE;
    }
  }
  else
    freegreplist(grep);
  return FALSE;
}


static BOOL doonefile (GREP *grep,CHAR *filename,FILEFINDBUF4 *f)
{
  /* process a single file */

  CHAR           *input;
  FILE           *inputFile;
  ULONG           pos;
  BOOL            ret = FALSE,strmatch = FALSE;

  grep->fileCount++;
  if(grep->sayfiles)
    WinSetWindowText(grep->hwndCurFile,
                     filename);

  if(grep->greaterthan || grep->lessthan) {

    BOOL  keep = TRUE;
    ULONG adjsize;

    adjsize = f->cbFile +
              (grep->searchEAs ? CBLIST_TO_EASIZE(f->cbList) : 0);
    if(grep->greaterthan) {
      if(adjsize < grep->greaterthan)
        keep = FALSE;
    }
    if(keep && grep->lessthan) {
      if(adjsize > grep->lessthan)
        keep = FALSE;
    }
    if(!keep)
      return ret;
  }

  if(grep->newerthan || grep->olderthan) {

    BOOL  keep = TRUE;
    ULONG numsecs;

    numsecs = SecsSince1980(&f->fdateLastWrite,
                            &f->ftimeLastWrite);
    if(grep->newerthan) {
      if(numsecs < grep->newerthan)
        keep = FALSE;
    }
    if(keep && grep->olderthan) {
      if(numsecs > grep->olderthan)
        keep = FALSE;
    }
    if(!keep)
      return ret;
  }

  if((!grep->searchEAs && !grep->searchFiles) ||
      !*grep->searchPattern)    /* just a find */
    return insert_grepfile(grep,filename,f);

  if(grep->searchEAs) {

    HOLDFEA *head,*info;
    USHORT  type,len;
    BOOL    alltext;
    CHAR    *data,temp;

    head = GetFileEAs(filename,FALSE,TRUE);
    if(head) {
      info = head;
      while(info && !strmatch) {
        alltext = TRUE;
        switch(*(USHORT *)info->value) {
          case EAT_ASCII:
            if(match(info->value + (sizeof(USHORT) * 2),
                     grep->searchPattern,grep->absFlag,
                     (grep->caseFlag == FALSE),
                     info->cbValue - (sizeof(USHORT) * 2),
                     grep->numlines,
                     grep->matched,
                     !grep->findifany)) {
              strmatch = TRUE;
            }
            break;
          case EAT_MVST:
            type = *(USHORT *)(info->value + (sizeof(USHORT) * 3));
            if(type == EAT_ASCII) {
              data = info->value + (sizeof(USHORT) * 4);
              len = *(USHORT *)data;
              data += sizeof(USHORT);
              while((data - info->value) + len <=
                    info->cbValue) {
                temp = *(data + len);
                *(data + len) = 0;
                if(match(data,
                         grep->searchPattern,
                         grep->absFlag,
                         (grep->caseFlag == FALSE),
                         len,
                         grep->numlines,
                         grep->matched,
                         !grep->findifany)) {
                  strmatch = TRUE;
                  break;
                }
                data += len;
                if(data - info->value >= info->cbValue)
                  break;
                *data = temp;
                len = *(USHORT *)data;
                data += sizeof(USHORT);
              }
            }
            break;
          case EAT_MVMT:
            data = info->value + (sizeof(USHORT) * 3);
            type = *(USHORT *)data;
            data += sizeof(USHORT);
            len = *(USHORT *)data;
            data += sizeof(USHORT);
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
                temp = *(data + len);
                *(data + len) = 0;
                if(match(data,
                         grep->searchPattern,
                         grep->absFlag,
                         (grep->caseFlag == FALSE),
                         len,
                         grep->numlines,
                         grep->matched,
                         !grep->findifany)) {
                  strmatch = TRUE;
                  break;
                }
                data += len;
                *data = temp;
                if(data - info->value >= info->cbValue)
                  break;
                type = *(USHORT *)data;
                data += sizeof(USHORT);
                len = *(USHORT *)data;
                data += sizeof(USHORT);
              }
            }
            break;
          default:
            break;
        }
        info = info->next;
      } // while
      Free_FEAList(head);
      DosSleep(1L);
    }
  }

  if(grep->searchFiles) {
    input = malloc(65537);
    if(input) {

      LONG len;

      if((inputFile = _fsopen(filename,"rb",SH_DENYNO)) != NULL) {
        pos = ftell(inputFile);
        while(!feof(inputFile)) {
          if(pos)
            fseek(inputFile,pos - 1024,SEEK_SET);
          len = fread(input,1,65536,inputFile);
          if(len >= 0) {
            if(*grep->stopflag)
              break;
            if(match(input,
                     grep->searchPattern,
                     grep->absFlag,
                     (grep->caseFlag == FALSE),
                     len,
                     grep->numlines,
                     grep->matched,
                     !grep->findifany)) {
              strmatch = TRUE;
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

Match:

  if(strmatch)
    ret = insert_grepfile(grep,
                          filename,
                          f);
  return ret;
}


#pragma alloc_text(DUPES,InsertDupe,FillDupes,FreeDupes,CRCFile,CRCBlock)
#pragma alloc_text(DUPES,comparenamesq,comparenamesqe,comparenamesb)
#pragma alloc_text(DUPES,comparenamesbe,comparesizesq,comparesizesb)

static LONG cr3tab[] = {    /* CRC polynomial 0xEDB88320 */

  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
  0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
  0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
  0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
  0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
  0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
  0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
  0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
  0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
  0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
  0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
  0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
  0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
  0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
  0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
  0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
  0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
  0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
  0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
  0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
  0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
  0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
  0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
  0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
  0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
  0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
  0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
  0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
  0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
  0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
  0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
  0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
  0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
  0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
  0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
  0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
  0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
  0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
  0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
  0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
  0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
  0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


LONG CRCBlock (register CHAR *str, register INT blklen, register LONG crc)
{
  while (blklen--) {
    crc = cr3tab[((INT) crc ^ *str) & 0xff] ^ (((ULONG)crc >> 8) & 0x00FFFFFF);
    str++;
  }
  return crc;
}


LONG CRCFile (CHAR *filename,INT *error)
{
  LONG CRC = -1L,len;
  FILE *fp;
  CHAR *buffer;

  *error = 0;
  buffer = malloc(65535);
  if(buffer) {
    fp = _fsopen(filename,"rb",SH_DENYNO);
    if(fp) {
      while(!feof(fp)) {
        len = fread(buffer,1,65535,fp);
        if(len && len < 65536L)
          CRC = CRCBlock(buffer,len,CRC);
        else
          break;
        DosSleep(0L);
      }
      fclose(fp);
      DosSleep(1L);
    }
    else
      *error = -2;
    free(buffer);
  }
  else
    *error = -1;
  return CRC;
}


static VOID FreeDupes (GREP *g)
{
  DUPES *i,*next;

  i = g->dupehead;
  while(i) {
    next = i->next;
    if(i->name)
      free(i->name);
    free(i);
    i = next;
  }
  g->dupehead = g->dupelast = NULL;
  if(g->dupenames)
    free(g->dupenames);
  if(g->dupesizes)
    free(g->dupesizes);
  g->dupesizes = g->dupenames = NULL;
}


int comparenamesq (const void *v1,const void *v2)
{
  DUPES *d1 = *(DUPES **)v1;
  DUPES *d2 = *(DUPES **)v2;
  CHAR  *p1,*p2;

  p1 = strrchr(d1->name,'\\');
  if(p1)
    p1++;
  else
    p1 = d1->name;
  p2 = strrchr(d2->name,'\\');
  if(p2)
    p2++;
  else
    p2 = d2->name;
  return stricmp(p1,p2);
}


int comparenamesqe (const void *v1,const void *v2)
{
  DUPES *d1 = *(DUPES **)v1;
  DUPES *d2 = *(DUPES **)v2;
  CHAR  *p1,*p2,*p1e,*p2e,e1,e2;
  int    ret;

  p1 = strrchr(d1->name,'\\');
  if(p1)
    p1++;
  else
    p1 = d1->name;
  p1e = strrchr(p1,'.');
  if(p1e) {
    e1 = *p1e;
    *p1e = 0;
  }
  p2 = strrchr(d2->name,'\\');
  if(p2)
    p2++;
  else
    p2 = d2->name;
  p2e = strrchr(p2,'.');
  if(p2e) {
    e2 = *p2e;
    *p2e = 0;
  }
  ret = stricmp(p1,p2);
  if(p1e)
    *p1e = e1;
  if(p2e)
    *p2e = e2;
  return ret;
}


int comparesizesq (const void *v1,const void *v2)
{
  DUPES *d1 = *(DUPES **)v1;
  DUPES *d2 = *(DUPES **)v2;

  return (d1->size > d2->size) ? 1 : (d1->size == d2->size) ? 0 : -1;
}


int comparenamesb (const void *v1,const void *v2)
{
  DUPES *d1 = (DUPES *)v1;
  DUPES *d2 = *(DUPES **)v2;
  CHAR  *p1,*p2;

  p1 = strrchr(d1->name,'\\');
  if(p1)
    p1++;
  else
    p1 = d1->name;
  p2 = strrchr(d2->name,'\\');
  if(p2)
    p2++;
  else
    p2 = d2->name;
  return stricmp(p1,p2);
}


int comparenamesbe (const void *v1,const void *v2)
{
  DUPES *d1 = (DUPES *)v1;
  DUPES *d2 = *(DUPES **)v2;
  CHAR  *p1,*p2,*p1e,*p2e,e1,e2;
  int    ret;

  p1 = strrchr(d1->name,'\\');
  if(p1)
    p1++;
  else
    p1 = d1->name;
  p1e = strrchr(p1,'.');
  if(p1e) {
    e1 = *p1e;
    *p1e = 0;
  }
  p2 = strrchr(d2->name,'\\');
  if(p2)
    p2++;
  else
    p2 = d2->name;
  p2e = strrchr(p2,'.');
  if(p2e) {
    e2 = *p2e;
    *p2e = 0;
  }
  ret = stricmp(p1,p2);
  if(p1e)
    *p1e = e1;
  if(p2e)
    *p2e = e2;
  return ret;
}


int comparesizesb (const void *v1,const void *v2)
{
  DUPES *d1 = (DUPES *)v1;
  DUPES *d2 = *(DUPES **)v2;

  return (d1->size > d2->size) ? 1 : (d1->size == d2->size) ? 0 : -1;
}


static VOID FillDupes (GREP *g)
{
  DUPES         *c,*i,**r;
  register CHAR *pc,*pi;
  CHAR         **list = NULL;
  INT            numfiles = 0,numalloced = 0,error;
  register ULONG x = 0L,y = 0L;
  ULONG          cntr = 100;

  if(g->CRCdupes)
    cntr = 50;
  i = g->dupehead;
  while(i) {
    x++;
    i = i->next;
  }
  if(x) {
    WinSetWindowText(g->hwndCurFile,
                     GetPString(IDS_GREPDUPESORTINGTEXT));
    DosSleep(1L);
    g->dupenames = malloc(sizeof(DUPES *) * (x + 1));
    if(!g->nosizedupes)
      g->dupesizes = malloc(sizeof(DUPES *) * (x + 1));
    if(g->dupenames && (g->nosizedupes || g->dupesizes)) {
      i = g->dupehead;
      while(i) {
        g->dupenames[y] = i;
        if(!g->nosizedupes)
          g->dupesizes[y] = i;
        i = i->next;
        y++;
      }
      g->dupenames[y] = NULL;
      if(!g->nosizedupes)
        g->dupesizes[y] = NULL;
      DosSleep(1L);
      qsort(g->dupenames,
            x,
            sizeof(DUPES *),
            ((g->ignoreextdupes) ?
             comparenamesqe :
             comparenamesq));
      DosSleep(1L);
      if(!g->nosizedupes) {
        qsort(g->dupesizes,
              x,
              sizeof(DUPES *),
              comparesizesq);
        DosSleep(1L);
      }
      WinSetWindowText(g->hwndCurFile,
                       GetPString(IDS_GREPDUPECOMPARINGTEXT));

      i = g->dupehead;
      y = 0L;
      while(i) {
        if(*g->stopflag)
          break;
        if(!(i->flags & GF_SKIPME)) {
          r = (DUPES **)bsearch(i,g->dupenames,x,sizeof(DUPES *),
                                ((g->ignoreextdupes) ? comparenamesbe :
                                 comparenamesb));
          if(r) {
            while(r > g->dupenames && ((g->ignoreextdupes) ?
                  !comparenamesqe((r - 1),&i) :
                  !comparenamesq((r - 1),&i)))
              r--;
            while(*r && ((g->ignoreextdupes) ?
                         !comparenamesqe(r,&i) :
                         !comparenamesq(r,&i))) {
              if(*r == i || ((*r)->flags & (GF_INSERTED | GF_SKIPME))) {
                r++;
                continue;
              }
              if(g->CRCdupes) {
                if((*r)->CRC == -1L) {
                  (*r)->CRC = CRCFile((*r)->name,&error);
                  if(error)
                    (*r)->CRC = -1L;
                  else if((*r)->CRC == -1L)
                    (*r)->CRC = 0L;
                }
                if(i->CRC == -1L) {
                  i->CRC = CRCFile(i->name,&error);
                  if(error)
                    i->CRC = -1L;
                  else if(i->CRC == -1L)
                    i->CRC = 0L;
                }
                if(((*r)->size != i->size) || ((*r)->CRC != -1L &&
                   i->CRC != -1L && (*r)->CRC != i->CRC)) {
                  r++;
                  continue;
                }
              }
              if(!AddToList((*r)->name,
                            &list,
                            &numfiles,
                            &numalloced)) {
                (*r)->flags |= GF_INSERTED;
                if(g->sayfiles)
                  WinSetWindowText(g->hwndFiles,
                                   (*r)->name);
                if((*r)->size == i->size &&
                   (i->date.year == (*r)->date.year &&
                    i->date.month == (*r)->date.month &&
                    i->date.day == (*r)->date.day &&
                    i->time.hours == (*r)->time.hours &&
                    i->time.minutes == (*r)->time.minutes &&
                    i->time.twosecs == (*r)->time.twosecs))
                  (*r)->flags |= GF_SKIPME;
              }
              if(!(i->flags & (GF_INSERTED | GF_SKIPME))) {
                if(!AddToList(i->name,
                              &list,
                              &numfiles,
                              &numalloced)) {
                  i->flags |= GF_INSERTED;
                  if((*r)->flags & GF_SKIPME)
                    i->flags |= GF_SKIPME;
                }
              }
              r++;
            }
          }
          if(!g->nosizedupes) {
            r = (DUPES **)bsearch(i,
                                  g->dupesizes,
                                  x,
                                  sizeof(DUPES *),
                                  comparesizesb);
            if(r) {
              while(r > g->dupesizes && !comparesizesq((r - 1),&i))
                r--;
              while(*r && !comparesizesq(r,&i)) {
                if(*r == i || ((*r)->flags & (GF_INSERTED | GF_SKIPME)) ||
                   (i->date.year != (*r)->date.year ||
                    i->date.month != (*r)->date.month ||
                    i->date.day != (*r)->date.day ||
                    i->time.hours != (*r)->time.hours ||
                    i->time.minutes != (*r)->time.minutes ||
                    i->time.twosecs != (*r)->time.twosecs)) {
                  r++;
                  continue;
                }
                if(g->CRCdupes) {
                  if((*r)->CRC == -1L) {
                    (*r)->CRC = CRCFile((*r)->name,&error);
                    if(error)
                      (*r)->CRC = -1L;
                    else if((*r)->CRC == -1L)
                      (*r)->CRC = 0L;
                  }
                  if(i->CRC == -1L) {
                    i->CRC = CRCFile(i->name,&error);
                    if(error)
                      i->CRC = -1L;
                    else if(i->CRC == -1L)
                      i->CRC = 0L;
                  }
                  if((*r)->CRC != -1L && i->CRC != -1L &&
                     (*r)->CRC != i->CRC) {
                    *r++;
                    continue;
                  }
                }
                if(!AddToList((*r)->name,
                              &list,
                              &numfiles,
                              &numalloced)) {
                  if(g->sayfiles)
                    WinSetWindowText(g->hwndCurFile,
                                     (*r)->name);
                  (*r)->flags |= GF_INSERTED;
                  if(((g->ignoreextdupes) ?
                      comparenamesqe(r,&i) :
                      comparenamesq(r,&i)))
                    (*r)->flags |= GF_SKIPME;
                }
                if(!(i->flags & (GF_INSERTED | GF_SKIPME))) {
                  if(!AddToList(i->name,
                                &list,
                                &numfiles,
                                &numalloced)) {
                    i->flags |= GF_INSERTED;
                    if((*r)->flags & GF_SKIPME)
                      i->flags |= GF_SKIPME;
                  }
                }
                r++;
              }
            }
          }
        }
        i = i->next;
        y++;
        if(!(y % cntr)) {

          CHAR s[44];

          sprintf(s,
                  GetPString(IDS_GREPDUPECHECKPROGTEXT),
                  y,
                  g->numfiles);
          WinSetWindowText(g->hwndCurFile,
                           s);
          DosSleep(128L);
        }
        DosSleep(y % 2);
      }
    }
    else {
      DosBeep(50,100);
      WinSetWindowText(g->hwndCurFile,
                       GetPString(IDS_GREPDUPECOMPARINGTEXT));
      x = y = 0L;
      if(g->dupenames) {
        free(g->dupenames);
        g->dupenames = NULL;
      }
      if(g->dupesizes) {
        free(g->dupesizes);
        g->dupesizes = NULL;
      }
      i = g->dupehead;
      while(i) {
        if(*g->stopflag)
          break;
        if(!(i->flags & GF_SKIPME)) {
          if(!(y % cntr)) {

            CHAR s[44];

            sprintf(s,
                    GetPString(IDS_GREPDUPECHECKPROGTEXT),
                    y,
                    g->numfiles);
            WinSetWindowText(g->hwndCurFile,
                             s);
            DosSleep(0L);
          }
          y++;
          pi = strrchr(i->name,'\\');
          if(pi)
            *pi++;
          else
            pi = i->name;
          c = g->dupehead;
          while(c) {
            if(*g->stopflag)
              break;
            if(c != i && !(c->flags & (GF_INSERTED | GF_SKIPME))) {
              x++;
              pc = strrchr(c->name,'\\');
              if(pc)
                pc++;
              else
                pc = c->name;
              if((!g->nosizedupes && i->size == c->size &&
                  i->date.year == c->date.year &&
                  i->date.month == c->date.month &&
                  i->date.day == c->date.day &&
                  i->time.hours == c->time.hours &&
                  i->time.minutes == c->time.minutes &&
                  i->time.twosecs == c->time.twosecs) ||
                 !stricmp(pc,pi)) {         /* potential dupe */
                if(g->CRCdupes) {
                  if(g->CRCdupes) {
                    if(c->CRC == -1L) {
                      c->CRC = CRCFile(c->name,&error);
                      if(error)
                        c->CRC = -1L;
                      else if(c->CRC == -1L)
                        c->CRC = 0L;
                    }
                    if(i->CRC == -1L) {
                      i->CRC = CRCFile(i->name,&error);
                      if(error)
                        i->CRC = -1L;
                      else if(i->CRC == -1L)
                          i->CRC = 0L;
                    }
                    if((c->size != i->size) || (c->CRC != -1L &&
                       i->CRC != -1L && c->CRC != i->CRC)) {
                      c = c->next;
                      continue;
                    }
                  }
                }
                if(AddToList(c->name,
                             &list,
                             &numfiles,
                             &numalloced))
                  goto BreakOut;
                if(!(i->flags & GF_INSERTED)) {
                  if(AddToList(i->name,
                               &list,
                               &numfiles,
                               &numalloced))
                    goto BreakOut;
                }
                if(g->sayfiles)
                  WinSetWindowText(g->hwndCurFile,
                                   pc);
                c->flags |= GF_INSERTED;
                i->flags |= GF_INSERTED;
                if(!stricmp(pc,pi)) {
                  c->flags |= GF_SKIPME;
                  i->flags |= GF_SKIPME;
                }
              }
              else if(!(x % 100L))
                DosSleep(1L);
            }
            c = c->next;
          }
        }
        i = i->next;
      }
    }
  }
BreakOut:
  FreeDupes(g);
  if(numfiles && list) {
    if(!PostMsg(g->hwndFiles,
                WM_COMMAND,
                MPFROM2SHORT(IDM_COLLECTOR,0),
                MPFROMP(list)))
      FreeList(list);
  }
  else
    DosPostEventSem(CompactSem);
}


static BOOL InsertDupe (GREP *g,CHAR *dir,FILEFINDBUF4 *f)
{
  DUPES *info;

  if(*dir) {
    info = malloc(sizeof(DUPES));
    if(info) {
      memset(info,0,sizeof(DUPES));
      info->name = strdup(dir);
      if(info->name) {
        info->size = f->cbFile;
        info->date = f->fdateLastWrite;
        info->time = f->ftimeLastWrite;
        info->CRC = -1L;
        g->numfiles++;
        if(!g->dupehead)
          g->dupehead = info;
        if(g->dupelast)
          g->dupelast->next = info;
        g->dupelast = info;
        info->next = NULL;
        return TRUE;
      }
      else
        free(info);
    }
    DosBeep(50,100);
    return FALSE;
  }
  return TRUE;
}
