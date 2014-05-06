// ../tunnel.o ../format.o ../nacl/lib/amd64/libnacl.a ../nacl/lib/amd64/randombytes.o
#include "../tunnel.h"
#include "assert.h"
#include "../format.h"

int main() {
  {
    struct tunnel server,
                  client;

    crypto_box_keypair(server.localPublickey, server.localSecretkey);

    uint64_t tid = createTid();

    tunnel_initClient(&client, tid, server.localPublickey);

    uint8_t message[] = "Attack at dawn.",
            packet[mlt_PACKET_OVERHEAD + sizeof message];

    size_t packetSize = tunnel_buildPacket(&client, packet, message, sizeof message);

    uint64_t serverTid;
    assertError("Can inspect a packet", inspectPacket(packet, packetSize, &serverTid));
    assertEq("Received the same tid", serverTid, tid);

    tunnel_initServer(&server, serverTid);

    uint8_t output[packetSize];
    size_t outputSize;

    assertError("Can open a packet (1)", tunnel_openPacket(&server, packet, output, packetSize, &outputSize));

    assertEqBuf("Received the same message (1)", message, sizeof message, output, outputSize);

    uint8_t message2[] = "Ok.",
            packet2[mlt_PACKET_OVERHEAD + sizeof message2];

    size_t packetSize2 = tunnel_buildPacket(&client, packet2, message2, sizeof message2);

    uint8_t output2[packetSize2];
    size_t  outputSize2;

    assertError("Can open a packet (2)", tunnel_openPacket(&client, packet2, output2, packetSize2, &outputSize2));

    assertEqBuf("Received the same message (2)", message2, sizeof message2, output2, outputSize2);
  }
}

