#!/bin/bash

set -ex

if [ $TRAVIS_OS_NAME == osx ]; then
    brew update
    brew upgrade boost cmake
    brew install libsodium
fi

if [ $TRAVIS_OS_NAME == linux ]; then
    export CXX=${CXX_VERSION}
    export CC=${CC_VERSION}

    sudo add-apt-repository -y ppa:chris-lea/libsodium
    sudo apt-get update -qq
    sudo apt-get install -qq -y libsodium-dev
    sudo apt-get remove -y cmake

    wget https://cmake.org/files/v3.11/cmake-3.11.4-Linux-x86_64.sh
    sudo sh cmake-3.11.4-Linux-x86_64.sh  --skip-license  --prefix=/usr/local
    export PATH="/usr/local/bin:$PATH"
fi
