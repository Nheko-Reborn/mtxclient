#!/bin/bash

set -ex

CMAKE_VERSION=3.15.5
CMAKE_SHORT_VERSION=3.15

if [ $TRAVIS_OS_NAME == linux ]; then
    sudo apt-get update -qq
    sudo apt-get install -y --no-install-recommends apt-transport-https software-properties-common curl ninja-build
    # cmake
    curl https://cmake.org/files/v${CMAKE_SHORT_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh -o cmake-install.sh
    sudo bash cmake-install.sh --skip-license --prefix=/usr/local
    export PATH="/usr/local/bin:$PATH"

    # Toolchains
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    curl -L https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    sudo apt-add-repository "deb https://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main"
    sudo apt-get update -qq
    sudo apt-get install -y --no-install-recommends \
        pkg-config \
        make \
        clang++-6.0 \
        clang-6.0 \
        g++-8 \
        unzip git \
        libssl-dev \
        openssl

    mkdir -p build-libsodium
    ( cd build-libsodium
      curl -L https://download.libsodium.org/libsodium/releases/libsodium-1.0.17.tar.gz -o libsodium-1.0.17.tar.gz
      tar xfz libsodium-1.0.17.tar.gz
      cd libsodium-1.0.17/
      ./configure && make && sudo make install )
    mkdir -p build-lcov
    ( cd build-lcov
      curl -L http://downloads.sourceforge.net/ltp/lcov-1.14.tar.gz -o lcov-1.14.tar.gz
      tar xfz lcov-1.14.tar.gz
      cd lcov-1.14/
      sudo make install )
fi
if [ $TRAVIS_OS_NAME == osx ]; then
    brew update || true
    brew upgrade boost || true
    brew install libsodium clang-format ninja
    brew tap nlohmann/json
    # the nlohmann install seems to make travis angry 
    # because of the number of log messages
    brew install --with-cmake nlohmann_json > /dev/null
fi
