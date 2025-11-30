#pragma once
#include <netinet/in.h>

#include "config.h"
#include "Router.h"


/**
 * @brief Server Class
 * 
 */
class Server {
	public:
		Server(Config &config, Router &router);

		/**
         * @brief Launch the server.
         * 
         * @return int 
         */
		int launchServer();

		/**
         * @brief Start listening for connections.
         * 
         * @return int 
         */
		int Listen();

	private:
		int s_socket_;
		Config &config_;
		Router &router_;
		static constexpr int bufferSizeLimit_ = 4096;
		struct sockaddr_in s_address_; // server address, i.e., http://127.0.0.1:8080 or domain.com

		/**
         * @brief Parse the request buffer into string for easier handling.
         * 
         * @param buffer Data from client.
         * @param bytesReceived The total size of the request data.
         * @param clientSocket The id of the client socket to send a response back to.
         */
		void process_request(const char *buffer, ssize_t bytesReceived, const int &clientSocket);
};
