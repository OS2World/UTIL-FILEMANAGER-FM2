
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H
  08 Mar 09 GKY Removed variable aurguments from docopyf and unlinkf (not used)
  04 Aug 12 GKY Changes to use Unlock to unlock files if Unlock.exe is in path both from menu/toolbar and as part of
                copy, move and delete operations
  04 Aug 12 GKY Changes to allow copy and move over readonly files with a warning dialog; also added a warning dialog
                for delete of readonly files

***********************************************************************/

#if !defined(COPYF_H)

#define COPYF_H

BOOL AdjustWildcardName(CHAR * oldname, CHAR * newname);
char *MakeTempName(char *buffer, char *temproot, int type);
BOOL WriteLongName(CHAR * filename, CHAR * longname);
APIRET docopyf(INT type, CHAR * oldname, CHAR * newname);
INT make_deleteable(CHAR * filename, INT error);
INT unlinkf(CHAR * string);
INT wipeallf(CHAR * string, ...);

#endif	// COPYF_H
