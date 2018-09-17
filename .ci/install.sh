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

    sudo apt-get update -qq
    sudo apt-get remove -y cmake

    # Build & install libsodium for source.
    mkdir -p libsodium && pushd libsodium
    curl -L \
        https://download.libsodium.org/libsodium/releases/libsodium-${LIBSODIUM_VERSION}.tar.gz \
        -o libsodium-${LIBSODIUM_VERSION}.tar.gz
    tar xfz libsodium-${LIBSODIUM_VERSION}.tar.gz
    pushd libsodium-${LIBSODIUM_VERSION}/
    ./configure && make && make check && make install         
    popd
    popd

    wget https://cmake.org/files/v3.11/cmake-3.11.4-Linux-x86_64.sh
    sudo sh cmake-3.11.4-Linux-x86_64.sh  --skip-license  --prefix=/usr/local
    export PATH="/usr/local/bin:$PATH"
fi
