#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
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
#include "nacl/include/amd64/randombytes.h"

struct tunnel {
};

static const uint64_t publickeyFlag = 1UL << 63,
                      puzzleFlag    = 1UL << 62,
                      tidFlags      = (1UL << 63) | (1UL << 62);

static const size_t tidSize   = sizeof(uint64_t),
                    nonceSize = crypto_box_NONCEBYTES;

static uint64_t pickTid(struct mlt_server *server) {
  uint64_t tid;

  do {
    randombytes((unsigned char*)&tid, sizeof tid);
  } while (map_get(&server->tunnelsByTid, (void*)tid, 0) != NULL);

  return tid;
}

static void *getInAddr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  } else {
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }
}

error mlt_server_init(struct mlt_server *server, const char *port) {
  int err;
  struct addrinfo hints,
                  *server_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags    = AI_PASSIVE;

  if ((err = getaddrinfo(NULL, port, &hints, &server_info))) {
    return gai_strerror(err);
  }

  int sock;

  for (struct addrinfo *p = server_info;; p = p->ai_next) {
    if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }

    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
      close(sock);
      continue;
    }

    if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
      close(sock);
      continue;
    }

    char addrstr[mlt_ADDRSTR_SIZE];

    struct sockaddr *addr = p->ai_addr;

    if (!inet_ntop(addr->sa_family, getInAddr(addr), addrstr, sizeof addrstr)) {
      close(sock);
      continue;
    }

    struct tunnel *tunnel = malloc(sizeof *tunnel);
    uint64_t tid = pickTid(server);

    map_set(&server->tunnelsByAddrstr, addrstr, sizeof addrstr, tunnel);
    map_set(&server->tunnelsByTid, (void*)tid, 0, tunnel);

    break;
    
    if (!p->ai_next) {
      return "Could not bind to any of the addresses.";
    }
  }

  freeaddrinfo(server_info);

  server->sock = sock;

  map_init(&server->tunnelsByAddrstr);
  map_init(&server->tunnelsByTid);

  return NULL;
}

void mlt_server_rekey(struct mlt_server *server) {
  crypto_box_keypair(server->publickey, server->secretkey);
}

error mlt_server_accept(struct mlt_server *server) {
  struct sockaddr_storage remote_addr;
  socklen_t               sin_size = sizeof remote_addr;

  uint8_t buffer[4096];

  ssize_t nread = recvfrom(server->sock, buffer, sizeof buffer, 0, (struct sockaddr*)&remote_addr, &sin_size);

  if (nread < 0) {
    return "Read error.";
  }

  // TODO: Check nread to make sure all these things are in place, that some things aren't
  // negative, etc.
  // TODO: Also deal with the whole crypto_box_BOXZEROBYTES requirement.
  uint64_t tidWithFlags = readUintLE64(buffer),
           tid          = tidWithFlags ^ tidFlags;
  bool     hasPublickey = tidWithFlags & publickeyFlag,
           hasPuzzle    = tidWithFlags & puzzleFlag;

  if (hasPuzzle) {
    return "Received a puzzle, which isn't supported.";
  }

  size_t headerSize  = tidSize + nonceSize + (hasPublickey ? mlt_PUBLICKEY_SIZE : 0),
         contentSize = nread - headerSize;

  uint8_t outputbuffer[4096];

  if (crypto_box_open(outputbuffer, &buffer[headerSize], contentSize, &buffer[tidSize], &buffer[tidSize + nonceSize], server->secretkey) != 0) {
    return "crypto_box_open failure";
  }

  tid = tid | tid; // Temporary so compiler doesn't complain.

  return NULL;
}

error mlt_server_connect(struct mlt_server *server, const char *host, const char *port, void *serverPublickey) {
  int err;
  struct addrinfo hints, *remote_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((err = getaddrinfo(host, port, &hints, &remote_info))) {
    return gai_strerror(err);
  }

  uint8_t  message[] = "Attack at dawn.",
           buffer[tidSize + nonceSize + mlt_PUBLICKEY_SIZE + crypto_box_ZEROBYTES + sizeof message];
  uint64_t tid = 123 | publickeyFlag;

  uint8_t publickey[mlt_PUBLICKEY_SIZE],
          secretkey[mlt_SECRETKEY_SIZE];

  crypto_box_keypair(publickey, secretkey);

  writeUintLE64(buffer, tid);
  memset(&buffer[tidSize], 0, nonceSize);
  memcpy(&buffer[tidSize + nonceSize], publickey, mlt_PUBLICKEY_SIZE);
  memset(&buffer[tidSize + nonceSize + mlt_PUBLICKEY_SIZE], 0, crypto_box_ZEROBYTES);
  memcpy(&buffer[tidSize + nonceSize + mlt_PUBLICKEY_SIZE + crypto_box_ZEROBYTES], message, sizeof message);

  crypto_box(&buffer[tidSize + nonceSize + mlt_PUBLICKEY_SIZE], &buffer[tidSize + nonceSize + mlt_PUBLICKEY_SIZE], crypto_box_ZEROBYTES + sizeof message, &buffer[tidSize], serverPublickey, secretkey);

  // We might need to iterate the addrinfo like in accept above.
  size_t nsent = sendto(server->sock, buffer, sizeof buffer, 0, remote_info->ai_addr, remote_info->ai_addrlen);

  if (nsent < sizeof buffer) {
    return "Didn't send enough.";
  }

  return NULL;
}

// TODO: We need to free the maps and the tunnels in them.
void mlt_server_close(struct mlt_server *server) {
  close(server->sock);
}

