
/***********************************************************************

  $Id$

  Version labels

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2004 Steven H.Levine

  Revisions	11 Jun 02 SHL - Add CheckVersion VERREALMINOR
		11 Jun 03 SHL - v3.02b: add JFS and FAT32 support
		04 Nov 03 SHL - v3.02c: misc icon cleanup
		20 Nov 03 SHL - v3.02d: defview fixes
		27 Nov 03 SHL - v3.02e: seeall, viewer traps
		06 Jan 04 SHL - v3.02f: dirsize applet large drives
		10 Jan 04 SHL - v3.02f: filldir: avoid spurious error reports
		11 Mar 04 SHL - v3.02g: valid::CheckDrive: debug removable detect

***********************************************************************/

#define VERMAJOR       3
#define VERMINOR       02
#define VERREALMINOR   "g"		// SHL

#define APPNAME         "FM2"

int CheckVersion (int vermajor,int verminor);
