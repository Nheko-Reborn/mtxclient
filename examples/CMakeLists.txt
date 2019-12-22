find_package(spdlog 1.0.0 CONFIG)
set_package_properties(spdlog PROPERTIES
    DESCRIPTION "Very fast, header only, C++ logging library"
    URL "https://github.com/gabime/spdlog"
    TYPE REQUIRED
)

include_directories(../tests)

add_executable(room_feed room_feed.cpp)
target_link_libraries(room_feed MatrixClient::MatrixClient)

include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("filesystem" HAVE_STD_FILESYSTEM)

if (HAVE_STD_FILESYSTEM)
  add_executable(media_downloader media_downloader.cpp)
  target_link_libraries(media_downloader MatrixClient::MatrixClient)
endif()

add_executable(simple_bot simple_bot.cpp)
target_link_libraries(simple_bot MatrixClient::MatrixClient)

add_executable(crypto_bot crypto_bot.cpp)
target_link_libraries(crypto_bot
                      spdlog::spdlog
                      MatrixClient::MatrixClient
                      Olm::Olm)
