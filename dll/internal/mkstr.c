#define INCL_DOS
#define INCL_WIN

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "..\version.h"
#include "..\fm3str.h"

/*
 * Creates indexed FM3RES.STR file from text file FM3DLL.STR.
 * FM3DLL.STR contains text strings, one per line, for use by
 * FM3DLL.DLL.
 */

int index (const char *s,const char c) {

    char *p;

    p = strchr(s,c);
    if(p == NULL || !*p)
      return -1;
    return (int)(p - s);
}


/*
 * literal.c
 * Translate a string with tokens in it according to several conventions:
 * 1.  \x1b translates to char(0x1b)
 * 2.  \27  translates to char(27)
 * 3.  \"   translates to "
 * 4.  \'   translates to '
 * 5.  \\   translates to \
 * 6.  \r   translates to carriage return
 * 7.  \n   translates to linefeed
 * 8.  \b   translates to backspace
 * 9.  \t   translates to tab
 * 10. \a   translates to bell
 * 11. \f   translates to formfeed
 *
 * Synopsis
 *    *s = "this\x20is\32a test of \\MSC\\CSM\7"
 *    literal(s);
 *
 *    ( s now equals "this is a test of \MSC\CSM")
 */

#define HEX "0123456789ABCDEF"
#define DEC "0123456789"

int literal (char *fsource) {

  register int wpos,w;
  int          len,oldw;
  char        *fdestin,*freeme,wchar;

  if(!fsource || !*fsource)
    return 0;
  len = strlen(fsource) + 1;
  freeme = fdestin = malloc(len + 1);
  memset(fdestin,0,len);              /* start out with zero'd string */

  w = 0;                              /* set index to first character */
  while(fsource[w]) {                 /* until end of string */
    switch(fsource[w]) {
      case '\\':
        switch(fsource[w + 1]) {
          case 'x' :                  /* hexadecimal */
            wchar = 0;
            w += 2;                   /* get past "\x" */
            if(index(HEX,(char)toupper(fsource[w])) != -1) {
              oldw = w;
              while(((wpos = index(HEX,(char)toupper(fsource[w]))) != -1) &&
                    w < oldw + 2) {
                wchar = (char)(wchar << 4) + (char)wpos;
                w++;
              }
            }
            else
              wchar = 'x';            /* just an x */
            w--;
            *fdestin++ = wchar;
            break;

          case '\\' :                 /* we want a "\" */
            w++;
            *fdestin++ = '\\';
            break;

          case 't' :                  /* tab char */
            w++;
            *fdestin++ = '\t';
            break;

          case 'n' :                  /* new line */
            w++;
            *fdestin++ = '\n';
            break;

          case 'r' :                  /* carr return */
            w++;
            *fdestin++ = '\r';
            break;

          case 'b' :                  /* back space */
            w++;
            *fdestin++ = '\b';
            break;

          case 'f':                   /* formfeed */
            w++;
            *fdestin++ = '\x0c';
            break;

          case 'a':                   /* bell */
            w++;
            *fdestin++ = '\07';
            break;

          case '\'' :                 /* single quote */
            w++;
            *fdestin++ = '\'';
            break;

          case '\"' :                 /* double quote */
            w++;
            *fdestin++ = '\"';
            break;

          default :                   /* decimal */
            w++;                      /* get past "\" */
            wchar = 0;
            if(index(DEC,fsource[w]) != -1) {
              oldw = w;
              do {                    /* cvt to binary */
                wchar = (char)(wchar * 10 + (fsource[w++] - 48));
              } while (index(DEC,fsource[w]) != -1 && w < oldw + 3);
              w--;
            }
            else
              wchar = fsource[w];
            *fdestin ++ = wchar;
            break;
        }
        break;

      default :
        *fdestin++ = fsource[w];
        break;
   }
   w++;
  }
  *fdestin = 0;                               /* terminate the string */

  len = fdestin - freeme;
  memcpy(fsource,freeme,len + 1);             /* swap 'em */
  free(freeme);

  return len;                                 /* return length of string */
}


int main (void) {

  FILE       *fpin,*fpout;
  ULONG       len,x = 0,totallen = 0,thislen;
  USHORT      vermajor = 0,verminor = 0;
  static char str[8192];
  char       *outfile = "FM3RES.STR",*infile = "FM3DLL.STR";

  vermajor = VERMAJOR;
  verminor = VERMINOR;
  printf("\nFM/2 resource string compiler copyright (c) 1998 by M. Kimes"
         "\n Compiles FM3DLL.STR into FM3RES.STR.  For FM/2 version %d.%d.\n",
         vermajor,
         verminor);
  /* open input file */
  fpin = fopen(infile,"r");
  if(fpin) {
    /* open (create) output file */
    fpout = fopen(outfile,"wb");
    if(fpout) {
      /* skip space for numstrs and textlen parameter in outfile */
      if(fseek(fpout,(sizeof(ULONG) * 2) + (sizeof(USHORT) * 2),SEEK_SET)) {
        printf("\n **Seek error on %s!\n",outfile);
        fclose(fpin);
        fclose(fpout);
        remove(outfile);
        return 1;
      }
      /* step through strings, reading from input, writing to output */
      while(!feof(fpin)) {
        if(!fgets(str,sizeof(str),fpin))
          break;
        str[8191] = 0;
        if(*str &&
           str[strlen(str) - 1] == '\n')
          str[strlen(str) - 1] = 0;
        len = (*str) ? literal(str) : 0;
        /* write string to output file, including terminating NULL */
        thislen = fwrite(str,1,len + 1,fpout);
        if(thislen != len + 1) {
          printf("\n **Write error on %s!\n",outfile);
          fclose(fpin);
          fclose(fpout);
          remove(outfile);
          return 1;
        }
        totallen += thislen;
        x++;
      }
      if(x > IDS_NUMSTRS)
        printf("\n **Warning:  There are more strings in file "
               "than there should be -- should be %lu, are %lu.\n",
               IDS_NUMSTRS,
               x);
      else if(x < IDS_NUMSTRS) {
        printf("\n **Error -- insufficient strings in file -- "
               "should be %lu, are %lu.\n",
               IDS_NUMSTRS,
               x);
        fclose(fpin);
        fclose(fpout);
        remove(outfile);
        return 1;
      }
      /* start output file with number of strings (long),
       * size of text (long), and version (2 shorts) */
      if(fseek(fpout,0,SEEK_SET) ||
         fwrite(&x,sizeof(x),1,fpout) != 1 ||
         fwrite(&totallen,sizeof(totallen),1,fpout) != 1 ||
         fwrite(&vermajor,sizeof(vermajor),1,fpout) != 1 ||
         fwrite(&verminor,sizeof(verminor),1,fpout) != 1) {
        printf("\n **Error -- bad seek or write to %s!\n",outfile);
        fclose(fpin);
        fclose(fpout);
        remove(outfile);
        return 1;
      }
      fclose(fpout);
      printf("\n%s successfully written from %s (%lu strings).\n",
             outfile,infile,x);
    }
    else
      printf("\n **Can't create %s!\n",outfile);
    fclose(fpin);
  }
  else
    printf("\n **Can't open %s!\n",infile);
  return 0;
}

