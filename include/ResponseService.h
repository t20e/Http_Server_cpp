#pragma once

#include <string>
#include "Config.h"
#include "utils/http_data.h"

class ResponseService {
	public:
		ResponseService(Config &config);

		/**
         * @brief Create a Response object
         * 
         * @param body The response body
         * @param status The HTTP status
         * @param origin The requests origin (http://localhost:8080) to send the response back to.
         * @param content_type What type of content will be sent.
         * @param jwt_token A new JWT token sent to the user, so we can later authenticate them for every request, other than logging in and registering.
         * @param delJwtToken Whether to delete the token or note
         * @return std::string Returns a string format of the response.
         */
		std::string createResponse(const std::string &body, int status, const std::string &origin, const std::string &content_type = "application/json", const std::string &jwt_token = "", const bool &delJwtToken = false);


		/**
         * @brief Sends the response to the client.
         * 
         * @param res The response in json-string format
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		int sendResponse(std::string res, const int &clientSocket);


		/**
         * @brief On a POST request, with `withCredentials` or another custom header like `Content-Type: application/json`, the browser will send an OPTIONS preflight request before the POST request to verify if it's safe, this function handles that OPTIONS request.
         * 
         * @param req The parsed request contents.
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		int handlePreflight(const HttpRequest &req, const int &clientSocket);


		/**
         * @brief Respond with a 404 content not found
         *       
         * @param req The parsed request contents
         * @param client_socket 
         */
		void send404(const HttpRequest &req, const int &client_socket);

		/**
         * @brief Respond with a 403 forbidden origin.
         * 
         * @param req The parsed request contents
         * @param client_socket 
         */
		void sendForbidden(const HttpRequest &req, const int &client_socket);


	private:
		Config &config_;
};
