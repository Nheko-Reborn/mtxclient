if(WIN32)
  link_directories(${DEPS_PREFIX}/../build/olm/build)
  include_directories(SYSTEM ${DEPS_PREFIX}/../build/olm/include)
  return()
endif()

find_path(OLM_INCLUDE_DIR NAMES olm/olm.h HINTS ${DEPS_PREFIX})
find_library(OLM_LIBRARY NAMES olm HINTS ${DEPS_PREFIX})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OLM DEFAULT_MSG OLM_INCLUDE_DIR OLM_LIBRARY)
