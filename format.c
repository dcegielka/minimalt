#include "format.h"
#include <stdio.h>
#include <ctype.h>

uint64_t readUintLE64(const uint8_t *buffer) {
  return ((uint64_t)buffer[0] << 0x00) |
         ((uint64_t)buffer[1] << 0x08) |
         ((uint64_t)buffer[2] << 0x10) |
         ((uint64_t)buffer[3] << 0x18) |
         ((uint64_t)buffer[4] << 0x20) |
         ((uint64_t)buffer[5] << 0x28) |
         ((uint64_t)buffer[6] << 0x30) |
         ((uint64_t)buffer[7] << 0x38);
}

void writeUintLE64(uint8_t *buffer, uint64_t n) {
  buffer[0] = n >> 0x00;
  buffer[1] = n >> 0x08;
  buffer[2] = n >> 0x10;
  buffer[3] = n >> 0x18;
  buffer[4] = n >> 0x20;
  buffer[5] = n >> 0x28;
  buffer[6] = n >> 0x30;
  buffer[7] = n >> 0x38;
}

void showhex(const uint8_t *buffer, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    printf("%02x", buffer[i]);
  }
}

void showascii(const uint8_t *buffer, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (isprint(buffer[i])) {
      printf("%c ", buffer[i]);
    } else {
      printf("%02x", buffer[i]);
    }
  }
}

