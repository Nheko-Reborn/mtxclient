if(WIN32)
  message(STATUS "Building gtest in Windows is not supported (skipping)")
  return()
endif()

ExternalProject_Add(
  GTest

  URL ${GTEST_URL}
  URL_HASH SHA1=${GTEST_SHA1}
  DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}/gtest
  DOWNLOAD_NO_PROGRESS 0

  BUILD_IN_SOURCE 1
  SOURCE_DIR ${DEPS_BUILD_DIR}/gtest
  CONFIGURE_COMMAND ${CMAKE_COMMAND}
        -DCMAKE_INSTALL_PREFIX=${DEPS_INSTALL_DIR}
        -DCMAKE_BUILD_TYPE=Release
        ${DEPS_BUILD_DIR}/gtest
  BUILD_COMMAND ${CMAKE_COMMAND}
        --build ${DEPS_BUILD_DIR}/gtest
        --config Release)

list(APPEND THIRD_PARTY_DEPS GTest)
