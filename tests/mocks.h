#include <gmock/gmock.h>
#include "Interfaces.h"
#include "gmock/gmock.h"
#include "utils/result_types.h"

class MockDB_controller : public IDB_controller {
	public:
		MOCK_METHOD(int, createDatabase, (), (override));

		MOCK_METHOD(DbResult, addUser, (std::string username, std::string password), (override));

		MOCK_METHOD(DbResult, getUser, (int userId), (override));

		MOCK_METHOD(DbResult, getUser, (std::string username), (override));

		MOCK_METHOD(DbListResult, getAllUsers, (), (override));
};


class MockIResponseService : public IResponseService {
	public:
		MOCK_METHOD(std::string, createResponse, (const std::string &body, int status, const std::string &origin, const std::string &content_type, const std::string &jwt_token, const bool &delJwtToken), (override));

		MOCK_METHOD(int, sendResponse, (std::string res, const int &clientSocket), (override));

		MOCK_METHOD(int, handlePreflight, (const HttpRequest &req, const int &client_socket), (override));

		MOCK_METHOD(void, send404, (const HttpRequest &req, const int &client_socket), (override));

		MOCK_METHOD(void, sendForbidden, (const HttpRequest &req, const int &client_socket), (override));
 
};
