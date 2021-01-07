#include <nlohmann/json.hpp>

#include "mtx/events/ephemeral/receipt.hpp"

namespace mtx {
namespace events {
namespace ephemeral {

void
from_json(const nlohmann::json &obj, Receipt &content)
{
        for (const auto &eventReceipts : obj.items()) {
                for (const auto &userReceipts : eventReceipts.value().at("m.read").items()) {
                        content.receipts[eventReceipts.key()].users[userReceipts.key()].ts =
                          userReceipts.value().value<uint64_t>("ts", 0);
                }
        }
}

void
to_json(nlohmann::json &obj, const Receipt &content)
{
        for (const auto &[event_id, userReceipts] : content.receipts) {
                for (const auto &[user_id, receipt] : userReceipts.users) {
                        obj[event_id]["m.read"][user_id]["ts"] = receipt.ts;
                }
        }
}

} // namespace state
} // namespace events
} // namespace mtx
