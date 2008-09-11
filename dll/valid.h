
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H

***********************************************************************/

#if !defined(VALID_H)

#define VALID_H

VOID ArgDriveFlags(INT argc, CHAR ** argv);
INT CheckDrive(CHAR Drive, CHAR * FileSystem, ULONG * type);
VOID DriveFlagsOne(INT x);
VOID FillInDriveFlags(VOID * dummy);
VOID GetDesktopName(CHAR * objectpath, ULONG size);
BOOL IsBinary(CHAR * str, ULONG len);
BOOL IsExecutable(CHAR * filename);
INT IsFile(CHAR * filename);
BOOL IsFullName(CHAR * filename);
BOOL IsNewer(char *file1, char *file2);
BOOL IsRoot(CHAR * filename);
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
BOOL needs_quoting(CHAR * f);

#endif	// VALID_H
