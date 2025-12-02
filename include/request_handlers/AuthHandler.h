#pragma once

#include <variant>
#include "DB_controller.h"
#include "ResponseService.h"
#include "Config.h"
#include "request_handlers/BaseHandler.h"
#include "utils/http_data.h"


/**
 * @brief Handles all authentication (login, register, logout, etc...) HTTP requests and responses.
 * 
 */
class AuthHandler : BaseHandler {

	private:

		/**
         * @brief Create a Jwt Session Token.
         * 
         * @param user_id The user's ID. This way we can verify the user's token without having to query the database.
         * @return std::string The token string.
         */
		std::string createJwtSessionToken(const int &user_id);

        /**
         * @brief Verify JWT session tokens to authenticate the user per each request!
         * 
         * @param token The incoming requests session token.
         * @return std::expected<int, std::string> Returns an integer (the user's userID), if their JWT session token is valid, otherwise an error message string.
         */
		std::expected<int, std::string> verifyJwtSessionToken(std::string &token);

		/**
         * @brief Extract the JWT token from header
         * 
         * @param cookie String, e.g., "session_token=eyJhbGciOiJIUzI1NiIsIn...."

         * @return std::string 
         */
		std::string extractJwtToken(std::string cookie);


	public:
		AuthHandler(
			Config &config,
			DB_controller &db,
			ResponseService &responseService);

		/**
         * @brief Check if the user's session token is valid. If valid return the user's data, otherwise null.
         * 
         * @param req The parsed request contents.
         * @param client_socket The clients socket to send that response to.
         * @return int 
         */
		int getUserByToken(const HttpRequest &req, const int &client_socket);

		/**
         * @brief Middleware: Authenticate the user's session token for every request other than /login and /register.
         * 
         * @param req The parsed request contents.
         * @param client_socket The clients socket to send that response to.
         * @return true If token is valid, otherwise false.
         */
		bool authenticateSessionToken(const HttpRequest &req);

		/**
         * @brief Log the user out. Because we are using an HTTP-only session cookie (JWT-token) for user authentication, and React's javascript isn't allowed to access it. We need the backend to send the frontend Browser info, so the browser can delete its stored session cookie, and log the user out.
         * 
         * @param req The parsed request contents.
         * @param client_socket The clients socket to send that response to.
         * @return int 
         */
		int logout(const HttpRequest &req, const int &client_socket);


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
};
