
/***********************************************************************

  $Id: $

  Path handling utility functions

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Jan 08 SHL Move from arccnrs.c and comp.c to here
  06 Jan 08 GKY Add NormalizeCmdLine to check program strings on entry

***********************************************************************/

#include <string.h>

#define INCL_WIN
#define INCL_DOS
#define INCL_LONGLONG

#include "pathutil.h"
#include "fm3dll.h"			// needs_quoting
#include "fm3str.h"
#include "errutil.h"			// Dos_Error...
#include "strutil.h"			// GetPString

// #pragma data_seg(DATA1)

/**
 * Build full path name in callers buffer given directory
 * name and filename
 * @param pszPathName points to drive/directory if not NULL
 * @returns pointer to full path name in caller's buffer
 * @note OK for pszFullPathName and pszPathName to point to same buffer
 *
 */

PSZ BldFullPathName(PSZ pszFullPathName, PSZ pszPathName, PSZ pszFileName)
{
  UINT c = pszPathName ? strlen(pszPathName) : 0;
  if (c > 0) {
    memcpy(pszFullPathName, pszPathName, c);
    if (pszFullPathName[c - 1] != '\\')
      pszFullPathName[c++] = '\\';
  }
  strcpy(pszFullPathName + c, pszFileName);
  return pszFullPathName;
}

/**
 * Build quoted full path name in callers buffer given
 * directory name and filename
 * @param pszPathName points to drive/directory if not NULL
 * @returns pointer to quoted path name in caller's buffer
 */

PSZ BldQuotedFullPathName(PSZ pszFullPathName, PSZ pszPathName, PSZ pszFileName)
{
  UINT c = pszPathName ? strlen(pszPathName) : 0;
  BOOL q = needs_quoting(pszPathName) ||
	   needs_quoting(pszFileName);
  PSZ psz = pszFullPathName;

  if (q)
    *psz++ = '"';
  if (c > 0) {
    memcpy(psz, pszPathName, c);
    psz += c;
    if (*(psz - 1) != '\\')
      *psz++ = '\\';
  }
  strcpy(psz, pszFileName);
  if (q) {
    psz += strlen(psz);
    *psz++ = '"';
    *psz = 0;
  }
  return pszFullPathName;
}

/**
 * Build quoted full path name in callers buffer given a filename
 * @returns pointer to quoted file name in caller's buffer
 */

PSZ BldQuotedFileName(PSZ pszQuotedFileName, PSZ pszFileName)
{
  BOOL q = needs_quoting(pszFileName);
  PSZ psz = pszQuotedFileName;

  if (q)
    *psz++ = '"';
  strcpy(psz, pszFileName);
  if (q) {
    psz += strlen(psz);
    *psz++ = '"';
    *psz = 0;
  }
  return pszQuotedFileName;
}

/** NormalizeCmdLine
 * Checks a command line for common errors (missing quotes, missing extension,
 * no space between exe and args etc)
 * Command line passed as pszCmdLine_
 * A pointer to a buffer of the size MAXCOMLINESTRG should be supplied in
 * pszWorkBuf. This is where the quoted etc as necessary command
 * line string will be returned.
 */

PCSZ NormalizeCmdLine(PSZ pszWorkBuf, PSZ pszCmdLine_)
{
  char szCmdLine[MAXCOMLINESTRG], szArgs[MAXCOMLINESTRG];
  char *offset = '\0', *offsetexe, *offsetcom, *offsetcmd, *offsetbtm, *offsetbat;
  APIRET ret;
  ULONG ulAppType;
  char *pszChar;
  FILEFINDBUF3 FindBuffer;
  ULONG ulResultBufLen = sizeof(FILEFINDBUF3);
  HDIR hdirFindHandle = HDIR_CREATE;
  ULONG ulFindCount = 1;
  PSZ pszNewCmdLine = pszWorkBuf;

  bstrip(pszCmdLine_);
  strcpy(szCmdLine, pszCmdLine_);
  if (szCmdLine[0] != '\0') {
    offsetexe = strstr(strlwr(pszCmdLine_), ".exe");
    offsetcmd = strstr(strlwr(pszCmdLine_), ".cmd");
    offsetcom = strstr(strlwr(pszCmdLine_), ".com");
    offsetbtm = strstr(strlwr(pszCmdLine_), ".btm");
    offsetbat = strstr(strlwr(pszCmdLine_), ".bat");
    if (offsetexe)
      offset = offsetexe;
    else {
      if (offsetcom)
	offset = offsetcom;
      else {
	if (offsetcmd)
	  offset = offsetcmd;
	else {
	  if (offsetbtm)
	    offset = offsetbtm;
	  else {
	    if (offsetbat)
	      offset = offsetexe;
	  }
	}
      }
    }
    if (offset) {
      szCmdLine[offset + 4 - pszCmdLine_] = '\0';
      strcpy(szArgs, &pszCmdLine_[offset + 4 - pszCmdLine_]);
      while (strchr(szCmdLine, '\"'))
	   remove_first_occurence_of_character("\"", szCmdLine);
      if ((szArgs[0] == '\"' && szArgs[1] == ' ') ||
	   !strstr(pszCmdLine_, "\\:")||
	   strchr(szArgs, '\"') == strrchr(szArgs, '\"'))
	remove_first_occurence_of_character("\"", szArgs);
      if (strchr(szArgs, '\"') != strrchr(szArgs, '\"'))
	saymsg(MB_OK, HWND_DESKTOP,
	       NullStr,
	       GetPString(IDS_QUOTESINARGSTEXT),
	       pszCmdLine_);
      if (!offsetexe) {
	ret = DosFindFirst(szCmdLine, &hdirFindHandle, FILE_NORMAL, &FindBuffer,
			   ulResultBufLen, &ulFindCount, FIL_STANDARD);
	if (ret) {
	  pszChar = szCmdLine;
	  while (pszChar) {
	    if (*pszChar == ' ') {
	      *pszChar = '\0';
	      strcat(szCmdLine, ".exe");
	      ret = DosQueryAppType(szCmdLine, &ulAppType);
	      //printf("%d %s\n", ret, szCmdLine); fflush(stdout);
	      if (!ret) {
		strcpy(szArgs, pszCmdLine_ + strlen(szCmdLine) - 3);
		break;
	      }
	    }
	    strcpy(szCmdLine, pszCmdLine_);
	    pszChar++;
	  }
	}
      }
      else
	ret = DosQueryAppType(szCmdLine, &ulAppType);
      BldQuotedFileName(pszNewCmdLine, szCmdLine);
      //printf("%d A", ret); fflush(stdout);
      if (ret) {
        ret = saymsg(MB_YESNO,
                     HWND_DESKTOP,
                     NullStr,
                     GetPString(IDS_PROGRAMNOTFOUNDTEXT),
                     pszCmdLine_);
        if (ret == MBID_YES){
          pszNewCmdLine = pszCmdLine_;
      }
        else{
          fCancelAction = TRUE;
          pszNewCmdLine = pszCmdLine_;
        }
      }
      else{
        if (szArgs[0] != ' ')
          strcat(pszNewCmdLine, " ");
        strcat(pszNewCmdLine, szArgs);
      }

    }
    else if (szCmdLine && (!strchr(szCmdLine, '.') ||
			 strrchr(szCmdLine, '.' ) < strrchr(szCmdLine, '\\'))) {
      if (!strchr(szCmdLine, ' ')) {
	while (strchr(szCmdLine, '\"'))
	  remove_first_occurence_of_character("\"", szCmdLine);
	strcat(szCmdLine, ".exe");
	ret = DosFindFirst(szCmdLine, &hdirFindHandle, FILE_NORMAL, &FindBuffer,
			   ulResultBufLen, &ulFindCount, FIL_STANDARD);
	//printf("%d", ret); fflush(stdout);
      }
      else {
	pszChar = szCmdLine;
	while (pszChar) {
	  while (strchr(szCmdLine, '\"'))
	    remove_first_occurence_of_character("\"", szCmdLine);
	  if (*pszChar == ' ') {
	    *pszChar = '\0';
	    strcat(szCmdLine, ".exe");
	    ret = DosQueryAppType(szCmdLine, &ulAppType);
	    //printf("%d %s\n", ret, szCmdLine); fflush(stdout);
	    if (!ret) {
	      break;
	    }
	  }
	  strcpy(szCmdLine, pszCmdLine_);
	  pszChar++;
	}
      }
      if (!ret){
        BldQuotedFileName(pszNewCmdLine, szCmdLine);
        strcpy(szArgs, pszCmdLine_ + strlen(szCmdLine) - 3);
        if ((szArgs[0] == '\"' && szArgs[1] == ' ') ||
             !strstr(pszCmdLine_, "\\:" ) ||
             strchr(szArgs, '\"') == strrchr(szArgs, '\"'))
          remove_first_occurence_of_character("\"", szArgs);
        if (strchr(szArgs, '\"') != strrchr(szArgs, '\"'))
          saymsg(MB_OK, HWND_DESKTOP,
                 NullStr,
                 GetPString(IDS_QUOTESINARGSTEXT),
                 pszCmdLine_);
        if (szArgs[0] != ' ')
          strcat(pszNewCmdLine, " ");
        strcat(pszNewCmdLine, szArgs);
      }
      else {
        ret = saymsg(MB_OK,
                     HWND_DESKTOP,
                     NullStr,
                     GetPString(IDS_PROGRAMNOTEXE2TEXT),
                     pszCmdLine_);
          fCancelAction = TRUE;
          pszNewCmdLine = pszCmdLine_;
      }
    }
    else {
      pszChar = strrchr(szCmdLine, '.');
      while (pszChar && *pszChar !=' ') {
	pszChar++;
      }
      *pszChar = '\0';
      strcpy (szArgs, pszCmdLine_ + strlen(szCmdLine));
      while (strchr(szCmdLine, '\"'))
	remove_first_occurence_of_character("\"", szCmdLine);
    if ((szArgs[0] == '\"' && szArgs[1] == ' ') ||
	 !strstr(pszCmdLine_, "\\:")||
	 strchr(szArgs, '\"') == strrchr(szArgs, '\"'))
      remove_first_occurence_of_character("\"", szArgs);
    if (strchr(szArgs, '\"') != strrchr(szArgs, '\"'))
      saymsg(MB_OK, HWND_DESKTOP,
	     NullStr,
	     GetPString(IDS_QUOTESINARGSTEXT),
	     pszCmdLine_);
    ret = DosFindFirst(szCmdLine, &hdirFindHandle, FILE_NORMAL, &FindBuffer,
			 ulResultBufLen, &ulFindCount, FIL_STANDARD);

    BldQuotedFileName(pszNewCmdLine, szCmdLine);
    //printf("%d %s ", ret, szCmdLine); fflush(stdout);
    if (ret) {
      ret = saymsg(MB_YESNO,
		   HWND_DESKTOP,
		   NullStr,
		   GetPString(IDS_PROGRAMNOTFOUNDTEXT),
		   pszCmdLine_);
      if (ret == MBID_YES) {
	pszWorkBuf = pszCmdLine_;
      }
      else {
	fCancelAction = TRUE;
	pszWorkBuf = pszCmdLine_;
      }
    }
    ret = saymsg(MB_YESNOCANCEL,
                   HWND_DESKTOP,
                   NullStr,
                   GetPString(IDS_PROGRAMNOTEXE3TEXT),
                   pszCmdLine_, pszNewCmdLine);
      if (ret == MBID_YES){
        if (szArgs[0] != ' ')
          strcat(pszNewCmdLine, " ");
        strcat(pszNewCmdLine, szArgs);
      }
      if (ret == MBID_CANCEL){
        fCancelAction = TRUE;
        pszNewCmdLine = pszCmdLine_;
      }
    }
  }
  return pszWorkBuf;
}

#pragma alloc_text(PATHUTIL,BldFullPathName)
#pragma alloc_text(PATHUTIL,BldQuotedFileName)
#pragma alloc_text(PATHUTIL,BldQuotedFullPathName)
#pragma alloc_text(PATHUTIL,NormalizeCmdLine)
