#!/bin/bash

test_dir=$(dirname "$(readlink -f "$0")")
tmp_dir=$(mktemp -d -t nain4-independent-XXXXXX)

export NAIN4_INSTALL=$tmp_dir/nain4/install/nain4

cd $tmp_dir
meson setup nain4/build/nain4 $test_dir/../nain4/src --prefix $tmp_dir/nain4/install/nain4
meson compile -C nain4/build/nain4
meson install -C nain4/build/nain4

cd $tmp_dir
cmake -S $test_dir/client_independent_installation/ -B build && cmake --build build && ./build/client_exe

exit $?
