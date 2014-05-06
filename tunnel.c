#include "tunnel.h"
#include <stdbool.h>
#include "format.h"

#define PUBLICKEY_FLAG (1LU << 63)
#define PUZZLE_FLAG    (1LU << 62)
#define TID_FLAGS      (PUBLICKEY_FLAG | PUZZLE_FLAG)

void tunnel_initClient(struct tunnel *t, uint64_t tid, void *serverPublickey) {
  t->tid   = tid;
  t->state = TUNNEL_STATE_CLIENT_PRE_HANDSHAKE;
  crypto_box_keypair(t->localPublickey, t->localSecretkey);
  memcpy(t->remotePublickey, serverPublickey, sizeof t->remotePublickey);
}

void tunnel_initServer(struct tunnel *t, uint64_t tid) {
  t->tid   = tid;
  t->state = TUNNEL_STATE_SERVER_PRE_HANDSHAKE;
}

size_t tunnel_buildPacket(struct tunnel *t, uint8_t *packet, uint8_t *message, size_t messageSize) {
  const bool      sendingPublickey = t->state == TUNNEL_STATE_CLIENT_PRE_HANDSHAKE;
  const uint64_t  tidWithFlags     = t->tid | (sendingPublickey ? PUBLICKEY_FLAG : 0);
  const size_t    headerSize       = sizeof tidWithFlags +
                                     sizeof t->nonce +
                                     (sendingPublickey ? sizeof t->localPublickey : 0);

  writeUintLE64(packet, tidWithFlags);
  memcpy(&packet[sizeof tidWithFlags], t->nonce, sizeof t->nonce);

  if (sendingPublickey) {
    memcpy(&packet[sizeof tidWithFlags + sizeof t->nonce],
           t->localPublickey,
           sizeof t->localPublickey);
  }

  uint8_t crypted[crypto_box_ZEROBYTES + messageSize];

  memset(crypted, 0, crypto_box_ZEROBYTES);
  memcpy(&crypted[crypto_box_ZEROBYTES], message, messageSize);

  crypto_box(crypted, crypted, sizeof crypted, t->nonce, t->remotePublickey, t->localSecretkey);

  memcpy(&packet[headerSize],
         &crypted[crypto_box_BOXZEROBYTES],
         sizeof crypted - crypto_box_BOXZEROBYTES);

  return headerSize + sizeof crypted - crypto_box_BOXZEROBYTES;
}

void openPacket(uint8_t *packet, uint64_t *tid, ) {
  uint64_t tidWithFlags = readUintLE64(packet),
           tid          = tidWithFlags ^ TID_FLAGS;
  bool     hasPublickey = tidWithFlags & PUBLICKEY_FLAG,
           hasPuzzle    = tidWithFlags & PUZZLE_FLAG;


}


