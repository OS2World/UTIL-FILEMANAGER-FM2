
/***********************************************************************

  $Id$

  System Interfaces

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2003, 2004 Steven H.Levine

  Revisions	21 Nov 03 SHL - Comments
  		31 Jul 04 SHL - Indent -i2
		01 Aug 04 SHL - Rework lstrip/rstrip usage

***********************************************************************/

#define INCL_WIN
#define INCL_WINERRORS
#define INCL_DOS
#define INCL_DOSERRORS

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "fm3dll.h"
#include "fm3dlg.h"
#include "fm3str.h"

#pragma alloc_text(SYSTEMF,ShowSession,ExecOnList,runemf2)

#define MAXSTRG (4096)			/* used to build command line strings */

/* quick and dirty program launcher for OS/2 2.x */

BOOL ShowSession(HWND hwnd, PID pid)
{

  HSWITCH hswitch;
  SWCNTRL swctl;
  ULONG rc;

  hswitch = WinQuerySwitchHandle((pid) ? (HWND) 0 : hwnd, pid);
  if (hswitch)
  {
    rc = WinQuerySwitchEntry(hswitch, &swctl);
    if (!rc)
    {
      if (swctl.idProcess == pid && swctl.uchVisibility == SWL_VISIBLE)
	rc = WinSwitchToProgram(hswitch);
      if (!rc)
	return TRUE;
      // else saymsg(MB_ENTER,HWND_DESKTOP,DEBUG_STRING,"Failed: %lu/%lx",rc,rc);

    }
  }
  return FALSE;
}

int ExecOnList(HWND hwnd, char *command, int flags, char *tpath,
	       char **list, char *prompt)
{

  /* executes the command once for all files in list */

  char path[CCHMAXPATH], commandline[2048], modpath[CCHMAXPATH], listfile[CCHMAXPATH],
       *p, *pp, drive, *file, *ext, *dot;
  register int x;
  BOOL spaces;

  if (!command || !*command)
    return -1;
  *listfile = 0;
  bstrip(command);

  *path = 0;
  if (tpath && *tpath)
    strcpy(path, tpath);
  else if (*command != '<' || !strchr(command, '>'))
  {
    strcpy(path, command + (*command == '"'));
    if (*command == '\"')
      p = strchr(path, '\"');
    else
      p = strchr(path, ' ');
    if (p)
      *p = 0;
    p = strrchr(path, '\\');
    if (!p)
      p = strrchr(path, ':');
    if (p)
    {
      if (*p == ':')
      {
	p++;
	*p = '\\';
	p++;
      }
      *p = 0;
    }
    else
      *path = 0;
  }
  if (!*path)
  {
    if (list && list[0])
      strcpy(path, list[0]);
    p = strrchr(path, '\\');
    if (!p)
      p = strrchr(path, ':');
    if (p)
    {
      if (*p == ':')
      {
	p++;
	*p = '\\';
	p++;
      }
      *p = 0;
    }
    else
      *path = 0;
  }
  *modpath = 0;
  if (list && list[0])
    strcpy(modpath, list[0]);
  p = strrchr(modpath, '\\');
  if (!p)
    p = strrchr(modpath, ':');
  if (p)
  {
    if (*p == ':')
    {
      p++;
      *p = '\\';
      p++;
    }
    *p = 0;
  }
  else
    *modpath = 0;
  if (!*modpath)
    strcpy(modpath, path);
  if (*path)
    MakeFullName(path);
  if (*modpath)
    MakeFullName(modpath);
  if (IsFullName(path))
    drive = toupper(*path);
  else
    drive = 0;

  p = command;				// substitue for special % sequences

  pp = commandline;
  *commandline = 0;
  while (*p)
  {
    if (*p == '%')
    {
      switch (*(p + 1))
      {
      case '!':			/* write list to file, add filename */
	if (list)
	{
	  if (!*listfile)
	  {

	    FILE *fp;

	    save_dir2(listfile);
	    if (listfile[strlen(listfile) - 1] != '\\')
	      strcat(listfile, "\\");
	    sprintf(&listfile[strlen(listfile)], "%s%03x",
		    LISTTEMPROOT, (clock() & 4095L));
	    fp = fopen(listfile, "w");
	    if (fp)
	    {
	      for (x = 0; list[x]; x++)
	      {
		fputs(list[x], fp);
		if (list[x + 1])
		  fputc('\n', fp);
	      }
	      fclose(fp);
	    }
	  }
	  strcpy(pp, listfile);
	  pp += strlen(listfile);
	}
	p += 2;
	break;

      case 'c':			/* add name of command processor */
	{
	  char *env = GetCmdSpec(FALSE);

	  if (needs_quoting(env) && !strchr(env, '\"'))
	  {
	    *pp = '\"';
	    pp++;
	    spaces = TRUE;
	  }
	  else
	    spaces = FALSE;
	  strcpy(pp, env);
	  p += 2;
	  pp += strlen(env);
	  if (spaces)
	  {
	    *pp = '\"';
	    pp++;
	  }
	}
	break;

      case 't':			/* add Target directory */
	if (needs_quoting(targetdir) && !strchr(targetdir, '\"'))
	{
	  *pp = '\"';
	  pp++;
	  spaces = TRUE;
	}
	else
	  spaces = FALSE;
	strcpy(pp, targetdir);
	p += 2;
	pp += strlen(targetdir);
	if (spaces)
	{
	  *pp = '\"';
	  pp++;
	}
	break;

      case '$':			/* add drive letter */
	if (drive)
	  *pp = drive;
	else
	{

	  ULONG ulDriveNum = 3L, ulDriveMap;

	  DosQCurDisk(&ulDriveNum, &ulDriveMap);
	  *pp = (char) (ulDriveNum + '@');
	}
	pp++;
	p += 2;
	break;

      case 'U':			/* add path of first list component */
      case 'u':
	if (*modpath)
	{
	  if (needs_quoting(modpath) && !strchr(modpath, '\"'))
	  {
	    spaces = TRUE;
	    *pp = '\"';
	    pp++;
	  }
	  else
	    spaces = FALSE;
	  if (*(p + 1) == 'u')
	  {
	    strcpy(pp, modpath);
	    pp += strlen(modpath);
	  }
	  else
	  {
	    strcpy(pp, modpath + 2);
	    pp += strlen(modpath + 2);
	  }
	  if (spaces)
	  {
	    if (modpath[strlen(modpath) - 1] == '\\')
	    {
	      *pp = '\\';
	      pp++;
	    }
	    *pp = '\"';
	    pp++;
	  }
	}
	else
	{

	  char temp[CCHMAXPATH];

	  save_dir2(temp);
	  if (needs_quoting(temp) && !strchr(temp, '\"'))
	  {
	    spaces = TRUE;
	    *pp = '\"';
	    pp++;
	  }
	  else
	    spaces = FALSE;
	  strcpy(pp, temp);
	  pp += strlen(temp);
	  if (spaces)
	  {
	    if (temp[strlen(temp) - 1] == '\\')
	    {
	      *pp = '\\';
	      pp++;
	    }
	    *pp = '\"';
	    pp++;
	  }
	}
	p += 2;
	break;

      case 'P':			/* add path of execution */
      case 'p':
	if (*path)
	{
	  if (needs_quoting(path) && !strchr(path, '\"'))
	  {
	    spaces = TRUE;
	    *pp = '\"';
	    pp++;
	  }
	  else
	    spaces = FALSE;
	  if (*(p + 1) == 'p')
	  {
	    strcpy(pp, path);
	    pp += strlen(path);
	  }
	  else
	  {
	    strcpy(pp, path + 2);
	    pp += strlen(path + 2);
	  }
	  if (spaces)
	  {
	    if (path[strlen(path) - 1] == '\\')
	    {
	      *pp = '\\';
	      pp++;
	    }
	    *pp = '\"';
	    pp++;
	  }
	}
	else
	{

	  char temp[CCHMAXPATH];

	  save_dir2(temp);
	  if (needs_quoting(temp) && !strchr(temp, '\"'))
	  {
	    spaces = TRUE;
	    *pp = '\"';
	    pp++;
	  }
	  else
	    spaces = FALSE;
	  strcpy(pp, temp);
	  pp += strlen(temp);
	  if (spaces)
	  {
	    if (temp[strlen(temp) - 1] == '\\')
	    {
	      *pp = '\\';
	      pp++;
	    }
	    *pp = '\"';
	    pp++;
	  }
	}
	p += 2;
	break;

      case 'D':
	if (hwndMain)
	{

	  PCNRITEM pci;

	  pci = (PCNRITEM) WinSendMsg(WinWindowFromID(WinWindowFromID(
					   hwndTree, FID_CLIENT), TREE_CNR),
				      CM_QUERYRECORDEMPHASIS,
				      MPFROMLONG(CMA_FIRST),
				      MPFROMSHORT(CRA_CURSORED));
	  if (pci && (int) pci != -1 && *pci -> szFileName)
	  {
	    if (needs_quoting(pci -> szFileName) &&
		!strchr(pci -> szFileName, '\"'))
	    {
	      *pp = '\"';
	      pp++;
	      spaces = TRUE;
	    }
	    else
	      spaces = FALSE;
	    strcpy(pp, pci -> szFileName);
	    pp += strlen(pci -> szFileName);
	    if (spaces)
	    {
	      *pp = '\"';
	      pp++;
	    }
	  }
	}
	p += 2;
	break;

      case 'd':
	if (hwndMain)
	{

	  HENUM henum;
	  char retstr[CCHMAXPATH];
	  HWND hwndC, hwndDir;
	  USHORT id;
	  BOOL first = TRUE;

	  henum = WinBeginEnumWindows(hwndMain);
	  while ((hwndC = WinGetNextWindow(henum)) != NULLHANDLE)
	  {
	    if (hwndC != hwndTree)
	    {
	      id = WinQueryWindowUShort(hwndC, QWS_ID);
	      if (id)
	      {
		hwndDir = WinWindowFromID(hwndC, FID_CLIENT);
		if (hwndDir)
		{
		  hwndDir = WinWindowFromID(hwndDir, DIR_CNR);
		  if (hwndDir)
		  {
		    *retstr = 0;
		    WinSendMsg(hwndC, UM_CONTAINERDIR, MPFROMP(retstr), MPVOID);
		    if (*retstr)
		    {
		      if (!first)
		      {
			*pp = ' ';
			pp++;
		      }
		      first = FALSE;
		      if (needs_quoting(retstr) && !strchr(retstr, '\"'))
		      {
			*pp = '\"';
			pp++;
			spaces = TRUE;
		      }
		      else
			spaces = FALSE;
		      strcpy(pp, retstr);
		      pp += strlen(retstr);
		      if (spaces)
		      {
			*pp = '\"';
			pp++;
		      }
		    }
		  }
		}
	      }
	    }
	  }
	  WinEndEnumWindows(henum);
	}
	p += 2;
	break;

      case '%':
	*pp = '%';
	pp++;
	p += 2;
	break;

      case 'R':
      case 'F':
      case 'A':
      case 'r':
      case 'f':
      case 'a':
      case 'e':
	if (list)
	{
	  for (x = 0; list[x]; x++)
	  {
	    file = strrchr(list[x], '\\');
	    if (!file)
	      file = strrchr(list[x], ':');
	    if (file)
	      file++;
	    else
	      file = list[x];
	    ext = strrchr(file, '.');
	    dot = ext;
	    if (ext)
	      ext++;
	    switch (*(p + 1))
	    {
	    case 'R':
	    case 'r':
	      if (pp + strlen(list[x]) > commandline + 1250)
		goto BreakOut;
	      if (*(p + 1) == 'r')
	      {
		strcpy(pp, list[x]);
		pp += strlen(list[x]);
	      }
	      else
	      {
		strcpy(pp, list[x] + 2);
		pp += strlen(list[x] + 2);
	      }
	      break;

	    case 'F':
	    case 'f':
	      if (*(p + 1) == 'F' && dot)
		*dot = 0;
	      if (pp + strlen(file) > commandline + 1250)
		goto BreakOut;
	      if (needs_quoting(file))
	      {
		spaces = TRUE;
		*pp = '\"';
		pp++;
	      }
	      else
		spaces = FALSE;
	      strcpy(pp, file);
	      pp += strlen(file);
	      if (*(p + 1) == 'F' && dot)
		*dot = '.';
	      if (spaces)
	      {
		if (*(pp - 1) != '\"')
		{
		  *pp = '\"';
		  pp++;
		}
	      }
	      break;

	    case 'A':
	    case 'a':
	      if (pp + strlen(list[x]) > commandline + 1250)
		goto BreakOut;
	      if (needs_quoting(list[x]) && !strchr(list[x], '\"'))
	      {
		spaces = TRUE;
		*pp = '\"';
		pp++;
	      }
	      else
		spaces = FALSE;
	      if (*(p + 1) == 'a')
	      {
		strcpy(pp, list[x]);
		pp += strlen(list[x]);
	      }
	      else
	      {
		strcpy(pp, list[x] + 2);
		pp += strlen(list[x] + 2);
	      }
	      if (spaces)
	      {
		if (list[x][strlen(list[x]) - 1] == '\\')
		{
		  *pp = '\\';
		  pp++;
		}
		*pp = '\"';
		pp++;
	      }
	      break;

	    case 'e':
	      if (ext)
	      {
		if (pp + strlen(ext) > commandline + 1250)
		  goto BreakOut;
		if (needs_quoting(ext))
		{
		  spaces = TRUE;
		  *pp = '\"';
		  pp++;
		}
		else
		  spaces = FALSE;
		strcpy(pp, ext);
		pp += strlen(ext);
		if (spaces)
		{
		  if (*(pp - 1) != '\"')
		  {
		    *pp = '\"';
		    pp++;
		  }
		}
	      }
	      break;
	    }
	    if (list[x + 1])
	    {
	      *pp = ' ';
	      pp++;
	    }
	  }
	}
	p += 2;
	break;

      default:
	*pp = *p;
	p++;
	pp++;
	break;
      }
    }
    else
    {
      *pp = *p;
      pp++;
      p++;
    }
    *pp = 0;
  }

BreakOut:

  {

    EXECARGS ex;
    ULONG size;
    int ret;

    memset(&ex, 0, sizeof(EXECARGS));
    size = sizeof(ex.environment) - 1;
    PrfQueryProfileData(fmprof, FM3Str, command, ex.environment, &size);
    if (flags & PROMPT)
    {					/* allow editing command line */
      ex.flags = (flags & (~PROMPT));
      ex.commandline = commandline;
      strcpy(ex.path, path);
      if (prompt)
	strcpy(ex.title, prompt);
      ret = WinDlgBox(HWND_DESKTOP, hwnd, CmdLineDlgProc, FM3ModHandle,
		      EXEC_FRAME, &ex);
      if (ret != 1)
	return (ret == 0) ? -1 : -2;
    }
    else
      ex.flags = flags;
    ex.flags &= (~PROMPT);
    return runemf2(ex.flags, hwnd, path,
		   (*ex.environment) ? ex.environment : NULL,
		   "%s", commandline);
  }
}

int runemf2(int type, HWND hwnd, char *directory, char *environment,
	    char *formatstring,...)
{

  /* example:

   * status = runemf2(SEPARATE | WINDOWED,
   *                  hwnd,
   *                  NullStr,
   *                  NULL,
   *                  "%s /C %s",
   *                  getenv("COMSPEC"),
   *                  batchfilename);
   *
   * use (HWND)0 for hwnd if window handle not handy.
   */

  /*
   * type bitmapped flag -- see FM3DLL.H
   */

  va_list parguments;
  int ret = -1;
  RESULTCODES rt;
  STARTDATA start;
  REQUESTDATA rq;
  ULONG sessID, apptype, ulLength, ctr = 0;
  PID sessPID;
  BOOL wasquote;
  char *s = NULL, *s2 = NULL, object[32] = "", *p, savedir[CCHMAXPATH];
  HQUEUE hque = (HQUEUE) 0;
  char queue_name[] = "\\QUEUES\\FM3WAIT", tempdir[CCHMAXPATH];
  PUSHORT pusInfo = (PUSHORT) NULL;
  BYTE bPriority;
  APIRET rc;

  if (directory &&
      *directory)
  {
    if (!DosQueryPathInfo(directory,
			  FIL_QUERYFULLNAME,
			  tempdir,
			  sizeof(tempdir)))
      directory = tempdir;
  }

  if (!hwnd)
    hwnd = HWND_DESKTOP;

  rc = DosAllocMem((PVOID) & s,
		   MAXSTRG,
		   PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
  if (rc)
    return -1;

  *savedir = 0;

  *s = 0;
  va_start(parguments,
	   formatstring);
  vsprintf(s,
	   formatstring,
	   parguments);
  va_end(parguments);

  if (environment)
  {
    p = &environment[strlen(environment)] + 1;
    *p = 0;
    p = environment;
    while ((p = strchr(p, '\n')) != NULL)
    {
      *p = 0;
      p++;
    }
  }

  if (!*s)
  {
    p = GetCmdSpec(FALSE);
    strcpy(s, p);
    if (!*s)
      return -1;
  }

  if (*s)
  {
    if (*s == '<' &&
	strchr(s, '>'))
    {					/* is a workplace object */

      HOBJECT hWPSObject;
      char temp;

      p = strchr(s, '>');
      p++;
      temp = *p;
      if (temp)
	rc = DosAllocMem((PVOID) & s2,
			 MAXSTRG * 2,
			 PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
      else
	s2 = NULL;
      *p = 0;
      /* Find the handle of the WPS object */
      hWPSObject = WinQueryObject(s);
      *p = temp;
      if (hWPSObject != NULLHANDLE)
      {
	if (s2 && *p)
	{
	  sprintf(s2,
		  "OPEN=DEFAULT;PARAMETERS=\"%s\"",
		  p);
	  WinSetObjectData(hWPSObject,
			   s2);
	}
	else
	  WinSetObjectData(hWPSObject,
			   "OPEN=DEFAULT");
	ret = 0;
      }
      goto ObjectInterrupt;
    }

    if ((type & 15) == SYNCHRONOUS ||
	(type & 15) == ASYNCHRONOUS ||
	(type & 15) == DETACHED)
    {
      strip_lead_char(" \t", s);
      p = s;
      wasquote = FALSE;
      while (*p &&
	     (wasquote ||
	      (*p != ' ' &&
	       *p != '\t')))
      {
	if (*p == '\"')
	{
	  if (!wasquote)
	  {
	    wasquote = TRUE;
	    memmove(p,
		    p + 1,
		    strlen(p));
	    while (*p == ' ' ||
		   *p == '\t')
	      p++;
	  }
	  else
	  {
	    memmove(p,
		    p + 1,
		    strlen(p));
	    break;
	  }
	}
	else
	  p++;
      }
      if (*p)
      {
	*p = 0;
	p++;
      }
      else
	p = s;
      p[strlen(p) + 1] = 0;		/* double-terminate args */
      if (*s)
      {
	if (!strchr(s, '\\') &&
	    !strchr(s, ':') &&
	    directory &&
	    *directory)
	{
	  save_dir2(savedir);
	  switch_to(directory);
	}
	ret = (int) DosQAppType(s,
				&apptype);
	if (!strchr(s, '\\') &&
	    !strchr(s, ':') &&
	    directory &&
	    *directory)
	  switch_to(savedir);
	if (ret)
	{
	  DosBeep(50, 100);
	  if (s)
	    DosFreeMem(s);
	  if (s2)
	    DosFreeMem(s2);
	  return -1;
	}
	if (apptype)
	{
	  if ((apptype & FAPPTYP_DLL) || (apptype & FAPPTYP_VIRTDRV) ||
	      (apptype & FAPPTYP_PHYSDRV) || (apptype & FAPPTYP_PROTDLL))
	  {
	    DosBeep(250, 100);
	    if (s)
	      DosFreeMem(s);
	    if (s2)
	      DosFreeMem(s2);
	    return -1;
	  }
	  if ((apptype & FAPPTYP_DOS) || (apptype & FAPPTYP_WINDOWSREAL) ||
	      (apptype & FAPPTYP_WINDOWSPROT) || (apptype & 0x1000))
	  {
	    DosBeep(500, 100);
	    if (s)
	      DosFreeMem(s);
	    if (s2)
	      DosFreeMem(s2);
	    return -1;
	  }
	}
	memset(&rt, 0, sizeof(RESULTCODES));
	if (directory && *directory)
	{
	  save_dir2(savedir);
	  switch_to(directory);
	}
	ret = (int) DosExecPgm(object, 24L,
		      (ULONG) (((type & 15) == ASYNCHRONOUS) * EXEC_ASYNC) +
			       (((type & 15) == DETACHED) * EXEC_BACKGROUND),
			       s, environment, &rt, s);
	if (directory && *directory)
	  switch_to(savedir);
	if (ret)
	  Dos_Error(MB_ENTER,
		    ret,
		    hwnd,
		    __FILE__,
		    __LINE__,
		    GetPString(IDS_DOSEXECPGMFAILEDTEXT));
      }
    }
    else
    {
      if (!(type & FULLSCREEN))
	type |= WINDOWED;
      rc = DosAllocMem((PVOID) & s2, MAXSTRG * 2,
		       PAG_COMMIT | OBJ_TILE | PAG_READ | PAG_WRITE);
      if (rc)
      {
	DosFreeMem(s);
	return -1;
      }
      *s2 = 0;
      memset(&start, 0, sizeof(STARTDATA));
      strip_lead_char(" \t", s);
      p = s;
      wasquote = FALSE;
      while (*p && (wasquote || (*p != ' ' && *p != '\t')))
      {
	if (*p == '\"')
	{
	  if (!wasquote)
	  {
	    wasquote = TRUE;
	    memmove(p, p + 1, strlen(p));
	    while (*p == ' ' || *p == '\t')
	      p++;
	  }
	  else
	  {
	    memmove(p, p + 1, strlen(p));
	    break;
	  }
	}
	else
	  p++;
      }
      if (*p)
      {
	*p = 0;
	p++;
      }
      else
	p = NullStr;
      if (*p)
	strcpy(s2, p);

      p = strrchr(s, '.');
      if (p)
      {

	char temp[CCHMAXPATH + 1];

	if (!stricmp(p, ".BAT"))
	{
	  strcpy(temp, s);
	  strcpy(s, s2);
	  strcpy(s2, "/C ");
	  strcat(s2, temp);
	  strcat(s2, " ");
	  strcat(s2, s);
	  strcpy(s, GetCmdSpec(TRUE));
	}
	else if (!stricmp(p, ".CMD"))
	{
	  strcpy(temp, s);
	  strcpy(s, s2);
	  strcpy(s2, "/C ");
	  strcat(s2, temp);
	  strcat(s2, " ");
	  strcat(s2, s);
	  strcpy(s, GetCmdSpec(FALSE));
	}
      }

      /* goddamned OS/2 limit */

      if (strlen(s) + strlen(s2) > 1024)
	s2[1024 - strlen(s)] = 0;

      if (!strchr(s, '\\') &&
	  !strchr(s, ':') &&
	  directory &&
	  *directory)
      {
	save_dir2(savedir);
	switch_to(directory);
      }
      ret = (int) DosQAppType(s,
			      &apptype);
      if (!strchr(s, '\\') &&
	  !strchr(s, ':') &&
	  directory &&
	  *directory)
	switch_to(savedir);
      if (ret)
      {
	if (s)
	  DosFreeMem(s);
	if (s2)
	  DosFreeMem(s2);
	return -1;
      }

      if (apptype)
      {
	if ((apptype & FAPPTYP_DLL) || (apptype & FAPPTYP_VIRTDRV) ||
	    (apptype & FAPPTYP_PHYSDRV) || (apptype & FAPPTYP_PROTDLL))
	{
	  if (s)
	    DosFreeMem(s);
	  if (s2)
	    DosFreeMem(s2);
	  return -1;
	}
	apptype &= (~FAPPTYP_BOUND);
	if ((apptype & FAPPTYP_DOS) || (apptype & FAPPTYP_WINDOWSREAL) ||
	    (apptype & FAPPTYP_WINDOWSPROT) || (apptype & 0x1000))
	{
	  if ((apptype & FAPPTYP_WINDOWSREAL) ||
	      (apptype & FAPPTYP_WINDOWSPROT) || (apptype & 0x1000))
	  {
	    if (!(type & FULLSCREEN) && ((apptype & FAPPTYP_WINDOWSREAL) ||
		     (apptype & FAPPTYP_WINDOWSPROT) || (apptype & 0x1000)))
	    {
	      ret = RunSeamless(s, s2, hwnd);
	      if (s)
		DosFreeMem(s);
	      if (s2)
		DosFreeMem(s2);
	      return (ret) ? 0 : -1;
	    }
	    else
	    {
	      strcat(s, " ");
	      strcat(s, s2);
	      *s2 = 0;
	      if ((apptype & FAPPTYP_WINDOWSPROT) ||
		  (apptype & FAPPTYP_WINDOWSREAL) ||
		  (apptype & 0x1000))
		strcat(s2, "/3 ");
	      strcat(s2, s);
	      strcpy(s, "WINOS2.COM");
	    }
	  }
	  else
	  {
	    if (!(type & FULLSCREEN))
	    {
	      type |= WINDOWED;
	      apptype = SSF_TYPE_WINDOWEDVDM;
	    }
	    else
	    {
	      type &= (~WINDOWED);
	      apptype = SSF_TYPE_VDM;
	    }
	  }
	}
	else if (apptype & FAPPTYP_32BIT)
	{
	  apptype &= (~FAPPTYP_32BIT);
	  if (apptype == FAPPTYP_WINDOWAPI)
	    apptype = SSF_TYPE_PM;
	  else if (apptype == FAPPTYP_WINDOWCOMPAT)
	    apptype = SSF_TYPE_WINDOWABLEVIO;
	  else if (apptype == FAPPTYP_NOTWINDOWCOMPAT)
	  {
	    apptype = SSF_TYPE_FULLSCREEN;
	    type &= (~WINDOWED);
	    type |= FULLSCREEN;
	  }
	  else				/* ? */
	    apptype = SSF_TYPE_WINDOWABLEVIO;
	}
	else if (apptype == FAPPTYP_WINDOWAPI)
	  apptype = SSF_TYPE_PM;
	else if (apptype == FAPPTYP_WINDOWCOMPAT)
	  apptype = SSF_TYPE_WINDOWABLEVIO;
	else if (apptype == FAPPTYP_NOTWINDOWCOMPAT)
	{
	  type &= (~WINDOWED);
	  apptype = SSF_TYPE_FULLSCREEN;
	}
	else
	  apptype = SSF_TYPE_DEFAULT;
	if (((type & FULLSCREEN) || !(type & WINDOWED)) &&
	    apptype == SSF_TYPE_WINDOWABLEVIO)
	{
	  apptype = SSF_TYPE_FULLSCREEN;
	}
	else if (((type & FULLSCREEN) || !(type & WINDOWED) &&
		  apptype == SSF_TYPE_WINDOWEDVDM))
	  apptype = SSF_TYPE_VDM;
      }
      if (apptype == SSF_TYPE_WINDOWEDVDM && (type & SEPARATEKEEP))
      {
	type &= (~SEPARATEKEEP);
	type |= SEPARATE;
      }

      if (type & WAIT)
      {
	if (DosCreateQueue(&hque, QUE_FIFO | QUE_CONVERT_ADDRESS, queue_name))
	  hque = (HQUEUE) 0;
      }
      else
	*queue_name = 0;
      start.Length = sizeof(start);
      start.Related = ((type & WAIT) != 0) ?
	SSF_RELATED_CHILD :
	((type & CHILD) != 0) ?
	SSF_RELATED_CHILD :
	SSF_RELATED_INDEPENDENT;
      start.FgBg = ((type & BACKGROUND) != 0) * SSF_FGBG_BACK;
      start.TraceOpt = SSF_TRACEOPT_NONE;
      start.PgmTitle = NULL;
      start.PgmName = s;
      start.PgmInputs = (*s2) ? s2 : NULL;
      start.TermQ = (*queue_name) ? queue_name : NULL;
      start.Environment = environment;
      start.InheritOpt = SSF_INHERTOPT_PARENT;
      start.SessionType = (USHORT) apptype;
      start.ObjectBuffer = object;
      start.ObjectBuffLen = 31;
      start.IconFile = NULL;
      start.PgmHandle = 0L;
      start.Reserved = 0;
      start.PgmControl = (USHORT) ((SSF_CONTROL_NOAUTOCLOSE * ((type & 15) == SEPARATEKEEP)) |
			(SSF_CONTROL_MAXIMIZE * ((type & MAXIMIZED) != 0)) |
			(SSF_CONTROL_MINIMIZE * ((type & MINIMIZED) != 0)) |
		       (SSF_CONTROL_INVISIBLE * ((type & INVISIBLE) != 0)));
      if (directory && *directory)
      {
	save_dir2(savedir);
	switch_to(directory);
      }
      ret = (int) DosStartSession(&start, &sessID, &sessPID);
      if (directory && *directory)
	switch_to(savedir);
      if (ret && ret != ERROR_SMG_START_IN_BACKGROUND)
	Dos_Error(MB_CANCEL | MB_ICONEXCLAMATION,
		  ret,
		  hwnd,
		  __FILE__,
		  __LINE__,
		  GetPString(IDS_DOSSTARTSESSIONFAILEDTEXT),
		  s,
		  s2);
      else if (type & WAIT)
      {
	if (!(type & (BACKGROUND | MINIMIZED | INVISIBLE)))
	  ShowSession(hwnd, sessPID);

	if (!hque)
	{

	  STATUSDATA sd;

	  memset(&sd, 0, sizeof(sd));
	  sd.Length = (USHORT) sizeof(sd);
	  sd.SelectInd = SET_SESSION_UNCHANGED;
	  sd.BondInd = SET_SESSION_UNCHANGED;
	  for (ctr = 0;; ctr++)
	  {
	    DosSleep(200L);
	    if (DosSetSession(sessID, &sd))	/* cheap trick */
	      break;
	    if (ctr > 10)
	      ShowSession(hwnd, sessPID);
	  }
	}
	else
	{
	  for (ctr = 0;; ctr++)
	  {
	    ulLength = sizeof(rq);
	    rc = DosReadQueue(hque, &rq, &ulLength, (PPVOID) & pusInfo, 0,
			      DCWW_NOWAIT, &bPriority, 0);
	    if (rc == ERROR_QUE_EMPTY)
	    {
	      if (ctr > 20)
	      {
		ShowSession(hwnd, sessPID);
		ulLength = sizeof(rq);
		DosReadQueue(hque, &rq, &ulLength, (PPVOID) & pusInfo, 0,
			     DCWW_WAIT, &bPriority, 0);
		break;
	      }
	      DosSleep(100L);
	    }
	    else
	    {
	      ulLength = sizeof(rq);
	      if (rc)
		DosReadQueue(hque, &rq, &ulLength, (PPVOID) & pusInfo, 0,
			     DCWW_WAIT, &bPriority, 0);
	      break;
	    }
	  }
	  if (pusInfo)
	  {
	    ret = (!(!pusInfo[1]));
	    DosFreeMem(pusInfo);
	  }
	  DosCloseQueue(hque);
	}
      }
      else if (!(type & (BACKGROUND | MINIMIZED | INVISIBLE)))
	ShowSession(hwnd, sessPID);
    }
  }

ObjectInterrupt:

  if (s)
    DosFreeMem(s);
  if (s2)
    DosFreeMem(s2);
  return ret;
}

HAPP Exec(HWND hwndNotify, BOOL child, char *startdir, char *env,
	  PROGTYPE * progt, ULONG fl, char *formatstring,...)
{

  PROGDETAILS pgd;
  register char *p;
  char *parameters = NULL, *executable = NULL;
  HAPP happ = (HAPP) 0;
  ULONG ulOptions = SAF_INSTALLEDCMDLINE;
  BOOL wasquote;
  va_list parguments;

  if (child)
    ulOptions |= SAF_STARTCHILDAPP;

  executable = malloc(MAXSTRG);
  if (executable)
  {
    memset(executable, 0, MAXSTRG);
    va_start(parguments, formatstring);
    vsprintf(executable, formatstring, parguments);
    va_end(parguments);
    strip_lead_char(" \t", executable);
    if (*executable)
    {
      parameters = malloc(MAXSTRG);
      if (parameters)
      {
	p = executable;
	wasquote = FALSE;
	while (*p && (wasquote || (*p != ' ' && *p != '\t')))
	{
	  if (*p == '\"')
	  {
	    if (!wasquote)
	    {
	      wasquote = TRUE;
	      memmove(p, p + 1, strlen(p));
	      while (*p == ' ' || *p == '\t')
		p++;
	    }
	    else
	    {
	      memmove(p, p + 1, strlen(p));
	      break;
	    }
	  }
	  else
	    p++;
	}
	if (*p)
	{
	  *p = 0;
	  p++;
	}
	else
	  p = NullStr;
	if (*p)
	  strcpy(parameters, p);

	if (p && (!stricmp(p, ".BAT") || !stricmp(p, ".CMD")))
	{

	  char *temp;

	  temp = malloc(CCHMAXPATH * 2);
	  if (temp)
	  {
	    if (!stricmp(p, ".BAT"))
	    {
	      strcpy(temp, executable);
	      strcpy(executable, parameters);
	      strcpy(parameters, "/C ");
	      strcat(parameters, temp);
	      strcat(parameters, " ");
	      strcat(parameters, executable);
	      strcpy(executable, GetCmdSpec(TRUE));
	    }
	    else if (!stricmp(p, ".CMD"))
	    {
	      strcpy(temp, executable);
	      strcpy(executable, parameters);
	      strcpy(parameters, "/C ");
	      strcat(parameters, temp);
	      strcat(parameters, " ");
	      strcat(parameters, executable);
	      strcpy(executable, GetCmdSpec(FALSE));
	    }
	    free(temp);
	  }
	}

	memset(&pgd, 0, sizeof(pgd));
	pgd.Length = sizeof(pgd);
	pgd.progt = *progt;
	pgd.swpInitial.fl = fl;
	pgd.pszEnvironment = env;
	pgd.pszStartupDir = startdir;
	pgd.pszParameters = (*parameters) ? parameters : NULL;
	pgd.pszExecutable = executable;
	pgd.swpInitial.hwndInsertBehind = HWND_TOP;
	happ = WinStartApp(hwndNotify, &pgd, NULL, NULL, ulOptions);
	free(parameters);
      }
    }
    free(executable);
  }
  return happ;
}
