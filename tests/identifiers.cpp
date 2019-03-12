#include <gtest/gtest.h>

#include <mtx/identifiers.hpp>

using namespace mtx::identifiers;

TEST(MatrixIdentifiers, Hostname)
{
        auto t1 = parse<User>("@39fasdsdfsdf:333.22.22.22:5000");
        EXPECT_EQ(t1.hostname(), "333.22.22.22:5000");

        auto t2 = parse<User>("@39fasdsdfsdf:333:22:22.22:5000");
        EXPECT_EQ(t2.hostname(), "333:22:22.22:5000");
}

TEST(MatrixIdentifiers, RoomValid)
{
        Room room1 = parse<Room>("!39fasdsdfsdf:example.com:5000");

        EXPECT_EQ(room1.to_string(), "!39fasdsdfsdf:example.com:5000");
        EXPECT_EQ(room1.localpart(), "39fasdsdfsdf");
        EXPECT_EQ(room1.hostname(), "example.com:5000");

        Room room2 = parse<Room>("!39fasdsdfsdf:example.com");

        EXPECT_EQ(room2.to_string(), "!39fasdsdfsdf:example.com");
        EXPECT_EQ(room2.localpart(), "39fasdsdfsdf");
        EXPECT_EQ(room2.hostname(), "example.com");
}

TEST(MatrixIdentifiers, IdentifierInvalid)
{
        ASSERT_THROW(parse<Room>("39fasdsdfsdf:example.com:5000"), std::invalid_argument);
        ASSERT_THROW(parse<User>("39fasdsdfsdf:example.com:5000"), std::invalid_argument);
}
