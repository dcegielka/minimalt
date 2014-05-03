#ifndef ASSERT_H
#define ASSERT_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define RED   "\x1b[31m"
#define GREEN "\x1b[32m"
#define CLEAR "\x1b[0m"

#define SUCCESS "\xe2\x9c\x94"
#define FAILURE "\xe2\x9c\x98"

void assert(const char *message, bool condition) {
  if (condition) {
    fprintf(stderr, GREEN " " SUCCESS " %s\n" CLEAR, message);
  } else {
    fprintf(stderr, RED " " FAILURE " %s\n" CLEAR, message);
    exit(1);
  }
}

#endif
