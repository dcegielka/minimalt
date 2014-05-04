#ifndef TUNNEL_H
#define TUNNEL_H
#include "nacl/include/amd64/crypto_box.h"
#include <stdint.h>
#include <string.h>
#include "error.h"
#include <unistd.h>

struct tunnel {
  int     sock;
  struct addrinfo *remote;
  uint8_t nonce[crypto_box_NONCEBYTES];
};

error sockListen(const char *port, int *sock);
error findHost(const char *host, const char *port, struct addrinfo *info);

void tunnel_init(struct tunnel *t, int sock);
void tunnel_send(struct tunnel *t, void *buffer, size_t bufferlen);
void tunnel_recv(struct tunnel *t, void *buffer, size_t bufferlen);

#endif
