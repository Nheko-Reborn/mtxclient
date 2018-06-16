cmake_minimum_required(VERSION 3.1)

project(matrix_client VERSION 0.1.0 LANGUAGES CXX C)

option(ASAN "Compile with address sanitizers" OFF)
option(BUILD_LIB_TESTS "Build tests" ON)
option(BUILD_LIB_EXAMPLES "Build examples" ON)
option(COVERAGE "Calculate test coverage" OFF)
option(IWYU "Check headers with include-what-you-use" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
        -Wall \
        -Wextra \
        -Werror \
        -pipe \
        -pedantic \
        -fsized-deallocation \
        -fdiagnostics-color=always \
        -Wunreachable-code"
    )
endif()

if(ASAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
endif()

if(COVERAGE)
  include(CodeCoverage)
  add_custom_target(ctest COMMAND ${CMAKE_CTEST_COMMAND})
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
  setup_target_for_coverage(${PROJECT_NAME}_coverage ctest coverage)
endif()

if(NOT MSVC AND NOT APPLE)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
endif()

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(MatrixStructs REQUIRED)
find_package(spdlog 0.16.0 CONFIG REQUIRED)
find_package(Olm REQUIRED)
find_package(sodium REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.66 REQUIRED COMPONENTS system random thread iostreams)

add_library(matrix_client
            lib/http/client.cpp
            lib/http/session.cpp
            lib/crypto/client.cpp
            lib/utils.cpp)
target_include_directories(matrix_client
                           SYSTEM
                           PUBLIC
                           ${OPENSSL_INCLUDE_DIR}
                           ${Boost_INCLUDE_DIRS}
                           ${sodium_INCLUDE_DIR})
target_include_directories(
  matrix_client
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(matrix_client
                      PUBLIC
                      MatrixStructs::MatrixStructs
                      ${Boost_LIBRARIES}
                      ${sodium_LIBRARY_RELEASE}
                      OpenSSL::Crypto
                      OpenSSL::SSL
                      Olm::Olm
                      PRIVATE
                      ZLIB::ZLIB)

if(NOT MSVC AND NOT APPLE)
  target_link_libraries(matrix_client PRIVATE Threads::Threads)
endif()

if(IWYU)
  find_program(iwyu_path NAMES include-what-you-use iwyu)
  if(iwyu_path)
    set_property(TARGET matrix_client
                 PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
  else()
    message(WARNING "Could not find the program include-what-you-use")
  endif()
endif()

if(BUILD_LIB_TESTS)
  enable_testing()

  find_package(GTest REQUIRED)

  file(COPY fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

  add_executable(client_api tests/client_api.cpp)
  target_link_libraries(client_api matrix_client GTest::GTest GTest::Main)
  target_include_directories(client_api PRIVATE
                             ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_executable(media_api tests/media_api.cpp)
  target_link_libraries(media_api matrix_client GTest::GTest GTest::Main)
  target_include_directories(media_api PRIVATE
                             ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_executable(e2ee tests/e2ee.cpp)
  target_link_libraries(e2ee matrix_client GTest::GTest GTest::Main)
  target_include_directories(e2ee PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_executable(utils tests/utils.cpp)
  target_link_libraries(utils matrix_client GTest::GTest GTest::Main)
  target_include_directories(utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_executable(connection tests/connection.cpp)
  target_link_libraries(connection matrix_client GTest::GTest GTest::Main)
  target_include_directories(connection PRIVATE
                             ${CMAKE_CURRENT_SOURCE_DIR}/tests)

  add_test(BasicConnectivity connection)
  add_test(ClientAPI client_api)
  add_test(MediaAPI media_api)
  add_test(Encryption e2ee)
  add_test(Utilities utils)
endif()

if(BUILD_LIB_EXAMPLES)
  include_directories(tests)

  add_executable(room_feed examples/room_feed.cpp)
  target_link_libraries(room_feed matrix_client)

  add_executable(simple_bot examples/simple_bot.cpp)
  target_link_libraries(simple_bot matrix_client)

  add_executable(crypto_bot examples/crypto_bot.cpp)
  target_link_libraries(crypto_bot matrix_client Olm::Olm)
endif()

#
# Installation & Target configuration.
#
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/MatrixClient)

install(TARGETS matrix_client
        EXPORT matrix_client-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

set_target_properties(matrix_client PROPERTIES EXPORT_NAME MatrixClient)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT matrix_client-targets
               FILE
               MatrixClientTargets.cmake
               NAMESPACE
               MatrixClient::
               DESTINATION
               ${INSTALL_CONFIGDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientConfigVersion.cmake
  VERSION
  ${PROJECT_VERSION}
  COMPATIBILITY
  AnyNewerVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/MatrixClientConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientConfig.cmake
  INSTALL_DESTINATION
  ${INSTALL_CONFIGDIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientConfigVersion.cmake
              DESTINATION
              ${INSTALL_CONFIGDIR})

export(EXPORT
       matrix_client-targets
       FILE
       ${CMAKE_CURRENT_BINARY_DIR}/MatrixClientTargets.cmake
       NAMESPACE
       MatrixClient::)
export(PACKAGE MatrixClient)
