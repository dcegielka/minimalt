redo-ifchange cc $2.c
./cc -MD -MF $2.d -c -o $3 $2.c
read deps <$2.d
rm $2.d
redo-ifchange ${deps#*:}
