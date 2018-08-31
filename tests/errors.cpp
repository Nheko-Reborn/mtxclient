#include <gtest/gtest.h>

#include <mtx/errors.hpp>

using namespace mtx::errors;

TEST(MatrixErrors, BasicError)
{
        json data = R"({
	  "errcode": "M_UNRECOGNIZED",
	  "error": "Unrecognized request"
	})"_json;

        Error err = data;

        EXPECT_EQ(err.errcode, ErrorCode::M_UNRECOGNIZED);
        EXPECT_EQ(err.error, "Unrecognized request");

        json data2 = R"({
	  "errcode": "M_MISSING_TOKEN",
	  "error": "Missing access token"
	})"_json;

        err = data2;

        EXPECT_EQ(err.errcode, ErrorCode::M_MISSING_TOKEN);
        EXPECT_EQ(err.error, "Missing access token");
}
