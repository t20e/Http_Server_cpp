#pragma once
#include <map>
#include <string>


/**
 * @brief Class to handle JSON response
 * 
 */
class JsonResponse {
	public:
		/**
         * @brief Add key-value pairs to the vector
         * 
         */
		void add(const std::string &key, const std::string &value);

        /**
         * @brief Overload function to add integer value
         * 
         * @param key 
         * @param value 
         */
		void add(const std::string &key, const int &value);


        /**
         * @brief Convert the vector of key-value pairs into a JSON string.
         * 
         * @return std::string 
         */
		std::string dump() const;

	private:
        // store the key value pairs
		std::map<std::string, std::string> data_;
};
