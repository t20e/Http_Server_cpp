#include <bcrypt.h>
#include <expected>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>


#include "./request_handlers/UserHandler.h"
#include "DB_controller.h"
#include "ResponseService.h"
#include "config.h"
#include "request_handlers/BaseHandler.h"
#include "utils/JsonResponse.h"
#include "utils/Logger.h"
#include "utils/http_data.h"
#include "utils/result_types.h"


UserHandler::UserHandler(
	Config &config,
	DB_controller &db,
	ResponseService &responseService)
	: BaseHandler(config, db, responseService) {}


int UserHandler::getAllUser(const HttpRequest &req, const int &clientSocket)
{
	Logger::getInstance().log(LogLevel::INFO, std::format("Getting all users for IP: {}", req.client_ip));

	JsonResponse response;
	DbListResult db_result = db_.getAllUsers();

	// check for database error
	if (!db_result.has_value()) {
		response.add("Error", db_result.error());
		resService_.sendResponse(resService_.createResponse(response.dump(), 404, req.headers.at("Origin")), clientSocket);
		return 1;
	}

	std::vector<JsonResponse> usersVector;

	// Add each user to the JSON
	for (User user: *db_result) {
		JsonResponse userObj;
		// cout << std::format("User: {}\n", user.username);
		userObj.add("userID", user.userId);
		userObj.add("username", user.username);
		usersVector.push_back(userObj);
	}

	response.add("users", usersVector);
	// cout << "Response: " << response.dump() << std::endl;

	resService_.sendResponse(resService_.createResponse(response.dump(), 200, req.headers.at("Origin")), clientSocket);
	return 0;
}
