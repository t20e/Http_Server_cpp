#include <iostream>

#include "Router.h"
#include "RequestHandler.h"

Router::Router(RequestHandler &handler)
	: handler_(handler)
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

void Router::route(const HttpRequest &req, const int clientSocket)
{
	if (req.method == "OPTIONS") { // Handle POST OPTIONS preflight
		std::cout << "Handling OPTIONS preflight for: " << req.path << std::endl;
		return (void) handler_.handlePreflight(req, clientSocket);
	}

	std::string key = req.method + " " + req.path;
	auto i = router_map_.find(key);

	if (i != router_map_.end()) {
		i->second(req, clientSocket); // Call the associated handler function in the map.
	} else {
		std::cerr << "\nUnsupported request sending 404 (Not found) response. Key:" << key << std::endl;
		handler_.send404(clientSocket);
	}
}
