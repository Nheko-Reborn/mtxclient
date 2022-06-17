#include <gtest/gtest.h>

#include <mtx/errors.hpp>

#include <nlohmann/json.hpp>

using namespace mtx::errors;

TEST(MatrixErrors, BasicError)
{
    nlohmann::json data = R"({
	  "errcode": "M_UNRECOGNIZED",
	  "error": "Unrecognized request"
	})"_json;

    Error err = data.get<Error>();

    EXPECT_EQ(err.errcode, ErrorCode::M_UNRECOGNIZED);
    EXPECT_EQ(err.error, "Unrecognized request");

    nlohmann::json data2 = R"({
	  "errcode": "M_MISSING_TOKEN",
	  "error": "Missing access token"
	})"_json;

    err = data2.get<Error>();

    EXPECT_EQ(err.errcode, ErrorCode::M_MISSING_TOKEN);
    EXPECT_EQ(err.error, "Missing access token");
}
