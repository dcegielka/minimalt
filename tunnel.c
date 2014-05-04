#define _POSIX_C_SOURCE 200112L
#include "tunnel.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
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
#include <netinet/in.h>

error sockListen(const char *port, int *sock) {
  int err;
  struct addrinfo  hints,
                  *server_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags    = AI_PASSIVE;

  if ((err = getaddrinfo(NULL, port, &hints, &server_info))) {
    *sock = -1;
    return gai_strerror(err);
  }

  for (struct addrinfo *p = server_info;; p = p->ai_next) {
    if ((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }

    int yes = 1;
    if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
      close(*sock);
      continue;
    }

    if (bind(*sock, p->ai_addr, p->ai_addrlen) == -1) {
      close(*sock);
      continue;
    }

    break;
    
    if (!p->ai_next) {
      *sock = -1;
      return "Could not bind to any of the addresses.";
    }
  }

  freeaddrinfo(server_info);

  return NULL;
}

error findHost(const char *host, const char *port, struct addrinfo *host_info) {
  int err;
  struct addrinfo hints;

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((err = getaddrinfo(host, port, &hints, &host_info))) {
    return gai_strerror(err);
  }

  return NULL;
}

void tunnel_init(struct tunnel *t, int sock) {
  t->sock = sock;
}
