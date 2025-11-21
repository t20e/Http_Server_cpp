#include <iostream>

#include "router.h"

Router::Router(RequestHandler &handler) : handler_(handler)
{
	router_map_ = {
		// GET
		{
			"GET /api/getImage",
			[this](const HttpRequest &req, const int &clientSocket) { this->handler_.handle_test_get(req, clientSocket); },
		},
		// POST
		{
			"POST /api/login",
			[this](const HttpRequest &req, const int &clientSocket) { this->handler_.login(req, clientSocket); },
		},
		{
			"POST /api/register",
			[this](const HttpRequest &req, const int &clientSocket) { this->handler_.reg(req, clientSocket); },
		},
	};
}

void Router::route(const HttpRequest &req, const int clientSocket)
{
	std::string key = req.method + " " + req.path;
	auto i = router_map_.find(key);

	if (i != router_map_.end()) {
		i->second(req, clientSocket); // Call the associated handler function in the map.
	} else {
		std::cerr << "\nUnsupported request sending 404 (Not found) response. Key:" << key << std::endl;
		handler_.send404(clientSocket);
	}
}
