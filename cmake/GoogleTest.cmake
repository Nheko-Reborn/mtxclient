include(ExternalProject)

#
# Download & install Google Test from source.
#

set(THIRD_PARTY_ROOT ${CMAKE_SOURCE_DIR}/.third-party)
set(BUNDLED_GTEST_ROOT ${THIRD_PARTY_ROOT}/gtest_1_8_0)

ExternalProject_Add(
  GTest

  URL https://github.com/google/googletest/archive/release-1.8.0.tar.gz
  URL_HASH SHA1=e7e646a6204638fe8e87e165292b8dd9cd4c36ed
  DOWNLOAD_DIR ${THIRD_PARTY_ROOT}/downloads
  DOWNLOAD_NO_PROGRESS 0

  BUILD_IN_SOURCE 1
  SOURCE_DIR ${BUNDLED_GTEST_ROOT}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} ${BUNDLED_GTEST_ROOT}
  BUILD_COMMAND make
  INSTALL_COMMAND ""
)

set(GTEST_BOTH_LIBRARIES gtest gtest_main)

include_directories(SYSTEM ${BUNDLED_GTEST_ROOT}/googletest/include)
link_directories(${BUNDLED_GTEST_ROOT}/googlemock/gtest)
