#pragma once
#include <netinet/in.h>

#include "Config.h"
#include "Router.h"
#include "ThreadPool.h"


/**
 * @brief Server Class
 * 
 */
class Server {

	private:
		Config &config_;
		Router &router_;
        ThreadPool &threadPool_;

		int s_socket_;
		static constexpr int bufferSizeLimit_ = 4096;
		struct sockaddr_in s_address_; // server address, i.e., http://127.0.0.1:8080 or domain.com

		/**
         * @brief Parse the request buffer into string for easier handling.
         * 
         * @param buffer Data from client.
         * @param bytesReceived The total size of the request data.
         * @param clientSocket The ID of the client's socket to send a response back to.
         * @param clientIP_str The client's IP address, that's captured when the client connection is accepted.
         */
		void process_request(const char *buffer, ssize_t bytesReceived, const int &clientSocket, std::string clientIP_str);

        /**
         * @brief A helper function that is called by the Main thread once it has accepted a new HTTP connection, a worker thread takes over that specific connection.
         * 
         * @param clientSocket The ID of the client's socket to send a response back to.
         * @param clientIP_str The client's IP address, that's captured when the client connection is accepted.
         */
        void handleClient(int clientSocket, std::string clientIP_str);


	public:
		Server(Config &config, Router &router, ThreadPool &threadPool_);

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

};
