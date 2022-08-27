#include "events.hpp"

#include <nlohmann/json.hpp>

#include "mtx/events/encrypted.hpp"
#include "mtx/events/unknown.hpp"

namespace mtx::events {
namespace detail {

template<typename, typename = void>
struct can_edit : std::false_type
{};

template<typename Content>
struct can_edit<Content, std::void_t<decltype(Content::relations)>>
  : std::is_same<decltype(Content::relations), mtx::common::Relations>
{};
}

template<class Content>
[[gnu::used, gnu::retain]] void
to_json(nlohmann::json &obj, const Event<Content> &event)
{
    obj["content"] = event.content;
    obj["sender"]  = event.sender;
    if constexpr (std::is_same_v<Unknown, Content>)
        obj["type"] = event.content.type;
    else
        obj["type"] = ::mtx::events::to_string(event.type);
}

template<class Content>
[[gnu::used, gnu::retain]] void
from_json(const nlohmann::json &obj, Event<Content> &event)
{
    if (!std::is_same_v<Content, mtx::events::msg::Encrypted> &&
        obj.at("content").contains("m.new_content")) {
        auto new_content = obj.at("content").at("m.new_content");

        if (obj.at("content").contains("m.relates_to"))
            new_content["m.relates_to"] = obj.at("content").at("m.relates_to");
        if (obj.at("content").at("m.new_content").contains("m.relates_to"))
            new_content["m.new_content"]["m.relates_to"] =
              obj.at("content").at("m.new_content").at("m.relates_to");
        if (obj.at("content").contains("im.nheko.relations.v1.relations"))
            new_content["im.nheko.relations.v1.relations"] =
              obj.at("content").at("im.nheko.relations.v1.relations");

        event.content = new_content.get<Content>();
    } else if (obj.at("content").is_object()) {
        event.content = obj.at("content").get<Content>();
    } else {
        event.content = {};
    }

    auto type = obj.at("type").get<std::string>();
    if (type.size() > 255) {
        throw std::out_of_range("Type exceeds 255 bytes");
    }
    event.type = getEventType(type);

    event.sender = obj.value("sender", "");

    if (event.sender.size() > 255) {
        throw std::out_of_range("Sender exceeds 255 bytes");
    }

    if constexpr (std::is_same_v<Unknown, Content>)
        event.content.type = obj.at("type").get<std::string>();
}

template<class Content>
[[gnu::used, gnu::retain]] void
from_json(const nlohmann::json &obj, DeviceEvent<Content> &event)
{
    Event<Content> base_event = event;
    from_json(obj, base_event);
    event.content = base_event.content;
    event.type    = base_event.type;
    event.sender  = obj.at("sender").get<std::string>();
}

template<class Content>
[[gnu::used, gnu::retain]] void
to_json(nlohmann::json &obj, const DeviceEvent<Content> &event)
{
    Event<Content> base_event = event;
    to_json(obj, base_event);

    obj["sender"] = event.sender;
}

void
from_json(const nlohmann::json &obj, UnsignedData &data)
{
    if (obj.find("age") != obj.end())
        data.age = obj.at("age").get<uint64_t>();

    if (obj.find("transaction_id") != obj.end())
        data.transaction_id = obj.at("transaction_id").get<std::string>();

    if (obj.find("prev_sender") != obj.end())
        data.prev_sender = obj.at("prev_sender").get<std::string>();

    if (obj.find("replaces_state") != obj.end())
        data.replaces_state = obj.at("replaces_state").get<std::string>();

    if (obj.find("redacted_by") != obj.end())
        data.redacted_by = obj.at("redacted_by").get<std::string>();

    if (obj.find("redacted_because") != obj.end())
        data.redacted_because =
          obj.at("redacted_because").get<Event<mtx::events::msg::Redaction>>();
}

void
to_json(nlohmann::json &obj, const UnsignedData &event)
{
    if (!event.prev_sender.empty())
        obj["prev_sender"] = event.prev_sender;

    if (!event.transaction_id.empty())
        obj["transaction_id"] = event.transaction_id;

    if (!event.replaces_state.empty())
        obj["replaces_state"] = event.replaces_state;

    if (event.age != 0)
        obj["age"] = event.age;

    if (!event.redacted_by.empty())
        obj["redacted_by"] = event.redacted_by;

    if (event.redacted_because)
        obj["redacted_because"] = *event.redacted_because;
}

template<class Content>
[[gnu::used, gnu::retain]] void
from_json(const nlohmann::json &obj, StrippedEvent<Content> &event)
{
    Event<Content> &base = event;
    from_json(obj, base);

    event.state_key = obj.at("state_key").get<std::string>();

    if (event.state_key.size() > 255) {
        throw std::out_of_range("State key exceeds 255 bytes");
    }
}

template<class Content>
[[gnu::used, gnu::retain]] void
to_json(nlohmann::json &obj, const StrippedEvent<Content> &event)
{
    Event<Content> base_event = event;
    to_json(obj, base_event);

    obj["state_key"] = event.state_key;
}

template<class Content>
[[gnu::used, gnu::retain]] void
from_json(const nlohmann::json &obj, RoomEvent<Content> &event)
{
    Event<Content> &base = event;
    from_json(obj, base);

    event.event_id = obj.at("event_id").get<std::string>();

    if (event.event_id.size() > 255) {
        throw std::out_of_range("Event id exceeds 255 bytes");
    }

    event.origin_server_ts = obj.at("origin_server_ts").get<uint64_t>();

    // SPEC_BUG: Not present in the state array returned by /sync.
    if (obj.find("room_id") != obj.end())
        event.room_id = obj.at("room_id").get<std::string>();

    if (event.room_id.size() > 255) {
        throw std::out_of_range("Room id exceeds 255 bytes");
    }

    if (obj.find("unsigned") != obj.end())
        event.unsigned_data = obj.at("unsigned").get<UnsignedData>();
}

template<class Content>
[[gnu::used, gnu::retain]] void
to_json(nlohmann::json &obj, const RoomEvent<Content> &event)
{
    Event<Content> base_event = event;
    to_json(obj, base_event);

    if (!event.room_id.empty())
        obj["room_id"] = event.room_id;

    obj["event_id"]         = event.event_id;
    obj["unsigned"]         = event.unsigned_data;
    obj["origin_server_ts"] = event.origin_server_ts;
}

template<class Content>
[[gnu::used, gnu::retain]] void
to_json(nlohmann::json &obj, const StateEvent<Content> &event)
{
    RoomEvent<Content> base_event = event;
    to_json(obj, base_event);

    obj["state_key"] = event.state_key;
}

template<class Content>
[[gnu::used, gnu::retain]] void
from_json(const nlohmann::json &obj, StateEvent<Content> &event)
{
    RoomEvent<Content> &base = event;
    from_json(obj, base);

    event.state_key = obj.at("state_key").get<std::string>();

    if (event.state_key.size() > 255) {
        throw std::out_of_range("State key exceeds 255 bytes");
    }
}

template<class Content>
[[gnu::used, gnu::retain]] void
to_json(nlohmann::json &obj, const RedactionEvent<Content> &event)
{
    RoomEvent<Content> base_event = event;
    to_json(obj, base_event);

    obj["redacts"] = event.redacts;
}

template<class Content>
[[gnu::used, gnu::retain]] void
from_json(const nlohmann::json &obj, RedactionEvent<Content> &event)
{
    RoomEvent<Content> &base = event;
    from_json(obj, base);

    event.redacts = obj.at("redacts").get<std::string>();
}

template<class Content>
[[gnu::used, gnu::retain]] void
to_json(nlohmann::json &obj, const EncryptedEvent<Content> &event)
{
    RoomEvent<Content> base_event = event;
    to_json(obj, base_event);
}

template<class Content>
[[gnu::used, gnu::retain]] void
from_json(const nlohmann::json &obj, EncryptedEvent<Content> &event)
{
    RoomEvent<Content> &base = event;
    from_json(obj, base);
}

template<class Content>
[[gnu::used, gnu::retain]] void
to_json(nlohmann::json &obj, const EphemeralEvent<Content> &event)
{
    obj["content"] = event.content;
    if constexpr (std::is_same_v<Unknown, Content>)
        obj["type"] = event.content.type;
    else
        obj["type"] = ::mtx::events::to_string(event.type);

    if (!event.room_id.empty())
        obj["room_id"] = event.room_id;
}

template<class Content>
[[gnu::used, gnu::retain]] void
from_json(const nlohmann::json &obj, EphemeralEvent<Content> &event)
{
    event.content = obj.at("content").get<Content>();
    event.type    = getEventType(obj.at("type").get<std::string>());
    if constexpr (std::is_same_v<Unknown, Content>)
        event.content.type = obj.at("type").get<std::string>();

    if (obj.contains("room_id"))
        event.room_id = obj.at("room_id").get<std::string>();

    if (event.room_id.size() > 255) {
        throw std::out_of_range("Room id exceeds 255 bytes");
    }
}

}
