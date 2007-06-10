
/***********************************************************************

  $Id$

  Version labels

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2007 Steven H. Levine

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
  20 Jul 05 SHL v3.04: update standard makefile for resource replacements
  16 Aug 05 SHL v3.04pre3: release
  30 Dec 05 SHL v3.04pre7: wip
  02 Jan 06 SHL v3.04pre8: archiver enhancements
  29 May 06 SHL v3.04pre9: archiver enhancements
  16 Jun 06 SHL v3.04pre10: archiver enhancements
  12 Jul 06 SHL v3.04pre11: hide not selected, compare content
  13 Jul 06 SHL v3.04pre12: Rework error logic to report rather than beeping
  12 Aug 06 SHL v3.04: Release
  23 Aug 06 SHL v3.05beta01: Release
  31 Aug 06 SHL v3.05beta02: Release
  01 Sep 06 SHL v3.05beta03: Release
  24 Sep 06 SHL v3.05beta04: Release
  04 Nov 06 SHL v3.05beta05: Release
  06 Nov 06 SHL v3.05beta06: Release
  03 Feb 07 SHL v3.05beta07: Release
  14 Mar 07 SHL v3.05beta08: Release

***********************************************************************/

#define VERMAJOR       3
#define VERMINOR       05
// Ensure VERREALMINOR nul for non-beta releases
// #define VERREALMINOR   ""
#define VERREALMINOR   ".09 "
//#define VERREALMINOR   ".09 " __DATE__ " " __TIME__

#define APPNAME         "FM2"

int CheckVersion(int vermajor, int verminor);
