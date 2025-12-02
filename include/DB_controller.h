#pragma once

#include <sqlite3.h>
#include <string>

#include "utils/result_types.h"



/**
 * @brief SQLite class that manages the projects databases.
 * @param db_name The name of the database.
 */
class DB_controller {
	public:
		DB_controller(const std::string &db_name);
		~DB_controller(); //Destructor closes the Database.

		// Force this class to NOT be copyable.
		DB_controller(const DB_controller &) = delete;
		DB_controller operator=(const DB_controller &) = delete;

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

        /**
         * @brief Overload function to get user by username
         * 
         */
		DbResult getUser(std::string username);

        /**
         * @brief Get all users
         * 
         * @return DbResult A vector of users, if no users are found than it will return an empty vector.
         */
        DbListResult getAllUsers();


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

        // /**
        //  * @brief Fetch a user from DB.
        //  * 
        //  * @param stmt The query statement.
        //  * @return DbResult
        //  */
        // DbResult fetchUser(sqlite3_stmt *stmt);

        /**
         * @brief Helper function to extract a user's data from a row and into a User object.
         * 
         * @param stmt The query statement.
         * @return User 
         */
        User extractUserFromRow(sqlite3_stmt *stmt);
};
