
/***********************************************************************

  $Id$

  <<<description here>>>

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008 Steven H. Levine

  Change log

***********************************************************************/

#if !defined(PRESPARM_H)
#define PRESPARM_H

// PMBITMAP_INCLUDED - IBM Toolkit
// INCL_GPIBITMAPS - OpenWatcom Toolkit
// #if !defined(PMBITMAP_INCLUDED) && !defined(INCL_GPIBITMAPS)
// typedef struct _RGB2		/* rgb2 */
// {
//   BYTE bBlue;			/* Blue component of the color definition */
//   BYTE bGreen;			/* Green component of the color definition */
//   BYTE bRed;			/* Red component of the color definition  */
//   BYTE fcOptions;		/* Reserved, must be zero                 */
// }
// RGB2;
// typedef RGB2 *PRGB2;
// #endif

VOID CopyPresParams(HWND target, HWND source);
VOID PresParamChanged(HWND hwnd, CHAR * keyroot, MPARAM mp1, MPARAM mp2);
VOID RestorePresParams(HWND hwnd, CHAR * keyroot);
VOID SavePresParams(HWND hwnd, CHAR * keyroot);
#ifdef INCL_GPI
VOID SetPresParams(HWND hwnd, RGB2 * back, RGB2 * fore, RGB2 * border,
		   CHAR * font);
#endif


#endif // PRESPARM_H
