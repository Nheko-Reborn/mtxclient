#include "mtx/responses/empty.hpp"

using json = nlohmann::json;

namespace mtx {
namespace responses {

//Provides a deserialization function to use when empty responses are returned from the server
void
from_json(const json&, Empty&)
{}

}
}
