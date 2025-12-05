#pragma once

#include <sqlite3.h>
#include <string>

#include "Interfaces.h"
#include "utils/result_types.h"

/**
 * @brief SQLite class that manages the projects databases.
 * @param db_name The name of the database.
 */
class DB_controller : public IDB_controller {
	public:
		DB_controller(const std::string &db_name);
		~DB_controller(); //Destructor closes the Database.

		// Force this class to NOT be copyable.
		DB_controller(const DB_controller &) = delete;
		DB_controller operator=(const DB_controller &) = delete;

		int createDatabase();

		DbResult addUser(std::string username, std::string password);

		DbResult getUser(int userId);

		DbResult getUser(std::string username);

        /**
         * @copydoc IDB_controller::getAllUsers()
         */
		DbListResult getAllUsers();


	private:
		const std::string db_name_;
		sqlite3 *database_connection_ = nullptr;

		/**
         * @brief Creates or opens the database depending if it exists.
         * 
         * @return int 
         */
		int openDatabase();

		/**
         * @brief Initialize the database schema with the initial data.
         * 
         * @return int 
         */
		int initSchema();


		/**
         * @brief Helper function to extract a user's data from a row and into a User object.
         * 
         * @param stmt The query statement.
         * @return User 
         */
		User extractUserFromRow(sqlite3_stmt *stmt);
};
