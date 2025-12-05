#pragma once

#include <string>
#include "Config.h"
#include "Interfaces.h"
#include "utils/http_data.h"

class ResponseService : public IResponseService {
	public:
		ResponseService(Config &config);

		std::string createResponse(const std::string &body, int status, const std::string &origin, const std::string &content_type = "application/json", const std::string &jwt_token = "", const bool &delJwtToken = false);

		int sendResponse(std::string res, const int &clientSocket);

		int handlePreflight(const HttpRequest &req, const int &clientSocket);

		void send404(const HttpRequest &req, const int &client_socket);

		void sendForbidden(const HttpRequest &req, const int &client_socket);


	private:
		Config &config_;
};
