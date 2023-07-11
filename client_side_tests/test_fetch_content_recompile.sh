#!/bin/bash

if [[ $(id -u) -ne 0 ]]; then
  echo This program must be run with su privileges!
  exit 1
fi

tmp_dir=$(mktemp -d -t nain4-recompile-XXXXXX)
test_dir=$(dirname "$(readlink -f "$0")")

cd $tmp_dir
cmake $test_dir/client_fetch_content && make && rm exe

result1=$(unshare --net cmake $test_dir/client_fetch_content)
result2=$(unshare --net  make)

result=$result1 && $result2

echo RESULT $result1 $result2 $result

if [[ $result -eq 0 ]]; then
  exit 1;
else
  exit 0;
fi
