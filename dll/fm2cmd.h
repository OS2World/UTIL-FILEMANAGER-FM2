
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(FM2CMD_H)
#define FM2CMD_H

BOOL FM2Command(CHAR * directory, CHAR * command);

// Data declarations
extern BOOL fKeepCmdLine;
extern BOOL fSaveMiniCmds;

#endif // FM2CMD_H
