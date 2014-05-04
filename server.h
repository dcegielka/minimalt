#ifndef MLT_SERVER_H
#define MLT_SERVER_H
#include "error.h"
#include "nacl/include/amd64/crypto_box.h"
#include <stdint.h>

#define mlt_SECRETKEY_SIZE crypto_box_SECRETKEYBYTES
#define mlt_PUBLICKEY_SIZE  crypto_box_PUBLICKEYBYTES

struct mlt_server {
  int     sock;
  uint8_t secretkey[mlt_SECRETKEY_SIZE],
          publickey[mlt_PUBLICKEY_SIZE];
};

error mlt_server_init(struct mlt_server *server, const char *port);
void mlt_server_rekey(struct mlt_server *server);
error mlt_server_accept(struct mlt_server *server);
error mlt_server_connect(struct mlt_server *server, const char *host, const char *port, void* serverPublickey);
void mlt_server_close(struct mlt_server *server);

#endif
