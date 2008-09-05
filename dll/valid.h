VOID ArgDriveFlags(INT argc, CHAR ** argv);
VOID DriveFlagsOne(INT x);
VOID FillInDriveFlags(VOID * dummy);
VOID GetDesktopName(CHAR * objectpath, ULONG size);
BOOL IsBinary(CHAR * str, ULONG len);
BOOL IsExecutable(CHAR * filename);
BOOL IsNewer(char *file1, char *file2);
BOOL IsValidDir(CHAR * test);
BOOL IsValidDrive(CHAR drive);
int TestCDates(CDATE *datevar1, CTIME *timevar1, CDATE *datevar2, CTIME *timevar2);
int TestFDates(char *file1, char *file2, FDATE *datevar1, FTIME *timevar1,
	       FDATE *datevar2, FTIME *timevar2);

