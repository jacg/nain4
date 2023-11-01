# -*-Makefile-*-

default:
    just test-nain4

test PATTERN *FLAGS: install-tests
    sh install/nain4-test/run-each-test-in-separate-process.sh {{PATTERN}} {{FLAGS}}

test-nain4 *FLAGS: install-tests
    just test '' {{FLAGS}}

clean:
    fd --no-ignore "^build$"   --exec rm -rf {}
    fd --no-ignore "^install$" --exec rm -rf {}

install-nain4:
    meson setup nain4/build/nain4 nain4/src
    meson compile -C nain4/build/nain4
    meson install -C nain4/build/nain4

install-tests: install-nain4
    meson setup nain4/build/nain4-test nain4/test
    meson compile -C nain4/build/nain4-test
    meson install -C nain4/build/nain4-test

# ----------------------------------------------------------------------
# Add recipes here to help with discovery of recipes in subdirectories

test-examples:
    just examples/

test-client-side:
    just client_side_tests/test-all

test-compile-time:
    just compile-time-tests/test-all
