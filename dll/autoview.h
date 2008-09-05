MRESULT EXPENTRY AttrListDlgProc(HWND hwnd, ULONG msg, MPARAM mp1,
                                 MPARAM mp2);
MRESULT EXPENTRY AutoViewProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
ULONG CreateHexDump(CHAR * value, ULONG cbValue, CHAR * ret, ULONG retlen,
                    ULONG startval, BOOL longlead);

