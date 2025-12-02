#pragma once
#include <fstream>
#include <mutex>
#include <string>

/**
 * @brief Represents the log level.
 * @note CRITICAL indicates a server/fatal failure that threatens the integrity of the entire application. In deployment, if this error occurs, we would have to immediately patch it.
 * @note ERROR indicates a serious problem that prevents a specific operation/service from completing successfully, but the application remains operational.
 * @note WARNING could be for many things. One example is, a hacker is trying to brute-force login into someone else's account. We can tell this by logging the IP address and how many times that attempt happened.
 * @note DEBUG is for debugging errors.
 * @note INFO is for any information that is not an error and is typically used in deployment.
 */
enum class LogLevel { // Note: Using enum class makes it so that the enumerators (DEBUG, INFO, etc...) aren't added to the global namespace.
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	CRITICAL
};

/**
 * @brief Singleton class that handles logging messages to a file.
 * 
 */
class Logger {
	private:
		/**
         * @brief Private constructor that prevents external instantiation.
         * 
         * @param filename 
         */
		Logger(const std::string &filename = "http_server.log");

		// Delete copy/move operations. Prevents copying the instance.
		Logger(const Logger &) = delete;
		Logger &operator=(const Logger &) = delete;

		std::ofstream logFile; // File streaming.
		std::mutex logMutex_; // For thread safety.

		/**
         * @brief Converts log level to a string.
         * 
         * @param level 
         * @return std::string 
         */
		std::string levelToString(LogLevel level);

	public:
		/**
         * @brief Static method to get the single instance of the logger.
         * 
         * @param filename 
         * @return Logger& 
         */
		static Logger &getInstance(const std::string &filename = "http_server.log");

		// Destructor: Closes the log file.
		~Logger();

		/**
         * @brief Logs to the file, with a message.
         * 
         * @param level The messages log level. LogLevel.
         * @param message The log message string.
         * @param num_pre_line_breaks How many preceding line breaks to add before outputing the message. Default is zero.
         * @return int 
         */
		int log(LogLevel level, const std::string &message, int num_pre_line_breaks=0);
};
