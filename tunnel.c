#define _POSIX_C_SOURCE 200112L
#include "tunnel.h"

void tunnel_initClient(struct tunnel *t, void *serverPublickey) {
}

void tunnel_initServer(struct tunnel *t) {
}

size_t tunnel_buildPacket(struct tunnel *t, void *packet, void *message, size_t messageSize);
error tunnel_openPacket(struct tunnel *t, void *output, void *packet, size_t packetSize, size_t *outputSize);


