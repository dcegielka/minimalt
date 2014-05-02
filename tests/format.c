#include "../format.h"
#include <stdbool.h>

void assert(const char *message, bool condition) {
  if (!condition) {
    fprintf(stderr, "%s\n", message);
    exit(1);
  }
}

void testReadUintLE64() {
  {
    uint8_t buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    assert("Reads 0", readUintLE64(buffer) == 0);
  }

  {
    uint8_t buffer[8] = {0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff};
    assert("Reads a large integer", readUintLE64(buffer) == 0xff00ff00ff00ff00);
  }
}

int main() {
  testReadUintLE64();
}

