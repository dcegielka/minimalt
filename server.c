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

#define PUBLICKEY_FLAG (1L >> 63)
#define PUZZLE_FLAG    (1L >> 62)
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

error mlt_server_accept(struct mlt_server *server) {
  struct sockaddr_storage remote_addr;
  socklen_t               sin_size = sizeof remote_addr;

  uint8_t buffer[1000];

  ssize_t nread = recvfrom(server->sock, buffer, 1000, 0, (struct sockaddr*)&remote_addr, &sin_size);

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

  size_t headerSize  = TID_SIZE + NONCE_SIZE + hasPublickey*mlt_PUBLICKEY_SIZE + hasPuzzle*PUZZLE_SIZE,
         contentSize = nread - headerSize;

  uint8_t secretbuffer[1000+crypto_box_BOXZEROBYTES],
          message[1000+crypto_box_BOXZEROBYTES];

  memset(secretbuffer, 0, crypto_box_BOXZEROBYTES);
  memcpy(&secretbuffer[crypto_box_BOXZEROBYTES], &buffer[headerSize], contentSize);

  printf("TID    "); showHex(buffer, TID_SIZE); printf("\n");
  printf("Nonce  "); showHex(&buffer[TID_SIZE], NONCE_SIZE); printf("\n");
  printf("Pubkey "); showHex(&buffer[TID_SIZE+NONCE_SIZE], mlt_PUBLICKEY_SIZE); printf("\n");
  printf("Secret "); showHex(&buffer[TID_SIZE+NONCE_SIZE+mlt_PUBLICKEY_SIZE], crypto_box_ZEROBYTES + 15); printf("\n");

  crypto_box_open(&buffer[headerSize], message, contentSize, &buffer[TID_SIZE], &buffer[TID_SIZE + NONCE_SIZE], server->secretkey);

  printf("On tid %lu got %s\n", tid, message);

  return NULL;
}

error mlt_server_connect(struct mlt_server *server, const char *host, const char *port) {
  int err;
  struct addrinfo hints, *remote_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((err = getaddrinfo(host, port, &hints, &remote_info))) {
    return gai_strerror(err);
  }

  const char *message = "Attack at dawn.";

  uint8_t buffer[TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE + crypto_box_ZEROBYTES + 20];

  uint64_t tid = 123 | PUBLICKEY_FLAG;

  uint8_t publickey[mlt_PUBLICKEY_SIZE],
          secretkey[mlt_SECRETKEY_SIZE];

  crypto_box_keypair(publickey, secretkey);

  memset(buffer, 0, sizeof buffer);
  writeUintLE64(buffer, tid);

  memcpy(&buffer[TID_SIZE + NONCE_SIZE], publickey, mlt_PUBLICKEY_SIZE);
  memcpy(&buffer[TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE + crypto_box_ZEROBYTES], message, strlen(message));

  crypto_box(&buffer[TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE], &buffer[TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE], strlen(message) + crypto_box_ZEROBYTES, &buffer[TID_SIZE], publickey, secretkey);

  // We might need to iterates the addrinfo like in accept above.
  int nsent = sendto(server->sock, buffer, TID_SIZE + NONCE_SIZE + mlt_PUBLICKEY_SIZE + crypto_box_ZEROBYTES + strlen(message), 0, remote_info->ai_addr, remote_info->ai_addrlen);

  printf("TID    "); showHex(buffer, TID_SIZE); printf("\n");
  printf("Nonce  "); showHex(&buffer[TID_SIZE], NONCE_SIZE); printf("\n");
  printf("Pubkey "); showHex(&buffer[TID_SIZE+NONCE_SIZE], mlt_PUBLICKEY_SIZE); printf("\n");
  printf("Secret "); showHex(&buffer[TID_SIZE+NONCE_SIZE+mlt_PUBLICKEY_SIZE], crypto_box_ZEROBYTES + strlen(message)); printf("\n");

  printf("Sent %d\n", nsent);

  return NULL;
}

void mlt_server_close(struct mlt_server *server) {
  close(server->sock);
}

