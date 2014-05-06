#ifndef mlt_SERVER_H
#define mlt_SERVER_H
#include "error.h"
#include "nacl/include/amd64/crypto_box.h"
#include <stdint.h>
#include <netinet/in.h>
#include "map.h"
#include "tunnel.h"

#define mlt_SECRETKEY_SIZE crypto_box_SECRETKEYBYTES
#define mlt_PUBLICKEY_SIZE crypto_box_PUBLICKEYBYTES
#define mlt_ADDRSTR_SIZE   INET6_ADDRSTRLEN
#define MAX_PACKET_SIZE    4096

struct mlt_server {
  int        sock;
  uint8_t    secretkey[mlt_SECRETKEY_SIZE],
             publickey[mlt_PUBLICKEY_SIZE];
  struct map tunnelsByAddrstr,
             tunnelsByTid,
             tunnelsByCid;
};

error mlt_server_init   (struct mlt_server *server, const char *port);
void  mlt_server_rekey  (struct mlt_server *server);
error mlt_server_accept (struct mlt_server *server, uint8_t *message, size_t *messageSize);
error mlt_server_connect(struct mlt_server *server, const char *host, const char *port, void* serverPublickey, uint64_t *cid);
void  mlt_server_close  (struct mlt_server *server);

error mlt_server_send(struct mlt_server *server, uint64_t cid, uint8_t *message, size_t messageSize);

#endif
