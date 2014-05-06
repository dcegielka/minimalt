// ../tunnel.o ../nacl/lib/amd64/libnacl.a ../nacl/lib/amd64/randombytes.o
#include "../tunnel.h"
#include "assert.h"
#include <alloca.h>

int main() {
  {
    struct tunnel server,
                  client;
    uint8_t       serverPublickey[mlt_PUBLICKEY_SIZE],
                  serverSecretkey[mlt_SECRETKEY_SIZE];

    crypto_box_keypair(serverPublickey, serverSecretkey);

    tunnel_initClient(&client, 1, serverPublickey);

    uint8_t message[] = "Attack at dawn.",
            packet[mlt_PACKET_OVERHEAD + sizeof message];

    size_t packetSize = tunnel_buildPacket(&client, packet, message, sizeof message);

    tunnel_initServer(&server, 1);

    uint8_t *output = alloca(packetSize);
    size_t   outputSize;

    assertError("Can open a packet (1)", tunnel_openPacket(&server, output, packet, packetSize, &outputSize));
    assertEqBuf("Received the same message (1)", message, sizeof message, output, outputSize);

    uint8_t message2[] = "Ok.",
            packet2[mlt_PACKET_OVERHEAD + sizeof message2];

    size_t packetSize2 = tunnel_buildPacket(&client, packet2, message2, sizeof message2);

    uint8_t *output2 = alloca(packetSize2);
    size_t   outputSize2;

    assertError("Can open a packet (2)", tunnel_openPacket(&client, output2, packet2, packetSize2, &outputSize2));

    assertEqBuf("Received the same message (2)", message2, sizeof message2, output2, outputSize2);
  }
}

