include(ExternalProject)

#
# Download spdlog.
#

set(THIRD_PARTY_ROOT ${CMAKE_SOURCE_DIR}/.third-party)
set(SPDLOG_ROOT ${THIRD_PARTY_ROOT}/spdlog)

set(SPDLOG_INCLUDE_DIR ${SPDLOG_ROOT}/include)

ExternalProject_Add(
  SpdLog

  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG b1a58cd342b4d0737a6a45e7c9a2abec143f480a

  BUILD_IN_SOURCE 1
  SOURCE_DIR ${SPDLOG_ROOT}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
