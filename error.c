#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void fatal(error err) {
  if (err) {
    puts(err);
    exit(1);
  }
}

