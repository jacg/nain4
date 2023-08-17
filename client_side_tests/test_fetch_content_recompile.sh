#!/bin/bash
set -e

tmp_dir=$(mktemp -d -t nain4-recompile-XXXXXX)
test_dir=$(dirname "$(readlink -f "$0")")

cd $tmp_dir
cmake -S $test_dir/client_fetch_content -B build
cmake --build build
rm -f build/client_exe # Make sure the executable does not exist unless we recompile

# Block network access temporarily
sudo iptables -A OUTPUT -j DROP

cmake -S $test_dir/client_fetch_content -B build; status1=$?
cmake --build build; status2=$?
./build/client_exe; status3=$?

# Restore network access
sudo iptables -D OUTPUT -j DROP

(( status = status1 | status2 | status3 ))
exit $status
