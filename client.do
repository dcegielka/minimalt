deps="client.o net.o"
redo-ifchange $deps
./cc -g -o $3 $deps
