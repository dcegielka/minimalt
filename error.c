#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void fatal(error err) {
  if (err) {
    fprintf(stderr, "%s\n", err);
    exit(1);
  }
}

