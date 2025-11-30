#pragma once
#include <unordered_map>

#include "config.h"
#include "utils/http_data_types.h"
#include "./RequestHandler.h"

/**
 * @brief Classes that manages the appropiate handler function for HTTP methods and paths
 * 
 */
class Router {

	public:
		Router(RequestHandler &handler, Config &config);

		/**
         * @brief Handles routes E.g: "Get /api/login" goes to its appropriate handler function.ss
         * 
         * @param req The parsed request contents
         * @param clientSocket The id of the client socket to send a response back to.
         */
		void route(HttpRequest &req, const int clientSocket);


	private:
		RequestHandler &handler_;
		Config &config_;

		std::unordered_map<std::string, HttpHandler> router_map_;
};
