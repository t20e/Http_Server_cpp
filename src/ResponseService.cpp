#include <bcrypt.h>
#include <cstddef>
#include <expected>
#include <format>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>


#include "ResponseService.h"
#include "Config.h"
#include "utils/JsonResponse.h"
#include "utils/Logger.h"
#include "utils/http_data.h"


ResponseService::ResponseService(Config &config)
	: config_(config) {}


void ResponseService::send404(const HttpRequest &req, const int &clientSocket)
{
	JsonResponse json;
	json.add("Error", "Not Found!");

	std::string res = createResponse(json.dump(), 404, req.headers.at("Origin"));
	sendResponse(res, clientSocket);
}

void ResponseService::sendForbidden(const HttpRequest &req, const int &clientSocket)
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


std::string ResponseService::createResponse(const std::string &body, int status, const std::string &origin, const std::string &content_type, const std::string &jwt_token, const bool &delJwtToken)
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

int ResponseService::sendResponse(std::string res, const int &clientSocket)
{
	const char *buffer = res.c_str();
	size_t len = res.length();

	ssize_t bytes_sent = send(clientSocket, buffer, len, 0);
	if (bytes_sent == -1) {
		Logger::getInstance().log(LogLevel::ERROR, "Error sending client data.");
		return -1;
	} 
    // Cast to suppress compiler warning.
    size_t bytes_sent_cast = static_cast<size_t>(bytes_sent);

     if (bytes_sent_cast < len) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Warning: Only sent: {} of {} bytes in the http response!", bytes_sent, len));
	}
	close(clientSocket);
	return 0;
}

int ResponseService::handlePreflight(const HttpRequest &req, const int &clientSocket)
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
