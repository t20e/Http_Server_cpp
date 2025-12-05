#pragma once

#include "utils/http_data.h"
#include "utils/result_types.h"

/*
Interfaces needed for GMock testing.
*/

class IDB_controller {
	public:
		virtual ~IDB_controller() = default;

        /**
         * @brief Create a SQLite Database.
         * 
         * @return int 
         */
		virtual int createDatabase() = 0;

		/**
         * @brief Add a user to the database.
         * 
         * @param name User's full name.
         * @param password User's Hashed Password.
         * @return int 
         */
		virtual DbResult addUser(std::string username, std::string password) = 0;

        /**
         * @brief Get the user from the database using their userID
         * 
         * @param userId
         * @return DbResult 
         */
		virtual DbResult getUser(int userId) = 0;

        /**
         * @brief Overload function to get user by username
         * 
         */
		virtual DbResult getUser(std::string username) = 0;

        /**
         * @brief Get all users
         * 
         * @return DbResult A vector of users, if no users are found than it will return an empty vector.
         */
		virtual DbListResult getAllUsers() = 0;
};


class IResponseService {
	public:
		virtual ~IResponseService() = default;

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
		virtual std::string createResponse(const std::string &body, int status, const std::string &origin, const std::string &content_type = "application/json", const std::string &jwt_token = "", const bool &delJwtToken = false) = 0;

		/**
         * @brief Sends the response to the client.
         * 
         * @param res The response in json-string format
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		virtual int sendResponse(std::string res, const int &clientSocket) = 0;

		/**
         * @brief On a POST request, with `withCredentials` or another custom header like `Content-Type: application/json`, the browser will send an OPTIONS preflight request before the POST request to verify if it's safe, this function handles that OPTIONS request.
         * 
         * @param req The parsed request contents.
         * @param clientSocket The clients socket to send that response to.
         * @return int 
         */
		virtual int handlePreflight(const HttpRequest &req, const int &clientSocket) = 0;


		/**
         * @brief Respond with a 404 content not found
         *       
         * @param req The parsed request contents
         * @param client_socket 
         */
		virtual void send404(const HttpRequest &req, const int &client_socket) = 0;

		/**
         * @brief Respond with a 403 forbidden origin.
         * 
         * @param req The parsed request contents
         * @param client_socket 
         */
		virtual void sendForbidden(const HttpRequest &req, const int &client_socket) = 0;
};
