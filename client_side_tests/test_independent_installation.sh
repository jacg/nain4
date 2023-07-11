#!/bin/bash

test_dir=$(dirname "$(readlink -f "$0")")
tmp_dir=$(mktemp -d -t nain4-independent-XXXXXX)

export NAIN4_INSTALL=$tmp_dir/nain4/install/

mkdir    $tmp_dir/nain4/install
mkdir -p $tmp_dir/nain4/build && cd $_
cmake -DCMAKE_INSTALL_PREFIX=${NAIN4_INSTALL} $test_dir/../nain4/
make && make install

mkdir $tmp_dir/client-build && cd $_
cmake $test_dir/client_independent_installation/ && make && ./client_exe

exit $?
