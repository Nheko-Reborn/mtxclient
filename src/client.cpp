#include <boost/bind.hpp>

#include "client.hpp"
#include "utils.hpp"

#include "mtx/requests.hpp"
#include "mtx/responses.hpp"

using namespace mtx::client;
using namespace boost::beast;

Client::Client(const std::string &server)
  : resolver_{ios_}
  , server_{server}
{
        work_.reset(new boost::asio::io_service::work(ios_));

        const auto threads_num = std::max(1U, std::thread::hardware_concurrency());

        for (unsigned int i = 0; i < threads_num; ++i)
                thread_group_.add_thread(new boost::thread([this]() { ios_.run(); }));
}

void
Client::close()
{
        // Destroy work object. This allows the I/O thread to
        // exit the event loop when there are no more pending
        // asynchronous operations.
        work_.reset(nullptr);

        // Wait for the worker threads to exit.
        thread_group_.join_all();
}

void
Client::on_resolve(std::shared_ptr<Session> s,
                   boost::system::error_code ec,
                   boost::asio::ip::tcp::resolver::results_type results)
{
        if (ec)
                return s->on_failure(s->id, ec);

        // Add new session to the list of active sessions so that we can access
        // it if the user decides to cancel the corresponding request before
        // it completes.
        // Because active sessions list can be accessed from multiple threads,
        //
        // we guard it with a mutex to avoid data corruption.
        std::unique_lock<std::mutex> lock(active_sessions_guard_);
        active_sessions_[s->id] = s;
        lock.unlock();

        boost::asio::async_connect(
          s->socket.next_layer(),
          results.begin(),
          results.end(),
          std::bind(&Client::on_connect, shared_from_this(), s, std::placeholders::_1));
}

void
Client::on_connect(std::shared_ptr<Session> s, boost::system::error_code ec)
{
        if (ec) {
                remove_session(s);
                return s->on_failure(s->id, ec);
        }

        // Check if the request is already cancelled and we shouldn't move forward.
        std::unique_lock<std::mutex> cancel_lock(s->cancel_guard);
        if (s->is_cancelled)
                return on_request_complete(s);

        // Perform the SSL handshake
        s->socket.async_handshake(
          boost::asio::ssl::stream_base::client,
          std::bind(&Client::on_handshake, shared_from_this(), s, std::placeholders::_1));
}

void
Client::on_handshake(std::shared_ptr<Session> s, boost::system::error_code ec)
{
        if (ec) {
                remove_session(s);
                return s->on_failure(s->id, ec);
        }

        // Check if the request is already cancelled and we shouldn't move forward.
        std::unique_lock<std::mutex> cancel_lock(s->cancel_guard);
        if (s->is_cancelled)
                return on_request_complete(s);

        boost::beast::http::async_write(s->socket,
                                        s->request,
                                        std::bind(&Client::on_write,
                                                  shared_from_this(),
                                                  s,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2));
}

void
Client::on_write(std::shared_ptr<Session> s,
                 boost::system::error_code ec,
                 std::size_t bytes_transferred)
{
        boost::ignore_unused(bytes_transferred);

        if (ec) {
                remove_session(s);
                return s->on_failure(s->id, ec);
        }

        std::unique_lock<std::mutex> cancel_lock(s->cancel_guard);
        if (s->is_cancelled)
                return on_request_complete(s);

        // Receive the HTTP response
        http::async_read(
          s->socket,
          s->output_buf,
          s->parser,
          std::bind(
            &Client::on_read, shared_from_this(), s, std::placeholders::_1, std::placeholders::_2));
}

void
Client::on_read(std::shared_ptr<Session> s,
                boost::system::error_code ec,
                std::size_t bytes_transferred)
{
        boost::ignore_unused(bytes_transferred);

        if (ec)
                s->error_code = ec;

        on_request_complete(s);
}

void
Client::do_request(std::shared_ptr<Session> s)
{
        resolver_.async_resolve(server_,
                                "443",
                                std::bind(&Client::on_resolve,
                                          shared_from_this(),
                                          s,
                                          std::placeholders::_1,
                                          std::placeholders::_2));
}

void
Client::cancel_request(RequestID request_id)
{
        std::unique_lock<std::mutex> lock(active_sessions_guard_);

        auto it = active_sessions_.find(request_id);
        if (it != active_sessions_.end()) {
                std::unique_lock<std::mutex> cancel_lock(it->second->cancel_guard);
                it->second->is_cancelled = true;
        }
}

void
Client::remove_session(std::shared_ptr<Session> s)
{
        // Shutting down the connection. This method may
        // fail in case the socket is not connected. We don't
        // care about the error code if this function fails.
        boost::system::error_code ignored_ec;

        s->socket.async_shutdown([s](boost::system::error_code ec) {
                if (ec == boost::asio::error::eof) {
                        // Rationale:
                        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
                        ec.assign(0, ec.category());
                }

                if (ec)
                        // TODO: propagate the error.
                        std::cout << "shutdown: " << ec << std::endl;
        });

        // Remove the session from the map of active sessions.
        std::unique_lock<std::mutex> lock(active_sessions_guard_);

        auto it = active_sessions_.find(s->id);
        if (it != active_sessions_.end())
                active_sessions_.erase(it);

        lock.unlock();
}

void
Client::on_request_complete(std::shared_ptr<Session> s)
{
        remove_session(s);

        boost::system::error_code ec;

        if (s->error_code == 0 && s->is_cancelled) {
                ec = boost::asio::error::operation_aborted;
                s->on_failure(s->id, ec);
                return;
        } else {
                ec = s->error_code;
        }

        s->on_success(s->id, s->parser.get(), ec);
}

//
// Client API endpoints
//

void
Client::login(
  const std::string &user,
  const std::string &password,
  std::function<void(const mtx::responses::Login &response,
                     std::experimental::optional<mtx::client::errors::ClientError>)> callback)
{
        mtx::requests::Login req;
        req.user     = user;
        req.password = password;

        post<mtx::requests::Login, mtx::responses::Login>(
          "/login",
          req,
          [this, callback](const mtx::responses::Login &resp,
                           std::experimental::optional<mtx::client::errors::ClientError> err) {
                  if (!err && resp.access_token.size()) {
                          access_token_ = resp.access_token;
                  }
                  callback(resp, err);
          },
          false);
}

void
Client::logout(
  std::function<void(const mtx::responses::Logout &response,
                     std::experimental::optional<mtx::client::errors::ClientError>)> callback)
{
        mtx::requests::Logout req;

        post<mtx::requests::Logout, mtx::responses::Logout>(
          "/logout",
          req,
          [this, callback](const mtx::responses::Logout &res,
                           std::experimental::optional<mtx::client::errors::ClientError> err) {
                  if (!err) {
                          // Clear the now invalid access token when logout is successful
                          access_token_.clear();
                  }
                  // Pass up response and error to supplied callback
                  callback(res, err);
          });
}

void
Client::create_room(const mtx::requests::CreateRoom &room_options,
                    std::function<void(const mtx::responses::CreateRoom &, RequestErr)> callback)
{
        post<mtx::requests::CreateRoom, mtx::responses::CreateRoom>(
          "/createRoom", room_options, callback);
}

void
Client::join_room(const mtx::identifiers::Room &room_id,
                  std::function<void(const nlohmann::json &, RequestErr)> callback)
{
        auto api_path = "/rooms/" + room_id.toString() + "/join";

        post<std::string, nlohmann::json>(api_path, "", callback);
}

void
Client::join_room(const std::string &room,
                  std::function<void(const nlohmann::json &, RequestErr)> callback)
{
        auto api_path = "/join/" + room;

        post<std::string, nlohmann::json>(api_path, "", callback);
}

void
Client::leave_room(const mtx::identifiers::Room &room_id,
                   std::function<void(const nlohmann::json &, RequestErr)> callback)
{
        auto api_path = "/rooms/" + room_id.toString() + "/leave";

        post<std::string, nlohmann::json>(api_path, "", callback);
}

void
Client::sync(const std::string &filter,
             const std::string &since,
             bool full_state,
             uint16_t timeout,
             std::function<void(const mtx::responses::Sync &, RequestErr)> callback)
{
        std::map<std::string, std::string> params;

        if (!filter.empty())
                params.emplace("filter", filter);

        if (!since.empty())
                params.emplace("since", since);

        if (full_state)
                params.emplace("full_state", "true");

        params.emplace("timeout", std::to_string(timeout));

        get<mtx::responses::Sync>("/sync?" + utils::query_params(params), callback);
}
