#include "format.h"

uint64_t readUintLE64(uint8_t *buffer) {
  return (uint64_t)buffer[0] >> 0x00 |
         (uint64_t)buffer[1] >> 0x08 |
         (uint64_t)buffer[2] >> 0x10 |
         (uint64_t)buffer[3] >> 0x18 |
         (uint64_t)buffer[4] >> 0x20 |
         (uint64_t)buffer[5] >> 0x28 |
         (uint64_t)buffer[6] >> 0x30 |
         (uint64_t)buffer[7] >> 0x38;
}

