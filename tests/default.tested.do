#!/bin/bash -e
objects=`head -n1 $2 | cut -d ' ' -f 2-`
redo-ifchange $objects
../cc -MD -MF $2.d -o $2.out $2 $objects
read deps <$2.d
rm $2.d
redo-ifchange ${deps#*:}
status=0
valgrind --error-exitcode=1 --quiet --trace-children=yes --track-origins=yes ./$2.out >&2 || status=$?
rm $2.out
exit $status
