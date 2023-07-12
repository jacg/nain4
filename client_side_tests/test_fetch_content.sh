#!/bin/bash

test_dir=$(dirname "$(readlink -f "$0")")
tmp_dir=$(mktemp -d -t nain4-fetch-content-XXXXXX)

cd $tmp_dir
cmake -S $test_dir/client_fetch_content -B build && cmake --build build && ./build/client_exe

exit $?
