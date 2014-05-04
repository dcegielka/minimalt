#!/bin/bash
gcc -Wall -Werror -Wextra -std=c99 $* >&2 2>err
status=$?
if [ $status -ne 0 ]
then
  head err >&2
  rm err
  exit $status
else
  rm err
  exit $status
fi
