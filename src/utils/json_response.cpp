#include <sstream>
#include <format>

#include "utils/json_response.h"

void JsonResponse::add(const std::string &key, const std::string &value)
{
	data_[key] = value;
}

void JsonResponse::add(const std::string &key, const int &value)
{
	data_[key] = std::to_string(value);
}

std::string JsonResponse::dump() const
{
	std::stringstream ss;
	ss << "{";
	auto i = data_.begin();
	while (i != data_.end()) {
		ss << std::format("\"{}\": \"{}\"", i->first, i->second);

		if (++i != data_.end()) { // Add comma is not the last element
			ss << ", ";
		}
	}
	ss << "}";
	return ss.str();
}
