// ../tunnel.o ../nacl/lib/amd64/libnacl.a ../nacl/lib/amd64/randombytes.o
#include "../tunnel.h"
#include "assert.h"

int main() {
  {
    struct tunnel server, client;
    int serversock, clientsock;

    assertError("Server listen", sockListen("8000", &serversock));
    assertError("Client listen", sockListen("8001", &clientsock));

    assert("Server sock isn't negative", serversock >= 0);
    assert("Client sock isn't negative", clientsock >= 0);

    tunnel_init(&server, serversock);
    tunnel_init(&client, clientsock);

    close(serversock);
    close(clientsock);
  }
}

