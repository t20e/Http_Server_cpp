


/*
💡 NOTE: Files like these should never be shared, especially on github! This is a showcase project only reason I commited it!
// TODO when making the config secretive!
*/


#pragma once
#include <set>
#include <string>

// TODO Make the configurations private, like how we use .env with python
struct Config {
		int SERVER_PORT = 8080;
		bool DEV_MODE = true;
		bool ENABLE_MULTI_THREADING = false; // When in developing mode multi-threading can make print statements look weird, debugging becomes more difficult.

		// Key to sign User's JWT session tokens.
		std::string JWT_SECRET_KEY = "df0a190c4a48347945bb1d520e0a0a7c17a205a9243befe5978dffd7f056e2b2";

		// Only allow these origins and IPs to connect to the server.
		std::string allowed_origins[1] = {"http://localhost:3000"};
		//  - For less confusion origin: "http://127.0.0.1:3000" isn't allowed.

		// Unlike browsers like chrome the socket only sees numbers, and localhost and 127.0.0.1 are the same endpoints. Chrome sees them as two separate sites.
		std::set<std::string> allowed_IPs = {"127.0.0.1", "::1"}; // ::1 For IPv6
};
