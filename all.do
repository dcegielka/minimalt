find tests -name '*-test.c' -exec echo "{}.tested" \; | xargs redo-ifchange
