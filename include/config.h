#pragma once
#include <string>

// TODO Make the configurations private, like how we use .env with python
struct Config {
		int SERVER_PORT = 8080;
		bool DEV_MODE = true;
		bool ENABLE_MULTI_THREADING = false; // When in developing mode multi-threading can make print statements look weird, debugging becomes more difficult.


        // Key to sign User's JWT session tokens.
        std::string SECRET_KEY = "apfnmakf akfapfkpf334;lkamfa;dm";
};
