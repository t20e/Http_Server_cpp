#pragma once

#include "DB_controller.h"
#include "ResponseService.h"
#include "config.h"
#include "request_handlers/BaseHandler.h"
#include "utils/http_data.h"

/**
 * @brief Handles all image related requests.
 * 
 */
class ImageHandler : BaseHandler {

	private:

	public:
		ImageHandler(
			Config &config,
			DB_controller &db,
			ResponseService &responseService);


		/**
         * @brief Get a random image that is stored in the file system, not in the database
         * 
         * @param req The parsed request contents.
         * @param client_socket The clients socket to send that response to.
         * @return int 
         */
		int getRandomImage(const HttpRequest &req, const int &client_socket);
};
