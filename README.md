mtxclient
---
[![Build Status](https://travis-ci.org/Nheko-Reborn/mtxclient.svg?branch=master)](https://travis-ci.org/Nheko-Reborn/mtxclient)
[![Build status](https://ci.appveyor.com/api/projects/status/hyp1n9pq3wtv8dqu/branch/master?svg=true)](https://ci.appveyor.com/project/redsky17/mtxclient/branch/master)
[![codecov](https://codecov.io/gh/Nheko-Reborn/mtxclient/branch/master/graph/badge.svg)](https://codecov.io/gh/Nheko-Reborn/mtxclient)
[![unstable](http://badges.github.io/stability-badges/dist/unstable.svg)](http://github.com/badges/stability-badges)
[![documentation](https://img.shields.io/badge/documentation-doxygen-informational)](https://nheko-reborn.pages.nheko.im/mtxclient/index.html)

Client API library for the Matrix protocol.

## Build instructions

### Dependencies

- Coeurl (A C++ wrapper around curl.)
- OpenSSL
- C++ 20 compiler
- CMake 3.15 or greater (lower versions can work, but not all build system options may work.)
- Google Test (for testing)
- spdlog (for logging)

If you are missing some or all of those above dependencies, you can add `-DHUNTER_ENABLED=ON` to the cmake configure command to use bundled dependencies. You can finetune them with the following variables. They default to ON, if Hunter is enabled and to OFF otherwise.

| cmake flag          | description |
|---------------------|-------------|
| USE_BUNDLED_COEURL  | Use the bundled version of coeurl. |
| USE_BUNDLED_LIBEVENT| Use the bundled version of libevent (coeurl dependency). |
| USE_BUNDLED_LIBCURL | Use the bundled version of curl (coeurl dependency). |
| USE_BUNDLED_COEURL  | Use the bundled version of coeurl. |
| USE_BUNDLED_SPDLOG  | Use the bundled version of spdlog. |
| USE_BUNDLED_OLM     | Use the bundled version of libolm. |
| USE_BUNDLED_GTEST   | Use the bundled version of Google Test. |
| USE_BUNDLED_JSON    | Use the bundled version of nlohmann json. |
| USE_BUNDLED_OPENSSL | Use the bundled version of OpenSSL. |
| USE_BUNDLED_RE2 | Use the bundled version of re2. |

Below is an example which will build the library along with the tests & examples.

#### Linux 

```bash
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

#### macOS

You will need to pass as argument (`-DOPENSSL_ROOT_DIR`) the installation root of openssl. 

```bash
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl
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
