# -*-Makefile-*-

test:
  just run --beam-on 10

build:
  cmake -S src -B build/app
  cmake --build build/app

run *ARGS: build
  #!/usr/bin/env sh
  just run-dispatch-vis-cli ./build/app/CHANGEME-my-n4-prog {{ARGS}}
  exit $?

clean:
  rm build -rf

run-dispatch-vis-system *COMMAND:
  #!/usr/bin/env sh
  sh execute-with-nixgl-if-needed-by-system.sh {{COMMAND}}

run-dispatch-vis-cli *COMMAND:
  #!/usr/bin/env sh
  gui=$(echo "{{COMMAND}}" | grep -E "\-\-vis\-macro | \-g")
  if [ -z "$gui" ]; then
      {{COMMAND}}
  else
      just run-dispatch-vis-system {{COMMAND}}
  fi
