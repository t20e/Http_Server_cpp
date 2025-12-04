#include <iostream>
#include <arpa/inet.h>
#include <cstddef>
#include <ctime>
#include <format>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "Config.h"
#include "Server.h"
#include "ThreadPool.h"
#include "utils/Logger.h"


Server::Server(Config &config, Router &router, ThreadPool &threadPool)
	: config_(config), router_(router), threadPool_(threadPool) {}

int Server::launchServer()
{
	Logger::getInstance().log(LogLevel::INFO, "Launching the HTTP server...");

	// Create a socket
	s_socket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (s_socket_ == -1) {
		Logger::getInstance().log(LogLevel::CRITICAL, "Error creating Socket");
		return -1;
	}

	if (config_.DEV_MODE) {
		// Set SO_REUSEADDR to allow immediate resuse of the port. Only when developing.
		int opt = 1;
		if (setsockopt(s_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			Logger::getInstance().log(LogLevel::CRITICAL, "Set socket option failed");
			return -1;
		}
	}

	// Define server address
	s_address_.sin_family = AF_INET;
	s_address_.sin_port = htons(config_.SERVER_PORT);

	// INADDR_ANY accept connections on any IP
	s_address_.sin_addr.s_addr = INADDR_ANY;

	// Bind socket to address
	if (bind(s_socket_, (struct sockaddr *) &s_address_, sizeof(s_address_)) < 0) {
		Logger::getInstance().log(LogLevel::CRITICAL, "Error binding the server socket to the address.");
		return -1;
	};
	return 0;
}

int Server::Listen()
{
	if (listen(s_socket_, 5) < 0) {
		Logger::getInstance().log(LogLevel::CRITICAL, "Failed to start listening for incoming connections.");
		return -1;
	}

	Logger::getInstance().log(LogLevel::INFO, std::format("{} Server is listening on port: {} {}\n\n", std::string(16, '-'), config_.SERVER_PORT, std::string(16, '-')), 2);

	while (true) {
		// Main thread job is to only accept new connections.
		Logger::getInstance().log(LogLevel::INFO, "Waiting for a new connection...", 2);

		// Capture client IP
		sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		int clientSocket = accept(s_socket_, (sockaddr *) &clientAddr, &clientAddrLen);
		if (clientSocket < 0) {
			Logger::getInstance().log(LogLevel::ERROR, "Error accepting client socket.");
			continue;
		}
		// Convert client IP address to string
		char clientIP_cstr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP_cstr, sizeof(clientIP_cstr));
		std::string clientIP_str(clientIP_cstr);

		// Add IP whitelisting
		bool is_allowed = false;
		// Check if IP is allowed
        if(config_.ALLOWED_IPs.count(clientIP_str)) {
			is_allowed = true;
		}

		if (!is_allowed) {
			Logger::getInstance().log(LogLevel::WARNING, std::format("Rejected connection from IP: {}", clientIP_str));
			close(clientSocket);
			continue;
		}

		Logger::getInstance().log(LogLevel::INFO, std::format("Client Connected, IP: {}", clientIP_str));

		if (config_.ENABLE_MULTI_THREADING) {
			// Hand off connected connection task to a new worker thread, main thread goes back to loop from here.

			try {
				threadPool_.enqueue([this, clientSocket, clientIP_str] {
					this->handleClient(clientSocket, clientIP_str);
				});
			} catch (const std::runtime_error &e) {
				// If the thread pool is stopped, catch errors of adding a new task here
				Logger::getInstance().log(LogLevel::ERROR, std::format("Error why attempting to pass a new HTTP request task to a worker. Error: {} | Server is shutting down, rejecting client.", e.what()));
				close(clientSocket);
			}

		} else { // Single-threaded, main thread performs the task, then goes back to loop.
			this->handleClient(clientSocket, clientIP_str);
		}
	}
}

void Server::handleClient(int clientSocket, std::string clientIP_str)
{
	// Receive date from a client
	char buffer[bufferSizeLimit_] = {0};
	ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

	if (bytesReceived > 0) {
		process_request(buffer, bytesReceived, clientSocket, clientIP_str);
	} else if (bytesReceived == 0) {
		Logger::getInstance().log(LogLevel::WARNING, std::format("Client disconnected abruptly. IP: {}", clientIP_str));
	} else {
		Logger::getInstance().log(LogLevel::ERROR, "recv() call Failed");
	}

	close(clientSocket); // Close the client's socket when finished communicating
}

void Server::process_request(const char *buffer, ssize_t bytesReceived, const int &clientSocket, std::string clientIP_str)
{
	std::string full_request(buffer, bytesReceived); // Convert to string using only the bytes received
	HttpRequest req;

	req.client_ip = clientIP_str; // Add the IP address.

	// Extract (CRUD_Method, Path, http_version) from request line
	std::istringstream request_stream(full_request);
	std::string request_line;
	std::getline(request_stream, request_line);

	std::istringstream line_stream(request_line);
	if (!(line_stream >> req.method >> req.path >> req.http_version)) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Error parsing HTTP request line, IP: {}", req.client_ip));
		return;
	}

	// Extract Headers and content length
	size_t header_end_pos = full_request.find("\r\n\r\n");
	if (header_end_pos == std::string::npos) {
		Logger::getInstance().log(LogLevel::WARNING, std::format("HTTP request is malformed. IP: {}", req.client_ip));
		return;
	}

	std::string headers_only = full_request.substr(0, header_end_pos);
	std::istringstream header_stream(headers_only);
	std::string line;

	std::getline(header_stream, line); // Skip the first line which contains the (CRUD_Method, Path, http_version), since we already parse it above.

	while (std::getline(header_stream, line)) { // read headers line by line
		// HTTP lines end in \r\n, getline() consumes '\n', so we also need to remove the '\r'
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		if (line.empty())
			continue;

		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos) {
			// Get key:value pairs
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);

			// trim spaces from values, e.g., " test" → "test"
			size_t first = value.find_first_not_of(' ');
			if (std::string::npos != first)
				value = value.substr(first);

			// Store in map
			req.headers[key] = value;
		}
	}

	// Extract the request Body from the initial buffer.
	size_t body_start_pos = header_end_pos + 4; // + 4 for "\r\n\r\n"
	if (body_start_pos < full_request.length()) {
		req.body = full_request.substr(body_start_pos);
	}

	// Edge case where the body might be large than the bufferSizeLimit.
	if (static_cast<int>(req.body.length()) < req.content_length) {
		size_t remaining_bytes = req.content_length - req.body.length();

		while (remaining_bytes > 0) {
			char body_chunk[bufferSizeLimit_];
			size_t bytes_to_read = std::min((size_t) sizeof(body_chunk) - 1, remaining_bytes);
			ssize_t body_chunk_recv = recv(clientSocket, body_chunk, bytes_to_read, 0);

			if (body_chunk_recv < 0) {
				Logger::getInstance().log(LogLevel::ERROR, std::format("Error occurred while reading large body. IP: {}", bufferSizeLimit_, req.client_ip));
				break;
			}
			Logger::getInstance().log(LogLevel::INFO, std::format("Client disconnected unexpectedly mid-body transfer. IP: {}", req.client_ip));
			// Append the received chunk
			req.body.append(body_chunk, body_chunk_recv);
			remaining_bytes -= body_chunk_recv; //update the remaining counter
		}
	}

	Logger::getInstance().log(LogLevel::INFO, std::format("Extracted Method: {}, Extracted path: {}, Content-Length: {}", req.method, req.path, req.content_length));

	if (req.method == "POST" && req.body.length() > 0) {
		Logger::getInstance().log(LogLevel::DEBUG, std::format("Request Body Data (UrlEncoded string): {}", req.body));
	}
	router_.route(req, clientSocket);
}
