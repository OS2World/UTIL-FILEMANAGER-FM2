
/***********************************************************************

  $Id$

  <<description here>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2001, 2008 Steven H. Levine

  05 Sep 08 JBS Ticket 187: Refactor FM3DLL.H

***********************************************************************/

#if !defined(COLORS_H)

#define COLORS_H

typedef struct
{
  USHORT size;
  USHORT numcolors;
  USHORT flags;
  USHORT currentcolor;
  ULONG prompt;
  long *colors;
  ULONG descriptions;
  long *origs;
}
COLORS;

// PMBITMAP_INCLUDED - IBM Toolkit
// INCL_GPIBITMAPS - OpenWatcom Toolkit
#if !defined(PMBITMAP_INCLUDED) && !defined(INCL_GPIBITMAPS)
typedef struct _RGB2           /* rgb2 */
{
  BYTE bBlue;                  /* Blue component of the color definition */
  BYTE bGreen;                 /* Green component of the color definition */
  BYTE bRed;                   /* Red component of the color definition  */
  BYTE fcOptions;              /* Reserved, must be zero                 */
}
RGB2;
typedef RGB2 *PRGB2;
#endif

MRESULT EXPENTRY ColorDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

#endif	// COLORS_H
