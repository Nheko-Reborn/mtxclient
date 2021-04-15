#!/bin/bash

set -ex

export PATH="/usr/local/bin:$PATH"

mkdir -p build-lcov
( cd build-lcov
  curl -L http://downloads.sourceforge.net/ltp/lcov-1.14.tar.gz -o lcov-1.14.tar.gz
  tar xfz lcov-1.14.tar.gz
  cd lcov-1.14/
  make install )

