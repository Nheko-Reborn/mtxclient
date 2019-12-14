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

    # Build dependencies.
    cmake -GNinja -Hdeps -B.deps -DCMAKE_BUILD_TYPE=Debug
    cmake --build .deps

    # Build the library.
    cmake -GNinja -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_LIB_TESTS=ON \
        -DBUILD_SHARED_LIBS=ON \
	-DCMAKE_INSTALL_PREFIX=.deps/usr \
        -DCOVERAGE=${COVERAGE} || true
    cmake --build build

    # The tests will run anyway during coverage.
    make test
fi

if [ $TRAVIS_OS_NAME == osx ]; then
    # Build dependencies.
    cmake -Hdeps -B.deps -DCMAKE_BUILD_TYPE=Release \
        -DUSE_BUNDLED_BOOST=OFF \
        -DUSE_BUNDLED_GTEST=OFF \
        -DUSE_BUNDLED_JSON=OFF
    cmake --build .deps

    # Build the library.
    cmake -H. -Bbuild -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
        -DBUILD_LIB_TESTS=OFF \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_INSTALL_PREFIX=.deps/usr || true
    cmake --build build

    make lint
fi
