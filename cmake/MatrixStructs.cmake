include(ExternalProject)

#
# Build matrix-structs.
#

set(THIRD_PARTY_ROOT ${CMAKE_SOURCE_DIR}/.third-party)
set(MATRIX_STRUCTS_ROOT ${THIRD_PARTY_ROOT}/matrix_structs)

set(MATRIX_STRUCTS_INCLUDE_DIRS ${MATRIX_STRUCTS_ROOT}/deps)

ExternalProject_Add(
  MatrixStructs

  GIT_REPOSITORY https://github.com/mujx/matrix-structs
  GIT_TAG 0659554443a98bd018691337c8cfc11bfe909007

  BUILD_IN_SOURCE 1
  SOURCE_DIR ${MATRIX_STRUCTS_ROOT}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${MATRIX_STRUCTS_ROOT}
  BUILD_COMMAND ${CMAKE_COMMAND} --build ${MATRIX_STRUCTS_ROOT}
  INSTALL_COMMAND ""
)

include_directories(SYSTEM ${MATRIX_STRUCTS_ROOT}/deps)
include_directories(SYSTEM ${MATRIX_STRUCTS_ROOT}/include)
link_directories(${MATRIX_STRUCTS_ROOT})
