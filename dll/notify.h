BOOL AddNote(CHAR * note);
HWND DoNotify(char *text);
VOID EndNote(VOID);
VOID HideNote(VOID);
VOID NotifyError(CHAR * filename, APIRET error);
MRESULT EXPENTRY NotifyWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID ShowNote(VOID);

