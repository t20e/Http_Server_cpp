
/*
Header file contains type alias for all function return types.
*/ 

#pragma once
#include <expected>
#include <string>
#include <vector>

#include "utils/http_data.h"


/**
 * @brief Database Return result. It returns an error string, if an error occurs or a User structure. 
 */
using DbResult = std::expected<User, std::string>;

/**
 * @brief Database Return result. It returns either an array of User objects or a string containing an error message.
 */
using DbListResult = std::expected<std::vector<User>, std::string>;

