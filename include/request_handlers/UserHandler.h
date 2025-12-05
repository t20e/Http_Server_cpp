#pragma once

#include "Config.h"
#include "Interfaces.h"
#include "request_handlers/BaseHandler.h"
#include "utils/http_data.h"


/**
 * @brief Handles all user related HTTP requests and responses, other than authentication.
 * 
 */
class UserHandler : BaseHandler {

	private:
	public:
		UserHandler(
			Config &config,
			IDB_controller &db,
			IResponseService &responseService);
		// UserHandler(
		// 	Config &config,
		// 	DB_controller &db,
		// 	ResponseService &responseService);


		/**
         * @brief Get the All Users.
         * 
         * @param req 
         * @param clientSocket 
         * @return int 
         */
		int getAllUser(const HttpRequest &req, const int &clientSocket);
};
