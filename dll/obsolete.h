
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

