#include <iostream>
#include <server.h>
#include <sqlite3.h>

#include "config.h"
#include "request_handler.h"
#include "router.h"
#include "sqlite_db.h"

#include <iostream>
#include <string>

int main()
{
	std::cout << "\n\n\n";
	static Config config;

	SQLiteDB db("database.db");

	if (db.createDatabase() != 0) {
		return 1;
	}

	RequestHandler handler(db);
	Router router(handler);

	Server s = Server(config, router);
	if (s.launchServer() == -1) {
		std::cerr << "Server failed to launch. Exiting.\n";
		return 1;
	}
	s.Listen();
	return 0;

}
