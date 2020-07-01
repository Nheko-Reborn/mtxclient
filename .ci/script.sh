#!/bin/bash

set -ex

mkdir -p .deps

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
	-DHUNTER_ROOT=.deps \
        -DCOVERAGE=${COVERAGE} || true
	#-DHUNTER_CONFIGURATION_TYPES=Debug \ << needs gtest release for some reason
    cmake --build build

    # The tests will run anyway during coverage.
    make test
fi

if [ $TRAVIS_OS_NAME == osx ]; then
    # Build the library.
    cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_LIB_TESTS=OFF \
        -DBUILD_LIB_EXAMPLES=OFF \
        -DBUILD_SHARED_LIBS=ON \
	-DHUNTER_ENABLED=ON \
	-DHUNTER_CONFIGURATION_TYPES=Release \
	-DHUNTER_ROOT=.deps \
        -DUSE_BUNDLED_GTEST=OFF || true
    cmake --build build

    make lint
fi
