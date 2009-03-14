
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)

***********************************************************************/

#if !defined(COPYF_H)

#define COPYF_H

BOOL AdjustWildcardName(CHAR * oldname, CHAR * newname);
char *MakeTempName(char *buffer, char *temproot, int type);
BOOL WriteLongName(CHAR * filename, CHAR * longname);
APIRET docopyf(INT type, CHAR * oldname, CHAR * newname);
INT make_deleteable(CHAR * filename);
INT unlinkf(CHAR * string);
INT wipeallf(CHAR * string, ...);

#endif	// COPYF_H
