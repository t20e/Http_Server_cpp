#pragma once

#include "utils/http_data_types.h"
#include "config.h"

/**
 * @brief Class that manages all routes and request response formating.
 * 
 */
class RequestHandler {
	public:
        // RequestHandler();

		/**
         * @brief Respond with a 404 content not found
         * 
         */
		void send404(const int &client_socket);

		/**
         * @brief Send a cat image back to the user.
         *
         * @param req The parsed request contents
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		int handle_test_get(const HttpRequest &req, int clientSocket);


		/**
         * @brief Log the user in.
         * 
         * @param req The parsed request contents
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		int login(const HttpRequest &req, int clientSocket);
        
		/**
         * @brief Log the user in.
         * 
         * @param req The parsed request contents
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		int reg(const HttpRequest &req, int clientSocket);

	private:

		/**
         * @brief Create a Response object
         * 
         * @param body The response body
         * @param status The HTTP status
         * @param content_type What type of content will be sent.
         * @return std::string Returns a string format of the response.
         */
		std::string createResponse(const std::string &body, int status = 200, const std::string &content_type = "application/json");


		/**
         * @brief Sends the response to the client.
         * 
         * @param res The response in json-string format
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		int sendRes(std::string res, const int &clientSocket);
};
