cmake_minimum_required(VERSION 3.1)

project(matrix_client CXX C)

option(ASAN "Compile with address sanitizers" OFF)
option(BUILD_LIB_TESTS "Build tests" ON)
option(BUILD_LIB_EXAMPLES "Build examples" ON)
option(COVERAGE "Calculate test coverage" OFF)
option(IWYU "Check headers with include-what-you-use" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always -fsized-deallocation" )
    endif()
endif()

if(ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
endif()

if(NOT APPLE AND NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

if(COVERAGE)
    include(CodeCoverage)
    add_custom_target(ctest COMMAND ${CMAKE_CTEST_COMMAND})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    setup_target_for_coverage(${PROJECT_NAME}_coverage ctest coverage)
endif()

# Dependencies on ExternalProjects
set(MTXCLIENT_DEPS "")

# Libraries linked to matrix_client
set(MTXCLIENT_LIBS "")

#
# OpenSSL
#
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
set(MTXCLIENT_LIBS ${MTXCLIENT_LIBS} ${OPENSSL_LIBRARIES})

#
# libsodium
#
include(Findsodium)
include_directories(${sodium_INCLUDE_DIR})
set(MTXCLIENT_LIBS ${MTXCLIENT_LIBS} ${sodium_LIBRARY_RELEASE})

#
# Boost 1.66
#
# If we can't find an already installed version we will
# download it and build it from source.
#
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.66)
find_package(Boost 1.66 COMPONENTS system random thread iostreams)

if(NOT Boost_FOUND)
    include(Boost)
    set(MTXCLIENT_DEPS ${MTXCLIENT_DEPS} Boost)
endif()
include_directories(${Boost_INCLUDE_DIRS})
set(MTXCLIENT_LIBS ${MTXCLIENT_LIBS} ${Boost_LIBRARIES})

#
# zlib
#
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    set(MTXCLIENT_LIBS ${MTXCLIENT_LIBS} ${ZLIB_LIBRARIES})
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
include_directories(${MATRIX_STRUCTS_INCLUDE_DIR})
set(MTXCLIENT_LIBS ${MTXCLIENT_LIBS} ${MATRIX_STRUCTS_LIBRARY})


#
# libolm
#
include(Olm)
set(MTXCLIENT_DEPS ${MTXCLIENT_DEPS} Olm)
set(MTXCLIENT_LIBS ${MTXCLIENT_LIBS} olm)

include_directories(src)
set(SRC 
    src/client.cpp
    src/utils.cpp
    src/crypto.cpp)

add_library(matrix_client ${SRC})
add_dependencies(matrix_client ${MTXCLIENT_DEPS})
target_link_libraries(matrix_client ${MTXCLIENT_LIBS})
target_include_directories(matrix_client SYSTEM PRIVATE $ENV{INCLUDE})

if (IWYU)
    find_program(iwyu_path NAMES include-what-you-use iwyu)
    if (iwyu_path)
        set_property(TARGET matrix_client PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
    else()
        message(WARNING "Could not find the program include-what-you-use")
    endif()
endif()

if (BUILD_LIB_EXAMPLES)
    add_executable(room_feed examples/room_feed.cpp)
    target_link_libraries(room_feed matrix_client ${MATRIX_STRUCTS_LIBRARY})
endif()

if (BUILD_LIB_TESTS)
    enable_testing()

    find_package(GTest)

    file(COPY fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    if (NOT GTest_FOUND)
        include(GoogleTest)
    endif()

    include_directories(${GTEST_INCLUDE_DIRS})
    include_directories(tests)

    add_executable(client_api tests/client_api.cpp)
    target_link_libraries(client_api matrix_client ${GTEST_BOTH_LIBRARIES})

    add_executable(media_api tests/media_api.cpp)
    target_link_libraries(media_api matrix_client ${GTEST_BOTH_LIBRARIES})

    #add_executable(cache tests/cache.cpp)
    #target_link_libraries(cache matrix_client ${GTEST_BOTH_LIBRARIES})

    add_executable(e2ee tests/e2ee.cpp)
    target_link_libraries(e2ee matrix_client ${GTEST_BOTH_LIBRARIES})

    add_executable(utils tests/utils.cpp)
    target_link_libraries(utils matrix_client ${GTEST_BOTH_LIBRARIES})

    add_executable(connection tests/connection.cpp)
    target_link_libraries(connection matrix_client ${GTEST_BOTH_LIBRARIES})

    if (NOT GTest_FOUND)
        add_dependencies(client_api GTest)
        add_dependencies(connection GTest)
        add_dependencies(media_api GTest)
        add_dependencies(e2ee GTest)
        add_dependencies(utils GTest)
    endif()

    add_test(BasicConnectivity connection)
    add_test(ClientAPI client_api)
    add_test(MediaAPI media_api)
    add_test(Encryption e2ee)
    add_test(Utilities utils)
endif()
