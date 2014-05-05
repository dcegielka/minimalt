#ifndef TUNNEL_H
#define TUNNEL_H
#include "nacl/include/amd64/crypto_box.h"
#include <stdint.h>
#include <string.h>
#include "error.h"
#include <unistd.h>

#define mlt_SECRETKEY_SIZE crypto_box_SECRETKEYBYTES
#define mlt_PUBLICKEY_SIZE crypto_box_PUBLICKEYBYTES
#define mlt_ADDRSTR_SIZE   INET6_ADDRSTRLEN
#define mlt_PACKET_OVERHEAD (8 + crypto_box_NONCEBYTES + mlt_PUBLICKEY_SIZE + crypto_box_ZEROBYTES)

struct tunnel {
  uint8_t nonce[crypto_box_NONCEBYTES];
};

void tunnel_initClient(struct tunnel *t, void *serverPublickey);
void tunnel_initServer(struct tunnel *t);

size_t tunnel_buildPacket(struct tunnel *t, void *packet, void *message, size_t messageSize);
error tunnel_openPacket(struct tunnel *t, void *output, void *packet, size_t packetSize, size_t *outputSize);

#endif
