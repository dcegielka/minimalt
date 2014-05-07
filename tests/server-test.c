// ../server.o ../tunnel.o ../map.o ../net.o ../format.o ../nacl/lib/amd64/randombytes.o ../nacl/lib/amd64/libnacl.a
#include "../server.h"
#include "assert.h"

int main() {
  struct mlt_server alice, bob;
  uint8_t message[] = "Hi Bob, how are you?",
          message2[] = "Hi again.",
          message3[] = "Hi Alice, I'm doing well.";

  mlt_server_rekey(&bob);
  pid_t pid = fork();

  if (pid == 0) {
    uint64_t cid;

    // Alice
    assertError("Can initialize Alice", mlt_server_init(&alice, "8000"));
    assertError("Alice can connect to Bob", mlt_server_connect(&alice, "127.0.0.1", "8001", bob.publickey, &cid));
    assertError("Alice can send a message to Bob", mlt_server_send(&alice, cid, message, sizeof message));

    uint64_t cid2;

    assertError("Alice can reconnect to Bob", mlt_server_connect(&alice, "127.0.0.1", "8001", bob.publickey, &cid2));
    assert("Alice's new connection uses a different cid", cid != cid2);
    assertError("Alice can send a message to Bob (2)", mlt_server_send(&alice, cid2, message2, sizeof message2));
    
    uint8_t received[MAX_PACKET_SIZE];
    size_t  receivedSize;
    uint64_t cid3;

    assertError("Alice can receive a message on the same connection", mlt_server_accept(&alice, &cid3, received, &receivedSize));
    assertEqBuf("Alice received the correct message", received, receivedSize, message3, sizeof message3);
    assertEq("Alice received the message on the same connection", cid3, cid2);

    mlt_server_close(&alice);
  } else {
    uint8_t received[MAX_PACKET_SIZE];
    size_t  receivedSize;
    uint64_t cid, cid2;

    // Bob
    assertError("Can initialize Bob", mlt_server_init(&bob, "8001"));
    assertError("Bob can receive from Alice", mlt_server_accept(&bob, &cid, received, &receivedSize));
    assertEqBuf("Bob's received message is the same", received, receivedSize, message, sizeof message);

    assertError("Bob can receive another message from Alice", mlt_server_accept(&bob, &cid2, received, &receivedSize));
    assert("Bob's new connection uses a different cid", cid != cid2);
    assertEqBuf("Bob's received message is the same (2)", received, receivedSize, message2, sizeof message2);

    assertError("Bob can send a message to Alice on the same connection", mlt_server_send(&bob, cid2, message3, sizeof message3));

    mlt_server_close(&bob);
  }
}

