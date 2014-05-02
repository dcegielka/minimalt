deps="server.o net.o"
redo-ifchange $deps
./cc -o $3 $deps
