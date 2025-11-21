#include <iostream>
#include <server.h>

#include "config.h"
#include "request_handler.h"
#include "router.h"

int main()
{
	Config config;
	RequestHandler handler;
	Router router(handler);

	Server s = Server(config, router);
	if (s.launchServer() == -1) {
		std::cerr << "Server failed to launch. Exiting.\n";
		return 1;
	}
	s.Listen();
	return 0;
}
