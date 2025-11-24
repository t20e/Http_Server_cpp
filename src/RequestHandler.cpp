#include <bcrypt.h>
#include <exception>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>


#include "./RequestHandler.h"
#include "utils/JsonResponse.h"
#include "utils/error_handling_template.h"
#include "utils/http_data_types.h"
#include "utils/urlEncoded_parsing.h"

using std::cout;

RequestHandler::RequestHandler(SQLiteDB &db)
	: db_(db) {}


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


int RequestHandler::getAllUser(const HttpRequest &req, const int &clientSocket)
{
	cout << "Getting all users\n";

	JsonResponse response;
	DbListResult db_result = db_.getAllUsers();

	// check for database error
	if (!db_result.has_value()) {
		response.add("Error", db_result.error());
		sendResponse(createResponse(response.dump(), 404), clientSocket);
		return 1;
	}

	std::vector<JsonResponse> usersVector;

	// Add each user to the Json
	for (User user: *db_result) {
		JsonResponse userObj;
		// cout << std::format("User: {}\n", user.username);
		userObj.add("userID", user.userId);
		userObj.add("username", user.username);
		usersVector.push_back(userObj);
	}

	response.add("users", usersVector);
	cout << "Response: " << response.dump() << std::endl;

	sendResponse(createResponse(response.dump(), 200), clientSocket);
	return 0;
}

int RequestHandler::getRandomImage(const HttpRequest &req, const int &client_socket)
{
	std::string images_directory_path = "./server_images";

	std::set<std::string> img_extensions = {
		".jpg", ".jpeg", ".png"};

	try {
		std::vector<std::filesystem::path> found_images;

		// Iterate over the directory and collect valid image paths
		for (const auto &entry: std::filesystem::directory_iterator(images_directory_path)) {
			if (entry.is_regular_file()) {
				std::string ext = entry.path().extension().string();
				if (img_extensions.count(ext)) { // Match to an image extension
					found_images.push_back(entry.path());
				}
			}
		}

		// Handle case where no images are found.
		if (found_images.empty()) {
			std::cerr << std::format("No images found in: {}\n", images_directory_path);
			sendResponse(createResponse("No images found", 4904), client_socket);
			return 1;
		}

		// Select a random image
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, found_images.size() - 1);

		std::filesystem::path selected_file = found_images[distrib(gen)];
		cout << std::format("Random Image Path: {}\n", selected_file.string());

		// Open the image file in binary mode
		std::ifstream file(selected_file, std::ios::binary);

		if (!file.is_open()) {
			sendResponse(createResponse("Server Error: could not retrieve image", 500), client_socket);
			return 1;
			;
		}

		// Read the entire file into a string buffer
		std::string image_buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		// Determine the image extension content-type to create a response object
		std::string image_ext = selected_file.extension().string();
		std::string content_type = "image/jpeg"; // default

		if (image_ext == ".png")
			content_type = "image/png";

		// Construct response
		std::string headers = std::format(
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: {}\r\n"
			"Content-Length: {}\r\n"
			// ----- Headers
			"Access-Control-Allow-Origin: http://localhost:3000\r\n" // Connection to frontend
			"Access-Control-Allow-Credentials: true\r\n" // allow cookies
			"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
			"Access-Control-Allow-Headers: Content-Type\r\n" // allow json content
			// -----
			"Connection: close\r\n"
			"\r\n",
			content_type,
			image_buffer.size());

		// Respond
		// Send the preflight
		send(client_socket, headers.c_str(), headers.size(), 0);
		// Send the POST image buffer data
		send(client_socket, image_buffer.data(), image_buffer.size(), 0);
		return 0;

	} catch (const std::filesystem::filesystem_error &err) {
		std::cerr << std::format("Unknown error: {}\n", err.what());
		sendResponse(createResponse("Server error: getting a random image.", 500), client_socket);
	} catch (const std::exception &err) {
		std::cerr << std::format("Unknown error: {}\n", err.what());
		sendResponse(createResponse("Unknown Server error: getting a random image.", 500), client_socket);
	}
	return 0;
}
