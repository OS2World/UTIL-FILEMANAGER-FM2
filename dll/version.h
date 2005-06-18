
/***********************************************************************

  $Id$

  Version labels

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2005 Steven H. Levine

  11 Jun 02 SHL Add CheckVersion VERREALMINOR
  11 Jun 03 SHL v3.02b: add JFS and FAT32 support
  04 Nov 03 SHL v3.02c: misc icon cleanup
  20 Nov 03 SHL v3.02d: defview fixes
  27 Nov 03 SHL v3.02e: seeall, viewer traps
  06 Jan 04 SHL v3.02f: dirsize applet large drives
  10 Jan 04 SHL v3.02f: filldir: avoid spurious error reports
  11 Mar 04 SHL v3.02g: valid::CheckDrive: debug removable detect
  26 Jul 04 SHL v3.02h: valid::CheckDrive: debug invisible FAT32
  31 Jul 04 SHL v3.02h: avv::ArcReviewDlgProc: correct filename flags updates
  10 Jan 05 SHL v3.02h: Allow DND_TARGET to hold CCHMAXPATH
  23 May 05 SHL v3.02i: Avoid datamin delays
  25 May 05 SHL v3.02i: Rework large file/drive support
  11 Jun 05 SHL v3.02i: filldir: Resolve some icon selection oddities
  16 Jun 05 SHL v3.03: rework resource kit

***********************************************************************/

#define VERMAJOR       3
#define VERMINOR       03
#define VERREALMINOR   ""		// SHL

#define APPNAME         "FM2"

int CheckVersion (int vermajor,int verminor);
