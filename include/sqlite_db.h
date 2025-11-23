#pragma once

#include <sqlite3.h>
#include <string>

#include "utils/error_handling_template.h"

/**
 * @brief SQLite class that manages the projects databases.
 * @param db_name The name of the database.
 */
class SQLiteDB {
	public:
		SQLiteDB(const std::string &db_name);
		~SQLiteDB(); //Destructor closes the Database.

		// Force this class to NOT be copyable.
		SQLiteDB(const SQLiteDB &) = delete;
		SQLiteDB operator=(const SQLiteDB &) = delete;

		int createDatabase();

		/**
         * @brief Add a user to the database.
         * 
         * @param name User's full name.
         * @param password User's Hashed Password.
         * @return int 
         */
		DbResult addUser(std::string username, std::string password);

        /**
         * @brief Get the user from the database using their userID
         * 
         * @param userId 
         * @return DbResult 
         */
		DbResult getUser(int userId);
        // Overload function to get user by username
		DbResult getUser(std::string username);




	private:
		const std::string &db_name_;
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
         * @brief Fetch a user from DB.
         * 
         * @param stmt SQLite query.
         * @return DbResult
         */
        DbResult fetchUser(sqlite3_stmt *stmt);
};
