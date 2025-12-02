#include <gtest/gtest.h>
#include <string>

#include "./utils/URL_encoded_parsing.h"


TEST(URL_encoded_parsing, UrlDecodeTest)
{
	std::string urlEncodedStr = "John%20Doe";
	std::string expectedValue = "John Doe";
	std::string result = urlDecode(urlEncodedStr);

	ASSERT_EQ(result, expectedValue);
}


// Testing parseURLEncodedBody()

struct TestParam {
		std::string raw_body;
		std::unordered_map<std::string, std::string> expected_out;
};

class TestURL_encoded_parsing : public ::testing::TestWithParam<TestParam> {};

TEST_P(TestURL_encoded_parsing, Test_parseURLEncodedBody)
{
	const auto &params = GetParam();

	// Call the function under test
	std::unordered_map<std::string, std::string> actual_out = parseURLEncodedBody(params.raw_body);

	// Assert
	ASSERT_EQ(actual_out, params.expected_out) << "Testing raw body: " << params.raw_body;
}

INSTANTIATE_TEST_SUITE_P(
	TestParseURL_encoded_parsingTests,
	TestURL_encoded_parsing,
	::testing::Values(
		// Case 1: Test with just username
		TestParam{
			"username=John%20Doe",
			{{"username", "John Doe"}}},

		// Case 2: Test with username and & password
		TestParam{
			"username=John%20Doe%20&password=test%40",
			{{"username", "John Doe "},
			 {"password", "test@"}}}


		));
