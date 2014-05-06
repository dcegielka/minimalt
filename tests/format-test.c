// ../format.o
#include "../format.h"
#include "assert.h"

int main() {
  {
    uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    assert("readUint64LE reads 0", readUint64LE(buffer) == 0);
  }

  {
    uint8_t buffer[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    assert("readUint64LE reads a large integer", readUint64LE(buffer) == 0x0807060504030201);
  }
  
  {
    uint8_t buffer[8];
    writeUint64LE(buffer, 0);
    assert("writeUint64LE writes 0", buffer[0] == 0x00 && buffer[1] == 0x00 &&
                                     buffer[2] == 0x00 && buffer[3] == 0x00 &&
                                     buffer[4] == 0x00 && buffer[5] == 0x00 &&
                                     buffer[6] == 0x00 && buffer[7] == 0x00);
  }

  {
    uint8_t buffer[8];
    writeUint64LE(buffer, 0x0102030405060708);
    assert("writeUint64LE writes a large integer", buffer[0] == 0x08 && buffer[1] == 0x07 &&
                                                   buffer[2] == 0x06 && buffer[3] == 0x05 &&
                                                   buffer[4] == 0x04 && buffer[5] == 0x03 &&
                                                   buffer[6] == 0x02 && buffer[7] == 0x01);
  }
}

