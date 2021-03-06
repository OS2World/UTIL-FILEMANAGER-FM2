
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2009 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H
  25 Dec 08 GKY Add code to allow write verify to be turned off on a per drive basis
  08 Mar 09 GKY Additional strings move to PCSZs
  22 Jul 09 GKY Consolidated driveflag setting code in DriveFlagsOne

***********************************************************************/

#if !defined(VALID_H)

#define VALID_H

VOID ArgDriveFlags(INT argc, CHAR ** argv);
INT CheckDrive(CHAR Drive, CHAR * FileSystem, ULONG * type);
VOID DriveFlagsOne(INT x, CHAR *FileSystem, VOID *volser);
VOID FillInDriveFlags(VOID * dummy);
VOID GetDesktopName(CHAR * objectpath, ULONG size);
BOOL IsBinary(CHAR * str, ULONG len);
BOOL IsExecutable(CHAR * filename);
INT IsFile(PCSZ filename);
BOOL IsFullName(CHAR * filename);
BOOL IsNewer(char *file1, char *file2);
BOOL IsRoot(PCSZ filename);
BOOL IsValidDir(CHAR * test);
BOOL IsValidDrive(CHAR drive);
APIRET MakeFullName(char *filename);
CHAR *MakeValidDir(CHAR * path);
BOOL ParentIsDesktop(HWND hwnd, HWND hwndParent);
char *RootName(char *filename);
BOOL TestBinary(CHAR * filename);
int TestCDates(CDATE *datevar1, CTIME *timevar1, CDATE *datevar2, CTIME *timevar2);
int TestFDates(char *file1, char *file2, FDATE *datevar1, FTIME *timevar1,
	       FDATE *datevar2, FTIME *timevar2);
CHAR *assign_ignores(CHAR * s);
BOOL needs_quoting(PCSZ f);

// Data declarations
extern PCSZ CDFS;
extern PCSZ FAT32;
extern PCSZ FAT;
extern PCSZ HPFS;
extern PCSZ HPFS386;
extern PCSZ ISOFS;
extern PCSZ JFS;
extern PCSZ NTFS;
extern PCSZ NDFS32;
extern PCSZ RAMFS;
extern PCSZ CBSIFS;
extern PCSZ LAN;
extern BOOL fVerifyOffChecked[26];

#endif	// VALID_H
