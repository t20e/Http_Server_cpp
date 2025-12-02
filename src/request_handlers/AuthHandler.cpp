#include <bcrypt.h>
#include <chrono>
#include <exception>
#include <expected>
#include <format>
#include <iostream>
#include <regex>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>

#include "./request_handlers/AuthHandler.h"
#include "DB_controller.h"
#include "ResponseService.h"
#include "config.h"
#include "utils/JsonResponse.h"
#include "utils/Logger.h"
#include "utils/URL_encoded_parsing.h"
#include "utils/http_data.h"
#include "utils/result_types.h"


using std::cout;


AuthHandler::AuthHandler(
	Config &config,
	DB_controller &db,
	ResponseService &responseService)
	: BaseHandler(config, db, responseService) {}


int AuthHandler::login(const HttpRequest &req, int clientSocket)
{

	std::unordered_map<std::string, std::string> formData = parseURLEncodedBody(req.body);
	std::string inputUsername = formData["username"];
	std::string inputPassword = formData["password"];
	// std::cout << std::format("\nUsername: {}, Password: {}\n", inputUsername, inputPassword);

	Logger::getInstance().log(LogLevel::INFO, std::format("Attempting to login a user, user's IP: {}, username: {}", req.client_ip, inputUsername));

	JsonResponse json;

	DbResult dbResult = db_.getUser(inputUsername);

	if (!dbResult.has_value()) {
		json.add("Error", dbResult.error());
		std::string res = resService_.createResponse(json.dump(), 404, req.headers.at("Origin"));
		resService_.sendResponse(res, clientSocket);
		return 1;
	}

	// cout << std::format("\nValidating inputPassword: {} | against | hashedPassword: {}\n\n", inputPassword, dbResult->passwordHash);

	// Compare the inputPassword with the hashedPassword
	if (!bcrypt::validatePassword(inputPassword, dbResult->passwordHash)) {
		Logger::getInstance().log(LogLevel::WARNING, std::format("Failed login attempt from IP: {} | For username: {} | User's input password doesn't match the hashedPassword.", req.client_ip, inputUsername));
		json.add("Error", "Username or password is incorrect!");

		std::string res = resService_.createResponse(json.dump(), 401, req.headers.at("Origin"));
		resService_.sendResponse(res, clientSocket);
		return 1;
	}

	// Create JWT Token
	std::string jwtToken = createJwtSessionToken(dbResult->userId);

	// Send the back the users data
	json.add("username", dbResult->username);
	json.add("userID", dbResult->userId);

	resService_.sendResponse(
		resService_.createResponse(
			json.dump(),
			200,
			req.headers.at("Origin"),
			"application/json",
			jwtToken),
		clientSocket);
	return 0;
}

int AuthHandler::registerUser(const HttpRequest &req, int clientSocket)
{
	Logger::getInstance().log(LogLevel::INFO, "Registering user...");
	std::unordered_map<std::string, std::string> formData = parseURLEncodedBody(req.body);
	std::string username = formData["username"];
	std::string password = formData["password"];

	JsonResponse json;

	// Validate form
	if (username.length() == 0 || password.length() == 0) {
		json.add("Error", "Missing username or password.");
		std::string res = resService_.createResponse(json.dump(), 400, req.headers.at("Origin"));
		resService_.sendResponse(res, clientSocket);
		return 1;
	}

	if (username.length() < 3 || username.length() > 32) {
		json.add("Error", "Username length must be between 3 and 32 characters!.");
		std::string res = resService_.createResponse(json.dump(), 400, req.headers.at("Origin"));
		resService_.sendResponse(res, clientSocket);
		return 1;
	}

	if (password.length() < 5 || password.length() > 32) {
		json.add("Error", "password length must be between 5 and 32 characters!.");
		std::string res = resService_.createResponse(json.dump(), 400, req.headers.at("Origin"));
		resService_.sendResponse(res, clientSocket);
		return 1;
	}

	static const std::regex letters_only_regex("^[a-zA-Z]+$");

	if (!std::regex_match(username, letters_only_regex)) {
		cout << "Username contains non-letters! Username: " << username << std::endl;
		json.add("Error", "Username must be letters only with no white spaces!.");
		std::string res = resService_.createResponse(json.dump(), 400, req.headers.at("Origin"));
		resService_.sendResponse(res, clientSocket);
		return 1;
	}

	std::string hashedPassword = bcrypt::generateHash(password);

	// std::cout << std::format("\nUsername: {}, Password: {}, Hashed Password: {}\n", username, password, hashedPassword);

	DbResult dbResult = db_.addUser(username, hashedPassword);

	if (!dbResult.has_value()) {
		json.add("Error", dbResult.error());
		int statusCode = (dbResult.error() == "Username is taken!") ? 409 : 500;
		std::string res = resService_.createResponse(json.dump(), statusCode, req.headers.at("Origin"));
		resService_.sendResponse(res, clientSocket);
		return 1;
	}

	// Create JWT Token
	std::string jwtToken = createJwtSessionToken(dbResult->userId);

	// Send the back the users data
	json.add("username", username);
	json.add("userID", dbResult.value().userId);

	resService_.sendResponse(
		resService_.createResponse(
			json.dump(),
			201,
			req.headers.at("Origin"),
			"application/json",
			jwtToken),
		clientSocket);
	return 0;
}


std::string AuthHandler::createJwtSessionToken(const int &user_id)
{
	const jwt::date token_valid_for = std::chrono::system_clock::now() + std::chrono::seconds{3600 * 24}; // valid for 24 hours

	std::string token = jwt::create()
							.set_type("JWS")
							.set_issuer("auth0")
							.set_subject(std::to_string(user_id))
							.set_payload_claim("role", jwt::claim(std::string("user")))
							.set_issued_at(std::chrono::system_clock::now())
							.set_expires_at(token_valid_for)
							.sign(jwt::algorithm::hs256{config_.JWT_SECRET_KEY});
	Logger::getInstance().log(LogLevel::INFO, std::format("Created new token: {}", token));

	return token;
}

int AuthHandler::logout(const HttpRequest &req, const int &client_socket)
{
	Logger::getInstance().log(LogLevel::INFO, std::format("Logging user out. User's IP: {}", req.client_ip));
	resService_.sendResponse(resService_.createResponse("", 200, req.headers.at("Origin"), "application/json", "", true), client_socket);
	return 0;
}

std::string AuthHandler::extractJwtToken(std::string cookie)
{
	size_t pos = cookie.find("session_token=");

	if (pos != std::string::npos) {
		pos += 14; // remove "session_token="
		size_t end = cookie.find(";", pos);
		std::string token = cookie.substr(pos, end - pos);
		// std::cout << std::format("Token: '{}'\n", token);
		return token;
	}
	return "";
}

int AuthHandler::getUserByToken(const HttpRequest &req, const int &client_socket)
{
    Logger::getInstance().log(LogLevel::INFO, std::format("Checking User session token, IP: {}", req.client_ip));
	std::string token = "";

	if (req.headers.count("Cookie")) {
		std::string cookie = req.headers.at("Cookie");
		token = extractJwtToken(cookie);
	}

	JsonResponse json;

	// Verify
	if (token.empty()) {
        Logger::getInstance().log(LogLevel::WARNING, std::format("No session token provided, user's IP: {}", req.client_ip));
		json.add("message", "No session token provided");
		resService_.sendResponse(resService_.createResponse(json.dump(), 401, req.headers.at("Origin")), client_socket);
		return 1;
	}

	auto validateTokenRes = verifyJwtSessionToken(token);

	if (validateTokenRes.has_value()) {
		int userId = *validateTokenRes;

		DbResult user = db_.getUser(userId);

		if (user.has_value()) {
			// Send the back the users data
			json.add("username", user->username);
			json.add("userID", user->userId);

			resService_.sendResponse(
				resService_.createResponse(
					json.dump(),
					200,
					req.headers.at("Origin")),
				client_socket);

			return 0;
		} else {
			return 1;
		}
	}

	// Invalid
	json.add("message", "Session cookie is Invalid");
	resService_.sendResponse(resService_.createResponse(json.dump(), 401, req.headers.at("Origin")), client_socket);
	return 1;
}

std::expected<int, std::string> AuthHandler::verifyJwtSessionToken(std::string &token)
{
	try {
		auto decoded_token = jwt::decode(token);

		auto verifier = jwt::verify()
							.with_issuer("auth0")
							.allow_algorithm(jwt::algorithm::hs256{config_.JWT_SECRET_KEY});

		verifier.verify(decoded_token);

		return std::stoi(decoded_token.get_subject());

	} catch (const std::exception &e) {
		return std::unexpected(std::format("JWT token is Invalid: Error: {} | ", e.what()));
	}
}

bool AuthHandler::authenticateSessionToken(const HttpRequest &req)
{
	std::string logMsg = std::format("User's IP: {} | ", req.client_ip);
	bool returnType = false;

	if (req.headers.count("Cookie")) {
		std::string cookie = req.headers.at("Cookie");
		std::string token = extractJwtToken(cookie);

		if (token.empty()) {
			logMsg += "No session token provided";
			returnType = false;
		}

		auto validateTokenRes = verifyJwtSessionToken(token);

		if (validateTokenRes.has_value()) { // has the int
			logMsg += "Session token is valid.";
			returnType = true;
		} else {
			logMsg += "Session token is inValid";
			returnType = false;
		}
	} else {
		logMsg += "No session token available";
		returnType = false;
	}

	Logger::getInstance().log(LogLevel::INFO, logMsg);
	return returnType;
}
