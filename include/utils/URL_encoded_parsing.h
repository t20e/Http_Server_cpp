#pragma once

#include <sstream>
#include <unordered_map>

/**
 * @brief Helper function to decode strings, e.g., 'John%20Doe' converts into 'John Doe'. '%20' represents a white-space.
 * 
 * @param encodedStr The encoded string, e.g., 'John%20Doe'
 * @return std::string 
 */
inline std::string urlDecode(const std::string &encodedStr)
{
	std::string s;
	char ch;
	// std::string::size_type i;
	int ii;

	for (std::string::size_type i = 0; i < encodedStr.length(); i++) {
		if (encodedStr[i] == '%') { // example -> 'John%20Doe'
			if (i + 2 < encodedStr.length()) {
				std::istringstream hex_stream(encodedStr.substr(i + 1, 2));
				if (hex_stream >> std::hex >> ii) {
					ch = static_cast<char>(ii);
					s += ch;
					i += 2; // skip two hex characters, i.e., the 20 after the %
				}
			}
		} else if (encodedStr[i] == '+') { // In HTML forms whitespaces can also be represented as a '+'
			s += ' ';
		} else { //regular letters
			s += encodedStr[i];
		}
	}
	return s;
}


/**
 * @brief Parse a request body in URLEncodedForm (not JSON), and convert the data into a vector of key-value pairs, e.g., "username=johnDoe&password=test%40" turns into {'username':'johnDoe', 'password':'test@'}
 * 
 * @param body Raw URLEncodedForm string from a Request Body.
 * @return std::string 
 */
inline std::unordered_map<std::string, std::string> parseURLEncodedBody(const std::string &body)
{
	std::unordered_map<std::string, std::string> req_data;
	std::stringstream ss(body);
	std::string pair;

	while (std::getline(ss, pair, '&')) { // Split the key-value pairs by '&', e.g.,"username=JohnDoe&password=test"
		size_t equal_pos = pair.find('=');
		if (equal_pos != std::string::npos) {
			std::string key = pair.substr(0, equal_pos);
			std::string decodedKey = urlDecode(key);

			std::string value = pair.substr(equal_pos + 1);
			std::string decodedValue = urlDecode(value);

			req_data[decodedKey] = decodedValue;
		}
	}
	return req_data;
}
