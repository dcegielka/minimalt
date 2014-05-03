// ../nacl/lib/amd64/libnacl.a ../nacl/lib/amd64/randombytes.o
#include "../nacl/include/amd64/crypto_box.h"
#include <stdint.h>
#include "assert.h"
#include <string.h>

int main() {
  {
    int     err;
    uint8_t message[] = "Attack at dawn.",
            nonce[crypto_box_NONCEBYTES] = {0},
            publickey1[crypto_box_PUBLICKEYBYTES],
            secretkey1[crypto_box_SECRETKEYBYTES],
            publickey2[crypto_box_PUBLICKEYBYTES],
            secretkey2[crypto_box_SECRETKEYBYTES];

    err = crypto_box_keypair(publickey1, secretkey1);
    assert("crypto_box_keypair returns 0", err == 0);

    err = crypto_box_keypair(publickey2, secretkey2);
    assert("crypto_box_keypair returns 0 (again)", err == 0);

    uint8_t messagebuffer[crypto_box_ZEROBYTES + sizeof message],
            cryptedbuffer[sizeof messagebuffer];

    memset(messagebuffer, 0, crypto_box_ZEROBYTES);
    memcpy(&messagebuffer[crypto_box_ZEROBYTES], message, sizeof message);

    err = crypto_box(cryptedbuffer, messagebuffer, sizeof messagebuffer, nonce, publickey2, secretkey1);
    assert("crypto_box returns 0", err == 0);

    uint8_t outputbuffer[sizeof cryptedbuffer];

    err = crypto_box_open(outputbuffer, cryptedbuffer, sizeof outputbuffer, nonce, publickey1, secretkey2);
    assert("crypto_box_open returns 0", err == 0);

    assert("The correct message was decrypted", memcmp(&outputbuffer[crypto_box_ZEROBYTES], message, sizeof message) == 0);
  }
}

