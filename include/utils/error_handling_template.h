#pragma once
#include <expected>
#include <string>
#include <vector>

#include "utils/http_data_types.h"

// using result = std::variant<std::vector<unsigned char>, std::string>;
// using result = std::tuple<bool, std::string>;

/**
 * @brief Type alias for a result from a function. 
 * @note It contains an error string if an error occurs or is void on success. 
 */
using FuncResult = std::expected<void, std::string>;

/**
 * @brief Type alias for a result from a function. 
 * @note It contains an error string if an error occurs or a User structure. 
 */
using DbResult = std::expected<User, std::string>;

/**
 * @brief Return result, it is either a list of users or a string containing an error message.
 */
using DbListResult = std::expected<std::vector<User>, std::string>;

