// ../server.o ../format.o ../nacl/lib/amd64/libnacl.a ../nacl/lib/amd64/randombytes.o
#include "../server.h"
#include "assert.h"

int main() {
  {
    struct mlt_server server,
                      client;

    mlt_server_init(&server, "8000");
    mlt_server_init(&client, "8001");

    mlt_server_rekey(&server);

    mlt_server_connect(&client, "localhost", "8000", server.publickey);
    mlt_server_accept(&server);

    mlt_server_close(&client);
    mlt_server_close(&server);
    
    assert("Failed!", 0);
  }
}

