include(ExternalProject)

#
# Download & install Boost from source.
#

set(THIRD_PARTY_ROOT ${CMAKE_SOURCE_DIR}/.third-party)
set(BUNDLED_BOOST_ROOT ${THIRD_PARTY_ROOT}/boost_1_66_0)

ExternalProject_Add(
  Boost

  URL https://sourceforge.net/projects/boost/files/boost/1.66.0/boost_1_66_0.tar.bz2/download
  URL_HASH SHA1=b6b284acde2ad7ed49b44e856955d7b1ea4e9459
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
