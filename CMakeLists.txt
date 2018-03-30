cmake_minimum_required(VERSION 3.1)

project(matrix_client CXX)

option(BUILD_LIB_TESTS "Build tests" ON)
option(BUILD_LIB_EXAMPLES "Build examples" ON)
option(BUILD_OLM "Fetch & build libolm" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -std=c++14 \
        -Wall \
        -Wextra \
        -Werror \
        -pipe \
        -pedantic \
        -Wunreachable-code")

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

        if (GCC_VERSION VERSION_GREATER 4.9)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always" )
        endif()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always" )
    endif()
endif()

if(NOT APPLE AND NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

set(MTXCLIENT_DEPS "")

#
# OpenSSL
#
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

#
# Boost 1.66
#
# If we can't find an already installed version we will
# download it and build it from source.
#
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.66)
find_package(Boost 1.66 COMPONENTS system random thread)

if(NOT Boost_FOUND)
    message(STATUS "Fetching and building Boost 1.66")
    include(Boost)
endif()

#
# zlib
#
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif(ZLIB_FOUND)

#
# matrix-structs
#
find_library(MATRIX_STRUCTS_LIBRARY 
    NAMES matrix_structs 
    PATHS ${MATRIX_STRUCTS_ROOT}
          ${MATRIX_STRUCTS_ROOT}/lib
          ${MATRIX_STRUCTS_ROOT}/lib/static)

if(NOT MATRIX_STRUCTS_LIBRARY)
    include(MatrixStructs)
    set(MTXCLIENT_DEPS ${MTXCLIENT_DEPS} MatrixStructs)
endif()

include_directories(SYSTEM ${MATRIX_STRUCTS_INCLUDE_DIR})

set(MTXCLIENT_LIBS matrix_structs
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${ZLIB_LIBRARIES})

#
# libolm
#
if(BUILD_OLM)
    include(Olm)
    set(MTXCLIENT_DEPS ${MTXCLIENT_DEPS} Olm)
    set(MTXCLIENT_LIBS ${MTXCLIENT_LIBS} olm)
endif()

include_directories(src)
set(SRC 
    src/client.cpp
    src/utils.cpp
    src/crypto.cpp)

add_library(matrix_client ${SRC})
add_dependencies(matrix_client ${MTXCLIENT_DEPS})
target_link_libraries(matrix_client ${MTXCLIENT_LIBS})

if(NOT Boost_FOUND)
    add_dependencies(matrix_client Boost)
endif()

if (BUILD_LIB_EXAMPLES)
    add_executable(room_feed examples/room_feed.cpp)
    target_link_libraries(room_feed matrix_client matrix_structs)
endif()

if (BUILD_LIB_TESTS)
    enable_testing()

    find_package(GTest)

    file(COPY fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    if (NOT GTest_FOUND)
        message(STATUS "Fetching and building GTest 1.8")
        include(GoogleTest)
    endif()

    include_directories(${GTEST_INCLUDE_DIRS})

    add_executable(client_api tests/client_api.cpp)
    target_link_libraries(client_api matrix_client ${GTEST_BOTH_LIBRARIES})

    add_executable(media_api tests/media_api.cpp)
    target_link_libraries(media_api matrix_client ${GTEST_BOTH_LIBRARIES})

    #add_executable(cache tests/cache.cpp)
    #target_link_libraries(cache matrix_client ${GTEST_BOTH_LIBRARIES})

    add_executable(e2ee tests/e2ee.cpp)
    target_link_libraries(e2ee matrix_client ${GTEST_BOTH_LIBRARIES})

    add_executable(connection tests/connection.cpp)
    target_link_libraries(connection matrix_client ${GTEST_BOTH_LIBRARIES})

    if (NOT GTest_FOUND)
        add_dependencies(client_api GTest)
        add_dependencies(connection GTest)
        add_dependencies(media_api GTest)
        add_dependencies(e2ee GTest)
    endif()

    add_test(BasicConnectivity connection)
    add_test(ClientAPI client_api)
    add_test(MediaAPI media_api)
    add_test(Encryption e2ee)
endif()
