
/***********************************************************************

  $Id$

  Version labels

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  11 Jun 02 SHL Add CheckVersion VERREALMINOR
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
  13 May 07 GKY v3.05.09: Release 1st warpin installer version
  24 Jun 07 GKY v3.06: Release 1st OpenWatcom release
  21 Aug 07 GKY v3.07beta01: Release
  31 Aug 07 GKY v3.07: Release modified ini structure
  11 Nov 07 GKY v3.08 Release large file support
  10 Jan 08 GKY v3.09 Release
  02 Mar 08 GKY v3.10 Release
  06 Jul 08 GKY v3.11 Release
  21 Jul 08 GKY v3.12
  01 Sep 08 SHL v3.13
  10 May 09 GKY v3.16
  20 Sep 09 GKY v3.17

***********************************************************************/

#define VERMAJOR       3
#define VERMINOR       24
// Ensure VERREALMINOR nul for non-beta releases
// #define VERREALMINOR   ""
#define VERREALMINOR " "__DATE__
// #define VERREALMINOR    __DATE__ " " __TIME__

#define APPNAME         "FM2"

int CheckVersion(int vermajor, int verminor);
