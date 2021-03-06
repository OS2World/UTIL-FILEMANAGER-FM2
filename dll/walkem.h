
/***********************************************************************

  $Id$

  Misc persistent lists support

  Copyright (c) 1993-98 M. Kimes
  Copyright (c) 2008, 2009 Steven H. Levine

  17 Jul 08 SHL Baseline
  12 Aug 12 GKY Allow for selection of include subdirectories or a list file on initial startup of compare dirs

***********************************************************************/

#if !defined(WALKEM_H)
#define WALKEM_H

#if !defined(OS2_INCLUDED)
#include <os2.h>
#endif

typedef struct
{
  USHORT size;
  USHORT dummy;
  CHAR szCurrentPath1[CCHMAXPATH];
  CHAR szCurrentPath2[CCHMAXPATH];
  BOOL includesubdirs;
  BOOL listfile;

}
WALK2;

typedef struct LINKDIRS
{
  CHAR *path;
  struct LINKDIRS *next;
}
LINKDIRS;

VOID FillPathListBox(HWND hwnd, HWND hwnddrive, HWND hwnddir, PSZ path,
		     BOOL nounwriteable);
MRESULT EXPENTRY WalkAllDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY WalkCopyDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
MRESULT EXPENTRY WalkDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY WalkExtractDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				    MPARAM mp2);
MRESULT EXPENTRY WalkMoveDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
MRESULT EXPENTRY WalkTargetDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
MRESULT EXPENTRY WalkTwoCmpDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
MRESULT EXPENTRY WalkTwoSetDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				   MPARAM mp2);
INT add_setup(PCSZ stateName);
BOOL add_udir(BOOL userdirs, PSZ inpath);
VOID fill_setups_list(VOID);
VOID free_ldir(VOID);
VOID free_udirs(VOID);
VOID load_udirs(VOID);
INT remove_setup(PSZ stateName);
BOOL remove_udir(PSZ path);
VOID save_setups(VOID);
VOID save_udirs(VOID);

#ifdef FORTIFY
VOID free_ldir(VOID);
VOID free_setups(VOID);
#endif

// Data declarations
extern BOOL fUdirsChanged;
extern LINKDIRS *ldirhead;
extern BOOL loadedudirs;
extern LINKDIRS *udirhead;

#endif // WALKEM_H
