mtxclient
---
[![Build Status](https://travis-ci.org/Nheko-Reborn/mtxclient.svg?branch=master)](https://travis-ci.org/Nheko-Reborn/mtxclient)
[![Build status](https://ci.appveyor.com/api/projects/status/hyp1n9pq3wtv8dqu/branch/master?svg=true)](https://ci.appveyor.com/project/redsky17/mtxclient/branch/master)
[![codecov](https://codecov.io/gh/Nheko-Reborn/mtxclient/branch/master/graph/badge.svg)](https://codecov.io/gh/Nheko-Reborn/mtxclient)
[![experimental](https://img.shields.io/badge/stability-experimental-orange.svg)](http://github.com/badges/stability-badges)

Client API library for the Matrix protocol, built on top of Boost.Asio.

## Build instructions

### Dependencies

- Boost 1.70 (includes Boost.Beast and makes the strand interface usable)
- OpenSSL
- C++ 17 compiler
- CMake 3.15 or greater (lower versions can work, but they tend to mess up linking the right boost libraries)
- Google Test (for testing)
- libsodium 1.0.14 or greater

Boost and GTest will be built automatically by CMake if they're not found on  your system.

Below is an example which will build the library along with the tests & examples.

#### Linux 

```bash
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

#### macOS

You will need to pass as argument (`-DOPENSSL_ROOT_DIR`) the installation root of openssl. 

```bash
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl
cmake --build build
```

You can toggle off the tests & examples by passing `-DBUILD_LIB_TESTS=OFF` &
`-DBUILD_LIB_EXAMPLES=OFF` respectively.

## Running the tests

In order to run the integration tests you'll need a local synapse instance. You
can start an instance with docker by running the following

```bash
make synapse
```
then run the test suite

```bash
make test 
```
