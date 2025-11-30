#include <iostream>
#include <Server.h>
#include <sqlite3.h>
#include <string>


#include "config.h"
#include "./RequestHandler.h"
#include "./Router.h"
#include "./SQLiteDB.h"


int main()
{
	std::cout << "\n\n\n";
	static Config config;

	SQLiteDB db("database.db");

	if (db.createDatabase() != 0) {
		return 1;
	}

	RequestHandler handler(db, config);
	Router router(handler, config);

	Server s = Server(config, router);
	if (s.launchServer() == -1) {
		std::cerr << "Server failed to launch. Exiting.\n";
		return 1;
	}
	s.Listen();
	return 0;

}
