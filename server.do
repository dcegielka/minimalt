deps="main.o server.o net.o error.o"
redo-ifchange cc $deps
./cc -o $3 $deps
