#define INCL_DOS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <os2.h>

#define lstrip(s)         strip_lead_char(" \t",(s))
#define rstrip(s)         strip_trail_char(" \t",(s))
#define stripcr(s)        strip_trail_char("\r\n",(s))

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


int main (void) {

  FILE       *fpi,*fpo;
  static char s[256];
  char        w;
  int         x;

  printf("Fixing up FM3STR.H\n");
  fpi = fopen("FM3STR.H","r");
  if(fpi) {
    printf("FM3STR.H opened for read.\n");
    fpo = fopen("FM3STR.BAK","w");
    if(fpo) {
      printf("FM3STR.BAK opened for write.\n");
      for(x = 0;x < 4;x++) {
        if(feof(fpi) ||
           !fgets(s,
                  256,
                  fpi)) {
          printf("Premature end of FM3STR.H -- aborting.\n");
          exit(1);
        }
        s[255] = 0;
        stripcr(s);
        lstrip(rstrip(s));
        fprintf(fpo,
                "%s\n",
                s);
      }
      x = 0;
      while(!feof(fpi)) {
        if(!fgets(s,
                  256,
                  fpi))
          break;
        s[255] = 0;
        stripcr(s);
        lstrip(rstrip(s));
        w = s[60];
        s[60] = 0;
        if(*s &&
           strncmp(s,
                   "/*",
                   2)) {
          fprintf(fpo,
                  "%-60.60s  %u\n",
                  s,
                  x);
          x++;
          printf("%u       \r",
                 x);
          fflush(stdout);
        }
        else {
          s[60] = w;
          fprintf(fpo,
                  "%s\n",
                  s);
        }
      }
      printf("Complete.\n");
      fclose(fpo);
    }
    fclose(fpi);
  }
  return 0;
}
