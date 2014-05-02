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
#include "server.h"

error mlt_server_init(struct mlt_server *server, const char *port) {
  struct addrinfo hints,
                  *server_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags    = AI_PASSIVE;

  int err;
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

  char buffer[100];

  ssize_t nread = recvfrom(server->sock, buffer, 100, 0, (struct sockaddr*)&remote_addr, &sin_size);

  if (nread < 0) {
    return "Read error.";
  }

  buffer[nread] = '\0';

  printf("Got %s\n", buffer);

  return NULL;
}

void mlt_server_close(struct mlt_server *server) {
  close(server->sock);
}

