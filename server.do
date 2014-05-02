deps="main.o server.o net.o error.o format.o nacl/lib/amd64/libnacl.a"
redo-ifchange cc $deps
./cc -o $3 $deps
