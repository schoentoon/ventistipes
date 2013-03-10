#include "safefree.h"

#include <stdlib.h>

void safefree(void **pp)
{
  if (pp) {
    free(*pp);
    *pp = NULL;
  }
}