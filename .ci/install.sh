#!/bin/bash

set -ex

CMAKE_VERSION=3.15.5
CMAKE_SHORT_VERSION=3.15

if [ $TRAVIS_OS_NAME == linux ]; then
    # cmake
    curl https://cmake.org/files/v${CMAKE_SHORT_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh -o cmake-install.sh
    sudo bash cmake-install.sh --skip-license --prefix=/usr/local
    export PATH="/usr/local/bin:$PATH"

    mkdir -p build-lcov
    ( cd build-lcov
      curl -L http://downloads.sourceforge.net/ltp/lcov-1.14.tar.gz -o lcov-1.14.tar.gz
      tar xfz lcov-1.14.tar.gz
      cd lcov-1.14/
      sudo make install )
fi

