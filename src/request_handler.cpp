#include <bcrypt.h>
#include <expected>
#include <format>
#include <iostream>
#include <regex>
#include <sys/socket.h>
#include <unistd.h>

#include "request_handler.h"
#include "utils/error_handling_template.h"
#include "utils/http_data_types.h"
#include "utils/json_response.h"
#include "utils/urlEncoded_parsing.h"

using std::cout;

RequestHandler::RequestHandler(SQLiteDB &db)
	: db_(db) {}

int RequestHandler::handle_test_get(const HttpRequest &req, int clientSocket)
{
	cout << req.path;
	std::string res = createResponse("bodyTEST", 200);
	sendResponse(res, clientSocket);
	return 0;
}

int RequestHandler::login(const HttpRequest &req, int clientSocket)
{
	std::unordered_map<std::string, std::string> formData = parseURLEncodedBody(req.body);
	std::string inputUsername = formData["username"];
	std::string inputPassword = formData["password"];
	// std::cout << std::format("\nUsername: {}, Password: {}\n", inputUsername, inputPassword);

	JsonResponse json;

	DbResult dbResult = db_.getUser(inputUsername);

	if (!dbResult.has_value()) {
		json.add("Error", dbResult.error());
		std::string res = createResponse(json.dump(), 404);
		sendResponse(res, clientSocket);
		return 1;
	}

	// cout << std::format("\nValidating inputPassword: {} | against | hashedPassword: {}\n\n", inputPassword, dbResult->passwordHash);

	// Compare the inputPassword with the hashedPassword
	if (!bcrypt::validatePassword(inputPassword, dbResult->passwordHash)) {
		cout << "User's input password doesn't match the hashedPassword.\n";
		json.add("Error", "Username or password is incorrect!");
		std::string res = createResponse(json.dump(), 401);
		sendResponse(res, clientSocket);
		return 1;
	}
	// Send the back the users data
	json.add("username", dbResult->username);
	json.add("userID", dbResult->userId);

	std::string res = createResponse(json.dump(), 200);
	sendResponse(res, clientSocket);
	return 0;
}

int RequestHandler::registerUser(const HttpRequest &req, int clientSocket)
{
	std::unordered_map<std::string, std::string> formData = parseURLEncodedBody(req.body);
	std::string username = formData["username"];
	std::string password = formData["password"];

	JsonResponse json;

	// Validate form
	if (username.length() == 0 || password.length() == 0) {
		json.add("Error", "Missing username or password.");
		std::string res = createResponse(json.dump(), 400);
		sendResponse(res, clientSocket);
		return 1;
	}

	if (username.length() < 3 || username.length() > 32) {
		json.add("Error", "Username length must be between 3 and 32 characters!.");
		std::string res = createResponse(json.dump(), 400);
		sendResponse(res, clientSocket);
		return 1;
	}

	if (password.length() < 5 || password.length() > 32) {
		json.add("Error", "password length must be between 5 and 32 characters!.");
		std::string res = createResponse(json.dump(), 400);
		sendResponse(res, clientSocket);
		return 1;
	}

	static const std::regex letters_only_regex("^[a-zA-Z]+$");

	if (!std::regex_match(username, letters_only_regex)) {
		cout << "Username contains non-letters! Username: " << username << std::endl;
		json.add("Error", "Username must be letters only with no white spaces!.");
		std::string res = createResponse(json.dump(), 400);
		sendResponse(res, clientSocket);
		return 1;
	}

	std::string hashedPassword = bcrypt::generateHash(password);

	// std::cout << std::format("\nUsername: {}, Password: {}, Hashed Password: {}\n", username, password, hashedPassword);

	DbResult dbResult = db_.addUser(username, hashedPassword);

	if (!dbResult.has_value()) {
		json.add("Error", dbResult.error());
		int statusCode = (dbResult.error() == "Username is taken!") ? 409 : 500;
		std::string res = createResponse(json.dump(), statusCode);
		sendResponse(res, clientSocket);
		return 1;
	}

	// Send the back the users data
	json.add("username", username);
	json.add("userID", dbResult.value().userId);

	std::string res = createResponse(json.dump(), 201);
	sendResponse(res, clientSocket);
	return 0;
}

void RequestHandler::send404(const int &clientSocket)
{
	JsonResponse json;
	json.add("Error", "Not Found!");

	std::string res = createResponse(json.dump(), 404);
	sendResponse(res, clientSocket);
}


std::string RequestHandler::createResponse(const std::string &body, int status, const std::string &content_type)
{
	// Example json string body: "{\"name\": \"john\"}"
	std::string code_phrase = getCodePhrase(status);
	return std::format(
		"HTTP/1.1 {} {}\r\n"
		"Content-Type: {}\r\n"
		"Content-Length: {}\r\n"
		// ----- Headers
		"Access-Control-Allow-Origin: http://localhost:3000\r\n" // Connection to frontend
		"Access-Control-Allow-Credentials: true\r\n" // allow cookies
		"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
		"Access-Control-Allow-Headers: Content-Type\r\n" // allow json content
		// -----
		"\r\n"
		"{}",
		status, code_phrase, content_type, body.length(), body);
}

int RequestHandler::sendResponse(std::string res, const int &clientSocket)
{
	const char *buffer = res.c_str();
	size_t len = res.length();

	ssize_t bytes_sent = send(clientSocket, buffer, len, 0);
	if (bytes_sent == -1) {
		std::cerr << "Error sending client data";
		return -1;
	} else if (bytes_sent < len) {
		std::cerr << std::format("Warning: Only sent: {} of {} bytes.\n", bytes_sent, len); //TODO this should be logged.
	}
	close(clientSocket);
	return 0;
}

int RequestHandler::handlePreflight(const HttpRequest &req, const int &clientSocket)
{
	if (req.method == "OPTIONS") {
		std::string res = "HTTP/1.1 200 OK\r\n"
						  "Access-Control-Allow-Origin: http://localhost:3000\r\n"
						  "Access-Control-Allow-Credentials: true\r\n"
						  "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
						  "Access-Control-Allow-Headers: Content-Type\r\n"
						  "Content-Length: 0\r\n";
		send(clientSocket, res.c_str(), res.length(), 0);
		close(clientSocket);
	}
	return 0;
}
