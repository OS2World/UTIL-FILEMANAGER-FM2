#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fm3dll.h"

#pragma alloc_text(VALID,CheckDrive,IsRoot,IsFile,IsFullName,needsquoting)
#pragma alloc_text(VALID,IsValidDir,IsValidDrive,MakeValidDir,IsVowel)
#pragma alloc_text(VALID,IsFileSame,IsNewer,TestDates,RootName,MakeFullName)
#pragma alloc_text(VALID,IsExecutable,IsBinary,IsDesktop,ParentIsDesktop)
#pragma alloc_text(FILLFLAGS,FillInDriveFlags,assign_ignores)
#pragma alloc_text(FILLFLAGS,ArgDriveFlags,DriveFlagsOne)
#pragma alloc_text(FINDDESK,GetDesktopName)


APIRET MakeFullName (char *filename) {

  /* filename must be CCHMAXPATH long minimum! */

  char   fullname[CCHMAXPATH];
  APIRET rc;

  DosError(FERR_DISABLEHARDERR);
  rc = DosQueryPathInfo(filename,
                        FIL_QUERYFULLNAME,
                        fullname,
                        sizeof(fullname));
  if(rc)
    strcpy(filename,
           fullname);
  return rc;
}


char *RootName (char *filename) {

  char *p = NULL,*pp;

  if(filename) {
    p = strrchr(filename,'\\');
    pp = strrchr(filename,'/');
    p = (p) ?
         (pp) ?
          (p > pp) ?
           p :
           pp :
          p :
         pp;
  }
  if(!p)                  /* name is itself a root */
    p = filename;
  else                    /* skip past backslash */
    p++;
  if(p &&
     !*p &&
     p == filename + 3)   /* is root */
    p--;
  return p;
}


int TestDates (char *file1,char *file2) {

  /*
   * return 1 (file2 newer than file1),
   * 0 (files same)
   * or -1 (file1 newer than file2)
   */

  int         comp = 0;
  FILESTATUS3 fs3o,fs3n;

  DosError(FERR_DISABLEHARDERR);
  if(!DosQueryPathInfo(file1,
                       FIL_STANDARD,
                       &fs3o,
                       sizeof(fs3o))) {
    DosError(FERR_DISABLEHARDERR);
    if(!DosQueryPathInfo(file2,
                         FIL_STANDARD,
                         &fs3n,
                         sizeof(fs3n))) {
      comp = (fs3n.fdateLastWrite.year >
              fs3o.fdateLastWrite.year) ? 1 :
             (fs3n.fdateLastWrite.year <
              fs3o.fdateLastWrite.year) ? -1 :
             (fs3n.fdateLastWrite.month >
              fs3o.fdateLastWrite.month) ? 1 :
             (fs3n.fdateLastWrite.month <
              fs3o.fdateLastWrite.month) ? -1 :
             (fs3n.fdateLastWrite.day >
              fs3o.fdateLastWrite.day) ? 1 :
             (fs3n.fdateLastWrite.day <
              fs3o.fdateLastWrite.day) ? -1 :
             (fs3n.ftimeLastWrite.hours >
              fs3o.ftimeLastWrite.hours) ? 1 :
             (fs3n.ftimeLastWrite.hours <
              fs3o.ftimeLastWrite.hours) ? -1 :
             (fs3n.ftimeLastWrite.minutes >
              fs3o.ftimeLastWrite.minutes) ? 1 :
             (fs3n.ftimeLastWrite.minutes <
              fs3o.ftimeLastWrite.minutes) ? -1 :
             (fs3n.ftimeLastWrite.twosecs >
              fs3o.ftimeLastWrite.twosecs) ? 1 :
             (fs3n.ftimeLastWrite.twosecs <
              fs3o.ftimeLastWrite.twosecs) ? -1 :
             0;
    }
  }
  return comp;
}


BOOL IsNewer (char *file1,char *file2) {

  /* return TRUE if file2 is newer than file1 */

  return (TestDates(file1,file2) > 0);
}


BOOL IsDesktop (HAB hab,HWND hwnd) {

  HWND hwndDesktop;

  if(hwnd == HWND_DESKTOP)
    return TRUE;
  hwndDesktop = WinQueryDesktopWindow(hab,NULLHANDLE);
  if(hwnd == hwndDesktop)
    return TRUE;
  return FALSE;
}

BOOL ParentIsDesktop (HWND hwnd,HWND hwndParent) {

  HWND hwndDesktop;
  BOOL ret = FALSE;

  if(!hwndParent)
    hwndParent = WinQueryWindow(hwnd,QW_PARENT);
  if(hwndParent == HWND_DESKTOP)
    ret = TRUE;
  else {
    hwndDesktop = WinQueryDesktopWindow(WinQueryAnchorBlock(hwnd),(HWND)0);
    if(hwndDesktop == hwndParent)
      ret = TRUE;
  }
  return ret;
}


INT CheckDrive (CHAR Drive, CHAR *FileSystem, ULONG *type) {

  CHAR        Path[3],*Buffer = NULL,*pfsn = NULL,*pfsd = NULL;
  ULONG       Size,Status,action,LengthIn,LengthOut;
  HFILE       Handle;
  BYTE        Command = 0,NonRemovable;
  PFSQBUFFER2 pfsq;

  if(FileSystem)
    *FileSystem = 0;
  if(type)
    *type = 0;

  if(DosAllocMem((PVOID)&Buffer,4096,
                 PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE)) {
    DosBeep(50,50);
    return -1;
  }

  Path[0] = Drive;
  Path[1] = ':';
  Path[2] = 0;
  Size = 4096;
  DosError(FERR_DISABLEHARDERR);
  Status = DosQueryFSAttach(Path, 0, FSAIL_QUERYNAME,
                            (PFSQBUFFER2)Buffer, &Size);
  if(Status) {   /* can't get any info at all */
    DosFreeMem(Buffer);
    DosError(FERR_DISABLEHARDERR);
    return -1;
  }

  pfsq = (PFSQBUFFER2)Buffer;
  pfsn = pfsq->szName + pfsq->cbName + 1;
  pfsd = pfsn + pfsq->cbFSDName + 1;

  if(FileSystem) {
    strncpy(FileSystem, pfsn, CCHMAXPATH);
    FileSystem[CCHMAXPATH - 1] = 0;
  }

  if(type && !strcmp(pfsn,CDFS))
    (*type) |= (DRIVE_NOTWRITEABLE | DRIVE_CDROM | DRIVE_REMOVABLE);

  if(((PFSQBUFFER2)Buffer)->iType == FSAT_REMOTEDRV) {
    if(type)
      (*type) |= DRIVE_REMOTE;
    if(type && !strcmp(pfsn,CBSIFS)) {
      (*type) |= DRIVE_ZIPSTREAM;
      (*type) &= (~DRIVE_REMOTE);
      (*type) |= DRIVE_NOLONGNAMES;
      if(pfsq->cbFSAData) {

        ULONG FType;

        if(CheckDrive(*pfsd,NULL,&FType) != -1) {
          if(FType & DRIVE_REMOVABLE)
            (*type) |= DRIVE_REMOVABLE;
          if(!(FType & DRIVE_NOLONGNAMES))
            (*type) &= (~DRIVE_NOLONGNAMES);
        }
      }
    }
    if(type && (!strcmp(pfsn,HPFS) || !strcmp(pfsn,HPFS386)))
      (*type) &= (~DRIVE_NOLONGNAMES);
    DosFreeMem(Buffer);
    return 0;  /* assume remotes are non-removable */
  }

  if(strcmp(pfsn,HPFS) && strcmp(pfsn,CDFS) && strcmp(pfsn,HPFS386)) {
    if(type)
      (*type) |= DRIVE_NOLONGNAMES;
  }

  DosError(FERR_DISABLEHARDERR);
  Status = DosOpen(Path, &Handle, &action, 0, 0, FILE_OPEN,
                   OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE |
                   OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR, 0);
  if(Status) {
    DosError(FERR_DISABLEHARDERR);
    if(type)
      (*type) |= DRIVE_REMOVABLE;
    DosFreeMem(Buffer);
    return(1);  /* assume inaccessible local drives are removable */
  }
  LengthIn = sizeof(Command);
  LengthOut = sizeof(NonRemovable);
  NonRemovable = 1;
  DosError(FERR_DISABLEHARDERR);
  DosDevIOCtl(Handle, 8, 0x20,&Command, sizeof(Command), &LengthIn,
              &NonRemovable, sizeof(NonRemovable), &LengthOut);
  DosClose(Handle);
  if(!NonRemovable && type)
    (*type) |= DRIVE_REMOVABLE;
  DosFreeMem(Buffer);
  return (NonRemovable) ? 0 : 1;
}


BOOL IsFileSame (CHAR *filename1,CHAR *filename2) {

  /* returns:  -1 (error), 0 (is a directory), or 1 (is a file) */

  FILESTATUS3 fsa1,fsa2;
  APIRET      ret;

  if(filename1 && filename2) {
    DosError(FERR_DISABLEHARDERR);
    ret = DosQueryPathInfo(filename1,FIL_STANDARD,&fsa1,
                        (ULONG)sizeof(fsa1));
    if(!ret) {
      DosError(FERR_DISABLEHARDERR);
      ret = DosQueryPathInfo(filename2,FIL_STANDARD,&fsa2,
                             (ULONG)sizeof(fsa2));
      if(!ret) {
        if(fsa1.cbFile == fsa2.cbFile &&
           (fsa1.attrFile & (~FILE_ARCHIVED)) ==
           (fsa2.attrFile & (~FILE_ARCHIVED)))
          return TRUE;
      }
    }
  }
  return FALSE;
}


INT IsFile (CHAR *filename) {

  /* returns:  -1 (error), 0 (is a directory), or 1 (is a file) */

  FILESTATUS3 fsa;
  APIRET      ret;

  if(filename && *filename) {
    DosError(FERR_DISABLEHARDERR);
    ret = DosQueryPathInfo(filename,
                           FIL_STANDARD,
                           &fsa,
                           (ULONG)sizeof(fsa));
    if(!ret)
      return ((fsa.attrFile & FILE_DIRECTORY) == 0);
    else if(IsValidDrive(*filename) && IsRoot(filename))
      return 0;
  }
  return -1;  /* error; doesn't exist or can't read or null filename */
}


BOOL IsFullName (CHAR *filename) {

  return (filename) ?
          (isalpha(*filename) && filename[1] == ':' && filename[2] == '\\') :
          0;
}


BOOL IsRoot (CHAR *filename) {

  return (filename && isalpha(*filename) && filename[1] == ':' &&
          filename[2] == '\\' && !filename[3]);
}


BOOL IsValidDir (CHAR *path) {

  CHAR        fullname[CCHMAXPATH];
  FILESTATUS3 fs;

  if(path) {
    DosError(FERR_DISABLEHARDERR);
    if(!DosQueryPathInfo(path,
                         FIL_QUERYFULLNAME,
                         fullname,
                         sizeof(fullname))) {
      if(IsValidDrive(*fullname)) {
        if(!IsRoot(fullname)) {
          DosError(FERR_DISABLEHARDERR);
          if(!DosQueryPathInfo(fullname,
                               FIL_STANDARD,
                               &fs,
                               sizeof(fs)) &&
             (fs.attrFile & FILE_DIRECTORY))
            return TRUE;
        }
        else
          return TRUE;
      }
    }
  }
  return FALSE;
}


BOOL IsValidDrive (CHAR drive) {

  CHAR   Path[] = " :",Buffer[256];
  APIRET Status;
  ULONG  Size;
  ULONG  ulDriveNum,ulDriveMap;

  if(!isalpha(drive) ||
     (driveflags[toupper(drive) - 'A'] & (DRIVE_IGNORE | DRIVE_INVALID)))
    return FALSE;
  DosError(FERR_DISABLEHARDERR);
  Status = DosQCurDisk(&ulDriveNum,&ulDriveMap);
  if(!Status) {
    if(!(ulDriveMap & (1L << (ULONG)(toupper(drive) - 'A'))))
      return FALSE;
    Path[0] = toupper(drive);
    Size = sizeof(Buffer);
    DosError(FERR_DISABLEHARDERR);
    Status = DosQueryFSAttach(Path,
                              0,
                              FSAIL_QUERYNAME,
                              (PFSQBUFFER2)Buffer,
                              &Size);
  }
  return (Status == 0);
}


CHAR * MakeValidDir (CHAR *path) {

  CHAR           fullname[CCHMAXPATH],drive;
  register CHAR *p;
  FILESTATUS3    fs;
  APIRET         status;

  if(!MakeFullName(path)) {
    if(IsValidDrive(*path)) {
      for(;;) {
        if(IsRoot(path))
          return path;
        DosError(FERR_DISABLEHARDERR);
        status = DosQueryPathInfo(path,
                                  FIL_STANDARD,
                                  &fs,
                                  sizeof(fs));
        if(!status &&
           (fs.attrFile & FILE_DIRECTORY) != 0)
          return path;
        p = strrchr(path,'\\');
        if(p) {
          if(p < path + 3)
            p++;
          *p = 0;
        }
        else
          break;
      }
    }
  }
  DosError(FERR_DISABLEHARDERR);
  if(!DosQuerySysInfo(QSV_BOOT_DRIVE,
                      QSV_BOOT_DRIVE,
                      &drive,
                      1L)) {
    drive += '@';
    if(drive < 'C')
      drive = 'C';
    strcpy(path," :\\");
    *path = drive;
  }
  else
    save_dir2(path);
  return path;
}


BOOL IsExecutable (CHAR *filename) {

  register CHAR *p;
  APIRET         ret;
  ULONG          apptype;

  if(filename) {
    DosError(FERR_DISABLEHARDERR);
    p = strrchr(filename,'.');
    if(p)
      ret = DosQAppType(filename,
                        &apptype);
    else {

      char fname[CCHMAXPATH + 2];

      strcpy(fname,filename);
      strcat(fname,".");
      ret = DosQAppType(fname,
                        &apptype);
    }
    if((!ret && (!apptype ||
                 (apptype &
                  (FAPPTYP_NOTWINDOWCOMPAT |
                   FAPPTYP_WINDOWCOMPAT |
                   FAPPTYP_WINDOWAPI |
                   FAPPTYP_BOUND |
                   FAPPTYP_DOS |
                   FAPPTYP_WINDOWSREAL |
                   FAPPTYP_WINDOWSPROT |
                   FAPPTYP_32BIT |
                   0x1000)))) ||
       (p &&
        (!stricmp(p,".CMD") ||
        !stricmp(p,".BAT"))))
      return TRUE;
  }
  return FALSE;
}


VOID ArgDriveFlags (INT argc,CHAR **argv) {

  INT x;

  for(x = 1;x < argc;x++) {
    if(*argv[x] == '/' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while(isalpha(*p)) {
        driveflags[toupper(*p) - 'A'] |= DRIVE_IGNORE;
        p++;
      }
    }
    else if(*argv[x] == ';' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while(isalpha(*p)) {
        driveflags[toupper(*p) - 'A'] |= DRIVE_NOPRESCAN;
        p++;
      }
    }
    else if(*argv[x] == ',' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while(isalpha(*p)) {
        driveflags[toupper(*p) - 'A'] |= DRIVE_NOLOADICONS;
        p++;
      }
    }
    else if(*argv[x] == '`' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while(isalpha(*p)) {
        driveflags[toupper(*p) - 'A'] |= DRIVE_NOLOADSUBJS;
        p++;
      }
    }
    else if(*argv[x] == '\'' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while(isalpha(*p)) {
        driveflags[toupper(*p) - 'A'] |= DRIVE_NOLOADLONGS;
        p++;
      }
    }
  }
}


VOID DriveFlagsOne (INT x) {

  INT         removable;
  CHAR        szDrive[] = " :\\",FileSystem[CCHMAXPATH];
  ULONG       drvtype;

  *szDrive = (CHAR)(x + 'A');
  *FileSystem = 0;
  drvtype = 0;
  removable = CheckDrive(*szDrive,FileSystem,&drvtype);
  driveserial[x] = -1;
  driveflags[x] &= (DRIVE_IGNORE | DRIVE_NOPRESCAN | DRIVE_NOLOADICONS |
                    DRIVE_NOLOADSUBJS | DRIVE_NOLOADLONGS |
                    DRIVE_INCLUDEFILES | DRIVE_SLOW);
  if(removable != -1) {

    struct {
      ULONG serial;
      CHAR  volumelength;
      CHAR  volumelabel[CCHMAXPATH];
    }      volser;

    DosError(FERR_DISABLEHARDERR);
    if(!DosQueryFSInfo((ULONG)x + 1,FSIL_VOLSER,&volser,sizeof(volser)))
      driveserial[x] = volser.serial;
    else
      DosError(FERR_DISABLEHARDERR);
  }
  else
    driveflags[x] |= DRIVE_INVALID;
  driveflags[x] |= ((removable == -1 || removable == 1) ?
                                        DRIVE_REMOVABLE : 0);
  if(drvtype & DRIVE_REMOTE)
    driveflags[x] |= DRIVE_REMOTE;
  if(strcmp(FileSystem,HPFS) &&
     strcmp(FileSystem,HPFS386) &&
     strcmp(FileSystem,CDFS))
    driveflags[x] |= DRIVE_NOLONGNAMES;
  if(!strcmp(FileSystem,CDFS)) {
    removable = 1;
    driveflags[x] |= (DRIVE_REMOVABLE | DRIVE_NOTWRITEABLE |
                      DRIVE_CDROM);
  }
  else if(!stricmp(FileSystem,CBSIFS)) {
    driveflags[x] |= DRIVE_ZIPSTREAM;
    driveflags[x] &= (~DRIVE_REMOTE);
    if(drvtype & DRIVE_REMOVABLE)
      driveflags[x] |= DRIVE_REMOVABLE;
    if(!(drvtype & DRIVE_NOLONGNAMES))
      driveflags[x] &= (~DRIVE_NOLONGNAMES);
  }
}


VOID FillInDriveFlags (VOID *dummy) {

  ULONG        ulDriveNum,ulDriveMap;
  register INT x;

  for(x = 0;x < 26;x++)
    driveflags[x] &= (DRIVE_IGNORE | DRIVE_NOPRESCAN | DRIVE_NOLOADICONS |
                      DRIVE_NOLOADSUBJS | DRIVE_NOLOADLONGS |
                      DRIVE_INCLUDEFILES | DRIVE_SLOW);
  memset(driveserial,-1,sizeof(driveserial));
  DosError(FERR_DISABLEHARDERR);
  DosQCurDisk(&ulDriveNum,&ulDriveMap);
  for(x = 0;x < 26;x++) {
    if(ulDriveMap & (1L << x) && !(driveflags[x] & DRIVE_IGNORE)) {
      {
        CHAR  s[80];
        ULONG flags = 0,size = sizeof(ULONG);

        sprintf(s,"%c.DriveFlags",(CHAR)(x + 'A'));
        if(PrfQueryProfileData(fmprof,appname,s,&flags,&size) &&
           size == sizeof(ULONG))
          driveflags[x] |= flags;
      }

      if(x > 1) {
        if(!(driveflags[x] & DRIVE_NOPRESCAN))
          DriveFlagsOne(x);
        else
          driveserial[x] = -1;
      }
      else {
        driveflags[x] |= (DRIVE_REMOVABLE | DRIVE_NOLONGNAMES);
        driveserial[x] = -1;
      }
    }
    else if(!(ulDriveMap & (1L << x)))
      driveflags[x] |= DRIVE_INVALID;
  }
  {
    ULONG  startdrive = 3L;

    DosError(FERR_DISABLEHARDERR);
    DosQuerySysInfo(QSV_BOOT_DRIVE,QSV_BOOT_DRIVE,
                    (PVOID)&startdrive,(ULONG)sizeof(ULONG));
    if(startdrive)
      driveflags[startdrive - 1] |= DRIVE_BOOT;
  }
}


CHAR * assign_ignores (CHAR *s) {

  register INT   x;
  register CHAR *p,*pp;

  *s = '/';
  s[1] = 0;
  p = s + 1;
  if(s) {
    for(x = 0;x < 26;x++) {
      if((driveflags[x] & DRIVE_IGNORE) != 0) {
        *p = (CHAR)x + 'A';
        p++;
        *p = 0;
      }
    }
  }
  if(!s[1]) {
    *s = 0;
    pp = s;
  }
  else {
    pp = &s[strlen(s)];
    *pp = ' ';
    pp++;
  }
  *pp = ';';
  pp[1] = 0;
  p = pp + 1;
  if(pp) {
    for(x = 0;x < 26;x++) {
      if((driveflags[x] & DRIVE_NOPRESCAN) != 0) {
        *p = (CHAR)x + 'A';
        p++;
        *p = 0;
      }
    }
  }
  if(!pp[1])
    *pp = 0;
  pp = &s[strlen(s)];
  *pp = ' ';
  pp++;
  *pp = ',';
  pp[1] = 0;
  p = pp + 1;
  if(pp) {
    for(x = 0;x < 26;x++) {
      if((driveflags[x] & DRIVE_NOLOADICONS) != 0) {
        *p = (CHAR)x + 'A';
        p++;
        *p = 0;
      }
    }
  }
  if(!pp[1])
    *pp = 0;
  pp = &s[strlen(s)];
  *pp = ' ';
  pp++;
  *pp = '`';
  pp[1] = 0;
  p = pp + 1;
  if(pp) {
    for(x = 0;x < 26;x++) {
      if((driveflags[x] & DRIVE_NOLOADSUBJS) != 0) {
        *p = (CHAR)x + 'A';
        p++;
        *p = 0;
      }
    }
  }
  if(!pp[1])
    *pp = 0;
  pp = &s[strlen(s)];
  *pp = ' ';
  pp++;
  *pp = '\'';
  pp[1] = 0;
  p = pp + 1;
  if(pp) {
    for(x = 0;x < 26;x++) {
      if((driveflags[x] & DRIVE_NOLOADLONGS) != 0) {
        *p = (CHAR)x + 'A';
        p++;
        *p = 0;
      }
    }
  }
  if(!pp[1])
    *pp = 0;
  lstrip(rstrip(s));
  return s;
}


BOOL needs_quoting (register CHAR *f) {

  register CHAR *p = " &|<>";

  while(*p) {
    if(strchr(f,*p))
      return TRUE;
    p++;
  }
  return FALSE;
}


BOOL IsBinary (register CHAR *str,ULONG len) {

  register ULONG x = 0L;

  if(str) {
    while(x < len) {
      if(str[x] < ' ' && str[x] != '\r' && str[x] != '\n' && str[x] != '\t' &&
         str[x] != '\x1b' && str[x] != '\x1a' && str[x] != '\07' &&
         str[x] != '\x0c')
        return TRUE;
      x++;
    }
  }
  return FALSE;
}


BOOL TestBinary (CHAR *filename) {

  HFILE   handle;
  ULONG   action,len;
  APIRET  rc;
  CHAR    buff[512];

  if(filename) {
    if(!DosOpen(filename,&handle,&action,0L,0L,
                OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NOINHERIT |
                OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYNONE |
                OPEN_ACCESS_READONLY,0L)) {
      len = 512;
      rc = DosRead(handle,buff,len,&len);
      DosClose(handle);
      if(!rc && len)
        return IsBinary(buff,len);
    }
  }
  return FALSE;
}


char *IsVowel (char a) {

  return (strchr("aeiouAEIOU",a) != NULL) ? "n" : NullStr;
}


VOID GetDesktopName (CHAR *objectpath,ULONG size) {

  PFN     WQDPath;
  HMODULE hmod = 0;
  APIRET  rc;
  ULONG   startdrive = 3L;
  CHAR    objerr[CCHMAXPATH];

  if(!objectpath)
    return;
  *objectpath = 0;
  if(OS2ver[0] > 20 || (OS2ver[0] == 20 && OS2ver[1] >= 30)) {
    /*
     * if running under warp, we can get the desktop name
     * this way...
     */
    rc = DosLoadModule(objerr,
                       sizeof(objerr),
                       "PMWP",
                       &hmod);
    if(!rc) {
      rc = DosQueryProcAddr(hmod,
                            262,
                            NULL,
                            &WQDPath);
      if(!rc)
        WQDPath(objectpath,size);
      DosFreeModule(hmod);
    }
  }
  if(!*objectpath) {
    if(!PrfQueryProfileString(HINI_SYSTEMPROFILE,
                              "FolderWorkareaRunningObjects",
                              NULL,
                              "\0",
                              (PVOID)objectpath,
                              sizeof(objectpath)))
      *objectpath = 0;
    if(!*objectpath || IsFile(objectpath)) {
      DosBeep(250,10);
      DosError(FERR_DISABLEHARDERR);
      DosQuerySysInfo(QSV_BOOT_DRIVE,QSV_BOOT_DRIVE,
                      (PVOID)&startdrive,(ULONG)sizeof(ULONG));
      sprintf(objectpath,
              "%c:\\DESKTOP",
              ((CHAR)startdrive) + '@');
    }
  }
}
