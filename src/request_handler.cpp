#include <format>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

#include "request_handler.h"
#include "utils/json_parsing.h"
#include "utils/json_response.h"
#include "config.h"

using std::cout;

 
int RequestHandler::handle_test_get(const HttpRequest &req, int clientSocket)
{
	std::string res = createResponse("bodyTEST", 200);
	sendRes(res, clientSocket);
	return 0;
}

int RequestHandler::login(const HttpRequest &req, int clientSocket)
{
	std::unordered_map<std::string, std::string> formData = parseJsonStringFromBody(req.body);
	std::string username = formData["username"];
	std::string password = formData["password"];
    std::cout << std::format("\n\nUsername: {}, Password: {}", username, password);

	std::string res = createResponse("bodyTEST", 200);
	sendRes(res, clientSocket);
	return 0;
}

int RequestHandler::reg(const HttpRequest &req, int clientSocket)
{
	std::unordered_map<std::string, std::string> formData = parseJsonStringFromBody(req.body);
	std::string username = formData["username"];
	std::string password = formData["password"];
    std::cout << std::format("\n\nUsername: {}, Password: {}", username, password);

	std::string res = createResponse("bodyTEST", 200);
	sendRes(res, clientSocket);
	return 0;
}

void RequestHandler::send404(const int &clientSocket)
{
	JsonResponse json;
	json.add("Error", "Not Found");

	std::string res = createResponse(json.dump(), 404);
	sendRes(res, clientSocket);
}


std::string RequestHandler::createResponse(const std::string &body, int status, const std::string &content_type)
{
	// EXAMPLE JSON BODY: "{\"Name\": \"John\"}"

	std::string code_phrase = getCodePhrase(status);
	return std::format(
		"HTTP/1.1 {} {}\r\n"
		"Content-Type: {}\r\n"
		"Content-Length: {}\r\n"
		"\r\n"
		"{}",
		status, code_phrase, content_type, body.length(), body);
}


int RequestHandler::sendRes(std::string res, const int &clientSocket)
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
