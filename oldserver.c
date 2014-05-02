#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include "net.h"

int main() {
  int sockfd,
      new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, "8000", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (!p) {
    fprintf(stderr, "server: failed to bind\n");
    return 2;
  }

  freeaddrinfo(servinfo);

  char buf[100];

  printf("Waiting\n");

  for (;;) {
    sin_size = sizeof their_addr;
    ssize_t nread = recvfrom(sockfd, buf, 100, 0, (struct sockaddr*)&their_addr, &sin_size);

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
    printf("server got connection from %s\n", s);

    buf[nread] = '\0';

    printf("Got %s\n", buf);

    /*if (send(new_fd, "Hello, world!", 13, 0) == -1) {
      perror("send");
    }*/
  }

  return 0;
}

