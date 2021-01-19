#pragma once

/// @file
/// @brief A single http session, which is the context for a single http request.
///
/// You usually don't need to include this as session handling is handled by the library for you.

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

#include <nlohmann/json.hpp>

#include "mtxclient/http/errors.hpp"
#include "mtxclient/utils.hpp"

namespace mtx {
namespace http {

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
struct Session : public std::enable_shared_from_this<Session>
{
        Session(boost::asio::io_service &ios,
                boost::asio::ssl::context &ssl_ctx,
                const std::string &host,
                uint16_t port,
                RequestID id,
                SuccessCallback on_success,
                FailureCallback on_failure);

        //! DNS resolver.
        boost::asio::ip::tcp::resolver resolver_;
        //! Socket used for communication.
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket;
        //! Remote host.
        std::string host;
        //! Remote port.
        uint16_t port;
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

        void run() noexcept;
        //! Force shutdown all connections. Pending responses will not be processed.
        void terminate();

private:
        void shutdown();
        void on_resolve(boost::system::error_code ec,
                        boost::asio::ip::tcp::resolver::results_type results);
        void on_close(boost::system::error_code ec);
        void on_connect(const boost::system::error_code &ec);
        void on_handshake(const boost::system::error_code &ec);
        void on_read(const boost::system::error_code &ec, std::size_t bytes_transferred);
        void on_request_complete();
        void on_write(const boost::system::error_code &ec, std::size_t bytes_transferred);

        //! Flag to indicate that the connection of this session is closing and no
        //! response should be processed.
        std::atomic_bool is_shutting_down_;
};

template<boost::beast::http::verb HttpVerb>
void
setup_headers(mtx::http::Session *session,
              const nlohmann::json &req,
              const std::string &endpoint,
              const std::string &content_type       = "",
              const std::string &endpoint_namespace = "/_matrix")
{
        session->request.set(boost::beast::http::field::user_agent, "mtxclient v0.4.0");
        session->request.set(boost::beast::http::field::accept_encoding, "gzip,deflate");
        session->request.set(boost::beast::http::field::host, session->host);

        session->request.method(HttpVerb);
        session->request.target(endpoint_namespace + endpoint);
        session->request.body() = req;
        session->request.prepare_payload();

        if (!content_type.empty())
                session->request.set(boost::beast::http::field::content_type, content_type);
}
} // namespace http
} // namespace mtx
