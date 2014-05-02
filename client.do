deps="client.o net.o"
redo-ifchange cc $deps
./cc -g -o $3 $deps
