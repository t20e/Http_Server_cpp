#include <bcrypt.h>
#include <chrono>
#include <exception>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <random>
#include <regex>
#include <set>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>


#include "./RequestHandler.h"
#include "config.h"
#include "utils/JsonResponse.h"
#include "utils/error_handling_template.h"
#include "utils/http_data_types.h"
#include "utils/urlEncoded_parsing.h"

using std::cout;

RequestHandler::RequestHandler(SQLiteDB &db, Config &config)
	: db_(db), config_(config) {}


int RequestHandler::login(const HttpRequest &req, int clientSocket)
{
    std::cout << "Logging in user...\n";
	std::unordered_map<std::string, std::string> formData = parseURLEncodedBody(req.body);
	std::string inputUsername = formData["username"];
	std::string inputPassword = formData["password"];
	// std::cout << std::format("\nUsername: {}, Password: {}\n", inputUsername, inputPassword);

	JsonResponse json;

	DbResult dbResult = db_.getUser(inputUsername);

	if (!dbResult.has_value()) {
		json.add("Error", dbResult.error());
		std::string res = createResponse(json.dump(), 404, req.headers.at("Origin"));
		sendResponse(res, clientSocket);
		return 1;
	}

	// cout << std::format("\nValidating inputPassword: {} | against | hashedPassword: {}\n\n", inputPassword, dbResult->passwordHash);

	// Compare the inputPassword with the hashedPassword
	if (!bcrypt::validatePassword(inputPassword, dbResult->passwordHash)) {
		cout << "User's input password doesn't match the hashedPassword.\n";
		json.add("Error", "Username or password is incorrect!");
		std::string res = createResponse(json.dump(), 401, req.headers.at("Origin"));
		sendResponse(res, clientSocket);
		return 1;
	}

	// Create JWT Token
	std::string jwtToken = createJwtSessionToken(dbResult->userId);

	// Send the back the users data
	json.add("username", dbResult->username);
	json.add("userID", dbResult->userId);

	sendResponse(
		createResponse(
			json.dump(),
			200,
			req.headers.at("Origin"),
			"application/json",
			jwtToken),
		clientSocket);
	return 0;
}

int RequestHandler::registerUser(const HttpRequest &req, int clientSocket)
{
    std::cout << "Registering user...\n";
	std::unordered_map<std::string, std::string> formData = parseURLEncodedBody(req.body);
	std::string username = formData["username"];
	std::string password = formData["password"];

	JsonResponse json;

	// Validate form
	if (username.length() == 0 || password.length() == 0) {
		json.add("Error", "Missing username or password.");
		std::string res = createResponse(json.dump(), 400, req.headers.at("Origin"));
		sendResponse(res, clientSocket);
		return 1;
	}

	if (username.length() < 3 || username.length() > 32) {
		json.add("Error", "Username length must be between 3 and 32 characters!.");
		std::string res = createResponse(json.dump(), 400, req.headers.at("Origin"));
		sendResponse(res, clientSocket);
		return 1;
	}

	if (password.length() < 5 || password.length() > 32) {
		json.add("Error", "password length must be between 5 and 32 characters!.");
		std::string res = createResponse(json.dump(), 400, req.headers.at("Origin"));
		sendResponse(res, clientSocket);
		return 1;
	}

	static const std::regex letters_only_regex("^[a-zA-Z]+$");

	if (!std::regex_match(username, letters_only_regex)) {
		cout << "Username contains non-letters! Username: " << username << std::endl;
		json.add("Error", "Username must be letters only with no white spaces!.");
		std::string res = createResponse(json.dump(), 400, req.headers.at("Origin"));
		sendResponse(res, clientSocket);
		return 1;
	}

	std::string hashedPassword = bcrypt::generateHash(password);

	// std::cout << std::format("\nUsername: {}, Password: {}, Hashed Password: {}\n", username, password, hashedPassword);

	DbResult dbResult = db_.addUser(username, hashedPassword);

	if (!dbResult.has_value()) {
		json.add("Error", dbResult.error());
		int statusCode = (dbResult.error() == "Username is taken!") ? 409 : 500;
		std::string res = createResponse(json.dump(), statusCode, req.headers.at("Origin"));
		sendResponse(res, clientSocket);
		return 1;
	}

	// Create JWT Token
	std::string jwtToken = createJwtSessionToken(dbResult->userId);

	// Send the back the users data
	json.add("username", username);
	json.add("userID", dbResult.value().userId);

	sendResponse(
		createResponse(
			json.dump(),
			201,
			req.headers.at("Origin"),
			"application/json",
			jwtToken),
		clientSocket);
	return 0;
}

void RequestHandler::send404(const HttpRequest &req, const int &clientSocket)
{
	JsonResponse json;
	json.add("Error", "Not Found!");

	std::string res = createResponse(json.dump(), 404, req.headers.at("Origin"));
	sendResponse(res, clientSocket);
}

void RequestHandler::sendForbidden(const HttpRequest &req, const int &clientSocket)
{
	JsonResponse json;
	json.add("Error", "Forbidden!");

	std::string origin = "";
	if (req.headers.count("Origin")) {
		origin = req.headers.at("Origin");
	}

	std::string res = createResponse(json.dump(), 403, origin);
	sendResponse(res, clientSocket);
}


std::string RequestHandler::createResponse(const std::string &body, int status, const std::string &origin, const std::string &content_type, const std::string &jwt_token, const bool &delJwtToken)
{
	// Example json string body: "{\"name\": \"john\"}"

	std::string code_phrase = getCodePhrase(status);
	std::string jwt_token_cookie = "";

	// In production only allow cookies when using HTTPS for more security.
	const bool use_secure_cookie = config_.DEV_MODE ? false : true;

	if (!jwt_token.empty()) {
		// Note: Javascript won't be able to read this, which is more secure! The frontend browser will handle this cookie automatically!
		jwt_token_cookie = std::format(
			"Set-Cookie: session_token={}; HttpOnly; Path=/; Max-Age=43200; "
			// JWT token last 3600*12=43200 = one day
			"SameSite=Lax;{}"
			"\r\n",
			jwt_token,
			use_secure_cookie ? " Secure;" : "");
	}

	if (delJwtToken) {
		jwt_token_cookie = std::format(
			"Set-Cookie: session_token=deleted; HttpOnly; Path=/; Max-Age=0; "
			"SameSite=Lax;{}"
			"\r\n",
			use_secure_cookie ? " Secure;" : "");
	}


	return std::format(
		"HTTP/1.1 {} {}\r\n"
		"{}" // Insert jwt token cookie if preset
		"Content-Type: {}\r\n"
		"Content-Length: {}\r\n"
		// ----- Headers
		"Access-Control-Allow-Origin: {}\r\n" // Example: http://localhost:3000
		"Access-Control-Allow-Credentials: true\r\n" // allow cookies
		"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
		"Access-Control-Allow-Headers: Content-Type\r\n" // allow json content
		// -----
		"\r\n"
		"{}",
		status,
		code_phrase,
		jwt_token_cookie,
		content_type,
		body.length(),
		origin,
		body);
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
		std::string res = std::format("HTTP/1.1 200 OK\r\n"
									  "Access-Control-Allow-Origin: {}\r\n"
									  "Access-Control-Allow-Credentials: true\r\n"
									  "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
									  "Access-Control-Allow-Headers: Content-Type\r\n"
									  "Content-Length: 0\r\n",
									  req.headers.at("Origin"));
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
		sendResponse(createResponse(response.dump(), 404, req.headers.at("Origin")), clientSocket);
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

	sendResponse(createResponse(response.dump(), 200, req.headers.at("Origin")), clientSocket);
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
			sendResponse(createResponse("No images found", 500, req.headers.at("Origin")), client_socket);
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
			sendResponse(createResponse("Server Error: could not retrieve image", 500, req.headers.at("Origin")), client_socket);
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
			"Access-Control-Allow-Origin: {}\r\n" // Connection to frontend
			"Access-Control-Allow-Credentials: true\r\n" // allow cookies
			"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
			"Access-Control-Allow-Headers: Content-Type\r\n" // allow json content
			// -----
			"Connection: close\r\n"
			"\r\n",
			content_type,
			image_buffer.size(),
			req.headers.at("Origin")

		);

		// Respond
		// Send the preflight
		send(client_socket, headers.c_str(), headers.size(), 0);
		// Send the POST image buffer data
		send(client_socket, image_buffer.data(), image_buffer.size(), 0);
		return 0;

	} catch (const std::filesystem::filesystem_error &err) {
		std::cerr << std::format("Unknown error: {}\n", err.what());
		sendResponse(createResponse("Server error: getting a random image.", 500, req.headers.at("Origin")), client_socket);
	} catch (const std::exception &err) {
		std::cerr << std::format("Unknown error: {}\n", err.what());
		sendResponse(createResponse("Unknown Server error: getting a random image.", 500, req.headers.at("Origin")), client_socket);
	}
	return 0;
}


std::string RequestHandler::createJwtSessionToken(const int &user_id)
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
	std::cout << "Created new token: " << token << std::endl;

	return token;
}

int RequestHandler::logout(const HttpRequest &req, const int &client_socket)
{
	cout << "Logging the user out..\n";
	sendResponse(createResponse("", 200, req.headers.at("Origin"), "application/json", "", true), client_socket);
	return 0;
}

std::string RequestHandler::extractJwtToken(std::string cookie)
{
	size_t pos = cookie.find("session_token=");

	if (pos != std::string::npos) {
		pos += 14; // remove "session_token="
		size_t end = cookie.find(";", pos);
		std::string token = cookie.substr(pos, end - pos);
		std::cout << std::format("Token: '{}'\n", token);
		return token;
	}
	return "";
}

int RequestHandler::getLoggedUser(const HttpRequest &req, const int &client_socket)
{
	cout << "Checking User session token...\n";
	std::string token = "";

	if (req.headers.count("Cookie")) {
		std::string cookie = req.headers.at("Cookie");
		token = extractJwtToken(cookie);
	}

	JsonResponse json;

	// Verify
	if (token.empty()) {
		std::cout << "No session token provided\n";
		json.add("message", "No session token provided");
		sendResponse(createResponse(json.dump(), 401, req.headers.at("Origin")), client_socket);
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

			sendResponse(
				createResponse(
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
	sendResponse(createResponse(json.dump(), 401, req.headers.at("Origin")), client_socket);
	return 1;
}

std::optional<int> RequestHandler::verifyJwtSessionToken(std::string &token)
{
	try {
		auto decoded_token = jwt::decode(token);

		auto verifier = jwt::verify()
							.with_issuer("auth0")
							.allow_algorithm(jwt::algorithm::hs256{config_.JWT_SECRET_KEY});

		verifier.verify(decoded_token);

		std::cout << "JWT Token is valid!\n";

		std::cout << std::format("Role claim: {}, User's ID: {}\n", decoded_token.get_payload_claim("role").as_string(), decoded_token.get_subject());
		return std::stoi(decoded_token.get_subject());

	} catch (const std::exception &e) {
		std::cerr << std::format("JWT token is Invalid: Error: {}\n", e.what());
		return std::nullopt;
	}
}

bool RequestHandler::authenticateSessionToken(const HttpRequest &req)
{
	if (req.headers.count("Cookie")) {
		std::string cookie = req.headers.at("Cookie");
		std::string token = extractJwtToken(cookie);

		if (token.empty()) {
			std::cout << "No session token provided\n";
			return false;
		}

		auto validateTokenRes = verifyJwtSessionToken(token);
		if (validateTokenRes.has_value()) {
			cout << "Session token is valid\n";
			return true;
		} else {
			cout << "Session token is inValid\n";
			return false;
		}
	} else {
		cout << "No session token available\n";
		return false;
	}
}
