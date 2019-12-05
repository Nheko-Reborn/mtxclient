#!/bin/bash

set -ex

if [ $TRAVIS_OS_NAME == osx ]; then
    brew update || true
    brew upgrade boost || true
    brew install libsodium clang-format ninja
    brew tap nlohmann/json
    # the nlohmann install seems to make travis angry 
    # because of the number of log messages
    brew install --with-cmake nlohmann_json > /dev/null
fi
