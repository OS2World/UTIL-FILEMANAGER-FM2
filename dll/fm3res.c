#include <stdlib.h>
#include "version.h"


int _System ResVersion (long *vermajor,long *verminor) {

  int ret = 0;

  if(vermajor && verminor) {
    *vermajor = VERMAJOR;
    *verminor = VERMINOR;
    ret = 1;
  }
  return ret;
}
