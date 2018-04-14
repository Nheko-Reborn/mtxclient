include(ExternalProject)

#
# Build & install olm.
#

set(THIRD_PARTY_ROOT ${CMAKE_SOURCE_DIR}/.third-party)
set(OLM_ROOT ${THIRD_PARTY_ROOT}/olm)

if(MSVC)
    set(MAKE_CMD "mingw32-make.exe")
else()
    set(MAKE_CMD "make")
endif()

ExternalProject_Add(
  Olm

  GIT_REPOSITORY https://git.matrix.org/git/olm.git
  GIT_TAG 4065c8e11a33ba41133a086ed3de4da94dcb6bae

  BUILD_IN_SOURCE 1
  SOURCE_DIR ${OLM_ROOT}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ${MAKE_CMD} static
  INSTALL_COMMAND ""
)

include_directories(SYSTEM ${OLM_ROOT}/include ${OLM_ROOT}/include/olm)
link_directories(${OLM_ROOT}/build)
