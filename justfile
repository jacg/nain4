# -*-Makefile-*-

test-nain4:
    just nain4/test-all


test-examples:
    just examples/


test-client-side:
    just client_side_tests/test-all

test-compile-time:
    just compile-time-tests/test-all
