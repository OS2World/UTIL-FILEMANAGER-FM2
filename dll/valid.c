
/***********************************************************************

  $Id$

  File name manipulation routines

  Copyright (c) 1993, 1998 M. Kimes
  Copyright (c) 2002, 2008 Steven H.Levine

  23 Nov 02 SHL RootName: rework for sanity
  27 Nov 02 SHL MakeFullName: correct typo
  11 Jun 03 SHL Add JFS and FAT32 support
  15 Jun 04 SHL Implement Jim Read's removable logic
  31 Jul 04 SHL Comments
  01 Aug 04 SHL Rework lstrip/rstrip usage
  03 Jun 05 SHL Drop CD_DEBUG logic
  28 Nov 05 SHL MakeValidDir: correct DosQuerySysInfo args
  22 Jul 06 SHL Use Runtime_Error
  22 Oct 06 GKY Add NDFS32 support
  22 Oct 06 GKY Increased BUFFER_BYTES in CheckDrive to 8192 to fix NDFS32 scan failure
  07 Jan 07 GKY Move error strings etc. to string file
  18 Feb 07 GKY Add more drive types and icons
  16 Jun 07 SHL Update for OpenWatcom
  20 Aug 07 GKY Move #pragma alloc_text to end for OpenWatcom compat
  30 Dec 07 GKY Change TestDates to TestFDates can compare by filename or FDATE/FTIME data
  30 Dec 07 GKY Add TestCDates to compare CNRITEMs by CDATE/CTIME data
  19 Jul 08 GKY Replace save_dir2(dir) with pFM2SaveDirectory
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  03 Jan 09 GKY Check for system that is protectonly to gray out Dos/Win command lines and prevent
                Dos/Win programs from being inserted into the execute dialog with message why.
  03 Jan 08 GKY Modify IsExecutable to prevent some text files from being treated as executable
                and prevent dlls from being loaded into execute dialog.

***********************************************************************/

#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_DOSDEVIOCTL		// DosDevIOCtl
#define INCL_LONGLONG

#include "fm3dll.h"
#include "treecnr.h"			// Data declaration(s)
#include "info.h"			// Data declaration(s)
#include "notebook.h"			// Data declaration(s)
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString
#include "valid.h"
#include "dirs.h"			// save_dir2
#include "strips.h"			// bstrip
#include "init.h"			// GetTidForWindow


//static BOOL IsDesktop(HAB hab, HWND hwnd);
//static BOOL IsFileSame(CHAR * filename1, CHAR * filename2);
//static char *IsVowel(char a);

// Data definitions
static PSZ pszSrcFile = __FILE__;

#pragma data_seg(GLOBAL2)
CHAR *CDFS;
CHAR *FAT32;
CHAR *HPFS;
CHAR *HPFS386;
CHAR *ISOFS;
CHAR *JFS;
CHAR *NDFS32;
CHAR *NTFS;
CHAR *RAMFS;
BOOL fVerifyOffChecked[26];

APIRET MakeFullName(char *pszFileName)
{
  /* pszFileName must be CCHMAXPATH long minimum! */

  char szPathName[CCHMAXPATH];
  APIRET rc;

  DosError(FERR_DISABLEHARDERR);
  rc = DosQueryPathInfo(pszFileName,
			FIL_QUERYFULLNAME, szPathName, sizeof(szPathName));
  if (!rc)
    strcpy(pszFileName, szPathName);	// Pass back actual name
  return rc;
}

char *RootName(char *filename)
{
  char *p = NULL, *pp;

  // Return filename, strip path parts
  // Return empty string when filename ends with backslash

  if (filename) {
    p = strrchr(filename, '\\');
    pp = strrchr(filename, '/');
    p = (p) ? (pp) ? (p > pp) ? p : pp : p : pp;
  }
  if (!p)				/* name is itself a root */
    p = filename;
  else					/* skip past backslash */
    p++;
  return p;
}

  /** TestFDate
   * return 1 (file2 newer than file1),
   * 0 (files same)
   * or -1 (file1 newer than file2)
   * Make the FILSTATUS pointers NULL if passing file names
   * if the FILESTATUS information is already available it can be passed instead
   * Make the files NULL if passing FILESTATUS buffers
   */

int TestFDates(char *file1, char *file2, FDATE *datevar1, FTIME *timevar1,
               FDATE *datevar2, FTIME *timevar2)
{
  int comp = 0;
  FILESTATUS3 fs3o, fs3n;

  if (file1){
    DosError(FERR_DISABLEHARDERR);
    DosQueryPathInfo(file1, FIL_STANDARD, &fs3o, sizeof(fs3o));
    datevar1 = &fs3o.fdateLastWrite;
    timevar1 = &fs3o.ftimeLastWrite;
  }
  if (file2) {
    DosError(FERR_DISABLEHARDERR);
    DosQueryPathInfo(file2, FIL_STANDARD, &fs3n, sizeof(fs3n));
    datevar2 = &fs3n.fdateLastWrite;
    timevar2 = &fs3n.ftimeLastWrite;
  }
  if (&datevar1 && &datevar2 && &timevar1 && &timevar2) {
    comp = (datevar2->year >
            datevar1->year) ? 1 :
      (datevar2->year <
       datevar1->year) ? -1 :
      (datevar2->month >
       datevar1->month) ? 1 :
      (datevar2->month <
       datevar1->month) ? -1 :
      (datevar2->day >
       datevar1->day) ? 1 :
      (datevar2->day <
       datevar1->day) ? -1 :
      (timevar2->hours >
       timevar1->hours) ? 1 :
      (timevar2->hours <
       timevar1->hours) ? -1 :
      (timevar2->minutes >
       timevar1->minutes) ? 1 :
      (timevar2->minutes <
       timevar1->minutes) ? -1 :
      (timevar2->twosecs >
       timevar1->twosecs) ? 1 :
    (timevar2->twosecs < timevar1->twosecs) ? -1 : 0;
  }
    return comp;
}

  /** TestCDate
   * return 1 (file2 newer than file1),
   * 0 (files same)
   * or -1 (file1 newer than file2)
   */

int TestCDates(CDATE *datevar1, CTIME *timevar1,
               CDATE *datevar2, CTIME *timevar2)
{
  int comp = 0;

  if (&datevar1 && &datevar2 && &timevar1 && &timevar2) {
    comp = (datevar2->year >
            datevar1->year) ? 1 :
      (datevar2->year <
       datevar1->year) ? -1 :
      (datevar2->month >
       datevar1->month) ? 1 :
      (datevar2->month <
       datevar1->month) ? -1 :
      (datevar2->day >
       datevar1->day) ? 1 :
      (datevar2->day <
       datevar1->day) ? -1 :
      (timevar2->hours >
       timevar1->hours) ? 1 :
      (timevar2->hours <
       timevar1->hours) ? -1 :
      (timevar2->minutes >
       timevar1->minutes) ? 1 :
      (timevar2->minutes <
       timevar1->minutes) ? -1 :
      (timevar2->seconds >
       timevar1->seconds) ? 1 :
    (timevar2->seconds < timevar1->seconds) ? -1 : 0;
  }
    return comp;
}

BOOL IsNewer(char *file1, char *file2)
{
  /* return TRUE if file2 is newer than file1 */

  return (TestFDates(file1, file2, NULL, NULL, NULL, NULL) > 0);
}

#if 0 	// JBS	11 Sep 08
BOOL IsDesktop(HAB hab, HWND hwnd)
{
  HWND hwndDesktop;

  if (hwnd == HWND_DESKTOP)
    return TRUE;
  hwndDesktop = WinQueryDesktopWindow(hab, NULLHANDLE);
  if (hwnd == hwndDesktop)
    return TRUE;
  return FALSE;
}
#endif

BOOL ParentIsDesktop(HWND hwnd, HWND hwndParent)
{
  HWND hwndDesktop;
  BOOL ret = FALSE;

  if (!hwndParent)
    hwndParent = WinQueryWindow(hwnd, QW_PARENT);
  if (hwndParent == HWND_DESKTOP)
    ret = TRUE;
  else {
    hwndDesktop = WinQueryDesktopWindow(WinQueryAnchorBlock(hwnd), (HWND) 0);
    if (hwndDesktop == hwndParent)
      ret = TRUE;
  }
  return ret;
}

/** CheckDrive
 * @param chDrive drive letter
 * @param pszFileSystem pointer to buffer to return file system type or NULL
 * @param pulType pointer to long word to return drive flags or NULL
 * @returns removability flag, 1 = removable, 0 = not removable, -1 = error
 */

INT CheckDrive(CHAR chDrive, CHAR * pszFileSystem, ULONG * pulType)
{
  CHAR szPath[3];
  VOID *pvBuffer = NULL;
  CHAR *pfsn;
  CHAR *pfsd;
  ULONG clBufferSize;
  APIRET rc;
  ULONG ulAction;
  ULONG clParmBytes;
  ULONG clDataBytes;
  HFILE hDev;

# pragma pack(1)
  struct
  {
    BYTE Cmd;
    BYTE Unit;
  }
  parmPkt =
  {
  0, 0};
# define BPB_REMOVABLE_MEDIA	0x08	// 3 - Media is removable
  struct
  {
    BIOSPARAMETERBLOCK bpb;
    USHORT cCylinders;		// Documented but not implemented
    BYTE bDeviceType;		// Documented but not implemented
    USHORT fsDeviceAttr;	// Documented but not implemented
  }
  dataPkt;

# pragma pack()
  BYTE NonRemovable;
  PFSQBUFFER2 pfsq;

  if (pszFileSystem)
    *pszFileSystem = 0;

  if (pulType)
    *pulType = 0;

# define BUFFER_BYTES 8192
  rc = DosAllocMem(&pvBuffer, BUFFER_BYTES,
		PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
  if (rc) {
    Dos_Error(MB_CANCEL, rc, HWND_DESKTOP, pszSrcFile, __LINE__,
	      GetPString(IDS_OUTOFMEMORY));
    return -1;				// Say failed
  }

  szPath[0] = chDrive;
  szPath[1] = ':';
  szPath[2] = 0;
  clBufferSize = BUFFER_BYTES;
  DosError(FERR_DISABLEHARDERR);
  rc = DosQueryFSAttach(szPath, 0, FSAIL_QUERYNAME,
			(PFSQBUFFER2) pvBuffer, &clBufferSize);
  if (rc) {
    /* can't get any info at all */
    DosFreeMem(pvBuffer);
    DosError(FERR_DISABLEHARDERR);
    return -1;				// Say failed
  }

  pfsq = (PFSQBUFFER2) pvBuffer;
  pfsn = (PCHAR)(pfsq->szName) + pfsq->cbName + 1;
  pfsd = pfsn + pfsq->cbFSDName + 1;

  if (pszFileSystem) {
    strncpy(pszFileSystem, pfsn, CCHMAXPATH);
    pszFileSystem[CCHMAXPATH - 1] = 0;
  }

  if (pulType && (!strcmp(pfsn, CDFS) || !strcmp(pfsn, ISOFS)))
      *pulType |= DRIVE_NOTWRITEABLE | DRIVE_CDROM | DRIVE_REMOVABLE;
  if (pulType && !strcmp(pfsn, NTFS))
    *pulType |= DRIVE_NOTWRITEABLE;
  if (pulType && !strcmp(pfsn, NDFS32)){
        *pulType |= DRIVE_VIRTUAL;
    }
  if (pulType && !strcmp(pfsn, RAMFS)){
        *pulType |= DRIVE_RAMDISK;
    }
  if (((PFSQBUFFER2) pvBuffer)->iType == FSAT_REMOTEDRV &&
      (strcmp(pfsn, CDFS) || strcmp(pfsn, ISOFS))) {
    if (pulType)
      *pulType |= DRIVE_REMOTE;

    if (pulType && !strcmp(pfsn, CBSIFS)) {
      *pulType |= DRIVE_ZIPSTREAM;
      *pulType &= ~DRIVE_REMOTE;
      *pulType |= DRIVE_NOLONGNAMES;
      if (pfsq->cbFSAData) {
	ULONG FType;

	if (CheckDrive(*pfsd, NULL, &FType) != -1) {
	  if (FType & DRIVE_REMOVABLE)
	    *pulType |= DRIVE_REMOVABLE;
	  if (~FType & DRIVE_NOLONGNAMES)
	    *pulType &= ~DRIVE_NOLONGNAMES;
	}

      }
    }
    if (pulType &&
	(!strcmp(pfsn, HPFS) ||
	 !strcmp(pfsn, JFS) ||
	 !strcmp(pfsn, FAT32) ||
         !strcmp(pfsn, RAMFS) ||
         !strcmp(pfsn, NDFS32) ||
         !strcmp(pfsn, NTFS) ||
         !strcmp(pfsn, HPFS386))) {
      *pulType &= ~DRIVE_NOLONGNAMES;
    }

    DosFreeMem(pvBuffer);
    return 0;				// Remotes are non-removable
  }

  // Local drive
  if (strcmp(pfsn, HPFS) &&
      strcmp(pfsn, JFS) &&
      strcmp(pfsn, CDFS) &&
      strcmp(pfsn, ISOFS) &&
      strcmp(pfsn, RAMFS) &&
      strcmp(pfsn, FAT32) &&
      strcmp(pfsn, NDFS32) &&
      strcmp(pfsn, NTFS) &&
      strcmp(pfsn, HPFS386)) {
    if (pulType)
      (*pulType) |= DRIVE_NOLONGNAMES;	// Others can not have long names
  }


  DosError(FERR_DISABLEHARDERR);
  rc = DosOpen(szPath, &hDev, &ulAction, 0, 0, FILE_OPEN,
	       OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE |
	       OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR, 0);
  if (rc) {
    DosError(FERR_DISABLEHARDERR);
    if (pulType)
      *pulType |= DRIVE_REMOVABLE;	// Assume removable if can not access
    DosFreeMem(pvBuffer);
    return 1;				// Say removable
  }

  clParmBytes = sizeof(parmPkt.Cmd);
  clDataBytes = sizeof(NonRemovable);
  NonRemovable = 1;			// Preset as non removable
  DosError(FERR_DISABLEHARDERR);
  rc = DosDevIOCtl(hDev, IOCTL_DISK, DSK_BLOCKREMOVABLE, &parmPkt.Cmd,	/*  Address of the command-specific argument list. */
		   sizeof(parmPkt.Cmd),	/*  Length, in bytes, of pParams. */
		   &clParmBytes,	/*  Pointer to the length of parameters. */
		   &NonRemovable,	/*  Address of the data area. */
		   sizeof(NonRemovable),	/*  Length, in bytes, of pData. */
		   &clDataBytes);	/*  Pointer to the length of data. */

  if (!rc && NonRemovable) {
    // Could be USB so check BPB flags
    clParmBytes = sizeof(parmPkt.Cmd);
    clDataBytes = sizeof(dataPkt);
    memset(&dataPkt, 0xff, sizeof(dataPkt));
    DosError(FERR_DISABLEHARDERR);
    rc = DosDevIOCtl(hDev, IOCTL_DISK, DSK_GETDEVICEPARAMS, &parmPkt.Cmd,	/*  Address of the command-specific argument list. */
		     sizeof(parmPkt.Cmd),	/*  Length, in bytes, of pParams. */
		     &clParmBytes,	/*  Pointer to the length of parameters. */
		     &dataPkt,		/*  Address of the data area. */
		     sizeof(dataPkt),	/*  Length, in bytes, of pData. */
		     &clDataBytes);	/*  Pointer to the length of data. */

    if (!rc && (dataPkt.bpb.fsDeviceAttr & BPB_REMOVABLE_MEDIA))
      NonRemovable = 0;
  }

  DosClose(hDev);

  if (!NonRemovable && pulType)
    *pulType |= DRIVE_REMOVABLE;

  DosFreeMem(pvBuffer);

  return NonRemovable ? 0 : 1;
}

#if 0	// JBS	11 Sep 08
BOOL IsFileSame(CHAR * filename1, CHAR * filename2)
{
  /* returns:  -1 (error), 0 (is a directory), or 1 (is a file) */

  FILESTATUS3L fsa1, fsa2;
  APIRET ret;

  if (filename1 && filename2) {
    DosError(FERR_DISABLEHARDERR);
    ret = DosQueryPathInfo(filename1, FIL_STANDARDL, &fsa1,
			   (ULONG) sizeof(fsa1));
    if (!ret) {
      DosError(FERR_DISABLEHARDERR);
      ret = DosQueryPathInfo(filename2, FIL_STANDARDL, &fsa2,
			     (ULONG) sizeof(fsa2));
      if (!ret) {
	if (fsa1.cbFile == fsa2.cbFile &&
	    (fsa1.attrFile & (~FILE_ARCHIVED)) ==
	    (fsa2.attrFile & (~FILE_ARCHIVED)))
	  return TRUE;
      }
    }
  }
  return FALSE;
}
#endif

INT IsFile(CHAR * filename)
{
  /* returns:  -1 (error), 0 (is a directory), or 1 (is a file) */

  FILESTATUS3 fsa;
  APIRET ret;

  if (filename && *filename) {
    DosError(FERR_DISABLEHARDERR);
    ret = DosQueryPathInfo(filename, FIL_STANDARD, &fsa, (ULONG) sizeof(fsa));
    if (!ret)
      return ((fsa.attrFile & FILE_DIRECTORY) == 0);
    else if (IsValidDrive(*filename) && IsRoot(filename))
      return 0;
  }
  return -1;				/* error; doesn't exist or can't read or null filename */
}

BOOL IsFullName(CHAR * filename)
{
  return (filename) ?
    (isalpha(*filename) && filename[1] == ':' && filename[2] == '\\') : 0;
}

BOOL IsRoot(CHAR * filename)
{
  return (filename && isalpha(*filename) && filename[1] == ':' &&
	  filename[2] == '\\' && !filename[3]);
}

BOOL IsValidDir(CHAR * path)
{
  CHAR fullname[CCHMAXPATH];
  FILESTATUS3 fs;

  if (path) {
    DosError(FERR_DISABLEHARDERR);
    if (!DosQueryPathInfo(path,
			  FIL_QUERYFULLNAME, fullname, sizeof(fullname))) {
      if (IsValidDrive(*fullname)) {
	if (!IsRoot(fullname)) {
	  DosError(FERR_DISABLEHARDERR);
	  if (!DosQueryPathInfo(fullname,
				FIL_STANDARD,
				&fs,
				sizeof(fs)) && (fs.attrFile & FILE_DIRECTORY))
	    return TRUE;
	}
	else
	  return TRUE;
      }
    }
  }
  return FALSE;
}

BOOL IsValidDrive(CHAR drive)
{
  CHAR Path[] = " :", Buffer[256];
  APIRET Status;
  ULONG Size;
  ULONG ulDriveNum, ulDriveMap;

  if (!isalpha(drive) ||
      (driveflags[toupper(drive) - 'A'] & (DRIVE_IGNORE | DRIVE_INVALID)))
    return FALSE;
  DosError(FERR_DISABLEHARDERR);
  Status = DosQCurDisk(&ulDriveNum, &ulDriveMap);
  if (!Status) {
    if (!(ulDriveMap & (1 << (ULONG) (toupper(drive) - 'A'))))
      return FALSE;
    Path[0] = toupper(drive);
    Size = sizeof(Buffer);
    DosError(FERR_DISABLEHARDERR);
    Status = DosQueryFSAttach(Path,
			      0,
			      FSAIL_QUERYNAME, (PFSQBUFFER2) Buffer, &Size);
  }
  return (Status == 0);
}

//=== MakeValidDir() build valid directory name ===

CHAR *MakeValidDir(CHAR * path)
{
  ULONG ulDrv;
  CHAR *p;
  FILESTATUS3 fs;
  APIRET rc;

  if (!MakeFullName(path)) {
    if (IsValidDrive(*path)) {
      // Passed name is valid - trim to directory
      for (;;) {
	if (IsRoot(path))
	  return path;
	DosError(FERR_DISABLEHARDERR);
	rc = DosQueryPathInfo(path, FIL_STANDARD, &fs, sizeof(fs));
	if (!rc && (fs.attrFile & FILE_DIRECTORY))
	  return path;
	p = strrchr(path, '\\');
	if (p) {
	  if (p < path + 3)
	    p++;
	  *p = 0;
	}
	else
	  break;
      }
    }
  }
  // Fall back to boot drive
  DosError(FERR_DISABLEHARDERR);
  if (!DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &ulDrv, sizeof(ulDrv))) {
    ulDrv += '@';
    if (ulDrv < 'C')
      ulDrv = 'C';
    strcpy(path, " :\\");
    *path = (CHAR) ulDrv;
  }
  else
    strcpy(path, pFM2SaveDirectory);			// Fall back to fm3.ini drive or current dir - should never occur
  return path;
}

BOOL IsExecutable(CHAR * filename)
{
  register CHAR *p;
  APIRET ret;
  ULONG apptype;

  if (filename) {
    DosError(FERR_DISABLEHARDERR);
    p = strrchr(filename, '.');
    if (p)
      ret = DosQueryAppType(filename, &apptype);
    else {

      char fname[CCHMAXPATH + 2];

      strcpy(fname, filename);
      strcat(fname, ".");
      ret = DosQueryAppType(fname, &apptype);
    }
    if (apptype & (FAPPTYP_DLL |
                   FAPPTYP_PHYSDRV |
                   FAPPTYP_VIRTDRV |
                   FAPPTYP_PROTDLL))
      return FALSE;
    if (apptype == 0x000b && (!p ||
        (stricmp(p, ".EXE") &&
         stricmp(p, ".COM") &&
         stricmp(p, ".CMD") &&
         stricmp(p, ".BAT") &&
         stricmp(p, ".BTM"))))
      return FALSE;
    if (!fProtectOnly) {
      if ((!ret && (!apptype ||
                    (apptype &
                     (FAPPTYP_NOTWINDOWCOMPAT |
                      FAPPTYP_WINDOWCOMPAT |
                      FAPPTYP_WINDOWAPI |
                      FAPPTYP_BOUND |
                      FAPPTYP_DOS |
                      FAPPTYP_WINDOWSREAL |
                      FAPPTYP_WINDOWSPROT |
                      FAPPTYP_32BIT |
                      FAPPTYP_WINDOWSPROT31)))) ||
          (p && (!stricmp(p, ".CMD") || !stricmp(p, ".BAT") || !stricmp(p, ".BTM"))))
        return TRUE;
    }
    else if ((!ret && (!apptype ||
                       (apptype &
                        (FAPPTYP_NOTWINDOWCOMPAT |
                         FAPPTYP_WINDOWCOMPAT |
                         FAPPTYP_WINDOWAPI |
                         FAPPTYP_BOUND |
                         FAPPTYP_32BIT)))) ||
             (p && (!stricmp(p, ".CMD") || !stricmp(p, ".BTM"))))
      return TRUE;
    if (fProtectOnly && (apptype &
                         (FAPPTYP_DOS |
                          FAPPTYP_WINDOWSREAL |
                          FAPPTYP_WINDOWSPROT |
                          FAPPTYP_WINDOWSPROT31)) &&
        (p && (!stricmp(p, ".EXE") || !stricmp(p, ".COM"))))
      saymsg(MB_OK,
             HWND_DESKTOP,
             NullStr,
             GetPString(IDS_NOTPROTECTONLYEXE),
             filename);
  }
  return FALSE;
}

VOID ArgDriveFlags(INT argc, CHAR ** argv)
{
  INT x;

  for (x = 1; x < argc; x++) {
    if (*argv[x] == '/' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while (isalpha(*p)) {
	driveflags[toupper(*p) - 'A'] |= DRIVE_IGNORE;
	p++;
      }
    }
    else if (*argv[x] == ';' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while (isalpha(*p)) {
	driveflags[toupper(*p) - 'A'] |= DRIVE_NOPRESCAN;
	p++;
      }
    }
     else if (*argv[x] == '`' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while (isalpha(*p)) {
	driveflags[toupper(*p) - 'A'] |= DRIVE_NOSTATS;
	p++;
      }
    }
    else if (*argv[x] == ',' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while (isalpha(*p)) {
	driveflags[toupper(*p) - 'A'] |= DRIVE_NOLOADICONS;
	p++;
      }
    }
    else if (*argv[x] == '-' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while (isalpha(*p)) {
	driveflags[toupper(*p) - 'A'] |= DRIVE_NOLOADSUBJS;
	p++;
      }
    }
    else if (*argv[x] == '\'' && isalpha(argv[x][1])) {

      CHAR *p = &argv[x][1];

      while (isalpha(*p)) {
	driveflags[toupper(*p) - 'A'] |= DRIVE_NOLOADLONGS;
	p++;
      }
    }
  }
}

VOID DriveFlagsOne(INT x)
{
  INT removable;
  CHAR szDrive[] = " :\\", FileSystem[CCHMAXPATH];
  ULONG drvtype;

  *szDrive = (CHAR) (x + 'A');
  *FileSystem = 0;
  drvtype = 0;
  removable = CheckDrive(*szDrive, FileSystem, &drvtype);
  driveserial[x] = -1;
  driveflags[x] &= (DRIVE_IGNORE | DRIVE_NOPRESCAN | DRIVE_NOLOADICONS |
		    DRIVE_NOLOADSUBJS | DRIVE_NOLOADLONGS |
                    DRIVE_INCLUDEFILES | DRIVE_SLOW | DRIVE_NOSTATS |
                    DRIVE_WRITEVERIFYOFF);
  if (removable != -1) {
    struct
    {
      ULONG serial;
      CHAR volumelength;
      CHAR volumelabel[CCHMAXPATH];
    }
    volser;

    DosError(FERR_DISABLEHARDERR);
    if (!DosQueryFSInfo((ULONG) x + 1, FSIL_VOLSER, &volser, sizeof(volser)))
      driveserial[x] = volser.serial;
    else
      DosError(FERR_DISABLEHARDERR);
  }
  else
    driveflags[x] |= DRIVE_INVALID;
  driveflags[x] |= ((removable == -1 || removable == 1) ?
		    DRIVE_REMOVABLE : 0);
  if (drvtype & DRIVE_REMOTE)
    driveflags[x] |= DRIVE_REMOTE;
  if(!stricmp(FileSystem,NDFS32)){
    driveflags[x] |= DRIVE_VIRTUAL;
    driveflags[x] &= (~DRIVE_REMOTE);
  }
  if(!stricmp(FileSystem,RAMFS)){
    driveflags[x] |= DRIVE_RAMDISK;
    driveflags[x] &= (~DRIVE_REMOTE);
  }
  if(!stricmp(FileSystem,NTFS))
    driveflags[x] |= DRIVE_NOTWRITEABLE;
  if (strcmp(FileSystem, HPFS) &&
      strcmp(FileSystem, JFS) &&
      strcmp(FileSystem, CDFS) &&
      strcmp(FileSystem, ISOFS) &&
      strcmp(FileSystem, RAMFS) &&
      strcmp(FileSystem, FAT32) &&
      strcmp(FileSystem, NTFS) &&
      strcmp(FileSystem, NDFS32) &&
      strcmp(FileSystem, HPFS386)) {
    driveflags[x] |= DRIVE_NOLONGNAMES;
  }

  if (!strcmp(FileSystem, CDFS) || !strcmp(FileSystem, ISOFS)) {
    removable = 1;
    driveflags[x] |= (DRIVE_REMOVABLE | DRIVE_NOTWRITEABLE | DRIVE_CDROM);
  }
  if (!stricmp(FileSystem, CBSIFS)) {
    driveflags[x] |= DRIVE_ZIPSTREAM;
    driveflags[x] &= (~DRIVE_REMOTE);
    if (drvtype & DRIVE_REMOVABLE)
      driveflags[x] |= DRIVE_REMOVABLE;
    if (!(drvtype & DRIVE_NOLONGNAMES))
      driveflags[x] &= (~DRIVE_NOLONGNAMES);
  }
}

VOID FillInDriveFlags(VOID * dummy)
{
  ULONG ulDriveNum, ulDriveMap, size;
  register INT x;

  for (x = 0; x < 26; x++)
    driveflags[x] &= (DRIVE_IGNORE | DRIVE_NOPRESCAN | DRIVE_NOLOADICONS |
		      DRIVE_NOLOADSUBJS | DRIVE_NOLOADLONGS |
                      DRIVE_INCLUDEFILES | DRIVE_SLOW | DRIVE_NOSTATS |
                      DRIVE_WRITEVERIFYOFF);
  memset(driveserial, -1, sizeof(driveserial));
  DosError(FERR_DISABLEHARDERR);
  DosQCurDisk(&ulDriveNum, &ulDriveMap);
  for (x = 0; x < 26; x++) {
    if (ulDriveMap & (1 << x) && !(driveflags[x] & DRIVE_IGNORE)) {
      {
        ULONG flags = 0, size = sizeof(ULONG);
        CHAR FlagKey[80];

          sprintf(FlagKey, "%c.DriveFlags", (CHAR) (x + 'A'));
          if (PrfQueryProfileData(fmprof, appname, FlagKey, &flags, &size) &&
              size == sizeof(ULONG))
            driveflags[x] |= flags;
      }

      if (x > 1) {
	if (!(driveflags[x] & DRIVE_NOPRESCAN))
	  DriveFlagsOne(x);
	else
	  driveserial[x] = -1;
      }
      else {
	driveflags[x] |= (DRIVE_REMOVABLE | DRIVE_NOLONGNAMES);
	driveserial[x] = -1;
      }
    }
    else if (!(ulDriveMap & (1 << x)))
      driveflags[x] |= DRIVE_INVALID;
  }
  {
    ULONG startdrive = 3L;

    DosError(FERR_DISABLEHARDERR);
    DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
		    (PVOID) & startdrive, (ULONG) sizeof(ULONG));
    if (startdrive)
      driveflags[startdrive - 1] |= DRIVE_BOOT;
  }
  {
    INT x;
    CHAR Key[80];

    for (x = 2; x < 26; x++) {
      sprintf(Key, "%c.VerifyOffChecked", (CHAR) (x + 'A'));
      size = sizeof(BOOL);
      PrfQueryProfileData(fmprof, appname, Key, &fVerifyOffChecked[x], &size);
      if (!fVerifyOffChecked[x]) {
        if (driveflags[x] & DRIVE_REMOVABLE)
          driveflags[x] |= DRIVE_WRITEVERIFYOFF;
        if (!(driveflags[x] & DRIVE_INVALID)) {
          fVerifyOffChecked[x] = TRUE;
          PrfWriteProfileData(fmprof, appname, Key, &fVerifyOffChecked[x], sizeof(BOOL));
        }
      }
    }
  }
}

CHAR *assign_ignores(CHAR * s)
{
  register INT x;
  register CHAR *p, *pp;

  *s = '/';
  s[1] = 0;
  p = s + 1;
  if (s) {
    for (x = 0; x < 26; x++) {
      if ((driveflags[x] & DRIVE_IGNORE) != 0) {
	*p = (CHAR) x + 'A';
	p++;
	*p = 0;
      }
    }
  }
  if (!s[1]) {
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
  if (pp) {
    for (x = 0; x < 26; x++) {
      if ((driveflags[x] & DRIVE_NOPRESCAN) != 0) {
	*p = (CHAR) x + 'A';
	p++;
	*p = 0;
      }
    }
  }
  if (!pp[1])
    *pp = 0;
  pp = &s[strlen(s)];
  *pp = ' ';
  pp++;
  *pp = ',';
  pp[1] = 0;
  p = pp + 1;
  if (pp) {
    for (x = 0; x < 26; x++) {
      if ((driveflags[x] & DRIVE_NOLOADICONS) != 0) {
	*p = (CHAR) x + 'A';
	p++;
	*p = 0;
      }
    }
  }
  if (!pp[1])
    *pp = 0;
  pp = &s[strlen(s)];
  *pp = ' ';
  pp++;
  *pp = '-';
  pp[1] = 0;
  p = pp + 1;
  if (pp) {
    for (x = 0; x < 26; x++) {
      if ((driveflags[x] & DRIVE_NOLOADSUBJS) != 0) {
	*p = (CHAR) x + 'A';
	p++;
	*p = 0;
      }
    }
  }
  if (!pp[1])
    *pp = 0;
  pp = &s[strlen(s)];
  *pp = ' ';
  pp++;
  *pp = '`';
  pp[1] = 0;
  p = pp + 1;
  if (pp) {
    for (x = 0; x < 26; x++) {
      if ((driveflags[x] & DRIVE_NOSTATS) != 0) {
	*p = (CHAR) x + 'A';
	p++;
	*p = 0;
      }
    }
  }
  if (!pp[1])
    *pp = 0;
  pp = &s[strlen(s)];
  *pp = ' ';
  pp++;
  *pp = '\'';
  pp[1] = 0;
  p = pp + 1;
  if (pp) {
    for (x = 0; x < 26; x++) {
      if ((driveflags[x] & DRIVE_NOLOADLONGS) != 0) {
	*p = (CHAR) x + 'A';
	p++;
	*p = 0;
      }
    }
  }
  if (!pp[1])
    *pp = 0;
  bstrip(s);
  return s;
}

BOOL needs_quoting(register CHAR * f)
{
  register CHAR *p = " &|<>";

  while (*p) {
    if (strchr(f, *p))
      return TRUE;
    p++;
  }
  return FALSE;
}

BOOL IsBinary(register CHAR * str, ULONG len)
{
  register ULONG x = 0;

  if (str) {
    while (x < len) {
      if ((UINT) str[x] < ' ' && str[x] != '\r' && str[x] != '\n' && str[x] != '\t'
	  && str[x] != '\x1b' && str[x] != '\x1a' && str[x] != '\x07'
          && str[x] != '\x0c') {
        //DbgMsg(pszSrcFile, __LINE__, "IsBinary str %x x %x len %x", str[x], x, len);
        return TRUE;
      }
      x++;
    }
  }
  return FALSE;
}

BOOL TestBinary(CHAR * filename)
{
  HFILE handle;
  ULONG ulAction;
  ULONG len;
  APIRET rc;
  CHAR buff[4096];			// 06 Oct 07 SHL protect against NTFS defect

  if (filename) {
    if (!DosOpen(filename, &handle, &ulAction, 0, 0,
		 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
		 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NOINHERIT |
		 OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYNONE |
		 OPEN_ACCESS_READONLY, 0)) {
      len = 512;
      rc = DosRead(handle, buff, len, &len);
      DosClose(handle);
      if (!rc && len)
	return IsBinary(buff, len);
    }
  }
  return FALSE;
}

#if 0	// JBS	11 Sep 08
char *IsVowel(char a)
{
  return (strchr("aeiouAEIOU", a) != NULL) ? "n" : NullStr;
}
#endif

VOID GetDesktopName(CHAR * objectpath, ULONG size)
{
  PFN WQDPath;
  HMODULE hmod = 0;
  APIRET rc;
  ULONG startdrive = 3;
  CHAR objerr[CCHMAXPATH];

  if (!objectpath) {
    Runtime_Error(pszSrcFile, __LINE__, "null pointer");
    return;
  }
  *objectpath = 0;
  if (OS2ver[0] > 20 || (OS2ver[0] == 20 && OS2ver[1] >= 30)) {
    /*
     * if running under warp, we can get the desktop name
     * this way...
     */
    rc = DosLoadModule(objerr, sizeof(objerr), "PMWP", &hmod);
    if (!rc) {
      rc = DosQueryProcAddr(hmod, 262, NULL, &WQDPath);
      if (!rc)
	WQDPath(objectpath, size);
      DosFreeModule(hmod);
    }
  }
  if (!*objectpath) {
    // Fall back to INI content
    if (!PrfQueryProfileString(HINI_SYSTEMPROFILE,
			       "FolderWorkareaRunningObjects",
			       NULL,
			       "\0",
			       (PVOID) objectpath, sizeof(objectpath))) {
      Win_Error(HWND_DESKTOP, HWND_DESKTOP, pszSrcFile, __LINE__,
		"PrfQueryProfileString");
      *objectpath = 0;
    }
    else if (!*objectpath || IsFile(objectpath)) {
      Runtime_Error(pszSrcFile, __LINE__, "bad FolderWorkareaRunningObjects");
      *objectpath = 0;
    }
    if (!*objectpath) {
      // Fall back
      DosError(FERR_DISABLEHARDERR);
      DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
		      (PVOID) & startdrive, (ULONG) sizeof(ULONG));
      sprintf(objectpath, GetPString(IDS_PATHTODESKTOP), ((CHAR) startdrive) + '@');
    }
  }
}

#pragma alloc_text(VALID,CheckDrive,IsRoot,IsFile,IsFullName,needsquoting)
#pragma alloc_text(VALID,IsValidDir,IsValidDrive,MakeValidDir,IsVowel)
#pragma alloc_text(VALID,IsFileSame,IsNewer,TestFDates,TestCDates,RootName,MakeFullName)
#pragma alloc_text(VALID,IsExecutable,IsBinary,IsDesktop,ParentIsDesktop)
#pragma alloc_text(FILLFLAGS,FillInDriveFlags,assign_ignores)
#pragma alloc_text(FILLFLAGS,ArgDriveFlags,DriveFlagsOne)
#pragma alloc_text(FINDDESK,GetDesktopName)
