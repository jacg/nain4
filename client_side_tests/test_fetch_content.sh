#!/bin/bash

test_dir=$(dirname "$(readlink -f "$0")")
tmp_dir=$(mktemp -d -t nain4-fetch-content-XXXXXX)

cd $tmp_dir
cmake $test_dir/client_fetch_content && make && ./client_exe

exit $?
