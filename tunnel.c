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
  memset(t->nonce, 0, sizeof t->nonce);
}

void tunnel_initServer(struct tunnel *t, uint64_t tid) {
  t->tid   = tid;
  t->state = TUNNEL_STATE_SERVER_PRE_HANDSHAKE;
  memset(t->nonce, 0, sizeof t->nonce);
}

// TODO: This should probably be refactored so that the indices and stuff are more sane; to prevent
//       memory issues.
size_t tunnel_buildPacket(struct tunnel *t, uint8_t *packet, uint8_t *message, size_t messageSize) {
  const bool      sendingPublickey = t->state == TUNNEL_STATE_CLIENT_PRE_HANDSHAKE;
  const uint64_t  tidWithFlags     = t->tid | (sendingPublickey ? PUBLICKEY_FLAG : 0);
  const size_t    headerSize       = sizeof tidWithFlags +
                                     sizeof t->nonce +
                                     (sendingPublickey ? sizeof t->localPublickey : 0);

  writeUint64LE(packet, tidWithFlags);
  memcpy(&packet[sizeof tidWithFlags], t->nonce, sizeof t->nonce);

  if (sendingPublickey) {
    t->state = TUNNEL_STATE_NORMAL;
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

error inspectPacket(const uint8_t *packet, size_t packetSize, uint64_t *tid) {
  if (packetSize < sizeof *tid) {
    return "Invalid packet";
  }

  *tid = readUint64LE(packet) & ~TID_FLAGS;

  return NULL;
}

error tunnel_openPacket(struct tunnel *t, uint8_t *packet, uint8_t *message, size_t packetSize, size_t *messageSize) {
  if (packetSize < sizeof t->tid) {
    return "Invalid packet";
  }

  uint64_t tidWithFlags       = readUint64LE(packet),
           tid                = tidWithFlags & ~TID_FLAGS;
  bool     hasPublickey       = tidWithFlags & PUBLICKEY_FLAG,
           hasPuzzle          = tidWithFlags & PUZZLE_FLAG,
           expectingPublickey = t->state == TUNNEL_STATE_SERVER_PRE_HANDSHAKE;
  size_t   headerSize         = sizeof tid +
                                sizeof t->nonce +
                                (hasPublickey ? sizeof t->remotePublickey : 0),
           contentSize        = packetSize - headerSize;

  if (headerSize > packetSize) {
    return "Invalid packet (1)";
  }

  if (hasPuzzle) {
    return "Puzzles not supported";
  }

  if (hasPublickey && !expectingPublickey) {
    return "Unexpected public key";
  } else if (!hasPublickey && expectingPublickey) {
    return "Expected public key";
  } else if (hasPublickey && expectingPublickey) {
    t->state = TUNNEL_STATE_NORMAL;
    memcpy(t->remotePublickey, &packet[sizeof tid + sizeof t->nonce], sizeof t->remotePublickey);
  }

  uint8_t crypted[crypto_box_BOXZEROBYTES + contentSize];

  memset(crypted, 0, crypto_box_BOXZEROBYTES);
  memcpy(&crypted[crypto_box_BOXZEROBYTES], &packet[headerSize], contentSize);

  if (crypto_box_open(crypted, crypted, sizeof crypted, &packet[sizeof tid], t->remotePublickey, t->localSecretkey) == -1) {
    return "Invalid packet (2)";
  }

  *messageSize = sizeof crypted - crypto_box_ZEROBYTES;

  memcpy(message, &crypted[crypto_box_ZEROBYTES], *messageSize);

  return NULL;
}

