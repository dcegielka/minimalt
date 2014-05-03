#include "server.h"

int main() {
  struct mlt_server server;

  fatal(mlt_server_init(&server, "8001"));
  fatal(mlt_server_connect(&server, "127.0.0.1", "8000"));
  mlt_server_close(&server);
}

