#!/bin/bash

test_dir=$(dirname "$(readlink -f "$0")")
tmp_dir=$(mktemp -d -t nain4-independent-XXXXXX)

export NAIN4_INSTALL=$tmp_dir/nain4/install/

mkdir $tmp_dir/nain4 && cd $_
cmake -DCMAKE_INSTALL_PREFIX=${NAIN4_INSTALL} -S $test_dir/../nain4/src/ -B build
cmake --build build --target install


cd $tmp_dir
cmake -S $test_dir/client_independent_installation/ -B build && cmake --build build && ./build/client_exe

exit $?
