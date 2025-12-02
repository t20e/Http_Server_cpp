
#include <gtest/gtest.h>
#include <string>

#include "gtest/gtest.h"
#include "utils/JsonResponse.h"


TEST(JsonResponseTest, VerifyJsonDumpAgainstVariousInputs)
{
	JsonResponse json;
	json.add("TestingKey", "TestingValue");
	ASSERT_EQ(json.dump(), "{\"TestingKey\": \"TestingValue\"}");
}
