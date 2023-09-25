# -*-Makefile-*-

set positional-arguments := true

test:
  just run --beam-on 10

build:
  cmake -S src -B build/app
  cmake --build build/app

run *ARGS: build
  #!/usr/bin/env sh
  sh execute-with-nixgl-if-needed.sh ./build/app/CHANGEME-my-n4-prog "$@"
  exit $?

clean:
  rm build -rf
