get_filename_component(MatrixClient_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CMakeFindDependencyMacro)

list(APPEND CMAKE_MODULE_PATH ${MatrixClient_CMAKE_DIR})
list(REMOVE_AT CMAKE_MODULE_PATH -1)

if(NOT TARGET MatrixClient::MatrixClient)
  include("${MatrixClient_CMAKE_DIR}/MatrixClientTargets.cmake")
endif()

set(MatrixClient_LIBRARIES MatrixClient::MatrixClient)
