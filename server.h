#ifndef MLT_SERVER_H
#define MLT_SERVER_H
#include "error.h"

struct mlt_server {
  int sock;
};

error mlt_server_init(struct mlt_server *server, const char *port);
error mlt_server_accept(struct mlt_server *server);
void mlt_server_close(struct mlt_server *server);

#endif
