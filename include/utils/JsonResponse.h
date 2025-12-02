#pragma once
#include <map>
#include <string>
#include <variant>
#include <vector>

/**
 * @brief Class handles creating a JSON response payload, representing a single json object, that can contain either a single string, a single integer, or an array of JsonResponse objects.
 *
 */
class JsonResponse {
	public:
		/**
        * @brief Type alias. The value (of key:value pairs) can either be a string, an int, or a vector of nested JsonResponse objects.
        */
		using JsonValue = std::variant<std::string, int, std::vector<JsonResponse>>;

		/**
        * @brief Add key-value string pairs to the vector.
        */
		void add(const std::string &key, const std::string &value);

		/**
        * @brief Overload function to add key-value pairs but the value is an integer.
        *
        * @param key String
        * @param value integer
        */
		void add(const std::string &key, const int &value);

		/**
        * @brief Overload function to add key-value pairs but the value is a vector of JsonResponse objects.
        *
        * @param key String
        * @param value integer
        */
		void add(const std::string &key, const std::vector<JsonResponse> &value);

		/**
        * @brief Convert the stored vector of key-value pairs into a JSON string.
        *
        * @return std::string
        */
		std::string dump() const;

	private:
		// store the key value pairs
		std::map<std::string, JsonValue> data_;
};
