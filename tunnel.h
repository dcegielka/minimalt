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
#define PACKET_OVERHEAD (8 + crypto_box_NONCEBYTES + mlt_PUBLICKEY_SIZE + crypto_box_ZEROBYTES - crypto_box_BOXZEROBYTES)

enum tunnelState {
  TUNNEL_STATE_NORMAL, TUNNEL_STATE_CLIENT_PRE_HANDSHAKE, TUNNEL_STATE_SERVER_PRE_HANDSHAKE
};

// TODO: The beforenm and afternm crypto box functions should be used to speed up crypto.
struct tunnel {
  void             *extra;
  uint64_t          tid;
  enum tunnelState  state;
  uint8_t           remotePublickey[mlt_PUBLICKEY_SIZE],
                    localPublickey[mlt_PUBLICKEY_SIZE],
                    localSecretkey[mlt_SECRETKEY_SIZE],
                    nonce[crypto_box_NONCEBYTES];
};

void tunnel_initClient(struct tunnel *t, uint64_t tid, void *serverPublickey);
void tunnel_initServer(struct tunnel *t, uint64_t tid);

error inspectPacket(const uint8_t *packet, size_t packetSize, uint64_t *tid);

size_t tunnel_buildPacket(struct tunnel *t, uint8_t *packet, uint8_t *message, size_t messageSize);
error tunnel_openPacket(struct tunnel *t, uint8_t *packet, uint8_t *message, size_t packetSize, size_t *messageSize);

uint64_t createTid();

#endif
