#pragma once

/// @file
/// @brief All the template stuff needed for the http client.
///
/// If you implement custom event types, this can be useful, but in general including this header
/// just adds compile time without any benefits.

#include "client.hpp"
#include "mtxclient/utils.hpp" // for random_token, url_encode, des...

#include <nlohmann/json.hpp>

namespace mtx {
//! A few helpers for the http client.
namespace client {
namespace utils {

/// @brief deserialize a type or string from json.
///
/// Used internally to deserialize the response types for the various http methods.
template<class T>
inline T
deserialize(const std::string &data)
{
        return nlohmann::json::parse(data);
}

template<>
inline std::string
deserialize<std::string>(const std::string &data)
{
        return data;
}

/// @brief serialize a type or string to json.
///
/// Used internally to serialize the request types for the various http methods.
template<class T>
inline std::string
serialize(const T &obj)
{
        return nlohmann::json(obj).dump();
}

template<>
inline std::string
serialize<std::string>(const std::string &obj)
{
        return obj;
}
}
}
}
template<class Request, class Response>
void
mtx::http::Client::post(const std::string &endpoint,
                        const Request &req,
                        Callback<Response> callback,
                        bool requires_auth,
                        const std::string &content_type)
{
        post(
          endpoint,
          client::utils::serialize(req),
          prepare_callback<Response>(
            [callback](const Response &res, HeaderFields, RequestErr err) { callback(res, err); }),
          requires_auth,
          content_type);
}

// put function for the PUT HTTP requests that send responses
template<class Request, class Response>
void
mtx::http::Client::put(const std::string &endpoint,
                       const Request &req,
                       Callback<Response> callback,
                       bool requires_auth)
{
        put(
          endpoint,
          client::utils::serialize(req),
          prepare_callback<Response>(
            [callback](const Response &res, HeaderFields, RequestErr err) { callback(res, err); }),
          requires_auth);
}

// provides PUT functionality for the endpoints which dont respond with a body
template<class Request>
void
mtx::http::Client::put(const std::string &endpoint,
                       const Request &req,
                       ErrCallback callback,
                       bool requires_auth)
{
        mtx::http::Client::put<Request, mtx::responses::Empty>(
          endpoint,
          req,
          [callback](const mtx::responses::Empty, RequestErr err) { callback(err); },
          requires_auth);
}

template<class Response>
void
mtx::http::Client::get(const std::string &endpoint,
                       HeadersCallback<Response> callback,
                       bool requires_auth,
                       const std::string &endpoint_namespace)
{
        get(endpoint, prepare_callback<Response>(callback), requires_auth, endpoint_namespace);
}

template<class Response>
mtx::http::TypeErasedCallback
mtx::http::Client::prepare_callback(HeadersCallback<Response> callback)
{
        auto type_erased_cb = [callback](HeaderFields headers,
                                         const std::string &body,
                                         const boost::system::error_code &err_code,
                                         boost::beast::http::status status_code) {
                Response response_data;
                mtx::http::ClientError client_error;

                if (err_code) {
                        client_error.error_code = err_code;
                        return callback(response_data, headers, client_error);
                }

                // We only count 2xx status codes as success.
                if (static_cast<int>(status_code) < 200 || static_cast<int>(status_code) >= 300) {
                        client_error.status_code = status_code;

                        // Try to parse the response in case we have an endpoint that
                        // doesn't return an error struct for non 200 requests.
                        try {
                                response_data = client::utils::deserialize<Response>(body);
                        } catch (const nlohmann::json::exception &) {
                        }

                        // The homeserver should return an error struct.
                        try {
                                nlohmann::json json_error       = json::parse(body);
                                mtx::errors::Error matrix_error = json_error;

                                client_error.matrix_error = matrix_error;
                                return callback(response_data, headers, client_error);
                        } catch (const nlohmann::json::exception &e) {
                                client_error.parse_error = std::string(e.what()) + ": " + body;

                                return callback(response_data, headers, client_error);
                        }
                }

                // If we reach that point we most likely have a valid output from the
                // homeserver.
                try {
                        auto res = client::utils::deserialize<Response>(body);
                        callback(std::move(res), headers, {});
                } catch (const nlohmann::json::exception &e) {
                        client_error.parse_error = std::string(e.what()) + ": " + body;
                        callback(response_data, headers, client_error);
                }
        };

        return type_erased_cb;
}

template<typename EventContent>
[[gnu::used, llvm::used]] void
mtx::http::Client::send_to_device(
  const std::string &txid,
  const std::map<mtx::identifiers::User, std::map<std::string, EventContent>> &messages,
  ErrCallback callback)
{
        constexpr auto event_type = mtx::events::to_device_content_to_type<EventContent>;
        static_assert(event_type != mtx::events::EventType::Unsupported);

        json j;
        for (const auto &[user, deviceToMessage] : messages)
                for (const auto &[deviceid, message] : deviceToMessage)
                        j["messages"][user.to_string()][deviceid] = message;

        send_to_device(mtx::events::to_string(event_type), txid, j, callback);
}

template<class Payload>
[[gnu::used, llvm::used]] void
mtx::http::Client::send_room_message(const std::string &room_id,
                                     const Payload &payload,
                                     Callback<mtx::responses::EventId> callback)
{
        send_room_message<Payload>(room_id, generate_txn_id(), payload, callback);
}

template<class Payload>
[[gnu::used, llvm::used]] void
mtx::http::Client::send_room_message(const std::string &room_id,
                                     const std::string &txn_id,
                                     const Payload &payload,
                                     Callback<mtx::responses::EventId> callback)
{
        constexpr auto event_type = mtx::events::message_content_to_type<Payload>;
        static_assert(event_type != mtx::events::EventType::Unsupported);

        const auto api_path = "/client/r0/rooms/" + mtx::client::utils::url_encode(room_id) +
                              "/send/" + mtx::events::to_string(event_type) + "/" +
                              mtx::client::utils::url_encode(txn_id);

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload>
[[gnu::used, llvm::used]] void
mtx::http::Client::send_state_event(const std::string &room_id,
                                    const std::string &state_key,
                                    const Payload &payload,
                                    Callback<mtx::responses::EventId> callback)
{
        constexpr auto event_type = mtx::events::state_content_to_type<Payload>;
        static_assert(event_type != mtx::events::EventType::Unsupported);

        const auto api_path = "/client/r0/rooms/" + mtx::client::utils::url_encode(room_id) +
                              "/state/" + mtx::events::to_string(event_type) + "/" +
                              mtx::client::utils::url_encode(state_key);

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload>
[[gnu::used, llvm::used]] void
mtx::http::Client::send_state_event(const std::string &room_id,
                                    const Payload &payload,
                                    Callback<mtx::responses::EventId> callback)
{
        send_state_event<Payload>(room_id, "", payload, callback);
}
