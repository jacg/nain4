# -*-Makefile-*-

test-nain4 *FLAGS:
    just nain4/test-all {{FLAGS}}


test-examples:
    just examples/


test-client-side:
    just client_side_tests/test-all

test-compile-time:
    just compile-time-tests/test-all

clean-deep:
    fd --no-ignore "^build$"   --exec rm -rf {}
    fd --no-ignore "^install$" --exec rm -rf {}
