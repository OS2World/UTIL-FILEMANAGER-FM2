
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2010 Steven H. Levine

  Change log
  21 Dec 09 GKY Added CheckExecutibleFlags to streamline code in command.c assoc.c & cmdline.c
  17 JAN 10 GKY Changes to environment handling in ExecuteOnList to facilitate move of commands to INI and allow
                the same commandline to have different environments (multiple different command titles).
  17 JAN 10 GKY Add ENVIRONMENT_SIZE vaiable to replace multiple (often different hard coded sizes) set to 2048
                (the largest value I found).

***********************************************************************/

#if !defined(SYSTEMF_H)
#define SYSTEMF_H

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

#define ENVIRONMENT_SIZE 2048

typedef struct
{
  ULONG flags;
  CHAR *commandline;
  CHAR path[CCHMAXPATH];
  CHAR environment[ENVIRONMENT_SIZE];
  CHAR tempprompt[128];
  CHAR title[80];
  BOOL dropped;
}
EXECARGS;

ULONG CheckExecutibleFlags(HWND hwnd, INT caller);
INT ExecOnList(HWND hwnd, PSZ command, INT flags, PSZ tpath, PSZ environment,
	       PSZ *list, PCSZ prompt, PCSZ pszCallingFile, UINT uiLineNumber);
BOOL ShowSession(HWND hwnd, PID pid);
INT runemf2(INT type, HWND hwnd, PCSZ pszCallingFile, UINT uiLineNumber,
	    PSZ directory, PSZ environment,
	    PSZ formatstring, ...);


#endif // SYSTEMF_H
