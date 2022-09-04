#include <nlohmann/json.hpp>

#include "mtx/events/ephemeral/receipt.hpp"

namespace mtx {
namespace events {
namespace ephemeral {

void
from_json(const nlohmann::json &obj, Receipt &content)
{
    for (const auto &eventReceipts : obj.items()) {
        for (const auto &receiptsByType : eventReceipts.value().items()) {
            Receipt::ReceiptType t = Receipt::Read;
            if (receiptsByType.key() == "m.read")
                t = Receipt::Read;
            else if (receiptsByType.key() == "m.read.private" ||
                     receiptsByType.key() == "org.matrix.msc2285.read.private")
                t = Receipt::ReadPrivate;
            else
                continue;

            for (const auto &userReceipts : receiptsByType.value().items()) {
                content.receipts[eventReceipts.key()][t].users[userReceipts.key()].ts =
                  userReceipts.value().value<uint64_t>("ts", 0);
            }
        }
    }
}

void
to_json(nlohmann::json &obj, const Receipt &content)
{
    for (const auto &[event_id, receiptsByType] : content.receipts) {
        for (const auto &[receiptType, userReceipts] : receiptsByType) {
            for (const auto &[user_id, receipt] : userReceipts.users) {
                if (receiptType == Receipt::ReceiptType::Read)
                    obj[event_id]["m.read"][user_id]["ts"] = receipt.ts;
                else if (receiptType == Receipt::ReceiptType::ReadPrivate)

                    obj[event_id]["m.read.private"][user_id]["ts"] = receipt.ts;
            }
        }
    }
}

} // namespace state
} // namespace events
} // namespace mtx
