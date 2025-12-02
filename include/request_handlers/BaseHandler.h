#pragma once

#include "DB_controller.h"
#include "ResponseService.h"
#include "Config.h"


/**
 * @brief Base class for all HTTP request handlers.
 * 
 */
class BaseHandler {
	public:
		BaseHandler(Config &config, DB_controller &db, ResponseService &responseService)
			: config_(config), db_(db), resService_(responseService) {}

	protected:
		Config &config_;
		DB_controller &db_;
		ResponseService &resService_;
};
