include(ExternalProject)

#
# Download & install Boost from source.
#

set(THIRD_PARTY_ROOT ${CMAKE_SOURCE_DIR}/.third-party)
set(BUNDLED_BOOST_ROOT ${THIRD_PARTY_ROOT}/boost_1_66_0)

ExternalProject_Add(
  Boost

  URL https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.bz2
  URL_HASH SHA256=5721818253e6a0989583192f96782c4a98eb6204965316df9f5ad75819225ca9
  DOWNLOAD_DIR ${THIRD_PARTY_ROOT}/downloads
  DOWNLOAD_NO_PROGRESS 0

  BUILD_IN_SOURCE 1
  SOURCE_DIR ${BUNDLED_BOOST_ROOT}
  CONFIGURE_COMMAND ${BUNDLED_BOOST_ROOT}/bootstrap.sh
    --with-libraries=random,thread,system
    --prefix=${BUNDLED_BOOST_ROOT}
  BUILD_COMMAND ${BUNDLED_BOOST_ROOT}/b2 -d0 variant=release link=static threading=multi
  INSTALL_COMMAND ${BUNDLED_BOOST_ROOT}/b2 -d0 install
)

set(Boost_INCLUDE_DIRS ${BUNDLED_BOOST_ROOT})
set(Boost_LIBRARIES boost_random boost_system boost_thread)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_INCLUDE_DIRS}/stage/lib)
