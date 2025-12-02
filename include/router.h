#pragma once
#include <unordered_map>

#include "./request_handlers/AuthHandler.h"
#include "./request_handlers/ImageHandler.h"
#include "./request_handlers/UserHandler.h"
#include "ResponseService.h"
#include "config.h"
#include "utils/http_data.h"

/**
 * @brief Classes that manages the appropiate handler function for HTTP methods and paths
 * 
 */
class Router {

	private:
		Config &config_;
		ResponseService &resService_;
		AuthHandler &auth_handler_;
		ImageHandler &image_handler_;
		UserHandler &user_handler_;

        /**
         * @brief Contains a map (dictionary) of strings containing the request METHOD (GET, POST, etc...), and the function that will handle that route.
         * 
         */
		std::unordered_map<std::string, HttpHandler> router_map_;

        /**
        * @brief Validate if the request origin is present in the list of allowed origins.
        * 
        * @param requestOrigin The origin of a request, e.g., "http://localhost:3000".
        * @return true If the origin is allowed
        * @return false If the origin is not allowed.
        */
        bool validateOrigin( std::string requestOrigin);

	public:
		Router(
			Config &config,
			ResponseService &responseService,
			AuthHandler &auth_handler,
			ImageHandler &image_handler,
            UserHandler &user_handler
		);

		/**
         * @brief Handles routes E.g: "Get /api/login" goes to its appropriate handler function.ss
         * 
         * @param req The parsed request contents
         * @param clientSocket The id of the client socket to send a response back to.
         */
		void route(HttpRequest &req, const int clientSocket);

        /**
         * @brief Print the key:value pairs of the HTTP headers.
         * 
         * @param headers An unordered map of keys and values as strings.
         */
        void printHeaders(std::unordered_map<std::string, std::string> &headers);

};
