typedef struct MENU {
  USHORT        size;
  USHORT        cmd;
  USHORT        type;
  USHORT        dummy;
  CHAR         *text;
  struct MENU  *next;
} MENU;

#define ACTION    0
#define SUBMENU   1
#define SEPARATOR 2
