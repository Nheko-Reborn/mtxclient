#!/bin/bash

set -ex

if [ $TRAVIS_OS_NAME == osx ]; then
    brew update || true
    brew upgrade boost cmake
    brew install libsodium
fi
