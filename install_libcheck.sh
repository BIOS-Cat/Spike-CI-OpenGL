#!/bin/bash

echo 'Installing libcheck v. 0.11.0'

set -x

curl -sL https://github.com/libcheck/check/releases/download/0.11.0/check-0.11.0.tar.gz | tar xz
pushd check-0.11.0
autoreconf --install
./configure
make
make install
popd
