#pragma once

#include "Config.h"
#include "Interfaces.h"


/**
 * @brief Base class for all HTTP request handlers.
 * 
 */
class BaseHandler {
	public:
		BaseHandler(Config &config, IDB_controller &db, IResponseService &responseService)
			: config_(config), db_(db), resService_(responseService) {}

	protected:
		Config &config_;
		IDB_controller &db_;
		IResponseService &resService_;
};
