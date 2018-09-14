enable_testing()

file(COPY fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

find_package(GTest)
set_package_properties(GTest PROPERTIES
    DESCRIPTION "Google's framework for writing C++ tests"
    URL "https://github.com/google/googletest"
    TYPE REQUIRED
)

file(COPY fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(client_api client_api.cpp)
target_link_libraries(client_api
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)
target_include_directories(client_api PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_executable(media_api media_api.cpp)
target_link_libraries(media_api
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)
target_include_directories(media_api PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_executable(e2ee e2ee.cpp)
target_link_libraries(e2ee MatrixClient::MatrixClient GTest::GTest GTest::Main)
target_include_directories(e2ee PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_executable(utils utils.cpp)
target_link_libraries(utils MatrixClient::MatrixClient GTest::GTest GTest::Main)
target_include_directories(utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_executable(connection connection.cpp)
target_link_libraries(connection
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)
target_include_directories(connection PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_executable(identifiers identifiers.cpp)
target_link_libraries(identifiers
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)

add_executable(events events.cpp)
target_link_libraries(events
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)

add_executable(messages messages.cpp)
target_link_libraries(messages
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)

add_executable(responses responses.cpp)
target_link_libraries(responses
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)

add_executable(requests requests.cpp)
target_link_libraries(requests
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)

add_executable(errors errors.cpp)
target_link_libraries(errors
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)

add_executable(crypto crypto.cpp)
target_link_libraries(crypto
                      MatrixClient::MatrixClient
                      GTest::GTest
                      GTest::Main)

add_test(BasicConnectivity connection)
add_test(ClientAPI client_api)
add_test(MediaAPI media_api)
add_test(Encryption e2ee)
add_test(Utilities utils)
add_test(Identifiers identifiers)
add_test(Errors errors)
add_test(CryptoStructs crypto)
add_test(StateEvents events)
add_test(RoomEvents messages)
add_test(Responses responses)
add_test(Requests requests)
