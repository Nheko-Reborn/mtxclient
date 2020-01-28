# Written in 2016 by Henrik Steffen Gaßmann <henrik@gassmann.onl>
#
# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to the
# public domain worldwide. This software is distributed without any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication
# along with this software. If not, see
#
#     http://creativecommons.org/publicdomain/zero/1.0/
#
########################################################################
# Tries to find the local libsodium installation.
#
# On Windows the sodium_DIR environment variable is used as a default
# hint which can be overridden by setting the corresponding cmake variable.
#
# Once done the following variables will be defined:
#
#   libsodium_FOUND
#   libsodium_INCLUDE_DIR
#   libsodium_LIBRARY_DEBUG
#   libsodium_LIBRARY_RELEASE
#
#
# Furthermore an imported "libsodium" target is created.
#

if (CMAKE_C_COMPILER_ID STREQUAL "GNU"
    OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(_GCC_COMPATIBLE 1)
endif()

# static library option
if (NOT DEFINED libsodium_USE_STATIC_LIBS)
    option(libsodium_USE_STATIC_LIBS "enable to statically link against libsodium" OFF)
endif()
if(NOT (libsodium_USE_STATIC_LIBS EQUAL libsodium_USE_STATIC_LIBS_LAST))
    unset(libsodium_LIBRARY CACHE)
    unset(libsodium_LIBRARY_DEBUG CACHE)
    unset(libsodium_LIBRARY_RELEASE CACHE)
    unset(libsodium_DLL_DEBUG CACHE)
    unset(libsodium_DLL_RELEASE CACHE)
    set(libsodium_USE_STATIC_LIBS_LAST ${libsodium_USE_STATIC_LIBS} CACHE INTERNAL "internal change tracking variable")
endif()


########################################################################
# UNIX
if (UNIX)
    # import pkg-config
    find_package(PkgConfig QUIET)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(libsodium_PKG QUIET libsodium)
    endif()

    if(libsodium_USE_STATIC_LIBS)
        foreach(_libname ${libsodium_PKG_STATIC_LIBRARIES})
            if (NOT _libname MATCHES "^lib.*\\.a$") # ignore strings already ending with .a
                list(INSERT libsodium_PKG_STATIC_LIBRARIES 0 "lib${_libname}.a")
            endif()
        endforeach()
        list(REMOVE_DUPLICATES libsodium_PKG_STATIC_LIBRARIES)

        # if pkgconfig for libsodium doesn't provide
        # static lib info, then override PKG_STATIC here..
        if (libsodium_PKG_STATIC_LIBRARIES STREQUAL "")
            set(libsodium_PKG_STATIC_LIBRARIES libsodium.a)
        endif()

        set(XPREFIX libsodium_PKG_STATIC)
    else()
        if (libsodium_PKG_LIBRARIES STREQUAL "")
            set(libsodium_PKG_LIBRARIES sodium)
        endif()

        set(XPREFIX libsodium_PKG)
    endif()

    find_path(libsodium_INCLUDE_DIR sodium.h
        HINTS ${${XPREFIX}_INCLUDE_DIRS}
    )
    find_library(libsodium_LIBRARY_DEBUG NAMES ${${XPREFIX}_LIBRARIES}
        HINTS ${${XPREFIX}_LIBRARY_DIRS}
    )
    find_library(libsodium_LIBRARY_RELEASE NAMES ${${XPREFIX}_LIBRARIES}
        HINTS ${${XPREFIX}_LIBRARY_DIRS}
    )


########################################################################
# Windows
elseif (WIN32)
    set(libsodium_DIR "$ENV{libsodium_DIR}" CACHE FILEPATH "libsodium install directory")
    mark_as_advanced(libsodium_DIR)

    find_path(libsodium_INCLUDE_DIR sodium.h
        HINTS ${libsodium_DIR}
        PATH_SUFFIXES include
    )

    if (MSVC)
        # detect target architecture
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/arch.c" [=[
            #if defined _M_IX86
            #error ARCH_VALUE x86_32
            #elif defined _M_X64
            #error ARCH_VALUE x86_64
            #endif
            #error ARCH_VALUE unknown
        ]=])
        try_compile(_UNUSED_VAR "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/arch.c"
            OUTPUT_VARIABLE _COMPILATION_LOG
        )
        string(REGEX REPLACE ".*ARCH_VALUE ([a-zA-Z0-9_]+).*" "\\1" _TARGET_ARCH "${_COMPILATION_LOG}")

        # construct library path
        if (_TARGET_ARCH STREQUAL "x86_32")
            string(APPEND _PLATFORM_PATH "Win32")
        elseif(_TARGET_ARCH STREQUAL "x86_64")
            string(APPEND _PLATFORM_PATH "x64")
        else()
            message(FATAL_ERROR "the ${_TARGET_ARCH} architecture is not supported by Findsodium.cmake.")
        endif()
        string(APPEND _PLATFORM_PATH "/$$CONFIG$$")

        if (MSVC_VERSION LESS 1900)
            math(EXPR _VS_VERSION "${MSVC_VERSION} / 10 - 60")
        else()
            math(EXPR _VS_VERSION "${MSVC_VERSION} / 10 - 50")
        endif()
        string(APPEND _PLATFORM_PATH "/v${_VS_VERSION}")

        if (libsodium_USE_STATIC_LIBS)
            string(APPEND _PLATFORM_PATH "/static")
        else()
            string(APPEND _PLATFORM_PATH "/dynamic")
        endif()

        string(REPLACE "$$CONFIG$$" "Debug" _DEBUG_PATH_SUFFIX "${_PLATFORM_PATH}")
        string(REPLACE "$$CONFIG$$" "Release" _RELEASE_PATH_SUFFIX "${_PLATFORM_PATH}")

        find_library(libsodium_LIBRARY_DEBUG libsodium.lib
            HINTS ${libsodium_DIR}
            PATH_SUFFIXES ${_DEBUG_PATH_SUFFIX}
        )
        find_library(libsodium_LIBRARY_RELEASE libsodium.lib
            HINTS ${libsodium_DIR}
            PATH_SUFFIXES ${_RELEASE_PATH_SUFFIX}
        )
        if (NOT libsodium_USE_STATIC_LIBS)
            set(CMAKE_FIND_LIBRARY_SUFFIXES_BCK ${CMAKE_FIND_LIBRARY_SUFFIXES})
            set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
            find_library(libsodium_DLL_DEBUG libsodium
                HINTS ${libsodium_DIR}
                PATH_SUFFIXES ${_DEBUG_PATH_SUFFIX}
            )
            find_library(libsodium_DLL_RELEASE libsodium
                HINTS ${libsodium_DIR}
                PATH_SUFFIXES ${_RELEASE_PATH_SUFFIX}
            )
            set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_BCK})
        endif()

    elseif(_GCC_COMPATIBLE)
        if (libsodium_USE_STATIC_LIBS)
            find_library(libsodium_LIBRARY_DEBUG libsodium.a
                HINTS ${libsodium_DIR}
                PATH_SUFFIXES lib
            )
            find_library(libsodium_LIBRARY_RELEASE libsodium.a
                HINTS ${libsodium_DIR}
                PATH_SUFFIXES lib
            )
        else()
            find_library(libsodium_LIBRARY_DEBUG libsodium.dll.a
                HINTS ${libsodium_DIR}
                PATH_SUFFIXES lib
            )
            find_library(libsodium_LIBRARY_RELEASE libsodium.dll.a
                HINTS ${libsodium_DIR}
                PATH_SUFFIXES lib
            )

            file(GLOB _DLL
                LIST_DIRECTORIES false
                RELATIVE "${libsodium_DIR}/bin"
                "${libsodium_DIR}/bin/libsodium*.dll"
            )
            find_library(libsodium_DLL_DEBUG ${_DLL} libsodium
                HINTS ${libsodium_DIR}
                PATH_SUFFIXES bin
            )
            find_library(libsodium_DLL_RELEASE ${_DLL} libsodium
                HINTS ${libsodium_DIR}
                PATH_SUFFIXES bin
            )
        endif()
    else()
        message(FATAL_ERROR "this platform is not supported by FindSodium.cmake")
    endif()


########################################################################
# unsupported
else()
    message(FATAL_ERROR "this platform is not supported by FindSodium.cmake")
endif()


########################################################################
# common stuff

# extract libsodium version
if (libsodium_INCLUDE_DIR)
    set(_VERSION_HEADER "${libsodium_INCLUDE_DIR}/sodium/version.h")
    if (EXISTS "${_VERSION_HEADER}")
        file(READ "${_VERSION_HEADER}" _VERSION_HEADER_CONTENT)
        string(REGEX REPLACE ".*#[ \t]*define[ \t]*SODIUM_VERSION_STRING[ \t]*\"([^\n]*)\".*" "\\1"
            libsodium_VERSION "${_VERSION_HEADER_CONTENT}")
        set(libsodium_VERSION "${libsodium_VERSION}")
    endif()
endif()

# communicate results
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sodium
    REQUIRED_VARS
        libsodium_LIBRARY_RELEASE
        libsodium_LIBRARY_DEBUG
        libsodium_INCLUDE_DIR
    VERSION_VAR
        libsodium_VERSION
)

# mark file paths as advanced
mark_as_advanced(libsodium_INCLUDE_DIR)
mark_as_advanced(libsodium_LIBRARY_DEBUG)
mark_as_advanced(libsodium_LIBRARY_RELEASE)
if (WIN32)
    mark_as_advanced(libsodium_DLL_DEBUG)
    mark_as_advanced(libsodium_DLL_RELEASE)
endif()

# create imported target
if(libsodium_USE_STATIC_LIBS)
    set(_LIB_TYPE STATIC)
else()
    set(_LIB_TYPE SHARED)
endif()
add_library(libsodium ${_LIB_TYPE} IMPORTED GLOBAL)

set_target_properties(libsodium PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${libsodium_INCLUDE_DIR}"
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
)
add_library(libsodium::libsodium ALIAS libsodium)

if (libsodium_USE_STATIC_LIBS)
    set_target_properties(libsodium PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS "SODIUM_STATIC"
        IMPORTED_LOCATION "${libsodium_LIBRARY_RELEASE}"
        IMPORTED_LOCATION_DEBUG "${libsodium_LIBRARY_DEBUG}"
    )
else()
    if (UNIX)
        set_target_properties(libsodium PROPERTIES
            IMPORTED_LOCATION "${libsodium_LIBRARY_RELEASE}"
            IMPORTED_LOCATION_DEBUG "${libsodium_LIBRARY_DEBUG}"
        )
    elseif (WIN32)
        set_target_properties(libsodium PROPERTIES
            IMPORTED_IMPLIB "${libsodium_LIBRARY_RELEASE}"
            IMPORTED_IMPLIB_DEBUG "${libsodium_LIBRARY_DEBUG}"
        )
        if (NOT (libsodium_DLL_DEBUG MATCHES ".*-NOTFOUND"))
            set_target_properties(libsodium PROPERTIES
                IMPORTED_LOCATION_DEBUG "${libsodium_DLL_DEBUG}"
            )
        endif()
        if (NOT (libsodium_DLL_RELEASE MATCHES ".*-NOTFOUND"))
            set_target_properties(libsodium PROPERTIES
                IMPORTED_LOCATION_RELWITHDEBINFO "${libsodium_DLL_RELEASE}"
                IMPORTED_LOCATION_MINSIZEREL "${libsodium_DLL_RELEASE}"
                IMPORTED_LOCATION_RELEASE "${libsodium_DLL_RELEASE}"
            )
        endif()
    endif()
endif()
