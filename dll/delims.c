#include <os2.h>
#include <stdlib.h>
#include <string.h>

#pragma alloc_text(MISC8,skip_delim,to_delim)


char * skip_delim (char *a,register char *delim) {

  register char *p = a;

  if(p && delim) {
    while(*p) {
      if(strchr(delim,*p))
        p++;
      else
        break;
    }
  }
  return p;
}


char * to_delim (char *a,register char *delim) {

  register char *p = a;

  if(p && delim) {
    while(*p) {
      if(strchr(delim,*p))
        break;
      p++;
    }
  }
  return p;
}
