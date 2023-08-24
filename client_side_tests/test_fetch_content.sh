#!/bin/bash

test_dir=$(dirname "$(readlink -f "$0")")
tmp_dir=$(mktemp -d -t nain4-fetch-content-XXXXXX)

cmake -S $test_dir/client_fetch_content -B $tmp_dir/build && cmake --build $tmp_dir/build && $tmp_dir/build/client_exe

exit $?
