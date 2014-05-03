find -name '*-test.c' -exec echo "{}.tested" \; | xargs redo-ifchange
