#ifndef NET_H
#define NET_H
#include <arpa/inet.h>
#include "error.h"

#define ADDRSTR_SIZE INET6_ADDRSTRLEN

void getAddrstr(struct sockaddr *sa, char *addrstr);
error sockListen(const char *port, int *sock);
error findHost(const char *host, const char *port, struct sockaddr *addr);

#endif
