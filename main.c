#include "server.h"

int main() {
  struct mlt_server server;

  fatal(mlt_server_init(&server, "8000"));
  fatal(mlt_server_accept(&server));
  mlt_server_close(&server);
}

