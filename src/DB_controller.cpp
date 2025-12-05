#include <expected>
#include <format>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "./DB_controller.h"
#include "utils/Logger.h"
#include "utils/http_data.h"
#include "utils/result_types.h"

DB_controller::DB_controller(const std::string &db_name)
	: db_name_(db_name) {}

DB_controller::~DB_controller()
{
	if (database_connection_) {
		sqlite3_close(database_connection_);
		Logger::getInstance().log(LogLevel::INFO, "Database connection closed successfully.");
	}
}

int DB_controller::createDatabase()
{
	if (openDatabase() != 0) {
		return 1;
	} else if (initSchema() != 0) {
		return 1;
	}
	return 0;
}

int DB_controller::openDatabase()
{
	const char *curr_dir_c = getenv("HOME");

	if (!curr_dir_c) {
		Logger::getInstance().log(LogLevel::CRITICAL, "HOME environment variable not set!");
		return 1;
	}
	std::string curr_dir_str = curr_dir_c;
	std::string db_path = curr_dir_str + "/" + db_name_;

	int db = sqlite3_open(db_path.c_str(), &database_connection_);
	if (db) {
		Logger::getInstance().log(LogLevel::CRITICAL, std::format("Can't open database | Error_msg: {}", sqlite3_errmsg(database_connection_)));
		return 1;
	}
	Logger::getInstance().log(LogLevel::INFO, "Opened Database.");
	return 0;
}


int DB_controller::initSchema()
{
	char *zErrMsg = 0;

	const char *createTableSql = "CREATE TABLE IF NOT EXISTS USERS(UserId INTEGER PRIMARY KEY NOT NULL, username VARCHAR(32) UNIQUE NOT NULL, passwordHash VARCHAR(255) NOT NULL);";

	int rc = sqlite3_exec(database_connection_, createTableSql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Sql error creating the table: {}", zErrMsg));
		sqlite3_free(zErrMsg);
		return 1;
	}
	Logger::getInstance().log(LogLevel::INFO, "Schemas initialized.");

	const char *userSql = "INSERT OR IGNORE INTO USERS (username, passwordHash) VALUES"
						  "('johnDoe', 'fake_password'), "
						  "('thomasRidgy', 'fake_password'), "
						  "('juneArmstrong', 'fake_password'), "
						  "('AuroraJanes28', 'fake_password'), "
						  "('MonicaBrowns19', 'fake_password');";

	rc = sqlite3_exec(database_connection_, userSql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Sql error creating the users: {}", zErrMsg));
		sqlite3_free(zErrMsg);
		return 1;
	}

	Logger::getInstance().log(LogLevel::INFO, "Added default users to the database.");
	return 0;
}

DbResult DB_controller::addUser(std::string username, std::string password)
{

	const char *sql = "INSERT INTO USERS (username, passwordHash) VALUES (?, ?);";
	sqlite3_stmt *stmt; //Holds a compiled query.

	int rc = sqlite3_prepare_v2(database_connection_, sql, -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Failed to prepare query statement when adding a new user to database. Error: {}", sqlite3_errmsg(database_connection_)));
		return std::unexpected("Issue with your input.");
	}

	// Bind the data to the '?' placeholders
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt); // Execute the query

	// Check for errors like username already exists
	if (rc == SQLITE_CONSTRAINT) {
		Logger::getInstance().log(LogLevel::INFO, std::format("Error: username: '{}' already exists.", username));
		sqlite3_finalize(stmt);
		return std::unexpected("Username is taken!");
	}

	if (rc != SQLITE_DONE) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Sql execution failed: {}", sqlite3_errmsg(database_connection_)));
		sqlite3_finalize(stmt); // Finalize even on error to prevent memory leak!
		return std::unexpected("Server error!");
	}

	long new_user_id = sqlite3_last_insert_rowid(database_connection_);

	sqlite3_finalize(stmt); // Clean up memory.
	Logger::getInstance().log(LogLevel::INFO, std::format("Successfully added a user, username: '{}'", username));
	User user{(int) new_user_id, "", password = ""};
	return user;
}


DbResult DB_controller::getUser(int userId)
{
	const char *sql = "SELECT * FROM Users WHERE userID = ?;";
	sqlite3_stmt *stmt; //Holds a compiled query.

	if (sqlite3_prepare_v2(database_connection_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Failed to prepare query statement to fetch a single user from DB using their userID. Error: {}", sqlite3_errmsg(database_connection_)));

		return std::unexpected("Issue with your input.");
	}

	// Bind the userId int to the '?' placeholder
	sqlite3_bind_int(stmt, 1, userId);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		User user = extractUserFromRow(stmt);
		sqlite3_finalize(stmt);
		return user;
	}

	sqlite3_finalize(stmt);
	Logger::getInstance().log(LogLevel::INFO, std::format("Couldn't fetch user from their id: {}", userId));
	return std::unexpected("User not found");
}

DbResult DB_controller::getUser(std::string username)
{
	const char *sql = "SELECT * FROM Users WHERE username = ?;";
	sqlite3_stmt *stmt; //Holds a compiled query.

	if (sqlite3_prepare_v2(database_connection_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Failed to prepare query statement to fetch a single user from DB using their username. Error: {}", sqlite3_errmsg(database_connection_)));
		return std::unexpected("Issue with your input.");
	}

	// Bind the userId int to the '?' placeholder
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		User user = extractUserFromRow(stmt);
		sqlite3_finalize(stmt);
		return user;
	}

	sqlite3_finalize(stmt);
	Logger::getInstance().log(LogLevel::WARNING, std::format("Couldn't fetch user from their username: {}", username));
	return std::unexpected("User not found");
}


User DB_controller::extractUserFromRow(sqlite3_stmt *stmt)
{
	int id = sqlite3_column_int(stmt, 0);

	const char *usernamePtr = (const char *) sqlite3_column_text(stmt, 1);
	const char *hashedPasswordPtr = (const char *) sqlite3_column_text(stmt, 2);

	return User{
		id,
		usernamePtr ? std::string(usernamePtr) : "",
		hashedPasswordPtr ? std::string(hashedPasswordPtr) : "",
	};
}


DbListResult DB_controller::getAllUsers()
{
	const char *sql = "SELECT * FROM Users";
	sqlite3_stmt *stmt; //Holds a compiled query statement.

	if (sqlite3_prepare_v2(database_connection_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		Logger::getInstance().log(LogLevel::ERROR, std::format("Failed to prepare query statement, to fetch the user using their username. Error: {}", sqlite3_errmsg(database_connection_)));
		return std::unexpected("Issue with your input.");
	}

	std::vector<User> users;

	// Execute query statement per user and loop for all the rows of users.
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		users.push_back(extractUserFromRow(stmt));
	}
	sqlite3_finalize(stmt);
	return users;
}
