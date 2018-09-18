#!/bin/bash

set -ex

if [ $TRAVIS_OS_NAME == osx ]; then
    brew update || true
    brew upgrade boost
    brew install libsodium
fi
