#pragma once

// TODO Make the configurations private, like how we use .env with python
struct Config {
	int server_port = 8080;
	bool DEV_MODE = true;
};
