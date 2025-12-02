#include <Server.h>
#include <iostream>
#include <sqlite3.h>
#include <string>


#include "./DB_controller.h"
#include "./Router.h"
#include "ResponseService.h"
#include "config.h"
#include "request_handlers/AuthHandler.h"
#include "request_handlers/ImageHandler.h"
#include "request_handlers/UserHandler.h"
#include "utils/Logger.h"


int main()
{
	Logger::getInstance().log(LogLevel::INFO, std::format("{} Launching Application... {}\n\n", std::string(16, '-'), std::string(16, '-')), 5);

	static Config config;

	DB_controller db("database.db");

	if (db.createDatabase() != 0) {
		return 1;
	}

	ResponseService responseService(config);

	AuthHandler authHandler(config, db, responseService);
	ImageHandler imageHandler(config, db, responseService);
	UserHandler userHandler(config, db, responseService);

	Router router(
		config,
		responseService,
		authHandler,
		imageHandler,
		userHandler);

	Server s = Server(config, router);
	if (s.launchServer() == -1) {
		Logger::getInstance();
		Logger::getInstance().log(LogLevel::CRITICAL, "Server failed to launch. Exiting.", 3);
		return 1;
	}
	s.Listen();
	return 0;
}
