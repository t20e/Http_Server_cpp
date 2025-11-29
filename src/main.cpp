#include <iostream>
#include <Server.h>
#include <sqlite3.h>
#include <string>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>


#include "config.h"
#include "./RequestHandler.h"
#include "./Router.h"
#include "./SQLiteDB.h"


int main()
{

    std::string const token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXUyJ9.eyJpc3MiOiJhdXRoMCIsInNhbXBsZSI6InRlc3QifQ.lQm3N2bVlqt2-1L-FsOjtR6uE-L4E9zJutMWKIe1v1M";
    auto decoded = jwt::decode(token);

    for(auto& e : decoded.get_payload_json())
        std::cout << e.first << " = " << e.second << '\n';
    

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
