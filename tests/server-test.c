// ../server.o ../tunnel.o ../map.o ../net.o ../format.o ../nacl/lib/amd64/randombytes.o ../nacl/lib/amd64/libnacl.a
#include "../server.h"
#include "assert.h"

int main() {
  struct mlt_server alice, bob;
  uint8_t message[] = "Hello, world!";

  mlt_server_rekey(&bob);
  pid_t pid = fork();

  if (pid == 0) {
    struct mlt_conn conn;

    // Alice
    assertError("Can initialize Alice", mlt_server_init(&alice, "8000"));
    assertError("Alice can connect to Bob", mlt_server_connect(&alice, "127.0.0.1", "8001", bob.publickey, &conn));
    assertError("Alice can send a message to Bob", mlt_conn_send(&conn, message, sizeof message));
  } else {
    // Bob
    assertError("Can initialize Bob", mlt_server_init(&bob, "8001"));
    assertError("Bob can receive from Alice", mlt_server_accept(&bob));
  }
}

