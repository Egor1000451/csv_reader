#!/bin/bash

COMPILER="clang"
FLAGS="-Wall -Wextra -g"
LIBS=""
SOURCE="main.c"
OUTPUT="main"

$COMPILER $FLAGS $LIBS $SOURCE -o $OUTPUT

if [ $? -eq 0 ]; then
  echo "Build successful: ./$OUTPUT"
else
  echo "Build failed!"
  exit 1
fi
