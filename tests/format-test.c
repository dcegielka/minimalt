// ../format.o
#include "../format.h"
#include "assert.h"

int main() {
  {
    uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    assert("readUintLE64 reads 0", readUintLE64(buffer) == 0);
  }

  {
    uint8_t buffer[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    assert("readUintLE64 reads a large integer", readUintLE64(buffer) == 0x0807060504030201);
  }
  
  {
    uint8_t buffer[8];
    writeUintLE64(buffer, 0);
    assert("writeUintLE64 writes 0", buffer[0] == 0x00 && buffer[1] == 0x00 &&
                                     buffer[2] == 0x00 && buffer[3] == 0x00 &&
                                     buffer[4] == 0x00 && buffer[5] == 0x00 &&
                                     buffer[6] == 0x00 && buffer[7] == 0x00);
  }

  {
    uint8_t buffer[8];
    writeUintLE64(buffer, 0x0102030405060708);
    assert("writeUintLE64 writes a large integer", buffer[0] == 0x08 && buffer[1] == 0x07 &&
                                                   buffer[2] == 0x06 && buffer[3] == 0x05 &&
                                                   buffer[4] == 0x04 && buffer[5] == 0x03 &&
                                                   buffer[6] == 0x02 && buffer[7] == 0x01);
  }
}

