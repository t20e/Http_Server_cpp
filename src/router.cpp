#include <cstddef>
#include <iostream>
#include <ostream>

#include "RequestHandler.h"
#include "Router.h"
#include "config.h"
#include "utils/misc.h"

Router::Router(RequestHandler &handler, Config &config)
	: handler_(handler), config_(config)
{
	router_map_ = {
		// GET
		{
			"GET /api/getRandomImage",
			[this](const HttpRequest &req, const int &clientSocket) { this->handler_.getRandomImage(req, clientSocket); },
		},
		{
			"GET /api/getAllusers",
			[this](const HttpRequest &req, const int &clientSocket) { this->handler_.getAllUser(req, clientSocket); },
		},
		{
			"GET /api/logout",
			[this](const HttpRequest &req, const int &clientSocket) { this->handler_.logout(req, clientSocket); },
		},
		{
			"GET /api/getLoggedUser",
			[this](const HttpRequest &req, const int &clientSocket) { this->handler_.getLoggedUser(req, clientSocket); },
		},


		// POST
		{
			"POST /api/login", // Logging-in is a POST request for security
			[this](const HttpRequest &req, const int &clientSocket) { this->handler_.login(req, clientSocket); },
		},
		{
			"POST /api/register",
			[this](const HttpRequest &req, const int &clientSocket) { this->handler_.registerUser(req, clientSocket); },
		},
	};
}


void Router::route(HttpRequest &req, const int clientSocket)
{
	std::cout << "Printing Key:Value pairs from request headers:\n";
	for (const auto &pair: req.headers) {
		std::cout << std::format("  Key: '{}', Value: '{}'\n", pair.first, pair.second);
	}
	std::cout << std::endl;

	// Validate the the Origin of the request
	if (req.headers.count("Postman-Token") && config_.DEV_MODE) {
		// Allow the Postman app to connect while in development.
		std::cout << "Origin is allowed, Origin: Postman app." << std::endl;
		req.headers.insert({"Origin", "http://localhost:3000"});

	} else if (!req.headers.count("Origin")) {
		// Origin header is mandatory
		std::cout << "Origin is mandatory!\n";
		handler_.sendForbidden(req, clientSocket);
		return;
	} else {
        // TODO move validateOrigin to a better place
		if (!validateOrigin(config_.allowed_origins, req.headers.at("Origin"))) {
			std::cout << "Origin is not allowed, Origin: " << req.headers.at("Origin") << std::endl;
			handler_.sendForbidden(req, clientSocket);
			return;
		}
	}

	//TODO Read the body after the Origin has validated, remove that code form server-> process_request() to heres, so we dont read a large body right before checking if the origin is valid.


	// Handle POST OPTIONS preflight
	if (req.method == "OPTIONS") {
		std::cout << "Handling OPTIONS preflight for: " << req.path << std::endl;
		return (void) handler_.handlePreflight(req, clientSocket);
	}

	// MIDDLEWARE: Verify JWT cookie token for all Private Routes
	if (req.path != "/api/register" && req.path != "/api/login" && req.path != "/api/getLoggedUser") {
		if (!handler_.authenticateSessionToken(req)) {
			handler_.sendForbidden(req, clientSocket);
			return;
		}
	}

	std::string pathKey = req.method + " " + req.path; // "GET /api/login"
	auto i = router_map_.find(pathKey);

	if (i != router_map_.end()) {
		i->second(req, clientSocket); // Call the associated handler function in the map.
	} else {
		std::cerr << "\nUnsupported request sending 404 (Not found) response. Key:" << pathKey << std::endl;
		handler_.send404(req, clientSocket);
	}
}
