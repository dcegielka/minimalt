#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "server.h"
#include "format.h"
#include "net.h"
#include "nacl/include/amd64/randombytes.h"

struct tunnel_extra {
  struct sockaddr_storage addr;
};

static uint64_t pickTid(struct mlt_server *server) {
  uint64_t tid;

  do {
    tid = createTid();
  } while (map_get(&server->tunnelsByTid, (void*)tid, 0) != NULL);

  return tid;
}

static uint64_t pickCid(struct mlt_server *server) {
  uint64_t cid;

  do {
    randombytes((unsigned char*)&cid, sizeof cid);
  } while (map_get(&server->tunnelsByCid, (void*)cid, 0) != NULL);

  return cid;
}

error mlt_server_init(struct mlt_server *server, const char *port) {
  returnerr(sockListen(port, &server->sock));

  map_init(&server->tunnelsByAddrstr);
  map_init(&server->tunnelsByTid);
  map_init(&server->tunnelsByCid);

  return NULL;
}

void mlt_server_rekey(struct mlt_server *server) {
  crypto_box_keypair(server->publickey, server->secretkey);
}

error mlt_server_accept(struct mlt_server *server, uint64_t *cid, uint8_t *message, uint64_t *messageSize) {
  struct sockaddr_storage remote_addr;
  socklen_t               sin_size = sizeof remote_addr;

  uint8_t packet[MAX_PACKET_SIZE];

  ssize_t packetSize = recvfrom(server->sock, packet, sizeof packet, 0, (struct sockaddr*)&remote_addr, &sin_size);
  
  if (packetSize < 0) {
    return "Read error.";
  }

  uint64_t tid;
  returnerr(inspectPacket(packet, packetSize, &tid));

  struct tunnel *t = map_get(&server->tunnelsByTid, (void*)tid, 0);

  // TODO: Add tunnel to tunnelsByAddrstr
  if (t == NULL) {
    t = malloc(sizeof *t);

    if (!t) {
      return "malloc failure";
    }

    t->extra = malloc(sizeof(struct tunnel_extra));

    if (!t->extra) {
      return "malloc failure";
    }

    tunnel_initServer(t, tid);
    memcpy(t->localPublickey, server->publickey, sizeof t->localPublickey);
    memcpy(t->localSecretkey, server->secretkey, sizeof t->localSecretkey);
  }

  returnerr(tunnel_openPacket(t, packet, message, packetSize, messageSize));

  if (*messageSize < sizeof *cid) {
    return "Message too short";
  }

  *messageSize -= sizeof *cid;

  *cid = readUint64LE(message);
  memmove(message, message + sizeof *cid, *messageSize);

  return NULL;
}

error mlt_server_connect(struct mlt_server *server, const char *host, const char *port, void *serverPublickey, uint64_t *cid) {
  struct sockaddr_storage addr;
  char addrstr[ADDRSTR_SIZE];

  memset(&addr, 0, sizeof addr);
  returnerr(findHost(host, port, (struct sockaddr*)&addr));

  getAddrstr((struct sockaddr*)&addr, addrstr);

  struct tunnel *t = map_get(&server->tunnelsByAddrstr, addrstr, ADDRSTR_SIZE);

  if (t == NULL) {
    uint64_t tid = pickTid(server);

    t = malloc(sizeof *t);

    if (!t) {
      return "malloc failure";
    }

    t->extra = malloc(sizeof(struct tunnel_extra));

    if (!t->extra) {
      return "malloc failure";
    }

    tunnel_initClient(t, tid, serverPublickey);
    memcpy(&((struct tunnel_extra*)t->extra)->addr, &addr, sizeof addr);

    returnerr(map_set(&server->tunnelsByAddrstr, addrstr, ADDRSTR_SIZE, t));
    returnerr(map_set(&server->tunnelsByTid, (void*)tid, 0, t));
  }

  *cid = pickCid(server);

  returnerr(map_set(&server->tunnelsByCid, (void*)*cid, 0, t));

  return NULL;
}

error mlt_server_send(struct mlt_server *server, uint64_t cid, uint8_t *message, size_t messageSize) {
  struct tunnel *t = map_get(&server->tunnelsByCid, (void*)cid, 0);

  if (t == NULL) {
    return "tunnel doesn't exist";
  }

  uint8_t headerMessage[sizeof cid + messageSize];

  writeUint64LE(headerMessage, cid);
  memcpy(headerMessage + sizeof cid, message, messageSize);

  struct sockaddr *addr = (struct sockaddr*)&((struct tunnel_extra*)t->extra)->addr;

  uint8_t packet[PACKET_OVERHEAD + messageSize];

  size_t packetSize = tunnel_buildPacket(t, packet, headerMessage, sizeof headerMessage);

  if (sendto(server->sock, packet, packetSize, 0, addr, sizeof(struct sockaddr_storage)) == -1) {
    // TODO: Make this string dependent on errno.
    return "Failed to send.";
  }

  return NULL;
}

// TODO: We need to free the maps and the tunnels in them.
void mlt_server_close(struct mlt_server *server) {
  close(server->sock);
}

