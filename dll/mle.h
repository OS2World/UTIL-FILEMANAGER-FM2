/**************************************************************************/
/*                  MLE text editor/viewer source code                    */
/*                    copyright (c) 1993 by M. Kimes                      */
/*                        All rights reserved                             */
/**************************************************************************/

/* MLE support macros */

/* return number of lines in file */
#define MLEnumlines(h) (LONG)WinSendMsg((h),MLM_QUERYLINECOUNT,\
                       MPVOID,MPVOID)

/* return line # from position i */
#define MLElinefrompos(h,i) (LONG)WinSendMsg((h),MLM_LINEFROMCHAR,\
                            MPFROMLONG((LONG)(i)),MPVOID)

/* return current position of cursor */
#define MLEcurpos(h) (IPT)WinSendMsg((h),MLM_QUERYSEL,\
                     MPFROM2SHORT(MLFQS_CURSORSEL,0),MPVOID)

/* return current position of anchor */
#define MLEancpos(h) (IPT)WinSendMsg((h),MLM_QUERYSEL,\
                     MPFROM2SHORT(MLFQS_ANCHORSEL,0),MPVOID)

/* return current line # cursor is on */
#define MLEcurline(h) (LONG)WinSendMsg((h),MLM_LINEFROMCHAR,\
                      MPFROMLONG(WinSendMsg((h),MLM_QUERYSEL,\
                      MPFROM2SHORT(MLFQS_CURSORSEL,0),MPVOID)),MPVOID)

/* return current line # anchor is on */
#define MLEancline(h) (LONG)WinSendMsg((h),MLM_LINEFROMCHAR,\
                      MPFROMLONG(WinSendMsg((h),MLM_QUERYSEL,\
                      MPFROM2SHORT(MLFQS_CURSORSEL,0),MPVOID)),MPVOID)

/* return len of remainder of line at cursor position */
#define MLEcurlenleft(h) (LONG)WinSendMsg((h),MLM_QUERYFORMATLINELENGTH,\
                         MPFROMLONG(-1L),MPVOID)

/* return len of remainder of line at position i */
#define MLElinelenleft(h,i) (LONG)WinSendMsg((h),MLM_QUERYFORMATLINELENGTH,\
                            MPFROMLONG((LONG)i),MPVOID)

/* return total len of current line */
#define MLEcurlinelen(h) (LONG)WinSendMsg((h),MLM_QUERYFORMATLINELENGTH,\
                         MPFROMLONG(WinSendMsg((h),MLM_CHARFROMLINE,\
                         MPFROMLONG(-1L),MPVOID)),MPVOID)

/* return total len of line # l */
#define MLElinelen(h,l) (LONG)WinSendMsg((h),MLM_QUERYFORMATLINELENGTH,\
                        MPFROMLONG(WinSendMsg((h),MLM_CHARFROMLINE,\
                        MPFROMLONG((l)),MPVOID)),MPVOID)

/* return start pos of current line cursor is on */
#define MLEstartofcurline(h) (IPT)WinSendMsg((h),MLM_CHARFROMLINE,\
                             MPFROMLONG(-1L),MPVOID)

/* return start pos of line l */
#define MLEstartofline(h,l) (IPT)WinSendMsg((h),MLM_CHARFROMLINE,\
                            MPFROMLONG((l)),MPVOID)

/* copy selected text into buffer b, return len of text copied */
#define MLEgetseltext(h,b) (LONG)WinSendMsg((h),MLM_QUERYSELTEXT,\
                           MPFROMP((b)),MPVOID)

/* insert text s into selection */
#define MLEinsert(h,s) WinSendMsg((h),MLM_INSERT,MPFROMP((s)),MPVOID)

/* delete text from pos f to pos t */
#define MLEdelete(h,f,t) (LONG)WinSendMsg(h,MLM_DELETE,\
                         MPFROMLONG((IPT)(f)),\
                         MPFROMLONG((LONG)(t)))

/* set current cursor (and anchor) position to i */
#define MLEsetcurpos(h,i) WinSendMsg((h),MLM_SETSEL,\
                          MPFROMLONG((LONG)(i)),MPFROMLONG((LONG)(i)))

/* set current cursor position to i, don't move anchor */
#define MLEsetcurposc(h,i) WinSendMsg((h),MLM_SETSEL,\
                           MPFROMLONG((LONG)(i)),MPFROMLONG(-1L))

/* set current anchor position to i, don't move cursor */
#define MLEsetcurposa(h,i) WinSendMsg((h),MLM_SETSEL,\
                           MPFROMLONG(-1L),MPFROMLONG((LONG)(i)))

/* set first line in window to line # l */
#define MLEsettopline(h,l) WinSendMsg((h),MLM_SETFIRSTCHAR,(MPARAM)\
                           WinSendMsg((h),MLM_CHARFROMLINE,\
                           WinSendMsg((h),MLM_LINEFROMCHAR,\
                           MPFROMLONG((LONG)(l)),MPVOID),\
                           MPVOID),MPVOID)

/* set changed flag to boolean state b */
#define MLEsetchanged(h,b) WinSendMsg((h),MLM_SETCHANGED,\
                           MPFROM2SHORT((b),0),MPVOID)

/* get changed flag */
#define MLEgetchanged(h) (BOOL)WinSendMsg((h),MLM_QUERYCHANGED,MPVOID,MPVOID)

/* set MLE wrap to boolean state b */
#define MLEsetwrap(h,b) WinSendMsg((h),MLM_SETWRAP,MPFROM2SHORT((b),0),\
                        MPVOID)

/* return MLE wrap boolean state */
#define MLEgetwrap(h) (BOOL)WinSendMsg((h),MLM_QUERYWRAP,MPVOID,MPVOID)

/* set MLE readonly to boolean state b */
#define MLEsetreadonly(h,b) WinSendMsg((h),MLM_SETREADONLY,\
                            MPFROM2SHORT((b),0),MPVOID)

/* return MLE readonly boolean state */
#define MLEgetreadonly(h) (BOOL)WinSendMsg((h),MLM_QUERYREADONLY,MPVOID,MPVOID)

/* set text limit to l */
#define MLEsetlimit(h,l) WinSendMsg((h),MLM_SETTEXTLIMIT,\
                         MPFROMLONG((LONG)(l)),MPVOID)

/* return text limit */
#define MLEgetlimit(h) (LONG)WinSendMsg((h),MLM_QUERYFORMATTEXTLIMIT,MPVOID,MPVOID)

/* set format to format f */
#define MLEsetformat(h,f) WinSendMsg((h),MLM_FORMAT,\
                          MPFROM2SHORT((f),0), MPVOID)

/* disable MLE refresh */
#define MLEdisable(h) WinSendMsg((h),MLM_DISABLEREFRESH,MPVOID,MPVOID)

/* enable MLE refresh */
#define MLEenable(h) WinSendMsg((h),MLM_ENABLEREFRESH,MPVOID,MPVOID)

/* clear (cut and discard) current selection */
#define MLEclear(h) WinSendMsg((h),MLM_CLEAR,MPVOID,MPVOID)

/* cut current selection to clipboard */
#define MLEcut(h) WinSendMsg((h),MLM_CUT,MPVOID,MPVOID)

/* copy current selection to clipboard */
#define MLEcopy(h) WinSendMsg((h),MLM_COPY,MPVOID,MPVOID)

/* paste clipboard into current selection */
#define MLEpaste(h) WinSendMsg((h),MLM_PASTE,MPVOID,MPVOID)

/* undo last MLE operation */
#define MLEundo(h) WinSendMsg((h),MLM_UNDO,MPVOID,MPVOID)

/* return length of text in MLE */
#define MLEgetlen(h) (LONG)WinSendMsg((h),MLM_QUERYFORMATTEXTLENGTH,\
                     MPFROMLONG(0),MPFROMLONG((LONG)WinSendMsg((h),\
                     MLM_QUERYTEXTLENGTH,MPVOID,MPVOID)))

/* select all text in MLE */
#define MLEselectall(h) WinSendMsg((h),MLM_SETSEL,MPFROMLONG(0L),\
                        (MPARAM)WinSendMsg((h),MLM_QUERYFORMATTEXTLENGTH,\
                        MPFROMLONG(0),MPFROMLONG(0xfffffffe)))

/* select all text in MLE from cursor to end of file */
#define MLEselecttoeof(h) WinSendMsg((h),MLM_SETSEL,\
                          (IPT)WinSendMsg((h),MLM_QUERYSEL,\
                          MPFROM2SHORT(MLFQS_CURSORSEL,0),MPVOID),\
                          (MPARAM)WinSendMsg((h),MLM_QUERYFORMATTEXTLENGTH,\
                          MPFROMLONG(0),MPFROMLONG(0xfffffffe)))

/* set anchor point equal to cursor point (no text will be selected) */
#define MLEanctocur(h) WinSendMsg((h),MLM_SETSEL,\
                       MPFROMLONG((LONG)WinSendMsg((h),MLM_QUERYSEL,\
                       MPFROM2SHORT(MLFQS_CURSORSEL,0),MPVOID)),\
                       MPFROMLONG(-1L))

/* set cursor point equal to anchor point (no text will be selected) */
#define MLEcurtoanc(h) WinSendMsg((h),MLM_SETSEL,MPFROMLONG(-1L),\
                       MPFROMLONG((LONG)WinSendMsg((h),MLM_QUERYSEL,\
                       MPFROM2SHORT(MLFQS_ANCHORSEL,0),MPVOID)))

/* swap cursor point with anchor point */
#define MLEswappoints(h) WinSendMsg((h),MLM_SETSEL,\
                         MPFROMLONG((LONG)WinSendMsg((h),MLM_QUERYSEL,\
                         MPFROM2SHORT(MLFQS_ANCHORSEL,0),MPVOID)),\
                         MPFROMLONG((LONG)WinSendMsg((h),MLM_QUERYSEL,\
                         MPFROM2SHORT(MLFQS_CURSORSEL,0),MPVOID)))

/* declarations of functions in mle.c */
LONG MLEgetlinetext(HWND h, LONG l, CHAR * buf, INT maxlen);
LONG MLEdeleteline(HWND h, LONG l);
LONG MLEdeletecurline(HWND h);
LONG MLEdeletetoeol(HWND h);
VOID MLEclearall(HWND h);
LONG MLEtextatcursor(HWND h, CHAR * buffer, INT buflen);
LONG MLEtextatpos(HWND h, IPT i, CHAR * buffer, INT buflen);
LONG MLEsizeofsel(HWND h);
BOOL MLEdoblock(HWND h, INT action, CHAR * filename);
BOOL MLEquotepara(HWND h, CHAR * initials, BOOL fQuoteOld);
BOOL MLEinsertfile(HWND h, CHAR * filename);
BOOL MLEHexLoad(HWND h, CHAR * filename);
BOOL MLEloadfile(HWND h, CHAR * filename);
INT MLEbackgroundload(HWND hwndReport, ULONG msg, HWND h, CHAR * filename,
		      INT hex);
BOOL MLEexportfile(HWND h, CHAR * filename, INT tabspaces,
		   BOOL striptraillines, BOOL striptrailspaces);
typedef struct
{
  USHORT size;
  BOOL fInsensitive;
  BOOL sandr;
  BOOL rall;
  HWND hwndmle;
  MLE_SEARCHDATA se;
  CHAR search[258];
  CHAR replace[258];
}
SRCHPTR;
MRESULT EXPENTRY SandRDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL MLEfindfirst(HWND hwnd, SRCHPTR * vw);
INT MLEfindnext(HWND hwnd, SRCHPTR * vw);
VOID MLEinternet(HWND h, BOOL ftp);

/* declarations of functions in mlefont.c */
FATTRS *SetMLEFont(HWND hwndMLE, FATTRS * fattrs, ULONG flags);

/* struct used by MLE editor/viewer, saved in window pointer */

typedef struct
{
  USHORT size;
  USHORT hex;
  HAB hab;
  BOOL fWrap;
  SRCHPTR srch;
  HWND hwndMenu;
  HWND hwndPopupMenu;
  HACCEL accel;
  BOOL ch;
  BOOL fStripTrail;
  BOOL fStripTrailLines;
  INT ExpandTabs;
  INT TabStops;
  CHAR exportfilename[1027];
  CHAR importfilename[1027];
  FATTRS fattrs;
  ULONG cp;
  INT busy;
  ULONG lastpos;
  BOOL killme;
  BOOL dontclose;
  HWND hwndRestore, hwndFrame, hwndParent;
}
XMLEWNDPTR;

/* declarations of functions in mlemain.c */
HWND StartMLEEditor(HWND hwnd, INT flags, CHAR * filename, HWND hwndRestore);
MRESULT EXPENTRY MLEEditorProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* MLEdoblock() actions */

#define UPPERCASE     1
#define LOWERCASE     2
#define TOGGLECASE    3
#define ROT13         4
#define FORMAT        5
#define XOR           6
#define WRITE         7
#define APPENDCLIP    8

#define ParentOf(hwnd)  WinQueryWindow((hwnd),QW_PARENT)
#define GrandparentOf(hwnd) WinQueryWindow(WinQueryWindow(hwnd,QW_PARENT),QW_PARENT)
