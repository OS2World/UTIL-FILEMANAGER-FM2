#include "stdlib.h"
#include "stdio.h"
#include "string.h"

/*
 * Renumbers a dialog .h include file so that the numbers begin at x
 * (arbitrary starting number; see below) and increment by 1.  If a
 * blank line is encounted renumbering begins again at x + ((x % 100) + 100).
 * skips #directives and blank lines (which act as above), but is too
 * dumb to handle anything else.  note lines are "broken" at column 40.
 * handy when reusing (recombining) junk from other projects to form a
 * new project.
 *
 * TEST BEFORE USING TO DETERMINE APPLICABILITY.
 *
 * public domain from M. Kimes.
 */


int main (int argc,char *argv[]) {

  static   FILE  *fpi,*fpo;
  static   int    x = 20000;  /* arbitrary starting number */
  static   char   s[256];
  static   char   filein[260],fileout[260];
  register char  *p;
  register int    z;

  printf("\nUsage:  renum dlg.h\n Produces new renumbered dlg.h & dlg.bak\n");

  if(argc < 2)
    strcpy(filein,"FM3DLG.H");
  else
    strcpy(filein,argv[1]);
  strcpy(fileout,filein);
  p = strrchr(filein,'.');
  if(p)
    strcpy(p,".BAK");
  else
    strcat(p,".BAK");
  if(!stricmp(filein,fileout)) {
    printf(" **Error: input file == output file -- \"%s\" == \"%s\"\n",
           filein,fileout);
    return 1;
  }

  remove(filein);
  rename(fileout,filein);
  fpi = fopen(filein,"r+");
  if(fpi) {
    fpo = fopen(fileout,"w");
    if(fpo) {
      while(!feof(fpi)) {
        if(!fgets(s,256,fpi))
          break;
        if(s[strlen(s) - 1] == '\n')
          s[strlen(s) - 1] = 0;
        p = s;
        while(*p == ' ')
          p++;
        if(p != s)
          memmove(s,p,strlen(p) + 1);
        if(*s) {
          p = &s[strlen(s) - 1];
          while(*p == ' ' && p > s)
            p--;
          if(*p == ' ')
            *p = 0;
        }
        if(!*s) {
          fprintf(fpo,"\n");
          x = (x - (x % 100)) + 100;
        }
        else if((*s == '#' && strncmp(s,"#define ",8)) ||
                !strncmp(s,"/*",2) || *s == '*')
          fprintf(fpo,"%s%s\n",(*s == '*') ? " " : "",s);
        else {
          s[39] = 0;
          p = &s[strlen(s) - 1];
          while(*p == ' ' && p > s)
            p--;
          if(*p == ' ')
            *p = 0;
          if(*s) {
            fprintf(fpo,"%s",s);
            for(z = strlen(s);z < 40;z++)
              fprintf(fpo," ");
            fprintf(fpo,"%d\n",x++);
          }
          else
            fprintf(fpo,"\n");
        }
      }
      fclose(fpo);
    }
    else {
      fclose(fpi);
      fpi = NULL;
      rename(filein,fileout);
      printf(" **Error: couldn't open output file \"%s\"\n",fileout);
      return 1;
    }
    if(fpi)
      fclose(fpi);
  }
  else {
    rename(filein,fileout);
    printf(" **Error: couldn't open input file \"%s\"\n",filein);
    return 1;
  }
  printf("  Complete.\n");
  return 0;
}
