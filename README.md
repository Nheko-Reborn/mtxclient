mtxclient
---
[![Build Status](https://travis-ci.org/mujx/mtxclient.svg?branch=master)](https://travis-ci.org/mujx/mtxclient)

Client API library for the Matrix protocol, built on top of Boost.Asio.

## Build instructions

### Dependencies

- Boost 1.66 (includes Boost.Beast)
- OpenSSL
- C++ 11 compiler
- CMake 3.1 or greater
- Google Test (for testing)

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
