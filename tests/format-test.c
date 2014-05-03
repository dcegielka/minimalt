// ../format.o
#include "../format.h"
#include "assert.h"

void testReadUintLE64() {
  {
    uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    assert("readUintLE64 reads 0", readUintLE64(buffer) == 0);
  }

  {
    uint8_t buffer[8] = {0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff};
    assert("readUintLE64 reads a large integer", readUintLE64(buffer) == 0xff00ff00ff00ff00);
  }
}

int main() {
  testReadUintLE64();
}

