#ifndef FORMAT_H
#define FORMAT_H
#include <stdint.h>
#include <string.h>

uint64_t readUintLE64(const uint8_t *buffer);
void     writeUintLE64(uint8_t *buffer, uint64_t n);

void showhex(const uint8_t *buffer, size_t len);
void showascii(const uint8_t *buffer, size_t len);

#endif
