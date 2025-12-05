#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../mocks.h"
#include "Config.h"
#include "gmock/gmock.h"
#include "request_handlers/UserHandler.h"
#include "utils/http_data.h"

using ::testing::_;
using testing::Exactly;
using ::testing::Return;

TEST(UserHandlerTest, GetAllUsers)
{
	static Config config = Config::load(".env");

	// Setup mocks
	MockDB_controller mockDB;
	MockIResponseService mockResponse;

	UserHandler userHandler(config, mockDB, mockResponse);


	HttpRequest req;
	req.headers["Origin"] = "http://localhost:3000";
	req.client_ip = "127.0.01";
	int client_socket = 12;

	// Define Exceptions of how the function UserHandler::getAllUser should behave:

	// 1. Asks the DB for users
	std::vector<User> fakeUsers; // Setup dummy data that getAllUsers() should return.
	fakeUsers.push_back({1, "tom", "tom's password"});
	fakeUsers.push_back({2, "dan", "dan's password"});

	EXPECT_CALL(mockDB, getAllUsers())
		.Times(1) // Ensure getAllUsers() is only called once
		.WillOnce(Return(fakeUsers)); // Forces it to return fake users.


	// 2. Then calls createResponse()
	std::string mockHttpString = "HTTP/1.1 200 OK\r\n...";

	EXPECT_CALL(mockResponse, createResponse(testing::HasSubstr("tom"), // Check is body, json contains username "tom"
											 200, // Status code
											 "http://localhost:3000", // Browser Origin
											 _, _, _ // Use function defaults/dont care about these arguments.
											 ))
		.Times(Exactly(1))
		.WillOnce(Return(mockHttpString));


	// 3. Then it calls senResponse()
	EXPECT_CALL(mockResponse, sendResponse(mockHttpString, client_socket))
		.Times(Exactly(1))
		.WillOnce(Return(0));

	// Run the code
	userHandler.getAllUser(req, client_socket);
}
