// TODO

// #include <gmock/gmock.h>
// #include <gtest/gtest.h>
// #include <string>
// #include "gmock/gmock.h"
// #include "request_handlers/AuthHandler.h"


// /**
//  * @brief Mock class for the AuthHandler class.
//  * 
//  */
// class MockAuthHandler : public AuthHandler {
// 	public:

// 		virtual ~MockAuthHandler() = default;

// 		MOCK_METHOD(std::string, createJwtSessionToken, (const int &user_id), (override));

// 		// MOCK_METHOD(int, getUserByToken, (const HttpRequest &req, const int &client_socket), (override));

// 		// MOCK_METHOD(bool, authenticateSessionToken, (const HttpRequest &req), (override));

// 		// MOCK_METHOD(int, logout, (const HttpRequest &req, const int &client_socket), (override));

// 		// MOCK_METHOD(int, login, (const HttpRequest &req, int clientSocket), (override));

// 		// MOCK_METHOD(int, registerUser, (const HttpRequest &req, int clientSocket), (override));
// };


// TEST(TestAuthHandler, Register)
// {

// 	static Config config = Config::load(".env");

// 	DB_controller db("database.db");

// 	ResponseService responseService(config);

// 	MockAuthHandler mockAuthHandler(config, db, responseService);
// }
