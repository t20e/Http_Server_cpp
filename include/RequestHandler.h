#pragma once

#include "config.h"
#include "SQLiteDB.h"
#include "utils/http_data_types.h"

/**
 * @brief Class that manages all routes and request response formating.
 * 
 */
class RequestHandler {
	public:
		explicit RequestHandler(SQLiteDB &db);

		/**
         * @brief Respond with a 404 content not found
         * 
         */
		void send404(const int &client_socket);


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
         * @param req The parsed request contents.
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		int registerUser(const HttpRequest &req, int clientSocket);

		/**
         * @brief On a POST request, with `withCredentials` or another custom header like `Content-Type: application/json`, the browser will send an OPTIONS preflight request before the POST request to verify if it's safe, this function handles that OPTIONS request.
         * 
         * @param req The parsed request contents.
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		int handlePreflight(const HttpRequest &req, const int &clientSocket);


        /**
         * @brief Get all the users from the database.
         * 
         * @param req The parsed request contents.
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
        int getAllUser(const HttpRequest &req, const int &clientSocket);


        /**
         * @brief Get a random image that is stored in the file system, not in the database
         * 
         * @param req The parsed request contents.
         * @param client_socket The clients socket to send that response to.
         * @return int 
         */
        int getRandomImage(const HttpRequest &req, const int &client_socket);


        /**
         * @brief Check if the users session token is valid. If valid return the user's data, otherwise null.
         * 
         * @param req The parsed request contents.
         * @param client_socket The clients socket to send that response to.
         * @return int 
         */
        int checkSessionToken(const HttpRequest &req, const int &client_socket);

	private:
		SQLiteDB &db_;

		/**
         * @brief Create a Response object
         * 
         * @param body The response body
         * @param status The HTTP status
         * @param content_type What type of content will be sent.
         * @return std::string Returns a string format of the response.
         */
		std::string createResponse(const std::string &body, int status, const std::string &content_type = "application/json");


		/**
         * @brief Sends the response to the client.
         * 
         * @param res The response in json-string format
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		int sendResponse(std::string res, const int &clientSocket);
};
