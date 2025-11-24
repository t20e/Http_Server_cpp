#include <cstddef>
#include <iostream>
#include <sstream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config.h"
#include "Server.h"

using std::cout;

Server::Server(Config &config, Router &router)
	: config_(config), router_(router) {}

int Server::launchServer()
{
	// Create a socket
	s_socket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (s_socket_ == -1) {
		std::cerr << "Error creating Socket.\n";
		return -1;
	}

	if (config_.DEV_MODE) {
		// Set SO_REUSEADDR to allow immediate resuse of the port. Only when developing.
		int opt = 1;
		if (setsockopt(s_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			std::cerr << "Set socket option failed\n";
			return -1;
		}
	}

	// Define server address
	s_address_.sin_family = AF_INET;
	s_address_.sin_port = htons(config_.server_port);
	s_address_.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY accept connections on any IP

	// Bind socket to address
	if (bind(s_socket_, (struct sockaddr *) &s_address_, sizeof(s_address_)) < 0) {
		std::cerr << "Error binding the server socket to the address.\n";
		return -1;
	};
	return 0;
}

int Server::Listen()
{
	if (listen(s_socket_, 5) < 0) {
		cout << "Failed to start listening for incoming connections\n";
		return -1;
	}
	cout << "\n"
		 << std::string(16, '-') << std::format(" Listening on port: {} ", config_.server_port) << std::string(16, '-') << std::endl;

	while (true) {
		cout << "\nWaiting for a new connection...\n";

		int clientSocket = accept(s_socket_, nullptr, nullptr);

		if (clientSocket < 0) {
			cout << "Error accepting client socket\n";
			continue;
		}

		cout << "\n------------------------------------\nClient Connected:\n";

		// Receive date from a client
		char buffer[bufferSizeLimit_] = {0};
		ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

		if (bytesReceived > 0) {
			process_request(buffer, bytesReceived, clientSocket);
		} else if (bytesReceived == 0) {
			cout << "Client disconnected\n.";
		} else {
			cout << "recv() Failed";
		}

		close(clientSocket); // Close the client's socket when finished communicating
	}
}


void Server::process_request(const char *buffer, ssize_t bytesReceived, const int &clientSocket)
{
	std::string full_request(buffer, bytesReceived); // Convert to string using only the bytes received
	HttpRequest req;

	// Extract (CRUD_Method, Path, http_version) from request line
	std::istringstream request_stream(full_request);
	std::string request_line;
	std::getline(request_stream, request_line);

	std::istringstream line_stream(request_line);
	if (!(line_stream >> req.method >> req.path >> req.http_version)) {
		std::cerr << "Error parsing HTTP request line\n"; // TODO add error handling
		return;
	}

	// Extract Headers and content length
	size_t header_end_pos = full_request.find("\r\n\r\n");
	if (header_end_pos == std::string::npos) {
		std::cerr << "Request is malformed\n";
		return; // TODO add error handling
	}

	std::string headers_only = full_request.substr(0, header_end_pos);
	std::istringstream header_stream(headers_only);
	std::string line;

	while (std::getline(header_stream, line)) { // read headers line by line
		// HTTP lines end in \r\n, getline() consumes '\n', so we also need to remove the '\r'
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}

		if (line.rfind("Content-Length", 0) == 0) {
			try {
				// Extract the content length info
				size_t colonPos = line.find(':');
				if (colonPos != std::string::npos) {
					req.content_length = std::stoi(line.substr(colonPos + 1));
				}
			} catch (const std::exception &e) {
				std::cerr << "Content-Length parsing error" << e.what() << '\n';
				// TODO add error handling
			}
		}
		// TODO also extract Content-type
	}

	// Extract the request Body from the initial buffer.
	size_t body_start_pos = header_end_pos + 4; // + 4 for "\r\n\r\n"
	if (body_start_pos < full_request.length()) {
		req.body = full_request.substr(body_start_pos);
	}


	// Edge case where the body might be large than the bufferSizeLimit
	if (static_cast<int>(req.body.length()) < req.content_length) {
		size_t remaining_bytes = req.content_length - req.body.length();

		while (remaining_bytes > 0) {
			char body_chunk[bufferSizeLimit_];
			size_t bytes_to_read = std::min((size_t) sizeof(body_chunk) - 1, remaining_bytes);
			ssize_t body_chunk_recv = recv(clientSocket, body_chunk, bytes_to_read, 0);

			if (body_chunk_recv < 0) {
				std::cerr << std::format("Error occurred while reading large body.\n", bufferSizeLimit_); //TODO add error handling
				break;
			} else if (body_chunk_recv == 0) {
				std::cerr << "Client disconnected unexpectedly mid-body transfer.\n"; //TODO add error handling
				break;
			}
			// Append the received chunk
			req.body.append(body_chunk, body_chunk_recv);
			remaining_bytes -= body_chunk_recv; //update the remaining counter
		}
	}

	cout << std::format("Extracted Method: {}, Extracted path: {}, Content-Length: {}\n", req.method, req.path, req.content_length);

	if (req.method == "POST" && req.body.length() > 0) {
		cout << "Request Body Data (UrlEncoded string): " << req.body << "\n";
	}
	router_.route(req, clientSocket);
}
