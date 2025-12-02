#include <cstddef>
#include <format>
#include <iostream>
#include <ostream>

#include "./request_handlers/AuthHandler.h"
#include "./request_handlers/ImageHandler.h"

#include "ResponseService.h"
#include "Router.h"
#include "Config.h"
#include "request_handlers/UserHandler.h"
#include "utils/Logger.h"

Router::Router(
	Config &config,
	ResponseService &responseService,
	AuthHandler &auth_handler,
	ImageHandler &image_handler,
	UserHandler &user_handler)
	: config_(config),
	  resService_(responseService),
	  auth_handler_(auth_handler),
	  image_handler_(image_handler),
	  user_handler_(user_handler)
{
	router_map_ = {
		// GET
		{
			"GET /api/getRandomImage",
			[this](const HttpRequest &req, const int &clientSocket) { this->image_handler_.getRandomImage(req, clientSocket); },
		},
		{
			"GET /api/getAllUsers",
			[this](const HttpRequest &req, const int &clientSocket) { this->user_handler_.getAllUser(req, clientSocket); },
		},
		{
			"GET /api/logout",
			[this](const HttpRequest &req, const int &clientSocket) { this->auth_handler_.logout(req, clientSocket); },
		},
		{
			"GET /api/getUserByToken",
			[this](const HttpRequest &req, const int &clientSocket) { this->auth_handler_.getUserByToken(req, clientSocket); },
		},


		// POST
		{
			"POST /api/login", // Logging-in is a POST request for security
			[this](const HttpRequest &req, const int &clientSocket) { this->auth_handler_.login(req, clientSocket); },
		},
		{
			"POST /api/register",
			[this](const HttpRequest &req, const int &clientSocket) { this->auth_handler_.registerUser(req, clientSocket); },
		},
	};
}


bool Router::validateOrigin(std::string requestOrigin)
{
	if (config_.ALLOWED_ORIGINS.count(requestOrigin)) { // O(1)
		return true;
	}
	return false;
}

void Router::printHeaders(std::unordered_map<std::string, std::string> &headers)
{
	std::cout << "Printing Key:Value pairs from request headers:\n";
	for (const auto &pair: headers) {
		std::cout << std::format("  Key: '{}', Value: '{}'\n", pair.first, pair.second);
	}
	std::cout << std::endl;
}


void Router::route(HttpRequest &req, const int clientSocket)
{
	// printHeaders(req.headers);

	// Validate the the Origin of the request
	if (req.headers.count("Postman-Token") && config_.DEV_MODE) {
		// Allow the Postman app to connect while in development.
		Logger::getInstance().log(LogLevel::DEBUG, "Origin is allowed, Origin: Postman app.");
		req.headers.insert({"Origin", "http://localhost:3000"});

	} else if (!req.headers.count("Origin")) {
		// Origin header is mandatory
		Logger::getInstance().log(LogLevel::WARNING, std::format("Origin not set in request header. Origin is mandatory, Client forcibly disconnected!, IP: {}", req.client_ip));
		resService_.sendForbidden(req, clientSocket);
		return;
	} else {
		if (!validateOrigin(req.headers.at("Origin"))) {
			Logger::getInstance().log(LogLevel::WARNING, std::format("Client's origin is not allowed, Origin: {}, IP: {}. Client forcibly disconnected!", req.headers.at("Origin"), req.client_ip));
			resService_.sendForbidden(req, clientSocket);
			return;
		}
	}

	//TODO Read the body after the Origin has validated, remove that code form server-> process_request() to heres, so we dont read a large body right before checking if the origin is valid.


	// Handle POST OPTIONS preflight
	if (req.method == "OPTIONS") {
		Logger::getInstance().log(LogLevel::INFO, std::format("Handling OPTIONS preflight for: {}, IP: {}", req.path, req.client_ip));
		return (void) resService_.handlePreflight(req, clientSocket);
	}

	// MIDDLEWARE: Verify JWT cookie token for all Private Routes
	if (req.path != "/api/register" && req.path != "/api/login" && req.path != "/api/getUserByToken") {
		if (!auth_handler_.authenticateSessionToken(req)) {
			resService_.sendForbidden(req, clientSocket);
			return;
		}
	}

	std::string pathKey = req.method + " " + req.path; // "GET /api/login"
	auto i = router_map_.find(pathKey);

	if (i != router_map_.end()) {
		i->second(req, clientSocket); // Call the associated handler function in the map.
	} else {
		Logger::getInstance().log(LogLevel::WARNING, std::format("Unsupported request sending 404 (Not found) response. Key: {}, IP: {}", pathKey, req.client_ip));

		resService_.send404(req, clientSocket);
	}
}
