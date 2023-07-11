#!/bin/bash

test_dir=$(dirname "$(readlink -f "$0")")
tmp_dir=$(mktemp -d -t nain4-subdirectory-XXXXXX)

cp -r $test_dir/client_subdirectory $tmp_dir/
cd $tmp_dir/client_subdirectory

ln -s $test_dir/../ nain4

mkdir build && cd $_
cmake ../ && make && ./client_exe

exit $?
