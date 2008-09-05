MRESULT EXPENTRY CmdLine2DlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
				 MPARAM mp2);
MRESULT EXPENTRY CmdLineDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL add_cmdline(CHAR * cmdline, BOOL big);
VOID save_cmdlines(BOOL big);

