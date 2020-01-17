#!/bin/bash

set -ex

if [ $TRAVIS_OS_NAME == linux ]; then
    export CXX=${CXX_VERSION}
    export CC=${CC_VERSION}
    export PATH="/usr/local/bin:$PATH"

    cmake --version

    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/${CC_VERSION} 10
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/${CXX_VERSION} 10
    sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-8 10

    sudo update-alternatives --set gcc "/usr/bin/${CC_VERSION}"
    sudo update-alternatives --set g++ "/usr/bin/${CXX_VERSION}"
    sudo update-alternatives --set gcov "/usr/bin/gcov-8"

    # Build the library.
    cmake -GNinja -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_LIB_TESTS=ON \
        -DBUILD_SHARED_LIBS=ON \
	-DHUNTER_ENABLED=ON \
	-DHUNTER_ROOT=/tmp/.deps \
	-DUSE_BUNDLED_OPENSSL=OFF \
        -DCOVERAGE=${COVERAGE} || true
    cmake --build build

    # The tests will run anyway during coverage.
    make test
fi

if [ $TRAVIS_OS_NAME == osx ]; then
    # Build the library.
    export OPENSSL_ROOT_DIR=/usr/local/opt/openssl
    cmake -H. -Bbuild \
	-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
	-DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include \
        -DBUILD_LIB_TESTS=OFF \
        -DBUILD_SHARED_LIBS=ON \
	-DHUNTER_ENABLED=ON \
	-DHUNTER_ROOT=/tmp/.deps \
	-DUSE_BUNDLED_OPENSSL=OFF \
        -DUSE_BUNDLED_GTEST=OFF || true
    cmake --build build

    make lint
fi
