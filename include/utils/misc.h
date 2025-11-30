#pragma once
#include <string>

/**
 * @brief Validate if the request origin is present in the list of allowed origins.
 * 
 * @param allowed_origins A pointer to the first string element of the allowed origins array.
 * @param requestOrigin The origin of a request, e.g., "http://localhost:3000".
 * @return true If the origin is allowed
 * @return false If the origin is not allowed.
 */
inline bool validateOrigin(std::string allowed_origins[], std::string requestOrigin)
{
	for (size_t i = 0; i < allowed_origins->size(); ++i) {

		if (allowed_origins[i] == requestOrigin) {
			return true;
		}
	}
	return false;
}
