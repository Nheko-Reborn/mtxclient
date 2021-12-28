#include <atomic>

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "mtxclient/crypto/client.hpp"
#include "mtxclient/crypto/types.hpp"
#include "mtxclient/http/client.hpp"

#include "mtx/requests.hpp"
#include "mtx/responses.hpp"

#include "test_helpers.hpp"

using namespace mtx::client;
using namespace mtx::http;
using namespace mtx::crypto;

using namespace mtx::identifiers;
using namespace mtx::events;
using namespace mtx::events::msg;
using namespace mtx::events::collections;
using namespace mtx::responses;

using namespace std;

using namespace nlohmann;

TEST(Devices, DeleteDevices)
{
    auto alice = make_test_client();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    auto alice_dummy = make_test_client();
    alice_dummy->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || alice_dummy->access_token().empty())
        sleep();

    std::string myDevice = alice->device_id();

    atomic_int responses(0);
    std::vector<std::string> device_ids;

    // Get list of devices, fill device_ids with all devices except current
    alice->query_devices(
      [&responses, &device_ids, myDevice](const mtx::responses::QueryDevices &res, RequestErr err) {
          check_error(err);
          for (auto it = res.devices.begin(); it != res.devices.end(); ++it)
              if (it->device_id != myDevice)
                  device_ids.push_back(it->device_id);
          responses += 1;
      });

    while (responses != 1)
        sleep();

    // Delete all devices except current
    alice->delete_devices(
      device_ids,
      mtx::http::UIAHandler([](const mtx::http::UIAHandler &h,
                               const mtx::user_interactive::Unauthorized &unauthorized) {
          ASSERT_EQ(unauthorized.flows.size(), 1);
          ASSERT_EQ(unauthorized.flows[0].stages.size(), 1);
          ASSERT_EQ(unauthorized.flows[0].stages[0], mtx::user_interactive::auth_types::password);

          mtx::user_interactive::Auth auth;
          auth.session = unauthorized.session;
          mtx::user_interactive::auth::Password pass{};
          pass.password        = "secret";
          pass.identifier_user = "alice";
          pass.identifier_type = mtx::user_interactive::auth::Password::IdType::UserId;
          auth.content         = pass;
          h.next(auth);
      }),
      [&responses](RequestErr e) {
          check_error(e);
          responses += 1;
      });

    while (responses != 2)
        sleep();

    // Check if the current device is the only device left
    alice->query_devices(
      [&responses, myDevice](const mtx::responses::QueryDevices &res, RequestErr err) {
          check_error(err);
          ASSERT_EQ(res.devices.size(), 1);
          ASSERT_EQ(res.devices.begin()->device_id, myDevice);
          responses += 1;
      });

    // Check if dummy can no longer retrieve list of devices because their token was removed
    alice_dummy->query_devices([](const mtx::responses::QueryDevices &res, RequestErr err) {
        ASSERT_TRUE(err);
        ASSERT_EQ(res.devices.size(), 0);
        EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_UNKNOWN_TOKEN");
    });

    while (responses != 3)
        sleep();

    // Delete current device
    alice->delete_device(
      myDevice,
      mtx::http::UIAHandler([](const mtx::http::UIAHandler &h,
                               const mtx::user_interactive::Unauthorized &unauthorized) {
          ASSERT_EQ(unauthorized.flows.size(), 1);
          ASSERT_EQ(unauthorized.flows[0].stages.size(), 1);
          ASSERT_EQ(unauthorized.flows[0].stages[0], mtx::user_interactive::auth_types::password);

          mtx::user_interactive::Auth auth;
          auth.session = unauthorized.session;
          mtx::user_interactive::auth::Password pass{};
          pass.password        = "secret";
          pass.identifier_user = "alice";
          pass.identifier_type = mtx::user_interactive::auth::Password::IdType::UserId;
          auth.content         = pass;
          h.next(auth);
      }),
      [&responses](RequestErr e) {
          check_error(e);
          responses += 1;
      });

    alice->close();
}

TEST(Devices, RenameDevices)
{
    auto alice_one = make_test_client();
    auto alice_two = make_test_client();

    alice_one->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });
    alice_two->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice_one->access_token().empty() || alice_two->access_token().empty())
        sleep();

    atomic_int responses(0);

    // Rename the two logged in devices to Alice One and Alice Two
    alice_one->set_device_name(alice_one->device_id(), "Alice One", [&responses](RequestErr err) {
        check_error(err);
        responses += 1;
    });
    alice_two->set_device_name(alice_two->device_id(), "Alice Two", [&responses](RequestErr err) {
        check_error(err);
        responses += 1;
    });

    while (responses != 2)
        sleep();

    // Request info for current devices and confirm display name changed
    alice_one->get_device(alice_one->device_id(),
                          [&responses](const mtx::responses::Device &res, RequestErr err) {
                              ASSERT_EQ(res.display_name, "Alice One");
                              check_error(err);
                              responses += 1;
                          });
    alice_two->get_device(alice_two->device_id(),
                          [&responses](const mtx::responses::Device &res, RequestErr err) {
                              ASSERT_EQ(res.display_name, "Alice Two");
                              check_error(err);
                              responses += 1;
                          });

    while (responses != 4)
        sleep();

    // Request device list and check both One and Two are in the list
    alice_one->query_devices([&responses](const mtx::responses::QueryDevices &res, RequestErr err) {
        check_error(err);
        bool oneFound = false;
        bool twoFound = false;
        for (auto it = res.devices.begin(); it != res.devices.end(); ++it) {
            if (it->display_name == "Alice One")
                oneFound = true;
            if (it->display_name == "Alice Two")
                twoFound = true;
        }
        ASSERT_EQ(oneFound, true);
        ASSERT_EQ(twoFound, true);
        responses += 1;
    });

    while (responses != 5)
        sleep();

    alice_one->close();
    alice_two->close();
}
