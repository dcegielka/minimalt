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

#define PUBLICKEY_FLAG (1UL << 63)
#define PUZZLE_FLAG    (1UL << 62)
#define TID_FLAGS      (PUBLICKEY_FLAG | PUZZLE_FLAG)

#define TID_SIZE 8
#define NONCE_SIZE  crypto_box_NONCEBYTES
#define PUZZLE_SIZE 8

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
      return strerror(errno);
    }

    if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
      close(sock);
      continue;
    }

    break;
    
    if (!p->ai_next) {
      return "Could not bind to any of the addresses.";
    }
  }

  freeaddrinfo(server_info);

  server->sock = sock;

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

  printf("Read %ld bytes\n", nread);

  // TODO: Check nread to make sure all these things are in place, that some things aren't
  // negative, etc.
  // TODO: Also deal with the whole crypto_box_BOXZEROBYTES requirement.
  uint64_t tidWithFlags = readUintLE64(buffer),
           tid          = tidWithFlags ^ TID_FLAGS;
  bool     hasPublickey = tidWithFlags & PUBLICKEY_FLAG,
           hasPuzzle    = tidWithFlags & PUZZLE_FLAG;

  size_t headerSize  = TID_SIZE + NONCE_SIZE + (hasPublickey ? mlt_PUBLICKEY_SIZE : 0) + (hasPuzzle ? PUZZLE_SIZE : 0),
         contentSize = nread - headerSize;

  printf("Has public key %d, puzzle %d\n", hasPublickey, hasPuzzle);
  printf("Header size (%lu) should be %d\n", headerSize, TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE);

  printf("TID    "); showhex(buffer, TID_SIZE); printf("\n");
  printf("Nonce  "); showhex(&buffer[TID_SIZE], NONCE_SIZE); printf("\n");
  printf("Pubkey "); showhex(&buffer[TID_SIZE+NONCE_SIZE], mlt_PUBLICKEY_SIZE); printf("\n");
  printf("Secret "); showhex(&buffer[TID_SIZE+NONCE_SIZE+mlt_PUBLICKEY_SIZE], contentSize); printf("\n");

  uint8_t outputbuffer[4096];

  printf("The first %d bytes should be zero, but it probably sent %d bytes as zero.\n", crypto_box_BOXZEROBYTES, crypto_box_ZEROBYTES);

  int err = crypto_box_open(outputbuffer, &buffer[headerSize], contentSize, &buffer[TID_SIZE], &buffer[TID_SIZE + NONCE_SIZE], server->secretkey);

  if (err == -1) {
    printf("ERROR\n");
  }

  printf("On tid %lu got ", tid); showhex(&outputbuffer[crypto_box_ZEROBYTES], contentSize - crypto_box_ZEROBYTES); printf("\n");

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
           buffer[TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE + crypto_box_ZEROBYTES + sizeof message];
  uint64_t tid = 123 | PUBLICKEY_FLAG;

  uint8_t publickey[mlt_PUBLICKEY_SIZE],
          secretkey[mlt_SECRETKEY_SIZE];

  crypto_box_keypair(publickey, secretkey);

  printf("Tid is %lu with %ld set\n", tid, PUBLICKEY_FLAG);
  writeUintLE64(buffer, tid);
  memset(&buffer[TID_SIZE], 0, NONCE_SIZE);
  memcpy(&buffer[TID_SIZE + NONCE_SIZE], publickey, mlt_PUBLICKEY_SIZE);
  memset(&buffer[TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE], 0, crypto_box_ZEROBYTES);
  memcpy(&buffer[TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE + crypto_box_ZEROBYTES], message, sizeof message);

  crypto_box(&buffer[TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE], &buffer[TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE], crypto_box_ZEROBYTES + sizeof message, &buffer[TID_SIZE], serverPublickey, secretkey);

  // We might need to iterate the addrinfo like in accept above.
  int nsent = sendto(server->sock, buffer, sizeof buffer, 0, remote_info->ai_addr, remote_info->ai_addrlen);

  printf("Message "); showhex(message, sizeof message); printf("\n");
  printf("TID     "); showhex(buffer, TID_SIZE); printf("\n");
  printf("Nonce   "); showhex(&buffer[TID_SIZE], NONCE_SIZE); printf("\n");
  printf("Pubkey  "); showhex(&buffer[TID_SIZE+NONCE_SIZE], mlt_PUBLICKEY_SIZE); printf("\n");
  printf("Secret  "); showhex(&buffer[TID_SIZE+NONCE_SIZE+mlt_PUBLICKEY_SIZE], crypto_box_ZEROBYTES + sizeof message); printf("\n");

  printf("Sent %d\n", nsent);

  return NULL;
}

void mlt_server_close(struct mlt_server *server) {
  close(server->sock);
}

