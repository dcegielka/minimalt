deps="client.o server.o error.o format.o nacl/lib/amd64/libnacl.a nacl/lib/amd64/randombytes.o"
redo-ifchange cc $deps
./cc -g -o $3 $deps
