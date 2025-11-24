#include <cstddef>
#include <format>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "utils/JsonResponse.h"

void JsonResponse::add(const std::string &key, const std::string &value)
{
	data_[key] = value;
}

void JsonResponse::add(const std::string &key, const int &value)
{
	data_[key] = value;
}

void JsonResponse::add(const std::string &key, const std::vector<JsonResponse> &value)
{
	data_[key] = value;
}

std::string JsonResponse::dump() const
{
	std::stringstream ss; // Start string stream
	ss << "{"; // Add the first '{'

	auto i = data_.begin();
	while (i != data_.end()) {
		const auto &key = i->first;
		const auto &value = i->second;

		// Write the main key
		ss << std::format("\"{}\": ", key);

		// Write the value based on if its a vector, int, or a string
		if (std::holds_alternative<int>(value)) {
			// int
			ss << std::get<int>(value);
		} else if (std::holds_alternative<std::string>(value)) {
			// string
			ss << std::format("\"{}\"", std::get<std::string>(value));
		} else if (std::holds_alternative<std::vector<JsonResponse>>(value)) {
			// Vector of Json Responses, wrap in [] braces
			const auto &list = std::get<std::vector<JsonResponse>>(value);
			ss << "[";
			for (size_t k = 0; k < list.size(); ++k) {
				ss << list[k].dump(); // Call it recursively

				if (k < list.size() - 1) { // Add a comma if it's not the last element of the vector of JsonResponses.
					ss << ", ";
				}
			}
			ss << "]";
		}

		if (++i != data_.end()) { // Add a comma if it's not the last key in the map.
			ss << ", ";
		}
	}

	ss << "}"; // add the end '}'
	return ss.str();
}
