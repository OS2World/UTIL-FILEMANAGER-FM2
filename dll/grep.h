ULONG SecsSince1980 (FDATE *date,FTIME *time);
VOID dogrep (VOID *arg);

typedef struct DUPES {
  CHAR *name;
  ULONG size;
  FDATE date;
  FTIME time;
  INT   flags;
  LONG  CRC;
  struct DUPES *next;
} DUPES;

#define GF_INSERTED 1
#define GF_SKIPME   2

typedef struct {
  USHORT      size;
  CHAR        tosearch[8192];
  CHAR        fileMask[CCHMAXPATH + 14];
  CHAR        curdir[CCHMAXPATH];
  LONG        fileCount;
  HWND        hwnd;
  HWND        hwndFiles;
  HWND        hwndCurFile;
  BOOL        caseFlag;
  BOOL        absFlag;
  BOOL        dirFlag;
  BOOL        sayfiles;
  BOOL        searchEAs;
  BOOL        searchFiles;
  BOOL        finddupes;
  BOOL        CRCdupes;
  BOOL        nosizedupes;
  BOOL        ignoreextdupes;
  BOOL        findifany;
  BOOL        anyexcludes;
  ULONG       greaterthan;
  ULONG       lessthan;
  ULONG       olderthan;
  ULONG       newerthan;
  ULONG       numfiles;
  HAB         ghab;
  CHAR       *stopflag;
  ULONG       toinsert;
  ULONG       insertedbytes;
  FILEFINDBUF4 **insertffb;
  CHAR      **dir;
  ULONG       attrFile;
  ULONG       antiattr;
  DUPES      *dupehead,*dupelast,**dupenames,**dupesizes;
  ULONG       FilesToGet;
  CHAR        searchPattern[4096];
  ULONG       numlines;
  CHAR       *matched;
} GREP;

extern volatile CHAR diegrep;

