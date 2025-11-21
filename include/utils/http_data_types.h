#pragma once
#include <functional>
#include <string>


/**
 * @brief Struct to store the parsed components of an incoming HTTP request.
 * 
 */
struct HttpRequest {
	std::string method;
	std::string path;
	std::string http_version;
	int content_length = 0;
	std::string content_type;

	std::string body;
};

/**
 * @brief Accepts the parse request and the client socket ID
 * 
 */
using HttpHandler = std::function<void(const HttpRequest &req, const int &clientSocket)>;

/**
 * @brief Get the Reason Phrase for a response status code.
 * 
 * @param status 
 * @return std::string 
 */
inline std::string getCodePhrase(int status)
{
	switch (status) {
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 400:
			return "Bad Request";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 500:
			return "Internal Server Error";
		default:
			return "Unknown Status";
	}
}
