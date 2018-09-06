#include "mtxclient/http/session.hpp"

using namespace mtx::http;

Session::Session(boost::asio::io_service &ios,
                 boost::asio::ssl::context &ssl_ctx,
                 const std::string &host,
                 uint16_t port,
                 RequestID id,
                 SuccessCallback on_success,
                 FailureCallback on_failure)
  : resolver_(ios)
  , socket(ios, ssl_ctx)
  , host(std::move(host))
  , port{port}
  , id(std::move(id))
  , on_success(std::move(on_success))
  , on_failure(std::move(on_failure))
  , is_shutting_down_(false)
{
        parser.header_limit(8192);
        parser.body_limit(1 * 1024 * 1024 * 1024); // 1 GiB
}

void
Session::on_resolve(boost::system::error_code ec,
                    boost::asio::ip::tcp::resolver::results_type results)
{
        if (ec) {
                on_failure(id, ec);
                return;
        }

        boost::asio::async_connect(
          socket.next_layer(),
          results,
          std::bind(&Session::on_connect, shared_from_this(), std::placeholders::_1));
}

void
Session::on_close(boost::system::error_code ec)
{
        if (ec == boost::asio::error::eof) {
                // Rationale:
                // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
                ec.assign(0, ec.category());
        }

// SSL_R_SHORT_READ is removed in openssl-1.1
#if defined SSL_R_SHORT_READ
        if (ERR_GET_REASON(ec.value()) == SSL_R_SHORT_READ)
                return;
#else
        if (ERR_GET_REASON(ec.value()) == boost::asio::ssl::error::stream_truncated)
                return;
#endif

        if (ec)
                // TODO: propagate the error.
                std::cout << "shutdown: " << ec.message() << std::endl;
}

void
Session::on_connect(const boost::system::error_code &ec)
{
        if (ec) {
                on_failure(id, ec);
                return;
        }

        // Perform the SSL handshake
        socket.async_handshake(
          boost::asio::ssl::stream_base::client,
          std::bind(&Session::on_handshake, shared_from_this(), std::placeholders::_1));
}

void
Session::terminate()
{
        is_shutting_down_ = true;
        shutdown();
}

void
Session::shutdown()
{
        socket.async_shutdown(
          std::bind(&Session::on_close, shared_from_this(), std::placeholders::_1));
}

void
Session::on_request_complete()
{
        if (is_shutting_down_)
                return;

        boost::system::error_code ec(error_code);
        on_success(id, parser.get(), ec);

        shutdown();
}

void
Session::on_handshake(const boost::system::error_code &ec)
{
        if (ec) {
                on_failure(id, ec);
                return;
        }

        boost::beast::http::async_write(
          socket,
          request,
          std::bind(
            &Session::on_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void
Session::on_write(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
        boost::ignore_unused(bytes_transferred);

        if (ec) {
                on_failure(id, ec);
                return;
        }

        // Receive the HTTP response
        boost::beast::http::async_read(
          socket,
          output_buf,
          parser,
          std::bind(
            &Session::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void
Session::on_read(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
        boost::ignore_unused(bytes_transferred);

        if (ec)
                error_code = ec;

        on_request_complete();
}

void
Session::run() noexcept
{
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(socket.native_handle(), host.c_str())) {
                boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                             boost::asio::error::get_ssl_category()};
                std::cerr << ec.message() << "\n";

                return on_failure(id, ec);
        }

        resolver_.async_resolve(host,
                                std::to_string(port),
                                std::bind(&Session::on_resolve,
                                          shared_from_this(),
                                          std::placeholders::_1,
                                          std::placeholders::_2));
}
