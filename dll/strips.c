#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#pragma alloc_text(MISC8,strip_trail_char,strip_lead_char)


char * strip_trail_char (char *strip,char *a) {

  register char *p;

  if(a && *a && strip && *strip) {
    p = &a[strlen(a) - 1];
    while (*a && strchr(strip,*p) != NULL) {
      *p = 0;
      p--;
    }
  }
  return a;
}

char * strip_lead_char (char *strip,char *a) {

  register char *p = a;

  if(a && *a && strip && *strip) {
    while(*p && strchr(strip,*p) != NULL)
      p++;
    if(p != a)
      memmove(a,p,strlen(p) + 1);
  }
  return a;
}

