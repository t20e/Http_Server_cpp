#pragma once
#include <map>
#include <string>
#include <variant>
#include <vector>

/**
 * @brief Class to handle JSON response
 * 
 */
class JsonResponse {
	public:
		/**
         * @brief Type alias return type. It can either return a string, or an int, or a vector of this same JsonResponse class.
         * 
         */
		using JsonValue = std::variant<std::string, int, std::vector<JsonResponse>>;

		/**
         * @brief Add key-value string pairs to the vector.
         * 
         */
		void add(const std::string &key, const std::string &value);

		/**
         * @brief Overload function to add key-value pairs but key is a string and value is an integer.
         * 
         * @param key String
         * @param value integer
         */
		void add(const std::string &key, const int &value);

		/**
         * @brief Overload function to add key-value pairs but key is a string and value is a vector of Json Responses. // FIXME this class is very confusing, tweak it!
         * 
         * @param key String
         * @param value integer
         */
		void add(const std::string &key, const std::vector<JsonResponse> &value);

		/**
         * @brief Convert the vector of key-value pairs into a JSON string.
         * 
         * @return std::string 
         */
		std::string dump() const;

	private:
		// store the key value pairs
		std::map<std::string, JsonValue> data_;
};
