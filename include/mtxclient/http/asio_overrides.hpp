#include <boost/predef/architecture.h>

// Note(Nico): Boost's epoll reactor just crashes on the first request on arm. Until
// https://github.com/chriskohlhoff/asio/issues/588 is fixed, disable it.
#ifdef BOOST_ARCH_ARM
#define BOOST_ASIO_DISABLE_EPOLL
#endif
