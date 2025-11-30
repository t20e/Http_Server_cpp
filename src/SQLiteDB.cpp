#include <expected>
#include <format>
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "./SQLiteDB.h"
#include "utils/error_handling_template.h"
#include "utils/http_data_types.h"

SQLiteDB::SQLiteDB(const std::string &db_name)
	: db_name_(db_name) {}

SQLiteDB::~SQLiteDB()
{
	if (database_connection_) {
		sqlite3_close(database_connection_);
		std::cout << "Database connection closed successfully.\n";
	}
}

int SQLiteDB::createDatabase()
{
	if (openDatabase() != 0) {
		return 1;
	} else if (initSchema() != 0) {
		return 1;
	}
	return 0;
}

int SQLiteDB::openDatabase()
{
	int db = sqlite3_open(db_name_.c_str(), &database_connection_);
	if (db) {
		std::cerr << std::format("Can't open database: {}\n", sqlite3_errmsg(database_connection_));
		return 1;
	}
	std::cout << "Opened Database.\n";
	return 0;
}

int SQLiteDB::initSchema()
{
	char *zErrMsg = 0;

	const char *createTableSql = "CREATE TABLE IF NOT EXISTS USERS(UserId INTEGER PRIMARY KEY NOT NULL, username VARCHAR(32) UNIQUE NOT NULL, passwordHash VARCHAR(255) NOT NULL);";

	int rc = sqlite3_exec(database_connection_, createTableSql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		std::cerr << std::format("Sql error creating the table: {}\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return 1;
	}
	std::cout << "Schemas initialized.\n";

	const char *userSql = "INSERT OR IGNORE INTO USERS (username, passwordHash) VALUES"
						  "('johnDoe', 'fake_password'), "
						  "('thomasRidgy', 'fake_password'), "
						  "('juneArmstrong', 'fake_password'), "
						  "('AuroraJanes28', 'fake_password'), "
						  "('MonicaBrowns19', 'fake_password');";

	rc = sqlite3_exec(database_connection_, userSql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		std::cerr << std::format("Sql error creating the users: {}\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return 1;
	}

	std::cout << "Added default users.\n";
	return 0;
}

DbResult SQLiteDB::addUser(std::string username, std::string password)
{

	const char *sql = "INSERT INTO USERS (username, passwordHash) VALUES (?, ?);";
	sqlite3_stmt *stmt; //Holds a compiled query.

	int rc = sqlite3_prepare_v2(database_connection_, sql, -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare query statement" << sqlite3_errmsg(database_connection_) << std::endl;
		return std::unexpected("Issue with your input.");
	}

	// Bind the data to the '?' placeholders
	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt); // Execute the query

	// Check for errors like username already exists
	if (rc == SQLITE_CONSTRAINT) {
		std::cerr << std::format("Error: username: '{}' already exists\n", username);
		sqlite3_finalize(stmt);
		return std::unexpected("Username is taken!");
	}

	if (rc != SQLITE_DONE) {
		std::cerr << std::format("Sql execution failed: {}\n", sqlite3_errmsg(database_connection_));
		sqlite3_finalize(stmt); // Finalize even on error to prevent memory leak!
		return std::unexpected("Server error!");
	}

	long new_user_id = sqlite3_last_insert_rowid(database_connection_);

	sqlite3_finalize(stmt); // Clean up memory.
	std::cout << "Added user successfully, username:'" << username << "'.\n";
	User user{(int) new_user_id, ""};
	return user;
}


// DbResult SQLiteDB::fetchUser(sqlite3_stmt *stmt)
// {
// 	int rc = sqlite3_step(stmt); // Execute the query

// 	if (rc == SQLITE_ROW) {
// 		std::cout << "Found User.\n";
// 		int id = sqlite3_column_int((stmt), 0);
// 		const char *usernamePtr = (const char *) sqlite3_column_text(stmt, 1);
// 		const char *hashedPasswordPtr = (const char *) sqlite3_column_text(stmt, 2);

// 		// std::cout << std::format("User's username: {} | and | hashedPassword: {} from the database.\n", usernamePtr, hashedPasswordPtr);
// 		User user{id, usernamePtr ? std::string(usernamePtr) : "", hashedPasswordPtr ? std::string(hashedPasswordPtr) : ""};

// 		sqlite3_finalize(stmt); //clean up memory
// 		return user;
// 	}
// 	if (rc == SQLITE_DONE) {
// 		std::cerr << std::format("Sql execution failed, user not found | Error: {}\n", sqlite3_errmsg(database_connection_));
// 		sqlite3_finalize(stmt); // Finalize even on error to prevent memory leak!
// 		return std::unexpected("User not found!");
// 	}
// 	sqlite3_finalize(stmt); // Finalize even on error to prevent memory leak!
// 	std::cout << "Server error during query execution!\n";
// 	return std::unexpected("Server error during query execution!");
// }


DbResult SQLiteDB::getUser(int userId)
{
	const char *sql = "SELECT * FROM Users WHERE userID = ?;";
	sqlite3_stmt *stmt; //Holds a compiled query.

	if (sqlite3_prepare_v2(database_connection_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "Failed to prepare query statement, to fetch the user using their userID" << sqlite3_errmsg(database_connection_) << std::endl;
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
    std::cout << "Couldn't fetch user from their id: " << userId << std::endl;
	return std::unexpected("User not found");
}

DbResult SQLiteDB::getUser(std::string username)
{
	const char *sql = "SELECT * FROM Users WHERE username = ?;";
	sqlite3_stmt *stmt; //Holds a compiled query.

	if (sqlite3_prepare_v2(database_connection_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "Failed to prepare query statement, to fetch the user using their username" << sqlite3_errmsg(database_connection_) << std::endl;
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
	std::cout << "Couldn't fetch user from their username: " << username << std::endl;
	return std::unexpected("User not found");
}


User SQLiteDB::extractUserFromRow(sqlite3_stmt *stmt)
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


DbListResult SQLiteDB::getAllUsers()
{
	const char *sql = "SELECT * FROM Users";
	sqlite3_stmt *stmt; //Holds a compiled query statement.

	if (sqlite3_prepare_v2(database_connection_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "Failed to prepare query statement, to fetch the user using their username" << sqlite3_errmsg(database_connection_) << std::endl;
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
