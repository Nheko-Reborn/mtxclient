#!/bin/bash

set -ex

if [ $TRAVIS_OS_NAME == osx ]; then
    brew update
    brew upgrade boost cmake
    brew install libsodium
fi
