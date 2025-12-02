#pragma once
#include <functional>
#include <string>
#include <unordered_map>


struct User {
		int userId;
		std::string username;
		std::string passwordHash;
};

/**
 * @brief Struct to store the parsed components of an incoming HTTP request.
 * 
 */
struct HttpRequest {
		std::string method;
		std::string path;
		std::string http_version;
		int content_length = 0;
		std::string content_type;

		std::string body;

        std::string client_ip; // Note: I add this to HttpRequest, when I accept a new HTTP connection. 

		std::unordered_map<std::string, std::string> headers;
		// headers example:
		// Key: Accept-Language, Value:  en-US,en;q=0.9,de;q=0.8
		// Key: Accept-Encoding, Value:  gzip, deflate, br, zstd
		// Key: Referer, Value:  http://localhost:3000/
		// Key: Sec-Fetch-Dest, Value:  empty
		// Key: Sec-Fetch-Mode, Value:  cors
		// Key: sec-ch-ua-mobile, Value:  ?0
		// Key: Content-Type, Value:  application/x-www-form-urlencoded;charset=UTF-8
		// Key: Connection, Value:  keep-alive
		// Key: Sec-Fetch-Site, Value:  same-site
		// Key: Content-Length, Value:  36
		// Key: sec-ch-ua-platform, Value:  "macOS"
		// Key: Origin, Value:  http://localhost:3000
		// Key: Cookie, Value:  session_token=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXUyJ9.eyJleHAiOjE3NjQ1MTk0MTEsImlhdCI6MTc2NDQzMzAxMSwiaXNzIjoiYXV0aDAiLCJyb2xlIjoidXNlciIsInN1YiI6IjkifQ.r392w_1Id0PgNF1YODGRqzzJ6TVnjQsWwM8TJUDb8t4
		// Key: Accept, Value:  application/json, text/plain, */*
		// Key: User-Agent, Value:  Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36
		//Key: sec-ch-ua, Value:  "Chromium";v="142", "Google Chrome";v="142", "Not_A Brand";v="99"
		// Key: Host, Value:  localhost:8080

};

/**
 * @brief Accepts the parse request and the client socket ID
 * 
 */
using HttpHandler = std::function<void(const HttpRequest &req, const int &clientSocket)>;

/**
 * @brief Get the Reason Phrase for a response status code.
 * 
 * @param status 
 * @return std::string 
 */
inline std::string getCodePhrase(int status)
{
	switch (status) {
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 400:
			return "Bad Request";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 500:
			return "Internal Server Error";
		default:
			return "Unknown Status";
	}
}
