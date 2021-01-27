#include "events.hpp"

#include <nlohmann/json.hpp>

#include "mtx/events/unknown.hpp"

namespace mtx::events {

template<class Content>
[[gnu::used, llvm::used]] void
to_json(json &obj, const Event<Content> &event)
{
        obj["content"] = event.content;
        obj["sender"]  = event.sender;
        if constexpr (std::is_same_v<Unknown, Content>)
                obj["type"] = event.content.type;
        else
                obj["type"] = ::mtx::events::to_string(event.type);
}

template<class Content>
[[gnu::used, llvm::used]] void
from_json(const json &obj, Event<Content> &event)
{
        if (obj.at("content").contains("m.new_content")) {
                auto new_content = obj.at("content");
                for (const auto &e : obj["content"]["m.new_content"].items()) {
                        if (e.key() != "m.relates_to" &&
                            e.key() != "im.nheko.relations.v1.relations")
                                new_content[e.key()] = e.value();
                }
                event.content = new_content.get<Content>();
        } else {
                event.content = obj.at("content").get<Content>();
        }

        event.type   = getEventType(obj.at("type").get<std::string>());
        event.sender = obj.value("sender", "");

        if constexpr (std::is_same_v<Unknown, Content>)
                event.content.type = obj.at("type").get<std::string>();
}

template<class Content>
[[gnu::used, llvm::used]] void
from_json(const json &obj, DeviceEvent<Content> &event)
{
        Event<Content> base_event = event;
        from_json(obj, base_event);
        event.content = base_event.content;
        event.type    = base_event.type;
        event.sender  = obj.at("sender");
}

template<class Content>
[[gnu::used, llvm::used]] void
to_json(json &obj, const DeviceEvent<Content> &event)
{
        Event<Content> base_event = event;
        to_json(obj, base_event);

        obj["sender"] = event.sender;
}

void
from_json(const json &obj, UnsignedData &data)
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
to_json(json &obj, const UnsignedData &event)
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
[[gnu::used, llvm::used]] void
from_json(const json &obj, StrippedEvent<Content> &event)
{
        Event<Content> &base = event;
        from_json(obj, base);

        event.state_key = obj.at("state_key");
}

template<class Content>
[[gnu::used, llvm::used]] void
to_json(json &obj, const StrippedEvent<Content> &event)
{
        Event<Content> base_event = event;
        to_json(obj, base_event);

        obj["state_key"] = event.state_key;
}

template<class Content>
[[gnu::used, llvm::used]] void
from_json(const json &obj, RoomEvent<Content> &event)
{
        Event<Content> &base = event;
        from_json(obj, base);

        event.event_id         = obj.at("event_id");
        event.origin_server_ts = obj.at("origin_server_ts");

        // SPEC_BUG: Not present in the state array returned by /sync.
        if (obj.find("room_id") != obj.end())
                event.room_id = obj.at("room_id");

        if (obj.find("unsigned") != obj.end())
                event.unsigned_data = obj.at("unsigned");
}

template<class Content>
[[gnu::used, llvm::used]] void
to_json(json &obj, const RoomEvent<Content> &event)
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
[[gnu::used, llvm::used]] void
to_json(json &obj, const StateEvent<Content> &event)
{
        RoomEvent<Content> base_event = event;
        to_json(obj, base_event);

        obj["state_key"] = event.state_key;
}

template<class Content>
[[gnu::used, llvm::used]] void
from_json(const json &obj, StateEvent<Content> &event)
{
        RoomEvent<Content> &base = event;
        from_json(obj, base);

        event.state_key = obj.at("state_key").get<std::string>();
}

template<class Content>
[[gnu::used, llvm::used]] void
to_json(json &obj, const RedactionEvent<Content> &event)
{
        RoomEvent<Content> base_event = event;
        to_json(obj, base_event);

        obj["redacts"] = event.redacts;
}

template<class Content>
[[gnu::used, llvm::used]] void
from_json(const json &obj, RedactionEvent<Content> &event)
{
        RoomEvent<Content> &base = event;
        from_json(obj, base);

        event.redacts = obj.at("redacts").get<std::string>();
}

template<class Content>
[[gnu::used, llvm::used]] void
to_json(json &obj, const EncryptedEvent<Content> &event)
{
        RoomEvent<Content> base_event = event;
        to_json(obj, base_event);
}

template<class Content>
[[gnu::used, llvm::used]] void
from_json(const json &obj, EncryptedEvent<Content> &event)
{
        RoomEvent<Content> &base = event;
        from_json(obj, base);
}

template<class Content>
[[gnu::used, llvm::used]] void
to_json(json &obj, const EphemeralEvent<Content> &event)
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
[[gnu::used, llvm::used]] void
from_json(const json &obj, EphemeralEvent<Content> &event)
{
        event.content = obj.at("content").get<Content>();
        event.type    = getEventType(obj.at("type").get<std::string>());
        if constexpr (std::is_same_v<Unknown, Content>)
                event.content.type = obj.at("type").get<std::string>();

        if (obj.contains("room_id"))
                event.room_id = obj.at("room_id").get<std::string>();
}

}
