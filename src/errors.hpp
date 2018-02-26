#pragma once

#include "mtx/errors.hpp"
#include <boost/beast.hpp>

namespace mtx {
namespace client {
namespace errors {

//! Compound type that includes matrix & network related errors.
struct ClientError
{
        //! Matrix client api related error.
        mtx::errors::Error matrix_error;
        //! Error code if a network related error occured.
        boost::system::error_code error_code;
        //! Status code of the associated http response.
        boost::beast::http::status status_code;
};
}
}
}
