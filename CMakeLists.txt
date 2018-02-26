cmake_minimum_required(VERSION 3.1)

project(matrix_client CXX)

option(BUILD_LIB_TESTS "Build tests" ON)
option(BUILD_LIB_EXAMPLES "Build examples" ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/matrix-structs/.git")
    message(WARNING "The git submodules are not available.")
    message(STATUS "Running git submodule update --init --recursive ...")
    execute_process(COMMAND git submodule update --init --recursive)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -std=c++1z \
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

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.66 REQUIRED)
find_package(Boost 1.66 COMPONENTS system random thread)

find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(src)

add_subdirectory(libs/matrix-structs)
include_directories(${matrix_structs_SOURCE_DIR}/include)
include_directories(${matrix_structs_SOURCE_DIR}/deps)

set(SRC src/client.cpp src/utils.cpp)

add_library(matrix_client ${SRC})
target_link_libraries(matrix_client matrix_structs ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

if (BUILD_LIB_EXAMPLES)
    add_executable(room_feed examples/room_feed.cpp)
    target_link_libraries(room_feed matrix_client matrix_structs)
endif()

if (BUILD_LIB_TESTS)
    enable_testing()

    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    add_executable(client_api tests/client_api.cpp)
    target_link_libraries(client_api matrix_client ${GTEST_BOTH_LIBRARIES})

    #add_executable(media_api tests/media_api.cpp)
    #target_link_libraries(media_api matrix_client ${GTEST_BOTH_LIBRARIES})

    #add_executable(sync tests/sync.cpp)
    #target_link_libraries(sync matrix_client ${GTEST_BOTH_LIBRARIES})

    #add_executable(cache tests/cache.cpp)
    #target_link_libraries(cache matrix_client ${GTEST_BOTH_LIBRARIES})

    add_executable(connection tests/connection.cpp)
    target_link_libraries(connection matrix_client ${GTEST_BOTH_LIBRARIES})

    add_test(BasicConnectivity connection)
    add_test(ClientAPI client_api)
endif()
