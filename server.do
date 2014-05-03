deps="main.o server.o net.o error.o format.o nacl/lib/amd64/libnacl.a nacl/lib/amd64/randombytes.o"
redo-ifchange cc $deps
./cc -o $3 $deps
