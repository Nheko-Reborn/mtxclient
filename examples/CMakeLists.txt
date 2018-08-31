find_package(spdlog 1.0.0 CONFIG REQUIRED)

include_directories(../tests)

add_executable(room_feed room_feed.cpp)
target_link_libraries(room_feed MatrixClient::MatrixClient)

add_executable(simple_bot simple_bot.cpp)
target_link_libraries(simple_bot MatrixClient::MatrixClient)

add_executable(crypto_bot crypto_bot.cpp)
target_link_libraries(crypto_bot
                      spdlog::spdlog
                      MatrixClient::MatrixClient
                      Olm::Olm)
