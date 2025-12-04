#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "./utils/Logger.h"

namespace fs = std::filesystem;

Logger::Logger(const std::string &filename)
{
	fs::path log_dir = "./logs";
	fs::path full_path = log_dir / filename;

	fs::create_directories(log_dir); // Ensure directory exists.

	// Open the file logFile in append mode.
	logFile.open(full_path, std::ios::app);
	if (!logFile.is_open()) {
		std::cerr << "Error opening log file. Filename: " << filename << "\n";
	}
}

Logger::~Logger()
{
	logFile.close();
}

Logger &Logger::getInstance(const std::string &filename)
{
	// Static variable is initialized once, and is thread-safe.
	static Logger instance(filename);
	return instance;
}


int Logger::log(LogLevel level, const std::string &message, int num_pre_line_breaks)
{
	// Ensures that when multi-threading is used, that only one thread writes to the file at a time.
	std::lock_guard<std::mutex> lock(logMutex_);

	auto now = std::chrono::system_clock::now();
	time_t now_c = std::chrono::system_clock::to_time_t(now);
	auto timestamp = std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");


	// Create log entry
	std::ostringstream logEntry;

	// Add preceding line breaks if needed.
	for (int i = 0; i < num_pre_line_breaks; i++) {
		logEntry << "\n";
	}

	logEntry << "[" << timestamp << "] "
			 << levelToString(level) << ": "
			 << message << "\n";

	//  Output to console
	if (level == LogLevel::CRITICAL || level == LogLevel::ERROR) {
		std::cerr << logEntry.str() << std::endl;
	} else {
		std::cout << logEntry.str() << std::endl;
	}

	// Output to log file
	if (logFile.is_open()) {
		logFile << logEntry.str();
		logFile.flush(); //Ensure immediate write to the file.
	}
	return 0;
}

std::string Logger::levelToString(LogLevel level)
{
	switch (level) {
		case LogLevel::DEBUG:
			return "DEBUG";
		case LogLevel::INFO:
			return "INFO";
		case LogLevel::WARNING:
			return "WARNING";
		case LogLevel::ERROR:
			return "ERROR";
		case LogLevel::CRITICAL:
			return "CRITICAL";
		default:
			return "UNKNOWN ERROR LEVEL";
	}
}
