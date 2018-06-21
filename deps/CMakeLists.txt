cmake_minimum_required(VERSION 3.1)
project(MTXCLIENT_DEPS)

# Point CMake at any custom modules we may ship
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(DEPS_INSTALL_DIR "${CMAKE_BINARY_DIR}/usr"
    CACHE PATH "Dependencies install directory.")
set(DEPS_BIN_DIR "${DEPS_INSTALL_DIR}/bin"
    CACHE PATH "Dependencies binary install directory.")
set(DEPS_LIB_DIR "${DEPS_INSTALL_DIR}/lib"
    CACHE PATH "Dependencies library install directory.")
set(DEPS_BUILD_DIR "${CMAKE_BINARY_DIR}/build"
    CACHE PATH "Dependencies build directory.")
set(DEPS_DOWNLOAD_DIR "${DEPS_BUILD_DIR}/downloads"
    CACHE PATH "Dependencies download directory.")

option(USE_BUNDLED "Use bundled dependencies." ON)

option(USE_BUNDLED_BOOST "Use the bundled version of Boost." ${USE_BUNDLED})
option(USE_BUNDLED_SPDLOG "Use the bundled version of spdlog." ${USE_BUNDLED})
option(USE_BUNDLED_OLM "Use the bundled version of libolm." ${USE_BUNDLED})
option(USE_BUNDLED_MATRIX_STRUCTS "Use the bundled version of matrix-structs."
       ${USE_BUNDLED})
option(USE_BUNDLED_GTEST "Use the bundled version of Google Test."
       ${USE_BUNDLED})

option(USE_EXISTING_SRC_DIR
       "Skip download of deps sources in case of existing source directory."
       OFF)

include(ExternalProject)

set(BOOST_URL
    https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.bz2)
set(BOOST_SHA256
    5721818253e6a0989583192f96782c4a98eb6204965316df9f5ad75819225ca9)

set(GTEST_URL https://github.com/google/googletest/archive/release-1.8.0.tar.gz)
set(GTEST_SHA1 e7e646a6204638fe8e87e165292b8dd9cd4c36ed)

set(MATRIX_STRUCTS_URL https://github.com/mujx/matrix-structs)
set(MATRIX_STRUCTS_TAG c24cb9b38312dfa24b33413847e3238600c678cd)

set(OLM_URL https://git.matrix.org/git/olm.git)
set(OLM_TAG 4065c8e11a33ba41133a086ed3de4da94dcb6bae)

set(SPDLOG_URL https://github.com/gabime/spdlog)
set(SPDLOG_TAG 560df2878ad308b27873b3cc5e810635d69cfad6)

if(USE_BUNDLED_BOOST)
  include(Boost)
endif()

if(USE_BUNDLED_SPDLOG)
  include(SpdLog)
endif()

if(USE_BUNDLED_OLM)
  include(Olm)
endif()

if(USE_BUNDLED_MATRIX_STRUCTS)
  include(MatrixStructs)
endif()

if(USE_BUNDLED_GTEST)
  include(GoogleTest)
endif()

if(WIN32)
  if("${TARGET_ARCH}" STREQUAL "X86_64")
    set(TARGET_ARCH x64)
  elseif(TARGET_ARCH STREQUAL "X86")
    set(TARGET_ARCH ia32)
  endif()
endif()

add_custom_target(third-party ALL
                  COMMAND ${CMAKE_COMMAND} -E touch .third-party
                  DEPENDS ${THIRD_PARTY_DEPS})
