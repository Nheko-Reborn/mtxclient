#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

#include "utils.hpp"

namespace mtx {
namespace client {

//! Type of the unique request id.
using RequestID = std::string;

//! Type of the callback function on success.
using SuccessCallback =
  std::function<void(RequestID request_id,
                     const boost::beast::http::response<boost::beast::http::string_body> &response,
                     const boost::system::error_code &err)>;

//! Type of the callback function on failure.
using FailureCallback =
  std::function<void(RequestID request_id, const boost::system::error_code ec)>;

//! Represents a context of a single request.
struct Session
{
        Session(boost::asio::io_service &ios,
                boost::asio::ssl::context &ssl_ctx,
                const std::string &host,
                RequestID id,
                SuccessCallback on_success,
                FailureCallback on_failure)
          : socket{ios, ssl_ctx}
          , host{host}
          , id{id}
          , on_success{on_success}
          , on_failure{on_failure}
        {
                parser.header_limit(8192);
                parser.body_limit(1 * 1024 * 1024 * 1024); // 1 GiB
        }

        //! Socket used for communication.
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket;
        //! Remote host.
        std::string host;
        //! Buffer where the response will be stored.
        boost::beast::flat_buffer output_buf;
        //! Parser that will the response data.
        boost::beast::http::response_parser<boost::beast::http::string_body> parser;
        //! Request string.
        boost::beast::http::request<boost::beast::http::string_body> request;
        //! Contains the description of an error if one occurs
        //! during the request life cycle.
        boost::system::error_code error_code;
        //! Unique ID assigned to the request.
        RequestID id;
        //! Function to be called when the request succeeds.
        SuccessCallback on_success;
        //! Function to be called when the request fails.
        FailureCallback on_failure;
};

template<class Request, boost::beast::http::verb HttpVerb>
void
setup_headers(mtx::client::Session *session,
              const Request &req,
              const std::string &endpoint,
              const std::string &content_type = "")
{
        session->request.set(boost::beast::http::field::user_agent, "mtxclient v0.1.0");
        session->request.set(boost::beast::http::field::accept_encoding, "gzip,deflate");
        session->request.set(boost::beast::http::field::host, session->host);

        session->request.method(HttpVerb);
        session->request.target("/_matrix" + endpoint);
        session->request.body() = utils::serialize(req);
        session->request.prepare_payload();

        if (!content_type.empty())
                session->request.set(boost::beast::http::field::content_type, content_type);
}
}
}
