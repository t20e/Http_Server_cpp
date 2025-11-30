#pragma once

#include <string>

#include "SQLiteDB.h"
#include "config.h"
#include "utils/http_data_types.h"
/**
 * @brief Class that manages all routes and request response formating.
 * 
 */
class RequestHandler {
    // TODO!!! MAKE THIS CLASS DRYER! make this the parent and have child classes of 
    // 1. AuthController (Login, Register, Logout)
    // 2. ImageController (all images info)
    // 3. And other parents like validation
	public:
		explicit RequestHandler(SQLiteDB &db, Config &config);

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
         * @brief Log the user out. Because we are using an HTTP-only session cookie (JWT-token) for user authentication, and React's javascript isn't allowed to access it. We need the backend to send the frontend Browser info, so the browser can delete its stored session cookie, and log the user out.
         * 
         * @param req The parsed request contents.
         * @param client_socket The clients socket to send that response to.
         * @return int 
         */
		int logout(const HttpRequest &req, const int &client_socket);

		/**
         * @brief Check if the users session token is valid. If valid return the user's data, otherwise null.
         * 
         * @param req The parsed request contents.
         * @param client_socket The clients socket to send that response to.
         * @return int 
         */
		int getLoggedUser(const HttpRequest &req, const int &client_socket);

		/**
         * @brief Middleware: Authenticate the user's session token for every request other than /login and /register.
         * 
         * @param req The parsed request contents.
         * @param client_socket The clients socket to send that response to.
         * @return true If token is valid, otherwise false.
         */
		bool authenticateSessionToken(const HttpRequest &req);

	private:
		SQLiteDB &db_;
		Config &config_;

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
         * @return std::optional<int> Returns an integer (userID) if JWT token is valid, otherwise void.
         */
		std::optional<int> verifyJwtSessionToken(std::string &token);

		/**
         * @brief Extract the JWT token from header
         * 
         * @param cookie String, e.g., "session_token=eyJhbGciOiJIUzI1NiIsIn...."

         * @return std::string 
         */
		std::string extractJwtToken(std::string cookie);


};
