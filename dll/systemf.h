
/***********************************************************************

$Id: $

<<<description here>>>

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(SYSTEMF_H)
#define SYSTEMF_H

INT ExecOnList(HWND hwnd, CHAR * command, INT flags, CHAR * tpath,
	       CHAR ** list, CHAR * prompt, PCSZ pszCallingFile, UINT uiLineNumber);
BOOL ShowSession(HWND hwnd, PID pid);
INT runemf2(INT type, HWND hwnd, PCSZ pszCallingFile, UINT uiLineNumber,
	    CHAR * directory, CHAR * environment,
	    CHAR * formatstring, ...);

#define RUNTYPE_MASK  0xf
#define SYNCHRONOUS   1
#define ASYNCHRONOUS  2
#define DETACHED      3
#define SEPARATE      4
#define SEPARATEKEEP  5
#define WINDOWED      16
#define MAXIMIZED     32
#define MINIMIZED     64
#define FULLSCREEN    128
#define INVISIBLE     256
#define BACKGROUND    512
#define WAIT          1024
#define PROMPT        2048
#define KEEP          4096
#define ONCE          8192
#define DIEAFTER      16384
#define SEAMLESS      32768
#define CHILD         65536



#endif // SYSTEMF_H
