#!/bin/bash

if [ $TRAVIS_OS_NAME == linux ]; then
    sudo update-alternatives --remove-all gcc
    sudo update-alternatives --remove-all g++

    export CXX=${CXX_VERSION}
    export CC=${CC_VERSION}

    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/${CC_VERSION} 10
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/${CXX_VERSION} 10

    sudo update-alternatives --set gcc "/usr/bin/${CC_VERSION}"
    sudo update-alternatives --set g++ "/usr/bin/${CXX_VERSION}"
fi

# Build dependencies.
cmake -Hdeps -B.deps -DCMAKE_BUILD_TYPE=Release \
    -DUSE_BUNDLED_BOOST=${USE_BUNDLED_BOOST} \
    -DUSE_BUNDLED_GTEST=${TESTS}
cmake --build .deps

# Build the library.
cmake -H. -Bbuild -DOPENSSL_ROOT_DIR=${OPENSLL_ROOT_DIR} \
    -DBUILD_LIB_TESTS=${TESTS} \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_INSTALL_PREFIX=.deps/usr \
    -DCOVERAGE=${COVERAGE} || true
cmake --build build

# Unit & Integration tests
if [ $TESTS == ON ]; then
    make synapse
    make test
fi

# Linting
if [ $LINT == ON ]; then
    make lint
fi
