include_directories(../tests)

add_executable(room_feed room_feed.cpp)
target_link_libraries(room_feed MatrixClient::MatrixClient)

include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("filesystem" HAVE_STD_FILESYSTEM)

if (HAVE_STD_FILESYSTEM)
  add_executable(media_downloader media_downloader.cpp)
  target_link_libraries(media_downloader MatrixClient::MatrixClient)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(media_downloader stdc++fs)
  endif()
endif()

add_executable(memberstats memberstats.cpp)
target_link_libraries(memberstats MatrixClient::MatrixClient)

add_executable(simple_bot simple_bot.cpp)
target_link_libraries(simple_bot MatrixClient::MatrixClient)

add_executable(crypto_bot crypto_bot.cpp)
target_link_libraries(crypto_bot
                      spdlog::spdlog
                      MatrixClient::MatrixClient
                      Olm::Olm)

  add_executable(online_backup_exporter online_backup_exporter.cpp)
  target_link_libraries(online_backup_exporter MatrixClient::MatrixClient)
