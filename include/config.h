
#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "utils/Logger.h"


inline std::string trim_whitespaces(const std::string &str)
{
	size_t first = str.find_first_not_of(" \t");
	if (std::string::npos == first)
		return str;
	size_t last = str.find_last_not_of(" \t");
	return str.substr(first, (last - first + 1));
}

inline bool toBool(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return (str == "true" || str == "1");
}

// Split strings by commas to set, e.g., 127.0.0.1,::1
inline std::set<std::string> toSet(const std::string &str)
{
	std::set<std::string> result;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, ',')) {
		result.insert(trim_whitespaces(item));
	}
	return result;
}


struct Config {
	public:
		int SERVER_PORT;
		bool DEV_MODE;
		bool ENABLE_MULTI_THREADING;
		std::string JWT_SECRET_KEY;
		std::set<std::string> ALLOWED_ORIGINS;
		std::set<std::string> ALLOWED_IPs;

        /**
         * @brief Load the environment variables from a .env file.
         * 
         * @param filename 
         * @return Config 
         */
		static Config load(const std::string &filename)
		{
			Config config;
			std::ifstream file(filename);
			std::map<std::string, std::string> envMap;
			std::string line;

			if (!file.is_open()) {
				Logger::getInstance().log(LogLevel::ERROR, "Unable to open .env file!");
				return config;
			}

			// Parse file into map
			while (std::getline(file, line)) {
				// Remove comments
				size_t commentStartPos = line.find('#');
				if (commentStartPos != std::string::npos) {
					line = line.substr(0, commentStartPos);
				}

				line = trim_whitespaces(line);
				if (line.empty())
					continue; // Skip empty lines

				size_t delimiterPos = line.find('=');
				if (delimiterPos != std::string::npos) {
					std::string key = trim_whitespaces(line.substr(0, delimiterPos));
					std::string value = trim_whitespaces(line.substr(delimiterPos + 1));
					envMap[key] = value;
				}
			}

			// Map strings to Config Struct
			if (envMap.count("SERVER_PORT")) {
				config.SERVER_PORT = std::stoi(envMap["SERVER_PORT"]);
			} else {
				Logger::getInstance().log(LogLevel::CRITICAL, "SERVER_PORT not set in .env!");
			}

			if (envMap.count("DEV_MODE")) {
				config.DEV_MODE = toBool(envMap["DEV_MODE"]);
			} else {
				Logger::getInstance().log(LogLevel::CRITICAL, "DEV_MODE not set in .env!");
			}

			if (envMap.count("ENABLE_MULTI_THREADING")) {
				config.ENABLE_MULTI_THREADING = toBool(envMap["ENABLE_MULTI_THREADING"]);
			} else {
				Logger::getInstance().log(LogLevel::CRITICAL, "ENABLE_MULTI_THREADING not set in .env!");
			}

			if (envMap.count("JWT_SECRET_KEY")) {
				config.JWT_SECRET_KEY = envMap["JWT_SECRET_KEY"];
			} else {
				Logger::getInstance().log(LogLevel::CRITICAL, "JWT_SECRET_KEY not set in .env!");
			}

			if (envMap.count("ALLOWED_ORIGINS")) {
				config.ALLOWED_ORIGINS = toSet(envMap["ALLOWED_ORIGINS"]);
			} else {
				Logger::getInstance().log(LogLevel::CRITICAL, "ALLOWED_ORIGINS not set in .env!");
			}

			if (envMap.count("ALLOWED_IPs")) {
				config.ALLOWED_IPs = toSet(envMap["ALLOWED_IPs"]);
			} else {
				Logger::getInstance().log(LogLevel::CRITICAL, "ALLOWED_IPs not set in .env!");
			}

			return config;
		}

	private:
};
