#pragma once

#include "DB_controller.h"
#include "ResponseService.h"
#include "config.h"
#include "request_handlers/BaseHandler.h"
#include "utils/http_data.h"


/**
 * @brief Handles all user related HTTP requests and responses, other than authentication.
 * 
 */
class UserHandler :BaseHandler{

	private:

	public:
		UserHandler(
			Config &config,
			DB_controller &db,
			ResponseService &responseService);


		/**
         * @brief Get the All Users.
         * 
         * @param req 
         * @param clientSocket 
         * @return int 
         */
		int getAllUser(const HttpRequest &req, const int &clientSocket);
};
