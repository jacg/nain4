# -*-Makefile-*-

test-all:
    just test-nain4
    just test-client-side

test-nain4:
    just nain4/test-all

test-client-side:
    just client_side_tests/test-all
