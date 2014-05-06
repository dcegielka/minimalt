#define _POSIX_C_SOURCE 200112L
#include "net.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

void getAddrstr(struct sockaddr *sa, char *addrstr) {
  memset(addrstr, 0, ADDRSTR_SIZE);
  inet_ntop(AF_INET, sa, addrstr, ADDRSTR_SIZE);
}

/*static void *getInAddr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  } else {
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }
}*/

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

  // TODO: This loop is probably broken.
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

error findHost(const char *host, const char *port, struct sockaddr *addr) {
  int err;
  struct addrinfo hints, *host_info;

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((err = getaddrinfo(host, port, &hints, &host_info))) {
    return gai_strerror(err);
  }

  memcpy(addr, host_info->ai_addr, host_info->ai_addrlen);

  return NULL;
}

