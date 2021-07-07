#include <chrono>
#include <fstream>
#include <streambuf>
#include <thread>

#include <gtest/gtest.h>

#include <mtx/requests.hpp>
#include <mtx/responses.hpp>
#include <mtxclient/http/client.hpp>

#include "test_helpers.hpp"

using namespace mtx::http;
using namespace mtx::identifiers;

using namespace std;

string
read_file(const string &file_path)
{
        ifstream file(file_path);
        string data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

        return data;
}

void
validate_upload(const mtx::responses::ContentURI &res, RequestErr err)
{
        if (err) {
                if (err->status_code)
                        cout << err->status_code << "\n";
                if (!err->matrix_error.error.empty())
                        cout << err->matrix_error.error << "\n";
                if (err->error_code)
                        cout << err->error_code << "\n";
        }

        ASSERT_FALSE(err);
        ASSERT_TRUE(res.content_uri.size() > 10);
}

TEST(MediaAPI, UploadTextFile)
{
        std::shared_ptr<Client> alice = make_test_client();

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                ASSERT_FALSE(err);

                const auto text = "This is some random text";

                alice->upload(
                  text,
                  "text/plain",
                  "doc.txt",
                  [alice, text](const mtx::responses::ContentURI &res, RequestErr err) {
                          validate_upload(res, err);

                          alice->download(
                            res.content_uri,
                            [text](const string &data,
                                   const string &content_type,
                                   const string &original_filename,
                                   RequestErr err) {
                                    ASSERT_FALSE(err);
                                    EXPECT_EQ(data, text);
                                    EXPECT_EQ(content_type.substr(0, std::size("text/plain") - 1),
                                              "text/plain");
                                    EXPECT_EQ(original_filename, "doc.txt");
                            });
                  });
        });

        alice->close();
}

TEST(MediaAPI, UploadAudio)
{
        std::shared_ptr<Client> bob = make_test_client();

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                ASSERT_FALSE(err);

                const auto audio = read_file(fixture_prefix() + "/fixtures/sound.mp3");

                bob->upload(audio,
                            "audio/mp3",
                            "sound.mp3",
                            [bob, audio](const mtx::responses::ContentURI &res, RequestErr err) {
                                    validate_upload(res, err);

                                    bob->download(res.content_uri,
                                                  [audio](const string &data,
                                                          const string &content_type,
                                                          const string &original_filename,
                                                          RequestErr err) {
                                                          ASSERT_FALSE(err);
                                                          EXPECT_EQ(data, audio);
                                                          EXPECT_EQ(content_type, "audio/mp3");
                                                          EXPECT_EQ(original_filename, "sound.mp3");
                                                  });
                            });
        });

        bob->close();
}

TEST(MediaAPI, UploadImage)
{
        std::shared_ptr<Client> carl = make_test_client();

        carl->login("carl", "secret", [carl](const mtx::responses::Login &, RequestErr err) {
                ASSERT_FALSE(err);

                const auto img = read_file(fixture_prefix() + "/fixtures/test.jpeg");

                carl->upload(img,
                             "image/jpeg",
                             "a name that needs to be encode/d.jpeg",
                             [carl, img](const mtx::responses::ContentURI &res, RequestErr err) {
                                     validate_upload(res, err);
                             });

                carl->upload(
                  img,
                  "image/jpeg",
                  "test.jpeg",
                  [carl, img](const mtx::responses::ContentURI &res, RequestErr err) {
                          validate_upload(res, err);

                          ThumbOpts opts;
                          opts.mxc_url = res.content_uri;
                          carl->get_thumbnail(opts, [](const std::string &res, RequestErr err) {
                                  ASSERT_FALSE(err);
                                  ASSERT_FALSE(res.empty());
                          });

                          carl->download(res.content_uri,
                                         [img](const string &data,
                                               const string &content_type,
                                               const string &original_filename,
                                               RequestErr err) {
                                                 ASSERT_FALSE(err);
                                                 EXPECT_EQ(data, img);
                                                 EXPECT_EQ(content_type, "image/jpeg");
                                                 EXPECT_EQ(original_filename, "test.jpeg");
                                         });
                  });
        });

        carl->close();
}

TEST(MediaAPI, UploadSVG)
{
        std::shared_ptr<Client> carl = make_test_client();

        carl->login("carl", "secret", [carl](const mtx::responses::Login &, RequestErr err) {
                ASSERT_FALSE(err);

                const auto img = read_file(fixture_prefix() + "/fixtures/kiwi.svg");

                carl->upload(
                  img,
                  "image/svg",
                  "kiwi.svg",
                  [carl, img](const mtx::responses::ContentURI &res, RequestErr err) {
                          validate_upload(res, err);

                          ThumbOpts opts;
                          opts.mxc_url = res.content_uri;
                          carl->get_thumbnail(opts, [img](const std::string &res, RequestErr err) {
                                  ASSERT_FALSE(err);
                                  EXPECT_EQ(res, img);
                          });

                          carl->download(res.content_uri,
                                         [img](const string &data,
                                               const string &content_type,
                                               const string &original_filename,
                                               RequestErr err) {
                                                 ASSERT_FALSE(err);
                                                 EXPECT_EQ(data, img);
                                                 EXPECT_EQ(content_type, "image/svg");
                                                 EXPECT_EQ(original_filename, "kiwi.svg");
                                         });
                  });
        });

        carl->close();
}
