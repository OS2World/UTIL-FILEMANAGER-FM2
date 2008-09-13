
/***********************************************************************

$Id$

Function declarations neither called or defined in FM/2 source files

Copyright (c) 1993-98 M. Kimes
Copyright (c) 2008 Steven H. Levine

Change log

***********************************************************************/

#if !defined(OBSOLETE_H)
#define OBSOLETE_H



/**************************************************/
/* Lazy Drag API's.                               */
/**************************************************/

BOOL APIENTRY DrgLazyDrag(HWND hwndSource,
			  PDRAGINFO pdinfo,
			  PDRAGIMAGE pdimg, ULONG cdimg, PVOID pRsvd);

BOOL APIENTRY DrgCancelLazyDrag(VOID);

BOOL APIENTRY DrgLazyDrop(HWND hwndTarget,
			  ULONG ulOperation, PPOINTL pptlDrop);

PDRAGINFO APIENTRY DrgQueryDraginfoPtr(PDRAGINFO pRsvd);

PDRAGINFO APIENTRY DrgQueryDraginfoPtrFromHwnd(HWND hwndSource);

PDRAGINFO APIENTRY DrgQueryDraginfoPtrFromDragitem(PDRAGITEM pditem);

ULONG APIENTRY DrgQueryDragStatus(VOID);

PDRAGINFO APIENTRY DrgReallocDraginfo(PDRAGINFO pdinfoOld, ULONG cditem);

 /* Drag Status Flags */
#define DGS_DRAGINPROGRESS         0x0001	/* Standard Drag in Progress. */
#define DGS_LAZYDRAGINPROGRESS     0x0002	/* Lazy Drag in Progress.     */

MRESULT EXPENTRY CatalogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID CloseHelp(VOID);
MRESULT EXPENTRY FileListProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL FilterAttrs(PCNRITEM pci, MASK * mask);
MRESULT EXPENTRY ProgDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

#pragma data_seg(GLOBAL2)
INT butxsize;
INT butysize;
BOOL fUseMCI;

#pragma data_seg(GLOBAL1)
HPOINTER hptrCommon;
HWND hwndTrash;
USHORT nodes;


#endif // OBSOLETE_H
