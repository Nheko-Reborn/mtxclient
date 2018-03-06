#include <boost/algorithm/string.hpp>

#include <chrono>
#include <fstream>
#include <streambuf>
#include <thread>

#include <gtest/gtest.h>

#include "client.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses.hpp"

using namespace mtx::client;
using namespace mtx::identifiers;

using namespace std;

// std::vector<std::string> results;
// boost::split(results, res.content_uri, [](char c) { return c == '/'; });

using ErrType = std::experimental::optional<errors::ClientError>;

string
read_file(const string &file_path)
{
        ifstream file(file_path);
        string data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

        return data;
}

void
validate_upload(const mtx::responses::ContentURI &res, ErrType err)
{
        ASSERT_FALSE(err);
        ASSERT_TRUE(res.content_uri.size() > 10);
        cout << res.content_uri << endl;
}

TEST(MediaAPI, UploadTextFile)
{
        std::shared_ptr<Client> alice = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, ErrType err) {
                ASSERT_FALSE(err);

                alice->upload("This is a text content", "text/plain", "doc.txt", validate_upload);
        });

        alice->close();
}

TEST(MediaAPI, UploadAudio)
{
        std::shared_ptr<Client> bob = std::make_shared<Client>("localhost");

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, ErrType err) {
                ASSERT_FALSE(err);

                bob->upload(
                  read_file("./fixtures/sound.mp3"), "audio/mp3", "sound.mp3", validate_upload);
        });

        bob->close();
}

TEST(MediaAPI, UploadImage)
{
        std::shared_ptr<Client> carl = std::make_shared<Client>("localhost");

        carl->login("carl", "secret", [carl](const mtx::responses::Login &, ErrType err) {
                ASSERT_FALSE(err);

                carl->upload(
                  read_file("./fixtures/test.jpeg"), "image/jpeg", "test.jpeg", validate_upload);
        });

        carl->close();
}
