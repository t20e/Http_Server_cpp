#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../mocks.h"
#include "Config.h"
#include "gmock/gmock.h"
#include "request_handlers/AuthHandler.h"
#include "request_handlers/UserHandler.h"
#include "utils/http_data.h"

using ::testing::_;
using testing::Exactly;
using ::testing::Return;

TEST(AuthHandlerTest, RegisterUser)
{
	static Config config = Config::load(".env");

	// Setup mocks
	MockDB_controller mockDB;
	MockIResponseService mockResponse;

	AuthHandler authHandler(config, mockDB, mockResponse);

	// Setup registerUser() required arguments
	HttpRequest req;
	req.headers["Origin"] = "http://localhost:3000";
	req.client_ip = "127.0.01";
	req.body = "username=johnDoe&password=test%40"; // {'username':'johnDoe', 'password':'test@'}
	int client_socket = 12;

	// 1. After it extracts the data from request body, it calls db addUser()
	User user{1, "johnDoe", ""}; // The data that addUser() should return.
	EXPECT_CALL(mockDB, addUser)
		.Times(1)
		.WillOnce(Return(user));

	// WHAT ABOUT HERE? Then it calls createJwtSessionToken, createJwtSessionToken is a 'real' method of authHandler. 
	// std::string returnTokenStr = "eyJhbGciOiJIUz...";
	// EXPECT_CALL(, call)

	// 2. Then calls createResponse()
	std::string mockHttpString = "HTTP/1.1 201 OK\r\n...";

	EXPECT_CALL(mockResponse, createResponse(testing::HasSubstr("johnDoe"), // Check is body, json contains username "tom"
											 201, // Status code
											 "http://localhost:3000", // Browser Origin
											 _, _, _ // Use function defaults/dont care about these arguments.
											 ))
		.Times(Exactly(1))
		.WillOnce(Return(mockHttpString));


	// 3. Then it calls senResponse()
	EXPECT_CALL(mockResponse, sendResponse(mockHttpString, client_socket))
		.Times(Exactly(1))
		.WillOnce(Return(0));

    // Call method
    authHandler.registerUser(req, client_socket);
}
